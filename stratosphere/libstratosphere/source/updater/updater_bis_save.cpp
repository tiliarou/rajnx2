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

#include "updater_bis_save.hpp"

namespace sts::updater {

    size_t BisSave::GetVerificationFlagOffset(BootModeType mode) {
        switch (mode) {
            case BootModeType::Normal:
                return 0;
            case BootModeType::Safe:
                return 1;
            default:
                return 2;
        }
    }

    Result BisSave::Initialize(void *work_buffer, size_t work_buffer_size) {
        if (work_buffer_size < SaveSize || reinterpret_cast<uintptr_t>(work_buffer) & 0xFFF || work_buffer_size & 0x1FF) {
            std::abort();
        }

        R_TRY(this->accessor.Initialize());
        this->save_buffer = work_buffer;
        return ResultSuccess;
    }

    void BisSave::Finalize() {
        this->accessor.Finalize();
    }

    Result BisSave::Load() {
        size_t read_size;
        return this->accessor.Read(&read_size, this->save_buffer, SaveSize, Boot0Partition::BctSave);
    }

    Result BisSave::Save() {
        return this->accessor.Write(this->save_buffer, SaveSize, Boot0Partition::BctSave);
    }

    bool BisSave::GetNeedsVerification(BootModeType mode) {
        return reinterpret_cast<const u8 *>(this->save_buffer)[GetVerificationFlagOffset(mode)] != 0;
    }

    void BisSave::SetNeedsVerification(BootModeType mode, bool needs_verification) {
        reinterpret_cast<u8 *>(this->save_buffer)[GetVerificationFlagOffset(mode)] = needs_verification ? 1 : 0;
    }

}
