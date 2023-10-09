# Makefile for EFR32 programs using the Gecko SDK

#######################################
# Project configuration
#######################################

# Default Gekkokapula model
KAPULA ?= v2

# Name of the binary
TARGET = gekkofirmis_$(KAPULA)

BUILD_DIR = build_$(KAPULA)

# Optimization flags
OPT = -O2

DEBUG = 1

# C sources
C_SOURCES = $(wildcard src/*.c)
C_SOURCES += $(wildcard freertos/*.c)

# C include paths
C_INCLUDES = -I. -Iinc -Ifreertos

# C defines
C_DEFS = -DKAPULA_$(KAPULA)=1

# Other C flags
C_FLAGS = -std=gnu11 -Wall -Wextra -fdata-sections -ffunction-sections

# Assembly sources
ASM_SOURCES =

# Assembly include paths
AS_INCLUDES =

# Assembly defines
AS_DEFS =

# Other assembler flags
AS_FLAGS = -std=gnu11 -Wall -Wextra -fdata-sections -ffunction-sections

# Library paths
LIBDIR =

# Libraries
LIBS =

# Custom linker script
#LDSCRIPT =

# Other linker flags
LD_FLAGS = -Wl,--wrap=RAILINT_999bd22c50df2f99ce048cba68f11c3a


#######################################
# CMSIS and Gecko SDK
#######################################

# Path to Gecko SDK
GECKOSDK = ../gecko_sdk_suite/v2.7

# CMSIS
C_INCLUDES += -I$(GECKOSDK)/platform/CMSIS/Include

# CMSIS DSP
C_SOURCES += $(GECKOSDK)/platform/CMSIS/DSP_Lib/Source/CommonTables/arm_const_structs.c
C_SOURCES += $(GECKOSDK)/platform/CMSIS/DSP_Lib/Source/CommonTables/arm_common_tables.c
# Precompiled CMSIS DSP can't be used, since it uses hard float ABI whereas
# RAIL uses softfp ABI, so the whole program has to be compiled with softfp.
#LIBDIR += -L$(GECKOSDK)/platform/CMSIS/Lib/GCC
#LIBS += -larm_cortexM4lf_math
C_SOURCES   += $(GECKOSDK)/platform/CMSIS/DSP_Lib/Source/TransformFunctions/arm_cfft_f32.c
C_SOURCES   += $(GECKOSDK)/platform/CMSIS/DSP_Lib/Source/TransformFunctions/arm_cfft_radix8_f32.c
ASM_SOURCES += $(GECKOSDK)/platform/CMSIS/DSP_Lib/Source/TransformFunctions/arm_bitreversal2.S

# EMLIB
C_SOURCES += $(wildcard $(GECKOSDK)/platform/emlib/src/*.c)
C_INCLUDES += -I$(GECKOSDK)/platform/emlib/inc


#######################################
# EFR32 chip specific parts
#######################################

# The following variables depend on the model:
### C_DEFS: Define for the EFR32 part number
### C_INCLUDES: Include paths for for the Gecko SDK
### ASM_SOURCES, C_SOURCES: Startup and system code for the EFR32
### LDSCRIPT: Linker script for memory map of the EFR32

ifeq ($(KAPULA), v1)
  # First hardware version built from a radio module
  C_DEFS += -DEFR32MG1P232F256GM48

  C_INCLUDES  += -I$(GECKOSDK)/platform/Device/SiliconLabs/EFR32MG1P/Include
  ASM_SOURCES +=   $(GECKOSDK)/platform/Device/SiliconLabs/EFR32MG1P/Source/GCC/startup_efr32mg1p.S
  C_SOURCES   +=   $(GECKOSDK)/platform/Device/SiliconLabs/EFR32MG1P/Source/system_efr32mg1p.c
  LDSCRIPT     =   $(GECKOSDK)/platform/Device/SiliconLabs/EFR32MG1P/Source/GCC/efr32mg1p.ld

  LIBS += -lrail_efr32xg1_gcc_release
  C_SOURCES += ../railconfig/rail_config.c
  C_INCLUDES += -I../railconfig

else ifeq ($(KAPULA), bgm)
  # BGM Bluetooth module (just an idea, not built yet)
  C_DEFS += -DBGM111A256V2

  C_INCLUDES  += -I$(GECKOSDK)/platform/Device/SiliconLabs/BGM1/Include
  ASM_SOURCES +=   $(GECKOSDK)/platform/Device/SiliconLabs/BGM1/Source/GCC/startup_bgm1.S
  C_SOURCES   +=   $(GECKOSDK)/platform/Device/SiliconLabs/BGM1/Source/system_bgm1.c
  LDSCRIPT     =   $(GECKOSDK)/platform/Device/SiliconLabs/BGM1/Source/GCC/bgm1.ld

else ifeq ($(KAPULA), v2)
  # Dual-band model with a custom PCB (work in progress)
  C_DEFS += -DEFR32FG14P233F256GM48

  C_INCLUDES  += -I$(GECKOSDK)/platform/Device/SiliconLabs/EFR32FG14P/Include
  ASM_SOURCES +=   $(GECKOSDK)/platform/Device/SiliconLabs/EFR32FG14P/Source/GCC/startup_efr32fg14p.S
  C_SOURCES   +=   $(GECKOSDK)/platform/Device/SiliconLabs/EFR32FG14P/Source/system_efr32fg14p.c
  LDSCRIPT     =   $(GECKOSDK)/platform/Device/SiliconLabs/EFR32FG14P/Source/GCC/efr32fg14p.ld

  LIBS += -lrail_efr32xg14_gcc_release
  C_SOURCES += ../railconfig_v2/rail_config.c
  C_INCLUDES += -I../railconfig_v2

else
  $(error Unknown Gekkokapula model $(KAPULA))
endif

# RAIL library
C_INCLUDES += -I$(GECKOSDK)/platform/radio/rail_lib/common
C_INCLUDES += -I$(GECKOSDK)/platform/radio/rail_lib/chip/efr32/efr32xg1x
LIBDIR += -L$(GECKOSDK)/platform/radio/rail_lib/autogen/librail_release

# Processor core
MCU = -mcpu=cortex-m4 -mthumb

# Float ABI: softfp is needed to link RAIL
MCU += -mfpu=fpv4-sp-d16 -mfloat-abi=softfp

# TODO: avoid __FPU_PRESENT redefinition warnings somehow
C_DEFS += -DARM_MATH_CM4=1 -D__FPU_PRESENT=1


#######################################
# Compiler flags
#######################################

# Link the standard library
LIBS += -lc -lm -lnosys
LDFLAGS = $(MCU) -specs=nano.specs -T$(LDSCRIPT) $(LIBDIR) $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections $(LD_FLAGS)

ASFLAGS = $(MCU) $(AS_DEFS) $(AS_INCLUDES) $(OPT) $(AS_FLAGS)
CFLAGS = $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) $(C_FLAGS)

ifeq ($(DEBUG), 1)
CFLAGS += -g -gdwarf-2
endif

# Generate dependency information
CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)"


#######################################
# ARM toolchain
#######################################

PREFIX = arm-none-eabi-
CC = $(PREFIX)gcc
AS = $(PREFIX)gcc -x assembler-with-cpp
CP = $(PREFIX)objcopy
SZ = $(PREFIX)size
HEX = $(CP) -O ihex
BIN = $(CP) -O binary -S
OPENOCD = openocd


#######################################
# Build targets and commands
#######################################

# default action: build all
all: build

# More descriptive alternative target names for use in build task
build_only: build
build_and_flash: flash

#build: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin
build: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).bin

# list of objects
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))
# list of ASM program objects
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.S=.o)))
vpath %.S $(sort $(dir $(ASM_SOURCES)))

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR)
	$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@

$(BUILD_DIR)/%.o: %.S Makefile | $(BUILD_DIR)
	$(AS) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) Makefile
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	$(SZ) $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(HEX) $< $@

$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(BIN) $< $@

$(BUILD_DIR):
	mkdir $@

clean:
	-rm -fR $(BUILD_DIR)

flash: $(BUILD_DIR)/$(TARGET).elf
	$(OPENOCD) -f openocd/adapter.cfg -c init -c 'program "$(BUILD_DIR)/$(TARGET).elf" verify reset' -c exit

.PHONY: all build clean flash

# Dependencies
-include $(wildcard $(BUILD_DIR)/*.d)
