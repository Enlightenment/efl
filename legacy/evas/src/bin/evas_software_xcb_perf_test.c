#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>

#define X_H   /* make sure we aren't using symbols from X.h */

#include <X11/XCB/xcb.h>
#include <X11/XCB/xcb_icccm.h>
#include <X11/XCB/xcb_aux.h>

#include "Evas.h"
#include "Evas_Engine_Software_Xcb.h"

XCBConnection   *c = NULL;
XCBDRAWABLE      win;

Evas               *evas = NULL;

int                 win_w = 240;
int                 win_h = 240;


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
   int                  exposed = 0;
   XCBSCREEN           *screen;
   XCBGetInputFocusRep *reply;
   XCBGenericEvent     *e;
   SizeHints           *szhints;
   CARD32               mask = 0;
   CARD32               value[6];
   int                  screen_nbr;

   c = XCBConnect (NULL, &screen_nbr);
   if (!c)
     {
	printf("Error: cannot open a connection.\n");
	exit(-1);
     }

   screen = XCBAuxGetScreen (c, screen_nbr);

   mask = 
     XCBCWBackPixmap | XCBCWBorderPixel |
     XCBCWBitGravity | XCBCWBackingStore  |
     XCBCWEventMask  | XCBCWColormap;

   value[0] = XCBBackPixmapNone;
   value[1] = 0;
   value[2] = XCBGravityBitForget;
   value[3] = XCBBackingStoreNotUseful;
   value[4] = XCBEventMaskExposure | XCBEventMaskButtonPress | XCBEventMaskButtonRelease | XCBEventMaskPointerMotion;
   value[5] = screen->default_colormap.xid;

   win.window = XCBWINDOWNew(c);
   XCBCreateWindow (c,
		    XCBAuxGetDepth(c, screen),
		    win.window, screen->root,
		    0, 0,
		    win_w, win_h,
		    0,
		    XCBWindowClassInputOutput,
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

   XCBMapWindow (c, win.window);
   /* we sync */
   reply = XCBGetInputFocusReply(c,
                                 XCBGetInputFocusUnchecked(c),
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
      einfo->info.visual = XCBAuxGetVisualtype(c, screen_nbr, screen->root_visual);
      einfo->info.colormap = screen->default_colormap;
      einfo->info.drawable = win;
      einfo->info.depth = XCBAuxGetDepth(c, screen);
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
					     XCBAuxGetVisualtype(c, screen_nbr, screen->root_visual),
					     screen->default_colormap,
					     win,
					     XCBAuxGetDepth(c, screen));
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
