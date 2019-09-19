##########################################################################################################################
# ctiot_STM32F429IGTx_FIRE GCC compiler Makefile
##########################################################################################################################

# ------------------------------------------------
# Generic Makefile (based on gcc)
# ------------------------------------------------
include config.mk
include prune.mk
######################################
# target
######################################
TARGET = ctiot_mqtt_target
######################################
# building variables
######################################
# debug build?
DEBUG = 1
# optimization
OPT = -Og


USE_OTA := no
OTA_IS_NEED_DTLS := no


#######################################
# binaries
#######################################
PREFIX    = arm-none-eabi-
CC        = $(PREFIX)gcc
AS        = $(PREFIX)gcc -x assembler-with-cpp
OBJCOPY   = $(PREFIX)objcopy
OBJDUMP   = $(PREFIX)objdump
AR        = $(PREFIX)ar
SZ        = $(PREFIX)size
LD        = $(PREFIX)ld
HEX       = $(OBJCOPY) -O ihex
BIN       = $(OBJCOPY) -O binary -S


PROJECTBASE = $(PWD)
override PROJECTBASE    := $(abspath $(PROJECTBASE))
TOP_DIR = $(PROJECTBASE)/../../..


#######################################
# paths
#######################################
# firmware library path
PERIFLIB_PATH =

# Build path
BUILD_DIR = build

######################################
# source
######################################
# ASM
ASM_SOURCES_s =  \
        ${wildcard $(PROJECTBASE)/los_startup_gcc.s}


HAL_DRIVER_SRC =  \
        $(TOP_DIR)/targets/ctiot_STM32F429IGTx_FIRE/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash.c \
        $(TOP_DIR)/targets/ctiot_STM32F429IGTx_FIRE/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_tim_ex.c \
        $(TOP_DIR)/targets/ctiot_STM32F429IGTx_FIRE/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rcc.c \
        $(TOP_DIR)/targets/ctiot_STM32F429IGTx_FIRE/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pwr_ex.c \
        $(TOP_DIR)/targets/ctiot_STM32F429IGTx_FIRE/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pwr.c \
        $(TOP_DIR)/targets/ctiot_STM32F429IGTx_FIRE/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_gpio.c \
        $(TOP_DIR)/targets/ctiot_STM32F429IGTx_FIRE/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rcc_ex.c \
        $(TOP_DIR)/targets/ctiot_STM32F429IGTx_FIRE/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash_ex.c \
        $(TOP_DIR)/targets/ctiot_STM32F429IGTx_FIRE/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash_ramfunc.c \
        $(TOP_DIR)/targets/ctiot_STM32F429IGTx_FIRE/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_cortex.c \
        $(TOP_DIR)/targets/ctiot_STM32F429IGTx_FIRE/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_uart.c \
        $(TOP_DIR)/targets/ctiot_STM32F429IGTx_FIRE/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal.c \
        $(TOP_DIR)/targets/ctiot_STM32F429IGTx_FIRE/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_tim.c \
        $(TOP_DIR)/targets/ctiot_STM32F429IGTx_FIRE/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_spi.c \
        $(TOP_DIR)/targets/ctiot_STM32F429IGTx_FIRE/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_iwdg.c
        C_SOURCES += $(HAL_DRIVER_SRC)

