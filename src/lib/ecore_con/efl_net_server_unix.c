#define EFL_NET_SERVER_UNIX_PROTECTED 1
#define EFL_NET_SERVER_FD_PROTECTED 1
#define EFL_NET_SERVER_PROTECTED 1
#define EFL_LOOP_FD_PROTECTED 1

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

#define MY_CLASS EFL_NET_SERVER_UNIX_CLASS

typedef struct _Efl_Net_Server_Unix_Data
{
   unsigned int leading_directories_create_mode;
   Eina_Bool leading_directories_create;
   Eina_Bool unlink_before_bind;
} Efl_Net_Server_Unix_Data;

EOLIAN static void
_efl_net_server_unix_efl_object_destructor(Eo *o, Efl_Net_Server_Unix_Data *pd EINA_UNUSED)
{
   SOCKET fd = efl_loop_fd_get(o);

   if (fd != INVALID_SOCKET)
     {
        const char *address = efl_net_server_address_get(o);
        if ((address) && (strncmp(address, "abstract:", strlen("abstract:")) != 0))
          unlink(address);
     }

   efl_destructor(efl_super(o, MY_CLASS));
}

static Eina_Error
_efl_net_server_unix_bind(Eo *o, Efl_Net_Server_Unix_Data *pd)
{
   const char *address = efl_net_server_address_get(o);
   struct sockaddr_un addr = { .sun_family = AF_UNIX };
   socklen_t addrlen;
   SOCKET fd = INVALID_SOCKET;
   Eina_Error err = 0;
   int r;

   efl_net_server_fd_family_set(o, AF_UNIX);

   if ((pd->leading_directories_create) &&
       (strncmp(address, "abstract:", strlen("abstract:")) != 0))
     _ecore_con_local_mkpath(address, pd->leading_directories_create_mode);

   do
     {
        fd = efl_net_socket4(AF_UNIX, SOCK_STREAM, 0,
                             efl_net_server_fd_close_on_exec_get(o));
        if (fd == INVALID_SOCKET)
          {
             err = efl_net_socket_error_get();
             ERR("socket(AF_UNIX, SOCK_STREAM, 0): %s", eina_error_msg_get(err));
             goto error;
          }

        if (strncmp(address, "abstract:", strlen("abstract:")) == 0)
          {
             const char *path = address + strlen("abstract:");
             if (strlen(path) + 2 > sizeof(addr.sun_path))
               {
                  ERR("abstract path is too long: %s", path);
                  err = EFL_NET_ERROR_COULDNT_RESOLVE_HOST;
               }
             addr.sun_path[0] = '\0';
             memcpy(addr.sun_path + 1, path, strlen(path) + 1);
             addrlen = strlen(path) + 2 + offsetof(struct sockaddr_un, sun_path);
          }
        else
          {
             const char *path = address;
             if (strlen(path) + 1 > sizeof(addr.sun_path))
               {
                  ERR("path is too long: %s", path);
                  err = EFL_NET_ERROR_COULDNT_RESOLVE_HOST;
               }
             memcpy(addr.sun_path, path, strlen(path) + 1);
             addrlen = strlen(path) + 1 + offsetof(struct sockaddr_un, sun_path);
          }

        if ((pd->unlink_before_bind) && (addr.sun_path[0] != '\0'))
          {
             DBG("unlinking AF_UNIX path '%s'", addr.sun_path);
             unlink(addr.sun_path);
          }

        r = bind(fd, (struct sockaddr *)&addr, addrlen);
        if (r != 0)
          {
             err = efl_net_socket_error_get();
             if ((err == EADDRINUSE) && (pd->unlink_before_bind) && (addr.sun_path[0] != '\0'))
               {
                  closesocket(fd);
                  fd = INVALID_SOCKET;
                  err = 0;
                  continue;
               }
             if ((err == EADDRINUSE) && (addr.sun_path[0] != '\0') &&
                 (connect(fd, (struct sockaddr *)&addr, addrlen) != 0))
               {
                  DBG("bind(" SOCKET_FMT ", %s): failed with EADDRINUSE but connect also failed, so unlink socket file and try again", fd, address);
                  closesocket(fd);
                  unlink(addr.sun_path);
                  fd = INVALID_SOCKET;
                  err = 0;
                  continue;
               }
             DBG("bind(" SOCKET_FMT ", %s): %s", fd, address, eina_error_msg_get(err));
             goto error;
          }
        break;
     }
   while (pd->unlink_before_bind);

   if (fd == INVALID_SOCKET) goto error;
   efl_loop_fd_set(o, fd);

   r = listen(fd, 0);
   if (r != 0)
     {
        err = efl_net_socket_error_get();
        DBG("listen(" SOCKET_FMT "): %s", fd, eina_error_msg_get(err));
        goto error;
     }

   addrlen = sizeof(addr);
   if (getsockname(fd, (struct sockaddr *)&addr, &addrlen) != 0)
     {
        err = efl_net_socket_error_get();
        ERR("getsockname(" SOCKET_FMT "): %s", fd, eina_error_msg_get(err));
        goto error;
     }
   else
     {
        char str[sizeof(addr) + sizeof("abstract:")];
        if (!efl_net_unix_fmt(str, sizeof(str), fd, &addr, addrlen))
          ERR("could not format unix address");
        else
          {
             efl_net_server_address_set(o, str);
             address = efl_net_server_address_get(o);
          }
     }

   DBG("fd=" SOCKET_FMT " serving at %s", fd, address);
   efl_net_server_serving_set(o, EINA_TRUE);

 error:
   if (err)
     {
        efl_event_callback_call(o, EFL_NET_SERVER_EVENT_ERROR, &err);
        if (fd != INVALID_SOCKET) closesocket(fd);
        efl_loop_fd_set(o, SOCKET_TO_LOOP_FD(INVALID_SOCKET));
     }
   return err;
}

