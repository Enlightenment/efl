#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "evas_common_private.h"
#include "evas_private.h"

static int _evas_vg_saver_eet_log_dom = -1;

#ifdef ERR
# undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_evas_vg_saver_eet_log_dom, __VA_ARGS__)

#ifdef INF
# undef INF
#endif
#define INF(...) EINA_LOG_DOM_INFO(_evas_vg_saver_eet_log_dom, __VA_ARGS__)

int
evas_vg_save_file_eet(void *vg EINA_UNUSED, const char *file EINA_UNUSED, const char *key EINA_UNUSED, int compress EINA_UNUSED)
{
   INF("No Implementation Yet");
   return 0;
}

static Evas_Vg_Save_Func evas_vg_save_eet_func =
{
   evas_vg_save_file_eet
};

static int
module_open(Evas_Module *em)
{
   if (!em) return 0;
   em->functions = (void *)(&evas_vg_save_eet_func);
   return 1;
}

static void
module_close(Evas_Module *em EINA_UNUSED)
{
}

static Evas_Module_Api evas_modapi =
{
   EVAS_MODULE_API_VERSION,
   "eet",
   "none",
   {
     module_open,
     module_close
   }
};

EVAS_MODULE_DEFINE(EVAS_MODULE_TYPE_VG_LOADER, vg_saver, eet);

#ifndef EVAS_STATIC_BUILD_VG_EET
EVAS_EINA_MODULE_DEFINE(vg_saver, eet);
#endif