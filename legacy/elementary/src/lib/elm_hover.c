#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>

#include "elm_priv.h"
#include "elm_widget_hover.h"

#define MY_CLASS ELM_OBJ_HOVER_CLASS

#define MY_CLASS_NAME "Elm_Hover"
#define MY_CLASS_NAME_LEGACY "elm_hover"

#define ELM_HOVER_PARTS_FOREACH unsigned int i; \
  for (i = 0; i < sizeof(sd->subs) / sizeof(sd->subs[0]); i++)

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

   if (sd->parent) evas_object_geometry_get(sd->parent, &x, &y, &w, &h);
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

   evas_object_size_hint_min_get(sd->smt_sub->obj, &c_w, &c_h);
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
   elm_layout_content_unset(obj, sd->smt_sub->swallow);

   sub = sd->smt_sub->obj;

   sd->smt_sub->obj = NULL;

   sd->smt_sub =
     _elm_hover_smart_content_location_get(sd, spc_l, spc_t, spc_r, spc_b);

   sd->smt_sub->obj = sub;

   if (sd->smt_sub != prev)
     evas_object_smart_callback_call
       (obj, SIG_SMART_LOCATION_CHANGED, (void *)sd->smt_sub->swallow);

   if (elm_widget_mirrored_get(obj))
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

EOLIAN static Eina_Bool
_elm_hover_elm_widget_theme_apply(Eo *obj, Elm_Hover_Data *sd)
{
   Eina_Bool int_ret;
   eo_do_super(obj, MY_CLASS, elm_obj_widget_theme_apply(&int_ret));
   if (!int_ret) return EINA_FALSE;

   if (sd->smt_sub) _elm_hover_smt_sub_re_eval(obj);

   elm_layout_sizing_eval(obj);

   if (evas_object_visible_get(obj)) _hov_show_do(obj);

   return EINA_TRUE;
}

