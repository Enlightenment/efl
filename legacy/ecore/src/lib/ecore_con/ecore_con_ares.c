#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

/*
 * This version of ecore_con_info use c-ares to provide asynchronous dns lookup.
 *
 * Note: It doesn't fork nor does it use libc getaddrinfo.
 * http://c-ares.haxx.se/docs.html
 */

#include <string.h>
#include <arpa/inet.h>
#include <ares.h>

#include "Ecore.h"
#include "Ecore_Con.h"
#include "ecore_con_private.h"

typedef struct _Ecore_Con_FD    Ecore_Con_FD;
typedef struct _Ecore_Con_CAres Ecore_Con_CAres;

struct _Ecore_Con_FD
{
   Ecore_Fd_Handler *handler;
   int               active;
   int               fd;
};

struct _Ecore_Con_CAres
{
   Ecore_Con_Server *svr;
   Ecore_Con_Info_Cb done_cb;
   void             *data;
   struct addrinfo   hints;
   Ecore_Con_Info   *result;

   union {
      struct in_addr  v4;
      struct in6_addr v6;
   } addr;

   Eina_Bool byaddr : 1;
   Eina_Bool isv6 : 1;
};

static ares_channel info_channel;
static int info_init = 0;
static Eina_List *info_fds = NULL;
static int active = 0;
static Ecore_Timer *tm = NULL;
static fd_set info_readers, info_writers;

static void _ecore_con_info_ares_nameinfo(Ecore_Con_CAres *arg,
                                          int              status,
                                          int              timeouts,
                                          char            *node,
                                          char            *service);
static void _ecore_con_info_ares_host_cb(Ecore_Con_CAres *arg,
                                         int              status,
                                         int              timeouts,
                                         struct hostent  *hostent);
static Eina_Bool _ecore_con_info_cares_fd_cb(void             *data,
                                             Ecore_Fd_Handler *fd_handler);
static Eina_Bool _ecore_con_info_cares_timeout_cb(void *data);
static void      _ecore_con_info_cares_clean(void);

int
ecore_con_info_init(void)
{
   if (info_init == 0)
     {
        if (ares_library_init(ARES_LIB_INIT_ALL) != 0)
          return 0;

        if (ares_init(&info_channel) != ARES_SUCCESS)
          {
             ares_library_cleanup();
             return 0;
          }
     }

   info_init++;
   return info_init;
}

int
ecore_con_info_shutdown(void)
{
   info_init--;
   if (info_init == 0)
     {
        /* Cancel all ongoing request */
         ares_cancel(info_channel);
         ares_destroy(info_channel);

         /* Destroy FD handler here. */
         /* Shutdown ares */
         ares_library_cleanup();
     }

   return info_init;
}

int
ecore_con_info_tcp_connect(Ecore_Con_Server *svr,
                           Ecore_Con_Info_Cb done_cb,
                           void             *data)
{
   struct addrinfo hints;

   memset(&hints, 0, sizeof(struct addrinfo));
   hints.ai_family = AF_INET6;
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
   hints.ai_family = AF_INET6;
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
   hints.ai_family = AF_INET6;
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
   hints.ai_family = AF_INET6;
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
   hints.ai_family = AF_INET6;
   hints.ai_socktype = SOCK_DGRAM;
   hints.ai_flags = 0;
   hints.ai_protocol = IPPROTO_UDP;
   hints.ai_canonname = NULL;
   hints.ai_next = NULL;
   hints.ai_addr = NULL;

   return ecore_con_info_get(svr, done_cb, data, &hints);
}

static Eina_Bool
_ecore_con_info_ares_getnameinfo(Ecore_Con_CAres *arg,
                                 int              addrtype,
                                 const char      *name,
                                 struct sockaddr *addr,
                                 int              addrlen)
{
   int length = 0;

   if (name)
     length = strlen(name) + 1;
   else
     length = 1;

   arg->result = malloc(sizeof(Ecore_Con_Info) + length);
   if (!arg->result)
     return EINA_FALSE;

   /* FIXME: What to do when hint is not set ? */
   arg->result->info.ai_flags = arg->hints.ai_flags;
   arg->result->info.ai_socktype = arg->hints.ai_socktype;
   arg->result->info.ai_protocol = arg->hints.ai_protocol;

   arg->result->info.ai_family = addrtype;
   arg->result->info.ai_addrlen = addrlen;
   arg->result->info.ai_addr = addr;
   arg->result->info.ai_canonname = (char *)(arg->result + 1);

   if (!name)
     *arg->result->info.ai_canonname = '\0';
   else
     strcpy(arg->result->info.ai_canonname, name);

   arg->result->info.ai_next = NULL;

   ares_getnameinfo(
     info_channel, addr, addrlen,
     ARES_NI_NUMERICSERV | ARES_NI_NUMERICHOST |
     ARES_NI_LOOKUPSERVICE | ARES_NI_LOOKUPHOST,
     (ares_nameinfo_callback)_ecore_con_info_ares_nameinfo, arg);

   return EINA_TRUE;
}

