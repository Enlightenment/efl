#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Eina.h>
#include <Efl.h>
#include <Ecore.h>

#include "ecore_private.h"

#include "efl_model_composite.eo.h"

typedef struct _Efl_Model_Composite_Data Efl_Model_Composite_Data;

struct _Efl_Model_Composite_Data
{
   Efl_Model *source;
};

static void
_efl_model_composite_efl_object_destructor(Eo *obj, Efl_Model_Composite_Data *pd)
{
   if (pd->source)
     {
        efl_event_callback_forwarder_del(pd->source, EFL_MODEL_EVENT_CHILD_ADDED, obj);
        efl_event_callback_forwarder_del(pd->source, EFL_MODEL_EVENT_CHILD_REMOVED, obj);
        efl_event_callback_forwarder_del(pd->source, EFL_MODEL_EVENT_CHILDREN_COUNT_CHANGED, obj);
        efl_event_callback_forwarder_del(pd->source, EFL_MODEL_EVENT_PROPERTIES_CHANGED, obj);

        efl_unref(pd->source);
        pd->source = NULL;
     }

   efl_destructor(efl_super(obj, EFL_MODEL_COMPOSITE_CLASS));
}

static Efl_Object *
_efl_model_composite_efl_object_finalize(Eo *obj, Efl_Model_Composite_Data *pd)
{
   if (pd->source == NULL)
     {
        ERR("Source of the composite model wasn't defined at construction time.");
        return NULL;
     }

   return obj;
}

static void
_efl_model_composite_efl_ui_view_model_set(Eo *obj EINA_UNUSED, Efl_Model_Composite_Data *pd, Efl_Model *model)
{
   if (pd->source != NULL)
     {
        ERR("Source already set for composite model. It can only be set once.");
        return ;
     }
   pd->source = efl_ref(model);

   efl_event_callback_forwarder_priority_add(model, EFL_MODEL_EVENT_CHILD_ADDED, EFL_CALLBACK_PRIORITY_BEFORE, obj);
   efl_event_callback_forwarder_priority_add(model, EFL_MODEL_EVENT_CHILD_REMOVED, EFL_CALLBACK_PRIORITY_BEFORE, obj);
   efl_event_callback_forwarder_priority_add(model, EFL_MODEL_EVENT_CHILDREN_COUNT_CHANGED, EFL_CALLBACK_PRIORITY_BEFORE, obj);
   efl_event_callback_forwarder_priority_add(model, EFL_MODEL_EVENT_PROPERTIES_CHANGED, EFL_CALLBACK_PRIORITY_BEFORE, obj);
}

static Efl_Model *
_efl_model_composite_efl_ui_view_model_get(const Eo *obj EINA_UNUSED, Efl_Model_Composite_Data *pd)
{
   return pd->source;
}

static Eina_Future *
_efl_model_composite_efl_model_property_set(Eo *obj EINA_UNUSED, Efl_Model_Composite_Data *pd,
                                            const char *property, Eina_Value *value)
{
   return efl_model_property_set(pd->source, property, value);
}

static Eina_Value *
_efl_model_composite_efl_model_property_get(const Eo *obj EINA_UNUSED, Efl_Model_Composite_Data *pd,
                                            const char *property)
{
   return efl_model_property_get(pd->source, property);
}

static Eina_Iterator *
_efl_model_composite_efl_model_properties_get(const Eo *obj EINA_UNUSED, Efl_Model_Composite_Data *pd)
{
   return efl_model_properties_get(pd->source);
}

static unsigned int
_efl_model_composite_efl_model_children_count_get(const Eo *obj EINA_UNUSED, Efl_Model_Composite_Data *pd)
{
   return efl_model_children_count_get(pd->source);
}

static Eina_Future *
_efl_model_composite_efl_model_children_slice_get(Eo *obj EINA_UNUSED,
                                                  Efl_Model_Composite_Data *pd,
                                                  unsigned int start,
                                                  unsigned int count)
{
   return efl_model_children_slice_get(pd->source, start, count);
}

static Efl_Object *
_efl_model_composite_efl_model_child_add(Eo *obj EINA_UNUSED,
                                         Efl_Model_Composite_Data *pd)
{
   return efl_model_child_add(pd->source);
}

static void
_efl_model_composite_efl_model_child_del(Eo *obj EINA_UNUSED,
                                         Efl_Model_Composite_Data *pd,
                                         Efl_Object *child)
{
   efl_model_child_del(pd->source, child);
}

#include "efl_model_composite.eo.c"
