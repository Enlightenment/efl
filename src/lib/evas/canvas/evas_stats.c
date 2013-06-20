#include "evas_common_private.h"
#include "evas_private.h"
//#include "evas_cs.h"

EAPI Eina_Bool
evas_cserve_want_get(void)
{
   return 0;
}

EAPI Eina_Bool
evas_cserve_connected_get(void)
{
   return 0;
}

EAPI Eina_Bool
evas_cserve_stats_get(Evas_Cserve_Stats *stats EINA_UNUSED)
{
   return 0;
}

EAPI Eina_Bool
evas_cserve_image_cache_contents_get(Evas_Cserve_Image_Cache *cache EINA_UNUSED)
{
   return 0;
}

EAPI void
evas_cserve_image_cache_contents_clean(Evas_Cserve_Image_Cache *cache EINA_UNUSED)
{
}

EAPI Eina_Bool
evas_cserve_config_get(Evas_Cserve_Config *config EINA_UNUSED)
{
   return 0;
}

EAPI Eina_Bool
evas_cserve_config_set(const Evas_Cserve_Config *config EINA_UNUSED)
{
   return 0;
}

EAPI void
evas_cserve_disconnect(void)
{
}
