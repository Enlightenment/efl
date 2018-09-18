#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_OBJECT_PROTECTED
#define EFL_UI_FOCUS_COMPOSITION_PROTECTED

#include <Elementary.h>
#include <elm_box.eo.h>
#include "elm_priv.h"
#include "els_box.h"
#include "elm_widget_box.h"

#define MY_CLASS ELM_BOX_CLASS
#define MY_CLASS_NAME "Elm_Box"
#define MY_CLASS_NAME_LEGACY "elm_box"

static const char SIG_CHILD_ADDED[] = "child,added";
static const char SIG_CHILD_REMOVED[] = "child,removed";
static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_CHILD_ADDED, ""},
   {SIG_CHILD_REMOVED, ""},
   {NULL, NULL}
};

static void
_elm_box_efl_ui_focus_composition_prepare(Eo *obj, Elm_Box_Data *pd EINA_UNUSED)
{
   Eina_List *n, *nn;
   Efl_Ui_Widget *elem;

   Elm_Widget_Smart_Data *wpd = efl_data_scope_get(obj, EFL_UI_WIDGET_CLASS);
   Eina_List *order = evas_object_box_children_get(wpd->resize_obj);

   EINA_LIST_FOREACH_SAFE(order, n, nn, elem)
     {
        if (!efl_isa(elem, EFL_UI_WIDGET_CLASS))
          order = eina_list_remove(order, elem);
     }

   efl_ui_focus_composition_elements_set(obj, order);
}

static void
_child_added_cb_proxy(void *data, const Efl_Event *event)
{
   Evas_Object *box = data;
   Evas_Object_Box_Option *opt = event->info;
   efl_event_callback_legacy_call(box, ELM_BOX_EVENT_CHILD_ADDED, opt->obj);
}

static void
_child_removed_cb_proxy(void *data, const Efl_Event *event)
{
   Evas_Object *box = data;
   Evas_Object *child = event->info;
   efl_event_callback_legacy_call(box, ELM_BOX_EVENT_CHILD_REMOVED, child);
}

EOLIAN static Efl_Ui_Theme_Apply
_elm_box_efl_ui_widget_theme_apply(Eo *obj, Elm_Box_Data *sd EINA_UNUSED)
{
   Efl_Ui_Theme_Apply int_ret = EFL_UI_THEME_APPLY_FAILED;

   int_ret = efl_ui_widget_theme_apply(efl_super(obj, MY_CLASS));
   if (!int_ret) return EFL_UI_THEME_APPLY_FAILED;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EFL_UI_THEME_APPLY_FAILED);
   evas_object_smart_calculate(wd->resize_obj);

   return int_ret;
}

static void
_sizing_eval(Evas_Object *obj)
{
   Evas_Coord minw = -1, minh = -1, maxw = -1, maxh = -1;

   ELM_BOX_DATA_GET(obj, sd);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   if (!efl_alive_get(obj)) return;
   if (sd->delete_me) return;

   evas_object_size_hint_combined_min_get(wd->resize_obj, &minw, &minh);
   evas_object_size_hint_max_get(wd->resize_obj, &maxw, &maxh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, maxw, maxh);
}

static void
_on_size_hints_changed(void *data,
                       Evas *e EINA_UNUSED,
                       Evas_Object *obj EINA_UNUSED,
                       void *event_info EINA_UNUSED)
{
   _sizing_eval(data);
}

EOLIAN static Eina_Bool
_elm_box_efl_ui_widget_widget_sub_object_del(Eo *obj, Elm_Box_Data *_pd EINA_UNUSED, Evas_Object *child)
{
   Eina_Bool int_ret = EINA_FALSE;

   int_ret = elm_widget_sub_object_del(efl_super(obj, MY_CLASS), child);
   if (!int_ret) return EINA_FALSE;

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
                   efl_ui_mirrored_get(data));
}

static Eina_Bool
_transition_animation(void *data)
{
   evas_object_smart_changed(data);

   return ECORE_CALLBACK_RENEW;
}

static void
_transition_layout_child_added(void *data, const Efl_Event *event)
{
   Transition_Animation_Data *tad;
   Evas_Object_Box_Option *opt = event->info;
   Elm_Box_Transition *layout_data = data;

   tad = calloc(1, sizeof(Transition_Animation_Data));
   if (!tad) return;

   tad->obj = opt->obj;
   layout_data->objs = eina_list_append(layout_data->objs, tad);
   layout_data->recalculate = EINA_TRUE;
}

static void
_transition_layout_child_removed(void *data, const Efl_Event *event)
{
   Eina_List *l;
   Transition_Animation_Data *tad;
   Elm_Box_Transition *layout_data = data;

   EINA_LIST_FOREACH(layout_data->objs, l, tad)
     {
        if (tad->obj == event->info)
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
                                 Evas *e EINA_UNUSED,
                                 Evas_Object *obj EINA_UNUSED,
                                 void *event_info EINA_UNUSED)
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
   efl_event_callback_add
     (obj, ELM_BOX_EVENT_CHILD_ADDED, _transition_layout_child_added, layout_data);
   efl_event_callback_add
     (obj, ELM_BOX_EVENT_CHILD_REMOVED, _transition_layout_child_removed, layout_data);

   if (!layout_data->animator)
     layout_data->animator = ecore_evas_animator_add(obj, transition_animation_cb, obj);

   layout_data->animation_ended = EINA_FALSE;

   return EINA_TRUE;
}

