#include <Elementary.h>
#include "elm_priv.h"
#include "elm_widget_label.h"

EAPI const char ELM_LABEL_SMART_NAME[] = "elm_label";

static const Elm_Layout_Part_Alias_Description _text_aliases[] =
{
   {"default", "elm.text"},
   {NULL, NULL}
};

EVAS_SMART_SUBCLASS_NEW
  (ELM_LABEL_SMART_NAME, _elm_label, Elm_Label_Smart_Class,
  Elm_Layout_Smart_Class, elm_layout_smart_class_get, NULL);

static void
_recalc(void *data)
{
   ELM_LABEL_DATA_GET(data, sd);

   Evas_Coord minw = -1, minh = -1;
   Evas_Coord resw;

   evas_event_freeze(evas_object_evas_get(data));
   evas_object_geometry_get
     (ELM_WIDGET_DATA(sd)->resize_obj, NULL, NULL, &resw, NULL);
   if (sd->wrap_w > resw)
     resw = sd->wrap_w;

   edje_object_size_min_restricted_calc
     (ELM_WIDGET_DATA(sd)->resize_obj, &minw, &minh, resw, 0);

   /* This is a hack to workaround the way min size hints are treated.
    * If the minimum width is smaller than the restricted width, it means
    * the mininmum doesn't matter. */
   if ((minw <= resw) && (minw != sd->wrap_w))
     {
        Evas_Coord ominw = -1;

        evas_object_size_hint_min_get(data, &ominw, NULL);
        minw = ominw;
     }

   evas_object_size_hint_min_set(data, minw, minh);
   evas_event_thaw(evas_object_evas_get(data));
   evas_event_thaw_eval(evas_object_evas_get(data));
}

static void
_label_format_set(Evas_Object *obj,
                  const char *format)
{
   if (format)
     edje_object_part_text_style_user_push(obj, "elm.text", format);
   else
     edje_object_part_text_style_user_pop(obj, "elm.text");
}

static void
_label_sliding_change(Evas_Object *obj)
{
   char *plaintxt;
   int plainlen = 0;

   ELM_LABEL_DATA_GET(obj, sd);

   // doesn't support multiline sliding effect
   if (sd->linewrap)
     {
        sd->slidingmode = EINA_FALSE;
        return;
     }

   plaintxt = _elm_util_mkup_to_text
       (edje_object_part_text_get
         (ELM_WIDGET_DATA(sd)->resize_obj, "elm.text"));
   if (plaintxt != NULL)
     {
        plainlen = strlen(plaintxt);
        free(plaintxt);
     }
   // too short to slide label
   if (plainlen < 1)
     {
        sd->slidingmode = EINA_TRUE;
        return;
     }

   if (sd->slidingmode)
     {
        Edje_Message_Float_Set *msg =
          alloca(sizeof(Edje_Message_Float_Set) + (sizeof(double)));

        if (sd->ellipsis)
          {
             sd->slidingellipsis = EINA_TRUE;
             elm_label_ellipsis_set(obj, EINA_FALSE);
          }

        msg->count = 1;
        msg->val[0] = sd->slide_duration;

        edje_object_message_send
          (ELM_WIDGET_DATA(sd)->resize_obj, EDJE_MESSAGE_FLOAT_SET, 0, msg);
        edje_object_signal_emit
          (ELM_WIDGET_DATA(sd)->resize_obj, "elm,state,slide,start", "elm");
     }
   else
     {
        edje_object_signal_emit
          (ELM_WIDGET_DATA(sd)->resize_obj, "elm,state,slide,stop", "elm");
        if (sd->slidingellipsis)
          {
             sd->slidingellipsis = EINA_FALSE;
             elm_label_ellipsis_set(obj, EINA_TRUE);
          }
     }
}

static Eina_Bool
_elm_label_smart_theme(Evas_Object *obj)
{
   Eina_Bool ret;

   ELM_LABEL_DATA_GET(obj, sd);

   evas_event_freeze(evas_object_evas_get(obj));

   ret = ELM_WIDGET_CLASS(_elm_label_parent_sc)->theme(obj);
   if (!ret) goto end;

   _label_format_set(ELM_WIDGET_DATA(sd)->resize_obj, sd->format);
   _label_sliding_change(obj);

end:
   evas_event_thaw(evas_object_evas_get(obj));
   evas_event_thaw_eval(evas_object_evas_get(obj));

   return ret;
}

