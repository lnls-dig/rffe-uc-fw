#include "bsmp.h"

static char* error_str[BSMP_ERR_MAX] =
{
    [BSMP_SUCCESS]                  = "Success",
    [BSMP_ERR_PARAM_INVALID]        = "An invalid parameter was passed",
    [BSMP_ERR_PARAM_OUT_OF_RANGE]   = "A parameter was out of the acceptable"
                                      " range",
    [BSMP_ERR_OUT_OF_MEMORY]        = "Not enough memory to complete request",
    [BSMP_ERR_DUPLICATE]            = "Entity already registered",
    [BSMP_ERR_COMM]                 = "Sending or receiving a message failed",
    [BSMP_ERR_NOT_INITIALIZED]      = "Instance not initialized",
};

#define BINOPS_FUNC(name, operation)\
    void binops_##name (uint8_t *data, uint8_t *mask, uint8_t size) {\
        while(size--)\
            data[size] operation mask[size];\
    }

BINOPS_FUNC(and, &=)
BINOPS_FUNC(or, |=)
BINOPS_FUNC(xor, ^=)
BINOPS_FUNC(clear, &=~)

bin_op_function bin_op[] =
{
    ['A'] = binops_and,    // AND
    ['X'] = binops_xor,    // XOR
    ['O'] = binops_or,     // OR
    ['C'] = binops_clear,  // CLEAR BITS
    ['S'] = binops_or,     // SET BITS
    ['T'] = binops_xor     // TOGGLE BITS
};

char *bsmp_error_str (enum bsmp_err error)
{
    return error_str[error];
}

