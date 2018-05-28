#include "bsmp_priv.h"
#include "server_priv.h"
#include "server.h"
#include "md5/md5.h"

#include <stdlib.h>
#include <string.h>

/* Check entities */

enum bsmp_err var_check (struct bsmp_var *var)
{
    if(!var)
        return BSMP_ERR_PARAM_INVALID;

    // Check variable fields
    if(var->info.size > BSMP_VAR_MAX_SIZE)
        return BSMP_ERR_PARAM_OUT_OF_RANGE;

    if(!var->data)
        return BSMP_ERR_PARAM_INVALID;

    return BSMP_SUCCESS;
}

enum bsmp_err curve_check(struct bsmp_curve *curve)
{
    if(!curve)
        return BSMP_ERR_PARAM_INVALID;

    if(curve->info.nblocks > BSMP_CURVE_MAX_BLOCKS)
        return BSMP_ERR_PARAM_INVALID;

    if(curve->info.block_size > BSMP_CURVE_BLOCK_MAX_SIZE)
        return BSMP_ERR_PARAM_INVALID;

    if(!curve->read_block)
        return BSMP_ERR_PARAM_INVALID;

    if(curve->info.writable && !curve->write_block)
        return BSMP_ERR_PARAM_INVALID;

    return BSMP_SUCCESS;
}

enum bsmp_err func_check(struct bsmp_func *func)
{
    if(!func)
        return BSMP_ERR_PARAM_INVALID;

    if(!func->func_p)
        return BSMP_ERR_PARAM_INVALID;

    if(func->info.input_size > BSMP_FUNC_MAX_INPUT)
        return BSMP_ERR_PARAM_OUT_OF_RANGE;

    if(func->info.output_size > BSMP_FUNC_MAX_OUTPUT)
        return BSMP_ERR_PARAM_OUT_OF_RANGE;

    return BSMP_SUCCESS;
}

/* Helper Group functions */

void group_init (struct bsmp_group *grp, uint8_t id)
{
    memset(grp, 0, sizeof(*grp));
    grp->id       = id;
    grp->writable = true;
}

void group_add_var (struct bsmp_group *grp, struct bsmp_var *var)
{
    grp->vars.list[grp->vars.count++]  = &var->info;
    grp->size                         += var->info.size;
    grp->writable                     &= var->info.writable;
}

static void group_to_mod_list (bsmp_server_t *server, struct bsmp_group *grp)
{
    unsigned int i;
    for(i = 0; i < grp->vars.count; ++i)
        server->modified_list[i] = server->vars.list[grp->vars.list[i]->id];
    server->modified_list[i] = NULL;
}

/* Version */

SERVER_CMD_FUNCTION (query_version)
{
    // Check payload size
    if(recv_msg->payload_size != 0)
    {
        MESSAGE_SET_ANSWER(send_msg, CMD_ERR_INVALID_PAYLOAD_SIZE);
        return;
    }

    // Set answer's command_code and payload_size
    MESSAGE_SET_ANSWER(send_msg, CMD_VERSION);

    send_msg->payload_size = 3;
    send_msg->payload[0] = VERSION;
    send_msg->payload[1] = SUBVERSION;
    send_msg->payload[2] = REVISION;
}

/* Variables */

SERVER_CMD_FUNCTION (var_query_list)
{
    // Payload must be zero
    if(recv_msg->payload_size != 0)
        MESSAGE_SET_ANSWER_RET(send_msg, CMD_ERR_INVALID_PAYLOAD_SIZE);

    // Set answer's command_code and payload_size
    MESSAGE_SET_ANSWER(send_msg, CMD_VAR_LIST);

    // Variables are in order of their ID's
    struct bsmp_var *var;

    // Add each variable to the response
    unsigned int i;
    for(i = 0; i < server->vars.count; ++i)
    {
        var = server->vars.list[i];
        send_msg->payload[i]  = var->info.size & SIZE_MASK;
        send_msg->payload[i] |= var->info.writable ? WRITABLE : READ_ONLY;
    }
    send_msg->payload_size = server->vars.count;
}

