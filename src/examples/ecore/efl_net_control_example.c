#ifdef HAVE_SYS_SOCKET_H
 #include <sys/socket.h>
#endif
#define EFL_BETA_API_SUPPORT
#include <Efl_Net.h>
#include <Ecore_Getopt.h>
#include <ctype.h>

static Eina_Bool monitoring = EINA_TRUE;

static const char *
_access_point_state_to_str(Efl_Net_Control_Access_Point_State state)
{
   const char *strs[] = {
     [EFL_NET_CONTROL_ACCESS_POINT_STATE_IDLE] = "idle",
     [EFL_NET_CONTROL_ACCESS_POINT_STATE_ASSOCIATION] = "association",
     [EFL_NET_CONTROL_ACCESS_POINT_STATE_CONFIGURATION] = "configuration",
     [EFL_NET_CONTROL_ACCESS_POINT_STATE_LOCAL] = "local",
     [EFL_NET_CONTROL_ACCESS_POINT_STATE_ONLINE] = "online",
     [EFL_NET_CONTROL_ACCESS_POINT_STATE_DISCONNECT] = "disconnect",
     [EFL_NET_CONTROL_ACCESS_POINT_STATE_FAILURE] = "failure",
   };
   if ((unsigned)state > sizeof(strs)/sizeof(strs[0])) return "???";
   if (!strs[state]) return "???";
   return strs[state];
}

static const char *
_access_point_error_to_str(Efl_Net_Control_Access_Point_Error error)
{
   const char *strs[] = {
     [EFL_NET_CONTROL_ACCESS_POINT_ERROR_NONE] = "none",
     [EFL_NET_CONTROL_ACCESS_POINT_ERROR_OUT_OF_RANGE] = "out_of_range",
     [EFL_NET_CONTROL_ACCESS_POINT_ERROR_PIN_MISSING] = "pin_missing",
     [EFL_NET_CONTROL_ACCESS_POINT_ERROR_DHCP_FAILED] = "dhcp_failed",
     [EFL_NET_CONTROL_ACCESS_POINT_ERROR_CONNECT_FAILED] = "connect_failed",
     [EFL_NET_CONTROL_ACCESS_POINT_ERROR_LOGIN_FAILED] = "login_failed",
   };
   if ((unsigned)error > sizeof(strs)/sizeof(strs[0])) return "???";
   if (!strs[error]) return "???";
   return strs[error];
}

