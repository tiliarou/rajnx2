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

#include "fatal_repair.hpp"
#include "fatal_service_for_self.hpp"

namespace sts::fatal::srv {

    namespace {

        bool IsInRepair() {
            /* Before firmware 3.0.0, this wasn't implemented. */
            if (GetRuntimeFirmwareVersion() < FirmwareVersion_300) {
                return false;
            }

            bool in_repair;
            return R_SUCCEEDED(setsysGetFlag(SetSysFlag_InRepairProcessEnable, &in_repair)) && in_repair;
        }

        bool IsInRepairWithoutVolHeld() {
            if (IsInRepair()) {
                GpioPadSession vol_btn;
                if (R_FAILED(gpioOpenSession(&vol_btn, GpioPadName_ButtonVolUp))) {
                    return true;
                }

                /* Ensure we close even on early return. */
                ON_SCOPE_EXIT { gpioPadClose(&vol_btn); };

                /* Set direction input. */
                gpioPadSetDirection(&vol_btn, GpioDirection_Input);

                /* Ensure that we're holding the volume button for a full second. */
                TimeoutHelper timeout_helper(1'000'000'000ul);
                while (!timeout_helper.TimedOut()) {
                    GpioValue val;
                    if (R_FAILED(gpioPadGetValue(&vol_btn, &val)) || val != GpioValue_Low) {
                        return true;
                    }

                    /* Sleep for 100 ms. */
                    svcSleepThread(100'000'000ul);
                }
            }

            return false;
        }

        bool NeedsRunTimeReviser() {
            /* Before firmware 5.0.0, this wasn't implemented. */
            if (GetRuntimeFirmwareVersion() < FirmwareVersion_300) {
                return false;
            }

            bool requires_time_reviser;
            return R_SUCCEEDED(setsysGetFlag(SetSysFlag_RequiresRunRepairTimeReviser, &requires_time_reviser)) && requires_time_reviser;
        }

        bool IsTimeReviserCartridgeInserted() {
            FsGameCardHandle gc_hnd;
            u8 gc_attr;
            {
                FsDeviceOperator devop;
                if (R_FAILED(fsOpenDeviceOperator(&devop))) {
                    return false;
                }

                /* Ensure we close even on early return. */
                ON_SCOPE_EXIT { fsDeviceOperatorClose(&devop); };

                /* Check that a gamecard is inserted. */
                bool inserted;
                if (R_FAILED(fsDeviceOperatorIsGameCardInserted(&devop, &inserted)) || !inserted) {
                    return false;
                }

                /* Check that we can retrieve the gamecard's attributes. */
                if (R_FAILED(fsDeviceOperatorGetGameCardHandle(&devop, &gc_hnd)) || R_FAILED(fsDeviceOperatorGetGameCardAttribute(&devop, &gc_hnd, &gc_attr))) {
                    return false;
                }
            }

            /* Check that the gamecard is a repair tool. */
            return (gc_attr & FsGameCardAttribute_Repair) == FsGameCardAttribute_Repair;
        }

        bool IsInRepairWithoutTimeReviserCartridge() {
            return NeedsRunTimeReviser() && IsTimeReviserCartridgeInserted();
        }

    }

    void CheckRepairStatus() {
        if (IsInRepairWithoutVolHeld()) {
            ThrowFatalForSelf(ResultFatalInRepairWithoutVolHeld);
        }

        if (IsInRepairWithoutTimeReviserCartridge()) {
            ThrowFatalForSelf(ResultFatalInRepairWithoutTimeReviserCartridge);
        }
    }

}
