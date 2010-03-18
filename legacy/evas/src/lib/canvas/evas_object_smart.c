#include "evas_common.h"
#include "evas_private.h"

typedef struct _Evas_Object_Smart      Evas_Object_Smart;
typedef struct _Evas_Smart_Callback    Evas_Smart_Callback;

struct _Evas_Object_Smart
{
   DATA32            magic;
   void             *engine_data;
   void             *data;
   Eina_List        *callbacks;
   Eina_Inlist *contained;
   Evas_Smart_Cb_Description_Array callbacks_descriptions;
   int               walking_list;
   Eina_Bool         deletions_waiting : 1;
   Eina_Bool         need_recalculate : 1;
};

struct _Evas_Smart_Callback
{
   const char *event;
   void (*func) (void *data, Evas_Object *obj, void *event_info);
   void *func_data;
   char  delete_me : 1;
};

/* private methods for smart objects */
static void evas_object_smart_callbacks_clear(Evas_Object *obj);
static void evas_object_smart_init(Evas_Object *obj);
static void *evas_object_smart_new(void);
static void evas_object_smart_render(Evas_Object *obj, void *output, void *context, void *surface, int x, int y);
static void evas_object_smart_free(Evas_Object *obj);
static void evas_object_smart_render_pre(Evas_Object *obj);
static void evas_object_smart_render_post(Evas_Object *obj);

static unsigned int evas_object_smart_id_get(Evas_Object *obj);
static unsigned int evas_object_smart_visual_id_get(Evas_Object *obj);
static void *evas_object_smart_engine_data_get(Evas_Object *obj);

static const Evas_Object_Func object_func =
{
   /* methods (compulsory) */
   evas_object_smart_free,
     evas_object_smart_render,
     evas_object_smart_render_pre,
     evas_object_smart_render_post,
     evas_object_smart_id_get,
     evas_object_smart_visual_id_get,
     evas_object_smart_engine_data_get,
     /* these are optional. NULL = nothing */
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL
};

/* public funcs */
/**
 * Store a pointer to user data for a smart object.
 *
 * @param obj The smart object
 * @param data A pointer to user data
 * @ingroup Evas_Smart_Object_Group
 */
EAPI void
evas_object_smart_data_set(Evas_Object *obj, void *data)
{
   Evas_Object_Smart *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Smart *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Smart, MAGIC_OBJ_SMART);
   return;
   MAGIC_CHECK_END();
   o->data = data;
}

/**
 * Retrieve user data stored on a smart object.
 *
 * @param obj The smart object
 * @return A pointer to data stored using evas_object_smart_data_set(), or
 *         NULL if none has been set.
 * @ingroup Evas_Smart_Object_Group
 */
EAPI void *
evas_object_smart_data_get(const Evas_Object *obj)
{
   Evas_Object_Smart *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   o = (Evas_Object_Smart *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Smart, MAGIC_OBJ_SMART);
   return NULL;
   MAGIC_CHECK_END();
   return o->data;
}

/**
 * Get the Evas_Smart from which @p obj was created.
 *
 * @param obj a smart object
 * @return the Evas_Smart
 * @ingroup Evas_Smart_Object_Group
 */
EAPI Evas_Smart *
evas_object_smart_smart_get(const Evas_Object *obj)
{
   Evas_Object_Smart *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   o = (Evas_Object_Smart *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Smart, MAGIC_OBJ_SMART);
   return NULL;
   MAGIC_CHECK_END();
   return obj->smart.smart;
}

/**
 * Set an evas object as a member of a smart object.
 *
 * @param obj The member object
 * @param smart_obj The smart object
 *
 * Members will automatically be stacked and layered with the smart object.
 * The various stacking function will operate on members relative to the
 * other members instead of the entire canvas.
 *
 * Non-member objects can not interleave a smart object's members.
 *
 * @ingroup Evas_Smart_Object_Group
 */