EOLIAN static Eina_Error
_efl_net_server_unix_efl_net_server_fd_socket_activate(Eo *o, Efl_Net_Server_Unix_Data *pd EINA_UNUSED, const char *address)
{
   EINA_SAFETY_ON_TRUE_RETURN_VAL((SOCKET)efl_loop_fd_get(o) != INVALID_SOCKET, EALREADY);
   EINA_SAFETY_ON_NULL_RETURN_VAL(address, EINVAL);

#ifndef HAVE_SYSTEMD
   return efl_net_server_fd_socket_activate(efl_super(o, MY_CLASS), address);
#else
   {
      char buf[INET6_ADDRSTRLEN + sizeof("[]:65536")];
      Eina_Bool listening;
      Eina_Error err;
      struct sockaddr_storage addr;
      socklen_t addrlen;
      SOCKET fd;

      err = efl_net_ip_socket_activate_check(address, AF_UNIX, SOCK_STREAM, &listening);
      if (err) return err;

      err = efl_net_server_fd_socket_activate(efl_super(o, MY_CLASS), address);
      if (err) return err;

      fd = efl_loop_fd_get(o);

      if (!listening)
        {
           if (listen(fd, 0) != 0)
             {
                err = efl_net_socket_error_get();
                DBG("listen(" SOCKET_FMT "): %s", fd, eina_error_msg_get(err));
                goto error;
             }
        }

      addrlen = sizeof(addr);
      if (getsockname(fd, (struct sockaddr *)&addr, &addrlen) != 0)
        {
           err = efl_net_socket_error_get();
           ERR("getsockname(" SOCKET_FMT "): %s", fd, eina_error_msg_get(err));
           goto error;
        }
      else if (efl_net_ip_port_fmt(buf, sizeof(buf), (struct sockaddr *)&addr))
        efl_net_server_address_set(o, buf);

      DBG("fd=" SOCKET_FMT " serving at %s", fd, address);
      efl_net_server_serving_set(o, EINA_TRUE);
      return 0;

   error:
      efl_net_server_fd_family_set(o, AF_UNSPEC);
      efl_loop_fd_set(o, SOCKET_TO_LOOP_FD(INVALID_SOCKET));
      closesocket(fd);
      return err;
   }
#endif
}

EOLIAN static Eina_Error
_efl_net_server_unix_efl_net_server_serve(Eo *o, Efl_Net_Server_Unix_Data *pd, const char *address)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(address, EINVAL);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(address[0] == '\0', EINVAL);

   efl_net_server_address_set(o, address);

   return _efl_net_server_unix_bind(o, pd);
}

static void
_efl_net_server_unix_efl_net_server_fd_client_add(Eo *o, Efl_Net_Server_Unix_Data *pd EINA_UNUSED, int client_fd)
{
   Eo *client = efl_add(EFL_NET_SOCKET_UNIX_CLASS, o,
                        efl_io_closer_close_on_exec_set(efl_added, efl_net_server_fd_close_on_exec_get(o)),
                        efl_io_closer_close_on_invalidate_set(efl_added, EINA_TRUE),
                        efl_loop_fd_set(efl_added, client_fd));
   if (!client)
     {
        ERR("could not create client object fd=" SOCKET_FMT, (SOCKET)client_fd);
        closesocket(client_fd);
        return;
     }

   efl_net_server_client_announce(o, client);
}

static void
_efl_net_server_unix_efl_net_server_fd_client_reject(Eo *o, Efl_Net_Server_Unix_Data *pd EINA_UNUSED, int client_fd)
{
   struct sockaddr_un addr;
   socklen_t addrlen;
   char str[sizeof(addr) + sizeof("abstract:")] = "";

   addrlen = sizeof(addr);
   if (getpeername(client_fd, (struct sockaddr *)&addr, &addrlen) != 0)
     ERR("getpeername(%d): %s", client_fd, eina_error_msg_get(efl_net_socket_error_get()));
   else
     {
        if (!efl_net_unix_fmt(str, sizeof(str), client_fd, &addr, addrlen))
          ERR("could not format rejected client unix address fd=" SOCKET_FMT, (SOCKET)client_fd);
     }

   closesocket(client_fd);
   efl_event_callback_call(o, EFL_NET_SERVER_EVENT_CLIENT_REJECTED, str);
}

static void
_efl_net_server_unix_unlink_before_bind_set(Eo *o EINA_UNUSED, Efl_Net_Server_Unix_Data *pd, Eina_Bool unlink_before_bind)
{
   pd->unlink_before_bind = unlink_before_bind;
}

static Eina_Bool
_efl_net_server_unix_unlink_before_bind_get(const Eo *o EINA_UNUSED, Efl_Net_Server_Unix_Data *pd)
{
   return pd->unlink_before_bind;
}


static void
_efl_net_server_unix_leading_directories_create_set(Eo *o EINA_UNUSED, Efl_Net_Server_Unix_Data *pd, Eina_Bool do_it, unsigned int mode)
{
   pd->leading_directories_create = do_it;
   pd->leading_directories_create_mode = mode;
}

static void
_efl_net_server_unix_leading_directories_create_get(const Eo *o EINA_UNUSED, Efl_Net_Server_Unix_Data *pd, Eina_Bool *do_it, unsigned int *mode)
{
   if (do_it) *do_it = pd->leading_directories_create;
   if (mode) *mode = pd->leading_directories_create_mode;
}

#include "efl_net_server_unix.eo.c"
