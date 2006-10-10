#include "evas_test_main.h"

#include <unistd.h>

#define X_H   /* make sure we aren't using symbols from X.h */

#include <xcb/xcb.h>
#include <xcb/shm.h>
#include <xcb/render.h>

#include "Evas.h"
#include "Evas_Engine_XRender_Xcb.h"


static void
title_set (xcb_connection_t *c, xcb_window_t win, const char *title)
{
  xcb_intern_atom_cookie_t cookie_encoding;
  xcb_intern_atom_cookie_t cookie_property;
  xcb_intern_atom_reply_t *rep;
  xcb_atom_t               encoding;
  char                    *atom_name;

  atom_name = "UTF8_STRING";
  cookie_encoding = xcb_intern_atom (c,
                                   0,
                                   strlen (atom_name),
                                   atom_name);
  atom_name = "_NET_WM_NAME";
  cookie_property = xcb_intern_atom (c,
                                   0,
                                   strlen (atom_name),
                                   atom_name);

  rep = xcb_intern_atom_reply (c, cookie_encoding, NULL);
  encoding = rep->atom;
  free (rep);

  rep = xcb_intern_atom_reply (c, cookie_property, NULL);

  xcb_change_property(c, XCB_PROP_MODE_REPLACE,
                    win,
                    rep->atom, encoding, 8, strlen (title), title);
  free (rep);
}

static void
class_set (xcb_connection_t *c, xcb_window_t win, const char *name, const char *class)
{
  xcb_intern_atom_cookie_t cookie_encoding;
  xcb_intern_atom_cookie_t cookie_property;
  xcb_intern_atom_reply_t *rep;
  xcb_atom_t               encoding;
  char                    *atom_name;
  char                    *class_str;
  char                    *s;
  int                      length_name;
  int                      length_class;

  length_name = strlen (name);
  length_class = strlen (class);
  class_str = (char *)malloc (sizeof (char) * (length_name + length_class + 2));
  if (!class_str) return;
  s = class_str;
  memcpy (s, name, length_name);
  s += length_name;
  *s = '\0';
  s++;
  memcpy (s, class, length_class);
  s += length_class;
  *s = '\0';

  atom_name = "UTF8_STRING";
  cookie_encoding = xcb_intern_atom (c,
                                   0,
                                   strlen (atom_name),
                                   atom_name);
  atom_name = "_WM_CLASS";
  cookie_property = xcb_intern_atom (c,
                                   0,
                                   strlen (atom_name),
                                   atom_name);

  rep = xcb_intern_atom_reply (c, cookie_encoding, NULL);
  encoding = rep->atom;
  free (rep);

  rep = xcb_intern_atom_reply (c, cookie_property, NULL);

  xcb_change_property(c, XCB_PROP_MODE_REPLACE,
                    win,
                    rep->atom, encoding, 8, strlen (class_str), class_str);
  free (rep);
  free (class_str);
}

