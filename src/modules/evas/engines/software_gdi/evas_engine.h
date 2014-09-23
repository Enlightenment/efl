#ifndef EVAS_ENGINE_H
#define EVAS_ENGINE_H


#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN

#include "../software_generic/Evas_Engine_Software_Generic.h"

extern int _evas_engine_soft_gdi_log_dom;

#ifdef ERR
# undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_evas_engine_soft_gdi_log_dom, __VA_ARGS__)

#ifdef DBG
# undef DBG
#endif
#define DBG(...) EINA_LOG_DOM_DBG(_evas_engine_soft_gdi_log_dom, __VA_ARGS__)

#ifdef INF
# undef INF
#endif
#define INF(...) EINA_LOG_DOM_INFO(_evas_engine_soft_gdi_log_dom, __VA_ARGS__)

#ifdef WRN
# undef WRN
#endif
#define WRN(...) EINA_LOG_DOM_WARN(_evas_engine_soft_gdi_log_dom, __VA_ARGS__)

#ifdef CRI
# undef CRI
#endif
#define CRI(...) EINA_LOG_DOM_CRIT(_evas_engine_soft_gdi_log_dom, __VA_ARGS__)

typedef struct BITMAPINFO_GDI     BITMAPINFO_GDI;
typedef struct _Outbuf_Region     Outbuf_Region;
typedef struct _Gdi_Output_Buffer Gdi_Output_Buffer;

struct BITMAPINFO_GDI
{
   BITMAPINFOHEADER bih;
   DWORD            masks[3];
};

struct _Outbuf
{
   Outbuf_Depth          depth;
   int                   width;
   int                   height;
   int                   rot;
   int                   onebuf;

   struct {
      Convert_Pal       *pal;
      struct {
         BITMAPINFO_GDI *bitmap_info;
         HWND            window;
         HDC             dc;
         HRGN            regions;
         int             depth;
         unsigned char   borderless : 1;
         unsigned char   fullscreen : 1;
         unsigned char   region     : 1;
      } gdi;

      /* 1 big buffer for updates - flush on idle_flush */
      RGBA_Image        *onebuf;
      Eina_List         *onebuf_regions;

      /* a list of pending regions to write to the target */
      Eina_List         *pending_writes;
      /* a list of previous frame pending regions to write to the target */
      Eina_List         *prev_pending_writes;

      unsigned char      mask_dither       : 1;
      unsigned char      destination_alpha : 1;
      unsigned char      debug             : 1;
      unsigned char      synced            : 1;

      unsigned char      region_built      : 1;
   } priv;
};

struct _Outbuf_Region
{
   Gdi_Output_Buffer *gdiob;
   int                x;
   int                y;
   int                width;
   int                height;
};

struct _Gdi_Output_Buffer
{
   BITMAPINFO_GDI *bitmap_info;
   HBITMAP         bitmap;
   HDC             dc;
   int             width;
   int             height;
   void           *data;
   int             depth;
   int             pitch;
   int             psize;
};

/* evas_gdi_main.c */

int evas_software_gdi_init (HWND         window,
                            int          depth,
                            unsigned int borderless,
                            unsigned int fullscreen,
                            unsigned int region,
                            Outbuf      *buf);

void evas_software_gdi_shutdown(Outbuf *buf);

void evas_software_gdi_bitmap_resize(Outbuf *buf);

/* evas_gdi_buffer.c */

Gdi_Output_Buffer *evas_software_gdi_output_buffer_new(HDC             dc,
                                                       BITMAPINFO_GDI *bitmap_info,
                                                       int             depth,
                                                       int             width,
                                                       int             height,
                                                       void           *data);

void evas_software_gdi_output_buffer_free(Gdi_Output_Buffer *gdiob);

void evas_software_gdi_output_buffer_paste(Gdi_Output_Buffer *gdiob,
                                           int                x,
                                           int                y);

DATA8 *evas_software_gdi_output_buffer_data(Gdi_Output_Buffer *gdiob,
                                            int               *pitch);

int evas_software_gdi_output_buffer_depth(Gdi_Output_Buffer *gdiob);

/* evas_outbuf.c */

void evas_software_gdi_outbuf_init(void);

void evas_software_gdi_outbuf_free(Outbuf *buf);

Outbuf *evas_software_gdi_outbuf_setup(int          width,
                                       int          height,
                                       int          rotation,
                                       Outbuf_Depth depth,
                                       HWND         window,
                                       int          w_depth,
                                       unsigned int borderless,
                                       unsigned int fullscreen,
                                       unsigned int region,
                                       int          mask_dither,
                                       int          destination_alpha);

void evas_software_gdi_outbuf_reconfigure(Outbuf      *buf,
                                          int          width,
                                          int          height,
                                          int          rotation,
                                          Outbuf_Depth depth);

void *evas_software_gdi_outbuf_new_region_for_update(Outbuf *buf,
                                                     int     x,
                                                     int     y,
                                                     int     w,
                                                     int     h,
                                                     int    *cx,
                                                     int    *cy,
                                                     int    *cw,
                                                     int    *ch);

void evas_software_gdi_outbuf_push_updated_region(Outbuf     *buf,
                                                  RGBA_Image *update,
                                                  int         x,
                                                  int         y,
                                                  int         w,
                                                  int         h);

void evas_software_gdi_outbuf_free_region_for_update(Outbuf     *buf,
                                                     RGBA_Image *update);

void evas_software_gdi_outbuf_flush(Outbuf *buf, Tilebuf_Rect *rects, Evas_Render_Mode render_mode);

void evas_software_gdi_outbuf_idle_flush(Outbuf *buf);

int evas_software_gdi_outbuf_width_get(Outbuf *buf);

int evas_software_gdi_outbuf_height_get(Outbuf *buf);

Outbuf_Depth evas_software_gdi_outbuf_depth_get(Outbuf *buf);

int evas_software_gdi_outbuf_rot_get(Outbuf *buf);


#endif /* EVAS_ENGINE_H */
