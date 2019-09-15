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

#include <stratosphere.hpp>
#include <stratosphere/kvdb/kvdb_archive.hpp>

namespace sts::kvdb {

    namespace {

        /* Convenience definitions. */
        constexpr u8 ArchiveHeaderMagic[4] = {'I', 'M', 'K', 'V'};
        constexpr u8 ArchiveEntryMagic[4]  = {'I', 'M', 'E', 'N'};

        /* Archive types. */
        struct ArchiveHeader {
            u8 magic[sizeof(ArchiveHeaderMagic)];
            u32 pad;
            u32 entry_count;

            Result Validate() const {
                if (std::memcmp(this->magic, ArchiveHeaderMagic, sizeof(ArchiveHeaderMagic)) != 0) {
                    return ResultKvdbInvalidKeyValue;
                }
                return ResultSuccess;
            }

            static ArchiveHeader Make(size_t entry_count) {
                ArchiveHeader header = {};
                std::memcpy(header.magic, ArchiveHeaderMagic, sizeof(ArchiveHeaderMagic));
                header.entry_count = static_cast<u32>(entry_count);
                return header;
            }
        };
        static_assert(sizeof(ArchiveHeader) == 0xC && std::is_pod<ArchiveHeader>::value, "ArchiveHeader definition!");

        struct ArchiveEntryHeader {
            u8 magic[sizeof(ArchiveEntryMagic)];
            u32 key_size;
            u32 value_size;

            Result Validate() const {
                if (std::memcmp(this->magic, ArchiveEntryMagic, sizeof(ArchiveEntryMagic)) != 0) {
                    return ResultKvdbInvalidKeyValue;
                }
                return ResultSuccess;
            }

            static ArchiveEntryHeader Make(size_t ksz, size_t vsz) {
                ArchiveEntryHeader header = {};
                std::memcpy(header.magic, ArchiveEntryMagic, sizeof(ArchiveEntryMagic));
                header.key_size = ksz;
                header.value_size = vsz;
                return header;
            }
        };
        static_assert(sizeof(ArchiveEntryHeader) == 0xC && std::is_pod<ArchiveEntryHeader>::value, "ArchiveEntryHeader definition!");

    }

    /* Reader functionality. */
    Result ArchiveReader::Peek(void *dst, size_t size) {
        /* Bounds check. */
        if (this->offset + size > this->buffer.GetSize() || this->offset + size <= this->offset) {
            return ResultKvdbInvalidKeyValue;
        }

        std::memcpy(dst, this->buffer.Get() + this->offset, size);
        return ResultSuccess;
    }

    Result ArchiveReader::Read(void *dst, size_t size) {
        R_TRY(this->Peek(dst, size));
        this->offset += size;
        return ResultSuccess;
    }

    Result ArchiveReader::ReadEntryCount(size_t *out) {
        /* This should only be called at the start of reading stream. */
        if (this->offset != 0) {
            std::abort();
        }

        /* Read and validate header. */
        ArchiveHeader header;
        R_TRY(this->Read(&header, sizeof(header)));
        R_TRY(header.Validate());

        *out = header.entry_count;
        return ResultSuccess;
    }

    Result ArchiveReader::GetEntrySize(size_t *out_key_size, size_t *out_value_size) {
        /* This should only be called after ReadEntryCount. */
        if (this->offset == 0) {
            std::abort();
        }

        /* Peek the next entry header. */
        ArchiveEntryHeader header;
        R_TRY(this->Peek(&header, sizeof(header)));
        R_TRY(header.Validate());

        *out_key_size = header.key_size;
        *out_value_size = header.value_size;
        return ResultSuccess;
    }

    Result ArchiveReader::ReadEntry(void *out_key, size_t key_size, void *out_value, size_t value_size) {
        /* This should only be called after ReadEntryCount. */
        if (this->offset == 0) {
            std::abort();
        }

        /* Read the next entry header. */
        ArchiveEntryHeader header;
        R_TRY(this->Read(&header, sizeof(header)));
        R_TRY(header.Validate());

        /* Key size and Value size must be correct. */
        if (key_size != header.key_size || value_size != header.value_size) {
            std::abort();
        }

        R_ASSERT(this->Read(out_key, key_size));
        R_ASSERT(this->Read(out_value, value_size));
        return ResultSuccess;
    }

    /* Writer functionality. */
    Result ArchiveWriter::Write(const void *src, size_t size) {
        /* Bounds check. */
        if (this->offset + size > this->buffer.GetSize() || this->offset + size <= this->offset) {
            return ResultKvdbInvalidKeyValue;
        }

        std::memcpy(this->buffer.Get() + this->offset, src, size);
        this->offset += size;
        return ResultSuccess;
    }

    void ArchiveWriter::WriteHeader(size_t entry_count) {
        /* This should only be called at start of write. */
        if (this->offset != 0) {
            std::abort();
        }

        ArchiveHeader header = ArchiveHeader::Make(entry_count);
        R_ASSERT(this->Write(&header, sizeof(header)));
    }

    void ArchiveWriter::WriteEntry(const void *key, size_t key_size, const void *value, size_t value_size) {
        /* This should only be called after writing header. */
        if (this->offset == 0) {
            std::abort();
        }

        ArchiveEntryHeader header = ArchiveEntryHeader::Make(key_size, value_size);
        R_ASSERT(this->Write(&header, sizeof(header)));
        R_ASSERT(this->Write(key, key_size));
        R_ASSERT(this->Write(value, value_size));
    }

    /* Size helper functionality. */
    ArchiveSizeHelper::ArchiveSizeHelper() : size(sizeof(ArchiveHeader)) {
        /* ... */
    }

    void ArchiveSizeHelper::AddEntry(size_t key_size, size_t value_size) {
        this->size += sizeof(ArchiveEntryHeader) + key_size + value_size;
    }

}