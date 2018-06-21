#ifndef _ECORE_CON_PRIVATE_H
#define _ECORE_CON_PRIVATE_H

#include "ecore_private.h"
#include "Ecore_Con.h"

#define ECORE_MAGIC_CON_SERVER             0x77665544
#define ECORE_MAGIC_CON_CLIENT             0x77556677
#define ECORE_MAGIC_CON_URL                0x77074255

#ifdef HAVE_SYS_UN_H
#include <sys/un.h>
#endif

#ifdef HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif

#ifdef HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif

#include <inttypes.h>

#define READBUFSIZ 65536

extern int _ecore_con_log_dom;

#ifdef ECORE_CON_DEFAULT_LOG_COLOR
#undef ECORE_LOG_DEFAULT_LOG_COLOR
#endif
#define ECORE_CON_DEFAULT_LOG_COLOR EINA_COLOR_BLUE

#ifdef ERR
# undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_ecore_con_log_dom, __VA_ARGS__)

#ifdef DBG
# undef DBG
#endif
#define DBG(...) EINA_LOG_DOM_DBG(_ecore_con_log_dom, __VA_ARGS__)

#ifdef INF
# undef INF
#endif
#define INF(...) EINA_LOG_DOM_INFO(_ecore_con_log_dom, __VA_ARGS__)

#ifdef WRN
# undef WRN
#endif
#define WRN(...) EINA_LOG_DOM_WARN(_ecore_con_log_dom, __VA_ARGS__)

#ifdef CRI
# undef CRI
#endif
#define CRI(...) EINA_LOG_DOM_CRIT(_ecore_con_log_dom, __VA_ARGS__)

typedef struct Ecore_Con_Socks Ecore_Con_Socks_v4;
typedef struct Ecore_Con_Socks_v5 Ecore_Con_Socks_v5;

struct Ecore_Con_Socks /* v4 */
{
   unsigned char version;

   const char *ip;
   int port;
   const char *username;
   unsigned int ulen;
   Eina_Bool lookup : 1;
   Eina_Bool bind : 1;
};

struct Ecore_Con_Socks_v5
{
   unsigned char version;

   const char *ip;
   int port;
   const char *username;
   unsigned int ulen;
   Eina_Bool lookup : 1;
   Eina_Bool bind : 1;
   /* v5 only */
   unsigned char method;
   const char *password;
   unsigned int plen;
};

#ifdef HAVE_SYSTEMD
extern int sd_fd_index;
extern int sd_fd_max;
#endif

/* init must be called from main thread */
void ecore_con_libproxy_proxies_free(char **proxies);
/* BLOCKING! should be called from a worker thread */
char **ecore_con_libproxy_proxies_get(const char *url, Ecore_Thread *eth);


Eina_Bool ecore_con_server_check(const Ecore_Con_Server *svr);

extern Ecore_Con_Socks *_ecore_con_proxy_once;
extern Ecore_Con_Socks *_ecore_con_proxy_global;
void ecore_con_socks_init(void);
void ecore_con_socks_shutdown(void);

void ecore_con_mempool_init(void);
void ecore_con_mempool_shutdown(void);

void ecore_con_legacy_init(void);
void ecore_con_legacy_shutdown(void);

void _ecore_con_local_mkpath(const char *path, mode_t mode);

/* allow windows and posix to use the same error comparison */
#ifndef SOCKET_ERROR
#define SOCKET_ERROR -1
#endif
#ifndef INVALID_SOCKET
#define INVALID_SOCKET -1
#endif
#ifndef SOCKET_TO_LOOP_FD
#define SOCKET_TO_LOOP_FD(sock) ((int)sock)
#endif
#ifndef _WIN32
#define closesocket(fd) close(fd)
#define SOCKET int
#define SOCKET_FMT "%d"
#else
#define SOCKET_FMT "%"PRIuPTR
#endif


