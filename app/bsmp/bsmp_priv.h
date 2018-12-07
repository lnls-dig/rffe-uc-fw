/* Private definitions for server and client */

#ifndef BSMP_PRIV_H
#define BSMP_PRIV_H

#include <stdbool.h>
#include <stdint.h>

#include "bsmp.h"

#define WRITABLE_MASK       0x80
#define SIZE_MASK           0x7F

#define WRITABLE            0x80
#define READ_ONLY           0x00

enum command_code
{
    // Query commands
    CMD_QUERY_VERSION       = 0x00,
    CMD_VERSION,
    CMD_VAR_QUERY_LIST,
    CMD_VAR_LIST,
    CMD_GROUP_QUERY_LIST,
    CMD_GROUP_LIST,
    CMD_GROUP_QUERY,
    CMD_GROUP,
    CMD_CURVE_QUERY_LIST,
    CMD_CURVE_LIST,
    CMD_CURVE_QUERY_CSUM,
    CMD_CURVE_CSUM,
    CMD_FUNC_QUERY_LIST,
    CMD_FUNC_LIST,

    // Read commands
    CMD_VAR_READ            = 0x10,
    CMD_VAR_VALUE,
    CMD_GROUP_READ,
    CMD_GROUP_VALUES,

    // Write commands
    CMD_VAR_WRITE           = 0x20,
    CMD_GROUP_WRITE         = 0x22,
    CMD_VAR_BIN_OP          = 0x24,
    CMD_GROUP_BIN_OP        = 0x26,
    CMD_VAR_WRITE_READ      = 0x28,

    // Group manipulation commands
    CMD_GROUP_CREATE        = 0x30,
    CMD_GROUP_REMOVE_ALL    = 0x32,

    // Curve commands
    CMD_CURVE_BLOCK_REQUEST = 0x40,
    CMD_CURVE_BLOCK,
    CMD_CURVE_RECALC_CSUM,

    // Function commands
    CMD_FUNC_EXECUTE        = 0x50,
    CMD_FUNC_RETURN,
    CMD_FUNC_ERROR          = 0x53,

    // Error codes
    CMD_OK                  = 0xE0,
    CMD_ERR_MALFORMED_MESSAGE,
    CMD_ERR_OP_NOT_SUPPORTED,
    CMD_ERR_INVALID_ID,
    CMD_ERR_INVALID_VALUE,
    CMD_ERR_INVALID_PAYLOAD_SIZE,
    CMD_ERR_READ_ONLY,
    CMD_ERR_INSUFFICIENT_MEMORY,

    CMD_MAX
};

#endif  /* COMMON_H */


