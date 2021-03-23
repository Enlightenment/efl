#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif


#include <Efl_Ui.h>
#include "elm_priv.h"
#include "efl_ui_tab_bar_private.h"
#include "els_box.h"

#define MY_CLASS EFL_UI_TAB_BAR_CLASS
#define MY_CLASS_NAME "Efl.Ui.Tab_Bar"


EOLIAN static Efl_Ui_Selectable*
_efl_ui_tab_bar_efl_ui_single_selectable_last_selected_get(const Eo *obj EINA_UNUSED, Efl_Ui_Tab_Bar_Data *pd)
{
   return pd->selected;
}

EOLIAN static void
_efl_ui_tab_bar_efl_ui_single_selectable_fallback_selection_set(Eo *obj EINA_UNUSED, Efl_Ui_Tab_Bar_Data *pd, Efl_Ui_Selectable *fallback)
{
   pd->fallback_selection = fallback;
   if (!pd->selected)
     {
        efl_ui_selectable_selected_set(pd->fallback_selection, EINA_TRUE);
        if (pd->selected) evas_object_raise(pd->selected);
     }
}

EOLIAN static void
_efl_ui_tab_bar_efl_ui_single_selectable_allow_manual_deselection_set(Eo *obj EINA_UNUSED, Efl_Ui_Tab_Bar_Data *pd, Eina_Bool allow_manual_deselection)
{
   pd->allow_manual_deselection = !!allow_manual_deselection;
}

EOLIAN static Eina_Bool
_efl_ui_tab_bar_efl_ui_single_selectable_allow_manual_deselection_get(const Eo *obj EINA_UNUSED, Efl_Ui_Tab_Bar_Data *pd)
{
   return pd->allow_manual_deselection;
}


EOLIAN static Efl_Ui_Selectable*
_efl_ui_tab_bar_efl_ui_single_selectable_fallback_selection_get(const Eo *obj EINA_UNUSED, Efl_Ui_Tab_Bar_Data *pd)
{
   return pd->fallback_selection;
}

static void _remove_item(Eo *obj, Efl_Ui_Tab_Bar_Data *pd, Efl_Ui_Item *item);

static void
_selelction_change_cb(void *data, const Efl_Event *ev)
{
   Efl_Ui_Tab_Bar_Data *pd = efl_data_scope_safe_get(data, MY_CLASS);

   EINA_SAFETY_ON_NULL_RETURN(pd);

   if (!efl_ui_selectable_selected_get(ev->object))
     {
        if (pd->selected == ev->object)
          {
             pd->selected = NULL;
          }
        //checkout if we want to do fallback handling
        if (!pd->in_value_change)
          {
             if (!pd->selected && pd->fallback_selection)
               {
                  efl_ui_selectable_selected_set(pd->fallback_selection, EINA_TRUE);
                  if (pd->selected) evas_object_raise(pd->selected);
               }
          }
     }
   else
     {
        pd->in_value_change = EINA_TRUE;
        if (pd->selected)
          efl_ui_selectable_selected_set(pd->selected, EINA_FALSE);
        pd->in_value_change = EINA_FALSE;
        EINA_SAFETY_ON_FALSE_RETURN(!pd->selected);
        pd->selected = ev->object;
        efl_event_callback_call(data, EFL_UI_EVENT_ITEM_SELECTED, NULL);
        if (pd->selected) evas_object_raise(pd->selected);
     }
   if (!pd->in_value_change)
     {
        efl_event_callback_call(data, EFL_UI_SELECTABLE_EVENT_SELECTION_CHANGED, NULL);
     }
}

static void
_invalidate_cb(void *data, const Efl_Event *ev)
{
   Efl_Ui_Tab_Bar_Data *pd = efl_data_scope_safe_get(data, MY_CLASS);

   EINA_SAFETY_ON_NULL_RETURN(pd);
   _remove_item(data, pd, ev->object);
}

EFL_CALLBACKS_ARRAY_DEFINE(item_listeners,
  {EFL_UI_EVENT_SELECTED_CHANGED, _selelction_change_cb},
  {EFL_EVENT_INVALIDATE, _invalidate_cb},
)

