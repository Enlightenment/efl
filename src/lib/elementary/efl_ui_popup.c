#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_PART_PROTECTED
#define EFL_UI_POPUP_PROTECTED
#define EFL_PART_PROTECTED

#include <Elementary.h>

#include "elm_priv.h"
#include "efl_ui_popup_private.h"
#include "efl_ui_popup_part_backwall.eo.h"
#include "elm_part_helper.h"

#define MY_CLASS EFL_UI_POPUP_CLASS
#define MY_CLASS_NAME "Efl.Ui.Popup"

static const char PART_NAME_BACKWALL[] = "backwall";

static void
_backwall_clicked_cb(void *data,
                     Eo *o EINA_UNUSED,
                     const char *emission EINA_UNUSED,
                     const char *source EINA_UNUSED)
{
   Eo *obj = data;
   efl_event_callback_call(obj, EFL_UI_POPUP_EVENT_BACKWALL_CLICKED, NULL);
}

EOLIAN static void
_efl_ui_popup_efl_gfx_entity_position_set(Eo *obj, Efl_Ui_Popup_Data *pd EINA_UNUSED, Eina_Position2D pos)
{
   pd->align = EFL_UI_POPUP_ALIGN_NONE;
   efl_gfx_entity_position_set(efl_super(obj, MY_CLASS), pos);
}

static void
_calc_align(Eo *obj)
{
   Efl_Ui_Popup_Data *pd = efl_data_scope_get(obj, MY_CLASS);

   Eina_Rect p_geom = efl_gfx_entity_geometry_get(pd->win_parent);

   Eina_Rect o_geom = efl_gfx_entity_geometry_get(obj);

   Evas_Coord pw, ph;
   pw = p_geom.w;
   ph = p_geom.h;

   Evas_Coord ow, oh;
   ow = o_geom.w;
   oh = o_geom.h;

   switch (pd->align)
     {
      case EFL_UI_POPUP_ALIGN_CENTER:
         efl_gfx_entity_position_set(efl_super(obj, MY_CLASS), EINA_POSITION2D((pw - ow ) / 2, (ph - oh) / 2));
         break;
      case EFL_UI_POPUP_ALIGN_LEFT:
         efl_gfx_entity_position_set(efl_super(obj, MY_CLASS), EINA_POSITION2D(0, (ph - oh) / 2));
         break;
      case EFL_UI_POPUP_ALIGN_RIGHT:
         efl_gfx_entity_position_set(efl_super(obj, MY_CLASS), EINA_POSITION2D(pw - ow, (ph - oh) / 2));
         break;
      case EFL_UI_POPUP_ALIGN_TOP:
         efl_gfx_entity_position_set(efl_super(obj, MY_CLASS), EINA_POSITION2D((pw - ow) / 2, 0));
         break;
      case EFL_UI_POPUP_ALIGN_BOTTOM:
         efl_gfx_entity_position_set(efl_super(obj, MY_CLASS), EINA_POSITION2D((pw - ow) / 2, ph - oh));
         break;
      default:
         break;
     }
}

EOLIAN static void
_efl_ui_popup_efl_gfx_entity_size_set(Eo *obj, Efl_Ui_Popup_Data *pd EINA_UNUSED, Eina_Size2D size)
{
   efl_gfx_entity_size_set(efl_super(obj, MY_CLASS), size);

   elm_layout_sizing_eval(obj);
}

static void
_parent_geom_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Eo *obj = data;

   EFL_UI_POPUP_DATA_GET_OR_RETURN(obj, pd);

   //Add align calc only
   Eina_Bool needs_size_calc = pd->needs_size_calc;
   elm_layout_sizing_eval(obj);
   pd->needs_size_calc = needs_size_calc;
}

EOLIAN static void
_efl_ui_popup_efl_ui_widget_widget_parent_set(Eo *obj, Efl_Ui_Popup_Data *pd EINA_UNUSED, Eo *parent EINA_UNUSED)
{
   pd->win_parent = efl_provider_find(obj, EFL_UI_WIN_CLASS);
   if (!pd->win_parent)
     {
        ERR("Cannot find window parent");
        return;
     }

   Eina_Rect p_geom = efl_gfx_entity_geometry_get(pd->win_parent);

   efl_gfx_entity_position_set(pd->backwall, EINA_POSITION2D(p_geom.x, p_geom.y));
   efl_gfx_entity_size_set(pd->backwall, EINA_SIZE2D(p_geom.w, p_geom.h));

   efl_event_callback_add(pd->win_parent, EFL_GFX_ENTITY_EVENT_RESIZE, _parent_geom_cb, obj);
   efl_event_callback_add(pd->win_parent, EFL_GFX_ENTITY_EVENT_MOVE, _parent_geom_cb, obj);
}

EOLIAN static void
_efl_ui_popup_align_set(Eo *obj EINA_UNUSED, Efl_Ui_Popup_Data *pd, Efl_Ui_Popup_Align type)
{
   pd->align = type;

   //Add align calc only
   Eina_Bool needs_size_calc = pd->needs_size_calc;
   elm_layout_sizing_eval(obj);
   pd->needs_size_calc = needs_size_calc;
}

