#include <stdint.h>

#include "flash_api.h"

#include "mos.h"
#include "mdebug.h"
#include "mhal/flash.h"
#include "device_lock.h"


#define OTA_INDEX_1			1
#define OTA_INDEX_2			2

enum
{
  MXOS_PARTITION_BOOTLOADER,
  MXOS_PARTITION_KVRO,
  MXOS_PARTITION_APPLICATION,
  MXOS_PARTITION_OTA_TEMP,
  MXOS_PARTITION_KV,
  MXOS_PARTITION_OTA_HEARD,
  MXOS_PARTITION_USER,
  MXOS_PARTITION_MAX,
  MXOS_PARTITION_NONE,
};

static const mhal_flash_part_t flash_parts[] =
{
    [MXOS_PARTITION_BOOTLOADER] =
    {
        .owner = 1,
        .desc = "Bootloader",
        .addr = 0x4000,
        .len = 0x8000, //32K bytes
        .rw = PAR_OPT_READ_EN | PAR_OPT_WRITE_DIS,
    },
    [MXOS_PARTITION_KVRO] =
    {
        .owner = 1,
        .desc = "KVRO",
        .addr = 0xC000,
        .len = 0x4000, //16K bytes
        .rw = PAR_OPT_READ_EN | PAR_OPT_WRITE_EN,
    },
    [MXOS_PARTITION_KV] =
    {
        .owner = 1,
        .desc = "Kv",
        .addr = 0x10000,
        .len = 0x4000, //16K bytes
        .rw = PAR_OPT_READ_EN | PAR_OPT_WRITE_EN,
    },
    [MXOS_PARTITION_OTA_HEARD] =
    {
        .owner = 1,
        .desc = "OTA header",
        .addr = 0x14000,
        .len = 0x1000, //4K bytes
        .rw = PAR_OPT_READ_EN | PAR_OPT_WRITE_EN,
    },
    [MXOS_PARTITION_USER] =
    {
        .owner = 1,
        .desc = "user",
        .addr = 0x15000,
        .len = 0xB000, //44K bytes
        .rw = PAR_OPT_READ_EN | PAR_OPT_WRITE_EN,
    },
    [MXOS_PARTITION_APPLICATION] =
    {
        .owner = 1,
        .desc = "Application",
        .addr = 0x20000,
        .len = 0xF0000, //960K bytes
        .rw = PAR_OPT_READ_EN | PAR_OPT_WRITE_EN,
    },
    [MXOS_PARTITION_OTA_TEMP] =
    {
        .owner = 1,
        .desc = "OTA",
        .addr = 0x110000,
        .len = 0xF0000, //960K bytes
        .rw = PAR_OPT_READ_EN | PAR_OPT_WRITE_EN,
    },

};

static flash_t flash;

__attribute__((weak)) mhal_flash_part_t *mhal_flash_get_info(mhal_flash_part_name_t name)
{
    require(name >= MXOS_PARTITION_BOOTLOADER && name < MXOS_PARTITION_MAX, exit);

    switch(name)
    {
        case MXOS_PARTITION_BOOTLOADER:
            return &flash_parts[MXOS_PARTITION_BOOTLOADER];
        case MXOS_PARTITION_KVRO:
            return &flash_parts[MXOS_PARTITION_KVRO];
        case MXOS_PARTITION_APPLICATION:
        if(OTA_INDEX_1 == get_cur_fw_idx())
            return &flash_parts[MXOS_PARTITION_APPLICATION];
        else
            return &flash_parts[MXOS_PARTITION_OTA_TEMP];
        case MXOS_PARTITION_OTA_TEMP:
        if(OTA_INDEX_1 == get_cur_fw_idx())
            return &flash_parts[MXOS_PARTITION_OTA_TEMP];
        else
            return &flash_parts[MXOS_PARTITION_APPLICATION];
        case MXOS_PARTITION_KV:
            return &flash_parts[MXOS_PARTITION_KV];
        case MXOS_PARTITION_OTA_HEARD:
            return &flash_parts[MXOS_PARTITION_OTA_HEARD];
        default:
            return  &flash_parts[MXOS_PARTITION_NONE];
    }

exit:
    return NULL;
}

merr_t mhal_flash_erase(mhal_flash_part_name_t name, uint32_t offset, uint32_t len)
{
    merr_t err = kNoErr;
    uint32_t erase_size = 0;

    require_action_quiet(name >= MXOS_PARTITION_BOOTLOADER && name < MXOS_PARTITION_MAX, exit, err = kParamErr);

    mhal_flash_part_t *flash_part = mhal_flash_get_info(name);

    require_action_quiet(offset + len <= flash_part->len && (flash_part->rw & PAR_OPT_WRITE_EN), exit, err = kParamErr);
    erase_size = ((len - 1)/4096) + 1;
    device_mutex_lock(RT_DEV_LOCK_FLASH);
    for (int i = 0; i < erase_size; i ++)
    {
        flash_erase_sector(&flash, flash_part->addr + offset + i * 0x1000);
        osDelay(1);
    }
    device_mutex_unlock(RT_DEV_LOCK_FLASH);

exit:
    return err;
}

merr_t mhal_flash_write(mhal_flash_part_name_t name, uint32_t *poffset, uint8_t *buffer, uint32_t len)
{
    merr_t err = kNoErr;
    uint8_t *sram_buf = buffer;

    require_action_quiet(name >= MXOS_PARTITION_BOOTLOADER && name < MXOS_PARTITION_MAX, exit, err = kParamErr);

    mhal_flash_part_t *flash_part = mhal_flash_get_info(name);

    uint32_t offset = *poffset;
    require_action_quiet(offset + len <= flash_part->len && (flash_part->rw & PAR_OPT_WRITE_EN), exit, err = kParamErr);

    if (buffer >= 0x08000000 && buffer <= 0x08000000 + 0x200000)
    {
        if ((sram_buf = malloc(len)) == NULL)
        {
            return kNoMemoryErr;
        }
        memcpy(sram_buf, buffer, len);
    }

    device_mutex_lock(RT_DEV_LOCK_FLASH);
    flash_stream_write(&flash, flash_part->addr + offset, len, sram_buf);
    device_mutex_unlock(RT_DEV_LOCK_FLASH);

    *poffset += len;

exit:
    return err;
}

merr_t mhal_flash_read(mhal_flash_part_name_t name, uint32_t *poffset, uint8_t *buffer, uint32_t len)
{
    merr_t err = kNoErr;

    require_action_quiet(name >= MXOS_PARTITION_BOOTLOADER && name < MXOS_PARTITION_MAX, exit, err = kParamErr);
    
    mhal_flash_part_t *flash_part = mhal_flash_get_info(name);

    uint32_t offset = *poffset;
    require_action_quiet(offset + len <= flash_part->len && (flash_part->rw & PAR_OPT_READ_EN), exit, err = kParamErr);

    device_mutex_lock(RT_DEV_LOCK_FLASH);
    flash_stream_read(&flash, flash_part->addr + offset, len, buffer);
    device_mutex_unlock(RT_DEV_LOCK_FLASH);

    *poffset += len;

exit:
    return err;
}