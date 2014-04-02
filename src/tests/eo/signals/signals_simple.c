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
        EO_EVENT_DESCRIPTION("a,changed", "Called when a has changed.");

#define MY_CLASS SIMPLE_CLASS

static void
_a_set(Eo *obj, void *class_data, int a)
{
   Private_Data *pd = class_data;
   pd->a = a;
   printf("%s %d\n", __func__, pd->a);

   eo_do(obj, eo_event_callback_call(EV_A_CHANGED, &pd->a));
}

Eina_Bool
_cb_added(void *data, Eo *obj, const Eo_Event_Description *desc, void *event_info)
{
   Simple_Public_Data *pd = eo_data_scope_get(obj, MY_CLASS);
   const Eo_Callback_Array_Item *callback_array = event_info;
   (void) data;
   (void) desc;

   if (callback_array->desc != EV_A_CHANGED)
      return EINA_TRUE;

   pd->cb_count++;

   printf("Added EV_A_CHANGED callback to %p. Count: %d\n", obj, pd->cb_count);
   return EO_CALLBACK_CONTINUE;
}

Eina_Bool
_cb_deled(void *data, Eo *obj, const Eo_Event_Description *desc, void *event_info)
{
   Simple_Public_Data *pd = eo_data_scope_get(obj, MY_CLASS);
   const Eo_Callback_Array_Item *callback_array = event_info;
   (void) data;
   (void) desc;

   if (callback_array->desc != EV_A_CHANGED)
      return EINA_TRUE;

   pd->cb_count--;

   printf("Removed EV_A_CHANGED callback from %p. Count: %d\n", obj, pd->cb_count);
   return EO_CALLBACK_CONTINUE;
}

static void
_constructor(Eo *obj, void *class_data EINA_UNUSED)
{
   eo_do_super(obj, MY_CLASS, eo_constructor());

   eo_do(obj, eo_event_callback_add(EO_EV_CALLBACK_ADD, _cb_added, NULL));
   eo_do(obj, eo_event_callback_add(EO_EV_CALLBACK_DEL, _cb_deled, NULL));

   eo_do(obj, eo_key_data_set("cb_count", (intptr_t) 0, NULL));
}

EAPI EO_VOID_FUNC_BODYV(simple_a_set, EO_FUNC_CALL(a), int a);

static Eo_Op_Description op_descs[] = {
     EO_OP_FUNC_OVERRIDE(eo_constructor, _constructor),
     EO_OP_FUNC(simple_a_set, _a_set, "Set property a"),
     EO_OP_SENTINEL
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

EO_DEFINE_CLASS(simple_class_get, &class_desc, EO_BASE_CLASS, NULL);

