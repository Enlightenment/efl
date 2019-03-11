#include "evas_common_private.h"
#include "evas_private.h"

#define EFL_INTERNAL_UNSTABLE
#include "interfaces/efl_common_internal.h"

/* WARNING: This API is not used across EFL, hard to test! */

#ifdef DEBUG_UNTESTED_
// booh
#define SAFETY_CHECK(obj, klass, ...) \
   do { MAGIC_CHECK(dev, Evas_Device, 1); \
        return __VA_ARGS__; \
        MAGIC_CHECK_END(); \
   } while (0)

#else
#define SAFETY_CHECK(obj, klass, ...) \
   do { if (!obj) return __VA_ARGS__; } while (0)
#endif

/* FIXME: Ideally no work besides calling the Efl_Input_Device API
 * should be done here. But unfortunately, some knowledge of Evas is required
 * here (callbacks and canvas private data).
 */

static Eina_Bool
_is_pointer(Evas_Device_Class clas)
{
   if (clas == EVAS_DEVICE_CLASS_MOUSE ||
       clas == EVAS_DEVICE_CLASS_TOUCH ||
       clas == EVAS_DEVICE_CLASS_PEN ||
       clas == EVAS_DEVICE_CLASS_POINTER ||
       clas == EVAS_DEVICE_CLASS_WAND)
     return EINA_TRUE;
   return EINA_FALSE;
}

static Evas_Device *
_new_default_device_find(Evas_Public_Data *e, Evas_Device *old_dev)
{
   Eina_List *l;
   Evas_Device *dev, *def, *old_parent;
   Evas_Device_Class old_class;

   if (e->cleanup) return NULL;
   old_class = efl_input_device_type_get(old_dev);
   old_parent = efl_parent_get(old_dev);
   def = NULL;

   EINA_LIST_FOREACH(e->devices, l, dev)
     {
        if ((Evas_Device_Class)efl_input_device_type_get(dev) != old_class)
          continue;

        def = dev;
        //Prefer devices with the same parent.
        if (efl_parent_get(dev) == old_parent)
          break;
     }

   if (!def)
     {
        const char *class_str;
        if (old_class == EVAS_DEVICE_CLASS_SEAT)
          class_str = "seat";
        else if (old_class == EVAS_DEVICE_CLASS_KEYBOARD)
          class_str = "keyboard";
        else
          class_str = "mouse";
        WRN("Could not find a default %s device.", class_str);
     }
   return def;
}

static void
_del_cb(void *data, const Efl_Event *ev)
{
   Efl_Input_Device_Type devtype;
   Evas_Public_Data *e = data;

   e->devices_modified = EINA_TRUE;
   // can not be done in std destructor
   e->devices = eina_list_remove(e->devices, ev->object);

   if (e->default_seat == ev->object)
     e->default_seat = _new_default_device_find(e, ev->object);
   else if (e->default_mouse == ev->object)
     e->default_mouse = _new_default_device_find(e, ev->object);
   else if (e->default_keyboard == ev->object)
     e->default_keyboard = _new_default_device_find(e, ev->object);

   devtype = efl_input_device_type_get(ev->object);
   if ((devtype == EFL_INPUT_DEVICE_TYPE_SEAT) && (!e->default_seat))
     {
        Evas_Pointer_Data *pdata = _evas_pointer_data_by_device_get(e, ev->object);
        if (pdata)
          {
             Evas_Pointer_Seat *pseat;

             EINA_INLIST_FOREACH(e->seats, pseat)
               {
                  /* store to dummy seat data for when seat reattaches */
                  if (pseat->seat) continue;
                  pseat->x = pdata->seat->x;
                  pseat->y = pdata->seat->y;
                  pseat->inside = pdata->seat->inside;
                  break;
               }
          }
     }

   if (devtype == EFL_INPUT_DEVICE_TYPE_MOUSE)
     _evas_pointer_data_remove(e, ev->object);
   eina_hash_del_by_key(e->locks.masks, &ev->object);
   eina_hash_del_by_key(e->modifiers.masks, &ev->object);
   efl_event_callback_call(e->evas, EFL_CANVAS_SCENE_EVENT_DEVICE_REMOVED,
                           ev->object);
}