/*
 * This define will force SOCKET to be 'unsigned long', this will
 * force compile to emit errors when assuming "int"/"%d", which is the
 * case on UNIX but not on Windows.
 */
//#define EFL_NET_CHECK_SOCKET_TYPE 1
#if EFL_NET_CHECK_SOCKET_TYPE
#undef SOCKET
#undef SOCKET_FMT
#undef INVALID_SOCKET
#define SOCKET unsigned long
#define SOCKET_FMT "%lu"
#define INVALID_SOCKET ((SOCKET)-1)
#endif

/* some platforms do not have AI_V4MAPPED, then define to 0 so bitwise OR won't be changed */
#ifndef AI_V4MAPPED
#define AI_V4MAPPED 0
#endif
#ifndef AI_ADDRCONFIG
#define AI_ADDRCONFIG 0
#endif
#ifndef AI_CANONNAME
#define AI_CANONNAME 0
#endif

/* Windows do not define EAI_SYSTEM, so just define to some number
 * that won't be matched, effectively disabling the subsequent
 * checks/usage
 */
#ifndef EAI_SYSTEM
#define EAI_SYSTEM 254 /* number that won't match anything in EAI_* */
#endif


void _efl_net_server_udp_client_init(Eo *client, SOCKET fd, const struct sockaddr *addr, socklen_t addrlen, const char *str);
void _efl_net_server_udp_client_feed(Eo *client, Eina_Rw_Slice slice);

#ifdef EFL_NET_SOCKET_WINDOWS_CLASS
#define PIPE_NS "\\\\.\\pipe\\"
char *_efl_net_windows_error_msg_get(DWORD win32err);
Eina_Error _efl_net_socket_windows_init(Eo *o, HANDLE h);
Eina_Error _efl_net_socket_windows_io_start(Eo *o);
HANDLE _efl_net_socket_windows_handle_get(const Eo *o);

typedef struct _Efl_Net_Socket_Windows_Operation Efl_Net_Socket_Windows_Operation;
typedef Eina_Error (*Efl_Net_Socket_Windows_Operation_Success_Cb)(void *data, Eo *sock, DWORD used_size);
typedef Eina_Error (*Efl_Net_Socket_Windows_Operation_Failure_Cb)(void *data, Eo *sock, DWORD win32err);

Efl_Net_Socket_Windows_Operation *_efl_net_socket_windows_operation_new(Eo *sock, Efl_Net_Socket_Windows_Operation_Success_Cb success_cb, Efl_Net_Socket_Windows_Operation_Failure_Cb failure_cb, const void *data);
Eina_Error _efl_net_socket_windows_operation_failed(Efl_Net_Socket_Windows_Operation *op, DWORD win32err);
Eina_Error _efl_net_socket_windows_operation_succeeded(Efl_Net_Socket_Windows_Operation *op, DWORD used_size);

static inline OVERLAPPED *
_efl_net_socket_windows_operation_overlapped_get(Efl_Net_Socket_Windows_Operation *op)
{
   return (OVERLAPPED *)op;
}
#endif

#ifdef EFL_NET_SOCKET_UNIX_CLASS
Eina_Bool efl_net_unix_fmt(char *buf, size_t buflen, SOCKET fd, const struct sockaddr_un *addr, socklen_t addrlen);
#endif
Eina_Bool efl_net_ip_port_parse(const char *address, struct sockaddr_storage *storage);
Eina_Bool efl_net_ip_port_parse_split(const char *host, const char *port, struct sockaddr_storage *storage);
Eina_Bool efl_net_ip_port_fmt(char *buf, size_t buflen, const struct sockaddr *addr);

