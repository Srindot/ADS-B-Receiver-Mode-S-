# ==============================================================================
# One-Shot Bare-Metal Makefile (STM32F446RE)
# ==============================================================================

# --- Toolchain ---
CC      = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy

# --- Directories ---
SRC_DIR   = src
INC_DIR   = inc
BUILD_DIR = build

# --- Build Configuration ---
# Renamed from blink_led to adsb_receiver
TARGET    = $(BUILD_DIR)/adsb_receiver

# --- Source & Linker Files ---
# Automatically find all .c files inside the src/ folder
C_SOURCES = $(wildcard $(SRC_DIR)/*.c)

# Assuming the assembly startup file and linker script are in the main project folder
ASM_SOURCES = startup_stm32f446xx.s
LDSCRIPT  = linker_script.ld

# --- Compiler & Linker Flags ---
CPU       = -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16
# Added -I$(INC_DIR) to tell the compiler where to look for .h files
CFLAGS    = $(CPU) -Wall -g -O0 -I$(INC_DIR)
LDFLAGS   = $(CPU) -T$(LDSCRIPT) -nostartfiles -Wl,-Map=$(TARGET).map

# ==============================================================================
# Build Rules
# ==============================================================================

all: $(TARGET).bin

# The One-Shot Command: Compile and link all sources directly into the .elf
$(TARGET).elf: $(C_SOURCES) $(ASM_SOURCES)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

# Extract raw binary for flashing
$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean
