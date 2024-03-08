target_include_directories(${BOARD_NAME} PRIVATE inc)

target_sources(${BOARD_NAME}
    PRIVATE src/startup_LPC17xx.c
    PRIVATE src/system_LPC17xx.c)