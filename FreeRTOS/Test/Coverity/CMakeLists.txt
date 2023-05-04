# Minimum required version of CMake
cmake_minimum_required ( VERSION 3.13.0 )

# Name of the project
project ( "FreeRTOS-Kernel Static analysis"
          VERSION 1.0.0
          LANGUAGES C )

# Options to choose FreeRTOS configuration file
option(COVERITY_MULTI_CORE1_CONFIG "Multiple Core 1 Config" ON)
option(COVERITY_MULTI_CORE2_CONFIG "Multiple Core 2 Config" OFF)

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

if( COVERITY_MULTI_CORE1_CONFIG )
    file( GLOB FREERTOS_CONFIG_INCLUDE
          ${CMAKE_CURRENT_LIST_DIR}/MultiCore1Config )
elseif( COVERITY_MULTI_CORE2_CONFIG )
    file( GLOB FREERTOS_CONFIG_INCLUDE
          ${CMAKE_CURRENT_LIST_DIR}/MultiCore2Config )
else()
    file( GLOB FREERTOS_CONFIG_INCLUDE
          ${CMAKE_CURRENT_LIST_DIR}/SingleCoreConfig )
    message( "Using single core configuation." )
endif()

# A better way would be to create a library such that all other dependencies are
# ignored by the static analysis tool. But, since +TCP is very closely linked
# with the FreeRTOS-Kernel, an executable had to be created.

# Add the executable for static analysis
add_executable( StaticAnalysis ${KERNEL_SOURCES}
                               ${CMAKE_CURRENT_LIST_DIR}/Portable.c )

# Link the include directories with the target
target_include_directories( StaticAnalysis
                            PUBLIC "${KERNEL_DIRECTORY}/include"
                            PUBLIC "${FREERTOS_CONFIG_INCLUDE}" )

# Uncomment the below line if the desired platform is 32-bit
# set_target_properties( StaticAnalysis PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32" )

target_compile_options(StaticAnalysis PRIVATE
  -Wall -Wextra -Wpedantic
)
