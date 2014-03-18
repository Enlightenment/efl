#include "evas_common_private.h"
#include "evas_private.h"

#include "Eo.h"

#define MY_CLASS EVAS_OBJ_SMART_CLASS

#define MY_CLASS_NAME "Evas_Smart"
#define MY_CLASS_NAME_LEGACY "Evas_Object_Smart"

extern Eina_Hash* signals_hash_table;

static Eina_Hash *_evas_smart_class_names_hash_table = NULL;

typedef struct _Evas_Smart_Data      Evas_Smart_Data;

struct _Evas_Smart_Data
{
   struct {
      Evas_Coord_Rectangle bounding_box;
   } cur, prev;
   Evas_Object      *object;
   void             *engine_data;
   void             *data;
   Eina_Inlist      *callbacks;
   Eina_Inlist      *contained; /** list of smart member objects */

  /* ptr array + data blob holding all interfaces private data for
   * this object */
   void            **interface_privates;
   Eina_Clist        calc_entry;

   Evas_Smart_Cb_Description_Array callbacks_descriptions;

   int               walking_list;
   int               member_count; /** number of smart member objects */

   unsigned char     recalculate_cycle;

   Eina_Bool         deletions_waiting : 1;
   Eina_Bool         need_recalculate : 1;
   Eina_Bool         update_boundingbox_needed : 1;
};

typedef struct
{
   EINA_INLIST;
   Evas_Smart_Cb func;
   void *data;
   _Evas_Event_Description *desc;
} _eo_evas_smart_cb_info;


typedef struct _Evas_Object_Smart_Iterator Evas_Object_Smart_Iterator;
struct _Evas_Object_Smart_Iterator
{
   Eina_Iterator iterator;

   const Eina_Inlist *current;
   Evas_Object *parent;
};

static Eina_Bool
_eo_evas_smart_cb(void *data, Eo *eo_obj, const Eo_Event_Description *desc EINA_UNUSED, void *event_info)
{
   _eo_evas_smart_cb_info *info = data;
   if (info->func) info->func(info->data, eo_obj, event_info);
   return EINA_TRUE;
}

/* private methods for smart objects */
static void evas_object_smart_init(Evas_Object *eo_obj);
static void evas_object_smart_render(Evas_Object *eo_obj,
				     Evas_Object_Protected_Data *obj,
				     void *type_private_data,
				     void *output, void *context, void *surface,
				     int x, int y, Eina_Bool do_async);
static void evas_object_smart_render_pre(Evas_Object *eo_obj,
					 Evas_Object_Protected_Data *obj,
					 void *type_private_data);
static void evas_object_smart_render_post(Evas_Object *eo_obj,
					  Evas_Object_Protected_Data *obj,
					  void *type_private_data);

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


/* public funcs */
EOLIAN static void
_evas_smart_data_set(Eo *eo_obj EINA_UNUSED, Evas_Smart_Data *o, void *data)
{
   if (o->data) eo_data_unref(eo_obj, o->data);
   o->data = data;
   eo_data_ref(eo_obj, NULL);
}

EOLIAN static void *
_evas_smart_evas_object_smart_data_get(Eo *eo_obj EINA_UNUSED, Evas_Smart_Data *o)
{
   return o->data;
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
   Evas_Smart_Data *obj;
   Evas_Smart *s;

   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();

   obj = eo_data_scope_get(eo_obj, MY_CLASS);
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

EOLIAN static Evas_Smart*
_evas_smart_smart_get(Eo *eo_obj EINA_UNUSED, Evas_Smart_Data *o EINA_UNUSED)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJ_CLASS);
   return obj->smart.smart;
}

EAPI void
evas_object_smart_member_add(Evas_Object *eo_obj, Evas_Object *smart_obj)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   eo_do(smart_obj, evas_obj_smart_member_add(eo_obj));
}

EOLIAN static void
_evas_smart_member_add(Eo *smart_obj, Evas_Smart_Data *o, Evas_Object *eo_obj)
{

   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJ_CLASS);
   Evas_Object_Protected_Data *smart = eo_data_scope_get(smart_obj, EVAS_OBJ_CLASS);

   if (obj->delete_me)
     {
        CRI("Adding deleted object %p to smart obj %p", eo_obj, smart_obj);
        abort();
        return;
     }
   if (smart->delete_me)
     {
        CRI("Adding object %p to deleted smart obj %p", eo_obj, smart_obj);
        abort();
        return;
     }
   if (!smart->layer)
     {
        CRI("No evas surface associated with smart object (%p)", smart_obj);
        abort();
        return;
     }
   if ((obj->layer && smart->layer) &&
       (obj->layer->evas != smart->layer->evas))
     {
        CRI("Adding object %p from Evas (%p) from another Evas (%p)", eo_obj, obj->layer->evas, smart->layer->evas);
        abort();
        return;
     }

   if (obj->smart.parent == smart_obj) return;

   if (obj->smart.parent) evas_object_smart_member_del(eo_obj);

   o->member_count++;
   evas_object_release(eo_obj, obj, 1);
   obj->layer = smart->layer;
   EINA_COW_STATE_WRITE_BEGIN(obj, state_write, cur)
     {
       state_write->layer = obj->layer->layer;
     }
   EINA_COW_STATE_WRITE_END(obj, state_write, cur);

   obj->layer->usage++;
   obj->smart.parent = smart_obj;
   o->contained = eina_inlist_append(o->contained, EINA_INLIST_GET(obj));
   eo_data_ref(eo_obj, NULL);
   evas_object_smart_member_cache_invalidate(eo_obj, EINA_TRUE, EINA_TRUE,
                                             EINA_TRUE);
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
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJ_CLASS);
   if (!obj) return;
   if (!obj->smart.parent) return;
   Evas_Object *smart_obj = obj->smart.parent;

   eo_do(smart_obj, evas_obj_smart_member_del(eo_obj));
}

