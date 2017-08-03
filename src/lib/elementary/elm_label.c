#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define ELM_INTERFACE_ATSPI_ACCESSIBLE_PROTECTED
#define ELM_LAYOUT_PROTECTED

#include <Elementary.h>

#include "elm_priv.h"
#include "elm_widget_layout.h"
#include "elm_widget_label.h"
#include "elm_label_internal_part.eo.h"
#include "elm_part_helper.h"

#define MY_CLASS ELM_LABEL_CLASS

#define MY_CLASS_NAME "Elm_Label"
#define MY_CLASS_NAME_LEGACY "elm_label"

static const char SIG_SLIDE_END[] = "slide,end";

static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_WIDGET_LANG_CHANGED, ""}, /**< handled by elm_widget */
   {SIG_WIDGET_ACCESS_CHANGED, ""}, /**< handled by elm_widget */
   {SIG_SLIDE_END, ""},
   {NULL, NULL}
};

static void
_recalc(void *data)
{
   ELM_LABEL_DATA_GET(data, sd);
   ELM_WIDGET_DATA_GET_OR_RETURN(data, wd);

   Evas_Coord minw = -1, minh = -1;
   Evas_Coord resw, w;

   evas_event_freeze(evas_object_evas_get(data));
   edje_object_size_min_calc(wd->resize_obj, &minw, NULL);
   evas_object_geometry_get(wd->resize_obj, NULL, NULL, &w, NULL);

   if (sd->wrap_w > minw)
     resw = sd->wrap_w;
   else if ((sd->wrap_w > 0) && (minw > sd->wrap_w))
     resw = minw;
   else
     resw = w;
   edje_object_size_min_restricted_calc(wd->resize_obj, &minw, &minh, resw, 0);

   /* If wrap_w is not set, label's width has to be controlled
      by outside of label. So, we don't need to set minimum width. */
   if (sd->wrap_w == -1)
     evas_object_size_hint_min_set(data, 0, minh);
   else
     evas_object_size_hint_min_set(data, minw, minh);

   evas_event_thaw(evas_object_evas_get(data));
   evas_event_thaw_eval(evas_object_evas_get(data));
}

static void
_label_format_set(Evas_Object *obj, const char *format)
{
   if (format)
     edje_object_part_text_style_user_push(obj, "elm.text", format);
   else
     edje_object_part_text_style_user_pop(obj, "elm.text");
}

static void
_label_slide_change(Evas_Object *obj)
{
   const Evas_Object *tb;
   char *plaintxt;
   int plainlen = 0;

   ELM_LABEL_DATA_GET(obj, sd);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   if (!sd->slide_state) return;

   edje_object_signal_emit(wd->resize_obj, "elm,state,slide,stop", "elm");

   //doesn't support multiline slide effect
   if (sd->linewrap)
     {
        WRN("Doesn't support slide effect for multiline! : label=%p", obj);
        return;
     }

   //stop if the text is none.
   plaintxt = _elm_util_mkup_to_text
      (edje_object_part_text_get(wd->resize_obj, "elm.text"));
   if (plaintxt)
     {
        plainlen = strlen(plaintxt);
        free(plaintxt);
     }
   if (plainlen < 1) return;

   //has slide effect.
   if (sd->slide_mode != ELM_LABEL_SLIDE_MODE_NONE)
     {
        Evas_Coord w, tb_w;

        tb = edje_object_part_object_get(wd->resize_obj, "elm.text");
        evas_object_textblock_size_native_get(tb, &tb_w, NULL);
        evas_object_geometry_get(wd->resize_obj, NULL, NULL, &w, NULL);
        if (w <= 0) return;

        if (sd->ellipsis)
          {
             sd->slide_ellipsis = EINA_TRUE;
             elm_label_ellipsis_set(obj, EINA_FALSE);
          }

        //slide only if the slide area is smaller than text width size.
        if (sd->slide_mode == ELM_LABEL_SLIDE_MODE_AUTO)
          {
            if ((tb_w > 0) && (tb_w < w))
              {
                if (sd->slide_ellipsis)
                  {
                     sd->slide_ellipsis = EINA_FALSE;
                     elm_label_ellipsis_set(obj, EINA_TRUE);
                  }
                return;
              }
          }

        // calculate speed or duration
        if (!strcmp(elm_object_style_get(obj), "slide_long"))
          w = tb_w + w;
        else if (!strcmp(elm_object_style_get(obj), "slide_short") ||
                 !strcmp(elm_object_style_get(obj), "slide_bounce")) // slide_short or slide_bounce
          w = tb_w - w;
        else
          w = tb_w;

        if (sd->use_slide_speed)
          {
             if (sd->slide_speed <= 0) sd->slide_speed = 1;
             sd->slide_duration = w / sd->slide_speed;
          }
        else
          {
             if (sd->slide_duration <= 0) sd->slide_duration = 1;
             sd->slide_speed = w / sd->slide_duration;
          }

        Edje_Message_Float_Set *msg =
          alloca(sizeof(Edje_Message_Float_Set));

        msg->count = 1;
        msg->val[0] = sd->slide_duration;

        edje_object_message_send(wd->resize_obj, EDJE_MESSAGE_FLOAT_SET, 0, msg);
        edje_object_signal_emit(wd->resize_obj, "elm,state,slide,start", "elm");
     }
   //no slide effect.
   else
     {
        if (sd->slide_ellipsis)
          {
             sd->slide_ellipsis = EINA_FALSE;
             elm_label_ellipsis_set(obj, EINA_TRUE);
          }
     }
}

