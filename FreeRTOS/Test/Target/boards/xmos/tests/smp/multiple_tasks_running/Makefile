PROJECT_NAME := multiple_tasks_running
ROOT_TEST := $(ROOT_XMOS)/../../tests/smp/$(PROJECT_NAME)
TEST_BOARD_DIR := $(ROOT_XMOS)/tests/smp/$(PROJECT_NAME)

include ../../../xmos.mk

INCLUDE_DIRS := $(ROOT_TEST) $(DIRS_INC_COMMON)

SOURCES_TEST_CASE := $(ROOT_TEST)/$(PROJECT_NAME).c \
                    $(TEST_BOARD_DIR)/$(PROJECT_NAME)_test_runner.c

FLAGS_TEST_CASE := $(addprefix -I,$(INCLUDE_DIRS))
FLAGS_ALL := $(FLAGS_COMMON) $(FLAGS_TEST_CASE)
SOURCES_ALL := $(SOURCES_TEST_CASE) $(SOURCES_COMMON)

BUILD_DIR ?= build
OUT_DIR ?= bin

OBJS := $(addprefix $(BUILD_DIR)/,$(notdir $(addsuffix .o,$(SOURCES_ALL))))
$(info $$OBJS is [${OBJS}])

DIRS_ALL := $(ROOT_DIRS_COMMON) $(ROOT_TEST) $(TEST_BOARD_DIR)

vpath %.c $(DIRS_ALL)
vpath %.xc $(DIRS_ALL)
vpath %.S $(DIRS_ALL)

.PHONY: all clean run

all: $(OUT_DIR)/$(PROJECT_NAME).xe
	
clean:
	$(RM) -r $(OUT_DIR) $(BUILD_DIR)

$(BUILD_DIR)/%.o: %
	@mkdir -p $(@D)
	@echo "$(XCLANG) -c -MT"$@" -MMD -MP -MF"$(patsubst %.o,%.d,$@)" -MT"$(patsubst %.o,%.d,$@)" -o $@ $< $(FLAGS_ALL)"
	$(XCLANG) -c -MT"$@" -MMD -MP -MF"$(patsubst %.o,%.d,$@)" -MT"$(patsubst %.o,%.d,$@)" -o $@ $< $(FLAGS_ALL)

$(OUT_DIR)/$(PROJECT_NAME).xe: $(OBJS)
	@mkdir -p $(@D)
	$(XCLANG) -o $@ $^ $(FLAGS_ALL)

run: $(OUT_DIR)/$(PROJECT_NAME).xe
	xrun --xscope $<