EOLIAN static void
_evas_smart_member_del(Eo *smart_obj, Evas_Smart_Data *_pd EINA_UNUSED, Evas_Object *eo_obj)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJ_CLASS);

   if (!obj->smart.parent) return;

   Evas_Object_Protected_Data *smart = eo_data_scope_get(smart_obj, EVAS_OBJ_CLASS);
   if (smart->smart.smart && smart->smart.smart->smart_class->member_del)
     smart->smart.smart->smart_class->member_del(smart_obj, eo_obj);

   Evas_Smart_Data *o = eo_data_scope_get(smart_obj, MY_CLASS);
   o->contained = eina_inlist_remove(o->contained, EINA_INLIST_GET(obj));
   eo_data_unref(eo_obj, obj);
   o->member_count--;
   obj->smart.parent = NULL;
   evas_object_smart_member_cache_invalidate(eo_obj, EINA_TRUE, EINA_TRUE, EINA_TRUE);
   obj->layer->usage--;

   EINA_COW_STATE_WRITE_BEGIN(obj, state_write, cur)
     {
       state_write->layer = obj->layer->layer;
     }
   EINA_COW_STATE_WRITE_END(obj, state_write, cur);

   evas_object_inject(eo_obj, obj, obj->layer->evas->evas);
   obj->restack = 1;
   evas_object_change(eo_obj, obj);
   evas_object_mapped_clip_across_mark(eo_obj, obj);
}

EOLIAN static Eina_Bool
_evas_smart_evas_object_smart_type_check(Eo *eo_obj, Evas_Smart_Data *o EINA_UNUSED, const char *type)
{
   const Evas_Smart_Class *sc;
   Eo_Class *klass;
   Eina_Bool type_check = EINA_FALSE;

   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJ_CLASS);
   if (!obj) return EINA_FALSE;

   klass = eina_hash_find(_evas_smart_class_names_hash_table, type);
   if (klass) type_check = eo_isa(eo_obj, klass);

   /* Backward compatibility - walk over smart classes and compare type */
   if (EINA_FALSE == type_check)
     {
        if (obj->smart.smart)
          {
             sc = obj->smart.smart->smart_class;
             while (sc)
               {
                  if (!strcmp(sc->name, type)) return EINA_TRUE;
                  sc = sc->parent;
               }
          }
     }

   return type_check;
}

EOLIAN static Eina_Bool
_evas_smart_evas_object_smart_type_check_ptr(Eo *eo_obj, Evas_Smart_Data *o EINA_UNUSED, const char* type)
{
   Eo_Class *klass;
   const Evas_Smart_Class *sc;
   Eina_Bool type_check = EINA_FALSE;

   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJ_CLASS);
   if (!obj) return EINA_FALSE;

   klass = eina_hash_find(_evas_smart_class_names_hash_table, type);
   if (klass) type_check = eo_isa(eo_obj, klass);

   /* Backward compatibility - walk over smart classes and compare type */
   if (EINA_FALSE == type_check)
     {
        if (obj->smart.smart)
          {
             sc = obj->smart.smart->smart_class;
             while (sc)
               {
                  if (sc->name == type) return EINA_TRUE;
                  sc = sc->parent;
               }
          }
     }

   return type_check;
}

EAPI void
evas_smart_legacy_type_register(const char *type, const Eo_Class *klass)
{
   eina_hash_set(_evas_smart_class_names_hash_table, type, klass);
}

static Eina_Bool
_evas_object_smart_iterator_next(Evas_Object_Smart_Iterator *it, void **data)
{
   Evas_Object *eo;

   if (!it->current) return EINA_FALSE;

   eo = ((const Evas_Object_Protected_Data*)(it->current))->object;
   if (data) *data = eo;

   it->current = it->current->next;

   return EINA_TRUE;
}

static Evas_Object *
_evas_object_smart_iterator_get_container(Evas_Object_Smart_Iterator *it)
{
   return it->parent;
}

static void
_evas_object_smart_iterator_free(Evas_Object_Smart_Iterator *it)
{
   eo_unref(it->parent);
   free(it);
}

// Should we have an eo_children_iterator_new API and just inherit from it ?
EOLIAN static Eina_Iterator*
_evas_smart_iterator_new(Eo *o, Evas_Smart_Data *priv)
{
   Evas_Object_Smart_Iterator *it;

   if (!priv->contained) return NULL;

   it = calloc(1, sizeof(Evas_Object_Smart_Iterator));
   if (!it) return NULL;

   EINA_MAGIC_SET(&it->iterator, EINA_MAGIC_ITERATOR);
   it->parent = eo_ref(o);
   it->current = priv->contained;

   it->iterator.next = FUNC_ITERATOR_NEXT(_evas_object_smart_iterator_next);
   it->iterator.get_container = FUNC_ITERATOR_GET_CONTAINER(_evas_object_smart_iterator_get_container);
   it->iterator.free = FUNC_ITERATOR_FREE(_evas_object_smart_iterator_free);

   return &it->iterator;
}

EOLIAN static Eina_List*
_evas_smart_members_get(Eo *eo_obj EINA_UNUSED, Evas_Smart_Data *o)
{
   Eina_List *members;

   Eina_Inlist *member;

   members = NULL;
   for (member = o->contained; member; member = member->next)
     members = eina_list_append(members, ((Evas_Object_Protected_Data *)member)->object);

   return members;
}