EOLIAN static void
_elm_hover_elm_layout_sizing_eval(Eo *obj, Elm_Hover_Data *sd)
{
   Evas_Coord ofs_x, x = 0, y = 0, w = 0, h = 0, x2 = 0,
              y2 = 0, w2 = 0, h2 = 0;


   if (sd->on_del) return;

   if (sd->parent) evas_object_geometry_get(sd->parent, &x, &y, &w, &h);
   evas_object_geometry_get(obj, &x2, &y2, &w2, &h2);

   if (elm_widget_mirrored_get(obj)) ofs_x = w - (x2 - x) - w2;
   else ofs_x = x2 - x;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   evas_object_move(wd->resize_obj, x, y);
   evas_object_resize(wd->resize_obj, w, h);
   evas_object_size_hint_min_set(sd->offset, ofs_x, y2 - y);
   evas_object_size_hint_min_set(sd->size, w2, h2);
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
_elm_hover_elm_widget_sub_object_add(Eo *obj, Elm_Hover_Data *sd, Evas_Object *sobj)
{
   Eina_Bool int_ret;

   if (evas_object_data_get(sobj, "elm-parent") == obj) return EINA_TRUE;

   eo_do_super(obj, MY_CLASS, elm_obj_widget_sub_object_add(sobj, &int_ret));
   if (!int_ret) return EINA_FALSE;

   if (sd->smt_sub && sd->smt_sub->obj == sobj)
     evas_object_event_callback_add
       (sobj, EVAS_CALLBACK_CHANGED_SIZE_HINTS, _on_smt_sub_changed, obj);

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_elm_hover_elm_widget_sub_object_del(Eo *obj, Elm_Hover_Data *sd, Evas_Object *sobj)
{
   Eina_Bool int_ret;

   eo_do_super(obj, MY_CLASS, elm_obj_widget_sub_object_del(sobj, &int_ret));
   if (!int_ret) return EINA_FALSE;

   if (sd->smt_sub && sd->smt_sub->obj == sobj)
     {
        evas_object_event_callback_del_full
          (sd->smt_sub->obj, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
          _on_smt_sub_changed, obj);

        sd->smt_sub->obj = NULL;
        sd->smt_sub = NULL;
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
}

EOLIAN static Eina_Bool
_elm_hover_elm_container_content_set(Eo *obj, Elm_Hover_Data *sd, const char *swallow, Evas_Object *content)
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

   eo_do_super(obj, MY_CLASS, elm_obj_container_content_set(swallow, content, &int_ret));
   if (!int_ret) return EINA_FALSE;

   if (strstr(swallow, "elm.swallow.slot."))
     swallow += sizeof("elm.swallow.slot.");

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

EOLIAN static Evas_Object*
_elm_hover_elm_container_content_get(Eo *obj, Elm_Hover_Data *sd, const char *swallow)
{
   Evas_Object *ret;
   ret = NULL;

   if (!swallow) return ret;

   if (!strcmp(swallow, "smart"))
      eo_do_super(obj, MY_CLASS, elm_obj_container_content_get(sd->smt_sub->swallow, &ret));
   else
      eo_do_super(obj, MY_CLASS, elm_obj_container_content_get(swallow, &ret));

   return ret;
}

EOLIAN static Evas_Object*
_elm_hover_elm_container_content_unset(Eo *obj, Elm_Hover_Data *sd, const char *swallow)
{
   Evas_Object *int_ret;

   if (!swallow) return NULL;

   if (!strcmp(swallow, "smart"))
      eo_do_super(obj, MY_CLASS, elm_obj_container_content_unset
            (sd->smt_sub->swallow, &int_ret));
   else
      eo_do_super(obj, MY_CLASS, elm_obj_container_content_unset
            (swallow, &int_ret));
   return int_ret;
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
_hov_dismiss_cb(void *data,
                Evas_Object *obj EINA_UNUSED,
                const char *emission EINA_UNUSED,
                const char *source EINA_UNUSED)
{
   evas_object_hide(data);
   evas_object_smart_callback_call(data, SIG_CLICKED, NULL);
   evas_object_smart_callback_call(data, SIG_DISMISSED, NULL);
}

EOLIAN static void
_elm_hover_evas_smart_add(Eo *obj, Elm_Hover_Data *priv)
{
   unsigned int i;

   eo_do_super(obj, MY_CLASS, evas_obj_smart_add());
   elm_widget_sub_object_parent_add(obj);

   for (i = 0; i < sizeof(priv->subs) / sizeof(priv->subs[0]); i++)
     priv->subs[i].swallow = _content_aliases[i].alias;

   if (!elm_layout_theme_set(obj, "hover", "base", elm_widget_style_get(obj)))
     CRI("Failed to set layout!");

   elm_layout_signal_callback_add
     (obj, "elm,action,dismiss", "*", _hov_dismiss_cb, obj);

   priv->offset = evas_object_rectangle_add(evas_object_evas_get(obj));
   evas_object_pass_events_set(priv->offset, EINA_TRUE);
   evas_object_color_set(priv->offset, 0, 0, 0, 0);

   priv->size = evas_object_rectangle_add(evas_object_evas_get(obj));
   evas_object_pass_events_set(priv->size, EINA_TRUE);
   evas_object_color_set(priv->size, 0, 0, 0, 0);

   elm_layout_content_set(obj, "elm.swallow.offset", priv->offset);
   elm_layout_content_set(obj, "elm.swallow.size", priv->size);

   elm_widget_can_focus_set(obj, EINA_FALSE);
}

EOLIAN static void
_elm_hover_evas_smart_del(Eo *obj, Elm_Hover_Data *sd)
{

   sd->on_del = EINA_TRUE;

   if (evas_object_visible_get(obj))
     {
        evas_object_smart_callback_call(obj, SIG_CLICKED, NULL);
        evas_object_smart_callback_call(obj, SIG_DISMISSED, NULL);
     }

   elm_hover_target_set(obj, NULL);

   _elm_hover_parent_detach(obj);
   sd->parent = NULL;

   eo_do_super(obj, MY_CLASS, evas_obj_smart_del());
}

EOLIAN static void
_elm_hover_evas_smart_move(Eo *obj, Elm_Hover_Data *_pd EINA_UNUSED, Evas_Coord x, Evas_Coord y)
{
   eo_do_super(obj, MY_CLASS, evas_obj_smart_move(x, y));

   elm_layout_sizing_eval(obj);
}

EOLIAN static void
_elm_hover_evas_smart_resize(Eo *obj, Elm_Hover_Data *_pd EINA_UNUSED, Evas_Coord w, Evas_Coord h)
{
   eo_do_super(obj, MY_CLASS, evas_obj_smart_resize(w, h));

   elm_layout_sizing_eval(obj);
}

EOLIAN static void
_elm_hover_evas_smart_show(Eo *obj, Elm_Hover_Data *_pd EINA_UNUSED)
{
   eo_do_super(obj, MY_CLASS, evas_obj_smart_show());

   _hov_show_do(obj);
}

EOLIAN static void
_elm_hover_evas_smart_hide(Eo *obj, Elm_Hover_Data *sd)
{
   eo_do_super(obj, MY_CLASS, evas_obj_smart_hide());

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

EOLIAN static const Elm_Layout_Part_Alias_Description*
_elm_hover_elm_layout_content_aliases_get(Eo *obj EINA_UNUSED, Elm_Hover_Data *_pd EINA_UNUSED)
{
   return _content_aliases;
}

EAPI Evas_Object *
elm_hover_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   Evas_Object *obj = eo_add(MY_CLASS, parent);
   eo_unref(obj);
   return obj;
}

EOLIAN static void
_elm_hover_eo_base_constructor(Eo *obj, Elm_Hover_Data *_pd EINA_UNUSED)
{
   eo_do_super(obj, MY_CLASS, eo_constructor());
   eo_do(obj,
         evas_obj_type_set(MY_CLASS_NAME_LEGACY),
         evas_obj_smart_callbacks_descriptions_set(_smart_callbacks, NULL));
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
        elm_widget_hover_object_set(sd->target, NULL);
     }

   sd->target = target;
   if (sd->target)
     {
        evas_object_event_callback_add
          (sd->target, EVAS_CALLBACK_DEL, _target_del_cb, obj);
        evas_object_event_callback_add
          (sd->target, EVAS_CALLBACK_MOVE, _target_move_cb, obj);
        elm_widget_hover_object_set(target, obj);
        elm_layout_sizing_eval(obj);
     }
}

EAPI void
elm_hover_parent_set(Evas_Object *obj,
                     Evas_Object *parent)
{
   ELM_HOVER_CHECK(obj);
   eo_do(obj, elm_obj_widget_parent_set(parent));
}

EOLIAN static void
_elm_hover_elm_widget_parent_set(Eo *obj, Elm_Hover_Data *sd, Evas_Object *parent)
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

EOLIAN static Evas_Object*
_elm_hover_target_get(Eo *obj EINA_UNUSED, Elm_Hover_Data *sd)
{
   return sd->target;
}

EAPI Evas_Object *
elm_hover_parent_get(const Evas_Object *obj)
{
   ELM_HOVER_CHECK(obj) NULL;
   Evas_Object *ret = NULL;
   eo_do((Eo *) obj, elm_obj_widget_parent_get(&ret));
   return ret;
}

EOLIAN static Evas_Object*
_elm_hover_elm_widget_parent_get(Eo *obj EINA_UNUSED, Elm_Hover_Data *sd)
{
   return sd->parent;
}

EOLIAN static const char*
_elm_hover_best_content_location_get(Eo *obj EINA_UNUSED, Elm_Hover_Data *sd, Elm_Hover_Axis pref_axis)
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
        if (spc_t < spc_b) return (_HOV_BOTTOM)->swallow;
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
   elm_layout_signal_emit(obj, "elm,action,dismiss", ""); // XXX: for compat
   elm_layout_signal_emit(obj, "elm,action,dismiss", "elm");
}

EOLIAN static Eina_Bool
_elm_hover_elm_widget_focus_next_manager_is(Eo *obj EINA_UNUSED, Elm_Hover_Data *_pd EINA_UNUSED)
{
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_elm_hover_elm_widget_focus_direction_manager_is(Eo *obj EINA_UNUSED, Elm_Hover_Data *_pd EINA_UNUSED)
{
   return EINA_TRUE;
}


EOLIAN static void
_elm_hover_class_constructor(Eo_Class *klass)
{
   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}

#include "elm_hover.eo.c"