EAPI void
evas_object_smart_member_add(Evas_Object *obj, Evas_Object *smart_obj)
{
   Evas_Object_Smart *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   MAGIC_CHECK(smart_obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Smart *)(smart_obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Smart, MAGIC_OBJ_SMART);
   return;
   MAGIC_CHECK_END();

   if (obj->delete_me)
     {
        CRIT("Adding deleted object %p to smart obj %p", obj, smart_obj);
	abort();
	return;
     }
   if (smart_obj->delete_me)
     {
	CRIT("Adding object %p to deleted smart obj %p", obj, smart_obj);
	abort();
	return;
     }
   if (!smart_obj->layer)
     {
	CRIT("No evas surface associated with smart object (%p)", smart_obj);
	abort();
	return;
     }
   if (obj->layer && smart_obj->layer
       && obj->layer->evas != smart_obj->layer->evas)
     {
	CRIT("Adding object %p from Evas (%p) from another Evas (%p)", obj, obj->layer->evas, smart_obj->layer->evas);
	abort();
	return;
     }

   if (obj->smart.parent == smart_obj) return;

   if (obj->smart.parent) evas_object_smart_member_del(obj);

   evas_object_release(obj, 1);
   obj->layer = smart_obj->layer;
   obj->cur.layer = obj->layer->layer;
   obj->layer->usage++;
   obj->smart.parent = smart_obj;
   o->contained = eina_inlist_append(o->contained, EINA_INLIST_GET(obj));
   evas_object_smart_member_cache_invalidate(obj);
   obj->restack = 1;
   evas_object_change(obj);
   if (smart_obj->smart.smart->smart_class->member_add)
     smart_obj->smart.smart->smart_class->member_add(smart_obj, obj);
}

/**
 * Removes a member object from a smart object.
 *
 * @param obj the member object
 * @ingroup Evas_Smart_Object_Group
 *
 * This removes a member object from a smart object. The object will still
 * be on the canvas, but no longer associated with whichever smart object
 * it was associated with.
 *
 */
EAPI void
evas_object_smart_member_del(Evas_Object *obj)
{
   Evas_Object_Smart *o;
   Evas_Object *smart_obj;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();

   if (!obj->smart.parent) return;

   smart_obj = obj->smart.parent;
   if (smart_obj->smart.smart->smart_class->member_del)
     smart_obj->smart.smart->smart_class->member_del(smart_obj, obj);

   o = (Evas_Object_Smart *)(obj->smart.parent->object_data);
   o->contained = eina_inlist_remove(o->contained, EINA_INLIST_GET(obj));
   obj->smart.parent = NULL;
   evas_object_smart_member_cache_invalidate(obj);
   obj->layer->usage--;
   obj->cur.layer = obj->layer->layer;
   evas_object_inject(obj, obj->layer->evas);
   obj->restack = 1;
   evas_object_change(obj);
}

/**
 * Gets the smart parent of an Evas_Object
 * @param obj the Evas_Object you want to get the parent
 * @return Returns the smart parent of @a obj, or NULL if @a obj is not a smart member of another Evas_Object
 * @ingroup Evas_Smart_Object_Group
 */
EAPI Evas_Object *
evas_object_smart_parent_get(const Evas_Object *obj)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();

   return obj->smart.parent;
}

/**
 * Gets the list of the member objects of an Evas_Object
 * @param obj the Evas_Object you want to get the list of member objects
 * @return Returns the list of the member objects of @a obj.
 * The returned list should be freed with eina_list_free() when you no longer need it
 */
EAPI Eina_List *
evas_object_smart_members_get(const Evas_Object *obj)
{
   Evas_Object_Smart *o;
   Eina_List *members;
   Eina_Inlist *member;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   o = (Evas_Object_Smart *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Smart, MAGIC_OBJ_SMART);
   return NULL;
   MAGIC_CHECK_END();

   members = NULL;
   for (member = o->contained; member; member = member->next)
      members = eina_list_append(members, member);

   return members;
}

const Eina_Inlist *
evas_object_smart_members_get_direct(const Evas_Object *obj)
{
   Evas_Object_Smart *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   o = (Evas_Object_Smart *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Smart, MAGIC_OBJ_SMART);
   return NULL;
   MAGIC_CHECK_END();
   return o->contained;
}

/**
 * Instantiates a new smart object described by @p s.
 *
 * @param e the evas on which to add the object
 * @param s the Evas_Smart describing the smart object
 * @return a new Evas_Object
 * @ingroup Evas_Smart_Object_Group
 */
