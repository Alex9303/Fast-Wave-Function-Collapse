# Compiler and flags
CC = gcc
CFLAGS = -O3 -march=native

# Directories
SRC_DIR = src
OBJ_DIR = build

# Source files
SRCS = $(SRC_DIR)/wfc.c $(SRC_DIR)/json_utils.c $(SRC_DIR)/cjson/cJSON.c
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

# Output binary
TARGET = wfc

# Default target
all: $(TARGET)

# Build target
$(TARGET): $(OBJS)
        $(CC) $(CFLAGS) $^ -o $@

# Compile .c to .o
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
        @mkdir -p $(dir $@)
        $(CC) $(CFLAGS) -c $< -o $@

# Clean target
clean:
        rm -rf $(OBJ_DIR) $(TARGET)

# Phony targets
.PHONY: all clean