static void
_elm_label_horizontal_size_policy_update(Eo *obj, Elm_Label_Data *sd)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   if (!sd->ellipsis && (sd->linewrap == ELM_WRAP_NONE))
     edje_object_signal_emit(wd->resize_obj, "elm,state,horizontal,expandable", "elm");
   else
     edje_object_signal_emit(wd->resize_obj, "elm,state,horizontal,fixed", "elm");
   edje_object_message_signal_process(wd->resize_obj);
}

EOLIAN static Elm_Theme_Apply
_elm_label_elm_widget_theme_apply(Eo *obj, Elm_Label_Data *sd)
{
   Elm_Theme_Apply int_ret = ELM_THEME_APPLY_FAILED;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, ELM_THEME_APPLY_FAILED);

   evas_event_freeze(evas_object_evas_get(obj));

   int_ret = elm_obj_widget_theme_apply(efl_super(obj, MY_CLASS));
   if (!int_ret) return ELM_THEME_APPLY_FAILED;

   _elm_label_horizontal_size_policy_update(obj, sd);

   _label_format_set(wd->resize_obj, sd->format);
   _label_slide_change(obj);

   evas_event_thaw(evas_object_evas_get(obj));
   evas_event_thaw_eval(evas_object_evas_get(obj));

   elm_layout_sizing_eval(obj);

   return int_ret;
}

EOLIAN static void
_elm_label_elm_layout_sizing_eval(Eo *obj, Elm_Label_Data *_pd EINA_UNUSED)
{
   Evas_Coord minw = -1, minh = -1;
   Evas_Coord resw, resh;

   ELM_LABEL_DATA_GET(obj, sd);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   if (sd->linewrap)
     {
        evas_object_geometry_get(wd->resize_obj, NULL, NULL, &resw, &resh);
        if (resw == sd->lastw) return;
        sd->lastw = resw;
        _recalc(obj);
     }
   else
     {
        evas_event_freeze(evas_object_evas_get(obj));
        edje_object_size_min_calc(wd->resize_obj, &minw, &minh);
        if (sd->wrap_w > 0 && minw > sd->wrap_w) minw = sd->wrap_w;
        evas_object_size_hint_min_set(obj, minw, minh);
        evas_event_thaw(evas_object_evas_get(obj));
        evas_event_thaw_eval(evas_object_evas_get(obj));
     }
}

static void
_on_label_resize(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   ELM_LABEL_DATA_GET(data, sd);

   if (sd->slide_mode != ELM_LABEL_SLIDE_MODE_NONE) _label_slide_change(data);
   if (sd->linewrap) elm_layout_sizing_eval(data);
}

