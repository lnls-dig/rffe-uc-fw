#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "mbed.h"


#define MAX_PAYLOAD_COMM 256
#define MAX_VALUE_VAR 256
#define STATUS_CODE_ASK 0x00
#define STATUS_CODE_ANSWER 0x01
#define VAR_LIST_CODE_ASK 0x02
#define VAR_LIST_CODE_ANSWER 0x03
#define VAR_READ_CODE_ASK 0x10
#define VAR_READ_CODE_ANSWER 0x11
#define VAR_WRITE_CODE_ASK 0x20
#define PING_CODE 0xD6 
#define OK_CODE 0xE0
#define BAD_MESSAGE_CODE 0xE1
#define OP_NOT_SUPPORTED_CODE 0xE2
#define INVALID_ID_CODE 0xE3
#define INVALID_VALUE_CODE 0xE4
#define INVALID_PAYLOAD_CODE 0xE5
#define READ_ONLY_COMMAND 0xE6
#define NO_MEMORY_CODE 0xE7
#define INTERNAL_ERROR_CODE 0xE8

typedef struct vheader{
    unsigned char id;
    unsigned char type; // write = 1; read = 0
    unsigned char size;
    unsigned char value[MAX_VALUE_VAR];
}variable_header;

typedef struct cheader{
    unsigned char command;
    unsigned char size;
    unsigned char charge[MAX_PAYLOAD_COMM];
}command_header;

typedef struct vlist{
    variable_header ** var;
    unsigned int count;
}var_list;

typedef union ChargeType {
    uint8_t charge1[MAX_VALUE_VAR];
    int charge2[MAX_VALUE_VAR/2];
    double charge3[MAX_VALUE_VAR/4];
} Charge;

unsigned char calculate_size(unsigned char size);
void status_command_ask(command_header *status);
void status_command_answer(command_header *status);
void var_list_command_ask(command_header *varlist);
void var_list_command_answer(command_header *varlist,int size);
void var_read_command_ask(command_header *varread);
void var_read_command_answer(command_header *varread, int size);
void var_write_command_ask(command_header *varwrite,int size);
void ping_command(command_header *ping, int size);
void error_commands(command_header *error,int code);
void ok_command(command_header *error);
void bad_message_command(command_header *error);
void op_not_supported_command(command_header *error);
void invalid_id_command(command_header *error);
void invalid_value_command(command_header *error);
void invalid_payload_command(command_header *error);
void read_only_command(command_header *error);
void no_memory_command(command_header *error);
void internal_error_command(command_header *error);

void message(command_header *recv, command_header *send, var_list * Vars);

void set_value(int *var, int value);
void set_value(double *var, double value);
void set_value(char *var, char const* value, size_t sz);
void set_value(uint8_t *var, uint8_t value);
unsigned char get_value8(void *var);
int get_value32(void *var);
double get_value64(void *var);
