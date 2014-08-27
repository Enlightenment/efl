#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <pwd.h>

#ifdef HAVE_SYSTEMD
# include <systemd/sd-daemon.h>
#endif

#ifdef HAVE_WS2TCPIP_H
# include <ws2tcpip.h>
#endif

#include <Ecore.h>
#include <ecore_private.h>

#include "Ecore_Con.h"
#include "ecore_con_private.h"

#define LENGTH_OF_SOCKADDR_UN(s)                (strlen((s)->sun_path) +                 \
                                                 (size_t)(((struct sockaddr_un *)NULL)-> \
                                                          sun_path))
#define LENGTH_OF_ABSTRACT_SOCKADDR_UN(s, path) (strlen(path) + 1 +            \
                                                 (size_t)(((struct sockaddr_un \
                                                            *)NULL)->sun_path))

static int _ecore_con_local_init_count = 0;

int
ecore_con_local_init(void)
{
   if (++_ecore_con_local_init_count != 1)
     return _ecore_con_local_init_count;

   return _ecore_con_local_init_count;
}

int
ecore_con_local_shutdown(void)
{
   if (--_ecore_con_local_init_count != 0)
     return _ecore_con_local_init_count;

   return _ecore_con_local_init_count;
}

int
ecore_con_local_connect(Ecore_Con_Server *obj,
                        Eina_Bool (*cb_done)(void *data, Ecore_Fd_Handler *fd_handler),
                        void *data EINA_UNUSED)
{
#ifndef HAVE_LOCAL_SOCKETS
   return 0;
#else
   Ecore_Con_Server_Data *svr = eo_data_scope_get(obj, ECORE_CON_SERVER_CLASS);
   char buf[4096];
   struct sockaddr_un socket_unix;
   int curstate = 0;
   const char *homedir;
   int socket_unix_len;

   buf[0] = '\0';

   if ((svr->type & ECORE_CON_TYPE) == ECORE_CON_LOCAL_USER)
     {
#if defined(HAVE_GETUID) && defined(HAVE_GETEUID)
        if (getuid() == geteuid())
#endif
          {
             homedir = getenv("XDG_RUNTIME_DIR");
             if (!homedir)
               {
                  homedir = getenv("HOME");
                  if (!homedir)
                    {
                       homedir = getenv("TMP");
                       if (!homedir) homedir = "/tmp";
                    }
               }
             snprintf(buf, sizeof(buf), "%s/.ecore/%s/%i", homedir, svr->name,
                      svr->port);
          }
#if defined(HAVE_GETUID) && defined(HAVE_GETEUID)
        else
          {
             struct passwd *pw = getpwent();

             if ((!pw) || (!pw->pw_dir))
               snprintf(buf, sizeof(buf), "/tmp/%s/%i", svr->name,
                        svr->port);
             else
               snprintf(buf, sizeof(buf), "%s/.ecore/%s/%i", pw->pw_dir, svr->name,
                        svr->port);
          }
#endif
     }
   else if ((svr->type & ECORE_CON_TYPE) == ECORE_CON_LOCAL_SYSTEM)
     {
        if (svr->port < 0)
          {
             if (svr->name[0] == '/')
               {
                  strncpy(buf, svr->name, sizeof(buf) - 1);
                  buf[sizeof(buf) - 1] = 0;
               }
             else
               snprintf(buf, sizeof(buf), "/tmp/.ecore_service|%s", svr->name);
          }
        else
          {
             if (svr->name[0] ==
                 '/')
               snprintf(buf, sizeof(buf), "%s|%i", svr->name,
                        svr->port);
             else
               snprintf(buf, sizeof(buf), "/tmp/.ecore_service|%s|%i",
                        svr->name,
                        svr->port);
          }
     }
   else if ((svr->type & ECORE_CON_TYPE) == ECORE_CON_LOCAL_ABSTRACT)
     {
        strncpy(buf, svr->name, sizeof(buf) - 1);
        buf[sizeof(buf) - 1] = 0;
     }

   svr->fd = socket(AF_UNIX, SOCK_STREAM, 0);
   if (svr->fd < 0)
     return 0;

   if (fcntl(svr->fd, F_SETFL, O_NONBLOCK) < 0)
     return 0;

   if (fcntl(svr->fd, F_SETFD, FD_CLOEXEC) < 0)
     return 0;

   if (setsockopt(svr->fd, SOL_SOCKET, SO_REUSEADDR, (const void *)&curstate,
                  sizeof(curstate)) < 0)
     return 0;

   socket_unix.sun_family = AF_UNIX;

   if ((svr->type & ECORE_CON_TYPE) == ECORE_CON_LOCAL_ABSTRACT)
     {
#ifdef HAVE_ABSTRACT_SOCKETS
        /* copy name insto sun_path, prefixed by null to indicate abstract namespace */
        snprintf(socket_unix.sun_path, sizeof(socket_unix.sun_path), ".%s",
                 svr->name);
        socket_unix.sun_path[0] = '\0';
        socket_unix_len = LENGTH_OF_ABSTRACT_SOCKADDR_UN(&socket_unix,
                                                         svr->name);
#else
        WRN("Your system does not support abstract sockets!");
        return 0;
#endif
     }
   else
     {
        strncpy(socket_unix.sun_path, buf, sizeof(socket_unix.sun_path));
        socket_unix_len = LENGTH_OF_SOCKADDR_UN(&socket_unix);
     }

   if (connect(svr->fd, (struct sockaddr *)&socket_unix,
               socket_unix_len) < 0)
     {
        DBG("local connection failed: %s", strerror(errno));
        return 0;
     }

   svr->path = strdup(buf);
   if (!svr->path)
     return 0;

   if (svr->type & ECORE_CON_SSL)
     {
        if (!ecore_con_ssl_server_init(obj)) ERR("Can't init SSL");
     }

   svr->fd_handler =
     ecore_main_fd_handler_add(svr->fd, ECORE_FD_READ,
                               cb_done, obj, NULL, NULL);
   if (!svr->fd_handler)
     return 0;

   if (!svr->delete_me) ecore_con_event_server_add(obj);

   return 1;
#endif
}

