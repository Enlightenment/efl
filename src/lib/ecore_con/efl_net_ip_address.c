#define EFL_NET_IP_ADDRESS_PROTECTED 1

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef HAVE_ARPA_INET_H
# include <arpa/inet.h>
#endif

#ifdef _WIN32
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

typedef struct _Efl_Net_Ip_Address_Resolve_Value
{
   Eina_Stringshare *request_address; /**< The 'address' argument given to
                                 * Efl.Net.Ip_Address.resolve */
   Eina_Stringshare *canonical_name; /**< The canonical name, if it was requested in
                                * flags */
   const Eina_Value_Array results;  /**< The resolved objects. Do not modify this array but
                                     * you can keep reference to elements using efl_ref()
                                     * and efl_unref() */
} Efl_Net_Ip_Address_Resolve_Value;

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
_efl_net_ip_address_string_get(const Eo *o EINA_UNUSED, Efl_Net_Ip_Address_Data *pd)
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
_efl_net_ip_address_family_get(const Eo *o EINA_UNUSED, Efl_Net_Ip_Address_Data *pd)
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
_efl_net_ip_address_port_get(const Eo *o EINA_UNUSED, Efl_Net_Ip_Address_Data *pd)
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
_efl_net_ip_address_address_set(Eo *o EINA_UNUSED, Efl_Net_Ip_Address_Data *pd, Eina_Slice address)
{
   Eina_Rw_Slice rw_slice;
   size_t i;

   EINA_SAFETY_ON_TRUE_RETURN(pd->addr.sa_family == 0);

   rw_slice.mem = (void *)pd->addr_slice.mem;
   rw_slice.len = pd->addr_slice.len;

   EINA_SAFETY_ON_TRUE_RETURN(rw_slice.len != address.len);

   if (eina_slice_compare(eina_rw_slice_slice_get(rw_slice), address) == 0)
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
             if (!inet_ntop(pd->addr.sa_family, address.mem, new_str, sizeof(new_str)))
               {
                  new_str[0] = '?';
                  new_str[1] = '\0';
               }
             ERR("address already set to %s, new %s", old_str, new_str);
             return;
          }
     }

   eina_rw_slice_copy(rw_slice, address);
}

EOLIAN static Eina_Slice
_efl_net_ip_address_address_get(const Eo *o EINA_UNUSED, Efl_Net_Ip_Address_Data *pd)
{
   return pd->addr_slice;
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
_efl_net_ip_address_sockaddr_get(const Eo *o EINA_UNUSED, Efl_Net_Ip_Address_Data *pd)
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

   return efl_add_ref(cls, efl_main_loop_get(),
                  efl_net_ip_address_family_set(efl_added, family),
                  efl_net_ip_address_port_set(efl_added, port),
                  efl_net_ip_address_set(efl_added, address));
}

