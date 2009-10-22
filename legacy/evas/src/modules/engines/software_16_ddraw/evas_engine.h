#ifndef __EVAS_ENGINE_H__
#define __EVAS_ENGINE_H__

#include <windows.h>
#include <ddraw.h>

#include "evas_common_soft16.h"

extern int _evas_engine_soft16_ddraw_log_dom ;
#ifdef ERR
# undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_evas_engine_soft16_ddraw_log_dom, __VA_ARGS__)

#ifdef DBG
# undef DBG
#endif
#define DBG(...) EINA_LOG_DOM_DBG(_evas_engine_soft16_ddraw_log_dom, __VA_ARGS__)

#ifdef INF
# undef INF
#endif
#define INF(...) EINA_LOG_DOM_INFO(_evas_engine_soft16_ddraw_log_dom, __VA_ARGS__)

#ifdef WRN
# undef WRN
#endif
#define WRN(...) EINA_LOG_DOM_WARN(_evas_engine_soft16_ddraw_log_dom, __VA_ARGS__)

#ifdef CRIT
# undef CRIT
#endif
#define CRIT(...) EINA_LOG_DOM_CRIT(_evas_engine_soft16_ddraw_log_dom, __VA_ARGS__)

typedef struct _DDraw_Output_Buffer       DDraw_Output_Buffer;

struct _DDraw_Output_Buffer
{
   Soft16_Image *im;
  struct {
     HWND                window;
     LPDIRECTDRAW        object;
     LPDIRECTDRAWSURFACE surface_primary;
     LPDIRECTDRAWSURFACE surface_back;
     LPDIRECTDRAWSURFACE surface_source;
  } dd;
   void        *data;
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
