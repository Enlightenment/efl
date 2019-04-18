#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define ELM_WIDGET_PROTECTED
#define EFL_ACCESS_OBJECT_PROTECTED
#define EFL_ACCESS_WIDGET_ACTION_PROTECTED
#define ELM_LAYOUT_PROTECTED
#define EFL_PART_PROTECTED

#include <Elementary.h>

#include "elm_priv.h"
#include "elm_widget_hover.h"

#include "elm_hover_part.eo.h"
#include "elm_part_helper.h"

#define MY_CLASS ELM_HOVER_CLASS
#define MY_CLASS_PFX elm_hover

#define MY_CLASS_NAME "Elm_Hover"
#define MY_CLASS_NAME_LEGACY "elm_hover"

#define ELM_HOVER_PARTS_FOREACH                                         \
  for (unsigned int i = 0; i < sizeof(sd->subs) / sizeof(sd->subs[0]); i++)

#define _HOV_LEFT               (&(sd->subs[0]))
#define _HOV_TOP_LEFT           (&(sd->subs[1]))
#define _HOV_TOP                (&(sd->subs[2]))
#define _HOV_TOP_RIGHT          (&(sd->subs[2]))
#define _HOV_RIGHT              (&(sd->subs[4]))
#define _HOV_BOTTOM_RIGHT       (&(sd->subs[5]))
#define _HOV_BOTTOM             (&(sd->subs[6]))
#define _HOV_BOTTOM_LEFT        (&(sd->subs[7]))
#define _HOV_MIDDLE             (&(sd->subs[8]))

const Elm_Layout_Part_Alias_Description _content_aliases[] =
{
   {"left", "elm.swallow.slot.left"},
   {"top-left", "elm.swallow.slot.top-left"},
   {"top", "elm.swallow.slot.top"},
   {"top-right", "elm.swallow.slot.top-right"},
   {"right", "elm.swallow.slot.right"},
   {"bottom-right", "elm.swallow.slot.bottom-right"},
   {"bottom", "elm.swallow.slot.bottom"},
   {"bottom-left", "elm.swallow.slot.bottom-left"},
   {"middle", "elm.swallow.slot.middle"},
   {NULL, NULL}
};

const Elm_Layout_Part_Alias_Description _content_aliases_main_menu_submenu[] =
{
   {"bottom", "elm.swallow.slot.bottom"},
   {NULL, NULL}
};

#define ELM_PRIV_HOVER_SIGNALS(cmd) \
   cmd(SIG_CLICKED, "clicked", "") \
   cmd(SIG_DISMISSED, "dismissed", "") \
   cmd(SIG_SMART_LOCATION_CHANGED, "smart,changed", "")

ELM_PRIV_HOVER_SIGNALS(ELM_PRIV_STATIC_VARIABLE_DECLARE);

static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   ELM_PRIV_HOVER_SIGNALS(ELM_PRIV_SMART_CALLBACKS_DESC)
   {SIG_LAYOUT_FOCUSED, ""}, /**< handled by elm_layout */
   {SIG_LAYOUT_UNFOCUSED, ""}, /**< handled by elm_layout */
   {NULL, NULL}
};
#undef ELM_PRIV_HOVER_SIGNALS

static void
_parent_move_cb(void *data,
                Evas *e EINA_UNUSED,
                Evas_Object *obj EINA_UNUSED,
                void *event_info EINA_UNUSED)
{
   elm_layout_sizing_eval(data);
}

static void
_parent_resize_cb(void *data,
                  Evas *e EINA_UNUSED,
                  Evas_Object *obj EINA_UNUSED,
                  void *event_info EINA_UNUSED)
{
   elm_layout_sizing_eval(data);
}

static void
_parent_show_cb(void *data EINA_UNUSED,
                Evas *e EINA_UNUSED,
                Evas_Object *obj EINA_UNUSED,
                void *event_info EINA_UNUSED)
{
}

