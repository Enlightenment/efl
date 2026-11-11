#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

// Note: we do not rely on reflection here to implement select as it require to asynchronously access
// children. Could be done differently by implementing the children select in the parent instead of
// in the children. For later optimization.

#include <Elementary.h>
#include "elm_priv.h"

#include "efl_ui_select_model.eo.h"
#include "ecore_internal.h"

typedef struct _Efl_Ui_Select_Model_Data Efl_Ui_Select_Model_Data;

struct _Efl_Ui_Select_Model_Data
{
   Efl_Ui_Select_Model_Data *parent;

   Eina_Future *pending_selection_event;

   Efl_Ui_Select_Model *fallback_model;
   Efl_Ui_Select_Model *last_model;

   Efl_Ui_Select_Mode selection;
   // Counts the selections asked for: one by index fetches its row first, and in single mode
   // a selection asked for meanwhile is the later one
   unsigned int select_seq;

   Eina_Bool allow_manual_deselection : 1;
   // Set while the unselects that going to none asks for are applied: they are not a manual
   // deselection, and nothing takes the place of what they unselect
   Eina_Bool clearing : 1;
};

static void _unselect_range(Eo *obj, unsigned int a, unsigned int b,
                            Eina_Value (*then)(Eo *o, void *data, const Eina_Value v));
static Eina_Value _children_unselect_keep_picked_then(Eo *o, void *data, const Eina_Value v);
static Eina_Value _children_unselect_clear_then(Eo *o, void *data, const Eina_Value v);

static void
_efl_ui_select_model_apply_last_model(Eo *obj, Efl_Ui_Select_Model_Data *pd, Eo *last_model)
{
   efl_replace(&pd->last_model, last_model);
   efl_model_properties_changed(obj, "child.selected");
}

static void
_efl_ui_select_model_child_removed(void *data, const Efl_Event *event)
{
   Efl_Ui_Select_Model_Data *pd = data;
   Efl_Model_Children_Event *ev = event->info;

   if (ev->child == pd->last_model)
     _efl_ui_select_model_apply_last_model(event->object, pd, NULL);
}

static Eo*
_efl_ui_select_model_efl_object_constructor(Eo *obj,
                                            Efl_Ui_Select_Model_Data *pd)
{
   Eo *parent;

   obj = efl_constructor(efl_super(obj, EFL_UI_SELECT_MODEL_CLASS));

   efl_boolean_model_boolean_add(obj, "selected", EINA_FALSE);

   efl_event_callback_add(obj, EFL_MODEL_EVENT_CHILD_REMOVED, _efl_ui_select_model_child_removed, pd);

   /* Same default as Efl.Ui.Collection: unselecting what is selected is
      allowed until someone says otherwise. */
   pd->allow_manual_deselection = EINA_TRUE;

   parent = efl_parent_get(obj);
   if (efl_isa(parent, EFL_UI_SELECT_MODEL_CLASS))
     pd->parent = efl_data_scope_get(parent, EFL_UI_SELECT_MODEL_CLASS);

   return obj;
}

static void
_efl_ui_select_model_efl_object_invalidate(Eo *obj,
                                           Efl_Ui_Select_Model_Data *pd)
{
   efl_replace(&pd->fallback_model, NULL);
   efl_replace(&pd->last_model, NULL);

   efl_invalidate(efl_super(obj, EFL_UI_SELECT_MODEL_CLASS));
}

// @child was the selected row and is not anymore: the fallback takes its place, unless
// the fallback is @child, which is the one asked to give up the selection.
static void
_efl_ui_select_model_fallback(Efl_Ui_Select_Model_Data *parent_pd, Eo *child)
{
   Eina_Value selected;

   if (!parent_pd) return;
   if (!parent_pd->fallback_model) return;
   if (parent_pd->fallback_model == child) return;
   if (parent_pd->clearing) return;
   // I think it only make sense to trigger the fallback on single mode
   if (parent_pd->selection != EFL_UI_SELECT_MODE_SINGLE) return;

   selected = eina_value_bool_init(EINA_TRUE);
   efl_model_property_set(parent_pd->fallback_model, "self.selected", &selected);
   eina_value_flush(&selected);
}

