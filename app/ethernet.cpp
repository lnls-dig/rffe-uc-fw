#if 0
/* Ethernet PLL */
I2C pll_i2c(P0_27, P0_28);
CDCE906 pll(pll_i2c, 0b11010010);

RFFEServer::RFFEServer()
{

}

void RFFEServer::eth_init()
{
    _pll.cfg_eth();
    ThisThread::sleep_for(100);

    if (_addressing == DHCP) {
        _net.set_dhcp(true);
    } else if (_addressing == FIX_IP) {
        _net.set_network(IP_Addr,Mask_Addr,Gateway_Addr);
    }
}

void RFFEServer::eth_connect()
{
    int err;

    printf("Trying to bring up ethernet connection...\r\n");

    /* Blocking connection */
    do {
        err = _net.connect();

        if (!_cable_status.link()) {
            printf(MODULE_STR" Cable disconnected!\r\n");
        } else {
            switch (err) {
            case NSAPI_ERROR_OK:
                printf(MODULE_STR" Interface is UP!\r\n");
                break;

            case NSAPI_ERROR_NO_CONNECTION:
                printf(MODULE_STR" Link up has failed. Check if the cable link functional.\r\n");
                break;

            case NSAPI_ERROR_DHCP_FAILURE:
                printf(MODULE_STR" DHCP Failure. Could not obtain an IP Address.\r\n");
                printf("Please check if the address configuration server is functional or change the addressing to Fixed IP via CLI (type \"help\" for more info).\r\n");
                break;

            case NSAPI_ERROR_ALREADY:
                /* The ethernet interface is still trying to connect */
                break;

            default:
                printf(MODULE_STR" Unknown error: %d\r\n", err);
                break;
            }
        }

        ThisThread::sleep_for(1000);
    } while ( err != NSAPI_ERROR_OK );

    /* Bring up TCP Server */
    _server.open(&_net);
    _server.bind(_net.get_ip_address(), _port);
    _server.listen();
    _server.set_blocking(1500);

    printf(MODULE_STR" Listening on port %d...\r\n", _port);
}

const char* RFFEServer::get_ip_address( void )
{
    return _net.get_ip_address();
}

const char* RFFEServer::get_gateway( void )
{
    return _net.get_gateway();
}

const char* RFFEServer::get_netmask( void )
{
    return _net.get_netmask();
}

