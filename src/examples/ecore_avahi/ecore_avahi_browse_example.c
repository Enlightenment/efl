#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>

#ifdef HAVE_AVAHI

#include <avahi-client/client.h>
#include <avahi-client/lookup.h>
#include <avahi-client/publish.h>

#include <avahi-common/simple-watch.h>
#include <avahi-common/malloc.h>
#include <avahi-common/error.h>
#include <avahi-common/alternative.h>
#include <avahi-common/timeval.h>

#include <Ecore.h>
#include <Ecore_Avahi.h>

#define FREE_CLEAN(h, cb) do { if (h) cb(h); h = NULL; } while (0);

typedef struct _Ecore_Avahi_Example Ecore_Avahi_Example;
struct _Ecore_Avahi_Example
{
   AvahiServiceBrowser *sb;
   AvahiClient *client;
   const char *name;
};

static void
resolve_callback(AvahiServiceResolver *r, AvahiIfIndex interface EINA_UNUSED,
                 AvahiProtocol protocol EINA_UNUSED, AvahiResolverEvent event,
                 const char *name, const char *type, const char *domain,
                 const char *host_name, const AvahiAddress *address, uint16_t port,
                 AvahiStringList *txt, AvahiLookupResultFlags flags,
                 void* userdata EINA_UNUSED)
{
   assert(r);

   /* Called whenever a service has been resolved successfully or timed out */

   switch (event) {
    case AVAHI_RESOLVER_FAILURE:
       fprintf(stderr, "(Resolver) Failed to resolve service '%s' of type '%s' in domain '%s': %s\n", name, type, domain, avahi_strerror(avahi_client_errno(avahi_service_resolver_get_client(r))));
       break;

    case AVAHI_RESOLVER_FOUND: {
       char a[AVAHI_ADDRESS_STR_MAX], *t;

       fprintf(stderr, "Service '%s' of type '%s' in domain '%s':\n", name, type, domain);

       avahi_address_snprint(a, sizeof(a), address);
       t = avahi_string_list_to_string(txt);
       fprintf(stderr,
               "\t%s:%u (%s)\n"
               "\tTXT=%s\n"
               "\tcookie is %u\n"
               "\tis_local: %i\n"
               "\tour_own: %i\n"
               "\twide_area: %i\n"
               "\tmulticast: %i\n"
               "\tcached: %i\n",
               host_name, port, a,
               t,
               avahi_string_list_get_service_cookie(txt),
               !!(flags & AVAHI_LOOKUP_RESULT_LOCAL),
               !!(flags & AVAHI_LOOKUP_RESULT_OUR_OWN),
               !!(flags & AVAHI_LOOKUP_RESULT_WIDE_AREA),
               !!(flags & AVAHI_LOOKUP_RESULT_MULTICAST),
               !!(flags & AVAHI_LOOKUP_RESULT_CACHED));

       avahi_free(t);
    }
   }

   avahi_service_resolver_free(r);
}

static void
_browse_callback(AvahiServiceBrowser *b, AvahiIfIndex interface,
                 AvahiProtocol protocol, AvahiBrowserEvent event,
                 const char *name, const char *type, const char *domain,
                 AvahiLookupResultFlags flags EINA_UNUSED,
                 void* userdata)
{
   AvahiClient *c = userdata;
   assert(b);

   /* Called whenever a new services becomes available on the LAN or is removed from the LAN */
   switch (event) {
    case AVAHI_BROWSER_FAILURE:

       fprintf(stderr, "(Browser) %s\n", avahi_strerror(avahi_client_errno(avahi_service_browser_get_client(b))));
       ecore_main_loop_quit();
       return;

    case AVAHI_BROWSER_NEW:
       fprintf(stderr, "(Browser) NEW: service '%s' of type '%s' in domain '%s'\n", name, type, domain);

       /* We ignore the returned resolver object. In the callback
          function we free it. If the server is terminated before
          the callback function is called the server will free
          the resolver for us. */

       if (!(avahi_service_resolver_new(c, interface, protocol, name, type, domain, AVAHI_PROTO_UNSPEC, 0, resolve_callback, c)))
         fprintf(stderr, "Failed to resolve service '%s': %s\n", name, avahi_strerror(avahi_client_errno(c)));

       break;

    case AVAHI_BROWSER_REMOVE:
       fprintf(stderr, "(Browser) REMOVE: service '%s' of type '%s' in domain '%s'\n", name, type, domain);
       break;

    case AVAHI_BROWSER_ALL_FOR_NOW:
    case AVAHI_BROWSER_CACHE_EXHAUSTED:
       fprintf(stderr, "(Browser) %s\n", event == AVAHI_BROWSER_CACHE_EXHAUSTED ? "CACHE_EXHAUSTED" : "ALL_FOR_NOW");
       break;
   }
}

static void
_ecore_avahi_client_cb(AvahiClient *c, AvahiClientState state, void * userdata)
{
  Ecore_Avahi_Example *example = userdata;

  switch (state)
    {
    case AVAHI_CLIENT_S_REGISTERING:
    case AVAHI_CLIENT_S_COLLISION:
    case AVAHI_CLIENT_S_RUNNING:
      if (!example->sb)
        {
           example->sb = avahi_service_browser_new(c, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, "_ipp._tcp", NULL, 0, _browse_callback, c);
           if (!example->sb)
             {
                fprintf(stderr, "Failed to setup an Avahi Service Browser.");
                ecore_main_loop_quit();
                return ;
             }
        }
      break;

    case AVAHI_CLIENT_FAILURE:
       if (avahi_client_errno(c) == AVAHI_ERR_DISCONNECTED)
         {
            FREE_CLEAN(example->sb, avahi_service_browser_free);
            FREE_CLEAN(example->client, avahi_client_free);

            ecore_main_loop_quit();

            break;
         }
    case AVAHI_CLIENT_CONNECTING:
       FREE_CLEAN(example->sb, avahi_service_browser_free);
       break;
    }
}

int
main(int argc, char **argv)
{
   Ecore_Avahi_Example example = { 0 };
   Ecore_Avahi *handler;
   const AvahiPoll *poll_api;
   int exit_code = 0;
   int error = 0;

   if (argc < 1)
     {
        fprintf(stderr, "Usage : %s\n", argv[0]);
        return -1;
     }

   eina_init();
   ecore_init();

   handler = ecore_avahi_add();
   poll_api = ecore_avahi_poll_get(handler);

   if (!poll_api)
     {
        fprintf(stderr, "Build EFL with Avahi support.\n");
        exit_code = -1;
        goto fail;
     }

   example.name = NULL;
   example.client = avahi_client_new(poll_api, AVAHI_CLIENT_NO_FAIL, _ecore_avahi_client_cb, &example, &error);

   if (!example.client)
     {
        fprintf(stderr, "Failed to create avahi client: %s.\n", avahi_strerror(error));
        exit_code = -1;
        goto fail;
     }

   ecore_main_loop_begin();

   avahi_client_free(example.client);

 fail:
   ecore_shutdown();
   eina_shutdown();

   return exit_code;
}
#else
int
main(int argc, char **argv)
{
   fprintf(stderr, "This example require Avahi to be build !\n");
}
#endif