static void
_parent_hide_cb(void *data,
                Evas *e EINA_UNUSED,
                Evas_Object *obj EINA_UNUSED,
                void *event_info EINA_UNUSED)
{
   evas_object_hide(data);
}

static void
_parent_del_cb(void *data,
               Evas *e EINA_UNUSED,
               Evas_Object *obj EINA_UNUSED,
               void *event_info EINA_UNUSED)
{
   elm_hover_parent_set(data, NULL);
   elm_layout_sizing_eval(data);
}

static void
_elm_hover_parent_detach(Evas_Object *obj)
{
   ELM_HOVER_DATA_GET(obj, sd);

   if (sd->parent)
     {
        evas_object_event_callback_del_full
          (sd->parent, EVAS_CALLBACK_MOVE, _parent_move_cb, obj);
        evas_object_event_callback_del_full
          (sd->parent, EVAS_CALLBACK_RESIZE, _parent_resize_cb, obj);
        evas_object_event_callback_del_full
          (sd->parent, EVAS_CALLBACK_SHOW, _parent_show_cb, obj);
        evas_object_event_callback_del_full
          (sd->parent, EVAS_CALLBACK_HIDE, _parent_hide_cb, obj);
        evas_object_event_callback_del_full
          (sd->parent, EVAS_CALLBACK_DEL, _parent_del_cb, obj);
     }
}

static void
_elm_hover_left_space_calc(Elm_Hover_Data *sd,
                           Evas_Coord *spc_l,
                           Evas_Coord *spc_t,
                           Evas_Coord *spc_r,
                           Evas_Coord *spc_b)
{
   Evas_Coord x = 0, y = 0, w = 0, h = 0, x2 = 0, y2 = 0, w2 = 0, h2 = 0;

   if (sd->parent)
     {
        evas_object_geometry_get(sd->parent, &x, &y, &w, &h);
        if (efl_isa(sd->parent, EFL_UI_WIN_CLASS))
          {
             x = 0;
             y = 0;
          }
     }
   if (sd->target) evas_object_geometry_get(sd->target, &x2, &y2, &w2, &h2);

   *spc_l = x2 - x;
   *spc_r = (x + w) - (x2 + w2);
   if (*spc_l < 0) *spc_l = 0;
   if (*spc_r < 0) *spc_r = 0;

   *spc_t = y2 - y;
   *spc_b = (y + h) - (y2 + h2);
   if (*spc_t < 0) *spc_t = 0;
   if (*spc_b < 0) *spc_b = 0;
}

static Content_Info *
_elm_hover_smart_content_location_get(Elm_Hover_Data *sd,
                                      Evas_Coord spc_l,
                                      Evas_Coord spc_t,
                                      Evas_Coord spc_r,
                                      Evas_Coord spc_b)
{
   Evas_Coord c_w = 0, c_h = 0, mid_w, mid_h;
   int max;

   evas_object_size_hint_combined_min_get(sd->smt_sub->obj, &c_w, &c_h);
   mid_w = c_w / 2;
   mid_h = c_h / 2;

   if (spc_l > spc_r) goto left;

   max = MAX(spc_t, spc_r);
   max = MAX(max, spc_b);

   if (max == spc_t)
     {
        if (mid_w > spc_l) return _HOV_TOP_RIGHT;

        return _HOV_TOP;
     }

   if (max == spc_r)
     {
        if (mid_h > spc_t) return _HOV_BOTTOM_RIGHT;
        else if (mid_h > spc_b)
          return _HOV_TOP_RIGHT;

        return _HOV_RIGHT;
     }

   if (mid_h > spc_l)
     return _HOV_BOTTOM_RIGHT;

   return _HOV_BOTTOM;

left:
   max = MAX(spc_t, spc_l);
   max = MAX(max, spc_b);

   if (max == spc_t)
     {
        if (mid_w > spc_r) return _HOV_TOP_LEFT;

        return _HOV_TOP;
     }

   if (max == spc_l)
     {
        if (mid_h > spc_t) return _HOV_BOTTOM_LEFT;
        else if (mid_h > spc_b)
          return _HOV_TOP_LEFT;

        return _HOV_LEFT;
     }

   if (mid_h > spc_r) return _HOV_BOTTOM_LEFT;

   return _HOV_BOTTOM;
}

