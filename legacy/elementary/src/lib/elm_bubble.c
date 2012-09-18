#include <Elementary.h>
#include "elm_priv.h"
#include "elm_widget_bubble.h"

EAPI const char ELM_BUBBLE_SMART_NAME[] = "elm_bubble";

static const char SIG_CLICKED[] = "clicked";

static const Elm_Layout_Part_Alias_Description _content_aliases[] =
{
   {"default", "elm.swallow.content"},
   {"icon", "elm.swallow.icon"},
   {NULL, NULL}
};

static const Elm_Layout_Part_Alias_Description _text_aliases[] =
{
   {"default", "elm.text"},
   {"info", "elm.info"},
   {NULL, NULL}
};

static const Evas_Smart_Cb_Description _smart_callbacks[] =
{
   {SIG_CLICKED, ""},
   {NULL, NULL}
};

static const char *corner_string[] =
{
   "top_left",
   "top_right",
   "bottom_left",
   "bottom_right"
};

EVAS_SMART_SUBCLASS_NEW
  (ELM_BUBBLE_SMART_NAME, _elm_bubble, Elm_Bubble_Smart_Class,
  Elm_Layout_Smart_Class, elm_layout_smart_class_get, _smart_callbacks);

static void
_elm_bubble_smart_sizing_eval(Evas_Object *obj)
{
   Evas_Coord minw = -1, minh = -1, maxw = -1, maxh = -1;

   ELM_BUBBLE_DATA_GET(obj, sd);

   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   edje_object_size_min_restricted_calc
     (ELM_WIDGET_DATA(sd)->resize_obj, &minw, &minh, minw, minh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, maxw, maxh);
}

static void
_on_mouse_up(void *data,
             Evas *e __UNUSED__,
             Evas_Object *obj __UNUSED__,
             void *event_info)
{
   Evas_Event_Mouse_Up *ev = event_info;

   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD)
     return;

   evas_object_smart_callback_call(data, SIG_CLICKED, NULL);
}

/* overriding layout's focus_next() in order to just cycle through the
 * content's tree */
static Eina_Bool
_elm_bubble_smart_focus_next(const Evas_Object *obj,
                             Elm_Focus_Direction dir,
                             Evas_Object **next)
{
   Evas_Object *content;

   ELM_BUBBLE_DATA_GET(obj, sd);

   if ((elm_widget_can_focus_get(obj)) &&
       (!ELM_WIDGET_DATA(sd)->focused))
     {
        // ACCESS
        *next = (Evas_Object *)obj;
        return EINA_TRUE;
     }
   else
     {
        content = elm_layout_content_get(obj, NULL);
        if (!content) return EINA_FALSE;

        /* attempt to follow focus cycle into sub-object */
        return elm_widget_focus_next_get(content, dir, next);
     }
}

static Eina_Bool
_elm_bubble_smart_focus_direction(const Evas_Object *obj,
                                  const Evas_Object *base,
                                  double degree,
                                  Evas_Object **direction,
                                  double *weight)
{
   Evas_Object *content;

   content = elm_layout_content_get(obj, NULL);

   if (!content) return EINA_FALSE;

   /* Try Focus cycle in subitem */
   return elm_widget_focus_direction_get
            (content, base, degree, direction, weight);
}

static Eina_Bool
_elm_bubble_smart_text_set(Evas_Object *obj,
                           const char *item,
                           const char *label)
{
   if (!_elm_bubble_parent_sc->text_set(obj, item, label))
     return EINA_FALSE;

   if (item && (!strcmp(item, "info") || !strcmp(item, "elm.info")))
     {
        if (label)
          elm_layout_signal_emit(obj, "elm,state,info,visible", "elm");
        else
          elm_layout_signal_emit(obj, "elm,state,info,hidden", "elm");
     }

   elm_layout_sizing_eval(obj);

   return EINA_TRUE;
}

static char *
_access_info_cb(void *data __UNUSED__,
                Evas_Object *obj,
                Elm_Widget_Item *item __UNUSED__)
{
   char *ret;
   Eina_Strbuf *buf;
   buf = eina_strbuf_new();
   Evas_Object *content;
   const char *default_txt = NULL;
   const char *content_txt = NULL;
   const char *info_txt = NULL;

   default_txt = elm_widget_access_info_get(obj);
   if (!default_txt) default_txt = elm_layout_text_get(obj, NULL);
   if (default_txt) eina_strbuf_append(buf, default_txt);

   content = elm_layout_content_get(obj, NULL);
   if (content) content_txt = elm_layout_text_get(content, NULL);
   if (content_txt)
     {
        if (!eina_strbuf_length_get(buf))
          eina_strbuf_append(buf, content_txt);
        else
          eina_strbuf_append_printf(buf, ", %s", content_txt);
     }


   info_txt = edje_object_part_text_get(elm_layout_edje_get(obj), "elm.info");
   if (info_txt)
     {
        if (!eina_strbuf_length_get(buf))
          eina_strbuf_append(buf, info_txt);
        else
          eina_strbuf_append_printf(buf, ", %s", info_txt);
     }

   ret = eina_strbuf_string_steal(buf);
   eina_strbuf_free(buf);
   return ret;
}

