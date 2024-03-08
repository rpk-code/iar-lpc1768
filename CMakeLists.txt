cmake_minimum_required(VERSION 3.22)

include(${CMAKE_CURRENT_LIST_DIR}/cmake_scripts/board_cfg.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake_scripts/toolchain_gcc_cfg.cmake)

set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")
set(CMAKE_BUILD_TYPE ${BUILD_TYPE})

project(${BOARD_NAME} LANGUAGES C)

set(CMAKE_SYSTEM_PROCESSOR ${BOARD_MCU})
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${C_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS ${LINKER_FLAGS})

add_executable(${BOARD_NAME} "")

set_target_properties(${BOARD_NAME} PROPERTIES
    SUFFIX ".elf"
    LINK_DEPENDS ${LINKER_SCRIPT}
    ADDITIONAL_CLEAN_FILES ${CMAKE_CURRENT_BINARY_DIR}/${BOARD_NAME}.map)

target_compile_definitions(${BOARD_NAME} PRIVATE __USE_CMSIS=CMSIS_CORE_LPC17xx)

add_subdirectory(mcu)
add_subdirectory(os)
add_subdirectory(apps)