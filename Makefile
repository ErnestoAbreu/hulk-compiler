BIN_DIR := hulk
BUILD_DIR := build
BINARY := $(BIN_DIR)/hulk_compiler
SOURCE := src/main.cpp
SCRIPT := script.hulk
LLVMOBJ := $(BUILD_DIR)/output.ll
EXEC := $(BIN_DIR)/exec

all: compiler compile execute

compiler:
	@mkdir -p $(BIN_DIR)
	@clang++-19 -g -O3 $(SOURCE) `llvm-config --cxxflags --ldflags --system-libs --libs core` -fexceptions -o $(BINARY)

compile: compiler
	@./$(BINARY) $(SCRIPT)

execute: compile
	@clang++-19 $(LLVMOBJ) -o $(EXEC)
	@echo
	@echo "\033[34m----- Compile and execute generated LLVM IR -----\033[0m"
	@echo
	@./$(EXEC)

clean:
	@rm -rf $(BIN_DIR)
	@echo "Removed $(BIN_DIR) directory"

.PHONY: all compiler compile execute clean