#ifndef __EVAS_ENGINE_H__
#define __EVAS_ENGINE_H__


#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>

#include "evas_common.h"


typedef struct _Outbuf                   Outbuf;
typedef struct _Direct3D_Output_Buffer   Direct3D_Output_Buffer;


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
typedef enum   _Outbuf_Depth             Outbuf_Depth;


struct _Outbuf
{
   int             width;
   int             height;
   int             rot;
   Outbuf_Depth    depth;

   struct {
      struct {
         HWND                window;
         LPDIRECT3D9         object;
         LPDIRECT3DDEVICE9   device;
         LPD3DXSPRITE        sprite;
         LPDIRECT3DTEXTURE9  texture;
         int                 depth;
      } d3d;
      struct {
         DATA32    r, g, b;
      } mask;

      /* a list of pending regions to write to the target */
      Eina_List   *pending_writes;
   } priv;
};

struct _Direct3D_Output_Buffer
{
   void *image;
   int   x;
   int   y;
   int   width;
   int   height;
   int   depth;
   int   pitch;
};


#ifdef __cplusplus
extern "C" {
#endif


/* Outbuf functions */
void         evas_direct3d_outbuf_init(void);
void         evas_direct3d_outbuf_free(Outbuf *buf);
Outbuf      *evas_direct3d_outbuf_setup_d3d(int                width,
                                            int                height,
                                            int                rotation,
                                            Outbuf_Depth       depth,
                                            HWND               window,
                                            LPDIRECT3D9        object,
                                            LPDIRECT3DDEVICE9  device,
                                            LPD3DXSPRITE       sprite,
                                            LPDIRECT3DTEXTURE9 texture,
                                            int                w_depth);
RGBA_Image  *evas_direct3d_outbuf_new_region_for_update(Outbuf *buf,
                                                        int x,
                                                        int y,
                                                        int width,
                                                        int height,
                                                        int *cx,
                                                        int *cy,
                                                        int *cw,
                                                        int *ch);
void         evas_direct3d_outbuf_free_region_for_update(Outbuf     *buf,
                                                         RGBA_Image *update);
void         evas_direct3d_outbuf_flush(Outbuf *buf);
void         evas_direct3d_outbuf_push_updated_region(Outbuf     *buf,
                                                      RGBA_Image *update,
                                                      int         x,
                                                      int         y,
                                                      int         width,
                                                      int         height);
void         evas_direct3d_outbuf_reconfigure(Outbuf      *buf,
                                              int          width,
                                              int          height,
                                              int          rotation,
                                              Outbuf_Depth depth);
int          evas_direct3d_outbuf_width_get(Outbuf *buf);
int          evas_direct3d_outbuf_height_get(Outbuf *buf);
Outbuf_Depth evas_direct3d_outbuf_depth_get(Outbuf *buf);
int          evas_direct3d_outbuf_rot_get(Outbuf *buf);

/* Output Buffer functions */
Direct3D_Output_Buffer *evas_direct3d_output_buffer_new(int   depth,
                                                        int   width,
                                                        int   height,
                                                        void *data);
void   evas_direct3d_output_buffer_free(Direct3D_Output_Buffer *d3dob);
void   evas_direct3d_output_buffer_paste(Direct3D_Output_Buffer *d3dob,
                                         DATA8                  *d3d_data,
                                         int                     d3d_width,
                                         int                     d3d_height,
                                         int                     d3d_pitch,
                                         int                     x,
                                         int                     y);
DATA8 *evas_direct3d_output_buffer_data(Direct3D_Output_Buffer *d3dob,
                                        int                    *bytes_per_line_ret);
int    evas_direct3d_output_buffer_depth(Direct3D_Output_Buffer *d3dob);


int    evas_direct3d_masks_get(Outbuf *buf);
void  *evas_direct3d_lock(Outbuf *buf, int *d3d_width, int *d3d_height, int *d3d_pitch);
void   evas_direct3d_unlock(Outbuf *buf);


#ifdef __cplusplus
}
#endif


#endif /* __EVAS_ENGINE_H__ */