static void
_elm_hover_smt_sub_re_eval(Evas_Object *obj)
{
   Evas_Coord spc_l, spc_r, spc_t, spc_b;
   Content_Info *prev;
   Evas_Object *sub;
   char buf[1024];

   ELM_HOVER_DATA_GET(obj, sd);

   if (!sd->smt_sub) return;
   prev = sd->smt_sub;

   _elm_hover_left_space_calc(sd, &spc_l, &spc_t, &spc_r, &spc_b);

   sub = sd->smt_sub->obj;

   sd->smt_sub =
     _elm_hover_smart_content_location_get(sd, spc_l, spc_t, spc_r, spc_b);

   sd->smt_sub->obj = sub;

   if (sd->smt_sub != prev)
     efl_event_callback_legacy_call
       (obj, ELM_HOVER_EVENT_SMART_CHANGED, (void *)sd->smt_sub->swallow);

   if (efl_ui_mirrored_get(obj))
     {
        if (sd->smt_sub == _HOV_BOTTOM_LEFT) sd->smt_sub = _HOV_BOTTOM_RIGHT;
        else if (sd->smt_sub == _HOV_BOTTOM_RIGHT)
          sd->smt_sub = _HOV_BOTTOM_LEFT;
        else if (sd->smt_sub == _HOV_RIGHT)
          sd->smt_sub = _HOV_LEFT;
        else if (sd->smt_sub == _HOV_LEFT)
          sd->smt_sub = _HOV_RIGHT;
        else if (sd->smt_sub == _HOV_TOP_RIGHT)
          sd->smt_sub = _HOV_TOP_LEFT;
        else if (sd->smt_sub == _HOV_TOP_LEFT)
          sd->smt_sub = _HOV_TOP_RIGHT;
     }

   snprintf(buf, sizeof(buf), "elm.swallow.slot.%s", sd->smt_sub->swallow);
   elm_layout_content_set(obj, buf, sd->smt_sub->obj);
}

static void
_hov_show_do(Evas_Object *obj)
{
   ELM_HOVER_DATA_GET(obj, sd);

   elm_layout_signal_emit(obj, "elm,action,show", "elm");

   ELM_HOVER_PARTS_FOREACH
   {
      char buf[1024];

      if (sd->subs[i].obj)
        {
           snprintf
             (buf, sizeof(buf), "elm,action,slot,%s,show",
             sd->subs[i].swallow);

           elm_layout_signal_emit(obj, buf, "elm");
        }
   }
}

EOLIAN static Eina_Error
_elm_hover_efl_ui_widget_theme_apply(Eo *obj, Elm_Hover_Data *sd)
{
   Eina_Error int_ret = EFL_UI_THEME_APPLY_ERROR_GENERIC;
   int_ret = efl_ui_widget_theme_apply(efl_super(obj, MY_CLASS));
   if (int_ret == EFL_UI_THEME_APPLY_ERROR_GENERIC) return int_ret;

   if (sd->smt_sub) _elm_hover_smt_sub_re_eval(obj);

   elm_layout_sizing_eval(obj);

   if (evas_object_visible_get(obj)) _hov_show_do(obj);

   return int_ret;
}

