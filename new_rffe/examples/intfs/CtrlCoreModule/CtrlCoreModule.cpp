/*
 * Copyright (C) 2023 CNPEM (cnpem.br)
 * Author: Guilherme Ricioli <guilherme.ricioli@lnls.br>
 */

#include "CtrlCoreModule.hpp"

CtrlCoreModule::CtrlCoreModule(
    mbed::Callback<bool(Kernel::Clock::duration_u32, CtrlIntfModuleMessage**)>
    try_get_for_cb,
    /* Module params */
    osPriority priority, uint32_t stack_size, unsigned char *stack_mem,
    const char *name, void * ctx) :
  Module(priority, stack_size, stack_mem, name), _try_get_for_cb(try_get_for_cb), _scpi_mod(&scpi_commands[0], ctx) {}


CtrlCoreModule::~CtrlCoreModule() {}

void CtrlCoreModule::_task() {
  while(true) {
    bool status;

    CtrlIntfModuleMessage *p_ctrl_intf_mod_msg;

    /* Reads messages */
    status = _try_get_for_cb(rtos::Kernel::wait_for_u32_forever,
      &p_ctrl_intf_mod_msg);
      assert(status);
    /* Processes command */
    _scpi_mod.Process(p_ctrl_intf_mod_msg->buff);


    memcpy(p_ctrl_intf_mod_msg->buff, _scpi_mod.Response(), 32);
    

      /* Signalizes that response is ready */
    p_ctrl_intf_mod_msg->p_ready->release();
  }
}

