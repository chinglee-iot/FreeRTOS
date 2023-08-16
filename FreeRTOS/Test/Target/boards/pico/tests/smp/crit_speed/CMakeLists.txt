cmake_minimum_required(VERSION 3.13)

project(example C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

set(TEST_NAME crit_speed)
set(TEST_FUNCTION_NAME vRunCriticalSectionSpeed )

set(TEST_INCLUDE_PATHS ${CMAKE_CURRENT_LIST_DIR}/../../../../../tests/smp/${TEST_NAME})
set(TEST_SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/../../../../../tests/smp/${TEST_NAME})

add_library(${TEST_NAME} INTERFACE)
target_sources(${TEST_NAME} INTERFACE
        ${BOARD_LIBRARY_DIR}/main.c
        ${CMAKE_CURRENT_LIST_DIR}/rp2040_test_runner.c
        ${TEST_SOURCE_DIR}/${TEST_NAME}.c)

target_include_directories(${TEST_NAME} INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/../../..
        ${TEST_INCLUDE_PATHS}
        )

target_link_libraries(${TEST_NAME} INTERFACE
        FreeRTOS-Kernel
        FreeRTOS-Kernel-Heap4
        ${BOARD_LINK_LIBRARIES})

add_executable(test_${TEST_NAME})
enable_board_functions(test_${TEST_NAME})
target_link_libraries(test_${TEST_NAME} ${TEST_NAME})
target_include_directories(test_${TEST_NAME} PUBLIC
        ${BOARD_INCLUDE_PATHS})
target_compile_definitions(test_${TEST_NAME} PRIVATE
        ${BOARD_DEFINES}
)
target_compile_options( test_${TEST_NAME} PUBLIC -DUPSTREAM_BUILD=1 -DTEST_FUNCTION_NAME=${TEST_FUNCTION_NAME} )
