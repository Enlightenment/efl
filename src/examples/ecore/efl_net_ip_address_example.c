#define EFL_BETA_API_SUPPORT 1
#define EFL_EO_API_SUPPORT 1
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#include <Ecore.h>
#include <Ecore_Con.h>
#include <Ecore_Getopt.h>
#ifdef HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif
#ifdef HAVE_ARPA_INET_H
# include <arpa/inet.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif

static int retval = EXIT_SUCCESS;
static Eina_List *resolving = NULL;

static void
_print_ip_addr_info(const Eo *o)
{
   Eina_Slice addr;
   const struct sockaddr *sa;
   char buf[INET6_ADDRSTRLEN] = "?";
   uint16_t port;
   size_t i;

   printf("INFO: %p = %s\n", o, efl_net_ip_address_string_get(o));
   printf("INFO:   - family: %d (%s)\n", efl_net_ip_address_family_get(o), efl_net_ip_address_family_get(o) == AF_INET ? "AF_INET" : "AF_INET6");
   printf("INFO:   - port: %hu\n", efl_net_ip_address_port_get(o));

   addr = efl_net_ip_address_get(o);
   printf("INFO:   - address %zd bytes:", addr.len);
   for (i = 0; i < addr.len; i++)
     printf(" %#hhx", addr.bytes[i]);
   putchar('\n');

   sa = efl_net_ip_address_sockaddr_get(o);
   if (sa->sa_family == AF_INET)
     {
        struct sockaddr_in *a = (struct sockaddr_in *)sa;
        inet_ntop(sa->sa_family, &a->sin_addr, buf, sizeof(buf));
        port = eina_ntohs(a->sin_port);
     }
   else
     {
        struct sockaddr_in6 *a = (struct sockaddr_in6 *)sa;
        inet_ntop(sa->sa_family, &a->sin6_addr, buf, sizeof(buf));
        port = eina_ntohs(a->sin6_port);
     }

   printf("INFO:   - sockaddr=%p (%d, '%s', %u)\n",
          sa, sa->sa_family, buf, port);

   printf("INFO:   - ipv4_class_a: %d\n", efl_net_ip_address_ipv4_class_a_check(o));
   printf("INFO:   - ipv4_class_b: %d\n", efl_net_ip_address_ipv4_class_b_check(o));
   printf("INFO:   - ipv4_class_c: %d\n", efl_net_ip_address_ipv4_class_c_check(o));
   printf("INFO:   - ipv4_class_d: %d\n", efl_net_ip_address_ipv4_class_d_check(o));
   printf("INFO:   - ipv6_v4mapped: %d\n", efl_net_ip_address_ipv6_v4mapped_check(o));
   printf("INFO:   - ipv6_v4compat: %d\n", efl_net_ip_address_ipv6_v4compat_check(o));
   printf("INFO:   - ipv6_local_link: %d\n", efl_net_ip_address_ipv6_local_link_check(o));
   printf("INFO:   - ipv6_local_site: %d\n", efl_net_ip_address_ipv6_local_site_check(o));
   printf("INFO:   - multicast: %d\n", efl_net_ip_address_multicast_check(o));
   printf("INFO:   - loopback: %d\n", efl_net_ip_address_loopback_check(o));
   printf("INFO:   - any: %d\n", efl_net_ip_address_any_check(o));
}

static Eina_Value
_resolved(void *data, const Eina_Value v,
          const Eina_Future *dead_future)
{
   const char *address = data;
   const char *canonical_name = NULL;
   const char *request_address = NULL;
   const Eina_Value_Array desc = { 0 };
   const Efl_Net_Ip_Address **o = NULL;

   if (eina_value_type_get(&v) == EINA_VALUE_TYPE_ERROR)
     {
        Eina_Error err = 0;

        eina_value_error_get(&v, &err);

        fprintf(stderr, "ERROR: Failed to resolve '%s': %s\n",
                address, eina_error_msg_get(err));
        retval = EXIT_FAILURE;
        goto end;
     }

   eina_value_struct_get(&v, "canonical_name", canonical_name);
   eina_value_struct_get(&v, "request_address", request_address);
   eina_value_struct_get(&v, "results", &desc);

   printf("INFO: resolved '%s' to canonical name '%s':\n",
          request_address, canonical_name);

   EINA_INARRAY_FOREACH(desc.array, o)
     if (o) _print_ip_addr_info(*o);

   putchar('\n');

   eina_stringshare_del(canonical_name);
   eina_stringshare_del(request_address);

 end:
   resolving = eina_list_remove(resolving, dead_future);
   if (!resolving) ecore_main_loop_quit();

   return v;
}

int
main(int argc, char *argv[])
{
   int i;

   ecore_init();
   ecore_con_init();

   for (i = 1; i < argc; i++)
     {
        const char *address = argv[i];
        Eo *o = efl_net_ip_address_parse(EFL_NET_IP_ADDRESS_CLASS, address);
        if (o)
          {
             _print_ip_addr_info(o);
             efl_del(o);
          }
        else
          {
             Eina_Future *f = efl_net_ip_address_resolve(EFL_NET_IP_ADDRESS_CLASS, address, 0, 0);
             eina_future_then(f, _resolved, address);
             printf("INFO: %s is not numeric, resolving...\n", address);
             resolving = eina_list_append(resolving, f);
          }
     }

   if (resolving) ecore_main_loop_begin();

   ecore_con_shutdown();
   ecore_shutdown();

   return retval;
}
