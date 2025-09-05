/**
 *    @file     main.cpp
 *    @author   João Victor Santos (joao.ssantos@lnls.br)
 *
 *    @brief    Entry point for the RFFE (Radio Frequency Front-End) firmware.
 *
 *    @details
 *      This file initializes the hardware and starts the main control tasks of
 *      the RFFE system. The workflow includes:
 *        - Initializing the I²C bus and configuring the CDCE906 PLL synthesizer
 *          with the predefined settings in `pll_cfg.hpp`
 *        - Creating and initializing the global RFFE context (`GlobalRFFE`),
 *          which manages the digital attenuator, FRAM-backed configuration,
 *          and temperature sensors
 *        - Configuring the Ethernet interface according to persisted FRAM
 *          parameters, with DHCP/static fallback logic
 *        - Setting up the SCPI control core (`CtrlCoreModule`) and the TCP
 *          control interface (`TCPCtrlIntfModule`), wired together with an
 *          RTOS queue for command exchange
 *        - Bringing up the Ethernet interface in non-blocking mode, connecting
 *          to the network, and starting the control tasks
 *        - Running a monitoring loop that:
 *            - Polls connection status every few seconds
 *            - Prints MAC and IP addresses when available
 *            - Waits until the link drops, then restarts the cycle
 *
 *    @date 2025
 *
 *    @copyright
 *      Copyright (C) 2025 CNPEM (cnpem.br)
 *
 *      rffe-uc-fw is free software: you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation, either version 3 of the License, or
 *      (at your option) any later version.
 *
 *      rffe-uc-fw is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with RFFE.  If not, see <https://www.gnu.org/licenses/>.
 */


#include "mbed.h"
#include "EthernetInterface.h"

#include "CtrlCoreModule.hpp"
#include "GlobalRFFE.hpp"
#include "TCPCtrlIntfModule.hpp"
#include "pll_cfg.hpp"

#define CTRLCOREMODULE_STACK_SIZE 1024
#define TCPCTRLINTFMODULE_STACK_SIZE 1024

/* Statically allocated stacks */
static unsigned char ctrl_core_mod_stack[CTRLCOREMODULE_STACK_SIZE];
static unsigned char tcp_ctrl_intf_mod_stack[TCPCTRLINTFMODULE_STACK_SIZE];


int main() {
    I2C i2c(P0_27 , P0_28);
    int8_t addr7bit = 0x69;
    int8_t addr8bit = addr7bit << 1;

    CDCE906 pll(&i2c, addr8bit);

    uint8_t arr_write[25];

    if (pll.cdce906_write_cfg(write_cfg, arr_write) != 0)
    {
        debug("[PLL][ERROR] Failed to write configuration to CDCE906\n");
        return -1;
    }

    static GlobalRFFE rffe;
    bool status;
    nsapi_error_t nsapi_status;
    rtos::Queue<CtrlIntfModuleMessage, 1> queue;


    EthernetInterface net;


    rffe.config_get_eth_mode(net);

    CtrlCoreModule ctrl_core_mod(
        callback(&queue, &rtos::Queue<CtrlIntfModuleMessage, 1>::try_get_for),
        osPriorityNormal, CTRLCOREMODULE_STACK_SIZE, ctrl_core_mod_stack,
        "ctrl_core_mod_task", &rffe);
    TCPCtrlIntfModule tcp_ctrl_intf_mod(&net, 9001, 30000, '\n', 32,
        callback(&queue, &rtos::Queue<CtrlIntfModuleMessage, 1>::try_put_for),
        osPriorityNormal, TCPCTRLINTFMODULE_STACK_SIZE, tcp_ctrl_intf_mod_stack,
        "tcp_ctrl_intf_mod_task");

    nsapi_status = net.set_blocking(false);
    assert(nsapi_status == NSAPI_ERROR_OK);

    nsapi_status = net.connect();
    assert(nsapi_status == NSAPI_ERROR_OK);

    status = ctrl_core_mod.start();
    assert(status);
    status = tcp_ctrl_intf_mod.start();
    assert(status);



    while(true) {
      SocketAddress addr;

      do {
          ThisThread::sleep_for(5s);
          nsapi_status = net.get_connection_status();
          debug("[main] net.get_connection_status rc: %d\n", nsapi_status);
      } while(nsapi_status != NSAPI_STATUS_GLOBAL_UP);

      debug("[main] MAC: %s\n", net.get_mac_address());

      nsapi_status = net.get_ip_address(&addr);
      debug("[main] net.get_ip_address rc: %d\n", nsapi_status);
      assert(nsapi_status == NSAPI_ERROR_OK);
      debug("[main] IP: %s\n", addr.get_ip_address());

      do {
          ThisThread::sleep_for(500ms);
          nsapi_status = net.get_connection_status();
      } while(nsapi_status == NSAPI_STATUS_GLOBAL_UP);
    }

    return 0;
}
