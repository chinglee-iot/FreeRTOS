# Change according to what your unit test directory is.
$(info === Starting Build Process ===)
$(info Time: $(shell date))

include ../../makefile.in

.PRECIOUS:

PROJECT				:=   granular_lock
PROJECT_SRC			:=   tasks.c queue.c timers.c event_groups.c stream_buffer.c
PROJECT_SRC_PATHS	:= $(addprefix $(KERNEL_DIR)/,$(PROJECT_SRC))

# Test dependencies that should be excluded from coverage
TEST_DEPS_SRC    := list.c
TEST_DEPS_PATHS  := $(addprefix $(KERNEL_DIR)/,$(TEST_DEPS_SRC))

# Test/CMock/tasks
PROJECT_DIR			:=   $(abspath .)
PROJ_DIR			:=   $(abspath .)

# Test support files
TEST_SUPPORT_DIR := $(UT_ROOT_DIR)/smp
TEST_SUPPORT_SRC := smp_utest_common

# List the dependency files you wish to mock
MOCK_FILES_FP   +=  $(UT_ROOT_DIR)/config/fake_assert.h
MOCK_FILES_FP   +=  $(UT_ROOT_DIR)/config/fake_port.h
MOCK_FILES_FP   +=  $(UT_ROOT_DIR)/smp/granular_lock/portmacro.h
MOCK_FILES_FP   +=  $(KERNEL_DIR)/include/timers.h

# List special compilation flags for this module
CFLAGS           += -Wno-unused-function

# List special preprocessing flags for this module
CPPFLAGS         +=

# List special linking flags for this module
LDFLAGS          +=

# Coverage options
COVERAGE_OPTS    :=   -fprofile-arcs -ftest-coverage -fprofile-generate

# Directory definitions
PARENT_DIR       :=   $(notdir $(realpath $(dir $(PROJECT_DIR))))
SCRATCH_DIR      :=   $(GENERATED_DIR)/$(PARENT_DIR)/$(PROJECT)

# Mock file definitions
MOCK_FILES       :=   $(notdir $(MOCK_FILES_FP))
MOCK_SRC_LIST    :=   $(foreach disc,$(DISCRIMINATOR),$(addprefix $(SCRATCH_DIR)/$(disc)/mocks/mock_,$(MOCK_FILES:.h=.c)))

# Preprocessed file definitions
CPP_FILES        :=   $(foreach disc,$(DISCRIMINATOR),$(addprefix $(SCRATCH_DIR)/$(disc)/cpp/,$(notdir $(MOCK_FILES_FP))))
DEP_FILES        :=   $(foreach disc,$(DISCRIMINATOR),$(addsuffix .d,$(addprefix $(SCRATCH_DIR)/$(disc)/cpp/,$(notdir $(MOCK_FILES_FP)))))

# Test file definitions
PROJ_SRC_LIST    :=   $(sort $(wildcard $(PROJECT)_*_utest.c))
SUITE_UT_SRC     :=   $(subst $(PROJECT)_,,$(PROJ_SRC_LIST))
DISCRIMINATOR    :=   $(subst _utest,,$(subst .c,,$(subst $(PROJECT)_,,$(PROJ_SRC_LIST))))

# Directory paths
MOCK_DIRS        :=   $(foreach disc,$(DISCRIMINATOR),$(SCRATCH_DIR)/$(disc)/mocks)
CPP_DIRS         :=   $(foreach disc,$(DISCRIMINATOR),$(SCRATCH_DIR)/$(disc)/cpp)
INCLUDE_DIRS     :=   $(foreach disc,$(DISCRIMINATOR),$(SCRATCH_DIR)/$(disc)/include)
CONFIG_DIRS      :=   $(foreach disc,$(DISCRIMINATOR),$(SCRATCH_DIR)/$(disc))

# Debug Information
$(info === Project Configuration ===)
$(info PROJECT = $(PROJECT))
$(info PROJECT_DIR = $(PROJECT_DIR))
$(info KERNEL_DIR = $(KERNEL_DIR))
$(info === Source Files ===)
$(info PROJECT_SRC = $(PROJECT_SRC))
$(info PROJECT_SRC_PATHS = $(PROJECT_SRC_PATHS))
$(info === Test Dependencies ===)
$(info TEST_DEPS_SRC = $(TEST_DEPS_SRC))
$(info TEST_DEPS_PATHS = $(TEST_DEPS_PATHS))
$(info === Test Configuration ===)
$(info DISCRIMINATOR = $(DISCRIMINATOR))
$(info SUITE_UT_SRC = $(SUITE_UT_SRC))
$(info MOCK_FILES_FP = $(MOCK_FILES_FP))

