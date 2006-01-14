#include "evas_test_main.h"

#include <unistd.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>

#include "Evas.h"
#include "Evas_Engine_GL_X11.h"

int
main(int argc, char **argv)
{
   int                 pause_me = 0;
   Display            *disp;
   Window              win;
   XSetWindowAttributes attr;
   XClassHint          chint;

   disp = XOpenDisplay(NULL);
   if (!disp)
     {
	printf("Error: cannot open display.\n");
	exit(-1);
     }
   /* test evas_free....  :) */
   evas_init();
   evas = evas_new();
   evas_output_method_set(evas, evas_render_method_lookup("gl_x11"));
   evas_output_size_set(evas, win_w, win_h);
   evas_output_viewport_set(evas, 0, 0, win_w, win_h);
   {
      Evas_Engine_Info_GL_X11 *einfo;

      einfo = (Evas_Engine_Info_GL_X11 *) evas_engine_info_get(evas);

      /* the following is specific to the engine */
      attr.backing_store = NotUseful;
      attr.colormap = einfo->func.best_colormap_get(disp, DefaultScreen(disp));
      attr.border_pixel = 0;
      attr.background_pixmap = None;
      attr.event_mask =
	ExposureMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask |
	StructureNotifyMask;
      attr.bit_gravity = ForgetGravity;

      win = XCreateWindow(disp, DefaultRootWindow(disp), 0, 0, win_w, win_h, 0,
			  einfo->func.best_depth_get(disp, DefaultScreen(disp)),
			  InputOutput,
			  einfo->func.best_visual_get(disp, DefaultScreen(disp)),
			  CWBackingStore | CWColormap |
			  CWBackPixmap | CWBorderPixel |
			  CWBitGravity | CWEventMask, &attr);

      einfo->info.display = disp;
      einfo->info.visual = einfo->func.best_visual_get(disp, DefaultScreen(disp));
      einfo->info.colormap = einfo->func.best_colormap_get(disp, DefaultScreen(disp));
      einfo->info.drawable = win;
      einfo->info.depth = einfo->func.best_depth_get(disp, DefaultScreen(disp));

      evas_engine_info_set(evas, (Evas_Engine_Info *) einfo);
   }

   XStoreName(disp, win, "Evas GL X11 Test");
   chint.res_name = "Evas_GL_X11_Test";
   chint.res_class = "Main";
   XSetClassHint(disp, win, &chint);
#if 0
   szhints.flags = PMinSize | PMaxSize | PSize | USSize;
   szhints.min_width = szhints.max_width = win_w;
   szhints.min_height = szhints.max_height = win_h;
   XSetWMNormalHints(disp, win, &szhints);
#endif
   XMapWindow(disp, win);
   XSync(disp, False);

   setup();

   orig_start_time = start_time = get_time();
   for (;;)
     {
	XEvent              ev;

	while (XCheckMaskEvent(disp,
			       ExposureMask |
			       StructureNotifyMask |
			       KeyPressMask |
			       KeyReleaseMask |
			       ButtonPressMask |
			       ButtonReleaseMask | PointerMotionMask, &ev))
	  {
	     switch (ev.type)
	       {
	       case ButtonPress:
		  if (ev.xbutton.button == 3)
		     exit(0);
		  if (!pause_me)
		     pause_me = 1;
		  else if (pause_me == 1)
		     pause_me = 2;
		  else
		     pause_me = 1;
		  evas_event_feed_mouse_move(evas, ev.xbutton.x, ev.xbutton.y, 0, NULL);
		  evas_event_feed_mouse_down(evas, ev.xbutton.button, EVAS_BUTTON_NONE, 0, NULL);
		  break;
	       case ButtonRelease:
		  evas_event_feed_mouse_move(evas, ev.xbutton.x, ev.xbutton.y, 0, NULL);
		  evas_event_feed_mouse_up(evas, ev.xbutton.button, EVAS_BUTTON_NONE, 0, NULL);
		  break;
	       case MotionNotify:
		  evas_event_feed_mouse_move(evas, ev.xmotion.x, ev.xmotion.y, 0, NULL);
		  break;
	       case Expose:
		  evas_damage_rectangle_add(evas,
					    ev.xexpose.x,
					    ev.xexpose.y,
					    ev.xexpose.width,
					    ev.xexpose.height);
		  break;
	       case ConfigureNotify:
		  evas_output_size_set(evas,
				       ev.xconfigure.width,
				       ev.xconfigure.height);
		  break;
	       default:
		  break;
	       }
	  }
	if (!(pause_me == 1))
	  {
	     loop();
	     evas_render(evas);
	     XFlush(disp);
	  }
	if (pause_me == 2)
	   usleep(100000);
     }
   evas_shutdown();
   return 0;
}
