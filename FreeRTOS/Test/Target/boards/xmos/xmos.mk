TARGET = XCORE-AI-EXPLORER

BUILD_DIR = build
OUT_DIR = bin

ROOT_XMOS = ../../..
ROOT_KERNEL = $(ROOT_XMOS)/../../../../Source
ROOT_PORTABLE = $(ROOT_KERNEL)/portable
ROOT_MEMMANG = $(ROOT_PORTABLE)/MemMang
ROOT_XCORE_PORT = $(ROOT_PORTABLE)/ThirdParty/xClang/XCOREAI
ROOT_RTOS_SUPPORT = $(ROOT_XMOS)/lib_rtos_support
ROOT_UNITY = $(ROOT_XMOS)/../../../CMock/CMock/vendor/unity

SOURCES_BOARD_MAIN = $(ROOT_XMOS)/main.xc \
                     $(ROOT_XMOS)/main.c
SOURCES_UNITY = $(ROOT_UNITY)/src/unity.c
SOURCES_RTOS_SUPPORT = $(ROOT_RTOS_SUPPORT)/src/rtos_cores.c \
                       $(ROOT_RTOS_SUPPORT)/src/rtos_interrupt.c \
                       $(ROOT_RTOS_SUPPORT)/src/rtos_irq.c \
                       $(ROOT_RTOS_SUPPORT)/src/rtos_locks.c \
                       $(ROOT_RTOS_SUPPORT)/src/rtos_printf.c \
                       $(ROOT_RTOS_SUPPORT)/src/rtos_time.c
SOURCES_KERNEL = $(ROOT_KERNEL)/event_groups.c \
                 $(ROOT_KERNEL)/list.c \
                 $(ROOT_KERNEL)/queue.c \
                 $(ROOT_KERNEL)/stream_buffer.c \
                 $(ROOT_KERNEL)/tasks.c \
                 $(ROOT_KERNEL)/timers.c \
                 $(ROOT_MEMMANG)/heap_4.c \
                 $(ROOT_XCORE_PORT)/port.c \
                 $(ROOT_XCORE_PORT)/port.xc \
                 $(ROOT_XCORE_PORT)/portasm.S

SOURCES_COMMON = $(SOURCES_BOARD_MAIN) $(SOURCES_KERNEL) $(SOURCES_UNITY) $(SOURCES_RTOS_SUPPORT)
ROOT_DIRS_COMMON = $(ROOT_XMOS) $(ROOT_KERNEL) $(ROOT_MEMMANG) \
                   $(ROOT_XCORE_PORT) $(ROOT_RTOS_SUPPORT)/src \
                   $(ROOT_UNITY)/src
FLAGS_COMMON = -Wall -g -report -fxscope \
               $(ROOT_XMOS)/$(TARGET).xn \
               $(ROOT_XMOS)/config.xscope
DIRS_INC_COMMON = $(ROOT_UNITY)/src $(ROOT_XMOS) $(SOURCES_UNITY/src)\
                  $(ROOT_KERNEL)/include $(ROOT_XCORE_PORT) \
                  $(ROOT_RTOS_SUPPORT)/api $(ROOT_RTOS_SUPPORT)/src

$(info $$ROOT_XMOS is [${ROOT_XMOS}])
$(info $$ROOT_KERNEL is [${ROOT_KERNEL}])
$(info $$ROOT_PORTABLE is [${ROOT_PORTABLE}])
$(info $$ROOT_MEMMANG is [${ROOT_MEMMANG}])
$(info $$ROOT_XCORE_PORT is [${ROOT_XCORE_PORT}])
$(info $$ROOT_RTOS_SUPPORT is [${ROOT_RTOS_SUPPORT}])
$(info $$ROOT_UNITY is [${ROOT_UNITY}])
$(info $$SOURCES_BOARD_MAIN is [${SOURCES_BOARD_MAIN}])
$(info $$SOURCES_UNITY is [${SOURCES_UNITY}])
$(info $$SOURCES_RTOS_SUPPORT is [${SOURCES_RTOS_SUPPORT}])
$(info $$SOURCES_KERNEL is [${SOURCES_KERNEL}])
$(info $$SOURCES_COMMON is [${SOURCES_COMMON}])
$(info $$ROOT_DIRS_COMMON is [${ROOT_DIRS_COMMON}])
$(info $$FLAGS_COMMON is [${FLAGS_COMMON}])
$(info $$DIRS_INC_COMMON is [${DIRS_INC_COMMON}])

XCLANG = xcc
