#include <Elementary.h>
#include "elm_priv.h"

typedef struct _Widget_Data Widget_Data;

struct _Widget_Data
{
   Evas_Object *frm;
   Evas_Object *content;
   const char *label;
   unsigned int recalc_count;
   Eina_Bool collapsed : 1;
   Eina_Bool collapsible : 1;
   Eina_Bool anim : 1;
};

static const char SIG_CLICKED[] = "clicked";

static const Evas_Smart_Cb_Description _signals[] = {
   {SIG_CLICKED, ""},
   {NULL, NULL}
};

static const char *widtype = NULL;
static void _del_hook(Evas_Object *obj);
static void _mirrored_set(Evas_Object *obj, Eina_Bool rtl);
static void _theme_hook(Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);
static void _changed_size_hints(void *data,
                                Evas *e, Evas_Object *obj,
                                void *event_info);
static void _sub_del(void *data, Evas_Object *obj, void *event_info);

static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->label) eina_stringshare_del(wd->label);
   free(wd);
}

static void
_mirrored_set(Evas_Object *obj, Eina_Bool rtl)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   edje_object_mirrored_set(wd->frm, rtl);
}

static void
_theme_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   _elm_widget_mirrored_reload(obj);
   _mirrored_set(obj, elm_widget_mirrored_get(obj));
   _elm_theme_object_set(obj, wd->frm, "frame", "base",
                         elm_widget_style_get(obj));
   edje_object_part_text_set(wd->frm, "elm.text", wd->label);
   if (wd->content)
     edje_object_part_swallow(wd->frm, "elm.swallow.content", wd->content);
   edje_object_scale_set(wd->frm,
                         elm_widget_scale_get(obj) * _elm_config->scale);
   _sizing_eval(obj);
}

static Eina_Bool
_elm_frame_focus_next_hook(const Evas_Object *obj, Elm_Focus_Direction dir, Evas_Object **next)
{
   Widget_Data *wd = elm_widget_data_get(obj);

   if ((!wd) || (!wd->content))
     return EINA_FALSE;

   /* Try Focus cycle in subitem */
   return elm_widget_focus_next_get(wd->content, dir, next);
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord minw = -1, minh = -1;
   Evas_Coord cminw = -1, cminh = -1;
   if (!wd) return;
   edje_object_size_min_calc(wd->frm, &minw, &minh);
   evas_object_size_hint_min_get(obj, &cminw, &cminh);
   if ((minw == cminw) && (minh == cminh)) return;
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, -1, -1);
}

static void
_changed_size_hints(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;
   if (wd->anim) return;
   // FIXME: why is this needed? how does edje get this unswallowed or
   // lose its callbacks to edje
   edje_object_part_swallow(wd->frm, "elm.swallow.content", wd->content);
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
        evas_object_event_callback_del_full(sub,
                                            EVAS_CALLBACK_CHANGED_SIZE_HINTS,
                                            _changed_size_hints, obj);
        wd->content = NULL;
        _sizing_eval(obj);
     }
}

static void
_elm_frame_label_set(Evas_Object *obj, const char *item, const char *label)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (item && strcmp(item, "default")) return;
   if (!wd) return;
   eina_stringshare_replace(&(wd->label), label);
   edje_object_part_text_set(wd->frm, "elm.text", wd->label);
   _sizing_eval(obj);
}

static const char *
_elm_frame_label_get(const Evas_Object *obj, const char *item)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   if (item && strcmp(item, "default")) return NULL;
   return wd->label;
}

static void
_content_set_hook(Evas_Object *obj, const char *part, Evas_Object *content)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd;

   if (part && strcmp(part, "default")) return;
   wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->content == content) return;
   if (wd->content) evas_object_del(wd->content);
   wd->content = content;
   if (content)
     {
        elm_widget_sub_object_add(obj, content);
        evas_object_event_callback_add(content,
                                       EVAS_CALLBACK_CHANGED_SIZE_HINTS,
                                       _changed_size_hints, obj);
        edje_object_part_swallow(wd->frm, "elm.swallow.content", content);
     }
   _sizing_eval(obj);
}

static Evas_Object *
_content_get_hook(const Evas_Object *obj, const char *part)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd;

   if (part && strcmp(part, "default")) return NULL;
   wd = elm_widget_data_get(obj);
   if (!wd) return NULL;

   return wd->content;
}

