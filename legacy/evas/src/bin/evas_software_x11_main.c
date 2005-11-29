#include "evas_test_main.h"

#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "Evas.h"
#include "Evas_Engine_Software_X11.h"

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
   attr.backing_store = NotUseful;
   attr.colormap = DefaultColormap(disp, DefaultScreen(disp));
   attr.border_pixel = 0;
   attr.background_pixmap = None;
   attr.event_mask =
      ExposureMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask |
      StructureNotifyMask;
   attr.bit_gravity = ForgetGravity;
   win = XCreateWindow(disp, DefaultRootWindow(disp), 0, 0, win_w, win_h, 0,
		       DefaultDepth(disp, DefaultScreen(disp)), InputOutput,
		       DefaultVisual(disp, DefaultScreen(disp)),
		       CWBackingStore | CWColormap |
		       CWBackPixmap | CWBorderPixel |
		       CWBitGravity | CWEventMask, &attr);
   XStoreName(disp, win, "Evas Software X11 Test");
   chint.res_name = "Evas_Software_X11_Test";
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

   /* test evas_free....  :) */
   evas_init();
   evas = evas_new();
   evas_output_method_set(evas, evas_render_method_lookup("software_x11"));
   evas_output_size_set(evas, win_w, win_h);
   evas_output_viewport_set(evas, 0, 0, win_w, win_h);
   {
      Evas_Engine_Info_Software_X11 *einfo;

      einfo = (Evas_Engine_Info_Software_X11 *) evas_engine_info_get(evas);

      /* the following is specific to the engine */
      einfo->info.display = disp;
      einfo->info.visual = DefaultVisual(disp, DefaultScreen(disp));
      einfo->info.colormap = DefaultColormap(disp, DefaultScreen(disp));
      einfo->info.drawable = win;
      einfo->info.depth = DefaultDepth(disp, DefaultScreen(disp));
      einfo->info.rotation = 0;
      einfo->info.debug = 0;
      evas_engine_info_set(evas, (Evas_Engine_Info *) einfo);
   }
   setup();
   printf("################ evas free\n");
   setdown();
   evas_free(evas);
   printf("evas freed. DONE\n");

   evas = evas_new();
   evas_output_method_set(evas, evas_render_method_lookup("software_x11"));
   evas_output_size_set(evas, win_w, win_h);
   evas_output_viewport_set(evas, 0, 0, win_w, win_h);
   {
      Evas_Engine_Info_Software_X11 *einfo;

      einfo = (Evas_Engine_Info_Software_X11 *) evas_engine_info_get(evas);

      /* the following is specific to the engine */
      einfo->info.display = disp;
      einfo->info.visual = DefaultVisual(disp, DefaultScreen(disp));
      einfo->info.colormap = DefaultColormap(disp, DefaultScreen(disp));
      einfo->info.drawable = win;
      einfo->info.depth = DefaultDepth(disp, DefaultScreen(disp));
      einfo->info.rotation = 0;
      einfo->info.debug = 0;
      evas_engine_info_set(evas, (Evas_Engine_Info *) einfo);
   }
   setup();
   printf("################ evas free\n");
   setdown();
   evas_free(evas);
   printf("evas freed. DONE\n");

   evas = evas_new();
   evas_output_method_set(evas, evas_render_method_lookup("software_x11"));
   evas_output_size_set(evas, win_w, win_h);
   evas_output_viewport_set(evas, 0, 0, win_w, win_h);
   {
      Evas_Engine_Info_Software_X11 *einfo;

      einfo = (Evas_Engine_Info_Software_X11 *) evas_engine_info_get(evas);

      /* the following is specific to the engine */
      einfo->info.display = disp;
      einfo->info.visual = DefaultVisual(disp, DefaultScreen(disp));
      einfo->info.colormap = DefaultColormap(disp, DefaultScreen(disp));
      einfo->info.drawable = win;
      einfo->info.depth = DefaultDepth(disp, DefaultScreen(disp));
      einfo->info.rotation = 0;
      einfo->info.debug = 0;
      evas_engine_info_set(evas, (Evas_Engine_Info *) einfo);
   }
   setup();
   printf("################ evas free\n");
   setdown();
   evas_free(evas);
   printf("evas freed. DONE\n");

   evas = evas_new();
   evas_output_method_set(evas, evas_render_method_lookup("software_x11"));
   evas_output_size_set(evas, win_w, win_h);
   evas_output_viewport_set(evas, 0, 0, win_w, win_h);
   {
      Evas_Engine_Info_Software_X11 *einfo;

      einfo = (Evas_Engine_Info_Software_X11 *) evas_engine_info_get(evas);

      /* the following is specific to the engine */
      einfo->info.display = disp;
      einfo->info.visual = DefaultVisual(disp, DefaultScreen(disp));
      einfo->info.colormap = DefaultColormap(disp, DefaultScreen(disp));
      einfo->info.drawable = win;
      einfo->info.depth = DefaultDepth(disp, DefaultScreen(disp));
      einfo->info.rotation = 0;
      einfo->info.debug = 0;
      evas_engine_info_set(evas, (Evas_Engine_Info *) einfo);
   }
   setup();
   printf("################ evas free\n");
   setdown();
   evas_free(evas);
   printf("evas freed. DONE\n");

   evas = evas_new();
   evas_output_method_set(evas, evas_render_method_lookup("software_x11"));
   evas_output_size_set(evas, win_w, win_h);
   evas_output_viewport_set(evas, 0, 0, win_w, win_h);
   {
      Evas_Engine_Info_Software_X11 *einfo;

      einfo = (Evas_Engine_Info_Software_X11 *) evas_engine_info_get(evas);

      /* the following is specific to the engine */
      einfo->info.display = disp;
      einfo->info.visual = DefaultVisual(disp, DefaultScreen(disp));
      einfo->info.colormap = DefaultColormap(disp, DefaultScreen(disp));
      einfo->info.drawable = win;
      einfo->info.depth = DefaultDepth(disp, DefaultScreen(disp));
      einfo->info.rotation = 0;
      einfo->info.debug = 0;
      evas_engine_info_set(evas, (Evas_Engine_Info *) einfo);
   }
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
		  {
			  setdown();
			  evas_free(evas);
			  XCloseDisplay(disp);
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

   setdown();
   evas_free(evas);
   XCloseDisplay(disp);
   evas_shutdown();

   return 0;
}