#ifdef HAVE_SYSTEMD
/**
 * Checks if the next FD in the sd_fd_index:sd_fd_max is of the
 * expected family, protocol and if it's listening.
 *
 * This is similar to sd_is_socket()/sd_is_socket_inet(), but will
 * also parse address in our standard format "IP:PORT", including IPv6
 * within braces, and then will validate the address with
 * getsockaddr() for INET.
 *
 * @param address the address to validate
 * @param family AF_UNIX or AF_UNSPEC for INET, in that case AF_INET
 *        or AF_INET6 will be inferred from @a address.
 * @param type SOCK_STREAM or SOCK_DGRAM
 * @param[out] listening where to return listening state, should be
 *       NULL for @a type SOCK_DGRAM
 *
 * @return 0 on success, error otherwise.
 *
 * @internal
 */
Eina_Error efl_net_ip_socket_activate_check(const char *address, int family, int type, Eina_Bool *listening);
#endif

/**
 * @brief splits an address in the format "host:port" in two
 * null-terminated strings.
 *
 * The address may be 'server.com:1234', 'server.com:http',
 * 'server.com' (@c *p_port will be NULL), IPv4 127.0.0.1:456 or
 * IPv6 [::1]:456
 *
 * @param[inout] buf contains the string to be split and will be modified.
 * @param[out] p_host returns a pointer inside @a buf with
 *             null-terminated host part.
 * @param[out] p_port returns a pointer with null-terminated port
 *             part. The pointer may be inside @a buf if port was
 *             specified or #NULL if it wasn't specified.
 *
 * @return #EINA_TRUE on success, #EINA_FALSE on errors.
 *
 * @internal
 */
Eina_Bool efl_net_ip_port_split(char *buf, const char **p_host, const char **p_port);

SOCKET efl_net_socket4(int domain, int type, int protocol, Eina_Bool close_on_exec);

/**
 * @brief callback to notify of resolved address.
 *
 * The callback is given the ownership of the result, thus must free
 * it with freeaddrinfo().
 *
 * @internal
 */
typedef void (*Efl_Net_Ip_Resolve_Async_Cb)(void *data, const char *host, const char *port, const struct addrinfo *hints, struct addrinfo *result, int gai_error);

/**
 * @brief asynchronously resolve a host and port using getaddrinfo().
 *
 * This will call getaddrinfo() in a thread, taking care to return the
 * result to the main loop and calling @a cb with given user @a data.
 *
 * @internal
 */
Ecore_Thread *efl_net_ip_resolve_async_new(const char *host, const char *port, const struct addrinfo *hints, Efl_Net_Ip_Resolve_Async_Cb cb, const void *data);

/**
 * @brief callback to notify of connection.
 *
 * The callback is given the ownership of the socket (sockfd), thus
 * must close().
 *
 * @internal
 */
typedef void (*Efl_Net_Connect_Async_Cb)(void *data, const struct sockaddr *addr, socklen_t addrlen, SOCKET sockfd, Eina_Error error);

/**
 * @brief asynchronously create a socket and connect to the address.
 *
 * This will call socket() and connect() in a thread, taking care to
 * return the result to the main loop and calling @a cb with given
 * user @a data.
 *
 * For name resolution and proxy support use
 * efl_net_ip_connect_async_new()
 *
 * @internal
 */
Ecore_Thread *efl_net_connect_async_new(const struct sockaddr *addr, socklen_t addrlen, int type, int protocol, Eina_Bool close_on_exec, Efl_Net_Connect_Async_Cb cb, const void *data);

/**
 * @brief asynchronously create a socket and connect to the IP address.
 *
 * This wil resolve the address using getaddrinfo(), create a socket
 * and connect in a thread.
 *
 * If a @a proxy is given, then it's always used. Otherwise the
 * environment variable @a proxy_env is used unless it matches @a
 * no_proxy_env. Some systems may do special queries for proxy from
 * the thread.
 *
 * @param address the host:port to connect. Host may be a name or an
 *        IP address, IPv6 addresses should be enclosed in braces.
 * @param proxy a mandatory proxy to use. If "" (empty string), it's
 *        disabled. If NULL, then @a proxy_env is used unless it
 *        matches @a no_proxy_env.
 * @param proxy_env if @a proxy is NULL, then this will be used as the
 *        proxy unless it matches @a no_proxy_env.
 * @param no_proxy_env a comma-separated list of matches that will
 *        avoid using @a proxy_env. "server.com" will inhibit proxy
 *        for "server.com", "host.server.com" but not "xserver.com".
 * @param type the socket type, such as SOCK_STREAM or SOCK_DGRAM.
 * @param protocol the socket protocol, such as IPPROTO_TCP.
 * @param close_on_exec if EINA_TRUE, will set SOCK_CLOEXEC.
 * @param cb the callback to report connection
 * @param data data to give to callback
 *
 * @return an Ecore_Thread that will execute the connection.
 *
 * @internal
 */
