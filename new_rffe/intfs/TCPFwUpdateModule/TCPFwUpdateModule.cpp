/*
 * Copyright (C) 2024 CNPEM (cnpem.br)
 * Author: Guilherme Ricioli <guilherme.ricioli@lnls.br>
 */

#include "TCPFwUpdateModule.hpp"

#include "newboot.h"

#define TIME_UNTIL_RESET_FW_UPDATE  5s

const uint32_t *p_fw_update_flash_start = &__FWUpdateFlash_start;
const uint32_t *p_fw_update_flash_end   = &__FWUpdateFlash_end;

const fw_info_t *p_fw_info = &__FWInfo_addr;

static uint32_t compute_checksum(const uint8_t * const p, uint32_t size) {
    uint32_t checksum = 0;
    for(const uint8_t *q = p; q != p + size; q++) {
        checksum += *q;
    }

    return checksum;
}

/* Function that returns the associated flash sector base address of a given
   address in flash address space.

   Designed for LPC1768.
*/
uint32_t get_flash_sector_base_addr(const uint32_t addr) {
    uint32_t base_addr;

    assert(addr < 0x80000);

    /* 4kB blocks (16x) */
    if(addr < 0x10000) {
        base_addr = addr - (addr % 0x1000);
    /* 32kB blocks (14x) */
    } else {
        /* 0x10000 is aligned with 0x8000 blocks */
        base_addr = addr - (addr % 0x8000);
    }

    return base_addr;
}

TCPFwUpdateModule::TCPFwUpdateModule(
    EthernetInterface *p_net, uint16_t port, int timeout, const char terminator,
    const uint8_t max_cmd_len,
    /* Module params */
    osPriority priority, uint32_t stack_size, unsigned char *stack_mem,
    const char *name)
    : Module(priority, stack_size, stack_mem, name), _p_net(p_net), _port(port),
        _timeout(timeout),_terminator(terminator),
        _max_cmd_len(max_cmd_len) {
            int status;

            _p_flash_iap = new mbed::FlashIAP();
            assert(_p_flash_iap != NULL);

            status = _p_flash_iap->init();
            assert(status == 0);

            _page_size = _p_flash_iap->get_page_size();
            /* In the validation step, it's assumed that a page can hold the
               fw_info_t header entirely
            */
            assert(_page_size - FW_INFO_SIZE > 0);
            _p_page_buff = new uint8_t [_page_size];
            assert(_p_page_buff != NULL);

            _max_img_size = (uint32_t)(p_fw_update_flash_end) + 1 -
                (uint32_t)(p_fw_update_flash_start);
            /* Flash writing granularity is _page_size. To ease things, the last
               page of the firmware update region is reserved for the fw_info_t
               header.
            */
            _max_img_size -= _page_size;
        }

TCPFwUpdateModule::~TCPFwUpdateModule() {
    int status;

    status = _p_flash_iap->deinit();
    assert(status == 0);

    delete _p_flash_iap;
    delete _p_page_buff;
}

