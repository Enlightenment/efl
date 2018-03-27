#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Eeze.h>
#include <Eeze_Net.h>

#include "eeze_suite.h"

EFL_START_TEST(eeze_test_net_list)
{
   Eina_List *list = NULL;

   list = eeze_net_list();
   fail_if(list == NULL);
}
EFL_END_TEST

EFL_START_TEST(eeze_test_net_attr)
{
   int idx = 0;
   Eeze_Net *net = NULL;
   const char *tmp = NULL;

   net = eeze_net_new("lo");
   fail_if(net == NULL);

   tmp = eeze_net_mac_get(net);
   fail_if(tmp == NULL);
   tmp = NULL;

   idx = eeze_net_idx_get(net);
   fail_if(!idx);

   eeze_net_scan(net);

   tmp = eeze_net_addr_get(net, EEZE_NET_ADDR_TYPE_IP);
   fail_if(tmp == NULL);
   tmp = NULL;

   tmp = eeze_net_addr_get(net, EEZE_NET_ADDR_TYPE_BROADCAST);
   fail_if(tmp == NULL);
   tmp = NULL;

   tmp = eeze_net_addr_get(net, EEZE_NET_ADDR_TYPE_NETMASK);
   fail_if(tmp == NULL);
   tmp = NULL;

   tmp = eeze_net_attribute_get(net, "carrier");
   fail_if(tmp == NULL);
   tmp = NULL;

   tmp = eeze_net_syspath_get(net);
   fail_if(tmp == NULL);

   eeze_net_free(net);
}
EFL_END_TEST


#ifdef HAVE_IPV6
EFL_START_TEST(eeze_test_net_attr_ipv6)
{
   Eeze_Net *net = NULL;
   const char *tmp = NULL;

   net = eeze_net_new("lo");
   fail_if(net == NULL);

   eeze_net_scan(net);

   tmp = eeze_net_addr_get(net, EEZE_NET_ADDR_TYPE_IP6);
   if (tmp == NULL)
     {
        printf("Problems to use IPv6 setup on loopback device. Eeze_net IPv6 test skipped.\n");
        goto cleanup;
     }
   tmp = NULL;

   tmp = eeze_net_addr_get(net, EEZE_NET_ADDR_TYPE_BROADCAST6);
   fail_if(tmp == NULL);
   tmp = NULL;

   tmp = eeze_net_addr_get(net, EEZE_NET_ADDR_TYPE_NETMASK6);
   fail_if(tmp == NULL);

cleanup:
   eeze_net_free(net);
}
EFL_END_TEST
#endif


void eeze_test_net(TCase *tc)
{
   tcase_add_test(tc, eeze_test_net_list);
   tcase_add_test(tc, eeze_test_net_attr);
#ifdef HAVE_IPV6
   tcase_add_test(tc, eeze_test_net_attr_ipv6);
#endif
}