EOLIAN Efl_Input_Device *
_evas_canvas_efl_canvas_scene_device_get(Evas *eo_e EINA_UNUSED, Evas_Public_Data *e, const char *name)
{
   const char *dev_name;
   Evas_Device *dev;
   Eina_List *l;

   if (!name) return NULL;

   EINA_LIST_FOREACH(e->devices, l, dev)
     {
        dev_name = efl_name_get(dev);

        if (eina_streq(dev_name, name))
          return dev;
     }

   return NULL;
}

EAPI Evas_Device *
evas_device_get(Evas *eo_e, const char *name)
{
   return efl_canvas_scene_device_get(eo_e, name);
}

EOLIAN Efl_Input_Device *
_evas_canvas_efl_canvas_scene_seat_default_get(Evas *eo_e EINA_UNUSED, Evas_Public_Data *e)
{
   return e->default_seat;
}

EOLIAN Efl_Input_Device *
_evas_canvas_efl_canvas_scene_seat_get(Evas *eo_e EINA_UNUSED, Evas_Public_Data *e, unsigned int id)
{
   Evas_Device *dev;
   Eina_List *l;

   EINA_LIST_FOREACH(e->devices, l, dev)
     {
        if (efl_input_device_type_get(dev) != EFL_INPUT_DEVICE_TYPE_SEAT)
          continue;

        if (efl_input_device_seat_id_get(dev) == id)
          return dev;
     }

   return NULL;
}

EAPI Evas_Device *
evas_device_get_by_seat_id(Evas *eo_e, unsigned int id)
{
   return efl_canvas_scene_seat_get(eo_e, id);
}

EAPI Evas_Device *
evas_device_add(Evas *eo_e)
{
   return evas_device_add_full(eo_e, NULL, NULL, NULL, NULL,
                               EVAS_DEVICE_CLASS_NONE,
                               EVAS_DEVICE_SUBCLASS_NONE);
}

