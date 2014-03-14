#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>

#include "elm_priv.h"
#include "elm_widget_hover.h"

EAPI Eo_Op ELM_OBJ_HOVER_BASE_ID = EO_NOOP;

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
_elm_hover_left_space_calc(Elm_Hover_Smart_Data *sd,
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
_elm_hover_smart_content_location_get(Elm_Hover_Smart_Data *sd,
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

static void
_elm_hover_smart_theme(Eo *obj, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;
   Eina_Bool int_ret;
   Elm_Hover_Smart_Data *sd = _pd;

   eo_do_super(obj, MY_CLASS, elm_obj_widget_theme_apply(&int_ret));
   if (!int_ret) return;

   if (sd->smt_sub) _elm_hover_smt_sub_re_eval(obj);

   elm_layout_sizing_eval(obj);

   if (evas_object_visible_get(obj)) _hov_show_do(obj);

   if (ret) *ret = EINA_TRUE;
}

static void
_elm_hover_smart_sizing_eval(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Evas_Coord ofs_x, x = 0, y = 0, w = 0, h = 0, x2 = 0,
              y2 = 0, w2 = 0, h2 = 0;

   Elm_Hover_Smart_Data *sd = _pd;

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

static void
_elm_hover_smart_sub_object_add(Eo *obj, void *_pd, va_list *list)
{
   Evas_Object *sobj = va_arg(*list, Evas_Object *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;
   Eina_Bool int_ret;
   Elm_Hover_Smart_Data *sd = _pd;

   if (evas_object_data_get(sobj, "elm-parent") == obj)
     goto end;

   eo_do_super(obj, MY_CLASS, elm_obj_widget_sub_object_add(sobj, &int_ret));
   if (!int_ret) return;

   if (sd->smt_sub && sd->smt_sub->obj == sobj)
     evas_object_event_callback_add
       (sobj, EVAS_CALLBACK_CHANGED_SIZE_HINTS, _on_smt_sub_changed, obj);

end:
   if (ret) *ret = EINA_TRUE;
}

static void
_elm_hover_smart_sub_object_del(Eo *obj, void *_pd, va_list *list)
{
   Evas_Object *sobj = va_arg(*list, Evas_Object *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;
   Eina_Bool int_ret;
   Elm_Hover_Smart_Data *sd = _pd;

   eo_do_super(obj, MY_CLASS, elm_obj_widget_sub_object_del(sobj, &int_ret));
   if (!int_ret) return;

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

   if (ret) *ret = EINA_TRUE;
}

static void
_elm_hover_subs_del(Elm_Hover_Smart_Data *sd)
{
   ELM_HOVER_PARTS_FOREACH
     ELM_SAFE_FREE(sd->subs[i].obj, evas_object_del);
}

static void
_elm_hover_smart_content_set(Eo *obj, void *_pd, va_list *list)
{
   const char *swallow= va_arg(*list, const char *);
   Evas_Object *content = va_arg(*list, Evas_Object *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;
   Eina_Bool int_ret;
   Elm_Hover_Smart_Data *sd = _pd;

   if (!swallow) return;

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
   if (!int_ret) return;

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
   if (ret) *ret = EINA_TRUE;
}

static void
_elm_hover_smart_content_get(Eo *obj, void *_pd, va_list *list)
{
   const char *swallow = va_arg(*list, const char *);
   Evas_Object **ret = va_arg(*list, Evas_Object **);
   Elm_Hover_Smart_Data *sd = _pd;
   *ret = NULL;

   if (!swallow) return;

   if (!strcmp(swallow, "smart"))
      eo_do_super(obj, MY_CLASS, elm_obj_container_content_get(sd->smt_sub->swallow, ret));
   else
      eo_do_super(obj, MY_CLASS, elm_obj_container_content_get(swallow, ret));
}

static void
_elm_hover_smart_content_unset(Eo *obj, void *_pd, va_list *list)
{
   const char *swallow = va_arg(*list, const char *);
   Evas_Object **ret = va_arg(*list, Evas_Object **);
   if (ret) *ret = NULL;
   Evas_Object *int_ret;
   Elm_Hover_Smart_Data *sd = _pd;

   if (!swallow) return;

   if (!strcmp(swallow, "smart"))
      eo_do_super(obj, MY_CLASS, elm_obj_container_content_unset
            (sd->smt_sub->swallow, &int_ret));
   else
      eo_do_super(obj, MY_CLASS, elm_obj_container_content_unset
            (swallow, &int_ret));
   if (ret) *ret = int_ret;
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

static void
_elm_hover_smart_add(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_Hover_Smart_Data *priv = _pd;
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

static void
_elm_hover_smart_del(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_Hover_Smart_Data *sd = _pd;

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

static void
_elm_hover_smart_move(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Evas_Coord x = va_arg(*list, Evas_Coord);
   Evas_Coord y = va_arg(*list, Evas_Coord);
   eo_do_super(obj, MY_CLASS, evas_obj_smart_move(x, y));

   elm_layout_sizing_eval(obj);
}

static void
_elm_hover_smart_resize(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Evas_Coord w = va_arg(*list, Evas_Coord);
   Evas_Coord h = va_arg(*list, Evas_Coord);
   eo_do_super(obj, MY_CLASS, evas_obj_smart_resize(w, h));

   elm_layout_sizing_eval(obj);
}

static void
_elm_hover_smart_show(Eo *obj, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   eo_do_super(obj, MY_CLASS, evas_obj_smart_show());

   _hov_show_do(obj);
}

static void
_elm_hover_smart_hide(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_Hover_Smart_Data *sd = _pd;
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

static void
_elm_hover_smart_content_aliases_get(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   const Elm_Layout_Part_Alias_Description **aliases = va_arg(*list, const Elm_Layout_Part_Alias_Description **);
   *aliases = _content_aliases;
}

EAPI Evas_Object *
elm_hover_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   Evas_Object *obj = eo_add(MY_CLASS, parent);
   eo_unref(obj);
   return obj;
}

static void
_constructor(Eo *obj, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   eo_do_super(obj, MY_CLASS, eo_constructor());
   eo_do(obj,
         evas_obj_type_set(MY_CLASS_NAME_LEGACY),
         evas_obj_smart_callbacks_descriptions_set(_smart_callbacks, NULL));
}

EAPI void
elm_hover_target_set(Evas_Object *obj,
                     Evas_Object *target)
{
   ELM_HOVER_CHECK(obj);
   eo_do(obj, elm_obj_hover_target_set(target));
}

static void
_elm_hover_target_set(Eo *obj, void *_pd, va_list *list)
{
   Evas_Object *target = va_arg(*list, Evas_Object *);
   Elm_Hover_Smart_Data *sd = _pd;

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

static void
_elm_hover_smart_parent_set(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Evas_Object *parent = va_arg(*list, Evas_Object *);
   Elm_Hover_Smart_Data *sd = _pd;

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

EAPI Evas_Object *
elm_hover_target_get(const Evas_Object *obj)
{
   ELM_HOVER_CHECK(obj) NULL;
   Evas_Object *ret = NULL;
   eo_do((Eo *) obj, elm_obj_hover_target_get(&ret));
   return ret;
}

static void
_elm_hover_target_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Evas_Object **ret = va_arg(*list, Evas_Object **);
   Elm_Hover_Smart_Data *sd = _pd;
   *ret = sd->target;
}

EAPI Evas_Object *
elm_hover_parent_get(const Evas_Object *obj)
{
   ELM_HOVER_CHECK(obj) NULL;
   Evas_Object *ret = NULL;
   eo_do((Eo *) obj, elm_obj_widget_parent_get(&ret));
   return ret;
}

static void
_elm_hover_smart_parent_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Evas_Object **ret = va_arg(*list, Evas_Object **);
   Elm_Hover_Smart_Data *sd = _pd;
   *ret = sd->parent;
}

EAPI const char *
elm_hover_best_content_location_get(const Evas_Object *obj,
                                    Elm_Hover_Axis pref_axis)
{
   ELM_HOVER_CHECK(obj) NULL;
   const char *ret = NULL;
   eo_do((Eo *) obj, elm_obj_hover_best_content_location_get(pref_axis, &ret));
   return ret;
}

static void
_elm_hover_best_content_location_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Evas_Coord spc_l, spc_r, spc_t, spc_b;

   Elm_Hover_Axis pref_axis = va_arg(*list, Elm_Hover_Axis);
   const char **ret = va_arg(*list, const char **);
   Elm_Hover_Smart_Data *sd = _pd;

   _elm_hover_left_space_calc(sd, &spc_l, &spc_t, &spc_r, &spc_b);

   if (pref_axis == ELM_HOVER_AXIS_HORIZONTAL)
     {
        if (spc_l < spc_r) *ret = (_HOV_RIGHT)->swallow;
        else *ret = (_HOV_LEFT)->swallow;
        return;
     }
   else if (pref_axis == ELM_HOVER_AXIS_VERTICAL)
     {
        if (spc_t < spc_b) *ret = (_HOV_BOTTOM)->swallow;
        else *ret = (_HOV_TOP)->swallow;
        return;
     }

   if (spc_l < spc_r)
     {
        if (spc_t > spc_r)
           *ret = (_HOV_TOP)->swallow;
        else if (spc_b > spc_r)
           *ret = (_HOV_BOTTOM)->swallow;
        else
           *ret = (_HOV_RIGHT)->swallow;
        return;
     }

   if (spc_t > spc_r)
      *ret = (_HOV_TOP)->swallow;
   else if (spc_b > spc_r)
      *ret = (_HOV_BOTTOM)->swallow;
   else
      *ret = (_HOV_LEFT)->swallow;
}

EAPI void
elm_hover_dismiss(Evas_Object *obj)
{
   ELM_HOVER_CHECK(obj);
   eo_do(obj, elm_obj_hover_dismiss());
}

static void
_elm_hover_dismiss(Eo *obj, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   elm_layout_signal_emit(obj, "elm,action,dismiss", ""); // XXX: for compat
   elm_layout_signal_emit(obj, "elm,action,dismiss", "elm");
}

static void
_elm_hover_smart_focus_next_manager_is(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   *ret = EINA_TRUE;
}

static void
_elm_hover_smart_focus_direction_manager_is(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   *ret = EINA_TRUE;
}

static void
_class_constructor(Eo_Class *klass)
{
      const Eo_Op_Func_Description func_desc[] = {
           EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_CONSTRUCTOR), _constructor),

           EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_ADD), _elm_hover_smart_add),
           EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_DEL), _elm_hover_smart_del),
           EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_RESIZE), _elm_hover_smart_resize),
           EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_MOVE), _elm_hover_smart_move),
           EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_SHOW), _elm_hover_smart_show),
           EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_HIDE), _elm_hover_smart_hide),

           EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_THEME_APPLY), _elm_hover_smart_theme),
           EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_SUB_OBJECT_ADD), _elm_hover_smart_sub_object_add),
           EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_SUB_OBJECT_DEL), _elm_hover_smart_sub_object_del),
           EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_PARENT_SET), _elm_hover_smart_parent_set),
           EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_PARENT_GET), _elm_hover_smart_parent_get),
           EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_FOCUS_NEXT_MANAGER_IS), _elm_hover_smart_focus_next_manager_is),
           EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_FOCUS_DIRECTION_MANAGER_IS), _elm_hover_smart_focus_direction_manager_is),

           EO_OP_FUNC(ELM_OBJ_CONTAINER_ID(ELM_OBJ_CONTAINER_SUB_ID_CONTENT_SET), _elm_hover_smart_content_set),
           EO_OP_FUNC(ELM_OBJ_CONTAINER_ID(ELM_OBJ_CONTAINER_SUB_ID_CONTENT_GET), _elm_hover_smart_content_get),
           EO_OP_FUNC(ELM_OBJ_CONTAINER_ID(ELM_OBJ_CONTAINER_SUB_ID_CONTENT_UNSET), _elm_hover_smart_content_unset),

           EO_OP_FUNC(ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_SIZING_EVAL), _elm_hover_smart_sizing_eval),
           EO_OP_FUNC(ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_CONTENT_ALIASES_GET), _elm_hover_smart_content_aliases_get),

           EO_OP_FUNC(ELM_OBJ_HOVER_ID(ELM_OBJ_HOVER_SUB_ID_TARGET_SET), _elm_hover_target_set),
           EO_OP_FUNC(ELM_OBJ_HOVER_ID(ELM_OBJ_HOVER_SUB_ID_TARGET_GET), _elm_hover_target_get),
           EO_OP_FUNC(ELM_OBJ_HOVER_ID(ELM_OBJ_HOVER_SUB_ID_BEST_CONTENT_LOCATION_GET), _elm_hover_best_content_location_get),
           EO_OP_FUNC(ELM_OBJ_HOVER_ID(ELM_OBJ_HOVER_SUB_ID_DISMISS), _elm_hover_dismiss),
           EO_OP_FUNC_SENTINEL
      };
      eo_class_funcs_set(klass, func_desc);

   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}