static Eina_Value
_select_notification_cb(Eo *o, void *data EINA_UNUSED, const Eina_Value v)
{
   Efl_Ui_Select_Model_Data *pd = efl_data_scope_get(o, EFL_UI_SELECT_MODEL_CLASS);

   efl_event_callback_call(o, EFL_UI_SELECTABLE_EVENT_SELECTION_CHANGED, NULL);

   pd->pending_selection_event = NULL;

   return v;
}

static void
_efl_ui_select_model_selection_notification(Eo *parent, Efl_Ui_Select_Model_Data *pd)
{
   if (!pd) return;
   if (pd->pending_selection_event) return;

   pd->pending_selection_event = efl_future_then(parent,
                                                 efl_loop_job(efl_loop_get(parent)),
                                                 .success = _select_notification_cb);
}

static Eina_Value
_commit_change(Eo *child, void *data EINA_UNUSED, const Eina_Value v)
{
   Efl_Ui_Select_Model_Data *pd;
   Eo *parent;
   Eina_Value *selected = NULL;
   Eina_Bool selflag = EINA_FALSE;

   if (v.type == EINA_VALUE_TYPE_ERROR)
     goto on_error;

   selected = efl_model_property_get(child, "selected");

   parent = efl_parent_get(child);
   pd = efl_data_scope_get(parent, EFL_UI_SELECT_MODEL_CLASS);
   if (!pd) goto on_error;

   eina_value_bool_get(selected, &selflag);
   if (selflag)
     {
        // select case (in single mode the last one was set when the selection was asked for,
        // and a later one asked for before this commit is the last one)
        if ((pd->selection != EFL_UI_SELECT_MODE_SINGLE) || !pd->last_model)
          _efl_ui_select_model_apply_last_model(parent, pd, child);
        efl_event_callback_call(child, EFL_UI_SELECT_MODEL_EVENT_SELECTED, child);
     }
   else
     {
        // unselect case
        // There should only be one model which represent the same data at all in memory
        if (pd->last_model == child) // direct comparison of pointer is ok
          {
             _efl_ui_select_model_apply_last_model(parent, pd, NULL);

             // Just in case we need to refill the fallback
             _efl_ui_select_model_fallback(pd, child);
          }
        efl_event_callback_call(child, EFL_UI_SELECT_MODEL_EVENT_UNSELECTED, child);
     }
   _efl_ui_select_model_selection_notification(parent, pd);
   efl_model_properties_changed(child, "self.selected");

 on_error:
   eina_value_free(selected);
   return v;
}

EOLIAN static void
_efl_ui_select_model_efl_ui_single_selectable_allow_manual_deselection_set(Eo *obj EINA_UNUSED,
                                                                           Efl_Ui_Select_Model_Data *pd,
                                                                           Eina_Bool allow_manual_deselection)
{
   pd->allow_manual_deselection = !!allow_manual_deselection;
}

EOLIAN static Eina_Bool
_efl_ui_select_model_efl_ui_single_selectable_allow_manual_deselection_get(const Eo *obj EINA_UNUSED,
                                                                           Efl_Ui_Select_Model_Data *pd)
{
   return pd->allow_manual_deselection;
}

static void
_clear_child(Eo *child,
             void *data EINA_UNUSED,
	     const Eina_Future *dead_future EINA_UNUSED)
{
   efl_unref(child);
}

static Efl_Model *
_select_child_get(const Eina_Value *array, unsigned int idx)
{
   Efl_Model *ret = NULL;

   if (eina_value_type_get(array) != EINA_VALUE_TYPE_ARRAY)
     return NULL;

   if (idx >= eina_value_array_count(array))
     return NULL;

   eina_value_array_get(array, idx, &ret);

   return ret;
}

static Eina_Future *
_check_child_change(Efl_Model *child, Eina_Bool value)
{
   Eina_Future *r = NULL;
   Eina_Value *prev;
   Eina_Bool prevflag = EINA_FALSE;

   prev = efl_model_property_get(child, "selected");
   eina_value_bool_get(prev, &prevflag);
   eina_value_free(prev);

   if (prevflag & value)
     {
        r = efl_loop_future_resolved(child, eina_value_bool_init(value));
     }
   else
     {
        r = efl_model_property_set(child, "selected", eina_value_bool_new(!!value));
        r = efl_future_then(efl_ref(child), r,
                            .success = _commit_change,
                            .free = _clear_child);
     }

   return r;
}

