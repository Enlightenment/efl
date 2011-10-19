#include <Elementary.h>
#include "elm_priv.h"

typedef struct _Widget_Data Widget_Data;

struct _Widget_Data
{
   Evas_Object *lbl;
   Evas_Object *bg;
   const char *label;
   const char *format;
   Ecore_Job *deferred_recalc_job;
   double slide_duration;
   Evas_Coord lastw;
   Evas_Coord wrap_w;
   Evas_Coord wrap_h;
   Elm_Wrap_Type linewrap;
   Eina_Bool changed : 1;
   Eina_Bool bgcolor : 1;
   Eina_Bool ellipsis : 1;
   Eina_Bool slidingmode : 1;
   Eina_Bool slidingellipsis : 1;
};

static const char *widtype = NULL;
static void _del_hook(Evas_Object *obj);
static void _mirrored_set(Evas_Object *obj, Eina_Bool rtl);
static void _theme_hook(Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);
static int _get_value_in_key_string(const char *oldstring, const char *key, char **value);
static int _strbuf_key_value_replace(Eina_Strbuf *srcbuf, const char *key, const char *value, int deleteflag);
static int _stringshare_key_value_replace(const char **srcstring, const char *key, const char *value, int deleteflag);
static int _is_width_over(Evas_Object *obj);
static void _ellipsis_label_to_width(Evas_Object *obj);
static void _label_sliding_change(Evas_Object *obj);

static void
_elm_recalc_job(void *data)
{
   Widget_Data *wd = elm_widget_data_get(data);
   Evas_Coord minw = -1, minh = -1;
   Evas_Coord resw;
   if (!wd) return;
   evas_event_freeze(evas_object_evas_get(data));
   wd->deferred_recalc_job = NULL;
   evas_object_geometry_get(wd->lbl, NULL, NULL, &resw, NULL);
   if (wd->wrap_w > resw)
      resw = wd->wrap_w;

   edje_object_size_min_restricted_calc(wd->lbl, &minw, &minh, resw, 0);
   /* This is a hack to workaround the way min size hints are treated.
    * If the minimum width is smaller than the restricted width, it means
    * the mininmum doesn't matter. */
   if ((minw <= resw) && (minw != wd->wrap_w))
     {
        Evas_Coord ominw = -1;
        evas_object_size_hint_min_get(data, &ominw, NULL);
        minw = ominw;
     }
   evas_object_size_hint_min_set(data, minw, minh);
   evas_object_size_hint_max_set(data, wd->wrap_w, wd->wrap_h);

   if ((wd->ellipsis) && (wd->linewrap) && (wd->wrap_h > 0) &&
       (_is_width_over(data) == 1))
     _ellipsis_label_to_width(data);
   evas_event_thaw(evas_object_evas_get(data));
   evas_event_thaw_eval(evas_object_evas_get(data));
}

static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   evas_event_freeze(evas_object_evas_get(obj));
   if (wd->deferred_recalc_job) ecore_job_del(wd->deferred_recalc_job);
   if (wd->label) eina_stringshare_del(wd->label);
   if (wd->bg) evas_object_del(wd->bg);
   free(wd);
   evas_event_thaw(evas_object_evas_get(obj));
   evas_event_thaw_eval(evas_object_evas_get(obj));
}

static void
_theme_change(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;

   _elm_theme_object_set(obj, wd->lbl, "label", "base",
         elm_widget_style_get(obj));
}

static void
_mirrored_set(Evas_Object *obj, Eina_Bool rtl)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   edje_object_mirrored_set(wd->lbl, rtl);
}

