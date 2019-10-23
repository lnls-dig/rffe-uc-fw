#ifndef BSMP_SERVER_PRIV_H
#define BSMP_SERVER_PRIV_H

#include <stdint.h>
#include "bsmp.h"
#include "server.h"

#define VERSION     2
#define SUBVERSION  0
#define REVISION    0   // unused

#define MESSAGE_SET_ANSWER(msg, code)\
    do {\
        (msg)->command_code = (code);\
        (msg)->payload_size = 0;\
    }while(0)

#define MESSAGE_SET_ANSWER_RET(msg, code)\
    do {\
        (msg)->command_code = (code);\
        (msg)->payload_size = 0;\
        return;\
    }while(0)

struct message
{
    uint8_t  command_code;
    uint16_t payload_size;
    uint8_t  *payload;
};

struct generic_list
{
    uint32_t count;
    void *list[];
};

#define SERVER_CMD_FUNCTION(name) \
    void name (bsmp_server_t *server, struct message *recv_msg, \
               struct message *send_msg)

typedef SERVER_CMD_FUNCTION((*command_function_t));

struct bsmp_server
{
    struct bsmp_var_ptr_list    vars;
    struct bsmp_group_list      groups;
    struct bsmp_curve_ptr_list  curves;
    struct bsmp_func_ptr_list   funcs;

    struct bsmp_var             *modified_list[BSMP_MAX_VARIABLES+1];
    bsmp_hook_t                 hook;
};

enum bsmp_err var_check     (struct bsmp_var *var);
enum bsmp_err curve_check   (struct bsmp_curve *curve);
enum bsmp_err func_check    (struct bsmp_func *func);

void          group_init    (struct bsmp_group *grp, uint8_t id);
void          group_add_var (struct bsmp_group *grp, struct bsmp_var *var);

SERVER_CMD_FUNCTION (query_version);
SERVER_CMD_FUNCTION (var_query_list);
SERVER_CMD_FUNCTION (var_read);
SERVER_CMD_FUNCTION (var_write);
SERVER_CMD_FUNCTION (var_write_read);
SERVER_CMD_FUNCTION (var_bin_op);
SERVER_CMD_FUNCTION (group_query_list);
SERVER_CMD_FUNCTION (group_query);
SERVER_CMD_FUNCTION (group_read);
SERVER_CMD_FUNCTION (group_write);
SERVER_CMD_FUNCTION (group_bin_op);
SERVER_CMD_FUNCTION (group_create);
SERVER_CMD_FUNCTION (group_remove_all);
SERVER_CMD_FUNCTION (curve_query_list);
SERVER_CMD_FUNCTION (curve_query_csum);
SERVER_CMD_FUNCTION (curve_block_request);
SERVER_CMD_FUNCTION (curve_block);
SERVER_CMD_FUNCTION (curve_recalc_csum);
SERVER_CMD_FUNCTION (func_query_list);
SERVER_CMD_FUNCTION (func_execute);

#endif

