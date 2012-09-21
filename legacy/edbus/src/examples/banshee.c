#include "EDBus.h"
#include <Ecore.h>

static Eina_Bool
_timer1_cb(void *data)
{
   printf("\n## ecore_main_loop_quit()\n");
   ecore_main_loop_quit();
   return EINA_TRUE;
}

EDBus_Connection *conn;
EDBus_Signal_Handler *sh, *sh2, *sh3, *sh4;

static void
on_get_playlists(void *data, const EDBus_Message *msg, EDBus_Pending *pending)
{
   const char *errname, *errmsg;
   EDBus_Message_Iter *array, *struct_entry;
   char *path, *name, *image;
   int i = 0;

   if (edbus_message_error_get(msg, &errname, &errmsg))
     {
        fprintf(stderr, "Error: %s %s\n", errname, errmsg);
        return;
     }

   if (!edbus_message_arguments_get(msg, "a(oss)", &array))
     {
        fprintf(stderr, "Error: could not get entry contents\n");
        return;
     }
   printf("on_get_playlists() \n\n");
   while (edbus_message_iter_get_and_next(array, 'r', &struct_entry))
     {
        if (! edbus_message_iter_arguments_get(struct_entry, "oss", &path, &name, &image))
          {
             printf("error on edbus_massage_iterator_arguments_get()");
             return;
          }
        i++;
        printf("%d - %s | %s | %s\n", i, path, name, image);
     }
   printf("end of on_get_playlists()\n\n");
}

static void
on_introspect(void *data, const EDBus_Message *msg, EDBus_Pending *pending)
{
   const char *errname, *errmsg, *string;

   if (edbus_message_error_get(msg, &errname, &errmsg))
     {
        fprintf(stderr, "Error: %s %s\n", errname, errmsg);
        return;
     }

   if (!edbus_message_arguments_get(msg, "s", &string))
     {
        fprintf(stderr, "Error: could not get entry contents\n");
        return;
     }

   printf("on_introspect() data=\n%s\n\n", string);
}

static void
on_next(void *data, const EDBus_Message *msg, EDBus_Pending *pending)
{
   const char *errname, *errmsg;

   printf("on_next()\n");

   if (edbus_message_error_get(msg, &errname, &errmsg))
     {
        fprintf(stderr, "Error: %s %s\n", errname, errmsg);
     }
}

static void
on_pause(void *data, const EDBus_Message *msg, EDBus_Pending *pending)
{
   const char *errname, *errmsg;

   printf("on_pause()\n");

   if (edbus_message_error_get(msg, &errname, &errmsg))
     {
        fprintf(stderr, "Error: %s %s\n", errname, errmsg);
     }
}

static void
on_state_changed(void *data, const EDBus_Message *msg)
{
   char *status;
   if (edbus_message_error_get(msg, NULL, NULL))
     {
        fprintf(stderr, "on_state_changed error\n");
        return;
     }

   if (!edbus_message_arguments_get(msg, "s", &status))
     {
        fprintf(stderr, "Error: could not get entry contents\n");
        return;
     }

   printf("on_state_changed = %s\n", status);
}

static void
on_state_changed2(void *data, const EDBus_Message *msg)
{
   char *status;
   if (edbus_message_error_get(msg, NULL, NULL))
     {
        fprintf(stderr, "on_state_changed error\n");
        return;
     }

   if (!edbus_message_arguments_get(msg, "s", &status))
     {
        fprintf(stderr, "Error: could not get entry contents\n");
        return;
     }

   printf("on_state_changed2 = %s\n", status);
   edbus_signal_handler_unref(sh2);
   sh2 = NULL;
}

static void
on_name_owner_changed_by_id(void *data, const EDBus_Message *msg)
{
   char *bus, *older_id, *new_id;

   if (edbus_message_error_get(msg, NULL, NULL))
     return;
   if (!edbus_message_arguments_get(msg, "sss", &bus, &older_id, &new_id))
     printf("Error getting arguments from NameOwnerChanged");

   printf("banshee started on id=%s\n", new_id);
}

static void
on_name_owner_changed(void *data, const EDBus_Message *msg)
{
   char *bus, *older_id, *new_id;
   const char *name, *text;

   if (edbus_message_error_get(msg, &name, &text))
     printf("NameOwnerChanged name=%s text=%s", name, text);
   if (!edbus_message_arguments_get(msg, "sss", &bus, &older_id, &new_id))
     printf("Error getting arguments from NameOwnerChanged");

   printf("bus = %s older=%s new=%s\n\n", bus, older_id, new_id);
}

