BUILD_DIR := build
HULK_DIR := hulk
BINARY := $(BUILD_DIR)/hulk_compiler
SOURCE := src/main.cpp
SCRIPT := script.hulk
LLVMOBJ := $(HULK_DIR)/output.ll
EXEC := $(HULK_DIR)/exec

all: compiler compile execute

compiler:
	@mkdir -p $(BUILD_DIR)
	@clang++-19 -g -O3 $(SOURCE) `llvm-config --cxxflags --ldflags --system-libs --libs core` -fexceptions -o $(BINARY)

compile: compiler
	@mkdir -p $(HULK_DIR)
	@./$(BINARY) $(SCRIPT)
	@echo
	@echo "\033[31m----- Compiled script.hulk and generated LLVM IR -----\033[0m"
	@clang++-19 $(LLVMOBJ) -o $(EXEC)
	@echo
	@echo "\033[35m----- Generated executable -----\033[0m"

compile_llvm:
	@clang++-19 $(LLVMOBJ) -o $(EXEC)
	@echo
	@echo "\033[35m----- Generated executable -----\033[0m"

execute: compile
	@echo
	@echo "\033[34m----- Executing executable  -----\033[0m"
	@echo
	@./$(EXEC)

clean:
	@rm -rf $(HULK_DIR)
	@echo "Removed $(HULK_DIR) directory"

.PHONY: all compiler compile execute clean