# Handle empty mock files
ifeq ($(MOCK_FILES_FP),)
    $(info No mock files specified)
    $(shell mkdir -p  $(MOCK_DIRS))
    $(shell for n in $(MOCK_DIRS) ; do touch -a $(n)/mock_dummy.c)
endif

# Executable definitions
EXEC_LIST      := $(addprefix $(BIN_DIR)/smp_,$(patsubst %.c,%,$(PROJ_SRC_LIST)))

$(info === Build Targets ===)
$(info EXEC_LIST = $(EXEC_LIST))
$(info SCRATCH_DIR = $(SCRATCH_DIR))

.PHONY: all Makefile directories

.SECONDARY:
all : directories $(EXEC_LIST)

$(info === Defining Build Rules ===)

# Add rule for FreeRTOSConfig.h
define config-file-rule
$(info Defining config-file-rule for $(1))
$(SCRATCH_DIR)/$(1)/include/FreeRTOSConfig.h: $(PROJECT_DIR)/FreeRTOSConfig_$(1).h | directories
    @echo "=== Setting up FreeRTOSConfig.h for discriminator $(1) ==="
    cp $$< $$@
endef

# Generate rules for each discriminator and executable
define exec-rule
$(info Defining exec-rule for $(1))
$(BIN_DIR)/smp_$(PROJECT)_$(1)_utest: $(addprefix $(SCRATCH_DIR)/$(1)/mocks/mock_,$(MOCK_FILES:.h=.o)) \
                            $(SCRATCH_DIR)/$(1)/test_runner.o \
                            $(addprefix $(SCRATCH_DIR)/$(1)/,$(PROJECT_SRC:.c=.o)) \
                            $(addprefix $(SCRATCH_DIR)/$(1)/,$(TEST_DEPS_SRC:.c=.o)) \
                            $(SCRATCH_DIR)/$(1)/$(notdir $(TEST_SUPPORT_SRC)).o \
                            $(SCRATCH_DIR)/$(1)/$(PROJECT)_utest.o | libs directories
    @echo "=== Building executable for discriminator $(1) ==="
    @echo "Building $$@ with:"
    @echo "  Mocks: $$(filter %/mocks/mock_%,$$(^))"
    @echo "  Runner: $$(filter %/test_runner.o,$$(^))"
    @echo "  Sources: $$(filter $$(addprefix $(SCRATCH_DIR)/$(1)/,$$(PROJECT_SRC:.c=.o)),$$(^))"
    @echo "  Test Dependencies: $$(filter $$(addprefix $(SCRATCH_DIR)/$(1)/,$$(TEST_DEPS_SRC:.c=.o)),$$(^))"
    @echo "  Test Support: $$(filter %/$(notdir $(TEST_SUPPORT_SRC)).o,$$(^))"
    @echo "  Test: $$(filter %/$(PROJECT)_utest.o,$$(^))"
    $$(CC) $$^ $$(LDFLAGS) -o $$@
endef

# Generate rules for each source file and discriminator
define src-rule
$(info Defining src-rule for $(1) - $(2))
$(SCRATCH_DIR)/$(1)/$(2:.c=.i): $(KERNEL_DIR)/$(2) $(SCRATCH_DIR)/$(1)/include/FreeRTOSConfig.h | directories
    @echo "=== Preprocessing $(2) for discriminator $(1) ==="
    $$(eval INCLUDE = -I$(SCRATCH_DIR)/$(1)/include)
    $$(eval INCLUDE += -I$(SCRATCH_DIR)/$(1)/mocks)
    $$(eval INCLUDE += -I$(KERNEL_DIR)/include)
    $$(eval INCLUDE += -I$(PROJECT_DIR))
    $$(eval INCLUDE += -I$(UT_ROOT_DIR)/config)
    $$(eval INCLUDE += -I$(UT_ROOT_DIR)/smp/granular_lock)
    $$(eval INCLUDE += -I$(TEST_SUPPORT_DIR))
    $$(CC) -E $$< $$(INCLUDE) $$(CPPFLAGS) \
        -include stdint.h \
        -include FreeRTOSConfig.h \
        -include FreeRTOS.h \
        -include fake_port.h \
        -include portmacro.h \
        -include projdefs.h \
        -o $$@

$(SCRATCH_DIR)/$(1)/$(2:.c=.o): $(SCRATCH_DIR)/$(1)/$(2:.c=.i)
    @echo "=== Compiling $(2) for discriminator $(1) ==="
    $$(CC) -c $$< $$(CFLAGS) $$(COVERAGE_OPTS) -o $$@
