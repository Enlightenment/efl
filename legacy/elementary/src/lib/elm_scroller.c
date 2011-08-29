#include <Elementary.h>
#include "elm_priv.h"

typedef struct _Widget_Data Widget_Data;

struct _Widget_Data
{
   Evas_Object *scr;
   Evas_Object *content;
   const char *widget_name, *widget_base;
   Eina_Bool min_w : 1;
   Eina_Bool min_h : 1;
   double pagerel_h, pagerel_v;
   Evas_Coord pagesize_h, pagesize_v;
};

static const char *widtype = NULL;
static void _del_hook(Evas_Object *obj);
static void _theme_hook(Evas_Object *obj);
static void _show_region_hook(void *data, Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);
static void _sub_del(void *data, Evas_Object *obj, void *event_info);
static void _on_focus_hook(void *data, Evas_Object *obj);
static Eina_Bool _event_hook(Evas_Object *obj, Evas_Object *src,
                             Evas_Callback_Type type, void *event_info);


static const char SIG_SCROLL[] = "scroll";
static const char SIG_SCROLL_ANIM_START[] = "scroll,anim,start";
static const char SIG_SCROLL_ANIM_STOP[] = "scroll,anim,stop";
static const char SIG_SCROLL_DRAG_START[] = "scroll,drag,start";
static const char SIG_SCROLL_DRAG_STOP[] = "scroll,drag,stop";
static const char SIG_EDGE_LEFT[] = "edge,left";
static const char SIG_EDGE_RIGHT[] = "edge,right";
static const char SIG_EDGE_TOP[] = "edge,top";
static const char SIG_EDGE_BOTTOM[] = "edge,bottom";
static const Evas_Smart_Cb_Description _signals[] = {
  {SIG_SCROLL, ""},
  {SIG_SCROLL_ANIM_START, ""},
  {SIG_SCROLL_ANIM_STOP, ""},
  {SIG_SCROLL_DRAG_START, ""},
  {SIG_SCROLL_DRAG_STOP, ""},
  {SIG_EDGE_LEFT, ""},
  {SIG_EDGE_RIGHT, ""},
  {SIG_EDGE_TOP, ""},
  {SIG_EDGE_BOTTOM, ""},
  {NULL, NULL}
};

static Eina_Bool
_event_hook(Evas_Object *obj, Evas_Object *src __UNUSED__, Evas_Callback_Type type, void *event_info)
{
   if (type != EVAS_CALLBACK_KEY_DOWN) return EINA_FALSE;
   Evas_Event_Key_Down *ev = event_info;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return EINA_FALSE;
   if (elm_widget_disabled_get(obj)) return EINA_FALSE;

   Evas_Coord x = 0;
   Evas_Coord y = 0;
   Evas_Coord step_x = 0;
   Evas_Coord step_y = 0;
   Evas_Coord max_x = 0;
   Evas_Coord max_y = 0;
   Evas_Coord v_w = 0;
   Evas_Coord v_h = 0;
   Evas_Coord page_x = 0;
   Evas_Coord page_y = 0;

   elm_smart_scroller_child_pos_get(wd->scr, &x, &y);
   elm_smart_scroller_step_size_get(wd->scr, &step_x, &step_y);
   elm_smart_scroller_page_size_get(wd->scr, &page_x, &page_y);
   elm_smart_scroller_child_viewport_size_get(wd->scr, &v_w, &v_h);
   elm_scroller_child_size_get(obj, &max_x, &max_y);

   if ((!strcmp(ev->keyname, "Left")) || (!strcmp(ev->keyname, "KP_Left")))
     {
        x -= step_x;
     }
   else if ((!strcmp(ev->keyname, "Right")) || (!strcmp(ev->keyname, "KP_Right")))
     {
        x += step_x;
     }
   else if ((!strcmp(ev->keyname, "Up"))  || (!strcmp(ev->keyname, "KP_Up")))
     {
        y -= step_y;
     }
   else if ((!strcmp(ev->keyname, "Down")) || (!strcmp(ev->keyname, "KP_Down")))
     {
        y += step_y;
     }
   else if ((!strcmp(ev->keyname, "Home")) || (!strcmp(ev->keyname, "KP_Home")))
     {
        y = 0;
     }
   else if ((!strcmp(ev->keyname, "End")) || (!strcmp(ev->keyname, "KP_End")))
     {
        y = max_y - v_h;
     }
   else if ((!strcmp(ev->keyname, "Prior")) || (!strcmp(ev->keyname, "KP_Prior")))
     {
        if (page_y < 0)
          y -= -(page_y * v_h) / 100;
        else
          y -= page_y;
     }
   else if ((!strcmp(ev->keyname, "Next")) || (!strcmp(ev->keyname, "KP_Next")))
     {
        if (page_y < 0)
          y += -(page_y * v_h) / 100;
        else
          y += page_y;
     }
   else return EINA_FALSE;

   ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
   elm_smart_scroller_child_pos_set(wd->scr, x, y);
   return EINA_TRUE;
}