// Selecting by index is selecting the row: through "self.selected", which in single mode
// is what unselects the row selected before.
static Eina_Future *
_select_child(Efl_Model *child)
{
   Eina_Value select = eina_value_bool_init(EINA_TRUE);
   Eina_Future *r;

   r = efl_model_property_set(child, "self.selected", &select);
   eina_value_flush(&select);
   return r;
}

static Eina_Future *
_unselect_child(Efl_Model *child)
{
   return _check_child_change(child, EINA_FALSE);
}

static Eina_Value
_select_slice_then(Eo *obj,
                   void *data,
                   const Eina_Value v)
{
   Efl_Ui_Select_Model_Data *pd = efl_data_scope_get(obj, EFL_UI_SELECT_MODEL_CLASS);
   unsigned int seq = (unsigned int) (uintptr_t) data;
   Efl_Model *child = NULL;
   Eina_Future *r;

   // Single mode: a selection asked for after this one, while its row was fetched, wins
   if ((pd->selection == EFL_UI_SELECT_MODE_SINGLE) && (pd->select_seq != seq))
     return v;

   child = _select_child_get(&v, 0);
   if (!child) goto on_error;

   r = _select_child(child);
   return eina_future_as_value(r);

 on_error:
   return v;
}

static Eina_Value
_unselect_slice_then(Eo *obj EINA_UNUSED,
                     void *data EINA_UNUSED,
                     const Eina_Value v)
{
   Efl_Model *child = NULL;
   Eina_Future *r;

   child = _select_child_get(&v, 0);
   if (!child) goto on_error;

   r = _unselect_child(child);
   return eina_future_as_value(r);

 on_error:
   return v;
}

static Eina_Value
_regenerate_error(void *data,
                  const Eina_Value v,
                  const Eina_Future *dead_future EINA_UNUSED)
{
   Eina_Error *error = data;
   Eina_Value r = v;

   if (v.type == EINA_VALUE_TYPE_ERROR)
     goto cleanup;

   r = eina_value_error_init(*error);

 cleanup:
   free(error);

   return r;
}

static Eina_Value
_untangle_array(void *data EINA_UNUSED,
                const Eina_Value v)
{
   Eina_Value va = EINA_VALUE_EMPTY;

   // Only return the commit change, not the result of the unselect
   eina_value_array_get(&v, 0, &va);
   return va;
}

// What a select that unselects the row selected before keeps to roll back with
typedef struct
{
   Eo *obj;  // the row being selected
   Eo *prev; // the row that was selected, which this one was to replace
} Untangle;

static Untangle *
_untangle_new(Eo *obj, Eo *prev)
{
   Untangle *u = calloc(1, sizeof (Untangle));

   if (!u) return NULL;
   u->obj = efl_ref(obj);
   u->prev = efl_ref(prev);
   return u;
}

// We need to roll back the change, which means in this
// case to unselect this child as this is the only case
// where we could end up here. The row before stays the selected one, when it is still a
// row and is still selected, as its unselect is what failed.
static Eina_Value
_untangle_rollback(Untangle *u, Eina_Error err)
{
   Eo *child = u->obj;
   Efl_Ui_Select_Model_Data *pd = efl_data_scope_get(efl_parent_get(child),
                                                     EFL_UI_SELECT_MODEL_CLASS);
   Eina_Future *f;
   Eina_Error *error = calloc(1, sizeof (Eina_Error));

   f = efl_model_property_set(efl_super(child, EFL_UI_SELECT_MODEL_CLASS),
                              "selected", eina_value_bool_new(EINA_FALSE));

   // A later selection asked for in the meantime is the last one, not this one
   if ((pd) && (pd->last_model == child))
     {
        Eo *keep = NULL;

        if (u->prev && (efl_composite_model_index_get(u->prev) != 0xFFFFFFFF))
          {
             Eina_Value *selected;
             Eina_Bool flag = EINA_FALSE;

             selected = efl_model_property_get(efl_super(u->prev, EFL_UI_SELECT_MODEL_CLASS),
                                               "selected");
             eina_value_bool_get(selected, &flag);
             eina_value_free(selected);
             if (flag) keep = u->prev;
          }
        _efl_ui_select_model_apply_last_model(efl_parent_get(child), pd, keep);
     }

   if (!error) return eina_future_as_value(f);
   // Once this is done, we need to repropagate the error
   *error = err;
   f = eina_future_then(f, _regenerate_error, error, NULL);

   return eina_future_as_value(f);
}

