#define EFL_NET_IP_ADDRESS_PROTECTED 1

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef HAVE_ARPA_INET_H
# include <arpa/inet.h>
#endif

#ifdef HAVE_EVIL
# include <Evil.h>
#endif

#include "Ecore.h"
#include "Ecore_Con.h"
#include "ecore_con_private.h"

typedef struct _Efl_Net_Ip_Address_Data {
   char string[INET6_ADDRSTRLEN + sizeof("[]:65536")];
   union {
      struct sockaddr addr;
      struct sockaddr_in ipv4;
      struct sockaddr_in6 ipv6;
   };
   Eina_Slice addr_slice;
} Efl_Net_Ip_Address_Data;

#define MY_CLASS EFL_NET_IP_ADDRESS_CLASS

EOLIAN static Eo *
_efl_net_ip_address_efl_object_finalize(Eo *o, Efl_Net_Ip_Address_Data *pd)
{
   const uint16_t *pport;

   o = efl_finalize(efl_super(o, MY_CLASS));
   if (!o) return NULL;

   EINA_SAFETY_ON_TRUE_RETURN_VAL(pd->addr.sa_family == 0, NULL);

   if (!efl_net_ip_port_fmt(pd->string, sizeof(pd->string), &pd->addr))
     {
        ERR("Could not format address!");
        return NULL;
     }

   if (pd->addr.sa_family == AF_INET6)
     pport = &pd->ipv6.sin6_port;
   else
     pport = &pd->ipv4.sin_port;

   if (*pport == 0) /* port == 0, no ":0" in the string */
     {
        char *d = strrchr(pd->string, ':');
        EINA_SAFETY_ON_NULL_RETURN_VAL(d, NULL);
        *d = '\0';
     }

   return o;
}

EOLIAN static const char *
_efl_net_ip_address_string_get(Eo *o EINA_UNUSED, Efl_Net_Ip_Address_Data *pd)
{
   return pd->string;
}

EOLIAN static void
_efl_net_ip_address_family_set(Eo *o EINA_UNUSED, Efl_Net_Ip_Address_Data *pd, int family)
{
   if (pd->addr.sa_family == family) return;
   EINA_SAFETY_ON_TRUE_RETURN(pd->addr.sa_family != 0);
   EINA_SAFETY_ON_TRUE_RETURN((family != AF_INET) && (family != AF_INET6));
   pd->addr.sa_family = family;
   if (family == AF_INET6)
     {
        pd->addr_slice.mem = &pd->ipv6.sin6_addr;
        pd->addr_slice.len = sizeof(pd->ipv6.sin6_addr);
     }
   else
     {
        pd->addr_slice.mem = &pd->ipv4.sin_addr;
        pd->addr_slice.len = sizeof(pd->ipv4.sin_addr);
     }
}

EOLIAN static int
_efl_net_ip_address_family_get(Eo *o EINA_UNUSED, Efl_Net_Ip_Address_Data *pd)
{
   return pd->addr.sa_family;
}

EOLIAN static void
_efl_net_ip_address_port_set(Eo *o EINA_UNUSED, Efl_Net_Ip_Address_Data *pd, uint16_t port)
{
   uint16_t *pport, nport = htons(port);

   EINA_SAFETY_ON_TRUE_RETURN(pd->addr.sa_family == 0);
   if (pd->addr.sa_family == AF_INET6)
     pport = &pd->ipv6.sin6_port;
   else
     pport = &pd->ipv4.sin_port;

   if (*pport == nport) return;
   if (*pport)
     {
        ERR("port already set to %hu, new %hu", ntohs(*pport), port);
        return;
     }

   *pport = nport;
}

EOLIAN static uint16_t
_efl_net_ip_address_port_get(Eo *o EINA_UNUSED, Efl_Net_Ip_Address_Data *pd)
{
   const uint16_t *pport;

   EINA_SAFETY_ON_TRUE_RETURN_VAL(pd->addr.sa_family == 0, 0);
   if (pd->addr.sa_family == AF_INET6)
     pport = &pd->ipv6.sin6_port;
   else
     pport = &pd->ipv4.sin_port;

   return ntohs(*pport);
}