static void
_on_focus_hook(void *data __UNUSED__, Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (elm_widget_focus_get(obj))
     {
        edje_object_signal_emit(wd->scr, "elm,action,focus", "elm");
        evas_object_focus_set(wd->scr, EINA_TRUE);
     }
   else
     {
        edje_object_signal_emit(wd->scr, "elm,action,unfocus", "elm");
        evas_object_focus_set(wd->scr, EINA_FALSE);
     }
}

static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   free(wd);
}

static void
_mirrored_set(Evas_Object *obj, Eina_Bool mirrored)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->scr)
     elm_smart_scroller_mirrored_set(wd->scr, mirrored);
}

static void
_theme_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   _elm_widget_mirrored_reload(obj);
   if (wd->scr)
     {
        Evas_Object *edj;
        const char *str;

        _mirrored_set(obj, elm_widget_mirrored_get(obj));
        elm_smart_scroller_object_theme_set(obj, wd->scr,
                                            wd->widget_name, wd->widget_base,
                                            elm_widget_style_get(obj));
        //        edje_object_scale_set(wd->scr, elm_widget_scale_get(obj) * _elm_config->scale);
        edj = elm_smart_scroller_edje_object_get(wd->scr);
        str = edje_object_data_get(edj, "focus_highlight");
        if ((str) && (!strcmp(str, "on")))
          elm_widget_highlight_in_theme_set(obj, EINA_TRUE);
        else
          elm_widget_highlight_in_theme_set(obj, EINA_FALSE);
     }
   _sizing_eval(obj);
}

static Eina_Bool
_elm_scroller_focus_next_hook(const Evas_Object *obj, Elm_Focus_Direction dir, Evas_Object **next)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Object *cur;

   if ((!wd) || (!wd->content))
     return EINA_FALSE;

   cur = wd->content;

   /* Try Focus cycle in subitem */
   if ((elm_widget_can_focus_get(cur)) || (elm_widget_child_can_focus_get(cur)))
     return elm_widget_focus_next_get(cur, dir, next);

   /* Return */
   *next = (Evas_Object *)obj;
   return !elm_widget_focus_get(obj);
}

static void
_signal_emit_hook(Evas_Object *obj, const char *emission, const char *source)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   edje_object_signal_emit(elm_smart_scroller_edje_object_get(wd->scr),
                           emission, source);
}

static void
_signal_callback_add_hook(Evas_Object *obj, const char *emission, const char *source, Edje_Signal_Cb func_cb, void *data)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   edje_object_signal_callback_add(elm_smart_scroller_edje_object_get(wd->scr),
                                   emission, source, func_cb, data);
}

static void
_signal_callback_del_hook(Evas_Object *obj, const char *emission, const char *source, Edje_Signal_Cb func_cb, void *data)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   edje_object_signal_callback_del_full(
      elm_smart_scroller_edje_object_get(wd->scr), emission, source,
      func_cb, data);
}

static void
_show_region_hook(void *data, Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(data);
   Evas_Coord x, y, w, h;
   if (!wd) return;
   elm_widget_show_region_get(obj, &x, &y, &w, &h);
   if (wd->scr)
     elm_smart_scroller_child_region_show(wd->scr, x, y, w, h);
}