static Eina_Value
_untangle_error(void *data, Eina_Error err)
{
   return _untangle_rollback(data, err);
}

static void
_untangle_free(void *data,
               const Eina_Future *dead_future EINA_UNUSED)
{
   Untangle *u = data;

   efl_unref(u->obj);
   efl_unref(u->prev);
   free(u);
}

static Eina_Iterator *
_efl_ui_select_model_efl_model_properties_get(const Eo *obj,
                                              Efl_Ui_Select_Model_Data *pd EINA_UNUSED)
{
   EFL_COMPOSITE_MODEL_PROPERTIES_SUPER(props,
                                        obj, EFL_UI_SELECT_MODEL_CLASS,
                                        NULL,
                                        "self.selected", "child.selected", "single_selection");
   return props;
}

static Eina_Future *
_efl_ui_select_model_efl_model_property_set(Eo *obj,
                                            Efl_Ui_Select_Model_Data *pd,
                                            const char *property, Eina_Value *value)
{
   Eina_Value vf = EINA_VALUE_EMPTY;

   if (eina_streq("single_selection", property))
     {
        Eina_Bool single_selection = pd->selection == EFL_UI_SELECT_MODE_SINGLE;
        Eina_Bool new_selection;
        Eina_Bool changed;

        if (!eina_value_bool_get(value, &new_selection))
          return efl_loop_future_rejected(obj, EINVAL);

        changed = (!!new_selection != !!single_selection);
        if (new_selection) pd->selection = EFL_UI_SELECT_MODE_SINGLE;
        else pd->selection = EFL_UI_SELECT_MODE_MULTI;

        if (changed) efl_model_properties_changed(obj, "single_selection");

        return efl_loop_future_resolved(obj, vf);
     }

   if (eina_streq("child.selected", property))
     {
        unsigned long l = 0;

        if (pd->selection == EFL_UI_SELECT_MODE_NONE)
          return efl_loop_future_rejected(obj, EFL_MODEL_ERROR_READ_ONLY);

        if (!eina_value_ulong_convert(value, &l))
          return efl_loop_future_rejected(obj, EFL_MODEL_ERROR_INCORRECT_VALUE);

        return efl_future_then(efl_ref(obj), efl_model_children_slice_get(obj, l, 1),
                               .success = _select_slice_then,
                               .success_type = EINA_VALUE_TYPE_ARRAY,
                               .data = (void *) (uintptr_t) ++pd->select_seq,
                               .free = _clear_child);
     }

   if (pd->parent && eina_streq("self.selected", property))
     {
        Eina_Bool prevflag = EINA_FALSE, newflag = EINA_FALSE;
        Eina_Bool single_selection = EINA_FALSE;
        Eina_Bool success;
        Eina_Value *prev;
        Eina_Future *chain;
        Untangle *untangle = NULL;

        // A row with no index left while a slice held it: it is not a row to select
        if (efl_composite_model_index_get(obj) == 0xFFFFFFFF)
          return efl_loop_future_rejected(obj, EFL_MODEL_ERROR_INCORRECT_VALUE);

        prev = efl_model_property_get(efl_super(obj, EFL_UI_SELECT_MODEL_CLASS), "selected");
        success = eina_value_bool_get(prev, &prevflag);
        success &= eina_value_bool_convert(value, &newflag);

        if (!success) return efl_loop_future_rejected(obj, EFL_MODEL_ERROR_INCORRECT_VALUE);

        // None selects nothing, but what was selected can go: going to none unselects every
        // row, and those unselects arrive once the mode is none
        if ((pd->parent->selection == EFL_UI_SELECT_MODE_NONE) && newflag)
          return efl_loop_future_rejected(obj, EFL_MODEL_ERROR_INCORRECT_VALUE);

        // Nothing changed. A row still selected from before a switch to single selection,
        // that is not the selected one, is not that: picking it makes it the one, or the
        // unselects that follow the switch would take it too.
        if ((newflag == prevflag) &&
            !(newflag && (pd->parent->selection == EFL_UI_SELECT_MODE_SINGLE) &&
              (pd->parent->last_model != obj)))
          return efl_loop_future_resolved(obj, eina_value_bool_init(newflag));

        // (a select that changes nothing does not take the place of one by index in flight)
        if (newflag)
          {
             pd->parent->select_seq++;
          }

        /* Refusing to be the change that leaves nothing selected, which is
           what allow_manual_deselection is for. Refused before the value is
           stored rather than undone afterwards, so the selection is never
           briefly empty and nothing downstream sees a state that was not
           meant to exist.

           A fallback is not this case: something is still selected once it
           takes over, so the deselection is allowed to go through. */
        if (!newflag &&
            (pd->parent->selection == EFL_UI_SELECT_MODE_SINGLE) &&
            (!pd->parent->allow_manual_deselection) &&
            (!pd->parent->clearing) &&
            (!pd->parent->fallback_model) &&
            (pd->parent->last_model == obj))
          return efl_loop_future_resolved(obj, eina_value_bool_init(EINA_TRUE));

        single_selection = !!(pd->parent->selection == EFL_UI_SELECT_MODE_SINGLE);

        // What the select keeps to roll back with, before anything is stored: no memory
        // for it fails the select cleanly
        if (single_selection && newflag && pd->parent->last_model)
          {
             untangle = _untangle_new(obj, pd->parent->last_model);
             if (!untangle) return efl_loop_future_rejected(obj, ENOMEM);
          }

        // First store the new value in the boolean model we inherit from
        chain = efl_model_property_set(efl_super(obj, EFL_UI_SELECT_MODEL_CLASS),
                                       "selected", value);

        // Now act !
        if (single_selection)
          {
             // We are here either, because we weren't and are after this call
             // or because we were selected and are not anymore. In the later case,
             // there is nothing special to do, just normal commit change will do.
             if (!newflag)
               {
                  if (pd->parent->last_model == obj && !newflag)
                    {
                       _efl_ui_select_model_apply_last_model(efl_parent_get(obj), pd->parent, NULL);

                       _efl_ui_select_model_fallback(pd->parent, obj);
                    }
               }
             else if (pd->parent->last_model)
               {
                  Eo *parent;
                  unsigned long selected = 0;

                  // In this case we need to first unselect the previously selected one
                  // and then commit the change to this one.
                  selected = efl_composite_model_index_get(pd->parent->last_model);

                  // There was, need to unselect the previous one along setting the new value
                  parent = efl_parent_get(obj);
                  chain = eina_future_all(chain,
                                          efl_future_then(efl_ref(parent),
                                                          efl_model_children_slice_get(parent, selected, 1),
                                                          .success = _unselect_slice_then,
                                                          .success_type = EINA_VALUE_TYPE_ARRAY,
                                                          .free = _clear_child));
                  chain = eina_future_then_easy(chain,
                                                .success_type = EINA_VALUE_TYPE_ARRAY,
                                                .success = _untangle_array,
                                                .data = untangle,
                                                .error = _untangle_error,
                                                .free = _untangle_free);
                  // This is the last one now, not once it commits: a selection asked for
                  // before then unselects this one, not the one before it
                  _efl_ui_select_model_apply_last_model(parent, pd->parent, obj);
               }
             else
               {
                  _efl_ui_select_model_apply_last_model(efl_parent_get(obj), pd->parent, obj);
               }
          }

        return efl_future_then(efl_ref(obj), chain,
                               .success = _commit_change,
                               .free = _clear_child);
     }

   return efl_model_property_set(efl_super(obj, EFL_UI_SELECT_MODEL_CLASS),
                                 property, value);
}

