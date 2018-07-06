#include <Efl_Net.h>
#include <Ecore_Getopt.h>

static const char *
_state_str(Efl_Net_Session_State state)
{
   static const char *strs[] = {
     [EFL_NET_SESSION_STATE_OFFLINE] = "offline",
     [EFL_NET_SESSION_STATE_LOCAL] = "local",
     [EFL_NET_SESSION_STATE_ONLINE] = "online",
   };

   if ((unsigned)state > sizeof(strs)/sizeof(strs[0])) return "???";
   if (!strs[state]) return "???";
   return strs[state];
}

static const char *
_technology_str(Efl_Net_Session_Technology tech)
{
   static const char *strs[] = {
     [EFL_NET_SESSION_TECHNOLOGY_UNKNOWN] = "none",
     [EFL_NET_SESSION_TECHNOLOGY_ETHERNET] = "ethernet",
     [EFL_NET_SESSION_TECHNOLOGY_WIFI] = "wifi",
     [EFL_NET_SESSION_TECHNOLOGY_BLUETOOTH] = "bluetooth",
     [EFL_NET_SESSION_TECHNOLOGY_CELLULAR] = "cellular",
     [EFL_NET_SESSION_TECHNOLOGY_VPN] = "vpn",
     [EFL_NET_SESSION_TECHNOLOGY_GADGET] = "gadget",
   };

   if ((unsigned)tech > sizeof(strs)/sizeof(strs[0])) return "???";
   if (!strs[tech]) return "???";
   return strs[tech];
}

static void
_changed(void *data EINA_UNUSED, const Efl_Event *event)
{
   Eo *session = event->object;
   const char *ip, *netmask, *gateway;
   uint8_t prefix;

   printf("INFO: session changed:\n"
          "INFO:  - name: '%s'\n"
          "INFO:  - state: %s\n"
          "INFO:  - technology: %s\n"
          "INFO:  - interface: '%s'\n",
          efl_net_session_network_name_get(session),
          _state_str(efl_net_session_state_get(session)),
          _technology_str(efl_net_session_technology_get(session)),
          efl_net_session_interface_get(session));

   efl_net_session_ipv4_get(session, &ip, &netmask, &gateway);
   if (ip)
     {
        printf("INFO:  - IPv4: %s, gateway=%s, netmask=%s\n",
               ip, gateway, netmask);
     }

   efl_net_session_ipv6_get(session, &ip, &prefix, &netmask, &gateway);
   if (ip)
     {
        printf("INFO:  - IPv6: %s/%hhu, gateway=%s, netmask=%s\n",
               ip, prefix, gateway, netmask);
     }
}

EFL_CALLBACKS_ARRAY_DEFINE(session_events_cbs,
                           { EFL_NET_SESSION_EVENT_CHANGED, _changed });

static const Ecore_Getopt options = {
  "efl_net_session_example", /* program name */
  NULL, /* usage line */
  "1", /* version */
  "(C) 2016 Enlightenment Project", /* copyright */
  "BSD 2-Clause", /* license */
  /* long description, may be multiline and contain \n */
  "Example of Efl_Net_Session to request or monitor a network session for an application.\n",
  EINA_FALSE,
  {
    ECORE_GETOPT_STORE_TRUE('c', "connect", "Require a connection to the internet (See -o/--require-online)."),
    ECORE_GETOPT_STORE_TRUE('o', "require-online", "When connecting (-c/--connect), require connection to the internet. Otherwise a local network connection is enough"),
    ECORE_GETOPT_APPEND('t', "technology", "Bearer technologies to use when connecting (ethernet, wifi, bluetooth, cellular, vpn, gadget or all)", ECORE_GETOPT_TYPE_STR),

    ECORE_GETOPT_VERSION('V', "version"),
    ECORE_GETOPT_COPYRIGHT('C', "copyright"),
    ECORE_GETOPT_LICENSE('L', "license"),
    ECORE_GETOPT_HELP('h', "help"),
    ECORE_GETOPT_SENTINEL
  }
};

static Eo *session = NULL;

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
   /* FIXME: For the moment the main loop doesn't get
      properly destroyed on shutdown which disallow
      relying on parent destroying their children */
   if (session)
     {
        efl_del(session);
        session = NULL;
     }

   fprintf(stderr, "INFO: main loop finished.\n");
}

EAPI_MAIN void
efl_main(void *data EINA_UNUSED,
         const Efl_Event *ev)
{
   char *str;
   Eina_List *techs = NULL;
   Eina_Bool do_connect = EINA_FALSE;
   Eina_Bool require_online = EINA_FALSE;
   Eina_Bool quit_option = EINA_FALSE;
   Efl_Net_Session_Technology technologies = EFL_NET_SESSION_TECHNOLOGY_ALL;
   Ecore_Getopt_Value values[] = {
     ECORE_GETOPT_VALUE_BOOL(do_connect),
     ECORE_GETOPT_VALUE_BOOL(require_online),
     ECORE_GETOPT_VALUE_LIST(techs),

     /* standard block to provide version, copyright, license and help */
     ECORE_GETOPT_VALUE_BOOL(quit_option), /* -V/--version quits */
     ECORE_GETOPT_VALUE_BOOL(quit_option), /* -C/--copyright quits */
     ECORE_GETOPT_VALUE_BOOL(quit_option), /* -L/--license quits */
     ECORE_GETOPT_VALUE_BOOL(quit_option), /* -h/--help quits */
     ECORE_GETOPT_VALUE_NONE /* sentinel */
   };
   int args;

   args = ecore_getopt_parse(&options, values, 0, NULL);
   if (args < 0)
     {
        fputs("ERROR: Could not parse command line options.\n", stderr);
        goto end;
     }

   if (quit_option) goto end;

   if (techs)
     {
        technologies = 0;
        EINA_LIST_FREE(techs, str)
          {
             if (0) {}
#define MAP(X) else if (strcasecmp(str, #X) == 0) technologies |= EFL_NET_SESSION_TECHNOLOGY_ ## X
             MAP(ETHERNET);
             MAP(WIFI);
             MAP(BLUETOOTH);
             MAP(CELLULAR);
             MAP(VPN);
             MAP(GADGET);
#undef MAP
             else fprintf(stderr, "WARNING: unknown technology '%s' ignored.\n", str);
             free(str);
          }
     }

   session = efl_add(EFL_NET_SESSION_CLASS, ev->object,
                     efl_event_callback_array_add(efl_added, session_events_cbs(), NULL));
   if (!session)
     {
        fputs("ERROR: Could not create Efl.Net.Session object.\n", stderr);
        goto end;
     }

   if (do_connect)
     {
        printf("INFO: requesting a %s connection.\n", require_online ? "online" : "local");
        efl_net_session_connect(session, require_online, technologies);
     }

   printf("INFO: the session will active while this application runs. Use ^C (Control + C) to close it\n");

   return ;

 end:
   efl_loop_quit(ev->object, eina_value_int_init(EXIT_FAILURE));
}

EFL_MAIN_EX();
