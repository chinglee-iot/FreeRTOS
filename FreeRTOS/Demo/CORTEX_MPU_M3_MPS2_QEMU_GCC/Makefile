CC                     =    arm-none-eabi-gcc
BIN                   :=    RTOSDemo.axf

BUILD_DIR             :=    build

FREERTOS_DIR_REL      :=    ../../../FreeRTOS
FREERTOS_DIR          :=    $(abspath $(FREERTOS_DIR_REL))
KERNEL_DIR            :=    $(FREERTOS_DIR)/Source

# Startup sources
SOURCE_FILES          +=    init/startup.c  syscall.c main.c

# platform portable source file
SOURCE_FILES          +=    $(KERNEL_DIR)/portable/GCC/ARM_CM3_MPU/port.c
SOURCE_FILES          +=    $(KERNEL_DIR)/portable/GCC/ARM_CM3_MPU/mpu_wrappers_v2_asm.c

# Kernel source files
SOURCE_FILES          +=    $(KERNEL_DIR)/portable/Common/mpu_wrappers.c
SOURCE_FILES          +=    $(KERNEL_DIR)/portable/Common/mpu_wrappers_v2.c
SOURCE_FILES          +=    $(KERNEL_DIR)/tasks.c
SOURCE_FILES          +=    $(KERNEL_DIR)/list.c
SOURCE_FILES          +=    $(KERNEL_DIR)/queue.c
SOURCE_FILES          +=    $(KERNEL_DIR)/timers.c
SOURCE_FILES          +=    $(KERNEL_DIR)/event_groups.c
SOURCE_FILES          +=    ${KERNEL_DIR}/portable/MemMang/heap_4.c
SOURCE_FILES          +=    $(KERNEL_DIR)/stream_buffer.c

# application source files
SOURCE_FILES          +=    app_main.c
SOURCE_FILES          +=    mpu_demo.c

INCLUDE_DIRS          +=    -I$(FREERTOS_DIR)/Demo/CORTEX_MPU_M3_MPS2_QEMU_GCC
INCLUDE_DIRS          +=    -I$(FREERTOS_DIR)/Demo/CORTEX_MPU_M3_MPS2_QEMU_GCC/CMSIS
INCLUDE_DIRS          +=    -I$(KERNEL_DIR)/include
INCLUDE_DIRS          +=    -I$(KERNEL_DIR)/portable/GCC/ARM_CM3_MPU

DEFINES               :=    -DQEMU_SOC_MPS2
DEFINES               :=    -DHEAP4

CPPFLAGS              +=    $(DEFINES)

CFLAGS                +=    -mcpu=cortex-m3
CFLAGS                +=    -Wno-error=implicit-function-declaration
CFLAGS                +=    -Wno-builtin-declaration-mismatch
CFLAGS                +=    -Werror
CFLAGS                +=    -Wall
CFLAGS                +=    -Wextra
CFLAGS                +=    -fstrict-aliasing
CFLAGS                +=    -Wstrict-aliasing
CFLAGS                +=    -Wno-error=address-of-packed-member
CFLAGS                +=    -Wno-unused-parameter
CFLAGS                +=    $(INCLUDE_DIRS)

LDFLAGS                =    -T ./scripts/mps2_m3.ld
LDFLAGS               +=    -mthumb

LDFLAGS               +=    -Xlinker -Map=${BUILD_DIR}/output.map
LDFLAGS               +=    -nostartfiles -nostdlib -nolibc -nodefaultlibs

ifeq ($(DEBUG), 1)
    CFLAGS                +=     -ggdb3 -Og -save-temps=obj
else
    CFLAGS                +=     -O3
endif

ifeq ($(PICOLIBC), 1)
    CFLAGS                +=     --specs=picolibc.specs -DPICOLIBC_INTEGER_PRINTF_SCANF
   LDFLAGS                +=     --specs=picolibc.specs -DPICOLIBC_INTEGER_PRINTF_SCANF
endif

OBJ_FILES             :=    $(SOURCE_FILES:%.c=$(BUILD_DIR)/%.o)

.PHONY: clean

$(BUILD_DIR)/$(BIN) : $(OBJ_FILES)
	$(CC) -ffunction-sections -fdata-sections $(LDFLAGS) $+ -o $(@)

%.d: %.c
	@set -e; rm -f $@; \
	$(CC) -M $(CPPFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

INCLUDES              :=     $(SOURCE_FILES:%.c=$(BUILD_DIR)/%.d)
-include $(INCLUDES)

${BUILD_DIR}/%.o : %.c Makefile
	-mkdir -p $(@D)
	$(CC) $(CFLAGS) $(CPPFLAGS) -MMD -c $< -o $@

clean:
	-rm -rf build

