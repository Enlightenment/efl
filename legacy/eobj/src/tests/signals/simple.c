#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eo.h"
#include "simple.h"

EAPI Eo_Op SIMPLE_BASE_ID = 0;

typedef struct
{
   Simple_Public_Data pub;
   int a;
} Private_Data;

EAPI const Eo_Event_Description _EV_A_CHANGED =
        EO_EVENT_DESCRIPTION("a,changed", "Called when a has changed.");

#define MY_CLASS SIMPLE_CLASS

static void
_a_set(Eo *obj, void *class_data, va_list *list)
{
   Private_Data *pd = class_data;
   int a;
   a = va_arg(*list, int);
   pd->a = a;
   printf("%s %d\n", __func__, pd->a);

   eo_do(obj, eo_event_callback_call(EV_A_CHANGED, &pd->a, NULL));
}

Eina_Bool
_cb_added(void *data, Eo *obj, const Eo_Event_Description *desc, void *event_info)
{
   Simple_Public_Data *pd = eo_data_get(obj, MY_CLASS);
   const Eo_Event_Description *cb_desc = event_info;
   (void) data;
   (void) desc;

   if (cb_desc != EV_A_CHANGED)
      return EINA_TRUE;

   pd->cb_count++;

   printf("Added EV_A_CHANGED callback to %p. Count: %d\n", obj, pd->cb_count);
   return EO_CALLBACK_CONTINUE;
}

Eina_Bool
_cb_deled(void *data, Eo *obj, const Eo_Event_Description *desc, void *event_info)
{
   Simple_Public_Data *pd = eo_data_get(obj, MY_CLASS);
   const Eo_Event_Description *cb_desc = event_info;
   (void) data;
   (void) desc;

   if (cb_desc != EV_A_CHANGED)
      return EINA_TRUE;

   pd->cb_count--;

   printf("Removed EV_A_CHANGED callback from %p. Count: %d\n", obj, pd->cb_count);
   return EO_CALLBACK_CONTINUE;
}

static void
_constructor(Eo *obj, void *class_data EINA_UNUSED, va_list *list EINA_UNUSED)
{
   eo_do_super(obj, eo_constructor());

   eo_do(obj, eo_event_callback_add(EO_EV_CALLBACK_ADD, _cb_added, NULL));
   eo_do(obj, eo_event_callback_add(EO_EV_CALLBACK_DEL, _cb_deled, NULL));

   eo_do(obj, eo_base_data_set("cb_count", (intptr_t) 0, NULL));
}

static void
_class_constructor(Eo_Class *klass)
{
   const Eo_Op_Func_Description func_desc[] = {
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_CONSTRUCTOR), _constructor),
        EO_OP_FUNC(SIMPLE_ID(SIMPLE_SUB_ID_A_SET), _a_set),
        EO_OP_FUNC_SENTINEL
   };

   eo_class_funcs_set(klass, func_desc);
}

static const Eo_Op_Description op_desc[] = {
     EO_OP_DESCRIPTION(SIMPLE_SUB_ID_A_SET, "Set property A"),
     EO_OP_DESCRIPTION_SENTINEL
};

static const Eo_Event_Description *event_desc[] = {
     EV_A_CHANGED,
     NULL
};

static const Eo_Class_Description class_desc = {
     EO_VERSION,
     "Simple",
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(&SIMPLE_BASE_ID, op_desc, SIMPLE_SUB_ID_LAST),
     event_desc,
     sizeof(Private_Data),
     _class_constructor,
     NULL
};

EO_DEFINE_CLASS(simple_class_get, &class_desc, EO_BASE_CLASS, NULL);