EOLIAN static Efl_Ui_Popup_Align
_efl_ui_popup_align_get(const Eo *obj EINA_UNUSED, Efl_Ui_Popup_Data *pd)
{
   return pd->align;
}

EOLIAN static void
_efl_ui_popup_popup_size_set(Eo *obj, Efl_Ui_Popup_Data *pd EINA_UNUSED, Eina_Size2D size)
{
   efl_gfx_entity_size_set(obj, size);
}

EOLIAN static Eina_Size2D
_efl_ui_popup_popup_size_get(const Eo *obj, Efl_Ui_Popup_Data *pd EINA_UNUSED)
{
   return efl_gfx_entity_size_get(obj);
}

static Eina_Bool
_timer_cb(void *data)
{
   Eo *popup = data;
   efl_event_callback_call(popup, EFL_UI_POPUP_EVENT_TIMEOUT, NULL);
   efl_del(popup);

   return ECORE_CALLBACK_CANCEL;
}

static void
_timer_del(Efl_Ui_Popup_Data *pd)
{
   if (pd->timer)
     {
        ecore_timer_del(pd->timer);
        pd->timer = NULL;
     }
}

static void
_timer_init(Eo *obj, Efl_Ui_Popup_Data *pd)
{
   if (pd->timeout > 0.0)
     pd->timer = ecore_timer_add(pd->timeout, _timer_cb, obj);
}

EOLIAN static void
_efl_ui_popup_efl_gfx_entity_visible_set(Eo *obj, Efl_Ui_Popup_Data *pd, Eina_Bool v)
{
   if (_evas_object_intercept_call(obj, EVAS_OBJECT_INTERCEPT_CB_VISIBLE, 0, v))
     return;

   efl_gfx_entity_visible_set(efl_super(obj, MY_CLASS), v);

   if (v)
     {
        _timer_del(pd);
        _timer_init(obj, pd);
     }
}

EOLIAN static void
_efl_ui_popup_timeout_set(Eo *obj, Efl_Ui_Popup_Data *pd, double time)
{
   if (time < 0.0)
     time = 0.0;

   pd->timeout = time;

   _timer_del(pd);

   if (efl_gfx_entity_visible_get(obj))
     _timer_init(obj, pd);
}

EOLIAN static double
_efl_ui_popup_timeout_get(const Eo *obj EINA_UNUSED, Efl_Ui_Popup_Data *pd)
{
   return pd->timeout;
}

EOLIAN static Eo *
_efl_ui_popup_efl_object_constructor(Eo *obj, Efl_Ui_Popup_Data *pd)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, NULL);

   if (!elm_widget_theme_klass_get(obj))
     elm_widget_theme_klass_set(obj, "popup");
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   efl_canvas_object_type_set(obj, MY_CLASS_NAME);

   elm_widget_sub_object_parent_add(obj);

   elm_widget_can_focus_set(obj, EINA_TRUE);
   if (!elm_widget_theme_object_set(obj, wd->resize_obj,
                                       elm_widget_theme_klass_get(obj),
                                       elm_widget_theme_element_get(obj),
                                       elm_widget_theme_style_get(obj)))
     CRI("Failed to set layout!");

   pd->backwall = edje_object_add(evas_object_evas_get(obj));
   elm_widget_element_update(obj, pd->backwall, PART_NAME_BACKWALL);
   evas_object_smart_member_add(pd->backwall, obj);
   evas_object_stack_below(pd->backwall, wd->resize_obj);

   edje_object_signal_callback_add(pd->backwall, "efl,action,clicked", "*",
                                   _backwall_clicked_cb, obj);

   pd->align = EFL_UI_POPUP_ALIGN_CENTER;

   return obj;
}

EOLIAN static void
_efl_ui_popup_efl_object_destructor(Eo *obj, Efl_Ui_Popup_Data *pd)
{
   ELM_SAFE_DEL(pd->backwall);

   efl_event_callback_del(pd->win_parent, EFL_GFX_ENTITY_EVENT_RESIZE, _parent_geom_cb,
                          obj);
   efl_event_callback_del(pd->win_parent, EFL_GFX_ENTITY_EVENT_MOVE, _parent_geom_cb,
                          obj);

   efl_destructor(efl_super(obj, MY_CLASS));
}

static void
_sizing_eval(Eo *obj)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   Evas_Coord minw = -1, minh = -1;

   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   edje_object_size_min_restricted_calc
      (wd->resize_obj, &minw, &minh, minw, minh);
   efl_gfx_size_hint_min_set(obj, EINA_SIZE2D(minw, minh));

   Eina_Size2D size = efl_gfx_entity_size_get(obj);

   Eina_Size2D new_size;
   new_size.w = (minw > size.w ? minw : size.w);
   new_size.h = (minh > size.h ? minh : size.h);
   efl_gfx_entity_size_set(obj, new_size);
}

