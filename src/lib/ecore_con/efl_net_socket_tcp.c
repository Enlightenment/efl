#define EFL_NET_SOCKET_TCP_PROTECTED 1
#define EFL_NET_SOCKET_FD_PROTECTED 1
#define EFL_LOOP_FD_PROTECTED 1
#define EFL_IO_READER_FD_PROTECTED 1
#define EFL_IO_WRITER_FD_PROTECTED 1
#define EFL_IO_CLOSER_FD_PROTECTED 1
#define EFL_IO_READER_PROTECTED 1
#define EFL_IO_WRITER_PROTECTED 1
#define EFL_IO_CLOSER_PROTECTED 1
#define EFL_NET_SOCKET_PROTECTED 1

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Ecore.h"
#include "Ecore_Con.h"
#include "ecore_con_private.h"

#ifdef HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif
#ifdef HAVE_NETINET_TCP_H
# include <netinet/tcp.h>
#endif
#ifdef HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif
#ifdef HAVE_ARPA_INET_H
# include <arpa/inet.h>
#endif
#ifdef _WIN32
# include <Evil.h>
#endif

#define MY_CLASS EFL_NET_SOCKET_TCP_CLASS

typedef struct _Efl_Net_Socket_Tcp_Data
{
   Eina_Bool keep_alive;
   Eina_Bool no_delay;
   Eina_Bool cork;
} Efl_Net_Socket_Tcp_Data;

EOLIAN static void
_efl_net_socket_tcp_efl_loop_fd_fd_set(Eo *o, Efl_Net_Socket_Tcp_Data *pd, int pfd)
{
   SOCKET fd = (SOCKET)pfd;

   efl_loop_fd_set(efl_super(o, MY_CLASS), pfd);

   if (fd != INVALID_SOCKET)
     {
        struct sockaddr_storage addr;
        socklen_t addrlen;
        int family;

        /* apply postponed values */
        efl_net_socket_tcp_keep_alive_set(o, pd->keep_alive);
        efl_net_socket_tcp_no_delay_set(o, pd->no_delay);
        efl_net_socket_tcp_cork_set(o, pd->cork);

        family = efl_net_socket_fd_family_get(o);
        if (family == AF_UNSPEC) return;

        addrlen = sizeof(addr);
        if (getsockname(fd, (struct sockaddr *)&addr, &addrlen) != 0)
          ERR("getsockname(" SOCKET_FMT "): %s", fd, eina_error_msg_get(efl_net_socket_error_get()));
        else
          {
             char str[INET6_ADDRSTRLEN + sizeof("[]:65536")];
             if (efl_net_ip_port_fmt(str, sizeof(str), (struct sockaddr *)&addr))
               efl_net_socket_address_local_set(o, str);
          }

        addrlen = sizeof(addr);
        if (getpeername(fd, (struct sockaddr *)&addr, &addrlen) != 0)
          ERR("getpeername(" SOCKET_FMT "): %s", fd, eina_error_msg_get(efl_net_socket_error_get()));
        else
          {
             char str[INET6_ADDRSTRLEN + sizeof("[]:65536")];
             if (efl_net_ip_port_fmt(str, sizeof(str), (struct sockaddr *)&addr))
               efl_net_socket_address_remote_set(o, str);
          }
     }
}

EOLIAN static Eina_Bool
_efl_net_socket_tcp_keep_alive_set(Eo *o, Efl_Net_Socket_Tcp_Data *pd, Eina_Bool keep_alive)
{
   SOCKET fd;
   Eina_Bool old = pd->keep_alive;
#ifdef _WIN32
   DWORD value;
#else
   int value;
#endif

   pd->keep_alive = keep_alive;

   fd = efl_loop_fd_get(o);
   if (fd == INVALID_SOCKET) return EINA_TRUE; /* postpone until fd_set() */

   value = keep_alive;
   if (setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (const char *)&value, sizeof(value)) != 0)
     {
        ERR("setsockopt(" SOCKET_FMT ", SOL_SOCKET, SO_KEEPALIVE, %d): %s",
            fd, (int)value, eina_error_msg_get(efl_net_socket_error_get()));
        pd->keep_alive = old;
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_net_socket_tcp_keep_alive_get(const Eo *o, Efl_Net_Socket_Tcp_Data *pd)
{
   SOCKET fd;
#ifdef _WIN32
   DWORD value = 0;
   int valuelen;
#else
   int value = 0;
   socklen_t valuelen;
#endif

   fd = efl_loop_fd_get(o);
   if (fd == INVALID_SOCKET) return pd->keep_alive;

   /* if there is a fd, always query it directly as it may be modified
    * elsewhere by nasty users.
    */
   valuelen = sizeof(value);
   if (getsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (char *)&value, &valuelen) != 0)
     {
        ERR("getsockopt(" SOCKET_FMT ", SOL_SOCKET, SO_KEEPALIVE): %s",
            fd, eina_error_msg_get(efl_net_socket_error_get()));
        return EINA_FALSE;
     }

   pd->keep_alive = !!value; /* sync */
   return pd->keep_alive;
}

