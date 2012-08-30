#include <Elementary.h>
#include "elm_priv.h"
#include "elm_widget_panel.h"

EAPI const char ELM_PANEL_SMART_NAME[] = "elm_panel";

EVAS_SMART_SUBCLASS_NEW
  (ELM_PANEL_SMART_NAME, _elm_panel, Elm_Panel_Smart_Class,
  Elm_Layout_Smart_Class, elm_layout_smart_class_get, NULL);

static void
_mirrored_set(Evas_Object *obj,
              Eina_Bool rtl)
{
   ELM_PANEL_DATA_GET(obj, sd);

   elm_widget_mirrored_set(sd->bx, rtl);
   elm_panel_orient_set(obj, elm_panel_orient_get(obj));
}

static void
_elm_panel_smart_sizing_eval(Evas_Object *obj)
{
   Evas_Coord mw = -1, mh = -1;

   ELM_PANEL_DATA_GET(obj, sd);

   if (sd->on_deletion) return;

   evas_object_smart_calculate(sd->bx);
   edje_object_size_min_calc(ELM_WIDGET_DATA(sd)->resize_obj, &mw, &mh);
   evas_object_size_hint_min_set(obj, mw, mh);
   evas_object_size_hint_max_set(obj, -1, -1);
}

static void
_orient_set_do(Evas_Object *obj)
{
   ELM_PANEL_DATA_GET(obj, sd);

   switch (sd->orient)
     {
      case ELM_PANEL_ORIENT_TOP:
        elm_layout_theme_set(obj, "panel", "top", elm_widget_style_get(obj));
        break;

      case ELM_PANEL_ORIENT_BOTTOM:
        elm_layout_theme_set
          (obj, "panel", "bottom", elm_widget_style_get(obj));
        break;

      case ELM_PANEL_ORIENT_LEFT:
        if (!elm_widget_mirrored_get(obj))
          elm_layout_theme_set
            (obj, "panel", "left", elm_widget_style_get(obj));
        else
          elm_layout_theme_set
            (obj, "panel", "right", elm_widget_style_get(obj));
        break;

      case ELM_PANEL_ORIENT_RIGHT:
        if (!elm_widget_mirrored_get(obj))
          elm_layout_theme_set
            (obj, "panel", "right", elm_widget_style_get(obj));
        else
          elm_layout_theme_set
            (obj, "panel", "left", elm_widget_style_get(obj));
        break;
     }
}

static Eina_Bool
_elm_panel_smart_theme(Evas_Object *obj)
{
   const char *str;

   ELM_PANEL_DATA_GET(obj, sd);

   if (!ELM_WIDGET_CLASS(_elm_panel_parent_sc)->theme(obj))
     return EINA_FALSE;

   _mirrored_set(obj, elm_widget_mirrored_get(obj));

   str = edje_object_data_get
       (ELM_WIDGET_DATA(sd)->resize_obj, "focus_highlight");
   if ((str) && (!strcmp(str, "on")))
     elm_widget_highlight_in_theme_set(obj, EINA_TRUE);
   else
     elm_widget_highlight_in_theme_set(obj, EINA_FALSE);

   _orient_set_do(obj);

   elm_layout_sizing_eval(obj);

   return EINA_TRUE;
}

static Eina_Bool
_elm_panel_smart_focus_next(const Evas_Object *obj,
                            Elm_Focus_Direction dir,
                            Evas_Object **next)
{
   Evas_Object *cur;

   ELM_PANEL_DATA_GET(obj, sd);

   if (!sd->content)
     return EINA_FALSE;

   cur = sd->content;

   /* Try to Focus cycle in subitem */
   if (!sd->hidden)
     return elm_widget_focus_next_get(cur, dir, next);

   /* Return */
   *next = (Evas_Object *)obj;
   return !elm_widget_focus_get(obj);
}

static void
_box_layout_cb(Evas_Object *o,
               Evas_Object_Box_Data *priv,
               void *data __UNUSED__)
{
   _els_box_layout(o, priv, EINA_TRUE, EINA_FALSE, EINA_FALSE);
}

