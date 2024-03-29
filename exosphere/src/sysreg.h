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
 
#ifndef EXOSPHERE_SYSREG_H
#define EXOSPHERE_SYSREG_H

#include <stdint.h>

#include "memory_map.h"

/* Exosphere driver for the Tegra X1 System Registers. */

#define SYSREG_BASE  (MMIO_GET_DEVICE_ADDRESS(MMIO_DEVID_SYSREGS))

#define SB_BASE (SYSREG_BASE + 0x200)

#define MAKE_SYSREG(n) MAKE_REG32(SYSREG_BASE + n)
#define MAKE_SB_REG(n) MAKE_REG32(SB_BASE + n)

#define AHB_ARBITRATION_DISABLE_0 MAKE_SYSREG(0x004)

#define SB_CSR_0 MAKE_SB_REG(0x00)
#define SB_PIROM_START_0 MAKE_SB_REG(0x04)
#define SB_PFCFG_0 MAKE_SB_REG(0x08)
#define SB_SECURE_SPAREREG_0_0 MAKE_SB_REG(0x0C)
#define SB_SECURE_SPAREREG_1_0 MAKE_SB_REG(0x10)
#define SB_SECURE_SPAREREG_2_0 MAKE_SB_REG(0x14)
#define SB_SECURE_SPAREREG_3_0 MAKE_SB_REG(0x18)
#define SB_SECURE_SPAREREG_4_0 MAKE_SB_REG(0x1C)
#define SB_SECURE_SPAREREG_5_0 MAKE_SB_REG(0x20)
#define SB_SECURE_SPAREREG_6_0 MAKE_SB_REG(0x24)
#define SB_SECURE_SPAREREG_7_0 MAKE_SB_REG(0x28)
#define SB_AA64_RESET_LOW_0 MAKE_SB_REG(0x30)
#define SB_AA64_RESET_HIGH_0 MAKE_SB_REG(0x34)

#endif
