#ifndef __EVAS_ENGINE_H__
#define __EVAS_ENGINE_H__


#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN
#include <ddraw.h>

typedef struct _Outbuf                Outbuf;
typedef struct _Outbuf_Region         Outbuf_Region;
typedef struct _DD_Output_Buffer      DD_Output_Buffer;

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

typedef enum   _Outbuf_Depth          Outbuf_Depth;

struct _Outbuf
{
   Outbuf_Depth              depth;
   int                       width;
   int                       height;
   int                       rot;
   int                       onebuf;

   struct {
      Convert_Pal           *pal;
      struct {
         HWND                window;
         LPDIRECTDRAW        object;
         LPDIRECTDRAWSURFACE surface_primary;
         LPDIRECTDRAWSURFACE surface_back;
         LPDIRECTDRAWCLIPPER clipper;
         int                 depth;
         unsigned char       swap : 1;
         unsigned char       bit_swap : 1;
      } dd;
      struct {
         DATA32              r, g, b;
      } mask;

      /* 1 big buffer for updates - flush on idle_flush */
      RGBA_Image            *onebuf;
      Eina_List             *onebuf_regions;

      /* a list of pending regions to write to the target */
      Eina_List             *pending_writes;
      /* a list of previous frame pending regions to write to the target */
      Eina_List             *prev_pending_writes;

      unsigned char          mask_dither : 1;
      unsigned char          destination_alpha : 1;
      unsigned char          debug : 1;
      unsigned char          synced : 1;
   } priv;
};

struct _Outbuf_Region
{
   DD_Output_Buffer *ddob;
   int               x;
   int               y;
   int               width;
   int               height;
};

struct _DD_Output_Buffer
{
   void *data;
   int   width;
   int   height;
   int   depth;
   int   pitch;
/*    int              w, h, bpl; */
   int              psize;
};


/* evas_outbuf.c */

void evas_software_ddraw_outbuf_init(void);

void evas_software_ddraw_outbuf_free(Outbuf *buf);

Outbuf *evas_software_ddraw_outbuf_setup(int          width,
                                         int          height,
                                         int          rotation,
                                         Outbuf_Depth depth,
                                         HWND         window,
                                         int          w_depth);

void evas_software_ddraw_outbuf_reconfigure(Outbuf      *buf,
                                            int          width,
                                            int          height,
                                            int          rotation,
                                            Outbuf_Depth depth);

RGBA_Image *evas_software_ddraw_outbuf_new_region_for_update(Outbuf *buf,
                                                             int     x,
                                                             int     y,
                                                             int     w,
                                                             int     h,
                                                             int    *cx,
                                                             int    *cy,
                                                             int    *cw,
                                                             int    *ch);

void evas_software_ddraw_outbuf_push_updated_region(Outbuf     *buf,
                                                    RGBA_Image *update,
                                                    int        x,
                                                    int        y,
                                                    int        w,
                                                    int        h);

void evas_software_ddraw_outbuf_free_region_for_update(Outbuf     *buf,
                                                       RGBA_Image *update);

void evas_software_ddraw_outbuf_flush(Outbuf *buf);

void evas_software_ddraw_outbuf_idle_flush(Outbuf *buf);

int evas_software_ddraw_outbuf_width_get(Outbuf *buf);

int evas_software_ddraw_outbuf_height_get(Outbuf *buf);

Outbuf_Depth evas_software_ddraw_outbuf_depth_get(Outbuf *buf);

int evas_software_ddraw_outbuf_rot_get(Outbuf *buf);

/* evas_ddraw_buffer.c */

DD_Output_Buffer *evas_software_ddraw_output_buffer_new(int   depth,
                                                        int   width,
                                                        int   height,
                                                        void *data);

void evas_software_ddraw_output_buffer_free(DD_Output_Buffer *ddob);

void evas_software_ddraw_output_buffer_paste(DD_Output_Buffer *ddob,
                                             void             *ddraw_data,
                                             int               ddraw_width,
                                             int               ddraw_height,
                                             int               ddraw_pitch,
                                             int               ddraw_depth,
                                             int               x,
                                             int               y);

DATA8 *evas_software_ddraw_output_buffer_data(DD_Output_Buffer *ddob,
                                              int              *bytes_per_line_ret);

int evas_software_ddraw_output_buffer_depth(DD_Output_Buffer *ddob);

/* evas_ddraw_main.cpp */

#ifdef __cplusplus
extern "C" {
#endif

int evas_software_ddraw_init (HWND    window,
                              int     depth,
                              Outbuf *buf);

void evas_software_ddraw_shutdown(Outbuf *buf);

int evas_software_ddraw_masks_get(Outbuf *buf);

void *evas_software_ddraw_lock(Outbuf *buf,
                               int    *ddraw_width,
                               int    *ddraw_height,
                               int    *ddraw_pitch,
                               int    *ddraw_depth);

void evas_software_ddraw_unlock_and_flip(Outbuf *buf);

void evas_software_ddraw_surface_resize(Outbuf *buf);

#ifdef __cplusplus
}
#endif


#endif /* __EVAS_ENGINE_H__ */