static void
_theme_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   evas_event_freeze(evas_object_evas_get(obj));
   _elm_widget_mirrored_reload(obj);
   _mirrored_set(obj, elm_widget_mirrored_get(obj));
   _theme_change(obj);
   edje_object_part_text_set(wd->lbl, "elm.text", wd->format);
   edje_object_part_text_append(wd->lbl, "elm.text", wd->label);
   edje_object_scale_set(wd->lbl, elm_widget_scale_get(obj) *
                         _elm_config->scale);
   _label_sliding_change(obj);
   _sizing_eval(obj);
   evas_event_thaw(evas_object_evas_get(obj));
   evas_event_thaw_eval(evas_object_evas_get(obj));
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord minw = -1, minh = -1;
   Evas_Coord resw, resh;
   if (!wd) return;

   if (wd->linewrap)
     {
        evas_object_geometry_get(wd->lbl, NULL, NULL, &resw, &resh);
        if ((resw == wd->lastw) && (!wd->changed)) return;
        wd->changed = EINA_FALSE;
        wd->lastw = resw;
        _elm_recalc_job(obj);
        // FIXME: works ok. but NOT for genlist. what should genlist do?
        //        if (wd->deferred_recalc_job) ecore_job_del(wd->deferred_recalc_job);
        //        wd->deferred_recalc_job = ecore_job_add(_elm_recalc_job, obj);
     }
   else
     {
        evas_event_freeze(evas_object_evas_get(obj));
        evas_object_geometry_get(wd->lbl, NULL, NULL, &resw, &resh);
        edje_object_size_min_calc(wd->lbl, &minw, &minh);
        if (wd->wrap_w > 0 && minw > wd->wrap_w) minw = wd->wrap_w;
        if (wd->wrap_h > 0 && minh > wd->wrap_h) minh = wd->wrap_h;
        evas_object_size_hint_min_set(obj, minw, minh);
        evas_object_size_hint_max_set(obj, wd->wrap_w, wd->wrap_h);
        if ((wd->ellipsis) && (_is_width_over(obj) == 1))
          _ellipsis_label_to_width(obj);
        evas_event_thaw(evas_object_evas_get(obj));
        evas_event_thaw_eval(evas_object_evas_get(obj));
     }
}

static void
_lbl_resize(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;
   if (wd->linewrap) _sizing_eval(data);
}