static void
_remove_item(Eo *obj, Efl_Ui_Tab_Bar_Data *pd, Efl_Ui_Item *item)
{
   if (pd->selected == item)
     pd->selected = NULL;

   efl_event_callback_array_del(item, item_listeners(), obj);
   if (efl_alive_get(item))
     _elm_widget_sub_object_redirect_to_top(obj, item);
}

static Eina_Bool
_register_item(Eo *obj, Efl_Ui_Tab_Bar_Data *pd EINA_UNUSED, Eo *subitem)
{
   EINA_SAFETY_ON_FALSE_RETURN_VAL(efl_isa(subitem, EFL_UI_TAB_BAR_DEFAULT_ITEM_CLASS), EINA_FALSE);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(efl_ui_widget_sub_object_add(obj, subitem), EINA_FALSE);
   efl_ui_item_container_set(subitem, obj);
   efl_event_callback_array_add(subitem, item_listeners(), obj);
   efl_gfx_hint_align_set(subitem, EVAS_HINT_FILL, EVAS_HINT_FILL);
   return EINA_TRUE;
}

EOLIAN static void
_efl_ui_tab_bar_efl_object_destructor(Eo *obj, Efl_Ui_Tab_Bar_Data *sd EINA_UNUSED)
{
   efl_destructor(efl_super(obj, MY_CLASS));
}

EOLIAN static Efl_Object *
_efl_ui_tab_bar_efl_object_constructor(Eo *obj, Efl_Ui_Tab_Bar_Data *sd)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, NULL);

   if (!elm_widget_theme_klass_get(obj))
     elm_widget_theme_klass_set(obj, "tab_bar");

   obj = efl_constructor(efl_super(obj, MY_CLASS));

   sd->bx = efl_add(EFL_UI_BOX_CLASS, obj);
   efl_ui_box_homogeneous_set(sd->bx, EINA_TRUE);
   efl_ui_layout_orientation_set(sd->bx, EFL_UI_LAYOUT_ORIENTATION_HORIZONTAL);
   efl_ui_widget_internal_set(sd->bx, EINA_TRUE);

   efl_composite_attach(obj, sd->bx);

   return obj;
}

EOLIAN static Efl_Object*
_efl_ui_tab_bar_efl_object_finalize(Eo *obj, Efl_Ui_Tab_Bar_Data *pd)
{
   Eo *o = efl_finalize(efl_super(obj, MY_CLASS));

   if (!o) return NULL;

   efl_content_set(efl_part(obj,"efl.content"), pd->bx);

   return o;
}

EOLIAN static Eina_Bool
_efl_ui_tab_bar_efl_pack_pack_clear(Eo *obj, Efl_Ui_Tab_Bar_Data *pd)
{
   for (int i = 0; i < efl_content_count(pd->bx); ++i)
     {
        _remove_item(obj, pd, efl_pack_content_get(pd->bx, i));
     }
   return efl_pack_clear(pd->bx);
}

EOLIAN static Eina_Bool
_efl_ui_tab_bar_efl_pack_unpack_all(Eo *obj, Efl_Ui_Tab_Bar_Data *pd)
{
   for (int i = 0; i < efl_content_count(pd->bx); ++i)
     {
        _remove_item(obj, pd, efl_pack_content_get(pd->bx, i));
     }
   return efl_pack_unpack_all(pd->bx);
}

EOLIAN static Eina_Bool
_efl_ui_tab_bar_efl_pack_unpack(Eo *obj, Efl_Ui_Tab_Bar_Data *pd, Efl_Gfx_Entity *subobj)
{
   _remove_item(obj, pd, subobj);
   return efl_pack_unpack(pd->bx, subobj);
}

EOLIAN static Eina_Bool
_efl_ui_tab_bar_efl_pack_linear_pack_begin(Eo *obj, Efl_Ui_Tab_Bar_Data *pd, Efl_Gfx_Entity *subobj)
{
   Eina_Bool val;

   EINA_SAFETY_ON_FALSE_RETURN_VAL(_register_item(obj, pd, subobj), EINA_FALSE);
   val = efl_pack_begin(pd->bx, subobj);
   if (pd->selected) evas_object_raise(pd->selected);
   return val;
}