SERVER_CMD_FUNCTION (var_read)
{
    // Payload must contain exactly one byte
    if(recv_msg->payload_size != 1)
        MESSAGE_SET_ANSWER_RET(send_msg, CMD_ERR_INVALID_PAYLOAD_SIZE);

    // Check ID
    uint8_t var_id = recv_msg->payload[0];

    if(var_id >= server->vars.count)
        MESSAGE_SET_ANSWER_RET(send_msg, CMD_ERR_INVALID_ID);

    // Get desired variable
    struct bsmp_var *var = server->vars.list[var_id];

    if(server->hook)
    {
        server->modified_list[0] = var;
        server->modified_list[1] = NULL;
        server->hook(BSMP_OP_READ, server->modified_list);
    }

    // Set answer
    MESSAGE_SET_ANSWER(send_msg, CMD_VAR_VALUE);
    send_msg->payload_size = var->info.size;
    memcpy(send_msg->payload, var->data, var->info.size);
}

SERVER_CMD_FUNCTION (var_write)
{
    // Payload must contain, at least, two bytes (ID + 1 byte of value)
    if(recv_msg->payload_size < 2)
        MESSAGE_SET_ANSWER_RET(send_msg, CMD_ERR_INVALID_PAYLOAD_SIZE);

    // Check ID
    uint8_t var_id = recv_msg->payload[0];

    if(var_id >= server->vars.count)
        MESSAGE_SET_ANSWER_RET(send_msg, CMD_ERR_INVALID_ID);

    // Get desired var
    struct bsmp_var *var = server->vars.list[var_id];

    // Payload must contain, exactly 1 byte (ID) plus the Variable's size
    if(recv_msg->payload_size != 1 + var->info.size)
        MESSAGE_SET_ANSWER_RET(send_msg, CMD_ERR_INVALID_PAYLOAD_SIZE);

    // Check write permission
    if(!var->info.writable)
        MESSAGE_SET_ANSWER_RET(send_msg, CMD_ERR_READ_ONLY);

    // Check payload value
    if(var->value_ok && !var->value_ok(var, recv_msg->payload + 1))
        MESSAGE_SET_ANSWER_RET(send_msg, CMD_ERR_INVALID_VALUE);

    // Everything is OK, perform operation
    memcpy(var->data, recv_msg->payload + 1, var->info.size);

    // Call hook
    if(server->hook)
    {
        server->modified_list[0] = var;
        server->modified_list[1] = NULL;
        server->hook(BSMP_OP_WRITE, server->modified_list);
    }

    // Set answer code
    MESSAGE_SET_ANSWER(send_msg, CMD_OK);
}

