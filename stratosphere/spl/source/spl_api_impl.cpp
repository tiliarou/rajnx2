/*
 * Copyright (c) 2018-2019 Atmosphère-NX
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <switch.h>
#include <stratosphere.hpp>
#include <stratosphere/spl.hpp>

#include "spl_api_impl.hpp"

#include "spl_ctr_drbg.hpp"

namespace sts::spl::impl {

    namespace {

        /* Convenient defines. */
        constexpr size_t DeviceAddressSpaceAlignSize = 0x400000;
        constexpr size_t DeviceAddressSpaceAlignMask = DeviceAddressSpaceAlignSize - 1;
        constexpr u32 WorkBufferMapBase  = 0x80000000u;
        constexpr u32 CryptAesInMapBase  = 0x90000000u;
        constexpr u32 CryptAesOutMapBase = 0xC0000000u;
        constexpr size_t CryptAesSizeMax = static_cast<size_t>(CryptAesOutMapBase - CryptAesInMapBase);

        constexpr size_t RsaPrivateKeySize = 0x100;
        constexpr size_t RsaPrivateKeyMetaSize = 0x30;
        constexpr size_t LabelDigestSizeMax = 0x20;

        constexpr size_t WorkBufferSizeMax = 0x800;

        constexpr size_t MaxAesKeyslots = 6;
        constexpr size_t MaxAesKeyslotsDeprecated = 4;

        /* Max Keyslots helper. */
        inline size_t GetMaxKeyslots() {
            return (GetRuntimeFirmwareVersion() >= FirmwareVersion_600) ? MaxAesKeyslots : MaxAesKeyslotsDeprecated;
        }

        /* Type definitions. */
        class ScopedAesKeyslot {
            private:
                u32 slot;
                bool has_slot;
            public:
                ScopedAesKeyslot() : slot(0), has_slot(false) {
                    /* ... */
                }
                ~ScopedAesKeyslot() {
                    if (has_slot) {
                        FreeAesKeyslot(slot, this);
                    }
                }

                u32 GetKeyslot() const {
                    return this->slot;
                }

                Result Allocate() {
                    R_TRY(AllocateAesKeyslot(&this->slot, this));
                    this->has_slot = true;
                    return ResultSuccess;
                }
        };

        struct SeLinkedListEntry {
            u32 num_entries;
            u32 address;
            u32 size;
        };

        struct SeCryptContext {
            SeLinkedListEntry in;
            SeLinkedListEntry out;
        };

        class DeviceAddressSpaceMapHelper {
            private:
                Handle das_hnd;
                u64 dst_addr;
                u64 src_addr;
                size_t size;
                u32 perm;
            public:
                DeviceAddressSpaceMapHelper(Handle h, u64 dst, u64 src, size_t sz, u32 p) : das_hnd(h), dst_addr(dst), src_addr(src), size(sz), perm(p) {
                    R_ASSERT(svcMapDeviceAddressSpaceAligned(this->das_hnd, CUR_PROCESS_HANDLE, this->src_addr, this->size, this->dst_addr, this->perm));
                }
                ~DeviceAddressSpaceMapHelper() {
                    R_ASSERT(svcUnmapDeviceAddressSpace(this->das_hnd, CUR_PROCESS_HANDLE, this->src_addr, this->size, this->dst_addr));
                }
        };

        /* Global variables. */
        CtrDrbg g_drbg;
        Event g_se_event;
        IEvent *g_se_keyslot_available_event;

        Handle g_se_das_hnd;
        u32 g_se_mapped_work_buffer_addr;
        u8 __attribute__((aligned(0x1000))) g_work_buffer[2 * WorkBufferSizeMax];

        HosMutex g_async_op_lock;

        const void *g_keyslot_owners[MaxAesKeyslots];
        BootReasonValue g_boot_reason;
        bool g_boot_reason_set;

        /* Boot Reason accessors. */
        BootReasonValue GetBootReason() {
            return g_boot_reason;
        }

        bool IsBootReasonSet() {
            return g_boot_reason_set;
        }

        /* Initialization functionality. */
        void InitializeCtrDrbg() {
            u8 seed[CtrDrbg::SeedSize];

            if (smc::GenerateRandomBytes(seed, sizeof(seed)) != smc::Result::Success) {
                std::abort();
            }

            g_drbg.Initialize(seed);
        }

        void InitializeSeEvents() {
            u64 irq_num;
            smc::GetConfig(&irq_num, 1, SplConfigItem_SecurityEngineIrqNumber);
            Handle hnd;
            R_ASSERT(svcCreateInterruptEvent(&hnd, irq_num, 1));
            eventLoadRemote(&g_se_event, hnd, true);

            g_se_keyslot_available_event = CreateWriteOnlySystemEvent();
            g_se_keyslot_available_event->Signal();
        }

        void InitializeDeviceAddressSpace() {
            constexpr u64 DeviceName_SE = 29;

            /* Create Address Space. */
            R_ASSERT(svcCreateDeviceAddressSpace(&g_se_das_hnd, 0, (1ul << 32)));

            /* Attach it to the SE. */
            R_ASSERT(svcAttachDeviceAddressSpace(DeviceName_SE, g_se_das_hnd));

            const u64 work_buffer_addr = reinterpret_cast<u64>(g_work_buffer);
            g_se_mapped_work_buffer_addr = WorkBufferMapBase + (work_buffer_addr & DeviceAddressSpaceAlignMask);

            /* Map the work buffer for the SE. */
            R_ASSERT(svcMapDeviceAddressSpaceAligned(g_se_das_hnd, CUR_PROCESS_HANDLE, work_buffer_addr, sizeof(g_work_buffer), g_se_mapped_work_buffer_addr, 3));
        }

        /* RSA OAEP implementation helpers. */
        void CalcMgf1AndXor(void *dst, size_t dst_size, const void *src, size_t src_size) {
            uint8_t *dst_u8 = reinterpret_cast<u8 *>(dst);

            u32 ctr = 0;
            while (dst_size > 0) {
                size_t cur_size = SHA256_HASH_SIZE;
                if (cur_size > dst_size) {
                    cur_size = dst_size;
                }
                dst_size -= cur_size;

                u32 ctr_be = __builtin_bswap32(ctr++);
                u8 hash[SHA256_HASH_SIZE];
                {
                    Sha256Context ctx;
                    sha256ContextCreate(&ctx);
                    sha256ContextUpdate(&ctx, src, src_size);
                    sha256ContextUpdate(&ctx, &ctr_be, sizeof(ctr_be));
                    sha256ContextGetHash(&ctx, hash);
                }

                for (size_t i = 0; i < cur_size; i++) {
                    *(dst_u8++) ^= hash[i];
                }
            }
        }

        size_t DecodeRsaOaep(void *dst, size_t dst_size, const void *label_digest, size_t label_digest_size, const void *src, size_t src_size) {
            /* Very basic validation. */
            if (dst_size == 0 || src_size != 0x100 || label_digest_size != SHA256_HASH_SIZE) {
                return 0;
            }

            u8 block[0x100];
            std::memcpy(block, src, sizeof(block));

            /* First, validate byte 0 == 0, and unmask DB. */
            int invalid = block[0];
            u8 *salt = block + 1;
            u8 *db = salt + SHA256_HASH_SIZE;
            CalcMgf1AndXor(salt, SHA256_HASH_SIZE, db, src_size - (1 + SHA256_HASH_SIZE));
            CalcMgf1AndXor(db, src_size - (1 + SHA256_HASH_SIZE), salt, SHA256_HASH_SIZE);

            /* Validate label digest. */
            for (size_t i = 0; i < SHA256_HASH_SIZE; i++) {
                invalid |= db[i] ^ reinterpret_cast<const u8 *>(label_digest)[i];
            }

            /* Locate message after 00...0001 padding. */
            const u8 *padded_msg = db + SHA256_HASH_SIZE;
            size_t padded_msg_size = src_size - (1 + 2 * SHA256_HASH_SIZE);
            size_t msg_ind = 0;
            int not_found = 1;
            int wrong_padding = 0;
            size_t i = 0;
            while (i < padded_msg_size) {
                int zero = (padded_msg[i] == 0);
                int one = (padded_msg[i] == 1);
                msg_ind += static_cast<size_t>(not_found & one) * (++i);
                not_found &= ~one;
                wrong_padding |= (not_found & ~zero);
            }

            if (invalid | not_found | wrong_padding) {
                return 0;
            }

            /* Copy message out. */
            size_t msg_size = padded_msg_size - msg_ind;
            if (msg_size > dst_size) {
                return 0;
            }
            std::memcpy(dst, padded_msg + msg_ind, msg_size);
            return msg_size;
        }

        /* Internal RNG functionality. */
        Result GenerateRandomBytesInternal(void *out, size_t size) {
            if (!g_drbg.GenerateRandomBytes(out, size)) {
                /* We need to reseed. */
                {
                    u8 seed[CtrDrbg::SeedSize];

                    smc::Result res = smc::GenerateRandomBytes(seed, sizeof(seed));
                    if (res != smc::Result::Success) {
                        return smc::ConvertResult(res);
                    }

                    g_drbg.Reseed(seed);
                    g_drbg.GenerateRandomBytes(out, size);
                }
            }

            return ResultSuccess;
        }

        /* Internal async implementation functionality. */
        void WaitSeOperationComplete() {
            eventWait(&g_se_event, U64_MAX);
        }

        smc::Result WaitCheckStatus(smc::AsyncOperationKey op_key) {
            WaitSeOperationComplete();

            smc::Result op_res;
            smc::Result res = smc::CheckStatus(&op_res, op_key);
            if (res != smc::Result::Success) {
                return res;
            }

            return op_res;
        }

        smc::Result WaitGetResult(void *out_buf, size_t out_buf_size, smc::AsyncOperationKey op_key) {
            WaitSeOperationComplete();

            smc::Result op_res;
            smc::Result res = smc::GetResult(&op_res, out_buf, out_buf_size, op_key);
            if (res != smc::Result::Success) {
                return res;
            }

            return op_res;
        }

        /* Internal Keyslot utility. */
        Result ValidateAesKeyslot(u32 keyslot, const void *owner) {
            if (keyslot >= GetMaxKeyslots()) {
                return ResultSplInvalidKeyslot;
            }
            if (g_keyslot_owners[keyslot] != owner && GetRuntimeFirmwareVersion() > FirmwareVersion_100) {
                return ResultSplInvalidKeyslot;
            }
            return ResultSuccess;
        }

        /* Helper to do a single AES block decryption. */
        smc::Result DecryptAesBlock(u32 keyslot, void *dst, const void *src) {
            struct DecryptAesBlockLayout {
                SeCryptContext crypt_ctx;
                u8 in_block[AES_BLOCK_SIZE] __attribute__((aligned(AES_BLOCK_SIZE)));
                u8 out_block[AES_BLOCK_SIZE] __attribute__((aligned(AES_BLOCK_SIZE)));
            };
            DecryptAesBlockLayout *layout = reinterpret_cast<DecryptAesBlockLayout *>(g_work_buffer);

            layout->crypt_ctx.in.num_entries = 0;
            layout->crypt_ctx.in.address = g_se_mapped_work_buffer_addr + offsetof(DecryptAesBlockLayout, in_block);
            layout->crypt_ctx.in.size = sizeof(layout->in_block);
            layout->crypt_ctx.out.num_entries = 0;
            layout->crypt_ctx.out.address = g_se_mapped_work_buffer_addr + offsetof(DecryptAesBlockLayout, out_block);
            layout->crypt_ctx.out.size = sizeof(layout->out_block);

            std::memcpy(layout->in_block, src, sizeof(layout->in_block));

            armDCacheFlush(layout, sizeof(*layout));
            {
                std::scoped_lock<HosMutex> lk(g_async_op_lock);
                smc::AsyncOperationKey op_key;
                const IvCtr iv_ctr = {};
                const u32 mode = smc::GetCryptAesMode(smc::CipherMode::CbcDecrypt, keyslot);
                const u32 dst_ll_addr = g_se_mapped_work_buffer_addr + offsetof(DecryptAesBlockLayout, crypt_ctx.out);
                const u32 src_ll_addr = g_se_mapped_work_buffer_addr + offsetof(DecryptAesBlockLayout, crypt_ctx.in);

                smc::Result res = smc::CryptAes(&op_key, mode, iv_ctr, dst_ll_addr, src_ll_addr, sizeof(layout->in_block));
                if (res != smc::Result::Success) {
                    return res;
                }

                if ((res = WaitCheckStatus(op_key)) != smc::Result::Success) {
                    return res;
                }
            }
            armDCacheFlush(layout, sizeof(*layout));

            std::memcpy(dst, layout->out_block, sizeof(layout->out_block));
            return smc::Result::Success;
        }

        /* Implementation wrappers for API commands. */
        Result ImportSecureExpModKey(const void *src, size_t src_size, const AccessKey &access_key, const KeySource &key_source, u32 option) {
            struct ImportSecureExpModKeyLayout {
                u8 data[RsaPrivateKeyMetaSize + 2 * RsaPrivateKeySize + 0x10];
            };
            ImportSecureExpModKeyLayout *layout = reinterpret_cast<ImportSecureExpModKeyLayout *>(g_work_buffer);

            /* Validate size. */
            if (src_size > sizeof(ImportSecureExpModKeyLayout)) {
                return ResultSplInvalidSize;
            }

            std::memcpy(layout, src, src_size);

            armDCacheFlush(layout, sizeof(*layout));
            smc::Result smc_res;
            if (GetRuntimeFirmwareVersion() >= FirmwareVersion_500) {
                smc_res = smc::DecryptOrImportRsaPrivateKey(layout->data, src_size, access_key, key_source, static_cast<smc::DecryptOrImportMode>(option));
            } else {
                smc_res = smc::ImportSecureExpModKey(layout->data, src_size, access_key, key_source, option);
            }

            return smc::ConvertResult(smc_res);
        }

        Result SecureExpMod(void *out, size_t out_size, const void *base, size_t base_size, const void *mod, size_t mod_size, smc::SecureExpModMode mode) {
            struct SecureExpModLayout {
                u8 base[0x100];
                u8 mod[0x100];
            };
            SecureExpModLayout *layout = reinterpret_cast<SecureExpModLayout *>(g_work_buffer);

            /* Validate sizes. */
            if (base_size > sizeof(layout->base)) {
                return ResultSplInvalidSize;
            }
            if (mod_size > sizeof(layout->mod)) {
                return ResultSplInvalidSize;
            }
            if (out_size > WorkBufferSizeMax) {
                return ResultSplInvalidSize;
            }

            /* Copy data into work buffer. */
            const size_t base_ofs = sizeof(layout->base) - base_size;
            const size_t mod_ofs = sizeof(layout->mod) - mod_size;
            std::memset(layout, 0, sizeof(*layout));
            std::memcpy(layout->base + base_ofs, base, base_size);
            std::memcpy(layout->mod + mod_ofs, mod, mod_size);

            /* Do exp mod operation. */
            armDCacheFlush(layout, sizeof(*layout));
            {
                std::scoped_lock<HosMutex> lk(g_async_op_lock);
                smc::AsyncOperationKey op_key;

                smc::Result res = smc::SecureExpMod(&op_key, layout->base, layout->mod, mode);
                if (res != smc::Result::Success) {
                    return smc::ConvertResult(res);
                }

                if ((res = WaitGetResult(g_work_buffer, out_size, op_key)) != smc::Result::Success) {
                    return smc::ConvertResult(res);
                }
            }
            armDCacheFlush(g_work_buffer, sizeof(out_size));

            std::memcpy(out, g_work_buffer, out_size);
            return ResultSuccess;
        }

        Result UnwrapEsRsaOaepWrappedKey(AccessKey *out_access_key, const void *base, size_t base_size, const void *mod, size_t mod_size, const void *label_digest, size_t label_digest_size, u32 generation, smc::EsKeyType type) {
            struct UnwrapEsKeyLayout {
                u8 base[0x100];
                u8 mod[0x100];
            };
            UnwrapEsKeyLayout *layout = reinterpret_cast<UnwrapEsKeyLayout *>(g_work_buffer);

            /* Validate sizes. */
            if (base_size > sizeof(layout->base)) {
                return ResultSplInvalidSize;
            }
            if (mod_size > sizeof(layout->mod)) {
                return ResultSplInvalidSize;
            }
            if (label_digest_size > LabelDigestSizeMax) {
                return ResultSplInvalidSize;
            }

            /* Copy data into work buffer. */
            const size_t base_ofs = sizeof(layout->base) - base_size;
            const size_t mod_ofs = sizeof(layout->mod) - mod_size;
            std::memset(layout, 0, sizeof(*layout));
            std::memcpy(layout->base + base_ofs, base, base_size);
            std::memcpy(layout->mod + mod_ofs, mod, mod_size);

            /* Do exp mod operation. */
            armDCacheFlush(layout, sizeof(*layout));
            {
                std::scoped_lock<HosMutex> lk(g_async_op_lock);
                smc::AsyncOperationKey op_key;

                smc::Result res = smc::UnwrapTitleKey(&op_key, layout->base, layout->mod, label_digest, label_digest_size, smc::GetUnwrapEsKeyOption(type, generation));
                if (res != smc::Result::Success) {
                    return smc::ConvertResult(res);
                }

                if ((res = WaitGetResult(g_work_buffer, sizeof(*out_access_key), op_key)) != smc::Result::Success) {
                    return smc::ConvertResult(res);
                }
            }
            armDCacheFlush(g_work_buffer, sizeof(*out_access_key));

            std::memcpy(out_access_key, g_work_buffer, sizeof(*out_access_key));
            return ResultSuccess;
        }


    }

    /* Initialization. */
    void Initialize() {
        /* Initialize the Drbg. */
        InitializeCtrDrbg();
        /* Initialize SE interrupt + keyslot events. */
        InitializeSeEvents();
        /* Initialize DAS for the SE. */
        InitializeDeviceAddressSpace();
    }

    /* General. */
    Result GetConfig(u64 *out, SplConfigItem which) {
        /* Nintendo explicitly blacklists package2 hash here, amusingly. */
        /* This is not blacklisted in safemode, but we're never in safe mode... */
        if (which == SplConfigItem_Package2Hash) {
            return ResultSplInvalidArgument;
        }

        smc::Result res = smc::GetConfig(out, 1, which);

        /* Nintendo has some special handling here for hardware type/is_retail. */
        if (which == SplConfigItem_HardwareType && res == smc::Result::InvalidArgument) {
            *out = 0;
            res = smc::Result::Success;
        }
        if (which == SplConfigItem_IsRetail && res == smc::Result::InvalidArgument) {
            *out = 0;
            res = smc::Result::Success;
        }

        return smc::ConvertResult(res);
    }

    Result ExpMod(void *out, size_t out_size, const void *base, size_t base_size, const void *exp, size_t exp_size, const void *mod, size_t mod_size) {
        struct ExpModLayout {
            u8 base[0x100];
            u8 exp[0x100];
            u8 mod[0x100];
        };
        ExpModLayout *layout = reinterpret_cast<ExpModLayout *>(g_work_buffer);

        /* Validate sizes. */
        if (base_size > sizeof(layout->base)) {
            return ResultSplInvalidSize;
        }
        if (exp_size > sizeof(layout->exp)) {
            return ResultSplInvalidSize;
        }
        if (mod_size > sizeof(layout->mod)) {
            return ResultSplInvalidSize;
        }
        if (out_size > WorkBufferSizeMax) {
            return ResultSplInvalidSize;
        }

        /* Copy data into work buffer. */
        const size_t base_ofs = sizeof(layout->base) - base_size;
        const size_t mod_ofs = sizeof(layout->mod) - mod_size;
        std::memset(layout, 0, sizeof(*layout));
        std::memcpy(layout->base + base_ofs, base, base_size);
        std::memcpy(layout->exp, exp, exp_size);
        std::memcpy(layout->mod + mod_ofs, mod, mod_size);

        /* Do exp mod operation. */
        armDCacheFlush(layout, sizeof(*layout));
        {
            std::scoped_lock<HosMutex> lk(g_async_op_lock);
            smc::AsyncOperationKey op_key;

            smc::Result res = smc::ExpMod(&op_key, layout->base, layout->exp, exp_size, layout->mod);
            if (res != smc::Result::Success) {
                return smc::ConvertResult(res);
            }

            if ((res = WaitGetResult(g_work_buffer, out_size, op_key)) != smc::Result::Success) {
                return smc::ConvertResult(res);
            }
        }
        armDCacheFlush(g_work_buffer, sizeof(out_size));

        std::memcpy(out, g_work_buffer, out_size);
        return ResultSuccess;
    }

    Result SetConfig(SplConfigItem which, u64 value) {
        return smc::ConvertResult(smc::SetConfig(which, &value, 1));
    }

    Result GenerateRandomBytes(void *out, size_t size) {
        u8 *cur_dst = reinterpret_cast<u8 *>(out);

        for (size_t ofs = 0; ofs < size; ofs += CtrDrbg::MaxRequestSize) {
            const size_t cur_size = std::min(size - ofs, CtrDrbg::MaxRequestSize);

            R_TRY(GenerateRandomBytesInternal(cur_dst, size));
            cur_dst += cur_size;
        }

        return ResultSuccess;
    }

    Result IsDevelopment(bool *out) {
        u64 is_retail;
        R_TRY(GetConfig(&is_retail, SplConfigItem_IsRetail));

        *out = (is_retail == 0);
        return ResultSuccess;
    }

    Result SetBootReason(BootReasonValue boot_reason) {
        if (IsBootReasonSet()) {
            return ResultSplBootReasonAlreadySet;
        }

        g_boot_reason = boot_reason;
        g_boot_reason_set = true;
        return ResultSuccess;
    }

    Result GetBootReason(BootReasonValue *out) {
        if (!IsBootReasonSet()) {
            return ResultSplBootReasonNotSet;
        }

        *out = GetBootReason();
        return ResultSuccess;
    }

    /* Crypto. */
    Result GenerateAesKek(AccessKey *out_access_key, const KeySource &key_source, u32 generation, u32 option) {
        return smc::ConvertResult(smc::GenerateAesKek(out_access_key, key_source, generation, option));
    }

    Result LoadAesKey(u32 keyslot, const void *owner, const AccessKey &access_key, const KeySource &key_source) {
        R_TRY(ValidateAesKeyslot(keyslot, owner));
        return smc::ConvertResult(smc::LoadAesKey(keyslot, access_key, key_source));
    }

    Result GenerateAesKey(AesKey *out_key, const AccessKey &access_key, const KeySource &key_source) {
        smc::Result smc_rc;

        static const KeySource s_generate_aes_key_source = {
            .data = {0x89, 0x61, 0x5E, 0xE0, 0x5C, 0x31, 0xB6, 0x80, 0x5F, 0xE5, 0x8F, 0x3D, 0xA2, 0x4F, 0x7A, 0xA8}
        };

        ScopedAesKeyslot keyslot_holder;
        R_TRY(keyslot_holder.Allocate());

        smc_rc = smc::LoadAesKey(keyslot_holder.GetKeyslot(), access_key, s_generate_aes_key_source);
        if (smc_rc == smc::Result::Success) {
            smc_rc = DecryptAesBlock(keyslot_holder.GetKeyslot(), out_key, &key_source);
        }

        return smc::ConvertResult(smc_rc);
    }

    Result DecryptAesKey(AesKey *out_key, const KeySource &key_source, u32 generation, u32 option) {
        static const KeySource s_decrypt_aes_key_source = {
            .data = {0x11, 0x70, 0x24, 0x2B, 0x48, 0x69, 0x11, 0xF1, 0x11, 0xB0, 0x0C, 0x47, 0x7C, 0xC3, 0xEF, 0x7E}
        };

        AccessKey access_key;
        R_TRY(GenerateAesKek(&access_key, s_decrypt_aes_key_source, generation, option));

        return GenerateAesKey(out_key, access_key, key_source);
    }

    Result CryptAesCtr(void *dst, size_t dst_size, u32 keyslot, const void *owner, const void *src, size_t src_size, const IvCtr &iv_ctr) {
        R_TRY(ValidateAesKeyslot(keyslot, owner));

        /* Succeed immediately if there's nothing to crypt. */
        if (src_size == 0) {
            return ResultSuccess;
        }

        /* Validate sizes. */
        if (src_size > dst_size || src_size % AES_BLOCK_SIZE != 0) {
            return ResultSplInvalidSize;
        }

        /* We can only map 0x400000 aligned buffers for the SE. With that in mind, we have some math to do. */
        const uintptr_t src_addr = reinterpret_cast<uintptr_t>(src);
        const uintptr_t dst_addr = reinterpret_cast<uintptr_t>(dst);
        const uintptr_t src_addr_page_aligned = src_addr & ~0xFFFul;
        const uintptr_t dst_addr_page_aligned = dst_addr & ~0xFFFul;
        const size_t src_size_page_aligned = ((src_addr + src_size + 0xFFFul) & ~0xFFFul) - src_addr_page_aligned;
        const size_t dst_size_page_aligned = ((dst_addr + dst_size + 0xFFFul) & ~0xFFFul) - dst_addr_page_aligned;
        const u32 src_se_map_addr = CryptAesInMapBase + (src_addr_page_aligned & DeviceAddressSpaceAlignMask);
        const u32 dst_se_map_addr = CryptAesOutMapBase + (dst_addr_page_aligned & DeviceAddressSpaceAlignMask);
        const u32 src_se_addr = CryptAesInMapBase + (src_addr & DeviceAddressSpaceAlignMask);
        const u32 dst_se_addr = CryptAesOutMapBase + (dst_addr & DeviceAddressSpaceAlignMask);

        /* Validate aligned sizes. */
        if (src_size_page_aligned > CryptAesSizeMax || dst_size_page_aligned > CryptAesSizeMax) {
            return ResultSplInvalidSize;
        }

        /* Helpers for mapping/unmapping. */
        DeviceAddressSpaceMapHelper in_mapper(g_se_das_hnd,  src_se_map_addr, src_addr_page_aligned, src_size_page_aligned, 1);
        DeviceAddressSpaceMapHelper out_mapper(g_se_das_hnd, dst_se_map_addr, dst_addr_page_aligned, dst_size_page_aligned, 2);

        /* Setup SE linked list entries. */
        SeCryptContext *crypt_ctx = reinterpret_cast<SeCryptContext *>(g_work_buffer);
        crypt_ctx->in.num_entries = 0;
        crypt_ctx->in.address = src_se_addr;
        crypt_ctx->in.size = src_size;
        crypt_ctx->out.num_entries = 0;
        crypt_ctx->out.address = dst_se_addr;
        crypt_ctx->out.size = dst_size;

        armDCacheFlush(crypt_ctx, sizeof(*crypt_ctx));
        armDCacheFlush(const_cast<void *>(src), src_size);
        armDCacheFlush(dst, dst_size);
        {
            std::scoped_lock<HosMutex> lk(g_async_op_lock);
            smc::AsyncOperationKey op_key;
            const u32 mode = smc::GetCryptAesMode(smc::CipherMode::Ctr, keyslot);
            const u32 dst_ll_addr = g_se_mapped_work_buffer_addr + offsetof(SeCryptContext, out);
            const u32 src_ll_addr = g_se_mapped_work_buffer_addr + offsetof(SeCryptContext, in);

            smc::Result res = smc::CryptAes(&op_key, mode, iv_ctr, dst_ll_addr, src_ll_addr, src_size);
            if (res != smc::Result::Success) {
                return smc::ConvertResult(res);
            }

            if ((res = WaitCheckStatus(op_key)) != smc::Result::Success) {
                return smc::ConvertResult(res);
            }
        }
        armDCacheFlush(dst, dst_size);

        return ResultSuccess;
    }

    Result ComputeCmac(Cmac *out_cmac, u32 keyslot, const void *owner, const void *data, size_t size) {
        R_TRY(ValidateAesKeyslot(keyslot, owner));

        if (size > WorkBufferSizeMax) {
            return ResultSplInvalidSize;
        }

        std::memcpy(g_work_buffer, data, size);
        return smc::ConvertResult(smc::ComputeCmac(out_cmac, keyslot, g_work_buffer, size));
    }

    Result AllocateAesKeyslot(u32 *out_keyslot, const void *owner) {
        if (GetRuntimeFirmwareVersion() <= FirmwareVersion_100) {
            /* On 1.0.0, keyslots were kind of a wild west. */
            *out_keyslot = 0;
            return ResultSuccess;
        }

        for (size_t i = 0; i < GetMaxKeyslots(); i++) {
            if (g_keyslot_owners[i] == 0) {
                g_keyslot_owners[i] = owner;
                *out_keyslot = static_cast<u32>(i);
                return ResultSuccess;
            }
        }

        g_se_keyslot_available_event->Clear();
        return ResultSplOutOfKeyslots;
    }

    Result FreeAesKeyslot(u32 keyslot, const void *owner) {
        if (GetRuntimeFirmwareVersion() <= FirmwareVersion_100) {
            /* On 1.0.0, keyslots were kind of a wild west. */
            return ResultSuccess;
        }

        R_TRY(ValidateAesKeyslot(keyslot, owner));

        /* Clear the keyslot. */
        {
            AccessKey access_key = {};
            KeySource key_source = {};

            smc::LoadAesKey(keyslot, access_key, key_source);
        }
        g_keyslot_owners[keyslot] = nullptr;
        g_se_keyslot_available_event->Signal();
        return ResultSuccess;
    }

    /* RSA. */
    Result DecryptRsaPrivateKey(void *dst, size_t dst_size, const void *src, size_t src_size, const AccessKey &access_key, const KeySource &key_source, u32 option) {
        struct DecryptRsaPrivateKeyLayout {
            u8 data[RsaPrivateKeySize + RsaPrivateKeyMetaSize];
        };
        DecryptRsaPrivateKeyLayout *layout = reinterpret_cast<DecryptRsaPrivateKeyLayout *>(g_work_buffer);

        /* Validate size. */
        if (src_size < RsaPrivateKeyMetaSize || src_size > sizeof(DecryptRsaPrivateKeyLayout)) {
            return ResultSplInvalidSize;
        }

        std::memcpy(layout->data, src, src_size);
        armDCacheFlush(layout, sizeof(*layout));

        smc::Result smc_res;
        size_t copy_size = 0;
        if (GetRuntimeFirmwareVersion() >= FirmwareVersion_500) {
            copy_size = std::min(dst_size, src_size - RsaPrivateKeyMetaSize);
            smc_res = smc::DecryptOrImportRsaPrivateKey(layout->data, src_size, access_key, key_source, smc::DecryptOrImportMode::DecryptRsaPrivateKey);
        } else {
            smc_res = smc::DecryptRsaPrivateKey(&copy_size, layout->data, src_size, access_key, key_source, option);
            copy_size = std::min(dst_size, copy_size);
        }

        armDCacheFlush(layout, sizeof(*layout));
        if (smc_res == smc::Result::Success) {
            std::memcpy(dst, layout->data, copy_size);
        }

        return smc::ConvertResult(smc_res);
    }

    /* SSL */
    Result ImportSslKey(const void *src, size_t src_size, const AccessKey &access_key, const KeySource &key_source) {
        return ImportSecureExpModKey(src, src_size, access_key, key_source, static_cast<u32>(smc::DecryptOrImportMode::ImportSslKey));
    }

    Result SslExpMod(void *out, size_t out_size, const void *base, size_t base_size, const void *mod, size_t mod_size) {
        return SecureExpMod(out, out_size, base, base_size, mod, mod_size, smc::SecureExpModMode::Ssl);
    }

    /* ES */
    Result ImportEsKey(const void *src, size_t src_size, const AccessKey &access_key, const KeySource &key_source, u32 option) {
        if (GetRuntimeFirmwareVersion() >= FirmwareVersion_500) {
            return ImportSecureExpModKey(src, src_size, access_key, key_source, static_cast<u32>(smc::DecryptOrImportMode::ImportEsKey));
        } else {
            struct ImportEsKeyLayout {
                u8 data[RsaPrivateKeyMetaSize + 2 * RsaPrivateKeySize + 0x10];
            };
            ImportEsKeyLayout *layout = reinterpret_cast<ImportEsKeyLayout *>(g_work_buffer);

            /* Validate size. */
            if (src_size > sizeof(ImportEsKeyLayout)) {
                return ResultSplInvalidSize;
            }

            std::memcpy(layout, src, src_size);

            armDCacheFlush(layout, sizeof(*layout));
            return smc::ConvertResult(smc::ImportEsKey(layout->data, src_size, access_key, key_source, option));
        }
    }

    Result UnwrapTitleKey(AccessKey *out_access_key, const void *base, size_t base_size, const void *mod, size_t mod_size, const void *label_digest, size_t label_digest_size, u32 generation) {
        return UnwrapEsRsaOaepWrappedKey(out_access_key, base, base_size, mod, mod_size, label_digest, label_digest_size, generation, smc::EsKeyType::TitleKey);
    }

    Result UnwrapCommonTitleKey(AccessKey *out_access_key, const KeySource &key_source, u32 generation) {
        return smc::ConvertResult(smc::UnwrapCommonTitleKey(out_access_key, key_source, generation));
    }

    Result ImportDrmKey(const void *src, size_t src_size, const AccessKey &access_key, const KeySource &key_source) {
        return ImportSecureExpModKey(src, src_size, access_key, key_source, static_cast<u32>(smc::DecryptOrImportMode::ImportDrmKey));
    }

    Result DrmExpMod(void *out, size_t out_size, const void *base, size_t base_size, const void *mod, size_t mod_size) {
        return SecureExpMod(out, out_size, base, base_size, mod, mod_size, smc::SecureExpModMode::Drm);
    }

    Result UnwrapElicenseKey(AccessKey *out_access_key, const void *base, size_t base_size, const void *mod, size_t mod_size, const void *label_digest, size_t label_digest_size, u32 generation) {
        return UnwrapEsRsaOaepWrappedKey(out_access_key, base, base_size, mod, mod_size, label_digest, label_digest_size, generation, smc::EsKeyType::ElicenseKey);
    }

    Result LoadElicenseKey(u32 keyslot, const void *owner, const AccessKey &access_key) {
        /* Right now, this is just literally the same function as LoadTitleKey in N's impl. */
        return LoadTitleKey(keyslot, owner, access_key);
    }

    /* FS */
    Result ImportLotusKey(const void *src, size_t src_size, const AccessKey &access_key, const KeySource &key_source, u32 option) {
        if (GetRuntimeFirmwareVersion() >= FirmwareVersion_500) {
            option = static_cast<u32>(smc::DecryptOrImportMode::ImportLotusKey);
        }
        return ImportSecureExpModKey(src, src_size, access_key, key_source, option);
    }

    Result DecryptLotusMessage(u32 *out_size, void *dst, size_t dst_size, const void *base, size_t base_size, const void *mod, size_t mod_size, const void *label_digest, size_t label_digest_size) {
        /* Validate sizes. */
        if (dst_size > WorkBufferSizeMax || label_digest_size != LabelDigestSizeMax) {
            return ResultSplInvalidSize;
        }

        /* Nintendo doesn't check this result code, but we will. */
        R_TRY(SecureExpMod(g_work_buffer, 0x100, base, base_size, mod, mod_size, smc::SecureExpModMode::Lotus));

        size_t data_size = DecodeRsaOaep(dst, dst_size, label_digest, label_digest_size, g_work_buffer, 0x100);
        if (data_size == 0) {
            return ResultSplDecryptionFailed;
        }

        *out_size = static_cast<u32>(data_size);
        return ResultSuccess;
    }

    Result GenerateSpecificAesKey(AesKey *out_key, const KeySource &key_source, u32 generation, u32 which) {
        return smc::ConvertResult(smc::GenerateSpecificAesKey(out_key, key_source, generation, which));
    }

    Result LoadTitleKey(u32 keyslot, const void *owner, const AccessKey &access_key) {
        R_TRY(ValidateAesKeyslot(keyslot, owner));
        return smc::ConvertResult(smc::LoadTitleKey(keyslot, access_key));
    }

    Result GetPackage2Hash(void *dst, const size_t size) {
        u64 hash[4];

        if (size < sizeof(hash)) {
            return ResultSplInvalidSize;
        }

        smc::Result smc_res;
        if ((smc_res = smc::GetConfig(hash, 4, SplConfigItem_Package2Hash)) != smc::Result::Success) {
            return smc::ConvertResult(smc_res);
        }

        std::memcpy(dst, hash, sizeof(hash));
        return ResultSuccess;
    }

    /* Manu. */
    Result ReEncryptRsaPrivateKey(void *dst, size_t dst_size, const void *src, size_t src_size, const AccessKey &access_key_dec, const KeySource &source_dec, const AccessKey &access_key_enc, const KeySource &source_enc, u32 option) {
        struct ReEncryptRsaPrivateKeyLayout {
            u8 data[RsaPrivateKeyMetaSize + 2 * RsaPrivateKeySize + 0x10];
            AccessKey access_key_dec;
            KeySource source_dec;
            AccessKey access_key_enc;
            KeySource source_enc;
        };
        ReEncryptRsaPrivateKeyLayout *layout = reinterpret_cast<ReEncryptRsaPrivateKeyLayout *>(g_work_buffer);

        /* Validate size. */
        if (src_size < RsaPrivateKeyMetaSize || src_size > sizeof(ReEncryptRsaPrivateKeyLayout)) {
            return ResultSplInvalidSize;
        }

        std::memcpy(layout, src, src_size);
        layout->access_key_dec = access_key_dec;
        layout->source_dec = source_dec;
        layout->access_key_enc = access_key_enc;
        layout->source_enc = source_enc;

        armDCacheFlush(layout, sizeof(*layout));

        smc::Result smc_res = smc::ReEncryptRsaPrivateKey(layout->data, src_size, layout->access_key_dec, layout->source_dec, layout->access_key_enc, layout->source_enc, option);
        if (smc_res == smc::Result::Success) {
            size_t copy_size = std::min(dst_size, src_size);
            armDCacheFlush(layout, copy_size);
            std::memcpy(dst, layout->data, copy_size);
        }

        return smc::ConvertResult(smc_res);
    }

    /* Helper. */
    Result FreeAesKeyslots(const void *owner) {
        for (size_t i = 0; i < GetMaxKeyslots(); i++) {
            if (g_keyslot_owners[i] == owner) {
                FreeAesKeyslot(i, owner);
            }
        }
        return ResultSuccess;
    }

    Handle GetAesKeyslotAvailableEventHandle() {
        return g_se_keyslot_available_event->GetHandle();
    }

}