static void
_panel_toggle(void *data __UNUSED__,
              Evas_Object *obj,
              const char *emission __UNUSED__,
              const char *source __UNUSED__)
{
   ELM_PANEL_DATA_GET(obj, sd);

   if (sd->hidden)
     {
        elm_layout_signal_emit(obj, "elm,action,show", "elm");
        sd->hidden = EINA_FALSE;
        evas_object_repeat_events_set(obj, EINA_FALSE);
     }
   else
     {
        elm_layout_signal_emit(obj, "elm,action,hide", "elm");
        sd->hidden = EINA_TRUE;
        evas_object_repeat_events_set(obj, EINA_TRUE);
        if (elm_widget_focus_get(sd->content))
          {
             elm_widget_focused_object_clear(obj);
             elm_widget_focus_steal(obj);
          }
     }

   edje_object_message_signal_process(ELM_WIDGET_DATA(sd)->resize_obj);
}

static Eina_Bool
_elm_panel_smart_event(Evas_Object *obj,
                       Evas_Object *src,
                       Evas_Callback_Type type,
                       void *event_info)
{
   Evas_Event_Key_Down *ev = event_info;

   if (elm_widget_disabled_get(obj)) return EINA_FALSE;
   if ((src != obj) || (type != EVAS_CALLBACK_KEY_DOWN)) return EINA_FALSE;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return EINA_FALSE;

   if ((strcmp(ev->keyname, "Return")) &&
       (strcmp(ev->keyname, "KP_Enter")) &&
       (strcmp(ev->keyname, "space")))
     return EINA_FALSE;

   _panel_toggle(NULL, obj, NULL, NULL);

   ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
   return EINA_TRUE;
}

static Eina_Bool
_elm_panel_smart_content_set(Evas_Object *obj,
                             const char *part,
                             Evas_Object *content)
{
   ELM_PANEL_DATA_GET(obj, sd);

   if (part && strcmp(part, "default"))
     return ELM_CONTAINER_CLASS(_elm_panel_parent_sc)->content_set
              (obj, part, content);

   if (sd->content == content) return EINA_TRUE;
   if (sd->content)
     evas_object_box_remove_all(sd->bx, EINA_TRUE);
   sd->content = content;
   if (content)
     {
        evas_object_box_append(sd->bx, sd->content);
        evas_object_show(sd->content);
     }

   elm_layout_sizing_eval(obj);

   return EINA_TRUE;
}

static Evas_Object *
_elm_panel_smart_content_get(const Evas_Object *obj,
                             const char *part)
{
   ELM_PANEL_DATA_GET(obj, sd);

   if (part && strcmp(part, "default"))
     return ELM_CONTAINER_CLASS(_elm_panel_parent_sc)->content_get(obj, part);

   return sd->content;
}

static Evas_Object *
_elm_panel_smart_content_unset(Evas_Object *obj,
                               const char *part)
{
   Evas_Object *content;

   ELM_PANEL_DATA_GET(obj, sd);

   if (part && strcmp(part, "default"))
     return ELM_CONTAINER_CLASS
              (_elm_panel_parent_sc)->content_unset(obj, part);

   if (!sd->content) return NULL;
   content = sd->content;

   evas_object_box_remove_all(sd->bx, EINA_FALSE);
   sd->content = NULL;

   return content;
}

static void
_elm_panel_smart_add(Evas_Object *obj)
{
   EVAS_SMART_DATA_ALLOC(obj, Elm_Panel_Smart_Data);

   ELM_WIDGET_CLASS(_elm_panel_parent_sc)->base.add(obj);

   elm_widget_can_focus_set(obj, EINA_TRUE);

   /* just to bootstrap and have theme hook to work */
   elm_layout_theme_set(obj, "panel", "top", elm_widget_style_get(obj));

   _elm_panel_smart_theme(obj);

   elm_widget_can_focus_set(obj, EINA_TRUE);

   priv->hidden = EINA_FALSE;

   priv->bx = evas_object_box_add(evas_object_evas_get(obj));
   evas_object_size_hint_align_set(priv->bx, 0.5, 0.5);
   evas_object_box_layout_set(priv->bx, _box_layout_cb, priv, NULL);
   elm_layout_content_set(obj, "elm.swallow.content", priv->bx);
   evas_object_show(priv->bx);

   elm_layout_signal_callback_add
     (obj, "elm,action,panel,toggle", "*", _panel_toggle, obj);

   _mirrored_set(obj, elm_widget_mirrored_get(obj));

   elm_layout_sizing_eval(obj);
}