static void
_elm_bubble_smart_add(Evas_Object *obj)
{
   EVAS_SMART_DATA_ALLOC(obj, Elm_Bubble_Smart_Data);

   ELM_WIDGET_CLASS(_elm_bubble_parent_sc)->base.add(obj);

   priv->pos = ELM_BUBBLE_POS_TOP_LEFT; //default

   elm_widget_can_focus_set(obj, EINA_FALSE);

   evas_object_event_callback_add
     (ELM_WIDGET_DATA(priv)->resize_obj, EVAS_CALLBACK_MOUSE_UP,
     _on_mouse_up, obj);

   // ACCESS
   _elm_access_object_register(obj, ELM_WIDGET_DATA(priv)->resize_obj);
   _elm_access_text_set
     (_elm_access_object_get(obj), ELM_ACCESS_TYPE, E_("Bubble"));
   _elm_access_callback_set
     (_elm_access_object_get(obj), ELM_ACCESS_INFO, _access_info_cb, NULL);

   elm_layout_theme_set(obj, "bubble", "base", elm_widget_style_get(obj));

   elm_layout_sizing_eval(obj);

   if ((_elm_config->access_mode == ELM_ACCESS_MODE_ON))
     elm_widget_can_focus_set(obj, EINA_TRUE);
}

static void
_elm_bubble_smart_access(Evas_Object *obj, Eina_Bool is_access)
{
   ELM_BUBBLE_CHECK(obj);

   if (is_access)
     elm_widget_can_focus_set(obj, EINA_TRUE);
   else
     elm_widget_can_focus_set(obj, EINA_FALSE);
}

static void
_elm_bubble_smart_set_user(Elm_Bubble_Smart_Class *sc)
{
   ELM_WIDGET_CLASS(sc)->base.add = _elm_bubble_smart_add;

   ELM_WIDGET_CLASS(sc)->focus_next = _elm_bubble_smart_focus_next;
   ELM_WIDGET_CLASS(sc)->focus_direction = _elm_bubble_smart_focus_direction;
   ELM_WIDGET_CLASS(sc)->access = _elm_bubble_smart_access;

   ELM_LAYOUT_CLASS(sc)->text_set = _elm_bubble_smart_text_set;
   ELM_LAYOUT_CLASS(sc)->sizing_eval = _elm_bubble_smart_sizing_eval;

   ELM_LAYOUT_CLASS(sc)->content_aliases = _content_aliases;
   ELM_LAYOUT_CLASS(sc)->text_aliases = _text_aliases;
}

EAPI const Elm_Bubble_Smart_Class *
elm_bubble_smart_class_get(void)
{
   static Elm_Bubble_Smart_Class _sc =
     ELM_BUBBLE_SMART_CLASS_INIT_NAME_VERSION(ELM_BUBBLE_SMART_NAME);
   static const Elm_Bubble_Smart_Class *class = NULL;
   Evas_Smart_Class *esc = (Evas_Smart_Class *)&_sc;

   if (class)
     return class;

   _elm_bubble_smart_set(&_sc);
   esc->callbacks = _smart_callbacks;
   class = &_sc;

   return class;
}

EAPI Evas_Object *
elm_bubble_add(Evas_Object *parent)
{
   Evas_Object *obj;

   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);

   obj = elm_widget_add(_elm_bubble_smart_class_new(), parent);
   if (!obj) return NULL;

   if (!elm_widget_sub_object_add(parent, obj))
     ERR("could not add %p as sub object of %p", obj, parent);

   return obj;
}

EAPI void
elm_bubble_pos_set(Evas_Object *obj,
                   Elm_Bubble_Pos pos)
{
   ELM_BUBBLE_CHECK(obj);
   ELM_BUBBLE_DATA_GET_OR_RETURN(obj, sd);

   if (pos < ELM_BUBBLE_POS_TOP_LEFT || pos > ELM_BUBBLE_POS_BOTTOM_RIGHT)
     return;

   sd->pos = pos;

   eina_stringshare_replace
     (&(ELM_LAYOUT_DATA(sd)->group), corner_string[sd->pos]);

   ELM_WIDGET_DATA(sd)->api->theme(obj);
}

EAPI Elm_Bubble_Pos
elm_bubble_pos_get(const Evas_Object *obj)
{
   ELM_BUBBLE_CHECK(obj) ELM_BUBBLE_POS_INVALID;
   ELM_BUBBLE_DATA_GET_OR_RETURN_VAL(obj, sd, ELM_BUBBLE_POS_INVALID);

   return sd->pos;
}
