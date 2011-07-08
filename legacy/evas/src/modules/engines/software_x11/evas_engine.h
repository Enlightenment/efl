#ifndef EVAS_ENGINE_H
# define EVAS_ENGINE_H

# include <sys/ipc.h>
# include <sys/shm.h>

# ifdef BUILD_ENGINE_SOFTWARE_XLIB
#  include <X11/Xlib.h>
#  include <X11/Xutil.h>
#  include <X11/Xatom.h>
#  include <X11/extensions/XShm.h>
#  include <X11/Xresource.h> // xres - dpi
# endif

# ifdef BUILD_ENGINE_SOFTWARE_XCB
#  include <xcb/xcb.h>
#  include <xcb/shm.h>
#  include <xcb/xcb_image.h>
# endif

extern int _evas_engine_soft_x11_log_dom;

# ifdef ERR
#  undef ERR
# endif
# define ERR(...) EINA_LOG_DOM_ERR(_evas_engine_soft_x11_log_dom, __VA_ARGS__)

# ifdef DBG
#  undef DBG
# endif
# define DBG(...) EINA_LOG_DOM_DBG(_evas_engine_soft_x11_log_dom, __VA_ARGS__)

# ifdef INF
#  undef INF
# endif
# define INF(...) EINA_LOG_DOM_INFO(_evas_engine_soft_x11_log_dom, __VA_ARGS__)

# ifdef WRN
#  undef WRN
# endif
# define WRN(...) EINA_LOG_DOM_WARN(_evas_engine_soft_x11_log_dom, __VA_ARGS__)

# ifdef CRIT
#  undef CRIT
# endif
# define CRIT(...) \
   EINA_LOG_DOM_CRIT(_evas_engine_soft_x11_log_dom, __VA_ARGS__)

typedef enum _Outbuf_Depth Outbuf_Depth;

enum _Outbuf_Depth
{
   OUTBUF_DEPTH_NONE,
   OUTBUF_DEPTH_INHERIT,
   OUTBUF_DEPTH_RGB_16BPP_565_565_DITHERED,
   OUTBUF_DEPTH_RGB_16BPP_555_555_DITHERED,
   OUTBUF_DEPTH_RGB_16BPP_444_444_DITHERED,
   OUTBUF_DEPTH_RGB_16BPP_565_444_DITHERED,
   OUTBUF_DEPTH_RGB_32BPP_888_8888,
   OUTBUF_DEPTH_LAST
};

typedef struct _Outbuf Outbuf;

struct _Outbuf
{
   Outbuf_Depth depth;
   int w, h;
   int rot;
   int onebuf;

   struct 
     {
        Convert_Pal *pal;
        union 
          {
# ifdef BUILD_ENGINE_SOFTWARE_XLIB
             struct 
               {
                  Display *disp;
                  Window win;
                  Pixmap mask;
                  Visual *vis;
                  Colormap cmap;
                  int depth, shm;
                  GC gc, gcm;
                  unsigned char swap : 1;
                  unsigned char bit_swap : 1;
               } xlib;
# endif
# ifdef BUILD_ENGINE_SOFTWARE_XCB
             struct 
               {
                  xcb_connection_t *conn;
                  xcb_screen_t *screen;
                  xcb_window_t win;
                  xcb_pixmap_t mask;
                  xcb_visualtype_t *visual;
                  xcb_colormap_t cmap;
                  int depth, shm;
                  xcb_gcontext_t gc, gcm;
                  unsigned char swap : 1;
                  unsigned char bit_swap : 1;
               } xcb;
# endif
          } x11;
        struct 
          {
             DATA32 r, g, b;
          } mask;

        /* 1 big buffer for updates - flush on idle_flush */
        RGBA_Image *onebuf;
        Eina_List *onebuf_regions;

        /* a list of pending regions to write to the target */
        Eina_List *pending_writes;

        /* a list of previous frame pending regions to write to the target */
        Eina_List *prev_pending_writes;

# ifdef EVAS_FRAME_QUEUING
        /* protecting prev_pending_writes */
        LK(lock);
# endif

        unsigned char mask_dither : 1;
        unsigned char destination_alpha : 1;
        unsigned char debug : 1;
        unsigned char synced : 1;
     } priv;
};

void evas_software_xlib_x_init(void);
void evas_software_xcb_init(void);

#endif