static void
_focus_region_hook(Evas_Object *obj, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (wd->scr)
     elm_smart_scroller_child_region_show(wd->scr, x, y, w, h);
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord  vw = 0, vh = 0, minw = 0, minh = 0, maxw = 0, maxh = 0, w, h, vmw, vmh;
   double xw = 0.0, yw = 0.0;

   if (!wd) return;
   if (wd->content)
     {
        evas_object_size_hint_min_get(wd->content, &minw, &minh);
        evas_object_size_hint_max_get(wd->content, &maxw, &maxh);
        evas_object_size_hint_weight_get(wd->content, &xw, &yw);
     }
   if (wd->scr)
     {
        elm_smart_scroller_child_viewport_size_get(wd->scr, &vw, &vh);
        if (xw > 0.0)
          {
             if ((minw > 0) && (vw < minw)) vw = minw;
             else if ((maxw > 0) && (vw > maxw)) vw = maxw;
          }
        else if (minw > 0) vw = minw;
        if (yw > 0.0)
          {
             if ((minh > 0) && (vh < minh)) vh = minh;
             else if ((maxh > 0) && (vh > maxh)) vh = maxh;
          }
        else if (minh > 0) vh = minh;
        if (wd->content) evas_object_resize(wd->content, vw, vh);
        w = -1;
        h = -1;
        edje_object_size_min_calc(elm_smart_scroller_edje_object_get(wd->scr), &vmw, &vmh);
        if (wd->min_w) w = vmw + minw;
        if (wd->min_h) h = vmh + minh;
        evas_object_size_hint_max_get(obj, &maxw, &maxh);
        if ((maxw > 0) && (w > maxw)) w = maxw;
        if ((maxh > 0) && (h > maxh)) h = maxh;
        evas_object_size_hint_min_set(obj, w, h);
     }
}

static void
_changed_size_hints(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   _sizing_eval(data);
}

static void
_sub_del(void *data __UNUSED__, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Object *sub = event_info;

   if (!wd) return;
   if (sub == wd->content)
     {
        elm_widget_on_show_region_hook_set(wd->content, NULL, NULL);
        evas_object_event_callback_del_full (sub, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
                                             _changed_size_hints, obj);
        wd->content = NULL;
        _sizing_eval(obj);
     }
   else if (sub == wd->scr)
     wd->scr = NULL;
}

static void
_hold_on(void *data __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return;
   if (wd->scr)
     elm_smart_scroller_hold_set(wd->scr, 1);
}

static void
_hold_off(void *data __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return;
   if (wd->scr)
     elm_smart_scroller_hold_set(wd->scr, 0);
}

static void
_freeze_on(void *data __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return;
   if (wd->scr)
     elm_smart_scroller_freeze_set(wd->scr, 1);
}

static void
_freeze_off(void *data __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return;
   if (wd->scr)
     elm_smart_scroller_freeze_set(wd->scr, 0);
}

static void
_resize(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   _sizing_eval(data);
}

static void
_edge_left(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   evas_object_smart_callback_call(data, SIG_EDGE_LEFT, NULL);
}

static void
_edge_right(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   evas_object_smart_callback_call(data, SIG_EDGE_RIGHT, NULL);
}

static void
_edge_top(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   evas_object_smart_callback_call(data, SIG_EDGE_TOP, NULL);
}

static void
_edge_bottom(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   evas_object_smart_callback_call(data, SIG_EDGE_BOTTOM, NULL);
}

static void
_scroll(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   evas_object_smart_callback_call(data, SIG_SCROLL, NULL);
}

static void
_scroll_anim_start(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   evas_object_smart_callback_call(data, SIG_SCROLL_ANIM_START, NULL);
}

static void
_scroll_anim_stop(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   evas_object_smart_callback_call(data, SIG_SCROLL_ANIM_STOP, NULL);
}

static void
_scroll_drag_start(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   evas_object_smart_callback_call(data, SIG_SCROLL_DRAG_START, NULL);
}

static void
_scroll_drag_stop(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   evas_object_smart_callback_call(data, SIG_SCROLL_DRAG_STOP, NULL);
}