static void
_elm_label_smart_sizing_eval(Evas_Object *obj)
{
   Evas_Coord minw = -1, minh = -1;
   Evas_Coord resw, resh;

   ELM_LABEL_DATA_GET(obj, sd);

   if (sd->linewrap)
     {
        evas_object_geometry_get
          (ELM_WIDGET_DATA(sd)->resize_obj, NULL, NULL, &resw, &resh);
        if (resw == sd->lastw) return;
        sd->lastw = resw;
        _recalc(obj);
     }
   else
     {
        evas_event_freeze(evas_object_evas_get(obj));
        evas_object_geometry_get
          (ELM_WIDGET_DATA(sd)->resize_obj, NULL, NULL, &resw, &resh);
        edje_object_size_min_calc
          (ELM_WIDGET_DATA(sd)->resize_obj, &minw, &minh);
        if (sd->wrap_w > 0 && minw > sd->wrap_w) minw = sd->wrap_w;
        evas_object_size_hint_min_set(obj, minw, minh);
        evas_event_thaw(evas_object_evas_get(obj));
        evas_event_thaw_eval(evas_object_evas_get(obj));
     }
}

static void
_on_label_resize(void *data,
                 Evas *e __UNUSED__,
                 Evas_Object *obj __UNUSED__,
                 void *event_info __UNUSED__)
{
   ELM_LABEL_DATA_GET(data, sd);

   if (sd->linewrap) elm_layout_sizing_eval(data);
}

static int
_get_value_in_key_string(const char *oldstring,
                         const char *key,
                         char **value)
{
   char *curlocater, *endtag;
   int firstindex = 0, foundflag = -1;

   curlocater = strstr(oldstring, key);
   if (curlocater)
     {
        int key_len = strlen(key);
        endtag = curlocater + key_len;
        if ((!endtag) || (*endtag != '='))
          {
             foundflag = 0;
             return -1;
          }
        firstindex = abs(oldstring - curlocater);
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
_strbuf_key_value_replace(Eina_Strbuf *srcbuf,
                          const char *key,
                          const char *value,
                          int deleteflag)
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
             eina_strbuf_insert_printf
               (srcbuf, " %s=%s", eina_strbuf_length_get(srcbuf) - 1, key,
               value);
          }
        else
          {
             eina_strbuf_append_printf(srcbuf, "DEFAULT='%s=%s'", key, value);
          }
     }

   return 0;
}

static int
_stringshare_key_value_replace(const char **srcstring,
                               const char *key,
                               const char *value,
                               int deleteflag)
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
_elm_label_smart_text_set(Evas_Object *obj,
                          const char *item,
                          const char *label)
{
   ELM_LABEL_DATA_GET(obj, sd);

   if (!label) label = "";
   _label_format_set(ELM_WIDGET_DATA(sd)->resize_obj, sd->format);

   return _elm_label_parent_sc->text_set(obj, item, label);
}

static Eina_Bool
_elm_label_smart_translate(Evas_Object *obj)
{
   evas_object_smart_callback_call(obj, "language,changed", NULL);

   return EINA_TRUE;
}

static char *
_access_info_cb(void *data __UNUSED__,
                Evas_Object *obj,
                Elm_Widget_Item *item __UNUSED__)
{
   const char *txt = elm_widget_access_info_get(obj);

   if (!txt) txt = elm_layout_text_get(obj, NULL);
   if (txt) return strdup(txt);

   return NULL;
}

static void
_elm_label_smart_add(Evas_Object *obj)
{
   EVAS_SMART_DATA_ALLOC(obj, Elm_Label_Smart_Data);

   ELM_WIDGET_CLASS(_elm_label_parent_sc)->base.add(obj);

   priv->linewrap = ELM_WRAP_NONE;
   priv->ellipsis = EINA_FALSE;
   priv->slidingmode = EINA_FALSE;
   priv->slidingellipsis = EINA_FALSE;
   priv->wrap_w = -1;
   priv->slide_duration = 10;

   priv->format = eina_stringshare_add("");
   _label_format_set(ELM_WIDGET_DATA(priv)->resize_obj, priv->format);

   evas_object_event_callback_add
     (ELM_WIDGET_DATA(priv)->resize_obj, EVAS_CALLBACK_RESIZE,
     _on_label_resize, obj);

   elm_widget_can_focus_set(obj, EINA_FALSE);

   elm_layout_theme_set(obj, "label", "base", elm_widget_style_get(obj));
   elm_layout_text_set(obj, NULL, "<br>");

   _elm_access_object_register(obj, ELM_WIDGET_DATA(priv)->resize_obj);
   _elm_access_text_set
     (_elm_access_object_get(obj), ELM_ACCESS_TYPE, E_("Label"));
   _elm_access_callback_set
     (_elm_access_object_get(obj), ELM_ACCESS_INFO, _access_info_cb, NULL);
   
   elm_layout_sizing_eval(obj);
}

static void
_elm_label_smart_set_user(Elm_Label_Smart_Class *sc)
{
   ELM_WIDGET_CLASS(sc)->base.add = _elm_label_smart_add;

   /* not a 'focus chain manager' */
   ELM_WIDGET_CLASS(sc)->focus_next = NULL;
   ELM_WIDGET_CLASS(sc)->focus_direction = NULL;

   ELM_WIDGET_CLASS(sc)->theme = _elm_label_smart_theme;
   ELM_WIDGET_CLASS(sc)->translate = _elm_label_smart_translate;

   ELM_LAYOUT_CLASS(sc)->sizing_eval = _elm_label_smart_sizing_eval;
   ELM_LAYOUT_CLASS(sc)->text_set = _elm_label_smart_text_set;

   ELM_LAYOUT_CLASS(sc)->text_aliases = _text_aliases;
}