EOLIAN static void
_efl_net_ip_address_address_set(Eo *o EINA_UNUSED, Efl_Net_Ip_Address_Data *pd, const Eina_Slice *address)
{
   Eina_Rw_Slice rw_slice;
   size_t i;

   EINA_SAFETY_ON_TRUE_RETURN(pd->addr.sa_family == 0);
   EINA_SAFETY_ON_NULL_RETURN(address);

   rw_slice.mem = (void *)pd->addr_slice.mem;
   rw_slice.len = pd->addr_slice.len;

   EINA_SAFETY_ON_TRUE_RETURN(rw_slice.len != address->len);

   if (eina_slice_compare(eina_rw_slice_slice_get(rw_slice), *address) == 0)
     return;

   for (i = 0; i < rw_slice.len; i++)
     {
        if (rw_slice.bytes[i])
          {
             char old_str[INET6_ADDRSTRLEN] = "";
             char new_str[INET6_ADDRSTRLEN] = "";

             if (!inet_ntop(pd->addr.sa_family, rw_slice.mem, old_str, sizeof(old_str)))
               {
                  old_str[0] = '?';
                  old_str[1] = '\0';
               }
             if (!inet_ntop(pd->addr.sa_family, address->mem, new_str, sizeof(new_str)))
               {
                  new_str[0] = '?';
                  new_str[1] = '\0';
               }
             ERR("address already set to %s, new %s", old_str, new_str);
             return;
          }
     }

   eina_rw_slice_copy(rw_slice, *address);
}

EOLIAN static const Eina_Slice *
_efl_net_ip_address_address_get(Eo *o EINA_UNUSED, Efl_Net_Ip_Address_Data *pd)
{
   return &pd->addr_slice;
}

EOLIAN static void
_efl_net_ip_address_sockaddr_set(Eo *o EINA_UNUSED, Efl_Net_Ip_Address_Data *pd, const void *ptr)
{
   const struct sockaddr *sockaddr = ptr;

   EINA_SAFETY_ON_TRUE_RETURN(pd->addr.sa_family != 0);
   EINA_SAFETY_ON_NULL_RETURN(sockaddr);

   EINA_SAFETY_ON_TRUE_RETURN((sockaddr->sa_family != AF_INET) && (sockaddr->sa_family != AF_INET6));

   if (sockaddr->sa_family == AF_INET6)
     {
        memcpy(&pd->ipv6, sockaddr, sizeof(pd->ipv6));
        pd->addr_slice.mem = &pd->ipv6.sin6_addr;
        pd->addr_slice.len = sizeof(pd->ipv6.sin6_addr);
     }
   else
     {
        memcpy(&pd->ipv4, sockaddr, sizeof(pd->ipv4));
        pd->addr_slice.mem = &pd->ipv4.sin_addr;
        pd->addr_slice.len = sizeof(pd->ipv4.sin_addr);
     }
}

EOLIAN static const void *
_efl_net_ip_address_sockaddr_get(Eo *o EINA_UNUSED, Efl_Net_Ip_Address_Data *pd)
{
   return &pd->addr;
}

#define IPV4_ADDR_GET(pd) ntohl(pd->ipv4.sin_addr.s_addr)

EOLIAN static Eina_Bool
_efl_net_ip_address_ipv4_class_a_check(const Eo *o EINA_UNUSED, Efl_Net_Ip_Address_Data *pd)
{
   return (pd->addr.sa_family == AF_INET) && IN_CLASSA(IPV4_ADDR_GET(pd));
}

EOLIAN static Eina_Bool
_efl_net_ip_address_ipv4_class_b_check(const Eo *o EINA_UNUSED, Efl_Net_Ip_Address_Data *pd)
{
   return (pd->addr.sa_family == AF_INET) && IN_CLASSB(IPV4_ADDR_GET(pd));
}

EOLIAN static Eina_Bool
_efl_net_ip_address_ipv4_class_c_check(const Eo *o EINA_UNUSED, Efl_Net_Ip_Address_Data *pd)
{
   return (pd->addr.sa_family == AF_INET) && IN_CLASSC(IPV4_ADDR_GET(pd));
}

