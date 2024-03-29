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
#include "fatal_service.hpp"

namespace sts::fatal::srv {

    class ITask {
        public:
            static constexpr size_t DefaultStackSize = 0x1000;
        protected:
            const ThrowContext *context = nullptr;
        public:
            void Initialize(const ThrowContext *context) {
                this->context = context;
            }

            virtual Result Run() = 0;

            virtual const char *GetName() const = 0;

            virtual size_t GetStackSize() const {
                return DefaultStackSize;
            }
    };

    void RunTasks(const ThrowContext *ctx);

}