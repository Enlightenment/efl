#include "Eobj.h"
#include "simple.h"

#include "config.h"

EAPI Eobj_Op SIMPLE_BASE_ID = 0;

typedef struct
{
   Simple_Public_Data pub;
   int a;
} Private_Data;

EAPI const Eobj_Event_Description _SIG_A_CHANGED =
        EOBJ_EVENT_DESCRIPTION("a,changed", "i", "Called when a has changed.");

#define MY_CLASS SIMPLE_CLASS

static void
_a_set(Eobj *obj, void *class_data, va_list *list)
{
   Private_Data *pd = class_data;
   int a;
   a = va_arg(*list, int);
   pd->a = a;
   printf("%s %d\n", __func__, pd->a);

   eobj_event_callback_call(obj, SIG_A_CHANGED, &pd->a);
}

Eina_Bool
_cb_added(void *data, Eobj *obj, const Eobj_Event_Description *desc, void *event_info)
{
   Simple_Public_Data *pd = eobj_data_get(obj, MY_CLASS);
   const Eobj_Event_Description *cb_desc = event_info;
   (void) data;
   (void) desc;

   if (cb_desc != SIG_A_CHANGED)
      return EINA_TRUE;

   pd->cb_count++;

   printf("Added SIG_A_CHANGED callback to %p. Count: %d\n", obj, pd->cb_count);
   return EOBJ_CALLBACK_CONTINUE;
}

Eina_Bool
_cb_deled(void *data, Eobj *obj, const Eobj_Event_Description *desc, void *event_info)
{
   Simple_Public_Data *pd = eobj_data_get(obj, MY_CLASS);
   const Eobj_Event_Description *cb_desc = event_info;
   (void) data;
   (void) desc;

   if (cb_desc != SIG_A_CHANGED)
      return EINA_TRUE;

   pd->cb_count--;

   printf("Removed SIG_A_CHANGED callback from %p. Count: %d\n", obj, pd->cb_count);
   return EOBJ_CALLBACK_CONTINUE;
}

static void
_constructor(Eobj *obj, void *class_data EINA_UNUSED)
{
   eobj_constructor_super(obj);

   eobj_event_callback_add(obj, EOBJ_EV_CALLBACK_ADD, _cb_added, NULL);
   eobj_event_callback_add(obj, EOBJ_EV_CALLBACK_DEL, _cb_deled, NULL);

   eobj_do(obj, eobj_base_data_set("cb_count", (intptr_t) 0, NULL));
}

static void
_class_constructor(Eobj_Class *klass)
{
   const Eobj_Op_Func_Description func_desc[] = {
        EOBJ_OP_FUNC(SIMPLE_ID(SIMPLE_SUB_ID_A_SET), _a_set),
        EOBJ_OP_FUNC_SENTINEL
   };

   eobj_class_funcs_set(klass, func_desc);
}

static const Eobj_Op_Description op_desc[] = {
     EOBJ_OP_DESCRIPTION(SIMPLE_SUB_ID_A_SET, "i", "Set property A"),
     EOBJ_OP_DESCRIPTION_SENTINEL
};

static const Eobj_Event_Description *event_desc[] = {
     SIG_A_CHANGED,
     NULL
};

static const Eobj_Class_Description class_desc = {
     "Simple",
     EOBJ_CLASS_TYPE_REGULAR,
     EOBJ_CLASS_DESCRIPTION_OPS(&SIMPLE_BASE_ID, op_desc, SIMPLE_SUB_ID_LAST),
     event_desc,
     sizeof(Private_Data),
     _constructor,
     NULL,
     _class_constructor,
     NULL
};

EOBJ_DEFINE_CLASS(simple_class_get, &class_desc, EOBJ_BASE_CLASS, NULL);