const Eina_Inlist *
evas_object_smart_members_get_direct(const Evas_Object *eo_obj)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   if (!eo_isa(eo_obj, MY_CLASS)) return NULL;
   Evas_Smart_Data *o = eo_data_scope_get(eo_obj, MY_CLASS);
   return o->contained;
}

void
_evas_object_smart_members_all_del(Evas_Object *eo_obj)
{
   Evas_Smart_Data *o = eo_data_scope_get(eo_obj, MY_CLASS);
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
   Evas_Smart_Data *obj;
   const Evas_Smart_Class *sc;
   unsigned char *ptr;
   unsigned int i, total_priv_sz = 0;

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

   obj = eo_data_scope_get(eo_obj, MY_CLASS);
   obj->interface_privates = malloc(s->interfaces.size * sizeof(void *) + total_priv_sz);
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

EOLIAN static void
_evas_smart_eo_base_constructor(Eo *eo_obj, Evas_Smart_Data *class_data EINA_UNUSED)
{
   Evas_Object_Protected_Data *obj;
   Evas_Smart_Data *smart;
   Eo *parent;

   smart = class_data;
   smart->object = eo_obj;

   eo_do_super(eo_obj, MY_CLASS, eo_constructor());
   evas_object_smart_init(eo_obj);

   obj = eo_data_scope_get(eo_obj, EVAS_OBJ_CLASS);
   eo_do(eo_obj, eo_parent_get(&parent));
   evas_object_inject(eo_obj, obj, evas_object_evas_get(parent));
   eo_do(eo_obj,
         evas_obj_type_set(MY_CLASS_NAME_LEGACY),
         evas_obj_smart_add());
}

EOLIAN static void
_evas_smart_add(Eo *eo_obj, Evas_Smart_Data *o EINA_UNUSED)
{
   // If this function is reached, so we do nothing except trying to call
   // the function of the legacy smart class.
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJ_CLASS);
   Evas_Smart *s = obj->smart.smart;
   if (s && s->smart_class->add) s->smart_class->add(eo_obj);
}

EOLIAN static void
_evas_smart_del(Eo *eo_obj EINA_UNUSED, Evas_Smart_Data *o EINA_UNUSED)
{
}

EOLIAN static void
_evas_smart_resize(Eo *eo_obj, Evas_Smart_Data *o EINA_UNUSED, Evas_Coord w, Evas_Coord h)
{
   // If this function is reached, so we do nothing except trying to call
   // the function of the legacy smart class.
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJ_CLASS);
   Evas_Smart *s = obj->smart.smart;
   if (s && s->smart_class->resize) s->smart_class->resize(eo_obj, w, h);
}

EOLIAN static void
_evas_smart_move(Eo *eo_obj, Evas_Smart_Data *o EINA_UNUSED, Evas_Coord x, Evas_Coord y)
{
   // If this function is reached, so we do nothing except trying to call
   // the function of the legacy smart class.
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJ_CLASS);
   Evas_Smart *s = obj->smart.smart;
   if (s && s->smart_class->move) s->smart_class->move(eo_obj, x, y);
}

EOLIAN static void
_evas_smart_show(Eo *eo_obj, Evas_Smart_Data *o EINA_UNUSED)
{
   // If this function is reached, so we do nothing except trying to call
   // the function of the legacy smart class.
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJ_CLASS);
   Evas_Smart *s = obj->smart.smart;
   if (s && s->smart_class->show) s->smart_class->show(eo_obj);
}

EOLIAN static void
_evas_smart_hide(Eo *eo_obj, Evas_Smart_Data *o EINA_UNUSED)
{
   // If this function is reached, so we do nothing except trying to call
   // the function of the legacy smart class.
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJ_CLASS);
   Evas_Smart *s = obj->smart.smart;
   if (s && s->smart_class->hide) s->smart_class->hide(eo_obj);
}

EOLIAN static void
_evas_smart_color_set(Eo *eo_obj, Evas_Smart_Data *o EINA_UNUSED, int r, int g, int b, int a)
{
   // If this function is reached, so we do nothing except trying to call
   // the function of the legacy smart class.
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJ_CLASS);
   Evas_Smart *s = obj->smart.smart;
   if (s && s->smart_class->color_set) s->smart_class->color_set(eo_obj, r, g, b, a);
}

EOLIAN static void
_evas_smart_clip_set(Eo *eo_obj, Evas_Smart_Data *o EINA_UNUSED, Evas_Object *clip)
{
   // If this function is reached, so we do nothing except trying to call
   // the function of the legacy smart class.
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJ_CLASS);
   Evas_Smart *s = obj->smart.smart;
   if (s && s->smart_class->clip_set) s->smart_class->clip_set(eo_obj, clip);
}

EOLIAN static void
_evas_smart_clip_unset(Eo *eo_obj, Evas_Smart_Data *o EINA_UNUSED)
{
   // If this function is reached, so we do nothing except trying to call
   // the function of the legacy smart class.
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJ_CLASS);
   Evas_Smart *s = obj->smart.smart;
   if (s && s->smart_class->clip_unset) s->smart_class->clip_unset(eo_obj);
}

static void
_evas_smart_attach(Eo *eo_obj, Evas_Smart_Data *_pd EINA_UNUSED, Evas_Smart *s)
{
   MAGIC_CHECK(s, Evas_Smart, MAGIC_SMART);
   return;
   MAGIC_CHECK_END();
   unsigned int i;
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJ_CLASS);
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

