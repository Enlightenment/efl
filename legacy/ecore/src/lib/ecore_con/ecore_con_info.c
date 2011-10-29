/*
 * getaddrinfo with callback
 *
 * man getaddrinfo
 *
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef HAVE_ALLOCA_H
# include <alloca.h>
#elif defined __GNUC__
# define alloca __builtin_alloca
#elif defined _AIX
# define alloca __alloca
#elif defined _MSC_VER
# include <malloc.h>
# define alloca _alloca
#else
# include <stddef.h>
# ifdef  __cplusplus
extern "C"
# endif
void *alloca(size_t);
#endif

#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <ctype.h>
#ifdef __OpenBSD__
# include <sys/types.h>
#endif

#ifdef HAVE_ARPA_INET_H
# include <arpa/inet.h>
#endif

#ifdef HAVE_ARPA_NAMESER_H
# include <arpa/nameser.h>
#endif

#ifdef HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif

#ifdef HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif

#ifdef HAVE_NETDB_H
# include <netdb.h>
#endif

#include <errno.h>

#include "Ecore.h"
#include "ecore_private.h"
#include "ecore_con_private.h"

typedef struct _CB_Data CB_Data;

struct _CB_Data
{
                        EINA_INLIST;
   Ecore_Con_Info_Cb    cb_done;
   void                *data;
   Ecore_Fd_Handler    *fdh;
   pid_t                pid;
   Ecore_Event_Handler *handler;
   int                  fd2;
};

static void      _ecore_con_info_readdata(CB_Data *cbdata);
static void      _ecore_con_info_slave_free(CB_Data *cbdata);
static Eina_Bool _ecore_con_info_data_handler(void             *data,
                                              Ecore_Fd_Handler *fd_handler);
static Eina_Bool _ecore_con_info_exit_handler(void    *data,
                                              int type __UNUSED__,
                                              void    *event);

static int info_init = 0;
static CB_Data *info_slaves = NULL;

int
ecore_con_info_init(void)
{
   info_init++;
   return info_init;
}

int
ecore_con_info_shutdown(void)
{
   info_init--;
   if (info_init == 0)
     while (info_slaves) _ecore_con_info_slave_free(info_slaves);

   return info_init;
}

int
ecore_con_info_tcp_connect(Ecore_Con_Server *svr,
                           Ecore_Con_Info_Cb done_cb,
                           void             *data)
{
   struct addrinfo hints;

   memset(&hints, 0, sizeof(struct addrinfo));
   hints.ai_family = AF_UNSPEC;
   hints.ai_socktype = SOCK_STREAM;
   hints.ai_flags = AI_CANONNAME;
   hints.ai_protocol = IPPROTO_TCP;
   hints.ai_canonname = NULL;
   hints.ai_next = NULL;
   hints.ai_addr = NULL;

   return ecore_con_info_get(svr, done_cb, data, &hints);
}

int
ecore_con_info_tcp_listen(Ecore_Con_Server *svr,
                          Ecore_Con_Info_Cb done_cb,
                          void             *data)
{
   struct addrinfo hints;

   memset(&hints, 0, sizeof(struct addrinfo));
   hints.ai_family = AF_UNSPEC;
   hints.ai_socktype = SOCK_STREAM;
   hints.ai_flags = AI_PASSIVE;
   hints.ai_protocol = IPPROTO_TCP;
   hints.ai_canonname = NULL;
   hints.ai_next = NULL;
   hints.ai_addr = NULL;

   return ecore_con_info_get(svr, done_cb, data, &hints);
}

int
ecore_con_info_udp_connect(Ecore_Con_Server *svr,
                           Ecore_Con_Info_Cb done_cb,
                           void             *data)
{
   struct addrinfo hints;

   memset(&hints, 0, sizeof(struct addrinfo));
   hints.ai_family = AF_UNSPEC;
   hints.ai_socktype = SOCK_DGRAM;
   hints.ai_flags = AI_CANONNAME;
   hints.ai_protocol = IPPROTO_UDP;
   hints.ai_canonname = NULL;
   hints.ai_next = NULL;
   hints.ai_addr = NULL;

   return ecore_con_info_get(svr, done_cb, data, &hints);
}

int
ecore_con_info_udp_listen(Ecore_Con_Server *svr,
                          Ecore_Con_Info_Cb done_cb,
                          void             *data)
{
   struct addrinfo hints;

   memset(&hints, 0, sizeof(struct addrinfo));
   hints.ai_family = AF_UNSPEC;
   hints.ai_socktype = SOCK_DGRAM;
   hints.ai_flags = AI_PASSIVE;
   hints.ai_protocol = IPPROTO_UDP;
   hints.ai_canonname = NULL;
   hints.ai_next = NULL;
   hints.ai_addr = NULL;

   return ecore_con_info_get(svr, done_cb, data, &hints);
}

int
ecore_con_info_mcast_listen(Ecore_Con_Server *svr,
                            Ecore_Con_Info_Cb done_cb,
                            void             *data)
{
   struct addrinfo hints;

   memset(&hints, 0, sizeof(struct addrinfo));
   hints.ai_family = AF_UNSPEC;
   hints.ai_socktype = SOCK_DGRAM;
   hints.ai_flags = 0;
   hints.ai_protocol = IPPROTO_UDP;
   hints.ai_canonname = NULL;
   hints.ai_next = NULL;
   hints.ai_addr = NULL;

   return ecore_con_info_get(svr, done_cb, data, &hints);
}

EAPI int
ecore_con_info_get(Ecore_Con_Server *svr,
                   Ecore_Con_Info_Cb done_cb,
                   void             *data,
                   struct addrinfo  *hints)
{
   CB_Data *cbdata;
   int fd[2];

   if (pipe(fd) < 0)
     {
        ecore_con_event_server_error(svr, strerror(errno));
        return 0;
     }

   cbdata = calloc(1, sizeof(CB_Data));
   if (!cbdata)
     {
        close(fd[0]);
        close(fd[1]);
        return 0;
     }

   cbdata->cb_done = done_cb;
   cbdata->data = data;
   cbdata->fd2 = fd[1];
   if (!(cbdata->fdh = ecore_main_fd_handler_add(fd[0], ECORE_FD_READ,
                                                 _ecore_con_info_data_handler,
                                                 cbdata,
                                                 NULL, NULL)))
     {
        ecore_con_event_server_error(svr, "Memory allocation failure");
        free(cbdata);
        close(fd[0]);
        close(fd[1]);
        return 0;
     }

   if ((cbdata->pid = fork()) == 0)
     {
        Ecore_Con_Info *container;
        struct addrinfo *result = NULL;
        char service[NI_MAXSERV] = {0};
        char hbuf[NI_MAXHOST] = {0};
        char sbuf[NI_MAXSERV] = {0};
        unsigned char *tosend = NULL;
        int tosend_len;
        int canonname_len = 0;
        int err;

        eina_convert_itoa(svr->port, service);
        /* CHILD */
        if (!getaddrinfo(svr->name, service, hints, &result) && result)
          {
             if (result->ai_canonname)
               canonname_len = strlen(result->ai_canonname) + 1;

             tosend_len = sizeof(Ecore_Con_Info) + result->ai_addrlen +
               canonname_len;

             tosend = alloca(tosend_len);
             memset(tosend, 0, tosend_len);

             container = (Ecore_Con_Info *)tosend;
             container->size = tosend_len;

             memcpy(&container->info,
                    result,
                    sizeof(struct addrinfo));
             memcpy(tosend + sizeof(Ecore_Con_Info),
                    result->ai_addr,
                    result->ai_addrlen);
             if (result->ai_canonname) /* FIXME: else... */
               memcpy(tosend + sizeof(Ecore_Con_Info) + result->ai_addrlen,
                      result->ai_canonname,
                      canonname_len);

             if (!getnameinfo(result->ai_addr, result->ai_addrlen,
                              hbuf, sizeof(hbuf), sbuf, sizeof(sbuf),
                              NI_NUMERICHOST | NI_NUMERICSERV))
               {
                  memcpy(container->ip, hbuf, sizeof(container->ip));
                  memcpy(container->service, sbuf, sizeof(container->service));
               }

             err = write(fd[1], tosend, tosend_len);
          }

        if (result)
          freeaddrinfo(result);

        err = write(fd[1], "", 1);
        close(fd[1]);
