# Compiler and flags
CC = gcc
CFLAGS = -O3 -march=native

# Source files
SRCS = wfc.c json_utils.c cjson/cJSON.c

# Output binary
TARGET = wfc

# Default target
all: $(TARGET)

# Build target
$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $^ -o $@

# Clean target
clean:
	rm -f $(TARGET)

# Phony targets
.PHONY: all clean