EOLIAN static Eina_Bool
_efl_net_ip_address_ipv4_class_d_check(const Eo *o EINA_UNUSED, Efl_Net_Ip_Address_Data *pd)
{
   return (pd->addr.sa_family == AF_INET) && IN_CLASSD(IPV4_ADDR_GET(pd));
}

EOLIAN static Eina_Bool
_efl_net_ip_address_ipv6_v4mapped_check(const Eo *o EINA_UNUSED, Efl_Net_Ip_Address_Data *pd)
{
   return (pd->addr.sa_family == AF_INET6) &&
     IN6_IS_ADDR_V4MAPPED(&pd->ipv6.sin6_addr);
}

EOLIAN static Eina_Bool
_efl_net_ip_address_ipv6_v4compat_check(const Eo *o EINA_UNUSED, Efl_Net_Ip_Address_Data *pd)
{
   return (pd->addr.sa_family == AF_INET6) &&
     IN6_IS_ADDR_V4COMPAT(&pd->ipv6.sin6_addr);
}

EOLIAN static Eina_Bool
_efl_net_ip_address_ipv6_local_link_check(const Eo *o EINA_UNUSED, Efl_Net_Ip_Address_Data *pd)
{
   return (pd->addr.sa_family == AF_INET6) &&
     IN6_IS_ADDR_LINKLOCAL(&pd->ipv6.sin6_addr);
}

EOLIAN static Eina_Bool
_efl_net_ip_address_ipv6_local_site_check(const Eo *o EINA_UNUSED, Efl_Net_Ip_Address_Data *pd)
{
   return (pd->addr.sa_family == AF_INET6) &&
     IN6_IS_ADDR_SITELOCAL(&pd->ipv6.sin6_addr);
}

EOLIAN static Eina_Bool
_efl_net_ip_address_multicast_check(const Eo *o EINA_UNUSED, Efl_Net_Ip_Address_Data *pd)
{
   if (pd->addr.sa_family == AF_INET6)
     return IN6_IS_ADDR_MULTICAST(&pd->ipv6.sin6_addr);
   else
     return IN_MULTICAST(IPV4_ADDR_GET(pd));
}

EOLIAN static Eina_Bool
_efl_net_ip_address_loopback_check(const Eo *o EINA_UNUSED, Efl_Net_Ip_Address_Data *pd)
{
   if (pd->addr.sa_family == AF_INET6)
     return IN6_IS_ADDR_LOOPBACK(&pd->ipv6.sin6_addr);
   else
     return IPV4_ADDR_GET(pd) == INADDR_LOOPBACK;
}

EOLIAN static Eina_Bool
_efl_net_ip_address_any_check(const Eo *o EINA_UNUSED, Efl_Net_Ip_Address_Data *pd)
{
   size_t i;

   for (i = 0; i < pd->addr_slice.len; i++)
     {
        if (pd->addr_slice.bytes[i])
          return EINA_FALSE;
     }

   return i > 0;
}

EOLIAN static Efl_Net_Ip_Address *
_efl_net_ip_address_create(Eo *cls, void *pd EINA_UNUSED, uint16_t port, const Eina_Slice address)
{
   int family;

   EINA_SAFETY_ON_TRUE_RETURN_VAL(address.len != 4 && address.len != 16, NULL);

   if (address.len == 16)
     family = AF_INET6;
   else
     family = AF_INET;

   return efl_add(cls, NULL,
                  efl_net_ip_address_family_set(efl_added, family),
                  efl_net_ip_address_port_set(efl_added, port),
                  efl_net_ip_address_set(efl_added, &address));
}

EOLIAN static Efl_Net_Ip_Address *
_efl_net_ip_address_create_sockaddr(Eo *cls, void *pd EINA_UNUSED, const void *ptr)
{
   const struct sockaddr *sockaddr = ptr;

   EINA_SAFETY_ON_NULL_RETURN_VAL(sockaddr, NULL);
   EINA_SAFETY_ON_TRUE_RETURN_VAL((sockaddr->sa_family != AF_INET) && (sockaddr->sa_family != AF_INET6), NULL);

   return efl_add(cls, NULL,
                  efl_net_ip_address_sockaddr_set(efl_added, sockaddr));
}

