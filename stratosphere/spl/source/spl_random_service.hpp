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
#include <stratosphere/spl/spl_types.hpp>

namespace sts::spl {

    class RandomService final : public IServiceObject {
        protected:
            enum class CommandId {
                GenerateRandomBytes = 0,
            };
        public:
            RandomService() { /* ... */ }
            virtual ~RandomService() { /* ... */ }
        private:
            /* Actual commands. */
            virtual Result GenerateRandomBytes(OutBuffer<u8> out);
        public:
            DEFINE_SERVICE_DISPATCH_TABLE {
                MAKE_SERVICE_COMMAND_META(RandomService, GenerateRandomBytes),
            };
    };

}
