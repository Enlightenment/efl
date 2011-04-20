#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

EAPI int
elm_modapi_init(void *m __UNUSED__)
{
   return 1; // succeed always
}

EAPI int
elm_modapi_shutdown(void *m __UNUSED__)
{
   return 1; // succeed always
}

EAPI Eina_Bool
obj_hook(Evas_Object *obj __UNUSED__)
{
   return EINA_TRUE;
}

EAPI Eina_Bool
obj_unhook(Evas_Object *obj __UNUSED__)
{
   return EINA_TRUE;
}

EAPI Eina_Bool
obj_convert_geo_into_coord(const Evas_Object *obj __UNUSED__, int zoom __UNUSED__, double lon __UNUSED__, double lat __UNUSED__, int size __UNUSED__, int *x __UNUSED__, int *y __UNUSED__)
{
   return EINA_FALSE;
}

EAPI Eina_Bool
obj_convert_coord_into_geo(const Evas_Object *obj __UNUSED__, int zoom __UNUSED__, int x __UNUSED__, int y __UNUSED__, int size __UNUSED__, double *lon __UNUSED__, double *lat __UNUSED__)
{
   return EINA_FALSE;
}

EAPI char*
obj_url_request(Evas_Object *obj __UNUSED__, int x, int y, int zoom)
{
   char buf[PATH_MAX];
   snprintf(buf, sizeof(buf), "http://tile.openstreetmap.org/%d/%d/%d.png",
            zoom, x, y);
   return strdup(buf);
}

