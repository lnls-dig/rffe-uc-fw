/*
 * Copyright (C) 2021-2023 CNPEM (cnpem.br)
 * Author: Guilherme Ricioli <guilherme.ricioli@lnls.br>
 */

#include "UARTCtrlIntfModule.hpp"

UARTCtrlIntfModule::UARTCtrlIntfModule(
    PinName tx, PinName rx, int baud,
    /* CtrlIntfModule params */
    const char terminator, const uint8_t max_cmd_len,
    mbed::Callback<bool(Kernel::Clock::duration_u32, CtrlIntfModuleMessage*,
      uint8_t)> try_put_for_cb, osPriority priority, uint32_t stack_size,
    unsigned char *stack_mem, const char *name) :
  CtrlIntfModule(terminator, max_cmd_len, try_put_for_cb, priority, stack_size,
  stack_mem, name), _tx(tx), _rx(rx), _baud(baud) {}

UARTCtrlIntfModule::~UARTCtrlIntfModule() {}

void UARTCtrlIntfModule::_task() {
  bool status;
  mbed::BufferedSerial uart(_tx, _rx, _baud);
  /* '+ 1' is accounting for the string terminator */
  char buff[_max_cmd_len + 1];
  rtos::Semaphore ready(0, 1);
  CtrlIntfModuleMessage ctrl_intf_mod_msg(buff, &ready);

  while(true) {
    uint8_t count;

    memset(buff, 0, sizeof(buff));

    /* Reads chars from UART */
    uint8_t len = 0;
    while(true) {
      char c;

      count = uart.read(&c, 1);
      assert(count == 1);

      if(c == _terminator) {
        break;
      } else if(len == _max_cmd_len) {
        continue;
      } else {
        buff[len++] = c;
      }
    }

    /* Queues the command */
    status = _try_put_for_cb(rtos::Kernel::wait_for_u32_forever,
        &ctrl_intf_mod_msg, 0);
    assert(status);

    /* Waits for the command to be processed and its response be filled in
     * 'buff'
     */
    ready.acquire();

    /* Writes the response to UART */
    count = uart.write(buff, strlen(buff));
    assert(count == strlen(buff));
    count = uart.write(&_terminator, 1);
    assert(count == 1);
  }
}