static int
_get_value_in_key_string(const char *oldstring, const char *key, char **value)
{
   char *curlocater, *starttag, *endtag;
   int firstindex = 0, foundflag = -1;

   curlocater = strstr(oldstring, key);
   if (curlocater)
     {
        int key_len = strlen(key);
        starttag = curlocater;
        endtag = curlocater + key_len;
        if ((!endtag) || (*endtag != '='))
          {
             foundflag = 0;
             return -1;
          }
        firstindex = abs(oldstring - curlocater);
        firstindex += key_len + 1; // strlen("key") + strlen("=")
        *value = (char *)oldstring + firstindex;

        while (oldstring != starttag)
          {
             if (*starttag == '>')
               {
                  foundflag = 0;
                  break;
               }
             if (*starttag == '<')
               break;
             else
               starttag--;
             if (!starttag) break;
          }

        while (endtag)
          {
             if (*endtag == '<')
               {
                  foundflag = 0;
                  break;
               }
             if (*endtag == '>')
               break;
             else
               endtag++;
             if (!endtag) break;
          }

        if ((foundflag) && (*starttag == '<') && (*endtag == '>'))
          foundflag = 1;
        else
          foundflag = 0;
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
   const char *srcstring = NULL;
   Eina_Strbuf *repbuf = NULL, *diffbuf = NULL;
   char *curlocater, *replocater;
   char *starttag, *endtag;
   int tagtxtlen = 0, insertflag = 0;

   srcstring = eina_strbuf_string_get(srcbuf);
   curlocater = strstr(srcstring, key);

   if (!curlocater)
     insertflag = 1;
   else
     {
        int key_len = strlen(key);
        do
          {
             starttag = strchr(srcstring, '<');
             endtag = strchr(srcstring, '>');
             tagtxtlen = endtag - starttag;
             if (tagtxtlen <= 0) tagtxtlen = 0;
             if ((starttag < curlocater) && (curlocater < endtag)) break;
             if ((endtag) && ((endtag + 1)))
               srcstring = endtag + 1;
             else
               break;
          } while (strlen(srcstring) > 1);

        if ((starttag) && (endtag) && (tagtxtlen > key_len))
          {
             char *eqchar = NULL;
             repbuf = eina_strbuf_new();
             diffbuf = eina_strbuf_new();
             eina_strbuf_append_n(repbuf, starttag, tagtxtlen);
             srcstring = eina_strbuf_string_get(repbuf);
             curlocater = strstr(srcstring, key);
             // key=value
             //    ^ : move to here
             eqchar = curlocater + key_len;
             if ((curlocater) && (eqchar))
               {
                  // some case at useless many whitespaces (key  =value)
                  // find the separator(=) position
                  eqchar = strchr(curlocater + key_len, '=');
                  if (eqchar)
                    {
                       // key=value
                       //     ^ : move to here
                       replocater = eqchar + 1;
                       while ((*replocater) &&
                              (*replocater != ' ') &&
                              (*replocater != '>'))
                         replocater++;

                       if ((replocater - curlocater) > key_len)
                         eina_strbuf_append_n(diffbuf, curlocater,
                                              replocater-curlocater);
                       else
                         insertflag = 1;
                    }
                  else
                    insertflag = 1;
               }
             else
               insertflag = 1;
             eina_strbuf_reset(repbuf);
          }
        else
          insertflag = 1;
     }

   if (!repbuf) repbuf = eina_strbuf_new();
   if (!diffbuf) diffbuf = eina_strbuf_new();

   if (insertflag)
     {
        eina_strbuf_append_printf(repbuf, "<%s=%s>", key, value);
        eina_strbuf_prepend(srcbuf, eina_strbuf_string_get(repbuf));
     }
   else
     {
        if (deleteflag)
          {
             eina_strbuf_prepend(diffbuf, "<");
             eina_strbuf_append(diffbuf, ">");
             eina_strbuf_replace_first(srcbuf, eina_strbuf_string_get(diffbuf), "");
          }
        else
          {
             eina_strbuf_append_printf(repbuf, "%s=%s", key, value);
             eina_strbuf_replace_first(srcbuf, eina_strbuf_string_get(diffbuf), eina_strbuf_string_get(repbuf));
          }
     }

   if (repbuf) eina_strbuf_free(repbuf);
   if (diffbuf) eina_strbuf_free(diffbuf);

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

static int
_is_width_over(Evas_Object *obj)
{
   Evas_Coord x, y, w, h;
   Evas_Coord vx, vy, vw, vh;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return 0;

   evas_event_freeze(evas_object_evas_get(obj));
   edje_object_part_geometry_get(wd->lbl, "elm.text", &x, &y, NULL, NULL);
   /* Calc the formatted size with ellipsis turned off */
   if (wd->ellipsis)
     {
        const Evas_Object *tb;
        char *_kvalue;
        double ellipsis = 0.0;
        Eina_Bool found_key = EINA_FALSE;
        if (_get_value_in_key_string(wd->format, "ellipsis", &_kvalue) == 0)
          {
             ellipsis = atof(_kvalue);
             found_key = EINA_TRUE;
          }

        if (_stringshare_key_value_replace(&wd->format,
                 "ellipsis", NULL, 1) == 0)
          {
             edje_object_part_text_set(wd->lbl, "elm.text", wd->format);
             edje_object_part_text_append(wd->lbl, "elm.text", wd->label);
          }

        tb = edje_object_part_object_get(wd->lbl, "elm.text");
        evas_object_textblock_size_formatted_get(tb, &w, &h);

        if (found_key)
          {
             Eina_Strbuf *elpbuf;
             elpbuf = eina_strbuf_new();
             eina_strbuf_append_printf(elpbuf, "%f", ellipsis);
             if (_stringshare_key_value_replace(&wd->format, "ellipsis",
                      eina_strbuf_string_get(elpbuf), 0) == 0)
               {
                  edje_object_part_text_set(wd->lbl, "elm.text", wd->format);
                  edje_object_part_text_append(wd->lbl, "elm.text", wd->label);
               }
             eina_strbuf_free(elpbuf);
          }
     }
   else
     {
        const Evas_Object *tb;
        tb = edje_object_part_object_get(wd->lbl, "elm.text");
        evas_object_textblock_size_formatted_get(tb, &w, &h);
     }
   evas_object_geometry_get(obj, &vx, &vy, &vw, &vh);
   evas_event_thaw(evas_object_evas_get(obj));
   evas_event_thaw_eval(evas_object_evas_get(obj));

   if (w > wd->wrap_w || h > wd->wrap_h)
      return 1;

   return 0;
}

static void
_ellipsis_fontsize_set(Evas_Object *obj, int fontsize)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Eina_Strbuf *fontbuf = NULL;
   int removeflag = 0;
   if (!wd) return;

   fontbuf = eina_strbuf_new();
   eina_strbuf_append_printf(fontbuf, "%d", fontsize);
   if (fontsize == 0) removeflag = 1;  // remove fontsize tag

   if (_stringshare_key_value_replace(&wd->format, "font_size", eina_strbuf_string_get(fontbuf), removeflag) == 0)
     {
        edje_object_part_text_set(wd->lbl, "elm.text", wd->format);
        edje_object_part_text_append(wd->lbl, "elm.text", wd->label);
     }
   eina_strbuf_free(fontbuf);
}

static void
_ellipsis_label_to_width(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   int cur_fontsize = 0;
   char *kvalue;
   const char *minfont, *deffont, *maxfont;
   int minfontsize, maxfontsize;

   evas_event_freeze(evas_object_evas_get(obj));
   minfont = edje_object_data_get(wd->lbl, "min_font_size");
   if (minfont) minfontsize = atoi(minfont);
   else minfontsize = 1;
   maxfont = edje_object_data_get(wd->lbl, "max_font_size");
   if (maxfont) maxfontsize = atoi(maxfont);
   else maxfontsize = 1;
   deffont = edje_object_data_get(wd->lbl, "default_font_size");
   if (deffont) cur_fontsize = atoi(deffont);
   else cur_fontsize = 1;
   if (minfontsize > maxfontsize || cur_fontsize == 1) return;  // theme is not ready for ellipsis
   if (eina_stringshare_strlen(wd->label) <= 0) return;

   if (_get_value_in_key_string(wd->format, "font_size", &kvalue) == 0)
     {
        if (kvalue != NULL) cur_fontsize = atoi(kvalue);
     }

   while (_is_width_over(obj))
     {
        if (cur_fontsize > minfontsize)
          {
             cur_fontsize -= 3;
             if (cur_fontsize < minfontsize) cur_fontsize = minfontsize;
             _ellipsis_fontsize_set(obj, cur_fontsize);
          }
        else
          {
             break;
          }
     }
   evas_event_thaw(evas_object_evas_get(obj));
   evas_event_thaw_eval(evas_object_evas_get(obj));
}

static void
_label_sliding_change(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   char *plaintxt;
   int plainlen = 0;

   // dosen't support multiline sliding effect
   if (wd->linewrap)
     {
        wd->slidingmode = EINA_FALSE;
        return;
     }

   plaintxt = _elm_util_mkup_to_text(edje_object_part_text_get(wd->lbl, "elm.text"));
   if (plaintxt != NULL)
     {
        plainlen = strlen(plaintxt);
        free(plaintxt);
     }
   // too short to slide label
   if (plainlen < 1)
     {
        wd->slidingmode = EINA_TRUE;
        return;
     }

   if (wd->slidingmode)
     {
        Edje_Message_Float_Set *msg = alloca(sizeof(Edje_Message_Float_Set) + (sizeof(double)));

        if (wd->ellipsis)
          {
             wd->slidingellipsis = EINA_TRUE;
             elm_label_ellipsis_set(obj, EINA_FALSE);
          }

        msg->count = 1;
        msg->val[0] = wd->slide_duration;

        edje_object_message_send(wd->lbl, EDJE_MESSAGE_FLOAT_SET, 0, msg);
        edje_object_signal_emit(wd->lbl, "elm,state,slide,start", "elm");
     }
   else
     {
        edje_object_signal_emit(wd->lbl, "elm,state,slide,stop", "elm");
        if (wd->slidingellipsis)
          {
             wd->slidingellipsis = EINA_FALSE;
             elm_label_ellipsis_set(obj, EINA_TRUE);
          }
     }
}

static void
_elm_label_label_set(Evas_Object *obj, const char *item, const char *label)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (item && strcmp(item, "default")) return;
   if (!label) label = "";
   eina_stringshare_replace(&wd->label, label);
   edje_object_part_text_set(wd->lbl, "elm.text", wd->format);
   edje_object_part_text_append(wd->lbl, "elm.text", wd->label);
   wd->changed = 1;
   _sizing_eval(obj);
}

