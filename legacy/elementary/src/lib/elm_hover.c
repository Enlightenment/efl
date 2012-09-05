#include <Elementary.h>
#include "elm_priv.h"
#include "elm_widget_hover.h"

EAPI const char ELM_HOVER_SMART_NAME[] = "elm_hover";

#ifndef MAX
# define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif

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

static const char SIG_CLICKED[] = "clicked";
static const char SIG_SMART_LOCATION_CHANGED[] = "smart,changed";
static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_CLICKED, ""},
   {SIG_SMART_LOCATION_CHANGED, ""},
   {NULL, NULL}
};

EVAS_SMART_SUBCLASS_NEW
  (ELM_HOVER_SMART_NAME, _elm_hover, Elm_Hover_Smart_Class,
  Elm_Layout_Smart_Class, elm_layout_smart_class_get, _smart_callbacks);

static void
_parent_move_cb(void *data,
                Evas *e __UNUSED__,
                Evas_Object *obj __UNUSED__,
                void *event_info __UNUSED__)
{
   elm_layout_sizing_eval(data);
}

static void
_parent_resize_cb(void *data,
                  Evas *e __UNUSED__,
                  Evas_Object *obj __UNUSED__,
                  void *event_info __UNUSED__)
{
   elm_layout_sizing_eval(data);
}

static void
_parent_show_cb(void *data __UNUSED__,
                Evas *e __UNUSED__,
                Evas_Object *obj __UNUSED__,
                void *event_info __UNUSED__)
{
}

static void
_parent_hide_cb(void *data,
                Evas *e __UNUSED__,
                Evas_Object *obj __UNUSED__,
                void *event_info __UNUSED__)
{
   evas_object_hide(data);
}

static void
_parent_del_cb(void *data,
               Evas *e __UNUSED__,
               Evas_Object *obj __UNUSED__,
               void *event_info __UNUSED__)
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

static Eina_Bool
_elm_hover_smart_theme(Evas_Object *obj)
{
   ELM_HOVER_DATA_GET(obj, sd);

   if (!ELM_WIDGET_CLASS(_elm_hover_parent_sc)->theme(obj)) return EINA_FALSE;

   if (sd->smt_sub) _elm_hover_smt_sub_re_eval(obj);

   elm_layout_sizing_eval(obj);

   if (evas_object_visible_get(obj)) _hov_show_do(obj);

   return EINA_TRUE;
}

static void
_elm_hover_smart_sizing_eval(Evas_Object *obj)
{
   Evas_Coord ofs_x, x = 0, y = 0, w = 0, h = 0, x2 = 0,
              y2 = 0, w2 = 0, h2 = 0;

   ELM_HOVER_DATA_GET(obj, sd);

   if (sd->on_del) return;

   if (sd->parent) evas_object_geometry_get(sd->parent, &x, &y, &w, &h);
   evas_object_geometry_get(obj, &x2, &y2, &w2, &h2);

   if (elm_widget_mirrored_get(obj)) ofs_x = w - (x2 - x) - w2;
   else ofs_x = x2 - x;

   evas_object_move(ELM_WIDGET_DATA(sd)->resize_obj, x, y);
   evas_object_resize(ELM_WIDGET_DATA(sd)->resize_obj, w, h);
   evas_object_size_hint_min_set(sd->offset, ofs_x, y2 - y);
   evas_object_size_hint_min_set(sd->size, w2, h2);
}

static void
_on_smt_sub_changed(void *data,
                    Evas *e __UNUSED__,
                    Evas_Object *obj __UNUSED__,
                    void *event_info __UNUSED__)
{
   _elm_hover_smt_sub_re_eval(data);
}

static Eina_Bool
_elm_hover_smart_sub_object_add(Evas_Object *obj,
                                Evas_Object *sobj)
{
   ELM_HOVER_DATA_GET(obj, sd);

   if (!ELM_WIDGET_CLASS(_elm_hover_parent_sc)->sub_object_add(obj, sobj))
     return EINA_FALSE;

   if (sd->smt_sub && sd->smt_sub->obj == sobj)
     evas_object_event_callback_add
       (sobj, EVAS_CALLBACK_CHANGED_SIZE_HINTS, _on_smt_sub_changed, obj);

   return EINA_TRUE;
}

