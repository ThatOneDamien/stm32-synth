################################################################################
# Project
################################################################################

PROJ_NAME := stm32-learning

################################################################################
# Directories
################################################################################

SRC_DIR     := src
BUILD_DIR   := build
CMSIS_DIR   := $(STM_COMMON)/Drivers/CMSIS

################################################################################
# Toolchain
################################################################################

CC      := arm-none-eabi-gcc
OBJCOPY := arm-none-eabi-objcopy
SIZE    := arm-none-eabi-size

################################################################################
# MCU
################################################################################

CPUFLAGS := \
    -mcpu=cortex-m4 \
    -mthumb \
    -mfloat-abi=hard \
    -mfpu=fpv4-sp-d16

################################################################################
# Sources
################################################################################

C_SRCS := $(wildcard $(SRC_DIR)/*.c)

# Startup assembly (should be startup_stm32f446xx.s)
S_SRCS := $(wildcard $(SRC_DIR)/*.s)

################################################################################
# Objects
################################################################################

C_OBJS := $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(C_SRCS))
S_OBJS := $(patsubst $(SRC_DIR)/%.s, $(BUILD_DIR)/%.o, $(S_SRCS))

OBJS := $(C_OBJS) $(S_OBJS)
DEPS := $(OBJS:.o=.d)

################################################################################
# Includes
################################################################################

INCLUDES := \
    -I$(SRC_DIR) \
    -I$(CMSIS_DIR)/Core/Include \
    -I$(CMSIS_DIR)/Device/ST/STM32F4xx/Include

################################################################################
# Compiler
################################################################################

CFLAGS := \
    $(CPUFLAGS) \
    -DSTM32F446xx \
    $(INCLUDES) \
    -Og \
    -g3 \
    -Wall \
    -Wextra \
    -Wpedantic \
    -ffreestanding \
    -ffunction-sections \
    -fdata-sections \
	-nostdlib \
    -MMD \
    -MP

ASFLAGS := \
    $(CPUFLAGS)

################################################################################
# Linker
################################################################################

LDFLAGS := \
    $(CPUFLAGS) \
    -T linker.ld \
    -Wl,--gc-sections \
    -Wl,-Map=$(BUILD_DIR)/$(PROJ_NAME).map \
	-nostdlib \

################################################################################
# Output
################################################################################

ELF := $(BUILD_DIR)/$(PROJ_NAME).elf
BIN := $(BUILD_DIR)/$(PROJ_NAME).bin
HEX := $(BUILD_DIR)/$(PROJ_NAME).hex

################################################################################
# Rules
################################################################################

.PHONY: all clean flash chill

all: $(ELF) $(BIN) $(HEX)

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

#
# Compile C
#
$(C_OBJS): $(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	@echo "Compiling $<"
	@$(CC) $(CFLAGS) -c $< -o $@

#
# Assembly
#
$(S_OBJS): $(BUILD_DIR)/%.o: $(SRC_DIR)/%.s | $(BUILD_DIR)
	@echo "Assembling $<"
	@$(CC) $(ASFLAGS) -c $< -o $@

#
# Link
#
$(ELF): $(OBJS)
	@echo "Linking $@"
	@$(CC) $^ $(LDFLAGS) -o $@
	@$(SIZE) $@

#
# Binary
#
$(BIN): $(ELF)
	@echo "Generating $@"
	@$(OBJCOPY) -O binary $< $@

#
# Intel HEX
#
$(HEX): $(ELF)
	@echo "Generating $@"
	@$(OBJCOPY) -O ihex $< $@

#
# Flash
#
flash: $(BIN)
	@echo "Flashing $<"
	@st-flash write $< 0x08000000

#
# Chill mode
#
chill: 
	@echo "Chilling out..."
	@st-flash write chill.bin 0x08000000

#
# Cleanup
#
clean:
	rm -rf $(BUILD_DIR)

################################################################################
# Auto-generated dependency files
################################################################################

-include $(DEPS)
