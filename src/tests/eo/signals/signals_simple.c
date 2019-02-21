#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eo.h"
#include "signals_simple.h"

typedef struct
{
   Simple_Public_Data pub;
   int a;
} Private_Data;

EAPI const Efl_Event_Description _EV_A_CHANGED =
        EFL_EVENT_DESCRIPTION("a,changed");
EAPI const Efl_Event_Description _EV_RESTART =
        EFL_EVENT_DESCRIPTION_RESTART("restart");

#define MY_CLASS SIMPLE_CLASS

static void
_a_set(Eo *obj, void *class_data, int a)
{
   Private_Data *pd = class_data;
   pd->a = a;
   printf("%s %d\n", __func__, pd->a);

   efl_event_callback_legacy_call(obj, EV_A_CHANGED, &pd->a);
}

void
_cb_added(void *data EINA_UNUSED, const Efl_Event *event)
{
   Simple_Public_Data *pd = efl_data_scope_get(event->object, MY_CLASS);
   const Efl_Callback_Array_Item_Full *callback_array = event->info;

   if (callback_array->desc != EV_A_CHANGED)
      return;

   pd->cb_count++;

   printf("Added EV_A_CHANGED callback to %p. Count: %d\n", event->object, pd->cb_count);
}

void
_cb_deled(void *data EINA_UNUSED, const Efl_Event *event)
{
   Simple_Public_Data *pd = efl_data_scope_get(event->object, MY_CLASS);
   const Efl_Callback_Array_Item_Full *callback_array = event->info;

   if (callback_array->desc != EV_A_CHANGED)
      return;

   pd->cb_count--;

   printf("Removed EV_A_CHANGED callback from %p. Count: %d\n", event->object, pd->cb_count);
}

static Eo *
_constructor(Eo *obj, void *class_data EINA_UNUSED)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));

   efl_event_callback_add(obj, EFL_EVENT_CALLBACK_ADD, _cb_added, NULL);
   efl_event_callback_add(obj, EFL_EVENT_CALLBACK_DEL, _cb_deled, NULL);

   efl_key_data_set(obj, "cb_count", NULL);

   return obj;
}

EAPI EFL_VOID_FUNC_BODYV(simple_a_set, EFL_FUNC_CALL(a), int a);

static Eina_Bool
_class_initializer(Efl_Class *klass)
{
   EFL_OPS_DEFINE(ops,
         EFL_OBJECT_OP_FUNC(efl_constructor, _constructor),
         EFL_OBJECT_OP_FUNC(simple_a_set, _a_set),
   );

   return efl_class_functions_set(klass, &ops, NULL);
}

static const Efl_Class_Description class_desc = {
     EO_VERSION,
     "Simple",
     EFL_CLASS_TYPE_REGULAR,
     sizeof(Private_Data),
     _class_initializer,
     NULL,
     NULL
};

EFL_DEFINE_CLASS(simple_class_get, &class_desc, EO_CLASS, NULL);
