# Architecture directories
ARCHITECTURE_DIR = efr32
BUILD_DIR = build
OUTPUT_DIR = $(BUILD_DIR)/$(ARCHITECTURE_DIR)

# Stack and submodule directories
GLOBAL_BASE_DIR     = ../../../SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v1.1/platform/base/hal/..

SOURCE_FILES = \
  flex-bookkeeping.c \
  flex-callback-stubs.c \
  flex-callbacks.c \
  flex-events.c \
  main.c \
  rail_config.c \
  ../../../SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v1.1/hardware/kit/common/bsp/bsp_bcc.c \
  ../../../SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v1.1/hardware/kit/common/bsp/bsp_stk.c \
  ../../../SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v1.1/hardware/kit/common/bsp/bsp_stk_leds.c \
  ../../../SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v1.1/hardware/kit/common/drivers/dmactrl.c \
  ../../../SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v1.1/hardware/kit/common/drivers/i2cspm.c \
  ../../../SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v1.1/hardware/kit/common/drivers/retargetio.c \
  ../../../SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v1.1/hardware/kit/common/drivers/retargetserial.c \
  ../../../SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v1.1/hardware/kit/common/drivers/udelay.c \
  ../../../SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v1.1/platform/Device/SiliconLabs/EFR32MG1P/Source/GCC/startup_efr32mg1p.S \
  ../../../SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v1.1/platform/emdrv/gpiointerrupt/src/gpiointerrupt.c \
  ../../../SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v1.1/platform/emdrv/rtcdrv/src/rtcdriver.c \
  ../../../SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v1.1/platform/emdrv/ustimer/src/ustimer.c \
  ../../../SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v1.1/platform/emlib/src/em_assert.c \
  ../../../SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v1.1/platform/emlib/src/em_cmu.c \
  ../../../SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v1.1/platform/emlib/src/em_core.c \
  ../../../SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v1.1/platform/emlib/src/em_emu.c \
  ../../../SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v1.1/platform/emlib/src/em_gpio.c \
  ../../../SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v1.1/platform/emlib/src/em_i2c.c \
  ../../../SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v1.1/platform/emlib/src/em_ldma.c \
  ../../../SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v1.1/platform/emlib/src/em_leuart.c \
  ../../../SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v1.1/platform/emlib/src/em_msc.c \
  ../../../SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v1.1/platform/emlib/src/em_rmu.c \
  ../../../SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v1.1/platform/emlib/src/em_rtc.c \
  ../../../SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v1.1/platform/emlib/src/em_rtcc.c \
  ../../../SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v1.1/platform/emlib/src/em_system.c \
  ../../../SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v1.1/platform/emlib/src/em_timer.c \
  ../../../SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v1.1/platform/emlib/src/em_usart.c \
  ../../../SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v1.1/platform/radio/rail_lib/hal/efr32/hal_efr.c \
  ../../../SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v1.1/platform/radio/rail_lib/hal/hal_common.c \


LIB_FILES = \
  ../../../SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v1.1/protocol/flex_1.2/connect/plugins/libraries/phy-rail-efr32xg1-library-gcc.a \


CDEFS = -DNULL_BTL \
-DEFR32MG1P232F256GM48 \
-DEFR32MG1P \
  -DPLATFORM_HEADER=\"micro/cortexm3/compiler/gcc.h\" \
  -DEMBER_AF_API_EMBER_TYPES=\"stack/include/ember-types.h\" \
  -DEMBER_STACK_CONNECT \
  -DEFR32_SERIES1_CONFIG1_MICRO \
  -DHAL_MICRO \
  -DCORTEXM3 \
  -DCORTEXM3_EFM32_MICRO \
  -DCONFIGURATION_HEADER=\"flex-configuration.h\" \
  -DHAL_CONFIG=\"connect/hal-config/flex-config.h\" \
  -DCORTEXM3_EFR32 \
  -DPHY_EFR32 \
  -DCORTEXM3_EFR32_MICRO \
  -DDEBUG_EFR \
  -DAESDRV_HWACCEL_DMA_ENABLED=false \
  -DEMBER_SERIAL0_DEBUG \
  -DCMU_CTRL_HFRADIOCLK \
  -DDEBUG_LEVEL=NO_DEBUG \
  -DBOARD_BRD4151A \
  -DBOARD_HEADER=\"brd4151a.h\" \


