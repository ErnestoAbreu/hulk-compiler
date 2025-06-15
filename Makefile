BIN_DIR := hulk
BINARY := $(BIN_DIR)/hulk_compiler
SOURCE := src/main.cpp
SCRIPT := script.hulk

all: compile execute

compile:
	@mkdir -p $(BIN_DIR)
	@clang++-19 -g -O3 $(SOURCE) `llvm-config --cxxflags --ldflags --system-libs --libs core` -fexceptions -o $(BINARY)
	@echo "Compilation complete."

execute: compile
	@./$(BINARY) $(SCRIPT)

clean:
	@rm -rf $(BIN_DIR)
	@echo "Removed $(BIN_DIR) directory"

.PHONY: all compile execute clean