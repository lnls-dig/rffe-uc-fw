#ifndef BSMP_SERVER_H
#define BSMP_SERVER_H

#include "bsmp.h"

// Types

// Handle to a server instance
typedef struct bsmp_server bsmp_server_t;

// Hook function. Called before the values of a set of variables are read and
// after values of a set of variables are written.
enum bsmp_operation
{
    BSMP_OP_READ,                   // Read command arrived
    BSMP_OP_WRITE,                  // Write command arrived
};

typedef void (*bsmp_hook_t) (enum bsmp_operation op, struct bsmp_var **list);

// Structures

// Represent a packet that either was received or is to be sent
struct bsmp_raw_packet
{
    uint8_t *data;
    uint16_t len;
};

/**
 * Allocate a new server instance, returning a handle to it. This instance
 * should be deallocated with bsmp_destroy after its use. The instance returned
 * is already initialized. The instance is allocated with malloc().
 *
 * @return A handle to a server or NULL if there wasn't enough memory to do the
 *         allocation.
 */
bsmp_server_t *bsmp_server_new (void);

/**
 * Allocate a new server instance, returning a handle to it. This instance
 * should be deallocated with bsmp_destroy after its use. The instance returned
 * is already initialized. The instance is returned from a pool of instances
 * allocated on the heap.
 *
 * @return A handle to a server or NULL if there wasn't enough memory to do the
 *         allocation.
 */
bsmp_server_t *bsmp_server_new_from_pool (void);

/**
 * Deallocate a server instance
 * 
 * @param server [input] Handle to the instance to be deallocated.
 * 
 * @return BSMP_SUCCESS or one of the following errors:
 * <ul>
 *   <li>BSMP_ERR_PARAM_INVALID: server is a NULL pointer. </li>
 * </ul>
 */
enum bsmp_err bsmp_server_destroy (bsmp_server_t *server);

/**
 * Register a variable with a server instance. The memory pointed by the var
 * parameter must remain valid throughout the entire lifespan of the server
 * instance. The id field of the var parameter will be written by the BSMP lib.
 *
 * The fields writable, size and data of the var parameter must be filled
 * correctly.
 *
 * The user field is untouched.
 *
 * @param server [input] Handle to the instance.
 * @param var [input] Structure describing the variable to be registered.
 *
 * @return BSMP_SUCCESS or one of the following errors:
 * <ul>
 *   <li> BSMP_ERR_PARAM_INVALID: server or var->data is a NULL pointer. </li>
 *   <li> BSMP_PARAM_OUT_OF_RANGE: var->size is less than 1 or greater than 127.
 *   </li>
 * </ul>
 */
enum bsmp_err bsmp_register_variable (bsmp_server_t *server,
                                      struct bsmp_var *var);

/**
 * Register a curve with a BSMP instance. The memory pointed by the curve
 * parameter must remain valid throughout the entire lifespan of the server
 * instance. The id field of the curve parameter will be written by the BSMP
 * lib.
 *
 * The fields writable, nblocks and read_block must be filled correctly.
 * If writable is true, the field write_block must also be filled correctly.
 * Otherwise, write_block must be NULL.
 *
 * The user field is untouched.
 *
 * @param server [input] Handle to the server instance.
 * @param curve [input] Pointer to the curve to be registered.
 *
 * @return BSMP_SUCCESS or one of the following errors:
 * <ul>
 *   <li>BSMP_ERR_PARAM_INVALID: either server or curve is a NULL pointer.</li>
 *   <li>BSMP_ERR_PARAM_INVALID: curve->info.nblocks less than
 *                               BSMP_CURVE_MIN_BLOCKS or greater than
 *                               BSMP_CURVE_MAX_BLOCKS.</li>
 *   <li>BSMP_ERR_PARAM_INVALID: curve->read_block is NULL.</li>
 *   <li>BSMP_ERR_PARAM_INVALID: curve->writable is true and curve->write_block
 *                               is NULL.</li>
 *   <li>BSMP_ERR_PARAM_INVALID: curve->writable is false and curve->write_block
 *                               is not NULL.</li>
 * </ul>
 */
enum bsmp_err bsmp_register_curve (bsmp_server_t *server,
                                   struct bsmp_curve *curve);

/**
 * Register a function with a BSMP instance. The memory pointed by the func
 * parameter must remain valid throughout the entire lifespan of the server
 * instance. The info.id field of the func parameter will be written by the
 * BSMP lib.
 *
 * The fields func_p, info.input_size and info.output_size must be filled
 * correctly. info.input_size must be less than or equal to BSMP_FUNC_MAX_INPUT.
 * Likewise, info.output_size must be less than or equal to
 * BSMP_FUNC_MAS_OUTPUT.
 *
 * @param server [input] Handle to the server instance.
 * @param func [input] Pointer to the function to be registered.
 *
 * @return BSMP_SUCCESS or one of the following errors:
 * <ul>
 *   <li>BSMP_ERR_PARAM_INVALID: either server or func is a NULL pointer.</li>
 *   <li>BSMP_ERR_PARAM_INVALID: func->func_p is NULL.</li>
 *   <li>BSMP_ERR_PARAM_OUT_OF_RANGE: info.input_size is greater than
 *                                    BSMP_FUNC_MAX_INPUT.
 *   <li>BSMP_ERR_PARAM_OUT_OF_RANGE: info.output_size is greater than
 *                                    BSMP_FUNC_MAX_OUTPUT.
 * </ul>
 */
enum bsmp_err bsmp_register_function (bsmp_server_t *server,
                                      struct bsmp_func *func);

/**
 * Register a function that will be called in two moments:
 *
 * 1. When a command that reads one or more variables arrives, the hook function
 * is called right BEFORE reading the variables and putting the read values in
 * the response message.
 *
 * 2. When a command that writes to one or more variables arrives, the hook
 * function is called AFTER writing the values of the variables.
 *
 * Other commands don't call the hook function. It's not required to register a
 * hook function. It's possible to deregister a previously registered hook
 * function by simply passing a NULL pointer in the hook parameter.
 *
 * A hook function receives two parameters: the first one indicating the type
 * of operation being performed (specified in enum bsmp_operation) and the
 * second one containing the list of variables being affected by that operation.
 *
 * @param server [input] Handle to a BSMP instance.
 * @param hook [input] Hook function
 *
 * @return BSMP_SUCCESS or one of the following errors:
 * <ul>
 *   <li> BSMP_ERR_INVALID_PARAM: bsmp is a NULL pointer.
 * </ul>
 */
enum bsmp_err bsmp_register_hook (bsmp_server_t *server, bsmp_hook_t hook);

/**
 * Process a received message and prepare an answer.
 *
 * @param server [input] Handle to a server instance.
 * @param request [input] The message to be processed.
 * @param response [output] The answer to be sent
 *
 * @return BSMP_SUCCESS or one of the following errors:
 * <ul>
 *   <li> SSLP_ERR_PARAM_INVALID: Either server, request, or response is
 *                                a NULL pointer.</li>
 * </ul>
 */
enum bsmp_err bsmp_process_packet (bsmp_server_t *server,
                                   struct bsmp_raw_packet *request,
                                   struct bsmp_raw_packet *response);

#endif