#if defined(__USE_ISOC99) && !defined(__UCLIBC__)
        _Exit(0);
#else
        _exit(0);
#endif
     }

   /* PARENT */
   cbdata->handler =
     ecore_event_handler_add(ECORE_EXE_EVENT_DEL, _ecore_con_info_exit_handler,
                             cbdata);
   close(fd[1]);
   if (!cbdata->handler)
     {
        ecore_main_fd_handler_del(cbdata->fdh);
        free(cbdata);
        close(fd[0]);
        return 0;
     }

   info_slaves = (CB_Data *)eina_inlist_append(EINA_INLIST_GET(
                                                 info_slaves),
                                               EINA_INLIST_GET(cbdata));
   svr->infos = eina_list_append(svr->infos, cbdata);
   return 1;
}

void
ecore_con_info_data_clear(void *info)
{
   CB_Data *cbdata = info;
   cbdata->data = NULL;
}

static void
_ecore_con_info_readdata(CB_Data *cbdata)
{
   Ecore_Con_Info container;
   Ecore_Con_Info *recv_info;
   unsigned char *torecv;
   int torecv_len;

   ssize_t size;

   size = read(ecore_main_fd_handler_fd_get(cbdata->fdh), &container,
               sizeof(Ecore_Con_Info));
   if (size == sizeof(Ecore_Con_Info))
     {
        torecv_len = container.size;
        torecv = malloc(torecv_len);

        memcpy(torecv, &container, sizeof(Ecore_Con_Info));

        size = read(ecore_main_fd_handler_fd_get(cbdata->fdh),
                    torecv + sizeof(Ecore_Con_Info),
                    torecv_len - sizeof(Ecore_Con_Info));
        if ((size > 0) &&
            ((size_t)size == torecv_len - sizeof(Ecore_Con_Info)))
          {
             recv_info = (Ecore_Con_Info *)torecv;

             recv_info->info.ai_addr =
               (struct sockaddr *)(torecv + sizeof(Ecore_Con_Info));
             if ((size_t)torecv_len !=
                 (sizeof(Ecore_Con_Info) + recv_info->info.ai_addrlen))
               recv_info->info.ai_canonname = (char *)
                 (torecv + sizeof(Ecore_Con_Info) + recv_info->info.ai_addrlen);
             else
               recv_info->info.ai_canonname = NULL;

             recv_info->info.ai_next = NULL;

             if (cbdata->data)
               {
                  cbdata->cb_done(cbdata->data, recv_info);
                  ecore_con_server_infos_del(cbdata->data, cbdata);
               }

             free(torecv);
          }
        else
          {
             if (cbdata->data)
               {
                  cbdata->cb_done(cbdata->data, NULL);
                  ecore_con_server_infos_del(cbdata->data, cbdata);
               }
          }
     }
   else
     {
        if (cbdata->data)
          {
             ecore_con_event_server_error(cbdata->data, strerror(errno));
             cbdata->cb_done(cbdata->data, NULL);
             ecore_con_server_infos_del(cbdata->data, cbdata);
          }
     }

   cbdata->cb_done = NULL;
}

