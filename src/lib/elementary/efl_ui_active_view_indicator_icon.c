#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_PACK_LAYOUT_PROTECTED

#include <Elementary.h>
#include "elm_priv.h"

typedef struct {
   Efl_Ui_Box *indicator;
   Efl_Ui_Active_View_Container *container;
   double last_position;
} Efl_Ui_Active_View_Indicator_Icon_Data;

static void
_flush_state(Eo *item, double val)
{
   Eina_Value v = eina_value_double_init(val);

    efl_layout_signal_message_send(item, 1, v);
}

static void
_add_item(Eo *obj EINA_UNUSED, Efl_Ui_Active_View_Indicator_Icon_Data *pd)
{
   Efl_Canvas_Layout *item;

   item = efl_add(EFL_CANVAS_LAYOUT_CLASS, pd->indicator);
   elm_widget_theme_object_set(pd->indicator, item,
                               "pager", "indicator", "default");
   efl_gfx_hint_align_set(item, 0.5, 0.5);
   efl_gfx_hint_weight_set(item, 0, 0);
   efl_gfx_hint_fill_set(item, 0, 0);
   efl_pack_end(pd->indicator, item);
   _flush_state(item, 0.0);
}

static void
_flush_position(Eo *obj EINA_UNUSED, Efl_Ui_Active_View_Indicator_Icon_Data *pd)
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

EOLIAN static void
_efl_ui_active_view_indicator_icon_efl_ui_active_view_indicator_bind(Eo *obj, Efl_Ui_Active_View_Indicator_Icon_Data *pd, Efl_Ui_Active_View_Container *active_view)
{
   if (active_view)
     {
        pd->container = active_view;
        pd->indicator = efl_add(EFL_UI_BOX_CLASS, active_view);
        efl_ui_widget_internal_set(pd->indicator, EINA_TRUE);
        efl_content_set(efl_part(pd->container, "efl.indicator"), pd->indicator);
        efl_gfx_entity_visible_set(pd->indicator, EINA_TRUE);
        efl_ui_layout_orientation_set(pd->indicator, EFL_UI_LAYOUT_ORIENTATION_HORIZONTAL);
        efl_gfx_arrangement_content_padding_set(pd->indicator, 15, 15, EINA_TRUE);

        for (int i = 0; i < efl_content_count(pd->container); ++i)
          {
             _add_item(obj, pd);
          }
     }
}

EOLIAN static void
_efl_ui_active_view_indicator_icon_efl_ui_active_view_indicator_content_add(Eo *obj, Efl_Ui_Active_View_Indicator_Icon_Data *pd, Efl_Gfx_Entity *subobj EINA_UNUSED, int index EINA_UNUSED)
{
   _add_item(obj, pd);
   _flush_position(obj, pd);
}

EOLIAN static void
_efl_ui_active_view_indicator_icon_efl_ui_active_view_indicator_content_del(Eo *obj, Efl_Ui_Active_View_Indicator_Icon_Data *pd, Efl_Gfx_Entity *subobj EINA_UNUSED, int index EINA_UNUSED)
{
   efl_del(efl_pack_content_get(pd->indicator, 0));
   _flush_position(obj, pd);
}

EOLIAN static void
_efl_ui_active_view_indicator_icon_efl_ui_active_view_indicator_position_update(Eo *obj EINA_UNUSED, Efl_Ui_Active_View_Indicator_Icon_Data *pd, double position)
{
   pd->last_position = position;
   _flush_position(obj, pd);
}

#include "efl_ui_active_view_indicator_icon.eo.c"