void TCPFwUpdateModule::_task() {
    nsapi_error_t nsapi_status;
    TCPSocket server;
    enum fw_upd_state_t { IDLE, RECEIVE_BIN };
    fw_upd_state_t fw_upd_state = IDLE;
    fw_info_t fw_info;

    server.set_blocking(true);

    nsapi_status = server.open(_p_net);
    debug("[TCPFwUpdateModule::_task] server.open rc: %d\n",
        nsapi_status);
    assert(nsapi_status == NSAPI_ERROR_OK);

    nsapi_status = server.bind(_port);
    debug("[TCPFwUpdateModule::_task] server.bind rc: %d\n",
        nsapi_status);
    assert(nsapi_status == NSAPI_ERROR_OK);

    nsapi_status = server.listen(1);
    debug("[TCPFwUpdateModule::_task] server.listen rc: %d\n",
        nsapi_status);
    assert(nsapi_status == NSAPI_ERROR_OK);

    while(true) {
        TCPSocket *p_client;
        uint32_t page_buff_count = 0;
        uint32_t fw_flashed_count = 0;

        /* Blocking */
        do {
            ThisThread::sleep_for(5s);
            p_client = server.accept(&nsapi_status);
            debug("[TCPFwUpdateModule::_task] server.accept rc: %d\n",
                nsapi_status);
        } while(nsapi_status != NSAPI_ERROR_OK);

        /* Not blocking from now on */
        p_client->set_timeout(_timeout);

        while(true) {
            nsapi_size_or_error_t count;
            /* '+ 1' is accounting for the string terminator */
            char buff[_max_cmd_len + 1];
            char *tok = NULL, *saveptr = NULL;
            bool reset_req = false;

            memset(buff, 0, sizeof(buff));

            /* Reads chars from TCP socket */
            uint8_t len = 0;
            while(true) {
                char c;

                count = p_client->recv(&c, 1);
                if(count <= 0) {
                    debug("[TCPFwUpdateModule::_task] p_client->recv rc: %d\n",
                        count);
                    goto nsapi_error;
                }
                assert(count == 1);

                if(c == _terminator) {
                    debug("[TCPFwUpdateModule::_task] recv buff: %s\n", buff);
                    break;
                } else if(len == _max_cmd_len) {
                    continue;
                } else {
                    buff[len++] = c;
                }
            }

            /* Processes command */
            switch(fw_upd_state) {
                case IDLE:
                    tok = strtok_r(buff, " ", &saveptr);

                    /* @UPD ... */
                    if(strcmp(tok, "@UPD") == 0) {
                        /* ... <FW TYPE> ... */
                        tok = strtok_r(NULL, " ", &saveptr);
                        fw_info.fw_type = atoi(tok);

                        /* ... <BIN SIZE> ... */
                        tok = strtok_r(NULL, " ", &saveptr);
                        fw_info.size = atoi(tok);

                        /* ... <CHECKSUM> ... */
                        tok = strtok_r(NULL, " ", &saveptr);
                        fw_info.checksum = atoi(tok);

                        /* ... <MAJOR> ... */
                        tok = strtok_r(NULL, " ", &saveptr);
                        fw_info.version[0] = atoi(tok);

                        /* ... <MINOR> <PATCH> */
                        tok = strtok_r(NULL, " ", &saveptr);
                        fw_info.version[1] = atoi(tok);
                        fw_info.version[2] = atoi(saveptr);

                        if((fw_info.fw_type == FW_UPDATE_APP ||
                            fw_info.fw_type == FW_UPDATE_BOOT) &&
                            fw_info.size > 0 && fw_info.size <= _max_img_size) {
                            debug("[TCPFwUpdateModule::_task] "
                                "fw_info.{version,fw_type,size,checksum}:\n"
                                "%u.%u%.u, %u, %lu, %lu\n", fw_info.version[0],
                                fw_info.version[1], fw_info.version[2],
                                fw_info.fw_type, fw_info.size, fw_info.checksum);

                            for(const uint32_t *p = p_fw_update_flash_start;
                                p <= p_fw_update_flash_end; p++) {
                                if(*p != 0xFFFFFFFF) {
                                    int status;
                                    uint32_t base_addr;

                                    base_addr =
                                        get_flash_sector_base_addr((uint32_t)p);
                                    status = _p_flash_iap->erase(base_addr,
                                        _p_flash_iap->get_sector_size(
                                            (uint32_t)p));
                                    assert(status == 0);

                                    debug("[TCPFwUpdateModule::_task] "
                                        "flash sector at 0x%08lX erased\n",
                                        base_addr);
                                }
                            }

                            fw_flashed_count = 0;
                            fw_upd_state = RECEIVE_BIN;
                            strncpy(buff, "ACK", _max_cmd_len);
                        } else {
                            strncpy(buff, "NALL", _max_cmd_len);
                        }
                    } else {
                        strncpy(buff, "NACK", _max_cmd_len);
                    }

                    break;
                case RECEIVE_BIN:
                    tok = strtok_r(buff, " ", &saveptr);

                    /* @BIN <CHUNK SIZE> */
                    if(strcmp(tok, "@BIN") == 0) {
                        uint16_t chunk_size;

                        tok = strtok_r(NULL, " ", &saveptr);

                        chunk_size = atoi(tok);
                        if(chunk_size > 0) {
                            if(fw_flashed_count + chunk_size > fw_info.size) {
                                fw_upd_state = IDLE;
                                strncpy(buff, "NALL", _max_cmd_len);
                            } else {
                                uint16_t chunk_remain_count = chunk_size;
                                while(chunk_remain_count > 0) {
                                    uint16_t chunk_read_count;

                                    debug("[TCPFwUpdateModule::_task] "
                                        "chunk_remain_count: %u (total: %u)\n",
                                        chunk_remain_count, chunk_size);

                                    /* Do the remaining bytes of chunk fit page
                                    buff? */
                                    if(chunk_remain_count <=
                                        (_page_size - page_buff_count)) {
                                        chunk_read_count = chunk_remain_count;
                                    } else {
                                        chunk_read_count =
                                            _page_size - page_buff_count;
                                    }

                                    count = p_client->recv(
                                        &_p_page_buff[page_buff_count],
                                        chunk_read_count);
                                    if(count <= 0) {
                                        debug("[TCPFwUpdateModule::_task] "
                                            "p_client->recv rc: %d\n", count);
                                        goto nsapi_error;
                                    }

                                    page_buff_count += count;
                                    chunk_remain_count -= count;

                                    if(fw_flashed_count + page_buff_count ==
                                        fw_info.size ||
                                        page_buff_count == _page_size) {
                                        int status;
                                        uint32_t page_addr;

                                        page_addr =
                                            (uint32_t)p_fw_update_flash_start +
                                            fw_flashed_count;
                                        status =_p_flash_iap->program(
                                            _p_page_buff, page_addr,
                                            _page_size);
                                        debug("[TCPFwUpdateModule::_task] "
                                            "page flashed at 0x%08lX\n",
                                            page_addr);
                                        assert(status == 0);

                                        fw_flashed_count += page_buff_count;
                                        debug("[TCPFwUpdateModule::_task] "
                                            "fw_flashed_count: %lu\n",
                                            fw_flashed_count);

                                        memset(_p_page_buff, 0, _page_size);
                                        page_buff_count = 0;
                                    }

                                }
                                strncpy(buff, "ACK", _max_cmd_len);
                            }

                        } else {
                            strncpy(buff, "NALL", _max_cmd_len);
                        }
                    /* @STP */
                    } else if(strcmp(tok, "@STP") == 0) {
                        fw_upd_state = IDLE;
                        strncpy(buff, "ACK", _max_cmd_len);
                    /* @VAL */
                    } else if(strcmp(tok, "@VAL") == 0) {
                        uint32_t fw_flashed_checksum;

                        fw_flashed_checksum = compute_checksum(
                            (uint8_t *)p_fw_update_flash_start,
                            fw_flashed_count);
                        debug("[TCPFwUpdateModule::_task] "
                            "fw_flashed_checksum: %lu\n", fw_flashed_checksum);

                        if(fw_flashed_count == fw_info.size &&
                           fw_flashed_checksum == fw_info.checksum) {
                            fw_info.magic = 0xAAAAAAAA;
                            debug("[TCPFwUpdateModule::_task] "
                                "fw_info.{version,fw_type,magic,size,checksum}:"
                                "\n%u.%u.%u, %u, 0x%08lX, %lu, %lu\n",
                                fw_info.version[0], fw_info.version[1],
                                fw_info.version[2], fw_info.fw_type,
                                fw_info.magic, fw_info.size, fw_info.checksum);

                            /* TODO: Flash header */
                            int status;
                            uint32_t page_addr;

                            /* The fw_info_t header occupies the last
                               FW_INFO_SIZE bytes of the last page
                            */
                            memcpy(&_p_page_buff[_page_size - FW_INFO_SIZE],
                                &fw_info, sizeof(fw_info));
                            page_addr =
                                (uint32_t)p_fw_update_flash_end + 1 -
                                _page_size;
                            _p_flash_iap->program(_p_page_buff, page_addr,
                                _page_size);
                            debug("[TCPFwUpdateModule::_task] "
                                "page flashed at 0x%08lX (header page)\n",
                                page_addr);
                            assert(status == 0);

                            debug("[TCPFwUpdateModule::_task] "
                                "p_fw_info.{version,fw_type,magic,size,"
                                "checksum}:\n%u.%u.%u, %u, 0x%08lX, %lu, %lu\n",
                                p_fw_info->version[0], p_fw_info->version[1],
                                p_fw_info->version[2], p_fw_info->fw_type,
                                p_fw_info->magic, p_fw_info->size,
                                p_fw_info->checksum);

                            memset(_p_page_buff, 0, _page_size);
                            page_buff_count = 0;

                            reset_req = true;
                            strncpy(buff, "ACK", _max_cmd_len);
                        } else {
                            strncpy(buff, "NACK", _max_cmd_len);
                        }

                        fw_upd_state = IDLE;
                    } else {
                        strncpy(buff, "NACK", _max_cmd_len);
                    }

                    break;
            }

            /* Writes the response to TCP socket */
            count = p_client->send(buff, strlen(buff));
            if(count < 0) {
                debug("[TCPFwUpdateModule::_task] p_client->recv rc: %d\n",
                    count);
                goto nsapi_error;
            }
            debug("[TCPFwUpdateModule::_task] send buff: %s\n", buff);

            count = p_client->send(&_terminator, 1);
            if(count < 0) {
                debug("[TCPFwUpdateModule::_task] p_client->recv rc: %d\n",
                    count);
                goto nsapi_error;
            }

            if(reset_req == true) {
                debug("[TCPFwUpdateModule::_task] Resetting target\n");
                ThisThread::sleep_for(TIME_UNTIL_RESET_FW_UPDATE);
                system_reset();
            }
        }
nsapi_error:
        nsapi_status = p_client->close();
        debug("[TCPFwUpdateModule::_task] p_client->close rc: %d\n",
            nsapi_status);
        assert(nsapi_status == NSAPI_ERROR_OK);

        fw_upd_state = IDLE;
    }
}