SERVER_CMD_FUNCTION (var_write_read)
{
    // Payload must contain, at least, 3 bytes (2 ID's and 1 byte of the value)
    if(recv_msg->payload_size < 3)
        MESSAGE_SET_ANSWER_RET(send_msg, CMD_ERR_INVALID_PAYLOAD_SIZE);

    // Check ID
    uint8_t var_wr_id = recv_msg->payload[0];
    uint8_t var_rd_id = recv_msg->payload[1];

    if(var_wr_id >= server->vars.count || var_rd_id >= server->vars.count)
        MESSAGE_SET_ANSWER_RET(send_msg, CMD_ERR_INVALID_ID);

    // Get desired vars
    struct bsmp_var *var_wr = server->vars.list[var_wr_id];
    struct bsmp_var *var_rd = server->vars.list[var_rd_id];

    // Check payload size
    if(recv_msg->payload_size != 2 + var_wr->info.size)
        MESSAGE_SET_ANSWER_RET(send_msg, CMD_ERR_INVALID_PAYLOAD_SIZE);

    // Check write permission
    if(!var_wr->info.writable)
        MESSAGE_SET_ANSWER_RET(send_msg, CMD_ERR_READ_ONLY);

    // Check payload value
    if(var_wr->value_ok && !var_wr->value_ok(var_wr, recv_msg->payload + 2))
        MESSAGE_SET_ANSWER_RET(send_msg, CMD_ERR_INVALID_VALUE);

    // Everything is OK, perform WRITE operation
    memcpy(var_wr->data, recv_msg->payload + 2, var_wr->info.size);

    // Call hooks
    if(server->hook)
    {
        // Write hook
        server->modified_list[0] = var_wr;
        server->modified_list[1] = NULL;
        server->hook(BSMP_OP_WRITE, server->modified_list);

        server->modified_list[0] = var_rd;
        server->hook(BSMP_OP_READ, server->modified_list);
    }

    // Now perform READ operation
    MESSAGE_SET_ANSWER(send_msg, CMD_VAR_VALUE);
    send_msg->payload_size = var_rd->info.size;
    memcpy(send_msg->payload, var_rd->data, var_rd->info.size);
}

SERVER_CMD_FUNCTION (var_bin_op)
{
    // Check if body has at least 3 bytes (ID + binary operation + 1 mask byte)
    if(recv_msg->payload_size < 3)
        MESSAGE_SET_ANSWER_RET(send_msg, CMD_ERR_INVALID_PAYLOAD_SIZE);

    // Check ID
    uint8_t var_id = recv_msg->payload[0];

    if(var_id >= server->vars.count)
        MESSAGE_SET_ANSWER_RET(send_msg, CMD_ERR_INVALID_ID);

    // Get desired var
    struct bsmp_var *var = server->vars.list[var_id];

    // Get operation
    unsigned char operation = recv_msg->payload[1];

    // Check operation
    if(!bin_op[operation])
        MESSAGE_SET_ANSWER_RET(send_msg, CMD_ERR_OP_NOT_SUPPORTED);

    // Check payload size
    if(recv_msg->payload_size != 2 + var->info.size)
        MESSAGE_SET_ANSWER_RET(send_msg, CMD_ERR_INVALID_PAYLOAD_SIZE);

    // Check write permission
    if(!var->info.writable)
        MESSAGE_SET_ANSWER_RET(send_msg, CMD_ERR_READ_ONLY);

    // Everything is OK, perform operation
    bin_op[operation](var->data, recv_msg->payload + 2, var->info.size);

    // Call hook
    if(server->hook)
    {
        server->modified_list[0] = var;
        server->modified_list[1] = NULL;
        server->hook(BSMP_OP_WRITE, server->modified_list);
    }

    // Set answer code
    MESSAGE_SET_ANSWER(send_msg, CMD_OK);
}

/* Groups */

SERVER_CMD_FUNCTION (group_query_list)
{
    // Payload size must be zero
    if(recv_msg->payload_size != 0)
        MESSAGE_SET_ANSWER_RET(send_msg, CMD_ERR_INVALID_PAYLOAD_SIZE);

    // Set answer's command_code and payload_size
    MESSAGE_SET_ANSWER(send_msg, CMD_GROUP_LIST);

    // Add each group to the response
    struct bsmp_group *grp;

    unsigned int i;
    for(i = 0; i < server->groups.count; ++i)
    {
        grp = &server->groups.list[i];
        send_msg->payload[i]  = grp->writable ? WRITABLE : READ_ONLY;
        send_msg->payload[i] += grp->vars.count;
    }
    send_msg->payload_size = server->groups.count;
}

