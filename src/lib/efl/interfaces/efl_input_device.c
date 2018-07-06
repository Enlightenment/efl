#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Efl.h>

#define EFL_INTERNAL_UNSTABLE
#include "efl_common_internal.h"

#define MY_CLASS EFL_INPUT_DEVICE_CLASS

/* Efl Input Device = Evas Device */

typedef struct _Child_Device_Iterator Child_Device_Iterator;

struct _Child_Device_Iterator
{
   Eina_Iterator  iterator;
   Eina_List     *list;
   Eina_Iterator *real_iterator;
   Eo            *object;
};

static Eina_Bool
_is_pointer(Efl_Input_Device_Data *pd)
{
   return (pd->klass == EFL_INPUT_DEVICE_TYPE_MOUSE ||
           pd->klass == EFL_INPUT_DEVICE_TYPE_TOUCH ||
           pd->klass == EFL_INPUT_DEVICE_TYPE_PEN ||
           pd->klass == EFL_INPUT_DEVICE_TYPE_WAND);
}

static void
_seat_pointers_update(Efl_Input_Device_Data *seat, Efl_Input_Device_Data *dev)
{
   if (seat && _is_pointer(dev))
     seat->pointer_count++;
}

EOLIAN static Efl_Object *
_efl_input_device_efl_object_constructor(Eo *obj, Efl_Input_Device_Data *pd)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   pd->eo = obj;
   return obj;
}

EOLIAN static void
_efl_input_device_efl_object_destructor(Eo *obj, Efl_Input_Device_Data *pd)
{
   pd->children = eina_list_free(pd->children);
   if (pd->klass != EFL_INPUT_DEVICE_TYPE_SEAT)
     {
        Efl_Input_Device_Data *p;
        Eo *seat;

        seat = efl_input_device_seat_get(obj);
        p = efl_data_scope_get(seat, MY_CLASS);
        if (p) p->children = eina_list_remove(p->children, obj);
     }
   efl_unref(pd->source);

   return efl_destructor(efl_super(obj, MY_CLASS));
}

EOLIAN static void
_efl_input_device_efl_object_parent_set(Eo *obj, Efl_Input_Device_Data *pd EINA_UNUSED, Eo *parent)
{
   Efl_Input_Device_Data *p;

   if (parent)
     {
        if (efl_isa(parent, MY_CLASS))
          {
             p = efl_data_scope_get(parent, MY_CLASS);
             EINA_SAFETY_ON_FALSE_RETURN(p->klass == EFL_INPUT_DEVICE_TYPE_SEAT);
             if (!eina_list_data_find(p->children, obj))
               {
                  p->children = eina_list_append(p->children, obj);
                  _seat_pointers_update(p, pd);
               }
          }
        else if(!efl_isa(parent, EFL_CANVAS_SCENE_INTERFACE))
          {
             EINA_SAFETY_ERROR("The parent of a device must be a seat or the canvas");
             return;
          }
     }
   else
     {
        Eo *old_parent = efl_parent_get(obj);
        if (old_parent && efl_isa(old_parent, MY_CLASS))
          {
             p = efl_data_scope_get(old_parent, MY_CLASS);
             p->children = eina_list_remove(p->children, obj);
             if (_is_pointer(pd))
               p->pointer_count--;
          }
     }

   efl_parent_set(efl_super(obj, MY_CLASS), parent);
}

EOLIAN static void
_efl_input_device_device_type_set(Eo *obj, Efl_Input_Device_Data *pd, Efl_Input_Device_Type klass)
{
   EINA_SAFETY_ON_TRUE_RETURN(pd->klass);
   pd->klass = klass;
   if (klass != EFL_INPUT_DEVICE_TYPE_SEAT)
     {
        Efl_Input_Device_Data *seat = efl_data_scope_get(efl_input_device_seat_get(obj), MY_CLASS);
        _seat_pointers_update(seat, pd);
     }
}

EOLIAN static Efl_Input_Device_Type
_efl_input_device_device_type_get(const Eo *obj EINA_UNUSED, Efl_Input_Device_Data *pd)
{
   return pd->klass;
}

EOLIAN static void
_efl_input_device_source_set(Eo *obj EINA_UNUSED, Efl_Input_Device_Data *pd, Efl_Input_Device *src)
{
   if (pd->source == src) return;
   efl_unref(pd->source);
   pd->source = efl_ref(src);
}

EOLIAN static Efl_Input_Device *
_efl_input_device_source_get(const Eo *obj EINA_UNUSED, Efl_Input_Device_Data *pd)
{
   return pd->source;
}

EOLIAN static void
_efl_input_device_seat_id_set(Eo *obj EINA_UNUSED, Efl_Input_Device_Data *pd, unsigned int id)
{
   EINA_SAFETY_ON_TRUE_RETURN(pd->klass != EFL_INPUT_DEVICE_TYPE_SEAT);
   pd->id = id;
}

EOLIAN static unsigned int
_efl_input_device_seat_id_get(const Eo *obj, Efl_Input_Device_Data *pd)
{
   if (pd->klass == EFL_INPUT_DEVICE_TYPE_SEAT)
     return pd->id;
   return efl_input_device_seat_id_get(efl_input_device_seat_get(obj));
}

EOLIAN static Efl_Input_Device *
_efl_input_device_seat_get(const Eo *obj, Efl_Input_Device_Data *pd)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, NULL);

   if (pd->klass == EFL_INPUT_DEVICE_TYPE_SEAT)
     return pd->eo;

   while ((obj = efl_parent_get(obj)))
     {
        if (!efl_isa(obj, MY_CLASS)) break;
        pd = efl_data_scope_get(obj, MY_CLASS);
        if (pd->klass == EFL_INPUT_DEVICE_TYPE_SEAT)
          return pd->eo;
     }

   return NULL;
}

static Eina_Bool
_child_device_iterator_next(Child_Device_Iterator *it, void **data)
{
   Eo *sub;

   if (!eina_iterator_next(it->real_iterator, (void **) &sub))
     return EINA_FALSE;

   if (data) *data = sub;
   return EINA_TRUE;
}

static Eo *
_child_device_iterator_get_container(Child_Device_Iterator *it)
{
   return it->object;
}

static void
_child_device_iterator_free(Child_Device_Iterator *it)
{
   eina_iterator_free(it->real_iterator);
   free(it);
}

EOLIAN static Eina_Iterator *
_efl_input_device_children_iterate(Eo *obj, Efl_Input_Device_Data *pd)
{
   Child_Device_Iterator *it;

   it = calloc(1, sizeof(*it));
   if (!it) return NULL;

   EINA_MAGIC_SET(&it->iterator, EINA_MAGIC_ITERATOR);

   it->list = pd->children;
   it->real_iterator = eina_list_iterator_new(it->list);
   it->iterator.version = EINA_ITERATOR_VERSION;
   it->iterator.next = FUNC_ITERATOR_NEXT(_child_device_iterator_next);
   it->iterator.get_container = FUNC_ITERATOR_GET_CONTAINER(_child_device_iterator_get_container);
   it->iterator.free = FUNC_ITERATOR_FREE(_child_device_iterator_free);
   it->object = obj;

   return &it->iterator;
}

EOLIAN static unsigned int
_efl_input_device_has_pointer_caps(Eo *obj EINA_UNUSED, Efl_Input_Device_Data *pd)
{
   if (pd->klass == EFL_INPUT_DEVICE_TYPE_SEAT)
     return pd->pointer_count;
   return _is_pointer(pd);
}

#include "interfaces/efl_input_device.eo.c"