EAPI Evas_Device *
evas_device_add_full(Evas *eo_e, const char *name, const char *desc,
                     Evas_Device *parent_dev, Evas_Device *emulation_dev,
                     Evas_Device_Class clas, Evas_Device_Subclass sub_clas)
{
   Efl_Input_Device_Data *d;
   Evas_Public_Data *e;
   Evas_Device *dev;

   SAFETY_CHECK(eo_e, EVAS_CANVAS_CLASS, NULL);

   dev = efl_add_ref(EFL_INPUT_DEVICE_CLASS, parent_dev ?: eo_e,
                     efl_name_set(efl_added, name),
                     efl_comment_set(efl_added, desc),
                     efl_input_device_type_set(efl_added, clas),
                     efl_input_device_source_set(efl_added, emulation_dev));

   d = efl_data_scope_get(dev, EFL_INPUT_DEVICE_CLASS);
   d->evas = eo_e;

   // Legacy support, subclass is most likely unused
   d->subclass = (unsigned) sub_clas;

   e = efl_data_scope_get(eo_e, EVAS_CANVAS_CLASS);

   /* This is the case when the user is using wayland backend,
      since evas itself will not create the devices we must set them here. */
   if (!e->default_seat && clas == EVAS_DEVICE_CLASS_SEAT)
     e->default_seat = dev;
   else if (!e->default_keyboard && clas == EVAS_DEVICE_CLASS_KEYBOARD)
     e->default_keyboard = dev;
   else if (_is_pointer(clas))
     {
        Evas_Pointer_Data *pdata = _evas_pointer_data_add(e, dev);
        if (!pdata)
          {
             efl_del(dev);
             return NULL;
          }

        if (e->default_mouse)
          {
             if ((clas == EVAS_DEVICE_CLASS_MOUSE) &&
                 (parent_dev == e->default_seat) &&
                 (evas_device_class_get(e->default_mouse) != EVAS_DEVICE_CLASS_MOUSE))
               {
                  Eina_Bool inside = pdata->seat->inside;

                  if (inside)
                    evas_event_feed_mouse_out(eo_e, 0, NULL);
                  e->default_mouse = dev;
                  if (inside)
                    evas_event_feed_mouse_in(eo_e, 0, NULL);
               }
          }
        else
          {
             Evas_Pointer_Seat *pseat;

             EINA_INLIST_FOREACH(e->seats, pseat)
               if (!pseat->pointers) break;
             e->default_mouse = dev;
             if (pseat)
               {
                  if (pseat->inside)
                    evas_event_feed_mouse_in(eo_e, 0, NULL);
                  evas_event_feed_mouse_move(eo_e, pseat->x, pseat->y, 0, NULL);
               }
          }
     }

   e->devices = eina_list_append(e->devices, dev);
   efl_event_callback_add(dev, EFL_EVENT_DEL, _del_cb, e);

   efl_event_callback_call(eo_e, EFL_CANVAS_SCENE_EVENT_DEVICE_ADDED, dev);
   // Keeping this event to do not break things...
   evas_event_callback_call(eo_e, EVAS_CALLBACK_DEVICE_CHANGED, dev);
   if (e->pending_default_focus_obj && (e->default_seat == dev))
     {
        Eo *eo_obj = e->pending_default_focus_obj;
        e->pending_default_focus_obj = NULL;
        evas_object_focus_set(eo_obj, 1);
     }

   return dev;
}

EAPI void
evas_device_del(Evas_Device *dev)
{
   SAFETY_CHECK(dev, EFL_INPUT_DEVICE_CLASS);
   if (!efl_invalidated_get(dev))
     efl_del(dev);
   efl_unref(dev);
}

EAPI void
evas_device_push(Evas *eo_e, Evas_Device *dev)
{
   SAFETY_CHECK(eo_e, EVAS_CANVAS_CLASS);
   SAFETY_CHECK(dev, EFL_INPUT_DEVICE_CLASS);

   Evas_Public_Data *e = efl_data_scope_get(eo_e, EVAS_CANVAS_CLASS);
   if (!e->cur_device)
     {
        e->cur_device = eina_array_new(4);
        if (!e->cur_device) return;
     }
   efl_ref(dev);
   eina_array_push(e->cur_device, dev);
}

EAPI void
evas_device_pop(Evas *eo_e)
{
   Evas_Device *dev;

   SAFETY_CHECK(eo_e, EVAS_CANVAS_CLASS);

   Evas_Public_Data *e = efl_data_scope_get(eo_e, EVAS_CANVAS_CLASS);
   dev = eina_array_pop(e->cur_device);
   if (dev) efl_unref(dev);
}

EAPI const Eina_List *
evas_device_list(Evas *eo_e, const Evas_Device *dev)
{
   SAFETY_CHECK(eo_e, EVAS_CANVAS_CLASS, NULL);

   if (dev)
     {
        SAFETY_CHECK(dev, EFL_INPUT_DEVICE_CLASS, NULL);

        Efl_Input_Device_Data *d = efl_data_scope_get(dev, EFL_INPUT_DEVICE_CLASS);
        return d->children;
     }

   Evas_Public_Data *e = efl_data_scope_get(eo_e, EVAS_CANVAS_CLASS);
   return e->devices;
}

EAPI void
evas_device_name_set(Evas_Device *dev, const char *name)
{
   SAFETY_CHECK(dev, EFL_INPUT_DEVICE_CLASS);

   Efl_Input_Device_Data *d = efl_data_scope_get(dev, EFL_INPUT_DEVICE_CLASS);

   efl_name_set(dev, name);
   evas_event_callback_call(d->evas, EVAS_CALLBACK_DEVICE_CHANGED, dev);
}

