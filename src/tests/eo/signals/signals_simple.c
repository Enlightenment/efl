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

EAPI const Eo_Event_Description _EV_A_CHANGED =
        EO_EVENT_DESCRIPTION("a,changed");

#define MY_CLASS SIMPLE_CLASS

static void
_a_set(Eo *obj, void *class_data, int a)
{
   Private_Data *pd = class_data;
   pd->a = a;
   printf("%s %d\n", __func__, pd->a);

   eo_event_callback_call(obj, EV_A_CHANGED, &pd->a);
}

Eina_Bool
_cb_added(void *data EINA_UNUSED, const Eo_Event *event)
{
   Simple_Public_Data *pd = eo_data_scope_get(event->obj, MY_CLASS);
   const Eo_Callback_Array_Item *callback_array = event->event_info;

   if (callback_array->desc != EV_A_CHANGED)
      return EINA_TRUE;

   pd->cb_count++;

   printf("Added EV_A_CHANGED callback to %p. Count: %d\n", event->obj, pd->cb_count);
   return EO_CALLBACK_CONTINUE;
}

Eina_Bool
_cb_deled(void *data EINA_UNUSED, const Eo_Event *event)
{
   Simple_Public_Data *pd = eo_data_scope_get(event->obj, MY_CLASS);
   const Eo_Callback_Array_Item *callback_array = event->event_info;

   if (callback_array->desc != EV_A_CHANGED)
      return EINA_TRUE;

   pd->cb_count--;

   printf("Removed EV_A_CHANGED callback from %p. Count: %d\n", event->obj, pd->cb_count);
   return EO_CALLBACK_CONTINUE;
}

static Eo *
_constructor(Eo *obj, void *class_data EINA_UNUSED)
{
   obj = eo_constructor(eo_super(obj, MY_CLASS));

   eo_event_callback_add(obj, EO_BASE_EVENT_CALLBACK_ADD, _cb_added, NULL);
   eo_event_callback_add(obj, EO_BASE_EVENT_CALLBACK_DEL, _cb_deled, NULL);

   eo_key_data_set(obj, "cb_count", NULL);

   return obj;
}

EAPI EO_VOID_FUNC_BODYV(simple_a_set, _EO_EMPTY_HOOK, _EO_EMPTY_HOOK, EO_FUNC_CALL(a), int a);

static Eo_Op_Description op_descs[] = {
     EO_OP_FUNC_OVERRIDE(eo_constructor, _constructor),
     EO_OP_FUNC(simple_a_set, _a_set),
};


static const Eo_Event_Description *event_desc[] = {
     EV_A_CHANGED,
     NULL
};

static const Eo_Class_Description class_desc = {
     EO_VERSION,
     "Simple",
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(op_descs),
     event_desc,
     sizeof(Private_Data),
     NULL,
     NULL
};

EO_DEFINE_CLASS(simple_class_get, &class_desc, EO_CLASS, NULL);