static void
_ecore_con_info_slave_free(CB_Data *cbdata)
{
   info_slaves = (CB_Data *)eina_inlist_remove(EINA_INLIST_GET(info_slaves),
                                               EINA_INLIST_GET(cbdata));
   ecore_main_fd_handler_del(cbdata->fdh);
   ecore_event_handler_del(cbdata->handler);
   close(ecore_main_fd_handler_fd_get(cbdata->fdh));
   if (cbdata->data) ecore_con_server_infos_del(cbdata->data, cbdata);
   free(cbdata);
}

static Eina_Bool
_ecore_con_info_data_handler(void             *data,
                             Ecore_Fd_Handler *fd_handler)
{
   CB_Data *cbdata;

   cbdata = data;
   if (cbdata->cb_done)
     {
        if (ecore_main_fd_handler_active_get(fd_handler,
                                             ECORE_FD_READ))
          _ecore_con_info_readdata(cbdata);
        else
          {
             if (cbdata->data)
               {
                  cbdata->cb_done(cbdata->data, NULL);
                  cbdata->cb_done = NULL;
                  ecore_con_server_infos_del(cbdata->data, cbdata);
               }
          }
     }

   _ecore_con_info_slave_free(cbdata);
   return ECORE_CALLBACK_CANCEL;
}

static Eina_Bool
_ecore_con_info_exit_handler(void    *data,
                             int type __UNUSED__,
                             void    *event)
{
   CB_Data *cbdata;
   Ecore_Exe_Event_Del *ev;

   ev = event;
   cbdata = data;
   if (cbdata->pid != ev->pid)
     return ECORE_CALLBACK_RENEW;

   return ECORE_CALLBACK_CANCEL; /* FIXME: Woot ??? */
   _ecore_con_info_slave_free(cbdata);
   return ECORE_CALLBACK_CANCEL;
}

