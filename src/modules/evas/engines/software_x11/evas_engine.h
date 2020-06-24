#ifndef EVAS_ENGINE_H
# define EVAS_ENGINE_H

# include <sys/ipc.h>
# include <sys/shm.h>

# include <X11/Xlib.h>
# include <X11/Xutil.h>
# include <X11/Xatom.h>
# include <X11/extensions/XShm.h>
# include <X11/Xresource.h> // xres - dpi

#include "../software_generic/Evas_Engine_Software_Generic.h"

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

# ifdef CRI
#  undef CRI
# endif
# define CRI(...) \
   EINA_LOG_DOM_CRIT(_evas_engine_soft_x11_log_dom, __VA_ARGS__)

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
             struct
               {
                  Display *disp;
                  Window win;
                  Pixmap mask;
                  Visual *vis;
                  Colormap cmap;
                  int depth, imdepth, shm;
                  GC gc, gcm;
                  unsigned char swap : 1;
                  unsigned char bit_swap : 1;
               } xlib;
          } x11;
        struct
          {
             DATA32 r, g, b;
          } mask;

        /* 1 big buffer for updates - flush on idle_flush */
        RGBA_Image *onebuf;
        Eina_Array  onebuf_regions;

        void *swapper;

        /* a list of pending regions to write to the target */
        Eina_List *pending_writes;

        /* a list of previous frame pending regions to write to the target */
        Eina_List *prev_pending_writes;
        Eina_Spinlock lock;

        unsigned char mask_dither : 1;
        unsigned char destination_alpha : 1;
        unsigned char debug : 1;
        unsigned char synced : 1;
     } priv;
};

void evas_software_xlib_x_init(void);

#endif
