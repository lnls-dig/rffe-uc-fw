/*
 * Copyright (C) 2023 CNPEM (cnpem.br)
 * Author: Guilherme Ricioli <guilherme.ricioli@lnls.br>
 */

#ifndef CTRLINTFMODULE_HPP_
#define CTRLINTFMODULE_HPP_

#include "mbed.h"

#include "CtrlIntfModuleMessage.hpp"
#include "Module.hpp"

/* CtrlIntfModule is a class providing the basics for interface modules.
 */
class CtrlIntfModule :
  public Module {
    public:
      /* Besides Module's params, receives the command request/response
       * terminator, its maximum length and a callback for queueing messages.
       */
      CtrlIntfModule(const char terminator, const uint8_t max_cmd_len,
          mbed::Callback<bool(Kernel::Clock::duration_u32, CtrlIntfModuleMessage*,
            uint8_t)> try_put_for_cb,
          /* Module params */
          osPriority priority, uint32_t stack_size, unsigned char *stack_mem,
          const char *name);
      virtual ~CtrlIntfModule();

    protected:
      const char _terminator;
      const uint8_t _max_cmd_len;
      mbed::Callback<bool(Kernel::Clock::duration_u32, CtrlIntfModuleMessage*,
          uint8_t)> _try_put_for_cb;

    private:
      virtual void _task() = 0;
  };

#endif /* CTRLINTFMODULE_HPP_ */