SERVER_CMD_FUNCTION (group_query)
{
    // Payload size must be 1 (ID)
    if(recv_msg->payload_size != 1)
        MESSAGE_SET_ANSWER_RET(send_msg, CMD_ERR_INVALID_PAYLOAD_SIZE);

    // Set answer code
    MESSAGE_SET_ANSWER(send_msg, CMD_GROUP);

    // Check ID
    uint8_t group_id = recv_msg->payload[0];
    if(group_id >= server->groups.count)
        MESSAGE_SET_ANSWER_RET(send_msg, CMD_ERR_INVALID_ID);

    // Get desired group
    struct bsmp_group *grp = &server->groups.list[group_id];

    unsigned int i;
    for(i = 0; i < grp->vars.count; ++i)
        send_msg->payload[i] = grp->vars.list[i]->id;

    send_msg->payload_size = grp->vars.count;
}

SERVER_CMD_FUNCTION (group_read)
{
    // Payload size must be 1 (ID)
    if(recv_msg->payload_size != 1)
        MESSAGE_SET_ANSWER_RET(send_msg, CMD_ERR_INVALID_PAYLOAD_SIZE);

    // Check group ID
    uint8_t group_id = recv_msg->payload[0];

    if(group_id >= server->groups.count)
        MESSAGE_SET_ANSWER_RET(send_msg, CMD_ERR_INVALID_ID);

    // Get desired group
    struct bsmp_group *grp = &server->groups.list[group_id];

    // Call hook
    if(server->hook)
    {
        group_to_mod_list(server, grp);
        server->hook(BSMP_OP_READ, server->modified_list);
    }

    // Iterate over group's variables
    MESSAGE_SET_ANSWER(send_msg, CMD_GROUP_VALUES);

    struct bsmp_var *var;
    unsigned int i;
    uint8_t *payloadp = send_msg->payload;
    for(i = 0; i < grp->vars.count; ++i)
    {
        var = server->vars.list[grp->vars.list[i]->id];
        memcpy(payloadp, var->data, var->info.size);
        payloadp += var->info.size;
    }
    send_msg->payload_size = grp->size;
}

SERVER_CMD_FUNCTION (group_write)
{
    // Check if body has at least 2 bytes (ID + 1 byte of data)
    if(recv_msg->payload_size < 2)
        MESSAGE_SET_ANSWER_RET(send_msg, CMD_ERR_INVALID_PAYLOAD_SIZE);

    // Check ID
    uint8_t group_id = recv_msg->payload[0];

    if(group_id >= server->groups.count)
        MESSAGE_SET_ANSWER_RET(send_msg, CMD_ERR_INVALID_ID);

    // Get desired group
    struct bsmp_group *grp = &server->groups.list[group_id];

    // Check payload size
    if(recv_msg->payload_size != 1 + grp->size)
        MESSAGE_SET_ANSWER_RET(send_msg, CMD_ERR_INVALID_PAYLOAD_SIZE);

    // Check write permission
    if(!grp->writable)
        MESSAGE_SET_ANSWER_RET(send_msg, CMD_ERR_READ_ONLY);

    // Everything is OK, iterate
    struct bsmp_var *var;
    uint8_t *payloadp = recv_msg->payload + 1;
    unsigned int i;
    bool check_failed = false;

    for(i = 0; i < grp->vars.count; ++i)
    {
        var = server->vars.list[grp->vars.list[i]->id];

        // Check payload value
        if(var->value_ok && !var->value_ok(var, payloadp))
            check_failed = true;
        else
            memcpy(var->data, payloadp, var->info.size);
        payloadp += var->info.size;
    }

    // Call hook
    if(server->hook)
    {
        group_to_mod_list(server, grp);
        server->hook(BSMP_OP_WRITE, server->modified_list);
    }

    MESSAGE_SET_ANSWER(send_msg, check_failed ? CMD_ERR_INVALID_VALUE : CMD_OK);
}

