#include "server.h"
#include "server_priv.h"
#include "bsmp_priv.h"

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define SERVER_POOL_SIZE    1

struct
{
    struct bsmp_server list[SERVER_POOL_SIZE];
    unsigned int count;
    unsigned int allocated;
}server_pool = {
    .list = {},
    .count = 0,
    .allocated = 0,
};

static int server_pool_index (struct bsmp_server *server)
{
    int i;
    for(i = 0; i < SERVER_POOL_SIZE; ++i)
        if(server == server_pool.list + i)
            return i;
    return -1;
}

static void server_init (struct bsmp_server *server)
{
    memset(server, 0, sizeof(*server));

    group_init(&server->groups.list[GROUP_ALL_ID],   GROUP_ALL_ID);
    group_init(&server->groups.list[GROUP_READ_ID],  GROUP_READ_ID);
    group_init(&server->groups.list[GROUP_WRITE_ID], GROUP_WRITE_ID);

    server->groups.count = GROUP_STANDARD_COUNT;
}

bsmp_server_t *bsmp_server_new (void)
{
    struct bsmp_server *server = (struct bsmp_server*) malloc(sizeof(*server));

    if(!server)
        return NULL;

    server_init(server);

    return server;
}

bsmp_server_t *bsmp_server_new_from_pool (void)
{
    if(server_pool.count == SERVER_POOL_SIZE)
        return NULL;

    unsigned int i;
    for(i = 0; server_pool.allocated & (1 << i); ++i);

    ++server_pool.count;
    server_pool.allocated |= (1 << i);

    struct bsmp_server *server = &server_pool.list[i];

    server_init(server);

    return server;
}

enum bsmp_err bsmp_server_destroy (bsmp_server_t* server)
{
    if(!server)
        return BSMP_ERR_PARAM_INVALID;

    int pool_index = server_pool_index(server);

    if(pool_index < 0)
        free(server);
    else if(server_pool.allocated & (1 << pool_index))
    {
        --server_pool.count;
        server_pool.allocated ^= 1 << pool_index;
    }
    else
        return BSMP_ERR_PARAM_INVALID;

    return BSMP_SUCCESS;
}

#define SERVER_REGISTER(elem, max) \
    do {\
        if(!server) return BSMP_ERR_PARAM_INVALID;\
        enum bsmp_err err;\
        if((err = elem##_check(elem))) return err;\
        if(server->elem##s.count >= max) return BSMP_ERR_OUT_OF_MEMORY;\
        unsigned int i;\
        for(i = 0; i < server->elem##s.count; ++i)\
            if(server->elem##s.list[i] == elem)\
                return BSMP_ERR_DUPLICATE;\
        server->elem##s.list[server->elem##s.count] = elem;\
        elem->info.id = server->elem##s.count++;\
    }while(0)

enum bsmp_err bsmp_register_variable (bsmp_server_t *server,
                                      struct bsmp_var *var)
{
    SERVER_REGISTER(var, BSMP_MAX_VARIABLES);

    // Add to the group containing all variables
    group_add_var(&server->groups.list[GROUP_ALL_ID], var);

    // Add either to the WRITABLE or to the READ_ONLY group
    if(var->info.writable)
        group_add_var(&server->groups.list[GROUP_WRITE_ID], var);
    else
        group_add_var(&server->groups.list[GROUP_READ_ID], var);

    return BSMP_SUCCESS;
}

enum bsmp_err bsmp_register_curve (bsmp_server_t *server,
                                   struct bsmp_curve *curve)
{
    SERVER_REGISTER(curve, BSMP_MAX_CURVES);
    return BSMP_SUCCESS;
}

enum bsmp_err bsmp_register_function (bsmp_server_t *server,
                                      struct bsmp_func *func)
{
    SERVER_REGISTER(func, BSMP_MAX_FUNCTIONS);
    return BSMP_SUCCESS;
}

enum bsmp_err bsmp_register_hook(bsmp_server_t* server, bsmp_hook_t hook)
{
    if(!server || !hook)
        return BSMP_ERR_PARAM_INVALID;

    server->hook = hook;

    return BSMP_SUCCESS;
}

struct raw_message
{
    uint8_t command_code;
    uint8_t size[2];
    uint8_t payload[];
}__attribute__((packed));

static command_function_t command[256] =
{
    [CMD_QUERY_VERSION]         = query_version,

    // Variable's functions
    [CMD_VAR_QUERY_LIST]        = var_query_list,
    [CMD_VAR_READ]              = var_read,
    [CMD_VAR_WRITE]             = var_write,
    [CMD_VAR_BIN_OP]            = var_bin_op,
    [CMD_VAR_WRITE_READ]        = var_write_read,

    // Group's functions
    [CMD_GROUP_QUERY_LIST]      = group_query_list,
    [CMD_GROUP_QUERY]           = group_query,
    [CMD_GROUP_READ]            = group_read,
    [CMD_GROUP_WRITE]           = group_write,
    [CMD_GROUP_BIN_OP]          = group_bin_op,
    [CMD_GROUP_CREATE]          = group_create,
    [CMD_GROUP_REMOVE_ALL]      = group_remove_all,

    // Curve's functions
    [CMD_CURVE_QUERY_LIST]      = curve_query_list,
    [CMD_CURVE_QUERY_CSUM]      = curve_query_csum,
    [CMD_CURVE_BLOCK_REQUEST]   = curve_block_request,
    [CMD_CURVE_BLOCK]           = curve_block,
    [CMD_CURVE_RECALC_CSUM]     = curve_recalc_csum,

    // Function's functions
    [CMD_FUNC_QUERY_LIST]       = func_query_list,
    [CMD_FUNC_EXECUTE]          = func_execute
};

enum bsmp_err bsmp_process_packet (bsmp_server_t *server,
                                    struct bsmp_raw_packet *request,
                                    struct bsmp_raw_packet *response)
{
    if(!server || !request || !response)
        return BSMP_ERR_PARAM_INVALID;

    // Interpret packet payload as a message
    struct raw_message *recv_raw_msg = (struct raw_message *) request->data;
    struct raw_message *send_raw_msg = (struct raw_message *) response->data;

    // Create proper messages from the raw messages
    struct message recv_msg, send_msg;

    recv_msg.command_code = (enum command_code) recv_raw_msg->command_code;
    recv_msg.payload      = recv_raw_msg->payload;
    recv_msg.payload_size = (recv_raw_msg->size[0] << 8)+recv_raw_msg->size[1];

    send_msg.payload = send_raw_msg->payload;

    server->modified_list[0] = NULL;

    // Check inconsistency between the size of the received data and the size
    // specified in the message header
    if(request->len < BSMP_HEADER_SIZE ||
       request->len != recv_msg.payload_size + BSMP_HEADER_SIZE)
        MESSAGE_SET_ANSWER(&send_msg, CMD_ERR_MALFORMED_MESSAGE);
    // Check existence of the requested command
    else if(!command[recv_msg.command_code])
        MESSAGE_SET_ANSWER(&send_msg, CMD_ERR_OP_NOT_SUPPORTED);
    else
        command[recv_msg.command_code](server, &recv_msg, &send_msg);

    send_raw_msg->command_code = send_msg.command_code;

    send_raw_msg->size[0] = send_msg.payload_size >> 8;
    send_raw_msg->size[1] = send_msg.payload_size;

    response->len = send_msg.payload_size + BSMP_HEADER_SIZE;

    return BSMP_SUCCESS;
}

