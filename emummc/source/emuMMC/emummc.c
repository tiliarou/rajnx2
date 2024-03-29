/*
 * Copyright (c) 2019 m4xw <m4x@m4xw.net>
 * Copyright (c) 2019 Atmosphere-NX
 * Copyright (c) 2019 CTCaer
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

#include <stdlib.h>

#include "../soc/gpio.h"
#include "../utils/fatal.h"
#include "../libs/fatfs/diskio.h"
#include "emummc.h"
#include "emummc_ctx.h"

static bool sdmmc_first_init = false;
static bool storageMMCinitialized = false;
static bool storageSDinitialized = false;

// hekate sdmmmc vars
sdmmc_t sdmmc;
sdmmc_storage_t storage;
sdmmc_t sd_sdmmc;
sdmmc_storage_t sd_storage;

// init vars
bool custom_driver = true;
extern const volatile emuMMC_ctx_t emuMMC_ctx;

// FS funcs
_sdmmc_accessor_gc sdmmc_accessor_gc;
_sdmmc_accessor_sd sdmmc_accessor_sd;
_sdmmc_accessor_nand sdmmc_accessor_nand;
_lock_mutex lock_mutex;
_unlock_mutex unlock_mutex;

// FS misc
void *sd_mutex;
void *nand_mutex;
volatile int *active_partition;
volatile Handle *sdmmc_das_handle;

// FatFS
static bool fat_mounted = false;
static file_based_ctxt f_emu;

static void _sdmmc_ensure_device_attached(void)
{
    // This ensures that the sd device address space handle is always attached,
    // even if FS hasn't attached it
    static bool did_attach = false;
    if (!did_attach)
    {
        svcAttachDeviceAddressSpace(DeviceName_SDMMC1A, *sdmmc_das_handle);
        did_attach = true;
    }
}

static void _sdmmc_ensure_initialized(void)
{
    static bool init_done = false;

    // First Initial init
    if (!sdmmc_first_init)
    {
        sdmmc_initialize();
        sdmmc_first_init = true;
    }
    else
    {
        // The boot sysmodule will eventually kill power to SD. Detect this, and reinitialize when it happens.
        if (!init_done)
        {
            if (gpio_read(GPIO_PORT_E, GPIO_PIN_4) == 0)
            {
                sdmmc_finalize();
                sdmmc_initialize();
                init_done = true;
            }
        }
    }
}

static void _file_based_update_filename(char *outFilename, u32 sd_path_len, u32 part_idx)
{
    snprintf(outFilename + sd_path_len, 3, "%02d", part_idx);
}

static void _file_based_emmc_finalize(void)
{
    if ((emuMMC_ctx.EMMC_Type == emuMMC_SD_File) && fat_mounted)
    {
        // Close all open handles.
        f_close(&f_emu.fp_boot0);
        f_close(&f_emu.fp_boot1);

        for (int i = 0; i < f_emu.parts; i++)
        {
            f_close(&f_emu.fp_gpp[i]);
        }

        // Force unmount FAT volume.
        f_mount(NULL, "", 1);

        fat_mounted = false;
    }
}

void sdmmc_finalize(void)
{
    _file_based_emmc_finalize();

    if (!sdmmc_storage_end(&sd_storage))
    {
        fatal_abort(Fatal_InitSD);
    }

    storageSDinitialized = false;
}

static void _file_based_emmc_initialize(void)
{
    char path[sizeof(emuMMC_ctx.storagePath) + 0x20];
    memset(&path, 0, sizeof(path));
    memset(&f_emu, 0, sizeof(file_based_ctxt));

    memcpy(path, (void *)emuMMC_ctx.storagePath, sizeof(emuMMC_ctx.storagePath));
    strcat(path, "/eMMC/");
    int path_len = strlen(path);

    // Open BOOT0 physical partition.
    memcpy(path + path_len, "BOOT0", 6);
    if (f_open(&f_emu.fp_boot0, path, FA_READ | FA_WRITE) != FR_OK)
        fatal_abort(Fatal_InitSD);

    // Open BOOT1 physical partition.
    memcpy(path + path_len, "BOOT1", 6);
    if (f_open(&f_emu.fp_boot1, path, FA_READ | FA_WRITE) != FR_OK)
        fatal_abort(Fatal_InitSD);

    // Open handles for GPP physical partition files.
    _file_based_update_filename(path, path_len, 00);

    if (f_open(&f_emu.fp_gpp[0], path, FA_READ | FA_WRITE) != FR_OK)
        fatal_abort(Fatal_InitSD);

    f_emu.part_size = f_size(&f_emu.fp_gpp[0]) >> 9;

    // Iterate folder for split parts and stop if next doesn't exist.
    // Supports up to 32 parts of any size.
    // TODO: decide on max parts and define them. (hekate produces up to 30 parts on 1GB mode.)
    for (f_emu.parts = 1; f_emu.parts < 32; f_emu.parts++)
    {
        _file_based_update_filename(path, path_len, f_emu.parts);

        if (f_open(&f_emu.fp_gpp[f_emu.parts], path, FA_READ | FA_WRITE) != FR_OK)
        {
            // Check if single file.
            if (f_emu.parts == 1)
                f_emu.parts = 0;

            return;
        }
    }
}

bool sdmmc_initialize(void)
{
    if (!storageMMCinitialized)
    {
        if (sdmmc_storage_init_mmc(&storage, &sdmmc, SDMMC_4, SDMMC_BUS_WIDTH_8, 4))
        {
            if (sdmmc_storage_set_mmc_partition(&storage, FS_EMMC_PARTITION_GPP))
                storageMMCinitialized = true;
        }
        else
        {
            fatal_abort(Fatal_InitMMC);
        }
    }

    if (!storageSDinitialized)
    {
        int retries = 5;
        while (retries)
        {
            if (sdmmc_storage_init_sd(&sd_storage, &sd_sdmmc, SDMMC_1, SDMMC_BUS_WIDTH_4, 11))
            {
                storageSDinitialized = true;

                // File based emummc.
                if ((emuMMC_ctx.EMMC_Type == emuMMC_SD_File) && !fat_mounted)
                {
                    f_emu.sd_fs = (FATFS *)malloc(sizeof(FATFS));
                    if (f_mount(f_emu.sd_fs, "", 1) != FR_OK)
                        fatal_abort(Fatal_InitSD);
                    else
                        fat_mounted = true;

                    _file_based_emmc_initialize();
                }

                break;
            }

            retries--;
            msleep(100);
        }

        if (!storageSDinitialized)
        {
            fatal_abort(Fatal_InitSD);
        }
    }

    return storageMMCinitialized && storageSDinitialized;
}

sdmmc_accessor_t *sdmmc_accessor_get(int mmc_id)
{
    sdmmc_accessor_t *_this;
    switch (mmc_id)
    {
    case FS_SDMMC_EMMC:
        _this = sdmmc_accessor_nand();
        break;
    case FS_SDMMC_SD:
        _this = sdmmc_accessor_sd();
        break;
    case FS_SDMMC_GC:
        _this = sdmmc_accessor_gc();
        break;
    default:
        fatal_abort(Fatal_InvalidAccessor);
    }

    return _this;
}

void mutex_lock_handler(int mmc_id)
{
    if (custom_driver)
    {
        lock_mutex(sd_mutex);
    }
    lock_mutex(nand_mutex);
}

void mutex_unlock_handler(int mmc_id)
{
    unlock_mutex(nand_mutex);
    if (custom_driver)
    {
        unlock_mutex(sd_mutex);
    }
}

int sdmmc_nand_get_active_partition_index()
{
    switch (*active_partition)
    {
    case FS_EMMC_PARTITION_GPP:
        return 2;
    case FS_EMMC_PARTITION_BOOT1:
        return 1;
    case FS_EMMC_PARTITION_BOOT0:
        return 0;
    }

    fatal_abort(Fatal_InvalidAccessor);
}

static uint64_t emummc_read_write_inner(void *buf, unsigned int sector, unsigned int num_sectors, bool is_write)
{
    if ((emuMMC_ctx.EMMC_Type == emuMMC_SD))
    {
        // raw partition sector offset: emuMMC_ctx.EMMC_StoragePartitionOffset.
        sector += emuMMC_ctx.EMMC_StoragePartitionOffset;
        // Set physical partition offset.
        sector += (sdmmc_nand_get_active_partition_index() * BOOT_PARTITION_SIZE);
        if (!is_write)
            return sdmmc_storage_read(&sd_storage, sector, num_sectors, buf);
        else
            return sdmmc_storage_write(&sd_storage, sector, num_sectors, buf);
    }

    // File based emummc.
    FIL *fp_tmp = NULL;
    switch (*active_partition)
    {
    case FS_EMMC_PARTITION_GPP:
        if (f_emu.parts)
        {
            fp_tmp = &f_emu.fp_gpp[sector / f_emu.part_size];
            sector = sector % f_emu.part_size;
        }
        else
        {
            fp_tmp = &f_emu.fp_gpp[0];
        }
        break;
    case FS_EMMC_PARTITION_BOOT1:
        fp_tmp = &f_emu.fp_boot1;
        break;
    case FS_EMMC_PARTITION_BOOT0:
        fp_tmp = &f_emu.fp_boot0;
        break;
    }

    if (f_lseek(fp_tmp, sector << 9) != FR_OK)
    {
        ; //TODO. Out of range. close stuff and fatal?
    }

    uint64_t res = 0;
    if (!is_write)
        res = !(f_read(fp_tmp, buf, num_sectors << 9, NULL));
    else
        res = !(f_write(fp_tmp, buf, num_sectors << 9, NULL));

    return res;
}

// Controller close wrapper
uint64_t sdmmc_wrapper_controller_close(int mmc_id)
{
    sdmmc_accessor_t *_this;
    _this = sdmmc_accessor_get(mmc_id);

    if (_this != NULL)
    {
        if (mmc_id == FS_SDMMC_SD)
        {
            return 0;
        }
        
        if (mmc_id == FS_SDMMC_EMMC)
        {
            // Close file handles and unmount
            _file_based_emmc_finalize();

            // Close SD
            sdmmc_accessor_get(FS_SDMMC_SD)->vtab->sdmmc_accessor_controller_close(sdmmc_accessor_get(FS_SDMMC_SD));

            // Close eMMC
            return _this->vtab->sdmmc_accessor_controller_close(_this);
        }

        return _this->vtab->sdmmc_accessor_controller_close(_this);
    }

    fatal_abort(Fatal_CloseAccessor);
}

// FS read wrapper.
uint64_t sdmmc_wrapper_read(void *buf, uint64_t bufSize, int mmc_id, unsigned int sector, unsigned int num_sectors)
{
    sdmmc_accessor_t *_this;
    uint64_t read_res;

    _this = sdmmc_accessor_get(mmc_id);

    if (_this != NULL)
    {
        if (mmc_id == FS_SDMMC_EMMC || mmc_id == FS_SDMMC_SD)
        {
            mutex_lock_handler(mmc_id);
            // Assign FS accessor to the SDMMC driver
            _current_accessor = _this;
            // Make sure we're attached to the device address space.
            _sdmmc_ensure_device_attached();
            // Make sure we're still initialized if boot killed sd card power.
            _sdmmc_ensure_initialized();
        }

        if (mmc_id == FS_SDMMC_EMMC)
        {
            // Call hekates driver.
            if (emummc_read_write_inner(buf, sector, num_sectors, false))
            {
                mutex_unlock_handler(mmc_id);
                return 0;
            }

            mutex_unlock_handler(mmc_id);
            return FS_READ_WRITE_ERROR;
        }

        if (mmc_id == FS_SDMMC_SD)
        {
            static bool first_sd_read = true;
            if (first_sd_read)
            {
                first_sd_read = false;
                // Because some SD cards have issues with emuMMC's driver
                // we currently swap to FS's driver after first SD read
                // TODO: Fix remaining driver issues
                custom_driver = false;
                // FS will handle sd mutex w/o custom driver from here on
                unlock_mutex(sd_mutex);
            }

            // Call hekates driver.
            if (sdmmc_storage_read(&sd_storage, sector, num_sectors, buf))
            {
                mutex_unlock_handler(mmc_id);
                return 0;
            }

            mutex_unlock_handler(mmc_id);
            return FS_READ_WRITE_ERROR;
        }

        read_res = _this->vtab->read_write(_this, sector, num_sectors, buf, bufSize, 1);
        return read_res;
    }

    fatal_abort(Fatal_ReadNoAccessor);
}

// FS write wrapper.
uint64_t sdmmc_wrapper_write(int mmc_id, unsigned int sector, unsigned int num_sectors, void *buf, uint64_t bufSize)
{
    sdmmc_accessor_t *_this;
    uint64_t write_res;

    _this = sdmmc_accessor_get(mmc_id);

    if (_this != NULL)
    {
        if (mmc_id == FS_SDMMC_EMMC)
        {
            mutex_lock_handler(mmc_id);
            _current_accessor = _this;

            // Call hekates driver.
            if (emummc_read_write_inner(buf, sector, num_sectors, true))
            {
                mutex_unlock_handler(mmc_id);
                return 0;
            }

            mutex_unlock_handler(mmc_id);
            return FS_READ_WRITE_ERROR;
        }

        if (mmc_id == FS_SDMMC_SD)
        {
            mutex_lock_handler(mmc_id);
            _current_accessor = _this;

            sector += 0;

            // Call hekates driver.
            if (sdmmc_storage_write(&sd_storage, sector, num_sectors, buf))
            {
                mutex_unlock_handler(mmc_id);
                return 0;
            }

            mutex_unlock_handler(mmc_id);
            return FS_READ_WRITE_ERROR;
        }

        write_res = _this->vtab->read_write(_this, sector, num_sectors, buf, bufSize, 0);
        return write_res;
    }

    fatal_abort(Fatal_WriteNoAccessor);
}