static int
_get_value_in_key_string(const char *oldstring, const char *key, char **value)
{
   char *curlocater, *endtag;
   int firstindex = 0, foundflag = -1;

   curlocater = strstr(oldstring, key);
   if (curlocater)
     {
        int key_len = strlen(key);
        endtag = curlocater + key_len;
        if ((!endtag) || (*endtag != '='))
             return -1;

        firstindex = abs((int)(oldstring - curlocater));
        firstindex += key_len + 1; // strlen("key") + strlen("=")
        *value = (char *)oldstring + firstindex;

        foundflag = 1;
     }
   else
     {
        foundflag = 0;
     }

   if (foundflag == 1) return 0;

   return -1;
}

static int
_strbuf_key_value_replace(Eina_Strbuf *srcbuf, const char *key, const char *value, int deleteflag)
{
   char *kvalue;
   const char *srcstring = NULL;

   srcstring = eina_strbuf_string_get(srcbuf);

   if (_get_value_in_key_string(srcstring, key, &kvalue) == 0)
     {
        const char *val_end;
        int val_end_idx = 0;
        int key_start_idx = 0;
        val_end = strchr(kvalue, ' ');

        if (val_end)
          val_end_idx = val_end - srcstring;
        else
          val_end_idx = kvalue - srcstring + strlen(kvalue) - 1;

        /* -1 is because of the '=' */
        key_start_idx = kvalue - srcstring - 1 - strlen(key);
        eina_strbuf_remove(srcbuf, key_start_idx, val_end_idx);
        if (!deleteflag)
          {
             eina_strbuf_insert_printf(srcbuf, "%s=%s", key_start_idx, key,
                                       value);
          }
     }
   else if (!deleteflag)
     {
        if (*srcstring)
          {
             /* -1 because we want it before the ' */
             eina_strbuf_insert_printf (srcbuf, " %s=%s",
                                eina_strbuf_length_get(srcbuf) - 1, key, value);
          }
        else
          {
             eina_strbuf_append_printf(srcbuf, "DEFAULT='%s=%s'", key, value);
          }
     }

   return 0;
}

static int
_stringshare_key_value_replace(const char **srcstring, const char *key, const char *value, int deleteflag)
{
   Eina_Strbuf *sharebuf = NULL;

   sharebuf = eina_strbuf_new();
   eina_strbuf_append(sharebuf, *srcstring);
   _strbuf_key_value_replace(sharebuf, key, value, deleteflag);
   eina_stringshare_del(*srcstring);
   *srcstring = eina_stringshare_add(eina_strbuf_string_get(sharebuf));
   eina_strbuf_free(sharebuf);

   return 0;
}

static Eina_Bool
_elm_label_text_set(Eo *obj, Elm_Label_Data *sd, const char *part, const char *label)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);
   Eina_Bool int_ret = EINA_TRUE;

   if (!label) label = "";
   _label_format_set(wd->resize_obj, sd->format);

   efl_text_set(efl_part(efl_super(obj, MY_CLASS), part), label);

   if (int_ret)
     {
        sd->lastw = -1;
        elm_layout_sizing_eval(obj);
        _label_slide_change(obj);
     }
   return int_ret;
}

static char *
_access_info_cb(void *data EINA_UNUSED, Evas_Object *obj)
{
   const char *txt = elm_widget_access_info_get(obj);

   if (!txt)
     return _elm_util_mkup_to_text(elm_layout_text_get(obj, NULL));
   else return strdup(txt);
}

static void
_on_slide_end(void *data, Evas_Object *obj EINA_UNUSED,
              const char *emission EINA_UNUSED, const char *source EINA_UNUSED)
{
   ELM_LABEL_DATA_GET(data, sd);

   if (sd->slide_ellipsis)
     elm_obj_label_ellipsis_set(data, EINA_TRUE);

   efl_event_callback_legacy_call(data, ELM_LABEL_EVENT_SLIDE_END, NULL);
}

