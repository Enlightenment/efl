#include <Elementary.h>
#include "elm_priv.h"

#define SIG_CHILD_ADDED "child,added"
#define SIG_CHILD_REMOVED "child,removed"

typedef struct _Widget_Data Widget_Data;
typedef struct _Transition_Animation_Data Transition_Animation_Data;

struct _Widget_Data
{
   Evas_Object *box;
   Eina_Bool horizontal:1;
   Eina_Bool homogeneous:1;
};

struct _Elm_Box_Transition
{
   double initial_time;
   double duration;
   Eina_Bool animation_ended:1;
   Eina_Bool recalculate:1;
   Ecore_Animator *animator;

   struct
     {
        Evas_Object_Box_Layout layout;
        void *data;
        void(*free_data)(void *data);
     } start, end;

   void(*transition_end_cb)(void *data);
   void *transition_end_data;
   void (*transition_end_free_data)(void *data);
   Eina_List *objs;
   Evas_Object *box;
};

struct _Transition_Animation_Data
{
   Evas_Object *obj;
   struct
     {
        Evas_Coord x, y, w, h;
     } start, end;
};

static const char *widtype = NULL;
static void _del_hook(Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);
static void _changed_size_hints(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _sub_del(void *data, Evas_Object *obj, void *event_info);

static void
_del_pre_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   evas_object_event_callback_del_full
      (wd->box, EVAS_CALLBACK_CHANGED_SIZE_HINTS, _changed_size_hints, obj);
   evas_object_box_remove_all(wd->box, EINA_FALSE);
}

static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   free(wd);
}

static void *
_elm_box_list_data_get(const Eina_List *list)
{
   Evas_Object_Box_Option *opt = eina_list_data_get(list);
   return opt->obj;
}

static Eina_Bool
_elm_box_focus_next_hook(const Evas_Object *obj, Elm_Focus_Direction dir, Evas_Object **next)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   const Eina_List *items;
   void *(*list_data_get) (const Eina_List *list);

   if ((!wd) || (!wd->box))
     return EINA_FALSE;

   /* Focus chain */
   /* TODO: Change this to use other chain */
   if ((items = elm_widget_focus_custom_chain_get(obj)))
     list_data_get = eina_list_data_get;
   else
     {
        Evas_Object_Box_Data *bd = evas_object_smart_data_get(wd->box);
        items = bd->children;
        list_data_get = _elm_box_list_data_get;

        if (!items) return EINA_FALSE;
     }

   return elm_widget_focus_list_next_get(obj, items, list_data_get, dir, next);
}

static void
_theme_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   _elm_widget_mirrored_reload(obj);
   evas_object_smart_calculate(wd->box);
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord minw = -1, minh = -1, maxw = -1, maxh = -1;
   Evas_Coord w, h;
   if (!wd) return;
   evas_object_size_hint_min_get(wd->box, &minw, &minh);
   evas_object_size_hint_max_get(wd->box, &maxw, &maxh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, maxw, maxh);
   evas_object_geometry_get(obj, NULL, NULL, &w, &h);
   if (w < minw) w = minw;
   if (h < minh) h = minh;
   if ((maxw >= 0) && (w > maxw)) w = maxw;
   if ((maxh >= 0) && (h > maxh)) h = maxh;
   evas_object_resize(obj, w, h);
}

static void
_changed_size_hints(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   _sizing_eval(data);
}

static void
_sub_del(void *data __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   _sizing_eval(obj);
}

static void
_layout(Evas_Object *o, Evas_Object_Box_Data *priv, void *data)
{
   Evas_Object *obj = (Evas_Object *) data;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   _els_box_layout(o, priv, wd->horizontal, wd->homogeneous,
                   elm_widget_mirrored_get(obj));
}

static Eina_Bool
_transition_animation(void *data)
{
   evas_object_smart_changed(data);
   return ECORE_CALLBACK_RENEW;
}

static void
_transition_layout_child_added(void *data, Evas_Object *obj __UNUSED__, void *event_info)
{
   Transition_Animation_Data *tad;
   Evas_Object_Box_Option *opt = event_info;
   Elm_Box_Transition *layout_data = data;

   tad = calloc(1, sizeof(Transition_Animation_Data));
   if (!tad) return;
   tad->obj = opt->obj;
   layout_data->objs = eina_list_append(layout_data->objs, tad);
   layout_data->recalculate = EINA_TRUE;
}

static void
_transition_layout_child_removed(void *data, Evas_Object *obj __UNUSED__, void *event_info)
{
   Eina_List *l;
   Transition_Animation_Data *tad;
   Elm_Box_Transition *layout_data = data;

   EINA_LIST_FOREACH(layout_data->objs, l, tad)
     {
        if (tad->obj == event_info)
          {
             free(eina_list_data_get(l));
             layout_data->objs = eina_list_remove_list(layout_data->objs, l);
             layout_data->recalculate = EINA_TRUE;
             break;
          }
     }
}

