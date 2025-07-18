/*
 * Copyright (C) 2023 CNPEM (cnpem.br)
 * Author: Guilherme Ricioli <guilherme.ricioli@lnls.br>
 */

#include "mbed.h"

#include "CtrlCoreModule.hpp"
#include "UARTCtrlIntfModule.hpp"

#define CTRLCOREMODULE_STACK_SIZE 1024
#define UARTCTRLINTFMODULE_STACK_SIZE 1024

/* Statically allocated stacks */
static unsigned char ctrl_core_mod_stack[CTRLCOREMODULE_STACK_SIZE];
static unsigned char uart_ctrl_intf_mod_stack[UARTCTRLINTFMODULE_STACK_SIZE];

/* A simple counter example.
 *
 * Instantiates an UARTCtrlIntfModule and a CtrlCoreModule, invoking its
 * threads. The CtrlCoreModule module is designed for processing control
 * commands.
 *
 * The UART peripheral used is the one provided via USB (you'll need to adapt
 * this if your board doesn't provide one), with baud rate set to 115200. The
 * terminator char is '\r'. Valid commands are:
 *   '@INC', to increment the counter;
 *   '@DEC', to decrement the counter;
 *   '@ZERO', to zero the counter.
 * All commands return counter's value.
 */
int main() {
  bool status;
  rtos::Queue<CtrlIntfModuleMessage, 1> queue;
  CtrlCoreModule ctrl_core_mod(
      callback(&queue, &rtos::Queue<CtrlIntfModuleMessage, 1>::try_get_for),
      osPriorityNormal, CTRLCOREMODULE_STACK_SIZE, ctrl_core_mod_stack,
      "ctrl_core_mod_task");
  UARTCtrlIntfModule uart_ctrl_intf_mod(USBTX, USBRX, 115200, '\n', 32,
      callback(&queue, &rtos::Queue<CtrlIntfModuleMessage, 1>::try_put_for),
      osPriorityNormal, UARTCTRLINTFMODULE_STACK_SIZE, uart_ctrl_intf_mod_stack,
      "uart_ctrl_intf_mod_task");

  status = ctrl_core_mod.start();
  assert(status);
  status = uart_ctrl_intf_mod.start();
  assert(status);

  while(true) {}

  return 0;
}
