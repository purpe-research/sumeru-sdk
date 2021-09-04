TOOL_PREFIX = riscv32-unknown-elf-
ARCH = -march=rv32im -mabi=ilp32 

GCC = ${TOOL_PREFIX}gcc
CXX = ${TOOL_PREFIX}gcc
CC = ${TOOL_PREFIX}gcc
AS = ${TOOL_PREFIX}as
AR = ${TOOL_PREFIX}ar
LD = ${TOOL_PREFIX}ld
OBJDUMP = ${TOOL_PREFIX}objdump
OBJCOPY = ${TOOL_PREFIX}objcopy
RANLIB = ${TOOL_PREFIX}ranlib

# __SUMERU is now set by gcc automatically
#CFLAGS += --D__SUMERU

CFLAGS += -Wall -O2 
CFLAGS += -ffunction-sections -fdata-sections -mcmodel=medlow
CFLAGS += ${ARCH}

CXXFLAGS += ${CFLAGS}
