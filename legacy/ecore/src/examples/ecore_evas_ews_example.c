/**
 * Ecore example illustrating the ews of ecore evas usage.
 *
 * You'll need at least one Evas engine built for it (excluding the
 * buffer one). See stdout/stderr for output.
 *
 * @verbatim
 * gcc -o ecore_evas_ews_example ecore_evas_ews_example.c `pkg-config --libs --cflags ecore-evas`
 * @endverbatim
 */

#include <Ecore.h>
#include <Ecore_Evas.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>

static Eina_Bool
_wm_win_add(void *data, int type, void *event_info)
{
   Ecore_Evas *ee = event_info;
   printf("WM: new window=%p\n", ee);
   return EINA_TRUE;
}

static Eina_Bool
_wm_win_move(void *data, int type, void *event_info)
{
   Ecore_Evas *ee = event_info;
   int x, y;
   ecore_evas_geometry_get(ee, &x, &y, NULL, NULL);
   printf("WM: window=%p moved to %d,%d\n", ee, x, y);
   return EINA_TRUE;
}

static Eina_Bool
_wm_win_resize(void *data, int type, void *event_info)
{
   Ecore_Evas *ee = event_info;
   int w, h;
   ecore_evas_geometry_get(ee, NULL, NULL, &w, &h);
   printf("WM: window=%p resized to %dx%d\n", ee, w, h);
   return EINA_TRUE;
}

static Eina_Bool
_wm_win_show(void *data, int type, void *event_info)
{
   Ecore_Evas *ee = event_info;
   printf("WM: show window=%p\n", ee);
   return EINA_TRUE;
}

static void
optional_ews_window_manager_setup(void)
{
   ecore_event_handler_add(ECORE_EVAS_EWS_EVENT_ADD, _wm_win_add, NULL);
   ecore_event_handler_add(ECORE_EVAS_EWS_EVENT_MOVE, _wm_win_move, NULL);
   ecore_event_handler_add(ECORE_EVAS_EWS_EVENT_RESIZE, _wm_win_resize, NULL);
   ecore_event_handler_add(ECORE_EVAS_EWS_EVENT_SHOW, _wm_win_show, NULL);

   /* one may use any known unique identifier, like an app function pointer */
   ecore_evas_ews_manager_set(optional_ews_window_manager_setup);
}

static void
optional_ews_setup(void)
{
   Evas_Object *bg;
   Evas *e;

   ecore_evas_ews_setup(0, 0, 800, 600); /* "screen" size */
   e = ecore_evas_ews_evas_get(); /* forces "screen" to be allocated */

   bg = evas_object_rectangle_add(e);
   evas_object_color_set(bg, 128, 32, 32, 255);
   ecore_evas_ews_background_set(bg);
}