static const char *
_elm_label_label_get(const Evas_Object *obj, const char *item)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (item && strcmp(item, "default")) return NULL;
   if (!wd) return NULL;
   return wd->label;
}

static void
_translate_hook(Evas_Object *obj)
{
   evas_object_smart_callback_call(obj, "language,changed", NULL);
}

EAPI Evas_Object *
elm_label_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas *e;
   Widget_Data *wd;

   ELM_WIDGET_STANDARD_SETUP(wd, Widget_Data, parent, e, obj, NULL);

   ELM_SET_WIDTYPE(widtype, "label");
   elm_widget_type_set(obj, "label");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);
   elm_widget_can_focus_set(obj, EINA_FALSE);
   elm_widget_text_set_hook_set(obj, _elm_label_label_set);
   elm_widget_text_get_hook_set(obj, _elm_label_label_get);
   elm_widget_translate_hook_set(obj, _translate_hook);

   wd->bgcolor = EINA_FALSE;
   wd->bg = evas_object_rectangle_add(e);
   evas_object_color_set(wd->bg, 0, 0, 0, 0);

   wd->linewrap = ELM_WRAP_NONE;
   wd->ellipsis = EINA_FALSE;
   wd->slidingmode = EINA_FALSE;
   wd->slidingellipsis = EINA_FALSE;
   wd->wrap_w = -1;
   wd->wrap_h = -1;
   wd->slide_duration = 10;

   wd->lbl = edje_object_add(e);
   _elm_theme_object_set(obj, wd->lbl, "label", "base", "default");
   wd->format = eina_stringshare_add("");
   wd->label = eina_stringshare_add("<br>");
   edje_object_part_text_set(wd->lbl, "elm.text", wd->format);
   edje_object_part_text_append(wd->lbl, "elm.text", wd->label);

   elm_widget_resize_object_set(obj, wd->lbl);

   evas_object_event_callback_add(wd->lbl, EVAS_CALLBACK_RESIZE, _lbl_resize, obj);

   _mirrored_set(obj, elm_widget_mirrored_get(obj));
   wd->changed = 1;
   _sizing_eval(obj);
   return obj;
}

