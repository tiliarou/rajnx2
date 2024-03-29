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
#include <iterator>

/* Any broadly useful language defines should go here. */

#define NON_COPYABLE(cls) \
    cls(const cls&) = delete; \
    cls& operator=(const cls&) = delete

#define NON_MOVEABLE(cls) \
    cls(cls&&) = delete; \
    cls& operator=(cls&&) = delete

#define ALIGNED(algn) __attribute__((aligned(algn)))
#define WEAK          __attribute__((weak))

namespace sts::util {

    /* std::size() does not support zero-size C arrays. We're fixing that. */
    template<class C>
    constexpr auto size(const C& c) -> decltype(c.size()) {
        return std::size(c);
    }

    template<class C>
    constexpr std::size_t size(const C& c) {
        if constexpr (sizeof(C) == 0) {
            return 0;
        } else {
            return std::size(c);
        }
    }

}