Ecore_Thread *efl_net_ip_connect_async_new(const char *address, const char *proxy, const char *proxy_env, const char *no_proxy_env, int type, int protocol, Eina_Bool close_on_exec, Efl_Net_Connect_Async_Cb cb, const void *data);

static inline Eina_Error
efl_net_socket_error_get(void)
{
#ifndef _WIN32
   return errno;
#else
   Eina_Error err = WSAGetLastError();

   if (0) { }

   /* used by send() */
#if defined(WSAEACCES) && (WSAEACCES != EACCES)
   else if (err == WSAEACCES) err = EACCES;
#endif
#if defined(WSAEWOULDBLOCK) && (WSAEWOULDBLOCK != EAGAIN)
   else if (err == WSAEWOULDBLOCK) err = EAGAIN;
#endif
#if defined(WSAEBADF) && (WSAEBADF != EBADF)
   else if (err == WSAEBADF) err = EBADF;
#endif
#if defined(WSAECONNRESET) && (WSAECONNRESET != ECONNRESET)
   else if (err == WSAECONNRESET) err = ECONNRESET;
#endif
#if defined(WSAEDESTADDRREQ) && (WSAEDESTADDRREQ != EDESTADDRREQ)
   else if (err == WSAEDESTADDRREQ) err = EDESTADDRREQ;
#endif
#if defined(WSAEFAULT) && (WSAEFAULT != EFAULT)
   else if (err == WSAEFAULT) err = EFAULT;
#endif
#if defined(WSAEINTR) && (WSAEINTR != EINTR)
   else if (err == WSAEINTR) err = EINTR;
#endif
#if defined(WSAEINVAL) && (WSAEINVAL != EINVAL)
   else if (err == WSAEINVAL) err = EINVAL;
#endif
#if defined(WSAEISCONN) && (WSAEISCONN != EISCONN)
   else if (err == WSAEISCONN) err = EISCONN;
#endif
#if defined(WSAEMSGSIZE) && (WSAEMSGSIZE != EMSGSIZE)
   else if (err == WSAEMSGSIZE) err = EMSGSIZE;
#endif
#if defined(WSAENOBUFS) && (WSAENOBUFS != ENOBUFS)
   else if (err == WSAENOBUFS) err = ENOBUFS;
#endif
#if defined(WSA_NOT_ENOUGH_MEMORY) && (WSA_NOT_ENOUGH_MEMORY != ENOMEM)
   else if (err == WSA_NOT_ENOUGH_MEMORY) err = ENOMEM;
#endif
#if defined(WSAENOTCONN) && (WSAENOTCONN != ENOTCONN)
   else if (err == WSAENOTCONN) err = ENOTCONN;
#endif
#if defined(WSAENOTSOCK) && (WSAENOTSOCK != ENOTSOCK)
   else if (err == WSAENOTSOCK) err = ENOTSOCK;
#endif
#if defined(WSAEOPNOTSUPP) && (WSAEOPNOTSUPP != EOPNOTSUPP)
   else if (err == WSAEOPNOTSUPP) err = EOPNOTSUPP;
#endif
#if defined(WSAESHUTDOWN) && (WSAESHUTDOWN != EPIPE)
   else if (err == WSAESHUTDOWN) err = EPIPE;
#endif

   /* extras used by recv() */
#if defined(WSAECONNREFUSED) && (WSAECONNREFUSED != ECONNREFUSED)
   else if (err == WSAECONNREFUSED) err = ECONNREFUSED;
#endif

   /* extras used by getsockopt() */
#if defined(WSAENOPROTOOPT) && (WSAENOPROTOOPT != ENOPROTOOPT)
   else if (err == WSAENOPROTOOPT) err = ENOPROTOOPT;
#endif

   return err;
#endif
}

