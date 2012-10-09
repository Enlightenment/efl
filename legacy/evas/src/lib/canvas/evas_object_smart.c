#include "evas_common.h"
#include "evas_private.h"

#include "Eo.h"

EAPI Eo_Op EVAS_OBJ_SMART_BASE_ID = EO_NOOP;

#define MY_CLASS EVAS_OBJ_SMART_CLASS

#define MY_CLASS_NAME "Evas_Object_Smart"

extern Eina_Hash* signals_hash_table;

typedef struct _Evas_Object_Smart      Evas_Object_Smart;
typedef struct _Evas_Smart_Callback    Evas_Smart_Callback;

struct _Evas_Object_Smart
{
   void             *engine_data;
   void             *data;
   Eina_List        *callbacks;
   Eina_Inlist      *contained;
   Eina_Inlist      *smart_callbacks_infos;
   Evas_Smart_Cb_Description_Array callbacks_descriptions;
   int               walking_list;
   int               member_count;
   Eina_Bool         deletions_waiting : 1;
   Eina_Bool         need_recalculate : 1;
   Eina_Bool         update_boundingbox_needed : 1;
};

struct _Evas_Smart_Callback
{
   const char *event;
   Evas_Smart_Cb func;
   void *func_data;
   Evas_Callback_Priority priority;
   char  delete_me : 1;
};

/* private methods for smart objects */
static void evas_object_smart_callbacks_clear(Evas_Object *eo_obj);
static void evas_object_smart_init(Evas_Object *eo_obj);
static void evas_object_smart_render(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj, void *output, void *context, void *surface, int x, int y);
static void evas_object_smart_render_pre(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj);
static void evas_object_smart_render_post(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj);

static unsigned int evas_object_smart_id_get(Evas_Object *eo_obj);
static unsigned int evas_object_smart_visual_id_get(Evas_Object *eo_obj);
static void *evas_object_smart_engine_data_get(Evas_Object *eo_obj);

static const Evas_Object_Func object_func =
{
   /* methods (compulsory) */
   NULL,
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


EVAS_MEMPOOL(_mp_cb);
/* public funcs */
EAPI void
evas_object_smart_data_set(Evas_Object *eo_obj, void *data)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   eo_do(eo_obj, evas_obj_smart_data_set(data));
}

static void
_smart_data_set(Eo *eo_obj EINA_UNUSED, void *_pd, va_list *list)
{
   void *data = va_arg(*list, void *);
   Evas_Object_Smart *o = _pd;
   o->data = data;
}

EAPI void *
evas_object_smart_data_get(const Evas_Object *eo_obj)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   void *data = NULL;
   eo_do((Eo *)eo_obj, evas_obj_smart_data_get(&data));
   return data;
}

static void
_smart_data_get(Eo *eo_obj EINA_UNUSED, void *_pd, va_list *list)
{
   const Evas_Object_Smart *o = _pd;
   void **data = va_arg(*list, void **);
   *data = o->data;
}

EAPI const void *
evas_object_smart_interface_get(const Evas_Object *eo_obj,
                                const char *name)
{
   Evas_Smart *s;
   unsigned int i;

   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();

   if (!eo_isa(eo_obj, EVAS_OBJ_SMART_CLASS)) return NULL;

   s = evas_object_smart_smart_get(eo_obj);
   if (!s) return NULL;

   if (s)
     {
        for (i = 0; i < s->interfaces.size; i++)
          {
             const Evas_Smart_Interface *iface;

             iface = s->interfaces.array[i];

             if (iface->name == name)
                return iface;
          }
     }

   return NULL;
}

EAPI void *
evas_object_smart_interface_data_get(const Evas_Object *eo_obj,
                                     const Evas_Smart_Interface *iface)
{
   unsigned int i;
   Evas_Smart *s;

   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();

   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
   s = evas_object_smart_smart_get(eo_obj);
   if (!s) return NULL;

   if (s)
     {
        for (i = 0; i < s->interfaces.size; i++)
          {
             if (iface == s->interfaces.array[i])
                return obj->interface_privates[i];
          }
     }

   return NULL;
}

EAPI Evas_Smart *
evas_object_smart_smart_get(const Evas_Object *eo_obj)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   Evas_Smart *smart = NULL;
   eo_do((Eo *)eo_obj, evas_obj_smart_smart_get(&smart));
   return smart;
}

static void
_smart_smart_get(Eo *eo_obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
   Evas_Smart **smart = va_arg(*list, Evas_Smart **);
   if (smart) *smart = obj->smart.smart;
}

EAPI void
evas_object_smart_member_add(Evas_Object *eo_obj, Evas_Object *smart_obj)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   eo_do(smart_obj, evas_obj_smart_member_add(eo_obj));
}

static void
_smart_member_add(Eo *smart_obj, void *_pd, va_list *list)
{
   Evas_Object *eo_obj = va_arg(*list, Evas_Object *);

   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
   Evas_Object_Protected_Data *smart = eo_data_get(smart_obj, EVAS_OBJ_CLASS);
   Evas_Object_Smart *o = _pd;

   if (obj->delete_me)
     {
        CRIT("Adding deleted object %p to smart obj %p", eo_obj, smart_obj);
        abort();
        return;
     }
   if (smart->delete_me)
     {
        CRIT("Adding object %p to deleted smart obj %p", eo_obj, smart_obj);
        abort();
        return;
     }
   if (!smart->layer)
     {
        CRIT("No evas surface associated with smart object (%p)", smart_obj);
        abort();
        return;
     }
   if ((obj->layer && smart->layer) &&
       (obj->layer->evas != smart->layer->evas))
     {
        CRIT("Adding object %p from Evas (%p) from another Evas (%p)", eo_obj, obj->layer->evas, smart->layer->evas);
        abort();
        return;
     }

   if (obj->smart.parent == smart_obj) return;

   if (obj->smart.parent) evas_object_smart_member_del(eo_obj);

   o->member_count++;
   evas_object_release(eo_obj, obj, 1);
   obj->layer = smart->layer;
   obj->cur.layer = obj->layer->layer;
   obj->layer->usage++;
   obj->smart.parent = smart_obj;
   o->contained = eina_inlist_append(o->contained, EINA_INLIST_GET(obj));
   evas_object_smart_member_cache_invalidate(eo_obj, EINA_TRUE, EINA_TRUE);
   obj->restack = 1;
   evas_object_change(eo_obj, obj);
   evas_object_mapped_clip_across_mark(eo_obj, obj);
   if (smart->smart.smart && smart->smart.smart->smart_class->member_add)
     smart->smart.smart->smart_class->member_add(smart_obj, eo_obj);
   evas_object_update_bounding_box(eo_obj, obj);
}

