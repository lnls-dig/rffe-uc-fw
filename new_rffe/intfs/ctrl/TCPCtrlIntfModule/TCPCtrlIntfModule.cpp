/*
 * Copyright (C) 2021-2023 CNPEM (cnpem.br)
 * Author: Guilherme Ricioli <guilherme.ricioli@lnls.br>
 */

#include "TCPCtrlIntfModule.hpp"


TCPCtrlIntfModule::TCPCtrlIntfModule(
    EthernetInterface *p_net, uint16_t port, int timeout,
    /* CtrlIntfModule params */
    const char terminator, const uint8_t max_cmd_len,
    mbed::Callback<bool(Kernel::Clock::duration_u32, CtrlIntfModuleMessage*,
      uint8_t)> try_put_for_cb, osPriority priority, uint32_t stack_size,
    unsigned char *stack_mem, const char *name) :
  CtrlIntfModule(terminator, max_cmd_len, try_put_for_cb, priority, stack_size,
      stack_mem, name), _p_net(p_net), _port(port), _timeout(timeout) {}

TCPCtrlIntfModule::~TCPCtrlIntfModule() {}

void TCPCtrlIntfModule::_task() {
  /* '+ 1' is accounting for the string terminator */
  char buff[_max_cmd_len + 1];
  rtos::Semaphore ready(0, 1);
  CtrlIntfModuleMessage ctrl_intf_mod_msg(buff, &ready);

  nsapi_error_t nsapi_status;
  TCPSocket server;

  server.set_blocking(true);

  nsapi_status = server.open(_p_net);
  debug("[TCPCtrlIntfModule::_task] server.open rc: %d\n",nsapi_status);
  assert(nsapi_status == NSAPI_ERROR_OK);

  nsapi_status = server.bind(_port);
  debug("[TCPCtrlIntfModule::_task] server.bind rc: %d\n", nsapi_status);
  assert(nsapi_status == NSAPI_ERROR_OK);

  nsapi_status = server.listen(1);
  debug("[TCPCtrlIntfModule::_task] server.listen rc: %d\n", nsapi_status);
  assert(nsapi_status == NSAPI_ERROR_OK);

  while(true) {
    nsapi_size_or_error_t count;
    TCPSocket *client;

    /* Blocking */
    do {
        ThisThread::sleep_for(5s);
        client = server.accept(&nsapi_status);
        debug("[TCPCtrlIntfModule::_task] server.accept rc: %d\n",
            nsapi_status);
    } while(nsapi_status != NSAPI_ERROR_OK);

    /* Not blocking from now on */
    client->set_timeout(_timeout);

    while(true) {
      memset(buff, 0, sizeof(buff));

      /* Reads chars from TCP socket */
      uint8_t len = 0;
      while(true) {
        char c;

        count = client->recv(&c, 1);
        // debug("[TCPCtrlIntfModule::_task] client->recv rc: %d\n", count);
        if(count <= 0) {
          goto nsapi_error;
        }
        assert(count == 1);

        if(c == _terminator) {
          break;
        } else if(len == _max_cmd_len) {
          continue;
        } else {
          buff[len++] = c;
        }
      }

      debug("[TCPCtrlIntfModule::_task] client->recv : ");
      for (char c : buff)
      {
        debug("%c", c);
      }
      debug("\n");


      /* Queues the command */
      nsapi_status = _try_put_for_cb(rtos::Kernel::wait_for_u32_forever,
        &ctrl_intf_mod_msg, 0);
      assert(nsapi_status);

      /* Waits for the command to be processed and its response be filled in
       * 'buff'
       */
      ready.acquire();

      /* Writes the response to TCP socket */
      if (buff[0] != '\0')
      {
        count = client->send(buff, strlen(buff));
        debug("[TCPCtrlIntfModule::_task] client->send : ");
        for (char c : buff)
        {
          debug("%c", c);
        }
        debug("\n");
      }
      // debug("[TCPCtrlIntfModule::_task] client->send rc: %d\n", count);
      if(count < 0) {
        goto nsapi_error;
      }
      // count = client->send(&_terminator, 1);
      // debug("[TCPCtrlIntfModule::_task] client->send rc: %d\n", count);
      if(count < 0) {
        goto nsapi_error;
      }
    }

nsapi_error:
    nsapi_status = client->close();
    debug("[TCPCtrlIntfModule::_task] client->close rc: %d\n", nsapi_status);

    assert(nsapi_status == NSAPI_ERROR_OK);
  }
}
