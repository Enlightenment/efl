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

#ifdef HAVE_ARPA_INET_H
# include <arpa/inet.h>
#endif

#include <ares.h>

#include "Ecore.h"
#include "Ecore_Con.h"
#include "ecore_con_private.h"

typedef struct _Ecore_Con_FD    Ecore_Con_FD;
typedef struct _Ecore_Con_CAres Ecore_Con_CAres;

struct _Ecore_Con_FD
{
   Ecore_Fd_Handler *handler;
   Ecore_Timer      *timer;
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
#ifdef HAVE_IPV6
      struct in6_addr v6;
#endif
   } addr;

   Eina_Bool byaddr : 1;
   Eina_Bool isv6 : 1;
};

static ares_channel info_channel;
static int info_init = 0;
static Eina_List *info_fds = NULL;

static void _ecore_con_info_ares_nameinfo(Ecore_Con_CAres *arg,
                                          int              status,
                                          int              timeouts,
                                          char            *node,
                                          char            *service);
static void _ecore_con_info_ares_host_cb(Ecore_Con_CAres *arg,
                                         int              status,
                                         int              timeouts,
                                         struct hostent  *hostent);
static Eina_Bool _ecore_con_info_cares_fd_cb(Ecore_Con_FD     *ecf,
                            Ecore_Fd_Handler *fd_handler);
static Eina_Bool _ecore_con_info_cares_timeout_cb(void *data);

static void
_ecore_con_info_cares_state_cb(void *data,
                               ares_socket_t fd,
                               int readable,
                               int writable);
static int
_ecore_con_info_fds_search(const Ecore_Con_FD *fd1,
                           const Ecore_Con_FD *fd2);

int
ecore_con_info_init(void)
{
   struct ares_options opts;

   if (!info_init)
     {
        if (ares_library_init(ARES_LIB_INIT_ALL))
          return 0;

        opts.lookups = "fb"; /* hosts file then dns */
        opts.sock_state_cb = _ecore_con_info_cares_state_cb;

        if (ares_init_options(&info_channel, &opts,
            ARES_OPT_LOOKUPS | ARES_OPT_SOCK_STATE_CB) != ARES_SUCCESS)
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
#ifdef HAVE_IPV6
   hints.ai_family = AF_INET6;
#else
   hints.ai_family = AF_INET;
#endif
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
#ifdef HAVE_IPV6
   hints.ai_family = AF_INET6;
#else
   hints.ai_family = AF_INET;
#endif
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
#ifdef HAVE_IPV6
   hints.ai_family = AF_INET6;
#else
   hints.ai_family = AF_INET;
#endif
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
#ifdef HAVE_IPV6
   hints.ai_family = AF_INET6;
#else
   hints.ai_family = AF_INET;
#endif
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
#ifdef HAVE_IPV6
   hints.ai_family = AF_INET6;
#else
   hints.ai_family = AF_INET;
#endif
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
#ifdef HAVE_IPV6
   int ai_family = AF_INET6;
#else
   int ai_family = AF_INET;
#endif

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
#ifdef HAVE_IPV6
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
#endif
   else
     {
        cares->byaddr = EINA_FALSE;
        ares_gethostbyname(info_channel, svr->name, ai_family,
                           (ares_host_callback)_ecore_con_info_ares_host_cb,
                           cares);
     }

   svr->infos = eina_list_append(svr->infos, cares);
   return 1;
}

void
ecore_con_info_data_clear(void *info)
{
   Ecore_Con_CAres *cares = info;
   if (cares) cares->data = NULL;
}

static Eina_Bool
_ecore_con_info_cares_timeout_cb(void *data __UNUSED__)
{
   ares_process_fd(info_channel, ARES_SOCKET_BAD, ARES_SOCKET_BAD);
   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_ecore_con_info_cares_fd_cb(Ecore_Con_FD     *ecf,
                            Ecore_Fd_Handler *fd_handler)
{
   ares_socket_t read_fd, write_fd;

   read_fd = write_fd = ARES_SOCKET_BAD;

   if (ecore_main_fd_handler_active_get(fd_handler, ECORE_FD_READ))
     read_fd = ecf->fd;
   if (ecore_main_fd_handler_active_get(fd_handler, ECORE_FD_WRITE))
     write_fd = ecf->fd;

   ares_process_fd(info_channel, read_fd, write_fd);

   return ECORE_CALLBACK_RENEW;
}

static int
_ecore_con_info_fds_search(const Ecore_Con_FD *fd1,
                           const Ecore_Con_FD *fd2)
{
   return fd1->fd - fd2->fd;
}

static void
_ecore_con_info_cares_state_cb(void *data __UNUSED__,
                               ares_socket_t fd,
                               int readable,
                               int writable)
{
   int flags = 0;
   Ecore_Con_FD *search = NULL, *ecf = NULL;

   search = eina_list_search_unsorted(info_fds,
            (Eina_Compare_Cb)_ecore_con_info_fds_search, &ecf);

   if (!(readable | writable))
     {
        ares_process_fd(info_channel, ARES_SOCKET_BAD, ARES_SOCKET_BAD);
        if (search)
          {
             info_fds = eina_list_remove(info_fds, search);
             ecore_timer_del(search->timer);
             ecore_main_fd_handler_del(search->handler);
             free(search);
          }
        return;
     }

   if (!search)
     {
        search = malloc(sizeof(Ecore_Con_FD));
        EINA_SAFETY_ON_NULL_RETURN(search);

        search->fd = fd;
        search->handler = ecore_main_fd_handler_add(fd, ECORE_FD_WRITE | ECORE_FD_READ,
            (Ecore_Fd_Cb)_ecore_con_info_cares_fd_cb, search, NULL, NULL);
        /* c-ares default timeout is 5 seconds */
        search->timer = ecore_timer_add(5, _ecore_con_info_cares_timeout_cb, NULL);
        info_fds = eina_list_append(info_fds, search);
     }

   if (readable) flags |= ECORE_FD_READ;
   if (writable) flags |= ECORE_FD_WRITE;
   ecore_main_fd_handler_active_set(search->handler, flags);
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
#ifdef HAVE_IPV6
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
#endif
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
#ifdef HAVE_IPV6
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
#endif
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
#ifdef HAVE_IPV6
                                                    arg->isv6 ? AF_INET6 :
#endif
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
      case ARES_ETIMEOUT: /* connection timed out */
        ecore_con_event_server_error(arg->svr, ares_strerror(status));
        goto on_error;

      default:
        ERR("Unknown status returned by c-ares: %i assuming error", status);
        ecore_con_event_server_error(arg->svr, ares_strerror(status));
        goto on_error;
     }

   return;

on_mem_error:
   ERR("Not enough memory");

on_error:
   if (arg->data)
     {
        ecore_con_server_infos_del(arg->data, arg);
        arg->done_cb(arg->data, NULL);
     }
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

        if (arg->data) arg->done_cb(arg->data, arg->result);
        break;

      case ARES_ENOTIMP:
      case ARES_ENOTFOUND:
      case ARES_ENOMEM:
      case ARES_EDESTRUCTION:
      case ARES_EBADFLAGS:
        ecore_con_event_server_error(arg->svr, ares_strerror(status));
        if (arg->data) arg->done_cb(arg->data, NULL);
        break;
     }

   free(arg->result->info.ai_addr);
   free(arg->result);
   if (arg->data) ecore_con_server_infos_del(arg->data, arg);
   free(arg);
}

