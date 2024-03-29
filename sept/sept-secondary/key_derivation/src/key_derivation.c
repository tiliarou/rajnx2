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

#include <string.h>
#include "pmc.h"
#include "se.h"
#include "utils.h"

#define AL16 __attribute__((aligned(16)))

#define DERIVATION_ID_MAX 2

static const uint8_t AL16 keyblob_seed_00[0x10] = {
    0xDF, 0x20, 0x6F, 0x59, 0x44, 0x54, 0xEF, 0xDC, 0x70, 0x74, 0x48, 0x3B, 0x0D, 0xED, 0x9F, 0xD3
};

static const uint8_t AL16 masterkey_seed[0x10] = {
    0xD8, 0xA2, 0x41, 0x0A, 0xC6, 0xC5, 0x90, 0x01, 0xC6, 0x1D, 0x6A, 0x26, 0x7C, 0x51, 0x3F, 0x3C
};

static const uint8_t AL16 devicekey_seed[0x10] = {
    0x4F, 0x02, 0x5F, 0x0E, 0xB6, 0x6D, 0x11, 0x0E, 0xDC, 0x32, 0x7D, 0x41, 0x86, 0xC2, 0xF4, 0x78
};

static const uint8_t AL16 devicekey_4x_seed[0x10] = {
    0x0C, 0x91, 0x09, 0xDB, 0x93, 0x93, 0x07, 0x81, 0x07, 0x3C, 0xC4, 0x16, 0x22, 0x7C, 0x6C, 0x28
};

static const uint8_t AL16 masterkey_4x_seed[0x10] = {
    0x2D, 0xC1, 0xF4, 0x8D, 0xF3, 0x5B, 0x69, 0x33, 0x42, 0x10, 0xAC, 0x65, 0xDA, 0x90, 0x46, 0x66
};

static const uint8_t AL16 zeroes[0x10] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

/* Note: 9.0.0 did not change the TSEC firmware. Thus, the root key is the same. */
/* To avoid distribution of more and more sept binaries, we will simply derive the 9.0.0 master key */
/* on 8.1.0 and 9.0.0. Exosphere supports this already with no issues. */

static const uint8_t AL16 master_kek_seeds[DERIVATION_ID_MAX][0x10] = {
    {0x9A, 0x3E, 0xA9, 0xAB, 0xFD, 0x56, 0x46, 0x1C, 0x9B, 0xF6, 0x48, 0x7F, 0x5C, 0xFA, 0x09, 0x5C},
    /* 8.1.0: {0xDE, 0xDC, 0xE3, 0x39, 0x30, 0x88, 0x16, 0xF8, 0xAE, 0x97, 0xAD, 0xEC, 0x64, 0x2D, 0x41, 0x41}, */
    {0x1A, 0xEC, 0x11, 0x82, 0x2B, 0x32, 0x38, 0x7A, 0x2B, 0xED, 0xBA, 0x01, 0x47, 0x7E, 0x3B, 0x67},
};

static const uint8_t AL16 master_devkey_seeds[DERIVATION_ID_MAX][0x10] = {
    {0x8F, 0x77, 0x5A, 0x96, 0xB0, 0x94, 0xFD, 0x8D, 0x28, 0xE4, 0x19, 0xC8, 0x16, 0x1C, 0xDB, 0x3D},
    /* 8.1.0: {0x67, 0x62, 0xD4, 0x8E, 0x55, 0xCF, 0xFF, 0x41, 0x31, 0x15, 0x3B, 0x24, 0x0C, 0x7C, 0x07, 0xAE}, */
    {0x4A, 0xC3, 0x4E, 0x14, 0x8B, 0x96, 0x4A, 0xD5, 0xD4, 0x99, 0x73, 0xC4, 0x45, 0xAB, 0x8B, 0x49},
};

