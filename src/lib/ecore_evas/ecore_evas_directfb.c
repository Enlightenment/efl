#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <string.h>

#include <Ecore.h>
#include "ecore_private.h"

#include "ecore_evas_private.h"
#include "Ecore_Evas.h"

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