EAPI void
evas_object_smart_callback_add(Evas_Object *eo_obj, const char *event, Evas_Smart_Cb func, const void *data)
{
   evas_object_smart_callback_priority_add(eo_obj, event,
         EVAS_CALLBACK_PRIORITY_DEFAULT, func, data);
}

EAPI void
evas_object_smart_callback_priority_add(Evas_Object *eo_obj, const char *event, Evas_Callback_Priority priority, Evas_Smart_Cb func, const void *data)
{
   Evas_Smart_Data *o;

   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = eo_data_scope_get(eo_obj, MY_CLASS);

   if (!event) return;
   if (!func) return;

   _Evas_Event_Description *event_desc = eina_hash_find(signals_hash_table, event);
   if (!event_desc)
     {
        event_desc = calloc (1, sizeof(*event_desc));
        event_desc->eo_desc = calloc(1, sizeof(Eo_Event_Description));
        event_desc->eo_desc->name = eina_stringshare_add(event);
        event_desc->eo_desc->doc = "";
        event_desc->is_desc_allocated = EINA_TRUE;
        eina_hash_add(signals_hash_table, event, event_desc);
     }
   _eo_evas_smart_cb_info *cb_info = calloc(1, sizeof(*cb_info));
   cb_info->func = func;
   cb_info->data = (void *)data;
   cb_info->desc = event_desc;

   o->callbacks = eina_inlist_append(o->callbacks,
        EINA_INLIST_GET(cb_info));

   eo_do(eo_obj, eo_event_callback_priority_add(event_desc->eo_desc, priority, _eo_evas_smart_cb, cb_info));
}

EAPI void *
evas_object_smart_callback_del(Evas_Object *eo_obj, const char *event, Evas_Smart_Cb func)
{
   Evas_Smart_Data *o;
   _eo_evas_smart_cb_info *info;

   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   o = eo_data_scope_get(eo_obj, MY_CLASS);

   if (!event) return NULL;
   const _Evas_Event_Description *event_desc = eina_hash_find(signals_hash_table, event);
   if (!event_desc) return NULL;

   EINA_INLIST_FOREACH(o->callbacks, info)
     {
        if ((info->func == func) && (info->desc == event_desc))
          {
             void *tmp = info->data;
             eo_do(eo_obj, eo_event_callback_del(
                      event_desc->eo_desc, _eo_evas_smart_cb, info));

             o->callbacks =
                eina_inlist_remove(o->callbacks, EINA_INLIST_GET(info));
             free(info);
             return tmp;
          }
     }
   return NULL;
}

EAPI void *
evas_object_smart_callback_del_full(Evas_Object *eo_obj, const char *event, Evas_Smart_Cb func, const void *data)
{
   Evas_Smart_Data *o;
   _eo_evas_smart_cb_info *info;

   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   if (!event) return NULL;
   o = eo_data_scope_get(eo_obj, MY_CLASS);
   if (!o) return NULL;

   const _Evas_Event_Description *event_desc = eina_hash_find(signals_hash_table, event);
   if (!event_desc) return NULL;

   EINA_INLIST_FOREACH(o->callbacks, info)
     {
        if ((info->func == func) && (info->desc == event_desc) && (info->data == data))
          {
             void *tmp = info->data;
             eo_do(eo_obj, eo_event_callback_del(
                      event_desc->eo_desc, _eo_evas_smart_cb, info));

             o->callbacks =
                eina_inlist_remove(o->callbacks, EINA_INLIST_GET(info));
             free(info);
             return tmp;
          }
     }
   return NULL;
}

EAPI void
evas_object_smart_callback_call(Evas_Object *eo_obj, const char *event, void *event_info)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();

   if (!event) return;
   const _Evas_Event_Description *event_desc = eina_hash_find(signals_hash_table, event);
   if (event_desc)
      eo_do(eo_obj, eo_event_callback_call(event_desc->eo_desc, event_info, NULL));
}

EOLIAN static Eina_Bool
_evas_smart_callbacks_descriptions_set(Eo *eo_obj EINA_UNUSED, Evas_Smart_Data *o, const Evas_Smart_Cb_Description *descriptions)
{
   const Evas_Smart_Cb_Description *d;
   unsigned int i, count = 0;

   if ((!descriptions) || (!descriptions->name))
     {
        evas_smart_cb_descriptions_resize(&o->callbacks_descriptions, 0);
        return EINA_TRUE;
     }

   for (count = 0, d = descriptions; d->name; d++)
     count++;

   evas_smart_cb_descriptions_resize(&o->callbacks_descriptions, count);
   if (count == 0) return EINA_TRUE;

   for (i = 0, d = descriptions; i < count; d++, i++)
     o->callbacks_descriptions.array[i] = d;

   evas_smart_cb_descriptions_fix(&o->callbacks_descriptions);

   return EINA_TRUE;
}

EOLIAN static void
_evas_smart_callbacks_descriptions_get(Eo *eo_obj, Evas_Smart_Data *o, const Evas_Smart_Cb_Description ***class_descriptions, unsigned int *class_count, const Evas_Smart_Cb_Description ***instance_descriptions, unsigned int *instance_count)
{
   if (class_descriptions) *class_descriptions = NULL;
   if (class_count) *class_count = 0;

   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJ_CLASS);
   if (obj->smart.smart && class_descriptions)
      *class_descriptions = obj->smart.smart->callbacks.array;
   if (obj->smart.smart && class_count)
     *class_count = obj->smart.smart->callbacks.size;

   if (instance_descriptions)
     *instance_descriptions = o->callbacks_descriptions.array;
   if (instance_count)
     *instance_count = o->callbacks_descriptions.size;
}

