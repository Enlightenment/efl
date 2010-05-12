#include <Elementary.h>
#include "elm_priv.h"

/**
 * @defgroup Label Label
 *
 * Display text, with simple html-like markup. The theme of course
 * can invent new markup tags and style them any way it likes
 */

typedef struct _Widget_Data Widget_Data;

struct _Widget_Data
{
   Evas_Object *lbl;
   const char *label;
   Evas_Coord lastw;
   Ecore_Job *deferred_recalc_job;
   Evas_Coord wrap_w;
   Eina_Bool linewrap : 1;
   Eina_Bool changed : 1;
};

static const char *widtype = NULL;
static void _del_hook(Evas_Object *obj);
static void _theme_hook(Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);

static void
_elm_win_recalc_job(void *data)
{
   Widget_Data *wd = elm_widget_data_get(data);
   Evas_Coord minw = -1, minh = -1, maxh = -1;
   Evas_Coord resw, resh, minminw;
   if (!wd) return;
   wd->deferred_recalc_job = NULL;
   evas_object_geometry_get(wd->lbl, NULL, NULL, &resw, &resh);
   resh = 0;
   minminw = 0;
   edje_object_size_min_restricted_calc(wd->lbl, &minw, &minh, 0, 0);
   minminw = minw;
   if (wd->wrap_w >= resw) 
     {
        resw = wd->wrap_w;
        edje_object_size_min_restricted_calc(wd->lbl, &minw, &minh, resw, 0);
        evas_object_size_hint_min_set(data, minw, minh);
     }
   else
     {
        if (wd->wrap_w > minminw) minminw = wd->wrap_w;
        edje_object_size_min_restricted_calc(wd->lbl, &minw, &minh, resw, 0);
        evas_object_size_hint_min_set(data, minminw, minh);
     }
   maxh = minh;
   evas_object_size_hint_max_set(data, -1, maxh);
}

static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->deferred_recalc_job) ecore_job_del(wd->deferred_recalc_job);
   if (wd->label) eina_stringshare_del(wd->label);
   free(wd);
}

static void
_theme_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->linewrap)
     _elm_theme_object_set(obj, wd->lbl, "label", "base_wrap", elm_widget_style_get(obj));
   else
     _elm_theme_object_set(obj, wd->lbl, "label", "base", elm_widget_style_get(obj));
   edje_object_part_text_set(wd->lbl, "elm.text", wd->label);
   edje_object_scale_set(wd->lbl, elm_widget_scale_get(obj) * 
                         _elm_config->scale);
   _sizing_eval(obj);
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord minw = -1, minh = -1, maxw = -1, maxh = -1;
   Evas_Coord resw, resh;
   if (!wd) return;
   if (wd->linewrap)
     {
        evas_object_geometry_get(wd->lbl, NULL, NULL, &resw, &resh);
        if ((resw == wd->lastw) && (!wd->changed)) return;
        wd->changed = EINA_FALSE;
        wd->lastw = resw;
        _elm_win_recalc_job(obj);
// FIXME: works ok. but NOT for genlist. what should genlist do?        
//        if (wd->deferred_recalc_job) ecore_job_del(wd->deferred_recalc_job);
//        wd->deferred_recalc_job = ecore_job_add(_elm_win_recalc_job, obj);
     }
   else
     {
        evas_object_geometry_get(wd->lbl, NULL, NULL, &resw, &resh);
        edje_object_size_min_calc(wd->lbl, &minw, &minh);
        evas_object_size_hint_min_set(obj, minw, minh);
        maxh = minh;
        evas_object_size_hint_max_set(obj, maxw, maxh);
     }
}

static void 
_resize(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;
   if (wd->linewrap) _sizing_eval(data);
}
/**
 * Add a new label to the parent
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * @ingroup Label
 */
EAPI Evas_Object *
elm_label_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas *e;
   Widget_Data *wd;

   wd = ELM_NEW(Widget_Data);
   e = evas_object_evas_get(parent);
   obj = elm_widget_add(e);
   ELM_SET_WIDTYPE(widtype, "label");
   elm_widget_type_set(obj, "label");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);
   elm_widget_can_focus_set(obj, 0);

   wd->linewrap = EINA_FALSE;

   wd->lbl = edje_object_add(e);
   _elm_theme_object_set(obj, wd->lbl, "label", "base", "default");
   wd->label = eina_stringshare_add("<br>");
   edje_object_part_text_set(wd->lbl, "elm.text", "<br>");
   elm_widget_resize_object_set(obj, wd->lbl);
   
   evas_object_event_callback_add(wd->lbl, EVAS_CALLBACK_RESIZE, _resize, obj);
   
   wd->changed = 1;
   _sizing_eval(obj);
   return obj;
}

/**
 * Set the label on the label object
 *
 * @param obj The label object
 * @param label The label will be used on the label object
 *
 * @ingroup Label
 */
EAPI void
elm_label_label_set(Evas_Object *obj, const char *label)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (!label) label = "";
   eina_stringshare_replace(&wd->label, label);
   edje_object_part_text_set(wd->lbl, "elm.text", label);
   wd->changed = 1;
   _sizing_eval(obj);
}

/**
 * Get the label used on the label object
 *
 * @param obj The label object
 * @return The string inside the label
 * @ingroup Label
 */
EAPI const char *
elm_label_label_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return wd->label;
}

/**
 * Set the wrapping behavior of the label
 *
 * @param obj The label object
 * @param wrap To wrap text or not
 * @ingroup Label
 */
EAPI void
elm_label_line_wrap_set(Evas_Object *obj, Eina_Bool wrap)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   const char *t;
   if (wd->linewrap == wrap) return;
   wd->linewrap = wrap;
   t = eina_stringshare_add(elm_label_label_get(obj));
   if (wd->linewrap)
     _elm_theme_object_set(obj, wd->lbl, "label", "base_wrap", elm_widget_style_get(obj));
   else
     _elm_theme_object_set(obj, wd->lbl, "label", "base", elm_widget_style_get(obj));
   elm_label_label_set(obj, t);
   eina_stringshare_del(t);
   wd->changed = 1;
   _sizing_eval(obj);
}

/**
 * Get the wrapping behavior of the label
 *
 * @param obj The label object
 * @return To wrap text or not
 * @ingroup Label
 */
EAPI Eina_Bool
elm_label_line_wrap_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return wd->linewrap;
}

/**
 * Set wrap width of the label
 *
 * @param obj The label object
 * @param w The wrap width in pixels at a minimum where words need to wrap
 * @ingroup Label
 */
EAPI void
elm_label_wrap_width_set(Evas_Object *obj, Evas_Coord w)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->wrap_w == w) return;
   wd->wrap_w = w;
   _sizing_eval(obj);
}

/**
 * get wrap width of the label
 *
 * @param obj The label object
 * @return The wrap width in pixels at a minimum where words need to wrap
 * @ingroup Label
 */
EAPI Evas_Coord
elm_label_wrap_width_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) 0;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return 0;
   return wd->wrap_w;
}
