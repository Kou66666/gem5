CC = riscv64-unknown-elf-gcc
CFLAGS = -O2 -g
TARGET = matmul
SRC = matmul.c matmul.S

all: clean $(TARGET) dump

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $^

# Add a target to generate the disassembly
DUMP = $(TARGET).dump

# Update the dump target to include source code in the disassembly
dump:
	riscv64-unknown-elf-objdump -d -S $(TARGET) > $(DUMP)

clean:
	rm -f $(TARGET) $(TARGET)_x86 $(DUMP)

# x86 build target
CC_X86 = gcc
TARGET_X86 = matmul_x86
SRC_X86 = matmul.c x86_asm_stub.c

x86: clean_x86 $(TARGET_X86)

$(TARGET_X86): $(SRC_X86)
	$(CC_X86) $(CFLAGS) -o $@ $^

clean_x86:
	rm -f $(TARGET_X86)

.PHONY: x86 clean_x86
