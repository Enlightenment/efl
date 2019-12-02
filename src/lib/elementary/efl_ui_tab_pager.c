#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif


#include <Efl_Ui.h>
#include "elm_priv.h"

#include "efl_ui_tab_pager_private.h"
#include "efl_ui_tab_page_private.h"

#define MY_CLASS EFL_UI_TAB_PAGER_CLASS

static void
_efl_ui_tab_pager_spotlight_manager_set(Eo *obj EINA_UNUSED, Efl_Ui_Tab_Pager_Data *pd, Efl_Ui_Spotlight_Manager *manager)
{
   efl_ui_spotlight_manager_set(pd->spotlight, manager);
}


static void
_tab_select_cb(void *data, const Efl_Event *event)
{
   Efl_Ui_Tab_Bar_Default_Item *selected;
   Efl_Ui_Tab_Page *page;
   Efl_Ui_Tab_Pager_Data *pd;

   pd = efl_data_scope_get(data, MY_CLASS);
   EINA_SAFETY_ON_NULL_RETURN(pd);
   selected = efl_ui_selectable_last_selected_get(event->object);
   page = efl_parent_get(selected);
   if (efl_ui_spotlight_active_element_get(pd->spotlight))
     efl_ui_spotlight_active_element_set(pd->spotlight, page);
}

EOLIAN static Efl_Canvas_Object *
_efl_ui_tab_pager_tab_bar_get(const Eo *obj EINA_UNUSED, Efl_Ui_Tab_Pager_Data *sd)
{
   return sd->tab_bar;
}

EOLIAN static void
_efl_ui_tab_pager_efl_object_destructor(Eo *obj, Efl_Ui_Tab_Pager_Data *sd)
{
   if (sd->tab_bar != NULL)
     efl_event_callback_del(sd->tab_bar, EFL_UI_EVENT_ITEM_SELECTED, _tab_select_cb, obj);

   efl_destructor(efl_super(obj, MY_CLASS));
}

EOLIAN static Efl_Object *
_efl_ui_tab_pager_efl_object_constructor(Eo *obj, Efl_Ui_Tab_Pager_Data *sd)
{
   if (!elm_widget_theme_klass_get(obj))
     elm_widget_theme_klass_set(obj, "tab_pager");

   obj = efl_constructor(efl_super(obj, MY_CLASS));

   efl_ui_widget_focus_allow_set(obj, EINA_TRUE);

   sd->tab_bar = efl_add(EFL_UI_TAB_BAR_CLASS, obj);
   efl_event_callback_add(sd->tab_bar, EFL_UI_EVENT_ITEM_SELECTED, _tab_select_cb, obj);
   efl_event_callback_forwarder_del(sd->tab_bar, EFL_UI_SELECTABLE_EVENT_SELECTION_CHANGED, obj);

   sd->spotlight = efl_add(EFL_UI_SPOTLIGHT_CONTAINER_CLASS, obj);

   return obj;
}

EOLIAN static Eina_Error
_efl_ui_tab_pager_efl_ui_widget_theme_apply(Eo *obj, Efl_Ui_Tab_Pager_Data *pd)
{
   Eina_Error err;

   err = efl_ui_widget_theme_apply(efl_super(obj, MY_CLASS));

   efl_content_set(efl_part(obj, "efl.tab_root"), pd->tab_bar);
   efl_content_set(efl_part(obj, "efl.page_root"), pd->spotlight);

   return err;
}

EOLIAN static Efl_Object*
_efl_ui_tab_pager_efl_object_finalize(Eo *obj, Efl_Ui_Tab_Pager_Data *pd)
{
   obj = efl_finalize(efl_super(obj, MY_CLASS));

   efl_content_set(efl_part(obj, "efl.tab_root"), pd->tab_bar);

   return obj;
}

EOLIAN static int
_efl_ui_tab_pager_efl_container_content_count(Eo *obj EINA_UNUSED, Efl_Ui_Tab_Pager_Data *pd)
{
   return efl_content_count(pd->spotlight);
}

EOLIAN static Eina_Iterator*
_efl_ui_tab_pager_efl_container_content_iterate(Eo *obj EINA_UNUSED, Efl_Ui_Tab_Pager_Data *pd)
{
   return efl_content_iterate(pd->spotlight);
}

EOLIAN static Eina_Bool
_efl_ui_tab_pager_efl_pack_pack(Eo *obj EINA_UNUSED, Efl_Ui_Tab_Pager_Data *pd, Efl_Gfx_Entity *subobj)
{
   return efl_pack(pd->spotlight, subobj);
}

EOLIAN static Efl_Gfx_Entity*
_efl_ui_tab_pager_efl_pack_linear_pack_content_get(Eo *obj EINA_UNUSED, Efl_Ui_Tab_Pager_Data *pd, int index)
{
   return efl_pack_content_get(pd->spotlight, index);
}

EOLIAN static int
_efl_ui_tab_pager_efl_pack_linear_pack_index_get(Eo *obj EINA_UNUSED, Efl_Ui_Tab_Pager_Data *pd, const Efl_Gfx_Entity *subobj)
{
   return efl_pack_index_get(pd->spotlight, subobj);
}

EOLIAN static Eina_Bool
_efl_ui_tab_pager_efl_pack_pack_clear(Eo *obj EINA_UNUSED, Efl_Ui_Tab_Pager_Data *sd)
{
   if (!efl_pack_clear(sd->tab_bar))
     return EINA_FALSE;
   return efl_pack_clear(sd->spotlight);
}

