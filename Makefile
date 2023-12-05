# Paths and filenames
BINARY              := sensor_node
BUILD_DIR           := build
LIB_DIR             := lib
SRC_DIR             := src
NEWLIB_SYSCALLS_DIR := newlib_syscalls
OPENCM3_DIR         := submodules/libopencm3
BME280_DIR          := submodules/BME280_driver
LDSCRIPT            := stm32f103c8t6.ld

# Toolchain
TOOLCHAIN_PREFIX := arm-none-eabi
CC               := $(TOOLCHAIN_PREFIX)-gcc
CXX              := $(TOOLCHAIN_PREFIX)-g++
LD               := $(TOOLCHAIN_PREFIX)-g++
AR               := $(TOOLCHAIN_PREFIX)-ar
AS               := $(TOOLCHAIN_PREFIX)-as
OBJCOPY          := $(TOOLCHAIN_PREFIX)-objcopy
SIZE             := $(TOOLCHAIN_PREFIX)-size
OBJDUMP          := $(TOOLCHAIN_PREFIX)-objdump
GDB	             := $(TOOLCHAIN_PREFIX)-gdb
STFLASH          := $(shell which st-flash)

# Compiler & linker flags
STDLIB_INCLUDE   := $(shell $(CXX) -xc++ /dev/null -E -Wp,-v 2>&1 | sed -n 's,^ ,,p' | xargs -I{} echo -I{})
FP_FLAGS         := -msoft-float
ARCH_FLAGS       := -mthumb -mcpu=cortex-m3 -mfix-cortex-m3-ldrd $(FP_FLAGS)
OPT_FLAGS        := -Os -fno-exceptions -ffunction-sections -fdata-sections
WFLAGS           := -Wall -Wextra -Werror
DEFINES          := -DSTM32F1
I_FLAGS          := -I$(OPENCM3_DIR)/include -I$(BME280_DIR) -Isrc $(STDLIB_INCLUDE)

# C++ compiler flags
TGT_CXXFLAGS += $(OPT_FLAGS)
TGT_CXXFLAGS += -fanalyzer
TGT_CXXFLAGS += -fno-use-cxa-atexit -fno-rtti
TGT_CXXFLAGS += $(WFLAGS)
TGT_CXXFLAGS += $(ARCH_FLAGS)
TGT_CXXFLAGS += $(DEFINES)
TGT_CXXFLAGS += -std=c++20
TGT_CXXFLAGS += $(I_FLAGS)
TGT_CXXFLAGS += -MD
TGT_CXXFLAGS += -D WIFI_AP=$(WIFI_AP) -D WIFI_PASS=$(WIFI_PASS) -D SERVER_IP=$(SERVER_IP) -D SERVER_PORT=$(SERVER_PORT)

# C compiler flags for libopencm3
TGT_CFLAGS := $(OPT_FLAGS)
TGT_CFLAGS += $(WFLAGS)
TGT_CFLAGS += $(ARCH_FLAGS)
TGT_CFLAGS += $(DEFINES)

# Linker flags
TGT_LDFLAGS	 += -static -nostartfiles
TGT_LDFLAGS	 += -T$(LDSCRIPT)
TGT_LDFLAGS	 += $(ARCH_FLAGS)
TGT_LDFLAGS	 += -Wl,-Map=$(BUILD_DIR)/$(*).map
TGT_LDFLAGS  += -Wl,--gc-sections
LDLIBS       += -L$(LIB_DIR) -L$(OPENCM3_DIR)/lib
LDLIBS       += -lopencm3_stm32f1 -lbme280
LDLIBS       += -specs=nosys.specs

# Pass C flags to libopencm3 makefiles
CFLAGS += $(TGT_CFLAGS)
export FP_FLAGS
export CFLAGS

# Source files -> Object files
SRC_FILES := $(shell find $(SRC_DIR) -name '*.cpp')
OBJS      := $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SRC_FILES))
DEPENDS   := $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.d, $(SRC_FILES))

# Makefile debug
echo-stdlib-include:
	@echo "STDLIB_INCLUDE: $(STDLIB_INCLUDE)"

echo-src:
	@echo "SRC_FILES: $(SRC_FILES)"

echo-objs:
	@echo "OBJS: $(OBJS)"

echo-flags:
	@echo "CXX: $(TGT_CXXFLAGS)"
	@echo "LD: $(TGT_LDFLAGS)"

# Output directories
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(LIB_DIR):
	mkdir -p $(LIB_DIR)

# libopencm3
clean_libopencm3:
	rm -f submodules/libopencm3/lib/libopencm3_stm32f1.a
	-$(MAKE) -$(MAKEFLAGS) -C submodules/libopencm3 clean

submodules/libopencm3/lib/libopencm3_stm32f1.a:
	$(MAKE) -C submodules/libopencm3 TARGETS=stm32/f1

libopencm3: submodules/libopencm3/lib/libopencm3_stm32f1.a

# sensor node
elf: $(BUILD_DIR)/$(BINARY).elf

bin: $(BUILD_DIR)/$(BINARY).bin

$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf
	$(OBJCOPY) -Obinary $< $@

$(BUILD_DIR)/%.elf: libbme280 libopencm3 $(BUILD_DIR) $(OBJS)
	$(LD) $(TGT_LDFLAGS) $(OBJS) $(LDLIBS) -o $@
	$(SIZE) $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@echo "CXX $<"
	@$(CXX) $(TGT_CXXFLAGS) -o $@ -c $<

# Bosch BME280 driver
libbme280: $(LIB_DIR)/libbme280.a

$(LIB_DIR)/libbme280.a : $(LIB_DIR)/bme280.o
	$(AR) rcs $@ $<

$(LIB_DIR)/bme280.o: $(BME280_DIR)/bme280.c $(LIB_DIR)
	@echo "CXX $<"
	@$(CXX) $(TGT_CXXFLAGS) -Wno-missing-field-initializers -o $@ -c $< # Bosch driver gives missing-field-initializers warning

# Style checks
style-check:
	clang-format --Werror --dry-run src/**

style-fix:
	clang-format --Werror -i src/**

clang-tidy:
	clang-tidy -p compile_commands.json src/**

# Clean
clean:
	rm -rf $(BUILD_DIR)
	rm -rf $(LIB_DIR)

clean_lib:
	rm -rf $(LIB_DIR)

clean_all: clean clean_lib clean_libopencm3

# Flash
flash: $(BUILD_DIR)/$(BINARY).bin
	$(STFLASH) --connect-under-reset $(FLASHSIZE) write $< 0x8000000

# Disassemble
disassemble: $(BUILD_DIR)/$(BINARY).dump

$(BUILD_DIR)/$(BINARY).dump: $(BUILD_DIR)/$(BINARY).elf
	$(OBJDUMP) --wide --syms --source --line-numbers $< >$@

# Default target
default: bin
.DEFAULT_GOAL := default

-include $(DEPENDS)
-include .secrets
