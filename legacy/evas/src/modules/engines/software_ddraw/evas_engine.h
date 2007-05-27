#ifndef __EVAS_ENGINE_H__
#define __EVAS_ENGINE_H__


#include <windows.h>
#include <ddraw.h>

#include "evas_common.h"

typedef struct _Outbuf                Outbuf;
typedef struct _DDraw_Output_Buffer   DDraw_Output_Buffer;


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
   int             width;
   int             height;
   int             rot;
   Outbuf_Depth    depth;

   struct {
      struct {
         HWND                window;
         LPDIRECTDRAW        object;
         LPDIRECTDRAWSURFACE surface_primary;
         LPDIRECTDRAWSURFACE surface_back;
	 int                 depth;
      } dd;
      struct {
	 DATA32    r, g, b;
      } mask;

      /* a list of pending regions to write to the target */
      Evas_List   *pending_writes;
   } priv;
};

struct _DDraw_Output_Buffer
{
   void *image;
   int   x;
   int   y;
   int   width;
   int   height;
   int   depth;
   int   pitch;
};


/* Outbuf functions */
void         evas_software_ddraw_outbuf_init(void);
void         evas_software_ddraw_outbuf_free(Outbuf *buf);
Outbuf      *evas_software_ddraw_outbuf_setup_dd(int                 width,
						 int                 height,
						 int                 rotation,
						 Outbuf_Depth        depth,
						 HWND                window,
						 LPDIRECTDRAW        object,
						 LPDIRECTDRAWSURFACE surface_primary,
						 LPDIRECTDRAWSURFACE surface_back,
						 int                 w_depth);
RGBA_Image  *evas_software_ddraw_outbuf_new_region_for_update(Outbuf *buf,
							      int x,
							      int y,
							      int width,
							      int height,
							      int *cx,
							      int *cy,
							      int *cw,
							      int *ch);
void         evas_software_ddraw_outbuf_free_region_for_update(Outbuf     *buf,
							       RGBA_Image *update);
void         evas_software_ddraw_outbuf_push_updated_region(Outbuf     *buf,
							    RGBA_Image *update,
							    int         x,
							    int         y,
							    int         width,
							    int         height);
void         evas_software_ddraw_outbuf_reconfigure(Outbuf      *buf,
						    int          width,
						    int          height,
						    int          rotation,
						    Outbuf_Depth depth);
int          evas_software_ddraw_outbuf_width_get(Outbuf *buf);
int          evas_software_ddraw_outbuf_height_get(Outbuf *buf);
Outbuf_Depth evas_software_ddraw_outbuf_depth_get(Outbuf *buf);
int          evas_software_ddraw_outbuf_rot_get(Outbuf *buf);

/* Output Buffer functions */
DDraw_Output_Buffer *evas_software_ddraw_output_buffer_new(int   depth,
							   int   width,
							   int   height,
							   void *data);
void   evas_software_ddraw_output_buffer_free(DDraw_Output_Buffer *ddob);
void   evas_software_ddraw_output_buffer_paste(DDraw_Output_Buffer *ddob,
					       DDSURFACEDESC2      *surface_desc,
					       int                  x,
					       int                  y);
DATA8 *evas_software_ddraw_output_buffer_data(DDraw_Output_Buffer *ddob,
					      int                 *bytes_per_line_ret);
int    evas_software_ddraw_output_buffer_depth(DDraw_Output_Buffer *ddob);


#endif /* __EVAS_ENGINE_H__ */