endef

# Generate rules for test dependencies (without coverage)
define test-dep-rule
$(info Defining test-dep-rule for $(1) - $(2))
$(SCRATCH_DIR)/$(1)/$(2:.c=.i): $(KERNEL_DIR)/$(2) $(SCRATCH_DIR)/$(1)/include/FreeRTOSConfig.h | directories
    @echo "=== Preprocessing test dependency $(2) for discriminator $(1) ==="
    $$(eval INCLUDE = -I$(SCRATCH_DIR)/$(1)/include)
    $$(eval INCLUDE += -I$(SCRATCH_DIR)/$(1)/mocks)
    $$(eval INCLUDE += -I$(KERNEL_DIR)/include)
    $$(eval INCLUDE += -I$(PROJECT_DIR))
    $$(eval INCLUDE += -I$(UT_ROOT_DIR)/config)
    $$(eval INCLUDE += -I$(UT_ROOT_DIR)/smp/granular_lock)
    $$(eval INCLUDE += -I$(TEST_SUPPORT_DIR))
    $$(CC) -E $$< $$(INCLUDE) $$(CPPFLAGS) \
        -include stdint.h \
        -include FreeRTOSConfig.h \
        -include FreeRTOS.h \
        -include fake_port.h \
        -include portmacro.h \
        -include projdefs.h \
        -o $$@

$(SCRATCH_DIR)/$(1)/$(2:.c=.o): $(SCRATCH_DIR)/$(1)/$(2:.c=.i)
    @echo "=== Compiling test dependency $(2) for discriminator $(1) ==="
    $$(CC) -c $$< $$(CFLAGS) -o $$@
endef

# Add rule for test support files
define test-support-rule
$(info Defining test-support-rule for $(1) - $(notdir $(2)))
$(SCRATCH_DIR)/$(1)/$(2).o: $(TEST_SUPPORT_DIR)/$(2).c $(addprefix $(SCRATCH_DIR)/$(1)/mocks/mock_,$(MOCK_FILES:.h=.o)) | directories
    @echo "=== Compiling test support file $(2) for discriminator $(1) ==="
    $$(eval INCLUDE = -I$(SCRATCH_DIR)/$(1)/include)
    $$(eval INCLUDE += -I$(SCRATCH_DIR)/$(1)/mocks)
    $$(eval INCLUDE += -I$(KERNEL_DIR)/include)
    $$(eval INCLUDE += -I$(TEST_SUPPORT_DIR))
    $$(eval INCLUDE += -I$(UT_ROOT_DIR)/config)
    $$(eval INCLUDE += -I$(PROJECT_DIR))
    $$(eval INCLUDE += -I$(UT_ROOT_DIR)/smp/granular_lock)
    $$(CC) -c $$< $$(INCLUDE) $$(CPPFLAGS) $$(CFLAGS) -o $$@
endef

# Test runner compilation
define runner-rule
$(info Defining runner-rule for $(1))
$(SCRATCH_DIR)/$(1)/test_runner.o: $(SCRATCH_DIR)/$(1)/test_runner.c
    @echo "=== Compiling test runner for discriminator $(1) ==="
    $$(eval INCLUDE = -I$(SCRATCH_DIR)/$(1)/include)
    $$(eval INCLUDE += -I$(SCRATCH_DIR)/$(1)/mocks)
    $$(eval INCLUDE += -I$(KERNEL_DIR)/include)
    $$(eval INCLUDE += -I$(PROJECT_DIR))
    $$(eval INCLUDE += -I$(UT_ROOT_DIR)/config)
    $$(eval INCLUDE += -I$(UT_ROOT_DIR)/smp/granular_lock)
    $$(eval INCLUDE += -I$(TEST_SUPPORT_DIR))
    $$(eval INCLUDE += -I$(UNITY_SRC_DIR))
    $$(CC) -c $$< $$(INCLUDE) $$(CPPFLAGS) $$(CFLAGS) -o $$@

$(SCRATCH_DIR)/$(1)/test_runner.c: $(PROJECT_DIR)/$(PROJECT)_$(1)_utest.c Makefile | directories
    @echo "=== Generating test runner for discriminator $(1) ==="
    ruby $(UNITY_BIN_DIR)/generate_test_runner.rb "--use_param_tests=1" \
        $(PROJECT)_$(1)_utest.c $(PROJECT_DIR)/$(PROJECT).yml $$@
endef