int
ecore_con_local_listen(
  Ecore_Con_Server *obj,
  Eina_Bool (*
             cb_listen)(void *data,
                        Ecore_Fd_Handler *
                        fd_handler),
  void *data
  EINA_UNUSED)
{
#ifdef HAVE_LOCAL_SOCKETS
   Ecore_Con_Server_Data *svr = eo_data_scope_get(obj, ECORE_CON_SERVER_CLASS);
   char buf[4096];
   struct sockaddr_un socket_unix;
   struct linger lin;
   mode_t pmode;
   const char *homedir;
   struct stat st;
   mode_t mask;
   int socket_unix_len;
   Eina_Bool abstract_socket;

   mask = S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IWOTH | S_IXOTH;

   if ((svr->type & ECORE_CON_TYPE) == ECORE_CON_LOCAL_USER)
     {
#if defined(HAVE_GETUID) && defined(HAVE_GETEUID)
        if (getuid() == geteuid())
#endif
          {
             homedir = getenv("XDG_RUNTIME_DIR");
             if (!homedir)
               {
                  homedir = getenv("HOME");
                  if (!homedir)
                    {
                       homedir = getenv("TMP");
                       if (!homedir) homedir = "/tmp";
                    }
               }
          }
#if defined(HAVE_GETUID) && defined(HAVE_GETEUID)
        else
          {
             struct passwd *pw = getpwent();

             if ((!pw) || (!pw->pw_dir)) homedir = "/tmp";
             else homedir = pw->pw_dir;
          }
#endif
        mask = S_IRUSR | S_IWUSR | S_IXUSR;
        snprintf(buf, sizeof(buf), "%s/.ecore", homedir);
        if (stat(buf, &st) < 0)
          mkdir(buf, mask);

        snprintf(buf, sizeof(buf), "%s/.ecore/%s", homedir, svr->name);
        if (stat(buf, &st) < 0)
          mkdir(buf, mask);

        snprintf(buf,
                 sizeof(buf),
                 "%s/.ecore/%s/%i",
                 homedir,
                 svr->name,
                 svr->port);
        mask = S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IWOTH | S_IXOTH;
     }
   else if ((svr->type & ECORE_CON_TYPE) == ECORE_CON_LOCAL_SYSTEM)
     {
        mask = 0;
        if (svr->name[0] == '/')
          {
             if (svr->port >= 0)
               snprintf(buf,
                        sizeof(buf),
                        "%s|%i",
                        svr->name,
                        svr->port);
             else
               snprintf(buf,
                        sizeof(buf),
                        "%s",
                        svr->name);
          }
        else
          snprintf(buf,
                   sizeof(buf),
                   "/tmp/.ecore_service|%s|%i",
                   svr->name,
                   svr->port);
     }
   else if ((svr->type & ECORE_CON_TYPE) == ECORE_CON_LOCAL_ABSTRACT)
     {
        strncpy(buf, svr->name, sizeof(buf) - 1);
        buf[sizeof(buf) - 1] = 0;
     }

   pmode = umask(mask);
start:
   socket_unix.sun_family = AF_UNIX;
   if ((svr->type & ECORE_CON_TYPE) == ECORE_CON_LOCAL_ABSTRACT)
     {
        abstract_socket = EINA_TRUE;
#ifdef HAVE_ABSTRACT_SOCKETS
        /* . is a placeholder */
        snprintf(socket_unix.sun_path, sizeof(socket_unix.sun_path), ".%s",
                 svr->name);
        /* first char null indicates abstract namespace */
        socket_unix.sun_path[0] = '\0';
        socket_unix_len = LENGTH_OF_ABSTRACT_SOCKADDR_UN(&socket_unix,
                                                         svr->name);
#else
        ERR("Your system does not support abstract sockets!");
        goto error_umask;
#endif
     }
   else
     {
        abstract_socket = EINA_FALSE;
        strncpy(socket_unix.sun_path, buf, sizeof(socket_unix.sun_path));
        socket_unix_len = LENGTH_OF_SOCKADDR_UN(&socket_unix);
     }

#ifdef HAVE_SYSTEMD
   if (svr->type & ECORE_CON_SOCKET_ACTIVATE && sd_fd_index < sd_fd_max)
     {
        if (sd_is_socket_unix(SD_LISTEN_FDS_START + sd_fd_index,
                              SOCK_STREAM, 1,
                              socket_unix.sun_path,
                              abstract_socket ? socket_unix_len : 0) <= 0)
          {
             ERR("Your systemd unit seems to provide fd in the wrong order for Socket activation.");
             goto error_umask;
          }
        svr->fd = SD_LISTEN_FDS_START + sd_fd_index++;

        if (fcntl(svr->fd, F_SETFL, O_NONBLOCK) < 0)
          goto error_umask;

        lin.l_onoff = 1;
        lin.l_linger = 0;
        if (setsockopt(svr->fd, SOL_SOCKET, SO_LINGER, (const void *)&lin,
                       sizeof(struct linger)) < 0)
          goto error_umask;

        goto fd_ready;
     }
#else
   (void)abstract_socket;
#endif
   svr->fd = socket(AF_UNIX, SOCK_STREAM, 0);
   if (svr->fd < 0)
     goto error_umask;

   if (fcntl(svr->fd, F_SETFL, O_NONBLOCK) < 0)
     goto error_umask;

   if (fcntl(svr->fd, F_SETFD, FD_CLOEXEC) < 0)
     goto error_umask;

   lin.l_onoff = 1;
   lin.l_linger = 0;
   if (setsockopt(svr->fd, SOL_SOCKET, SO_LINGER, (const void *)&lin,
                  sizeof(struct linger)) < 0)
     goto error_umask;

   if (bind(svr->fd, (struct sockaddr *)&socket_unix, socket_unix_len) < 0)
     {
        if ((((svr->type & ECORE_CON_TYPE) == ECORE_CON_LOCAL_USER) ||
             ((svr->type & ECORE_CON_TYPE) == ECORE_CON_LOCAL_SYSTEM)) &&
            (connect(svr->fd, (struct sockaddr *)&socket_unix,
                     socket_unix_len) < 0) &&
            (unlink(buf) >= 0))
          goto start;
        else
          goto error_umask;
     }

   if (listen(svr->fd, 4096) < 0)
     goto error_umask;

#ifdef HAVE_SYSTEMD
fd_ready:
#endif
   svr->path = strdup(buf);
   if (!svr->path)
     goto error_umask;

   svr->fd_handler =
     ecore_main_fd_handler_add(svr->fd, ECORE_FD_READ,
                               cb_listen, obj, NULL, NULL);
   umask(pmode);
   if (!svr->fd_handler)
     goto error;

   return 1;

error_umask:
   umask(pmode);
error:
#endif /* HAVE_LOCAL_SOCKETS */
   return 0;
}