EAPI void
evas_object_smart_member_del(Evas_Object *eo_obj)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
   if (!obj) return;
   if (!obj->smart.parent) return;
   Evas_Object *smart_obj = obj->smart.parent;

   eo_do(smart_obj, evas_obj_smart_member_del(eo_obj));
}

static void
_smart_member_del(Eo *smart_obj, void *_pd EINA_UNUSED, va_list *list)
{
   Evas_Object *eo_obj = va_arg(*list, Evas_Object *);
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);

   if (!obj->smart.parent) return;

   Evas_Object_Protected_Data *smart = eo_data_get(smart_obj, EVAS_OBJ_CLASS);
   if (smart->smart.smart && smart->smart.smart->smart_class->member_del)
     smart->smart.smart->smart_class->member_del(smart_obj, eo_obj);

   Evas_Object_Smart *o = eo_data_get(smart_obj, MY_CLASS);
   o->contained = eina_inlist_remove(o->contained, EINA_INLIST_GET(obj));
   o->member_count--;
   obj->smart.parent = NULL;
   evas_object_smart_member_cache_invalidate(eo_obj, EINA_TRUE, EINA_TRUE);
   obj->layer->usage--;
   obj->cur.layer = obj->layer->layer;
   evas_object_inject(eo_obj, obj, obj->layer->evas->evas);
   obj->restack = 1;
   evas_object_change(eo_obj, obj);
   evas_object_mapped_clip_across_mark(eo_obj, obj);
}

EAPI Evas_Object *
evas_object_smart_parent_get(const Evas_Object *eo_obj)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   Evas_Object *smart_parent = NULL;
   eo_do((Eo *)eo_obj, evas_obj_smart_parent_get(&smart_parent));
   return smart_parent;
}

static void
_type_check(Eo *eo_obj, void *_pd EINA_UNUSED, va_list *list)
{
   const char *type = va_arg(*list, const char *);
   Eina_Bool *type_check = va_arg(*list, Eina_Bool *);
   if (0 == strcmp(type, "Evas_Object_Smart"))
      *type_check = EINA_TRUE;
   else
      eo_do_super(eo_obj, evas_obj_type_check(type, type_check));
}

EAPI Eina_Bool
evas_object_smart_type_check(const Evas_Object *eo_obj, const char *type)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return EINA_FALSE;
   MAGIC_CHECK_END();
   Eina_Bool type_check = EINA_FALSE;
   eo_do((Eo *)eo_obj, evas_obj_smart_type_check(type, &type_check));
   return type_check;
}

EAPI Eina_Bool
evas_object_smart_type_check_ptr(const Evas_Object *eo_obj, const char *type)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return EINA_FALSE;
   MAGIC_CHECK_END();
   Eina_Bool type_check_ptr = EINA_FALSE;
   eo_do((Eo *)eo_obj, evas_obj_smart_type_check_ptr(type, &type_check_ptr));
   return type_check_ptr;
}

EAPI Eina_List *
evas_object_smart_members_get(const Evas_Object *eo_obj)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   if (!eo_isa(eo_obj, MY_CLASS)) return NULL;
   Eina_List *list = NULL;
   eo_do((Eo *)eo_obj, evas_obj_smart_members_get(&list));
   return list;
}

static void
_smart_members_get(Eo *eo_obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_List **members_list = va_arg(*list, Eina_List **);
   const Evas_Object_Smart *o = _pd;
   Eina_List *members;

   Eina_Inlist *member;

   members = NULL;
   for (member = o->contained; member; member = member->next)
     members = eina_list_append(members, ((Evas_Object_Protected_Data *)member)->object);

   *members_list = members;
}

const Eina_Inlist *
evas_object_smart_members_get_direct(const Evas_Object *eo_obj)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   if (!eo_isa(eo_obj, MY_CLASS)) return NULL;
   Evas_Object_Smart *o = eo_data_get(eo_obj, MY_CLASS);
   return o->contained;
}

void
_evas_object_smart_members_all_del(Evas_Object *eo_obj)
{
   Evas_Object_Smart *o = eo_data_get(eo_obj, MY_CLASS);
   Evas_Object_Protected_Data *memobj;
   Eina_Inlist *itrn;
   EINA_INLIST_FOREACH_SAFE(o->contained, itrn, memobj)
     {
        evas_object_del((Evas_Object *)((Evas_Object_Protected_Data *)memobj->object));
     }
}

static void
_evas_smart_class_ifaces_private_data_alloc(Evas_Object *eo_obj,
                                            Evas_Smart *s)
{
   unsigned int i, total_priv_sz = 0;
   const Evas_Smart_Class *sc;
   unsigned char *ptr;

   /* get total size of interfaces private data */
   for (sc = s->smart_class; sc; sc = sc->parent)
     {
        const Evas_Smart_Interface **ifaces_array = sc->interfaces;
        if (!ifaces_array) continue;

        while (*ifaces_array)
          {
             const Evas_Smart_Interface *iface = *ifaces_array;

             if (!iface->name) break;

             if (iface->private_size > 0)
               {
                  unsigned int size = iface->private_size;

                  if (size % sizeof(void *) != 0)
                    size += sizeof(void *) - (size % sizeof(void *));
                  total_priv_sz += size;
               }

             ifaces_array++;
          }
     }

   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
   obj->interface_privates = malloc
       (s->interfaces.size * sizeof(void *) + total_priv_sz);
   if (!obj->interface_privates)
     {
        ERR("malloc failed!");
        return;
     }

   /* make private data array ptrs point to right places, WHICH LIE ON
    * THE SAME STRUCT, AFTER THE # OF INTERFACES COUNT */
   ptr = (unsigned char *)(obj->interface_privates + s->interfaces.size);
   for (i = 0; i < s->interfaces.size; i++)
     {
        unsigned int size;

        size = s->interfaces.array[i]->private_size;

        if (size == 0)
          {
             obj->interface_privates[i] = NULL;
             continue;
          }

        obj->interface_privates[i] = ptr;
        memset(ptr, 0, size);

        if (size % sizeof(void *) != 0)
          size += sizeof(void *) - (size % sizeof(void *));
        ptr += size;
     }
}