EAPI Evas_Object *
elm_scroller_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas *e;
   Widget_Data *wd;
   Evas_Coord minw, minh;

   ELM_WIDGET_STANDARD_SETUP(wd, Widget_Data, parent, e, obj, NULL);

   ELM_SET_WIDTYPE(widtype, "scroller");
   elm_widget_type_set(obj, "scroller");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_on_focus_hook_set(obj, _on_focus_hook, NULL);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);
   elm_widget_signal_emit_hook_set(obj, _signal_emit_hook);
   elm_widget_signal_callback_add_hook_set(obj, _signal_callback_add_hook);
   elm_widget_signal_callback_del_hook_set(obj, _signal_callback_del_hook);
   elm_widget_focus_next_hook_set(obj, _elm_scroller_focus_next_hook);
   elm_widget_can_focus_set(obj, EINA_TRUE);
   elm_widget_event_hook_set(obj, _event_hook);
   elm_widget_focus_region_hook_set(obj, _focus_region_hook);

   wd->widget_name = eina_stringshare_add("scroller");
   wd->widget_base = eina_stringshare_add("base");

   wd->scr = elm_smart_scroller_add(e);
   elm_smart_scroller_widget_set(wd->scr, obj);
   _theme_hook(obj);
   elm_widget_resize_object_set(obj, wd->scr);
   evas_object_event_callback_add(wd->scr, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
                                  _changed_size_hints, obj);

   edje_object_size_min_calc(elm_smart_scroller_edje_object_get(wd->scr), &minw, &minh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_RESIZE, _resize, obj);

   evas_object_smart_callback_add(obj, "sub-object-del", _sub_del, obj);
   evas_object_smart_callback_add(obj, "scroll-hold-on", _hold_on, obj);
   evas_object_smart_callback_add(obj, "scroll-hold-off", _hold_off, obj);
   evas_object_smart_callback_add(obj, "scroll-freeze-on", _freeze_on, obj);
   evas_object_smart_callback_add(obj, "scroll-freeze-off", _freeze_off, obj);

   evas_object_smart_callback_add(wd->scr, "edge,left", _edge_left, obj);
   evas_object_smart_callback_add(wd->scr, "edge,right", _edge_right, obj);
   evas_object_smart_callback_add(wd->scr, "edge,top", _edge_top, obj);
   evas_object_smart_callback_add(wd->scr, "edge,bottom", _edge_bottom, obj);
   evas_object_smart_callback_add(wd->scr, "scroll", _scroll, obj);
   evas_object_smart_callback_add(wd->scr, "animate,start", _scroll_anim_start, obj);
   evas_object_smart_callback_add(wd->scr, "animate,stop", _scroll_anim_stop, obj);
   evas_object_smart_callback_add(wd->scr, "drag,start", _scroll_drag_start, obj);
   evas_object_smart_callback_add(wd->scr, "drag,stop", _scroll_drag_stop, obj);

   _sizing_eval(obj);

   // TODO: convert Elementary to subclassing of Evas_Smart_Class
   // TODO: and save some bytes, making descriptions per-class and not instance!
   evas_object_smart_callbacks_descriptions_set(obj, _signals);
   _mirrored_set(obj, elm_widget_mirrored_get(obj));
   return obj;
}

Evas_Object *
_elm_scroller_edje_object_get(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return elm_smart_scroller_edje_object_get(wd->scr);
}

EAPI void
elm_scroller_content_set(Evas_Object *obj, Evas_Object *content)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->content == content) return;
   if (wd->content) evas_object_del(wd->content);
   wd->content = content;
   if (content)
     {
        elm_widget_on_show_region_hook_set(content, _show_region_hook, obj);
        elm_widget_sub_object_add(obj, content);
        if (wd->scr)
          elm_smart_scroller_child_set(wd->scr, content);
        evas_object_event_callback_add(content, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
                                       _changed_size_hints, obj);
     }
   _sizing_eval(obj);
}

EAPI Evas_Object *
elm_scroller_content_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return wd->content;
}

EAPI Evas_Object *
elm_scroller_content_unset(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Object *content;
   if (!wd) return NULL;
   if (!wd->content) return NULL;
   content = wd->content;
   elm_widget_sub_object_del(obj, wd->content);
   edje_object_part_unswallow(wd->scr, wd->content);
   wd->content = NULL;
   return content;
}

EAPI void
elm_scroller_custom_widget_base_theme_set(Evas_Object *obj, const char *widget, const char *base)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   EINA_SAFETY_ON_NULL_RETURN(widget);
   EINA_SAFETY_ON_NULL_RETURN(base);
   if (eina_stringshare_replace(&wd->widget_name, widget) |
       eina_stringshare_replace(&wd->widget_base, base))
     _theme_hook(obj);
}

EAPI void
elm_scroller_content_min_limit(Evas_Object *obj, Eina_Bool w, Eina_Bool h)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   wd->min_w = w;
   wd->min_h = h;
   _sizing_eval(obj);
}

EAPI void
elm_scroller_region_show(Evas_Object *obj, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if ((!wd) || (!wd->scr)) return;
   elm_smart_scroller_child_region_show(wd->scr, x, y, w, h);
}

