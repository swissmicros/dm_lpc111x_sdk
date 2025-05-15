##########################################################################
# User configuration and firmware specific object files	
##########################################################################

# The target, flash and ram of the LPC1xxx microprocessor.
# Use for the target the value: LPC11xx, LPC13xx or LPC17xx
TARGET = LPC11xx
FLASH = 64K
SRAM = 8K

# For USB support the LPC1xxx reserves 384 bytes from the sram,
# if you don't want to use the USB features, just use 0 here.
SRAM_USB = 0

VPATH = 
OBJS = main.o

##########################################################################
# Project-specific files 
##########################################################################

VPATH += project
OBJS += commands.o

VPATH += project/commands
OBJS += cmd_deepsleep.o
OBJS += cmd_sysinfo.o 
OBJS += cmd_lcd_cmds.o
OBJS += cmd_base.o


##########################################################################
# Optional driver files 
##########################################################################


# Bitmap LCD support (ST7565)
VPATH += drivers/lcd drivers/lcd/bitmap/st7565
OBJS += smallfonts.o st7565.o 

# read Keyboard
VPATH += drivers/keyboard
OBJS += keyboard.o 

# vref driver
VPATH+= drivers/vref
OBJS += vref.o

# rtc driver
VPATH+= drivers/rtc
OBJS += pcf8563.o rtc.o time.o


##########################################################################
# Library files 
##########################################################################
VPATH += core core/cmd core/cpu core/gpio core/pmu
VPATH += core/systick core/timer16 core/timer32 core/uart
VPATH += core/libc core/wdt core/iap core/i2c
OBJS += cpu.o cmd.o gpio.o pmu.o systick.o timer16.o
OBJS += timer32.o uart.o uart_buf.o stdio.o string.o wdt.o sysinit.o
OBJS += i2c.o iap.o
//OBJS += pwmdac.o 

##########################################################################
# GNU GCC compiler prefix and location
##########################################################################
CROSS_COMPILE = arm-none-eabi-
AS = $(CROSS_COMPILE)gcc
CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)gcc
SIZE = $(CROSS_COMPILE)size
OBJCOPY = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump
OUTFILE = firmware

##########################################################################
# GNU GCC compiler flags
##########################################################################
ROOT_PATH = .
INCLUDE_PATHS = -I$(ROOT_PATH) -I$(ROOT_PATH)/project

##########################################################################
# Startup files
##########################################################################

LD_PATH = lpc1xxx
LD_SCRIPT = $(LD_PATH)/linkscript.ld
LD_TEMP = $(LD_PATH)/memory.ld

ifeq (LPC11xx,$(TARGET))
  CORTEX_TYPE=m0
else
  CORTEX_TYPE=m3
endif

CPU_TYPE = cortex-$(CORTEX_TYPE)
VPATH += lpc1xxx
OBJS += $(TARGET)_handlers.o LPC1xxx_startup.o

##########################################################################
# Compiler settings, parameters and flags
##########################################################################
COMMONFLAGS = -c $(INCLUDE_PATHS) -Wall -mthumb -ffunction-sections -fdata-sections -fmessage-length=0 -mcpu=$(CPU_TYPE)
CFLAGSBASE  = $(COMMONFLAGS) -DTARGET=$(TARGET) -fno-builtin
CFLAGSBASE += -fmerge-all-constants
CFLAGSBASE += -D__NEWLIB__
# -funroll-loops
CFLAGSOPT+= $(CFLAGSBASE) -O2
CFLAGS   += $(CFLAGSBASE) -Os -g
ASFLAGS = $(COMMONFLAGS) -D__ASSEMBLY__ -x assembler-with-cpp
ASFLAGS += -Os
LDFLAGS = -nostartfiles -mcpu=$(CPU_TYPE) -mthumb -Wl,--gc-sections
OCFLAGS = --strip-unneeded  --set-start=0

all: firmware

#digop.o: core/nutemu/digop.c
#	$(CC) $(CFLAGSOPT) -o $@ $<

#nutcpu.o: core/nutemu/nutcpu.c
#	$(CC) $(CFLAGSOPT) -o $@ $<

%.o : %.c
	$(CC) $(CFLAGS) -o $@ $<

%.o : %.s
	$(AS) $(ASFLAGS) -o $@ $<

firmware: $(OBJS) $(SYS_OBJS)
	-@echo "MEMORY" > $(LD_TEMP)
	-@echo "{" >> $(LD_TEMP)
	-@echo "  flash(rx): ORIGIN = 0x00000000, LENGTH = $(FLASH)" >> $(LD_TEMP)
	-@echo "  sram(rwx): ORIGIN = 0x10000000+$(SRAM_USB), LENGTH = $(SRAM)-$(SRAM_USB)" >> $(LD_TEMP)
	-@echo "}" >> $(LD_TEMP)
	-@echo "INCLUDE $(LD_SCRIPT)" >> $(LD_TEMP)
	$(LD) $(LDFLAGS) -T $(LD_TEMP) -Wl,-Map=firmware.map -o $(OUTFILE).elf $(OBJS)
	-@echo ""
	$(SIZE) $(OUTFILE).elf
	-@echo ""
	$(OBJCOPY) $(OCFLAGS) -O binary $(OUTFILE).elf $(OUTFILE).bin
	$(OBJCOPY) $(OCFLAGS) -O ihex $(OUTFILE).elf $(OUTFILE).hex

clean:
	rm -f $(OBJS) $(LD_TEMP) $(OUTFILE).*
