#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>

#include "elm_priv.h"
#include "elm_part_helper.h"

#define MY_CLASS      EFL_UI_TAB_BAR_DEFAULT_ITEM_CLASS

typedef struct {
   Efl_Gfx_Image *icon;
   const char *icon_name;
} Efl_Ui_Tab_Bar_Default_Item_Data;

static void
_tab_icon_set_cb(void *data,
                 Eo *obj,
                 const char *emission,
                 const char *source)
{
   Efl_Ui_Item *ti = data;

   Efl_Ui_Image *new_img = efl_content_get(efl_part(obj, "efl.icon_new"));
   efl_content_set(efl_part(obj, "efl.icon"), new_img); //this must be efl.icon here as obj is the edje object

   efl_layout_signal_callback_del(obj, emission, source, ti, _tab_icon_set_cb, NULL);
   efl_layout_signal_emit(obj, "efl,state,icon,reset", "efl");
}

EOLIAN static void
_efl_ui_tab_bar_default_item_icon_set(Eo *obj, Efl_Ui_Tab_Bar_Default_Item_Data *pd, const char *standard_name)
{
   eina_stringshare_replace(&pd->icon_name, standard_name);

   //if there is a already a icon, create a animation
   if (pd->icon)
     {
        Efl_Ui_Image *new_icon = efl_add(EFL_UI_IMAGE_CLASS, obj);
        efl_content_set(efl_part(obj, "efl.icon_new"), new_icon);
        efl_ui_image_icon_set(new_icon, standard_name);
        efl_layout_signal_emit(obj, "efl,state,icon_new,set", "efl");
        efl_layout_signal_callback_add
          (obj, "efl,state,icon_set,done", "efl", obj, _tab_icon_set_cb, NULL);
     }
   else
     {
        pd->icon = efl_add(EFL_UI_IMAGE_CLASS, obj);
        efl_content_set(efl_part(obj,"icon"), pd->icon);
        efl_ui_image_icon_set(pd->icon, standard_name);
     }
}

EOLIAN static const char*
_efl_ui_tab_bar_default_item_icon_get(const Eo *obj EINA_UNUSED, Efl_Ui_Tab_Bar_Default_Item_Data *pd)
{
   return pd->icon_name;
}

EOLIAN static Efl_Object *
_efl_ui_tab_bar_default_item_efl_object_constructor(Eo *obj, Efl_Ui_Tab_Bar_Default_Item_Data *pd EINA_UNUSED)
{
   Eo *eo;

   eo = efl_constructor(efl_super(obj, MY_CLASS));

   if (!elm_widget_theme_klass_get(obj))
     elm_widget_theme_klass_set(obj, "tab_bar/tab");

   return eo;
}

#include "efl_ui_tab_bar_default_item.eo.c"
