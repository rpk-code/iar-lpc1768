# Compiler and tools
set(COMPILER_NAME "arm-none-eabi-gcc")
set(BIN_TOOL_NAME "arm-none-eabi-objcopy")
set(SIZE_TOOL_NAME "arm-none-eabi-size")

# C compile flags
set(COMMON_C_FLAGS "-mcpu=${BOARD_CPU} -mthumb -Wall -fno-common -fno-builtin -ffunction-sections -fdata-sections -fmerge-constants -fstack-usage")
if (${BUILD_TYPE} STREQUAL "Debug")
    message("Debug build is active")
    set(C_FLAGS "${COMMON_C_FLAGS} -O0 -g3 -gdwarf-4 -DDEBUG")
elseif(${BUILD_TYPE} STREQUAL "Release")
    message("Release build is active")
    set(C_FLAGS "${COMMON_C_FLAGS} -O2 -DRELEASE")
else()
    message(ERROR "Build type is invalid")
endif()

# Linker script
set(LINKER_SCRIPT_DIR "${CMAKE_CURRENT_LIST_DIR}/../linker_scripts")
set(LINKER_SCRIPT "${LINKER_SCRIPT_DIR}/${TARGET_MEM}.ld")

# Linker flags
set(LINKER_FLAGS "-mcpu=${BOARD_CPU} -mthumb -T ${LINKER_SCRIPT} -Wl,-Map,${BOARD_NAME}.map -Xlinker --cref -Xlinker --gc-sections -Xlinker -print-memory-usage -specs=nano.specs -specs=nosys.specs")

# Set CMAKE compiler
set(CMAKE_C_COMPILER ${COMPILER_NAME})

