#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include "elm_priv.h"

#define MY_CLASS EFL_UI_RADIO_BOX_CLASS

typedef struct {
   Eina_Bool in_pack;
   Efl_Ui_Radio_Group *group;
} Efl_Ui_Radio_Box_Data;

/* The radio joins the group once the box holds it. Packing it takes it from
   its former parent, and a radio box learns of that as its radio leaves, so
   the radio is out of the group it came from by the time it joins this one.
   A pack that calls another pack registers only in the outermost one. */
#define REGISTER_SAFE(f) \
  Eina_Bool result, is_radio; \
  if (pd->in_pack) return f; \
  is_radio = efl_isa(subobj, EFL_UI_RADIO_CLASS); \
  pd->in_pack = EINA_TRUE; \
  result = f; \
  pd->in_pack = EINA_FALSE; \
  if (result && is_radio) efl_ui_radio_group_register(pd->group, subobj); \
  return result;

static void
unpack_from_logical(Eo *obj, Efl_Ui_Radio_Box_Data *pd)
{
   int length = efl_content_count(obj);
   for (int i = 0; i < length; ++i)
     {
        efl_ui_radio_group_unregister(pd->group, efl_pack_content_get(obj, i));
     }
}

/* A radio another container takes leaves the group with the box. */
EOLIAN static Eina_Bool
_efl_ui_radio_box_efl_ui_widget_widget_sub_object_del(Eo *obj, Efl_Ui_Radio_Box_Data *pd, Efl_Gfx_Entity *sobj)
{
   Eina_Bool held = efl_isa(sobj, EFL_UI_RADIO_CLASS) &&
     (efl_pack_index_get(obj, sobj) >= 0);
   Eina_Bool ok = efl_ui_widget_sub_object_del(efl_super(obj, MY_CLASS), sobj);

   if (ok && held) efl_ui_radio_group_unregister(pd->group, sobj);
   return ok;
}

EOLIAN static Eina_Bool
_efl_ui_radio_box_efl_pack_pack_clear(Eo *obj, Efl_Ui_Radio_Box_Data *pd)
{
   unpack_from_logical(obj, pd);
   return efl_pack_clear(efl_super(obj, MY_CLASS));
}

EOLIAN static Eina_Bool
_efl_ui_radio_box_efl_pack_unpack_all(Eo *obj, Efl_Ui_Radio_Box_Data *pd)
{
   unpack_from_logical(obj, pd);
   return efl_pack_unpack_all(efl_super(obj, MY_CLASS));
}

EOLIAN static Eina_Bool
_efl_ui_radio_box_efl_pack_unpack(Eo *obj, Efl_Ui_Radio_Box_Data *pd, Efl_Gfx_Entity *subobj)
{
   efl_ui_radio_group_unregister(pd->group, subobj);
   return efl_pack_unpack(efl_super(obj, MY_CLASS), subobj);
}

EOLIAN static Efl_Gfx_Entity*
_efl_ui_radio_box_efl_pack_linear_pack_unpack_at(Eo *obj, Efl_Ui_Radio_Box_Data *pd, int index)
{
   efl_ui_radio_group_unregister(pd->group, efl_pack_content_get(obj, index));
   return efl_pack_unpack_at(efl_super(obj, MY_CLASS), index);
}

EOLIAN static Eina_Bool
_efl_ui_radio_box_efl_pack_pack(Eo *obj, Efl_Ui_Radio_Box_Data *pd, Efl_Gfx_Entity *subobj)
{
   REGISTER_SAFE(efl_pack(efl_super(obj, MY_CLASS), subobj))
}

EOLIAN static Eina_Bool
_efl_ui_radio_box_efl_pack_linear_pack_begin(Eo *obj, Efl_Ui_Radio_Box_Data *pd, Efl_Gfx_Entity *subobj)
{
   REGISTER_SAFE(efl_pack_begin(efl_super(obj, MY_CLASS), subobj))
}

EOLIAN static Eina_Bool
_efl_ui_radio_box_efl_pack_linear_pack_end(Eo *obj, Efl_Ui_Radio_Box_Data *pd, Efl_Gfx_Entity *subobj)
{
   REGISTER_SAFE(efl_pack_end(efl_super(obj, MY_CLASS), subobj))
}

EOLIAN static Eina_Bool
_efl_ui_radio_box_efl_pack_linear_pack_before(Eo *obj, Efl_Ui_Radio_Box_Data *pd, Efl_Gfx_Entity *subobj, const Efl_Gfx_Entity *existing)
{
   REGISTER_SAFE(efl_pack_before(efl_super(obj, MY_CLASS), subobj, existing));
}

EOLIAN static Eina_Bool
_efl_ui_radio_box_efl_pack_linear_pack_after(Eo *obj, Efl_Ui_Radio_Box_Data *pd, Efl_Gfx_Entity *subobj, const Efl_Gfx_Entity *existing)
{
   REGISTER_SAFE(efl_pack_after(efl_super(obj, MY_CLASS), subobj, existing));
}

EOLIAN static Eina_Bool
_efl_ui_radio_box_efl_pack_linear_pack_at(Eo *obj, Efl_Ui_Radio_Box_Data *pd, Efl_Gfx_Entity *subobj, int index)
{
   REGISTER_SAFE(efl_pack_at(efl_super(obj, MY_CLASS), subobj, index));
}

EOLIAN static Efl_Object*
_efl_ui_radio_box_efl_object_constructor(Eo *obj, Efl_Ui_Radio_Box_Data *pd)
{
   pd->group = efl_new(EFL_UI_RADIO_GROUP_IMPL_CLASS, NULL);
   efl_composite_attach(obj, pd->group);
   efl_event_callback_forwarder_add(pd->group, EFL_UI_SELECTABLE_EVENT_SELECTION_CHANGED, obj);
   efl_event_callback_forwarder_add(pd->group, EFL_UI_RADIO_GROUP_EVENT_VALUE_CHANGED, obj);
   return efl_constructor(efl_super(obj, MY_CLASS));
}


#include "efl_ui_radio_box.eo.c"
