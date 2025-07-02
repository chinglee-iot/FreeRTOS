# Test case "Schedule Highest Priority".
include ../../../xmos.mk

PROJECT_NAME = only_one_task_enter_critical

TEST_DIR = $(ROOT_XMOS)/../../tests/smp/only_one_task_enter_critical
TEST_BOARD_DIR = $(ROOT_XMOS)/tests/smp/only_one_task_enter_critical

INCLUDE_DIRS = $(TEST_DIR) $(DIRS_INC_COMMON)

SOURCES_TEST_CASE = $(TEST_DIR)/only_one_task_enter_critical.c \
                    $(TEST_BOARD_DIR)/only_one_task_enter_critical_test_runner.c

FLAGS_TEST_CASE = $(addprefix -I,$(INCLUDE_DIRS))

FLAGS_ALL = $(FLAGS_COMMON) $(FLAGS_TEST_CASE)

SOURCES_ALL = $(SOURCES_TEST_CASE) $(SOURCES_COMMON)

OBJS = $(addprefix $(BUILD_DIR)/,$(notdir $(addsuffix .o,$(SOURCES_ALL))))
$(info $$OBJS is [${OBJS}])

DIRS_ALL = $(ROOT_DIRS_COMMON) $(TEST_DIR) $(TEST_BOARD_DIR)

vpath %.c $(DIRS_ALL)
vpath %.xc $(DIRS_ALL)
vpath %.S $(DIRS_ALL)

.PHONY: all clean run

all: $(OUT_DIR)/$(PROJECT_NAME).xe
	
clean:
	$(RM) -r $(OUT_DIR) $(BUILD_DIR)

$(BUILD_DIR)/%.o: %
	@"mkdir" -p $(@D)
	@echo "$(XCLANG) -c -MT"$@" -MMD -MP -MF"$(patsubst %.o,%.d,$@)" -MT"$(patsubst %.o,%.d,$@)" -o $@ $< $(FLAGS_ALL)"
	$(XCLANG) -c -MT"$@" -MMD -MP -MF"$(patsubst %.o,%.d,$@)" -MT"$(patsubst %.o,%.d,$@)" -o $@ $< $(FLAGS_ALL)

$(OUT_DIR)/$(PROJECT_NAME).xe: $(OBJS)
	@"mkdir" -p $(@D)
	$(XCLANG) -o $@ $^ $(FLAGS_ALL)

run: $(OUT_DIR)/$(PROJECT_NAME).xe
	xrun --xscope $(OUT_DIR)/$(PROJECT_NAME).xe