EOLIAN static Eina_Bool
_efl_ui_tab_bar_efl_pack_linear_pack_end(Eo *obj, Efl_Ui_Tab_Bar_Data *pd, Efl_Gfx_Entity *subobj)
{
   Eina_Bool val;

   EINA_SAFETY_ON_FALSE_RETURN_VAL(_register_item(obj, pd, subobj), EINA_FALSE);
   val = efl_pack_end(pd->bx, subobj);
   if (pd->selected) evas_object_raise(pd->selected);
   return val;
}

EOLIAN static Eina_Bool
_efl_ui_tab_bar_efl_pack_linear_pack_before(Eo *obj, Efl_Ui_Tab_Bar_Data *pd, Efl_Gfx_Entity *subobj, const Efl_Gfx_Entity *existing)
{
   Eina_Bool val;

   if (existing)
     EINA_SAFETY_ON_FALSE_RETURN_VAL(efl_pack_index_get(pd->bx, existing) >= 0, EINA_FALSE);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(_register_item(obj, pd, subobj), EINA_FALSE);
   val = efl_pack_before(pd->bx, subobj, existing);
   if (pd->selected) evas_object_raise(pd->selected);
   return val;
}

EOLIAN static Eina_Bool
_efl_ui_tab_bar_efl_pack_linear_pack_after(Eo *obj, Efl_Ui_Tab_Bar_Data *pd, Efl_Gfx_Entity *subobj, const Efl_Gfx_Entity *existing)
{
   Eina_Bool val;

   if (existing)
     EINA_SAFETY_ON_FALSE_RETURN_VAL(efl_pack_index_get(pd->bx, existing) >= 0, EINA_FALSE);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(_register_item(obj, pd, subobj), EINA_FALSE);
   val = efl_pack_after(pd->bx, subobj, existing);
   if (pd->selected) evas_object_raise(pd->selected);
   return val;
}

EOLIAN static Eina_Bool
_efl_ui_tab_bar_efl_pack_linear_pack_at(Eo *obj, Efl_Ui_Tab_Bar_Data *pd, Efl_Gfx_Entity *subobj, int index)
{
   Eina_Bool val;

   EINA_SAFETY_ON_FALSE_RETURN_VAL(_register_item(obj, pd, subobj), EINA_FALSE);
   val = efl_pack_at(pd->bx, subobj, index);
   if (pd->selected) evas_object_raise(pd->selected);
   return val;
}

EOLIAN static int
_efl_ui_tab_bar_efl_container_content_count(Eo *obj EINA_UNUSED, Efl_Ui_Tab_Bar_Data *pd)
{
   return efl_content_count(pd->bx);
}

EOLIAN static Eina_Iterator*
_efl_ui_tab_bar_efl_container_content_iterate(Eo *obj EINA_UNUSED, Efl_Ui_Tab_Bar_Data *pd)
{
   return efl_content_iterate(pd->bx);
}

EOLIAN static Efl_Gfx_Entity*
_efl_ui_tab_bar_efl_pack_linear_pack_unpack_at(Eo *obj, Efl_Ui_Tab_Bar_Data *pd, int index)
{
   _remove_item(obj, pd, efl_pack_content_get(pd->bx, index));
   return efl_pack_unpack_at(pd->bx, index);
}

EOLIAN static Eina_Bool
_efl_ui_tab_bar_efl_pack_pack(Eo *obj, Efl_Ui_Tab_Bar_Data *pd, Efl_Gfx_Entity *subobj)
{
   Eina_Bool val;

   EINA_SAFETY_ON_FALSE_RETURN_VAL(_register_item(obj, pd, subobj), EINA_FALSE);
   val = efl_pack(pd->bx, subobj);
   if (pd->selected) evas_object_raise(pd->selected);
   return val;
}

#include "efl_ui_tab_bar.eo.c"
