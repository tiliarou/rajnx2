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
#include <strings.h>

#include "boot_power_utils.hpp"
#include "fusee-primary_bin.h"

namespace sts::boot {

    namespace {

        /* Convenience definitions. */
        constexpr uintptr_t IramBase = 0x40000000ull;
        constexpr uintptr_t IramPayloadBase = 0x40010000ull;
        constexpr size_t IramSize = 0x40000;
        constexpr size_t IramPayloadMaxSize = 0x2E000;

        /* Globals. */
        u8 __attribute__ ((aligned (0x1000))) g_work_page[0x1000];

        /* Helpers. */
        void ClearIram() {
            /* Make page FFs. */
            memset(g_work_page, 0xFF, sizeof(g_work_page));

            /* Overwrite all of IRAM with FFs. */
            for (size_t ofs = 0; ofs < IramSize; ofs += sizeof(g_work_page)) {
                CopyToIram(IramBase + ofs, g_work_page, sizeof(g_work_page));
            }
        }

        void DoRebootToPayload(AtmosphereFatalErrorContext *ctx) {
            /* Ensure clean IRAM state. */
            ClearIram();

            /* Copy in payload. */
            for (size_t ofs = 0; ofs < fusee_primary_bin_size; ofs += 0x1000) {
                std::memcpy(g_work_page, &fusee_primary_bin[ofs], std::min(static_cast<size_t>(fusee_primary_bin_size - ofs), size_t(0x1000)));
                CopyToIram(IramPayloadBase + ofs, g_work_page, 0x1000);
            }


            /* Copy in fatal error context, if relevant. */
            if (ctx != nullptr) {
                std::memset(g_work_page, 0xCC, sizeof(g_work_page));
                std::memcpy(g_work_page, ctx, sizeof(*ctx));
                CopyToIram(IramPayloadBase + IramPayloadMaxSize, g_work_page, sizeof(g_work_page));
            }

            RebootToIramPayload();
        }

    }

    void RebootSystem() {
        DoRebootToPayload(nullptr);
    }

    void RebootForFatalError(AtmosphereFatalErrorContext *ctx) {
        DoRebootToPayload(ctx);
    }

}
