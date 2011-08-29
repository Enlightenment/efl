#include <Elementary.h>
#include "elm_priv.h"

// FIXME: handle if canvas resizes

typedef struct _Widget_Data Widget_Data;

struct _Widget_Data
{
   Evas_Object *obj;
   Evas_Object *content;
   int last_calc_count; 
   Evas_Coord maxminw, maxminh;
   Eina_Bool eval : 1;
   Eina_Bool szeval : 1;
   Eina_Bool maxmin : 1;
};

static const char *widtype = NULL;
static void _del_hook(Evas_Object *obj);
static Eina_Bool _focus_next_hook(const Evas_Object *obj, Elm_Focus_Direction dir, Evas_Object **next);
static void _sizing_eval(Evas_Object *obj);
static void _eval(Evas_Object *obj);
static void _changed(Evas_Object *obj);
static void _move(void *data __UNUSED__, Evas *e __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__);
static void _resize(void *data __UNUSED__, Evas *e __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__);
static void _child_change(void *data __UNUSED__, Evas *e __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__);
static void _child_del(void *data, Evas *e __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__);

static const char SIG_REALIZE[] = "realize";
static const char SIG_UNREALIZE[] = "unrealize";

static const Evas_Smart_Cb_Description _signals[] = {
   {SIG_REALIZE, ""},
   {SIG_UNREALIZE, ""},
   {NULL, NULL}
};

static int fac = 0;

static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->content)
     {
        Evas_Object *o = wd->content;
        
        evas_object_event_callback_del_full(o,
                                            EVAS_CALLBACK_CHANGED_SIZE_HINTS,
                                            _child_change, obj);
        evas_object_event_callback_del_full(o,
                                            EVAS_CALLBACK_DEL,
                                            _child_del, obj);
        wd->content = NULL;
        evas_object_del(o);
        fac--;
//        printf("FAC-- = %i\n", fac);
     }
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
   if (wd->maxmin)
     {
        if (minw > wd->maxminw) wd->maxminw = minw;
        if (minh > wd->maxminh) wd->maxminh = minh;
        evas_object_size_hint_min_set(obj, wd->maxminw, wd->maxminh);
     }
   else
     {
        evas_object_size_hint_min_set(obj, minw, minh);
     }
   evas_object_size_hint_max_set(obj, maxw, maxh);
//   printf("FAC SZ: %i %i | %i %i\n", minw, minh, maxw, maxh);
}

static void
_eval(Evas_Object *obj)
{
   Evas_Coord x, y, w, h, cvx, cvy, cvw, cvh;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;

   evas_event_freeze(evas_object_evas_get(obj));
   evas_object_geometry_get(obj, &x, &y, &w, &h);
   if (w < 1) w = 1;
   if (h < 1) h = 1;
   evas_output_viewport_get(evas_object_evas_get(obj),
                            &cvx, &cvy, &cvw, &cvh);
   if ((cvw < 1) || (cvh < 1)) return;
   // need some fuzz value thats beyond the current viewport
   // for now just make it the viewport * 3 in size (so 1 vp in each direction)
   /*
   cvx -= cvw;
   cvy -= cvh;
   cvw *= 3;
   cvh *= 3;
    */
   if (ELM_RECTS_INTERSECT(x, y, w, h, cvx, cvy, cvw, cvh))
     {
        if (!wd->content)
          {
//             printf("                 + %i %i %ix%i <> %i %i %ix%i\n", x, y, w, h, cvx, cvy, cvw, cvh);
             evas_object_smart_callback_call(obj, SIG_REALIZE, NULL);
             if (wd->content)
               {
                  if (evas_object_smart_data_get(wd->content))
                     evas_object_smart_calculate(wd->content);
               }
             wd->last_calc_count = 
                evas_smart_objects_calculate_count_get(evas_object_evas_get(obj));
          }
     }
   else
     {
        if (wd->content)
          {
             if (wd->last_calc_count != 
                evas_smart_objects_calculate_count_get(evas_object_evas_get(obj)))
                evas_object_smart_callback_call(obj, SIG_UNREALIZE, NULL);
          }
     }
   evas_event_thaw(evas_object_evas_get(obj));
   evas_event_thaw_eval(evas_object_evas_get(obj));
}

static void
_changed(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->eval)
     {
        _eval(obj);
        wd->eval = EINA_FALSE;
     }
   if (wd->szeval)
     {
        _sizing_eval(obj);
        wd->szeval = EINA_FALSE;
     }
}

static void
_move(void *data __UNUSED__, Evas *e __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   wd->eval = EINA_TRUE;
   evas_object_smart_changed(obj);
}

static void
_resize(void *data __UNUSED__, Evas *e __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   wd->eval = EINA_TRUE;
   evas_object_smart_changed(obj);
}

static void
_child_change(void *data __UNUSED__, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;
   wd->eval = EINA_TRUE;
   wd->szeval = EINA_TRUE;
   evas_object_smart_changed(data);
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
   fac--;
//   printf("FAC-- = %i\n", fac);
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
   elm_widget_changed_hook_set(obj, _changed);

   evas_object_event_callback_add(obj, EVAS_CALLBACK_MOVE, _move, NULL);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_RESIZE, _resize, NULL);

   evas_object_smart_callbacks_descriptions_set(obj, _signals);

   wd->obj = obj;
   wd->last_calc_count = -1;
   return obj;
}

EAPI void
elm_factory_content_set(Evas_Object *obj, Evas_Object *content)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->content == content) return;
   if (wd->content)
      {
         Evas_Object *o = wd->content;

         evas_object_event_callback_del_full(wd->content,
                                             EVAS_CALLBACK_CHANGED_SIZE_HINTS,
                                             _child_change, obj);
         evas_object_event_callback_del_full(wd->content,
                                             EVAS_CALLBACK_DEL,
                                             _child_del, obj);
         wd->content = NULL;
         evas_object_del(o);
         fac--;
//         printf("FAC-- = %i\n", fac);
      }
   wd->content = content;
   if (wd->content)
     {
        fac++;
//        printf("FAC++ = %i\n", fac);
        elm_widget_resize_object_set(obj, wd->content);
        evas_object_event_callback_add(wd->content, EVAS_CALLBACK_DEL,
                                       _child_del, obj);
        evas_object_event_callback_add(wd->content, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
                                       _child_change, obj);
        wd->eval = EINA_TRUE;
        wd->szeval = EINA_TRUE;
        evas_object_smart_changed(obj);
     }
}

EAPI Evas_Object *
elm_factory_content_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return wd->content;
}

EAPI void
elm_factory_maxmin_mode_set(Evas_Object *obj, Eina_Bool enabled)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   wd->maxmin = !!enabled;
}

EAPI Eina_Bool
elm_factory_maxmin_mode_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return wd->maxmin;
}

EAPI void
elm_factory_maxmin_reset_set(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   wd->maxminw = 0;
   wd->maxminh = 0;
   wd->eval = EINA_TRUE;
   wd->szeval = EINA_TRUE;
   evas_object_smart_changed(obj);
}