static const Eo_Op_Description op_desc[] = {
     EO_OP_DESCRIPTION(ELM_OBJ_HOVER_SUB_ID_TARGET_SET, "Sets the target object for the hover."),
     EO_OP_DESCRIPTION(ELM_OBJ_HOVER_SUB_ID_TARGET_GET, "Get the target object for the hover."),
     EO_OP_DESCRIPTION(ELM_OBJ_HOVER_SUB_ID_BEST_CONTENT_LOCATION_GET, "Returns the best swallow location for content in the hover."),
     EO_OP_DESCRIPTION(ELM_OBJ_HOVER_SUB_ID_DISMISS, "Dismiss a hover object."),
     EO_OP_DESCRIPTION_SENTINEL
};

static const Eo_Class_Description class_desc = {
     EO_VERSION,
     MY_CLASS_NAME,
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(&ELM_OBJ_HOVER_BASE_ID, op_desc, ELM_OBJ_HOVER_SUB_ID_LAST),
     NULL,
     sizeof(Elm_Hover_Smart_Data),
     _class_constructor,
     NULL
};

EO_DEFINE_CLASS(elm_obj_hover_class_get, &class_desc, ELM_OBJ_LAYOUT_CLASS, EVAS_SMART_CLICKABLE_INTERFACE, NULL);
