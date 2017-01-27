EFL_OPTION(WITH_EFL_NET_CONTROL_BACKEND "Choose the Efl.Net.Control backend" "connman" CHOICE connman;none)

FUNC_CHECK(accept4 INCLUDE_FILES sys/types.h sys/socket.h DEFINITIONS "-D_GNU_SOURCE=1")
FUNC_CHECK(gmtime_r INCLUDE_FILES time.h DEFINITIONS "-D_POSIX_C_SOURCE=1 -D_BSD_SOURCE=1 -D_SVID_SOURCE=1")

HEADER_CHECK(arpa/inet.h)
HEADER_CHECK(net/if.h)
HEADER_CHECK(netinet/in.h)
HEADER_CHECK(netinet/ssl.h)
HEADER_CHECK(netinet/tcp.h)
HEADER_CHECK(netinet/udp.h)
HEADER_CHECK(sys/socket.h)
HEADER_CHECK(sys/un.h)
HEADER_CHECK(ws2tcpip.h)