ASMDEFS = -DNULL_BTL \
-DEFR32MG1P232F256GM48 \
-DEFR32MG1P \
  -DPLATFORM_HEADER=\"micro/cortexm3/compiler/gcc.h\" \
  -DEMBER_AF_API_EMBER_TYPES=\"stack/include/ember-types.h\" \
  -DEMBER_STACK_CONNECT \
  -DEFR32_SERIES1_CONFIG1_MICRO \
  -DHAL_MICRO \
  -DCORTEXM3 \
  -DCORTEXM3_EFM32_MICRO \
  -DCONFIGURATION_HEADER=\"flex-configuration.h\" \
  -DHAL_CONFIG=\"connect/hal-config/flex-config.h\" \
  -DCORTEXM3_EFR32 \
  -DPHY_EFR32 \
  -DCORTEXM3_EFR32_MICRO \
  -DDEBUG_EFR \
  -DAESDRV_HWACCEL_DMA_ENABLED=false \
  -DEMBER_SERIAL0_DEBUG \
  -DCMU_CTRL_HFRADIOCLK \
  -DDEBUG_LEVEL=NO_DEBUG \
  -DBOARD_BRD4151A \
  -DBOARD_HEADER=\"brd4151a.h\" \


CINC = -I. \
-I$(ARM_IAR7_DIR)/ARM/INC \
  -Ifreertos \
  -I../../../SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v1.1/protocol/flex_1.2/connect/plugins \
  -I../../../SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v1.1/protocol/flex_1.2/connect/plugins/stack \
  -I../../../SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v1.1/protocol/flex_1.2/connect/plugins/stack/include \
  -I../../../SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v1.1/platform/base \
  -I../../../SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v1.1/platform/base/hal \
  -I../../../SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v1.1/platform/base/hal/micro \
  -I../../../SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v1.1/platform/base/hal/micro/cortexm3/common \
  -I../../../SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v1.1/platform/base/hal/micro/cortexm3/efm32/board \
  -I../../../SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v1.1/platform/base/hal/micro/cortexm3/efm32/config \
  -I../../../SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v1.1/platform/base/hal/micro/cortexm3/efm32 \
  -I../../../SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v1.1/platform/CMSIS/Include \
  -I../../../SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v1.1/platform/emdrv/rtcdrv/config \
  -I../../../SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v1.1/platform/emdrv/rtcdrv/inc \
  -I../../../SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v1.1/platform/emdrv/sleep/inc \
  -I../../../SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v1.1/platform/emdrv/ustimer/config \
  -I../../../SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v1.1/platform/emdrv/ustimer/inc \
  -I../../../SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v1.1/platform/emdrv/gpiointerrupt/inc \
  -I../../../SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v1.1/platform/emdrv/common/inc \
  -I../../../SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v1.1/platform/emlib/inc \
  -I../../../SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v1.1/hardware/kit/common \
  -I../../../SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v1.1/hardware/kit/common/bsp \
  -I../../../SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v1.1/hardware/kit/common/drivers \
  -I../../../SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v1.1/platform/radio/rail_lib/hal/efr32 \
  -I../../../SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v1.1/platform/radio/rail_lib/hal \
  -I../../../SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v1.1/protocol/flex_1.2 \
  -I../../../SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v1.1/platform/base \
  -I../../../SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v1.1/platform/radio/rail_lib/chip/efr32 \
  -I../../../SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v1.1/platform/radio/rail_lib/chip/efr32/rf/common \
  -I../../../SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v1.1/platform/radio/rail_lib/chip/efr32/rf/common/cortex \
  -I../../../SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v1.1/platform/radio/rail_lib/common \
  -I../../../SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v1.1/platform/radio/rail_lib/protocol/ieee802154 \
  -I../../../SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v1.1/platform/base/phy \
  -I../../../SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v1.1/hardware/kit/EFR32MG1_BRD4151A/config \
  -I../../../SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v1.1/hardware/kit/common/bsp \
  -I../../../SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v1.1/hardware/kit/EFR32MG1_BRD4151A/config \


