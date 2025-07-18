/*
 * Copyright (C) 2021-2023 CNPEM (cnpem.br)
 * Author: Guilherme Ricioli <guilherme.ricioli@lnls.br>
 */

#ifndef MODULE_HPP_
#define MODULE_HPP_

#include "mbed.h"

/* Module is a class providing the basics for thread-based modules.
 */
class Module {
  public:
    /* NOTE: One can dinamically allocate the stack memory by passing
     *       'stack_mem' as nullptr. This is not recomended as it may lead to
     *       memory fragmentation. Static allocation is preferred.
     */
    Module(osPriority priority, uint32_t stack_size, unsigned char *stack_mem,
        const char *name) {
      _p_thread = new rtos::Thread(priority, stack_size, stack_mem, name);
    }

    virtual ~Module() {
      delete _p_thread;
    }

    /* Spawns a thread assigning internal method _task() for it to run. */
    bool start() {
      mbed::Callback<void()> cb = callback(this, &Module::_task);

      return _p_thread->start(cb) == osOK? true : false;
    }

  private:
    rtos::Thread *_p_thread;

    /* A method to be executed by the internal thread when start() is called.
     * NOTE: Inherited classes must implement this.
     */
    virtual void _task() = 0;
};

#endif /* MODULE_HPP_ */
