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

#include "boot_clock_initial_configuration.hpp"
#include "boot_pmc_wrapper.hpp"
#include "boot_registers_pmc.hpp"

namespace sts::boot {

    namespace {

        /* Convenience definitions. */
        constexpr u32 PmcClkOutCntrl = PmcBase + APBDEV_PMC_CLK_OUT_CNTRL;
        constexpr u32 InitialClockOutMask1x = 0x00C4;
        constexpr u32 InitialClockOutMask6x = 0xC4C4;

    }

    void SetInitialClockConfiguration() {
        /* Write mask to APBDEV_PMC_PWR_DET, then clear APBDEV_PMC_PWR_DET_VAL. */
        const u32 mask = GetRuntimeFirmwareVersion() >= FirmwareVersion_600 ? InitialClockOutMask6x : InitialClockOutMask1x;
        WritePmcRegister(PmcClkOutCntrl, mask, mask);
    }

}
