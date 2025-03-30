SCRIPT = script.hulk
BUILD_DIR = ./build
LEXER_DIR = src/lexer/lex_builder.l

ifeq ($(OS),Windows_NT)
    MKDIR = mkdir
    RMDIR = rmdir /s /q
    CHECK_DIR_CMD = if not exist "$(BUILD_DIR)" $(MKDIR) "$(BUILD_DIR)"
    CHECK_FILE_CMD = if not exist "$(SCRIPT)" (echo Error: '$(SCRIPT)' not found) else (echo '$(SCRIPT)' found)
	LEX_CMD = win_flex -o $(BUILD_DIR)/lex.yy.c $(LEXER_DIR)
else
    MKDIR = mkdir -p
    RMDIR = rm -rf
    CHECK_DIR_CMD = test -d "$(BUILD_DIR)" || $(MKDIR) "$(BUILD_DIR)"
    CHECK_FILE_CMD = test -f "$(SCRIPT)" || (echo "Error: '$(SCRIPT)' not found");
	LEX_CMD = flex -o $(BUILD_DIR)/lex.yy.c $(LEXER_DIR)
endif

.PHONY: build run clean

build:
	@$(CHECK_DIR_CMD) && echo "'$(BUILD_DIR)' directory ready"
	@$(CHECK_FILE_CMD)

run: build
	@echo "just run"

clean:
	@$(RMDIR) "$(BUILD_DIR)"
	@echo "'$(BUILD_DIR)' directory removed"

lex: build lex.yy.c
	@echo Compiling lexer 
	gcc -o $(BUILD_DIR)/lexer.exe $(BUILD_DIR)/lex.yy.c -lfl 
	$(BUILD_DIR)/lexer.exe

lex.yy.c: $(LEXER_DIR)
	@echo Generating lexer
	$(LEX_CMD)