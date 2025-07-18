#ifndef SCPI_TABLES_H_
#define SCPI_TABLES_H_

#include "scpi_cmd.hpp"

extern "C" {
    #include "scpi/scpi.h"
}

extern const scpi_command_t scpi_commands[];

#endif
