#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Ecore.h>
#include "eldbus_model_connection_private.h"
#include "eldbus_model_private.h"

#define MY_CLASS ELDBUS_MODEL_CONNECTION_CLASS
#define MY_CLASS_NAME "Eldbus_Model_Connection"

static void _eldbus_model_connection_names_list_cb(void *, const Eldbus_Message *, Eldbus_Pending *);

static Efl_Object*
_eldbus_model_connection_efl_object_constructor(Eo *obj, Eldbus_Model_Connection_Data *pd)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));

   pd->obj = obj;

   return obj;
}

static void
_eldbus_model_connection_efl_object_invalidate(Eo *obj, Eldbus_Model_Connection_Data *pd)
{
   Eldbus_Children_Slice_Promise *slice;

   if (pd->pending) eldbus_pending_cancel(pd->pending);

   EINA_LIST_FREE(pd->requests, slice)
     {
        eina_promise_reject(slice->p, EFL_MODEL_ERROR_UNKNOWN);
        free(slice);
     }

   pd->childrens = eina_list_free(pd->childrens);

   efl_invalidate(efl_super(obj, ELDBUS_MODEL_CONNECTION_CLASS));
}

static void
_eldbus_model_children_list(const Eo *obj, Eldbus_Model_Connection_Data *pd)
{
   Eldbus_Model_Data *sd;

   if (pd->pending || pd->is_listed) return ;

   sd = efl_data_scope_get(obj, ELDBUS_MODEL_CLASS);

   pd->pending = eldbus_names_list(sd->connection,
                                   &_eldbus_model_connection_names_list_cb,
                                   pd);
}

static Eina_Future *
_eldbus_model_connection_efl_model_children_slice_get(Eo *obj,
                                                      Eldbus_Model_Connection_Data *pd,
                                                      unsigned int start,
                                                      unsigned int count)
{
   Eldbus_Children_Slice_Promise* slice;
   Eina_Promise *p;

   if (pd->is_listed)
     {
        Eina_Value v;

        v = efl_model_list_value_get(pd->childrens, start, count);
        return eina_future_resolved(efl_loop_future_scheduler_get(obj), v);
     }

   p = efl_loop_promise_new(obj, _eldbus_eina_promise_cancel, NULL);

   slice = calloc(1, sizeof (Eldbus_Children_Slice_Promise));
   slice->p = p;
   slice->start = start;
   slice->count = count;

   pd->requests = eina_list_prepend(pd->requests, slice);

   _eldbus_model_children_list(obj, pd);
   return efl_future_then(obj, eina_future_new(p));;
}

static unsigned int
_eldbus_model_connection_efl_model_children_count_get(const Eo *obj,
                                                      Eldbus_Model_Connection_Data *pd)
{
   _eldbus_model_children_list(obj, pd);
   return eina_list_count(pd->childrens);
}

static void
_eldbus_model_connection_names_list_cb(void *data,
                                       const Eldbus_Message *msg,
                                       Eldbus_Pending *pending EINA_UNUSED)
{
   Eldbus_Model_Connection_Data *pd = (Eldbus_Model_Connection_Data*) data;
   Eldbus_Model_Data *sd;
   Eldbus_Children_Slice_Promise *slice;
   const char *error_name, *error_text;
   Eldbus_Message_Iter *array = NULL;
   const char *bus;

   pd->pending = NULL;

   if (eldbus_message_error_get(msg, &error_name, &error_text))
     {
        ERR("%s: %s", error_name, error_text);
        return;
     }

   if (!eldbus_message_arguments_get(msg, "as", &array))
     {
        ERR("%s", "Error getting arguments.");
        return;
     }

   sd = efl_data_scope_get(pd->obj, ELDBUS_MODEL_CLASS);

   while (eldbus_message_iter_get_and_next(array, 's', &bus))
     {
        Eo *child;

        DBG("(%p): bus = %s", pd->obj, bus);

        child = efl_add(ELDBUS_MODEL_OBJECT_CLASS, pd->obj,
                        eldbus_model_connection_set(efl_added, sd->connection),
                        eldbus_model_object_bus_set(efl_added, bus),
                        eldbus_model_object_path_set(efl_added, "/"));

        pd->childrens = eina_list_append(pd->childrens, child);
     }

   pd->is_listed = EINA_TRUE;

   efl_event_callback_call(pd->obj, EFL_MODEL_EVENT_CHILDREN_COUNT_CHANGED, NULL);

   EINA_LIST_FREE(pd->requests, slice)
     {
        Eina_Value v;

        v = efl_model_list_value_get(pd->childrens,
                                     slice->start, slice->count);
        eina_promise_resolve(slice->p, v);

        free(slice);
     }
}

#include "eldbus_model_connection.eo.c"