EOLIAN static void
_evas_smart_callback_description_find(Eo *eo_obj, Evas_Smart_Data *o, const char *name, const Evas_Smart_Cb_Description **class_description, const Evas_Smart_Cb_Description **instance_description)
{

   if (!name)
     {
        if (class_description) *class_description = NULL;
        if (instance_description) *instance_description = NULL;
        return;
     }

   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJ_CLASS);
   if (class_description)
     *class_description = evas_smart_cb_description_find
        (&obj->smart.smart->callbacks, name);

   if (instance_description)
     *instance_description = evas_smart_cb_description_find
        (&o->callbacks_descriptions, name);
}

EOLIAN static void
_evas_smart_need_recalculate_set(Eo *eo_obj, Evas_Smart_Data *o, Eina_Bool value)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJ_CLASS);

   // XXX: do i need this?
   if (obj->delete_me) return;

   /* remove this entry from calc_list or processed list */
   if (eina_clist_element_is_linked(&o->calc_entry))
     eina_clist_remove(&o->calc_entry);

   value = !!value;
   if (value)
     eina_clist_add_tail(&obj->layer->evas->calc_list, &o->calc_entry);
   else
     eina_clist_add_tail(&obj->layer->evas->calc_done, &o->calc_entry);

   if (o->need_recalculate == value) return;

   if (o->recalculate_cycle > 254)
     {
        ERR("Object %p is not stable during recalc loop", eo_obj);
        return;
     }
   if (obj->layer->evas->in_smart_calc) o->recalculate_cycle++;
   o->need_recalculate = value;
}

EOLIAN static Eina_Bool
_evas_smart_need_recalculate_get(Eo *eo_obj EINA_UNUSED, Evas_Smart_Data *o)
{
   return o->need_recalculate;
}

EOLIAN static void
_evas_smart_calculate(Eo *eo_obj, Evas_Smart_Data *o)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJ_CLASS);

   if (!obj->smart.smart || !obj->smart.smart->smart_class->calculate)
     return;

   o->need_recalculate = 0;
   obj->smart.smart->smart_class->calculate(eo_obj);
}

EOLIAN void
_evas_smart_objects_calculate(Eo *eo_e, Evas_Public_Data *o EINA_UNUSED)
{
   evas_call_smarts_calculate(eo_e);
}

EOLIAN int
_evas_smart_objects_calculate_count_get(Eo *eo_e EINA_UNUSED, Evas_Public_Data *e)
{
   return e->smart_calc_count;
}

/**
 * Call calculate() on all smart objects that need_recalculate.
 *
 * @internal
 */
void
evas_call_smarts_calculate(Evas *eo_e)
{
   Evas_Smart_Data *o;
   Eina_Clist *elem;
   Evas_Public_Data *e = eo_data_scope_get(eo_e, EVAS_CLASS);

   evas_event_freeze(eo_e);
   e->in_smart_calc++;

   while (NULL != (elem = eina_clist_head(&e->calc_list)))
     {
        Evas_Object_Protected_Data *obj;

        /* move the item to the processed list */
        o = EINA_CLIST_ENTRY(elem, Evas_Smart_Data, calc_entry);
        eina_clist_remove(&o->calc_entry);
        obj = eo_data_scope_get(o->object, EVAS_OBJ_CLASS);

        if (obj->delete_me) continue;
        eina_clist_add_tail(&e->calc_done, &o->calc_entry);

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
        o = EINA_CLIST_ENTRY(elem, Evas_Smart_Data, calc_entry);
        o->recalculate_cycle = 0;
        eina_clist_remove(&o->calc_entry);
     }

   e->in_smart_calc--;
   if (e->in_smart_calc == 0) e->smart_calc_count++;
   evas_event_thaw(eo_e);
   evas_event_thaw_eval(eo_e);
}

EOLIAN static void
_evas_smart_changed(Eo *eo_obj, Evas_Smart_Data *o EINA_UNUSED)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJ_CLASS);
   evas_object_change(eo_obj, obj);
   eo_do(eo_obj, evas_obj_smart_need_recalculate_set(1));
}

Eina_Bool
evas_object_smart_changed_get(Evas_Object *eo_obj)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJ_CLASS);
   Evas_Object_Protected_Data *o2;

   if (!evas_object_is_visible(eo_obj, obj) &&
       !evas_object_was_visible(eo_obj, obj))
     return EINA_FALSE;

   if (!obj->clip.clipees)
     {
        if (obj->changed && !obj->is_smart) return EINA_TRUE;
        if (_evas_render_has_map(eo_obj, obj))
          {
             if (((obj->changed_pchange) && (obj->changed_map)) ||
                 (obj->changed_color)) return EINA_TRUE;
          }
     }

   EINA_INLIST_FOREACH(evas_object_smart_members_get_direct(eo_obj), o2)
     if (evas_object_smart_changed_get(o2->object)) return EINA_TRUE;

   return EINA_FALSE;
}

void
evas_object_smart_del(Evas_Object *eo_obj)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJ_CLASS);
   Evas_Smart_Data *sobj;
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

   sobj = eo_data_scope_get(eo_obj, MY_CLASS);
   free(sobj->interface_privates);
   sobj->interface_privates = NULL;

   if (s) evas_object_smart_unuse(s);
}

