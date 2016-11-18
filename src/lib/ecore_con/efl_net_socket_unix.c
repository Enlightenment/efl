#define EFL_NET_SOCKET_UNIX_PROTECTED 1
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
#ifdef HAVE_SYS_UN_H
#include <sys/un.h>
#endif

/* no include EVIL as it's not supposed to be compiled on Windows */

#define MY_CLASS EFL_NET_SOCKET_UNIX_CLASS

typedef struct _Efl_Net_Socket_Unix_Data
{
} Efl_Net_Socket_Unix_Data;

EOLIAN static void
_efl_net_socket_unix_efl_loop_fd_fd_set(Eo *o, Efl_Net_Socket_Unix_Data *pd EINA_UNUSED, int pfd)
{
   SOCKET fd = (SOCKET)pfd;

   efl_loop_fd_set(efl_super(o, MY_CLASS), pfd);

   if (fd != INVALID_SOCKET)
     {
        struct sockaddr_un addr;
        socklen_t addrlen;
        int family;

        family = efl_net_socket_fd_family_get(o);
        if (family == AF_UNSPEC) return;
        EINA_SAFETY_ON_TRUE_RETURN(family != AF_UNIX);

        addrlen = sizeof(addr);
        if (getsockname(fd, (struct sockaddr *)&addr, &addrlen) != 0)
          ERR("getsockname(" SOCKET_FMT "): %s", fd, eina_error_msg_get(efl_net_socket_error_get()));
        else
          {
             char str[sizeof(addr) + sizeof("abstract:")];

             if (!efl_net_unix_fmt(str, sizeof(str), fd, &addr, addrlen))
               ERR("could not format local unix address");
             else
               efl_net_socket_address_local_set(o, str);
          }

        addrlen = sizeof(addr);
        if (getpeername(fd, (struct sockaddr *)&addr, &addrlen) != 0)
          ERR("getpeername(" SOCKET_FMT "): %s", fd, eina_error_msg_get(efl_net_socket_error_get()));
        else
          {
             char str[sizeof(addr) + sizeof("abstract:")];
             if (!efl_net_unix_fmt(str, sizeof(str), fd, &addr, addrlen))
               ERR("could not format remote unix address");
             else
               efl_net_socket_address_remote_set(o, str);
          }
     }
}

#include "efl_net_socket_unix.eo.c"
