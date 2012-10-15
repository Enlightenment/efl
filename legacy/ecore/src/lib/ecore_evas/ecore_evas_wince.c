#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h> /* for NULL */

#include <Ecore.h>
#include "ecore_private.h"

#include "ecore_evas_private.h"
#include "Ecore_Evas.h"

EAPI Ecore_Evas *
ecore_evas_software_wince_new(Ecore_WinCE_Window *parent __UNUSED__,
                              int                 x __UNUSED__,
                              int                 y __UNUSED__,
                              int                 width __UNUSED__,
                              int                 height __UNUSED__)
{
   return NULL;
}

EAPI Ecore_Evas *
ecore_evas_software_wince_fb_new(Ecore_WinCE_Window *parent __UNUSED__,
                                 int                 x __UNUSED__,
                                 int                 y __UNUSED__,
                                 int                 width __UNUSED__,
                                 int                 height __UNUSED__)
{
   return NULL;
}

EAPI Ecore_Evas *
ecore_evas_software_wince_gapi_new(Ecore_WinCE_Window *parent __UNUSED__,
                                   int                 x __UNUSED__,
                                   int                 y __UNUSED__,
                                   int                 width __UNUSED__,
                                   int                 height __UNUSED__)
{
   return NULL;
}

EAPI Ecore_Evas *
ecore_evas_software_wince_ddraw_new(Ecore_WinCE_Window *parent __UNUSED__,
                                    int                 x __UNUSED__,
                                    int                 y __UNUSED__,
                                    int                 width __UNUSED__,
                                    int                 height __UNUSED__)
{
   return NULL;
}

EAPI Ecore_Evas *
ecore_evas_software_wince_gdi_new(Ecore_WinCE_Window *parent __UNUSED__,
                                  int                 x __UNUSED__,
                                  int                 y __UNUSED__,
                                  int                 width __UNUSED__,
                                  int                 height __UNUSED__)
{
   return NULL;
}

EAPI Ecore_WinCE_Window *
ecore_evas_software_wince_window_get(const Ecore_Evas *ee __UNUSED__)
{
   return NULL;
}
