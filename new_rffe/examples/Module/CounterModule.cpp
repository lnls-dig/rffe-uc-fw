/*
 * Copyright (C) 2023 CNPEM (cnpem.br)
 * Author: Guilherme Ricioli <guilherme.ricioli@lnls.br>
 */

#include "CounterModule.hpp"

CounterModule::CounterModule(
    mbed::Callback<bool(Kernel::Clock::duration_u32, CountCmdMessage**)>
    try_get_for_cb,
    /* Module params */
    osPriority priority, uint32_t stack_size, unsigned char *stack_mem, const
    char *name) :
  Module(priority, stack_size, stack_mem, name),
  _try_get_for_cb(try_get_for_cb) {
    _count = 0;
  }

CounterModule::~CounterModule() {}

void CounterModule::_task() {
  while(true) {
    bool status;
    CountCmdMessage *count_cmd_msg;

    /* Reads messages */
    status = _try_get_for_cb(rtos::Kernel::wait_for_u32_forever,
        &count_cmd_msg);
    assert(status);

    /* Process command */
    switch(*count_cmd_msg) {
      case INCREASE:
        ++_count;
        break;
      case DECREASE:
        if(_count > 0) {
          --_count;
        }
        break;
      case ZERO:
        _count = 0;
        break;
    }

    /* Prints counter */
    printf("%lu\n", _count);
  }
}