int
main(void)
{
   EDBus_Object *player_engine_obj, *playback_controller_obj, *mediaplayer2_obj;
   EDBus_Proxy *player_engine, *playback_controler, *introspectable, *playlists;
   EDBus_Pending *pending;
   unsigned int bool2 = 1;
   unsigned int playlist_index = 0;
   unsigned int playlist_max_count = 30;
   unsigned int playlist_reverse_order = 0;
   const char *playlist_order = "asc";

   ecore_init();
   edbus_init();

   conn = edbus_connection_get(EDBUS_CONNECTION_TYPE_SESSION);

   player_engine_obj = edbus_object_get(conn, "org.bansheeproject.Banshee",
                                        "/org/bansheeproject/Banshee/PlayerEngine");
   if (!player_engine_obj)
     {
        fprintf(stderr, "Error: could not get object\n");
        return EXIT_FAILURE;
     }

   playback_controller_obj = edbus_object_get(conn, "org.bansheeproject.Banshee",
                                              "/org/bansheeproject/Banshee/PlaybackController");

   mediaplayer2_obj = edbus_object_get(conn, "org.bansheeproject.Banshee",
                                       "/org/mpris/MediaPlayer2");

   player_engine = edbus_proxy_get(player_engine_obj,
                                   "org.bansheeproject.Banshee.PlayerEngine");
   if (!player_engine)
     {
        fprintf(stderr, "Error: could not get binding\n");
        return EXIT_FAILURE;
     }

   introspectable = edbus_proxy_get(player_engine_obj,
                                    "org.freedesktop.DBus.Introspectable");
   if (!introspectable)
     {
        fprintf(stderr, "Error: could not get binding\n");
        return EXIT_FAILURE;
     }

   playback_controler = edbus_proxy_get(playback_controller_obj,
                             "org.bansheeproject.Banshee.PlaybackController");
   if (!playback_controler)
     {
        fprintf(stderr, "Error: could not get binding\n");
        return EXIT_FAILURE;
     }
   edbus_proxy_signal_handler_add(player_engine, "StateChanged", on_state_changed, NULL);

   playlists = edbus_proxy_get(mediaplayer2_obj, "org.mpris.MediaPlayer2.Playlists");

   pending = edbus_proxy_call(introspectable, "Introspect", on_introspect, NULL, -1, "");
   if (!pending)
     {
        fprintf(stderr, "Error: could not call\n");
        return EXIT_FAILURE;
     }

   pending = edbus_proxy_call(player_engine, "Pause", on_pause, NULL, -1, "");
   if (!pending)
     {
        fprintf(stderr, "Error: could not call\n");
        return EXIT_FAILURE;
     }

   pending = edbus_proxy_call(playback_controler, "Next", on_next, NULL, -1, "b", bool2);
   if (!pending)
     {
        fprintf(stderr, "Error: could not call\n");
        return EXIT_FAILURE;
     }

   edbus_proxy_call(playlists, "GetPlaylists", on_get_playlists, NULL, -1,
                    "uusb", playlist_index, playlist_max_count,
                    playlist_order, playlist_reverse_order);

   sh = edbus_signal_handler_add(conn, "org.bansheeproject.Banshee",
                            "/org/bansheeproject/Banshee/PlayerEngine",
                            "org.bansheeproject.Banshee.PlayerEngine",
                            "StateChanged", on_state_changed, NULL);

   sh2 = edbus_signal_handler_add(conn, "org.bansheeproject.Banshee",
                               "/org/bansheeproject/Banshee/PlayerEngine",
                               "org.bansheeproject.Banshee.PlayerEngine",
                               "StateChanged", on_state_changed2, NULL);

   sh3 = edbus_signal_handler_add(conn,
                                  EDBUS_FDO_BUS,
                                  EDBUS_FDO_PATH,
                                  EDBUS_FDO_INTERFACE,
                                  "NameOwnerChanged",
                                  on_name_owner_changed,
                                  NULL);
   edbus_signal_handler_match_extra_set(sh3, "arg0", "org.bansheeproject.Banshee", NULL);

   sh4 = edbus_signal_handler_add(conn,
                                  EDBUS_FDO_BUS,
                                  EDBUS_FDO_PATH,
                                  EDBUS_FDO_INTERFACE,
                                  "NameOwnerChanged",
                                  on_name_owner_changed_by_id,
                                  NULL);
   edbus_signal_handler_match_extra_set(sh4,
                                        "arg0", "org.bansheeproject.Banshee",
                                        "arg1", "", NULL);

   ecore_timer_add(50, _timer1_cb, NULL);

   ecore_main_loop_begin();

   edbus_signal_handler_unref(sh);
   edbus_proxy_unref(playback_controler);
   edbus_proxy_unref(introspectable);
   edbus_object_unref(player_engine_obj);
   edbus_connection_unref(conn);

   edbus_shutdown();
   ecore_shutdown();
   return 0;
}