TARGET = mgkokeilu2

CSOURCES = $(filter %.c, $(SOURCE_FILES))
ASMSOURCES = $(filter %.s79, $(SOURCE_FILES))
ASMSOURCES2 = $(filter %.s, $(SOURCE_FILES))

COBJS = $(addprefix $(OUTPUT_DIR)/,$(subst ../,,$(CSOURCES:.c=.o)))
ASMOBJS = $(addprefix $(OUTPUT_DIR)/,$(subst ../,,$(ASMSOURCES:.s79=.o)))
ASMOBJS2 = $(addprefix $(OUTPUT_DIR)/,$(subst ../,,$(ASMSOURCES2:.s=.o)))

OUTPUT_DIRS = $(sort $(dir $(COBJS)) $(dir $(ASMOBJS)) $(dir $(ASMOBJS2)))

ARCHITECTUREID = efr32~family[m]~series[1]~device_configuration[1]~performance[p]~radio[232]~flash[256k]~temp[g]~package[m]~pins[48]+none+gcc

# GNU ARM compiler
ifeq ($(findstring +gcc,$(ARCHITECTUREID)), +gcc)
$(info GCC Build)
	# Add linker circular reference as the order of objects may matter for any libraries used
	GROUP_START =-Wl,--start-group
	GROUP_END =-Wl,--end-group        

	CCFLAGS = -g3 \
    -gdwarf-2 \
    -mcpu=cortex-m4 \
    -mthumb \
    -std=gnu99 \
    -Os  \
    -Wall  \
    -c  \
    -fmessage-length=0  \
    -ffunction-sections  \
    -fdata-sections  \
    -mfpu=fpv4-sp-d16  \
    -mfloat-abi=softfp \
	$(CDEFS) \
	$(CINC) \

	ASMFLAGS = -c \
	-g3 \
	-gdwarf-2 \
	-mcpu=cortex-m4 \
	-mthumb \
	-c \
	-x assembler-with-cpp \
	$(CINC) \
	$(ASMDEFS)

	LDFLAGS = -g3 \
	-gdwarf-2 \
	-mcpu=cortex-m4 \
	-mthumb -T "$(GLOBAL_BASE_DIR)/hal/micro/cortexm3/efm32/gcc-cfg.ld" \
	-L"$(GLOBAL_BASE_DIR)/hal/micro/cortexm3/" \
	-Xlinker --defsym="SIMEEPROM_SIZE=8K" \
	-Xlinker --gc-sections \
	-Xlinker -Map="$(LST_DIR)/$(PROJECTNAME).map" \
	-mfpu=fpv4-sp-d16 \
	-mfloat-abi=softfp --specs=nano.specs -u _printf_float \
	-Wl,--start-group -lgcc -lc -lnosys   -Wl,--end-group

	ARCHFLAGS = r

	ELFTOOLFLAGS_BIN = -O binary
	ELFTOOLFLAGS_HEX = -O ihex
	ELFTOOLFLAGS_S37 = -O srec

	ARCH = $(JAMEXE_PREFIX) $(ARM_GNU_DIR)/bin/arm-none-eabi-ar.exe
	AS = $(JAMEXE_PREFIX) $(ARM_GNU_DIR)/bin/arm-none-eabi-gcc.exe
	CC = $(JAMEXE_PREFIX) $(ARM_GNU_DIR)/bin/arm-none-eabi-gcc.exe
	ELFTOOL = $(JAMEXE_PREFIX) $(ARM_GNU_DIR)/bin/arm-none-eabi-objcopy.exe
	LD = $(JAMEXE_PREFIX) $(ARM_GNU_DIR)/bin/arm-none-eabi-gcc.exe

endif