HARDWARE_SRC =  \
        ${wildcard $(TOP_DIR)/targets/ctiot_STM32F429IGTx_FIRE/Hardware/Src/*.c}
        C_SOURCES += $(HARDWARE_SRC)
		

HAL_DRIVER_SRC_NO_BOOTLOADER =  \
		$(TOP_DIR)/targets/ctiot_STM32F429IGTx_FIRE/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dma.c \
		$(TOP_DIR)/targets/ctiot_STM32F429IGTx_FIRE/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dma_ex.c \
		$(TOP_DIR)/targets/ctiot_STM32F429IGTx_FIRE/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_eth.c \
        $(TOP_DIR)/targets/ctiot_STM32F429IGTx_FIRE/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rng.c
		C_SOURCES += $(HAL_DRIVER_SRC_NO_BOOTLOADER)
KERNEL_SRC =  \
        ${wildcard $(TOP_DIR)/kernel/*.c} \
        ${wildcard $(TOP_DIR)/kernel/base/core/*.c} \
        ${wildcard $(TOP_DIR)/kernel/base/ipc/*.c} \
        ${wildcard $(TOP_DIR)/kernel/base/mem/bestfit_little/*.c} \
        ${wildcard $(TOP_DIR)/kernel/base/mem/membox/*.c} \
        ${wildcard $(TOP_DIR)/kernel/base/mem/common/*.c} \
        ${wildcard $(TOP_DIR)/kernel/base/misc/*.c} \
        ${wildcard $(TOP_DIR)/kernel/base/om/*.c} \
        ${wildcard $(TOP_DIR)/kernel/extended/tickless/*.c}
        C_SOURCES += $(KERNEL_SRC)

CMSIS_SRC =  \
        ${wildcard $(TOP_DIR)/osdepends/liteos/cmsis/*.c}
        C_SOURCES += $(CMSIS_SRC)

ARCH_SRC =  \
        ${wildcard $(TOP_DIR)/arch/arm/arm-m/src/*.c}
        C_SOURCES += $(ARCH_SRC)
		
ifeq ($(WITH_LWIP), yes)
LWIP_SRC =  \
        ${wildcard $(TOP_DIR)/components/net/lwip/lwip-2.0.3/src/api/*.c} \
        ${wildcard $(TOP_DIR)/components/net/lwip/lwip-2.0.3/src/core/ipv4/*.c} \
        ${wildcard $(TOP_DIR)/components/net/lwip/lwip-2.0.3/src/core/ipv6/*.c} \
        ${wildcard $(TOP_DIR)/components/net/lwip/lwip-2.0.3/src/core/*.c} \
        ${wildcard $(TOP_DIR)/components/net/lwip/lwip-2.0.3/src/netif/ethernet.c}
        C_SOURCES += $(LWIP_SRC)

LWIP_PORT_SRC = \
        ${wildcard $(TOP_DIR)/components/net/lwip/lwip_port/OS/*.c}
        C_SOURCES += $(LWIP_PORT_SRC)
endif

ifneq ($(OTA_IS_NEED_DTLS)_$(WITH_DTLS), no_no)
MBEDTLS_SRC = \
        ${wildcard $(TOP_DIR)/components/security/mbedtls/mbedtls-2.6.0/library/*.c}
        C_SOURCES += $(MBEDTLS_SRC)

MBEDTLS_PORT_SRC = \
        ${wildcard $(TOP_DIR)/components/security/mbedtls/mbedtls_port/*.c}
        C_SOURCES += $(MBEDTLS_PORT_SRC)
endif


OS_DEPENDS_SRC = \
        ${wildcard $(TOP_DIR)/osdepends/liteos/*.c}
        C_SOURCES += $(OS_DEPENDS_SRC)

SOCKET_ADAPTER_SRC = \
        ${wildcard $(TOP_DIR)/components/net/sal/*.c}
        C_SOURCES += $(SOCKET_ADAPTER_SRC)

NEWLIB_SRC = \
        ${wildcard $(TOP_DIR)/components/lib/libc/malloc.c}\
        ${wildcard $(TOP_DIR)/components/lib/libc/newlib_stub.c} \
	${wildcard $(TOP_DIR)/components/lib/libc/errno.c}
        C_SOURCES += $(NEWLIB_SRC)
		

ATINY_LOG = \
        ${wildcard $(TOP_DIR)/components/log/*.c}
        C_SOURCES += $(ATINY_LOG)


ifeq ($(WITH_MQTT), yes)
MQTT_SRC = \
        ${wildcard $(TOP_DIR)/components/connectivity/mqtt/MQTTPacket/src/*.c} \
        $(TOP_DIR)/components/connectivity/mqtt/MQTTClient-C/src/MQTTClient.c \
        $(TOP_DIR)/components/connectivity/mqtt/MQTTClient-C/src/liteOS/MQTTliteos.c
        C_SOURCES += $(MQTT_SRC)
CTIOT_MQTT_SRC = \
        ${wildcard $(TOP_DIR)/components/connectivity/ctiot/ctiot_mqtt/ctiot_mqtt_client.c}
        C_SOURCES += $(CTIOT_MQTT_SRC)
CTIOT_DEMO_MQTT_SRC = \
        ${wildcard $(TOP_DIR)/demos/ctiot_mqtt/*.c}
        C_SOURCES += $(CTIOT_DEMO_MQTT_SRC)
CJSON_SRC = \
        ${wildcard $(TOP_DIR)/components/lib/cJSON/cJSON.c}
        C_SOURCES += $(CJSON_SRC)
endif


USER_SRC =  \
        $(TOP_DIR)/targets/ctiot_STM32F429IGTx_FIRE/Src/main.c \
        $(TOP_DIR)/targets/ctiot_STM32F429IGTx_FIRE/Src/system_stm32f4xx.c \
        $(TOP_DIR)/targets/ctiot_STM32F429IGTx_FIRE/Src/usart.c \
        $(TOP_DIR)/targets/ctiot_STM32F429IGTx_FIRE/Src/dwt.c \
        $(TOP_DIR)/targets/ctiot_STM32F429IGTx_FIRE/Src/eth.c \
        $(TOP_DIR)/targets/ctiot_STM32F429IGTx_FIRE/Src/sys_init.c \
        $(TOP_DIR)/targets/ctiot_STM32F429IGTx_FIRE/Src/user_task.c
        C_SOURCES += $(USER_SRC)


# ASM sources

ASM_SOURCES_S =  \
        ${wildcard $(TOP_DIR)/arch/arm/arm-m/cortex-m4/gcc/los_dispatch_gcc.S}


######################################
# firmware library
######################################
PERIFLIB_SOURCES =


#######################################
# CFLAGS
#######################################
# cpu
CPU = -mcpu=cortex-m4
# fpu
FPU = -mfpu=fpv4-sp-d16
# float-abi
FLOAT-ABI = -mfloat-abi=hard
# mcu
MCU = $(CPU) -mthumb $(FPU) $(FLOAT-ABI)

# macros for gcc
# AS defines
AS_DEFS =

# C defines
C_DEFS =  \
        -D USE_HAL_DRIVER \
        -D STM32F429xx \
        -D NDEBUG      \
		-D WITH_CTIOT_DEMO

ifneq ($(OTA_IS_NEED_DTLS)_$(WITH_DTLS), no_no)
	ifeq ($(WITH_MQTT), yes)
		C_DEFS += -D MBEDTLS_CONFIG_FILE=\"los_mbedtls_config_cert.h\"
	else
		C_DEFS += -D MBEDTLS_CONFIG_FILE=\"los_mbedtls_config.h\"
	endif
	ifeq ($(USE_MBEDTLS_DEBUG_C), yes)
		C_DEFS += -DMBEDTLS_DEBUG_C
	endif

	ifeq ($(USE_MBEDTLS_AES_ROM_TABLES), yes)
		C_DEFS += -DMBEDTLS_AES_ROM_TABLES
	endif
endif

ifeq ($(WITH_DTLS), yes)
		C_DEFS += -DWITH_DTLS
endif

ifeq ($(WITH_LWIP), yes)
		C_DEFS += \
			-DWITH_LWIP \
			-D LWIP_TIMEVAL_PRIVATE=0 

	ifeq ($(USE_LWIP_TCP), yes)
		C_DEFS += -DLWIP_TCP=1
	else
		C_DEFS += -DLWIP_TCP=0
	endif
endif



ifeq ($(ATINY_DEBUG), yes)
		C_DEFS += -DATINY_DEBUG
endif


ifeq ($(WITH_MQTT), yes)
C_DEFS += -DWITH_MQTT
endif

ifeq ($(WITH_IPV4), no)
C_DEFS += -DLWIP_IPV4=0
else
C_DEFS += -DLWIP_IPV4=1
endif

ifeq ($(WITH_IPV6), yes)
C_DEFS += -DLWIP_IPV6=1
else
C_DEFS += -DLWIP_IPV6=0
endif

# AS includes
AS_INCLUDES =

# C includes
HAL_DRIVER_INC = \
        -I $(TOP_DIR)/targets/ctiot_STM32F429IGTx_FIRE/Drivers/STM32F4xx_HAL_Driver/Inc \
        -I $(TOP_DIR)/targets/ctiot_STM32F429IGTx_FIRE/Drivers/STM32F4xx_HAL_Driver/Inc/Legacy
        C_INCLUDES += $(HAL_DRIVER_INC)
		
INCLUDE_INC = \
        -I $(TOP_DIR)/include \
		-I $(TOP_DIR)/include/ctiot_mqtt
        C_INCLUDES += $(INCLUDE_INC)		

HARDWARE_INC = \
        -I $(TOP_DIR)/targets/ctiot_STM32F429IGTx_FIRE/Hardware/Inc
        C_INCLUDES += $(HARDWARE_INC)

USER_INC = \
        -I $(TOP_DIR)/targets/ctiot_STM32F429IGTx_FIRE/Inc
        C_INCLUDES += $(USER_INC)

CMSIS_INC = \
		-I $(TOP_DIR)/arch/arm/arm-m/include \
        -I $(TOP_DIR)/arch/arm/common/cmsis
        C_INCLUDES += $(CMSIS_INC)

LITEOS_CMSIS = \
        -I $(TOP_DIR)/osdepends/liteos/cmsis
        C_INCLUDES += $(LITEOS_CMSIS)
		

OS_CONFIG_INC = \
		-I $(TOP_DIR)/targets/ctiot_STM32F429IGTx_FIRE/OS_CONFIG
		C_INCLUDES += $(OS_CONFIG_INC)
KERNEL_INC = \
        -I $(TOP_DIR)/kernel/include \
	-I $(TOP_DIR)/kernel/base/include \
	-I $(TOP_DIR)/kernel/extended/include
        C_INCLUDES += $(KERNEL_INC)

ARCH_INC = \
        -I $(TOP_DIR)/arch/arm/arm-m/include
        C_INCLUDES += $(ARCH_INC)

	
ifeq ($(WITH_LWIP), yes)
LWIP_INC = \
        -I $(TOP_DIR)/components/net/lwip/lwip-2.0.3/src/include \
		-I $(TOP_DIR)/components/net/lwip/lwip-2.0.3/src/include/lwip \
		-I $(TOP_DIR)/components/net/lwip/lwip-2.0.3/src/include/lwip/prot        
        
		C_INCLUDES += $(LWIP_INC)

LWIP_PORT_INC = \
        -I $(TOP_DIR)/components/net/lwip/lwip_port \
        -I $(TOP_DIR)/components/net/lwip/lwip_port/OS
        C_INCLUDES += $(LWIP_PORT_INC)
endif

ifneq ($(OTA_IS_NEED_DTLS)_$(WITH_DTLS), no_no)
MBEDTLS_INC = \
        -I $(TOP_DIR)/components/security/mbedtls/mbedtls-2.6.0/include
        C_INCLUDES += $(MBEDTLS_INC)

MBEDTLS_PORT_INC = \
        -I $(TOP_DIR)/components/security/mbedtls/mbedtls_port
        C_INCLUDES += $(MBEDTLS_PORT_INC)
endif


ifeq ($(WITH_MQTT), yes)	
MQTT_INC = \
        -I $(TOP_DIR)/components/connectivity/mqtt/MQTTClient-C/src \
        -I $(TOP_DIR)/components/connectivity/mqtt/MQTTClient-C/src/liteOS \
		-I $(TOP_DIR)/components/connectivity/mqtt/MQTTPacket/src \
		-I $(TOP_DIR)/components/lib/cJSON     \
		-I $(TOP_DIR)/components/connectivity/ctiot/ctiot_mqtt
        C_INCLUDES += $(MQTT_INC)
endif

DEMO_INC = \
        -I $(TOP_DIR)/demos \
		-I $(TOP_DIR)/demos/ctiot_mqtt
		
        C_INCLUDES += $(DEMO_INC) 



# compile gcc flags
ASFLAGS = $(MCU) $(AS_DEFS) $(AS_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

CFLAGS = $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

ifeq ($(DEBUG), 1)
CFLAGS += -g -gdwarf-2
endif


# Generate dependency information
CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$@"


#######################################
# LDFLAGS
#######################################
# link script
LD_FILE = STM32F429IGTx_LiteOS.ld

LDSCRIPT = $(PROJECTBASE)/$(LD_FILE)

# libraries
LIBS = -lc -lm -lnosys
ifeq ($(USE_OTA), yes)
LIBS += -lhwpatch
endif
LIBDIR = -L$(PROJECTBASE)/../Lib
LDFLAGS = $(MCU) -specs=nano.specs -T$(LDSCRIPT) $(LIBDIR) $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections

ifeq ($(WITH_MQTT), yes)
LDFLAGS += -u _printf_float
endif

# default action: build all
all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin

#######################################
# build the application
#######################################
# list of objects
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))
# list of ASM program objects
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES_s:.s=.o)))
vpath %.s $(sort $(dir $(ASM_SOURCES_s)))

OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES_S:.S=.o)))
vpath %.S $(sort $(dir $(ASM_SOURCES_S)))


$(BUILD_DIR)/%.o: %.c Makefile config.mk prune.mk | $(BUILD_DIR)
	$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@

$(BUILD_DIR)/%.o: %.s Makefile config.mk prune.mk | $(BUILD_DIR)
	$(AS) -c $(CFLAGS) $< -o $@


$(BUILD_DIR)/%.o: %.S Makefile config.mk prune.mk | $(BUILD_DIR)
	$(AS) -c $(CFLAGS) $< -o $@


$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) Makefile config.mk prune.mk
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	$(SZ) $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(HEX) $< $@

$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(BIN) $< $@

$(BUILD_DIR):
	mkdir $@

#######################################
# clean up
#######################################
clean:
	-rm -fR .dep $(BUILD_DIR)

#######################################
# dependencies
#######################################
#-include $(shell mkdir .dep 2>/dev/null) $(wildcard .dep/*)
-include $(wildcard $(BUILD_DIR)/*.d)

# *** EOF ***