# Mock generation rules for each discriminator
define mock-gen-rule
$(info Defining mock-gen-rule for $(1))
$(SCRATCH_DIR)/$(1)/mocks/mock_%.c: $(SCRATCH_DIR)/$(1)/cpp/%.h $(PROJECT_DIR)/$(PROJECT).yml | directories
    @echo "=== Generating mock for $$(notdir $$<) ==="
    @echo "Source header: $$<"
    @echo "Target mock: $$@"
    cd $(SCRATCH_DIR)/$(1) && \
        CMOCK_DIR=$(CMOCK_DIR) \
        UNITY_DIR=$(UNITY_DIR) \
        ruby $(CMOCK_EXEC_DIR)/cmock.rb \
        -o$(PROJECT_DIR)/$(PROJECT).yml \
        --includes=stdint.h,FreeRTOS.h \
        cpp/$$(notdir $$<)
endef

# Create explicit rules for header files
define header-rule
$(info Defining header-rule for $(1) - $(notdir $(2)))
$(SCRATCH_DIR)/$(1)/cpp/$(notdir $(2)): $(2) | directories
    @echo "=== Copying header file for discriminator $(1) ==="
    @echo "Source: $(2)"
    @echo "Target: $$@"
    cp $$< $$@
endef

$(info === Applying Rules ===)
$(info Processing discriminators: $(DISCRIMINATOR))

# First, apply config file rules
$(foreach disc,$(DISCRIMINATOR),$(eval $(call config-file-rule,$(disc))))
$(info Config files created)

# Copy header files
$(foreach disc,$(DISCRIMINATOR),\
    $(foreach mock_fp,$(MOCK_FILES_FP),\
        $(eval $(call header-rule,$(disc),$(mock_fp)))))
$(info Header files copied)

# Apply mock generation rules
$(foreach disc,$(DISCRIMINATOR),$(eval $(call mock-gen-rule,$(disc))))
$(info Mock generation rules applied)

# Mock object compilation
define mock-rule
$(info Defining mock-rule for $(1))
$(SCRATCH_DIR)/$(1)/mocks/mock_%.o: $(SCRATCH_DIR)/$(1)/mocks/mock_%.c
    @echo "=== Compiling mock for discriminator $(1): $$(notdir $$<) ==="
    $$(eval INCLUDE = -I$(SCRATCH_DIR)/$(1)/include)
    $$(eval INCLUDE += -I$(SCRATCH_DIR)/$(1)/mocks)
    $$(eval INCLUDE += -I$(KERNEL_DIR)/include)
    $$(eval INCLUDE += -I$(PROJECT_DIR))
    $$(eval INCLUDE += -I$(UT_ROOT_DIR)/smp/granular_lock)
    $$(CC) -c $$< $$(INCLUDE) $$(CPPFLAGS) $$(CFLAGS) -fPIC \
        -include stdint.h \
        -include FreeRTOS.h \
        -DUNITY_EXCLUDE_MATH_H \
        -DUNITY_EXCLUDE_STDINT_H \
        -DUNITY_OUTPUT_CHAR -o $$@
endef

$(foreach disc,$(DISCRIMINATOR),$(eval $(call mock-rule,$(disc))))
$(info Mock compilation rules applied)

# Then apply source rules
$(foreach disc,$(DISCRIMINATOR),$(foreach src,$(PROJECT_SRC),$(eval $(call src-rule,$(disc),$(src)))))
$(info Source rules applied)

# Apply test dependency rules
$(foreach disc,$(DISCRIMINATOR),$(foreach src,$(TEST_DEPS_SRC),$(eval $(call test-dep-rule,$(disc),$(src)))))
$(info Test dependency rules applied)

# Apply test support rules
$(foreach disc,$(DISCRIMINATOR),\
    $(eval $(call test-support-rule,$(disc),$(TEST_SUPPORT_SRC))))
$(info Test support rules applied)

# Then apply test runner rules
$(foreach disc,$(DISCRIMINATOR),$(eval $(call runner-rule,$(disc))))
$(info Test runner rules applied)