static Eina_Value *
_efl_ui_select_model_efl_model_property_get(const Eo *obj, Efl_Ui_Select_Model_Data *pd, const char *property)
{
   if (eina_streq("single_selection", property))
     return eina_value_bool_new(pd->selection == EFL_UI_SELECT_MODE_SINGLE);
   // Last selected child
   if (eina_streq("child.selected", property))
     {
        if (pd->last_model)
          return eina_value_ulong_new(efl_composite_model_index_get(pd->last_model));
        else if (pd->fallback_model)
          return eina_value_ulong_new(efl_composite_model_index_get(pd->fallback_model));
        else // Nothing selected yet, try again later
          return eina_value_error_new(EAGAIN);
     }
   // Redirect to are we ourself selected
   if (pd->parent && eina_streq("self.selected", property))
     {
        if (pd->parent->selection == EFL_UI_SELECT_MODE_NONE)
          return eina_value_bool_new(EINA_FALSE);
        return efl_model_property_get(efl_super(obj, EFL_UI_SELECT_MODEL_CLASS), "selected");
     }

   return efl_model_property_get(efl_super(obj, EFL_UI_SELECT_MODEL_CLASS), property);
}

static Eina_Iterator *
_efl_ui_select_model_efl_ui_multi_selectable_index_range_selected_ndx_iterator_new(Eo *obj,
                                                                         Efl_Ui_Select_Model_Data *pd)
{
   if (pd->parent && pd->parent->selection == EFL_UI_SELECT_MODE_NONE)
     return eina_list_iterator_new(NULL); // Quick hack to get a valid empty iterator
   return efl_boolean_model_boolean_iterator_get(obj, "selected", EINA_TRUE);
}