EAPI Evas_Object *
evas_object_smart_add(Evas *e, Evas_Smart *s)
{
   Evas_Object *obj;

   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return NULL;
   MAGIC_CHECK_END();
   MAGIC_CHECK(s, Evas_Smart, MAGIC_SMART);
   return NULL;
   MAGIC_CHECK_END();

   obj = evas_object_new();
   if (!obj) return NULL;
   obj->smart.smart = s;
   obj->type = s->smart_class->name;
   evas_object_smart_init(obj);
   evas_object_inject(obj, e);

   evas_object_smart_use(s);

   if (s->smart_class->add) s->smart_class->add(obj);

   return obj;
}

/**
 * Add a callback for the smart event specified by @p event.
 *
 * @param obj a smart object
 * @param event the event name
 * @param func the callback function
 * @param data user data to be passed to the callback function
 * @ingroup Evas_Smart_Object_Group
 */
EAPI void
evas_object_smart_callback_add(Evas_Object *obj, const char *event, void (*func) (void *data, Evas_Object *obj, void *event_info), const void *data)
{
   Evas_Object_Smart *o;
   Evas_Smart_Callback *cb;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Smart *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Smart, MAGIC_OBJ_SMART);
   return;
   MAGIC_CHECK_END();
   if (!event) return;
   if (!func) return;
   cb = calloc(1, sizeof(Evas_Smart_Callback));
   cb->event = eina_stringshare_add(event);
   cb->func = func;
   cb->func_data = (void *)data;
   o->callbacks = eina_list_prepend(o->callbacks, cb);
}

/**
 * Remove a smart callback
 *
 * Removes a callback that was added by evas_object_smart_callback_add()
 *
 * @param obj a smart object
 * @param event the event name
 * @param func the callback function
 * @return the data pointer
 * @ingroup Evas_Smart_Object_Group
 */
EAPI void *
evas_object_smart_callback_del(Evas_Object *obj, const char *event, void (*func) (void *data, Evas_Object *obj, void *event_info))
{
   Evas_Object_Smart *o;
   Eina_List *l;
   Evas_Smart_Callback *cb;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   o = (Evas_Object_Smart *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Smart, MAGIC_OBJ_SMART);
   return NULL;
   MAGIC_CHECK_END();
   if (!event) return NULL;
   EINA_LIST_FOREACH(o->callbacks, l, cb)
     {
	if ((!strcmp(cb->event, event)) && (cb->func == func))
	  {
	     void *data;

	     data = cb->func_data;
	     cb->delete_me = 1;
	     o->deletions_waiting = 1;
	     evas_object_smart_callbacks_clear(obj);
	     return data;
	  }
     }
   return NULL;
}

/**
 * Call any smart callbacks on @p obj for @p event.
 *
 * @param obj the smart object
 * @param event the event name
 * @param event_info an event specific struct of info to pass to the callback
 *
 * This should be called internally in the smart object when some specific
 * event has occured. The documentation for the smart object should include
 * a list of possible events and what type of @p event_info to expect.
 *
 * @ingroup Evas_Smart_Object_Group
 */
EAPI void
evas_object_smart_callback_call(Evas_Object *obj, const char *event, void *event_info)
{
   Evas_Object_Smart *o;
   Eina_List *l;
   Evas_Smart_Callback *cb;
   const char *strshare;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Smart *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Smart, MAGIC_OBJ_SMART);
   return;
   MAGIC_CHECK_END();
   if (!event) return;
   if (obj->delete_me) return;
   o->walking_list++;
   strshare = eina_stringshare_add(event);
   EINA_LIST_FOREACH(o->callbacks, l, cb)
     {
	if (!cb->delete_me)
	  {
	     if (cb->event == strshare)
	       cb->func(cb->func_data, obj, event_info);
	  }
	if (obj->delete_me)
	  break;
     }
   eina_stringshare_del(strshare);
   o->walking_list--;
   evas_object_smart_callbacks_clear(obj);
}