EAPI Evas_Object *
evas_object_smart_add(Evas *eo_e, Evas_Smart *s)
{
   Evas_Object *eo_obj;

   MAGIC_CHECK(eo_e, Evas, MAGIC_EVAS);
   return NULL;
   MAGIC_CHECK_END();
   eo_obj = eo_add(EVAS_OBJ_SMART_CLASS, eo_e);
   eo_do(eo_obj, evas_obj_smart_attach(s));
   eo_unref(eo_obj);
   return eo_obj;
}

static void
_constructor(Eo *eo_obj, void *class_data EINA_UNUSED, va_list *list EINA_UNUSED)
{
   eo_do_super(eo_obj, eo_constructor());
   evas_object_smart_init(eo_obj);
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
   evas_object_inject(eo_obj, obj, evas_object_evas_get(eo_parent_get(eo_obj)));
   eo_do(eo_obj,
         evas_obj_type_set(MY_CLASS_NAME),
         evas_obj_smart_add());
}

static void
_smart_add(Eo *eo_obj, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   // If this function is reached, so we do nothing except trying to call
   // the function of the legacy smart class.
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
   Evas_Smart *s = obj->smart.smart;
   if (s && s->smart_class->add) s->smart_class->add(eo_obj);
}

static void
_smart_del(Eo *eo_obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
}

static void
_smart_resize(Eo *eo_obj, void *_pd EINA_UNUSED, va_list *list)
{
   Evas_Coord w = va_arg(*list, Evas_Coord);
   Evas_Coord h = va_arg(*list, Evas_Coord);
   // If this function is reached, so we do nothing except trying to call
   // the function of the legacy smart class.
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
   Evas_Smart *s = obj->smart.smart;
   if (s && s->smart_class->resize) s->smart_class->resize(eo_obj, w, h);
}

static void
_smart_move(Eo *eo_obj, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   Evas_Coord x = va_arg(*list, Evas_Coord);
   Evas_Coord y = va_arg(*list, Evas_Coord);
   // If this function is reached, so we do nothing except trying to call
   // the function of the legacy smart class.
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
   Evas_Smart *s = obj->smart.smart;
   if (s && s->smart_class->move) s->smart_class->move(eo_obj, x, y);
}

static void
_smart_show(Eo *eo_obj, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   // If this function is reached, so we do nothing except trying to call
   // the function of the legacy smart class.
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
   Evas_Smart *s = obj->smart.smart;
   if (s && s->smart_class->show) s->smart_class->show(eo_obj);
}

static void
_smart_hide(Eo *eo_obj, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   // If this function is reached, so we do nothing except trying to call
   // the function of the legacy smart class.
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
   Evas_Smart *s = obj->smart.smart;
   if (s && s->smart_class->hide) s->smart_class->hide(eo_obj);
}

static void
_smart_color_set(Eo *eo_obj, void *_pd EINA_UNUSED, va_list *list)
{
   int r = va_arg(*list, int);
   int g = va_arg(*list, int);
   int b = va_arg(*list, int);
   int a = va_arg(*list, int);
   // If this function is reached, so we do nothing except trying to call
   // the function of the legacy smart class.
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
   Evas_Smart *s = obj->smart.smart;
   if (s && s->smart_class->color_set) s->smart_class->color_set(eo_obj, r, g, b, a);
}

static void
_smart_clip_set(Eo *eo_obj, void *_pd EINA_UNUSED, va_list *list)
{
   Evas_Object *clip = va_arg(*list, Evas_Object *);
   // If this function is reached, so we do nothing except trying to call
   // the function of the legacy smart class.
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
   Evas_Smart *s = obj->smart.smart;
   if (s && s->smart_class->clip_set) s->smart_class->clip_set(eo_obj, clip);
}

static void
_smart_clip_unset(Eo *eo_obj, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   // If this function is reached, so we do nothing except trying to call
   // the function of the legacy smart class.
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
   Evas_Smart *s = obj->smart.smart;
   if (s && s->smart_class->clip_unset) s->smart_class->clip_unset(eo_obj);
}

static void
_smart_attach(Eo *eo_obj, void *_pd EINA_UNUSED, va_list *list)
{
   Evas_Smart *s = va_arg(*list, Evas_Smart *);
   MAGIC_CHECK(s, Evas_Smart, MAGIC_SMART);
   return;
   MAGIC_CHECK_END();
   unsigned int i;
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
   obj->smart.smart = s;
   obj->type = s->smart_class->name;
   evas_object_smart_use(s);

   _evas_smart_class_ifaces_private_data_alloc(eo_obj, s);

   for (i = 0; i < s->interfaces.size; i++)
     {
        const Evas_Smart_Interface *iface;

        iface = s->interfaces.array[i];
        if (iface->add)
          {
             if (!iface->add(eo_obj))
               {
                  ERR("failed to create interface %s\n", iface->name);
                  evas_object_del(eo_obj);
                  return;
               }
          }
     }

   eo_do(eo_obj, evas_obj_smart_add());
}

static int
_callback_priority_cmp(const void *_a, const void *_b)
{
   const Evas_Smart_Callback *a, *b;
   a = (const Evas_Smart_Callback *) _a;
   b = (const Evas_Smart_Callback *) _b;
   if (a->priority < b->priority)
      return -1;
   else
      return 1;
}

EAPI void
evas_object_smart_callback_add(Evas_Object *eo_obj, const char *event, Evas_Smart_Cb func, const void *data)
{
   evas_object_smart_callback_priority_add(eo_obj, event,
         EVAS_CALLBACK_PRIORITY_DEFAULT, func, data);
}