static Evas_Object *
_content_unset_hook(Evas_Object *obj, const char *part)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd;
   Evas_Object *content;
   if (part && strcmp(part, "default")) return NULL;
   wd = elm_widget_data_get(obj);
   if (!wd || !wd->content) return NULL;
   content = wd->content;
   elm_widget_sub_object_del(obj, wd->content);
   edje_object_part_unswallow(wd->frm, content);
   return content;
}

static void
_recalc(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   _sizing_eval(data);
}

static void
_recalc_done(void *data, Evas_Object *obj __UNUSED__, const char *sig __UNUSED__, const char *src __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;
   evas_object_smart_callback_del(wd->frm, "recalc", _recalc);
   wd->anim = EINA_FALSE;
   _sizing_eval(data);
}

static void
_signal_click(void *data, Evas_Object *obj __UNUSED__, const char *sig __UNUSED__, const char *src __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;
   if (wd->anim) return;
   if (wd->collapsible)
     {
        evas_object_smart_callback_add(wd->frm, "recalc", _recalc, data);
        edje_object_signal_emit(wd->frm, "elm,action,toggle", "elm");
        wd->collapsed++;
        wd->anim = EINA_TRUE;
     }
   evas_object_smart_callback_call(data, SIG_CLICKED, NULL);
}

EAPI Evas_Object *
elm_frame_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas *e;
   Widget_Data *wd;

   ELM_WIDGET_STANDARD_SETUP(wd, Widget_Data, parent, e, obj, NULL);

   ELM_SET_WIDTYPE(widtype, "frame");
   elm_widget_type_set(obj, "frame");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);
   elm_widget_focus_next_hook_set(obj, _elm_frame_focus_next_hook);
   elm_widget_can_focus_set(obj, EINA_FALSE);
   elm_widget_text_set_hook_set(obj, _elm_frame_label_set);
   elm_widget_text_get_hook_set(obj, _elm_frame_label_get);
   elm_widget_content_set_hook_set(obj, _content_set_hook);
   elm_widget_content_get_hook_set(obj, _content_get_hook);
   elm_widget_content_unset_hook_set(obj, _content_unset_hook);

   wd->frm = edje_object_add(e);
   _elm_theme_object_set(obj, wd->frm, "frame", "base", "default");
   elm_widget_resize_object_set(obj, wd->frm);

   evas_object_smart_callback_add(obj, "sub-object-del", _sub_del, obj);
   edje_object_signal_callback_add(wd->frm, "elm,anim,done", "elm",
                                   _recalc_done, obj);
   edje_object_signal_callback_add(wd->frm, "elm,action,click", "elm",
                                   _signal_click, obj);
   evas_object_smart_callbacks_descriptions_set(obj, _signals);

   _mirrored_set(obj, elm_widget_mirrored_get(obj));
   _sizing_eval(obj);
   return obj;
}

EAPI void
elm_frame_autocollapse_set(Evas_Object *obj, Eina_Bool autocollapse)
{
   Widget_Data *wd;
   ELM_CHECK_WIDTYPE(obj, widtype);
   wd = elm_widget_data_get(obj);
   if (!wd) return;
   wd->collapsible = !!autocollapse;
}

EAPI Eina_Bool
elm_frame_autocollapse_get(const Evas_Object *obj)
{
   Widget_Data *wd;
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return wd->collapsible;
}

EAPI void
elm_frame_collapse_set(Evas_Object *obj, Eina_Bool collapse)
{
   Widget_Data *wd;
   ELM_CHECK_WIDTYPE(obj, widtype);
   wd = elm_widget_data_get(obj);
   if (!wd) return;
   collapse = !!collapse;
   if (wd->collapsed == collapse) return;
   edje_object_signal_emit(wd->frm, "elm,action,switch", "elm");
   edje_object_message_signal_process(wd->frm);
   wd->collapsed = !!collapse;
   wd->anim = EINA_FALSE;
   _sizing_eval(obj);
}

EAPI void
elm_frame_collapse_go(Evas_Object *obj, Eina_Bool collapse)
{
   Widget_Data *wd;
   ELM_CHECK_WIDTYPE(obj, widtype);
   wd = elm_widget_data_get(obj);
   if (!wd) return;
   collapse = !!collapse;
   if (wd->collapsed == collapse) return;
   edje_object_signal_emit(wd->frm, "elm,action,toggle", "elm");
   evas_object_smart_callback_add(wd->frm, "recalc", _recalc, obj);
   wd->collapsed = collapse;
   wd->anim = EINA_TRUE;
}

EAPI Eina_Bool
elm_frame_collapse_get(const Evas_Object *obj)
{
   Widget_Data *wd;
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return wd->collapsed;
}