void
evas_object_update_bounding_box(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj)
{
   Evas_Smart_Data *s = NULL;
   Eina_Bool propagate = EINA_FALSE;
   Eina_Bool computeminmax = EINA_FALSE;
   Evas_Coord x, y, w, h;
   Evas_Coord px, py, pw, ph;
   Eina_Bool noclip;

   if (!obj->smart.parent) return;

   if (obj->child_has_map) return; /* Disable bounding box computation for this object and its parent */
   /* We could also remove object that are not visible from the bounding box, use the clipping information
      to reduce the bounding of the object they are clipping, but for the moment this will do it's jobs */
   noclip = !(obj->clip.clipees || obj->is_static_clip);

   if (obj->is_smart)
     {
        s = eo_data_scope_get(eo_obj, MY_CLASS);

        x = s->cur.bounding_box.x;
        y = s->cur.bounding_box.y;
        w = s->cur.bounding_box.w;
        h = s->cur.bounding_box.h;
        px = s->prev.bounding_box.x;
        py = s->prev.bounding_box.y;
        pw = s->prev.bounding_box.w;
        ph = s->prev.bounding_box.h;
     }
   else
     {
        x = obj->cur->geometry.x;
        y = obj->cur->geometry.y;
        w = obj->cur->geometry.w;
        h = obj->cur->geometry.h;
        px = obj->prev->geometry.x;
        py = obj->prev->geometry.y;
        pw = obj->prev->geometry.w;
        ph = obj->prev->geometry.h;
     }

   /* We are not yet trying to find the smallest bounding box, but we want to find a good approximation quickly.
    * That's why we initialiaze min and max search to geometry of the parent object.
    */
   Evas_Object_Protected_Data *smart_obj = eo_data_scope_get(obj->smart.parent, EVAS_OBJ_CLASS);
   Evas_Smart_Data *smart_parent = eo_data_scope_get(obj->smart.parent, MY_CLASS);
   if (!smart_parent || !smart_obj) return;

   if (smart_obj->cur->valid_bounding_box)
     {
        /* Update left limit */
        if (noclip && x < smart_parent->cur.bounding_box.x)
          {
	     smart_parent->cur.bounding_box.w += smart_parent->cur.bounding_box.x - x;
	     smart_parent->cur.bounding_box.x = x;

             propagate = EINA_TRUE;
          }
        else if ((px == smart_parent->prev.bounding_box.x &&
		  x > smart_parent->cur.bounding_box.x)
                 || (!noclip && x == smart_parent->cur.bounding_box.x))
          {
             computeminmax = EINA_TRUE;
          }

        /* Update top limit */
        if (noclip && y < smart_parent->cur.bounding_box.y)
          {
	     smart_parent->cur.bounding_box.h += smart_parent->cur.bounding_box.x - x;
	     smart_parent->cur.bounding_box.y = y;

             propagate = EINA_TRUE;
          }
        else if ((py == smart_parent->prev.bounding_box.y &&
		  y  > smart_parent->cur.bounding_box.y)
                 || (!noclip && y == smart_parent->cur.bounding_box.y))
          {
             computeminmax = EINA_TRUE;
          }

        /* Update right limit */
        if (noclip && x + w > smart_parent->cur.bounding_box.x + smart_parent->cur.bounding_box.w)
          {
	     smart_parent->cur.bounding_box.w = x + w - smart_parent->cur.bounding_box.x;
             
             propagate = EINA_TRUE;
          }
        else if ((px + pw == smart_parent->prev.bounding_box.x + smart_parent->prev.bounding_box.w &&
                  x + w < smart_parent->cur.bounding_box.x + smart_parent->cur.bounding_box.w)
                 || (!noclip && x + w == smart_parent->cur.bounding_box.x + smart_parent->cur.bounding_box.w))
          {
             computeminmax = EINA_TRUE;
          }

        /* Update bottom limit */
        if (noclip && y + h > smart_parent->cur.bounding_box.y + smart_parent->cur.bounding_box.h)
          {
	     smart_parent->cur.bounding_box.h = y + h - smart_parent->cur.bounding_box.y;

             propagate = EINA_TRUE;
          }
        else if ((py + ph == smart_parent->prev.bounding_box.y + smart_parent->prev.bounding_box.h &&
                  y + h < smart_parent->cur.bounding_box.y + smart_parent->cur.bounding_box.h) ||
                 (!noclip && y + h == smart_parent->cur.bounding_box.y + smart_parent->cur.bounding_box.h))
          {
             computeminmax = EINA_TRUE;
          }

	if (computeminmax)
          {
             evas_object_smart_need_bounding_box_update(obj->smart.parent);
          }
     }
   else
     {
        if (noclip)
          {
	     smart_parent->cur.bounding_box.x = x;
	     smart_parent->cur.bounding_box.y = y;
	     smart_parent->cur.bounding_box.w = w;
	     smart_parent->cur.bounding_box.h = h;

	     EINA_COW_STATE_WRITE_BEGIN(smart_obj, smart_write, cur)
	       smart_write->valid_bounding_box = EINA_TRUE;
	     EINA_COW_STATE_WRITE_END(smart_obj, smart_write, cur);

             propagate = EINA_TRUE;
          }
     }

   if (propagate)
     evas_object_update_bounding_box(obj->smart.parent, smart_obj);
}

void
evas_object_smart_bounding_box_get(Evas_Object *eo_obj,
				   Evas_Coord_Rectangle *cur_bounding_box,
				   Evas_Coord_Rectangle *prev_bounding_box)
{
   Evas_Smart_Data *s = eo_data_scope_get(eo_obj, MY_CLASS);

   if (cur_bounding_box) memcpy(cur_bounding_box,
				&s->cur.bounding_box,
				sizeof (*cur_bounding_box));
   if (prev_bounding_box) memcpy(prev_bounding_box,
				 &s->prev.bounding_box,
				 sizeof (*prev_bounding_box));
}