# IAR 7.xx toolchain
ifeq ($(findstring +iar,$(ARCHITECTUREID)), +iar)
$(info IAR Build)

	# IAR is not sensitive to linker lib order thus no groups needed.
	GROUP_START =
	GROUP_END =
	CINC += -I$(ARM_IAR6_DIR)/ARM/INC

	ifndef ARM_IAR7_DIR
	$(error ARM_IAR7_DIR is not set. Please define ARM_IAR7_DIR pointing to your IAR 7.xx installation folder.)
	endif

	CCFLAGS = --cpu=cortex-m3 \
	--cpu_mode=thumb \
	--diag_suppress=Pa050 \
	-e \
	--endian=little \
	--fpu=none \
	--no_path_in_file_macros \
	--separate_cluster_for_initialized_variables \
	--dlib_config=$(ARM_IAR7_DIR)/ARM/inc/c/DLib_Config_Normal.h \
	--debug \
	--dependencies=m $*.d \
	-Ohz \
	$(CDEFS) \
	$(CINC)

	ASMFLAGS = --cpu cortex-M3 \
	--fpu None \
	-s+ \
	"-M<>" \
	-w+ \
	-t2 \
	-r \
	$(CINC) \
	$(ASMDEFS)

	LDFLAGS = --entry halEntryPoint \
	--diag_suppress=Lp012 \
	--config $(GLOBAL_BASE_DIR)/hal/micro/cortexm3/efm32/iar-cfg.icf \
	--config_def NULL_BTL=1

	ARCH = $(JAMEXE_PREFIX) $(ARM_IAR7_DIR)/arm/bin/iarchive.exe
	AS = $(JAMEXE_PREFIX) $(ARM_IAR7_DIR)/arm/bin/iasmarm.exe
	CC = $(JAMEXE_PREFIX) $(ARM_IAR7_DIR)/arm/bin/iccarm.exe
	ELFTOOL = $(JAMEXE_PREFIX) $(ARM_IAR7_DIR)/arm/bin/ielftool.exe
	LD = $(JAMEXE_PREFIX) $(ARM_IAR7_DIR)/arm/bin/ilinkarm.exe

	# No archiver arguments needed for IAR.
	ARCHFLAGS =

	ELFTOOLFLAGS_BIN = --bin
	ELFTOOLFLAGS_HEX = --ihex
	ELFTOOLFLAGS_S37 = --srec --srec-s3only

endif

.PHONY: all clean PROLOGUE

all: PROLOGUE $(OUTPUT_DIRS) $(COBJS) $(ASMOBJS) $(ASMOBJS2) $(LIB_FILES)
	@echo 'Linking...'
	@$(LD) $(LDFLAGS) $(GROUP_START) $(COBJS) $(ASMOBJS) $(ASMOBJS2) $(LIB_FILES) $(GROUP_END) -o $(OUTPUT_DIR)/$(TARGET).out	
	@$(ELFTOOL) $(OUTPUT_DIR)/$(TARGET).out $(ELFTOOLFLAGS_BIN) $(OUTPUT_DIR)/$(TARGET).bin
	@$(ELFTOOL) $(OUTPUT_DIR)/$(TARGET).out $(ELFTOOLFLAGS_HEX) $(OUTPUT_DIR)/$(TARGET).hex
	@$(ELFTOOL) $(OUTPUT_DIR)/$(TARGET).out $(ELFTOOLFLAGS_S37) $(OUTPUT_DIR)/$(TARGET).s37
	@echo 'Done.'

PROLOGUE:
#	@echo $(COBJS)
#	@echo $(ASMOBJS)
#	@echo $(ASMOBJS2)

$(OUTPUT_DIRS):
	@mkdir -p $@

$(COBJS): %.o:
	@echo 'Building $(notdir $(@:%.o=%.c))...'
	@$(CC) $(CCFLAGS) -o $@ $(filter %$(@:$(OUTPUT_DIR)/%.o=%.c),$(CSOURCES)) > /dev/null; \

$(ASMOBJS): %.o:
	@echo 'Building $(notdir $(@:%.o=%.s79))...'
	@$(AS) $(ASMFLAGS) -o $@ $(filter %$(@:$(OUTPUT_DIR)/%.o=%.s79),$(ASMSOURCES)) > /dev/null

$(ASMOBJS2): %.o:
	@echo 'Building $(notdir $(@:%.o=%.s))...'
	@$(AS) $(ASMFLAGS) -o $@ $(filter %$(@:$(OUTPUT_DIR)/%.o=%.s),$(ASMSOURCES2)) > /dev/null
  
clean:
	$(RM) -r $(OUTPUT_DIR)