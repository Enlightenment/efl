#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h> /* for NULL */

#include <Ecore.h>
#include "ecore_private.h"

#include "ecore_evas_private.h"
#include "Ecore_Evas.h"

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

EAPI Ecore_WinCE_Window *
ecore_evas_software_wince_window_get(const Ecore_Evas *ee EINA_UNUSED)
{
   return NULL;
}