static Eina_Iterator *
_efl_ui_select_model_efl_ui_multi_selectable_index_range_unselected_ndx_iterator_new(Eo *obj,
                                                                           Efl_Ui_Select_Model_Data *pd EINA_UNUSED)
{
   return efl_boolean_model_boolean_iterator_get(obj, "selected", EINA_FALSE);
}

static Efl_Ui_Selectable *
_efl_ui_select_model_efl_ui_single_selectable_last_selected_get(const Eo *obj EINA_UNUSED,
                                                                Efl_Ui_Select_Model_Data *pd)
{
   return pd->last_model;
}

// The row kept on leaving multiple selection, when it was not the last selected: it is
// the selected one from now on, unless a single selection came first, which then wins.
static Eina_Value
_select_keep_one_then(Eo *obj, void *data EINA_UNUSED, const Eina_Value v)
{
   Efl_Ui_Select_Model_Data *pd = efl_data_scope_get(obj, EFL_UI_SELECT_MODEL_CLASS);
   Eina_Value *selected;
   Eina_Bool flag = EINA_FALSE;
   Eo *child = NULL;

   if (pd->selection != EFL_UI_SELECT_MODE_SINGLE) return v;
   if (eina_value_array_count(&v) != 1) return v;
   eina_value_array_get(&v, 0, &child);
   if (!child) return v;

   selected = efl_model_property_get(child, "selected");
   eina_value_bool_get(selected, &flag);
   eina_value_free(selected);
   if (!flag) return v;

   if (!pd->last_model)
     {
        _efl_ui_select_model_apply_last_model(obj, pd, child);
     }
   else if (pd->last_model != child)
     {
        Eina_Value unselect = eina_value_bool_init(EINA_FALSE);

        efl_model_property_set(child, "self.selected", &unselect);
        eina_value_flush(&unselect);
     }
   return v;
}

// Leaving multiple selection for single keeps one row and unselects the others, as
// Efl.Ui.Collection does. That is the row selected last when it is still selected;
// otherwise the lowest selected one (Collection keeps the most recently selected still
// selected, but this model only remembers the last one, and keeping an order of all
// selections would cost every selection a list entry for the rare mode switch).
static void
_select_keep_one(Eo *obj, Efl_Ui_Select_Model_Data *pd)
{
   Eina_Iterator *it;
   unsigned int *index, keep = 0, count;
   Eina_Bool have = EINA_FALSE, any = EINA_FALSE;

   if (pd->last_model)
     {
        keep = efl_composite_model_index_get(pd->last_model);
        // (no index: it is not a row any more)
        have = (keep != 0xFFFFFFFF);
     }
   it = efl_ui_multi_selectable_selected_ndx_iterator_new(obj);
   EINA_ITERATOR_FOREACH(it, index)
     {
        any = EINA_TRUE;
        if (!have)
          {
             keep = *index;
             have = EINA_TRUE;
          }
     }
   eina_iterator_free(it);
   if (!any) return;

   count = efl_model_children_count_get(obj);
   if (keep > 0) _unselect_range(obj, 0, keep - 1, _children_unselect_keep_picked_then);
   if (keep < (count - 1))
     {
        _unselect_range(obj, keep + 1, count - 1, _children_unselect_keep_picked_then);
     }

   if (pd->last_model && (efl_composite_model_index_get(pd->last_model) == keep)) return;
   efl_future_then(obj, efl_model_children_slice_get(obj, keep, 1),
                   .success_type = EINA_VALUE_TYPE_ARRAY,
                   .success = _select_keep_one_then);
}

