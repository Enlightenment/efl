#include <Elementary.h>
#include "elm_priv.h"
#include "els_scroller.h"
#include "els_box.h"

typedef struct _Widget_Data Widget_Data;
struct _Widget_Data
{
   Evas_Object *scr, *bx, *content;
   Elm_Panel_Orient orient;
   Eina_Bool hidden : 1;
};

static const char *widtype = NULL;
static void _del_hook(Evas_Object *obj);
static void _theme_hook(Evas_Object *obj);
static void _on_focus_hook(void *data, Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);
static void _resize(void *data, Evas *evas, Evas_Object *obj, void *event);
static void _layout(Evas_Object *o, Evas_Object_Box_Data *priv, void *data);
static void _toggle_panel(void *data, Evas_Object *obj, const char *emission, const char *source);

static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   free(wd);
}

static void
_mirrored_set(Evas_Object *obj, Eina_Bool rtl)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;

   if (wd->scr)
     {
        elm_widget_mirrored_set(wd->bx, rtl);
        elm_panel_orient_set(obj, elm_panel_orient_get(obj));
     }
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
        elm_smart_scroller_object_theme_set(obj, wd->scr, "panel", "base",
                                            elm_widget_style_get(obj));
        //   scale = (elm_widget_scale_get(obj) * _elm_config->scale);
        //   edje_object_scale_set(wd->scr, scale);
        edj = elm_smart_scroller_edje_object_get(wd->scr);
        str = edje_object_data_get(edj, "focus_highlight");
        if ((str) && (!strcmp(str, "on")))
          elm_widget_highlight_in_theme_set(obj, EINA_TRUE);
        else
          elm_widget_highlight_in_theme_set(obj, EINA_FALSE);
     }

   _sizing_eval(obj);
}

static void
_on_focus_hook(void *data __UNUSED__, Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (elm_widget_focus_get(obj))
     evas_object_focus_set(obj, EINA_TRUE);
   else
     evas_object_focus_set(obj, EINA_FALSE);
}

static Eina_Bool
_elm_panel_focus_next_hook(const Evas_Object *obj, Elm_Focus_Direction dir, Evas_Object **next)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Object *cur;

   if ((!wd) || (!wd->content))
     return EINA_FALSE;

   cur = wd->content;

   /* Try Focus cycle in subitem */
   if (!wd->hidden)
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
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord mw = -1, mh = -1;
   Evas_Coord vw = 0, vh = 0;
   Evas_Coord w, h;
   if (!wd) return;
   evas_object_smart_calculate(wd->bx);
   edje_object_size_min_calc(elm_smart_scroller_edje_object_get(wd->scr),
                             &mw, &mh);
   evas_object_geometry_get(obj, NULL, NULL, &w, &h);
   if (w < mw) w = mw;
   if (h < mh) h = mh;
   evas_object_resize(wd->scr, w, h);

   evas_object_size_hint_min_get(wd->bx, &mw, &mh);
   if (w > mw) mw = w;
   if (h > mh) mh = h;
   evas_object_resize(wd->bx, mw, mh);

   elm_smart_scroller_child_viewport_size_get(wd->scr, &vw, &vh);
   mw = mw + (w - vw);
   mh = mh + (h - vh);
   evas_object_size_hint_min_set(obj, mw, mh);
   evas_object_size_hint_max_set(obj, -1, -1);
}

static void
_resize(void *data, Evas *evas __UNUSED__, Evas_Object *obj __UNUSED__, void *event __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   Evas_Coord mw, mh, vw, vh, w, h;
   if (!wd) return;
   elm_smart_scroller_child_viewport_size_get(wd->scr, &vw, &vh);
   evas_object_size_hint_min_get(wd->bx, &mw, &mh);
   evas_object_geometry_get(wd->bx, NULL, NULL, &w, &h);
   if ((vw >= mw) || (vh >= mh))
     {
        if ((w != vw) || (h != vh)) evas_object_resize(wd->bx, vw, vh);
     }
}

static void
_layout(Evas_Object *o, Evas_Object_Box_Data *priv, void *data)
{
   Widget_Data *wd = data;
   if (!wd) return;
   _els_box_layout(o, priv, EINA_TRUE, EINA_FALSE, EINA_FALSE);
}

static void
_toggle_panel(void *data, Evas_Object *obj, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;
   if (wd->hidden)
     {
        edje_object_signal_emit(elm_smart_scroller_edje_object_get(wd->scr),
                                "elm,action,show", "elm");
        wd->hidden = EINA_FALSE;
        evas_object_repeat_events_set(obj, EINA_FALSE);
     }
   else
     {
        edje_object_signal_emit(elm_smart_scroller_edje_object_get(wd->scr),
                                "elm,action,hide", "elm");
        wd->hidden = EINA_TRUE;
        evas_object_repeat_events_set(obj, EINA_TRUE);
        if (elm_widget_focus_get(wd->content))
          {
             elm_widget_focused_object_clear(obj);
             elm_widget_focus_steal(obj);
          }
     }
}

static Eina_Bool
_event_hook(Evas_Object *obj, Evas_Object *src __UNUSED__, Evas_Callback_Type type, void *event_info)
{
   if ((src != obj) || (type != EVAS_CALLBACK_KEY_DOWN)) return EINA_FALSE;

   Evas_Event_Key_Down *ev = event_info;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;

   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return EINA_FALSE;

   if ((strcmp(ev->keyname, "Return")) &&
       (strcmp(ev->keyname, "KP_Enter")) &&
       (strcmp(ev->keyname, "space")))
     return EINA_FALSE;

   _toggle_panel(obj, NULL, "elm,action,panel,toggle", "*");

   ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
   return EINA_TRUE;
}

