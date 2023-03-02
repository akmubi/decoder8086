CC := clang
APP_NAME := main.out
BUILD_DIR := build
CFLAGS := -Wall -Wextra -Werror -Wdeclaration-after-statement -g
SRCS := $(shell find -name "*.c" -printf "%P\n")
OBJS := $(SRCS:%=%.o)
OBJS := $(addprefix $(BUILD_DIR)/,$(OBJS))

.PHONY: all target compile run clean

all: compile

compile: clean target

target: $(BUILD_DIR)/$(APP_NAME)

$(BUILD_DIR)/$(APP_NAME): $(OBJS)
	$(CC) $^ -o $@

run: $(BUILD_DIR)/$(APP_NAME)
	@$<

$(BUILD_DIR)/%.c.o: %.c
	@mkdir -p $(shell dirname $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	@echo "Removing build files...."
	@rm -rf $(BUILD_DIR)/
	@echo "Done!"