static void
_efl_ui_select_model_efl_ui_multi_selectable_select_mode_set(Eo *obj,
                                                                   Efl_Ui_Select_Model_Data *pd,
                                                                   Efl_Ui_Select_Mode mode)
{
   switch (mode)
     {
      case EFL_UI_SELECT_MODE_SINGLE:
         mode = EFL_UI_SELECT_MODE_SINGLE;
         if (pd->selection == EFL_UI_SELECT_MODE_MULTI) _select_keep_one(obj, pd);
         break;
      case EFL_UI_SELECT_MODE_NONE:
         // Every row, from single mode too: one selected but not known as the last one (kept
         // on leaving multiple selection, before it is fetched) would come back in multi
         {
            unsigned int count = efl_model_children_count_get(obj);

            if (count) _unselect_range(obj, 0, count - 1, _children_unselect_clear_then);
         }
         break;
      case EFL_UI_SELECT_MODE_MULTI:
         break;
      default:
         ERR("Unknown select mode passed to %s: %i.", efl_debug_name_get(obj), mode);
         return;
     }

   pd->selection = mode;
   efl_model_properties_changed(obj, "single_selection", "child.selected");
}

static Efl_Ui_Select_Mode
_efl_ui_select_model_efl_ui_multi_selectable_select_mode_get(const Eo *obj EINA_UNUSED,
                                                                   Efl_Ui_Select_Model_Data *pd)
{
   return pd->selection;
}

static void
_efl_ui_select_model_efl_ui_multi_selectable_all_select(Eo *obj,
                                                              Efl_Ui_Select_Model_Data *pd EINA_UNUSED)
{
   unsigned int count, i;

   // Not the fastest way to implement it, but will reuse more code and be easier as a v1.
   // It also make it not very async which could be noticeable.
   count = efl_model_children_count_get(obj);

   for (i = 0; i < count; i++)
     {
        Eina_Value p = eina_value_uint_init(i);

        efl_model_property_set(obj, "child.selected", &p);

        eina_value_flush(&p);
     }
}

static void
_efl_ui_select_model_efl_ui_multi_selectable_all_unselect(Eo *obj,
                                                          Efl_Ui_Select_Model_Data *pd EINA_UNUSED)
{
   unsigned int count = efl_model_children_count_get(obj);

   if (!count) return;
   efl_ui_multi_selectable_ndx_range_unselect(obj, 0, count - 1);
}

static void
_efl_ui_select_model_efl_ui_multi_selectable_index_range_ndx_range_select(Eo *obj,
                                                                          Efl_Ui_Select_Model_Data *pd EINA_UNUSED,
                                                                          unsigned int a, unsigned int b)
{
   unsigned long count, i;

   if (a > b)
     {
        unsigned int tmp = a;
        a = b;
        b = tmp;
     }

   // Not the fastest way to implement it, but will reuse more code and be easier as a v1.
   // It also make it not very async which could be noticeable.
   count = MIN(efl_model_children_count_get(obj), (unsigned long) b + 1);

   for (i = a; i < count; i++)
     {
        Eina_Value p = eina_value_ulong_init(i);

        efl_model_property_set(obj, "child.selected", &p);

        eina_value_flush(&p);
     }
}

static Eina_Value
_children_unselect_then(Eo *o EINA_UNUSED, void *data EINA_UNUSED, const Eina_Value v)
{
   Eo *target;
   Eina_Value unselect;
   unsigned int i, len;

   unselect = eina_value_bool_init(EINA_FALSE);

   EINA_VALUE_ARRAY_FOREACH(&v, len, i, target)
     {
        efl_model_property_set(target, "self.selected", &unselect);
     }

   eina_value_flush(&unselect);

   return v;
}

// The rows left over when leaving multiple selection: the one picked since, which is the
// selected one now, stays.
static Eina_Value
_children_unselect_keep_picked_then(Eo *o, void *data EINA_UNUSED, const Eina_Value v)
{
   Efl_Ui_Select_Model_Data *pd = efl_data_scope_get(o, EFL_UI_SELECT_MODEL_CLASS);
   Eo *target;
   Eina_Value unselect;
   unsigned int i, len;

   unselect = eina_value_bool_init(EINA_FALSE);

   EINA_VALUE_ARRAY_FOREACH(&v, len, i, target)
     {
        if ((pd->selection == EFL_UI_SELECT_MODE_SINGLE) && (pd->last_model == target)) continue;
        efl_model_property_set(target, "self.selected", &unselect);
     }

   eina_value_flush(&unselect);

   return v;
}

