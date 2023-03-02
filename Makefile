CC        := clang
APP_NAME  := main.out
BUILD_DIR := build

.PHONY: all target compile clean

all: compile

compile: clean target

target: $(BUILD_DIR)/$(APP_NAME)

$(BUILD_DIR)/$(APP_NAME): $(BUILD_DIR)/main.o
	$(CC) $^ -o $@

$(BUILD_DIR)/main.o: main.c
	@mkdir -p $(BUILD_DIR)
	$(CC) -Wall -Wextra -Werror -g -c $< -o $@

clean:
	@echo "Removing build files...."
	@rm -rf $(BUILD_DIR)/
	@echo "Done!"

