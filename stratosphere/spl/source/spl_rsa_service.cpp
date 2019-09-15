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
#include "spl_rsa_service.hpp"

namespace sts::spl {

    Result RsaService::DecryptRsaPrivateKeyDeprecated(OutPointerWithClientSize<u8> dst, InPointer<u8> src, AccessKey access_key, KeySource key_source, u32 option) {
        return impl::DecryptRsaPrivateKey(dst.pointer, dst.num_elements, src.pointer, src.num_elements, access_key, key_source, option);
    }

    Result RsaService::DecryptRsaPrivateKey(OutPointerWithClientSize<u8> dst, InPointer<u8> src, AccessKey access_key, KeySource key_source) {
        return impl::DecryptRsaPrivateKey(dst.pointer, dst.num_elements, src.pointer, src.num_elements, access_key, key_source, static_cast<u32>(smc::DecryptOrImportMode::DecryptRsaPrivateKey));
    }

}