/**
 * Set smart object instance callbacks descriptions.
 *
 * These descriptions are hints to be used by introspection and are
 * not enforced in any way.
 *
 * It will not be checked if instance callbacks descriptions have the
 * same name as another in class. Both are kept in different arrays
 * and users of evas_object_smart_callbacks_descriptions_get() should
 * handle this case as they wish.
 *
 * @param obj The smart object
 * @param descriptions NULL terminated (name != NULL) array with
 *        descriptions.  Array elements will not be modified, but
 *        reference to them and their contents will be made, so this
 *        array should be kept alive during object lifetime.
 * @return 1 on success, 0 on failure.
 * @ingroup Evas_Smart_Object_Group
 *
 * @note while instance callbacks descriptions are possible, they are
 *       not recommended. Use class callbacks descriptions instead as they
 *       make user's life simpler and will use less memory as descriptions
 *       and arrays will be shared among all instances.
 */
EAPI Eina_Bool
evas_object_smart_callbacks_descriptions_set(Evas_Object *obj, const Evas_Smart_Cb_Description *descriptions)
{
   const Evas_Smart_Cb_Description *d;
   Evas_Object_Smart *o;
   unsigned int i, count = 0;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   o = (Evas_Object_Smart *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Smart, MAGIC_OBJ_SMART);
   return 0;
   MAGIC_CHECK_END();

   if ((!descriptions) || (!descriptions->name))
     {
	evas_smart_cb_descriptions_resize(&o->callbacks_descriptions, 0);
	return 1;
     }

   for (count = 0, d = descriptions; d->name != NULL; d++)
     count++;

   evas_smart_cb_descriptions_resize(&o->callbacks_descriptions, count);
   for (i = 0, d = descriptions; i < count; d++, i++)
     o->callbacks_descriptions.array[i] = d;

   evas_smart_cb_descriptions_fix(&o->callbacks_descriptions);

   return 1;
}

/**
 * Get the callbacks descriptions known by this smart object.
 *
 * This call retrieves processed callbacks descriptions for both
 * instance and class. These arrays are sorted by description's name
 * and are @c NULL terminated, so both @a class_count and
 * @a instance_count can be ignored, the terminator @c NULL is not
 * counted in these values.
 *
 * @param obj the smart object.
 * @param class_descriptions where to store class callbacks
 *        descriptions array, if any is known. If no descriptions are
 *        known, @c NULL is returned. This parameter may be @c NULL if
 *        it is not of interest.
 * @param class_count returns how many class callbacks descriptions
 *        are known.
 * @param instance_descriptions where to store instance callbacks
 *        descriptions array, if any is known. If no descriptions are
 *        known, @c NULL is returned. This parameter may be @c NULL if
 *        it is not of interest.
 * @param instance_count returns how many instance callbacks
 *        descriptions are known.
 *
 * @note if just class descriptions are of interest, try
 *       evas_smart_callbacks_descriptions_get() instead.
 *
 * @see evas_smart_callbacks_descriptions_get()
 * @ingroup Evas_Smart_Object_Group
 */
EAPI void
evas_object_smart_callbacks_descriptions_get(const Evas_Object *obj, const Evas_Smart_Cb_Description ***class_descriptions, unsigned int *class_count, const Evas_Smart_Cb_Description ***instance_descriptions, unsigned int *instance_count)
{
   Evas_Object_Smart *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   if (class_descriptions) *class_descriptions = NULL;
   if (class_count) *class_count = 0;
   if (instance_descriptions) *instance_descriptions = NULL;
   if (instance_count) *instance_count = 0;
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Smart *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Smart, MAGIC_OBJ_SMART);
   if (class_descriptions) *class_descriptions = NULL;
   if (class_count) *class_count = 0;
   if (instance_descriptions) *instance_descriptions = NULL;
   if (instance_count) *instance_count = 0;
   return;
   MAGIC_CHECK_END();

   if (class_descriptions)
     *class_descriptions = obj->smart.smart->callbacks.array;
   if (class_count)
     *class_count = obj->smart.smart->callbacks.size;

   if (instance_descriptions)
     *instance_descriptions = o->callbacks_descriptions.array;
   if (instance_count)
     *instance_count = o->callbacks_descriptions.size;
}