EOLIAN static void
_elm_label_efl_canvas_group_group_add(Eo *obj, Elm_Label_Data *priv)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   efl_canvas_group_add(efl_super(obj, MY_CLASS));

   elm_widget_sub_object_parent_add(obj);

   priv->linewrap = ELM_WRAP_NONE;
   priv->wrap_w = -1;
   priv->slide_duration = 10;

   priv->format = eina_stringshare_add("");
   _label_format_set(wd->resize_obj, priv->format);

   evas_object_event_callback_add(wd->resize_obj, EVAS_CALLBACK_RESIZE,
                                  _on_label_resize, obj);

   edje_object_signal_callback_add(wd->resize_obj, "elm,state,slide,end", "elm",
                                   _on_slide_end, obj);

   /* access */
   elm_widget_can_focus_set(obj, _elm_config->access_mode);

   _elm_access_object_register(obj, wd->resize_obj);
   _elm_access_text_set(_elm_access_info_get(obj), ELM_ACCESS_TYPE,
                        E_("Label"));
   _elm_access_callback_set(_elm_access_info_get(obj), ELM_ACCESS_INFO,
                            _access_info_cb, NULL);

   if (!elm_layout_theme_set(obj, "label", "base", elm_widget_style_get(obj)))
     CRI("Failed to set layout!");

   elm_layout_text_set(obj, NULL, "<br>");
   elm_layout_sizing_eval(obj);
}

EAPI Evas_Object *
elm_label_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   return efl_add(MY_CLASS, parent, efl_canvas_object_legacy_ctor(efl_added));
}

EOLIAN static Eo *
_elm_label_efl_object_constructor(Eo *obj, Elm_Label_Data *_pd EINA_UNUSED)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   efl_canvas_object_type_set(obj, MY_CLASS_NAME_LEGACY);
   evas_object_smart_callbacks_descriptions_set(obj, _smart_callbacks);
   elm_interface_atspi_accessible_role_set(obj, ELM_ATSPI_ROLE_LABEL);

   return obj;
}

EOLIAN static void
_elm_label_line_wrap_set(Eo *obj, Elm_Label_Data *sd, Elm_Wrap_Type wrap)
{
   const char *wrap_str, *text;
   int len;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   if (sd->linewrap == wrap) return;

   sd->linewrap = wrap;
   sd->lastw = -1;

   _elm_label_horizontal_size_policy_update(obj, sd);

   text = elm_layout_text_get(obj, NULL);
   if (!text) return;

   len = strlen(text);
   if (len <= 0) return;

   switch (wrap)
     {
      case ELM_WRAP_CHAR:
        wrap_str = "char";
        break;

      case ELM_WRAP_WORD:
        wrap_str = "word";
        break;

      case ELM_WRAP_MIXED:
        wrap_str = "mixed";
        break;

      default:
        wrap_str = "none";
        break;
     }

   if (_stringshare_key_value_replace(&sd->format, "wrap", wrap_str, 0) == 0)
     {
        sd->lastw = -1;
        _label_format_set(wd->resize_obj, sd->format);
        elm_layout_sizing_eval(obj);
     }
}

EOLIAN static Elm_Wrap_Type
_elm_label_line_wrap_get(Eo *obj EINA_UNUSED, Elm_Label_Data *sd)
{
   return sd->linewrap;
}

EOLIAN static void
_elm_label_wrap_width_set(Eo *obj, Elm_Label_Data *sd, Evas_Coord w)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   if (w < 0) w = 0;

   if (sd->wrap_w == w) return;

   if (sd->ellipsis)
     _label_format_set(wd->resize_obj, sd->format);
   sd->wrap_w = w;
   sd->lastw = -1;

   elm_layout_sizing_eval(obj);
}

EOLIAN static Evas_Coord
_elm_label_wrap_width_get(Eo *obj EINA_UNUSED, Elm_Label_Data *sd)
{
   return sd->wrap_w;
}

EOLIAN static void
_elm_label_ellipsis_set(Eo *obj, Elm_Label_Data *sd, Eina_Bool ellipsis)
{
   Eina_Strbuf *fontbuf = NULL;
   int len, removeflag = 0;
   const char *text;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   if (sd->ellipsis == ellipsis) return;
   sd->ellipsis = ellipsis;
   sd->lastw = -1;

   _elm_label_horizontal_size_policy_update(obj, sd);

   text = elm_layout_text_get(obj, NULL);
   if (!text) return;

   len = strlen(text);
   if (len <= 0) return;

   if (ellipsis == EINA_FALSE) removeflag = 1;  // remove fontsize tag

   fontbuf = eina_strbuf_new();
   eina_strbuf_append_printf(fontbuf, "%f", 1.0);

   if (_stringshare_key_value_replace
         (&sd->format, "ellipsis", eina_strbuf_string_get
           (fontbuf), removeflag) == 0)
     {
        _label_format_set(wd->resize_obj, sd->format);
        elm_layout_sizing_eval(obj);
     }
   eina_strbuf_free(fontbuf);
}

