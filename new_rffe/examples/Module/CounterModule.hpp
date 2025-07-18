  /*
 * Copyright (C) 2023 CNPEM (cnpem.br)
 * Author: Guilherme Ricioli <guilherme.ricioli@lnls.br>
 */

#ifndef COUNTERMODULE_HPP_
#define COUNTERMODULE_HPP_

#include "Module.hpp"

enum CountCmdMessage {
  INCREASE,
  DECREASE,
  ZERO
};

class CounterModule :
  public Module {
    public:
      CounterModule(
          mbed::Callback<bool(Kernel::Clock::duration_u32, CountCmdMessage**)>
          try_get_for_cb,
          /* Module params */
          osPriority priority, uint32_t stack_size,
          unsigned char *stack_mem, const char *name);
      ~CounterModule();

    private:
      uint32_t _count;
      mbed::Callback<bool(Kernel::Clock::duration_u32, CountCmdMessage**)>
        _try_get_for_cb;

      void _task();
  };

#endif /* COUNTERMODULE_HPP_ */
