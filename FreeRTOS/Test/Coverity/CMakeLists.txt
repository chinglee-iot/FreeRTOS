# Minimum required version of CMake
cmake_minimum_required ( VERSION 3.13.0 )

# Name of the project
project ( "FreeRTOS-Kernel Static analysis"
          VERSION 1.0.0
          LANGUAGES C )

# Allow the project to be organized into folders.
set_property( GLOBAL PROPERTY USE_FOLDERS ON )
set(CMAKE_VERBOSE_MAKEFILE ON)

# Use C99 for stdint
set( CMAKE_C_STANDARD 99 )
set( CMAKE_C_STANDARD_REQUIRED ON )

# Do not allow in-source build.
if( ${PROJECT_SOURCE_DIR} STREQUAL ${PROJECT_BINARY_DIR} )
    message( FATAL_ERROR "In-source build is not allowed. Please build in a separate directory, such as ${PROJECT_SOURCE_DIR}/build." )
endif()

# Set global path variables.
get_filename_component(__MODULE_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../" ABSOLUTE)
set( MODULE_ROOT_DIR ${__MODULE_ROOT_DIR} CACHE INTERNAL "FreeRTOS repository root." )

# Set the kernel directory
set( KERNEL_DIRECTORY ${MODULE_ROOT_DIR}/FreeRTOS/Source )

# If the kernel is not submoduled, download it.
if( NOT EXISTS ${KERNEL_DIRECTORY}/include )
    # Inform the user of the actions
    message( STATUS "FreeRTOS-Kernel is required for this build. Submoduling it..." )
    execute_process( COMMAND git submodule update --init --checkout ${KERNEL_DIRECTORY}
                     WORKING_DIRECTORY ${MODULE_ROOT_DIR} )
endif()

# Add kernel sources to a list
file( GLOB KERNEL_SOURCES
      ${KERNEL_DIRECTORY}/*.c )

message( STATUS "KERNEL_SOURCES are ${KERNEL_SOURCES}" )

file( GLOB FREERTOS_CONFIG_INCLUDE_MULTI_CORE1
      ${CMAKE_CURRENT_LIST_DIR}/MultiCore1Config )

file( GLOB FREERTOS_CONFIG_INCLUDE_MULTI_CORE2
      ${CMAKE_CURRENT_LIST_DIR}/MultiCore2Config )

file( GLOB FREERTOS_CONFIG_INCLUDE_SINGLE_CORE
      ${CMAKE_CURRENT_LIST_DIR}/SingleCoreConfig )

# A better way would be to create a library such that all other dependencies are
# ignored by the static analysis tool. But, since +TCP is very closely linked
# with the FreeRTOS-Kernel, an executable had to be created.

# Add the executable for static analysis
add_executable( smp1 ${KERNEL_SOURCES}
                               ${CMAKE_CURRENT_LIST_DIR}/Portable.c )

# Link the include directories with the target
target_include_directories( smp1
                            PUBLIC "${KERNEL_DIRECTORY}/include"
                            PUBLIC "${FREERTOS_CONFIG_INCLUDE_MULTI_CORE1}" )

# Uncomment the below line if the desired platform is 32-bit
# set_target_properties( smp1 PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32" )

target_compile_options(smp1 PRIVATE
  -Wall -Wextra -Wpedantic
)

# Add the executable for static analysis
add_executable( smp2 ${KERNEL_SOURCES}
                               ${CMAKE_CURRENT_LIST_DIR}/Portable.c )

# Link the include directories with the target
target_include_directories( smp2
                            PUBLIC "${KERNEL_DIRECTORY}/include"
                            PUBLIC "${FREERTOS_CONFIG_INCLUDE_MULTI_CORE2}" )

# Uncomment the below line if the desired platform is 32-bit
# set_target_properties( smp2 PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32" )

target_compile_options(smp2 PRIVATE
  -Wall -Wextra -Wpedantic
)

# Add the executable for static analysis
add_executable( single ${KERNEL_SOURCES}
                               ${CMAKE_CURRENT_LIST_DIR}/Portable.c )

# Link the include directories with the target
target_include_directories( single
                            PUBLIC "${KERNEL_DIRECTORY}/include"
                            PUBLIC "${FREERTOS_CONFIG_INCLUDE_SINGLE_CORE}" )

# Uncomment the below line if the desired platform is 32-bit
# set_target_properties( single PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32" )

target_compile_options(single PRIVATE
  -Wall -Wextra -Wpedantic
)



