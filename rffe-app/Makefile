############################################################################
# apps/examples/rffe/Makefile
#
#   Copyright (C) 2008, 2010-2013 Gregory Nutt. All rights reserved.
#   Author: Gregory Nutt <gnutt@nuttx.org>
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in
#    the documentation and/or other materials provided with the
#    distribution.
# 3. Neither the name NuttX nor the names of its contributors may be
#    used to endorse or promote products derived from this software
#    without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
# COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
# OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
# AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
# ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#
############################################################################

-include $(TOPDIR)/Make.defs

# Rffe, World! built-in application info

CONFIG_EXAMPLES_RFFE_PRIORITY ?= SCHED_PRIORITY_DEFAULT
CONFIG_EXAMPLES_RFFE_STACKSIZE ?= 2048

APPNAME = rffe

PRIORITY  = $(CONFIG_EXAMPLES_RFFE_PRIORITY)
STACKSIZE = $(CONFIG_EXAMPLES_RFFE_STACKSIZE)

# Rffe, World! Example

ASRCS =
CSRCS = cdce906.c netconfig.c scpi_server.c scpi_tables.c scpi_rffe_cmd.c scpi_interface.c config_file.c config_file_migrate.c rffe_console_cfg.c fw_update.c pid.c temp_control.c\
	$(addprefix ./libscpi/src/, \
	error.c fifo.c ieee488.c \
	minimal.c parser.c units.c utils.c \
	lexer.c expression.c \
	)
MAINSRC = rffe_main.c

CONFIG_EXAMPLES_RFFE_PROGNAME ?= rffe$(EXEEXT)
PROGNAME = $(CONFIG_EXAMPLES_RFFE_PROGNAME)

CXXFLAGS += -I ./libscpi/inc/
CFLAGS += -I ./libscpi/inc/

MODULE = CONFIG_EXAMPLES_RFFE

include $(APPDIR)/Application.mk
