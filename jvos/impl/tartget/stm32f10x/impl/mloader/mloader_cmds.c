#include "flash_api.h"
#include "mloader.h"

#include "shell.h"
#include "cmsis.h"

int flash_cmd_read(uint32_t addr, uint32_t size, uint8_t *buf)
{
    static flash_t flash;

    flash_stream_read(&flash, addr, size, buf);
    
    return 0;
}

int flash_cmd_write(uint32_t addr, uint32_t size, uint8_t *buf)
{
    static flash_t flash;

    flash_stream_write(&flash, addr, size, buf);

    return 0;
}

int flash_cmd_erase(uint32_t addr, uint32_t size, uint8_t *buf)
{
    static flash_t flash;

    while(1)
    {
         if(size > 0x10000 || size == 0x10000)
         {
             /*erase 64K*/
             flash_erase_block(&flash, addr);
             size -= 0x10000;
             addr += 0x10000;
             continue;
         }else if(size > 0x1000 || size == 0x1000)
         { 
            /*erase 4K*/
            flash_erase_sector(&flash, addr);
            size -= 0x1000;
            addr += 0x1000;
            continue;
         }else if(size > 0)
         {
             flash_erase_sector(&flash, addr);
             size = 0;
             continue;
         }else if(size == 0)
         {
             break;
         }
    }
    
    return 0;
}

int flash_cmd_mac(uint32_t addr, uint32_t size, uint8_t *buf)
{
    uint8_t read_buf_mac[6];

    efuse_logical_read(0x11A, 6, read_buf_mac);
    memcpy(buf,read_buf_mac,6);

    return 0;
}

int flash_cmd_unlock()
{
    return 0;
}

mloader_cmd_t mloader_cmds[] = {
    [READ] = flash_cmd_read,
    [WRITE] = flash_cmd_write,
    [ERASE] = flash_cmd_erase,
    [MAC] = flash_cmd_mac,
    [UNLOCK] = flash_cmd_unlock,
};