EOLIAN static Efl_Net_Ip_Address *
_efl_net_ip_address_create_sockaddr(Eo *cls, void *pd EINA_UNUSED, const void *ptr)
{
   const struct sockaddr *sockaddr = ptr;

   EINA_SAFETY_ON_NULL_RETURN_VAL(sockaddr, NULL);
   EINA_SAFETY_ON_TRUE_RETURN_VAL((sockaddr->sa_family != AF_INET) && (sockaddr->sa_family != AF_INET6), NULL);

   return efl_add_ref(cls, efl_main_loop_get(),
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
                  tmp[1 + len + 1] = '\0';
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

   return efl_add_ref(cls, efl_main_loop_get(),
                  efl_net_ip_address_sockaddr_set(efl_added, &ss));
}

typedef struct _Efl_Net_Ip_Address_Resolve_Context {
   Eina_Stringshare *request_address;
   Ecore_Thread *thread;
   Eina_Promise *promise;
} Efl_Net_Ip_Address_Resolve_Context;

static Eina_Value_Struct_Desc *
_efl_net_ip_address_resolve_value_desc_get(void)
{
   static Eina_Value_Struct_Member struct_members[] = {
     // no eina_value_type as they are not constant initializers, see below.
     EINA_VALUE_STRUCT_MEMBER(NULL, Efl_Net_Ip_Address_Resolve_Value, canonical_name),
     EINA_VALUE_STRUCT_MEMBER(NULL, Efl_Net_Ip_Address_Resolve_Value, request_address),
     EINA_VALUE_STRUCT_MEMBER(NULL, Efl_Net_Ip_Address_Resolve_Value, results)
   };
   static Eina_Value_Struct_Desc struct_desc = {
      EINA_VALUE_STRUCT_DESC_VERSION,
      NULL,
      struct_members,
      EINA_C_ARRAY_LENGTH(struct_members),
      sizeof (Efl_Net_Ip_Address_Resolve_Value)
   };
   struct_members[0].type = EINA_VALUE_TYPE_STRINGSHARE;
   struct_members[1].type = EINA_VALUE_TYPE_STRINGSHARE;
   struct_members[2].type = EINA_VALUE_TYPE_ARRAY;
   struct_desc.ops = EINA_VALUE_STRUCT_OPERATIONS_BINSEARCH;

   return &struct_desc;
}

static void
_efl_net_ip_address_resolve_del(void *data,
                                const Eina_Promise *dead_promise EINA_UNUSED)
{
   Efl_Net_Ip_Address_Resolve_Context *ctx = data;

   ctx->promise = NULL;

   eina_stringshare_replace(&ctx->request_address, NULL);

   if (ctx->thread)
     {
        ecore_thread_cancel(ctx->thread);
        ecore_thread_wait(ctx->thread, 1);
        ctx->thread = NULL;
     }

   free(ctx);
}

static inline int
_efl_net_ip_address_find(const Eina_Value *array, const struct sockaddr *addr)
{
   const Efl_Net_Ip_Address *o;
   unsigned int i, len;

   EINA_VALUE_ARRAY_FOREACH(array, len, i, o)
     {
        const struct sockaddr *other = efl_net_ip_address_sockaddr_get(o);

        if (addr->sa_family == AF_INET6)
          {
             if (other->sa_family == AF_INET6)
               {
                  if (memcmp(other,  addr, sizeof(struct sockaddr_in6)) == 0)
                    return (int)i;
               }
          }
        else
          {
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
_efl_net_ip_address_resolve_done(void *data,
                                 const char *host, const char *port,
                                 const struct addrinfo *hints EINA_UNUSED,
                                 struct addrinfo *result,
                                 int gai_error)
{
   Efl_Net_Ip_Address_Resolve_Context *ctx = data;
   Eina_Value_Array desc = { 0 };
   Eina_Value s = EINA_VALUE_EMPTY;
   Eina_Value r = EINA_VALUE_EMPTY;
   Eina_Error err = EFL_NET_ERROR_COULDNT_RESOLVE_HOST;
   const struct addrinfo *a;

   DBG("done resolving '%s' (host='%s', port='%s'): %s",
       ctx->request_address, host, port,
       gai_error ? gai_strerror(gai_error) : "success");

   ctx->thread = NULL;

   if (gai_error)
     {
        if (gai_error == EAI_SYSTEM)
          err = errno;

        goto on_error;
     }

   err = ENOMEM;
   if (!eina_value_array_setup(&r, EINA_VALUE_TYPE_OBJECT, 1))
     goto on_error;

   if (!eina_value_struct_setup(&s, _efl_net_ip_address_resolve_value_desc_get()))
     goto on_error;

   eina_value_struct_set(&s, "request_address", ctx->request_address);

   for (a = result; a != NULL; a = a->ai_next)
     {
        Eina_Stringshare *canonical_name = NULL;
        Eo *o;

        eina_value_struct_get(&s, "canonical_name", &canonical_name);
        if (EINA_UNLIKELY((canonical_name == NULL) &&
                          (a->ai_canonname != NULL)))
          {
             canonical_name = eina_stringshare_add(a->ai_canonname);
             eina_value_struct_set(&s, "canonical_name", canonical_name);
             eina_stringshare_del(canonical_name);
          }

        /* some addresses get duplicated with different options that we
         * do not care, so check for duplicates.
         */
        if (EINA_UNLIKELY(_efl_net_ip_address_find(&r, a->ai_addr) >= 0))
          continue;

        o = efl_net_ip_address_create_sockaddr(EFL_NET_IP_ADDRESS_CLASS, a->ai_addr);
        if (!o) continue ;

        eina_value_array_append(&r, o);
        efl_unref(o);
     }
   freeaddrinfo(result);

   if (!eina_value_pget(&r, &desc)) goto on_error;
   if (!eina_value_struct_pset(&s, "results", &desc)) goto on_error;
   eina_value_flush(&r);

   eina_promise_resolve(ctx->promise, s);

   eina_stringshare_replace(&ctx->request_address, NULL);
   free(ctx);

   return ;

 on_error:
   eina_promise_reject(ctx->promise, err);

   eina_stringshare_replace(&ctx->request_address, NULL);
   free(ctx);
}

EOLIAN static Eina_Future *
_efl_net_ip_address_resolve(Eo *cls EINA_UNUSED, void *pd EINA_UNUSED, const char *address, int family, int flags)
{
   Efl_Net_Ip_Address_Resolve_Context *ctx;
   struct addrinfo hints = { };
   const char *host = NULL, *port = NULL;
   Eina_Bool r;
   char *str;

   EINA_SAFETY_ON_NULL_RETURN_VAL(address, NULL);

   if (family == 0) family = AF_UNSPEC;
   EINA_SAFETY_ON_TRUE_RETURN_VAL((family != AF_UNSPEC) && (family != AF_INET) && (family != AF_INET6), NULL);

   if (flags == 0) flags = AI_ADDRCONFIG | AI_V4MAPPED | AI_CANONNAME;
   hints.ai_family = family;
   hints.ai_flags = flags;

   str = strdup(address);
   EINA_SAFETY_ON_NULL_RETURN_VAL(str, NULL);

   r = efl_net_ip_port_split(str, &host, &port);
   if ((!r) || (!host) || (host[0] == '\0'))
     {
        host = address;
        port = "0";
     }
   if (!port) port = "0";

   ctx = calloc(1, sizeof(Efl_Net_Ip_Address_Resolve_Context));
   EINA_SAFETY_ON_NULL_GOTO(ctx, error_ctx);

   ctx->request_address = eina_stringshare_add(address);
   EINA_SAFETY_ON_NULL_GOTO(ctx->request_address, error_result_address);

   ctx->thread = efl_net_ip_resolve_async_new(host, port, &hints, _efl_net_ip_address_resolve_done, ctx);
   EINA_SAFETY_ON_NULL_GOTO(ctx->thread, error_thread);

   ctx->promise = efl_loop_promise_new(efl_main_loop_get(), _efl_net_ip_address_resolve_del, ctx);
   EINA_SAFETY_ON_NULL_GOTO(ctx->promise, error_promise);

   free(str);
   return eina_future_new(ctx->promise);

 error_promise:
   ecore_thread_cancel(ctx->thread);
 error_thread:
   eina_stringshare_del(ctx->request_address);
 error_result_address:
   free(ctx);
 error_ctx:
   free(str);
   return NULL;
}

#include "efl_net_ip_address.eo.c"