static void
_elm_panel_smart_del(Evas_Object *obj)
{
   Evas_Object *child;
   Eina_List *l;

   ELM_PANEL_DATA_GET(obj, sd);

   sd->on_deletion = EINA_TRUE;

   /* let's make our box object the *last* to be processed, since it
    * may (smart) parent other sub objects here */
   EINA_LIST_FOREACH (ELM_WIDGET_DATA(sd)->subobjs, l, child)
     {
        if (child == sd->bx)
          {
             ELM_WIDGET_DATA(sd)->subobjs =
               eina_list_demote_list(ELM_WIDGET_DATA(sd)->subobjs, l);
             break;
          }
     }

   ELM_WIDGET_CLASS(_elm_panel_parent_sc)->base.del(obj);
}

static void
_elm_panel_smart_set_user(Elm_Panel_Smart_Class *sc)
{
   ELM_WIDGET_CLASS(sc)->base.add = _elm_panel_smart_add;
   ELM_WIDGET_CLASS(sc)->base.del = _elm_panel_smart_del;

   ELM_WIDGET_CLASS(sc)->focus_next = _elm_panel_smart_focus_next;
   ELM_WIDGET_CLASS(sc)->theme = _elm_panel_smart_theme;
   ELM_WIDGET_CLASS(sc)->event = _elm_panel_smart_event;

   ELM_CONTAINER_CLASS(sc)->content_set = _elm_panel_smart_content_set;
   ELM_CONTAINER_CLASS(sc)->content_get = _elm_panel_smart_content_get;
   ELM_CONTAINER_CLASS(sc)->content_unset = _elm_panel_smart_content_unset;

   ELM_LAYOUT_CLASS(sc)->sizing_eval = _elm_panel_smart_sizing_eval;
}

EAPI const Elm_Panel_Smart_Class *
elm_panel_smart_class_get(void)
{
   static Elm_Panel_Smart_Class _sc =
     ELM_PANEL_SMART_CLASS_INIT_NAME_VERSION(ELM_PANEL_SMART_NAME);
   static const Elm_Panel_Smart_Class *class = NULL;

   if (class) return class;

   _elm_panel_smart_set(&_sc);
   class = &_sc;

   return class;
}

EAPI Evas_Object *
elm_panel_add(Evas_Object *parent)
{
   Evas_Object *obj;

   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);

   obj = elm_widget_add(_elm_panel_smart_class_new(), parent);
   if (!obj) return NULL;

   if (!elm_widget_sub_object_add(parent, obj))
     ERR("could not add %p as sub object of %p", obj, parent);

   return obj;
}

EAPI void
elm_panel_orient_set(Evas_Object *obj,
                     Elm_Panel_Orient orient)
{
   ELM_PANEL_CHECK(obj);
   ELM_PANEL_DATA_GET(obj, sd);

   if (sd->orient == orient) return;
   sd->orient = orient;

   _orient_set_do(obj);

   elm_layout_sizing_eval(obj);
}

EAPI Elm_Panel_Orient
elm_panel_orient_get(const Evas_Object *obj)
{
   ELM_PANEL_CHECK(obj) ELM_PANEL_ORIENT_LEFT;
   ELM_PANEL_DATA_GET(obj, sd);

   return sd->orient;
}

EAPI void
elm_panel_hidden_set(Evas_Object *obj,
                     Eina_Bool hidden)
{
   ELM_PANEL_CHECK(obj);
   ELM_PANEL_DATA_GET(obj, sd);

   if (sd->hidden == hidden) return;

   _panel_toggle(NULL, obj, NULL, NULL);
}

EAPI Eina_Bool
elm_panel_hidden_get(const Evas_Object *obj)
{
   ELM_PANEL_CHECK(obj) EINA_FALSE;
   ELM_PANEL_DATA_GET(obj, sd);

   return sd->hidden;
}

EAPI void
elm_panel_toggle(Evas_Object *obj)
{
   ELM_PANEL_CHECK(obj);

   _panel_toggle(NULL, obj, NULL, NULL);
}