EOLIAN static void
_elm_hover_elm_layout_sizing_eval(Eo *obj, Elm_Hover_Data *sd)
{
   Evas_Coord ofs_x, x = 0, y = 0, w = 0, h = 0, x2 = 0,
              y2 = 0, w2 = 0, h2 = 0;


   if (sd->on_del) return;

   if (sd->parent)
     {
        evas_object_geometry_get(sd->parent, &x, &y, &w, &h);
        if (efl_isa(sd->parent, EFL_UI_WIN_CLASS))
          {
             if (efl_canvas_object_is_frame_object_get(obj))
               evas_object_geometry_get(obj, &x, &y, NULL, NULL);
             else
               {
                  x = 0;
                  y = 0;
               }
          }
     }
   evas_object_geometry_get(obj, &x2, &y2, &w2, &h2);

   if (efl_ui_mirrored_get(obj)) ofs_x = w - (x2 - x) - w2;
   else ofs_x = x2 - x;

   if (y < 0)
     h += (-y);

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   evas_object_size_hint_min_set(sd->offset, ofs_x, y2 - y);
   evas_object_size_hint_min_set(sd->size, w2, h2);
   evas_object_geometry_set(wd->resize_obj, x, y, w, h);
}

static void
_on_smt_sub_changed(void *data,
                    Evas *e EINA_UNUSED,
                    Evas_Object *obj EINA_UNUSED,
                    void *event_info EINA_UNUSED)
{
   _elm_hover_smt_sub_re_eval(data);
}

