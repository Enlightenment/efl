#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define ELM_INTERFACE_ATSPI_ACCESSIBLE_PROTECTED
#define ELM_INTERFACE_ATSPI_WIDGET_ACTION_PROTECTED
#define ELM_WIN_PROTECTED

#include <Elementary.h>

#include "elm_priv.h"

#define MY_CLASS ELM_WIN_STANDARD_CLASS

EOLIAN static Eo *
_elm_win_standard_eo_base_finalize(Eo *obj, void *pd EINA_UNUSED)
{
   eo_do_super(obj, MY_CLASS, obj = eo_finalize());
   if (!obj)
     return NULL;

   Evas_Object *bg = eo_add(ELM_BG_CLASS, obj);
   if (!bg)
     {
        ERR("Cannot create background.");
        evas_object_del(obj);
        return NULL;
     }
   eo_do(bg, evas_obj_size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND));
   eo_do(obj, elm_obj_win_resize_object_add(bg));
   eo_do(bg, efl_gfx_visible_set(EINA_TRUE));

   return obj;
}

#include "elm_win_standard.eo.c"
