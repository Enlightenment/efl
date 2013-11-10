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

typedef struct _Ecore_Avahi_Example Ecore_Avahi_Example;
struct _Ecore_Avahi_Example
{
   AvahiClient *client;
   AvahiEntryGroup *group;
   const char *server;
   int port;
};

static void
_ecore_avahi_group_cb(AvahiEntryGroup *g, AvahiEntryGroupState state, void *userdata)
{
   Ecore_Avahi_Example *example = userdata;

   switch (state)
     {
      case AVAHI_ENTRY_GROUP_ESTABLISHED :
         /* The entry group has been established successfully */
         fprintf(stderr, "Service '%s' successfully established.\n", example->server);
         break;

      case AVAHI_ENTRY_GROUP_COLLISION :
	 fprintf(stderr, "Service name collision.\n");
	 ecore_main_loop_quit();
	 break;

      case AVAHI_ENTRY_GROUP_FAILURE :

         fprintf(stderr, "Entry group failure: %s\n", avahi_strerror(avahi_client_errno(avahi_entry_group_get_client(g))));
         /* Some kind of failure happened while we were registering our services */
	 ecore_main_loop_quit();
         break;

      case AVAHI_ENTRY_GROUP_UNCOMMITED:
      case AVAHI_ENTRY_GROUP_REGISTERING:
         ;
     }
}

static void
_ecore_avahi_service_create(AvahiClient *c, Ecore_Avahi_Example *example)
{
   int error;

   example->group = avahi_entry_group_new(c, _ecore_avahi_group_cb, example);
   if (!example->group)
     {
	fprintf(stderr, "avahi_entry_group_new() failed: %s\n", avahi_strerror(avahi_client_errno(c)));
	goto fail;
     }

   /* If the group is empty (either because it was just created, or
    * because it was reset previously, add our entries.  */
   if (!avahi_entry_group_is_empty(example->group)) return ;

   error = avahi_entry_group_add_service(example->group, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, 0,
					 example->server, "_ipp._tcp", NULL, NULL, example->port, "name=ThisIsATest", NULL);
   if (error < 0)
     {
	fprintf(stderr, "Failed to add _ipp._tcp service with error: %s.\n", avahi_strerror(error));
	goto fail;
     }

   error = avahi_entry_group_commit(example->group);
   if (error < 0)
     {
	fprintf(stderr, "Failed to commit entry group with error: %s\n", avahi_strerror(error));
	goto fail;
     }

   return ;

 fail:
   ecore_main_loop_quit();
}

static void
_ecore_avahi_client_cb(AvahiClient *c, AvahiClientState state, void * userdata)
{
   Ecore_Avahi_Example *example = userdata;

   switch (state)
     {
      case AVAHI_CLIENT_S_RUNNING:
         /* The server has started successfully and registered its host
          * name on the network, so it's time to create our services */
         _ecore_avahi_service_create(c, example);
         break;
      case AVAHI_CLIENT_FAILURE:
         fprintf(stderr, "Avahi client failure: %s\n", avahi_strerror(avahi_client_errno(c)));
         break;
      case AVAHI_CLIENT_S_COLLISION:
         /* Let's drop our registered services. When the server is back
          * in AVAHI_SERVER_RUNNING state we will register them
          * again with the new host name. */
      case AVAHI_CLIENT_S_REGISTERING:
         /* The server records are now being established. This
          * might be caused by a host name change. We need to wait
          * for our own records to register until the host name is
          * properly esatblished. */
         if (example->group) avahi_entry_group_reset(example->group);
         break;
      case AVAHI_CLIENT_CONNECTING:
         ;
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

   if (argc < 3)
     {
	fprintf(stderr, "Usage : %s name port\n", argv[0]);
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

   example.server = eina_stringshare_add(argv[1]);
   example.port = atoi(argv[2]);
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
   eina_stringshare_del(example.server);

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
