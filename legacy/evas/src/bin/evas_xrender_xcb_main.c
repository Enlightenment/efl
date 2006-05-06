#include "evas_test_main.h"

#include <unistd.h>

#define X_H   /* make sure we aren't using symbols from X.h */

#include <X11/XCB/xcb.h>
#include <X11/XCB/shm.h>
#include <X11/XCB/render.h>
#include <X11/XCB/xcb_aux.h>

#include "Evas.h"
#include "Evas_Engine_XRender_Xcb.h"


static void
title_set (XCBConnection *c, XCBWINDOW win, const char *title)
{
  XCBInternAtomCookie cookie_encoding;
  XCBInternAtomCookie cookie_property;
  XCBInternAtomRep   *rep;
  XCBATOM             encoding;
  char               *atom_name;

  atom_name = "UTF8_STRING";
  cookie_encoding = XCBInternAtom (c,
                                   0,
                                   strlen (atom_name),
                                   atom_name);
  atom_name = "_NET_WM_NAME";
  cookie_property = XCBInternAtom (c,
                                   0,
                                   strlen (atom_name),
                                   atom_name);

  rep = XCBInternAtomReply (c, cookie_encoding, NULL);
  encoding = rep->atom;
  free (rep);

  rep = XCBInternAtomReply (c, cookie_property, NULL);

  XCBChangeProperty(c, XCBPropModeReplace,
                    win,
                    rep->atom, encoding, 8, strlen (title), title);
  free (rep);
}

static void
class_set (XCBConnection *c, XCBWINDOW win, const char *name, const char *class)
{
  XCBInternAtomCookie cookie_encoding;
  XCBInternAtomCookie cookie_property;
  XCBInternAtomRep   *rep;
  XCBATOM             encoding;
  char               *atom_name;
  char               *class_str;
  char               *s;
  int                 length_name;
  int                 length_class;

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
  cookie_encoding = XCBInternAtom (c,
                                   0,
                                   strlen (atom_name),
                                   atom_name);
  atom_name = "_WM_CLASS";
  cookie_property = XCBInternAtom (c,
                                   0,
                                   strlen (atom_name),
                                   atom_name);

  rep = XCBInternAtomReply (c, cookie_encoding, NULL);
  encoding = rep->atom;
  free (rep);

  rep = XCBInternAtomReply (c, cookie_property, NULL);

  XCBChangeProperty(c, XCBPropModeReplace,
                    win,
                    rep->atom, encoding, 8, strlen (class_str), class_str);
  free (rep);
}

int
main(int argc, char **argv)
{
   int                         pause_me = 0;
   XCBConnection              *conn;
   const XCBQueryExtensionRep *rep_shm;
   const XCBQueryExtensionRep *rep_xrender;
   XCBSCREEN                  *screen;
   XCBDRAWABLE                 win;
   XCBGenericEvent            *e;
   CARD32                      mask;
   CARD32                      value[6];
   int                         screen_nbr;

   conn = XCBConnect (NULL, &screen_nbr);
   if (!conn)
     {
	printf("Error: cannot open a connection.\n");
	exit(-1);
     }

   XCBPrefetchExtensionData (conn, &XCBShmId);
   XCBPrefetchExtensionData (conn, &XCBRenderId);
   rep_shm = XCBGetExtensionData(conn, &XCBShmId);
   rep_xrender = XCBGetExtensionData(conn, &XCBRenderId);

   screen = XCBAuxGetScreen (conn, screen_nbr);

   mask =
     XCBCWBackPixmap | XCBCWBorderPixel |
     XCBCWBitGravity | XCBCWBackingStore |
     XCBCWEventMask  | XCBCWColormap;

   value[0] = XCBBackPixmapNone;
   value[1] = 0;
   value[2] = XCBGravityBitForget;
   value[3] = XCBBackingStoreNotUseful;
   value[4] = XCBEventMaskExposure | XCBEventMaskButtonPress | XCBEventMaskButtonRelease | XCBEventMaskPointerMotion | XCBEventMaskStructureNotify;
   value[5] = screen->default_colormap.xid;

   win.window = XCBWINDOWNew(conn);
   XCBCreateWindow (conn,
		    XCBAuxGetDepth(conn, screen),
		    win.window, screen->root,
		    0, 0,
		    win_w, win_h,
		    0,
		    XCBWindowClassInputOutput,
		    screen->root_visual,
		    mask, value);
   title_set (conn, win.window, "Evas XRender Xcb Test");
   class_set (conn, win.window, "Evas_XRender_XCB_Test", "Main");
#if 0
   szhints.flags = PMinSize | PMaxSize | PSize | USSize;
   szhints.min_width = szhints.max_width = win_w;
   szhints.min_height = szhints.max_height = win_h;
   XSetWMNormalHints(disp, win, &szhints);
#endif
   XCBMapWindow (conn, win.window);
   XCBSync(conn, 0);

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
       e = XCBPollForEvent(conn, NULL);

       if (e) {
	 switch (e->response_type)
	   {
	   case XCBButtonPress: {
	     XCBButtonPressEvent *ev = (XCBButtonPressEvent *)e;

	     if (ev->detail.id == 3)
	       {
		 setdown();
		 evas_free(evas);
                 free(e);
		 XCBDisconnect(conn);
		 evas_shutdown();
		 exit(0);
		 
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
	   case XCBButtonRelease: {
	     XCBButtonReleaseEvent *ev = (XCBButtonReleaseEvent *)e;

	     evas_event_feed_mouse_move(evas, ev->event_x, ev->event_y, 0, NULL);
	     evas_event_feed_mouse_up(evas, ev->state, EVAS_BUTTON_NONE, 0, NULL);
	     break;
	   }
	   case XCBMotionNotify: {
	     XCBMotionNotifyEvent *ev = (XCBMotionNotifyEvent *)e;

	     evas_event_feed_mouse_move(evas, ev->event_x, ev->event_y, 0, NULL);
	     break;
	   }
	   case XCBExpose: {
	     XCBExposeEvent *ev = (XCBExposeEvent *)e;

	     evas_damage_rectangle_add(evas,
				       ev->x,
				       ev->y,
				       ev->width,
				       ev->height);
	     break;
	   }
	   case XCBConfigureNotify: {
	     XCBConfigureNotifyEvent *ev = (XCBConfigureNotifyEvent *)e;

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
	   XCBFlush(conn);
	 }
       if (pause_me == 2)
	 usleep(100000);
     }

   setdown();
   evas_free(evas);
   XCBDisconnect(conn);
   evas_shutdown();

   return 0;
}