static void
_transition_layout_obj_resize_cb(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Elm_Box_Transition *layout_data = data;
   layout_data->recalculate = EINA_TRUE;
}

static void
_transition_layout_calculate_coords(Evas_Object *obj, Evas_Object_Box_Data *priv,
                                    Elm_Box_Transition *layout_data)
{
   Eina_List *l;
   Transition_Animation_Data *tad;
   Evas_Coord x, y, w, h;
   const double curtime = ecore_loop_time_get();

   layout_data->duration =
      layout_data->duration - (curtime - layout_data->initial_time);
   layout_data->initial_time = curtime;

   evas_object_geometry_get(obj, &x, &y, &w, &h);
   EINA_LIST_FOREACH(layout_data->objs, l, tad)
     {
        evas_object_geometry_get(tad->obj, &tad->start.x, &tad->start.y,
                                 &tad->start.w, &tad->start.h);
        tad->start.x = tad->start.x - x;
        tad->start.y = tad->start.y - y;
     }
   layout_data->end.layout(obj, priv, layout_data->end.data);
   EINA_LIST_FOREACH(layout_data->objs, l, tad)
     {
        evas_object_geometry_get(tad->obj, &tad->end.x, &tad->end.y,
                                 &tad->end.w, &tad->end.h);
        tad->end.x = tad->end.x - x;
        tad->end.y = tad->end.y - y;
     }
}

static Eina_Bool
_transition_layout_load_children_list(Evas_Object_Box_Data *priv,
                                      Elm_Box_Transition *layout_data)
{
   Eina_List *l;
   Evas_Object_Box_Option *opt;
   Transition_Animation_Data *tad;

   EINA_LIST_FREE(layout_data->objs, tad)
      free(tad);

   EINA_LIST_FOREACH(priv->children, l, opt)
     {
        tad = calloc(1, sizeof(Transition_Animation_Data));
        if (!tad)
          {
             EINA_LIST_FREE(layout_data->objs, tad)
                free(tad);
             layout_data->objs = NULL;
             return EINA_FALSE;
          }
        tad->obj = opt->obj;
        layout_data->objs = eina_list_append(layout_data->objs, tad);
     }
   return EINA_TRUE;
}

static Eina_Bool
_transition_layout_animation_start(Evas_Object *obj, Evas_Object_Box_Data *priv,
                                   Elm_Box_Transition *layout_data, Eina_Bool(*transition_animation_cb)(void *data))
{
   layout_data->start.layout(obj, priv, layout_data->start.data);
   layout_data->box = obj;
   layout_data->initial_time = ecore_loop_time_get();

   if (!_transition_layout_load_children_list(priv, layout_data))
     return EINA_FALSE;
   _transition_layout_calculate_coords(obj, priv, layout_data);

   evas_object_event_callback_add(obj, EVAS_CALLBACK_RESIZE,
                                  _transition_layout_obj_resize_cb, layout_data);
   evas_object_smart_callback_add(obj, SIG_CHILD_ADDED,
                                  _transition_layout_child_added, layout_data);
   evas_object_smart_callback_add(obj, SIG_CHILD_REMOVED,
                                  _transition_layout_child_removed, layout_data);
   if (!layout_data->animator)
     layout_data->animator = ecore_animator_add(transition_animation_cb, obj);
   layout_data->animation_ended = EINA_FALSE;
   return EINA_TRUE;
}

static void
_transition_layout_animation_stop(Elm_Box_Transition *layout_data)
{
   layout_data->animation_ended = EINA_TRUE;
   if (layout_data->animator)
     {
        ecore_animator_del(layout_data->animator);
        layout_data->animator = NULL;
     }

   if (layout_data->transition_end_cb)
     layout_data->transition_end_cb(layout_data->transition_end_data);
}

static void
_transition_layout_animation_exec(Evas_Object *obj, Evas_Object_Box_Data *priv __UNUSED__,
                                  Elm_Box_Transition *layout_data, const double curtime)
{
   Eina_List *l;
   Transition_Animation_Data *tad;
   Evas_Coord x, y, w, h;
   Evas_Coord cur_x, cur_y, cur_w, cur_h;
   double progress = 0.0;

   progress = (curtime - layout_data->initial_time) / layout_data->duration;
   evas_object_geometry_get(obj, &x, &y, &w, &h);

   EINA_LIST_FOREACH(layout_data->objs, l, tad)
     {
        cur_x = x + tad->start.x + ((tad->end.x - tad->start.x) * progress);
        cur_y = y + tad->start.y + ((tad->end.y - tad->start.y) * progress);
        cur_w = tad->start.w + ((tad->end.w - tad->start.w) * progress);
        cur_h = tad->start.h + ((tad->end.h - tad->start.h) * progress);
        evas_object_move(tad->obj, cur_x, cur_y);
        evas_object_resize(tad->obj, cur_w, cur_h);
     }
}

