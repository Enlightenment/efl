#ifndef _EVAS_ENGINE_SOFTWARE_XCB_H
#define _EVAS_ENGINE_SOFTWARE_XCB_H

#include <X11/XCB/xcb.h>

typedef struct _Evas_Engine_Info_Software_Xcb   Evas_Engine_Info_Software_Xcb;

struct _Evas_Engine_Info_Software_Xcb
{
   /* PRIVATE - don't mess with this baby or evas will poke its tongue out */
   /* at you and make nasty noises */
   Evas_Engine_Info magic;

   /* engine specific data & parameters it needs to set up */
   struct {
      XCBConnection  *conn;
      XCBDRAWABLE     drawable;
      XCBDRAWABLE     mask;
      XCBVISUALTYPE  *visual;
      XCBCOLORMAP     colormap;
      int       depth;
      int       rotation;

      int       alloc_grayscale : 1;
      int       debug : 1;
      int       shape_dither : 1;

      int       alloc_colors_max;
   } info;
   /* engine specific function calls to query stuff about the destination */
   /* engine (what visual & colormap & depth to use, performance info etc. */
   struct {
      XCBVISUALTYPE * (*best_visual_get)   (XCBConnection *conn, int screen);
      XCBCOLORMAP     (*best_colormap_get) (XCBConnection *conn, int screen);
      int             (*best_depth_get)    (XCBConnection *conn, int screen);

      Evas_Performance *(*performance_test)         (Evas          *e,
						     XCBConnection *conn,
						     XCBVISUALTYPE *vis,
						     XCBCOLORMAP    cmap,
						     XCBDRAWABLE    draw,
						     int            depth);
      void              (*performance_free)         (Evas_Performance *perf);
      char *            (*performance_data_get)     (Evas_Performance *perf);
      char *            (*performance_key_get)      (Evas_Performance *perf);
      Evas_Performance *(*performance_new)          (Evas          *e,
						     XCBConnection *conn,
						     XCBVISUALTYPE *vis,
						     XCBCOLORMAP    cmap,
						     XCBDRAWABLE    draw,
						     int            depth);
      void              (*performance_build)        (Evas_Performance *perf,
						     const char       *data);
      void              (*performance_device_store) (Evas_Performance *perf);
   } func;
};

#endif /* _EVAS_ENGINE_SOFTWARE_XCB_H */
