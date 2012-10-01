#include <Elementary.h>
#include "elm_priv.h"
#include "els_box.h"
#include "elm_widget_box.h"

EAPI const char ELM_BOX_SMART_NAME[] = "elm_box";

EVAS_SMART_SUBCLASS_NEW
  (ELM_BOX_SMART_NAME, _elm_box, Elm_Box_Smart_Class,
  Elm_Widget_Smart_Class, elm_widget_smart_class_get, NULL);

static const char SIG_CHILD_ADDED[] = "child,added";
static const char SIG_CHILD_REMOVED[] = "child,removed";
static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_CHILD_ADDED, ""},
   {SIG_CHILD_REMOVED, ""},
   {NULL, NULL}
};

static void *
_elm_box_list_data_get(const Eina_List *list)
{
   Evas_Object_Box_Option *opt = eina_list_data_get(list);
   return opt->obj;
}

static void
_child_added_cb_proxy(void *data,
                      Evas_Object *o __UNUSED__,
                      void *event_info)
{
   Evas_Object *box = data;
   Evas_Object_Box_Option *opt = event_info;
   evas_object_smart_callback_call(box, SIG_CHILD_ADDED, opt->obj);
}

static void
_child_removed_cb_proxy(void *data,
                        Evas_Object *o __UNUSED__,
                        void *event_info)
{
   Evas_Object *box = data;
   Evas_Object *child = event_info;
   evas_object_smart_callback_call(box, SIG_CHILD_REMOVED, child);
}

static Eina_Bool
_elm_box_smart_focus_next(const Evas_Object *obj,
                          Elm_Focus_Direction dir,
                          Evas_Object **next)
{
   const Eina_List *items;
   void *(*list_data_get)(const Eina_List *list);

   ELM_BOX_DATA_GET(obj, sd);

   /* Focus chain */
   /* TODO: Change this to use other chain */
   if ((items = elm_widget_focus_custom_chain_get(obj)))
     list_data_get = eina_list_data_get;
   else
     {
        Evas_Object_Box_Data *bd =
          evas_object_smart_data_get(ELM_WIDGET_DATA(sd)->resize_obj);

        items = bd->children;
        list_data_get = _elm_box_list_data_get;

        if (!items) return EINA_FALSE;
     }

   return elm_widget_focus_list_next_get(obj, items, list_data_get, dir, next);
}

static Eina_Bool
_elm_box_smart_focus_direction(const Evas_Object *obj,
                               const Evas_Object *base,
                               double degree,
                               Evas_Object **direction,
                               double *weight)
{
   const Eina_List *items;
   void *(*list_data_get)(const Eina_List *list);

   ELM_BOX_DATA_GET(obj, sd);

   if ((items = elm_widget_focus_custom_chain_get(obj)))
     list_data_get = eina_list_data_get;
   else
     {
        Evas_Object_Box_Data *bd =
          evas_object_smart_data_get(ELM_WIDGET_DATA(sd)->resize_obj);

        items = bd->children;
        list_data_get = _elm_box_list_data_get;

        if (!items) return EINA_FALSE;
     }
   return elm_widget_focus_list_direction_get
            (obj, base, items, list_data_get, degree, direction, weight);
}

static Eina_Bool
_elm_box_smart_theme(Evas_Object *obj)
{
   ELM_BOX_DATA_GET(obj, sd);

   if (!_elm_box_parent_sc->theme(obj)) return EINA_FALSE;

   evas_object_smart_calculate(ELM_WIDGET_DATA(sd)->resize_obj);

   return EINA_TRUE;
}