EAPI int
ecore_con_info_get(Ecore_Con_Server *svr,
                   Ecore_Con_Info_Cb done_cb,
                   void             *data,
                   struct addrinfo  *hints)
{
   Ecore_Con_CAres *cares;
   int ai_family = AF_INET6;

   cares = calloc(1, sizeof(Ecore_Con_CAres));
   if (!cares)
     return 0;

   cares->svr = svr;
   cares->done_cb = done_cb;
   cares->data = data;

   if (hints)
     {
        ai_family = hints->ai_family;
        memcpy(&cares->hints, hints, sizeof(struct addrinfo));
     }

   if (inet_pton(AF_INET, svr->name, &cares->addr.v4) == 1)
     {
        cares->byaddr = EINA_TRUE;
        cares->isv6 = EINA_FALSE;
        ares_gethostbyaddr(info_channel, &cares->addr.v4,
                           sizeof(cares->addr.v4),
                           AF_INET,
                           (ares_host_callback)_ecore_con_info_ares_host_cb,
                           cares);
     }
   else if (inet_pton(AF_INET6, svr->name, &cares->addr.v6) == 1)
     {
        cares->byaddr = EINA_TRUE;
        cares->isv6 = EINA_TRUE;
        ares_gethostbyaddr(info_channel, &cares->addr.v6,
                           sizeof(cares->addr.v6),
                           AF_INET6,
                           (ares_host_callback)_ecore_con_info_ares_host_cb,
                           cares);
     }
   else
     {
        cares->byaddr = EINA_FALSE;
        ares_gethostbyname(info_channel, svr->name, ai_family,
                           (ares_host_callback)_ecore_con_info_ares_host_cb,
                           cares);
     }

   _ecore_con_info_cares_clean();

   return 1;
}

static int
_ecore_con_info_fds_search(const Ecore_Con_FD *fd1,
                           const Ecore_Con_FD *fd2)
{
   return fd1->fd - fd2->fd;
}

static Eina_Bool
_ecore_con_info_fds_lookup(int fd)
{
   Ecore_Con_FD fdl;
   Ecore_Con_FD *search;

   fdl.fd = fd;

   search = eina_list_search_unsorted(
       info_fds, (Eina_Compare_Cb)_ecore_con_info_fds_search, &fdl);

   if (search)
     {
        search->active = active;
        return EINA_TRUE;
     }

   return EINA_FALSE;
}

static void
_ecore_con_info_cares_clean(void)
{
   fd_set readers, writers;
   Eina_List *l, *l_next;
   Ecore_Con_FD *ecf;
   int nfds;
   int i;

   FD_ZERO(&readers);
   FD_ZERO(&writers);
   nfds = ares_fds(info_channel, &readers, &writers);

   active++;
   for (i = 0; i < nfds; ++i)
     {
        int flags = 0;

        if (FD_ISSET(i, &readers))
          flags |= ECORE_FD_READ;

        if (FD_ISSET(i, &writers))
          flags |= ECORE_FD_WRITE;

        if (flags &&  (!_ecore_con_info_fds_lookup(i)))
          {
             ecf = malloc(sizeof(Ecore_Con_FD));
             if (ecf)
               {
                  ecf->fd = i;
                  ecf->active = active;
                  ecf->handler = ecore_main_fd_handler_add(
                      i, ECORE_FD_WRITE | ECORE_FD_READ,
                      _ecore_con_info_cares_fd_cb,
                      NULL, NULL, NULL);
                  info_fds = eina_list_append(info_fds, ecf);
               }
          }
     }

   info_readers = readers;
   info_writers = writers;

   EINA_LIST_FOREACH_SAFE(info_fds, l, l_next, ecf)
     {
        if (ecf->active != active)
          {
             if (ecf->handler) ecore_main_fd_handler_del(ecf->handler);
             free(ecf);
             info_fds = eina_list_remove_list(info_fds, l);
          }
     }

   if (!info_fds)
     {
        if (tm)
          ecore_timer_del(tm);

        tm = NULL;
     }
   else
     {
        struct timeval tv;

        ares_timeout(info_channel, NULL, &tv);

        if (tm)
          ecore_timer_delay(tm, tv.tv_sec);
        else
          tm =
            ecore_timer_add((double)tv.tv_sec,
                            _ecore_con_info_cares_timeout_cb,
                            NULL);
     }
}