EOLIAN static Eina_Bool
_elm_hover_efl_ui_widget_widget_sub_object_add(Eo *obj, Elm_Hover_Data *sd, Evas_Object *sobj)
{
   Eina_Bool int_ret = EINA_FALSE;

   if (evas_object_data_get(sobj, "elm-parent") == obj) return EINA_TRUE;

   int_ret = elm_widget_sub_object_add(efl_super(obj, MY_CLASS), sobj);
   if (!int_ret) return EINA_FALSE;

   if (sd->smt_sub && sd->smt_sub->obj == sobj)
     evas_object_event_callback_add
       (sobj, EVAS_CALLBACK_CHANGED_SIZE_HINTS, _on_smt_sub_changed, obj);

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_elm_hover_efl_ui_widget_widget_sub_object_del(Eo *obj, Elm_Hover_Data *sd, Evas_Object *sobj)
{
   Eina_Bool int_ret = EINA_FALSE;

   int_ret = elm_widget_sub_object_del(efl_super(obj, MY_CLASS), sobj);
   if (!int_ret) return EINA_FALSE;

   if (sd->smt_sub && sd->smt_sub->obj == sobj)
     {
        evas_object_event_callback_del_full
          (sd->smt_sub->obj, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
          _on_smt_sub_changed, obj);
     }
   else
     {
        ELM_HOVER_PARTS_FOREACH
        {
           if (sd->subs[i].obj == sobj)
             {
                sd->subs[i].obj = NULL;
                break;
             }
        }
     }

   return EINA_TRUE;
}

static void
_elm_hover_subs_del(Elm_Hover_Data *sd)
{
   ELM_HOVER_PARTS_FOREACH
     ELM_SAFE_FREE(sd->subs[i].obj, evas_object_del);
   sd->smt_sub = NULL;
}

static Eina_Bool
_elm_hover_content_set(Eo *obj, Elm_Hover_Data *sd, const char *swallow, Evas_Object *content)
{
   Eina_Bool int_ret;

   if (!swallow) return EINA_FALSE;

   if (!strcmp(swallow, "smart"))
     {
        if (sd->smt_sub)     /* already under 'smart' mode */
          {
             if (sd->smt_sub->obj != content)
               {
                  evas_object_del(sd->smt_sub->obj);
                  sd->smt_sub = _HOV_LEFT;
                  sd->smt_sub->obj = content;
               }

             if (!content)
               {
                  sd->smt_sub->obj = NULL;
                  sd->smt_sub = NULL;
               }
             else _elm_hover_smt_sub_re_eval(obj);

             goto end;
          }
        else     /* switch from pristine spots to 'smart' */
          {
             _elm_hover_subs_del(sd);
             sd->smt_sub = _HOV_LEFT;
             sd->smt_sub->obj = content;

             _elm_hover_smt_sub_re_eval(obj);

             goto end;
          }
     }

   int_ret = efl_content_set(efl_part(efl_super(obj, MY_CLASS), swallow), content);
   if (!int_ret) return EINA_FALSE;

   if (!strncmp(swallow, "elm.swallow.slot.", sizeof("elm.swallow.slot")))
     swallow += sizeof("elm.swallow.slot");

   ELM_HOVER_PARTS_FOREACH
   {
      if (!strcmp(swallow, sd->subs[i].swallow))
        {
           sd->subs[i].obj = content;
           break;
        }
   }

end:
   elm_layout_sizing_eval(obj);
   return EINA_TRUE;
}

static Evas_Object*
_elm_hover_content_get(Eo *obj, Elm_Hover_Data *sd, const char *swallow)
{
   if (!swallow) return NULL;

   if (!strcmp(swallow, "smart"))
     return efl_content_get(efl_part(efl_super(obj, MY_CLASS), sd->smt_sub->swallow));
   else
     return efl_content_get(efl_part(efl_super(obj, MY_CLASS), swallow));
}

static Evas_Object*
_elm_hover_content_unset(Eo *obj, Elm_Hover_Data *sd, const char *swallow)
{
   if (!swallow) return NULL;

   if (!strcmp(swallow, "smart"))
     return efl_content_unset(efl_part(efl_super(obj, MY_CLASS), sd->smt_sub->swallow));
   else
     return efl_content_unset(efl_part(efl_super(obj, MY_CLASS), swallow));
}

static void
_target_del_cb(void *data,
               Evas *e EINA_UNUSED,
               Evas_Object *obj EINA_UNUSED,
               void *event_info EINA_UNUSED)
{
   ELM_HOVER_DATA_GET(data, sd);

   sd->target = NULL;
}

static void
_target_move_cb(void *data,
                Evas *e EINA_UNUSED,
                Evas_Object *obj EINA_UNUSED,
                void *event_info EINA_UNUSED)
{
   elm_layout_sizing_eval(data);
   _elm_hover_smt_sub_re_eval(data);
}

static void
_hide_signals_emit(Evas_Object *obj)
{
   ELM_HOVER_DATA_GET(obj, sd);

   elm_layout_signal_emit(obj, "elm,action,hide", "elm");

   ELM_HOVER_PARTS_FOREACH
     {
        char buf[1024];

        if (sd->subs[i].obj)
          {
             snprintf(buf, sizeof(buf), "elm,action,slot,%s,hide",
                      sd->subs[i].swallow);
             elm_layout_signal_emit(obj, buf, "elm");
          }
     }
}

static void
_hov_hide_cb(void *data,
                Evas_Object *obj EINA_UNUSED,
                const char *emission EINA_UNUSED,
                const char *source EINA_UNUSED)
{
   const char *dismissstr;

   dismissstr = elm_layout_data_get(data, "dismiss");

   if (dismissstr && !strcmp(dismissstr, "on"))
     {
        evas_object_hide(data);
        efl_event_callback_legacy_call(data, ELM_HOVER_EVENT_DISMISSED, NULL);
     }
}

static void
_hov_dismiss_cb(void *data,
                Evas_Object *obj EINA_UNUSED,
                const char *emission EINA_UNUSED,
                const char *source EINA_UNUSED)
{
   const char *dismissstr;

   dismissstr = elm_layout_data_get(data, "dismiss");

   if (dismissstr && !strcmp(dismissstr, "on"))
     {
        _hide_signals_emit(data);
        efl_event_callback_legacy_call
          (data, EFL_UI_EVENT_CLICKED, NULL);
     }
   else
     {
        evas_object_hide(data);
        efl_event_callback_legacy_call
          (data, EFL_UI_EVENT_CLICKED, NULL);
        efl_event_callback_legacy_call(data, ELM_HOVER_EVENT_DISMISSED, NULL);
     } // for backward compatibility
}

EOLIAN static void
_elm_hover_efl_canvas_group_group_add(Eo *obj, Elm_Hover_Data *sd)
{
   efl_canvas_group_add(efl_super(obj, MY_CLASS));

   ELM_HOVER_PARTS_FOREACH
     sd->subs[i].swallow = _content_aliases[i].alias;

   if (!elm_layout_theme_set(obj, "hover", "base", elm_widget_style_get(obj)))
     CRI("Failed to set layout!");

   elm_layout_signal_callback_add
     (obj, "elm,action,dismiss", "*", _hov_dismiss_cb, obj);
   elm_layout_signal_callback_add
     (obj, "elm,action,hide,finished", "elm", _hov_hide_cb, obj);

   sd->offset = evas_object_rectangle_add(evas_object_evas_get(obj));
   evas_object_pass_events_set(sd->offset, EINA_TRUE);
   evas_object_color_set(sd->offset, 0, 0, 0, 0);

   sd->size = evas_object_rectangle_add(evas_object_evas_get(obj));
   evas_object_pass_events_set(sd->size, EINA_TRUE);
   evas_object_color_set(sd->size, 0, 0, 0, 0);

   elm_layout_content_set(obj, "elm.swallow.offset", sd->offset);
   elm_layout_content_set(obj, "elm.swallow.size", sd->size);

   elm_widget_can_focus_set(obj, EINA_FALSE);
}

EOLIAN static void
_elm_hover_efl_canvas_group_group_del(Eo *obj, Elm_Hover_Data *sd)
{

   sd->on_del = EINA_TRUE;

   if (evas_object_visible_get(obj))
     {
        efl_event_callback_legacy_call
          (obj, EFL_UI_EVENT_CLICKED, NULL);
        efl_event_callback_legacy_call(obj, ELM_HOVER_EVENT_DISMISSED, NULL);
     }

   elm_hover_target_set(obj, NULL);

   _elm_hover_parent_detach(obj);
   sd->parent = NULL;

   efl_canvas_group_del(efl_super(obj, MY_CLASS));
}

EOLIAN static void
_elm_hover_efl_gfx_entity_position_set(Eo *obj, Elm_Hover_Data *_pd EINA_UNUSED, Eina_Position2D pos)
{
   if (_evas_object_intercept_call(obj, EVAS_OBJECT_INTERCEPT_CB_MOVE, 0, pos.x, pos.y))
     return;

   efl_gfx_entity_position_set(efl_super(obj, MY_CLASS), pos);

   elm_layout_sizing_eval(obj);
}

EOLIAN static void
_elm_hover_efl_gfx_entity_size_set(Eo *obj, Elm_Hover_Data *_pd EINA_UNUSED, Eina_Size2D sz)
{
   if (_evas_object_intercept_call(obj, EVAS_OBJECT_INTERCEPT_CB_RESIZE, 0, sz.w, sz.h))
     return;

   efl_gfx_entity_size_set(efl_super(obj, MY_CLASS), sz);

   elm_layout_sizing_eval(obj);
}

EOLIAN static void
_elm_hover_efl_gfx_entity_visible_set(Eo *obj, Elm_Hover_Data *pd EINA_UNUSED, Eina_Bool vis)
{
   if (_evas_object_intercept_call(obj, EVAS_OBJECT_INTERCEPT_CB_VISIBLE, 0, vis))
     return;

   efl_gfx_entity_visible_set(efl_super(obj, MY_CLASS), vis);

   if (vis) _hov_show_do(obj);
   else
     {
        // for backward compatibility
        const char *dismissstr = elm_layout_data_get(obj, "dismiss");

        if (!eina_streq(dismissstr, "on"))
          _hide_signals_emit(obj);
     }
}

static const Elm_Layout_Part_Alias_Description*
_elm_hover_content_aliases_get(Eo *obj, void *_pd EINA_UNUSED)
{
   const char *style = elm_widget_style_get(obj);

   // main_menu_submenu only has a single slot "bottom"
   if (style && strstr(style, "main_menu_submenu"))
     return _content_aliases_main_menu_submenu;
   return _content_aliases;
}

EAPI Evas_Object *
elm_hover_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   return elm_legacy_add(MY_CLASS, parent);
}

