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

constexpr InitialConfig InitialConfigsIowa[] = {
    {0x04, GpioDirection_Input, GpioValue_High},
    {0x05, GpioDirection_Output, GpioValue_Low},
    {0x06, GpioDirection_Input, GpioValue_Low},
    {0x02, GpioDirection_Output, GpioValue_Low},
    {0x3C, GpioDirection_Input, GpioValue_Low},
    {0x0F, GpioDirection_Input, GpioValue_High},
    {0x08, GpioDirection_Input, GpioValue_Low},
    {0x09, GpioDirection_Input, GpioValue_Low},
    {0x0A, GpioDirection_Output, GpioValue_Low},
    {0x0B, GpioDirection_Input, GpioValue_Low},
    {0x0D, GpioDirection_Output, GpioValue_Low},
    {0x0E, GpioDirection_Input, GpioValue_Low},
    {0x10, GpioDirection_Input, GpioValue_Low},
    {0x11, GpioDirection_Input, GpioValue_Low},
    {0x12, GpioDirection_Input, GpioValue_Low},
    {0x13, GpioDirection_Input, GpioValue_Low},
    {0x14, GpioDirection_Input, GpioValue_High},
    {0x16, GpioDirection_Input, GpioValue_Low},
    {0x15, GpioDirection_Input, GpioValue_Low},
    {0x17, GpioDirection_Input, GpioValue_High},
    {0x18, GpioDirection_Input, GpioValue_Low},
    {0x19, GpioDirection_Input, GpioValue_High},
    {0x1A, GpioDirection_Input, GpioValue_High},
    {0x1B, GpioDirection_Input, GpioValue_Low},
    {0x1C, GpioDirection_Input, GpioValue_Low},
    {0x1D, GpioDirection_Output, GpioValue_Low},
    {0x1E, GpioDirection_Output, GpioValue_Low},
    {0x20, GpioDirection_Output, GpioValue_Low},
    {0x21, GpioDirection_Input, GpioValue_Low},
    {0x38, GpioDirection_Input, GpioValue_High},
    {0x22, GpioDirection_Input, GpioValue_Low},
    {0x23, GpioDirection_Input, GpioValue_High},
    {0x01, GpioDirection_Output, GpioValue_Low},
    {0x39, GpioDirection_Output, GpioValue_Low},
    {0x24, GpioDirection_Output, GpioValue_Low},
    {0x34, GpioDirection_Input, GpioValue_Low},
    {0x25, GpioDirection_Input, GpioValue_Low},
    {0x26, GpioDirection_Input, GpioValue_Low},
    {0x27, GpioDirection_Input, GpioValue_Low},
    {0x2B, GpioDirection_Output, GpioValue_Low},
    {0x28, GpioDirection_Input, GpioValue_High},
    {0x1F, GpioDirection_Output, GpioValue_Low},
    {0x29, GpioDirection_Input, GpioValue_High},
    {0x3A, GpioDirection_Output, GpioValue_Low},
    {0x0C, GpioDirection_Input, GpioValue_Low},
    {0x2D, GpioDirection_Output, GpioValue_Low},
    {0x2E, GpioDirection_Output, GpioValue_Low},
    {0x37, GpioDirection_Input, GpioValue_Low},
    {0x2F, GpioDirection_Output, GpioValue_Low},
    {0x03, GpioDirection_Output, GpioValue_Low},
    {0x30, GpioDirection_Input, GpioValue_Low},
    {0x3B, GpioDirection_Input, GpioValue_Low},
    {0x31, GpioDirection_Output, GpioValue_Low},
    {0x32, GpioDirection_Output, GpioValue_Low},
    {0x33, GpioDirection_Output, GpioValue_Low},
    {0x35, GpioDirection_Input, GpioValue_High},
    {0x2C, GpioDirection_Output, GpioValue_Low},
    {0x36, GpioDirection_Output, GpioValue_Low},
};

constexpr u32 NumInitialConfigsIowa = (sizeof(InitialConfigsIowa) / sizeof(InitialConfigsIowa[0]));
