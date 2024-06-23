target_compile_definitions(${BOARD_NAME}
    PRIVATE LPC175x_6x
    PRIVATE RTE_FREERTOS)

target_include_directories(${BOARD_NAME}
    PRIVATE inc
    PRIVATE inc/common)

target_include_directories(${BOARD_NAME} PRIVATE cfg)

target_sources(${BOARD_NAME}
    PRIVATE src/GPIO_LPC17xx.c
    PRIVATE src/PIN_LPC17xx.c)

target_sources(${BOARD_NAME} PRIVATE src/GPDMA_LPC17xx.c)

target_sources(${BOARD_NAME} PRIVATE src/UART_LPC17xx.c)

target_sources(${BOARD_NAME} PRIVATE src/EMAC_LPC17xx.c)
