#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_OBJECT_PROTECTED
#define ELM_LAYOUT_PROTECTED
#define EFL_PART_PROTECTED

#include <Elementary.h>
#include "elm_priv.h"
#include "elm_bubble_eo.h"
#include "elm_widget_bubble.h"
#include "elm_widget_layout.h"

#include "elm_bubble_part.eo.h"
#include "elm_part_helper.h"

#define MY_CLASS ELM_BUBBLE_CLASS
#define MY_CLASS_PFX elm_bubble

#define MY_CLASS_NAME "Elm_Bubble"
#define MY_CLASS_NAME_LEGACY "elm_bubble"

static const char SIG_CLICKED[] = "clicked";

static const Evas_Smart_Cb_Description _smart_callbacks[] =
{
   {SIG_CLICKED, ""},
   {SIG_LAYOUT_FOCUSED, ""}, /**< handled by elm_layout */
   {SIG_LAYOUT_UNFOCUSED, ""}, /**< handled by elm_layout */
   {NULL, NULL}
};

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

static const char *corner_string[] =
{
   "top_left",
   "top_right",
   "bottom_left",
   "bottom_right"
};

EOLIAN static void
_elm_bubble_elm_layout_sizing_eval(Eo *obj, Elm_Bubble_Data *_pd EINA_UNUSED)
{
   Evas_Coord minw = -1, minh = -1;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   edje_object_size_min_restricted_calc
     (wd->resize_obj, &minw, &minh, minw, minh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, -1, -1);
}

static void
_on_mouse_up(void *data,
             Evas *e EINA_UNUSED,
             Evas_Object *obj EINA_UNUSED,
             void *event_info)
{
   Evas_Event_Mouse_Up *ev = event_info;

   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD)
     return;

   efl_event_callback_legacy_call(data, EFL_UI_EVENT_CLICKED, NULL);
}

static Eina_Bool
_elm_bubble_text_set(Eo *obj, Elm_Bubble_Data *_pd EINA_UNUSED, const char *part, const char *label)
{
   if (!_elm_layout_part_aliasing_eval(obj, &part, EINA_TRUE))
     return EINA_FALSE;

   efl_text_set(efl_part(efl_super(obj, MY_CLASS), part), label);

   if (!strcmp(part, "elm.info"))
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
_access_info_cb(void *data EINA_UNUSED, Evas_Object *obj)
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

EOLIAN static void
_elm_bubble_efl_canvas_group_group_add(Eo *obj, Elm_Bubble_Data *priv)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   efl_canvas_group_add(efl_super(obj, MY_CLASS));
   elm_widget_sub_object_parent_add(obj);

   priv->pos = ELM_BUBBLE_POS_TOP_LEFT; //default

   elm_widget_can_focus_set(obj, EINA_FALSE);

   evas_object_event_callback_add
     (wd->resize_obj, EVAS_CALLBACK_MOUSE_UP,
     _on_mouse_up, obj);

   // ACCESS
   _elm_access_object_register(obj, wd->resize_obj);
   _elm_access_text_set
     (_elm_access_info_get(obj), ELM_ACCESS_TYPE, E_("Bubble"));
   _elm_access_callback_set
     (_elm_access_info_get(obj), ELM_ACCESS_INFO, _access_info_cb, NULL);

   if (!elm_layout_theme_set(obj, "bubble", "base", elm_widget_style_get(obj)))
     CRI("Failed to set layout!");

   elm_layout_sizing_eval(obj);

   if (_elm_config->access_mode == ELM_ACCESS_MODE_ON)
     elm_widget_can_focus_set(obj, EINA_TRUE);
}

EOLIAN static void
_elm_bubble_efl_ui_widget_on_access_update(Eo *obj, Elm_Bubble_Data *_pd EINA_UNUSED, Eina_Bool is_access)
{
   ELM_BUBBLE_CHECK(obj);

   if (is_access)
     elm_widget_can_focus_set(obj, EINA_TRUE);
   else
     elm_widget_can_focus_set(obj, EINA_FALSE);
}

EAPI Evas_Object *
elm_bubble_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   return elm_legacy_add(MY_CLASS, parent);
}

EOLIAN static Eo *
_elm_bubble_efl_object_constructor(Eo *obj, Elm_Bubble_Data *_pd EINA_UNUSED)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   efl_canvas_object_type_set(obj, MY_CLASS_NAME_LEGACY);
   evas_object_smart_callbacks_descriptions_set(obj, _smart_callbacks);
   efl_access_object_role_set(obj, EFL_ACCESS_ROLE_FILLER);

   return obj;
}

EOLIAN static void
_elm_bubble_pos_set(Eo *obj, Elm_Bubble_Data *sd, Elm_Bubble_Pos pos)
{
   /* FIXME: Why is this dealing with layout data directly? */
   if (pos < ELM_BUBBLE_POS_TOP_LEFT || pos > ELM_BUBBLE_POS_BOTTOM_RIGHT)
     return;

   sd->pos = pos;

   elm_widget_theme_element_set(obj, corner_string[sd->pos]);

   efl_ui_widget_theme_apply(obj);
}

EOLIAN static Elm_Bubble_Pos
_elm_bubble_pos_get(const Eo *obj EINA_UNUSED, Elm_Bubble_Data *sd)
{
   return sd->pos;
}

EOLIAN static void
_elm_bubble_class_constructor(Efl_Class *klass)
{
   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}

/* Efl.Part begin */
ELM_PART_OVERRIDE(elm_bubble, ELM_BUBBLE, Elm_Bubble_Data)
ELM_PART_OVERRIDE_TEXT_SET(elm_bubble, ELM_BUBBLE, Elm_Bubble_Data)

#include "elm_bubble_part.eo.c"
/* Efl.Part end */

/* Internal EO APIs and hidden overrides */

ELM_LAYOUT_CONTENT_ALIASES_IMPLEMENT(MY_CLASS_PFX)
ELM_LAYOUT_TEXT_ALIASES_IMPLEMENT(MY_CLASS_PFX)

#define ELM_BUBBLE_EXTRA_OPS \
   ELM_LAYOUT_CONTENT_ALIASES_OPS(MY_CLASS_PFX), \
   ELM_LAYOUT_TEXT_ALIASES_OPS(MY_CLASS_PFX), \
   ELM_LAYOUT_SIZING_EVAL_OPS(elm_bubble), \
   EFL_CANVAS_GROUP_ADD_OPS(elm_bubble)

#include "elm_bubble_eo.c"
