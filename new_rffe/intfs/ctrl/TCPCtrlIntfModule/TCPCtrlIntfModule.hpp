/*
 * Copyright (C) 2021-2023 CNPEM (cnpem.br)
 * Author: Guilherme Ricioli <guilherme.ricioli@lnls.br>
 */

#ifndef TCPCTRLINTFMODULE_HPP_
#define TCPCTRLINTFMODULE_HPP_

#include "EthernetInterface.h"
#include "mbed.h"

#include "CtrlIntfModule.hpp"

#include "I2CEeprom.hpp"


/* TCPCtrlIntfModule is a class providing text-based TCP command interface.
 */
class TCPCtrlIntfModule final :
  public CtrlIntfModule {
    public:
      TCPCtrlIntfModule(
          EthernetInterface *p_net, uint16_t port, int timeout,
          /* CtrlIntfModule params */
          const char terminator, const uint8_t max_cmd_len,
          mbed::Callback<bool(Kernel::Clock::duration_u32,
            CtrlIntfModuleMessage*, uint8_t)> try_put_for_cb,
          osPriority priority, uint32_t stack_size, unsigned char *stack_mem,
          const char *name);
      ~TCPCtrlIntfModule();

    private:
      EthernetInterface *_p_net;
      uint16_t _port;
      int _timeout;

      void _task();
  };

#endif /* TCPCTRLINTFMODULE_HPP_ */