EAPI void
evas_object_smart_callback_priority_add(Evas_Object *eo_obj, const char *event, Evas_Callback_Priority priority, Evas_Smart_Cb func, const void *data)
{
   Evas_Object_Smart *o;
   Evas_Smart_Callback *cb;

   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = eo_data_get(eo_obj, MY_CLASS);
   if (!event) return;
   if (!func) return;
   EVAS_MEMPOOL_INIT(_mp_cb, "evas_smart_callback", Evas_Smart_Callback, 32, );
   cb = EVAS_MEMPOOL_ALLOC(_mp_cb, Evas_Smart_Callback);
   if (!cb) return;
   EVAS_MEMPOOL_PREP(_mp_cb, cb, Evas_Smart_Callback);
   cb->event = eina_stringshare_add(event);
   cb->func = func;
   cb->func_data = (void *)data;
   cb->priority = priority;
   o->callbacks = eina_list_sorted_insert(o->callbacks, _callback_priority_cmp,
         cb);
}

EAPI void *
evas_object_smart_callback_del(Evas_Object *eo_obj, const char *event, Evas_Smart_Cb func)
{
   Evas_Object_Smart *o;
   Eina_List *l;
   Evas_Smart_Callback *cb;

   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   o = eo_data_get(eo_obj, MY_CLASS);

   if (!event) return NULL;
   EINA_LIST_FOREACH(o->callbacks, l, cb)
     {
        if ((!strcmp(cb->event, event)) && (cb->func == func))
          {
             void *data;

             data = cb->func_data;
             cb->delete_me = 1;
             o->deletions_waiting = 1;
             evas_object_smart_callbacks_clear(eo_obj);
             return data;
          }
     }
   return NULL;
}

EAPI void *
evas_object_smart_callback_del_full(Evas_Object *eo_obj, const char *event, Evas_Smart_Cb func, const void *data)
{
   Evas_Object_Smart *o;
   Eina_List *l;
   Evas_Smart_Callback *cb;

   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   o = eo_data_get(eo_obj, MY_CLASS);
   if (!event) return NULL;
   EINA_LIST_FOREACH(o->callbacks, l, cb)
     {
        if ((!strcmp(cb->event, event)) && (cb->func == func) && (cb->func_data == data))
          {
             void *ret;

             ret = cb->func_data;
             cb->delete_me = 1;
             o->deletions_waiting = 1;
             evas_object_smart_callbacks_clear(eo_obj);
             return ret;
          }
     }
   return NULL;
}

EAPI void
evas_object_smart_callback_call(Evas_Object *eo_obj, const char *event, void *event_info)
{
   Evas_Object_Smart *o;
   Eina_List *l;
   Evas_Smart_Callback *cb;
   const char *strshare;

   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = eo_data_get(eo_obj, MY_CLASS);
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);

   if (!event) return;
   if (obj->delete_me) return;
   o->walking_list++;
   strshare = eina_stringshare_add(event);
   EINA_LIST_FOREACH(o->callbacks, l, cb)
     {
        if (!cb->delete_me)
          {
             if (cb->event == strshare)
               cb->func(cb->func_data, eo_obj, event_info);
          }
        if (obj->delete_me)
          break;
     }
   eina_stringshare_del(strshare);
   o->walking_list--;
   evas_object_smart_callbacks_clear(eo_obj);

   const _Evas_Event_Description *event_desc = eina_hash_find(signals_hash_table, event);
   if (event_desc)
      eo_do(eo_obj, eo_event_callback_call(event_desc->eo_desc, event_info, NULL));
}

EAPI Eina_Bool
evas_object_smart_callbacks_descriptions_set(Evas_Object *eo_obj, const Evas_Smart_Cb_Description *descriptions)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return EINA_FALSE;
   MAGIC_CHECK_END();
   Eina_Bool result = EINA_FALSE;
   eo_do(eo_obj, evas_obj_smart_callbacks_descriptions_set(descriptions, &result));
   return result;
}

static void
_smart_callbacks_descriptions_set(Eo *eo_obj EINA_UNUSED, void *_pd, va_list *list)
{
   const Evas_Smart_Cb_Description *descriptions = va_arg(*list, Evas_Smart_Cb_Description *);
   Eina_Bool *result = va_arg(*list, Eina_Bool *);
   if (result) *result = EINA_TRUE;

   const Evas_Smart_Cb_Description *d;
   Evas_Object_Smart *o = _pd;
   unsigned int i, count = 0;

   if ((!descriptions) || (!descriptions->name))
     {
        evas_smart_cb_descriptions_resize(&o->callbacks_descriptions, 0);
        return;
     }

   for (count = 0, d = descriptions; d->name; d++)
     count++;

   evas_smart_cb_descriptions_resize(&o->callbacks_descriptions, count);
   if (count == 0) return;

   for (i = 0, d = descriptions; i < count; d++, i++)
     o->callbacks_descriptions.array[i] = d;

   evas_smart_cb_descriptions_fix(&o->callbacks_descriptions);
}

EAPI void
evas_object_smart_callbacks_descriptions_get(const Evas_Object *eo_obj, const Evas_Smart_Cb_Description ***class_descriptions, unsigned int *class_count, const Evas_Smart_Cb_Description ***instance_descriptions, unsigned int *instance_count)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   if (class_descriptions) *class_descriptions = NULL;
   if (class_count) *class_count = 0;
   if (instance_descriptions) *instance_descriptions = NULL;
   if (instance_count) *instance_count = 0;
   return;
   MAGIC_CHECK_END();
   eo_do((Eo *)eo_obj, evas_obj_smart_callbacks_descriptions_get(class_descriptions, class_count, instance_descriptions, instance_count));
}

static void
_smart_callbacks_descriptions_get(Eo *eo_obj, void *_pd, va_list *list)
{
   const Evas_Smart_Cb_Description ***class_descriptions = va_arg(*list, const Evas_Smart_Cb_Description ***);
   unsigned int *class_count = va_arg(*list, unsigned int *);
   const Evas_Smart_Cb_Description ***instance_descriptions = va_arg(*list, const Evas_Smart_Cb_Description ***);
   unsigned int *instance_count = va_arg(*list, unsigned int *);
   const Evas_Object_Smart *o = _pd;
   if (class_descriptions) *class_descriptions = NULL;
   if (class_count) *class_count = 0;

   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
   if (obj->smart.smart && class_descriptions)
      *class_descriptions = obj->smart.smart->callbacks.array;
   if (obj->smart.smart && class_count)
     *class_count = obj->smart.smart->callbacks.size;

   if (instance_descriptions)
     *instance_descriptions = o->callbacks_descriptions.array;
   if (instance_count)
     *instance_count = o->callbacks_descriptions.size;
}

