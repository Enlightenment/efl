#include "vg_common.h"

static int _evas_vg_saver_svg_log_dom = -1;

#ifdef ERR
# undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_evas_vg_saver_eet_log_dom, __VA_ARGS__)

#ifdef INF
# undef INF
#endif
#define INF(...) EINA_LOG_DOM_INFO(_evas_vg_saver_eet_log_dom, __VA_ARGS__)

int
evas_vg_save_file_svg(Vg_File_Data *evg_data, const char *file, const char *key EINA_UNUSED, int compress EINA_UNUSED)
{
   Svg_Node *root EINA_UNUSED;
   Eet_File *ef;

   ef = eet_open(file, EET_FILE_MODE_WRITE);
   if (!ef)
     return EVAS_LOAD_ERROR_GENERIC;

   root = vg_common_create_svg_node(evg_data);
   //TODO: parse root into SVG source code and save into file as text file
   eet_close(ef);

   return EVAS_LOAD_ERROR_NONE;
}

static Evas_Vg_Save_Func evas_vg_save_svg_func =
{
   evas_vg_save_file_svg
};

static int
module_open(Evas_Module *em)
{
   if (!em) return 0;
   em->functions = (void *)(&evas_vg_save_svg_func);
   _evas_vg_saver_svg_log_dom = eina_log_domain_register
     ("vg-save-svg", EVAS_DEFAULT_LOG_COLOR);
   if (_evas_vg_saver_svg_log_dom < 0)
     {
        EINA_LOG_ERR("Can not create a module log domain.");
        return 0;
     }
   return 1;
}

static void
module_close(Evas_Module *em EINA_UNUSED)
{
}

static Evas_Module_Api evas_modapi =
{
   EVAS_MODULE_API_VERSION,
   "svg",
   "none",
   {
     module_open,
     module_close
   }
};

EVAS_MODULE_DEFINE(EVAS_MODULE_TYPE_VG_SAVER, vg_saver, svg);

#ifndef EVAS_STATIC_BUILD_VG_SVG
EVAS_EINA_MODULE_DEFINE(vg_saver, svg);
#endif
