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
   int               walking_list;
   Evas_Bool         deletions_waiting : 1;
   Evas_Bool         need_recalculate : 1;
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
     NULL
};

/* public funcs */
/**
 * @defgroup Evas_Smart_Object_Group Evas Smart Object Functions
 *
 * Functions dealing with evas smart objects.
 *
 * Smart objects are groupings of primitive evas objects that behave as a
 * cohesive group. For instance, a file manager icon may be a smart object
 * composed of an image object, a text label and two rectangles that appear
 * behind the image and text when the icon is selected. As a smart object,
 * the normal evas api could be used on the icon object.
 *
 */
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
	printf("EVAS ERROR: Adding deleted object %p to smart obj %p\n", obj, smart_obj);
	abort();
	return;
     }
   if (smart_obj->delete_me)
     {
	printf("EVAS ERROR: Adding object %p to deleted smart obj %p\n", obj, smart_obj);
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

   o = (Evas_Object_Smart *)(obj->object_data);
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
   EINA_LIST_FOREACH(o->callbacks, l, cb)
     {
	if (!cb->delete_me)
	  {
	     if (!strcmp(cb->event, event))
	       cb->func(cb->func_data, obj, event_info);
	  }
	if (obj->delete_me)
	  break;
     }
   o->walking_list--;
   evas_object_smart_callbacks_clear(obj);
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
evas_object_smart_need_recalculate_set(Evas_Object *obj, Evas_Bool value)
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
EAPI Evas_Bool
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
 * @return if flag is set or not.
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

   if (obj->smart.smart->smart_class->calculate)
     obj->smart.smart->smart_class->calculate(obj);
   o->need_recalculate = 0;
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
	     obj->smart.smart->smart_class->calculate(obj);
	     o->need_recalculate = 0;
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
evas_object_smart_render(Evas_Object *obj, void *output, void *context, void *surface, int x, int y)
{
   return;
   obj = output = context = surface = NULL;
   x = y = 0;
}

static void
evas_object_smart_render_pre(Evas_Object *obj)
{
   if (obj->pre_render_done) return;
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
