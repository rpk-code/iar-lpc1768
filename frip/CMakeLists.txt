message("Target is ${BOARD_NAME}")

target_include_directories(${BOARD_NAME} PRIVATE include)

target_include_directories(${BOARD_NAME} PRIVATE config)

target_sources(${BOARD_NAME}
    PRIVATE FreeRTOS_ARP.c
    PRIVATE FreeRTOS_BitConfig.c
    PRIVATE FreeRTOS_DHCP.c
    PRIVATE FreeRTOS_DHCPv6.c
    PRIVATE FreeRTOS_DNS.c
    PRIVATE FreeRTOS_DNS_Cache.c
    PRIVATE FreeRTOS_DNS_Callback.c
    PRIVATE FreeRTOS_DNS_Networking.c
    PRIVATE FreeRTOS_DNS_Parser.c
    PRIVATE FreeRTOS_ICMP.c
    PRIVATE FreeRTOS_IP.c
    PRIVATE FreeRTOS_IP_Timers.c
    PRIVATE FreeRTOS_IP_Utils.c
    PRIVATE FreeRTOS_IPv4.c
    PRIVATE FreeRTOS_IPv4_Sockets.c
    PRIVATE FreeRTOS_IPv4_Utils.c
    PRIVATE FreeRTOS_IPv6.c
    PRIVATE FreeRTOS_IPv6_Sockets.c
    PRIVATE FreeRTOS_IPv6_Utils.c
    PRIVATE FreeRTOS_ND.c
    PRIVATE FreeRTOS_RA.c
    PRIVATE FreeRTOS_Routing.c
    PRIVATE FreeRTOS_Sockets.c
    PRIVATE FreeRTOS_Stream_Buffer.c
    PRIVATE FreeRTOS_TCP_IP.c
    PRIVATE FreeRTOS_TCP_IP_IPv4.c
    PRIVATE FreeRTOS_TCP_IP_IPv6.c
    PRIVATE FreeRTOS_TCP_Reception.c
    PRIVATE FreeRTOS_TCP_State_Handling.c
    PRIVATE FreeRTOS_TCP_State_Handling_IPv4.c
    PRIVATE FreeRTOS_TCP_State_Handling_IPv6.c
    PRIVATE FreeRTOS_TCP_Transmission.c
    PRIVATE FreeRTOS_TCP_Transmission_IPv4.c
    PRIVATE FreeRTOS_TCP_Transmission_IPv6.c
    PRIVATE FreeRTOS_TCP_Utils.c
    PRIVATE FreeRTOS_TCP_Utils_IPv4.c
    PRIVATE FreeRTOS_TCP_Utils_IPv6.c
    PRIVATE FreeRTOS_TCP_WIN.c
    PRIVATE FreeRTOS_Tiny_TCP.c
    PRIVATE FreeRTOS_UDP_IP.c
    PRIVATE FreeRTOS_UDP_IPv4.c
    PRIVATE FreeRTOS_UDP_IPv6.c)

add_subdirectory(portable)