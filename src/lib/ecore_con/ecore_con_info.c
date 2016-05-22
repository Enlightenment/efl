/*
 * getaddrinfo with callback
 *
 * man getaddrinfo
 *
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef STDC_HEADERS
# include <stdlib.h>
# include <stddef.h>
#else
# ifdef HAVE_STDLIB_H
#  include <stdlib.h>
# endif
#endif

#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#ifdef HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif
#ifdef __OpenBSD__
# include <sys/types.h>
#endif
#ifdef HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif
#ifdef HAVE_ARPA_INET_H
# include <arpa/inet.h>
#endif
#ifdef HAVE_ARPA_NAMESER_H
# include <arpa/nameser.h>
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
   Ecore_Thread        *thread;
   struct addrinfo      hints;
   Ecore_Con_Info      *result;
   int                  error;
   char                 service[NI_MAXSERV];
   char                 name[NI_MAXHOST];
};

static void  _ecore_con_info_slave_free  (CB_Data *cbdata);
static void  _ecore_con_info_slave_result(void *data, Ecore_Thread *th);
static void  _ecore_con_info_slave_cancel(void *data, Ecore_Thread *th);
static void  _ecore_con_info_slave_lookup(void *data, Ecore_Thread *th);

static int      info_init = 0;
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
     {
        while (info_slaves)
          {
             CB_Data *cbdata;

             cbdata = info_slaves;
             info_slaves = (CB_Data *)eina_inlist_remove
               (EINA_INLIST_GET(info_slaves), EINA_INLIST_GET(info_slaves));
             ecore_thread_cancel(cbdata->thread);
          }
     }
   return info_init;
}

static void
_hints_fill(struct addrinfo *hints, int flags, int proto)
{
   memset(hints, 0, sizeof(struct addrinfo));
   hints->ai_family = AF_UNSPEC;
   hints->ai_flags = flags;
   if (proto == IPPROTO_TCP) hints->ai_socktype = SOCK_STREAM;
   else hints->ai_socktype = SOCK_DGRAM;
   hints->ai_protocol = proto;
}

int
ecore_con_info_tcp_connect(Ecore_Con_Server *svr,
                           Ecore_Con_Info_Cb done_cb,
                           void *data)
{
   struct addrinfo hints;
   _hints_fill(&hints, AI_CANONNAME, IPPROTO_TCP);
   return ecore_con_info_get(svr, done_cb, data, &hints);
}

int
ecore_con_info_tcp_listen(Ecore_Con_Server *svr,
                          Ecore_Con_Info_Cb done_cb,
                          void *data)
{
   struct addrinfo hints;
   _hints_fill(&hints, AI_PASSIVE, IPPROTO_TCP);
   return ecore_con_info_get(svr, done_cb, data, &hints);
}

int
ecore_con_info_udp_connect(Ecore_Con_Server *svr,
                           Ecore_Con_Info_Cb done_cb,
                           void *data)
{
   struct addrinfo hints;
   _hints_fill(&hints, AI_CANONNAME, IPPROTO_UDP);
   return ecore_con_info_get(svr, done_cb, data, &hints);
}

int
ecore_con_info_udp_listen(Ecore_Con_Server *svr,
                          Ecore_Con_Info_Cb done_cb,
                          void *data)
{
   struct addrinfo hints;
   _hints_fill(&hints, AI_PASSIVE, IPPROTO_UDP);
   return ecore_con_info_get(svr, done_cb, data, &hints);
}

int
ecore_con_info_mcast_listen(Ecore_Con_Server *svr,
                            Ecore_Con_Info_Cb done_cb,
                            void *data)
{
   struct addrinfo hints;
   _hints_fill(&hints, 0, IPPROTO_UDP);
   return ecore_con_info_get(svr, done_cb, data, &hints);
}

EAPI int
ecore_con_info_get(Ecore_Con_Server *obj,
                   Ecore_Con_Info_Cb done_cb,
                   void *data,
                   struct addrinfo *hints)
{
   Efl_Network_Server_Data *svr = eo_data_scope_get(obj, EFL_NETWORK_SERVER_CLASS);
   CB_Data *cbdata;

   if (!svr) return 0;
   cbdata = calloc(1, sizeof(CB_Data));
   if (!cbdata)
     {
        ecore_con_event_server_error(obj, "Memory allocation failure");
        return 0;
     }

   cbdata->cb_done = done_cb;
   cbdata->data = data;
   cbdata->hints = *hints;
   cbdata->thread = ecore_thread_run(_ecore_con_info_slave_lookup,
                                     _ecore_con_info_slave_result,
                                     _ecore_con_info_slave_cancel,
                                     cbdata);
   if (!cbdata->thread)
     {
        free(cbdata);
        ecore_con_event_server_error(obj, "Memory allocation failure");
        return 0;
     }
   eina_convert_itoa(svr->ecs ? svr->ecs->port : svr->port, cbdata->service);
   strncpy(cbdata->name, svr->ecs ? svr->ecs->ip : svr->name, NI_MAXHOST - 1);
   cbdata->name[NI_MAXHOST - 1] = 0;
   info_slaves = (CB_Data *)eina_inlist_append(EINA_INLIST_GET(info_slaves),
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
_ecore_con_info_slave_free(CB_Data *cbdata)
{
   info_slaves = (CB_Data *)eina_inlist_remove(EINA_INLIST_GET(info_slaves),
                                               EINA_INLIST_GET(cbdata));
   if (cbdata->result) free(cbdata->result);
   cbdata->result = NULL;
   if (cbdata->data) ecore_con_server_infos_del(cbdata->data, cbdata);
   cbdata->data = NULL;
   free(cbdata);
}

static void
_ecore_con_info_slave_result(void *data, Ecore_Thread *th EINA_UNUSED)
{
   CB_Data *cbdata = data;

   if (cbdata->result) // lookup ok
     {
        if (cbdata->data) cbdata->cb_done(cbdata->data, cbdata->result);
     }
   else // an error occured
     {
        if (cbdata->data)
          {
             char *str = strerror(cbdata->error);
             ecore_con_event_server_error(cbdata->data, str);
             cbdata->cb_done(cbdata->data, NULL);
          }
     }
   if (cbdata->data) ecore_con_server_infos_del(cbdata->data, cbdata);
   cbdata->data = NULL;
   _ecore_con_info_slave_free(cbdata);
}

static void
_ecore_con_info_slave_cancel(void *data, Ecore_Thread *th EINA_UNUSED)
{
   CB_Data *cbdata = data;
   _ecore_con_info_slave_free(cbdata);
}

static void
_ecore_con_info_slave_lookup(void *data, Ecore_Thread *th EINA_UNUSED)
{
   CB_Data *cbdata = data;
   struct addrinfo *result = NULL;

   // do lookup, fill cbdata
   if ((!getaddrinfo(cbdata->name, cbdata->service, &(cbdata->hints), &result))
       && (result))
     {
        Ecore_Con_Info *info;
        unsigned int canonname_size = 0, size;

        if (result->ai_canonname)
          canonname_size = strlen(result->ai_canonname) + 1;
        size = sizeof(Ecore_Con_Info) + result->ai_addrlen + canonname_size;
        info = calloc(1, size);
        if (info)
          {
             char hbuf[NI_MAXHOST] = { 0 }, sbuf[NI_MAXSERV] = { 0 }, *p;

             info->size = size;
             memcpy(&(info->info), result, sizeof(struct addrinfo));
             p = ((char *)info) + sizeof(Ecore_Con_Info);
             memcpy(p, result->ai_addr, result->ai_addrlen);
             info->info.ai_addr = (struct sockaddr *)p;
             if (result->ai_canonname)
               {
                  p = ((char *)info) + sizeof(Ecore_Con_Info) + result->ai_addrlen;
                  memcpy(p, result->ai_canonname, canonname_size);
                  info->info.ai_canonname = p;
               }
             // we don't care about multiple entries - take first one then
             info->info.ai_next = NULL;
             if (!getnameinfo(result->ai_addr, result->ai_addrlen,
                              hbuf, sizeof(hbuf), sbuf, sizeof(sbuf),
                              NI_NUMERICHOST | NI_NUMERICSERV))
               {
                  memcpy(info->ip, hbuf, sizeof(info->ip));
                  memcpy(info->service, sbuf, sizeof(info->service));
               }
             cbdata->result = info;
          }
        if (!cbdata->result) free(info);
     }
   if (!cbdata->result) cbdata->error = errno;
   if (result) freeaddrinfo(result);
}
