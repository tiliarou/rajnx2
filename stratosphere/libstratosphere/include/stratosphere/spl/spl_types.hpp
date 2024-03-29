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

#pragma once
#include <switch.h>
#include "../results.hpp"

namespace sts::spl {

    namespace smc {

        enum class FunctionId : u32 {
            SetConfig                     = 0xC3000401,
            GetConfig                     = 0xC3000002,
            CheckStatus                   = 0xC3000003,
            GetResult                     = 0xC3000404,
            ExpMod                        = 0xC3000E05,
            GenerateRandomBytes           = 0xC3000006,
            GenerateAesKek                = 0xC3000007,
            LoadAesKey                    = 0xC3000008,
            CryptAes                      = 0xC3000009,
            GenerateSpecificAesKey        = 0xC300000A,
            ComputeCmac                   = 0xC300040B,
            ReEncryptRsaPrivateKey        = 0xC300D60C,
            DecryptOrImportRsaPrivateKey  = 0xC300100D,

            SecureExpMod                  = 0xC300060F,
            UnwrapTitleKey                = 0xC3000610,
            LoadTitleKey                  = 0xC3000011,
            UnwrapCommonTitleKey          = 0xC3000012,

            /* Deprecated functions. */
            ImportEsKey                   = 0xC300100C,
            DecryptRsaPrivateKey          = 0xC300100D,
            ImportSecureExpModKey         = 0xC300100E,
        };

        enum class Result {
            Success               = 0,
            NotImplemented        = 1,
            InvalidArgument       = 2,
            InProgress            = 3,
            NoAsyncOperation      = 4,
            InvalidAsyncOperation = 5,
            Blacklisted           = 6,

            Max                   = 99,
        };

        inline ::Result ConvertResult(Result result) {
            if (result == Result::Success) {
                return ResultSuccess;
            }
            if (result < Result::Max) {
                return MAKERESULT(Module_Spl, static_cast<u32>(result));
            }
            return ResultSplUnknownSmcResult;
        }

        enum class CipherMode {
            CbcEncrypt = 0,
            CbcDecrypt = 1,
            Ctr        = 2,
        };

        enum class DecryptOrImportMode {
            DecryptRsaPrivateKey = 0,
            ImportLotusKey       = 1,
            ImportEsKey          = 2,
            ImportSslKey         = 3,
            ImportDrmKey         = 4,
        };

        enum class SecureExpModMode {
            Lotus = 0,
            Ssl   = 1,
            Drm   = 2,
        };

        enum class EsKeyType {
            TitleKey    = 0,
            ElicenseKey = 1,
        };

        struct AsyncOperationKey {
            u64 value;
        };
    }

    enum class HardwareType {
        Icosa = 0,
        Copper = 1,
        Hoag = 2,
        Iowa = 3,
    };

    enum MemoryArrangement {
        MemoryArrangement_Standard             = 0,
        MemoryArrangement_StandardForAppletDev = 1,
        MemoryArrangement_StandardForSystemDev = 2,
        MemoryArrangement_Expanded             = 3,
        MemoryArrangement_ExpandedForAppletDev = 4,

        /* Note: MemoryArrangement_Dynamic is not official. */
        /* Atmosphere uses it to maintain compatibility with firmwares prior to 6.0.0, */
        /* which removed the explicit retrieval of memory arrangement from PM. */
        MemoryArrangement_Dynamic              = 5,
        MemoryArrangement_Count,
    };

    struct BootReasonValue {
        union {
            struct {
                u8 power_intr;
                u8 rtc_intr;
                u8 nv_erc;
                u8 boot_reason;
            };
            u32 value;
        };
    };
    static_assert(sizeof(BootReasonValue) == sizeof(u32), "BootReasonValue definition!");
    #pragma pack(push, 1)

    struct AesKey {
        union {
            u8 data[AES_128_KEY_SIZE];
            u64 data64[AES_128_KEY_SIZE / sizeof(u64)];
        };
    };
    static_assert(alignof(AesKey) == alignof(u8), "AesKey definition!");

    struct IvCtr {
        union {
            u8 data[AES_128_KEY_SIZE];
            u64 data64[AES_128_KEY_SIZE / sizeof(u64)];
        };
    };
    static_assert(alignof(IvCtr) == alignof(u8), "IvCtr definition!");

    struct Cmac {
        union {
            u8 data[AES_128_KEY_SIZE];
            u64 data64[AES_128_KEY_SIZE / sizeof(u64)];
        };
    };
    static_assert(alignof(Cmac) == alignof(u8), "Cmac definition!");

    struct AccessKey {
        union {
            u8 data[AES_128_KEY_SIZE];
            u64 data64[AES_128_KEY_SIZE / sizeof(u64)];
        };
    };
    static_assert(alignof(AccessKey) == alignof(u8), "AccessKey definition!");

    struct KeySource {
        union {
            u8 data[AES_128_KEY_SIZE];
            u64 data64[AES_128_KEY_SIZE / sizeof(u64)];
        };
    };
    static_assert(alignof(AccessKey) == alignof(u8), "KeySource definition!");
    #pragma pack(pop)

}
