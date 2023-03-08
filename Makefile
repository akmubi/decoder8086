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

#
# TESTING
#

# Example: tests/0001.asm ==> tests/tmp/0001.asm.out
TEST_DIR     := tests
TEST_ASM     := $(wildcard ${TEST_DIR}/*.asm)
TEST_ASM_TMP := $(subst ${TEST_DIR}/,${TEST_DIR}/tmp/,${TEST_ASM})

# tests/0001.asm
TEST_ASM_ORIG_SRC := $(TEST_ASM)
# tests/tmp/0001.asm.out
TEST_ASM_ORIG_OBJ := $(addsuffix .out,${TEST_ASM_TMP})
# tests/tmp/0001.asm.gen
TEST_ASM_GEN_SRC  := $(addsuffix .gen,${TEST_ASM_TMP})
# tests/tmp/0001.asm.gen.out
TEST_ASM_GEN_OBJ  := $(addsuffix .gen.out,${TEST_ASM_TMP})

.PHONY: test test_tmp_dir util_rm_file

test: test_tmp_dir cmp_files.out rm_file.out $(TEST_ASM_GEN_OBJ)
	@./rm_file.out tests/tmp
	@./rm_file.out cmp_files.out
	@./rm_file.out rm_file.out

test_tmp_dir:
	@-mkdir $(TEST_DIR)/tmp

rm_file.out: $(TEST_DIR)/rm_file.c
	@$(CC) $< -o $@

cmp_files.out: $(TEST_DIR)/cmp_files.c
	@$(CC) $< -o $@

# tests/0001.asm ==> tests/tmp/0001.asm.out
$(TEST_ASM_ORIG_OBJ): $(TEST_DIR)/tmp/%.asm.out: $(TEST_DIR)/%.asm
	@nasm $< -o $@

# tests/tmp/0001.asm.out ==> tests/tmp/0001.asm.gen
$(TEST_ASM_GEN_SRC): %.gen: %.out $(BUILD_DIR)/$(APP_NAME)
	@$(BUILD_DIR)/$(APP_NAME) $< > $@

# tests/tmp/0001.asm.gen ==> tests/tmp/0001.asm.gen.out
$(TEST_ASM_GEN_OBJ): %.gen.out: %.gen cmp_files.out
	@nasm $< -o $@
	@echo -n "[Testing '$*'] "
	@-./cmp_files.out $*.out $*.gen.out || true
	@./rm_file.out $*.out
	@./rm_file.out $*.gen
	@./rm_file.out $*.gen.out