EOLIAN static Eina_Bool
_efl_net_socket_tcp_no_delay_set(Eo *o, Efl_Net_Socket_Tcp_Data *pd, Eina_Bool no_delay)
{
   SOCKET fd;
   Eina_Bool old = pd->no_delay;
#ifdef _WIN32
   BOOL value;
#else
   int value;
#endif

   pd->no_delay = no_delay;

   fd = efl_loop_fd_get(o);
   if (fd == INVALID_SOCKET) return EINA_TRUE; /* postpone until fd_set() */

   value = no_delay;
   if (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (const char *)&value, sizeof(value)) != 0)
     {
        ERR("setsockopt(" SOCKET_FMT ", IPPROTO_TCP, TCP_NODELAY, %d): %s",
            fd, value, eina_error_msg_get(efl_net_socket_error_get()));
        pd->no_delay = old;
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_net_socket_tcp_no_delay_get(const Eo *o, Efl_Net_Socket_Tcp_Data *pd)
{
   SOCKET fd;
#ifdef _WIN32
   BOOL value;
   int valuelen;
#else
   int value;
   socklen_t valuelen;
#endif

   fd = efl_loop_fd_get(o);
   if (fd == INVALID_SOCKET) return pd->no_delay;

   /* if there is a fd, always query it directly as it may be modified
    * elsewhere by nasty users.
    */
   valuelen = sizeof(value);
   if (getsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char *)&value, &valuelen) != 0)
     {
        ERR("getsockopt(" SOCKET_FMT ", IPPROTO_TCP, TCP_NODELAY): %s",
            fd, eina_error_msg_get(efl_net_socket_error_get()));
        return EINA_FALSE;
     }

   pd->no_delay = !!value; /* sync */
   return pd->no_delay;
}

static inline int
_cork_option_get(void)
{
#if defined(HAVE_TCP_CORK)
   return TCP_CORK;
#elif defined(HAVE_TCP_NOPUSH)
   return TCP_NOPUSH;
#else
   return -1;
#endif
}

EOLIAN static Eina_Bool
_efl_net_socket_tcp_cork_set(Eo *o, Efl_Net_Socket_Tcp_Data *pd, Eina_Bool cork)
{
   SOCKET fd;
   int value, option;
   Eina_Bool old = pd->cork;

   option = _cork_option_get();
   if (EINA_UNLIKELY(option < 0))
     {
        if (cork)
          ERR("Could not find a TCP_CORK equivalent on your system");
        return EINA_FALSE;
     }

   pd->cork = cork;

   fd = efl_loop_fd_get(o);
   if (fd == INVALID_SOCKET) return EINA_TRUE; /* postpone until fd_set() */

   value = cork;
   if (setsockopt(fd, IPPROTO_TCP, option, (const char *)&value, sizeof(value)) != 0)
     {
        ERR("setsockopt(" SOCKET_FMT ", IPPROTO_TCP, 0x%x, %d): %s",
            fd, option, value, eina_error_msg_get(efl_net_socket_error_get()));
        pd->cork = old;
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_net_socket_tcp_cork_get(const Eo *o, Efl_Net_Socket_Tcp_Data *pd)
{
   SOCKET fd;
   int value = 0;
   socklen_t valuelen;
   int option;

   option = _cork_option_get();
   if (EINA_UNLIKELY(option < 0))
     {
        WRN("Could not find a TCP_CORK equivalent on your system");
        return EINA_FALSE;
     }

   fd = efl_loop_fd_get(o);
   if (fd == INVALID_SOCKET) return pd->cork;

   /* if there is a fd, always query it directly as it may be modified
    * elsewhere by nasty users.
    */
   valuelen = sizeof(value);
   if (getsockopt(fd, IPPROTO_TCP, option, (char *)&value, &valuelen) != 0)
     {
        ERR("getsockopt(" SOCKET_FMT ", IPPROTO_TCP, 0x%x): %s",
            fd, option, eina_error_msg_get(efl_net_socket_error_get()));
        return EINA_FALSE;
     }

   pd->cork = !!value; /* sync */
   return pd->cork;
}

#include "efl_net_socket_tcp.eo.c"
