#include "evas_common.h"
#include "evas_private.h"

typedef struct _Evas_Object_Smart      Evas_Object_Smart;
typedef struct _Evas_Smart_Callback    Evas_Smart_Callback;

struct _Evas_Object_Smart
{
   DATA32            magic;
   void             *engine_data;
   void             *data;
   Evas_List        *callbacks;
   Evas_Object_List *contained;
   int               walking_list;
   Evas_Bool         deletions_waiting : 1;
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
   o->contained = evas_object_list_append(o->contained, obj);
   evas_object_smart_member_cache_invalidate(obj);
   obj->restack = 1;
   evas_object_change(obj);
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

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();

   if (!obj->smart.parent) return;

   o = (Evas_Object_Smart *)(obj->smart.parent->object_data);
   o->contained = evas_object_list_remove(o->contained, obj);
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
 * The returned list should be freed with evas_list_free() when you no longer need it
 */
EAPI Evas_List *
evas_object_smart_members_get(const Evas_Object *obj)
{
   Evas_Object_Smart *o;
   Evas_List *members;
   Evas_Object_List *member;
   
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   o = (Evas_Object_Smart *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Smart, MAGIC_OBJ_SMART);
   return NULL;
   MAGIC_CHECK_END();

   members = NULL;
   for (member = o->contained; member; member = member->next)
      members = evas_list_append(members, member);
   
   return members;
}

const Evas_Object_List *
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
   cb->event = evas_stringshare_add(event);
   cb->func = func;
   cb->func_data = (void *)data;
   o->callbacks = evas_list_prepend(o->callbacks, cb);
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
   Evas_List *l;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   o = (Evas_Object_Smart *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Smart, MAGIC_OBJ_SMART);
   return NULL;
   MAGIC_CHECK_END();
   if (!event) return NULL;
   for (l = o->callbacks; l; l = l->next)
     {
	Evas_Smart_Callback *cb;

	cb = l->data;
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
   Evas_List *l;

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
   for (l = o->callbacks; l; l = l->next)
     {
	Evas_Smart_Callback *cb;

	cb = l->data;
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

/* internal calls */
static void
evas_object_smart_callbacks_clear(Evas_Object *obj)
{
   Evas_Object_Smart *o;
   Evas_List *l;

   o = (Evas_Object_Smart *)(obj->object_data);

   if (o->walking_list) return;
   if (!o->deletions_waiting) return;
   for (l = o->callbacks; l;)
     {
	Evas_Smart_Callback *cb;

	cb = l->data;
	l = l->next;
	if (cb->delete_me)
	  {
	     o->callbacks = evas_list_remove(o->callbacks, cb);
	     if (cb->event) evas_stringshare_del(cb->event);
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
	     o->callbacks = evas_list_remove(o->callbacks, cb);
	     if (cb->event) evas_stringshare_del(cb->event);
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
   Evas_Object_List *l;

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
   o->contained = evas_object_list_remove(o->contained, member);
   o->contained = evas_object_list_append(o->contained, member);
}

void
evas_object_smart_member_lower(Evas_Object *member)
{
   Evas_Object_Smart *o;

   o = (Evas_Object_Smart *)(member->smart.parent->object_data);
   o->contained = evas_object_list_remove(o->contained, member);
   o->contained = evas_object_list_prepend(o->contained, member);
}

void
evas_object_smart_member_stack_above(Evas_Object *member, Evas_Object *other)
{
   Evas_Object_Smart *o;

   o = (Evas_Object_Smart *)(member->smart.parent->object_data);
   o->contained = evas_object_list_remove(o->contained, member);
   o->contained = evas_object_list_append_relative(o->contained, member, other);
}

void
evas_object_smart_member_stack_below(Evas_Object *member, Evas_Object *other)
{
   Evas_Object_Smart *o;

   o = (Evas_Object_Smart *)(member->smart.parent->object_data);
   o->contained = evas_object_list_remove(o->contained, member);
   o->contained = evas_object_list_prepend_relative(o->contained, member, other);
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
