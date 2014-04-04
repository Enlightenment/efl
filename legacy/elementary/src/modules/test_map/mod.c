#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include "Elementary.h"
#include <Eina.h>

EAPI Eina_Stringshare *
map_module_source_name_get(void)
{
   return eina_stringshare_add("test_map");
}

EAPI int
map_module_tile_zoom_min_get(void)
{
   return 0;
}

EAPI int
map_module_tile_zoom_max_get(void)
{
   return 18;
}

EAPI char *
map_module_tile_url_get(Evas_Object *obj EINA_UNUSED, int x, int y, int zoom)
{
   char buf[PATH_MAX];
   snprintf(buf, sizeof(buf), "http://tile.openstreetmap.org/%d/%d/%d.png",
            zoom, x, y);
   return strdup(buf);
}

EAPI char *
map_module_route_source_get(void)
{
   return NULL;
}

EAPI char *
map_module_route_url_get(Evas_Object *obj EINA_UNUSED, const char *type_name EINA_UNUSED, int method EINA_UNUSED, double flon EINA_UNUSED, double flat EINA_UNUSED, double tlon EINA_UNUSED, double tlat EINA_UNUSED)
{
   return strdup("");
}

EAPI char *
map_module_name_url_get(Evas_Object *obj EINA_UNUSED, int method EINA_UNUSED, const char *name EINA_UNUSED, double lon EINA_UNUSED, double lat EINA_UNUSED)
{
   return strdup("");
}

EAPI Eina_Bool
map_module_tile_geo_to_coord(const Evas_Object *obj EINA_UNUSED, int zoom EINA_UNUSED, double lon EINA_UNUSED, double lat EINA_UNUSED, int size EINA_UNUSED, int *x EINA_UNUSED, int *y EINA_UNUSED)
{
   return EINA_FALSE;
}

EAPI Eina_Bool
map_module_tile_coord_to_geo(const Evas_Object *obj EINA_UNUSED, int zoom EINA_UNUSED, int x EINA_UNUSED, int y EINA_UNUSED, int size EINA_UNUSED, double *lon EINA_UNUSED, double *lat EINA_UNUSED)
{
   return EINA_FALSE;
}

static Eina_Bool
_module_init(void)
{
   return EINA_TRUE;
}

static void
_module_shutdown(void)
{
}

EINA_MODULE_INIT(_module_init);
EINA_MODULE_SHUTDOWN(_module_shutdown);

