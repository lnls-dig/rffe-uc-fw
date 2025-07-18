#ifndef NEWBOOT_H_
#define NEWBOOT_H_

/* TODO: How to proper license this? */

#include "stdint.h"

#define FW_INFO_SIZE    256

enum fw_update_type_t {
    FW_UPDATE_APP = 1,
    FW_UPDATE_BOOT = 2,
};

/* Firmware update header */
typedef struct {
    uint8_t version[3];
    uint8_t fw_type;
    uint32_t magic;
    /* Extended fields from openMMC's struct */
    uint32_t size;
    uint32_t checksum;
} fw_info_t;

/* Firmware update symbols

   Must be defined by the application linker script and must match the ones
   defined in the newboot linker script.
*/
extern const uint32_t __FWUpdateFlash_start;
extern const uint32_t __FWUpdateFlash_end;
extern const fw_info_t __FWInfo_addr;

#endif /* NEWBOOT_H_ */