EAPI void
evas_object_smart_callback_description_find(const Evas_Object *eo_obj, const char *name, const Evas_Smart_Cb_Description **class_description, const Evas_Smart_Cb_Description **instance_description)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   if (class_description) *class_description = NULL;
   if (instance_description) *instance_description = NULL;
   return;
   MAGIC_CHECK_END();
   eo_do((Eo *)eo_obj, evas_obj_smart_callback_description_find(name, class_description, instance_description));
}

static void
_smart_callback_description_find(Eo *eo_obj, void *_pd, va_list *list)
{
   const char *name = va_arg(*list, const char *);
   const Evas_Smart_Cb_Description **class_description = va_arg(*list, const Evas_Smart_Cb_Description **);
   const Evas_Smart_Cb_Description **instance_description = va_arg(*list, const Evas_Smart_Cb_Description **);

   if (!name)
     {
        if (class_description) *class_description = NULL;
        if (instance_description) *instance_description = NULL;
        return;
     }

   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
   const Evas_Object_Smart *o = _pd;
   if (class_description)
     *class_description = evas_smart_cb_description_find
        (&obj->smart.smart->callbacks, name);

   if (instance_description)
     *instance_description = evas_smart_cb_description_find
        (&o->callbacks_descriptions, name);
}

EAPI void
evas_object_smart_need_recalculate_set(Evas_Object *eo_obj, Eina_Bool value)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   eo_do(eo_obj, evas_obj_smart_need_recalculate_set(value));
}

static void
_smart_need_recalculate_set(Eo *eo_obj, void *_pd, va_list *list)
{
   Eina_Bool value = va_arg(*list, int);
   Evas_Object_Smart *o = _pd;
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);

   // XXX: do i need this?
   if (obj->delete_me) return;

   /* remove this entry from calc_list or processed list */
   if (eina_clist_element_is_linked(&obj->calc_entry))
     eina_clist_remove(&obj->calc_entry);

   value = !!value;
   if (value)
     eina_clist_add_tail(&obj->layer->evas->calc_list, &obj->calc_entry);
   else
     eina_clist_add_tail(&obj->layer->evas->calc_done, &obj->calc_entry);

   if (o->need_recalculate == value) return;

   if (obj->recalculate_cycle > 254)
     {
        ERR("Object %p is not stable during recalc loop", obj);
        return;
     }
   if (obj->layer->evas->in_smart_calc) obj->recalculate_cycle++;
   o->need_recalculate = value;
}

EAPI Eina_Bool
evas_object_smart_need_recalculate_get(const Evas_Object *eo_obj)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return EINA_FALSE;
   MAGIC_CHECK_END();
   Eina_Bool need_recalculate = EINA_FALSE;
   eo_do((Eo *)eo_obj, evas_obj_smart_need_recalculate_get(&need_recalculate));
   return need_recalculate;
}

static void
_smart_need_recalculate_get(Eo *eo_obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *need_recalculate = va_arg(*list, Eina_Bool *);
   const Evas_Object_Smart *o = _pd;
   if (need_recalculate) *need_recalculate = o->need_recalculate;
}

EAPI void
evas_object_smart_calculate(Evas_Object *eo_obj)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   eo_do(eo_obj, evas_obj_smart_calculate());
}

static void
_smart_calculate(Eo *eo_obj, void *_pd, va_list *list EINA_UNUSED)
{
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
   Evas_Object_Smart *o = _pd;

   if (!obj->smart.smart || !obj->smart.smart->smart_class->calculate)
     return;

   o->need_recalculate = 0;
   obj->smart.smart->smart_class->calculate(eo_obj);
}

EAPI void
evas_smart_objects_calculate(Evas *eo_e)
{
   eo_do(eo_e, evas_canvas_smart_objects_calculate());
}

void
_canvas_smart_objects_calculate(Eo *eo_e, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   evas_call_smarts_calculate(eo_e);
}

EAPI int
evas_smart_objects_calculate_count_get(const Evas *eo_e)
{
   MAGIC_CHECK(eo_e, Evas, MAGIC_EVAS);
   return 0;
   MAGIC_CHECK_END();
   int ret = 0;
   eo_do((Eo *)eo_e, evas_canvas_smart_objects_calculate_count_get(&ret));
   return ret;
}

void
_canvas_smart_objects_calculate_count_get(Eo *eo_e EINA_UNUSED, void *_pd, va_list *list)
{
   int *ret = va_arg(*list, int *);
   const Evas_Public_Data *e = _pd;
   *ret = e->smart_calc_count;
}

/**
 * Call calculate() on all smart objects that need_recalculate.
 *
 * @internal
 */
void
evas_call_smarts_calculate(Evas *eo_e)
{
   Eina_Clist *elem;
   Evas_Object_Protected_Data *obj;
   Evas_Public_Data *e = eo_data_get(eo_e, EVAS_CLASS);

//   printf("+CAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAALC-----------v\n");
   evas_event_freeze(eo_e);
   e->in_smart_calc++;

   while (NULL != (elem = eina_clist_head(&e->calc_list)))
     {
        Evas_Object_Smart *o;

        /* move the item to the processed list */
        obj = EINA_CLIST_ENTRY(elem, Evas_Object_Protected_Data, calc_entry);
        eina_clist_remove(&obj->calc_entry);
        if (obj->delete_me) continue;
        eina_clist_add_tail(&e->calc_done, &obj->calc_entry);

        o = eo_data_get(obj->object, MY_CLASS);

        if (o->need_recalculate)
          {
             o->need_recalculate = 0;
	     if (obj->smart.smart && obj->smart.smart->smart_class->calculate)
               obj->smart.smart->smart_class->calculate(obj->object);
             else
               eo_do(obj->object, evas_obj_smart_calculate());
          }
     }

   while (NULL != (elem = eina_clist_head(&e->calc_done)))
     {
        obj = EINA_CLIST_ENTRY(elem, Evas_Object_Protected_Data, calc_entry);
        obj->recalculate_cycle = 0;
        eina_clist_remove(&obj->calc_entry);
     }

   e->in_smart_calc--;
   if (e->in_smart_calc == 0) e->smart_calc_count++;
   evas_event_thaw(eo_e);
   evas_event_thaw_eval(eo_e);
}