EAPI const char *
evas_device_name_get(const Evas_Device *dev)
{
   return efl_name_get(dev);
}

EAPI void
evas_device_description_set(Evas_Device *dev, const char *desc)
{
   SAFETY_CHECK(dev, EFL_INPUT_DEVICE_CLASS);

   efl_comment_set(dev, desc);

   Efl_Input_Device_Data *d = efl_data_scope_get(dev, EFL_INPUT_DEVICE_CLASS);
   evas_event_callback_call(d->evas, EVAS_CALLBACK_DEVICE_CHANGED, dev);
}

EAPI const char *
evas_device_description_get(const Evas_Device *dev)
{
   return efl_comment_get(dev);
}

EAPI void
evas_device_parent_set(Evas_Device *dev, Evas_Device *parent)
{
   // Note: This function should be deprecated. parent_set doesn't make sense
   // unless the parent is a seat device. Parent shouldn't be changed after
   // creation.

   SAFETY_CHECK(dev, EFL_INPUT_DEVICE_CLASS);

   Efl_Input_Device_Data *d = efl_data_scope_get(dev, EFL_INPUT_DEVICE_CLASS);
   if (parent)
     {
        SAFETY_CHECK(parent, EFL_INPUT_DEVICE_CLASS);
     }
   else if (efl_parent_get(dev))
     {
        efl_ref(dev);
     }

   efl_parent_set(dev, parent);
   evas_event_callback_call(d->evas, EVAS_CALLBACK_DEVICE_CHANGED, dev);
}

EAPI const Evas_Device *
evas_device_parent_get(const Evas_Device *dev)
{
   Eo *parent = efl_parent_get(dev);

   if (!efl_isa(parent, EFL_INPUT_DEVICE_CLASS))
     return NULL;

   return parent;
}

EAPI void
evas_device_class_set(Evas_Device *dev, Evas_Device_Class clas)
{
   SAFETY_CHECK(dev, EFL_INPUT_DEVICE_CLASS);
   EINA_SAFETY_ON_TRUE_RETURN(efl_finalized_get(dev));

   Efl_Input_Device_Data *d = efl_data_scope_get(dev, EFL_INPUT_DEVICE_CLASS);
   Evas_Public_Data *edata = efl_data_scope_get(d->evas, EVAS_CANVAS_CLASS);

   if ((Evas_Device_Class)d->klass == clas)
     return;

   if (_is_pointer(d->klass))
     _evas_pointer_data_remove(edata, dev);

   efl_input_device_type_set(dev, clas);

   if (_is_pointer(clas))
     _evas_pointer_data_add(edata, dev);

   evas_event_callback_call(d->evas, EVAS_CALLBACK_DEVICE_CHANGED, dev);
}

EAPI Evas_Device_Class
evas_device_class_get(const Evas_Device *dev)
{
   return efl_input_device_type_get(dev);
}

EAPI void
evas_device_subclass_set(Evas_Device *dev, Evas_Device_Subclass clas)
{
   SAFETY_CHECK(dev, EFL_INPUT_DEVICE_CLASS);
   Efl_Input_Device_Data *d = efl_data_scope_get(dev, EFL_INPUT_DEVICE_CLASS);

   d->subclass = (unsigned) clas;
   evas_event_callback_call(d->evas, EVAS_CALLBACK_DEVICE_CHANGED, dev);
}

EAPI Evas_Device_Subclass
evas_device_subclass_get(const Evas_Device *dev)
{
   SAFETY_CHECK(dev, EFL_INPUT_DEVICE_CLASS, EVAS_DEVICE_SUBCLASS_NONE);
   Efl_Input_Device_Data *d = efl_data_scope_get(dev, EFL_INPUT_DEVICE_CLASS);

   return (Evas_Device_Subclass) d->subclass;
}

