cmake_minimum_required(VERSION 3.15)
project(example)

set(FREERTOS_KERNEL_PATH "../../../../../../../Source")

set(TARGET_TEST_PATH "../../../../../tests" )

set(PLATFROM_PATH "../../")

# Add the freertos_config for FreeRTOS-Kernel
add_library(freertos_config INTERFACE)

target_include_directories(freertos_config
    INTERFACE
    ${PLATFROM_PATH}
    ${TARGET_TEST_PATH}/smp/template
)

# Select the heap port.  values between 1-4 will pick a heap.
set(FREERTOS_HEAP "4" CACHE STRING "" FORCE)

# Select the native compile PORT
set(FREERTOS_PORT "TEMPLATE" CACHE STRING "" FORCE)

# Adding the FreeRTOS-Kernel subdirectory
add_subdirectory(${FREERTOS_KERNEL_PATH} FreeRTOS-Kernel)

add_executable(${PROJECT_NAME}
    ${PLATFROM_PATH}/main.c
    ${CMAKE_CURRENT_SOURCE_DIR}/test_runner.c
    ${TARGET_TEST_PATH}/smp/template/test_name.c
)

target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${FREERTOS_KERNEL_PATH}/include
        ${FREERTOS_PORTABLE_PATH}
        ${TARGET_TEST_PATH}/include
)

target_link_libraries(${PROJECT_NAME} freertos_kernel freertos_config)

set_property(TARGET freertos_kernel PROPERTY C_STANDARD 90)