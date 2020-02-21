#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_PACK_LAYOUT_PROTECTED

#include <Elementary.h>
#include "elm_priv.h"

typedef struct {
   Efl_Ui_Box *indicator;
   Efl_Ui_Spotlight_Container *container;
   Efl_Canvas_Layout *layout;
   double last_position;
   Eina_Size2D min, max;
} Efl_Ui_Spotlight_Icon_Indicator_Data;

static void
_flush_state(Eo *item, double val)
{
   Eina_Value v = eina_value_double_init(val);

    efl_layout_signal_message_send(item, 1, v);
}

static void
_add_item(Eo *obj EINA_UNUSED, Efl_Ui_Spotlight_Icon_Indicator_Data *pd)
{
   Efl_Canvas_Layout *item;

   item = efl_add(EFL_CANVAS_LAYOUT_CLASS, pd->indicator);
   elm_widget_theme_object_set(pd->indicator, item,
                               "spotlight", "indicator", "default");
   efl_gfx_hint_align_set(item, 0.5, 0.5);
   efl_gfx_hint_weight_set(item, 0, 0);
   efl_gfx_hint_fill_set(item, 0, 0);
   efl_gfx_hint_size_min_set(item,  efl_layout_calc_size_min(item, EINA_SIZE2D(0, 0)));
   efl_pack_end(pd->indicator, item);
   _flush_state(item, 0.0);
}

static void
_flush_position(Eo *obj EINA_UNUSED, Efl_Ui_Spotlight_Icon_Indicator_Data *pd)
{
   int next = -1, closer;
   double closer_val;

   for (int i = 0; i < efl_content_count(pd->indicator); ++i)
     {
        _flush_state(efl_pack_content_get(pd->indicator, i), 0.0f);
     }

   closer = MIN(MAX(round(pd->last_position), 0), efl_content_count(pd->container) - 1);
   closer_val = fabs(1.0f - fabs(closer - pd->last_position));

   _flush_state(efl_pack_content_get(pd->indicator, closer), closer_val);

   if (pd->last_position > closer)
     next = closer + 1;
   else if (pd->last_position < closer)
     next = closer - 1;
   else
     {
        next = closer + 1;
        if (next < efl_content_count(pd->container))
          _flush_state(efl_pack_content_get(pd->indicator, next), fabs(1.0f - fabs((next - pd->last_position))));
        next = closer - 1;
        if (next >= 0)
          _flush_state(efl_pack_content_get(pd->indicator, next), fabs(1.0f - fabs((next - pd->last_position))));
     }

   if (next >= 0 && next < efl_content_count(pd->container))
     _flush_state(efl_pack_content_get(pd->indicator, next), fabs(1.0f - fabs((next - pd->last_position))));
}

static void
_resize_cb(void *data, const Efl_Event *ev)
{
   efl_gfx_entity_size_set(data, efl_gfx_entity_size_get(ev->object));
}

static void
_position_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   efl_gfx_entity_position_set(data, efl_gfx_entity_position_get(ev->object));
}

EFL_CALLBACKS_ARRAY_DEFINE(spotlight_resized,
  {EFL_GFX_ENTITY_EVENT_SIZE_CHANGED, _resize_cb},
  {EFL_GFX_ENTITY_EVENT_POSITION_CHANGED, _position_cb},
)

EOLIAN static void
_efl_ui_spotlight_icon_indicator_efl_ui_spotlight_indicator_bind(Eo *obj, Efl_Ui_Spotlight_Icon_Indicator_Data *pd, Efl_Ui_Spotlight_Container *spotlight)
{
   if (spotlight)
     {
        pd->container = spotlight;
        pd->layout = efl_add(EFL_CANVAS_LAYOUT_CLASS, pd->container);
        efl_event_callback_array_add(pd->container, spotlight_resized(), pd->layout);

        if (elm_widget_theme_object_set(pd->container, pd->layout,
                                       "spotlight",
                                       "indicator_holder",
                                       elm_widget_theme_style_get(pd->container)) == EFL_UI_THEME_APPLY_ERROR_GENERIC)
          CRI("Failed to set layout!");
        efl_canvas_group_member_add(pd->container, pd->layout);
        efl_gfx_entity_geometry_set(pd->layout, efl_gfx_entity_geometry_get(pd->container));

        pd->indicator = efl_add(EFL_UI_BOX_CLASS, pd->container);
        efl_ui_widget_internal_set(pd->indicator, EINA_TRUE);
        efl_ui_layout_orientation_set(pd->indicator, EFL_UI_LAYOUT_ORIENTATION_HORIZONTAL);
        efl_content_set(efl_part(pd->layout, "efl.indicator"), pd->indicator);

        for (int i = 0; i < efl_content_count(pd->container); ++i)
          {
             Eo *subobj = efl_pack_content_get(pd->container, i);
             _add_item(obj, pd);
             efl_gfx_stack_above(pd->layout, subobj);
          }
     }
}

EOLIAN static void
_efl_ui_spotlight_icon_indicator_efl_ui_spotlight_indicator_content_add(Eo *obj, Efl_Ui_Spotlight_Icon_Indicator_Data *pd, Efl_Gfx_Entity *subobj EINA_UNUSED, int index EINA_UNUSED)
{
   _add_item(obj, pd);
   _flush_position(obj, pd);
   efl_gfx_stack_above(pd->layout, subobj);
}

EOLIAN static void
_efl_ui_spotlight_icon_indicator_efl_ui_spotlight_indicator_content_del(Eo *obj, Efl_Ui_Spotlight_Icon_Indicator_Data *pd, Efl_Gfx_Entity *subobj EINA_UNUSED, int index EINA_UNUSED)
{
   efl_del(efl_pack_content_get(pd->indicator, 0));
   _flush_position(obj, pd);
}

EOLIAN static void
_efl_ui_spotlight_icon_indicator_efl_ui_spotlight_indicator_position_update(Eo *obj EINA_UNUSED, Efl_Ui_Spotlight_Icon_Indicator_Data *pd, double position)
{
   pd->last_position = position;
   _flush_position(obj, pd);
}

EOLIAN static void
_efl_ui_spotlight_icon_indicator_efl_object_destructor(Eo *obj EINA_UNUSED, Efl_Ui_Spotlight_Icon_Indicator_Data *pd)
{
   if (pd->layout)
     efl_del(pd->layout);
   if (pd->indicator)
     efl_del(pd->indicator);

   efl_destructor(efl_super(obj, EFL_UI_SPOTLIGHT_ICON_INDICATOR_CLASS));
}


#include "efl_ui_spotlight_icon_indicator.eo.c"
