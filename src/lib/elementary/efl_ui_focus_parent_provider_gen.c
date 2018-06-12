#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_priv.h"
#include "efl_ui_focus_parent_provider_gen.eo.h"

typedef struct {
   Eina_Hash *map;
   Efl_Ui_Widget *container;
   Efl_Ui_Focus_Parent_Provider *provider;
} Efl_Ui_Focus_Parent_Provider_Gen_Data;

EOLIAN static void
_efl_ui_focus_parent_provider_gen_content_item_map_set(Eo *obj, Efl_Ui_Focus_Parent_Provider_Gen_Data *pd, Eina_Hash *map)
{
   EINA_SAFETY_ON_TRUE_RETURN(efl_finalized_get(obj));

   pd->map = map;
}

EOLIAN static Eina_Hash*
_efl_ui_focus_parent_provider_gen_content_item_map_get(const Eo *obj EINA_UNUSED, Efl_Ui_Focus_Parent_Provider_Gen_Data *pd)
{
   return pd->map;
}

EOLIAN static void
_efl_ui_focus_parent_provider_gen_container_set(Eo *obj, Efl_Ui_Focus_Parent_Provider_Gen_Data *pd, Efl_Ui_Widget *container)
{
   EINA_SAFETY_ON_TRUE_RETURN(efl_finalized_get(obj));

   pd->container = container;

   EINA_SAFETY_ON_NULL_RETURN(efl_parent_get(pd->container));

   pd->provider = efl_provider_find(efl_parent_get(pd->container), EFL_UI_FOCUS_PARENT_PROVIDER_INTERFACE);
}

EOLIAN static Efl_Ui_Widget*
_efl_ui_focus_parent_provider_gen_container_get(const Eo *obj EINA_UNUSED, Efl_Ui_Focus_Parent_Provider_Gen_Data *pd)
{
   return pd->container;
}


EOLIAN static Efl_Ui_Focus_Object*
_efl_ui_focus_parent_provider_gen_efl_ui_focus_parent_provider_find_logical_parent(Eo *obj EINA_UNUSED, Efl_Ui_Focus_Parent_Provider_Gen_Data *pd EINA_UNUSED, Efl_Ui_Focus_Object *widget)
{
   //first check if this item is in the map
   Elm_Widget_Item *item, *above_gengrid = widget;
   Efl_Ui_Widget *parent = elm_widget_parent_widget_get(widget);

   if (parent == pd->container)
     {
        item = eina_hash_find(pd->map, &above_gengrid);
        efl_ui_focus_object_prepare_logical(pd->container);

        if (item)
          return item;
     }

   // We dont have a map entry
   return efl_ui_focus_parent_provider_find_logical_parent(pd->provider, widget);
}

EOLIAN static Efl_Ui_Widget*
_efl_ui_focus_parent_provider_gen_item_fetch(Eo *obj, Efl_Ui_Focus_Parent_Provider_Gen_Data *pd, Efl_Ui_Widget *widget)
{
   //first check if this item is in the map
   Elm_Widget_Item *item, *above_gengrid = widget;
   Efl_Ui_Widget *parent = elm_widget_parent_widget_get(widget);

   //move forward so we get the last widget above the gengrid level,
   //this may be the widget out of the map
   while (parent && parent != pd->container)
     {
        above_gengrid = parent;
        parent = elm_widget_parent_widget_get(above_gengrid);
     }

   item = eina_hash_find(pd->map, &above_gengrid);

   efl_ui_focus_object_prepare_logical(pd->container);

   return item;
}

#include "efl_ui_focus_parent_provider_gen.eo.c"