static void
_transition_layout_animation_stop(Elm_Box_Transition *layout_data)
{
   layout_data->animation_ended = EINA_TRUE;

   ELM_SAFE_FREE(layout_data->animator, ecore_animator_del);

   if (layout_data->transition_end_cb)
     layout_data->transition_end_cb(layout_data->transition_end_data);
}

static void
_transition_layout_animation_exec(Evas_Object *obj,
                                  Evas_Object_Box_Data *priv EINA_UNUSED,
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

EOLIAN static void
_elm_box_efl_canvas_group_group_calculate(Eo *obj, Elm_Box_Data *_pd EINA_UNUSED)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   elm_box_recalculate(obj);
}

EOLIAN static void
_elm_box_efl_canvas_group_group_add(Eo *obj, Elm_Box_Data *_pd EINA_UNUSED)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   elm_widget_resize_object_set(obj, evas_object_box_add(evas_object_evas_get(obj)));

   evas_object_box_layout_set(wd->resize_obj, _elm_box_custom_layout,
                              obj, NULL);

   evas_object_event_callback_add(wd->resize_obj,
                                  EVAS_CALLBACK_CHANGED_SIZE_HINTS,
                                  _on_size_hints_changed, obj);

   efl_canvas_group_add(efl_super(obj, MY_CLASS));
   elm_widget_sub_object_parent_add(obj);

   efl_event_callback_add
      (wd->resize_obj, ELM_BOX_EVENT_CHILD_ADDED, _child_added_cb_proxy, obj);
   efl_event_callback_add
     (wd->resize_obj, ELM_BOX_EVENT_CHILD_REMOVED, _child_removed_cb_proxy, obj);

   elm_widget_can_focus_set(obj, EINA_FALSE);
   elm_widget_highlight_ignore_set(obj, EINA_TRUE);
}

EOLIAN static void
_elm_box_efl_canvas_group_group_del(Eo *obj, Elm_Box_Data *sd)
{
   Eina_List *l;
   Evas_Object *child;

   sd->delete_me = EINA_TRUE;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   evas_object_event_callback_del_full
     (wd->resize_obj, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
     _on_size_hints_changed, obj);

   /* let's make our box object the *last* to be processed, since it
    * may (smart) parent other sub objects here */
   EINA_LIST_FOREACH (wd->subobjs, l, child)
     {
        if (child == wd->resize_obj)
          {
             wd->subobjs =
               eina_list_demote_list(wd->subobjs, l);
             break;
          }
     }

   efl_canvas_group_del(efl_super(obj, MY_CLASS));
}

EAPI Evas_Object *
elm_box_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   return elm_legacy_add(MY_CLASS, parent);
}

EOLIAN static Eo *
_elm_box_efl_object_constructor(Eo *obj, Elm_Box_Data *_pd EINA_UNUSED)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   efl_canvas_object_type_set(obj, MY_CLASS_NAME_LEGACY);
   evas_object_smart_callbacks_descriptions_set(obj, _smart_callbacks);
   efl_access_object_access_type_set(obj, EFL_ACCESS_TYPE_SKIPPED);
   efl_access_object_role_set(obj, EFL_ACCESS_ROLE_FILLER);

   return obj;
}

EOLIAN static void
_elm_box_horizontal_set(Eo *obj, Elm_Box_Data *sd, Eina_Bool horizontal)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   sd->horizontal = !!horizontal;
   evas_object_smart_calculate(wd->resize_obj);
}

EOLIAN static Eina_Bool
_elm_box_horizontal_get(const Eo *obj EINA_UNUSED, Elm_Box_Data *sd)
{
   return sd->horizontal;
}

EOLIAN static void
_elm_box_homogeneous_set(Eo *obj, Elm_Box_Data *sd, Eina_Bool homogeneous)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   sd->homogeneous = !!homogeneous;
   evas_object_smart_calculate(wd->resize_obj);
}

EOLIAN static Eina_Bool
_elm_box_homogeneous_get(const Eo *obj EINA_UNUSED, Elm_Box_Data *sd)
{
   return sd->homogeneous;
}

EOLIAN static void
_elm_box_pack_start(Eo *obj, Elm_Box_Data *pd EINA_UNUSED, Evas_Object *subobj)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   elm_widget_sub_object_add(obj, subobj);
   evas_object_box_prepend(wd->resize_obj, subobj);
   efl_ui_focus_composition_dirty(obj);
}

EOLIAN static void
_elm_box_pack_end(Eo *obj, Elm_Box_Data *pd EINA_UNUSED, Evas_Object *subobj)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   elm_widget_sub_object_add(obj, subobj);
   evas_object_box_append(wd->resize_obj, subobj);
   efl_ui_focus_composition_dirty(obj);
}

