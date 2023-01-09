TARGET = APP

SYS_DIR = -I./Hardware
SYS_SOURCE = ./Hardware
######################################

# 若添加新的文件在目录Hardware下，只需在“USER_INC”添加即可，若不是需要在“USER_SOURCE”添加.c文件的路径
# user_include 添加.h文件所在路径
USER_INC = \
$(SYS_DIR)/usart 

# user_Source 添加.c文件
USER_SOURCE = \
$(wildcard $(SYS_SOURCE)/*/*.c)
######################################
#openocd路径配置 
#OPENOCD_DOWN_PATH = C:/Program Files (x86)/openocd/share/openocd/scripts/interface/jlink.cfg
#OPENOCD_CHIP_PATH =C:/Program Files (x86)/openocd/share/openocd/scripts/target/stm32f1x.cfg
OPENOCD_DOWN_PATH = D:\freertos\ARM-GCC\openocd\share\openocd\scripts\interface\jlink.cfg
OPENOCD_CHIP_PATH =D:\freertos\ARM-GCC\openocd\share\openocd\scripts/target\stm32f1x.cfg

######################################
#芯片flash大小选择
# High_density : 是 STM32F101xx 和 STM32F103xx 微控制器，其中闪存密度范围在 256KB 和 512 KB 之间。
# Medium_density:是 STM32F101xx、STM32F102xx 和 STM32F103xx微控制器，其中闪存密度范围在 32KB 和 128 KB 之间。
FLASH_SIZE = Medium_density

######################################
# 启动文件选择
#startup_stm32f10x_cl.s         互联型的器件，STM32F105xx，STM32F107xx
#startup_stm32f10x_hd.s         大容量的STM32F101xx，STM32F102xx，STM32F103xx
#startup_stm32f10x_hd_vl.s      大容量的STM32F100xx
#startup_stm32f10x_ld.s         小容量的STM32F101xx，STM32F102xx，STM32F103xx
#startup_stm32f10x_ld_vl.s      小容量的STM32F100xx
#startup_stm32f10x_md.s         中容量的STM32F101xx，STM32F102xx，STM32F103xx
#startup_stm32f10x_md_vl.s      中容量的STM32F100xx
#startup_stm32f10x_xl.s         FLASH在512K到1024K字节的STM32F101xx，STM32F102xx，STM32F103xx

#STM32F103C8T6型号使用 md
STARTUP = startup_stm32f10x_md.s
#全局宏定义选择
DEFINE = STM32F10X_MD 

######################################
# building variables
######################################
# debug build?
DEBUG = 1
# optimization
OPT = -O0


#######################################
# paths
#######################################
# Build path
BUILD_DIR = build

######################################
# source
######################################
# C sources
C_SOURCES =  \
./app/main.c \
./jvos/MCU/stm32/stm32f103/system_stm32f10x.c 
#$(USER_SOURCE)

# ASM sources
ASM_SOURCES =  \
jvos/MCU/stm32/stm32f103/startup/$(STARTUP)


#######################################
# binaries
#######################################
PREFIX = arm-none-eabi-
# The gcc compiler bin path can be either defined in make command via GCC_PATH variable (> make GCC_PATH=xxx)
# either it can be added to the PATH environment variable.
ifdef GCC_PATH
CC = $(GCC_PATH)/$(PREFIX)gcc
AS = $(GCC_PATH)/$(PREFIX)gcc -x assembler-with-cpp
CP = $(GCC_PATH)/$(PREFIX)objcopy
SZ = $(GCC_PATH)/$(PREFIX)size
else
CC = $(PREFIX)gcc
AS = $(PREFIX)gcc -x assembler-with-cpp
CP = $(PREFIX)objcopy
SZ = $(PREFIX)size
endif
HEX = $(CP) -O ihex
BIN = $(CP) -O binary -S
 
#######################################
# CFLAGS
#######################################
# cpu
CPU = -mcpu=cortex-m3

# fpu
# NONE for Cortex-M0/M0+/M3

# float-abi


# mcu
MCU = $(CPU) -mthumb $(FPU) $(FLOAT-ABI)

# macros for gcc
# AS defines
AS_DEFS = 

# C defines
C_DEFS =  \
-D$(DEFINE)


# AS includes
AS_INCLUDES = 

# C includes
C_INCLUDES =  \
-I./jvos/MCU/stm32/stm32f103 
#$(USER_INC)


# compile gcc flags
ASFLAGS = $(MCU) $(AS_DEFS) $(AS_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

CFLAGS = $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

ifeq ($(DEBUG), 1)
CFLAGS += -g -gdwarf-2
endif


# Generate dependency information
CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)"


#######################################
# LDFLAGS
#######################################
# link script
LDSCRIPT = ./jvos/MCU/stm32/stm32f103/Link/$(FLASH_SIZE)/stm32_flash.ld

# libraries
LIBS = -lc -lm -lnosys 
LIBDIR = 
LDFLAGS = $(MCU) -specs=nano.specs -T$(LDSCRIPT) $(LIBDIR) $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections -lc -lrdimon -u _printf_float

# default action: build all
all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin


#######################################
# build the application
#######################################
# list of objects
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))
# list of ASM program objects
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR) 
	$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@

$(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR)
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

#######################################
# clean up
#######################################
#根据自己的系统选择
#windows
clean:
	del /s/q $(BUILD_DIR)
#linux
# clean:
# 	rm -rf $(BUILD_DIR)

download:
	openocd -f $(OPENOCD_DOWN_PATH) -f $(OPENOCD_CHIP_PATH)  -c init -c halt -c "flash write_image erase ./build/$(TARGET).hex" -c reset -c halt -c shutdown

#######################################
# dependencies
#######################################
-include $(wildcard $(BUILD_DIR)/*.d)

# *** EOF ***
