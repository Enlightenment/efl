#include "evas_common.h"
#include "evas_private.h"
#include "Evas.h"

typedef struct _Evas_Object_Smart      Evas_Object_Smart;
typedef struct _Evas_Smart_Callback    Evas_Smart_Callback;

struct _Evas_Object_Smart
{
   DATA32            magic;
   void             *engine_data;
};

struct _Evas_Smart_Callback
{
   char *event;
   char  delete_me : 1;
   void (*func) (void *data, Evas_Object *obj, void *event_info);
   void *func_data;
};

/* private methods for smart objects */
static void evas_object_smart_callbacks_clear(Evas_Object *obj);
static void evas_object_smart_init(Evas_Object *obj);
static void *evas_object_smart_new(void);
static void evas_object_smart_render(Evas_Object *obj, void *output, void *context, void *surface, int x, int y);
static void evas_object_smart_free(Evas_Object *obj);
static void evas_object_smart_render_pre(Evas_Object *obj);
static void evas_object_smart_render_post(Evas_Object *obj);

static Evas_Object_Func object_func =
{
   /* methods (compulsory) */
   evas_object_smart_free,
     evas_object_smart_render,
     evas_object_smart_render_pre,
     evas_object_smart_render_post,
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
 * To be documented.
 *
 * FIXME: To be fixed.
 * 
 */
void
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
   obj->smart.data = data;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * 
 */
void *
evas_object_smart_data_get(Evas_Object *obj)
{
   Evas_Object_Smart *o;
   
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   o = (Evas_Object_Smart *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Smart, MAGIC_OBJ_SMART);
   return NULL;
   MAGIC_CHECK_END();
   return obj->smart.data;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * 
 */
Evas_Smart *
evas_object_smart_smart_get(Evas_Object *obj)
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
 * To be documented.
 *
 * FIXME: To be fixed.
 * 
 */
void
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

   if (obj->smart.parent) evas_object_smart_member_del(obj);
   
   obj->smart.parent = smart_obj;
   smart_obj->smart.contained = evas_list_append(smart_obj->smart.contained, obj);
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * 
 */
void
evas_object_smart_member_del(Evas_Object *obj)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();

   if (!obj->smart.parent) return;
   
   obj->smart.parent->smart.contained = evas_list_remove(obj->smart.parent->smart.contained, obj);
   obj->smart.parent = NULL;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * 
 */
Evas_Object *
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
   obj->type = s->name;
   evas_object_smart_init(obj);
   evas_object_inject(obj, e);
   
   evas_object_smart_use(s);
   
   if (s->func_add) s->func_add(obj);
   
   return obj;   
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * 
 */
void
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
   cb->event = strdup(event);
   cb->func = func;
   cb->func_data = (void *)data;
   obj->smart.callbacks = evas_list_prepend(obj->smart.callbacks, cb);
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * 
 */
void *
evas_object_smart_callback_del(Evas_Object *obj, const char *event, void (*func) (void *data, Evas_Object *obj, void *event_info))
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
   for (l = obj->smart.callbacks; l; l = l->next)
     {
	Evas_Smart_Callback *cb;
	
	cb = l->data;
	if ((!strcmp(cb->event, event)) && (cb->func == func))
	  {
	     void *data;
	     
	     data = cb->func_data;
	     cb->delete_me = 1;
	     obj->smart.deletions_waiting = 1;
	     evas_object_smart_callbacks_clear(obj);
	     return data;
	  }
     }
   return NULL;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * 
 */
void
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
   obj->smart.walking_list++;
   for (l = obj->smart.callbacks; l; l = l->next)
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
   obj->smart.walking_list--;
   evas_object_smart_callbacks_clear(obj);
}

/* internal calls */
static void
evas_object_smart_callbacks_clear(Evas_Object *obj)
{
   Evas_List *l;
   
   if (obj->smart.walking_list) return;
   if (!obj->smart.deletions_waiting) return;
   for (l = obj->smart.callbacks; l;)
     {
	Evas_Smart_Callback *cb;
	
	cb = l->data;
	l = l->next;
	if (cb->delete_me)
	  {
	     obj->smart.callbacks = evas_list_remove(obj->smart.callbacks, cb);
	     if (cb->event) free(cb->event);
	     free(cb);
	  }
     }
}

void
evas_object_smart_del(Evas_Object *obj)
{
   Evas_Smart *s;
   
   s = obj->smart.smart;
   if ((s) && (s->func_del)) s->func_del(obj);
   if (obj->smart.parent) evas_object_smart_member_del(obj);
   if (s) evas_object_smart_unuse(s);
}

void
evas_object_smart_cleanup(Evas_Object *obj)
{
   Evas_Smart *s;
   
   s = obj->smart.smart;
   while (obj->smart.contained)
     obj->smart.contained = evas_list_remove(obj->smart.contained, obj->smart.contained->data);
   while (obj->smart.callbacks)
     {
	Evas_Smart_Callback *cb;
	
	cb = obj->smart.callbacks->data;
	obj->smart.callbacks = evas_list_remove(obj->smart.callbacks, cb);
	if (cb->event) free(cb->event);
	free(cb);
     }
   obj->smart.parent;
   obj->smart.data = NULL;
   obj->smart.smart = NULL;
   if (s) evas_object_smart_unuse(s);
}

/* all nice and private */
static void
evas_object_smart_init(Evas_Object *obj)
{
   /* alloc image ob, setup methods and default values */
   obj->object_data = evas_object_smart_new();
   /* set up default settings for this kind of object */
   obj->cur.color.r = 255;
   obj->cur.color.g = 255;
   obj->cur.color.b = 255;
   obj->cur.color.a = 255;
   obj->cur.geometry.x = 0.0;
   obj->cur.geometry.y = 0.0;
   obj->cur.geometry.w = 32.0;
   obj->cur.geometry.h = 32.0;
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