/**
 * Find callback description for callback called @a name.
 *
 * @param obj the smart object.
 * @param name name of desired callback, must @b not be @c NULL.  The
 *        search have a special case for @a name being the same
 *        pointer as registered with Evas_Smart_Cb_Description, one
 *        can use it to avoid excessive use of strcmp().
 * @param class_description pointer to return class description or @c
 *        NULL if not found. If parameter is @c NULL, no search will
 *        be done on class descriptions.
 * @param instance_description pointer to return instance description
 *        or @c NULL if not found. If parameter is @c NULL, no search
 *        will be done on instance descriptions.
 * @return reference to description if found, @c NULL if not found.
 */
EAPI void
evas_object_smart_callback_description_find(const Evas_Object *obj, const char *name, const Evas_Smart_Cb_Description **class_description, const Evas_Smart_Cb_Description **instance_description)
{
   Evas_Object_Smart *o;

   if (!name)
     {
	if (class_description) *class_description = NULL;
	if (instance_description) *instance_description = NULL;
	return;
     }

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   if (class_description) *class_description = NULL;
   if (instance_description) *instance_description = NULL;
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Smart *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Smart, MAGIC_OBJ_SMART);
   if (class_description) *class_description = NULL;
   if (instance_description) *instance_description = NULL;
   return;
   MAGIC_CHECK_END();

   if (class_description)
     *class_description = evas_smart_cb_description_find
       (&obj->smart.smart->callbacks, name);

   if (instance_description)
     *instance_description = evas_smart_cb_description_find
       (&o->callbacks_descriptions, name);
}

/**
 * Set the need_recalculate flag of given smart object.
 *
 * If this flag is set then calculate() callback (method) of the given
 * smart object will be called, if one is provided, during render phase
 * usually evas_render(). After this step, this flag will be automatically
 * unset.
 *
 * If no calculate() is provided, this flag will be left unchanged.
 *
 * @note just setting this flag will not make scene dirty and evas_render()
 *       will have no effect. To do that, use evas_object_smart_changed(),
 *       that will automatically call this function with 1 as parameter.
 *
 * @param obj the smart object
 * @param value if one want to set or unset the need_recalculate flag.
 *
 * @ingroup Evas_Smart_Object_Group
 */
EAPI void
evas_object_smart_need_recalculate_set(Evas_Object *obj, Eina_Bool value)
{
   Evas_Object_Smart *o;
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = obj->object_data;
   MAGIC_CHECK(o, Evas_Object_Smart, MAGIC_OBJ_SMART);
   return;
   MAGIC_CHECK_END();

   value = !!value;
   if (o->need_recalculate == value)
     return;
   o->need_recalculate = value;

   if (!obj->smart.smart->smart_class->calculate)
     return;

   /* XXX: objects can be present multiple times in calculate_objects()
    * XXX: after a set-unset-set cycle, but it's not a problem since
    * XXX: on _evas_render_call_smart_calculate() will check for the flag
    * XXX: and it will be unset after the first.
    */
   if (o->need_recalculate)
     {
	Evas *e;
	e = obj->layer->evas;
	eina_array_push(&e->calculate_objects, obj);
     }
   /* TODO: else, remove from array */
}

/**
 * Get the current value of need_recalculate flag.
 *
 * @note this flag will be unset during the render phase, after calculate()
 *       is called if one is provided.  If no calculate() is provided, then
 *       the flag will be left unchanged after render phase.
 *
 * @param obj the smart object
 * @return if flag is set or not.
 *
 * @ingroup Evas_Smart_Object_Group
 */
EAPI Eina_Bool
evas_object_smart_need_recalculate_get(const Evas_Object *obj)
{
   Evas_Object_Smart *o;
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   o = obj->object_data;
   MAGIC_CHECK(o, Evas_Object_Smart, MAGIC_OBJ_SMART);
   return 0;
   MAGIC_CHECK_END();

   return o->need_recalculate;
}

/**
 * Call user provided calculate() and unset need_calculate.
 *
 * @param obj the smart object
 *
 * @ingroup Evas_Smart_Object_Group
 */
EAPI void
evas_object_smart_calculate(Evas_Object *obj)
{
   Evas_Object_Smart *o;
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = obj->object_data;
   MAGIC_CHECK(o, Evas_Object_Smart, MAGIC_OBJ_SMART);
   return;
   MAGIC_CHECK_END();

   if (!obj->smart.smart->smart_class->calculate)
     return;

   o->need_recalculate = 0;
   obj->smart.smart->smart_class->calculate(obj);
}

