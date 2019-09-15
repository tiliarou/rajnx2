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

#include <switch.h>
#include <stratosphere.hpp>

#include "spl_api_impl.hpp"
#include "spl_ssl_service.hpp"

namespace sts::spl {

    Result SslService::ImportSslKey(InPointer<u8> src, AccessKey access_key, KeySource key_source) {
        return impl::ImportSslKey(src.pointer, src.num_elements, access_key, key_source);
    }

    Result SslService::SslExpMod(OutPointerWithClientSize<u8> out, InPointer<u8> base, InPointer<u8> mod) {
        return impl::SslExpMod(out.pointer, out.num_elements, base.pointer, base.num_elements, mod.pointer, mod.num_elements);
    }

}
