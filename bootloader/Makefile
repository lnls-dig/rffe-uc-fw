PRJ_NAME   = bootloader
CC         = arm-none-eabi-gcc
SRCDIR     = src
SRC        = $(wildcard $(SRCDIR)/*.c)
ASRC       = $(wildcard $(SRCDIR)/*.s)
OBJ        = $(SRC:.c=.o) $(ASRC:.s=.o)
OBJCOPY    = arm-none-eabi-objcopy
OBJDUMP    = arm-none-eabi-objdump
PROGRAMMER = openocd
PGFLAGS    = -f ../scripts/openocd/lpc17-cmsis.cfg -c "program $(PRJ_NAME).bin verify reset" -c shutdown
OPT       ?= -Og
LDSCRIPT   = ld.script
CFLAGS     = -fdata-sections -ffunction-sections -g3 -Wall -mcpu=cortex-m3 -mlittle-endian -mthumb $(OPT) -I inc/ -mlong-calls
ASFLAGS    =  $(CFLAGS)
LDFLAGS    = -T $(LDSCRIPT) -Wl,--gc-sections --specs=nano.specs --specs=nosys.specs

.PHONY: all clean flash

all: $(PRJ_NAME).bin

$(PRJ_NAME).elf: $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $@ $(LDFLAGS)
	arm-none-eabi-size $(PRJ_NAME).elf

%.o: %.c $(DEPS)
	$(CC) -MMD -c $(CFLAGS) $< -o $@

%.o: %.s $(DEPS)
	$(CC) -MMD -c $(ASFLAGS) $< -o $@

-include $(SRCDIR)/*.d

clean:
	rm -f $(OBJ) $(PRJ_NAME).elf $(PRJ_NAME)-pad.elf $(PRJ_NAME).hex $(PRJ_NAME).bin $(SRCDIR)/*.d

flash: $(PRJ_NAME).bin
	$(PROGRAMMER) $(PGFLAGS)

$(PRJ_NAME)-pad.elf: $(PRJ_NAME).elf
	$(OBJCOPY) --gap-fill 0xFF --pad-to 0x10000 $< $@

$(PRJ_NAME).bin: $(PRJ_NAME)-pad.elf
	$(OBJCOPY) -O binary $< $@