EOLIAN static void
_efl_ui_popup_elm_layout_sizing_eval(Eo *obj, Efl_Ui_Popup_Data *pd)
{
   if (pd->needs_group_calc) return;
   pd->needs_group_calc = EINA_TRUE;

   /* These flags can be modified by sub classes not to calculate size or align
    * their super classes.
    * e.g. Efl.Ui.Popup.Alert.Scroll class sets the flag as follows not to
    *      calculate size by its super class.
    *
    *      ppd->needs_size_calc = EINA_FALSE;
    *      efl_canvas_group_calculate(efl_super(obj, MY_CLASS));
    */
   pd->needs_size_calc = EINA_TRUE;
   pd->needs_align_calc = EINA_TRUE;

   evas_object_smart_changed(obj);
}

EOLIAN static void
_efl_ui_popup_efl_canvas_group_group_calculate(Eo *obj, Efl_Ui_Popup_Data *pd)
{
   /* When elm_layout_sizing_eval() is called, just flag is set instead of size
    * calculation.
    * The actual size calculation is done here when the object is rendered to
    * avoid duplicate size calculations. */
   if (pd->needs_group_calc)
     {
        if (pd->needs_size_calc)
          {
             _sizing_eval(obj);
             pd->needs_size_calc = EINA_FALSE;
          }
        if (pd->needs_align_calc)
          {
             _calc_align(obj);
             pd->needs_align_calc = EINA_FALSE;
          }

        Eina_Rect p_geom = efl_gfx_entity_geometry_get(pd->win_parent);

        efl_gfx_entity_position_set(pd->backwall, EINA_POSITION2D(0, 0));
        efl_gfx_entity_size_set(pd->backwall, EINA_SIZE2D(p_geom.w, p_geom.h));

        pd->needs_group_calc = EINA_FALSE;
     }
}

/* Standard widget overrides */

ELM_PART_CONTENT_DEFAULT_IMPLEMENT(efl_ui_popup, Efl_Ui_Popup_Data)

EOLIAN static Eo *
_efl_ui_popup_efl_part_part_get(const Eo *obj, Efl_Ui_Popup_Data *_pd EINA_UNUSED, const char *part)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(part, NULL);

   if (eina_streq(part, PART_NAME_BACKWALL))
     return ELM_PART_IMPLEMENT(EFL_UI_POPUP_PART_BACKWALL_CLASS, obj, part);

   return efl_part_get(efl_super(obj, MY_CLASS), part);
}

EOLIAN static void
_efl_ui_popup_part_backwall_efl_canvas_object_repeat_events_set(Eo *obj, void *_pd EINA_UNUSED, Eina_Bool repeat)
{
   Elm_Part_Data *pd = efl_data_scope_get(obj, EFL_UI_WIDGET_PART_CLASS);
   Efl_Ui_Popup_Data *sd = efl_data_scope_get(pd->obj, EFL_UI_POPUP_CLASS);

   efl_canvas_object_repeat_events_set(sd->backwall, repeat);
}

EOLIAN static Eina_Bool
_efl_ui_popup_part_backwall_efl_canvas_object_repeat_events_get(const Eo *obj, void *_pd EINA_UNUSED)
{
   Elm_Part_Data *pd = efl_data_scope_get(obj, EFL_UI_WIDGET_PART_CLASS);
   Efl_Ui_Popup_Data *sd = efl_data_scope_get(pd->obj, EFL_UI_POPUP_CLASS);

   return efl_canvas_object_repeat_events_get(sd->backwall);
}

EOLIAN static Eina_Bool
_efl_ui_popup_part_backwall_efl_file_file_set(Eo *obj, void *_pd EINA_UNUSED, const char *file, const char *group)
{
   Elm_Part_Data *pd = efl_data_scope_get(obj, EFL_UI_WIDGET_PART_CLASS);
   Efl_Ui_Popup_Data *sd = efl_data_scope_get(pd->obj, EFL_UI_POPUP_CLASS);

   Eo *prev_obj = edje_object_part_swallow_get(sd->backwall, "efl.content");
   if (prev_obj)
     {
        edje_object_signal_emit(sd->backwall, "efl,state,content,unset", "efl");
        edje_object_part_unswallow(sd->backwall, prev_obj);
        efl_del(prev_obj);
     }

   Eo *image = elm_image_add(pd->obj);
   Eina_Bool ret = elm_image_file_set(image, file, group);
   if (!ret)
     {
        efl_del(image);
        return EINA_FALSE;
     }
   edje_object_part_swallow(sd->backwall, "efl.content", image);
   edje_object_signal_emit(sd->backwall, "efl,state,content,set", "efl");

   return EINA_TRUE;
}

#include "efl_ui_popup_part_backwall.eo.c"

/* Efl.Part end */

/* Internal EO APIs and hidden overrides */

#define EFL_UI_POPUP_EXTRA_OPS \
   ELM_LAYOUT_SIZING_EVAL_OPS(efl_ui_popup)

#include "efl_ui_popup.eo.c"