void
evas_object_smart_cleanup(Evas_Object *eo_obj)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJ_CLASS);

   if (obj->smart.parent)
     evas_object_smart_member_del(eo_obj);

   if (obj->is_smart)
     {
        Evas_Smart_Data *o = eo_data_scope_get(eo_obj, MY_CLASS);
        if (o->calc_entry.next)
          eina_clist_remove(&o->calc_entry);

        while (o->contained)
          {
             Evas_Object *contained_obj = ((Evas_Object_Protected_Data *)o->contained)->object;
             evas_object_smart_member_del(contained_obj);
          }

        while (o->callbacks)
          {
             _eo_evas_smart_cb_info *info = (_eo_evas_smart_cb_info *)o->callbacks;
             eo_do(eo_obj, eo_event_callback_del(
                      info->desc->eo_desc, _eo_evas_smart_cb, info));
             o->callbacks = eina_inlist_remove(o->callbacks, EINA_INLIST_GET(info));
             free(info);
          }

        evas_smart_cb_descriptions_resize(&o->callbacks_descriptions, 0);
        eo_do(eo_obj, evas_obj_smart_data_set(NULL));
     }

   obj->smart.parent = NULL;
   obj->smart.smart = NULL;
}

void
evas_object_smart_member_cache_invalidate(Evas_Object *eo_obj,
                                          Eina_Bool pass_events,
                                          Eina_Bool freeze_events,
                                          Eina_Bool source_invisible)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();

   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJ_CLASS);
   Evas_Object_Protected_Data *member;

   if (pass_events)
     obj->parent_cache.pass_events_valid = EINA_FALSE;
   if (freeze_events)
     obj->parent_cache.freeze_events_valid = EINA_FALSE;
   if (source_invisible)
     obj->parent_cache.src_invisible_valid = EINA_FALSE;

   if (!obj->is_smart) return;
   Evas_Smart_Data *o = eo_data_scope_get(eo_obj, MY_CLASS);
   EINA_INLIST_FOREACH(o->contained, member)
     {
        Evas_Object *eo_member = member->object;
        evas_object_smart_member_cache_invalidate(eo_member, pass_events,
                                                  freeze_events,
                                                  source_invisible);
     }
}

void
evas_object_smart_member_raise(Evas_Object *eo_member)
{
   Evas_Smart_Data *o;
   Evas_Object_Protected_Data *member = eo_data_scope_get(eo_member, EVAS_OBJ_CLASS);
   o = eo_data_scope_get(member->smart.parent, MY_CLASS);
   o->contained = eina_inlist_demote(o->contained, EINA_INLIST_GET(member));
}

void
evas_object_smart_member_lower(Evas_Object *eo_member)
{
   Evas_Smart_Data *o;
   Evas_Object_Protected_Data *member = eo_data_scope_get(eo_member, EVAS_OBJ_CLASS);
   o = eo_data_scope_get(member->smart.parent, MY_CLASS);
   o->contained = eina_inlist_promote(o->contained, EINA_INLIST_GET(member));
}

void
evas_object_smart_member_stack_above(Evas_Object *eo_member, Evas_Object *eo_other)
{
   Evas_Smart_Data *o;
   Evas_Object_Protected_Data *member = eo_data_scope_get(eo_member, EVAS_OBJ_CLASS);
   Evas_Object_Protected_Data *other = eo_data_scope_get(eo_other, EVAS_OBJ_CLASS);
   o = eo_data_scope_get(member->smart.parent, MY_CLASS);
   o->contained = eina_inlist_remove(o->contained, EINA_INLIST_GET(member));
   o->contained = eina_inlist_append_relative(o->contained, EINA_INLIST_GET(member), EINA_INLIST_GET(other));
}

void
evas_object_smart_member_stack_below(Evas_Object *eo_member, Evas_Object *eo_other)
{
   Evas_Smart_Data *o;
   Evas_Object_Protected_Data *member = eo_data_scope_get(eo_member, EVAS_OBJ_CLASS);
   Evas_Object_Protected_Data *other = eo_data_scope_get(eo_other, EVAS_OBJ_CLASS);
   o = eo_data_scope_get(member->smart.parent, MY_CLASS);
   o->contained = eina_inlist_remove(o->contained, EINA_INLIST_GET(member));
   o->contained = eina_inlist_prepend_relative(o->contained, EINA_INLIST_GET(member), EINA_INLIST_GET(other));
}

void
evas_object_smart_need_bounding_box_update(Evas_Object *eo_obj)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJ_CLASS);
   Evas_Smart_Data *o = eo_data_scope_get(eo_obj, MY_CLASS);

   if (o->update_boundingbox_needed) return;
   o->update_boundingbox_needed = EINA_TRUE;

   if (obj->smart.parent) evas_object_smart_need_bounding_box_update(obj->smart.parent);
}

