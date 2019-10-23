#include "mbed.h"

#define BOOTLOADER_MAGIC_WORD 0xAAAAAAAA
#define UPDATE_ADDRESS_OFFSET 0x48000

RawSerial pc(P0_2, P0_3); // Serial USB port. (NOTE: All printf() calls are redirected to this port)
FlashIAP flash;
const uint32_t page_size = flash.get_page_size();
const uint32_t flash_size = flash.get_flash_size();

typedef struct {
    uint8_t version[3];
    uint8_t fw_type;
    uint32_t upgr_fw_id;
} fw_info;

void update( uint32_t address )
{
    char *page_buffer = new char[page_size];
    uint32_t addr = address;
    uint32_t next_sector = addr + flash.get_sector_size(addr);
    bool sector_erased = false;
    uint32_t src = UPDATE_ADDRESS_OFFSET;

    while (src < flash_size) {
        // Read data for this page
        memset(page_buffer, 0, sizeof(page_buffer));
        memcpy(page_buffer, (uint32_t *) src, page_size);

        // Erase this page if it hasn't been erased
        if (!sector_erased) {
            flash.erase(addr, flash.get_sector_size(addr));
            sector_erased = true;
        }

        // Program page
        flash.program(page_buffer, addr, page_size);

        addr += page_size;
        src += page_size;
        if (addr >= next_sector) {
            next_sector = addr + flash.get_sector_size(addr);
            sector_erased = false;
        }
    }

    /* Erase the update sector of the Flash */
    src = UPDATE_ADDRESS_OFFSET;
    while (src < flash_size) {
        flash.erase(src, flash.get_sector_size(src));
        src += flash.get_sector_size(src);
    }

    delete[] page_buffer;
}

int main( void )
{
    fw_info header;

    flash.init();

    pc.baud(115200);

    flash.read(&header, (flash_size - sizeof(fw_info)), sizeof(fw_info));

    if(header.upgr_fw_id == BOOTLOADER_MAGIC_WORD) {
        if (header.fw_type == 1) {
            printf("[BOOTLOADER] Updating firmware to new version %d.%d.%d\r\n", header.version[0], header.version[1], header.version[2]);
            update(POST_APPLICATION_ADDR);
        } else if (header.fw_type == 2) {
            printf("[BOOTLOADER] New bootloader firmware was sent! Erasing the temporary data\r\n");
            /* Erase the update sector of the Flash */
            uint32_t src = UPDATE_ADDRESS_OFFSET;
            while (src < flash_size) {
                flash.erase(src, flash.get_sector_size(src));
                src += flash.get_sector_size(src);
            }
        } else {
            printf("[BOOTLOADER] Unknown firmware type, ignoring\r\n");
        }
    }

    flash.deinit();

    mbed_start_application(POST_APPLICATION_ADDR);

    while (1);
}