EOLIAN static Efl_Net_Ip_Address *
_efl_net_ip_address_parse(Eo *cls, void *pd EINA_UNUSED, const char *numeric_address)
{
   struct sockaddr_storage ss;
   Eina_Bool r;
   const char *address = numeric_address;
   char *tmp = NULL;

   EINA_SAFETY_ON_NULL_RETURN_VAL(numeric_address, NULL);

   if (numeric_address[0] != '[')
     {
        const char *p = strchr(numeric_address, ':');
        if (p)
          {
             p = strchr(p + 1, ':');
             if (p)
               {
                  size_t len = strlen(numeric_address);
                  /* IPv6 no braces: ::1, etc... no port, add braces */
                  tmp = malloc(len + sizeof("[]"));
                  EINA_SAFETY_ON_NULL_RETURN_VAL(tmp, NULL);
                  tmp[0] = '[';
                  memcpy(tmp + 1, numeric_address, len);
                  tmp[1 + len] = ']';
                  address = tmp;
               }
          }
     }

   r = efl_net_ip_port_parse(address, &ss);
   free(tmp);
   if (!r)
     {
        DBG("could not parse numeric address: %s", numeric_address);
        return NULL;
     }

   return efl_add(cls, NULL,
                  efl_net_ip_address_sockaddr_set(efl_added, &ss));
}

typedef struct _Efl_Net_Ip_Address_Resolve_Context {
   Efl_Net_Ip_Address_Resolve_Results *result;
   Ecore_Thread *thread;
   Efl_Promise *promise;
} Efl_Net_Ip_Address_Resolve_Context;

static void
_efl_net_ip_address_resolve_results_free(void *data)
{
   Efl_Net_Ip_Address_Resolve_Results *r = data;

   if (r->results)
     {
        Eina_Array_Iterator it;
        unsigned int i;
        Efl_Net_Ip_Address *o;

        EINA_ARRAY_ITER_NEXT(r->results, i, o, it)
          efl_unref(o);

        eina_array_free(r->results);
        r->results = NULL;
     }

   eina_stringshare_replace(&r->canonical_name, NULL);
   eina_stringshare_replace(&r->request_address, NULL);
   free(r);
}

static void
_efl_net_ip_address_resolve_del(void *data, const Efl_Event *event EINA_UNUSED)
{
   Efl_Net_Ip_Address_Resolve_Context *ctx = data;

   ctx->promise = NULL;

   if (ctx->thread)
     {
        ecore_thread_cancel(ctx->thread);
        ctx->thread = NULL;
     }

   if (ctx->result)
     {
        _efl_net_ip_address_resolve_results_free(ctx->result);
        ctx->result = NULL;
     }

   free(ctx);
}

static inline int
_efl_net_ip_address_find(const Eina_Array *array, const struct sockaddr *addr)
{
   Eina_Array_Iterator it;
   unsigned int i;
   const Efl_Net_Ip_Address *o;

   if (addr->sa_family == AF_INET6)
     {
        EINA_ARRAY_ITER_NEXT(array, i, o, it)
          {
             const struct sockaddr *other = efl_net_ip_address_sockaddr_get(o);
             if (other->sa_family == AF_INET6)
               {
                  if (memcmp(other,  addr, sizeof(struct sockaddr_in6)) == 0)
                    return (int)i;
               }
          }
     }
   else
     {
        EINA_ARRAY_ITER_NEXT(array, i, o, it)
          {
             const struct sockaddr *other = efl_net_ip_address_sockaddr_get(o);
             if (other->sa_family == AF_INET)
               {
                  if (memcmp(other,  addr, sizeof(struct sockaddr_in)) == 0)
                    return (int)i;
               }
          }
     }
   return -1;
}