SERVER_CMD_FUNCTION (group_bin_op)
{
    // Check if body has at least two bytes (ID + binary operation)
    if(recv_msg->payload_size < 2)
        MESSAGE_SET_ANSWER_RET(send_msg, CMD_ERR_INVALID_PAYLOAD_SIZE);

    // Check ID
    uint8_t group_id = recv_msg->payload[0];

    if(group_id >= server->groups.count)
        MESSAGE_SET_ANSWER_RET(send_msg, CMD_ERR_INVALID_ID);

    // Get desired group
    struct bsmp_group *grp = &server->groups.list[group_id];

    // Get operation
    unsigned char operation = recv_msg->payload[1];

    // Check operation
    if(!bin_op[operation])
        MESSAGE_SET_ANSWER_RET(send_msg, CMD_ERR_OP_NOT_SUPPORTED);

    // Check payload size
    if(recv_msg->payload_size != 2 + grp->size)
        MESSAGE_SET_ANSWER_RET(send_msg, CMD_ERR_INVALID_PAYLOAD_SIZE);

    // Check write permission
    if(!grp->writable)
        MESSAGE_SET_ANSWER_RET(send_msg, CMD_ERR_READ_ONLY);

    // Everything is OK, iterate
    struct bsmp_var *var;
    uint8_t *payloadp = recv_msg->payload + 2;

    unsigned int i;
    for(i = 0; i < grp->vars.count; ++i)
    {
        var = server->vars.list[grp->vars.list[i]->id];
        bin_op[operation](var->data, payloadp, var->info.size);
        payloadp += var->info.size;
    }

    // Call hook
    if(server->hook)
    {
        group_to_mod_list(server, grp);
        server->hook(BSMP_OP_WRITE, server->modified_list);
    }

    MESSAGE_SET_ANSWER(send_msg, CMD_OK);
}

SERVER_CMD_FUNCTION (group_create)
{
    // Check if there's at least one variable to put on the group
    if(recv_msg->payload_size < 1 ||
       recv_msg->payload_size > server->vars.count)
        MESSAGE_SET_ANSWER_RET(send_msg, CMD_ERR_INVALID_PAYLOAD_SIZE);

    // Check if there's available space for the new group
    if(server->groups.count == BSMP_MAX_GROUPS)
        MESSAGE_SET_ANSWER_RET(send_msg, CMD_ERR_INSUFFICIENT_MEMORY);

    struct bsmp_group *grp = &server->groups.list[server->groups.count];

    // Initialize group id
    group_init(grp, server->groups.count);

    // Populate group
    int i;
    for(i = 0; i < recv_msg->payload_size; ++i)
    {
        // Check var ID
        uint8_t var_id = recv_msg->payload[i];

        if(var_id >= server->vars.count)
            MESSAGE_SET_ANSWER_RET(send_msg, CMD_ERR_INVALID_ID);

        if(i && (var_id <= grp->vars.list[i-1]->id))
            MESSAGE_SET_ANSWER_RET(send_msg, CMD_ERR_INVALID_ID);

        // Add var by ID
        group_add_var(grp, server->vars.list[var_id]);
    }

    // Group created
    ++server->groups.count;

    // Prepare answer
    MESSAGE_SET_ANSWER(send_msg, CMD_OK);
}

SERVER_CMD_FUNCTION (group_remove_all)
{
    // Payload size must be zero
    if(recv_msg->payload_size != 0)
        MESSAGE_SET_ANSWER_RET(send_msg, CMD_ERR_INVALID_PAYLOAD_SIZE);

    server->groups.count = GROUP_STANDARD_COUNT;
    MESSAGE_SET_ANSWER(send_msg, CMD_OK);
}

/* Curves */

SERVER_CMD_FUNCTION (curve_query_list)
{
    // Payload size must be zero
    if(recv_msg->payload_size != 0)
        MESSAGE_SET_ANSWER_RET(send_msg, CMD_ERR_INVALID_PAYLOAD_SIZE);

    MESSAGE_SET_ANSWER(send_msg, CMD_CURVE_LIST);

    struct bsmp_curve_info *curve;
    unsigned int i;
    uint8_t *payloadp = send_msg->payload;

    for(i = 0; i < server->curves.count; ++i)
    {
        curve = &server->curves.list[i]->info;

        *(payloadp++) = curve->writable;
        *(payloadp++) = curve->block_size >> 8;
        *(payloadp++) = curve->block_size;
        *(payloadp++) = curve->nblocks >> 8;
        *(payloadp++) = curve->nblocks;
    }

    send_msg->payload_size = server->curves.count*BSMP_CURVE_LIST_INFO;
}

