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
        port = ntohs(a->sin_port);
     }
   else
     {
        struct sockaddr_in6 *a = (struct sockaddr_in6 *)sa;
        inet_ntop(sa->sa_family, &a->sin6_addr, buf, sizeof(buf));
        port = ntohs(a->sin6_port);
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

static void
_resolved(void *data EINA_UNUSED, const Efl_Event *event)
{
   Efl_Future *future = event->object;
   Efl_Future_Event_Success *f = event->info;
   Efl_Net_Ip_Address_Resolve_Results *r = f->value;
   Eina_Array_Iterator it;
   unsigned int i;
   const Efl_Net_Ip_Address *o;

   printf("INFO: resolved '%s' to canonical name '%s':\n",
          r->request_address, r->canonical_name);

   EINA_ARRAY_ITER_NEXT(r->results, i, o, it)
     _print_ip_addr_info(o);

   putchar('\n');

   resolving = eina_list_remove(resolving, future);
   if (!resolving) ecore_main_loop_quit();
}

static void
_resolve_failed(void *data, const Efl_Event *event)
{
   const char *address = data;
   Efl_Future *future = event->object;
   Efl_Future_Event_Failure *f = event->info;
   fprintf(stderr, "ERROR: Failed to resolve '%s': %s\n",
           address, eina_error_msg_get(f->error));
   retval = EXIT_FAILURE;

   resolving = eina_list_remove(resolving, future);
   if (!resolving) ecore_main_loop_quit();
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
             Efl_Future *f = efl_net_ip_address_resolve(EFL_NET_IP_ADDRESS_CLASS, address, 0, 0);
             if (!f)
               {
                  fprintf(stderr, "ERROR: cannot resolve '%s'!\n", address);
                  retval = EXIT_FAILURE;
               }
             else
               {
                  printf("INFO: %s is not numeric, resolving...\n", address);
                  efl_future_then(f, _resolved, _resolve_failed, NULL, address);
                  resolving = eina_list_append(resolving, f);
               }
          }
     }

   if (resolving) ecore_main_loop_begin();

   ecore_con_shutdown();
   ecore_shutdown();

   return retval;
}