static void
_parent_setup(Eo *obj, Elm_Hover_Data *sd, Evas_Object *parent)
{
   _elm_hover_parent_detach(obj);

   sd->parent = parent;
   if (sd->parent)
     {
        evas_object_event_callback_add
          (sd->parent, EVAS_CALLBACK_MOVE, _parent_move_cb, obj);
        evas_object_event_callback_add
          (sd->parent, EVAS_CALLBACK_RESIZE, _parent_resize_cb, obj);
        evas_object_event_callback_add
          (sd->parent, EVAS_CALLBACK_SHOW, _parent_show_cb, obj);
        evas_object_event_callback_add
          (sd->parent, EVAS_CALLBACK_HIDE, _parent_hide_cb, obj);
        evas_object_event_callback_add
          (sd->parent, EVAS_CALLBACK_DEL, _parent_del_cb, obj);
     }

   elm_layout_sizing_eval(obj);
}

EOLIAN static Eo *
_elm_hover_efl_object_constructor(Eo *obj, Elm_Hover_Data *pd EINA_UNUSED)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   efl_canvas_object_type_set(obj, MY_CLASS_NAME_LEGACY);
   evas_object_smart_callbacks_descriptions_set(obj, _smart_callbacks);
   efl_access_object_role_set(obj, EFL_ACCESS_ROLE_POPUP_MENU);
   legacy_child_focus_handle(obj);
   _parent_setup(obj, pd, efl_parent_get(obj));

   return obj;
}

