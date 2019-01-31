#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eina.h"
#include "Efl.h"
#include <Ecore.h>
#include "Eo.h"

#include "efl_select_model.eo.h"
#include "efl_model_accessor_view_private.h"
#include "efl_composite_model_private.h"

typedef struct _Efl_Select_Model_Data Efl_Select_Model_Data;

struct _Efl_Select_Model_Data
{
   Efl_Select_Model_Data *parent;
   unsigned long last;

   Eina_Bool exclusive : 1;
   Eina_Bool none : 1;
};

static Eo*
_efl_select_model_efl_object_constructor(Eo *obj,
                                         Efl_Select_Model_Data *pd EINA_UNUSED)
{
   Eo *parent;

   obj = efl_constructor(efl_super(obj, EFL_SELECT_MODEL_CLASS));

   efl_boolean_model_boolean_add(obj, "selected", EINA_FALSE);

   pd->last = -1;

   parent = efl_parent_get(obj);
   if (efl_isa(parent, EFL_SELECT_MODEL_CLASS))
     pd->parent = efl_data_scope_get(parent, EFL_SELECT_MODEL_CLASS);

   return obj;
}

static Eina_Value
_commit_change(Eo *child, void *data EINA_UNUSED, const Eina_Value v)
{
   Efl_Select_Model_Data *pd;
   Eina_Value *selected = NULL;
   Eina_Bool selflag = EINA_FALSE;

   if (v.type == EINA_VALUE_TYPE_ERROR)
     goto on_error;

   selected = efl_model_property_get(child, "selected");

   pd = efl_data_scope_get(efl_parent_get(child), EFL_SELECT_MODEL_CLASS);
   if (!pd) goto on_error;

   eina_value_bool_get(selected, &selflag);
   if (selflag)
     {
        // select case
        pd->none = EINA_FALSE;
        pd->last = efl_composite_model_index_get(child);
        efl_event_callback_call(child, EFL_SELECT_MODEL_EVENT_SELECTED, child);
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
        efl_event_callback_call(child, EFL_SELECT_MODEL_EVENT_UNSELECTED, child);
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
   efl_del(child);
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
        r = efl_future_then(child, r, .success = _commit_change, .free = _clear_child);
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

   f = efl_model_property_set(efl_super(child, EFL_SELECT_MODEL_CLASS),
                              "selected", eina_value_bool_new(EINA_FALSE));
   // Once this is done, we need to repropagate the error
   *error = err;
   f = eina_future_then(f, _regenerate_error, error, NULL);

   return eina_future_as_value(f);
}

static Eina_Iterator *
_efl_select_model_efl_model_properties_get(const Eo *obj,
                                           Efl_Select_Model_Data *pd EINA_UNUSED)
{
   EFL_COMPOSITE_MODEL_PROPERTIES_SUPER(props,
                                        obj, EFL_SELECT_MODEL_CLASS,
                                        NULL,
                                        "self.selected", "child.selected", "exclusive");
   return props;
}

static Eina_Future *
_efl_select_model_efl_model_property_set(Eo *obj,
                                                      Efl_Select_Model_Data *pd,
                                                      const char *property, Eina_Value *value)
{
   Eina_Value vf = EINA_VALUE_EMPTY;

   if (!strcmp("exclusive", property))
     {
        Eina_Bool exclusive = pd->exclusive;
        Eina_Bool changed;

        vf = eina_value_bool_init(exclusive);
        eina_value_convert(value, &vf);
        eina_value_bool_get(&vf, &exclusive);

        changed = (!pd->exclusive != !exclusive);
        pd->exclusive = !!exclusive;

        if (changed) efl_model_properties_changed(obj, "exclusive");

        return efl_loop_future_resolved(obj, vf);
     }

   if (!strcmp("child.selected", property))
     {
        unsigned long l = 0;

        if (!eina_value_ulong_convert(value, &l))
          return efl_loop_future_rejected(obj, EFL_MODEL_ERROR_INCORRECT_VALUE);

        return efl_future_then(obj, efl_model_children_slice_get(obj, l, 1),
                               .success = _select_slice_then,
                               .success_type = EINA_VALUE_TYPE_ARRAY);
     }

   if (pd->parent && !strcmp("self.selected", property))
     {
        Eina_Bool prevflag = EINA_FALSE, newflag = EINA_FALSE;
        Eina_Bool exclusive = EINA_FALSE;
        Eina_Bool success;
        Eina_Value *prev;
        Eina_Future *chain;

        prev = efl_model_property_get(efl_super(obj, EFL_SELECT_MODEL_CLASS), "selected");
        success = eina_value_bool_get(prev, &prevflag);
        success &= eina_value_bool_convert(value, &newflag);

        if (!success) return efl_loop_future_rejected(obj, EFL_MODEL_ERROR_INCORRECT_VALUE);

        // Nothing changed
        if (newflag == prevflag)
          return efl_loop_future_resolved(obj, eina_value_bool_init(newflag));

        exclusive = pd->parent->exclusive;

        // First store the new value in the boolean model we inherit from
        chain = efl_model_property_set(efl_super(obj, EFL_SELECT_MODEL_CLASS),
                                       "selected", value);

        // Now act !
        if (exclusive)
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
                                          efl_future_then(parent, efl_model_children_slice_get(parent, selected, 1),
                                                          .success = _unselect_slice_then,
                                                          .success_type = EINA_VALUE_TYPE_ARRAY));
                  chain = eina_future_then_easy(chain,
                                                .success_type = EINA_VALUE_TYPE_ARRAY,
                                                .success = _untangle_array,
                                                .data = obj,
                                                .error = _untangle_error);
               }
          }

        return efl_future_then(obj, chain, .success = _commit_change);
     }

   return efl_model_property_set(efl_super(obj, EFL_SELECT_MODEL_CLASS),
                                 property, value);
}

static Eina_Value *
_efl_select_model_efl_model_property_get(const Eo *obj, Efl_Select_Model_Data *pd, const char *property)
{
   if (!strcmp("exclusive", property))
     return eina_value_bool_new(pd->exclusive);
   // Last selected child
   if (!strcmp("child.selected", property))
     {
        if (pd->none)
          return eina_value_error_new(EFL_MODEL_ERROR_INCORRECT_VALUE);
        else
          return eina_value_ulong_new(pd->last);
     }
   // Redirect to are we ourself selected
   if (pd->parent && !strcmp("self.selected", property))
     {
        return efl_model_property_get(efl_super(obj, EFL_SELECT_MODEL_CLASS), "selected");
     }

   return efl_model_property_get(efl_super(obj, EFL_SELECT_MODEL_CLASS), property);
}

#include "efl_select_model.eo.c"
