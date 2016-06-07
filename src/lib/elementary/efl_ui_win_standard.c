#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define ELM_INTERFACE_ATSPI_ACCESSIBLE_PROTECTED
#define ELM_INTERFACE_ATSPI_WIDGET_ACTION_PROTECTED
#define ELM_WIN_PROTECTED

#include <Elementary.h>

#include "elm_priv.h"

#define MY_CLASS EFL_UI_WIN_STANDARD_CLASS

EOLIAN static Eo *
_efl_ui_win_standard_eo_base_finalize(Eo *obj, void *pd EINA_UNUSED)
{
   obj = eo_finalize(eo_super(obj, MY_CLASS));
   if (!obj)
     return NULL;

   Evas_Object *bg = eo_add(ELM_BG_CLASS, obj);
   if (!bg)
     {
        ERR("Cannot create background.");
        evas_object_del(obj);
        return NULL;
     }
   evas_obj_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   efl_pack(obj, bg);
   efl_gfx_visible_set(bg, EINA_TRUE);

   return obj;
}

#include "efl_ui_win_standard.eo.c"