EAPI Evas_Object *
elm_box_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas *e;
   Widget_Data *wd;

   ELM_WIDGET_STANDARD_SETUP(wd, Widget_Data, parent, e, obj, NULL);

   ELM_SET_WIDTYPE(widtype, "box");
   elm_widget_type_set(obj, "box");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_del_pre_hook_set(obj, _del_pre_hook);
   elm_widget_focus_next_hook_set(obj, _elm_box_focus_next_hook);
   elm_widget_can_focus_set(obj, EINA_FALSE);
   elm_widget_highlight_ignore_set(obj, EINA_TRUE);
   elm_widget_theme_hook_set(obj, _theme_hook);

   wd->box = evas_object_box_add(e);
   /*evas_object_box_layout_set(wd->box, evas_object_box_layout_vertical,
     NULL, NULL);*/
   evas_object_box_layout_set(wd->box, _layout, obj, NULL);

   evas_object_event_callback_add(wd->box, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
                                  _changed_size_hints, obj);
   elm_widget_resize_object_set(obj, wd->box);

   evas_object_smart_callback_add(obj, "sub-object-del", _sub_del, obj);

   return obj;
}

EAPI void
elm_box_horizontal_set(Evas_Object *obj, Eina_Bool horizontal)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   wd->horizontal = !!horizontal;
   evas_object_smart_calculate(wd->box);
 /*if (wd->horizontal)
     {
        if (wd->homogeneous)
          evas_object_box_layout_set(wd->box,
                                     evas_object_box_layout_homogeneous_horizontal, NULL, NULL);
        else
          evas_object_box_layout_set(wd->box, evas_object_box_layout_horizontal,
                                     NULL, NULL);
     }
   else
     {
        if (wd->homogeneous)
          evas_object_box_layout_set(wd->box,
                                     evas_object_box_layout_homogeneous_vertical, NULL, NULL);
        else
          evas_object_box_layout_set(wd->box, evas_object_box_layout_horizontal,
                                     NULL, NULL);
     } */
}

EAPI Eina_Bool
elm_box_horizontal_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return wd->horizontal;
}

EAPI void
elm_box_homogeneous_set(Evas_Object *obj, Eina_Bool homogeneous)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   wd->homogeneous = !!homogeneous;
   evas_object_smart_calculate(wd->box);
 /*if (wd->horizontal)
     {
        if (wd->homogeneous)
          evas_object_box_layout_set(wd->box,
                                     evas_object_box_layout_homogeneous_horizontal, NULL, NULL);
        else
          evas_object_box_layout_set(wd->box, evas_object_box_layout_horizontal,
                                     NULL, NULL);
     }
   else
     {
        if (wd->homogeneous)
          evas_object_box_layout_set(wd->box,
                                     evas_object_box_layout_homogeneous_vertical, NULL, NULL);
        else
          evas_object_box_layout_set(wd->box, evas_object_box_layout_horizontal,
                                     NULL, NULL);
     } */
}

EINA_DEPRECATED EAPI void
elm_box_homogenous_set(Evas_Object *obj, Eina_Bool homogenous)
{
   elm_box_homogeneous_set(obj, homogenous);
}

EAPI Eina_Bool
elm_box_homogeneous_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return wd->homogeneous;
}

EINA_DEPRECATED EAPI Eina_Bool
elm_box_homogenous_get(const Evas_Object *obj)
{
   return elm_box_homogeneous_get(obj);
}

EAPI void
elm_box_pack_start(Evas_Object *obj, Evas_Object *subobj)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_widget_sub_object_add(obj, subobj);
   evas_object_box_prepend(wd->box, subobj);
}

EAPI void
elm_box_pack_end(Evas_Object *obj, Evas_Object *subobj)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_widget_sub_object_add(obj, subobj);
   evas_object_box_append(wd->box, subobj);
}

EAPI void
elm_box_pack_before(Evas_Object *obj, Evas_Object *subobj, Evas_Object *before)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_widget_sub_object_add(obj, subobj);
   evas_object_box_insert_before(wd->box, subobj, before);
}

EAPI void
elm_box_pack_after(Evas_Object *obj, Evas_Object *subobj, Evas_Object *after)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_widget_sub_object_add(obj, subobj);
   evas_object_box_insert_after(wd->box, subobj, after);
}

