#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>

#define X_H   /* make sure we aren't using symbols from X.h */

#include <xcb/xcb.h>
#include <xcb/xcb_icccm.h>

#include "Evas.h"
#include "Evas_Engine_Software_Xcb.h"

xcb_connection_t *c = NULL;
xcb_drawable_t    win;

Evas             *evas = NULL;

int               win_w = 240;
int               win_h = 240;


static xcb_visualtype_t *
visualtype_get(xcb_connection_t *conn, xcb_screen_t *screen)
{
   xcb_depth_iterator_t  iter_depth;

   if (!conn || !screen) return NULL;

   iter_depth = xcb_screen_allowed_depths_iterator(screen);
   for (; iter_depth.rem; xcb_depth_next (&iter_depth))
     {
        xcb_visualtype_iterator_t iter_vis;

        iter_vis = xcb_depth_visuals_iterator(iter_depth.data);
        for (; iter_vis.rem; --screen, xcb_visualtype_next (&iter_vis))
          {
            if (screen->root_visual.id == iter_vis.data->visual_id.id)
              return iter_vis.data;
          }
     }

   return NULL;
}


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
}

int
main(int argc, char **argv)
{
   int                          exposed = 0;
   xcb_screen_t                *screen = NULL;
   xcb_screen_iterator_t        iter_screen;
   xcb_get_input_focus_reply_t *reply;
   xcb_generic_event_t         *e;
   SizeHints                   *szhints;
   uint32_t                     mask = 0;
   uint32_t                     value[6];
   int                          screen_nbr;

   c = xcb_connect (NULL, &screen_nbr);
   if (!c)
     {
	printf("Error: cannot open a connection.\n");
	exit(-1);
     }

   iter_screen = xcb_setup_roots_iterator (xcb_get_setup (c));
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
     XCB_CW_BIT_GRAVITY | XCB_CW_BACKING_STORE  |
     XCB_CW_EVENT_MASK  | XCB_CW_COLORMAP;

   value[0] = XCB_BACK_PIXMAP_NONE;
   value[1] = 0;
   value[2] = XCB_GRAVITY_BIT_FORGET;
   value[3] = XCB_BACKING_STORE_NOT_USEFUL;
   value[4] = XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE | XCB_EVENT_MASK_POINTER_MOTION;
   value[5] = screen->default_colormap.xid;

   win.window = xcb_window_new(c);
   xcb_create_window (c,
                      screen->root_depth,
                      win.window, screen->root,
                      0, 0,
                      win_w, win_h,
                      0,
                      XCB_WINDOW_CLASS_INPUT_OUTPUT,
                      screen->root_visual,
                      mask, value);
   title_set (c, win.window, "Evas XCB Performance Test");
   class_set (c, win.window, "Evas_XCB_Perf_Test", "Main");

   szhints = AllocSizeHints();
   SizeHintsSetMinSize(szhints, win_w, win_h);
   SizeHintsSetMaxSize(szhints, win_w, win_h);
   SizeHintsSetSize(szhints, 0, win_w, win_h);
   SetWMNormalHints(c, win.window, szhints);
   FreeSizeHints(szhints);

   xcb_map_window (c, win.window);
   /* we sync */
   reply = xcb_get_input_focus_reply(c,
                                     xcb_get_input_focus_unchecked(c),
                                     NULL);
   free(reply);
   sleep(1);

   evas_init();
   evas = evas_new();
   evas_output_method_set(evas, evas_render_method_lookup("software_xcb"));
   evas_output_size_set(evas, win_w, win_h);
   evas_output_viewport_set(evas, 0, 0, win_w, win_h);
   {
      Evas_Engine_Info_Software_Xcb *einfo;

      einfo = (Evas_Engine_Info_Software_Xcb *) evas_engine_info_get(evas);

      /* the following is specific to the engine */
      einfo->info.conn = c;
      einfo->info.visual = visualtype_get(c, screen);
      einfo->info.colormap = screen->default_colormap;
      einfo->info.drawable = win;
      einfo->info.depth = screen->root_depth;
      einfo->info.rotation = 0;

      evas_engine_info_set(evas, (Evas_Engine_Info *) einfo);
   }

   while (1)
     {
        e = xcb_poll_for_event(c, NULL);

	if (e)
	  {
	    switch (e->response_type)
	      {
	      case XCB_BUTTON_PRESS: {
	        xcb_button_press_event_t *ev = (xcb_button_press_event_t *)e;

/*		  evas_event_button_down(evas, ev->event_x, ev->event_y, ev->detail.id);*/
		break;
	      }
	      case XCB_BUTTON_RELEASE: {
	        xcb_button_release_event_t *ev = (xcb_button_release_event_t *)e;

		/*		  evas_event_button_up(evas, ev->event_x, ev->event_y, ev->detail.id);*/
		break;
	      }
	      case XCB_EXPOSE: {
		xcb_expose_event_t *ev = (xcb_expose_event_t *)e;

	        exposed = 1;
		evas_damage_rectangle_add(evas,
					  ev->x,
					  ev->y,
					  ev->width,
					  ev->height);
		break;
	      }
	      }
	    free (e);
	  }

	evas_render(evas);
	xcb_flush(c);

	if (exposed)
	  {
	     char                           buf[4096];
	     Evas_Engine_Info_Software_Xcb *einfo;
	     Evas_Performance              *perf;
	     char                          *data, *key;
	     static mode_t                  default_mode =
		S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH |
		S_IXOTH;
	     FILE                          *f;

	     einfo =
		(Evas_Engine_Info_Software_Xcb *) evas_engine_info_get(evas);
	     perf =
		einfo->func.performance_test(evas, c,
					     visualtype_get(c, screen),
					     screen->default_colormap,
					     win,
					     screen->root_depth);
	     einfo->func.performance_device_store(perf);
	     data = einfo->func.performance_data_get(perf);
	     key = einfo->func.performance_key_get(perf);
	     snprintf(buf, sizeof(buf), "%s/.evas", getenv("HOME"));
	     mkdir(buf, default_mode);
	     snprintf(buf, sizeof(buf), "%s/.evas/%s", getenv("HOME"), key);
	     free(key);
	     f = fopen(buf, "w");
	     if (f)
	       {
		  fprintf(f, "%s", data);
		  free(data);
		  fclose(f);
	       }
	     einfo->func.performance_free(perf);
	     exit(0);
	  }
     }

   evas_shutdown();
   return 0;
}
