#include "evas_common.h"
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
evas_cserve_stats_get(Evas_Cserve_Stats *stats __UNUSED__)
{
   return 0;
}

EAPI Eina_Bool
evas_cserve_image_cache_contents_get(Evas_Cserve_Image_Cache *cache __UNUSED__)
{
   return 0;
}

EAPI void
evas_cserve_image_cache_contents_clean(Evas_Cserve_Image_Cache *cache __UNUSED__)
{
}

EAPI Eina_Bool
evas_cserve_config_get(Evas_Cserve_Config *config __UNUSED__)
{
   return 0;
}

EAPI Eina_Bool
evas_cserve_config_set(const Evas_Cserve_Config *config __UNUSED__)
{
   return 0;
}

EAPI void
evas_cserve_disconnect(void)
{
}