void EthernetThread( void )
{
    uint8_t state = 0;
    uint32_t last_page_addr, next_sector;
    bool full_page = false, sector_erased = false;
    uint8_t v_major = 0, v_minor = 0, v_patch = 0;
    uint8_t buf[BUFSIZE];

    int recv_sz, sent_sz;

    RFFEServer rffe_eth(&pll, FIX_IP);

    /* Ethernet initialization */
    rffe_eth.eth_init();

    /* Try to connect the ethernet interface (will block here until ready) */
    rffe_eth.eth_connect();

    //led3 = 1;

    if (get_value8(Eth_Addressing) == DHCP) {
        set_value(IP_Addr, rffe_eth.get_ip_address(), sizeof(IP_Addr));
        set_value(Gateway_Addr, rffe_eth.get_gateway(), sizeof(Gateway_Addr));
        set_value(Mask_Addr, rffe_eth.get_netmask(), sizeof(Mask_Addr));

        printf("Ethernet configs from DHCP:\r\n");
        printf("\tIP: %s\r\n", IP_Addr);
        printf("\tNetmask: %s\r\n", Mask_Addr);
        printf("\tGateway: %s\r\n", Gateway_Addr);
    }

    while (true) {
        printf("Waiting for new client connection...\r\n");

        server.accept(&client, &client_addr);
        client.set_blocking(1500);

        printf("Connection from client: %s\r\n", client_addr.get_ip_address());

        while ( cable_status.link() ) {
            /* Wait to receive data from client */
            recv_sz = client.recv((char*)buf, 3);

            if (recv_sz == 3) {
                /* We received a complete message header */
                uint16_t payload_len = (buf[1] << 8) | buf[2];
                /* Check if we need to receive some more bytes. This
                 * fixes #9 github issue, in that we end up stuck here
                 * waiting for more bytes that never comes */
                if (payload_len > 0) {
                    recv_sz += client.recv( (char*) &buf[3], payload_len );
                }
            } else if (recv_sz <= 0) {
                /* Special case for disconnections - just discard the socket and await a new connection */
                break;
            } else {
                printf("Received malformed message header of size: %d , discarding...", recv_sz );
                continue;
            }

#ifdef DEBUG_PRINTF
            printf("Received message of %d bytes: ", recv_sz);
            for (int i = 0; i < recv_sz; i++) {
                printf("0x%X ",buf[i]);
            }
            printf("\r\n");
#endif
            bsmp_mail_t *mail = bsmp_mail_box.alloc();

            mail->response_mail_box = &eth_mail_box;
            mail->msg.data = buf;
            mail->msg.len = recv_sz;

            bsmp_mail_box.put(mail);

            osEvent evt = eth_mail_box.get();

            if (evt.status != osEventMail) {
                /* Quietly ignore errors for now */
                continue;
            }

            struct bsmp_raw_packet *response_mail = (struct bsmp_raw_packet *)evt.value.p;

            sent_sz = client.send((char*)response_mail->data, response_mail->len);

#ifdef DEBUG_PRINTF
            printf("Sending message of %d bytes: ", sent_sz);
            for (int i = 0; i < sent_sz; i++) {
                printf("0x%X ",response_mail->data[i]);
            }
            printf("\r\n");
#endif
            free(response_mail->data);
            eth_mail_box.free(response_mail);

            if (sent_sz <= 0) {
                printf("ERROR while writing to socket!\r\n");
                continue;
            }

            if (state != get_value8(Reprogramming)) {
                switch (get_value8(Reprogramming)) {
                case 1:
                    /* Read new firmware version */
                    v_major = Data[0];
                    v_minor = Data[1];
                    v_patch = Data[2];

                    sector_erased = false;
                    last_page_addr = (APPLICATION_ADDR + APPLICATION_SIZE);
                    next_sector = last_page_addr + flash.get_sector_size(last_page_addr);
                    memset(fw_buffer, 0xFF, sizeof(fw_buffer));
                    break;

                case 2:
                {
                    const uint32_t magic_addr = flash.get_flash_size() - 256;
                    memcpy(fw_buffer, (uint32_t *)magic_addr, 256);

                    /* Store version number */
                    fw_buffer[248] = v_major;
                    fw_buffer[249] = v_minor;
                    fw_buffer[250] = v_patch;

                    /* Write the bootloader magic word in the last 4 bytes of the page */
                    const uint32_t magic_word[] = {0xAAAAAAAA};

                    printf("Writing bootloader magic word at 0x%lX\r\n", magic_addr + 252);
                    memcpy(&fw_buffer[252], magic_word, sizeof(magic_word));

                    /* Write back to flash */
                    __disable_irq();
                    flash.write((uint32_t *)fw_buffer, magic_addr, 256);
                    __enable_irq();
                    break;
                }
                default:
                    break;
                }
                state = get_value8(Reprogramming);
            }

            if (get_value8(Reprogramming) == 1 && buf[0] == 0x20 && buf[3] == 0x0A ) {
                if (full_page == false) {
                    memcpy(fw_buffer, Data, FILE_DATASIZE);
                    full_page = true;
                } else {
                    memcpy(&fw_buffer[FILE_DATASIZE], Data, FILE_DATASIZE);
#ifdef DEBUG_PRINTF
                    printf("[REPROGRAM] Writing page 0x%X\r\n", last_page_addr);
#endif
                    /* A full firmware page was sent, copy data to file */
                    if (!sector_erased) {
                        __disable_irq();
                        flash.erase(flash.find_sector(last_page_addr));
                        __enable_irq();
                        sector_erased = true;
                    }
                    __disable_irq();
                    flash.write((uint32_t *)fw_buffer, last_page_addr, 256);
                    __enable_irq();
                    last_page_addr += 256;
                    if (last_page_addr >= next_sector) {
                        next_sector = last_page_addr + flash.get_sector_size(last_page_addr);
                        sector_erased = false;
                    }

                    full_page = false;
                }
            }

            if (get_value8(Reset) == 1) {
                printf("Resetting MBED!\r\n");
                mbed_reset();
            }
        }

        client.close();
        printf("Client Disconnected!\r\n");

        if (cable_status.link() == 0) {
            /* Eth link is down, clean-up server connection */
            server.close();
            net.disconnect();
            break;
        }
    }
}
#endif
