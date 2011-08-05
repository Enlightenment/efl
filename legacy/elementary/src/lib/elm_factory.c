#include <Elementary.h>
#include "elm_priv.h"

typedef struct _Widget_Data Widget_Data;

struct _Widget_Data
{
   Evas_Object *obj;
   Evas_Object *content;
   Ecore_Job *eval_job;
};

static const char *widtype = NULL;
static void _del_hook(Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);
static void _changed_size_hints(void *data, Evas *e, Evas_Object *obj, void *event_info);

static const char SIG_REALIZE[] = "realize";
static const char SIG_UNREALIZE[] = "unrealize";

static const Evas_Smart_Cb_Description _signals[] = {
   {SIG_REALIZE, ""},
   {SIG_UNREALIZE, ""},
   {NULL, NULL}
};

static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->eval_job) ecore_job_del(wd->eval_job);
   free(wd);
}

static Eina_Bool
_focus_next_hook(const Evas_Object *obj, Elm_Focus_Direction dir, Evas_Object **next)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Object *cur;
   
   if ((!wd) || (!wd->content)) return EINA_FALSE;
   cur = wd->content;
   return elm_widget_focus_next_get(cur, dir, next);
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord minw = -1, minh = -1, maxw = -1, maxh = -1;
   
   if (!wd) return;
   if (!wd->content) return;
   evas_object_size_hint_min_get(wd->content, &minw, &minh);
   evas_object_size_hint_max_get(wd->content, &maxw, &maxh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, maxw, maxh);
}

static void
_eval_do(void *data)
{
   Evas_Object *obj = data;
   Evas_Coord x, y, w, h, cvx, cvy, cvw, cvh;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   
   wd->eval_job = NULL;
   evas_object_geometry_get(obj, &x, &y, &w, &h);
   evas_output_viewport_get(evas_object_evas_get(obj), 
                            &cvx, &cvy, &cvw, &cvh);
   if (ELM_RECTS_INTERSECT(x, y, w, h, cvx, cvy, cvw, cvh))
     {
        if (!wd->content)
           evas_object_smart_callback_call(obj, SIG_REALIZE, NULL);
     }
   else
     {
        if (wd->content)
           evas_object_smart_callback_call(obj, SIG_UNREALIZE, NULL);
     }
}

static void
_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   
   if (wd->eval_job) ecore_job_del(wd->eval_job);
   wd->eval_job = ecore_job_add(_eval_do, obj);
}

static void
_move(void *data __UNUSED__, Evas *e __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   _eval(obj);
}
               
static void
_resize(void *data __UNUSED__, Evas *e __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{              
   _eval(obj);
}

static void
_child_change(void *data __UNUSED__, Evas *e __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   _eval(obj);
}

static void
_child_del(void *data, Evas *e __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   Evas_Object *fobj = data;
   Widget_Data *wd = elm_widget_data_get(fobj);
   if (!wd) return;
   if (wd->content != obj) return;
   evas_object_event_callback_del_full(wd->content, 
                                       EVAS_CALLBACK_CHANGED_SIZE_HINTS,
                                       _child_change, obj);
   evas_object_event_callback_del_full(wd->content, 
                                       EVAS_CALLBACK_DEL,
                                       _child_del, obj);
   wd->content = NULL;
}

EAPI Evas_Object *
elm_factory_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas *e;
   Widget_Data *wd;

   ELM_WIDGET_STANDARD_SETUP(wd, Widget_Data, parent, e, obj, NULL);

   ELM_SET_WIDTYPE(widtype, "factory");
   elm_widget_type_set(obj, "factory");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_focus_next_hook_set(obj, _focus_next_hook);
   elm_widget_can_focus_set(obj, EINA_FALSE);

   evas_object_event_callback_add(obj, EVAS_CALLBACK_MOVE, _move, NULL);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_RESIZE, _resize, NULL);

   evas_object_smart_callbacks_descriptions_set(obj, _signals);
   
   wd->obj = obj;
   return obj;
}

EAPI void
elm_factory_content_set(Evas_Object *obj, Evas_Object *content)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->content == content) return;
   if (wd->content) evas_object_del(wd->content);
   wd->content = content;
   elm_widget_resize_object_set(obj, wd->content);
   evas_object_event_callback_add(wd->content, EVAS_CALLBACK_DEL,
                                  _child_del, obj);
   evas_object_event_callback_add(wd->content, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
                                  _child_change, obj);
    _sizing_eval(obj);
}

EAPI Evas_Object *
elm_factory_content_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return wd->content;
}