static void
_content_set_hook(Evas_Object *obj, const char *part __UNUSED__, Evas_Object *content)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->content == content) return;
   if (wd->content)
     evas_object_box_remove_all(wd->bx, EINA_TRUE);
   wd->content = content;
   if (content)
     {
        evas_object_box_append(wd->bx, wd->content);
        evas_object_show(wd->content);
     }
   _sizing_eval(obj);
}

static Evas_Object *
_content_get_hook(const Evas_Object *obj, const char *part __UNUSED__)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return wd->content;
}

static Evas_Object *
_content_unset_hook(Evas_Object *obj, const char *part __UNUSED__)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Object *content;
   if (!wd) return NULL;
   if (!wd->content) return NULL;
   content = wd->content;
   evas_object_box_remove_all(wd->bx, EINA_FALSE);
   wd->content = NULL;
   return content;
}

EAPI Evas_Object *
elm_panel_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas *e;
   Widget_Data *wd;

   ELM_WIDGET_STANDARD_SETUP(wd, Widget_Data, parent, e, obj, NULL);

   ELM_SET_WIDTYPE(widtype, "panel");
   elm_widget_type_set(obj, "panel");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);
   elm_widget_on_focus_hook_set(obj, _on_focus_hook, NULL);
   elm_widget_signal_emit_hook_set(obj, _signal_emit_hook);
   elm_widget_signal_callback_add_hook_set(obj, _signal_callback_add_hook);
   elm_widget_signal_callback_del_hook_set(obj, _signal_callback_del_hook);
   elm_widget_focus_next_hook_set(obj, _elm_panel_focus_next_hook);
   elm_widget_can_focus_set(obj, EINA_TRUE);
   elm_widget_event_hook_set(obj, _event_hook);
   elm_widget_content_set_hook_set(obj, _content_set_hook);
   elm_widget_content_get_hook_set(obj, _content_get_hook);
   elm_widget_content_unset_hook_set(obj, _content_unset_hook);

   wd->scr = elm_smart_scroller_add(e);
   elm_smart_scroller_widget_set(wd->scr, obj);
   _theme_hook(obj);
   elm_smart_scroller_bounce_allow_set(wd->scr, EINA_FALSE, EINA_FALSE);
   elm_widget_resize_object_set(obj, wd->scr);
   elm_smart_scroller_policy_set(wd->scr, ELM_SMART_SCROLLER_POLICY_OFF,
                                 ELM_SMART_SCROLLER_POLICY_OFF);

   wd->hidden = EINA_FALSE;
   wd->orient = ELM_PANEL_ORIENT_LEFT;

   wd->bx = evas_object_box_add(e);
   evas_object_size_hint_align_set(wd->bx, 0.5, 0.5);
   evas_object_box_layout_set(wd->bx, _layout, wd, NULL);
   elm_widget_sub_object_add(obj, wd->bx);
   elm_smart_scroller_child_set(wd->scr, wd->bx);
   evas_object_show(wd->bx);

   edje_object_signal_callback_add(elm_smart_scroller_edje_object_get(wd->scr),
                                   "elm,action,panel,toggle", "*",
                                   _toggle_panel, obj);

   evas_object_event_callback_add(wd->scr, EVAS_CALLBACK_RESIZE, _resize, obj);

   _mirrored_set(obj, elm_widget_mirrored_get(obj));
   _sizing_eval(obj);
   return obj;
}

EAPI void
elm_panel_orient_set(Evas_Object *obj, Elm_Panel_Orient orient)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   wd->orient = orient;
   switch (orient)
     {
      case ELM_PANEL_ORIENT_TOP:
         elm_smart_scroller_object_theme_set(obj, wd->scr, "panel", "base", "top");
         break;
      case ELM_PANEL_ORIENT_BOTTOM:
         elm_smart_scroller_object_theme_set(obj, wd->scr, "panel", "base", "bottom");
         break;
      case ELM_PANEL_ORIENT_LEFT:
         if (!elm_widget_mirrored_get(obj))
           elm_smart_scroller_object_theme_set(obj, wd->scr, "panel", "base", "left");
         else
           elm_smart_scroller_object_theme_set(obj, wd->scr, "panel", "base", "right");
         break;
      case ELM_PANEL_ORIENT_RIGHT:
         if (!elm_widget_mirrored_get(obj))
           elm_smart_scroller_object_theme_set(obj, wd->scr, "panel", "base", "right");
         else
           elm_smart_scroller_object_theme_set(obj, wd->scr, "panel", "base", "left");
         break;
     }

   _sizing_eval(obj);
}

EAPI Elm_Panel_Orient
elm_panel_orient_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) ELM_PANEL_ORIENT_LEFT;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return ELM_PANEL_ORIENT_LEFT;
   return wd->orient;
}

EAPI void
elm_panel_content_set(Evas_Object *obj, Evas_Object *content)
{
   _content_set_hook(obj, NULL, content);
}

EAPI Evas_Object *
elm_panel_content_get(const Evas_Object *obj)
{
   return _content_get_hook(obj, NULL);
}

EAPI Evas_Object *
elm_panel_content_unset(Evas_Object *obj)
{
   return _content_unset_hook(obj, NULL);
}

EAPI void
elm_panel_hidden_set(Evas_Object *obj, Eina_Bool hidden)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->hidden == hidden) return;
   _toggle_panel(obj, NULL, "elm,action,panel,toggle", "*");
}

EAPI Eina_Bool
elm_panel_hidden_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return wd->hidden;
}

EAPI void
elm_panel_toggle(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   _toggle_panel(obj, NULL, "elm,action,panel,toggle", "*");
}
