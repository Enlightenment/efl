#include "evas_test_main.h"

#include <X11/XCB/xcb.h>
#include <X11/XCB/xcb_icccm.h>

#include "Evas.h"
#include "Evas_Engine_Software_Xcb.h"



XCBVISUALTYPE *
get_visual(XCBConnection *conn,
	   XCBSCREEN     *root)
{
   XCBDEPTH         *d;
   XCBVISUALTYPEIter iter;
   int               cur;

   d = XCBSCREENAllowedDepthsIter(root).data;
   if (!d) return NULL;

   iter = XCBDEPTHVisualsIter(d);
   for (cur = 0 ; cur < iter.rem ; XCBVISUALTYPENext(&iter), ++cur)
      if (root->root_visual.id == iter.data->visual_id.id)
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

int
main(int argc, char **argv)
{
   int              pause_me = 0;
   XCBConnection   *conn;
   XCBDRAWABLE      win;
   XCBSCREEN       *screen;
   XCBGenericEvent *e;
   CARD32           mask;
   CARD32           value[6];
/*    XClassHint       chint; */
   SizeHints       *szhints;

   conn = XCBConnectBasic ();
   if (!conn)
     {
	printf("Error: cannot open a connection.\n");
	exit(-1);
     }
   
   screen = XCBConnSetupSuccessRepRootsIter (XCBGetSetup(conn)).data;

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

/*    XStoreName(disp, win, "Evas Software X11 Test"); */
/*    chint.res_name = "Evas_Software_X11_Test"; */
/*    chint.res_class = "Main"; */
/*    SetClassHint(disp, win, &chint); */
#if 0
   szhints = AllocSizeHints();
   SizeHintsSetMinSize(szhints, win_w, win_h);
   SizeHintsSetMaxSize(szhints, win_w, win_h);
   SizeHintsSetSize(szhints, 0, win_w, win_h);
   SetWMNormalHints(conn, win.window, szhints);
   FreeSizeHints(szhints);
#endif
   XCBMapWindow (conn, win.window);
   XCBSync(conn, 0);

   /* test evas_free....  :) */
   evas = evas_new();
   evas_output_method_set(evas, evas_render_method_lookup("software_xcb"));
   evas_output_size_set(evas, win_w, win_h);
   evas_output_viewport_set(evas, 0, 0, win_w, win_h);
   {
      Evas_Engine_Info_Software_Xcb *einfo;

      einfo = (Evas_Engine_Info_Software_Xcb *) evas_engine_info_get(evas);

      /* the following is specific to the engine */
      einfo->info.conn = conn;
      einfo->info.visual = get_visual (conn, screen);
      einfo->info.colormap = screen->default_colormap;
      einfo->info.drawable = win;
      einfo->info.depth = get_depth(conn, screen);
      einfo->info.rotation = 0;
      einfo->info.debug = 0;
      evas_engine_info_set(evas, (Evas_Engine_Info *) einfo);
   }
   setup();
   printf("################ evas free\n");
   evas_free(evas);
   printf("evas freed. DONE\n");
	
   evas = evas_new();
   evas_output_method_set(evas, evas_render_method_lookup("software_xcb"));
   evas_output_size_set(evas, win_w, win_h);
   evas_output_viewport_set(evas, 0, 0, win_w, win_h);
   {
      Evas_Engine_Info_Software_Xcb *einfo;

      einfo = (Evas_Engine_Info_Software_Xcb *) evas_engine_info_get(evas);

      /* the following is specific to the engine */
      einfo->info.conn = conn;
      einfo->info.visual = get_visual (conn, screen);
      einfo->info.colormap = screen->default_colormap;
      einfo->info.drawable = win;
      einfo->info.depth = get_depth(conn, screen);
      einfo->info.rotation = 0;
      einfo->info.debug = 0;
      evas_engine_info_set(evas, (Evas_Engine_Info *) einfo);
   }
   setup();
   printf("################ evas free\n");
   evas_free(evas);
   printf("evas freed. DONE\n");
	
   evas = evas_new();
   evas_output_method_set(evas, evas_render_method_lookup("software_xcb"));
   evas_output_size_set(evas, win_w, win_h);
   evas_output_viewport_set(evas, 0, 0, win_w, win_h);
   {
      Evas_Engine_Info_Software_Xcb *einfo;

      einfo = (Evas_Engine_Info_Software_Xcb *) evas_engine_info_get(evas);

      /* the following is specific to the engine */
      einfo->info.conn = conn;
      einfo->info.visual = get_visual (conn, screen);
      einfo->info.colormap = screen->default_colormap;
      einfo->info.drawable = win;
      einfo->info.depth = get_depth(conn, screen);
      einfo->info.rotation = 0;
      einfo->info.debug = 0;
      evas_engine_info_set(evas, (Evas_Engine_Info *) einfo);
   }
   setup();
   printf("################ evas free\n");
   evas_free(evas);
   printf("evas freed. DONE\n");
	
   evas = evas_new();
   evas_output_method_set(evas, evas_render_method_lookup("software_xcb"));
   evas_output_size_set(evas, win_w, win_h);
   evas_output_viewport_set(evas, 0, 0, win_w, win_h);
   {
      Evas_Engine_Info_Software_Xcb *einfo;

      einfo = (Evas_Engine_Info_Software_Xcb *) evas_engine_info_get(evas);

      /* the following is specific to the engine */
      einfo->info.conn = conn;
      einfo->info.visual = get_visual (conn, screen);
      einfo->info.colormap = screen->default_colormap;
      einfo->info.drawable = win;
      einfo->info.depth = get_depth(conn, screen);
      einfo->info.rotation = 0;
      einfo->info.debug = 0;
      evas_engine_info_set(evas, (Evas_Engine_Info *) einfo);
   }
   setup();
   printf("################ evas free\n");
   evas_free(evas);
   printf("evas freed. DONE\n");
   
   evas = evas_new();
   evas_output_method_set(evas, evas_render_method_lookup("software_xcb"));
   evas_output_size_set(evas, win_w, win_h);
   evas_output_viewport_set(evas, 0, 0, win_w, win_h);
   {
      Evas_Engine_Info_Software_Xcb *einfo;

      einfo = (Evas_Engine_Info_Software_Xcb *) evas_engine_info_get(evas);

      /* the following is specific to the engine */
      einfo->info.conn = conn;
      einfo->info.visual = get_visual (conn, screen);
      einfo->info.colormap = screen->default_colormap;
      einfo->info.drawable = win;
      einfo->info.depth = get_depth(conn, screen);
      einfo->info.rotation = 0;
      einfo->info.debug = 0;
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
	     
	     if ((ev->state | Button3Mask) == Button3Mask)
	       exit(0);
#if 0		  
	     if (!pause_me)
	       pause_me = 1;
	     else if (pause_me == 1)
	       pause_me = 2;
	     else
	       pause_me = 1;
#endif		  
	     evas_event_feed_mouse_move(evas, ev->event_x, ev->event_y, NULL);
	     evas_event_feed_mouse_down(evas, ev->state, EVAS_BUTTON_NONE, NULL);
	     break;
	   }
	   case XCBButtonRelease: {
	     XCBButtonReleaseEvent *ev = (XCBButtonReleaseEvent *)e;
	     
	     evas_event_feed_mouse_move(evas, ev->event_x, ev->event_y, NULL);
	     evas_event_feed_mouse_up(evas, ev->state, EVAS_BUTTON_NONE, NULL);
	     break;
	   }
	   case XCBMotionNotify: {
	     XCBMotionNotifyEvent *ev = (XCBMotionNotifyEvent *)e;
	     
	     evas_event_feed_mouse_move(evas, ev->event_x, ev->event_y, NULL);
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
   
   return 0;
}

