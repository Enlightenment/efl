#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h> /* for NULL */

#include <Ecore.h>
#include "ecore_private.h"

#include "Ecore_Evas.h"
#include "ecore_evas_private.h"


/* Ecore_Evas WinCE support was removed. However we keep the functions
 * to not break ABI.
 */

EAPI Ecore_Evas *
ecore_evas_software_wince_new(Ecore_WinCE_Window *parent EINA_UNUSED,
                              int                 x EINA_UNUSED,
                              int                 y EINA_UNUSED,
                              int                 width EINA_UNUSED,
                              int                 height EINA_UNUSED)
{
   return NULL;
}

EAPI Ecore_Evas *
ecore_evas_software_wince_fb_new(Ecore_WinCE_Window *parent EINA_UNUSED,
                                 int                 x EINA_UNUSED,
                                 int                 y EINA_UNUSED,
                                 int                 width EINA_UNUSED,
                                 int                 height EINA_UNUSED)
{
   return NULL;
}

EAPI Ecore_Evas *
ecore_evas_software_wince_gapi_new(Ecore_WinCE_Window *parent EINA_UNUSED,
                                   int                 x EINA_UNUSED,
                                   int                 y EINA_UNUSED,
                                   int                 width EINA_UNUSED,
                                   int                 height EINA_UNUSED)
{
   return NULL;
}

EAPI Ecore_Evas *
ecore_evas_software_wince_ddraw_new(Ecore_WinCE_Window *parent EINA_UNUSED,
                                    int                 x EINA_UNUSED,
                                    int                 y EINA_UNUSED,
                                    int                 width EINA_UNUSED,
                                    int                 height EINA_UNUSED)
{
   return NULL;
}

EAPI Ecore_Evas *
ecore_evas_software_wince_gdi_new(Ecore_WinCE_Window *parent EINA_UNUSED,
                                  int                 x EINA_UNUSED,
                                  int                 y EINA_UNUSED,
                                  int                 width EINA_UNUSED,
                                  int                 height EINA_UNUSED)
{
   return NULL;
}

EAPI Ecore_Evas *
ecore_evas_direct3d_new(Ecore_Win32_Window *parent EINA_UNUSED,
			int                 x EINA_UNUSED,
			int                 y EINA_UNUSED,
			int                 width EINA_UNUSED,
			int                 height EINA_UNUSED)
{
   return NULL;
}

EAPI Ecore_Evas *
ecore_evas_gl_glew_new(Ecore_Win32_Window *parent EINA_UNUSED,
		       int                 x EINA_UNUSED,
		       int                 y EINA_UNUSED,
		       int                 width EINA_UNUSED,
		       int                 height EINA_UNUSED)
{
   return NULL;
}

EAPI Ecore_WinCE_Window *
ecore_evas_software_wince_window_get(const Ecore_Evas *ee EINA_UNUSED)
{
   return NULL;
}

/* Ecore_Evas DirectFB support was removed. However we keep the functions
 * to not break ABI.
 */

EAPI Ecore_Evas *
ecore_evas_directfb_new(const char *disp_name EINA_UNUSED, int windowed EINA_UNUSED, int x EINA_UNUSED, int y EINA_UNUSED, int w EINA_UNUSED, int h EINA_UNUSED)
{
   return NULL;
}

EAPI Ecore_DirectFB_Window *
ecore_evas_directfb_window_get(const Ecore_Evas *ee EINA_UNUSED)
{
  return NULL;
}

/* Ecore_Evas X11 16 bits support was removed. However we keep the functions
 * to not break ABI.
 */
EAPI Ecore_Evas *
ecore_evas_software_x11_16_new(const char *disp_name EINA_UNUSED, Ecore_X_Window parent EINA_UNUSED,
                               int x EINA_UNUSED, int y EINA_UNUSED, int w EINA_UNUSED, int h EINA_UNUSED)
{
   return NULL;
}

EAPI Ecore_X_Window
ecore_evas_software_x11_16_window_get(const Ecore_Evas *ee EINA_UNUSED)
{
   return 0;
}

EAPI void
ecore_evas_software_x11_16_direct_resize_set(Ecore_Evas *ee EINA_UNUSED, Eina_Bool on EINA_UNUSED)
{
}

EAPI Eina_Bool
ecore_evas_software_x11_16_direct_resize_get(const Ecore_Evas *ee EINA_UNUSED)
{
   return EINA_FALSE;
}

EAPI void
ecore_evas_software_x11_16_extra_event_window_add(Ecore_Evas *ee EINA_UNUSED, Ecore_X_Window win EINA_UNUSED)
{
}

/* Ecore_Evas X11 8 bits support was removed. However we keep the functions
 * to not break ABI.
 */
EAPI Ecore_Evas *
ecore_evas_software_x11_8_new(const char *disp_name EINA_UNUSED, Ecore_X_Window parent EINA_UNUSED, int x EINA_UNUSED, int y EINA_UNUSED, int w EINA_UNUSED, int h EINA_UNUSED)
{
   return NULL;
}

EAPI Ecore_X_Window
ecore_evas_software_x11_8_window_get(const Ecore_Evas *ee EINA_UNUSED)
{
   return 0;
}

EAPI Ecore_X_Window
ecore_evas_software_x11_8_subwindow_get(const Ecore_Evas *ee EINA_UNUSED)
{
   return 0;
}

EAPI void
ecore_evas_software_x11_8_direct_resize_set(Ecore_Evas *ee EINA_UNUSED, Eina_Bool on EINA_UNUSED)
{
}

EAPI Eina_Bool
ecore_evas_software_x11_8_direct_resize_get(const Ecore_Evas *ee EINA_UNUSED)
{
   return EINA_FALSE;
}

EAPI void
ecore_evas_software_x11_8_extra_event_window_add(Ecore_Evas *ee EINA_UNUSED, Ecore_X_Window win EINA_UNUSED)
{
   return;
}

/* Ecore_Evas XRender support was removed. However we keep the functions
 * to not break ABI.
 */
EAPI Ecore_Evas *
ecore_evas_xrender_x11_new(const char *disp_name EINA_UNUSED, Ecore_X_Window parent EINA_UNUSED,
                           int x EINA_UNUSED, int y EINA_UNUSED, int w EINA_UNUSED, int h EINA_UNUSED)
{
   return NULL;
}

EAPI Ecore_X_Window
ecore_evas_xrender_x11_window_get(const Ecore_Evas *ee EINA_UNUSED)
{
   return 0;
}

EAPI void
ecore_evas_xrender_x11_direct_resize_set(Ecore_Evas *ee EINA_UNUSED, Eina_Bool on EINA_UNUSED)
{
}

EAPI Eina_Bool
ecore_evas_xrender_x11_direct_resize_get(const Ecore_Evas *ee EINA_UNUSED)
{
   return 0;
}

EAPI void
ecore_evas_xrender_x11_extra_event_window_add(Ecore_Evas *ee EINA_UNUSED, Ecore_X_Window win EINA_UNUSED)
{
}
