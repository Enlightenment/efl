#ifndef __EVAS_ENGINE_H__
#define __EVAS_ENGINE_H__

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN
#include "evas_common.h"
#include "evas_common_soft16.h"

extern int _evas_engine_soft16_wince_log_dom;
#ifdef ERR
# undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_evas_engine_soft16_wince_log_dom, __VA_ARGS__)

#ifdef DBG
# undef DBG
#endif
#define DBG(...) EINA_LOG_DOM_DBG(_evas_engine_soft16_wince_log_dom, __VA_ARGS__)

#ifdef INF
# undef INF
#endif
#define INF(...) EINA_LOG_DOM_INFO(_evas_engine_soft16_wince_log_dom, __VA_ARGS__)

#ifdef WRN
# undef WRN
#endif
#define WRN(...) EINA_LOG_DOM_WARN(_evas_engine_soft16_wince_log_dom, __VA_ARGS__)

#ifdef CRIT
# undef CRIT
#endif
#define CRIT(...) EINA_LOG_DOM_CRIT(_evas_engine_soft16_wince_log_dom, __VA_ARGS__)

typedef struct _FB_Output_Buffer FB_Output_Buffer;

struct _FB_Output_Buffer
{
   Soft16_Image *im;
   void         *priv;
};


/* Raw FrameBuffer */

void             *evas_software_wince_fb_init (HWND window,
                                               int  width,
                                               int  height);
FB_Output_Buffer *evas_software_wince_fb_output_buffer_new (void *priv,
                                                            int   width,
                                                            int   height);
void              evas_software_wince_fb_shutdown(void *priv);
void              evas_software_wince_fb_output_buffer_free (FB_Output_Buffer *fbob);
void              evas_software_wince_fb_output_buffer_paste (FB_Output_Buffer *fbob);

void              evas_software_wince_fb_surface_resize(FB_Output_Buffer *fbob);


/* GAPI */

void             *evas_software_wince_gapi_init (HWND window,
                                                 int  width,
                                                 int  height);
FB_Output_Buffer *evas_software_wince_gapi_output_buffer_new (void *priv,
                                                              int   width,
                                                              int   height);
void              evas_software_wince_gapi_shutdown(void *priv);
void              evas_software_wince_gapi_output_buffer_free (FB_Output_Buffer *fbob);
void              evas_software_wince_gapi_output_buffer_paste (FB_Output_Buffer *fbob);

void              evas_software_wince_gapi_surface_resize(FB_Output_Buffer *fbob);

void             *evas_software_wince_gapi_default_keys(void);
int               evas_software_wince_gapi_suspend(void);
int               evas_software_wince_gapi_resume(void);


/* DirectDraw */

#ifdef __cplusplus
extern "C" {
#endif


void             *evas_software_wince_ddraw_init (HWND window,
                                                  int  width,
                                                  int  height);
FB_Output_Buffer *evas_software_wince_ddraw_output_buffer_new (void *priv,
                                                               int   width,
                                                               int   height);
void              evas_software_wince_ddraw_shutdown(void *priv);
void              evas_software_wince_ddraw_output_buffer_free (FB_Output_Buffer *fbob);
void              evas_software_wince_ddraw_output_buffer_paste (FB_Output_Buffer *fbob);

void              evas_software_wince_ddraw_surface_resize(FB_Output_Buffer *fbob);


#ifdef __cplusplus
}
#endif


/* GDI */

void             *evas_software_wince_gdi_init (HWND window,
                                                int  width,
                                                int  height,
                                                int  fullscreen);
FB_Output_Buffer *evas_software_wince_gdi_output_buffer_new (void *priv,
                                                             int   width,
                                                             int   height);
void              evas_software_wince_gdi_shutdown(void *priv);
void              evas_software_wince_gdi_output_buffer_free (FB_Output_Buffer *fbob);
void              evas_software_wince_gdi_output_buffer_paste (FB_Output_Buffer *fbob);

void              evas_software_wince_gdi_surface_resize(FB_Output_Buffer *fbob);


#endif /* __EVAS_ENGINE_H__ */
