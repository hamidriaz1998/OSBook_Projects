# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c99
DEBUG_FLAGS = -DDEBUG -g
RELEASE_FLAGS = -O2

# Directories
OBJ_DIR = obj
BIN_DIR = bin

# Target executable
TARGET = $(BIN_DIR)/shell

# Source files
SRCS = main.c shell.c
OBJS = $(SRCS:%.c=$(OBJ_DIR)/%.o)

# Header files
HEADERS = shell.h

# Default target
all: $(TARGET)

# Build the executable
$(TARGET): $(OBJS) | $(BIN_DIR)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET)

# Build with debug flags
debug: CFLAGS += $(DEBUG_FLAGS)
debug: clean $(TARGET)

# Build with release optimizations
release: CFLAGS += $(RELEASE_FLAGS)
release: clean $(TARGET)

# Compile source files to object files
$(OBJ_DIR)/%.o: %.c $(HEADERS) | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Create directories if they don't exist
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Clean build artifacts
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

# Remove all generated files including test files
distclean: clean
	rm -f *.txt a.out

# Run the shell
run: $(TARGET)
	./$(TARGET)

# Run with debug mode
run-debug: debug
	./$(TARGET)

# Rebuild everything
rebuild: clean all

# Install (optional - copies to /usr/local/bin)
install: release
	install -m 755 $(TARGET) /usr/local/bin/shell

# Uninstall
uninstall:
	rm -f /usr/local/bin/shell

# Show help
help:
	@echo "Available targets:"
	@echo "  all        - Build the shell (default)"
	@echo "  debug      - Build with debug flags (-DDEBUG -g)"
	@echo "  release    - Build with optimizations (-O2)"
	@echo "  clean      - Remove obj/ and bin/ directories"
	@echo "  distclean  - Remove all generated files"
	@echo "  run        - Build and run the shell"
	@echo "  run-debug  - Build with debug and run the shell"
	@echo "  rebuild    - Clean and rebuild"
	@echo "  install    - Install to /usr/local/bin (requires root)"
	@echo "  uninstall  - Remove from /usr/local/bin (requires root)"
	@echo "  help       - Show this help message"

.PHONY: all debug release clean distclean run run-debug rebuild install uninstall help