EAPI void
elm_scroller_policy_set(Evas_Object *obj, Elm_Scroller_Policy policy_h, Elm_Scroller_Policy policy_v)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   const Elm_Scroller_Policy map[3] =
     {
        ELM_SMART_SCROLLER_POLICY_AUTO,
        ELM_SMART_SCROLLER_POLICY_ON,
        ELM_SMART_SCROLLER_POLICY_OFF
     };
   if ((!wd) || (!wd->scr)) return;
   if ((policy_h >= 3) || (policy_v >= 3)) return;
   elm_smart_scroller_policy_set(wd->scr, map[policy_h], map[policy_v]);
}

EAPI void
elm_scroller_policy_get(const Evas_Object *obj, Elm_Scroller_Policy *policy_h, Elm_Scroller_Policy *policy_v)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if ((!wd) || (!wd->scr)) return;
   elm_smart_scroller_policy_get(wd->scr,
                                 (Elm_Smart_Scroller_Policy *) policy_h,
                                 (Elm_Smart_Scroller_Policy *) policy_v);
}

EAPI void
elm_scroller_region_get(const Evas_Object *obj, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if ((!wd) || (!wd->scr)) return;
   if ((x) || (y)) elm_smart_scroller_child_pos_get(wd->scr, x, y);
   if ((w) || (h)) elm_smart_scroller_child_viewport_size_get(wd->scr, w, h);
}

EAPI void
elm_scroller_child_size_get(const Evas_Object *obj, Evas_Coord *w, Evas_Coord *h)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   evas_object_geometry_get(wd->content, NULL, NULL, w, h);
}

EAPI void
elm_scroller_bounce_set(Evas_Object *obj, Eina_Bool h_bounce, Eina_Bool v_bounce)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if ((!wd) || (!wd->scr)) return;
   elm_smart_scroller_bounce_allow_set(wd->scr, h_bounce, v_bounce);
}

EAPI void
elm_scroller_bounce_get(const Evas_Object *obj, Eina_Bool *h_bounce, Eina_Bool *v_bounce)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_smart_scroller_bounce_allow_get(wd->scr, h_bounce, v_bounce);
}

EAPI void
elm_scroller_page_relative_set(Evas_Object *obj, double h_pagerel, double v_pagerel)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   wd->pagerel_h = h_pagerel;
   wd->pagerel_v = v_pagerel;
   if (wd->scr)
     elm_smart_scroller_paging_set(wd->scr, wd->pagerel_h, wd->pagerel_v,
                                   wd->pagesize_h, wd->pagesize_v);
}

EAPI void
elm_scroller_page_size_set(Evas_Object *obj, Evas_Coord h_pagesize, Evas_Coord v_pagesize)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   wd->pagesize_h = h_pagesize;
   wd->pagesize_v = v_pagesize;
   if (wd->scr)
     elm_smart_scroller_paging_set(wd->scr, wd->pagerel_h, wd->pagerel_v,
                                   wd->pagesize_h, wd->pagesize_v);
}

EAPI void
elm_scroller_current_page_get(Evas_Object *obj, int *h_pagenumber, int *v_pagenumber)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->scr)
     elm_smart_scroller_current_page_get(wd->scr, h_pagenumber, v_pagenumber);
}

EAPI void
elm_scroller_last_page_get(Evas_Object *obj, int *h_pagenumber, int *v_pagenumber)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->scr)
     elm_smart_scroller_last_page_get(wd->scr, h_pagenumber, v_pagenumber);
}

EAPI void
elm_scroller_page_show(Evas_Object *obj, int h_pagenumber, int v_pagenumber)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->scr)
     elm_smart_scroller_page_show(wd->scr, h_pagenumber, v_pagenumber);
}

EAPI void
elm_scroller_page_bring_in(Evas_Object *obj, int h_pagenumber, int v_pagenumber)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->scr)
     elm_smart_scroller_page_bring_in(wd->scr, h_pagenumber, v_pagenumber);
}

EAPI void
elm_scroller_region_bring_in(Evas_Object *obj, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if ((!wd) || (!wd->scr)) return;
   elm_smart_scroller_region_bring_in(wd->scr, x, y, w, h);
}

EAPI void
elm_scroller_propagate_events_set(Evas_Object *obj, Eina_Bool propagation)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;

   evas_object_propagate_events_set(wd->scr, propagation);
}

EAPI Eina_Bool
elm_scroller_propagate_events_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;

   return evas_object_propagate_events_get(wd->scr);
}