EOLIAN static Eina_Bool
_efl_ui_tab_pager_efl_pack_unpack_all(Eo *obj EINA_UNUSED, Efl_Ui_Tab_Pager_Data *sd)
{
   if (!efl_pack_unpack_all(sd->tab_bar))
     return EINA_FALSE;
   return efl_pack_unpack_all(sd->spotlight);
}

#define ITEM(s) efl_ui_tab_page_tab_bar_item_get(s)

EOLIAN static Eina_Bool
_efl_ui_tab_pager_efl_pack_unpack(Eo *obj EINA_UNUSED, Efl_Ui_Tab_Pager_Data *sd, Efl_Gfx_Entity *subobj)
{
   if (!efl_pack_unpack(sd->tab_bar, ITEM(subobj)))
     return EINA_FALSE;
   return efl_pack_unpack(sd->spotlight, subobj);
}

EOLIAN static Eina_Bool
_efl_ui_tab_pager_efl_pack_linear_pack_begin(Eo *obj EINA_UNUSED,
                                             Efl_Ui_Tab_Pager_Data *sd,
                                             Efl_Gfx_Entity *subobj)
{
   if (!efl_pack_begin(sd->tab_bar, ITEM(subobj)))
     return EINA_FALSE;
   return efl_pack_begin(sd->spotlight, subobj);
}

EOLIAN static Eina_Bool
_efl_ui_tab_pager_efl_pack_linear_pack_end(Eo *obj EINA_UNUSED,
                                           Efl_Ui_Tab_Pager_Data *sd,
                                           Efl_Gfx_Entity *subobj)
{
   if (!efl_pack_end(sd->tab_bar, ITEM(subobj)))
     return EINA_FALSE;
   return efl_pack_end(sd->spotlight, subobj);
}

EOLIAN static Eina_Bool
_efl_ui_tab_pager_efl_pack_linear_pack_before(Eo *obj EINA_UNUSED,
                                              Efl_Ui_Tab_Pager_Data *sd,
                                              Efl_Gfx_Entity *subobj,
                                              const Efl_Gfx_Entity *existing)
{
   if (!efl_pack_before(sd->tab_bar, ITEM(subobj), ITEM(existing)))
     return EINA_FALSE;
   return efl_pack_before(sd->spotlight, subobj, existing);
}

EOLIAN static Eina_Bool
_efl_ui_tab_pager_efl_pack_linear_pack_after(Eo *obj EINA_UNUSED,
                                             Efl_Ui_Tab_Pager_Data *sd,
                                             Efl_Gfx_Entity *subobj,
                                             const Efl_Gfx_Entity *existing)
{
   if (!efl_pack_after(sd->tab_bar, ITEM(subobj), ITEM(existing)))
     return EINA_FALSE;
   return efl_pack_after(sd->spotlight, subobj, existing);
}

EOLIAN static Eina_Bool
_efl_ui_tab_pager_efl_pack_linear_pack_at(Eo *obj EINA_UNUSED,
                                          Efl_Ui_Tab_Pager_Data *sd,
                                          Efl_Gfx_Entity *subobj,
                                          int index)
{
   if (!efl_pack_at(sd->tab_bar, ITEM(subobj), index))
     return EINA_FALSE;
   return efl_pack_at(sd->spotlight, subobj, index);
}

EOLIAN static Efl_Gfx_Entity *
_efl_ui_tab_pager_efl_pack_linear_pack_unpack_at(Eo *obj EINA_UNUSED, Efl_Ui_Tab_Pager_Data *sd, int index)
{
   if (!efl_pack_unpack_at(sd->tab_bar, index))
     return NULL;
   return efl_pack_unpack_at(sd->spotlight, index);
}

EOLIAN static Efl_Ui_Selectable*
_efl_ui_tab_pager_efl_ui_single_selectable_last_selected_get(const Eo *obj EINA_UNUSED, Efl_Ui_Tab_Pager_Data *pd)
{
   Efl_Ui_Tab_Bar_Default_Item *item = efl_ui_selectable_last_selected_get(pd->tab_bar);

   return efl_parent_get(item);
}

EOLIAN static void
_efl_ui_tab_pager_efl_ui_single_selectable_fallback_selection_set(Eo *obj EINA_UNUSED, Efl_Ui_Tab_Pager_Data *pd, Efl_Ui_Selectable *fallback)
{
   efl_ui_selectable_fallback_selection_set(pd->tab_bar, efl_ui_tab_page_tab_bar_item_get(fallback));
}

EOLIAN static Efl_Ui_Selectable*
_efl_ui_tab_pager_efl_ui_single_selectable_fallback_selection_get(const Eo *obj EINA_UNUSED, Efl_Ui_Tab_Pager_Data *pd)
{
   Efl_Ui_Tab_Bar_Default_Item *item = efl_ui_selectable_fallback_selection_get(pd->tab_bar);

   return efl_parent_get(item);
}

EOLIAN static void
_efl_ui_tab_pager_efl_ui_single_selectable_allow_manual_deselection_set(Eo *obj EINA_UNUSED, Efl_Ui_Tab_Pager_Data *pd, Eina_Bool allow_manual_deselection)
{
   efl_ui_selectable_allow_manual_deselection_set(pd->tab_bar, allow_manual_deselection);
}


EOLIAN static Eina_Bool
_efl_ui_tab_pager_efl_ui_single_selectable_allow_manual_deselection_get(const Eo *obj EINA_UNUSED, Efl_Ui_Tab_Pager_Data *pd)
{
   return efl_ui_selectable_allow_manual_deselection_get(pd->tab_bar);
}


#include "efl_ui_tab_pager.eo.c"