# Test file compilation
define test-rule
$(info Defining test-rule for $(1))
$(SCRATCH_DIR)/$(1)/$(PROJECT)_utest.o: $(PROJECT_DIR)/$(PROJECT)_$(1)_utest.c \
                                     $(addprefix $(SCRATCH_DIR)/$(1)/mocks/mock_,$(MOCK_FILES:.h=.o)) \
                                     $(SCRATCH_DIR)/$(1)/$(notdir $(TEST_SUPPORT_SRC)).o \
                                     $(SCRATCH_DIR)/$(1)/test_runner.o \
                                     $(UT_ROOT_DIR)/smp/global_vars.h | directories
    @echo "=== Compiling test file for discriminator $(1) ==="
    $$(eval INCLUDE = -I$(SCRATCH_DIR)/$(1)/include)
    $$(eval INCLUDE += -I$(SCRATCH_DIR)/$(1)/mocks)
    $$(eval INCLUDE += -I$(KERNEL_DIR)/include)
    $$(eval INCLUDE += -I$(PROJECT_DIR))
    $$(eval INCLUDE += -I$(UT_ROOT_DIR)/config)
    $$(eval INCLUDE += -I$(UT_ROOT_DIR)/smp/granular_lock)
    $$(eval INCLUDE += -I$(TEST_SUPPORT_DIR))
    $$(eval INCLUDE += -I$(UNITY_SRC_DIR))
    $$(CC) -c $$< $$(INCLUDE) $$(CPPFLAGS) $$(CFLAGS) -o $$@
endef

$(foreach disc,$(DISCRIMINATOR),$(eval $(call test-rule,$(disc))))
$(info Test compilation rules applied)

# Finally apply exec rules
$(foreach disc,$(DISCRIMINATOR),$(eval $(call exec-rule,$(disc))))
$(info Exec rules applied)

# Directory creation
directories:
    @echo "=== Creating build directories ==="
    -mkdir -p $(SCRATCH_DIR)
    -mkdir -p $(CONFIG_DIRS)
    -mkdir -p $(MOCK_DIRS)
    -mkdir -p $(CPP_DIRS)
    -mkdir -p $(INCLUDE_DIRS)
    -mkdir -p $(BIN_DIR)
    -mkdir -p $(LIB_DIR)
    @echo "Directories created:"
    @echo "  SCRATCH_DIR: $(SCRATCH_DIR)"
    @echo "  CONFIG_DIRS: $(CONFIG_DIRS)"
    @echo "  MOCK_DIRS: $(MOCK_DIRS)"
    @echo "  CPP_DIRS: $(CPP_DIRS)"
    @echo "  INCLUDE_DIRS: $(INCLUDE_DIRS)"
    @echo "  BIN_DIR: $(BIN_DIR)"
    @echo "  LIB_DIR: $(LIB_DIR)"

# Verification targets
.PHONY: show-build show-sources show-paths

show-build:
    @echo "=== Build Configuration ==="
    @echo "Project: $(PROJECT)"
    @echo "Source Files:"
    @for src in $(PROJECT_SRC); do echo "  $$src"; done
    @echo "Test Dependencies:"
    @for dep in $(TEST_DEPS_SRC); do echo "  $$dep"; done
    @echo "Test Files:"
    @for test in $(SUITE_UT_SRC); do echo "  $$test"; done
    @echo "Final Executables:"
    @for exec in $(EXEC_LIST); do echo "  $$exec"; done

show-sources:
    @echo "=== Source Files to Process ==="
    @for src in $(PROJECT_SRC); do \
        echo "Source: $(KERNEL_DIR)/$$src"; \
        for disc in $(DISCRIMINATOR); do \
            echo "  Disc: $$disc - Preprocessed: $(SCRATCH_DIR)/$$disc/$${src%.c}.i"; \
            echo "  Disc: $$disc - Object: $(SCRATCH_DIR)/$$disc/$${src%.c}.o"; \
        done; \
    done
    @echo "=== Test Dependencies to Process ==="
    @for dep in $(TEST_DEPS_SRC); do \
        echo "Dependency: $(KERNEL_DIR)/$$dep"; \
        for disc in $(DISCRIMINATOR); do \
            echo "  Disc: $$disc - Preprocessed: $(SCRATCH_DIR)/$$disc/$${dep%.c}.i"; \
            echo "  Disc: $$disc - Object: $(SCRATCH_DIR)/$$disc/$${dep%.c}.o"; \
        done; \
    done

show-paths:
    @echo "=== Directory Paths ==="
    @echo "KERNEL_DIR: $(KERNEL_DIR)"
    @echo "PROJECT_DIR: $(PROJECT_DIR)"
    @echo "SCRATCH_DIR: $(SCRATCH_DIR)"
    @echo "MOCK_DIRS: $(MOCK_DIRS)"
    @echo "CPP_DIRS: $(CPP_DIRS)"
    @echo "INCLUDE_DIRS: $(INCLUDE_DIRS)"

$(info === Build Process Setup Complete ===)
$(info Time: $(shell date))

EXEC_PREFIX     := smp_$(PROJECT)
$(info EXEC_PREFIX: $(EXEC_PREFIX))

include ../../coverage.mk