SERVER_CMD_FUNCTION (curve_query_csum)
{
    // Check payload size
    if(recv_msg->payload_size != 1)
        MESSAGE_SET_ANSWER_RET(send_msg, CMD_ERR_INVALID_PAYLOAD_SIZE);

    uint8_t curve_id = recv_msg->payload[0];

    if(curve_id >= server->curves.count)
        MESSAGE_SET_ANSWER_RET(send_msg, CMD_ERR_INVALID_ID);

    struct bsmp_curve *curve = server->curves.list[curve_id];

    MESSAGE_SET_ANSWER(send_msg, CMD_CURVE_CSUM);
    memcpy(send_msg->payload, curve->info.checksum, BSMP_CURVE_CSUM_SIZE);
    send_msg->payload_size = BSMP_CURVE_CSUM_SIZE;
}

SERVER_CMD_FUNCTION (curve_block_request)
{
    // Payload size must be equal to BSMP_CURVE_BLOCK_INFO
    if(recv_msg->payload_size != BSMP_CURVE_BLOCK_INFO)
        MESSAGE_SET_ANSWER_RET(send_msg, CMD_ERR_INVALID_PAYLOAD_SIZE);

    // Check curve ID
    uint8_t curve_id = recv_msg->payload[0];

    if(curve_id >= server->curves.count)
        MESSAGE_SET_ANSWER_RET(send_msg, CMD_ERR_INVALID_ID);

    // Get curve
    struct bsmp_curve *curve = server->curves.list[curve_id];

    uint16_t block_offset = (recv_msg->payload[1] << 8) + recv_msg->payload[2];

    if(block_offset >= curve->info.nblocks)
        MESSAGE_SET_ANSWER_RET(send_msg, CMD_ERR_INVALID_VALUE);

    MESSAGE_SET_ANSWER(send_msg, CMD_CURVE_BLOCK);
    send_msg->payload[0] = recv_msg->payload[0];    // Curve ID
    send_msg->payload[1] = recv_msg->payload[1];    // Offset (most sig.)
    send_msg->payload[2] = recv_msg->payload[2];    // Offset (less sig.)

    curve->read_block(curve, block_offset,
                      send_msg->payload + BSMP_CURVE_BLOCK_INFO,
                      &send_msg->payload_size);

    send_msg->payload_size += BSMP_CURVE_BLOCK_INFO;
}

SERVER_CMD_FUNCTION (curve_block)
{
    // Payload must contain, at least, 4 bytes (1 for ID, 2 for offset, 1 for
    // data)
    if(recv_msg->payload_size < BSMP_CURVE_BLOCK_INFO + 1)
        MESSAGE_SET_ANSWER_RET(send_msg, CMD_ERR_INVALID_PAYLOAD_SIZE);

    // Check curve ID
    uint8_t curve_id = recv_msg->payload[0];

    if(curve_id >= server->curves.count)
        MESSAGE_SET_ANSWER_RET(send_msg, CMD_ERR_INVALID_ID);

    // Get curve
    struct bsmp_curve *curve = server->curves.list[curve_id];

    // Check block size
    if(recv_msg->payload_size > curve->info.block_size + BSMP_CURVE_BLOCK_INFO)
        MESSAGE_SET_ANSWER_RET(send_msg, CMD_ERR_INVALID_PAYLOAD_SIZE);

    // Check offset
    uint16_t block_offset = (recv_msg->payload[1] << 8) + recv_msg->payload[2];
    if(block_offset >= curve->info.nblocks)
        MESSAGE_SET_ANSWER_RET(send_msg, CMD_ERR_INVALID_VALUE);

    // Everything ok, write block
    curve->write_block(curve, block_offset,
                       recv_msg->payload + BSMP_CURVE_BLOCK_INFO,
                       recv_msg->payload_size - BSMP_CURVE_BLOCK_INFO);

    memset(curve->info.checksum, 0, sizeof(curve->info.checksum));
    MESSAGE_SET_ANSWER(send_msg, CMD_OK);
}

