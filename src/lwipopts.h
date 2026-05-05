#ifndef _LWIPOPTS_H
#define _LWIPOPTS_H

// Running without RTOS
#define NO_SYS                      1
#define LWIP_SOCKET                 0
#define LWIP_COMPAT_SOCKETS         0
#define LWIP_POSIX_SOCKETS_IO_NAMES 0
#define LWIP_IGMP                   0

#define LWIP_ARP                    1
#define LWIP_ETHERNET               1
#define LWIP_ICMP                   1
#define LWIP_RAW                    1
#define LWIP_IPV4                   1
#define LWIP_UDP                    1

#endif