EOLIAN static void
_elm_hover_target_set(Eo *obj, Elm_Hover_Data *sd, Evas_Object *target)
{

   if (sd->target)
     {
        evas_object_event_callback_del_full
          (sd->target, EVAS_CALLBACK_DEL, _target_del_cb, obj);
        evas_object_event_callback_del_full
          (sd->target, EVAS_CALLBACK_MOVE, _target_move_cb, obj);
        evas_object_event_callback_del_full
          (sd->target, EVAS_CALLBACK_RESIZE, _target_move_cb, obj);
        elm_widget_hover_object_set(sd->target, NULL);
     }

   sd->target = target;
   if (sd->target)
     {
        evas_object_event_callback_add
          (sd->target, EVAS_CALLBACK_DEL, _target_del_cb, obj);
        evas_object_event_callback_add
          (sd->target, EVAS_CALLBACK_MOVE, _target_move_cb, obj);
        evas_object_event_callback_add
          (sd->target, EVAS_CALLBACK_RESIZE, _target_move_cb, obj);
        elm_widget_hover_object_set(target, obj);
        elm_layout_sizing_eval(obj);
     }

}
EAPI void
elm_hover_parent_set(Evas_Object *obj,
                     Evas_Object *parent)
{
   ELM_HOVER_CHECK(obj);
   ELM_HOVER_DATA_GET(obj, sd);
   efl_ui_widget_sub_object_add(parent, obj);
   _parent_setup(obj, sd, parent);
}

EOLIAN static Evas_Object*
_elm_hover_target_get(const Eo *obj EINA_UNUSED, Elm_Hover_Data *sd)
{
   return sd->target;
}

EAPI Evas_Object *
elm_hover_parent_get(const Evas_Object *obj)
{
   ELM_HOVER_CHECK(obj) NULL;
   return efl_ui_widget_parent_get((Eo *) obj);
}