static void
_sizing_eval(Evas_Object *obj)
{
   Evas_Coord minw = -1, minh = -1, maxw = -1, maxh = -1;
   Evas_Coord w, h;

   ELM_BOX_DATA_GET(obj, sd);

   if (sd->on_deletion)
     return;

   evas_object_size_hint_min_get
     (ELM_WIDGET_DATA(sd)->resize_obj, &minw, &minh);
   evas_object_size_hint_max_get
     (ELM_WIDGET_DATA(sd)->resize_obj, &maxw, &maxh);
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
_on_size_hints_changed(void *data,
                       Evas *e __UNUSED__,
                       Evas_Object *obj __UNUSED__,
                       void *event_info __UNUSED__)
{
   _sizing_eval(data);
}

static Eina_Bool
_elm_box_smart_sub_object_del(Evas_Object *obj,
                              Evas_Object *child)
{
   if (!_elm_box_parent_sc->sub_object_del(obj, child)) return EINA_FALSE;

   _sizing_eval(obj);

   return EINA_TRUE;
}

static void
_elm_box_custom_layout(Evas_Object *o,
                       Evas_Object_Box_Data *priv,
                       void *data)
{
   ELM_BOX_DATA_GET(data, sd);

   _els_box_layout(o, priv, sd->horizontal, sd->homogeneous,
                   elm_widget_mirrored_get(data));
}

static Eina_Bool
_transition_animation(void *data)
{
   evas_object_smart_changed(data);

   return ECORE_CALLBACK_RENEW;
}

static void
_transition_layout_child_added(void *data,
                               Evas_Object *obj __UNUSED__,
                               void *event_info)
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
_transition_layout_child_removed(void *data,
                                 Evas_Object *obj __UNUSED__,
                                 void *event_info)
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
_transition_layout_obj_resize_cb(void *data,
                                 Evas *e __UNUSED__,
                                 Evas_Object *obj __UNUSED__,
                                 void *event_info __UNUSED__)
{
   Elm_Box_Transition *layout_data = data;

   layout_data->recalculate = EINA_TRUE;
}

static void
_transition_layout_calculate_coords(Evas_Object *obj,
                                    Evas_Object_Box_Data *priv,
                                    Elm_Box_Transition *layout_data)
{
   Eina_List *l;
   Evas_Coord x, y, w, h;
   Transition_Animation_Data *tad;
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

   EINA_LIST_FREE (layout_data->objs, tad)
     free(tad);

   EINA_LIST_FOREACH(priv->children, l, opt)
     {
        tad = calloc(1, sizeof(Transition_Animation_Data));
        if (!tad)
          {
             EINA_LIST_FREE (layout_data->objs, tad)
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
_transition_layout_animation_start(Evas_Object *obj,
                                   Evas_Object_Box_Data *priv,
                                   Elm_Box_Transition *layout_data,
                                   Eina_Bool (*transition_animation_cb)
                                   (void *data))
{
   layout_data->start.layout(obj, priv, layout_data->start.data);
   layout_data->box = obj;
   layout_data->initial_time = ecore_loop_time_get();

   if (!_transition_layout_load_children_list(priv, layout_data))
     return EINA_FALSE;

   _transition_layout_calculate_coords(obj, priv, layout_data);

   evas_object_event_callback_add
     (obj, EVAS_CALLBACK_RESIZE, _transition_layout_obj_resize_cb,
     layout_data);
   evas_object_smart_callback_add
     (obj, SIG_CHILD_ADDED, _transition_layout_child_added, layout_data);
   evas_object_smart_callback_add
     (obj, SIG_CHILD_REMOVED, _transition_layout_child_removed, layout_data);

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
_transition_layout_animation_exec(Evas_Object *obj,
                                  Evas_Object_Box_Data *priv __UNUSED__,
                                  Elm_Box_Transition *layout_data,
                                  const double curtime)
{
   Eina_List *l;
   double progress = 0.0;
   Evas_Coord x, y, w, h;
   Transition_Animation_Data *tad;
   Evas_Coord cur_x, cur_y, cur_w, cur_h;

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

static void
_elm_box_smart_add(Evas_Object *obj)
{
   EVAS_SMART_DATA_ALLOC(obj, Elm_Box_Smart_Data);

   ELM_WIDGET_DATA(priv)->resize_obj =
     evas_object_box_add(evas_object_evas_get(obj));
   evas_object_box_layout_set
     (ELM_WIDGET_DATA(priv)->resize_obj, _elm_box_custom_layout, obj, NULL);

   evas_object_event_callback_add
     (ELM_WIDGET_DATA(priv)->resize_obj, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
     _on_size_hints_changed, obj);

   _elm_box_parent_sc->base.add(obj);

   evas_object_smart_callback_add
     (ELM_WIDGET_DATA(priv)->resize_obj, SIG_CHILD_ADDED,
     _child_added_cb_proxy, obj);
   evas_object_smart_callback_add
     (ELM_WIDGET_DATA(priv)->resize_obj, SIG_CHILD_REMOVED,
     _child_removed_cb_proxy, obj);

   elm_widget_can_focus_set(obj, EINA_FALSE);
   elm_widget_highlight_ignore_set(obj, EINA_TRUE);
}

static void
_elm_box_smart_del(Evas_Object *obj)
{
   Eina_List *l;
   Evas_Object *child;

   ELM_BOX_DATA_GET(obj, sd);

   sd->on_deletion = EINA_TRUE;

   evas_object_event_callback_del_full
     (ELM_WIDGET_DATA(sd)->resize_obj, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
     _on_size_hints_changed, obj);

   /* let's make our box object the *last* to be processed, since it
    * may (smart) parent other sub objects here */
   EINA_LIST_FOREACH(ELM_WIDGET_DATA(sd)->subobjs, l, child)
     {
        if (child == ELM_WIDGET_DATA(sd)->resize_obj)
          {
             ELM_WIDGET_DATA(sd)->subobjs =
               eina_list_demote_list(ELM_WIDGET_DATA(sd)->subobjs, l);
             break;
          }
     }

   _elm_box_parent_sc->base.del(obj);
}

static void
_elm_box_smart_set_user(Elm_Box_Smart_Class *sc)
{
   ELM_WIDGET_CLASS(sc)->base.add = _elm_box_smart_add;
   ELM_WIDGET_CLASS(sc)->base.del = _elm_box_smart_del;

   ELM_WIDGET_CLASS(sc)->sub_object_del = _elm_box_smart_sub_object_del;
   ELM_WIDGET_CLASS(sc)->theme = _elm_box_smart_theme;
   ELM_WIDGET_CLASS(sc)->focus_next = _elm_box_smart_focus_next;
   ELM_WIDGET_CLASS(sc)->focus_direction = _elm_box_smart_focus_direction;
}

EAPI const Elm_Box_Smart_Class *
elm_box_smart_class_get(void)
{
   static Elm_Box_Smart_Class _sc =
     ELM_BOX_SMART_CLASS_INIT_NAME_VERSION(ELM_BOX_SMART_NAME);
   static const Elm_Box_Smart_Class *class = NULL;
   Evas_Smart_Class *esc = (Evas_Smart_Class *)&_sc;

   if (class) return class;

   _elm_box_smart_set(&_sc);
   esc->callbacks = _smart_callbacks;
   class = &_sc;

   return class;
}

EAPI Evas_Object *
elm_box_add(Evas_Object *parent)
{
   Evas_Object *obj;

   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);

   obj = elm_widget_add(_elm_box_smart_class_new(), parent);
   if (!obj) return NULL;

   if (!elm_widget_sub_object_add(parent, obj))
     ERR("could not add %p as sub object of %p", obj, parent);

   return obj;
}

EAPI void
elm_box_horizontal_set(Evas_Object *obj,
                       Eina_Bool horizontal)
{
   ELM_BOX_CHECK(obj);
   ELM_BOX_DATA_GET(obj, sd);

   sd->horizontal = !!horizontal;
   evas_object_smart_calculate(ELM_WIDGET_DATA(sd)->resize_obj);
}

EAPI Eina_Bool
elm_box_horizontal_get(const Evas_Object *obj)
{
   ELM_BOX_CHECK(obj) EINA_FALSE;
   ELM_BOX_DATA_GET(obj, sd);

   return sd->horizontal;
}

EAPI void
elm_box_homogeneous_set(Evas_Object *obj,
                        Eina_Bool homogeneous)
{
   ELM_BOX_CHECK(obj);
   ELM_BOX_DATA_GET(obj, sd);

   sd->homogeneous = !!homogeneous;
   evas_object_smart_calculate(ELM_WIDGET_DATA(sd)->resize_obj);
}

EAPI Eina_Bool
elm_box_homogeneous_get(const Evas_Object *obj)
{
   ELM_BOX_CHECK(obj) EINA_FALSE;
   ELM_BOX_DATA_GET(obj, sd);

   return sd->homogeneous;
}

EAPI void
elm_box_pack_start(Evas_Object *obj,
                   Evas_Object *subobj)
{
   ELM_BOX_CHECK(obj);
   ELM_BOX_DATA_GET(obj, sd);

   elm_widget_sub_object_add(obj, subobj);
   evas_object_box_prepend(ELM_WIDGET_DATA(sd)->resize_obj, subobj);
}

EAPI void
elm_box_pack_end(Evas_Object *obj,
                 Evas_Object *subobj)
{
   ELM_BOX_CHECK(obj);
   ELM_BOX_DATA_GET(obj, sd);

   elm_widget_sub_object_add(obj, subobj);
   evas_object_box_append(ELM_WIDGET_DATA(sd)->resize_obj, subobj);
}

EAPI void
elm_box_pack_before(Evas_Object *obj,
                    Evas_Object *subobj,
                    Evas_Object *before)
{
   ELM_BOX_CHECK(obj);
   ELM_BOX_DATA_GET(obj, sd);

   elm_widget_sub_object_add(obj, subobj);
   evas_object_box_insert_before
     (ELM_WIDGET_DATA(sd)->resize_obj, subobj, before);
}

EAPI void
elm_box_pack_after(Evas_Object *obj,
                   Evas_Object *subobj,
                   Evas_Object *after)
{
   ELM_BOX_CHECK(obj);
   ELM_BOX_DATA_GET(obj, sd);

   elm_widget_sub_object_add(obj, subobj);
   evas_object_box_insert_after
     (ELM_WIDGET_DATA(sd)->resize_obj, subobj, after);
}

EAPI void
elm_box_clear(Evas_Object *obj)
{
   ELM_BOX_CHECK(obj);
   ELM_BOX_DATA_GET(obj, sd);

   /* EINA_TRUE means to delete objects as well */
   evas_object_box_remove_all(ELM_WIDGET_DATA(sd)->resize_obj, EINA_TRUE);
}

EAPI void
elm_box_unpack(Evas_Object *obj,
               Evas_Object *subobj)
{
   ELM_BOX_CHECK(obj);
   ELM_BOX_DATA_GET(obj, sd);

   if (evas_object_box_remove(ELM_WIDGET_DATA(sd)->resize_obj, subobj))
     elm_widget_sub_object_del(obj, subobj);
}

EAPI void
elm_box_unpack_all(Evas_Object *obj)
{
   Evas_Object_Box_Data *bd;
   Evas_Object_Box_Option *opt;
   Eina_List *l;

   ELM_BOX_CHECK(obj);
   ELM_BOX_DATA_GET(obj, sd);

   bd = evas_object_smart_data_get(ELM_WIDGET_DATA(sd)->resize_obj);
   EINA_LIST_FOREACH(bd->children, l, opt)
     elm_widget_sub_object_del(obj, opt->obj);

   /* EINA_FALSE means to delete objects as well */
   evas_object_box_remove_all(ELM_WIDGET_DATA(sd)->resize_obj, EINA_FALSE);
}

EAPI void
elm_box_layout_set(Evas_Object *obj,
                   Evas_Object_Box_Layout cb,
                   const void *data,
                   Ecore_Cb free_data)
{
   ELM_BOX_CHECK(obj);
   ELM_BOX_DATA_GET(obj, sd);

   if (cb)
     evas_object_box_layout_set
       (ELM_WIDGET_DATA(sd)->resize_obj, cb, data, free_data);
   else
     evas_object_box_layout_set
       (ELM_WIDGET_DATA(sd)->resize_obj, _elm_box_custom_layout, obj, NULL);
}

EAPI void
elm_box_layout_transition(Evas_Object *obj,
                          Evas_Object_Box_Data *priv,
                          void *data)
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
                       Evas_Object_Box_Layout start_layout,
                       void *start_layout_data,
                       Ecore_Cb start_layout_free_data,
                       Evas_Object_Box_Layout end_layout,
                       void *end_layout_data,
                       Ecore_Cb end_layout_free_data,
                       Ecore_Cb transition_end_cb,
                       void *transition_end_data)
{
   Elm_Box_Transition *box_data;

   EINA_SAFETY_ON_NULL_RETURN_VAL(start_layout, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(end_layout, NULL);

   box_data = calloc(1, sizeof(Elm_Box_Transition));
   if (!box_data) return NULL;

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
   EINA_LIST_FREE (box_data->objs, tad)
     free(tad);

   evas_object_event_callback_del
     (box_data->box, EVAS_CALLBACK_RESIZE, _transition_layout_obj_resize_cb);
   evas_object_smart_callback_del
     (box_data->box, SIG_CHILD_ADDED, _transition_layout_child_added);
   evas_object_smart_callback_del
     (box_data->box, SIG_CHILD_REMOVED, _transition_layout_child_removed);
   if (box_data->animator)
     {
        ecore_animator_del(box_data->animator);
        box_data->animator = NULL;
     }

   free(data);
}

EAPI Eina_List *
elm_box_children_get(const Evas_Object *obj)
{
   ELM_BOX_CHECK(obj) NULL;
   ELM_BOX_DATA_GET(obj, sd);

   return evas_object_box_children_get(ELM_WIDGET_DATA(sd)->resize_obj);
}

EAPI void
elm_box_padding_set(Evas_Object *obj,
                    Evas_Coord horizontal,
                    Evas_Coord vertical)
{
   ELM_BOX_CHECK(obj);
   ELM_BOX_DATA_GET(obj, sd);

   evas_object_box_padding_set
     (ELM_WIDGET_DATA(sd)->resize_obj, horizontal, vertical);
}

EAPI void
elm_box_padding_get(const Evas_Object *obj,
                    Evas_Coord *horizontal,
                    Evas_Coord *vertical)
{
   ELM_BOX_CHECK(obj);
   ELM_BOX_DATA_GET(obj, sd);

   evas_object_box_padding_get
     (ELM_WIDGET_DATA(sd)->resize_obj, horizontal, vertical);
}

EAPI void
elm_box_align_set(Evas_Object *obj,
                  double horizontal,
                  double vertical)
{
   ELM_BOX_CHECK(obj);
   ELM_BOX_DATA_GET(obj, sd);

   evas_object_size_hint_align_set
     (ELM_WIDGET_DATA(sd)->resize_obj, horizontal, vertical);
}

EAPI void
elm_box_align_get(const Evas_Object *obj,
                  double *horizontal,
                  double *vertical)
{
   ELM_BOX_CHECK(obj);
   ELM_BOX_DATA_GET(obj, sd);

   evas_object_size_hint_align_get
     (ELM_WIDGET_DATA(sd)->resize_obj, horizontal, vertical);
}

EAPI void
elm_box_recalculate(Evas_Object *obj)
{
   ELM_BOX_CHECK(obj);
   ELM_BOX_DATA_GET(obj, sd);

   if (sd->recalc) return;

   evas_object_smart_need_recalculate_set
     (ELM_WIDGET_DATA(sd)->resize_obj, EINA_TRUE);
   sd->recalc++;
   evas_object_smart_calculate(ELM_WIDGET_DATA(sd)->resize_obj);
   sd->recalc--;
}
