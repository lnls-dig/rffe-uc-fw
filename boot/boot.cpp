#include "mbed.h"
#include "boot.h"
#include "IAP.h"

IAP iap;

int main (void)
{
    uint32_t upgr_fw_id;
    upgr_fw_id = *(uint32_t*)(UPDATE_ADDRESS_OFFSET+0x37FFC);

    if(upgr_fw_id == BOOTLOADER_MAGIC_WORD) {
        update_firmware();
    }

    mbed_start_application(RUNTIME_ADDRESS_OFFSET);

    while (1);
}

void update_firmware( void )
{
    uint32_t *dst = (uint32_t *)RUNTIME_ADDRESS_OFFSET;
    uint32_t *src = (uint32_t *)UPDATE_ADDRESS_OFFSET;
    uint32_t fw_size = UPDATE_ADDRESS_OFFSET-RUNTIME_ADDRESS_OFFSET-1;
    uint32_t page[64];
    uint32_t offset;

    memset(&page[0], 0xFF, sizeof(page));

    __disable_irq();
    iap.erase( RUNTIME_SECTOR_START, RUNTIME_SECTOR_END );

    for ( offset = 0; offset < fw_size; dst += sizeof(page)/4) {
        /* Populate a page from source address */
        for (uint8_t i = 0; i<sizeof(page)/4; i++, src++, offset+=4 ) {
            page[i] = *src;
        }
        /* Program it into dst */
        iap.write( page, dst, sizeof(page) );

        /* Reset the data in local storage */
        memset(&page[0], 0xFF, sizeof(page));
    }
    iap.erase( UPDATE_SECTOR_START, UPDATE_SECTOR_END );
    __enable_irq();
}
