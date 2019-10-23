/* Public definitions for both server and client */

#ifndef BSMP_H
#define BSMP_H

#include <stdint.h>
#include <stdbool.h>

/* Library-wide limits */

#define BSMP_HEADER_SIZE            3       // Command code + 2 bytes for size
#define BSMP_MAX_PAYLOAD            65535
#define BSMP_MAX_MESSAGE            (BSMP_HEADER_SIZE+BSMP_MAX_PAYLOAD)

#define BSMP_MAX_VARIABLES          30
#define BSMP_MAX_GROUPS             8
#define BSMP_MAX_CURVES             128
#define BSMP_MAX_FUNCTIONS          128

/* Version info */

#define BSMP_VERSION_STR_MAX_LEN    20
struct bsmp_version
{
    uint8_t major;
    uint8_t minor;
    uint8_t revision;
    char    str[BSMP_VERSION_STR_MAX_LEN];
};

/* Standard group ID */

enum group_id
{
    GROUP_ALL_ID,
    GROUP_READ_ID,
    GROUP_WRITE_ID,

    GROUP_STANDARD_COUNT,
};

/* Binary operations */

enum bsmp_bin_op
{
    BIN_OP_AND,
    BIN_OP_OR,
    BIN_OP_XOR,
    BIN_OP_SET,
    BIN_OP_CLEAR,
    BIN_OP_TOGGLE,

    BIN_OP_COUNT,   // Number of binary operations
};

typedef void (*bin_op_function) (uint8_t *data, uint8_t *mask, uint8_t size);
extern bin_op_function bin_op[];

/* Error codes */

enum bsmp_err
{
    BSMP_SUCCESS,                   // Operation executed successfully
    BSMP_ERR_PARAM_INVALID,         // An invalid parameter was passed
    BSMP_ERR_PARAM_OUT_OF_RANGE,    // A param not in the acceptable range was
                                    // passed
    BSMP_ERR_OUT_OF_MEMORY,         // Not enough memory to complete operation
    BSMP_ERR_DUPLICATE,             // Trying to register an already registered
                                    // object
    BSMP_ERR_COMM,                  // There was a communication error reported
                                    // by one of the communication functions.
    BSMP_ERR_NOT_INITIALIZED,       // Instance wasn't initialized
    BSMP_ERR_MAX
};

/**** Entities ****/

/* Limits */

#define BSMP_VAR_MAX_SIZE           128

#define BSMP_CURVE_MAX_BLOCKS       65536
#define BSMP_CURVE_BLOCK_MAX_SIZE   65520
#define BSMP_CURVE_LIST_INFO        5
#define BSMP_CURVE_BLOCK_INFO       3
#define BSMP_CURVE_CSUM_SIZE        16

#define BSMP_FUNC_MAX_INPUT         15
#define BSMP_FUNC_MAX_OUTPUT        15

/**** Structures and lists ****/

/* Variable */

struct bsmp_var_info
{
    uint8_t id;                 // ID of the variable, used in the protocol.
    bool    writable;           // Determine if the variable is writable.
    uint8_t size;               // Indicates how many bytes 'data' contains.
};

struct bsmp_var
{
    struct bsmp_var_info info;  // Information about the variable identification
    bool                 (*value_ok) (struct bsmp_var *, uint8_t *);  // Checker
    void                 *data; // Pointer to the value of the variable.
    void                 *user; // The user can make use of this pointer at
                                // will. It is not touched by BSMP.
};

struct bsmp_var_info_list
{
    uint32_t count;
    struct bsmp_var_info list[BSMP_MAX_VARIABLES];
};

struct bsmp_var_info_ptr_list
{
    uint32_t count;
    struct bsmp_var_info *list[BSMP_MAX_VARIABLES];
};

struct bsmp_var_ptr_list
{
    uint32_t count;
    struct bsmp_var *list[BSMP_MAX_VARIABLES];
};

/* Group */

struct bsmp_group
{
    uint8_t id;           // ID of the group
    bool    writable;     // Whether all variables in the group are writable
    uint16_t size;        // Sum of the sizes of all variables in the group

    // List of pointers to the variables of this group
    struct bsmp_var_info_ptr_list vars;
};

struct bsmp_group_list
{
    uint32_t count;
    struct bsmp_group list[BSMP_MAX_GROUPS];
};

/* Curve */

struct bsmp_curve_info
{
    uint8_t  id;                    // ID of the curve, used in the protocol.
    bool     writable;              // Determine if the curve is writable.
    uint32_t nblocks;               // How many blocks the curve contains.
    uint16_t block_size;            // Maximum number of bytes in a block
    uint8_t  checksum[16];          // MD5 checksum of the curve
};

struct bsmp_curve;

typedef void (*bsmp_curve_read_t)  (struct bsmp_curve *curve, uint16_t block,
                                    uint8_t *data, uint16_t *len);
typedef void (*bsmp_curve_write_t) (struct bsmp_curve *curve, uint16_t block,
                                    uint8_t *data, uint16_t len);
struct bsmp_curve
{
    // Info about the curve identification
    struct bsmp_curve_info info;

    // Functions to read/write a block
    void (*read_block)(struct bsmp_curve *curve, uint16_t block, uint8_t *data,
                       uint16_t *len);

    void (*write_block)(struct bsmp_curve *curve, uint16_t block, uint8_t *data,
                        uint16_t len);

    // The user can make use of this variable as he wishes. It is not touched by
    // BSMP
    void *user;
};

struct bsmp_curve_info_list
{
    uint32_t count;
    struct bsmp_curve_info list[BSMP_MAX_CURVES];
};

struct bsmp_curve_ptr_list
{
    uint32_t count;
    struct bsmp_curve *list[BSMP_MAX_CURVES];
};

/* Function */

struct bsmp_func_info
{
    uint8_t id;                     // ID of the function, used in the protocol
    uint8_t input_size;             // How many bytes of input
    uint8_t output_size;            // How many bytes of output
};

typedef uint8_t (*bsmp_func_t) (uint8_t *input, uint8_t *output);
struct bsmp_func
{
    struct bsmp_func_info info;     // Information about the function
    bsmp_func_t           func_p;   // Pointer to the function to be executed
};


struct bsmp_func_info_list
{
    uint32_t count;
    struct bsmp_func_info list[BSMP_MAX_FUNCTIONS];
};

struct bsmp_func_ptr_list
{
    uint32_t count;
    struct bsmp_func *list[BSMP_MAX_FUNCTIONS];
};

/**
 * Returns an error string associated with the given error code
 *
 * @param error [input] The error code
 *
 * @return A string that describes the error
 */
char * bsmp_error_str (enum bsmp_err error);

#endif

