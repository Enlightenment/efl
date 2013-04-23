//Compile with:
// gcc -o banshee banshee.c `pkg-config --cflags --libs eldbus ecore eina`

#include "Eldbus.h"
#include <Ecore.h>

#define BUS "org.bansheeproject.Banshee"
#define ENGINE_PATH "/org/bansheeproject/Banshee/PlayerEngine"
#define CONTROLLER_PATH "/org/bansheeproject/Banshee/PlaybackController"
#define MPRIS_PATH "/org/mpris/MediaPlayer2"

#define ENGINE_IFACE "org.bansheeproject.Banshee.PlayerEngine"
#define CONTROLLER_IFACE "org.bansheeproject.Banshee.PlaybackController"
#define MPRIS_IFACE "org.mpris.MediaPlayer2.Playlists"

static Eldbus_Signal_Handler *state_changed2;

static Eina_Bool
_timeout_application(void *data EINA_UNUSED)
{
   printf("\n## ecore_main_loop_quit()\n");
   ecore_main_loop_quit();
   return EINA_TRUE;
}

static void
on_get_playlists(void *data EINA_UNUSED, const Eldbus_Message *msg, Eldbus_Pending *pending EINA_UNUSED)
{
   Eldbus_Message_Iter *array, *struct_entry;
   const char *path, *name, *image;
   int i = 0;

   EINA_SAFETY_ON_TRUE_RETURN(eldbus_message_error_get(msg, NULL, NULL));

   if (!eldbus_message_arguments_get(msg, "a(oss)", &array))
     {
        fprintf(stderr, "Error: could not get entry contents\n");
        return;
     }
   printf("on_get_playlists() \n\n");
   while (eldbus_message_iter_get_and_next(array, 'r', &struct_entry))
     {
        if (!eldbus_message_iter_arguments_get(struct_entry, "oss", &path, &name, &image))
          {
             printf("error on eldbus_massage_iterator_arguments_get()");
             return;
          }
        i++;
        printf("%d - %s | %s | %s\n", i, path, name, image);
     }
   printf("end of on_get_playlists()\n\n");
}

static void
iterate_dict(void *data EINA_UNUSED, const void *key, Eldbus_Message_Iter *var)
{
   const char *skey = key;

   if (!strcmp(skey, "PlaylistCount"))
     {
        unsigned count;
        if (!eldbus_message_iter_arguments_get(var, "u", &count))
          printf("error2\n");
        printf("PlaylistCount=%u\n", count);
     }
   else if (!strcmp(skey, "Orderings"))
     {
        Eldbus_Message_Iter *as;
        const char *txt;
        printf("- Orderings\n");
        if (!eldbus_message_iter_arguments_get(var, "as", &as))
          printf("error1\n");
        while (eldbus_message_iter_get_and_next(as, 's', &txt))
          printf("\t%s\n", txt);
     }
}

static void
playlist_get_all_cb(void *data EINA_UNUSED, const Eldbus_Message *msg, Eldbus_Pending *pending EINA_UNUSED)
{
   Eldbus_Message_Iter *array;
   EINA_SAFETY_ON_TRUE_RETURN(eldbus_message_error_get(msg, NULL, NULL));

   if (eldbus_message_arguments_get(msg, "a{sv}", &array))
     eldbus_message_iter_dict_iterate(array, "sv", iterate_dict, NULL);
}

static void
on_introspect(void *data EINA_UNUSED, const Eldbus_Message *msg, Eldbus_Pending *pending EINA_UNUSED)
{
   const char *string;

   EINA_SAFETY_ON_TRUE_RETURN(eldbus_message_error_get(msg, NULL, NULL));

   if (!eldbus_message_arguments_get(msg, "s", &string))
     {
        fprintf(stderr, "Error: could not get entry contents\n");
        return;
     }

   printf("on_introspect() data=\n%s\n\n", string);
}

static void
on_next_or_pause(void *data, const Eldbus_Message *msg, Eldbus_Pending *pending EINA_UNUSED)
{
   const char *status = data;

   EINA_SAFETY_ON_TRUE_RETURN(eldbus_message_error_get(msg, NULL, NULL));

   printf("%s\n", status);
}

static void
on_state_changed(void *data EINA_UNUSED, const Eldbus_Message *msg)
{
   const char *status;
   EINA_SAFETY_ON_TRUE_RETURN(eldbus_message_error_get(msg, NULL, NULL));

   if (!eldbus_message_arguments_get(msg, "s", &status))
     {
        fprintf(stderr, "Error: could not get entry contents\n");
        return;
     }

   printf("on_state_changed = %s\n", status);
}

