#include "libefl_mono_native_test.h"

typedef struct Dummy_Part_Holder_Data
{
  Eo *one;
  Eo *two;
} Dummy_Part_Holder_Data;

void part_deleted_cb(void *data, const Efl_Event *evt)
{
   Dummy_Part_Holder_Data *pd = data;

   if (evt->object == pd->one)
     pd->one = NULL;
   else if (evt->object == pd->two)
     pd->two = NULL;
}

// Part holder
static Efl_Object*
_dummy_part_holder_efl_object_constructor(Eo *obj, Dummy_Part_Holder_Data *pd)
{
   efl_constructor(efl_super(obj, DUMMY_PART_HOLDER_CLASS));

   // To avoid an infinite loop calling the same constructor
   if (!efl_parent_get(obj))
     {
        pd->one = efl_add(DUMMY_TEST_OBJECT_CLASS, obj, efl_name_set(efl_added, "part_one"));
        efl_event_callback_add(pd->one, EFL_EVENT_DEL, part_deleted_cb, pd);
        pd->two = efl_add(DUMMY_TEST_OBJECT_CLASS, obj, efl_name_set(efl_added, "part_two"));
        efl_event_callback_add(pd->two, EFL_EVENT_DEL, part_deleted_cb, pd);


     }

   return obj;
}

static void
_dummy_part_holder_efl_object_destructor(EINA_UNUSED Eo* obj, Dummy_Part_Holder_Data *pd)
{
   if (pd->one)
     efl_parent_set(pd->one, NULL);
   if (pd->two)
     efl_parent_set(pd->two, NULL);
}

Efl_Object *_dummy_part_holder_efl_part_part_get(EINA_UNUSED const Eo *obj, Dummy_Part_Holder_Data *pd, const char *name)
{
    if (!strcmp(name, "one"))
      return pd->one;
    else if (!strcmp(name, "two"))
      return pd->two;
    else
      return NULL;
}

#include "dummy_part_holder.eo.c"