SERVER_CMD_FUNCTION (curve_recalc_csum)
{
    // Payload must contain Curve ID only
    if(recv_msg->payload_size != 1)
        MESSAGE_SET_ANSWER_RET(send_msg, CMD_ERR_INVALID_PAYLOAD_SIZE);

    // Check curve ID
    uint8_t curve_id = recv_msg->payload[0];

    if(curve_id >= server->curves.count)
        MESSAGE_SET_ANSWER_RET(send_msg, CMD_ERR_INVALID_ID);

    // Get curve
    struct bsmp_curve *curve = server->curves.list[curve_id];

    // Calculate checksum (this might take a while)
    uint8_t block[curve->info.block_size];
    MD5_CTX md5ctx;

    MD5Init(&md5ctx);

    unsigned int i;
    for(i = 0; i < curve->info.nblocks; ++i)
    {
        uint16_t read_bytes = 0;
        curve->read_block(curve, (uint16_t)i, block, &read_bytes);
        MD5Update(&md5ctx, block, read_bytes);
    }
    MD5Final(curve->info.checksum, &md5ctx);

    MESSAGE_SET_ANSWER(send_msg, CMD_CURVE_CSUM);
    memcpy(send_msg->payload, curve->info.checksum, BSMP_CURVE_CSUM_SIZE);
    send_msg->payload_size = BSMP_CURVE_CSUM_SIZE;
}

/* Functions */

SERVER_CMD_FUNCTION (func_query_list)
{
    // Check payload size
    if(recv_msg->payload_size != 0)
        MESSAGE_SET_ANSWER_RET(send_msg, CMD_ERR_INVALID_PAYLOAD_SIZE);

    MESSAGE_SET_ANSWER(send_msg, CMD_FUNC_LIST);

    struct bsmp_func_info *func_info;
    unsigned int i;
    for(i = 0; i < server->funcs.count; ++i)
    {
        func_info = &server->funcs.list[i]->info;

        send_msg->payload[i] = (func_info->input_size << 4) |
                               (func_info->output_size & 0x0F);
    }

    send_msg->payload_size = server->funcs.count;
}

SERVER_CMD_FUNCTION (func_execute)
{
    if(!recv_msg->payload_size)
        MESSAGE_SET_ANSWER_RET(send_msg, CMD_ERR_INVALID_PAYLOAD_SIZE);

    uint8_t func_id = recv_msg->payload[0];

    if(func_id >= server->funcs.count)
        MESSAGE_SET_ANSWER_RET(send_msg, CMD_ERR_INVALID_ID);

    struct bsmp_func *func = server->funcs.list[func_id];

    if(recv_msg->payload_size != 1 + func->info.input_size)
        MESSAGE_SET_ANSWER_RET(send_msg, CMD_ERR_INVALID_PAYLOAD_SIZE);

    uint8_t ret;

    ret = func->func_p(&recv_msg->payload[1], &send_msg->payload[0]);

    if(ret)
    {
        MESSAGE_SET_ANSWER(send_msg, CMD_FUNC_ERROR);
        send_msg->payload[0] = ret;
        send_msg->payload_size = 1;
    }
    else
    {
        MESSAGE_SET_ANSWER(send_msg, CMD_FUNC_RETURN);
        send_msg->payload_size = func->info.output_size;
    }
}


