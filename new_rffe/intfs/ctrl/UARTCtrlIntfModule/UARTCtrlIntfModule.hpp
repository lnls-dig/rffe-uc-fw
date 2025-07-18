/*
 * Copyright (C) 2021-2023 CNPEM (cnpem.br)
 * Author: Guilherme Ricioli <guilherme.ricioli@lnls.br>
 */

#ifndef UARTCTRLINTFMODULE_HPP_
#define UARTCTRLINTFMODULE_HPP_

#include "mbed.h"

#include "CtrlIntfModule.hpp"

/* UARTCtrlIntfModule is a class providing text-based UART command interface.
 */
class UARTCtrlIntfModule final :
  public CtrlIntfModule {
    public:
      UARTCtrlIntfModule(
          PinName tx, PinName rx, int baud,
          /* CtrlIntfModule params */
          const char terminator, const uint8_t max_cmd_len,
          mbed::Callback<bool(Kernel::Clock::duration_u32,
            CtrlIntfModuleMessage*, uint8_t)> try_put_for_cb,
          osPriority priority, uint32_t stack_size, unsigned char *stack_mem,
          const char *name);
      ~UARTCtrlIntfModule();

    private:
      PinName _tx, _rx;
      int _baud;

      void _task();
  };

#endif /* UARTCTRLINTFMODULE_HPP_ */