/**
 * Call user provided calculate() and unset need_calculate on all objects.
 *
 * @param e The canvas to calculate all objects in
 *
 * @ingroup Evas_Smart_Object_Group
 */
EAPI void
evas_smart_objects_calculate(Evas *e)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();
   evas_call_smarts_calculate(e);
}

/**
 * Call calculate() on all smart objects that need_recalculate.
 *
 * @internal
 */
void
evas_call_smarts_calculate(Evas *e)
{
   Eina_Array *calculate;
   unsigned int i;

   calculate = &e->calculate_objects;
   for (i = 0; i < calculate->count; ++i)
     {
	Evas_Object *obj;
	Evas_Object_Smart *o;

	obj = eina_array_data_get(calculate, i);
	if (obj->delete_me)
	  continue;

	o = obj->object_data;
	if (o->need_recalculate)
	  {
	     o->need_recalculate = 0;
	     obj->smart.smart->smart_class->calculate(obj);
	  }
     }

   eina_array_flush(calculate);
}

/**
 * Mark smart object as changed, dirty.
 *
 * This will inform the scene that it changed and needs to be redraw, also
 * setting need_recalculate on the given object.
 *
 * @see evas_object_smart_need_recalculate_set().
 *
 * @ingroup Evas_Smart_Object_Group
 */
EAPI void
evas_object_smart_changed(Evas_Object *obj)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   evas_object_change(obj);
   evas_object_smart_need_recalculate_set(obj, 1);
}

/* internal calls */
static void
evas_object_smart_callbacks_clear(Evas_Object *obj)
{
   Evas_Object_Smart *o;
   Eina_List *l;
   Evas_Smart_Callback *cb;

   o = (Evas_Object_Smart *)(obj->object_data);

   if (o->walking_list) return;
   if (!o->deletions_waiting) return;
   for (l = o->callbacks; l;)
     {
	cb = eina_list_data_get(l);
	l = eina_list_next(l);
	if (cb->delete_me)
	  {
	     o->callbacks = eina_list_remove(o->callbacks, cb);
	     if (cb->event) eina_stringshare_del(cb->event);
	     free(cb);
	  }
     }
}

void
evas_object_smart_del(Evas_Object *obj)
{
   Evas_Smart *s;

   s = obj->smart.smart;
   if (obj->delete_me) return;
   if ((s) && (s->smart_class->del)) s->smart_class->del(obj);
   if (obj->smart.parent) evas_object_smart_member_del(obj);
   if (s) evas_object_smart_unuse(s);
}

void
evas_object_smart_cleanup(Evas_Object *obj)
{
   Evas_Object_Smart *o;

   if (obj->smart.parent)
     evas_object_smart_member_del(obj);

   o = (Evas_Object_Smart *)(obj->object_data);
   if (o->magic == MAGIC_OBJ_SMART)
     {
	while (o->contained)
	  evas_object_smart_member_del((Evas_Object *)o->contained);

	while (o->callbacks)
	  {
	     Evas_Smart_Callback *cb;

	     cb = o->callbacks->data;
	     o->callbacks = eina_list_remove(o->callbacks, cb);
	     if (cb->event) eina_stringshare_del(cb->event);
	     free(cb);
	  }

	o->data = NULL;
     }

   obj->smart.parent = NULL;
   obj->smart.smart = NULL;
}

void
evas_object_smart_member_cache_invalidate(Evas_Object *obj)
{
   Evas_Object_Smart *o;
   Eina_Inlist *l;

   o = (Evas_Object_Smart *)(obj->object_data);
   if (o->magic != MAGIC_OBJ_SMART)
     return;

   obj->parent_cache_valid = 0;
   for (l = o->contained; l; l = l->next)
     {
	Evas_Object *obj2;

	obj2 = (Evas_Object *)l;
	evas_object_smart_member_cache_invalidate(obj2);
     }
}