EOLIAN static void
_elm_box_pack_before(Eo *obj, Elm_Box_Data *pd EINA_UNUSED, Evas_Object *subobj, Evas_Object *before)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   elm_widget_sub_object_add(obj, subobj);

   evas_object_box_insert_before
     (wd->resize_obj, subobj, before);
   efl_ui_focus_composition_dirty(obj);
}

EOLIAN static void
_elm_box_pack_after(Eo *obj, Elm_Box_Data *pd EINA_UNUSED, Evas_Object *subobj, Evas_Object *after)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   elm_widget_sub_object_add(obj, subobj);
   evas_object_box_insert_after
     (wd->resize_obj, subobj, after);
   efl_ui_focus_composition_dirty(obj);
}

EOLIAN static void
_elm_box_clear(Eo *obj, Elm_Box_Data *pd EINA_UNUSED)
{
   /* EINA_TRUE means to delete objects as well */
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   evas_object_box_remove_all(wd->resize_obj, EINA_TRUE);
   efl_ui_focus_composition_dirty(obj);
}

EOLIAN static void
_elm_box_unpack(Eo *obj, Elm_Box_Data *pd EINA_UNUSED, Evas_Object *subobj)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   if (evas_object_box_remove(wd->resize_obj, subobj))
     _elm_widget_sub_object_redirect_to_top(obj, subobj);
   efl_ui_focus_composition_dirty(obj);
}

EOLIAN static void
_elm_box_unpack_all(Eo *obj, Elm_Box_Data *pd EINA_UNUSED)
{
   Evas_Object_Box_Data *bd;
   Evas_Object_Box_Option *opt;
   Eina_List *l, *children = NULL;
   Evas_Object *c;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   /* set this to block _sizing_eval() calls */
   pd->delete_me = EINA_TRUE;
   bd = evas_object_smart_data_get(wd->resize_obj);
   if (bd)
     {
        EINA_LIST_FOREACH (bd->children, l, opt)
          children = eina_list_append(children, opt->obj);
     }
   pd->delete_me = EINA_FALSE;

   /* EINA_FALSE means do not delete objects */
   evas_object_box_remove_all(wd->resize_obj, EINA_FALSE);
   EINA_LIST_FREE(children, c)
     _elm_widget_sub_object_redirect_to_top(obj, c);
   /* update size hints */
   _sizing_eval(obj);
   efl_ui_focus_composition_dirty(obj);
}

EAPI void
elm_box_layout_set(Eo *obj, Evas_Object_Box_Layout cb, const void *data, Ecore_Cb free_data)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   if (cb)
     evas_object_box_layout_set
       (wd->resize_obj, cb, data, free_data);
   else
     evas_object_box_layout_set
       (wd->resize_obj, _elm_box_custom_layout, obj, NULL);
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
   EINA_LIST_FREE(box_data->objs, tad)
     free(tad);

   evas_object_event_callback_del
     (box_data->box, EVAS_CALLBACK_RESIZE, _transition_layout_obj_resize_cb);
   efl_event_callback_del(box_data->box, ELM_BOX_EVENT_CHILD_ADDED, _transition_layout_child_added, box_data);
   efl_event_callback_del(box_data->box, ELM_BOX_EVENT_CHILD_REMOVED, _transition_layout_child_removed, box_data);
   ecore_animator_del(box_data->animator);

   free(data);
}

EOLIAN static Eina_List*
_elm_box_children_get(const Eo *obj, Elm_Box_Data *_pd EINA_UNUSED)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, NULL);
   return evas_object_box_children_get(wd->resize_obj);
}

EOLIAN static void
_elm_box_padding_set(Eo *obj, Elm_Box_Data *_pd EINA_UNUSED, Evas_Coord horizontal, Evas_Coord vertical)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   evas_object_box_padding_set
     (wd->resize_obj, horizontal, vertical);
}

EOLIAN static void
_elm_box_padding_get(const Eo *obj, Elm_Box_Data *_pd EINA_UNUSED, Evas_Coord *horizontal, Evas_Coord *vertical)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   evas_object_box_padding_get
     (wd->resize_obj, horizontal, vertical);
}

EOLIAN static void
_elm_box_align_set(Eo *obj, Elm_Box_Data *_pd EINA_UNUSED, double horizontal, double vertical)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   evas_object_box_align_set(wd->resize_obj, horizontal, vertical);
}

EOLIAN static void
_elm_box_align_get(const Eo *obj, Elm_Box_Data *_pd EINA_UNUSED, double *horizontal, double *vertical)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   evas_object_box_align_get(wd->resize_obj, horizontal, vertical);
}

EOLIAN static void
_elm_box_recalculate(Eo *obj, Elm_Box_Data *sd)
{
   if (sd->recalc) return;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   evas_object_smart_need_recalculate_set
     (wd->resize_obj, EINA_TRUE);
   sd->recalc++;
   evas_object_smart_calculate(wd->resize_obj);
   sd->recalc--;
}

static void
_elm_box_class_constructor(Efl_Class *klass)
{
   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}
/* Internal EO APIs and hidden overrides */

#define ELM_BOX_EXTRA_OPS \
   EFL_CANVAS_GROUP_ADD_DEL_OPS(elm_box)

#include "elm_box.eo.c"
