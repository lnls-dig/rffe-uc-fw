#include "rtos.h"
#include "pcbnAPI.h"

//int build_variable(int ID, int type, int size, int value);

// Mutex to access shared resources (write/read from BSMP variables)
Mutex bsmp_var_mutex;

unsigned char calculate_size(unsigned char size){
    if (size || 128) {
        return 128*(size+1)+2;
    }
    return size;
}

void status_command_ask(command_header *status){
    status->command = STATUS_CODE_ASK;
    status->size = 0x00;
    return;
}

void status_command_answer(command_header *status){
    status->command = STATUS_CODE_ANSWER;
    status->size = 0x00;
    return;
}

void var_list_command_ask(command_header *varlist){
    varlist->command = VAR_LIST_CODE_ASK;
    varlist->size = 0x00;
    return;
}

void var_list_command_answer(command_header *varlist,int size){
    varlist->command = VAR_LIST_CODE_ANSWER;
    varlist->size = size;
    return;
}

void var_read_command_ask(command_header *varread){
    varread->command = VAR_READ_CODE_ASK;
    varread->size = 0x01;
    return;
}

void var_read_command_answer(command_header *varread, int size){
    varread->command = VAR_READ_CODE_ANSWER;
    varread->size = size;
    return;
}

void var_write_command_ask(command_header *varwrite,int size){
    varwrite->command = VAR_WRITE_CODE_ASK;
    varwrite->size = 1 + size;
    return;
}

void ping_command(command_header *ping, int size){
    ping->command = PING_CODE;
    ping->size = 8 + size;
    return;
}

void error_commands(command_header *error,int code){
    error->command = code;
    error->size = 0x0;
    return;
}

void ok_command(command_header *error){
    error_commands(error,OK_CODE);
    return;
}
void bad_message_command(command_header *error){
    error_commands(error,BAD_MESSAGE_CODE);
    return;
}

void op_not_supported_command(command_header *error){
    error_commands(error,OP_NOT_SUPPORTED_CODE);
    return;
}

void invalid_id_command(command_header *error){
    error_commands(error,INVALID_ID_CODE);
    return;
}

void invalid_value_command(command_header *error){
    error_commands(error,INVALID_VALUE_CODE);
    return;
}

void invalid_payload_command(command_header *error){
    error_commands(error,INVALID_PAYLOAD_CODE);
    return;
}

void read_only_command(command_header *error){
    error_commands(error,READ_ONLY_COMMAND);
    return;
}

void no_memory_command(command_header *error){
    error_commands(error,NO_MEMORY_CODE);
    return;
}

void internal_error_command(command_header *error){
    error_commands(error,INTERNAL_ERROR_CODE);
    return;
}

void message(command_header *recv, command_header *send, var_list * Vars){
    Charge aux;
    switch (recv->command)
    {
    case STATUS_CODE_ASK:
        if (recv->size != 0) {
            bad_message_command(send);
            break;
        }
        status_command_answer(send);
        break;

    case VAR_LIST_CODE_ASK:
        if (recv->size != 0) {
            bad_message_command(send);
            break;
        }
        var_list_command_answer(send,Vars->count);
	bsmp_var_mutex.lock();
        for (uint16_t n = 0; n < Vars->count; n++) {
            send->charge[n] = Vars->var[n]->size + 0x80*(Vars->var[n]->type);
        }
	bsmp_var_mutex.unlock();
        break;

    case VAR_READ_CODE_ASK:
        if (recv->size != 1) {
            invalid_payload_command(send);
            break;
        }

        if (recv->charge[0] >= Vars->count) {
            invalid_id_command(send);
            break;
        }

	bsmp_var_mutex.lock();
        var_read_command_answer(send,Vars->var[recv->charge[0]]->size);

        memcpy(aux.charge1,&(Vars->var[recv->charge[0]]->value),Vars->var[recv->charge[0]]->size);
#if 0
        printf("\nVariable ID: %d",recv->charge[0]);
        if (Vars->var[recv->charge[0]]->size == 1)
            printf("\nValue: %2X\n",aux.charge1[0]);
        if (Vars->var[recv->charge[0]]->size == 2)
            printf("\nValue: %d\n",aux.charge2[0]);
        if (Vars->var[recv->charge[0]]->size == 4)
            printf("\nValue: %f\n",aux.charge3[0]);
#endif
        memcpy(send->charge,&(Vars->var[recv->charge[0]]->value),Vars->var[recv->charge[0]]->size);
	bsmp_var_mutex.unlock();
        break;

    case VAR_WRITE_CODE_ASK:
        if (recv->charge[0] >= Vars->count) {
            invalid_id_command(send);
            break;
        }

        if (recv->size != (Vars->var[recv->charge[0]]->size + 1)) {
            invalid_payload_command(send);
            break;
        }

        if (Vars->var[recv->charge[0]]->type == 0) {
            read_only_command(send);
            break;
        }

	bsmp_var_mutex.lock();
        memcpy(&(Vars->var[recv->charge[0]]->value),&recv->charge[1],Vars->var[recv->charge[0]]->size);
        ok_command(send);
	bsmp_var_mutex.unlock();
        break;

    default:
        op_not_supported_command(send);
    }
    /*
      free(aux.charge1);
      free(aux.charge2);
      free(aux.charge3);
    */
    return;
}

void set_value(double *var, double value){
    Charge aux;

    bsmp_var_mutex.lock();
    aux.charge3[0] = value;
    memcpy(var,&aux, sizeof(double));
    bsmp_var_mutex.unlock();
}


void set_value(char *var, char const* value, size_t sz){
    bsmp_var_mutex.lock();
    strncpy(var,value, sz);
    bsmp_var_mutex.unlock();
}

void set_value(int *var, int value){
    Charge aux;

    bsmp_var_mutex.lock();
    aux.charge2[0] = value;
    memcpy(var,&aux, sizeof(int));
    bsmp_var_mutex.unlock();
}

void set_value(uint8_t *var, uint8_t value){
    Charge aux;

    bsmp_var_mutex.lock();
    aux.charge1[0] = value;
    memcpy(var,&aux, sizeof(uint8_t));
    bsmp_var_mutex.unlock();
}

unsigned char get_value8(void *var){
    Charge aux;
    bsmp_var_mutex.lock();
    memcpy(&aux,var, sizeof(unsigned char));
    bsmp_var_mutex.unlock();
    return aux.charge1[0];
}

int get_value32(void *var){
    Charge aux;

    bsmp_var_mutex.lock();
    memcpy(&aux,var, sizeof(int));
    bsmp_var_mutex.unlock();

    return aux.charge2[0];
}

double get_value64(void *var){
    Charge aux;

    bsmp_var_mutex.lock();
    memcpy(&aux,var, sizeof(double));
    bsmp_var_mutex.unlock();

    return aux.charge3[0];
}
