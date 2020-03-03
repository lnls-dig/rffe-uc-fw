# RFFE Bootloader

The flash memory is divided in three regions:
* Bootloader 0x00000000 - 0x0000FFFF (64KiB);
* Application 0x00010000 - 0x00047FFF (224KiB);
* Firmware update 0x00048000 - 0x00080000 (224KiB).

```
New firmware record at flash address 0x0007FFF0:

  +-----------------+-----------------+-----------------+---------------+------------+
  |  Major version  |  Minor version  |  Build version  | Firmware type | Magic word |
  | number (1 byte) | number (1 byte) | number (1 byte) |   (1 byte)    | (4 bytes)  |
  +-----------------+-----------------+-----------------+---------------+------------+

```

The bootloader checks the last 32bit word in flash is equal to 0xAAAAAAAA (firmware update magic word), if it is, the new firmware will be copied from 0x00048000 to 0x00010000 or 0x00000000 depending on the firmware type (application or bootloader). After finishing the copying, the bootloader will erase the last flash sector to prevent unnecessary rewrites to flash after reset.

The Firmware type byte indicates what to update, (0x01: application, 0x02: bootloader). All flash writing logic is executed from SRAM to allow self updating.