EOLIAN static Eina_Bool
_elm_label_ellipsis_get(Eo *obj EINA_UNUSED, Elm_Label_Data *sd)
{
   return sd->ellipsis;
}

EOLIAN static void
_elm_label_slide_mode_set(Eo *obj EINA_UNUSED, Elm_Label_Data *sd, Elm_Label_Slide_Mode mode)
{
   sd->slide_mode = mode;
}

EOLIAN static Elm_Label_Slide_Mode
_elm_label_slide_mode_get(Eo *obj EINA_UNUSED, Elm_Label_Data *sd)
{
   return sd->slide_mode;
}

EINA_DEPRECATED EAPI void
elm_label_slide_set(Evas_Object *obj, Eina_Bool slide)
{
   if (slide)
     elm_label_slide_mode_set(obj, ELM_LABEL_SLIDE_MODE_ALWAYS);
   else
     elm_label_slide_mode_set(obj, ELM_LABEL_SLIDE_MODE_NONE);
}

EINA_DEPRECATED EAPI Eina_Bool
elm_label_slide_get(const Evas_Object *obj)
{
   Eina_Bool ret = EINA_FALSE;
   if (elm_label_slide_mode_get(obj) == ELM_LABEL_SLIDE_MODE_ALWAYS)
     ret = EINA_TRUE;
   return ret;
}

EOLIAN static void
_elm_label_slide_duration_set(Eo *obj EINA_UNUSED, Elm_Label_Data *sd, double duration)
{
   sd->slide_duration = duration;
   sd->use_slide_speed = EINA_FALSE;
}

EOLIAN static void
_elm_label_slide_speed_set(Eo *obj EINA_UNUSED, Elm_Label_Data *sd, double speed)
{
   sd->slide_speed = speed;
   sd->use_slide_speed = EINA_TRUE;
}

EOLIAN static double
_elm_label_slide_speed_get(Eo *obj EINA_UNUSED, Elm_Label_Data *sd)
{
   return sd->slide_speed;
}

EOLIAN static void
_elm_label_slide_go(Eo *obj, Elm_Label_Data *sd)
{
   if (!sd->slide_state) sd->slide_state = EINA_TRUE;

   _label_slide_change(obj);
   elm_layout_sizing_eval(obj);
}

EOLIAN static double
_elm_label_slide_duration_get(Eo *obj EINA_UNUSED, Elm_Label_Data *sd)
{
   return sd->slide_duration;
}

EOLIAN static Eina_Bool
_elm_label_elm_widget_focus_next_manager_is(Eo *obj EINA_UNUSED, Elm_Label_Data *_pd EINA_UNUSED)
{
   return EINA_FALSE;
}

EOLIAN static Eina_Bool
_elm_label_elm_widget_focus_direction_manager_is(Eo *obj EINA_UNUSED, Elm_Label_Data *_pd EINA_UNUSED)
{
   return EINA_FALSE;
}

EOLIAN static void
_elm_label_class_constructor(Efl_Class *klass)
{
   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}

/* Efl.Part begin */
ELM_PART_OVERRIDE(elm_label, ELM_LABEL, ELM_LAYOUT, Elm_Label_Data, Elm_Part_Data)
ELM_PART_OVERRIDE_TEXT_SET(elm_label, ELM_LABEL, ELM_LAYOUT, Elm_Label_Data, Elm_Part_Data)

#include "elm_label_internal_part.eo.c"
/* Efl.Part end */

/* Internal EO APIs and hidden overrides */

#define ELM_LABEL_EXTRA_OPS \
   EFL_CANVAS_GROUP_ADD_OPS(elm_label)

#include "elm_label.eo.c"
