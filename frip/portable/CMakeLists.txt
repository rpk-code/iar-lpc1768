target_include_directories(${BOARD_NAME} PRIVATE Compiler/GCC)

target_sources(${BOARD_NAME} PRIVATE BufferManagement/BufferAllocation_2.c)

add_subdirectory(NetworkInterface/LPC17xx)