EAPI const Elm_Label_Smart_Class *
elm_label_smart_class_get(void)
{
   static Elm_Label_Smart_Class _sc =
     ELM_LABEL_SMART_CLASS_INIT_NAME_VERSION(ELM_LABEL_SMART_NAME);
   static const Elm_Label_Smart_Class *class = NULL;

   if (class)
     return class;

   _elm_label_smart_set(&_sc);
   class = &_sc;

   return class;
}

EAPI Evas_Object *
elm_label_add(Evas_Object *parent)
{
   Evas_Object *obj;

   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);

   obj = elm_widget_add(_elm_label_smart_class_new(), parent);
   if (!obj) return NULL;

   if (!elm_widget_sub_object_add(parent, obj))
     ERR("could not add %p as sub object of %p", obj, parent);

   return obj;
}

EAPI void
elm_label_line_wrap_set(Evas_Object *obj,
                        Elm_Wrap_Type wrap)
{
   const char *wrap_str, *text;
   int len;

   ELM_LABEL_CHECK(obj);
   ELM_LABEL_DATA_GET(obj, sd);

   if (sd->linewrap == wrap) return;

   sd->linewrap = wrap;
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
        _label_format_set(ELM_WIDGET_DATA(sd)->resize_obj, sd->format);
        elm_layout_sizing_eval(obj);
     }
}

EAPI Elm_Wrap_Type
elm_label_line_wrap_get(const Evas_Object *obj)
{
   ELM_LABEL_CHECK(obj) EINA_FALSE;
   ELM_LABEL_DATA_GET(obj, sd);

   return sd->linewrap;
}

EAPI void
elm_label_wrap_width_set(Evas_Object *obj,
                         Evas_Coord w)
{
   ELM_LABEL_CHECK(obj);
   ELM_LABEL_DATA_GET(obj, sd);

   if (w < 0) w = 0;

   if (sd->wrap_w == w) return;

   if (sd->ellipsis)
     _label_format_set(ELM_WIDGET_DATA(sd)->resize_obj, sd->format);
   sd->wrap_w = w;

   elm_layout_sizing_eval(obj);
}

EAPI Evas_Coord
elm_label_wrap_width_get(const Evas_Object *obj)
{
   ELM_LABEL_CHECK(obj) 0;
   ELM_LABEL_DATA_GET(obj, sd);

   return sd->wrap_w;
}

EAPI void
elm_label_ellipsis_set(Evas_Object *obj,
                       Eina_Bool ellipsis)
{
   Eina_Strbuf *fontbuf = NULL;
   int len, removeflag = 0;
   const char *text;

   ELM_LABEL_CHECK(obj);
   ELM_LABEL_DATA_GET(obj, sd);

   if (sd->ellipsis == ellipsis) return;
   sd->ellipsis = ellipsis;

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
        _label_format_set(ELM_WIDGET_DATA(sd)->resize_obj, sd->format);
        elm_layout_sizing_eval(obj);
     }
   eina_strbuf_free(fontbuf);
}

EAPI Eina_Bool
elm_label_ellipsis_get(const Evas_Object *obj)
{
   ELM_LABEL_CHECK(obj) EINA_FALSE;
   ELM_LABEL_DATA_GET(obj, sd);

   return sd->ellipsis;
}

EAPI void
elm_label_slide_set(Evas_Object *obj,
                    Eina_Bool slide)
{
   ELM_LABEL_CHECK(obj);
   ELM_LABEL_DATA_GET(obj, sd);

   if (sd->slidingmode == slide) return;
   sd->slidingmode = slide;

   _label_sliding_change(obj);
   elm_layout_sizing_eval(obj);
}

EAPI Eina_Bool
elm_label_slide_get(const Evas_Object *obj)
{
   ELM_LABEL_CHECK(obj) EINA_FALSE;
   ELM_LABEL_DATA_GET(obj, sd);

   return sd->slidingmode;
}

EAPI void
elm_label_slide_duration_set(Evas_Object *obj, double duration)
{
   ELM_LABEL_CHECK(obj);
   ELM_LABEL_DATA_GET(obj, sd);

   Edje_Message_Float_Set *msg =
     alloca(sizeof(Edje_Message_Float_Set) + (sizeof(double)));

   sd->slide_duration = duration;
   msg->count = 1;
   msg->val[0] = sd->slide_duration;
   edje_object_message_send
     (ELM_WIDGET_DATA(sd)->resize_obj, EDJE_MESSAGE_FLOAT_SET, 0, msg);
}

EAPI double
elm_label_slide_duration_get(const Evas_Object *obj)
{
   ELM_LABEL_CHECK(obj) 0.0;
   ELM_LABEL_DATA_GET(obj, sd);

   return sd->slide_duration;
}
