#include <X11/XCB/xcb.h>
#include <X11/XCB/xcb_icccm.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "Evas.h"
#include "Evas_Engine_Software_Xcb.h"

XCBConnection   *c = NULL;
XCBDRAWABLE      win;

Evas               *evas = NULL;

int                 win_w = 240;
int                 win_h = 240;

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
   int                 exposed = 0;

   XCBSCREEN       *screen;
   XCBGenericEvent *e;
   CARD32           mask = 0;
   CARD32           value[6];
/*    XClassHint       chint; */
   SizeHints       *szhints;

   c = XCBConnectBasic ();
   if (!c)
     {
	printf("Error: cannot open a connection.\n");
	exit(-1);
     }
   
   screen = XCBConnSetupSuccessRepRootsIter (XCBGetSetup(c)).data;

   mask = CWBackingStore | CWColormap |
     CWBackPixmap | CWBorderPixel |
     CWBitGravity | CWEventMask;

   value[0] = None;
   value[1] = 0;
   value[2] = ForgetGravity;
   value[3] = NotUseful;
   value[4] = ExposureMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask;
   value[5] = screen->default_colormap.xid;

   win.window = XCBWINDOWNew(c);
   XCBCreateWindow (c,
		    get_depth(c, screen),
		    win.window, screen->root,
		    0, 0,
		    win_w, win_h,
		    0,
		    InputOutput,
		    screen->root_visual,
		    mask, value);
/*    XStoreName(disp, win, "Evas Performance Test"); */
/*    chint.res_name = "Evas_Test"; */
/*    chint.res_class = "Main"; */
/*    XSetClassHint(disp, win, &chint); */

   szhints = AllocSizeHints();
   SizeHintsSetMinSize(szhints, win_w, win_h);
   SizeHintsSetMaxSize(szhints, win_w, win_h);
   SizeHintsSetSize(szhints, 0, win_w, win_h);
   SetWMNormalHints(c, win.window, szhints);
   FreeSizeHints(szhints);

   XCBMapWindow (c, win.window);
   XCBSync(c, 0);
   sleep(1);

   evas = evas_new();
   evas_output_method_set(evas, evas_render_method_lookup("software_xcb"));
   evas_output_size_set(evas, win_w, win_h);
   evas_output_viewport_set(evas, 0, 0, win_w, win_h);
   {
      Evas_Engine_Info_Software_Xcb *einfo;

      einfo = (Evas_Engine_Info_Software_Xcb *) evas_engine_info_get(evas);

      /* the following is specific to the engine */
      einfo->info.conn = c;
      einfo->info.visual = get_visual (c, screen);
      einfo->info.colormap = screen->default_colormap;
      einfo->info.drawable = win;
      einfo->info.depth = get_depth(c, screen);
      einfo->info.rotation = 0;

      evas_engine_info_set(evas, (Evas_Engine_Info *) einfo);
   }

   while (1)
     {
        e = XCBPollForEvent(c, NULL);

	if (e)
	  {
	    switch (e->response_type)
	      {
	      case XCBButtonPress: {
	        XCBButtonPressEvent *ev = (XCBButtonPressEvent *)e;
		
/*		  evas_event_button_down(evas, ev->event_x, ev->event_y, ev->detail.id);*/
		break;
	      }
	      case XCBButtonRelease: {
	        XCBButtonReleaseEvent *ev = (XCBButtonReleaseEvent *)e;
		
		/*		  evas_event_button_up(evas, ev->event_x, ev->event_y, ev->detail.id);*/
		break;
	      }
	      case XCBExpose: {
		XCBExposeEvent *ev = (XCBExposeEvent *)e;

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
	XCBFlush(c);

	if (exposed)
	  {
	     Evas_Engine_Info_Software_Xcb *einfo;
	     Evas_Performance              *perf;
	     char                           buf[4096];
	     char                          *data, *key;
	     static mode_t                  default_mode =
		S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH |
		S_IXOTH;
	     FILE                          *f;

	     einfo =
		(Evas_Engine_Info_Software_Xcb *) evas_engine_info_get(evas);
	     perf =
		einfo->func.performance_test(evas, c,
					     get_visual (c, screen),
					     screen->default_colormap,
					     win,
					     get_depth(c, screen));
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
   
   return 0;
}
