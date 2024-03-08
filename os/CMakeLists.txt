# FreeRTOS core
target_include_directories(${BOARD_NAME} PRIVATE include)

target_sources(${BOARD_NAME}
    PRIVATE croutine.c
    PRIVATE event_groups.c
    PRIVATE list.c
    PRIVATE queue.c
    PRIVATE stream_buffer.c
    PRIVATE tasks.c
    PRIVATE timers.c)

# FreeRTOS port
target_include_directories(${BOARD_NAME} PRIVATE portable/GCC/ARM_CM3)

target_sources(${BOARD_NAME} PRIVATE portable/GCC/ARM_CM3/port.c)
target_sources(${BOARD_NAME} PRIVATE portable/GCC/ARM_CM3/hooks.c)
target_sources(${BOARD_NAME} PRIVATE portable/MemMang/heap_4.c)

# FreeRTOS config
target_include_directories(${BOARD_NAME} PRIVATE config)