static const uint8_t AL16 master_devkey_vectors[DERIVATION_ID_MAX][0x10] = {
    {0xD8, 0xD3, 0x67, 0x4F, 0xF3, 0xA2, 0xA4, 0x4E, 0xE4, 0x04, 0x37, 0xC2, 0xD9, 0xCF, 0x41, 0x6F},
    /* 8.1.0: {0x72, 0xD0, 0xAD, 0xEB, 0xE1, 0xF6, 0x35, 0x90, 0xB4, 0x43, 0xCC, 0x4B, 0xC4, 0xDC, 0x88, 0x0A}, */
    {0x8B, 0xD6, 0x13, 0x2F, 0xC3, 0x4D, 0x53, 0x2D, 0x10, 0xA1, 0x63, 0x85, 0x49, 0x2B, 0xCF, 0x3F},
};

void derive_keys(void) {
    /* Set mailbox. */
    volatile uint32_t *mailbox = (volatile uint32_t *)0x4003FF00;
    const uint32_t derivation_id = *((volatile uint32_t *)0x4003E800);

    if (derivation_id < DERIVATION_ID_MAX) {
        uint8_t *enc_se_state = (uint8_t *)0x4003E000;

        uint32_t AL16 work_buffer[4];

        /* Derive Keyblob Key 00. */
        se_aes_ecb_decrypt_block(0xC, work_buffer, 0x10, keyblob_seed_00, 0x10);
        decrypt_data_into_keyslot(0xF, 0xE, work_buffer, 0x10);

        /* Derive master kek. */
        decrypt_data_into_keyslot(0xE, 0xD, master_kek_seeds[derivation_id], 0x10);

        /* Clear the copy of the root key inside the SE. */
        clear_aes_keyslot(0xD);

        /* Derive master key, device master key. */
        decrypt_data_into_keyslot(0xC, 0xE, masterkey_seed, 0x10);
        decrypt_data_into_keyslot(0xE, 0xE, masterkey_4x_seed, 0x10);
        clear_aes_keyslot(0xD);

        /* Derive device keys. */
        decrypt_data_into_keyslot(0xA, 0xF, devicekey_4x_seed, 0x10);
        decrypt_data_into_keyslot(0xF, 0xF, devicekey_seed, 0x10);
        clear_aes_keyslot(0xD);

        /* Derive firmware specific device key. */
        se_aes_ecb_decrypt_block(0xA, work_buffer, 0x10, master_devkey_seeds[derivation_id], 0x10);
        decrypt_data_into_keyslot(0xE, 0xE, work_buffer, 0x10);
        clear_aes_keyslot(0xD);

        /* Test against a vector. */
        for (size_t i = 0; i < 4; i++) {
            work_buffer[i] = 0;
        }
        if (memcmp(work_buffer, zeroes, 0x10) != 0) {
            clear_aes_keyslot(0xE);
            clear_aes_keyslot(0xD);
            clear_aes_keyslot(0xC);
            clear_aes_keyslot(0xA);
            clear_aes_keyslot(0xF);
            generic_panic();
        }

        se_aes_ecb_decrypt_block(0xE, work_buffer, 0x10, master_devkey_vectors[derivation_id], 0x10);

        if (memcmp(work_buffer, zeroes, 0x10) == 0) {
            clear_aes_keyslot(0xE);
            clear_aes_keyslot(0xD);
            clear_aes_keyslot(0xC);
            clear_aes_keyslot(0xA);
            clear_aes_keyslot(0xF);
            generic_panic();
        }

        /* Clear work buffer. */
        for (size_t i = 0; i < 4; i++) {
            work_buffer[i] = 0xCCCCCCCC;
        }

        /* Save context for real. */
        se_set_in_context_save_mode(true);
        se_save_context(KEYSLOT_SWITCH_SRKGENKEY, KEYSLOT_SWITCH_RNGKEY, enc_se_state);
        se_set_in_context_save_mode(false);
    }

    /* Clear all keyslots. */
    for (size_t i = 0; i < 0x10; i++) {
        clear_aes_keyslot(i);
    }

    *mailbox = 7;
    while (1) { /* Wait for sept to handle the rest. */ }
}
