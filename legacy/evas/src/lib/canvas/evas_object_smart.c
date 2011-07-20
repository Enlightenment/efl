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
   Evas_Smart_Cb func;
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

EVAS_MEMPOOL(_mp_obj);
EVAS_MEMPOOL(_mp_cb);

/* public funcs */
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
   evas_object_mapped_clip_across_mark(obj);
   if (smart_obj->smart.smart->smart_class->member_add)
     smart_obj->smart.smart->smart_class->member_add(smart_obj, obj);
}

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
   evas_object_mapped_clip_across_mark(obj);
}

EAPI Evas_Object *
evas_object_smart_parent_get(const Evas_Object *obj)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();

   return obj->smart.parent;
}

EAPI Eina_Bool
evas_object_smart_type_check(const Evas_Object *obj, const char *type)
{
   const Evas_Smart_Class *sc;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return EINA_FALSE;
   MAGIC_CHECK_END();

   if (!obj->smart.smart)
     return EINA_FALSE;
   sc = obj->smart.smart->smart_class;
   while (sc)
     {
        if (!strcmp(sc->name, type))
          return EINA_TRUE;
        sc = sc->parent;
     }

   return EINA_FALSE;
}

EAPI Eina_Bool
evas_object_smart_type_check_ptr(const Evas_Object *obj, const char *type)
{
   const Evas_Smart_Class *sc;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return EINA_FALSE;
   MAGIC_CHECK_END();

   if (!obj->smart.smart)
     return EINA_FALSE;
   sc = obj->smart.smart->smart_class;
   while (sc)
     {
        if (sc->name == type)
          return EINA_TRUE;
        sc = sc->parent;
     }

   return EINA_FALSE;
}

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

void
_evas_object_smart_members_all_del(Evas_Object *obj)
{
   Evas_Object_Smart *o = (Evas_Object_Smart *)(obj->object_data);
   while (o->contained)
     evas_object_del((Evas_Object *)(o->contained));
}

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

   obj = evas_object_new(e);
   if (!obj) return NULL;
   obj->smart.smart = s;
   obj->type = s->smart_class->name;
   evas_object_smart_init(obj);
   evas_object_inject(obj, e);

   evas_object_smart_use(s);

   if (s->smart_class->add) s->smart_class->add(obj);

   return obj;
}

EAPI void
evas_object_smart_callback_add(Evas_Object *obj, const char *event, Evas_Smart_Cb func, const void *data)
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
   EVAS_MEMPOOL_INIT(_mp_cb, "evas_smart_callback", Evas_Smart_Callback, 512, );
   cb = EVAS_MEMPOOL_ALLOC(_mp_cb, Evas_Smart_Callback);
   if (!cb) return;
   EVAS_MEMPOOL_PREP(_mp_cb, cb, Evas_Smart_Callback);
   cb->event = eina_stringshare_add(event);
   cb->func = func;
   cb->func_data = (void *)data;
   o->callbacks = eina_list_prepend(o->callbacks, cb);
}

EAPI void *
evas_object_smart_callback_del(Evas_Object *obj, const char *event, Evas_Smart_Cb func)
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

   for (count = 0, d = descriptions; d->name; d++)
     count++;

   evas_smart_cb_descriptions_resize(&o->callbacks_descriptions, count);
   if (count == 0) return 1;

   for (i = 0, d = descriptions; i < count; d++, i++)
     o->callbacks_descriptions.array[i] = d;

   evas_smart_cb_descriptions_fix(&o->callbacks_descriptions);

   return 1;
}

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

   if (obj->recalculate_cycle > 64)
     {
        ERR("Object %p is not stable during recalc loop", obj);
        return ;
     }
   if (obj->layer->evas->in_smart_calc)
     obj->recalculate_cycle++;
   o->need_recalculate = value;

   if (!obj->smart.smart->smart_class->calculate) return;

   /* XXX: objects can be present multiple times in calculate_objects()
    * XXX: after a set-unset-set cycle, but it's not a problem since
    * XXX: on _evas_render_call_smart_calculate() will check for the flag
    * XXX: and it will be unset after the first.
    */
   if (o->need_recalculate)
     {
        Evas *e = obj->layer->evas;
        eina_array_push(&e->calculate_objects, obj);
     }
   /* TODO: else, remove from array */
}

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
   Evas_Object *obj;
   Eina_Array_Iterator it;
   unsigned int i;

   e->in_smart_calc++;
   calculate = &e->calculate_objects;
   for (i = 0; i < eina_array_count_get(calculate); ++i)
     {
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
   EINA_ARRAY_ITER_NEXT(calculate, i, obj, it)
     {
        obj->recalculate_cycle = 0;
     }
   e->in_smart_calc--;
   if (e->in_smart_calc == 0) eina_array_clean(calculate);
}

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
             EVAS_MEMPOOL_FREE(_mp_cb, cb);
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
             EVAS_MEMPOOL_FREE(_mp_cb, cb);
          }

        evas_smart_cb_descriptions_resize(&o->callbacks_descriptions, 0);
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

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();

   obj->parent_cache_valid = 0;

   o = (Evas_Object_Smart *)(obj->object_data);
   if (o->magic != MAGIC_OBJ_SMART)
     return;

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
   EVAS_MEMPOOL_INIT(_mp_obj, "evas_object_smart", Evas_Object_Smart, 256, NULL);
   o = EVAS_MEMPOOL_ALLOC(_mp_obj, Evas_Object_Smart);
   if (!o) return NULL;
   EVAS_MEMPOOL_PREP(_mp_obj, o, Evas_Object_Smart);
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
   EVAS_MEMPOOL_FREE(_mp_obj, o);
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