EAPI void
elm_label_label_set(Evas_Object *obj, const char *label)
{
  _elm_label_label_set(obj, NULL, label);
}

EAPI const char *
elm_label_label_get(const Evas_Object *obj)
{
  return _elm_label_label_get(obj, NULL);
}

EAPI void
elm_label_line_wrap_set(Evas_Object *obj, Elm_Wrap_Type wrap)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   const char *wrap_str;
   int len;

   if (!wd) return;
   if (wd->linewrap == wrap) return;
   wd->linewrap = wrap;
   len = strlen(wd->label);
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

   if (_stringshare_key_value_replace(&wd->format,
            "wrap", wrap_str, 0) == 0)
     {
        edje_object_part_text_set(wd->lbl, "elm.text", wd->format);
        edje_object_part_text_append(wd->lbl, "elm.text", wd->label);
        wd->changed = 1;
        _sizing_eval(obj);
     }
}

EAPI Elm_Wrap_Type
elm_label_line_wrap_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return wd->linewrap;
}

EAPI void
elm_label_wrap_width_set(Evas_Object *obj, Evas_Coord w)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (w < 0) w = 0;
   if (wd->wrap_w == w) return;
   if (wd->ellipsis)
     {
        edje_object_part_text_set(wd->lbl, "elm.text", wd->format);
        edje_object_part_text_append(wd->lbl, "elm.text", wd->label);
     }
   wd->wrap_w = w;
   _sizing_eval(obj);
}

EAPI Evas_Coord
elm_label_wrap_width_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) 0;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return 0;
   return wd->wrap_w;
}

EAPI void
elm_label_wrap_height_set(Evas_Object *obj,
                          Evas_Coord   h)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (h < 0) h = 0;
   if (wd->wrap_h == h) return;
   if (wd->ellipsis)
     {
        edje_object_part_text_set(wd->lbl, "elm.text", wd->format);
        edje_object_part_text_append(wd->lbl, "elm.text", wd->label);
     }
   wd->wrap_h = h;
   _sizing_eval(obj);
}

EAPI Evas_Coord
elm_label_wrap_height_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) 0;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return 0;
   return wd->wrap_h;
}

EAPI void
elm_label_fontsize_set(Evas_Object *obj, int fontsize)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   Eina_Strbuf *fontbuf = NULL;
   int len, removeflag = 0;

   if (!wd) return;
   _elm_dangerous_call_check(__FUNCTION__);
   len = strlen(wd->label);
   if (len <= 0) return;
   fontbuf = eina_strbuf_new();
   eina_strbuf_append_printf(fontbuf, "%d", fontsize);

   if (fontsize == 0) removeflag = 1;  // remove fontsize tag

   if (_stringshare_key_value_replace(&wd->format, "font_size", eina_strbuf_string_get(fontbuf), removeflag) == 0)
     {
        edje_object_part_text_set(wd->lbl, "elm.text", wd->format);
        edje_object_part_text_append(wd->lbl, "elm.text", wd->label);
        wd->changed = 1;
        _sizing_eval(obj);
     }
   eina_strbuf_free(fontbuf);
}

