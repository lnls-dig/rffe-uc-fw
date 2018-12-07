#include "mbed.h"

#define BOOTLOADER_MAGIC_WORD 0xAAAAAAAA
#define UPDATE_ADDRESS_OFFSET 0x48000

//DigitalOut led1(P1_18);
//DigitalOut led2(P1_20);
//DigitalOut led3(P1_21);
//DigitalOut led4(P1_23);

RawSerial pc(P0_2, P0_3); // Serial USB port. (NOTE: All printf() calls are redirected to this port)
FlashIAP flash;
const uint32_t page_size = flash.get_page_size();
const uint32_t flash_size = flash.get_flash_size();

void update(uint32_t address)
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

    delete[] page_buffer;

}

int main (void)
{
    uint32_t upgr_fw_id;

    uint8_t version[3] = {0};

    flash.init();

    pc.baud(115200);

    flash.read(&upgr_fw_id, (flash_size - 4), 4);
    flash.read(version, (flash_size - 8), 3);

    if(upgr_fw_id == BOOTLOADER_MAGIC_WORD) {
        //led1 = led2 = led3 = led4 = 1;
        printf("Updating firmware to new version %d.%d.%d\n", version[0], version[1], version[2]);
        update(POST_APPLICATION_ADDR);
        printf("Update finished!\n");
    }

    flash.deinit();

    mbed_start_application(POST_APPLICATION_ADDR);

    while (1);
}