void
evas_object_smart_member_raise(Evas_Object *member)
{
   Evas_Object_Smart *o;

   o = (Evas_Object_Smart *)(member->smart.parent->object_data);
   o->contained = eina_inlist_demote(o->contained, EINA_INLIST_GET(member));
}

void
evas_object_smart_member_lower(Evas_Object *member)
{
   Evas_Object_Smart *o;

   o = (Evas_Object_Smart *)(member->smart.parent->object_data);
   o->contained = eina_inlist_promote(o->contained, EINA_INLIST_GET(member));
}

void
evas_object_smart_member_stack_above(Evas_Object *member, Evas_Object *other)
{
   Evas_Object_Smart *o;

   o = (Evas_Object_Smart *)(member->smart.parent->object_data);
   o->contained = eina_inlist_remove(o->contained, EINA_INLIST_GET(member));
   o->contained = eina_inlist_append_relative(o->contained, EINA_INLIST_GET(member), EINA_INLIST_GET(other));
}

void
evas_object_smart_member_stack_below(Evas_Object *member, Evas_Object *other)
{
   Evas_Object_Smart *o;

   o = (Evas_Object_Smart *)(member->smart.parent->object_data);
   o->contained = eina_inlist_remove(o->contained, EINA_INLIST_GET(member));
   o->contained = eina_inlist_prepend_relative(o->contained, EINA_INLIST_GET(member), EINA_INLIST_GET(other));
}

/* all nice and private */
static void
evas_object_smart_init(Evas_Object *obj)
{
   /* alloc smart obj, setup methods and default values */
   obj->object_data = evas_object_smart_new();
   /* set up default settings for this kind of object */
   obj->cur.color.r = 255;
   obj->cur.color.g = 255;
   obj->cur.color.b = 255;
   obj->cur.color.a = 255;
   obj->cur.geometry.x = 0;
   obj->cur.geometry.y = 0;
   obj->cur.geometry.w = 0;
   obj->cur.geometry.h = 0;
   obj->cur.layer = 0;
   /* set up object-specific settings */
   obj->prev = obj->cur;
   /* set up methods (compulsory) */
   obj->func = &object_func;
}

static void *
evas_object_smart_new(void)
{
   Evas_Object_Smart *o;

   /* alloc obj private data */
   o = calloc(1, sizeof(Evas_Object_Smart));
   o->magic = MAGIC_OBJ_SMART;
   return o;
}

static void
evas_object_smart_free(Evas_Object *obj)
{
   Evas_Object_Smart *o;

   /* frees private object data. very simple here */
   o = (Evas_Object_Smart *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Smart, MAGIC_OBJ_SMART);
   return;
   MAGIC_CHECK_END();
   /* free obj */
   o->magic = 0;
   free(o);
}

static void
evas_object_smart_render(Evas_Object *obj __UNUSED__, void *output __UNUSED__, void *context __UNUSED__, void *surface __UNUSED__, int x __UNUSED__, int y __UNUSED__)
{
   return;
}

static void
evas_object_smart_render_pre(Evas_Object *obj)
{
   if (obj->pre_render_done) return;
   if ((obj->cur.map != obj->prev.map) ||
       (obj->cur.usemap != obj->prev.usemap))
     {
	evas_object_render_pre_prev_cur_add(&obj->layer->evas->clip_changes, obj);
        goto done;
     }
   done:
   obj->pre_render_done = 1;
}

static void
evas_object_smart_render_post(Evas_Object *obj)
{
   obj->prev = obj->cur;
}

static unsigned int evas_object_smart_id_get(Evas_Object *obj)
{
   Evas_Object_Smart *o;

   o = (Evas_Object_Smart *)(obj->object_data);
   if (!o) return 0;
   return MAGIC_OBJ_SMART;
}

static unsigned int evas_object_smart_visual_id_get(Evas_Object *obj)
{
   Evas_Object_Smart *o;

   o = (Evas_Object_Smart *)(obj->object_data);
   if (!o) return 0;
   return MAGIC_OBJ_CONTAINER;
}

static void *evas_object_smart_engine_data_get(Evas_Object *obj)
{
   Evas_Object_Smart *o;

   o = (Evas_Object_Smart *)(obj->object_data);
   if (!o) return NULL;
   return o->engine_data;
}
