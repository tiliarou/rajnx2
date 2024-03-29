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
 
#ifndef EXOSPHERE_MISC_H
#define EXOSPHERE_MISC_H

#include <stdint.h>
#include "memory_map.h"

/* Exosphere driver for the Tegra X1 MISC Registers. */

#define MISC_BASE  (MMIO_GET_DEVICE_ADDRESS(MMIO_DEVID_MISC))
#define MAKE_MISC_REG(n) MAKE_REG32(MISC_BASE + n)

#define APB_MISC_SECURE_REGS_APB_SLAVE_SECURITY_ENABLE_REG0_0 MAKE_MISC_REG(0x0C00)
#define APB_MISC_SECURE_REGS_APB_SLAVE_SECURITY_ENABLE_REG1_0 MAKE_MISC_REG(0x0C04)
#define APB_MISC_SECURE_REGS_APB_SLAVE_SECURITY_ENABLE_REG2_0 MAKE_MISC_REG(0x0C08)

#endif