static void
_efl_net_ip_address_resolve_done(void *data, const char *host, const char *port, const struct addrinfo *hints EINA_UNUSED, struct addrinfo *result, int gai_error)
{
   Efl_Net_Ip_Address_Resolve_Context *ctx = data;
   Efl_Net_Ip_Address_Resolve_Results *r;
   const struct addrinfo *a;

   DBG("done resolving '%s' (host='%s', port='%s'): %s",
       ctx->result->request_address, host, port,
       gai_error ? gai_strerror(gai_error) : "success");

   ctx->thread = NULL;

   if (gai_error)
     {
        Eina_Error err = EFL_NET_ERROR_COULDNT_RESOLVE_HOST;

        if (gai_error == EAI_SYSTEM)
          err = errno;

        efl_promise_failed_set(ctx->promise, err);
        return;
     }

   ctx->result->results = eina_array_new(16);
   if (!ctx->result->results)
     {
        efl_promise_failed_set(ctx->promise, ENOMEM);
        return;
     }

   r = ctx->result;
   ctx->result = NULL; /* steal for efl_promise_value_set() */

   for (a = result; a != NULL; a = a->ai_next)
     {
        Eo *o;

        if (EINA_UNLIKELY((r->canonical_name == NULL) &&
                          (a->ai_canonname != NULL)))
          r->canonical_name = eina_stringshare_add(a->ai_canonname);

        /* some addresses get duplicated with different options that we
         * do not care, so check for duplicates.
         */
        if (EINA_UNLIKELY(_efl_net_ip_address_find(r->results, a->ai_addr) >= 0))
          continue;

        o = efl_net_ip_address_create_sockaddr(EFL_NET_IP_ADDRESS_CLASS, a->ai_addr);
        if (o)
          {
             if (!eina_array_push(r->results, o))
               efl_del(o);
          }
     }
   freeaddrinfo(result);

   efl_promise_value_set(ctx->promise, r, _efl_net_ip_address_resolve_results_free);
}

EOLIAN static Efl_Future *
_efl_net_ip_address_resolve(Eo *cls EINA_UNUSED, void *pd EINA_UNUSED, const char *address, int family, int flags)
{
   Efl_Net_Ip_Address_Resolve_Context *ctx;
   struct addrinfo hints = { };
   const char *host = NULL, *port = NULL;
   char *str;

   EINA_SAFETY_ON_NULL_RETURN_VAL(address, NULL);

   if (family == 0) family = AF_UNSPEC;
   EINA_SAFETY_ON_TRUE_RETURN_VAL((family != AF_UNSPEC) && (family != AF_INET) && (family != AF_INET6), NULL);

   if (flags == 0) flags = AI_ADDRCONFIG | AI_V4MAPPED | AI_CANONNAME;
   hints.ai_family = family;
   hints.ai_flags = flags;

   str = strdup(address);
   EINA_SAFETY_ON_NULL_RETURN_VAL(str, NULL);

   efl_net_ip_port_split(str, &host, &port);
   if ((!host) || (host[0] == '\0'))
     {
        host = address;
        port = "0";
     }
   if (!port) port = "0";

   ctx = calloc(1, sizeof(Efl_Net_Ip_Address_Resolve_Context));
   EINA_SAFETY_ON_NULL_GOTO(ctx, error_ctx);

   ctx->result = calloc(1, sizeof(Efl_Net_Ip_Address_Resolve_Results));
   EINA_SAFETY_ON_NULL_GOTO(ctx->result, error_result);

   ctx->result->request_address = eina_stringshare_add(address);
   EINA_SAFETY_ON_NULL_GOTO(ctx->result->request_address, error_result_address);

   ctx->thread = efl_net_ip_resolve_async_new(host, port, &hints, _efl_net_ip_address_resolve_done, ctx);
   EINA_SAFETY_ON_NULL_GOTO(ctx->thread, error_thread);

   ctx->promise = efl_add(EFL_PROMISE_CLASS, ecore_main_loop_get(),
                          efl_event_callback_add(efl_added, EFL_EVENT_DEL, _efl_net_ip_address_resolve_del, ctx));
   EINA_SAFETY_ON_NULL_GOTO(ctx->promise, error_promise);

   free(str);
   return efl_promise_future_get(ctx->promise);

 error_promise:
   ecore_thread_cancel(ctx->thread);
 error_thread:
   eina_stringshare_del(ctx->result->request_address);
 error_result_address:
   free(ctx->result);
 error_result:
   free(ctx);
 error_ctx:
   free(str);
   return NULL;
}

#include "efl_net_ip_address.eo.c"