EAPI void
evas_object_smart_changed(Evas_Object *eo_obj)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   eo_do(eo_obj, evas_obj_smart_changed());
}

static void
_smart_changed(Eo *eo_obj, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
   evas_object_change(eo_obj, obj);
   eo_do(eo_obj, evas_obj_smart_need_recalculate_set(1));
}

/* internal calls */
static void
evas_object_smart_callbacks_clear(Evas_Object *eo_obj)
{
   Evas_Object_Smart *o;
   Eina_List *l;
   Evas_Smart_Callback *cb;

   o = eo_data_get(eo_obj, MY_CLASS);

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
evas_object_smart_del(Evas_Object *eo_obj)
{
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
   Evas_Smart *s;
   unsigned int i;

   if (obj->delete_me) return;
   s = obj->smart.smart;
   if (s && s->smart_class->del)
      s->smart_class->del(eo_obj);
   else
      eo_do(eo_obj, evas_obj_smart_del());
   if (obj->smart.parent) evas_object_smart_member_del(eo_obj);

   if (s)
     {
        for (i = 0; i < s->interfaces.size; i++)
          {
             const Evas_Smart_Interface *iface;

             iface = s->interfaces.array[i];
             if (iface->del) iface->del(eo_obj);
          }
     }

   free(obj->interface_privates);
   obj->interface_privates = NULL;

   if (s) evas_object_smart_unuse(s);
}

void
evas_object_smart_cleanup(Evas_Object *eo_obj)
{
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);

   if (obj->smart.parent)
     evas_object_smart_member_del(eo_obj);

   if (obj->is_smart)
     {
        Evas_Object_Smart *o = eo_data_get(eo_obj, MY_CLASS);
        if (obj->calc_entry.next)
          eina_clist_remove(&obj->calc_entry);

        while (o->contained)
          {
             Evas_Object *contained_obj = ((Evas_Object_Protected_Data *)o->contained)->object;
             evas_object_smart_member_del(contained_obj);
          }

        while (o->callbacks)
          {
             Evas_Smart_Callback *cb = o->callbacks->data;
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
evas_object_smart_member_cache_invalidate(Evas_Object *eo_obj,
                                          Eina_Bool pass_events,
                                          Eina_Bool freeze_events)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();

   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
   Evas_Object_Protected_Data *member;

   if (pass_events)
     obj->parent_cache.pass_events_valid = EINA_FALSE;
   if (freeze_events)
     obj->parent_cache.freeze_events_valid = EINA_FALSE;

   if (!obj->is_smart) return;
   Evas_Object_Smart *o = eo_data_get(eo_obj, MY_CLASS);
   EINA_INLIST_FOREACH(o->contained, member)
     {
        Evas_Object *eo_member = member->object;
        evas_object_smart_member_cache_invalidate(eo_member,
                                               pass_events,
                                               freeze_events);
     }
}

void
evas_object_smart_member_raise(Evas_Object *eo_member)
{
   Evas_Object_Smart *o;
   Evas_Object_Protected_Data *member = eo_data_get(eo_member, EVAS_OBJ_CLASS);
   o = eo_data_get(member->smart.parent, MY_CLASS);
   o->contained = eina_inlist_demote(o->contained, EINA_INLIST_GET(member));
}

void
evas_object_smart_member_lower(Evas_Object *eo_member)
{
   Evas_Object_Smart *o;
   Evas_Object_Protected_Data *member = eo_data_get(eo_member, EVAS_OBJ_CLASS);
   o = eo_data_get(member->smart.parent, MY_CLASS);
   o->contained = eina_inlist_promote(o->contained, EINA_INLIST_GET(member));
}

void
evas_object_smart_member_stack_above(Evas_Object *eo_member, Evas_Object *eo_other)
{
   Evas_Object_Smart *o;
   Evas_Object_Protected_Data *member = eo_data_get(eo_member, EVAS_OBJ_CLASS);
   Evas_Object_Protected_Data *other = eo_data_get(eo_other, EVAS_OBJ_CLASS);
   o = eo_data_get(member->smart.parent, MY_CLASS);
   o->contained = eina_inlist_remove(o->contained, EINA_INLIST_GET(member));
   o->contained = eina_inlist_append_relative(o->contained, EINA_INLIST_GET(member), EINA_INLIST_GET(other));
}

void
evas_object_smart_member_stack_below(Evas_Object *eo_member, Evas_Object *eo_other)
{
   Evas_Object_Smart *o;
   Evas_Object_Protected_Data *member = eo_data_get(eo_member, EVAS_OBJ_CLASS);
   Evas_Object_Protected_Data *other = eo_data_get(eo_other, EVAS_OBJ_CLASS);
   o = eo_data_get(member->smart.parent, MY_CLASS);
   o->contained = eina_inlist_remove(o->contained, EINA_INLIST_GET(member));
   o->contained = eina_inlist_prepend_relative(o->contained, EINA_INLIST_GET(member), EINA_INLIST_GET(other));
}

void
evas_object_smart_need_bounding_box_update(Evas_Object *eo_obj)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
   Evas_Object_Smart *o = eo_data_get(eo_obj, MY_CLASS);

   if (o->update_boundingbox_needed) return ;
   o->update_boundingbox_needed = EINA_TRUE;

   if (obj->smart.parent) evas_object_smart_need_bounding_box_update(obj->smart.parent);
}

void
evas_object_smart_bouding_box_update(Evas_Object *eo_obj)
{
   Eina_Inlist *list;
   Evas_Object_Protected_Data *o;
   Evas_Coord minx;
   Evas_Coord miny;
   Evas_Coord maxw = 0;
   Evas_Coord maxh = 0;

   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
   Evas_Object_Smart *os = eo_data_get(eo_obj, MY_CLASS);

   if (!os->update_boundingbox_needed) return ;
   os->update_boundingbox_needed = EINA_FALSE;

   minx = obj->layer->evas->output.w;
   miny = obj->layer->evas->output.h;

   list = os->contained;
   EINA_INLIST_FOREACH(list, o)
     {
        Evas_Coord tx;
        Evas_Coord ty;
        Evas_Coord tw;
        Evas_Coord th;

        if (o == obj) continue ;
        if (o->clip.clipees || o->is_static_clip) continue ;

        if (eo_isa(o->object, EVAS_OBJ_SMART_CLASS))
          {
             evas_object_smart_bouding_box_update(o->object);

             tx = o->cur.bounding_box.x;
             ty = o->cur.bounding_box.y;
             tw = o->cur.bounding_box.x + o->cur.bounding_box.w;
             th = o->cur.bounding_box.y + o->cur.bounding_box.h;
          }
        else
          {
             tx = o->cur.geometry.x;
             ty = o->cur.geometry.y;
             tw = o->cur.geometry.x + o->cur.geometry.w;
             th = o->cur.geometry.y + o->cur.geometry.h;
          }

        if (tx < minx) minx = tx;
        if (ty < miny) miny = ty;
        if (tw > maxw) maxw = tw;
        if (th > maxh) maxh = th;
     }

   if (minx != obj->cur.bounding_box.x)
     {
        obj->cur.bounding_box.w += obj->cur.bounding_box.x - minx;
        obj->cur.bounding_box.x = minx;
     }

   if (miny != obj->cur.bounding_box.y)
     {
        obj->cur.bounding_box.h += obj->cur.bounding_box.y - miny;
        obj->cur.bounding_box.y = miny;
     }

   if (maxw != obj->cur.bounding_box.x + obj->cur.bounding_box.w)
     {
        obj->cur.bounding_box.w = maxw - obj->cur.bounding_box.x;
     }

   if (maxh != obj->cur.bounding_box.y + obj->cur.bounding_box.h)
     {
        obj->cur.bounding_box.h = maxh - obj->cur.bounding_box.y;
     }
}

/* all nice and private */
static void
evas_object_smart_init(Evas_Object *eo_obj)
{
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
   obj->is_smart = EINA_TRUE;
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

static void
evas_object_smart_render(Evas_Object *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj EINA_UNUSED, void *output EINA_UNUSED, void *context EINA_UNUSED, void *surface EINA_UNUSED, int x EINA_UNUSED, int y EINA_UNUSED)
{
   return;
}

static void
evas_object_smart_render_pre(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj)
{
   Evas_Public_Data *e;

   if (obj->pre_render_done) return;
   if (!obj->child_has_map && !obj->cur.cached_surface)
     {
#if 0
        Evas_Object_Smart *o;

        fprintf(stderr, "");
        o = (Evas_Object_Smart *)(obj->object_data);
        if (/* o->member_count > 1 && */
            obj->cur.bounding_box.w == obj->prev.bounding_box.w &&
            obj->cur.bounding_box.h == obj->prev.bounding_box.h &&
            (obj->cur.bounding_box.x != obj->prev.bounding_box.x ||
             obj->cur.bounding_box.y != obj->prev.bounding_box.y))
          {
             Eina_Bool cache_map = EINA_FALSE;

             /* Check parent speed */
             /* - same speed => do not map this object */
             /* - different speed => map this object */
             /* - if parent is mapped then map this object */

             if (!obj->smart.parent || obj->smart.parent->child_has_map)
               {
                  cache_map = EINA_TRUE;
               }
             else
               {
                  if (_evas_render_has_map(obj->smart.parent))
                    {
                       cache_map = EINA_TRUE;
                    }
                  else
                    {
                       int speed_x, speed_y;
                       int speed_px, speed_py;

                       speed_x = obj->cur.geometry.x - obj->prev.geometry.x;
                       speed_y = obj->cur.geometry.y - obj->prev.geometry.y;

                       speed_px = obj->smart.parent->cur.geometry.x - obj->smart.parent->prev.geometry.x;
                       speed_py = obj->smart.parent->cur.geometry.y - obj->smart.parent->prev.geometry.y;

                       /* speed_x = obj->cur.bounding_box.x - obj->prev.bounding_box.x; */
                       /* speed_y = obj->cur.bounding_box.y - obj->prev.bounding_box.y; */

                       /* speed_px = obj->smart.parent->cur.bounding_box.x - obj->smart.parent->prev.bounding_box.x; */
                       /* speed_py = obj->smart.parent->cur.bounding_box.y - obj->smart.parent->prev.bounding_box.y; */

                       fprintf(stderr, "speed: '%s',%p (%i, %i) vs '%s',%p (%i, %i)\n",
                               evas_object_type_get(eo_obj), obj, speed_x, speed_y,
                               evas_object_type_get(obj->smart.parent), obj->smart.parent, speed_px, speed_py);

                       if (speed_x != speed_px || speed_y != speed_py)
                         cache_map = EINA_TRUE;
                    }
               }

             if (cache_map)
               fprintf(stderr, "Wouhou, I can detect moving smart object (%s, %p [%i, %i, %i, %i] < %s, %p [%i, %i, %i, %i])\n",
                       evas_object_type_get(eo_obj), obj,
                       obj->cur.bounding_box.x - obj->prev.bounding_box.x,
                       obj->cur.bounding_box.y - obj->prev.bounding_box.y,
                       obj->cur.bounding_box.w, obj->cur.bounding_box.h,
                       evas_object_type_get(obj->smart.parent), obj->smart.parent,
                       obj->smart.parent->cur.bounding_box.x - obj->smart.parent->prev.bounding_box.x,
                       obj->smart.parent->cur.bounding_box.y - obj->smart.parent->prev.bounding_box.y,
                       obj->smart.parent->cur.bounding_box.w, obj->smart.parent->cur.bounding_box.h);

             obj->cur.cached_surface = cache_map;
          }
#endif
     }

   e = obj->layer->evas;

   if (obj->changed_map)
     {
        evas_object_render_pre_prev_cur_add(&e->clip_changes, eo_obj, obj);
     }

   obj->pre_render_done = EINA_TRUE;
}

static void
evas_object_smart_render_post(Evas_Object *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj EINA_UNUSED)
{
   evas_object_cur_prev(eo_obj);
}

static unsigned int evas_object_smart_id_get(Evas_Object *eo_obj)
{
   Evas_Object_Smart *o = eo_data_get(eo_obj, MY_CLASS);
   if (!o) return 0;
   return MAGIC_OBJ_SMART;
}

static unsigned int evas_object_smart_visual_id_get(Evas_Object *eo_obj)
{
   Evas_Object_Smart *o = eo_data_get(eo_obj, MY_CLASS);
   if (!o) return 0;
   return MAGIC_OBJ_CONTAINER;
}

static void *evas_object_smart_engine_data_get(Evas_Object *eo_obj)
{
   Evas_Object_Smart *o = eo_data_get(eo_obj, MY_CLASS);
   if (!o) return NULL;
   return o->engine_data;
}

static void
_class_constructor(Eo_Class *klass)
{
   const Eo_Op_Func_Description func_desc[] = {
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_CONSTRUCTOR), _constructor),
        EO_OP_FUNC(EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_TYPE_CHECK), _type_check),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_DATA_SET), _smart_data_set),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_SMART_GET), _smart_smart_get),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_MEMBER_ADD), _smart_member_add),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_MEMBER_DEL), _smart_member_del),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_MEMBERS_GET), _smart_members_get),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_CALLBACKS_DESCRIPTIONS_SET), _smart_callbacks_descriptions_set),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_CALLBACKS_DESCRIPTIONS_GET), _smart_callbacks_descriptions_get),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_CALLBACK_DESCRIPTION_FIND), _smart_callback_description_find),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_NEED_RECALCULATE_SET), _smart_need_recalculate_set),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_NEED_RECALCULATE_GET), _smart_need_recalculate_get),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_CALCULATE), _smart_calculate),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_CHANGED), _smart_changed),
        EO_OP_FUNC(EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_SMART_DATA_GET), _smart_data_get),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_ATTACH), _smart_attach),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_ADD), _smart_add),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_DEL), _smart_del),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_RESIZE), _smart_resize),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_MOVE), _smart_move),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_SHOW), _smart_show),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_HIDE), _smart_hide),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_COLOR_SET), _smart_color_set),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_CLIP_SET), _smart_clip_set),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_CLIP_UNSET), _smart_clip_unset),
        EO_OP_FUNC_SENTINEL
   };

   eo_class_funcs_set(klass, func_desc);
}

