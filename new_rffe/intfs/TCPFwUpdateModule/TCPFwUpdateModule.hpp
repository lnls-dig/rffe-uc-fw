/*
 * Copyright (C) 2024 CNPEM (cnpem.br)
 * Author: Guilherme Ricioli <guilherme.ricioli@lnls.br>
 */

#ifndef TCPFWUPDATEMODULE_HPP_
#define TCPFWUPDATEMODULE_HPP_

#include "EthernetInterface.h"

#include "Module.hpp"

/* TCPFwUpdateModule is a class providing mechanisms for updating the bootloader
   or the application remotely.
 */
class TCPFwUpdateModule :
  public Module {
    public:
        TCPFwUpdateModule(
            EthernetInterface *p_net, uint16_t port, int timeout,
            const char terminator, const uint8_t max_cmd_len,
            /* Module params */
            osPriority priority, uint32_t stack_size, unsigned char *stack_mem,
            const char *name);
        virtual ~TCPFwUpdateModule();

    private:
        EthernetInterface *_p_net;
        uint16_t _port;
        int _timeout;
        const char _terminator;
        const uint8_t _max_cmd_len;
        mbed::FlashIAP *_p_flash_iap;
        uint32_t _page_size;
        uint8_t *_p_page_buff;
        uint32_t _max_img_size;

        virtual void _task();
  };

#endif /* TCPFWUPDATEMODULE_HPP_ */