/**
 * Join a multicast group specified by address.
 *
 * Address must be an IPv4 or IPv6 depending on @a fd and will be
 * parsed using inet_pton() with corresponding @a family. The address
 * may contain an '@@' delimiter to specify the local interface IP
 * address to use. No interface means '0.0.0.0'.
 *
 * @param fd socket to operate on.
 * @param family the socket family of fd, AF_INET or AF_INET6.
 * @param address the address in the format IP[@@IFACE]
 *
 * @return 0 on success, errno mapping otherwise.
 * @internal
 */
Eina_Error efl_net_multicast_join(SOCKET fd, int family, const char *address);

/**
 * Leave a multicast group specified by address.
 *
 * This reverses the effect of efl_net_multicast_join().
 *
 * @param fd socket to operate on.
 * @param family the socket family of fd, AF_INET or AF_INET6.
 * @param address the address in the format IP[@@IFACE]
 *
 * @return 0 on success, errno mapping otherwise.
 * @internal
 */
Eina_Error efl_net_multicast_leave(SOCKET fd, int family, const char *address);

/**
 * Sets the Time-To-Live of multicast packets. <= 1 disables going
 * outside of local network.
 *
 * @param fd socket to operate on.
 * @param family the socket family of fd, AF_INET or AF_INET6.
 * @param ttl the time-to-live in units.
 *
 * @return 0 on success, errno mapping otherwise.
 * @internal
 */
Eina_Error efl_net_multicast_ttl_set(SOCKET fd, int family, uint8_t ttl);

/**
 * Retrieves the current time-to-live of multicast packets.
 *
 * @param fd socket to operate on.
 * @param family the socket family of fd, AF_INET or AF_INET6.
 * @param[out] ttl returns the time-to-live in units.
 *
 * @return 0 on success, errno mapping otherwise.
 * @internal
 */
Eina_Error efl_net_multicast_ttl_get(SOCKET fd, int family, uint8_t *ttl);

/**
 * Sets if the current local address should get a copy of the packets sent.
 *
 * @param fd socket to operate on.
 * @param family the socket family of fd, AF_INET or AF_INET6.
 * @param loopback if #EINA_TRUE, enables receive of local copy. #EINA_FALSE means only remote peers will do.
 *
 * @return 0 on success, errno mapping otherwise.
 * @internal
 */
Eina_Error efl_net_multicast_loopback_set(SOCKET fd, int family, Eina_Bool loopback);

/**
 * Gets if the current local address should get a copy of the packets sent.
 *
 * @param fd socket to operate on.
 * @param family the socket family of fd, AF_INET or AF_INET6.
 * @param[out] loopback returns if #EINA_TRUE, enables receive of local copy. #EINA_FALSE means only remote peers will do.
 *
 * @return 0 on success, errno mapping otherwise.
 * @internal
 */
Eina_Error efl_net_multicast_loopback_get(SOCKET fd, int family, Eina_Bool *loopback);

/**
 * Query the size of the next UDP datagram pending on queue.
 *
 * @param fd socket to operate on.
 * @return the size in bytes.
 * @internal
 */
size_t efl_net_udp_datagram_size_query(SOCKET fd);


/* SSL abstraction API */
extern void *efl_net_ssl_context_connection_new(Efl_Net_Ssl_Context *context);

#define EFL_NET_DIALER_HTTP_BUFFER_RECEIVE_SIZE (1U << 14) /* 16Kb to receive */

#endif
