# RFFE Firmware (Mbed OS)

## Overview

This repository contains the firmware for the **Radio Frequency Front-End (RFFE)** system, developed on top of **Mbed OS**.
It provides control and monitoring of hardware modules via **SCPI commands over TCP/IP**, supporting configuration, attenuation control, temperature monitoring, and network management.

Main features include:

* Control of the **DAT-31R5-SP+ digital attenuator** via GPIO bit-banging.
* Configuration of the **CDCE906 PLL synthesizer** for clock generation.
* Persistent storage of parameters (IP, mask, gateway, MAC, attenuation, etc.) in **I²C FRAM (FERAM)**.
* Temperature measurement using **LM71 sensors** via SPI.
* **SCPI command interface** exposed over TCP (default port `9001`).

---

## Repository Structure

```
app/
├── main.cpp                     # Firmware entry point
├── CMakeLists.txt               # Build configuration for Mbed OS
│
├── CtrlCoreModule/              # SCPI core processing
│   ├── CtrlCoreModule.cpp
│   └── CtrlCoreModule.hpp
│
├── GlobalRFFE/                  # Global context: hardware drivers + FRAM-backed config
│   ├── GlobalRFFE.cpp
│   └── GlobalRFFE.hpp
│
├── PLL/                         # CDCE906 PLL synthesizer config
│   ├── pll_cfg.cpp
│   └── pll_cfg.hpp
│
└── SCPI_RFFE/                   # SCPI command handlers and tables
    ├── scpi_cmd.cpp
    ├── scpi_cmd.hpp
    ├── scpi_tables.cpp
    └── scpi_tables.hpp
```

---

## Build Instructions

### Requirements

* [Mbed OS](https://os.mbed.com/) source tree (path must be provided via `-DMBED_OS_PATH`)
* CMake ≥ 3.16
* Ninja build system
* ARM GCC toolchain (e.g. `arm-none-eabi-gcc`)

### Steps

```bash
# From inside the build directory:
cmake .. -GNinja \
  -DMBED_TARGET=LPC1768 \
  -DMBED_OS_PATH=/home/joao.ssantos/repos/rffe-uc-fw/new_rffe/mbed-os

# Build firmware
ninja
```

The output binaries (`.elf`, `.bin`, `.hex`) are placed in `app/build/`.

---

## Usage

1. Flash the firmware to the target board (tested on **NXP LPC1768**).
2. On boot, the firmware:

   * Configures the PLL via I²C.
   * Applies Ethernet configuration from FRAM (DHCP/static).
   * Starts the SCPI command server on TCP port `9001`.
3. Connect via a TCP client and issue SCPI commands.

---

## Supported SCPI Commands

Examples:

* **Attenuation control**

  * `SET:ATTEnuation <value>`
  * `GET:ATTEnuation?`

* **Network configuration**

  * `SET:IPAddr <addr>` / `GET:IPAddr?`
  * `SET:NETMask <mask>` / `GET:NETMask?`
  * `SET:GATEwayaddr <addr>` / `GET:GATEwayaddr?`
  * `SET:DHCPMode <0|1>` / `GET:DHCPMode?`

* **System**

  * `GET:VERsion?`
  * `SYSTem:RESet`

* **Measurements**

  * `MEASure:TEMPerature:AC?`
  * `MEASure:TEMPerature:BD?`

---

## Planned Features

The following features are planned but not yet implemented:

* **PID-based temperature control**
  Implement a closed-loop controller using LM71 sensors to stabilize thermal conditions.

* **Remote firmware update**
  Support in-field updates through a dedicated update module and custom linker script.

* **Embed Git commit hash in firmware**
  Include build-time metadata in the binary for version traceability.

---

## License

```
rffe-uc-fw is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

rffe-uc-fw is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.
```