static Eina_Bool
_ecore_con_info_cares_timeout_cb(void *data __UNUSED__)
{
   ares_process(info_channel, &info_readers, &info_writers);
   _ecore_con_info_cares_clean();

   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_ecore_con_info_cares_fd_cb(void             *data  __UNUSED__,
                            Ecore_Fd_Handler *fd_handler  __UNUSED__)
{
   ares_process(info_channel, &info_readers, &info_writers);
   _ecore_con_info_cares_clean();

   return ECORE_CALLBACK_RENEW;
}

static void
_ecore_con_info_ares_host_cb(Ecore_Con_CAres *arg,
                             int              status,
                             int              timeouts  __UNUSED__,
                             struct hostent  *hostent)
{
   struct sockaddr *addr;
   int addrlen;

   /* Found something ? */
   switch (status)
     {
      case ARES_SUCCESS:
        if (!hostent->h_addr_list[0])
          {
             ERR("No IP found");
             goto on_error;
          }

        switch (hostent->h_addrtype)
          {
           case AF_INET:
           {
              struct sockaddr_in *addri;

              addrlen = sizeof(struct sockaddr_in);
              addri = malloc(addrlen);

              if (!addri)
                goto on_mem_error;

              addri->sin_family = AF_INET;
              addri->sin_port = htons(arg->svr->port);

              memcpy(&addri->sin_addr.s_addr,
                     hostent->h_addr_list[0], sizeof(struct in_addr));

              addr = (struct sockaddr *)addri;
              break;
           }

           case AF_INET6:
           {
              struct sockaddr_in6 *addri6;

              addrlen = sizeof(struct sockaddr_in6);
              addri6 = malloc(addrlen);

              if (!addri6)
                goto on_mem_error;

              addri6->sin6_family = AF_INET6;
              addri6->sin6_port = htons(arg->svr->port);
              addri6->sin6_flowinfo = 0;
              addri6->sin6_scope_id = 0;

              memcpy(&addri6->sin6_addr.s6_addr,
                     hostent->h_addr_list[0], sizeof(struct in6_addr));

              addr = (struct sockaddr *)addri6;
              break;
           }

           default:
             ERR("Unknown addrtype %i", hostent->h_addrtype);
             goto on_error;
          }

        if (!_ecore_con_info_ares_getnameinfo(arg, hostent->h_addrtype,
                                              hostent->h_name,
                                              addr, addrlen))
          goto on_error;

        break;

      case ARES_ENOTFOUND: /* address notfound */
        if (arg->byaddr)
          {
             /* This happen when host doesn't have a reverse. */
              if (arg->isv6)
                {
                   struct sockaddr_in6 *addri6;

                   addrlen = sizeof(struct sockaddr_in6);
                   addri6 = malloc(addrlen);

                   if (!addri6)
                     goto on_mem_error;

                   addri6->sin6_family = AF_INET6;
                   addri6->sin6_port = htons(arg->svr->port);
                   addri6->sin6_flowinfo = 0;
                   addri6->sin6_scope_id = 0;

                   memcpy(&addri6->sin6_addr.s6_addr,
                          &arg->addr.v6, sizeof(struct in6_addr));

                   addr = (struct sockaddr *)addri6;
                }
              else
                {
                   struct sockaddr_in *addri;

                   addrlen = sizeof(struct sockaddr_in);
                   addri = malloc(addrlen);

                   if (!addri)
                     goto on_mem_error;

                   addri->sin_family = AF_INET;
                   addri->sin_port = htons(arg->svr->port);

                   memcpy(&addri->sin_addr.s_addr,
                          &arg->addr.v4, sizeof(struct in_addr));

                   addr = (struct sockaddr *)addri;
                }

              if (!_ecore_con_info_ares_getnameinfo(arg,
                                                    arg->isv6 ? AF_INET6 :
                                                    AF_INET,
                                                    NULL, addr,
                                                    addrlen))
                goto on_error;

              break;
          }

      case ARES_ENOTIMP: /* unknown family */
      case ARES_EBADNAME: /* not a valid internet address */
      case ARES_ENOMEM: /* not enough memory */
      case ARES_EDESTRUCTION: /* request canceled, shuting down */
      case ARES_ENODATA: /* no data returned */
      case ARES_ECONNREFUSED: /* connection refused */
        goto on_error;

      default:
        ERR("Unknown status returned by c-ares: %i assuming error",
                status);
        goto on_error;
     }

   return;

on_mem_error:
   ERR("Not enough memory");

on_error:
   arg->done_cb(arg->data, NULL);
   free(arg);
}

static void
_ecore_con_info_ares_nameinfo(Ecore_Con_CAres *arg,
                              int              status,
                              int              timeouts __UNUSED__,
                              char            *node,
                              char            *service)
{
   switch (status)
     {
      case ARES_SUCCESS:
        if (node)
          strcpy(arg->result->ip, node);
        else
          *arg->result->ip = '\0';

        if (service)
          strcpy(arg->result->service, service);
        else
          *arg->result->service = '\0';

        arg->done_cb(arg->data, arg->result);
        break;

      case ARES_ENOTIMP:
      case ARES_ENOTFOUND:
      case ARES_ENOMEM:
      case ARES_EDESTRUCTION:
      case ARES_EBADFLAGS:
        arg->done_cb(arg->data, NULL);
        break;
     }

   free(arg->result->info.ai_addr);
   free(arg->result);
   free(arg);
}