EAPI void
elm_label_text_align_set(Evas_Object *obj, const char *alignmode)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   int len;

   if (!wd) return;
   _elm_dangerous_call_check(__FUNCTION__);
   len = strlen(wd->label);
   if (len <= 0) return;

   if (_stringshare_key_value_replace(&wd->format, "align", alignmode, 0) == 0)
     {
        edje_object_part_text_set(wd->lbl, "elm.text", wd->format);
        edje_object_part_text_append(wd->lbl, "elm.text", wd->label);
     }

   wd->changed = 1;
   _sizing_eval(obj);
}

EAPI void
elm_label_text_color_set(Evas_Object *obj,
                         unsigned int r,
                         unsigned int g,
                         unsigned int b,
                         unsigned int a)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   Eina_Strbuf *colorbuf = NULL;
   int len;

   if (!wd) return;
   _elm_dangerous_call_check(__FUNCTION__);
   len = strlen(wd->label);
   if (len <= 0) return;
   colorbuf = eina_strbuf_new();
   eina_strbuf_append_printf(colorbuf, "#%02X%02X%02X%02X", r, g, b, a);

   if (_stringshare_key_value_replace(&wd->format, "color", eina_strbuf_string_get(colorbuf), 0) == 0)
     {
        edje_object_part_text_set(wd->lbl, "elm.text", wd->format);
        edje_object_part_text_append(wd->lbl, "elm.text", wd->label);
        wd->changed = 1;
        _sizing_eval(obj);
     }
   eina_strbuf_free(colorbuf);
}

EAPI void
elm_label_background_color_set(Evas_Object *obj,
                               unsigned int r,
                               unsigned int g,
                               unsigned int b,
                               unsigned int a)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   evas_object_color_set(wd->bg, r, g, b, a);

   if (!wd) return;
   _elm_dangerous_call_check(__FUNCTION__);
   if (wd->bgcolor == EINA_FALSE)
     {
        wd->bgcolor = 1;
        edje_object_part_swallow(wd->lbl, "label.swallow.background", wd->bg);
     }
}

EAPI void
elm_label_ellipsis_set(Evas_Object *obj, Eina_Bool ellipsis)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   Eina_Strbuf *fontbuf = NULL;
   int len, removeflag = 0;

   if (!wd) return;
   if (wd->ellipsis == ellipsis) return;
   wd->ellipsis = ellipsis;
   len = strlen(wd->label);
   if (len <= 0) return;

   if (ellipsis == EINA_FALSE) removeflag = 1;  // remove fontsize tag

   fontbuf = eina_strbuf_new();
   eina_strbuf_append_printf(fontbuf, "%f", 1.0);

   if (_stringshare_key_value_replace(&wd->format,
            "ellipsis", eina_strbuf_string_get(fontbuf), removeflag) == 0)
     {
        edje_object_part_text_set(wd->lbl, "elm.text", wd->format);
        edje_object_part_text_append(wd->lbl, "elm.text", wd->label);
        wd->changed = 1;
        _sizing_eval(obj);
     }
   eina_strbuf_free(fontbuf);

}

EAPI void
elm_label_slide_set(Evas_Object *obj,
                    Eina_Bool    slide)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;

   if (wd->slidingmode == slide) return;
   wd->slidingmode = slide;
   _label_sliding_change(obj);
   wd->changed = 1;
   _sizing_eval(obj);
}

EAPI Eina_Bool
elm_label_slide_get(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return wd->slidingmode;
}

EAPI void
elm_label_slide_duration_set(Evas_Object *obj, double duration)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   Edje_Message_Float_Set *msg = alloca(sizeof(Edje_Message_Float_Set) + (sizeof(double)));

   if (!wd) return;
   wd->slide_duration = duration;
   msg->count = 1;
   msg->val[0] = wd->slide_duration;
   edje_object_message_send(wd->lbl, EDJE_MESSAGE_FLOAT_SET, 0, msg);
}

EAPI double
elm_label_slide_duration_get(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) 0.0;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return 0;
   return wd->slide_duration;
}

