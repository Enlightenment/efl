#include "evas_test_main.h"

#include <unistd.h>
#include <X11/XCB/xcb.h>

#include "Evas.h"
#include "Evas_Engine_XRender_Xcb.h"


XCBSCREEN *
get_screen (XCBConnection *c,
            int            screen)
{
  XCBSCREENIter iter;

  iter = XCBConnSetupSuccessRepRootsIter (XCBGetSetup (c));
  for (; iter.rem; --screen, XCBSCREENNext (&iter))
    if (screen == 0)
      return iter.data;

  return NULL;
}

int
get_depth(XCBConnection *conn,
	  XCBSCREEN     *root)
{
  XCBDRAWABLE        drawable;
  XCBGetGeometryRep *geom;
  int                depth;

  drawable.window = root->root;
  geom = XCBGetGeometryReply (conn, XCBGetGeometry(conn, drawable), 0);

  if(!geom)
    {
      perror ("GetGeometry(root) failed");
      exit (0);
    }

  depth = geom->depth;
  free (geom);

  return depth;
}

static void title_set (XCBConnection *conn, XCBWINDOW window, const char *title)
{
  XCBInternAtomRep *rep;
  XCBATOM           encoding;
  char             *atom_name;

  /* encoding */
  atom_name = "UTF8_STRING";
  rep = XCBInternAtomReply (conn,
                            XCBInternAtom (conn,
                                           0,
                                           strlen (atom_name),
                                           atom_name),
                            NULL);
  encoding = rep->atom;
  free (rep);

  /* ICCCM */
/*   SetWMName (f->xcb.c, f->xcb.draw.window, encoding, strlen (title), title); */

  /* NETWM */
  atom_name = "_NET_WM_NAME";
  rep = XCBInternAtomReply (conn,
                            XCBInternAtom (conn,
                                           0,
                                           strlen (atom_name),
                                           atom_name),
                            NULL);
  XCBChangeProperty(conn, PropModeReplace,
                    window,
                    rep->atom, encoding, 8, strlen (title), title);
  free (rep);
}

int
main(int argc, char **argv)
{
   int              pause_me = 0;
   XCBConnection   *conn;
   XCBSCREEN       *screen;
   XCBDRAWABLE      win;
   XCBGenericEvent *e;
   CARD32           mask;
   CARD32           value[6];
   int              screen_nbr;

   conn = XCBConnect (NULL, &screen_nbr);
   if (!conn)
     {
	printf("Error: cannot open a connection.\n");
	exit(-1);
     }

   screen = get_screen (conn, screen_nbr);

   mask =
     XCBCWBackingStore | XCBCWColormap |
     XCBCWBackPixmap   | XCBCWBorderPixel |
     XCBCWBitGravity   | XCBCWEventMask;

   value[0] = None;
   value[1] = 0;
   value[2] = ForgetGravity;
   value[3] = NotUseful;
   value[4] = ExposureMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask | StructureNotifyMask;
   value[5] = screen->default_colormap.xid;

   win.window = XCBWINDOWNew(conn);
   XCBCreateWindow (conn,
		    get_depth(conn, screen),
		    win.window, screen->root,
		    0, 0,
		    win_w, win_h,
		    0,
		    InputOutput,
		    screen->root_visual,
		    mask, value);
   title_set (conn, win.window, "Evas XRender Xcb Test");
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
	   case Expose: {
	     XCBExposeEvent *ev = (XCBExposeEvent *)e;

	     evas_damage_rectangle_add(evas,
				       ev->x,
				       ev->y,
				       ev->width,
				       ev->height);
	     break;
	   }
	   case ConfigureNotify: {
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