int
main(int argc, char **argv)
{
   int                                pause_me = 0;
   xcb_connection_t                  *conn;
   const xcb_query_extension_reply_t *rep_shm;
   const xcb_query_extension_reply_t *rep_xrender;
   xcb_get_input_focus_reply_t       *reply;
   xcb_screen_t                      *screen = NULL;
   xcb_screen_iterator_t              iter_screen;
   xcb_window_t                       win;
   xcb_generic_event_t               *e;
   uint32_t                           mask;
   uint32_t                           value[6];
   int                                screen_nbr;

   conn = xcb_connect (NULL, &screen_nbr);
   if (!conn)
     {
	printf("Error: cannot open a connection.\n");
	exit(-1);
     }

   xcb_prefetch_extension_data (conn, &xcb_shm_id);
   xcb_prefetch_extension_data (conn, &xcb_render_id);
   rep_shm = xcb_get_extension_data(conn, &xcb_shm_id);
   rep_xrender = xcb_get_extension_data(conn, &xcb_render_id);

   iter_screen = xcb_setup_roots_iterator (xcb_get_setup (conn));
   for (; iter_screen.rem; --screen_nbr, xcb_screen_next (&iter_screen))
      if (screen == 0)
        {
           screen = iter_screen.data;
           break;
        }
   if (!screen)
     {
	printf("Error: cannot get the screen.\n");
	exit(-1);
     }

   mask =
     XCB_CW_BACK_PIXMAP | XCB_CW_BORDER_PIXEL |
     XCB_CW_BIT_GRAVITY | XCB_CW_BACKING_STORE |
     XCB_CW_EVENT_MASK  | XCB_CW_COLORMAP;

   value[0] = XCB_BACK_PIXMAP_NONE;
   value[1] = 0;
   value[2] = XCB_GRAVITY_BIT_FORGET;
   value[3] = XCB_BACKING_STORE_NOT_USEFUL;
   value[4] = XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE | XCB_EVENT_MASK_POINTER_MOTION | XCB_EVENT_MASK_STRUCTURE_NOTIFY;
   value[5] = screen->default_colormap;

   win = xcb_generate_id (conn);
   xcb_create_window (conn,
                      screen->root_depth,
                      win, screen->root,
                      0, 0,
                      win_w, win_h,
                      0,
                      XCB_WINDOW_CLASS_INPUT_OUTPUT,
                      screen->root_visual,
                      mask, value);
   title_set (conn, win, "Evas XRender Xcb Test");
   class_set (conn, win, "Evas_XRender_XCB_Test", "Main");
#if 0
   szhints.flags = PMinSize | PMaxSize | PSize | USSize;
   szhints.min_width = szhints.max_width = win_w;
   szhints.min_height = szhints.max_height = win_h;
   XSetWMNormalHints(disp, win, &szhints);
#endif
   xcb_map_window (conn, win);
   /* we sync */
   reply = xcb_get_input_focus_reply(conn,
                                     xcb_get_input_focus_unchecked(conn),
                                     NULL);
   free(reply);

   /* test evas_free....  :) */
   evas_init();
   evas = evas_new();
   evas_output_method_set(evas, evas_render_method_lookup("xrender_xcb"));
   evas_output_size_set(evas, win_w, win_h);
   evas_output_viewport_set(evas, 0, 0, win_w, win_h);
   {
      Evas_Engine_Info_XRender_Xcb *einfo;

      einfo = (Evas_Engine_Info_XRender_Xcb *) evas_engine_info_get(evas);

      /* the following is specific to the engine */
      einfo->info.conn = conn;
      einfo->info.visual = screen->root_visual;
      einfo->info.drawable = win;
      evas_engine_info_set(evas, (Evas_Engine_Info *) einfo);
   }
   setup();

   orig_start_time = start_time = get_time();
   while (1)
     {
       e = xcb_poll_for_event(conn);

       if (e) {
	 switch (e->response_type)
	   {
	   case XCB_BUTTON_PRESS: {
	     xcb_button_press_event_t *ev = (xcb_button_press_event_t *)e;

	     if (ev->detail == 3)
               {
                 free(e);
                 goto exit;
               }
#if 0
	     if (!pause_me)
	       pause_me = 1;
	     else if (pause_me == 1)
	       pause_me = 2;
	     else
	       pause_me = 1;
#endif
	     evas_event_feed_mouse_move(evas, ev->event_x, ev->event_y, 0, NULL);
	     evas_event_feed_mouse_down(evas, ev->state, EVAS_BUTTON_NONE, 0, NULL);
	     break;
	   }
	   case XCB_BUTTON_RELEASE: {
	     xcb_button_release_event_t *ev = (xcb_button_release_event_t *)e;

	     evas_event_feed_mouse_move(evas, ev->event_x, ev->event_y, 0, NULL);
	     evas_event_feed_mouse_up(evas, ev->state, EVAS_BUTTON_NONE, 0, NULL);
	     break;
	   }
	   case XCB_MOTION_NOTIFY: {
	     xcb_motion_notify_event_t *ev = (xcb_motion_notify_event_t *)e;

	     evas_event_feed_mouse_move(evas, ev->event_x, ev->event_y, 0, NULL);
	     break;
	   }
	   case XCB_EXPOSE: {
	     xcb_expose_event_t *ev = (xcb_expose_event_t *)e;

	     evas_damage_rectangle_add(evas,
				       ev->x,
				       ev->y,
				       ev->width,
				       ev->height);
	     break;
	   }
	   case XCB_CONFIGURE_NOTIFY: {
	     xcb_configure_notify_event_t *ev = (xcb_configure_notify_event_t *)e;

	     evas_output_size_set(evas,
				  ev->width,
				  ev->height);
	     break;
	   default:
	     break;
	   }
	   }
	 free (e);
       }
       if (!(pause_me == 1))
	 {
	   loop();
	   evas_render(evas);
	   xcb_flush(conn);
	 }
       if (pause_me == 2)
	 usleep(100000);
     }

 exit:
   setdown();
   evas_free(evas);
   xcb_disconnect(conn);
   evas_shutdown();

   return 0;
}