EOLIAN static const char*
_elm_hover_best_content_location_get(const Eo *obj EINA_UNUSED, Elm_Hover_Data *sd, Elm_Hover_Axis pref_axis)
{
   Evas_Coord spc_l, spc_r, spc_t, spc_b;

   _elm_hover_left_space_calc(sd, &spc_l, &spc_t, &spc_r, &spc_b);

   if (pref_axis == ELM_HOVER_AXIS_HORIZONTAL)
     {
        if (spc_l < spc_r) return (_HOV_RIGHT)->swallow;
        else return (_HOV_LEFT)->swallow;
     }
   else if (pref_axis == ELM_HOVER_AXIS_VERTICAL)
     {
        if (spc_t <= spc_b) return (_HOV_BOTTOM)->swallow;
        else return (_HOV_TOP)->swallow;
     }

   if (spc_l < spc_r)
     {
        if (spc_t > spc_r)
           return (_HOV_TOP)->swallow;
        else if (spc_b > spc_r)
           return (_HOV_BOTTOM)->swallow;
        else
           return (_HOV_RIGHT)->swallow;
     }

   if (spc_t > spc_r)
      return (_HOV_TOP)->swallow;
   else if (spc_b > spc_r)
      return (_HOV_BOTTOM)->swallow;
   else
      return (_HOV_LEFT)->swallow;

   return NULL;
}

EOLIAN static void
_elm_hover_dismiss(Eo *obj, Elm_Hover_Data *_pd EINA_UNUSED)
{
   const char *dismissstr;

   dismissstr = elm_layout_data_get(obj, "dismiss");

   if (!dismissstr || strcmp(dismissstr, "on"))
     elm_layout_signal_emit(obj, "elm,action,dismiss", ""); // XXX: for compat
   elm_layout_signal_emit(obj, "elm,action,dismiss", "elm");
}

EOLIAN static void
_elm_hover_class_constructor(Efl_Class *klass)
{
   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}

static Eina_Bool
_action_dismiss(Evas_Object *obj, const char *params EINA_UNUSED)
{
   elm_obj_hover_dismiss(obj);
   return EINA_TRUE;
}

EOLIAN const Efl_Access_Action_Data *
_elm_hover_efl_access_widget_action_elm_actions_get(const Eo *obj EINA_UNUSED, Elm_Hover_Data *pd EINA_UNUSED)
{
   static Efl_Access_Action_Data atspi_actions[] = {
          { "dismiss", NULL, NULL, _action_dismiss},
          { NULL, NULL, NULL, NULL}
   };
   return &atspi_actions[0];
}

EOLIAN static Efl_Access_State_Set
_elm_hover_efl_access_object_state_set_get(const Eo *obj, Elm_Hover_Data *pd EINA_UNUSED)
{
   Efl_Access_State_Set states;
   states = efl_access_object_state_set_get(efl_super(obj, MY_CLASS));

   STATE_TYPE_SET(states, EFL_ACCESS_STATE_TYPE_MODAL);
   return states;
}

/* Efl.Part begin */

ELM_PART_OVERRIDE(elm_hover, ELM_HOVER, Elm_Hover_Data)
ELM_PART_OVERRIDE_CONTENT_SET(elm_hover, ELM_HOVER, Elm_Hover_Data)
ELM_PART_OVERRIDE_CONTENT_GET(elm_hover, ELM_HOVER, Elm_Hover_Data)
ELM_PART_OVERRIDE_CONTENT_UNSET(elm_hover, ELM_HOVER, Elm_Hover_Data)
#include "elm_hover_part.eo.c"

/* Efl.Part end */

/* Internal EO APIs and hidden overrides */

// ELM_LAYOUT_CONTENT_ALIASES_IMPLEMENT(MY_CLASS_PFX) is overridden with an if()
// ELM_LAYOUT_CONTENT_ALIASES_OPS(MY_CLASS_PFX) somehow doesn't compile!?

#define ELM_HOVER_EXTRA_OPS \
   EFL_CANVAS_GROUP_ADD_DEL_OPS(elm_hover), \
   ELM_LAYOUT_SIZING_EVAL_OPS(elm_hover), \
   _ELM_LAYOUT_ALIASES_OPS(elm_hover, content)

#include "elm_hover_eo.c"
