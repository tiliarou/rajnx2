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
 
#ifndef EXOSPHERE_PMC_H
#define EXOSPHERE_PMC_H

#include <stdint.h>
#include "memory_map.h"

/* Exosphere register definitions for the Tegra X1 PMC. */

static inline uintptr_t get_pmc_base(void) {
    return MMIO_GET_DEVICE_ADDRESS(MMIO_DEVID_RTC_PMC) + 0x400ull;
}

#define PMC_BASE (get_pmc_base())

#define APBDEV_PMC_DPD_ENABLE_0 MAKE_REG32(PMC_BASE + 0x24)

#define APBDEV_PMC_PWRGATE_TOGGLE_0 MAKE_REG32(PMC_BASE + 0x30)
#define APBDEV_PMC_PWRGATE_STATUS_0 MAKE_REG32(PMC_BASE + 0x38)

#define APBDEV_PMC_SCRATCH0_0 MAKE_REG32(PMC_BASE + 0x50)

#define APBDEV_PMC_CRYPTO_OP_0 MAKE_REG32(PMC_BASE + 0xF4)

#define APBDEV_PM_0 MAKE_REG32(PMC_BASE + 0x14)
#define APBDEV_PMC_WAKE2_STATUS_0 MAKE_REG32(PMC_BASE + 0x168)
#define APBDEV_PMC_CNTRL2_0 MAKE_REG32(PMC_BASE + 0x440)

#define APBDEV_PMC_SCRATCH43_0 MAKE_REG32(PMC_BASE + 0x22C)
#define APBDEV_PMC_SEC_DISABLE8_0 MAKE_REG32(PMC_BASE + 0x5C0)
#define APBDEV_PMC_SECURE_SCRATCH112_0 MAKE_REG32(PMC_BASE + 0xB18)
#define APBDEV_PMC_SECURE_SCRATCH113_0 MAKE_REG32(PMC_BASE + 0xB1C)
#define APBDEV_PMC_SECURE_SCRATCH114_0 MAKE_REG32(PMC_BASE + 0xB20)
#define APBDEV_PMC_SECURE_SCRATCH115_0 MAKE_REG32(PMC_BASE + 0xB24)

#define APBDEV_PMC_SCRATCH200_0 MAKE_REG32(PMC_BASE + 0x840)


#define APBDEV_PMC_SEC_DISABLE3_0 MAKE_REG32(PMC_BASE + 0x2D8)
#define APBDEV_PMC_SECURE_SCRATCH34_0 MAKE_REG32(PMC_BASE + 0x368)
#define APBDEV_PMC_SECURE_SCRATCH35_0 MAKE_REG32(PMC_BASE + 0x36C)

#define APBDEV_PMC_SECURE_SCRATCH16_0 MAKE_REG32(PMC_BASE + 0x320)
#define APBDEV_PMC_SECURE_SCRATCH51_0 MAKE_REG32(PMC_BASE + 0x3AC)
#define APBDEV_PMC_SECURE_SCRATCH55_0 MAKE_REG32(PMC_BASE + 0x3BC)
#define APBDEV_PMC_SECURE_SCRATCH74_0 MAKE_REG32(PMC_BASE + 0x408)
#define APBDEV_PMC_SECURE_SCRATCH75_0 MAKE_REG32(PMC_BASE + 0x40C)
#define APBDEV_PMC_SECURE_SCRATCH76_0 MAKE_REG32(PMC_BASE + 0x410)
#define APBDEV_PMC_SECURE_SCRATCH77_0 MAKE_REG32(PMC_BASE + 0x414)
#define APBDEV_PMC_SECURE_SCRATCH78_0 MAKE_REG32(PMC_BASE + 0x418)
#define APBDEV_PMC_SECURE_SCRATCH99_0 MAKE_REG32(PMC_BASE + 0xAE4)
#define APBDEV_PMC_SECURE_SCRATCH100_0 MAKE_REG32(PMC_BASE + 0xAE8)
#define APBDEV_PMC_SECURE_SCRATCH101_0 MAKE_REG32(PMC_BASE + 0xAEC)
#define APBDEV_PMC_SECURE_SCRATCH102_0 MAKE_REG32(PMC_BASE + 0xAF0)
#define APBDEV_PMC_SECURE_SCRATCH103_0 MAKE_REG32(PMC_BASE + 0xAF4)
#define APBDEV_PMC_SECURE_SCRATCH39_0 MAKE_REG32(PMC_BASE + 0x37C)


#endif
