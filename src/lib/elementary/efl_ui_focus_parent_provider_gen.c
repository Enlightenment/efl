#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_priv.h"
#include "efl_ui_focus_parent_provider_gen.eo.h"

typedef struct {
   Eina_Hash *map;
   Elm_Widget *container;
   Efl_Ui_Focus_Parent_Provider *provider;
} Efl_Ui_Focus_Parent_Provider_Gen_Data;

EOLIAN static void
_efl_ui_focus_parent_provider_gen_content_item_map_set(Eo *obj, Efl_Ui_Focus_Parent_Provider_Gen_Data *pd, Eina_Hash *map)
{
   EINA_SAFETY_ON_TRUE_RETURN(efl_finalized_get(obj));

   pd->map = map;
}

EOLIAN static Eina_Hash*
_efl_ui_focus_parent_provider_gen_content_item_map_get(Eo *obj EINA_UNUSED, Efl_Ui_Focus_Parent_Provider_Gen_Data *pd)
{
   return pd->map;
}

EOLIAN static void
_efl_ui_focus_parent_provider_gen_container_set(Eo *obj, Efl_Ui_Focus_Parent_Provider_Gen_Data *pd, Elm_Widget *container)
{
   EINA_SAFETY_ON_TRUE_RETURN(efl_finalized_get(obj));

   pd->container = container;

   EINA_SAFETY_ON_NULL_RETURN(efl_parent_get(pd->container));

   pd->provider = efl_provider_find(efl_parent_get(pd->container), EFL_UI_FOCUS_PARENT_PROVIDER_INTERFACE);
}

EOLIAN static Elm_Widget*
_efl_ui_focus_parent_provider_gen_container_get(Eo *obj EINA_UNUSED, Efl_Ui_Focus_Parent_Provider_Gen_Data *pd)
{
   return pd->container;
}


EOLIAN static Efl_Ui_Focus_Object*
_efl_ui_focus_parent_provider_gen_efl_ui_focus_parent_provider_find_logical_parent(Eo *obj EINA_UNUSED, Efl_Ui_Focus_Parent_Provider_Gen_Data *pd EINA_UNUSED, Efl_Ui_Focus_Object *widget)
{
   //first check if this item is in the map
   Elm_Widget_Item *item;

   item = eina_hash_find(pd->map, &widget);

   efl_ui_focus_composition_elements_flush(pd->container);

   // We dont have a map entry
   if (!item)
     {
        return efl_ui_focus_parent_provider_find_logical_parent(pd->provider, widget);
     }
   else
     {
        return item;
     }
}


#include "efl_ui_focus_parent_provider_gen.eo.c"
