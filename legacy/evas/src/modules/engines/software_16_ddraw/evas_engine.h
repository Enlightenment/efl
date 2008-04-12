#ifndef __EVAS_ENGINE_H__
#define __EVAS_ENGINE_H__


#include <windows.h>
#include <ddraw.h>

#include "evas_common_soft16.h"

typedef struct _DDraw_Output_Buffer       DDraw_Output_Buffer;

struct _DDraw_Output_Buffer
{
   Soft16_Image im;
  struct {
     HWND                window;
     LPDIRECTDRAW        object;
     LPDIRECTDRAWSURFACE surface_primary;
     LPDIRECTDRAWSURFACE surface_back;
     LPDIRECTDRAWSURFACE surface_source;
  } dd;
   int          x;
   int          y;
   int          width;
   int          height;
   int          depth;
   int          pitch;
};

/****/

#ifdef __cplusplus
extern "C" {
#endif


DDraw_Output_Buffer *evas_software_ddraw_output_buffer_new        (HWND                window,
                                                                   LPDIRECTDRAW        object,
                                                                   LPDIRECTDRAWSURFACE surface_primary,
                                                                   LPDIRECTDRAWSURFACE surface_back,
                                                                   LPDIRECTDRAWSURFACE surface_source,
                                                                   int width,
                                                                   int height);
void                 evas_software_ddraw_output_buffer_free       (DDraw_Output_Buffer *ddob, int sync);
void                 evas_software_ddraw_output_buffer_paste      (DDraw_Output_Buffer *ddob);


void *evas_software_ddraw_lock(DDraw_Output_Buffer *ddob, int *ddraw_width, int *ddraw_height, int *ddraw_pitch, int *ddraw_depth);

void  evas_software_ddraw_unlock_and_flip(DDraw_Output_Buffer *ddob);

void  evas_software_ddraw_surface_resize(DDraw_Output_Buffer *ddob);


#ifdef __cplusplus
}
#endif

#endif /* __EVAS_ENGINE_H__ */
