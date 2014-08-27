#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

/*
 * This version of ecore_con_info uses dns.c to provide asynchronous dns lookup.
 *
 * dns.c is written by William Ahern:
 * http://25thandclement.com/~william/projects/dns.c.html
 */

#include <string.h>
#include <sys/types.h>
#include <errno.h> /* for EAGAIN */
#include <netinet/in.h>
#include <arpa/inet.h>

#include "dns.h"

#include "Ecore.h"
#include "Ecore_Con.h"
#include "ecore_con_private.h"

typedef struct dns_addrinfo    dns_addrinfo;
typedef struct dns_resolv_conf dns_resolv_conf;
typedef struct dns_resolver    dns_resolver;
typedef struct dns_hosts       dns_hosts;

typedef struct _Ecore_Con_DNS  Ecore_Con_DNS;

struct _Ecore_Con_DNS
{
   Ecore_Con_Server *svr;
   Ecore_Con_Info_Cb done_cb;
   void             *data;
   dns_addrinfo     *ai;
   dns_resolver     *resolv;
   struct addrinfo   hints;
   Ecore_Fd_Handler *fdh;
   Ecore_Timer      *timer;
};

static int _ecore_con_dns_init = 0;
static dns_resolv_conf *resconf = NULL;
static dns_hosts *hosts = NULL;

static void
_ecore_con_dns_free(Ecore_Con_DNS *dns)
{
   if (dns->timer) ecore_timer_del(dns->timer);
   if (dns->fdh) ecore_main_fd_handler_del(dns->fdh);
   if (dns->ai) dns_ai_close(dns->ai);
   dns_res_close(dns_res_mortal(dns->resolv));
   free(dns);
}

static void
_ecore_con_dns_del(Ecore_Con_DNS *dns)
{
   if (dns->svr)
     {
        Ecore_Con_Server_Data *svr;

        svr = eo_data_scope_get(dns->svr, ECORE_CON_CLIENT_CLASS);
        if ((svr) && (svr->infos))
          svr->infos = eina_list_remove(svr->infos, dns);
     }
   _ecore_con_dns_free(dns);
}

static Eina_Bool
_dns_addrinfo_get(Ecore_Con_DNS *dns, const char *addr, int port)
{
   int error = 0;
   char service[NI_MAXSERV];

   snprintf(service, sizeof(service), "%d", port);
   dns->ai = dns_ai_open(addr, service, DNS_T_A, (const struct addrinfo *)&dns->hints, dns->resolv, &error);
   return error;
}

static int
_ecore_con_dns_check(Ecore_Con_DNS *dns)
{
   struct addrinfo *ent = NULL;
   int error = 0;

   error = dns_ai_nextent(&ent, dns->ai);

   switch (error)
     {
      case 0:
        break;

      case EAGAIN:
        return 1;

      default:
        ERR("resolve failed: %s", dns_strerror(error));
        goto error;
     }

   {
      Ecore_Con_Info result = {0, .ip = {0}, .service = {0}};
#if 0
      char pretty[512];
      dns_ai_print(pretty, sizeof(pretty), ent, dns->ai);
      printf("%s\n", pretty);
#endif
      result.size = 0;
      dns_inet_ntop(dns_sa_family(ent->ai_addr), dns_sa_addr(dns_sa_family(ent->ai_addr), ent->ai_addr), result.ip, sizeof(result.ip));
      snprintf(result.service, sizeof(result.service), "%u", ntohs(*dns_sa_port(dns_sa_family(ent->ai_addr), ent->ai_addr)));
      memcpy(&result.info, ent, sizeof(result.info));
      if (dns->fdh) ecore_main_fd_handler_del(dns->fdh);
      dns->fdh = NULL;
      dns->done_cb(dns->data, &result);
      free(ent);
      _ecore_con_dns_del(dns);
   }

   return 0;
error:
   dns->done_cb(dns->data, NULL);
   _ecore_con_dns_del(dns);
   return -1;
}

