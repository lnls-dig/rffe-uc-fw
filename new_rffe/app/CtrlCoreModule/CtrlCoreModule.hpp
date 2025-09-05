/**
 *    @file     CtrlCoreModule.hpp
 *    @author   João Victor Silva e Santos (joao.ssantos@lnls.br)
 *
 *    @brief Header file declaring the CtrlCoreModule class for the RFFE system.
 *
 *    @details
 *      The CtrlCoreModule class is responsible for managing the core control logic
 *      of the Radio Frequency Front-End (RFFE) system. It integrates:
 *        - A SCPI interface module (`SCPIIntfModule`) for command parsing
 *        - An interface to the global RFFE context (`GlobalRFFE`)
 *        - A message callback mechanism for asynchronous control operations
 *
 *      The module runs as an independent task, using mbed OS facilities
 *      (priority, stack, and scheduling), and processes incoming control
 *      messages or SCPI commands.
 *
 *      This class acts as the central control unit, orchestrating communication
 *      between hardware modules and higher-level software interfaces.
 *
 *    @date 2025
 *
 *    @copyright
 *      Copyright (C) 2025 CNPEM (cnpem.br)
 *
 *    rffe-uc-fw is free software: you can redistribute it and/or modify
*     it under the terms of the GNU General Public License as published by
*     the Free Software Foundation, either version 3 of the License, or
*     (at your option) any later version.
*
*     rffe-uc-fw is distributed in the hope that it will be useful,
*     but WITHOUT ANY WARRANTY; without even the implied warranty of
*     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*     GNU General Public License for more details.
*
*     You should have received a copy of the GNU General Public License
*     along with RFFE.  If not, see <https://www.gnu.org/licenses/>.
*/

 #ifndef CTRLCOREMODULE_HPP_
 #define CTRLCOREMODULE_HPP_

 #include "mbed.h"

 #include "CtrlIntfModuleMessage.hpp"
 #include "SCPIIntfModule.hpp"
 #include "Module.hpp"
 #include "GlobalRFFE.hpp"
 #include "scpi_tables.hpp"



 class CtrlCoreModule :
   public Module {
     public:
       CtrlCoreModule(
           mbed::Callback<bool(Kernel::Clock::duration_u32,
             CtrlIntfModuleMessage**)> try_get_for_cb,
           /* Module params */
           osPriority priority, uint32_t stack_size, unsigned char *stack_mem,
           const char *name, void * ctx
         );
       ~CtrlCoreModule();

     private:
       mbed::Callback<bool(Kernel::Clock::duration_u32, CtrlIntfModuleMessage**)>
         _try_get_for_cb;
       SCPIIntfModule _scpi_mod;
       void _task();
   };

 #endif /* CTRLCOREMODULE_HPP_ */