static Eina_Bool
_elm_hover_smart_sub_object_del(Evas_Object *obj,
                                Evas_Object *sobj)
{
   ELM_HOVER_DATA_GET(obj, sd);

   if (!ELM_WIDGET_CLASS(_elm_hover_parent_sc)->sub_object_del(obj, sobj))
     return EINA_FALSE;

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
_elm_hover_subs_del(Elm_Hover_Smart_Data *sd)
{
   ELM_HOVER_PARTS_FOREACH
   {
      if (sd->subs[i].obj)
        {
           evas_object_del(sd->subs[i].obj);
           sd->subs[i].obj = NULL;
        }
   }
}

static Eina_Bool
_elm_hover_smart_content_set(Evas_Object *obj,
                             const char *swallow,
                             Evas_Object *content)
{
   ELM_HOVER_CHECK(obj) EINA_FALSE;
   ELM_HOVER_DATA_GET(obj, sd);

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

   if (!ELM_CONTAINER_CLASS(_elm_hover_parent_sc)->content_set
         (obj, swallow, content))
     return EINA_FALSE;

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

static Evas_Object *
_elm_hover_smart_content_get(const Evas_Object *obj,
                             const char *swallow)
{
   ELM_HOVER_CHECK(obj) NULL;

   ELM_HOVER_DATA_GET(obj, sd);

   if (!swallow) return NULL;

   if (!strcmp(swallow, "smart"))
     return ELM_CONTAINER_CLASS(_elm_hover_parent_sc)->content_get
              (obj, sd->smt_sub->swallow);

   return ELM_CONTAINER_CLASS(_elm_hover_parent_sc)->content_get(obj, swallow);
}

static Evas_Object *
_elm_hover_smart_content_unset(Evas_Object *obj,
                               const char *swallow)
{
   ELM_HOVER_CHECK(obj) NULL;

   ELM_HOVER_DATA_GET(obj, sd);

   if (!swallow) return NULL;

   if (!strcmp(swallow, "smart"))
     return ELM_CONTAINER_CLASS(_elm_hover_parent_sc)->content_unset
              (obj, sd->smt_sub->swallow);

   return ELM_CONTAINER_CLASS(_elm_hover_parent_sc)->content_unset
            (obj, swallow);

   return NULL;
}

static void
_target_del_cb(void *data,
               Evas *e __UNUSED__,
               Evas_Object *obj __UNUSED__,
               void *event_info __UNUSED__)
{
   ELM_HOVER_DATA_GET(data, sd);

   sd->target = NULL;
}

static void
_target_move_cb(void *data,
                Evas *e __UNUSED__,
                Evas_Object *obj __UNUSED__,
                void *event_info __UNUSED__)
{
   elm_layout_sizing_eval(data);
   _elm_hover_smt_sub_re_eval(data);
}

static void
_hov_dismiss_cb(void *data,
                Evas_Object *obj __UNUSED__,
                const char *emission __UNUSED__,
                const char *source __UNUSED__)
{
   evas_object_hide(data);
   evas_object_smart_callback_call(data, SIG_CLICKED, NULL);
}

static void
_elm_hover_smart_add(Evas_Object *obj)
{
   unsigned int i;

   EVAS_SMART_DATA_ALLOC(obj, Elm_Hover_Smart_Data);

   ELM_WIDGET_CLASS(_elm_hover_parent_sc)->base.add(obj);

   for (i = 0; i < sizeof(priv->subs) / sizeof(priv->subs[0]); i++)
     priv->subs[i].swallow = _content_aliases[i].alias;

   elm_layout_theme_set(obj, "hover", "base", elm_widget_style_get(obj));
   elm_layout_signal_callback_add
     (obj, "elm,action,dismiss", "", _hov_dismiss_cb, obj);

   priv->offset = evas_object_rectangle_add(evas_object_evas_get(obj));
   evas_object_pass_events_set(priv->offset, EINA_TRUE);
   evas_object_color_set(priv->offset, 0, 0, 0, 0);

   priv->size = evas_object_rectangle_add(evas_object_evas_get(obj));
   evas_object_pass_events_set(priv->size, EINA_TRUE);
   evas_object_color_set(priv->size, 0, 0, 0, 0);

   elm_layout_content_set(obj, "elm.swallow.offset", priv->offset);
   elm_layout_content_set(obj, "elm.swallow.size", priv->size);

   elm_widget_can_focus_set(obj, EINA_TRUE);
}

static void
_elm_hover_smart_del(Evas_Object *obj)
{
   ELM_HOVER_DATA_GET(obj, sd);

   sd->on_del = EINA_TRUE;

   if (evas_object_visible_get(obj))
     evas_object_smart_callback_call(obj, SIG_CLICKED, NULL);

   elm_hover_target_set(obj, NULL);

   _elm_hover_parent_detach(obj);
   sd->parent = NULL;

   ELM_WIDGET_CLASS(_elm_hover_parent_sc)->base.del(obj);
}

static void
_elm_hover_smart_move(Evas_Object *obj,
                      Evas_Coord x,
                      Evas_Coord y)
{
   ELM_WIDGET_CLASS(_elm_hover_parent_sc)->base.move(obj, x, y);

   elm_layout_sizing_eval(obj);
}

static void
_elm_hover_smart_resize(Evas_Object *obj,
                        Evas_Coord w,
                        Evas_Coord h)
{
   ELM_WIDGET_CLASS(_elm_hover_parent_sc)->base.resize(obj, w, h);

   elm_layout_sizing_eval(obj);
}

static void
_elm_hover_smart_show(Evas_Object *obj)
{
   ELM_WIDGET_CLASS(_elm_hover_parent_sc)->base.show(obj);

   _hov_show_do(obj);
}

static void
_elm_hover_smart_hide(Evas_Object *obj)
{
   ELM_HOVER_DATA_GET(obj, sd);

   ELM_WIDGET_CLASS(_elm_hover_parent_sc)->base.hide(obj);

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
_elm_hover_smart_parent_set(Evas_Object *obj,
                            Evas_Object *parent)
{
   elm_hover_parent_set(obj, parent);

   elm_layout_sizing_eval(obj);
}

static void
_elm_hover_smart_set_user(Elm_Hover_Smart_Class *sc)
{
   ELM_WIDGET_CLASS(sc)->base.add = _elm_hover_smart_add;
   ELM_WIDGET_CLASS(sc)->base.del = _elm_hover_smart_del;
   ELM_WIDGET_CLASS(sc)->base.move = _elm_hover_smart_move;
   ELM_WIDGET_CLASS(sc)->base.resize = _elm_hover_smart_resize;
   ELM_WIDGET_CLASS(sc)->base.show = _elm_hover_smart_show;
   ELM_WIDGET_CLASS(sc)->base.hide = _elm_hover_smart_hide;

   ELM_WIDGET_CLASS(sc)->parent_set = _elm_hover_smart_parent_set;
   ELM_WIDGET_CLASS(sc)->sub_object_add = _elm_hover_smart_sub_object_add;
   ELM_WIDGET_CLASS(sc)->sub_object_del = _elm_hover_smart_sub_object_del;
   ELM_WIDGET_CLASS(sc)->theme = _elm_hover_smart_theme;

   /* not a 'focus chain manager' */
   ELM_WIDGET_CLASS(sc)->focus_next = NULL;
   ELM_WIDGET_CLASS(sc)->focus_direction = NULL;

   ELM_CONTAINER_CLASS(sc)->content_set = _elm_hover_smart_content_set;
   ELM_CONTAINER_CLASS(sc)->content_get = _elm_hover_smart_content_get;
   ELM_CONTAINER_CLASS(sc)->content_unset = _elm_hover_smart_content_unset;

   ELM_LAYOUT_CLASS(sc)->sizing_eval = _elm_hover_smart_sizing_eval;

   ELM_LAYOUT_CLASS(sc)->content_aliases = _content_aliases;
}

EAPI const Elm_Hover_Smart_Class *
elm_hover_smart_class_get(void)
{
   static Elm_Hover_Smart_Class _sc =
     ELM_HOVER_SMART_CLASS_INIT_NAME_VERSION(ELM_HOVER_SMART_NAME);
   static const Elm_Hover_Smart_Class *class = NULL;
   Evas_Smart_Class *esc = (Evas_Smart_Class *)&_sc;

   if (class)
     return class;

   _elm_hover_smart_set(&_sc);
   esc->callbacks = _smart_callbacks;
   class = &_sc;

   return class;
}

EAPI Evas_Object *
elm_hover_add(Evas_Object *parent)
{
   Evas_Object *obj;

   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);

   obj = elm_widget_add(_elm_hover_smart_class_new(), parent);
   if (!obj) return NULL;

   if (!elm_widget_sub_object_add(parent, obj))
     ERR("could not add %p as sub object of %p", obj, parent);

   return obj;
}

EAPI void
elm_hover_target_set(Evas_Object *obj,
                     Evas_Object *target)
{
   ELM_HOVER_CHECK(obj);
   ELM_HOVER_DATA_GET(obj, sd);

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
   ELM_HOVER_DATA_GET(obj, sd);

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
   ELM_HOVER_DATA_GET(obj, sd);

   return sd->target;
}

EAPI Evas_Object *
elm_hover_parent_get(const Evas_Object *obj)
{
   ELM_HOVER_CHECK(obj) NULL;
   ELM_HOVER_DATA_GET(obj, sd);

   return sd->parent;
}

EAPI const char *
elm_hover_best_content_location_get(const Evas_Object *obj,
                                    Elm_Hover_Axis pref_axis)
{
   Evas_Coord spc_l, spc_r, spc_t, spc_b;

   ELM_HOVER_CHECK(obj) NULL;
   ELM_HOVER_DATA_GET(obj, sd);

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
        if (spc_t > spc_r) return (_HOV_TOP)->swallow;
        else if (spc_b > spc_r)
          return (_HOV_BOTTOM)->swallow;

        return (_HOV_RIGHT)->swallow;
     }

   if (spc_t > spc_r) return (_HOV_TOP)->swallow;
   else if (spc_b > spc_r)
     return (_HOV_BOTTOM)->swallow;

   return (_HOV_LEFT)->swallow;
}

EAPI void
elm_hover_dismiss(Evas_Object *obj)
{
   ELM_HOVER_CHECK(obj);

   elm_layout_signal_emit(obj, "elm,action,dismiss", "");
}