static Eina_Bool
_dns_fd_cb(Ecore_Con_DNS *dns, Ecore_Fd_Handler *fdh EINA_UNUSED)
{
   if (_ecore_con_dns_check(dns) != 1) return ECORE_CALLBACK_RENEW;
   if (ecore_main_fd_handler_fd_get(dns->fdh) != dns_ai_pollfd(dns->ai))
     {
        ecore_main_fd_handler_del(dns->fdh);
        dns->fdh = ecore_main_fd_handler_add(dns_ai_pollfd(dns->ai), dns_ai_events(dns->ai), (Ecore_Fd_Cb)_dns_fd_cb, dns, NULL, NULL);
     }
   else
     ecore_main_fd_handler_active_set(dns->fdh, dns_ai_events(dns->ai));
   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_dns_timer_cb(Ecore_Con_DNS *dns)
{
   dns->done_cb(dns->data, NULL);
   _ecore_con_dns_del(dns);
   return EINA_FALSE;
}

int
ecore_con_info_init(void)
{
   int err;
   if (_ecore_con_dns_init) return ++_ecore_con_dns_init;

   resconf = dns_resconf_local(&err);
   if (!resconf)
     {
        ERR("resconf_open: %s", dns_strerror(err));
        return 0;
     }
   hosts = dns_hosts_local(&err);
   if (!hosts)
     {
        ERR("hosts_open: %s", dns_strerror(err));
        dns_resconf_close(resconf);
        resconf = NULL;
        return 0;
     }
   /* this is super slow don't do it */
   //resconf->options.recurse = 1;
   return ++_ecore_con_dns_init;
}

int
ecore_con_info_shutdown(void)
{
   if (!_ecore_con_dns_init) return 0;
   if (--_ecore_con_dns_init) return _ecore_con_dns_init;
   dns_resconf_close(resconf);
   resconf = NULL;
   dns_hosts_close(hosts);
   hosts = NULL;
   return 0;
}

void
ecore_con_info_data_clear(void *info)
{
   if (info)
     _ecore_con_dns_free(info);
}

int
ecore_con_info_tcp_connect(Ecore_Con_Server *svr,
                           Ecore_Con_Info_Cb done_cb,
                           void *data)
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

   return ecore_con_info_get(svr, done_cb, data, &hints);
}

int
ecore_con_info_tcp_listen(Ecore_Con_Server *svr,
                          Ecore_Con_Info_Cb done_cb,
                          void *data)
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

   return ecore_con_info_get(svr, done_cb, data, &hints);
}

int
ecore_con_info_udp_connect(Ecore_Con_Server *svr,
                           Ecore_Con_Info_Cb done_cb,
                           void *data)
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

   return ecore_con_info_get(svr, done_cb, data, &hints);
}

int
ecore_con_info_udp_listen(Ecore_Con_Server *svr,
                          Ecore_Con_Info_Cb done_cb,
                          void *data)
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

   return ecore_con_info_get(svr, done_cb, data, &hints);
}

int
ecore_con_info_mcast_listen(Ecore_Con_Server *svr,
                            Ecore_Con_Info_Cb done_cb,
                            void *data)
{
   struct addrinfo hints;

   memset(&hints, 0, sizeof(struct addrinfo));
#ifdef HAVE_IPV6
   hints.ai_family = AF_INET6;
#else
   hints.ai_family = AF_INET;
#endif
   hints.ai_socktype = SOCK_DGRAM;
   hints.ai_protocol = IPPROTO_UDP;

   return ecore_con_info_get(svr, done_cb, data, &hints);
}

EAPI int
ecore_con_info_get(Ecore_Con_Server *obj,
                   Ecore_Con_Info_Cb done_cb,
                   void *data,
                   struct addrinfo *hints)
{
   Ecore_Con_Server_Data *svr = eo_data_scope_get(obj, ECORE_CON_SERVER_CLASS);
   Ecore_Con_DNS *dns;
   int error = 0;

   if (!svr)
      return 0;

   dns = calloc(1, sizeof(Ecore_Con_DNS));
   if (!dns) return 0;

   dns->svr = obj;
   dns->done_cb = done_cb;
   dns->data = data;

   if (hints)
     memcpy(&dns->hints, hints, sizeof(struct addrinfo));

   if (!(dns->resolv = dns_res_open(resconf, hosts, dns_hints_mortal(dns_hints_local(resconf, &error)), NULL, dns_opts(), &error)))
     {
        ERR("res_open: %s", dns_strerror(error));
        goto reserr;
     }

   error = _dns_addrinfo_get(dns, svr->ecs ? svr->ecs->ip : svr->name, svr->ecs ? svr->ecs->port : svr->port);
   if (error && (error != EAGAIN))
     {
        ERR("resolver: %s", dns_strerror(error));
        goto seterr;
     }

   switch (_ecore_con_dns_check(dns))
     {
      case 0:
        break;

      case 1:
        dns->fdh = ecore_main_fd_handler_add(dns_ai_pollfd(dns->ai), dns_ai_events(dns->ai), (Ecore_Fd_Cb)_dns_fd_cb, dns, NULL, NULL);
        svr->infos = eina_list_append(svr->infos, dns);
        dns->timer = ecore_timer_add(5.0, (Ecore_Task_Cb)_dns_timer_cb, dns);
        break;

      default:
        return 0;
     }

   return 1;
seterr:
   if (dns->resolv) dns_res_close(dns_res_mortal(dns->resolv));
reserr:
   free(dns);
   return 0;
}