static void
on_state_changed2(void *data EINA_UNUSED, const Eldbus_Message *msg)
{
   const char *status;
   EINA_SAFETY_ON_TRUE_RETURN(eldbus_message_error_get(msg, NULL, NULL));

   if (!eldbus_message_arguments_get(msg, "s", &status))
     {
        fprintf(stderr, "Error: could not get entry contents\n");
        return;
     }

   printf("on_state_changed2 = %s\n", status);
   eldbus_signal_handler_unref(state_changed2);
   state_changed2 = NULL;
}

static void
on_banshee_startup(void *data EINA_UNUSED, const Eldbus_Message *msg)
{
   const char *bus, *older_id, *new_id;

   EINA_SAFETY_ON_TRUE_RETURN(eldbus_message_error_get(msg, NULL, NULL));
   if (!eldbus_message_arguments_get(msg, "sss", &bus, &older_id, &new_id))
     {
        printf("Error getting arguments from NameOwnerChanged");
        return;
     }

   printf("banshee started on id=%s\n", new_id);
}

static void
on_name_owner_changed(void *data EINA_UNUSED, const Eldbus_Message *msg)
{
   const char *bus, *older_id, *new_id;

   EINA_SAFETY_ON_TRUE_RETURN(eldbus_message_error_get(msg, NULL, NULL));
   if (!eldbus_message_arguments_get(msg, "sss", &bus, &older_id, &new_id))
     {
        printf("Error getting arguments from NameOwnerChanged");
        return;
     }

   printf("bus = %s older=%s new=%s\n\n", bus, older_id, new_id);
}

int
main(void)
{
   Eldbus_Connection *conn;
   Eldbus_Object *engine_obj, *controller_obj, *mpris_obj;
   Eldbus_Proxy *engine, *controler, *playlists;
   Eldbus_Signal_Handler *sh;

   ecore_init();
   eldbus_init();

   conn = eldbus_connection_get(ELDBUS_CONNECTION_TYPE_SESSION);

   engine_obj = eldbus_object_get(conn, BUS, ENGINE_PATH);
   controller_obj = eldbus_object_get(conn, BUS, CONTROLLER_PATH);
   mpris_obj = eldbus_object_get(conn, BUS, MPRIS_PATH);

   engine = eldbus_proxy_get(engine_obj, ENGINE_IFACE);
   EINA_SAFETY_ON_NULL_GOTO(engine, end);
   controler = eldbus_proxy_get(controller_obj, CONTROLLER_IFACE);
   EINA_SAFETY_ON_NULL_GOTO(controler, end);
   playlists = eldbus_proxy_get(mpris_obj, MPRIS_IFACE);
   EINA_SAFETY_ON_NULL_GOTO(playlists, end);

   eldbus_object_introspect(engine_obj, on_introspect, NULL);

   eldbus_proxy_signal_handler_add(engine, "StateChanged", on_state_changed, NULL);
   eldbus_proxy_call(engine, "Pause", on_next_or_pause, "Pause", -1, "");

   eldbus_proxy_call(controler, "Next", on_next_or_pause, "Next", -1, "b", EINA_TRUE);

   eldbus_proxy_property_get_all(playlists, playlist_get_all_cb, NULL);
   eldbus_proxy_call(playlists, "GetPlaylists", on_get_playlists, NULL, -1,
                    "uusb", (unsigned)0, (unsigned)30, "asc", EINA_FALSE);

   eldbus_signal_handler_add(conn, BUS, ENGINE_PATH, ENGINE_IFACE,
                            "StateChanged", on_state_changed, NULL);
   state_changed2 = eldbus_signal_handler_add(conn, BUS, ENGINE_PATH, ENGINE_IFACE,
                                             "StateChanged", on_state_changed2, NULL);

   sh = eldbus_signal_handler_add(conn, ELDBUS_FDO_BUS, ELDBUS_FDO_PATH,
                                 ELDBUS_FDO_INTERFACE, "NameOwnerChanged",
                                 on_name_owner_changed, NULL);
   eldbus_signal_handler_match_extra_set(sh, "arg0", BUS, NULL);

   sh = eldbus_signal_handler_add(conn, ELDBUS_FDO_BUS, ELDBUS_FDO_PATH,
                                 ELDBUS_FDO_INTERFACE, "NameOwnerChanged",
                                 on_banshee_startup, NULL);
   eldbus_signal_handler_match_extra_set(sh, "arg0", BUS, "arg1", "", NULL);

   ecore_timer_add(50, _timeout_application, NULL);

   ecore_main_loop_begin();

end:
   /**
    *  It's not necessary unref all objecs, proxys and signal handlers
    *  When a parent have ref = 0, it will unref all your childrens
    *  before free it self.
    **/
   eldbus_connection_unref(conn);

   eldbus_shutdown();
   ecore_shutdown();
   return 0;
}
