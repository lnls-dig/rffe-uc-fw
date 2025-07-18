/*
 * Copyright (C) 2023 CNPEM (cnpem.br)
 * Author: Guilherme Ricioli <guilherme.ricioli@lnls.br>
 */

#include "CtrlIntfModule.hpp"

CtrlIntfModule::CtrlIntfModule(const char terminator, const uint8_t max_cmd_len,
    mbed::Callback<bool(Kernel::Clock::duration_u32, CtrlIntfModuleMessage*,
      uint8_t)> try_put_for_cb,
    /* Module params */
    osPriority priority, uint32_t stack_size, unsigned char *stack_mem,
    const char *name) :
  Module(priority, stack_size, stack_mem, name), _terminator(terminator),
  _max_cmd_len(max_cmd_len), _try_put_for_cb(try_put_for_cb) {}

CtrlIntfModule::~CtrlIntfModule() {}