// What going to none unselects, which allow_manual_deselection does not refuse
static Eina_Value
_children_unselect_clear_then(Eo *o, void *data, const Eina_Value v)
{
   Efl_Ui_Select_Model_Data *pd = efl_data_scope_get(o, EFL_UI_SELECT_MODEL_CLASS);
   Eina_Value r;

   pd->clearing = EINA_TRUE;
   r = _children_unselect_then(o, data, v);
   pd->clearing = EINA_FALSE;

   return r;
}

#define BATCH_MAX 100

static void
_unselect_range(Eo *obj, unsigned int a, unsigned int b,
                Eina_Value (*then)(Eo *o, void *data, const Eina_Value v))
{
   unsigned long count, batch, i;

   if (a > b)
     {
        unsigned int tmp = a;
        a = b;
        b = tmp;
     }

   count = MIN(efl_model_children_count_get(obj), (unsigned long) b + 1);

   // Fetch group request of children in batches not to big to allow for throttling
   // In the callback edit said object property to be unselected
   i = a;
   batch = 0;

   while (i < count)
     {
        Eina_Future *f;

        batch = MIN(i + BATCH_MAX, count);
        batch -= i;

        f = efl_model_children_slice_get(obj, i, batch);
        efl_future_then(obj, f, .success_type = EINA_VALUE_TYPE_ARRAY,
                        .success = then);

        i += batch;
     }
}

static void
_efl_ui_select_model_efl_ui_multi_selectable_index_range_ndx_range_unselect(Eo *obj,
                                                                            Efl_Ui_Select_Model_Data *pd EINA_UNUSED,
                                                                            unsigned int a, unsigned int b)
{
   _unselect_range(obj, a, b, _children_unselect_then);
}

static void
_efl_ui_select_model_efl_ui_single_selectable_fallback_selection_set(Eo *obj,
                                                                     Efl_Ui_Select_Model_Data *pd,
                                                                     Efl_Ui_Selectable *fallback)
{
   Eina_Value *index;

   if (!efl_isa(fallback, EFL_UI_SELECT_MODEL_CLASS))
     {
        ERR("Class of object '%s' does not provide the necessary interface for Efl.Ui.Select_Model.fallback.", efl_debug_name_get(fallback));
        return;
     }
   if (efl_parent_get(fallback) != obj)
     {
        ERR("Provided object '%s' for fallback isn't a child of '%s'.",
            efl_debug_name_get(fallback), efl_debug_name_get(obj));
        return;
     }

   efl_replace(&pd->fallback_model, fallback);

   if (!pd->last_model) return ;

   // When we provide a fallback, we should use it!
   index = efl_model_property_get(fallback, EFL_COMPOSITE_MODEL_CHILD_INDEX);
   efl_model_property_set(obj, "child.selected", index);
   eina_value_free(index);
}

static Efl_Ui_Selectable *
_efl_ui_select_model_efl_ui_single_selectable_fallback_selection_get(const Eo *obj EINA_UNUSED,
                                                                     Efl_Ui_Select_Model_Data *pd)
{
   return pd->fallback_model;
}

static void
_efl_ui_select_model_efl_ui_selectable_selected_set(Eo *obj,
                                                    Efl_Ui_Select_Model_Data *pd EINA_UNUSED,
                                                    Eina_Bool selected)
{
   Eina_Value set = eina_value_bool_init(selected);

   efl_model_property_set(obj, "self.selected", &set);

   eina_value_flush(&set);
}

static Eina_Bool
_efl_ui_select_model_efl_ui_selectable_selected_get(const Eo *obj,
                                                    Efl_Ui_Select_Model_Data *pd EINA_UNUSED)
{
   Eina_Value *selected;
   Eina_Bool r = EINA_FALSE;

   selected = efl_model_property_get(obj, "self.selected");
   eina_value_bool_convert(selected, &r);
   eina_value_free(selected);

   return r;
}

#include "efl_ui_select_model.eo.c"
#include "efl_ui_multi_selectable_index_range.eo.c"
