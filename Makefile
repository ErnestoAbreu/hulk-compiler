ifeq ($(OS),Windows_NT)
    MKDIR = mkdir
    RMDIR = rmdir /s /q
    CHECK_DIR_CMD = if not exist "$(BUILD_DIR)" $(MKDIR) "$(BUILD_DIR)"
    CHECK_FILE_CMD = if not exist "$(SCRIPT)" (echo Error: '$(SCRIPT)' not found && exit 1) else (echo '$(SCRIPT)' found)
else
    MKDIR = mkdir -p
    RMDIR = rm -rf
    CHECK_DIR_CMD = test -d "$(BUILD_DIR)" || $(MKDIR) "$(BUILD_DIR)"
    CHECK_FILE_CMD = test -f "$(SCRIPT)" || (echo "Error: '$(SCRIPT)' not found" && exit 1);
endif

SCRIPT = script.hulk
BUILD_DIR = ./build

.PHONY: build run clean

build:
	@$(CHECK_DIR_CMD) && echo "'$(BUILD_DIR)' directory ready"
	@$(CHECK_FILE_CMD)

run: build
	@echo "just run"

clean:
	@$(RMDIR) "$(BUILD_DIR)"
	@echo "'$(BUILD_DIR)' directory removed"