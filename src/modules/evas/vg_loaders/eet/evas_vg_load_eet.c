#include "vg_common.h"

static int _evas_vg_loader_eet_log_dom = -1;

#ifdef ERR
# undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_evas_vg_loader_eet_log_dom, __VA_ARGS__)

#ifdef INF
# undef INF
#endif
#define INF(...) EINA_LOG_DOM_INFO(_evas_vg_loader_eet_log_dom, __VA_ARGS__)

static Vg_File_Data*
evas_vg_load_file_open_eet(const char *file, const char *key, int *error EINA_UNUSED)
{
   Eet_Data_Descriptor *svg_node_eet;
   Svg_Node *node;
   Eet_File *ef;

   ef = eet_open(file, EET_FILE_MODE_READ);
   if (!ef)
     {
        *error = EVAS_LOAD_ERROR_CORRUPT_FILE;
        return NULL;
     }

   svg_node_eet = vg_common_svg_node_eet();
   node = eet_data_read(ef, svg_node_eet, key);
   eet_close(ef);

   if (!node)
     {
        *error = EVAS_LOAD_ERROR_GENERIC;
     }
   else
     {
        *error = EVAS_LOAD_ERROR_NONE;
     }

   return vg_common_create_vg_node(node);
}

static Evas_Vg_Load_Func evas_vg_load_eet_func =
{
   evas_vg_load_file_open_eet
};

static int
module_open(Evas_Module *em)
{
   if (!em) return 0;
   em->functions = (void *)(&evas_vg_load_eet_func);
   _evas_vg_loader_eet_log_dom = eina_log_domain_register
     ("vg-load-eet", EVAS_DEFAULT_LOG_COLOR);
   if (_evas_vg_loader_eet_log_dom < 0)
     {
        EINA_LOG_ERR("Can not create a module log domain.");
        return 0;
     }
   return 1;
}

static void
module_close(Evas_Module *em EINA_UNUSED)
{
   if (_evas_vg_loader_eet_log_dom >= 0)
     {
        eina_log_domain_unregister(_evas_vg_loader_eet_log_dom);
        _evas_vg_loader_eet_log_dom = -1;
     }
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

EVAS_MODULE_DEFINE(EVAS_MODULE_TYPE_VG_LOADER, vg_loader, eet);

#ifndef EVAS_STATIC_BUILD_VG_EET
EVAS_EINA_MODULE_DEFINE(vg_loader, eet);
#endif