static const Eo_Op_Description op_desc[] = {
     EO_OP_DESCRIPTION(EVAS_OBJ_SMART_SUB_ID_DATA_SET, "Store a pointer to user data for a given smart object."),
     EO_OP_DESCRIPTION(EVAS_OBJ_SMART_SUB_ID_SMART_GET, "Get the #Evas_Smart from which obj smart object was created."),
     EO_OP_DESCRIPTION(EVAS_OBJ_SMART_SUB_ID_MEMBER_ADD, "Set an Evas object as a member of a given smart object."),
     EO_OP_DESCRIPTION(EVAS_OBJ_SMART_SUB_ID_MEMBER_DEL, "Removes a member object from a given smart object."),
     EO_OP_DESCRIPTION(EVAS_OBJ_SMART_SUB_ID_MEMBERS_GET, "Retrieves the list of the member objects of a given Evas smart"),
     EO_OP_DESCRIPTION(EVAS_OBJ_SMART_SUB_ID_CALLBACKS_DESCRIPTIONS_SET, "Set an smart object instance's smart callbacks descriptions."),
     EO_OP_DESCRIPTION(EVAS_OBJ_SMART_SUB_ID_CALLBACKS_DESCRIPTIONS_GET, "Retrieve an smart object's know smart callback descriptions (both"),
     EO_OP_DESCRIPTION(EVAS_OBJ_SMART_SUB_ID_CALLBACK_DESCRIPTION_FIND, "Find callback description for callback called name."),
     EO_OP_DESCRIPTION(EVAS_OBJ_SMART_SUB_ID_NEED_RECALCULATE_SET, "Set or unset the flag signalling that a given smart object needs to"),
     EO_OP_DESCRIPTION(EVAS_OBJ_SMART_SUB_ID_NEED_RECALCULATE_GET, "Get the value of the flag signalling that a given smart object needs to"),
     EO_OP_DESCRIPTION(EVAS_OBJ_SMART_SUB_ID_CALCULATE, "Call the calculate() smart function immediately on a given smart"),
     EO_OP_DESCRIPTION(EVAS_OBJ_SMART_SUB_ID_CHANGED, "Mark smart object as changed, dirty."),
     EO_OP_DESCRIPTION(EVAS_OBJ_SMART_SUB_ID_ATTACH, "Attach an object to a canvas."),
     EO_OP_DESCRIPTION(EVAS_OBJ_SMART_SUB_ID_ADD, "Add an object to a canvas."),
     EO_OP_DESCRIPTION(EVAS_OBJ_SMART_SUB_ID_DEL, "Remove an object from a canvas."),
     EO_OP_DESCRIPTION(EVAS_OBJ_SMART_SUB_ID_RESIZE, "Resize an object on a canvas."),
     EO_OP_DESCRIPTION(EVAS_OBJ_SMART_SUB_ID_MOVE, "Move an object on a canvas."),
     EO_OP_DESCRIPTION(EVAS_OBJ_SMART_SUB_ID_SHOW, "Show an object on a canvas."),
     EO_OP_DESCRIPTION(EVAS_OBJ_SMART_SUB_ID_HIDE, "Hide an object on a canvas."),
     EO_OP_DESCRIPTION(EVAS_OBJ_SMART_SUB_ID_COLOR_SET, "Set color of an object on a canvas."),
     EO_OP_DESCRIPTION(EVAS_OBJ_SMART_SUB_ID_CLIP_SET, "Set a clipper of an object on a canvas."),
     EO_OP_DESCRIPTION(EVAS_OBJ_SMART_SUB_ID_CLIP_UNSET, "Unset a clipper of an object on a canvas."),
     EO_OP_DESCRIPTION_SENTINEL
};

static const Eo_Class_Description class_desc = {
     EO_VERSION,
     MY_CLASS_NAME,
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(&EVAS_OBJ_SMART_BASE_ID, op_desc, EVAS_OBJ_SMART_SUB_ID_LAST),
     NULL,
     sizeof(Evas_Object_Smart),
     _class_constructor,
     NULL
};

EO_DEFINE_CLASS(evas_object_smart_class_get, &class_desc, EVAS_OBJ_CLASS, EVAS_SMART_SIGNAL_INTERFACE, NULL);