void
evas_object_smart_bounding_box_update(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj)
{
   Evas_Smart_Data *os;
   Eina_Inlist *list;
   Evas_Object_Protected_Data *o;
   Evas_Coord minx;
   Evas_Coord miny;
   Evas_Coord maxw = 0;
   Evas_Coord maxh = 0;

   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();

   os = eo_data_scope_get(eo_obj, MY_CLASS);

   if (!os->update_boundingbox_needed) return;
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

	if (o->is_smart)
          {
	     Evas_Smart_Data *s = eo_data_scope_get(o->object, MY_CLASS);

             evas_object_smart_bounding_box_update(o->object, o);

             tx = s->cur.bounding_box.x;
             ty = s->cur.bounding_box.y;
             tw = s->cur.bounding_box.x + s->cur.bounding_box.w;
             th = s->cur.bounding_box.y + s->cur.bounding_box.h;
          }
        else
          {
             tx = o->cur->geometry.x;
             ty = o->cur->geometry.y;
             tw = o->cur->geometry.x + o->cur->geometry.w;
             th = o->cur->geometry.y + o->cur->geometry.h;
          }

        if (tx < minx) minx = tx;
        if (ty < miny) miny = ty;
        if (tw > maxw) maxw = tw;
        if (th > maxh) maxh = th;
     }

   if (minx != os->cur.bounding_box.x)
     {
        os->cur.bounding_box.w += os->cur.bounding_box.x - minx;
	os->cur.bounding_box.x = minx;
     }

   if (miny != os->cur.bounding_box.y)
     {
        os->cur.bounding_box.h += os->cur.bounding_box.y - miny;
	os->cur.bounding_box.y = miny;
     }

   if (maxw != os->cur.bounding_box.x + os->cur.bounding_box.w)
     {
        os->cur.bounding_box.w = maxw - os->cur.bounding_box.x;
     }

   if (maxh != os->cur.bounding_box.y + os->cur.bounding_box.h)
     {
        os->cur.bounding_box.h = maxh - os->cur.bounding_box.y;
     }
}

/* all nice and private */
static void
evas_object_smart_init(Evas_Object *eo_obj)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJ_CLASS);
   obj->is_smart = EINA_TRUE;
   /* set up methods (compulsory) */
   obj->func = &object_func;
   obj->private_data = eo_data_ref(eo_obj, MY_CLASS);
}

static void
evas_object_smart_render(Evas_Object *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj EINA_UNUSED,  void *type_private_data EINA_UNUSED, void *output EINA_UNUSED, void *context EINA_UNUSED, void *surface EINA_UNUSED, int x EINA_UNUSED, int y EINA_UNUSED, Eina_Bool do_async EINA_UNUSED)
{
   return;
}

static void
evas_object_smart_render_pre(Evas_Object *eo_obj,
			     Evas_Object_Protected_Data *obj,
			     void *type_private_data EINA_UNUSED)
{
   if (obj->pre_render_done) return;
   if (!obj->child_has_map && !obj->cur->cached_surface)
     {
#if 0
       // REDO to handle smart move
        Evas_Smart_Data *o;

        fprintf(stderr, "");
        o = type_private_data;
        if (/* o->member_count > 1 && */
            o->cur.bounding_box.w == o->prev.bounding_box.w &&
            obj->cur->bounding_box.h == obj->prev->bounding_box.h &&
            (obj->cur->bounding_box.x != obj->prev->bounding_box.x ||
             obj->cur->bounding_box.y != obj->prev->bounding_box.y))
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

                       speed_x = obj->cur->geometry.x - obj->prev->geometry.x;
                       speed_y = obj->cur->geometry.y - obj->prev->geometry.y;

                       speed_px = obj->smart.parent->cur.geometry.x - obj->smart.parent->prev.geometry.x;
                       speed_py = obj->smart.parent->cur.geometry.y - obj->smart.parent->prev.geometry.y;

                       /* speed_x = obj->cur->bounding_box.x - obj->prev->bounding_box.x; */
                       /* speed_y = obj->cur->bounding_box.y - obj->prev->bounding_box.y; */

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
                       obj->cur->bounding_box.x - obj->prev->bounding_box.x,
                       obj->cur->bounding_box.y - obj->prev->bounding_box.y,
                       obj->cur->bounding_box.w, obj->cur->bounding_box.h,
                       evas_object_type_get(obj->smart.parent), obj->smart.parent,
                       obj->smart.parent->cur.bounding_box.x - obj->smart.parent->prev.bounding_box.x,
                       obj->smart.parent->cur.bounding_box.y - obj->smart.parent->prev.bounding_box.y,
                       obj->smart.parent->cur.bounding_box.w, obj->smart.parent->cur.bounding_box.h);

             obj->cur->cached_surface = cache_map;
          }
#endif
     }

   if (obj->changed_map || obj->changed_src_visible)
     evas_object_render_pre_prev_cur_add(&obj->layer->evas->clip_changes,
                                         eo_obj, obj);

   obj->pre_render_done = EINA_TRUE;
}

static void
evas_object_smart_render_post(Evas_Object *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj EINA_UNUSED, void *type_private_data)
{
   Evas_Smart_Data *o = type_private_data;
   evas_object_cur_prev(eo_obj);
   o->prev = o->cur;
}

static unsigned int evas_object_smart_id_get(Evas_Object *eo_obj)
{
   Evas_Smart_Data *o = eo_data_scope_get(eo_obj, MY_CLASS);
   if (!o) return 0;
   return MAGIC_OBJ_SMART;
}

static unsigned int evas_object_smart_visual_id_get(Evas_Object *eo_obj)
{
   Evas_Smart_Data *o = eo_data_scope_get(eo_obj, MY_CLASS);
   if (!o) return 0;
   return MAGIC_OBJ_CONTAINER;
}

static void *evas_object_smart_engine_data_get(Evas_Object *eo_obj)
{
   Evas_Smart_Data *o = eo_data_scope_get(eo_obj, MY_CLASS);
   if (!o) return NULL;
   return o->engine_data;
}

static void
_evas_smart_class_constructor(Eo_Class *klass EINA_UNUSED)
{
   _evas_smart_class_names_hash_table = eina_hash_string_small_new(NULL);
   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}

static void
_evas_smart_class_destructor(Eo_Class *klass EINA_UNUSED)
{
   eina_hash_free(_evas_smart_class_names_hash_table);
}

#include "canvas/evas_smart.eo.c"
