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
#ifdef HAVE_EVIL
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
_efl_net_socket_tcp_efl_loop_fd_fd_set(Eo *o, Efl_Net_Socket_Tcp_Data *pd EINA_UNUSED, int fd)
{
   efl_loop_fd_set(efl_super(o, MY_CLASS), fd);

   if (fd >= 0)
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
        if (getsockname(fd, (struct sockaddr *)&addr, &addrlen) < 0)
          ERR("getsockname(%d): %s", fd, strerror(errno));
        else
          {
             char str[INET6_ADDRSTRLEN + sizeof("[]:65536")];
             if (efl_net_ip_port_fmt(str, sizeof(str), (struct sockaddr *)&addr))
               efl_net_socket_address_local_set(o, str);
          }

        addrlen = sizeof(addr);
        if (getpeername(fd, (struct sockaddr *)&addr, &addrlen) < 0)
          ERR("getpeername(%d): %s", fd, strerror(errno));
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
   int value, fd;
   Eina_Bool old = pd->keep_alive;

   pd->keep_alive = keep_alive;

   fd = efl_loop_fd_get(o);
   if (fd < 0) return EINA_TRUE; /* postpone until fd_set() */

   value = keep_alive;
   if (setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &value, sizeof(value)) < 0)
     {
        ERR("setsockopt(%d, SOL_SOCKET, SO_KEEPALIVE, %d): %s",
            fd, value, strerror(errno));
        pd->keep_alive = old;
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_net_socket_tcp_keep_alive_get(Eo *o, Efl_Net_Socket_Tcp_Data *pd)
{
   int value = 0, fd;
   socklen_t valuelen;

   fd = efl_loop_fd_get(o);
   if (fd < 0) return pd->keep_alive;

   /* if there is a fd, always query it directly as it may be modified
    * elsewhere by nasty users.
    */
   valuelen = sizeof(value);
   if (getsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &value, &valuelen) < 0)
     {
        ERR("getsockopt(%d, SOL_SOCKET, SO_KEEPALIVE): %s",
            fd, strerror(errno));
        return EINA_FALSE;
     }

   pd->keep_alive = !!value; /* sync */
   return pd->keep_alive;
}

EOLIAN static Eina_Bool
_efl_net_socket_tcp_no_delay_set(Eo *o, Efl_Net_Socket_Tcp_Data *pd, Eina_Bool no_delay)
{
   int value, fd;
   Eina_Bool old = pd->no_delay;

   pd->no_delay = no_delay;

   fd = efl_loop_fd_get(o);
   if (fd < 0) return EINA_TRUE; /* postpone until fd_set() */

   value = no_delay;
   if (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &value, sizeof(value)) < 0)
     {
        ERR("setsockopt(%d, IPPROTO_TCP, TCP_NODELAY, %d): %s",
            fd, value, strerror(errno));
        pd->no_delay = old;
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_net_socket_tcp_no_delay_get(Eo *o, Efl_Net_Socket_Tcp_Data *pd)
{
   int value = 0, fd;
   socklen_t valuelen;

   fd = efl_loop_fd_get(o);
   if (fd < 0) return pd->no_delay;

   /* if there is a fd, always query it directly as it may be modified
    * elsewhere by nasty users.
    */
   valuelen = sizeof(value);
   if (getsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &value, &valuelen) < 0)
     {
        ERR("getsockopt(%d, IPPROTO_TCP, TCP_NODELAY): %s",
            fd, strerror(errno));
        return EINA_FALSE;
     }

   pd->no_delay = !!value; /* sync */
   return pd->no_delay;
}

EOLIAN static Eina_Bool
_efl_net_socket_tcp_cork_set(Eo *o, Efl_Net_Socket_Tcp_Data *pd, Eina_Bool cork)
{
   int value, fd;
   Eina_Bool old = pd->cork;

   pd->cork = cork;

   fd = efl_loop_fd_get(o);
   if (fd < 0) return EINA_TRUE; /* postpone until fd_set() */

   value = cork;
   if (setsockopt(fd, IPPROTO_TCP, TCP_CORK, &value, sizeof(value)) < 0)
     {
        ERR("setsockopt(%d, IPPROTO_TCP, TCP_CORK, %d): %s",
            fd, value, strerror(errno));
        pd->cork = old;
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_net_socket_tcp_cork_get(Eo *o, Efl_Net_Socket_Tcp_Data *pd)
{
   int value = 0, fd;
   socklen_t valuelen;

   fd = efl_loop_fd_get(o);
   if (fd < 0) return pd->cork;

   /* if there is a fd, always query it directly as it may be modified
    * elsewhere by nasty users.
    */
   valuelen = sizeof(value);
   if (getsockopt(fd, IPPROTO_TCP, TCP_CORK, &value, &valuelen) < 0)
     {
        ERR("getsockopt(%d, IPPROTO_TCP, TCP_CORK): %s",
            fd, strerror(errno));
        return EINA_FALSE;
     }

   pd->cork = !!value; /* sync */
   return pd->cork;
}

#include "efl_net_socket_tcp.eo.c"