static Eina_Bool
_stdin_cb(void *data, Ecore_Fd_Handler *handler)
{
   const Eina_List *l;
   Ecore_Evas *ee;
   char c = getchar();

   if (c == EOF)
     {
        ecore_main_loop_quit();
        return EINA_FALSE;
     }

   switch (c) {
    case 'h':
       printf("hide all windows\n");
       EINA_LIST_FOREACH(ecore_evas_ews_children_get(), l, ee)
         ecore_evas_hide(ee);
       break;
    case 's':
       printf("show all windows\n");
       EINA_LIST_FOREACH(ecore_evas_ews_children_get(), l, ee)
         ecore_evas_show(ee);
       break;
    case 'l':
       printf("move all windows left\n");
       EINA_LIST_FOREACH(ecore_evas_ews_children_get(), l, ee)
         {
            int x, y;
            ecore_evas_geometry_get(ee, &x, &y, NULL, NULL);
            ecore_evas_move(ee, x - 10, y);
         }
       break;
    case 'r':
       printf("move all windows right\n");
       EINA_LIST_FOREACH(ecore_evas_ews_children_get(), l, ee)
         {
            int x, y;
            ecore_evas_geometry_get(ee, &x, &y, NULL, NULL);
            ecore_evas_move(ee, x + 10, y);
         }
       break;
    case 't':
       printf("move all windows top\n");
       EINA_LIST_FOREACH(ecore_evas_ews_children_get(), l, ee)
         {
            int x, y;
            ecore_evas_geometry_get(ee, &x, &y, NULL, NULL);
            ecore_evas_move(ee, x, y - 10);
         }
       break;
    case 'b':
       printf("move all windows bottom\n");
       EINA_LIST_FOREACH(ecore_evas_ews_children_get(), l, ee)
         {
            int x, y;
            ecore_evas_geometry_get(ee, &x, &y, NULL, NULL);
            ecore_evas_move(ee, x, y + 10);
         }
       break;
    case 'S':
       printf("make all windows smaller\n");
       EINA_LIST_FOREACH(ecore_evas_ews_children_get(), l, ee)
         {
            int w, h;
            ecore_evas_geometry_get(ee, NULL, NULL, &w, &h);
            ecore_evas_resize(ee, w - 10, h - 10);
         }
       break;
    case 'B':
       printf("make all windows bigger\n");
       EINA_LIST_FOREACH(ecore_evas_ews_children_get(), l, ee)
         {
            int w, h;
            ecore_evas_geometry_get(ee, NULL, NULL, &w, &h);
            ecore_evas_resize(ee, w + 10, h + 10);
         }
       break;
    case 'm':
       printf("make all windows unmaximized\n");
       EINA_LIST_FOREACH(ecore_evas_ews_children_get(), l, ee)
            ecore_evas_maximized_set(ee, EINA_FALSE);
       break;
    case 'M':
       printf("make all windows maximized\n");
       EINA_LIST_FOREACH(ecore_evas_ews_children_get(), l, ee)
            ecore_evas_maximized_set(ee, EINA_TRUE);
       break;
    case 'i':
       printf("make all windows uniconified\n");
       EINA_LIST_FOREACH(ecore_evas_ews_children_get(), l, ee)
            ecore_evas_iconified_set(ee, EINA_FALSE);
       break;
    case 'I':
       printf("make all windows iconified\n");
       EINA_LIST_FOREACH(ecore_evas_ews_children_get(), l, ee)
            ecore_evas_iconified_set(ee, EINA_TRUE);
       break;
    case 'f':
       printf("make all windows unfullscreen\n");
       EINA_LIST_FOREACH(ecore_evas_ews_children_get(), l, ee)
            ecore_evas_fullscreen_set(ee, EINA_FALSE);
       break;
    case 'F':
       printf("make all windows fullscreen\n");
       EINA_LIST_FOREACH(ecore_evas_ews_children_get(), l, ee)
            ecore_evas_fullscreen_set(ee, EINA_TRUE);
       break;
    case 'q':
       printf("quit\n");
       ecore_main_loop_quit();
       break;
    default:
       if (!isspace(c))
         printf("Unknown command: %c\n", c);
   }
   return ECORE_CALLBACK_RENEW;
}

static void
_on_delete(Ecore_Evas *ee)
{
   free(ecore_evas_data_get(ee, "key"));
   ecore_main_loop_quit();
}

int
main(void)
{
   Ecore_Evas *ee;
   Evas *canvas;
   Evas_Object *bg;

   if (ecore_evas_init() <= 0)
      return 1;

   optional_ews_setup();
   optional_ews_window_manager_setup();

   /* everything should look similar to ecore_evas_basic_example */
   ee = ecore_evas_ews_new(0, 0, 200, 200);
   ecore_evas_title_set(ee, "Ecore Evas EWS Example");
   ecore_evas_show(ee);

   ecore_evas_data_set(ee, "key", strdup("hello"));
   ecore_evas_callback_delete_request_set(ee, _on_delete);

   printf("Using %s engine!\n", ecore_evas_engine_name_get(ee));

   canvas = ecore_evas_get(ee);
   if(ecore_evas_ecore_evas_get(canvas) == ee)
      printf("Everything is sane!\n");

   bg = evas_object_rectangle_add(canvas);
   evas_object_color_set(bg, 0, 0, 255, 255);
   evas_object_resize(bg, 200, 200);
   evas_object_show(bg);
   ecore_evas_object_associate(ee, bg, ECORE_EVAS_OBJECT_ASSOCIATE_BASE);

   /* moving the window should move it in the screen */
   ecore_evas_move(ee, 50, 50);

   ecore_main_fd_handler_add(STDIN_FILENO,
              ECORE_FD_READ | ECORE_FD_ERROR,
              _stdin_cb,
              NULL, NULL, NULL);

   ecore_main_loop_begin();

   ecore_evas_free(ee);
   ecore_evas_shutdown();

   return 0;
}
