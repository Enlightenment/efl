#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif


#include <Efl_Ui.h>
#include "elm_priv.h"

#include "efl_ui_tab_pager_private.h"
#include "efl_ui_tab_page_private.h"

#define MY_CLASS EFL_UI_TAB_PAGER_CLASS

static void
_select(Eo *obj, int index)
{
   Efl_Ui_Tab_Page *page = efl_pack_content_get(obj, index);

   efl_ui_selectable_selected_set(efl_ui_tab_page_tab_bar_item_get(page), EINA_TRUE);
}

static void
_tab_select_cb(void *data, const Efl_Event *event)
{
   EFL_UI_TAB_PAGER_DATA_GET(data, sd);
   Efl_Ui_Tab_Bar_Default_Item *selected;
   int i = 0;

   selected = efl_ui_single_selectable_last_selected_get(event->object);
   i = efl_pack_index_get(sd->tab_bar, selected);
   if (efl_ui_spotlight_active_index_get(data) != i)
     efl_ui_spotlight_active_index_set(data, i);
}

EOLIAN static Efl_Canvas_Object *
_efl_ui_tab_pager_tab_bar_get(const Eo *obj EINA_UNUSED, Efl_Ui_Tab_Pager_Data *sd)
{
   return sd->tab_bar;
}

EOLIAN static void
_efl_ui_tab_pager_efl_ui_spotlight_container_active_index_set(Eo *obj, Efl_Ui_Tab_Pager_Data *sd EINA_UNUSED, int index)
{
   efl_ui_spotlight_active_index_set(efl_super(obj, MY_CLASS), index);
   _select(obj, index);
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

   return obj;
}

EOLIAN static Efl_Object*
_efl_ui_tab_pager_efl_object_finalize(Eo *obj, Efl_Ui_Tab_Pager_Data *pd)
{
   obj = efl_finalize(efl_super(obj, MY_CLASS));

   efl_content_set(efl_part(obj, "efl.tab_root"), pd->tab_bar);

   return obj;
}

EOLIAN static Eina_Bool
_efl_ui_tab_pager_efl_pack_pack_clear(Eo *obj, Efl_Ui_Tab_Pager_Data *sd)
{
   if (!efl_pack_clear(sd->tab_bar))
     return EINA_FALSE;
   return efl_pack_clear(efl_super(obj, MY_CLASS));
}

EOLIAN static Eina_Bool
_efl_ui_tab_pager_efl_pack_unpack_all(Eo *obj, Efl_Ui_Tab_Pager_Data *sd)
{
   if (!efl_pack_unpack_all(sd->tab_bar))
     return EINA_FALSE;
   return efl_pack_unpack_all(efl_super(obj, MY_CLASS));
}

#define ITEM(s) efl_ui_tab_page_tab_bar_item_get(s)

EOLIAN static Eina_Bool
_efl_ui_tab_pager_efl_pack_unpack(Eo *obj, Efl_Ui_Tab_Pager_Data *sd, Efl_Gfx_Entity *subobj)
{
   if (!efl_pack_unpack(sd->tab_bar, ITEM(subobj)))
     return EINA_FALSE;
   return efl_pack_unpack(efl_super(obj, MY_CLASS), subobj);
}

EOLIAN static Eina_Bool
_efl_ui_tab_pager_efl_pack_linear_pack_begin(Eo *obj,
                                             Efl_Ui_Tab_Pager_Data *sd,
                                             Efl_Gfx_Entity *subobj)
{
   if (!efl_pack_begin(sd->tab_bar, ITEM(subobj)))
     return EINA_FALSE;
   return efl_pack_begin(efl_super(obj, MY_CLASS), subobj);
}

EOLIAN static Eina_Bool
_efl_ui_tab_pager_efl_pack_linear_pack_end(Eo *obj,
                                           Efl_Ui_Tab_Pager_Data *sd,
                                           Efl_Gfx_Entity *subobj)
{
   if (!efl_pack_end(sd->tab_bar, ITEM(subobj)))
     return EINA_FALSE;
   return efl_pack_end(efl_super(obj, MY_CLASS), subobj);
}

EOLIAN static Eina_Bool
_efl_ui_tab_pager_efl_pack_linear_pack_before(Eo *obj,
                                              Efl_Ui_Tab_Pager_Data *sd,
                                              Efl_Gfx_Entity *subobj,
                                              const Efl_Gfx_Entity *existing)
{
   if (!efl_pack_before(sd->tab_bar, ITEM(subobj), ITEM(existing)))
     return EINA_FALSE;
   return efl_pack_before(efl_super(obj, MY_CLASS), subobj, existing);
}

EOLIAN static Eina_Bool
_efl_ui_tab_pager_efl_pack_linear_pack_after(Eo *obj,
                                             Efl_Ui_Tab_Pager_Data *sd,
                                             Efl_Gfx_Entity *subobj,
                                             const Efl_Gfx_Entity *existing)
{
   if (!efl_pack_after(sd->tab_bar, ITEM(subobj), ITEM(existing)))
     return EINA_FALSE;
   return efl_pack_after(efl_super(obj, MY_CLASS), subobj, existing);
}

EOLIAN static Eina_Bool
_efl_ui_tab_pager_efl_pack_linear_pack_at(Eo *obj,
                                          Efl_Ui_Tab_Pager_Data *sd,
                                          Efl_Gfx_Entity *subobj,
                                          int index)
{
   if (!efl_pack_at(sd->tab_bar, ITEM(subobj), index))
     return EINA_FALSE;
   return efl_pack_at(efl_super(obj, MY_CLASS), subobj, index);
}

EOLIAN static Efl_Gfx_Entity *
_efl_ui_tab_pager_efl_pack_linear_pack_unpack_at(Eo *obj, Efl_Ui_Tab_Pager_Data *sd, int index)
{
   if (!efl_pack_unpack_at(sd->tab_bar, index))
     return NULL;
   return efl_pack_unpack_at(efl_super(obj, MY_CLASS), index);
}

#include "efl_ui_tab_pager.eo.c"
