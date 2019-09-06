#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Elementary.h>
#include "elm_priv.h"

#include "efl_ui_select_model.eo.h"
#include "ecore_internal.h"

typedef struct _Efl_Ui_Select_Model_Data Efl_Ui_Select_Model_Data;

struct _Efl_Ui_Select_Model_Data
{
   Efl_Ui_Select_Model_Data *parent;
   unsigned long last;

   Eina_Bool single_selection : 1;
   Eina_Bool none : 1;
};

static Eo*
_efl_ui_select_model_efl_object_constructor(Eo *obj,
                                         Efl_Ui_Select_Model_Data *pd EINA_UNUSED)
{
   Eo *parent;

   obj = efl_constructor(efl_super(obj, EFL_UI_SELECT_MODEL_CLASS));

   efl_boolean_model_boolean_add(obj, "selected", EINA_FALSE);

   pd->last = -1;

   parent = efl_parent_get(obj);
   if (efl_isa(parent, EFL_UI_SELECT_MODEL_CLASS))
     pd->parent = efl_data_scope_get(parent, EFL_UI_SELECT_MODEL_CLASS);

   return obj;
}

static Eina_Value
_commit_change(Eo *child, void *data EINA_UNUSED, const Eina_Value v)
{
   Efl_Ui_Select_Model_Data *pd;
   Eina_Value *selected = NULL;
   Eina_Bool selflag = EINA_FALSE;

   if (v.type == EINA_VALUE_TYPE_ERROR)
     goto on_error;

   selected = efl_model_property_get(child, "selected");

   pd = efl_data_scope_get(efl_parent_get(child), EFL_UI_SELECT_MODEL_CLASS);
   if (!pd) goto on_error;

   eina_value_bool_get(selected, &selflag);
   if (selflag)
     {
        // select case
        pd->none = EINA_FALSE;
        pd->last = efl_composite_model_index_get(child);
        efl_event_callback_call(child, EFL_UI_SELECT_MODEL_EVENT_SELECTED, child);
     }
   else
     {
        // unselect case
        unsigned long last;

        last = efl_composite_model_index_get(child);
        if (pd->last == last)
          {
             pd->last = 0;
             pd->none = EINA_TRUE;
          }
        efl_event_callback_call(child, EFL_UI_SELECT_MODEL_EVENT_UNSELECTED, child);
     }

 on_error:
   eina_value_free(selected);
   return v;
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

static Eina_Future *
_select_child(Efl_Model *child)
{
   return _check_child_change(child, EINA_TRUE);
}

static Eina_Future *
_unselect_child(Efl_Model *child)
{
   return _check_child_change(child, EINA_FALSE);
}

static Eina_Value
_select_slice_then(Eo *obj EINA_UNUSED,
                   void *data EINA_UNUSED,
                   const Eina_Value v)
{
   Efl_Model *child = NULL;
   Eina_Future *r;

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

static Eina_Value
_untangle_error(void *data, Eina_Error err)
{
   Efl_Model *child = data;
   Eina_Future *f;

   // We need to roll back the change, which means in this
   // case to unselect this child as this is the only case
   // where we could end up here.
   Eina_Error *error = calloc(1, sizeof (Eina_Error));

   f = efl_model_property_set(efl_super(child, EFL_UI_SELECT_MODEL_CLASS),
                              "selected", eina_value_bool_new(EINA_FALSE));
   // Once this is done, we need to repropagate the error
   *error = err;
   f = eina_future_then(f, _regenerate_error, error, NULL);

   return eina_future_as_value(f);
}

static void
_untangle_free(void *data,
               const Eina_Future *dead_future EINA_UNUSED)
{
   Eo *obj = data;

   efl_unref(obj);
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
        Eina_Bool single_selection = pd->single_selection;
        Eina_Bool changed;

        vf = eina_value_bool_init(single_selection);
        eina_value_convert(value, &vf);
        eina_value_bool_get(&vf, &single_selection);

        changed = (!pd->single_selection != !single_selection);
        pd->single_selection = !!single_selection;

        if (changed) efl_model_properties_changed(obj, "single_selection");

        return efl_loop_future_resolved(obj, vf);
     }

   if (eina_streq("child.selected", property))
     {
        unsigned long l = 0;

        if (!eina_value_ulong_convert(value, &l))
          return efl_loop_future_rejected(obj, EFL_MODEL_ERROR_INCORRECT_VALUE);

        return efl_future_then(efl_ref(obj), efl_model_children_slice_get(obj, l, 1),
                               .success = _select_slice_then,
                               .success_type = EINA_VALUE_TYPE_ARRAY,
                               .free = _clear_child);
     }

   if (pd->parent && eina_streq("self.selected", property))
     {
        Eina_Bool prevflag = EINA_FALSE, newflag = EINA_FALSE;
        Eina_Bool single_selection = EINA_FALSE;
        Eina_Bool success;
        Eina_Value *prev;
        Eina_Future *chain;

        prev = efl_model_property_get(efl_super(obj, EFL_UI_SELECT_MODEL_CLASS), "selected");
        success = eina_value_bool_get(prev, &prevflag);
        success &= eina_value_bool_convert(value, &newflag);

        if (!success) return efl_loop_future_rejected(obj, EFL_MODEL_ERROR_INCORRECT_VALUE);

        // Nothing changed
        if (newflag == prevflag)
          return efl_loop_future_resolved(obj, eina_value_bool_init(newflag));

        single_selection = pd->parent->single_selection;

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
                  unsigned int i;

                  i = efl_composite_model_index_get(obj);
                  if (pd->parent->last == i && !newflag)
                    pd->parent->none = EINA_TRUE;
               }
             else
               {
                  Eo *parent;
                  unsigned long selected = 0;

                  // In this case we need to first unselect the previously selected one
                  // and then commit the change to this one.
                  selected = pd->parent->last;

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
                                                .data = efl_ref(obj),
                                                .error = _untangle_error,
                                                .free = _untangle_free);
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
     return eina_value_bool_new(pd->single_selection);
   // Last selected child
   if (eina_streq("child.selected", property))
     {
        if (pd->none)
          return eina_value_error_new(EFL_MODEL_ERROR_INCORRECT_VALUE);
        else
          return eina_value_ulong_new(pd->last);
     }
   // Redirect to are we ourself selected
   if (pd->parent && eina_streq("self.selected", property))
     {
        return efl_model_property_get(efl_super(obj, EFL_UI_SELECT_MODEL_CLASS), "selected");
     }

   return efl_model_property_get(efl_super(obj, EFL_UI_SELECT_MODEL_CLASS), property);
}

static void
_efl_ui_select_model_single_selection_set(Eo *obj EINA_UNUSED, Efl_Ui_Select_Model_Data *pd, Eina_Bool enable)
{
   pd->single_selection = enable;
}

static Eina_Bool
_efl_ui_select_model_single_selection_get(const Eo *obj EINA_UNUSED, Efl_Ui_Select_Model_Data *pd)
{
   return pd->single_selection;
}

static Eina_Iterator *
_efl_ui_select_model_selected_get(Eo *obj, Efl_Ui_Select_Model_Data *pd EINA_UNUSED)
{
   return efl_boolean_model_boolean_iterator_get(obj, "selected", EINA_TRUE);
}

static Eina_Iterator *
_efl_ui_select_model_unselected_get(Eo *obj, Efl_Ui_Select_Model_Data *pd EINA_UNUSED)
{
   return efl_boolean_model_boolean_iterator_get(obj, "selected", EINA_FALSE);
}

#include "efl_ui_select_model.eo.c"
