/*
 * Copyright (C) 2023 CNPEM (cnpem.br)
 * Author: Guilherme Ricioli <guilherme.ricioli@lnls.br>
 */

#include "mbed.h"

#include "CounterModule.hpp"

#define COUNTERMODULE_STACK_SIZE 1024

/* Statically allocated stack */
static unsigned char count_mod_stack[COUNTERMODULE_STACK_SIZE];

int main() {
  bool status;
  rtos::Queue<CountCmdMessage, 1> queue;
  CounterModule count_mod(
      callback(&queue, &rtos::Queue<CountCmdMessage, 1>::try_get_for),
      osPriorityNormal, COUNTERMODULE_STACK_SIZE, count_mod_stack,
      "count_mod_task");

  status = count_mod.start();
  assert(status);

  while(true) {
    char c;
    CountCmdMessage count_cmd_msg;
    bool is_valid_key = true;

    c = getc(stdin);
    switch(c) {
      case 'i':
      case 'I':
        count_cmd_msg = INCREASE;
        break;
      case 'd':
      case 'D':
        count_cmd_msg = DECREASE;
        break;
      case 'z':
      case 'Z':
        count_cmd_msg = ZERO;
        break;
      default:
        is_valid_key = false;
    }

    if(is_valid_key) {
      /* Messages CounterModule with requested command */
      status = queue.try_put_for(rtos::Kernel::wait_for_u32_forever,
          &count_cmd_msg);
      assert(status);
    } else {
      printf("Valid keys are: i, d, z\n");
    }
  }

  return 0;
}
