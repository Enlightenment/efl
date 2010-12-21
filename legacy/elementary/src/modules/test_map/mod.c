#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

EAPI int 
elm_modapi_init(void *m) 
{   
   return 1; // succeed always
}

EAPI int 
elm_modapi_shutdown(void *m) 
{
   return 1; // succeed always
}

EAPI Eina_Bool
obj_hook(Evas_Object *obj)
{
   return EINA_TRUE;
}

EAPI Eina_Bool
obj_unhook(Evas_Object *obj)
{ 
   return EINA_TRUE;
}

EAPI Eina_Bool 
obj_convert_geo_into_coord(const Evas_Object *obj, int zoom, double lon, double lat, int size, int *x, int *y)
{
   return EINA_FALSE;
}

EAPI Eina_Bool
obj_convert_coord_into_geo(const Evas_Object *obj, int zoom, int x, int y, int size, double *lon, double *lat)
{
   return EINA_FALSE;
}

EAPI char*
obj_url_request(Evas_Object *obj, int x, int y, int zoom)
{
   char buf[PATH_MAX];
   snprintf(buf, sizeof(buf), "http://tile.openstreetmap.org/%d/%d/%d.png",
            zoom, x, y);
   return strdup(buf);
}