static const char *
_access_point_security_to_str(Efl_Net_Control_Access_Point_Security security)
{
   static char buf[128];
   int pos = 0;

#define MAP(x, s) \
   if ((security & EFL_NET_CONTROL_ACCESS_POINT_SECURITY_ ## x) == EFL_NET_CONTROL_ACCESS_POINT_SECURITY_ ## x) \
     snprintf(buf + pos, sizeof(buf) - pos, "%s%s", \
              (pos == 0) ? "" : ", ", s)

   MAP(NONE, "none");
   MAP(WEP, "wep");
   MAP(PSK, "psk");
   MAP(IEEE802_1X, "ieee802.1X");
#undef MAP

   return buf;
}

static void
_str_iterator_print(const char *header, const char *footer, Eina_Iterator *it)
{
   Eina_Bool first = EINA_TRUE;
   const char *str;

   fputs(header, stdout);
   EINA_ITERATOR_FOREACH(it, str)
     {
        if (first)
          {
             putc('[', stdout);
             first = EINA_FALSE;
          }
        else fputs(", ", stdout);
        printf("'%s'", str);
     }

   if (first) fputs("<none>", stdout);
   else putc(']', stdout);
   fputs(footer, stdout);
   eina_iterator_free(it);
}

static void
_access_point_print(Eo *ap)
{
   const char *address, *netmask, *gateway, *str;
   Efl_Net_Control_Access_Point_Ipv4_Method ipv4_method;
   Efl_Net_Control_Access_Point_Ipv6_Method ipv6_method;
   Efl_Net_Control_Access_Point_Proxy_Method proxy_method;
   uint8_t prefix_length;
   Eina_Iterator *servers, *excludes;

   printf("INFO:  - name=%s\n"
          "INFO:  - state=%s\n"
          "INFO:  - error=%s\n"
          "INFO:  - priority=%u\n"
          "INFO:  - technology=%s\n"
          "INFO:  - strength=%hhu%%\n"
          "INFO:  - roaming=%hhu\n"
          "INFO:  - auto_connect=%hhu\n"
          "INFO:  - remembered=%hhu\n"
          "INFO:  - immutable=%hhu\n"
          "INFO:  - security=%#x %s\n",
          efl_net_control_access_point_ssid_get(ap),
          _access_point_state_to_str(efl_net_control_access_point_state_get(ap)),
          _access_point_error_to_str(efl_net_control_access_point_error_get(ap)),
          efl_net_control_access_point_priority_get(ap),
          efl_name_get(efl_net_control_access_point_technology_get(ap)),
          efl_net_control_access_point_strength_get(ap),
          efl_net_control_access_point_roaming_get(ap),
          efl_net_control_access_point_auto_connect_get(ap),
          efl_net_control_access_point_remembered_get(ap),
          efl_net_control_access_point_immutable_get(ap),
          efl_net_control_access_point_security_get(ap),
          _access_point_security_to_str(efl_net_control_access_point_security_get(ap)));

   /* actual values */
   efl_net_control_access_point_ipv4_get(ap, &ipv4_method, &address, &netmask, &gateway);
   if (ipv4_method == EFL_NET_CONTROL_ACCESS_POINT_IPV4_METHOD_OFF)
     printf("INFO:  - ipv4=off\n");
   else
     {
        str = "???";
        switch (ipv4_method)
          {
           case EFL_NET_CONTROL_ACCESS_POINT_IPV4_METHOD_UNSET:
              str = "<unset>"; break;
           case EFL_NET_CONTROL_ACCESS_POINT_IPV4_METHOD_OFF:
              str = "off"; break;
           case EFL_NET_CONTROL_ACCESS_POINT_IPV4_METHOD_DHCP:
              str = "dhcp"; break;
           case EFL_NET_CONTROL_ACCESS_POINT_IPV4_METHOD_MANUAL:
              str = "manual"; break;
          }
        printf("INFO:  - ipv4=%s address=%s, netmask=%s, gateway=%s\n",
               str, address, netmask, gateway);
     }

   efl_net_control_access_point_ipv6_get(ap, &ipv6_method, &address, &prefix_length, &netmask, &gateway);
   str = "???";
   switch (ipv6_method)
     {
      case EFL_NET_CONTROL_ACCESS_POINT_IPV6_METHOD_FIXED:
         str = "fixed (operator)"; break;
      case EFL_NET_CONTROL_ACCESS_POINT_IPV6_METHOD_MANUAL:
         str = "manual"; break;
      case EFL_NET_CONTROL_ACCESS_POINT_IPV6_METHOD_AUTO_PRIVACY_NONE:
         str = "auto (privacy=none)"; break;
      case EFL_NET_CONTROL_ACCESS_POINT_IPV6_METHOD_AUTO_PRIVACY_PUBLIC:
         str = "auto (privacy=public)"; break;
      case EFL_NET_CONTROL_ACCESS_POINT_IPV6_METHOD_AUTO_PRIVACY_TEMPORARY:
         str = "auto (privacy=temporary)"; break;
      case EFL_NET_CONTROL_ACCESS_POINT_IPV6_METHOD_TUNNEL6TO4:
         str = "6-to-4"; break;
      case EFL_NET_CONTROL_ACCESS_POINT_IPV6_METHOD_OFF:
         str = "off"; break;
      case EFL_NET_CONTROL_ACCESS_POINT_IPV6_METHOD_UNSET:
         str = "<unset>"; break;
     }
   if ((ipv6_method == EFL_NET_CONTROL_ACCESS_POINT_IPV6_METHOD_OFF) ||
       (ipv6_method == EFL_NET_CONTROL_ACCESS_POINT_IPV6_METHOD_UNSET))
     printf("INFO:  - ipv6=%s\n", str);
   else
     printf("INFO:  - ipv6=%s address=%s/%hhu, netmask=%s, gateway=%s\n",
            str, address, prefix_length, netmask, gateway);

   _str_iterator_print("INFO:  - name_servers=", "\n",
                       efl_net_control_access_point_name_servers_get(ap));
   _str_iterator_print("INFO:  - time_servers=", "\n",
                       efl_net_control_access_point_time_servers_get(ap));
   _str_iterator_print("INFO:  - domains=", "\n",
                       efl_net_control_access_point_domains_get(ap));

   efl_net_control_access_point_proxy_get(ap, &proxy_method, &address, &servers, &excludes);
   str = "???";
   switch (proxy_method)
     {
      case EFL_NET_CONTROL_ACCESS_POINT_PROXY_METHOD_OFF:
         str = "off"; break;
      case EFL_NET_CONTROL_ACCESS_POINT_PROXY_METHOD_AUTO:
         str = "auto"; break;
      case EFL_NET_CONTROL_ACCESS_POINT_PROXY_METHOD_MANUAL:
         str = "manual"; break;
      case EFL_NET_CONTROL_ACCESS_POINT_PROXY_METHOD_UNSET:
         str = "<unset>"; break;
     }
   printf("INFO:  - proxy=%s, address=%s", str, address ? address : "<none>");
   _str_iterator_print(", servers=", "", servers);
   _str_iterator_print(", excludes=", "\n", excludes);


   /* configured values */
   efl_net_control_access_point_configuration_ipv4_get(ap, &ipv4_method, &address, &netmask, &gateway);
   if (ipv4_method == EFL_NET_CONTROL_ACCESS_POINT_IPV4_METHOD_UNSET)
     printf("INFO:  - configured ipv4=<unset>\n");
   else if (ipv4_method == EFL_NET_CONTROL_ACCESS_POINT_IPV4_METHOD_OFF)
     printf("INFO:  - configured ipv4=off\n");
   else
     {
        str = "???";
        switch (ipv4_method)
          {
           case EFL_NET_CONTROL_ACCESS_POINT_IPV4_METHOD_UNSET:
              str = "<unset>"; break;
           case EFL_NET_CONTROL_ACCESS_POINT_IPV4_METHOD_OFF:
              str = "off"; break;
           case EFL_NET_CONTROL_ACCESS_POINT_IPV4_METHOD_DHCP:
              str = "dhcp"; break;
           case EFL_NET_CONTROL_ACCESS_POINT_IPV4_METHOD_MANUAL:
              str = "manual"; break;
          }
        printf("INFO:  - configured ipv4=%s address=%s, netmask=%s, gateway=%s\n",
               str, address, netmask, gateway);
     }

   efl_net_control_access_point_configuration_ipv6_get(ap, &ipv6_method, &address, &prefix_length, &netmask, &gateway);
   str = "???";
   switch (ipv6_method)
     {
      case EFL_NET_CONTROL_ACCESS_POINT_IPV6_METHOD_FIXED:
         str = "fixed (operator)"; break;
      case EFL_NET_CONTROL_ACCESS_POINT_IPV6_METHOD_MANUAL:
         str = "manual"; break;
      case EFL_NET_CONTROL_ACCESS_POINT_IPV6_METHOD_AUTO_PRIVACY_NONE:
         str = "auto (privacy=none)"; break;
      case EFL_NET_CONTROL_ACCESS_POINT_IPV6_METHOD_AUTO_PRIVACY_PUBLIC:
         str = "auto (privacy=public)"; break;
      case EFL_NET_CONTROL_ACCESS_POINT_IPV6_METHOD_AUTO_PRIVACY_TEMPORARY:
         str = "auto (privacy=temporary)"; break;
      case EFL_NET_CONTROL_ACCESS_POINT_IPV6_METHOD_TUNNEL6TO4:
         str = "6-to-4"; break;
      case EFL_NET_CONTROL_ACCESS_POINT_IPV6_METHOD_OFF:
         str = "off"; break;
      case EFL_NET_CONTROL_ACCESS_POINT_IPV6_METHOD_UNSET:
         str = "<unset>"; break;
     }
   if ((ipv6_method == EFL_NET_CONTROL_ACCESS_POINT_IPV6_METHOD_OFF) ||
       (ipv6_method == EFL_NET_CONTROL_ACCESS_POINT_IPV6_METHOD_UNSET))
     printf("INFO:  - configured ipv6=%s\n", str);
   else
     printf("INFO:  - configured ipv6=%s address=%s/%hhu, netmask=%s, gateway=%s\n",
               str, address, prefix_length, netmask, gateway);

   _str_iterator_print("INFO:  - configured name_servers=", "\n",
                       efl_net_control_access_point_configuration_name_servers_get(ap));
   _str_iterator_print("INFO:  - configured time_servers=", "\n",
                       efl_net_control_access_point_configuration_time_servers_get(ap));
   _str_iterator_print("INFO:  - configured domains=", "\n",
                       efl_net_control_access_point_configuration_domains_get(ap));

   efl_net_control_access_point_configuration_proxy_get(ap, &proxy_method, &address, &servers, &excludes);
   str = "???";
   switch (proxy_method)
     {
      case EFL_NET_CONTROL_ACCESS_POINT_PROXY_METHOD_OFF:
         str = "off"; break;
      case EFL_NET_CONTROL_ACCESS_POINT_PROXY_METHOD_AUTO:
         str = "auto"; break;
      case EFL_NET_CONTROL_ACCESS_POINT_PROXY_METHOD_MANUAL:
         str = "manual"; break;
      case EFL_NET_CONTROL_ACCESS_POINT_PROXY_METHOD_UNSET:
         str = "<unset>"; break;
     }
   printf("INFO:  - configured proxy=%s, address=%s", str, address ? address : "<none>");
   _str_iterator_print(", servers=", "", servers);
   _str_iterator_print(", excludes=", "\n", excludes);
}

static void
_access_point_changed(void *data EINA_UNUSED, const Efl_Event *event)
{
   Eo *ap = event->object;
   if (!monitoring) return;
   printf("INFO: Access Point Changed:\n");
   _access_point_print(ap);
}

static void
_ctl_access_point_add(void *data EINA_UNUSED, const Efl_Event *event)
{
   Eo *ap = event->info;
   if (!monitoring) return;
   printf("INFO: Access Point Added:\n");
   _access_point_print(ap);
   efl_event_callback_add(ap, EFL_NET_CONTROL_ACCESS_POINT_EVENT_CHANGED, _access_point_changed, NULL);
}

static void
_ctl_access_point_del(void *data EINA_UNUSED, const Efl_Event *event)
{
   Eo *ap = event->info;
   if (!monitoring) return;
   printf("INFO: Access Point Removed: %s [%s]\n",
          efl_net_control_access_point_ssid_get(ap),
          efl_name_get(efl_net_control_access_point_technology_get(ap)));
}

static void
_access_points_list(Eina_Iterator *it)
{
   Eo *ap;
   Eina_Bool first = EINA_TRUE;

   EINA_ITERATOR_FOREACH(it, ap)
     {
        if (first)
          {
             printf("INFO: PRIO | NAME               | FLAGS | STATE         | TECH\n"
                    "INFO: -----+--------------------+-------+---------------+---------------\n");
             first = EINA_FALSE;
          }

        printf("INFO: %4u | %-18.18s | %c%c%c   | %-13.13s | %s\n",
               efl_net_control_access_point_priority_get(ap),
               efl_net_control_access_point_ssid_get(ap),
               efl_net_control_access_point_remembered_get(ap) ? 'R' : '.',
               efl_net_control_access_point_immutable_get(ap) ? 'I' : '.',
               efl_net_control_access_point_auto_connect_get(ap) ? 'A' : '.',
               _access_point_state_to_str(efl_net_control_access_point_state_get(ap)),
               efl_name_get(efl_net_control_access_point_technology_get(ap)));
     }

   if (!first)
     {
        printf("INFO: -----+--------------------+-------+---------------+---------------\n"
               "INFO: FLAGS: [R]emembered, [I]mmutable, [A]uto-connect\n");
     }
   eina_iterator_free(it);
}

static void
_ctl_access_points_changed(void *data EINA_UNUSED, const Efl_Event *event)
{
   if (!monitoring) return;
   _access_points_list(efl_net_control_manager_access_points_get(event->object));
}

static const char *
_technology_type_str(Efl_Net_Control_Technology_Type type)
{
   const char *strs[] = {
     [EFL_NET_CONTROL_TECHNOLOGY_TYPE_UNKNOWN] = "unknown",
     [EFL_NET_CONTROL_TECHNOLOGY_TYPE_SYSTEM] = "system",
     [EFL_NET_CONTROL_TECHNOLOGY_TYPE_ETHERNET] = "ethernet",
     [EFL_NET_CONTROL_TECHNOLOGY_TYPE_WIFI] = "wifi",
     [EFL_NET_CONTROL_TECHNOLOGY_TYPE_BLUETOOTH] = "bluetooth",
     [EFL_NET_CONTROL_TECHNOLOGY_TYPE_CELLULAR] = "cellular",
     [EFL_NET_CONTROL_TECHNOLOGY_TYPE_GPS] = "gps",
     [EFL_NET_CONTROL_TECHNOLOGY_TYPE_VPN] = "vpn",
     [EFL_NET_CONTROL_TECHNOLOGY_TYPE_GADGET] = "gadget",
     [EFL_NET_CONTROL_TECHNOLOGY_TYPE_P2P] = "p2p",
   };
   if ((unsigned)type > sizeof(strs)/sizeof(strs[0])) return "???";
   if (!strs[type]) return "???";
   return strs[type];
}

static void
_technology_print(Eo *tech)
{
   Eina_Bool tethering;
   const char *teth_id, *teth_pass;

   efl_net_control_technology_tethering_get(tech, &tethering, &teth_id, &teth_pass);
   if (!tethering) teth_id = teth_pass = "";

   printf("INFO:  - name=%s\n"
          "INFO:  - powered=%hhu\n"
          "INFO:  - connected=%hhu\n"
          "INFO:  - tethering=%hhu (id='%s', passphrase='%s')\n"
          "INFO:  - type=%s\n",
          efl_name_get(tech),
          efl_net_control_technology_powered_get(tech),
          efl_net_control_technology_connected_get(tech),
          tethering, teth_id, teth_pass,
          _technology_type_str(efl_net_control_technology_type_get(tech)));
}

static void
_technology_changed(void *data EINA_UNUSED, const Efl_Event *event)
{
   Eo *ap = event->object;
   if (!monitoring) return;
   printf("INFO: Technology Changed:\n");
   _technology_print(ap);
}

static void
_ctl_technology_add(void *data EINA_UNUSED, const Efl_Event *event)
{
   Eo *tech = event->info;
   if (!monitoring) return;
   printf("INFO: Technology Added:\n");
   _technology_print(tech);
   efl_event_callback_add(tech, EFL_NET_CONTROL_TECHNOLOGY_EVENT_CHANGED, _technology_changed, NULL);
}

static void
_ctl_technology_del(void *data EINA_UNUSED, const Efl_Event *event)
{
   Eo *tech = event->info;
   if (!monitoring) return;
   printf("INFO: Technology Removed: %s [%s]\n",
          efl_name_get(tech),
          _technology_type_str(efl_net_control_technology_type_get(tech)));
}

static void
_ctl_radios_offline_changed(void *data EINA_UNUSED, const Efl_Event *event)
{
   if (!monitoring) return;
   printf("INFO: radios_offline=%hhu\n", efl_net_control_manager_radios_offline_get(event->object));
}

static void
_ctl_state_changed(void *data EINA_UNUSED, const Efl_Event *event)
{
   const char *str = "???";
   Efl_Net_Control_State state = efl_net_control_manager_state_get(event->object);
   switch (state)
     {
      case EFL_NET_CONTROL_STATE_OFFLINE: str = "offline"; break;
      case EFL_NET_CONTROL_STATE_ONLINE: str = "online"; break;
      case EFL_NET_CONTROL_STATE_LOCAL: str = "local"; break;
     }
   if (!monitoring) return;
   printf("INFO: state=%s\n", str);
}

static void
_ctl_agent_released(void *data EINA_UNUSED, const Efl_Event *event EINA_UNUSED)
{
   printf("INFO: Agent released\n");
}

static void
_ctl_agent_error(void *data EINA_UNUSED, const Efl_Event *event)
{
   const Efl_Net_Control_Agent_Error *e = event->info;
   printf("INFO: Agent Error:\n"
          "INFO:  - Access Point: %s\n"
          "INFO:  - Message: %s\n",
          efl_net_control_access_point_ssid_get(e->access_point),
          e->message);
}

static void
_ctl_agent_browser_url(void *data EINA_UNUSED, const Efl_Event *event)
{
   const Efl_Net_Control_Agent_Browser_Url *e = event->info;
   printf("INFO: Agent requested to open browser:\n"
          "INFO:  - Access Point: %s\n"
          "INFO:  - URL: %s\n",
          efl_net_control_access_point_ssid_get(e->access_point),
          e->url);
}

static void
_ctl_agent_request_input(void *data EINA_UNUSED, const Efl_Event *event)
{
   char buf[100];
   Eo *ctl = event->object;
   Efl_Net_Control_Agent_Request_Input *ri = event->info;
   Eina_List *n;
   Efl_Net_Control_Agent_Request_Input_Information *info;
   char *name = NULL;
   char *username = NULL;
   char *passphrase = NULL;
   char *ssid = NULL;
   char *wps = NULL;
   Eina_Slice ssid_slice = { };
   size_t len;

   printf("INFO: Needs agent input!\n");

   EINA_LIST_FOREACH(ri->informational, n, info)
     printf("INFO:  - %s: %s\n", info->name, info->value);


   if (ri->fields)
     {
        printf("INFO: The following fields may be entered:\n");
        if (ri->fields & EFL_NET_CONTROL_AGENT_REQUEST_INPUT_FIELD_NAME)
          {
             printf("INFO: Name="); fflush(stdout);
             if ((name = fgets(buf, sizeof(buf), stdin)) != NULL)
               {
                  len = strlen(name);
                  if (name[len - 1] == '\n')
                    {
                       name[len - 1] = '\0';
                    }
                  else
                    {
                       free(name);
                       name = NULL;
                    }
               }
          }

        if (ri->fields & EFL_NET_CONTROL_AGENT_REQUEST_INPUT_FIELD_SSID)
          {
             printf("INFO: SSID="); fflush(stdout);
             if ((ssid = fgets(buf, sizeof(buf), stdin)) != NULL)
               {
                  len = strlen(ssid);
                  if (ssid[len - 1] == '\n')
                    {
                       ssid[len - 1] = '\0';
                       ssid_slice.mem = ssid;
                       ssid_slice.len = len - 1;
                    }
                  else
                    {
                       free(ssid);
                       ssid = NULL;
                    }
               }
          }

        if (ri->fields & EFL_NET_CONTROL_AGENT_REQUEST_INPUT_FIELD_USERNAME)
          {
             printf("INFO: Username="); fflush(stdout);
             if ((username = fgets(buf, sizeof(buf), stdin)) != NULL)
               {
                  len = strlen(username);
                  if (username[len - 1] == '\n')
                    {
                       username[len - 1] = '\0';
                    }
                  else
                    {
                       free(username);
                       username = NULL;
                    }
               }
          }

        if (ri->fields & EFL_NET_CONTROL_AGENT_REQUEST_INPUT_FIELD_PASSPHRASE)
          {
             if (ri->passphrase_type)
               printf("INFO: Passphrase(%s)=", ri->passphrase_type);
             else
               printf("INFO: Passphrase=");
             fflush(stdout);
             if ((passphrase = fgets(buf, sizeof(buf), stdin)) != NULL)
               {
                  len = strlen(passphrase);
                  if (passphrase[len - 1] == '\n')
                    {
                       passphrase[len - 1] = '\0';
                    }
                  else
                    {
                       free(passphrase);
                       passphrase = NULL;
                    }
               }
          }

        if (ri->fields & EFL_NET_CONTROL_AGENT_REQUEST_INPUT_FIELD_WPS)
          {
             printf("INFO: WPS (use a single dot, '.', for pushbutton)="); fflush(stdout);
             if ((wps = fgets(buf, sizeof(buf), stdin)) != NULL)
               {
                  len = strlen(wps);
                  if (wps[len - 1] == '\n')
                    {
                       wps[len - 1] = '\0';
                       if ((len == 2) && (wps[0] == '.'))
                         wps[0] = '\0'; /* API uses empty string for pushbutton */
                    }
                  else
                    {
                       free(wps);
                       wps = NULL;
                    }
               }
          }
     }

   efl_net_control_manager_agent_reply(ctl,
                               name,
                               ssid ? &ssid_slice : NULL,
                               username,
                               passphrase,
                               wps);

   free(name);
   free(ssid);
   free(username);
   free(passphrase);
   free(wps);
}

static void
_cmd_technologies_list(Eo *ctl, size_t argc EINA_UNUSED, char **argv EINA_UNUSED)
{
   Eina_Iterator *it = efl_net_control_manager_technologies_get(ctl);
   Eo *tech;
   Eina_Bool first = EINA_TRUE;

   EINA_ITERATOR_FOREACH(it, tech)
     {
        if (first)
          {
             printf("INFO:  NAME               | ON | CONN | TYPE\n"
                    "INFO: --------------------+----+------+---------------\n");
             first = EINA_FALSE;
          }

        printf("INFO:  %-18.18s | %c  | %c    | %s\n",
               efl_name_get(tech),
               efl_net_control_technology_powered_get(tech) ? 'X' : ' ',
               efl_net_control_technology_connected_get(tech) ? 'X' : ' ',
               _technology_type_str(efl_net_control_technology_type_get(tech)));
     }

   if (!first)
     {
        printf("INFO: --------------------+----+------+---------------\n");
     }
   eina_iterator_free(it);
}

static Eina_Bool
_parse_bool(const char *header, const char *str, Eina_Bool *ret)
{
   if (str == NULL)
     {
        fprintf(stderr, "ERROR: %s requires a boolean.\n", header);
        return EINA_FALSE;
     }
   if (strcmp(str, "on") == 0)
     {
        *ret = EINA_TRUE;
        return EINA_TRUE;
     }
   else if (strcmp(str, "off") == 0)
     {
        *ret = EINA_FALSE;
        return EINA_TRUE;
     }
   else
     {
        fprintf(stderr, "ERROR: %s required boolean 'on' or 'off', got '%s'\n", header, str);
        return EINA_FALSE;
     }
}

static const char *
_fmt_bool(Eina_Bool val)
{
   return val ? "on" : "off";
}

static Eo *
_technology_find(Eo *ctl, const char *name)
{
   Eina_Iterator *it;
   Eo *child;

   if (!name)
     {
        fprintf(stderr, "ERROR: required technology name\n");
        return NULL;
     }

   it = efl_net_control_manager_technologies_get(ctl);
   EINA_ITERATOR_FOREACH(it, child)
     {
        const char *tn = efl_name_get(child);
        if (strcasecmp(name, tn) == 0)
          {
             eina_iterator_free(it);
             return child;
          }
     }
   fprintf(stderr, "ERROR: did not find technology named '%s'\n", name);
   eina_iterator_free(it);
   return NULL;
}

static void
_cmd_technology_show(Eo *ctl, size_t argc EINA_UNUSED, char **argv)
{
   Eo *tech = _technology_find(ctl, argv[1]);
   if (!tech) return;
   printf("INFO: show technology '%s'\n", argv[1]);
   _technology_print(tech);
}

static Eina_Value
_technology_scan_done(void *data, const Eina_Value v,
                      const Eina_Future *dead EINA_UNUSED)
{
   Eo *tech = data;

   if (v.type == EINA_VALUE_TYPE_ERROR)
     {
        Eina_Error err = 0;

        eina_value_error_get(&v, &err);
        printf("INFO: technology '%s' could not scan: %s\n",
               efl_name_get(tech),
               eina_error_msg_get(err));
     }
   else
     {
        printf("INFO: technology '%s' finished scan.\n",
               efl_name_get(tech));
     }

   return v;
}

static void
_cmd_technology_scan(Eo *ctl, size_t argc EINA_UNUSED, char **argv)
{
   Eo *tech = _technology_find(ctl, argv[1]);
   if (!tech) return;
   printf("INFO: started scan on technology '%s'\n", argv[1]);
   eina_future_then(efl_net_control_technology_scan(tech),
                    _technology_scan_done, tech);
}

static void
_cmd_technology_powered(Eo *ctl, size_t argc, char **argv)
{
   Eo *tech = _technology_find(ctl, argv[1]);
   const char *name;
   Eina_Bool powered;

   if (!tech) return;

   name = efl_name_get(tech);
   if (argc <= 2)
     {
        printf("INFO: technology '%s' powered %s\n",
               name,
               _fmt_bool(efl_net_control_technology_powered_get(tech)));
        return;
     }

   if (!_parse_bool(argv[0], argv[2], &powered))
     return;

   printf("INFO: technology '%s' set to powered %s\n",
          name, _fmt_bool(powered));
   efl_net_control_technology_powered_set(tech, powered);
}

static void
_cmd_technology_tethering(Eo *ctl, size_t argc, char **argv)
{
   Eo *tech = _technology_find(ctl, argv[1]);
   const char *name;
   Eina_Bool enabled;
   const char *id = NULL, *pass = NULL;

   if (!tech) return;

   name = efl_name_get(tech);
   if (argc <= 2)
     {
        efl_net_control_technology_tethering_get(tech, &enabled, &id, &pass);
        printf("INFO: technology '%s' tethering %s, id='%s', passphrase='%s'\n",
               name,
               _fmt_bool(efl_net_control_technology_powered_get(tech)),
               id ? id : "", pass ? pass : "");
        return;
     }

   if (!_parse_bool(argv[0], argv[2], &enabled))
     return;

   if (argc > 3) id = argv[3];
   if (argc > 4) pass = argv[4];

   printf("INFO: technology '%s' set to tethering %s, id='%s', pass='%s'\n",
          name, _fmt_bool(enabled), id ? id : "", pass ? pass : "");
   efl_net_control_technology_tethering_set(tech, enabled, id, pass);
}

static void
_cmd_access_points_list(Eo *ctl, size_t argc EINA_UNUSED, char **argv EINA_UNUSED)
{
   _access_points_list(efl_net_control_manager_access_points_get(ctl));
}

static Eo *
_access_point_find(Eo *ctl, const char *name)
{
   Eina_Iterator *it;
   Eo *child;
   uint32_t prio = UINT32_MAX;

   if (!name)
     {
        fprintf(stderr, "ERROR: required access_point name\n");
        return NULL;
     }

   if (name[0] == '#')
     prio = strtoul(name + 1, NULL, 10);

   it = efl_net_control_manager_access_points_get(ctl);
   EINA_ITERATOR_FOREACH(it, child)
     {
        if (prio == UINT32_MAX)
          {
             const char *n = efl_net_control_access_point_ssid_get(child);
             if (strcasecmp(name, n) == 0)
               {
                  eina_iterator_free(it);
                  return child;
               }
          }
        else
          {
             uint32_t p = efl_net_control_access_point_priority_get(child);
             if (p == prio)
               {
                  eina_iterator_free(it);
                  return child;
               }
          }
     }

   if (prio == UINT32_MAX)
     fprintf(stderr, "ERROR: did not find access_point named '%s'\n", name);
   else
     fprintf(stderr, "ERROR: did not find access_point at priority '%u'\n", prio);
   eina_iterator_free(it);
   return NULL;
}

static void
_cmd_access_point_show(Eo *ctl, size_t argc EINA_UNUSED, char **argv)
{
   Eo *ap = _access_point_find(ctl, argv[1]);
   if (!ap) return;
   printf("INFO: show access point '%s'\n", argv[1]);
   _access_point_print(ap);
}

static Eina_Value
_access_point_connect(void *data, const Eina_Value v,
                      const Eina_Future *dead EINA_UNUSED)
{
   Eo *ap = data;

   if (v.type == EINA_VALUE_TYPE_ERROR)
     {
        Eina_Error err = 0;

        eina_value_error_get(&v, &err);
        printf("INFO: access point '%s' could not connect: %s\n",
               efl_net_control_access_point_ssid_get(ap),
               eina_error_msg_get(err));
     }
   else
     {
        printf("INFO: access point '%s' finished connect.\n",
               efl_net_control_access_point_ssid_get(ap));
     }

   return v;
}

static void
_cmd_access_point_connect(Eo *ctl, size_t argc EINA_UNUSED, char **argv)
{
   Eo *ap = _access_point_find(ctl, argv[1]);
   if (!ap) return;
   printf("INFO: connecting point '%s'\n", argv[1]);
   eina_future_then(efl_net_control_access_point_connect(ap),
                    _access_point_connect, ap);
}

static void
_cmd_access_point_disconnect(Eo *ctl, size_t argc EINA_UNUSED, char **argv)
{
   Eo *ap = _access_point_find(ctl, argv[1]);
   if (!ap) return;
   printf("INFO: disconnect access point '%s'\n", argv[1]);
   efl_net_control_access_point_disconnect(ap);
}

static void
_cmd_access_point_forget(Eo *ctl, size_t argc EINA_UNUSED, char **argv)
{
   Eo *ap = _access_point_find(ctl, argv[1]);
   if (!ap) return;
   printf("INFO: forget access point '%s'\n", argv[1]);
   efl_net_control_access_point_forget(ap);
}

static void
_cmd_access_point_reorder(Eo *ctl, size_t argc, char **argv)
{
   Eo *ap = _access_point_find(ctl, argv[1]);
   uint32_t prio;
   if (!ap) return;

   if (argc < 3)
     {
        fprintf(stderr, "ERROR: missing priority\n");
        return;
     }
   else if ((strcasecmp(argv[2], "UINT32_MAX") == 0) ||
            (strcasecmp(argv[2], "UINT_MAX") == 0) ||
            (strcasecmp(argv[2], "last") == 0))
     prio = UINT32_MAX;
   else
     prio = strtoul(argv[2], NULL, 10);

   printf("INFO: reorder access point '%s' %u -> %u\n",
          argv[1], efl_net_control_access_point_priority_get(ap), prio);
   efl_net_control_access_point_priority_set(ap, prio);
}

static void
_cmd_access_point_auto_connect(Eo *ctl, size_t argc EINA_UNUSED, char **argv)
{
   Eo *ap = _access_point_find(ctl, argv[1]);
   const char *name;
   Eina_Bool auto_connect;

   if (!ap) return;

   name = efl_net_control_access_point_ssid_get(ap);
   if (argc <= 2)
     {
        printf("INFO: access_point '%s' auto connect %s\n",
               name,
               _fmt_bool(efl_net_control_access_point_auto_connect_get(ap)));
        return;
     }

   if (!_parse_bool(argv[0], argv[2], &auto_connect))
     return;

   printf("INFO: access_point '%s' set to auto connect %s\n",
          name, _fmt_bool(auto_connect));
   efl_net_control_access_point_auto_connect_set(ap, auto_connect);
}

static void
_cmd_access_point_configure_name_servers(Eo *ctl, size_t argc EINA_UNUSED, char **argv)
{
   Eo *ap = _access_point_find(ctl, argv[1]);
   Eina_Iterator *it;
   if (!ap) return;

   it = eina_carray_iterator_new((void **)(argv + 2));
   printf("INFO: configure access point '%s' name servers\n", argv[1]);
   efl_net_control_access_point_configuration_name_servers_set(ap, it);
}

static void
_cmd_access_point_configure_time_servers(Eo *ctl, size_t argc EINA_UNUSED, char **argv)
{
   Eo *ap = _access_point_find(ctl, argv[1]);
   Eina_Iterator *it;
   if (!ap) return;

   it = eina_carray_iterator_new((void **)(argv + 2));
   printf("INFO: configure access point '%s' time servers\n", argv[1]);
   efl_net_control_access_point_configuration_time_servers_set(ap, it);
}

static void
_cmd_access_point_configure_domains(Eo *ctl, size_t argc EINA_UNUSED, char **argv)
{
   Eo *ap = _access_point_find(ctl, argv[1]);
   Eina_Iterator *it;
   if (!ap) return;

   it = eina_carray_iterator_new((void **)(argv + 2));
   printf("INFO: configure access point '%s' domains\n", argv[1]);
   efl_net_control_access_point_configuration_domains_set(ap, it);
}

static void
_cmd_monitor_set(Eo *ctl EINA_UNUSED, size_t argc, char **argv)
{
   if (argc == 1)
     {
        printf("INFO: monitor is %s\n", _fmt_bool(monitoring));
        return;
     }

   if (!_parse_bool(argv[0], argv[1], &monitoring))
     return;

   printf("INFO: monitor is now %s\n", _fmt_bool(monitoring));
}

static void
_cmd_agent_set(Eo *ctl, size_t argc, char **argv)
{
   Eina_Bool enabled;

   if (argc == 1)
     {
        printf("INFO: agent is %s\n", _fmt_bool(efl_net_control_manager_agent_enabled_get(ctl)));
        return;
     }

   if (!_parse_bool(argv[0], argv[1], &enabled))
     return;

   efl_net_control_manager_agent_enabled_set(ctl, enabled);
   printf("INFO: agent is now %s\n", _fmt_bool(enabled));
}

static void
_cmd_access_point_configure_ipv4(Eo *ctl, size_t argc, char **argv)
{
   Eo *ap = _access_point_find(ctl, argv[1]);
   const char *name;
   Efl_Net_Control_Access_Point_Ipv4_Method ipv4_method;
   const char *address = NULL;
   const char *netmask = NULL;
   const char *gateway = NULL;

   if (!ap) return;

   if (argc <= 2)
     {
        fprintf(stderr, "ERROR: missing IPv4 configuration method: off, dhcp or manual.\n");
        return;
     }

   name = efl_net_control_access_point_ssid_get(ap);

   if (strcmp(argv[2], "off") == 0)
     ipv4_method = EFL_NET_CONTROL_ACCESS_POINT_IPV4_METHOD_OFF;
   else if (strcmp(argv[2], "dhcp") == 0)
     ipv4_method = EFL_NET_CONTROL_ACCESS_POINT_IPV4_METHOD_DHCP;
   else if (strcmp(argv[2], "manual") == 0)
     ipv4_method = EFL_NET_CONTROL_ACCESS_POINT_IPV4_METHOD_MANUAL;
   else
     {
        fprintf(stderr, "ERROR: invalid IPv4 configuration method '%s', expected: off, dhcp or manual.\n", argv[2]);
        return;
     }

   if (argc > 3) address = argv[3];
   if (argc > 4) netmask = argv[4];
   if (argc > 5) gateway = argv[5];

   printf("INFO: access point '%s' IPv4 set to %s address=%s, netmask=%s, gateway=%s\n",
          name, argv[2], address, netmask, gateway);
   efl_net_control_access_point_configuration_ipv4_set(ap, ipv4_method, address, netmask, gateway);
}

static void
_cmd_access_point_configure_ipv6(Eo *ctl, size_t argc, char **argv)
{
   Eo *ap = _access_point_find(ctl, argv[1]);
   const char *name;
   Efl_Net_Control_Access_Point_Ipv6_Method ipv6_method;
   const char *address = NULL;
   const char *netmask = NULL;
   const char *gateway = NULL;
   uint8_t prefix = 0;

   if (!ap) return;

   if (argc <= 2)
     {
        fprintf(stderr, "ERROR: missing IPv6 configuration method: off, manual, auto-privacy-none, auto-privacy-public or auto-privacy-temporary.\n");
        return;
     }

   name = efl_net_control_access_point_ssid_get(ap);

   if (strcmp(argv[2], "off") == 0)
     ipv6_method = EFL_NET_CONTROL_ACCESS_POINT_IPV6_METHOD_OFF;
   else if (strcmp(argv[2], "manual") == 0)
     ipv6_method = EFL_NET_CONTROL_ACCESS_POINT_IPV6_METHOD_MANUAL;
   else if (strcmp(argv[2], "auto-privacy-none") == 0)
     ipv6_method = EFL_NET_CONTROL_ACCESS_POINT_IPV6_METHOD_AUTO_PRIVACY_NONE;
   else if (strcmp(argv[2], "auto-privacy-public") == 0)
     ipv6_method = EFL_NET_CONTROL_ACCESS_POINT_IPV6_METHOD_AUTO_PRIVACY_PUBLIC;
   else if (strcmp(argv[2], "auto-privacy-temporary") == 0)
     ipv6_method = EFL_NET_CONTROL_ACCESS_POINT_IPV6_METHOD_AUTO_PRIVACY_TEMPORARY;
   else
     {
        fprintf(stderr, "ERROR: invalid IPv6 configuration method '%s', expected: off, manual, auto-privacy-none, auto-privacy-public or auto-privacy-temporary.\n", argv[2]);
        return;
     }

   if (argc > 3) address = argv[3];
   if (argc > 4)
     {
        char *endptr;
        prefix = strtoul(argv[4], &endptr, 10);
        if ((endptr == argv[4]) || (*endptr != '\0'))
          {
             fprintf(stderr, "ERROR: invalid IPv6 prefix length: %s\n", argv[4]);
             return;
          }
     }
   if (argc > 5) netmask = argv[5];
   if (argc > 6) gateway = argv[6];

   printf("INFO: access point '%s' IPv6 set to %s address=%s/%hhu, netmask=%s, gateway=%s\n",
          name, argv[2], address, prefix, netmask, gateway);
   efl_net_control_access_point_configuration_ipv6_set(ap, ipv6_method, address, prefix, netmask, gateway);
}

static void
_cmd_access_point_configure_proxy(Eo *ctl, size_t argc, char **argv)
{
   Eo *ap = _access_point_find(ctl, argv[1]);
   const char *name;
   Efl_Net_Control_Access_Point_Proxy_Method proxy_method;
   const char *url = NULL;
   Eina_Iterator *servers = NULL;
   Eina_Iterator *excludes = NULL;

   if (!ap) return;

   if (argc <= 2)
     {
        fprintf(stderr, "ERROR: missing Proxy configuration method: off, dhcp or manual.\n");
        return;
     }

   name = efl_net_control_access_point_ssid_get(ap);

   if (strcmp(argv[2], "off") == 0)
     proxy_method = EFL_NET_CONTROL_ACCESS_POINT_PROXY_METHOD_OFF;
   else if (strcmp(argv[2], "auto") == 0)
     proxy_method = EFL_NET_CONTROL_ACCESS_POINT_PROXY_METHOD_AUTO;
   else if (strcmp(argv[2], "manual") == 0)
     proxy_method = EFL_NET_CONTROL_ACCESS_POINT_PROXY_METHOD_MANUAL;
   else
     {
        fprintf(stderr, "ERROR: invalid Proxy configuration method '%s', expected: off, dhcp or manual.\n", argv[2]);
        return;
     }

   if (proxy_method == EFL_NET_CONTROL_ACCESS_POINT_PROXY_METHOD_AUTO)
     {
        if (argc > 3) url = argv[3];
     }
   else if (proxy_method == EFL_NET_CONTROL_ACCESS_POINT_PROXY_METHOD_MANUAL)
     {
        size_t i, servers_start = 3, excludes_start = 0;
        for (i = 3; i < argc; i++)
          {
             if (strcmp(argv[i], "--servers") == 0)
               {
                  argv[i] = NULL; /* carray iterator operates on null terminated arrays */
                  servers_start = i + 1;
               }
             else if (strcmp(argv[i], "--excludes") == 0)
               {
                  argv[i] = NULL; /* carray iterator operates on null terminated arrays */
                  excludes_start = i + 1;
               }
          }

        servers = eina_carray_iterator_new((void **)(argv + servers_start));
        if (excludes_start)
          excludes = eina_carray_iterator_new((void **)(argv + excludes_start));
     }

   printf("INFO: access point '%s' Proxy set to %s url=%s, servers=%p, excludes=%p\n",
          name, argv[2], url, servers, excludes);
   efl_net_control_access_point_configuration_proxy_set(ap, proxy_method, url, servers, excludes);
}

static void
_cmd_quit(Eo *ctl, size_t argc EINA_UNUSED, char **argv EINA_UNUSED)
{
   printf("INFO: bye!\n");
   efl_loop_quit(efl_loop_get(ctl), EINA_VALUE_EMPTY);
}

static void
_cmd_help(Eo *ctl EINA_UNUSED, size_t argc EINA_UNUSED, char **argv EINA_UNUSED)
{
   puts("HELP:\n"
"technologies-list                    Show all technologies.\n"
"technology-show <name>               Show all detail of a given technology.\n"
"technology-scan <name>               Trigger technology scan.\n"
"technology-powered <name> [on|off]   Get or set technology powered state.\n"
"technology-tethering <name> [<on|off> [identifier] [passphrase]]\n"
"                                     Get or set technology tethering\n"
"                                     configuration.\n"
"\n"
"access-points-list                   Show all access points.\n"
"access-point-show <name>             Show all detail of a given access point.\n"
"access-point-connect <name>          Connect access point.\n"
"access-point-disconnect <name>       Disconnect access point.\n"
"access-point-forget <name>           Disconnect and forget access point\n"
"                                     configurations.\n"
"access-point-reorder <name> <prio>   Change access point priority, 0 means\n"
"                                     first, UINT32_MAX means last.\n"
"access-point-auto-connect <name> [on|off]\n"
"                                     Get or set access point auto connect.\n"
"access-point-configure-name-servers <name> <ns1> [ns2] ...\n"
"                                     Configure Name Servers to use.\n"
"access-point-configure-time-servers <name> <ts1> [ts2] ...\n"
"                                     Configure Time Servers to use.\n"
"access-point-configure-domains <name> <d1> [d2] ...\n"
"                                     Configure Domains to use.\n"
"access-point-configure-ipv4 <name> <off|dhcp|manual> [address [netmask [gateway]]]\n"
"                                     Configure IPv4.\n"
"access-point-configure-ipv6 <name> <off|manual|auto-privacy-none|auto-privacy-public|auto-privacy-temporary> [address [prefix_length [netmask [gateway]]]]\n"
"                                     Configure IPv6.\n"
"access-point-configure-proxy <name> off\n"
"access-point-configure-proxy <name> auto [url]\n"
"access-point-configure-proxy <name> manual --servers <s1> ... --excludes <e1> ...\n"
"                                     Configure Proxy.\n"
"\n"
"agent [on|off]                       Enable the agent to answer for requests.\n"
"monitor [on|off]                     Automatically print all changed objects.\n"
"help                                 Shows commands and their usage.\n"
"quit                                 Quit the application.\n"
"");
}

static char **
_cmd_split(Eina_Rw_Slice arguments, size_t *argc)
{
   char **array = calloc(arguments.len + 1, sizeof(char *));
   char *buf = arguments.mem;
   size_t len = arguments.len, src, dst, item = 0;
   char quote = 0;
   Eina_Bool is_escaped = EINA_FALSE;

   for (src = 0, dst = 0; src < len; src++)
     {
        char c = buf[src];

        if (is_escaped)
          {
             is_escaped = EINA_FALSE;
             switch (c) {
              case 'n': buf[dst++] = '\n'; break;
              case 't': buf[dst++] = '\t'; break;
              case 'r': buf[dst++] = '\r'; break;
              case 'f': buf[dst++] = '\f'; break;
              case '"': buf[dst++] = '"'; break;
              case '\'': buf[dst++] = '\''; break;
              case '\\': buf[dst++] = '\\'; break;
              default:
                 buf[dst++] = '\\';
                 buf[dst++] = c;
             }
          }
        else
          {
             if (c == '\\')
               {
                  is_escaped = EINA_TRUE;

                  if (!array[item])
                    array[item] = buf + dst;
               }
             else if ((quote == 0) && ((c == '"') || (c == '\'')))
               {
                  quote = c;
                  if (!array[item])
                    array[item] = buf + dst;
               }
             else
               {
                  if ((quote == 0) && (isspace(c)))
                    {
                       buf[dst] = '\0';
                       dst++;
                       if (array[item]) item++;
                       continue;
                    }
                  else if (quote == c)
                    {
                       quote = 0;
                       continue;
                    }
                  else if (dst < src)
                    buf[dst] = buf[src];

                  if (!array[item])
                    array[item] = buf + dst;

                  dst++;
               }
          }
     }

   if (dst < len)
     buf[dst] = '\0';
   if (array[item]) item++;

   *argc = item;

   return array;
}

static void
_cmd_line(void *data, const Efl_Event *event)
{
   Eo *ctl = data;
   Eina_Binbuf *binbuf = efl_io_copier_binbuf_steal(event->object);
   Eina_Rw_Slice rw_slice = eina_binbuf_rw_slice_get(binbuf);
   const struct {
      const char *cmd;
      void (*cb)(Eo *ctl, size_t argc, char **argv);
   } *itr, map[] = {
     {"technologies-list", _cmd_technologies_list},
     {"t-list", _cmd_technologies_list},
     {"technology-show", _cmd_technology_show},
     {"t-show", _cmd_technology_show},
     {"technology-scan", _cmd_technology_scan},
     {"t-scan", _cmd_technology_scan},
     {"technology-powered", _cmd_technology_powered},
     {"t-powered", _cmd_technology_powered},
     {"technology-tethering", _cmd_technology_tethering},
     {"t-tethering", _cmd_technology_tethering},
     {"access-points-list", _cmd_access_points_list},
     {"ap-list", _cmd_access_points_list},
     {"access-point-show", _cmd_access_point_show},
     {"ap-show", _cmd_access_point_show},
     {"access-point-connect", _cmd_access_point_connect},
     {"ap-connect", _cmd_access_point_connect},
     {"access-point-disconnect", _cmd_access_point_disconnect},
     {"ap-disconnect", _cmd_access_point_disconnect},
     {"access-point-forget", _cmd_access_point_forget},
     {"ap-forget", _cmd_access_point_forget},
     {"access-point-reorder", _cmd_access_point_reorder},
     {"ap-reorder", _cmd_access_point_reorder},
     {"access-point-auto-connect", _cmd_access_point_auto_connect},
     {"ap-auto-connect", _cmd_access_point_auto_connect},
     {"access-point-configure-name-servers", _cmd_access_point_configure_name_servers},
     {"ap-configure-name-servers", _cmd_access_point_configure_name_servers},
     {"access-point-configure-time-servers", _cmd_access_point_configure_time_servers},
     {"ap-configure-time-servers", _cmd_access_point_configure_time_servers},
     {"access-point-configure-domains", _cmd_access_point_configure_domains},
     {"ap-configure-domains", _cmd_access_point_configure_domains},
     {"access-point-configure-ipv4", _cmd_access_point_configure_ipv4},
     {"ap-configure-ipv4", _cmd_access_point_configure_ipv4},
     {"access-point-configure-ipv6", _cmd_access_point_configure_ipv6},
     {"ap-configure-ipv6", _cmd_access_point_configure_ipv6},
     {"access-point-configure-proxy", _cmd_access_point_configure_proxy},
     {"ap-configure-proxy", _cmd_access_point_configure_proxy},
     {"agent", _cmd_agent_set},
     {"monitor", _cmd_monitor_set},
     {"help", _cmd_help},
     {"quit", _cmd_quit},
     { }
   };
   char **argv;
   size_t argc;

   if (rw_slice.len == 0)
     {
        char *dummy[1] = { NULL };
        _cmd_quit(ctl, 0, dummy);
        return;
     }

   if (eina_rw_slice_endswith(rw_slice, (Eina_Slice)EINA_SLICE_STR_LITERAL("\n")))
     {
        rw_slice.len--;
        rw_slice.bytes[rw_slice.len] = '\0';
     }

   argv = _cmd_split(rw_slice, &argc);
   if (!argv[0]) goto end;
   for (itr = map; itr->cb; itr++)
     {
        if (strcmp(itr->cmd, argv[0]) == 0)
          {
             itr->cb(ctl, argc, argv);
             break;
          }
     }

   if (!itr->cb)
     fprintf(stderr, "ERROR: command not found: '%s'\n", argv[0]);

 end:
   free(argv);
   eina_binbuf_free(binbuf);
}

EFL_CALLBACKS_ARRAY_DEFINE(ctl_events_cbs,
                           { EFL_NET_CONTROL_MANAGER_EVENT_ACCESS_POINT_ADD, _ctl_access_point_add },
                           { EFL_NET_CONTROL_MANAGER_EVENT_ACCESS_POINT_DEL, _ctl_access_point_del },
                           { EFL_NET_CONTROL_MANAGER_EVENT_ACCESS_POINTS_CHANGED, _ctl_access_points_changed },
                           { EFL_NET_CONTROL_MANAGER_EVENT_TECHNOLOGY_ADD, _ctl_technology_add },
                           { EFL_NET_CONTROL_MANAGER_EVENT_TECHNOLOGY_DEL, _ctl_technology_del },
                           { EFL_NET_CONTROL_MANAGER_EVENT_RADIOS_OFFLINE_CHANGED, _ctl_radios_offline_changed },
                           { EFL_NET_CONTROL_MANAGER_EVENT_STATE_CHANGED, _ctl_state_changed },
                           { EFL_NET_CONTROL_MANAGER_EVENT_AGENT_RELEASED, _ctl_agent_released },
                           { EFL_NET_CONTROL_MANAGER_EVENT_AGENT_ERROR, _ctl_agent_error },
                           { EFL_NET_CONTROL_MANAGER_EVENT_AGENT_BROWSER_URL, _ctl_agent_browser_url },
                           { EFL_NET_CONTROL_MANAGER_EVENT_AGENT_REQUEST_INPUT, _ctl_agent_request_input });

static Eo *copier = NULL;


EAPI_MAIN void
efl_pause(void *data EINA_UNUSED,
          const Efl_Event *ev EINA_UNUSED)
{
}

EAPI_MAIN void
efl_resume(void *data EINA_UNUSED,
           const Efl_Event *ev EINA_UNUSED)
{
}

EAPI_MAIN void
efl_terminate(void *data EINA_UNUSED,
              const Efl_Event *ev EINA_UNUSED)
{
   efl_del(copier);
   copier = NULL;
}

EAPI_MAIN void
efl_main(void *data EINA_UNUSED,
         const Efl_Event *ev)
{
   Eo *ctl;
   Eo *input;
   Eina_Slice line_delimiter = EINA_SLICE_STR("\n");

   ctl = efl_add(EFL_NET_CONTROL_MANAGER_CLASS, ev->object,
                 efl_event_callback_array_add(efl_added, ctl_events_cbs(), NULL));
   if (!ctl)
     {
        fputs("ERROR: Could not create Efl.Net.Control object.\n", stderr);
        goto end;
     }

   input = efl_add(EFL_IO_STDIN_CLASS, ev->object);
   copier = efl_add(EFL_IO_COPIER_CLASS, ev->object,
                    efl_io_copier_source_set(efl_added, input),
                    efl_io_copier_line_delimiter_set(efl_added, line_delimiter),
                    efl_io_copier_buffer_limit_set(efl_added, 8192),
                    efl_io_copier_read_chunk_size_set(efl_added, 8192),
                    efl_event_callback_add(efl_added, EFL_IO_COPIER_EVENT_LINE, _cmd_line, ctl));

   printf("INFO: monitoring is on, disable with 'monitor off'. See 'help'.\n");
   printf("INFO: type commands, if unsure try: 'help'\n");

   return ;

 end:
   efl_loop_quit(efl_loop_get(ev->object), eina_value_int_init(EXIT_FAILURE));
}

EFL_MAIN_EX();
