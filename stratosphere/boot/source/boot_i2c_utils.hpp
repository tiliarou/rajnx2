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
#include <stratosphere.hpp>

#include "i2c/driver/i2c_api.hpp"

namespace sts::boot {

    /* I2C Utilities. */
    Result ReadI2cRegister(i2c::driver::Session &session, u8 *dst, size_t dst_size, const u8 *cmd, size_t cmd_size);
    Result WriteI2cRegister(i2c::driver::Session &session, const u8 *src, size_t src_size, const u8 *cmd, size_t cmd_size);
    Result WriteI2cRegister(i2c::driver::Session &session, const u8 address, const u8 value);

}