EAPI void
elm_box_clear(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   evas_object_box_remove_all(wd->box, EINA_TRUE);
}

EAPI void
elm_box_unpack(Evas_Object *obj, Evas_Object *subobj)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   evas_object_box_remove(wd->box, subobj);
}

EAPI void
elm_box_unpack_all(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   evas_object_box_remove_all(wd->box, EINA_FALSE);
}

EAPI void
elm_box_layout_set(Evas_Object *obj, Evas_Object_Box_Layout cb, const void *data, void (*free_data)(void *data))
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;

   if (cb)
     evas_object_box_layout_set(wd->box, cb, data, free_data);
   else
     evas_object_box_layout_set(wd->box, _layout, obj, NULL);
}

EAPI void
elm_box_layout_transition(Evas_Object *obj, Evas_Object_Box_Data *priv, void *data)
{
   Elm_Box_Transition *box_data = data;
   const double curtime = ecore_loop_time_get();

   if (box_data->animation_ended)
     {
        box_data->end.layout(obj, priv, box_data->end.data);
        return;
     }

   if (!box_data->animator)
     {
        if (!_transition_layout_animation_start(obj, priv, box_data,
                                                _transition_animation))
          return;
     }
   else
     {
        if (box_data->recalculate)
          {
             _transition_layout_calculate_coords(obj, priv, box_data);
             box_data->recalculate = EINA_FALSE;
          }
     }

   if ((curtime >= box_data->duration + box_data->initial_time))
     _transition_layout_animation_stop(box_data);
   else
     _transition_layout_animation_exec(obj, priv, box_data, curtime);
}

EAPI Elm_Box_Transition *
elm_box_transition_new(const double duration,
                       Evas_Object_Box_Layout start_layout, void *start_layout_data,
                       void(*start_layout_free_data)(void *data),
                       Evas_Object_Box_Layout end_layout, void *end_layout_data,
                       void(*end_layout_free_data)(void *data),
                       void(*transition_end_cb)(void *data),
                       void *transition_end_data)
{
   Elm_Box_Transition *box_data;

   EINA_SAFETY_ON_NULL_RETURN_VAL(start_layout, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(end_layout, NULL);

   box_data = calloc(1, sizeof(Elm_Box_Transition));
   if (!box_data)
     return NULL;

   box_data->start.layout = start_layout;
   box_data->start.data = start_layout_data;
   box_data->start.free_data = start_layout_free_data;
   box_data->end.layout = end_layout;
   box_data->end.data = end_layout_data;
   box_data->end.free_data = end_layout_free_data;
   box_data->duration = duration;
   box_data->transition_end_cb = transition_end_cb;
   box_data->transition_end_data = transition_end_data;
   return box_data;
}

EAPI void
elm_box_transition_free(void *data)
{
   EINA_SAFETY_ON_NULL_RETURN(data);

   Transition_Animation_Data *tad;
   Elm_Box_Transition *box_data = data;
   if ((box_data->start.free_data) && (box_data->start.data))
     box_data->start.free_data(box_data->start.data);
   if ((box_data->end.free_data) && (box_data->end.data))
     box_data->end.free_data(box_data->end.data);
   EINA_LIST_FREE(box_data->objs, tad)
      free(tad);
   evas_object_event_callback_del(box_data->box, EVAS_CALLBACK_RESIZE, _transition_layout_obj_resize_cb);
   evas_object_smart_callback_del(box_data->box, SIG_CHILD_ADDED, _transition_layout_child_added);
   evas_object_smart_callback_del(box_data->box, SIG_CHILD_REMOVED, _transition_layout_child_removed);
   if (box_data->animator)
     {
        ecore_animator_del(box_data->animator);
        box_data->animator = NULL;
     }
   free(data);
}

EAPI const Eina_List *
elm_box_children_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return evas_object_box_children_get(wd->box);
}

EAPI void
elm_box_padding_set(Evas_Object *obj, Evas_Coord horizontal, Evas_Coord vertical)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   evas_object_box_padding_set(wd->box, horizontal, vertical);
}

EAPI void
elm_box_padding_get(const Evas_Object *obj, Evas_Coord *horizontal, Evas_Coord *vertical)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   evas_object_box_padding_get(wd->box, horizontal, vertical);
}

EAPI void
elm_box_align_set(Evas_Object *obj, double horizontal, double vertical)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   evas_object_size_hint_align_set(wd->box, horizontal, vertical);
}

EAPI void
elm_box_align_get(const Evas_Object *obj, double *horizontal, double *vertical)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   evas_object_size_hint_align_get(wd->box, horizontal, vertical);
}