EAPI void
evas_device_emulation_source_set(Evas_Device *dev, Evas_Device *src)
{
   SAFETY_CHECK(dev, EFL_INPUT_DEVICE_CLASS);
   Efl_Input_Device_Data *d = efl_data_scope_get(dev, EFL_INPUT_DEVICE_CLASS);

   efl_input_device_source_set(dev, src);
   evas_event_callback_call(d->evas, EVAS_CALLBACK_DEVICE_CHANGED, dev);
}

EAPI const Evas_Device *
evas_device_emulation_source_get(const Evas_Device *dev)
{
   return efl_input_device_source_get(dev);
}

EAPI void
evas_device_seat_id_set(Evas_Device *dev, unsigned int id)
{
   efl_input_device_seat_id_set(dev, id);
}

EAPI unsigned int
evas_device_seat_id_get(const Evas_Device *dev)
{
   return efl_input_device_seat_id_get(dev);
}

void
_evas_device_cleanup(Evas *eo_e)
{
   Eina_List *cpy, *deleted = NULL;
   Evas_Device *dev;
   Evas_Public_Data *e = efl_data_scope_get(eo_e, EVAS_CANVAS_CLASS);

   if (e->cur_device)
     {
        while ((dev = eina_array_pop(e->cur_device)))
          efl_unref(dev);
        eina_array_free(e->cur_device);
        e->cur_device = NULL;
     }

   /* If the device is deleted, _del_cb will remove the device
    * from the devices list. Ensure we delete them only once, and only if this
    * Evas is the owner, otherwise we would kill external references (eg.
    * from efl_duplicate()). */
again:
   e->devices_modified = EINA_FALSE;
   cpy = eina_list_clone(e->devices);
   EINA_LIST_FREE(cpy, dev)
     {
        if (!eina_list_data_find(deleted, dev) && (efl_parent_get(dev) == eo_e))
          {
             evas_device_del(dev);
             deleted = eina_list_append(deleted, dev);
             if (e->devices_modified)
               {
                  eina_list_free(cpy);
                  goto again;
               }
          }
     }
   eina_list_free(deleted);

   /* Not all devices were deleted. The user probably will unref them later.
      Since Evas will be deleted, remove the del callback from them and
      tell the user that the device was removed.
   */
   EINA_LIST_FREE(e->devices, dev)
     {
        efl_event_callback_call(e->evas, EFL_CANVAS_SCENE_EVENT_DEVICE_REMOVED, dev);
        efl_event_callback_del(dev, EFL_EVENT_DEL, _del_cb, e);
     }
}

Evas_Device *
_evas_device_top_get(const Evas *eo_e)
{
   int num;
   
   Evas_Public_Data *e = efl_data_scope_get(eo_e, EVAS_CANVAS_CLASS);
   if (!e->cur_device) return NULL;
   num = eina_array_count(e->cur_device);
   if (num < 1) return NULL;
   return eina_array_data_get(e->cur_device, num - 1);
}

EOLIAN Eina_Bool
_evas_canvas_efl_canvas_scene_pointer_position_get(const Eo *eo_e, Evas_Public_Data *e, Efl_Input_Device *seat, Eina_Position2D *pos)
{
   Eina_Iterator *it;
   Eo *child;

   if (pos) *pos = EINA_POSITION2D(0, 0);
   if (!e->default_seat) return EINA_FALSE;
   if (!seat)
     {
        evas_pointer_canvas_xy_get(eo_e, &pos->x, &pos->y);
        return EINA_TRUE;
     }
   it = efl_input_device_children_iterate(seat);
   EINA_SAFETY_ON_NULL_RETURN_VAL(it, EINA_FALSE);

   EINA_ITERATOR_FOREACH(it, child)
     if (_is_pointer(efl_input_device_type_get(child)))
       break;
   if (child)
     *pos = efl_input_pointer_position_get(child);
   eina_iterator_free(it);
   return !!child;
}
