#define EFL_CANVAS_GROUP_PROTECTED
#define EFL_CANVAS_GROUP_BETA

#include "evas_common_private.h"
#include "evas_private.h"

#define MY_CLASS EFL_CANVAS_GROUP_CLASS

#define MY_CLASS_NAME "Evas_Smart"
#define MY_CLASS_NAME_LEGACY "Evas_Object_Smart"

#define EVAS_OBJECT_SMART_GET_OR_RETURN(eo_obj, ...) \
   Evas_Smart_Data *o = efl_data_scope_safe_get(eo_obj, MY_CLASS); \
   do { if (!o) { MAGIC_CHECK_FAILED(eo_obj,0,MAGIC_SMART) return __VA_ARGS__; } } while (0)

extern Eina_Hash* signals_hash_table;

static Eina_Hash *_evas_smart_class_names_hash_table = NULL;

struct _Evas_Smart_Data
{
   struct {
      Eina_Rectangle bounding_box;
   } cur, prev;
   Evas_Object      *object;
   void             *engine_data;
   void             *data;
   Eina_Inlist      *callbacks;
   Eina_Inlist      *contained; /** list of smart member objects */

   void             *render_cache;
  /* ptr array + data blob holding all interfaces private data for
   * this object */
   void            **interface_privates;
   Eina_Clist        calc_entry;

   Evas_Smart_Cb_Description_Array callbacks_descriptions;

   int               x, y;
   int               walking_list;
   int               member_count; /** number of smart member objects */

   unsigned short    recalculate_cycle;

   Evas_BiDi_Direction paragraph_direction : 2;
   Eina_Bool         inherit_paragraph_direction : 1;
   Eina_Bool         deletions_waiting : 1;
   Eina_Bool         need_recalculate : 1;
   Eina_Bool         update_boundingbox_needed : 1;
   Eina_Bool         group_del_called : 1;
   Eina_Bool         clipped : 1; /* If true, smart clipped */
   Eina_Bool         data_nofree : 1; /* If true, do NOT free the data */
   Eina_Bool         constructed : 1; /* constructor finished */
};

typedef struct
{
   EINA_INLIST;
   Evas_Smart_Cb func;
   void *data;
   const Efl_Event_Description *event;
} _eo_evas_smart_cb_info;


typedef struct _Evas_Object_Smart_Iterator Evas_Object_Smart_Iterator;
struct _Evas_Object_Smart_Iterator
{
   Eina_Iterator iterator;

   const Eina_Inlist *current;
   Evas_Object *parent;
};

static void
_eo_evas_smart_cb(void *data, const Efl_Event *event)
{
   _eo_evas_smart_cb_info *info = data;
   if (info->func) info->func(info->data, event->object, event->info);
}

/* private methods for smart objects */
static void evas_object_smart_render(Evas_Object *eo_obj,
                                     Evas_Object_Protected_Data *obj,
                                     void *type_private_data,
                                     void *engine, void *output, void *context, void *surface,
                                     int x, int y, Eina_Bool do_async);
static void evas_object_smart_render_pre(Evas_Object *eo_obj,
					 Evas_Object_Protected_Data *obj,
					 void *type_private_data);
static void evas_object_smart_render_post(Evas_Object *eo_obj,
					  Evas_Object_Protected_Data *obj,
					  void *type_private_data);

static void *evas_object_smart_engine_data_get(Evas_Object *eo_obj);
static void _efl_canvas_group_group_paragraph_direction_set_internal(Eo *eo_obj,
                                                                Evas_BiDi_Direction dir);

static const Evas_Object_Func object_func =
{
   /* methods (compulsory) */
   NULL,
   evas_object_smart_render,
   evas_object_smart_render_pre,
   evas_object_smart_render_post,
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
   NULL   // render_prepare
};

/* helpers */
static inline Evas_Object *
_smart_clipper_get(Evas_Smart_Data *o)
{
   Evas_Object_Smart_Clipped_Data *cso = o->clipped ? o->data : NULL;
   return cso ? cso->clipper : NULL;
}

/* public funcs */
EAPI void
evas_object_smart_data_set(Evas_Object *eo_obj, void *data)
{
   EVAS_OBJECT_SMART_GET_OR_RETURN(eo_obj);
   if (o->data != data)
     {
        if (o->data && !o->data_nofree)
          free(o->data);
        o->data = data;
        o->data_nofree = EINA_TRUE;
     }
}

EAPI void *
evas_object_smart_data_get(const Evas_Object *eo_obj)
{
   EVAS_OBJECT_SMART_GET_OR_RETURN(eo_obj, NULL);
   return o->data;
}

EAPI const void *
evas_object_smart_interface_get(const Evas_Object *eo_obj,
                                const char *name)
{
   Evas_Smart *s;
   unsigned int i;

   EVAS_OBJECT_SMART_GET_OR_RETURN(eo_obj, NULL);
   s = evas_object_smart_smart_get(eo_obj);
   if (!s) return NULL;

   for (i = 0; i < s->interfaces.size; i++)
     {
        const Evas_Smart_Interface *iface;

        iface = s->interfaces.array[i];

        if (iface->name == name)
          return iface;
     }

   return NULL;
}

EAPI void *
evas_object_smart_interface_data_get(const Evas_Object *eo_obj,
                                     const Evas_Smart_Interface *iface)
{
   unsigned int i;
   Evas_Smart *s;

   EVAS_OBJECT_SMART_GET_OR_RETURN(eo_obj, NULL);
   s = evas_object_smart_smart_get(eo_obj);
   if (!s) return NULL;

   if (s)
     {
        for (i = 0; i < s->interfaces.size; i++)
          {
             if (iface == s->interfaces.array[i])
                return o->interface_privates[i];
          }
     }

   return NULL;
}

EAPI Evas_Smart*
evas_object_smart_smart_get(const Efl_Canvas_Group *eo_obj)
{
   Evas_Object_Protected_Data *obj = EVAS_OBJ_GET_OR_RETURN(eo_obj, NULL);
   return obj->smart.smart;
}

EAPI void
evas_object_smart_member_add(Evas_Object *eo_obj, Evas_Object *smart_obj)
{
   efl_canvas_group_member_add(smart_obj, eo_obj);
}

EOLIAN static void
_efl_canvas_group_group_member_add(Eo *smart_obj, Evas_Smart_Data *o, Evas_Object *eo_obj)
{

   Evas_Object_Protected_Data *obj = efl_data_scope_safe_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   Evas_Object_Protected_Data *smart = efl_data_scope_get(smart_obj, EFL_CANVAS_OBJECT_CLASS);
   Evas_Smart_Data *member_o = NULL;

   if ((!obj) || (!smart)) return;
   if (obj->delete_me)
     {
        CRI("Adding deleted object %p to smart obj %p", eo_obj, smart_obj);
        return;
     }
   if (smart->delete_me)
     {
        CRI("Adding object %p to deleted smart obj %p", eo_obj, smart_obj);
        return;
     }
   if (!smart->layer)
     {
        CRI("No evas surface associated with smart object (%p)", smart_obj);
        return;
     }
   if ((obj->layer && smart->layer) &&
       (obj->layer->evas != smart->layer->evas))
     {
        CRI("Adding object %p from Evas (%p) from another Evas (%p)", eo_obj, obj->layer->evas, smart->layer->evas);
        return;
     }

   if (obj->smart.parent == smart_obj) return;

   evas_object_async_block(obj);
   if (obj->smart.parent) evas_object_smart_member_del(eo_obj);

   if (obj->layer != smart->layer)
     {
        if (obj->in_layer)
          evas_object_release(eo_obj, obj, 1);
        else if (obj->layer && ((--obj->layer->usage) == 0))
          evas_layer_del(obj->layer);
     }
   else if (obj->in_layer)
     {
        evas_object_release(eo_obj, obj, 1);
     }
   obj->layer = smart->layer;
   obj->layer->usage++;
   if (obj->layer->layer != obj->cur->layer)
     {
        EINA_COW_STATE_WRITE_BEGIN(obj, state_write, cur)
          state_write->layer = obj->layer->layer;
        EINA_COW_STATE_WRITE_END(obj, state_write, cur);
     }

   o->member_count++;
   obj->smart.parent = smart_obj;
   obj->smart.parent_data = o;
   obj->smart.parent_object_data = smart;
   o->contained = eina_inlist_append(o->contained, EINA_INLIST_GET(obj));
   evas_object_smart_member_cache_invalidate(eo_obj, EINA_TRUE, EINA_TRUE,
                                             EINA_TRUE);
   obj->restack = 1;

   if (obj->is_smart)
     {
        member_o = efl_data_scope_get(eo_obj, MY_CLASS);

        if ((member_o->inherit_paragraph_direction) &&
            (member_o->paragraph_direction != o->paragraph_direction))
          {
             member_o->paragraph_direction = o->paragraph_direction;
             _efl_canvas_group_group_paragraph_direction_set_internal(eo_obj, o->paragraph_direction);
          }
     }

   if (!smart->is_frame_top && (smart->is_frame != obj->is_frame))
     efl_canvas_object_is_frame_object_set(eo_obj, smart->is_frame);

   if (o->clipped)
     {
        Evas_Object *clipper = _smart_clipper_get(o);
        Eina_Bool had_clippees = efl_canvas_object_clipees_has(clipper);

        if (EINA_UNLIKELY(!clipper && !o->constructed))
          {
             _evas_object_smart_clipped_init(smart_obj);
             clipper = _smart_clipper_get(o);
          }

        if (clipper != eo_obj)
          {
             EINA_SAFETY_ON_NULL_RETURN(clipper);
             efl_canvas_object_clip_set(eo_obj, clipper);
             if (!had_clippees && smart->cur->visible)
               efl_gfx_entity_visible_set(clipper, 1);
          }
     }

   evas_object_change(eo_obj, obj);
   evas_object_mapped_clip_across_mark(eo_obj, obj);
   if (smart->smart.smart && smart->smart.smart->smart_class->member_add)
     smart->smart.smart->smart_class->member_add(smart_obj, eo_obj);
   evas_object_update_bounding_box(eo_obj, obj, member_o);
}

EAPI void
evas_object_smart_member_del(Evas_Object *eo_obj)
{
   Evas_Object_Protected_Data *obj;

   if (!eo_obj) return ;
   obj = efl_data_scope_safe_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   if (!obj) return;
   if (!obj->smart.parent) return;
   Evas_Object *smart_obj = obj->smart.parent;
   efl_canvas_group_member_del(smart_obj, eo_obj);
}

EOLIAN static void
_efl_canvas_group_group_member_del(Eo *smart_obj, Evas_Smart_Data *_pd EINA_UNUSED, Evas_Object *eo_obj)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_safe_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   Evas_Object_Protected_Data *smart;
   Evas_Smart_Data *member_o, *o;

   if (!obj || !obj->smart.parent) return;

   evas_object_async_block(obj);

   smart = efl_data_scope_get(smart_obj, EFL_CANVAS_OBJECT_CLASS);
   if (smart->smart.smart && smart->smart.smart->smart_class->member_del)
     smart->smart.smart->smart_class->member_del(smart_obj, eo_obj);

   o = efl_data_scope_get(smart_obj, MY_CLASS);

   if (o->clipped)
     {
        Evas_Object *clipper = _smart_clipper_get(o);

        EINA_SAFETY_ON_NULL_RETURN(clipper);
        efl_canvas_object_clip_set(eo_obj, NULL);
        if (!efl_canvas_object_clipees_has(clipper))
          efl_gfx_entity_visible_set(clipper, 0);
     }

   o->contained = eina_inlist_remove(o->contained, EINA_INLIST_GET(obj));
   o->member_count--;
   obj->smart.parent = NULL;
   evas_object_smart_member_cache_invalidate(eo_obj, EINA_TRUE, EINA_TRUE, EINA_TRUE);

   if (obj->layer->layer != obj->cur->layer)
     {
        EINA_COW_STATE_WRITE_BEGIN(obj, state_write, cur)
          state_write->layer = obj->layer->layer;
        EINA_COW_STATE_WRITE_END(obj, state_write, cur);
     }

   if (obj->is_smart)
     {
        member_o = efl_data_scope_get(eo_obj, MY_CLASS);

        if ((member_o->inherit_paragraph_direction) &&
            (member_o->paragraph_direction != EVAS_BIDI_DIRECTION_NEUTRAL))
          {
             member_o->paragraph_direction = EVAS_BIDI_DIRECTION_NEUTRAL;
             _efl_canvas_group_group_paragraph_direction_set_internal(eo_obj, EVAS_BIDI_DIRECTION_NEUTRAL);
          }
     }

   if (EINA_UNLIKELY(obj->in_layer))
     {
        ERR("Invalid internal state of object %p (child marked as being a"
            "top-level object)!", obj->object);
        evas_object_release(obj->object, obj, 1);
     }
   else
     {
        // Layer usage shouldn't reach 0 here (as parent is still in layer)
        obj->layer->usage--;
     }
   /* layer may be destroyed in evas_object_release() call */
   if (obj->layer)
     evas_object_inject(eo_obj, obj, obj->layer->evas->evas);
   obj->restack = 1;
   evas_object_change(eo_obj, obj);
   evas_object_mapped_clip_across_mark(eo_obj, obj);
}

EAPI Eina_Bool
evas_object_smart_type_check(const Evas_Object *eo_obj, const char *type)
{
   const Evas_Smart_Class *sc;
   Efl_Class *klass;
   Eina_Bool type_check = EINA_FALSE;

   Evas_Object_Protected_Data *obj = EVAS_OBJ_GET_OR_RETURN(eo_obj, EINA_FALSE);

   klass = eina_hash_find(_evas_smart_class_names_hash_table, type);
   if (klass) type_check = efl_isa(eo_obj, klass);

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

EAPI Eina_Bool
evas_object_smart_type_check_ptr(const Eo *eo_obj, const char* type)
{
   Efl_Class *klass;
   const Evas_Smart_Class *sc;
   Eina_Bool type_check = EINA_FALSE;

   Evas_Object_Protected_Data *obj = EVAS_OBJ_GET_OR_RETURN(eo_obj, EINA_FALSE);

   klass = eina_hash_find(_evas_smart_class_names_hash_table, type);
   if (klass) type_check = efl_isa(eo_obj, klass);

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
evas_smart_legacy_type_register(const char *type, const Efl_Class *klass)
{
   eina_hash_set(_evas_smart_class_names_hash_table, type, klass);
}

static Eina_Bool
_efl_canvas_group_group_iterator_next(Evas_Object_Smart_Iterator *it, void **data)
{
   Evas_Object *eo;

   if (!it->current) return EINA_FALSE;

   eo = ((const Evas_Object_Protected_Data*)(it->current))->object;
   if (data) *data = eo;

   it->current = it->current->next;

   return EINA_TRUE;
}

static Evas_Object *
_efl_canvas_group_group_iterator_get_container(Evas_Object_Smart_Iterator *it)
{
   return it->parent;
}

static void
_efl_canvas_group_group_iterator_free(Evas_Object_Smart_Iterator *it)
{
   efl_unref(it->parent);
   free(it);
}

// Should we have an efl_children_iterator_new API and just inherit from it ?
// No, because each hierarchy is different (Eo, Smart, Widget) -- jpeg
EOLIAN static Eina_Iterator*
_efl_canvas_group_group_members_iterate(const Eo *eo_obj, Evas_Smart_Data *priv)
{
   Evas_Object_Smart_Iterator *it;
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);

   if (!priv->contained) return NULL;

   evas_object_async_block(obj);
   it = calloc(1, sizeof(Evas_Object_Smart_Iterator));
   if (!it) return NULL;

   EINA_MAGIC_SET(&it->iterator, EINA_MAGIC_ITERATOR);
   it->parent = efl_ref(eo_obj);
   it->current = priv->contained;

   it->iterator.next = FUNC_ITERATOR_NEXT(_efl_canvas_group_group_iterator_next);
   it->iterator.get_container = FUNC_ITERATOR_GET_CONTAINER(_efl_canvas_group_group_iterator_get_container);
   it->iterator.free = FUNC_ITERATOR_FREE(_efl_canvas_group_group_iterator_free);

   return &it->iterator;
}

EOLIAN static Eina_Bool
_efl_canvas_group_group_member_is(const Eo *eo_obj, Evas_Smart_Data *pd EINA_UNUSED, const Eo *sub_obj)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   Evas_Object_Protected_Data *sub = efl_data_scope_safe_get(sub_obj, EFL_CANVAS_OBJECT_CLASS);

   evas_object_async_block(obj);

   if (!sub) return EINA_FALSE;
   return (sub->smart.parent == eo_obj);
}

EAPI Eina_List*
evas_object_smart_members_get(const Evas_Object *eo_obj)
{
   EVAS_OBJECT_SMART_GET_OR_RETURN(eo_obj, NULL);
   Eina_List *members = NULL;
   Eina_Inlist *member;

   for (member = o->contained; member; member = member->next)
     members = eina_list_append(members, ((Evas_Object_Protected_Data *)member)->object);

   return members;
}

void
evas_object_smart_render_cache_clear(Evas_Object *eo_obj)
{
   Evas_Smart_Data *o = efl_data_scope_get(eo_obj, MY_CLASS);
   if (!o) return;
   if (!o->render_cache) return;
   evas_render_object_render_cache_free(eo_obj, o->render_cache);
   o->render_cache = NULL;
}

void *
evas_object_smart_render_cache_get(const Evas_Object *eo_obj)
{
   Evas_Smart_Data *o = efl_data_scope_get(eo_obj, MY_CLASS);
   if (!o) return NULL;
   return o->render_cache;
}

void
evas_object_smart_render_cache_set(Evas_Object *eo_obj, void *data)
{
   Evas_Smart_Data *o = efl_data_scope_get(eo_obj, MY_CLASS);
   if (!o) return;
   o->render_cache = data;
}

const Eina_Inlist *
evas_object_smart_members_get_direct(const Evas_Object *eo_obj)
{
   Evas_Smart_Data *o = efl_data_scope_get(eo_obj, MY_CLASS);
   if (!o) return NULL;
   return o->contained;
}

static void
_efl_canvas_group_group_members_all_del_internal(Evas_Smart_Data *o)
{
   Evas_Object *clipper;
   Evas_Object_Protected_Data *memobj;
   Eina_Inlist *itrn;

   clipper = _smart_clipper_get(o);
   if (clipper)
     {
        EINA_INLIST_FOREACH_SAFE(o->contained, itrn, memobj)
          {
             if (memobj->object != clipper)
               _evas_wrap_del(&memobj->object, memobj);
          }
        _evas_wrap_del(&clipper, efl_data_scope_get(clipper, EFL_CANVAS_OBJECT_CLASS));
     }

   o->group_del_called = EINA_TRUE;
}

void
_efl_canvas_group_group_members_all_del(Evas_Object *eo_obj)
{
   Evas_Smart_Data *o = efl_data_scope_get(eo_obj, MY_CLASS);
   _efl_canvas_group_group_members_all_del_internal(o);
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

   if (!s->interfaces.size && !total_priv_sz) return;

   obj = efl_data_scope_get(eo_obj, MY_CLASS);
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

   eo_e = evas_find(eo_e);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(efl_isa(eo_e, EVAS_CANVAS_CLASS), NULL);
   eo_obj = efl_add(MY_CLASS, eo_e, efl_canvas_object_legacy_ctor(efl_added));
   evas_object_smart_attach(eo_obj, s);
   return eo_obj;
}

EOLIAN static Eo *
_efl_canvas_group_efl_object_constructor(Eo *eo_obj, Evas_Smart_Data *sd)
{
   Evas_Object_Protected_Data *obj;

   sd->object = eo_obj;
   sd->inherit_paragraph_direction = EINA_TRUE;

   eo_obj = efl_constructor(efl_super(eo_obj, MY_CLASS));
   EINA_SAFETY_ON_NULL_RETURN_VAL(eo_obj, NULL);

   obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   obj->is_smart = EINA_TRUE;
   obj->func = &object_func;
   obj->private_data = efl_data_ref(eo_obj, MY_CLASS);
   if (sd->clipped && !sd->data)
     _evas_object_smart_clipped_init(eo_obj);

   efl_canvas_object_type_set(eo_obj, MY_CLASS_NAME_LEGACY);
   efl_canvas_group_add(eo_obj);

   sd->constructed = EINA_TRUE;
   return eo_obj;
}

EOLIAN static void
_efl_canvas_group_efl_object_destructor(Eo *eo_obj, Evas_Smart_Data *o)
{
   efl_destructor(efl_super(eo_obj, MY_CLASS));
   if (o->data && !o->data_nofree)
     free(o->data);
   if (!o->group_del_called)
     {
        ERR("efl_canvas_group_del() was not called on this object: %p (%s)",
            eo_obj, efl_class_name_get(eo_obj));
     }
}

EOLIAN static void
_efl_canvas_group_efl_object_debug_name_override(Eo *eo_obj, Evas_Smart_Data *o, Eina_Strbuf *sb)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   const char *smart_class = NULL;

   if (obj->smart.smart && obj->smart.smart->smart_class)
     smart_class = obj->smart.smart->smart_class->name;

   efl_debug_name_override(efl_super(eo_obj, MY_CLASS), sb);
   if (smart_class)
     {
        eina_strbuf_append_printf(sb, ":children=%d:smart_class=%s",
                                  eina_inlist_count(o->contained), smart_class);
     }
   else
     {
        eina_strbuf_append_printf(sb, ":children=%d", eina_inlist_count(o->contained));
     }
}

static inline void
_evas_object_smart_move_relative_internal(Evas_Smart_Data *o, Evas_Coord dx, Evas_Coord dy)
{
   Evas_Object_Protected_Data *child;

   EINA_INLIST_FOREACH(o->contained, child)
     {
        Evas_Coord orig_x, orig_y;

        if (child->delete_me) continue;
        if (child->is_static_clip) continue;
        orig_x = child->cur->geometry.x;
        orig_y = child->cur->geometry.y;
        evas_object_move(child->object, orig_x + dx, orig_y + dy);
     }
}

EAPI void
evas_object_smart_move_children_relative(Eo *eo_obj, Evas_Coord dx, Evas_Coord dy)
{
   EVAS_OBJECT_SMART_GET_OR_RETURN(eo_obj);

   if ((dx == 0) && (dy == 0)) return;
   _evas_object_smart_move_relative_internal(o, dx, dy);
}

void
_evas_object_smart_clipped_smart_move_internal(Evas_Object *eo_obj, Evas_Coord x, Evas_Coord y)
{
   Evas_Smart_Data *o = efl_data_scope_get(eo_obj, MY_CLASS);
   Evas_Coord orig_x, orig_y;

   orig_x = o->x;
   orig_y = o->y;
   o->x = x;
   o->y = y;
   _evas_object_smart_move_relative_internal(o, x - orig_x, y - orig_y);
}

void
_evas_object_smart_clipped_init(Evas_Object *eo_obj)
{
   EVAS_OBJECT_SMART_GET_OR_RETURN(eo_obj);
   Evas_Object_Smart_Clipped_Data *cso;
   Evas_Object *clipper;

   // user may realloc this... (legacy only!)
   cso = o->data;
   if (!cso)
     {
        cso = calloc(1, sizeof(*cso));
        o->data = cso;
        o->data_nofree = EINA_FALSE;
     }

   cso->evas = evas_object_evas_get(eo_obj);
   clipper = evas_object_rectangle_add(cso->evas);
   evas_object_static_clip_set(clipper, 1);
   cso->clipper = clipper;
   o->clipped = 0;
   evas_object_smart_member_add(clipper, eo_obj);
   o->clipped = 1;
   evas_object_color_set(cso->clipper, 255, 255, 255, 255);
   evas_object_move(cso->clipper, -100000, -100000);
   evas_object_resize(cso->clipper, 200000, 200000);
   evas_object_pass_events_set(cso->clipper, 1);
   evas_object_hide(cso->clipper); /* show when have something clipped to it */
   efl_canvas_object_no_render_set(cso->clipper, 1);
}

EOLIAN static void
_efl_canvas_group_group_add(Eo *eo_obj EINA_UNUSED, Evas_Smart_Data *o EINA_UNUSED)
{
}

EOLIAN static void
_efl_canvas_group_group_del(Eo *eo_obj EINA_UNUSED, Evas_Smart_Data *o)
{
   if (o->clipped)
     {
        _efl_canvas_group_group_members_all_del_internal(o);
     }
   o->group_del_called = EINA_TRUE;
}

EOLIAN static void
_efl_canvas_group_efl_canvas_object_no_render_set(Eo *eo_obj, Evas_Smart_Data *o, Eina_Bool enable)
{
   Evas_Object_Protected_Data *obj2;
   Eo *clipper;

   enable = !!enable;
   if (efl_canvas_object_no_render_get(eo_obj) == enable) return;

   efl_canvas_object_no_render_set(efl_super(eo_obj, MY_CLASS), enable);

   clipper = (o->clipped) ? _smart_clipper_get(o) : NULL;
   EINA_INLIST_FOREACH(o->contained, obj2)
     {
        if (obj2->object != clipper)
          efl_canvas_object_no_render_set(obj2->object, enable);
     }
}

EOLIAN static void
_efl_canvas_group_efl_gfx_color_color_set(Eo *eo_obj, Evas_Smart_Data *o, int r, int g, int b, int a)
{
   if (_evas_object_intercept_call(eo_obj, EVAS_OBJECT_INTERCEPT_CB_COLOR_SET, 0, r, g, b, a))
     return;

   efl_gfx_color_set(efl_super(eo_obj, MY_CLASS), r, g, b, a);

   if (o->clipped)
     {
        Evas_Object *clipper = _smart_clipper_get(o);
        EINA_SAFETY_ON_NULL_RETURN(clipper);

        efl_gfx_color_set(clipper, r, g, b, a);
        // Note: Legacy impl (and Widget) didn't call super in this case...
     }
}

EOLIAN static void
_efl_canvas_group_efl_gfx_entity_visible_set(Eo *eo_obj, Evas_Smart_Data *o, Eina_Bool vis)
{
   if (_evas_object_intercept_call(eo_obj, EVAS_OBJECT_INTERCEPT_CB_VISIBLE, 0, vis))
     return;

   efl_gfx_entity_visible_set(efl_super(eo_obj, MY_CLASS), vis);

   if (o->clipped)
     {
        Evas_Object *clipper = _smart_clipper_get(o);
        EINA_SAFETY_ON_NULL_RETURN(clipper);

        // note: maybe this is not necessary with no_render set on the clipper
        if (vis && !evas_object_clipees_has(clipper))
          return;

        efl_gfx_entity_visible_set(clipper, vis);
     }
}

EOLIAN static void
_efl_canvas_group_efl_gfx_entity_position_set(Eo *eo_obj, Evas_Smart_Data *o, Eina_Position2D pos)
{
   Eina_Bool is_overridden;
   Evas_Object_Protected_Data *obj = EVAS_OBJ_GET_OR_RETURN(eo_obj);

   if (_evas_object_intercept_call(eo_obj, EVAS_OBJECT_INTERCEPT_CB_MOVE, 0, pos.x, pos.y))
     return;

   is_overridden = (obj->is_smart && obj->smart.smart &&
                    obj->smart.smart->smart_class->move !=
                    (void *)evas_object_smart_clipped_smart_move);

   if (o->clipped && !is_overridden)
     _evas_object_smart_clipped_smart_move_internal(eo_obj, pos.x, pos.y);
   efl_gfx_entity_position_set(efl_super(eo_obj, MY_CLASS), pos);
}

EOLIAN static void
_efl_canvas_group_efl_canvas_object_clip_set(Eo *eo_obj, Evas_Smart_Data *o, Evas_Object *clip)
{
   EINA_SAFETY_ON_FALSE_RETURN(!clip || efl_isa(clip, EFL_CANVAS_OBJECT_CLASS));
   if (_evas_object_intercept_call(eo_obj, EVAS_OBJECT_INTERCEPT_CB_CLIP_SET, 0, clip))
     return;

   efl_canvas_object_clip_set(efl_super(eo_obj, MY_CLASS), clip);

   if (o->clipped)
     {
        Evas_Object *clipper = _smart_clipper_get(o);
        EINA_SAFETY_ON_NULL_RETURN(clipper);

        efl_canvas_object_clip_set(clipper, clip);
     }
}

void
evas_object_smart_attach(Evas_Object *eo_obj, Evas_Smart *s)
{
   Evas_Object_Protected_Data *obj = EVAS_OBJ_GET_OR_RETURN(eo_obj);
   unsigned int i;

   MAGIC_CHECK(s, Evas_Smart, MAGIC_SMART);
   return;
   MAGIC_CHECK_END();

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

   //efl_canvas_group_add(eo_obj);
   if (s->smart_class->add) s->smart_class->add(eo_obj);
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
   EVAS_OBJECT_SMART_GET_OR_RETURN(eo_obj);

   if (!event) return;
   if (!func) return;

   const Efl_Event_Description *eo_desc = efl_object_legacy_only_event_description_get(event);
   _eo_evas_smart_cb_info *cb_info = calloc(1, sizeof(*cb_info));
   cb_info->func = func;
   cb_info->data = (void *)data;
   cb_info->event = eo_desc;

   o->callbacks = eina_inlist_append(o->callbacks,
        EINA_INLIST_GET(cb_info));

   efl_event_callback_priority_add(eo_obj, eo_desc, priority, _eo_evas_smart_cb, cb_info);
}

EAPI void *
evas_object_smart_callback_del(Evas_Object *eo_obj, const char *event, Evas_Smart_Cb func)
{
   _eo_evas_smart_cb_info *info;

   EVAS_OBJECT_SMART_GET_OR_RETURN(eo_obj, NULL);

   if (!event) return NULL;

   const Efl_Event_Description *eo_desc = efl_object_legacy_only_event_description_get(event);

   EINA_INLIST_FOREACH(o->callbacks, info)
     {
        if ((info->func == func) && (info->event == eo_desc))
          {
             void *tmp = info->data;
             efl_event_callback_del(eo_obj, eo_desc, _eo_evas_smart_cb, info);

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
   _eo_evas_smart_cb_info *info;

   EVAS_OBJECT_SMART_GET_OR_RETURN(eo_obj, NULL);

   if (!event) return NULL;

   const Efl_Event_Description *eo_desc = efl_object_legacy_only_event_description_get(event);

   EINA_INLIST_FOREACH(o->callbacks, info)
     {
        if ((info->func == func) && (info->event == eo_desc) && (info->data == data))
          {
             void *tmp = info->data;
             efl_event_callback_del(eo_obj, eo_desc, _eo_evas_smart_cb, info);

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
   const Efl_Event_Description *eo_desc = efl_object_legacy_only_event_description_get(event);
   efl_event_callback_legacy_call(eo_obj, eo_desc, event_info);
}

EAPI Eina_Bool
evas_object_smart_callbacks_descriptions_set(Eo *eo_obj, const Evas_Smart_Cb_Description *descriptions)
{
   EVAS_OBJECT_SMART_GET_OR_RETURN(eo_obj, EINA_FALSE);
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

EAPI void
evas_object_smart_callbacks_descriptions_get(const Eo *eo_obj, const Evas_Smart_Cb_Description ***class_descriptions, unsigned int *class_count, const Evas_Smart_Cb_Description ***instance_descriptions, unsigned int *instance_count)
{
   EVAS_OBJECT_SMART_GET_OR_RETURN(eo_obj);
   if (class_descriptions) *class_descriptions = NULL;
   if (class_count) *class_count = 0;

   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
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
evas_object_smart_callback_description_find(const Eo *eo_obj, const char *name, const Evas_Smart_Cb_Description **class_description, const Evas_Smart_Cb_Description **instance_description)
{
   EVAS_OBJECT_SMART_GET_OR_RETURN(eo_obj);

   if (!name)
     {
        if (class_description) *class_description = NULL;
        if (instance_description) *instance_description = NULL;
        return;
     }

   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   if (class_description)
     *class_description = evas_smart_cb_description_find
        (&obj->smart.smart->callbacks, name);

   if (instance_description)
     *instance_description = evas_smart_cb_description_find
        (&o->callbacks_descriptions, name);
}

EOLIAN static void
_efl_canvas_group_group_need_recalculate_set(Eo *eo_obj, Evas_Smart_Data *o, Eina_Bool value)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);

   // XXX: do i need this?
   if (!obj || !obj->layer || obj->delete_me) return;

   evas_object_async_block(obj);
   /* remove this entry from calc_list or processed list */
   if (eina_clist_element_is_linked(&o->calc_entry))
     eina_clist_remove(&o->calc_entry);

   value = !!value;
   if (value)
     eina_clist_add_tail(&obj->layer->evas->calc_list, &o->calc_entry);
   else
     eina_clist_add_tail(&obj->layer->evas->calc_done, &o->calc_entry);

   if (o->need_recalculate == value) return;

   if (o->recalculate_cycle > 16382)
     {
        ERR("Object %p is not stable during recalc loop", eo_obj);
        return;
     }
   if (obj->layer->evas->in_smart_calc) o->recalculate_cycle++;
   o->need_recalculate = value;
}

EOLIAN static Eina_Bool
_efl_canvas_group_group_need_recalculate_get(const Eo *eo_obj EINA_UNUSED, Evas_Smart_Data *o)
{
   return o->need_recalculate;
}

EOLIAN static void
_efl_canvas_group_group_calculate(Eo *eo_obj, Evas_Smart_Data *o)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);

   if (!obj->smart.smart || !obj->smart.smart->smart_class->calculate)
     return;

   evas_object_async_block(obj);
   o->need_recalculate = 0;
   obj->smart.smart->smart_class->calculate(eo_obj);
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
   Evas_Public_Data *e = efl_data_scope_get(eo_e, EVAS_CANVAS_CLASS);

   evas_canvas_async_block(e);
   evas_event_freeze(eo_e);
   e->in_smart_calc++;

   while (NULL != (elem = eina_clist_head(&e->calc_list)))
     {
        Evas_Object_Protected_Data *obj;

        /* move the item to the processed list */
        o = EINA_CLIST_ENTRY(elem, Evas_Smart_Data, calc_entry);
        eina_clist_remove(&o->calc_entry);
        obj = efl_data_scope_get(o->object, EFL_CANVAS_OBJECT_CLASS);

        if (obj->delete_me) continue;
        eina_clist_add_tail(&e->calc_done, &o->calc_entry);

        if (o->need_recalculate)
          {
             o->need_recalculate = 0;
	     if (obj->smart.smart && obj->smart.smart->smart_class->calculate)
               obj->smart.smart->smart_class->calculate(obj->object);
             else
               efl_canvas_group_calculate(obj->object);
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
_efl_canvas_group_group_change(Eo *eo_obj, Evas_Smart_Data *o EINA_UNUSED)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   evas_object_async_block(obj);
   evas_object_change(eo_obj, obj);
   efl_canvas_group_need_recalculate_set(eo_obj, 1);
}

Eina_Bool
evas_object_smart_changed_get(Evas_Object_Protected_Data *obj)
{
   Eina_Bool has_map = EINA_FALSE;

   /* If object is invisible, it's meaningless to figure out changed state
      for rendering. */

   //a. Object itself visibility
   if (obj->no_render || (!obj->prev->visible && !obj->cur->visible) ||
       ((obj->prev->color.a == 0) && (obj->cur->color.a == 0)))
     return EINA_FALSE;

   //b. Object clipper visibility
   if ((obj->prev->clipper && obj->cur->clipper) &&
       ((!obj->prev->clipper->cur->visible &&
         !obj->cur->clipper->cur->visible) ||
        ((obj->prev->clipper->cur->color.a == 0) &&
         (obj->cur->clipper->cur->color.a == 0))))
     return EINA_FALSE;

   if (!obj->clip.clipees)
     {
        has_map = _evas_render_has_map(obj) && !_evas_render_can_map(obj);
        if (obj->changed && !obj->is_smart && !has_map) return EINA_TRUE;

        if (has_map)
          {
             if ((obj->need_surface_clear && obj->changed && !obj->is_smart) ||
                 ((obj->changed_pchange) && (obj->changed_map)))
               return EINA_TRUE;
          }
     }

   if (obj->is_smart)
     {
        Evas_Object_Protected_Data *o2;

        EINA_INLIST_FOREACH(evas_object_smart_members_get_direct(obj->object), o2)
          if (evas_object_smart_changed_get(o2)) return EINA_TRUE;
     }

   return EINA_FALSE;
}

void
evas_object_smart_del(Evas_Object *eo_obj)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   Evas_Smart_Data *sobj;
   Evas_Smart *s;
   unsigned int i;

   if (obj->delete_me) return;

   sobj = efl_data_scope_get(eo_obj, MY_CLASS);
   s = obj->smart.smart;
   if (s && s->smart_class->del)
     {
        s->smart_class->del(eo_obj);
        //this is legacy, this will never be called..., smart things dont have inheritance
        sobj->group_del_called = EINA_TRUE;
     }
   else
      efl_canvas_group_del(eo_obj);
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

   free(sobj->interface_privates);
   sobj->interface_privates = NULL;

   if (s) evas_object_smart_unuse(s);
}

void
evas_object_update_bounding_box(Evas_Object *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj, Evas_Smart_Data *s)
{
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
        if (!s) s = obj->private_data;

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
   Evas_Object_Protected_Data *smart_obj = obj->smart.parent_object_data;
   Evas_Smart_Data *smart_parent = obj->smart.parent_data;
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
             smart_parent->cur.bounding_box.h += smart_parent->cur.bounding_box.y - y;
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
             evas_object_smart_need_bounding_box_update(obj->smart.parent_data,
                                                        obj->smart.parent_object_data);
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
     evas_object_update_bounding_box(obj->smart.parent, smart_obj, smart_parent);
}

void
evas_object_smart_bounding_box_get(Evas_Object_Protected_Data *obj,
                                   Eina_Rectangle *cur_bounding_box,
                                   Eina_Rectangle *prev_bounding_box)
{
   Evas_Smart_Data *s = obj->private_data;

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
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);

   if (obj->smart.parent)
     evas_object_smart_member_del(eo_obj);

   if (obj->is_smart)
     {
        Evas_Smart_Data *o = efl_data_scope_get(eo_obj, MY_CLASS);
        if (o->calc_entry.next)
          eina_clist_remove(&o->calc_entry);

        if (o->render_cache)
          {
             evas_render_object_render_cache_free(eo_obj, o->render_cache);
             o->render_cache = NULL;
          }

        while (o->contained)
          {
             Evas_Object_Protected_Data *contained =
               (Evas_Object_Protected_Data *)o->contained;
             Evas_Object *contained_obj = contained->object;

             if (!contained_obj)
               {
                  ERR("Found an undefined object %p in %s.", contained, efl_debug_name_get(eo_obj));
                  o->contained = eina_inlist_remove
                    (o->contained, EINA_INLIST_GET(contained));
               }
             else if (contained->smart.parent != eo_obj)
               {
                  Evas_Layer *lay = obj->layer;

                  ERR("This is bad - object %p in child list for %p has parent %p", contained_obj, eo_obj, contained->smart.parent);
                  o->contained = eina_inlist_remove
                    (o->contained, EINA_INLIST_GET(contained));
                  if (lay)
                    {
                       // this SHOULD be eina_inlist_append() BUT seemingly
                       // if we call this this object gets magically added
                       // back to o->contained above NOT lay->objects. this
                       // is utterly bizarre and the only explanation i
                       // can come up with right now is a compiler bug.
                       lay->objects = (Evas_Object_Protected_Data *)
                         eina_inlist_prepend(EINA_INLIST_GET(lay->objects),
                                             EINA_INLIST_GET(contained));
                       if (contained->layer != lay)
                         {
                            if (contained->layer) contained->layer->usage--;
                            contained->layer = lay;
                            contained->in_layer = 1;
                            lay->usage++;
                         }
                    }
               }
             else evas_object_smart_member_del(contained_obj);
          }

        while (o->callbacks)
          {
             _eo_evas_smart_cb_info *info = (_eo_evas_smart_cb_info *)o->callbacks;
             efl_event_callback_del(eo_obj, info->event, _eo_evas_smart_cb, info);
             o->callbacks = eina_inlist_remove(o->callbacks, EINA_INLIST_GET(info));
             free(info);
          }

        if (o->render_cache)
          {
             evas_render_object_render_cache_free(eo_obj, o->render_cache);
             o->render_cache = NULL;
          }

        evas_smart_cb_descriptions_resize(&o->callbacks_descriptions, 0);
        evas_object_smart_data_set(eo_obj, NULL);
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

   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   Evas_Object_Protected_Data *member;

   if (pass_events)
     obj->parent_cache.pass_events_valid = EINA_FALSE;
   if (freeze_events)
     obj->parent_cache.freeze_events_valid = EINA_FALSE;
   if (source_invisible)
     obj->parent_cache.src_invisible_valid = EINA_FALSE;

   if (!obj->is_smart) return;
   Evas_Smart_Data *o = efl_data_scope_get(eo_obj, MY_CLASS);
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
   Evas_Object_Protected_Data *member = efl_data_scope_get(eo_member, EFL_CANVAS_OBJECT_CLASS);
   o = efl_data_scope_get(member->smart.parent, MY_CLASS);
   o->contained = eina_inlist_demote(o->contained, EINA_INLIST_GET(member));
}

void
evas_object_smart_member_lower(Evas_Object *eo_member)
{
   Evas_Smart_Data *o;
   Evas_Object_Protected_Data *member = efl_data_scope_get(eo_member, EFL_CANVAS_OBJECT_CLASS);
   o = efl_data_scope_get(member->smart.parent, MY_CLASS);
   o->contained = eina_inlist_promote(o->contained, EINA_INLIST_GET(member));
}

void
evas_object_smart_member_stack_above(Evas_Object *eo_member, Evas_Object *eo_other)
{
   Evas_Smart_Data *o;
   Evas_Object_Protected_Data *member = efl_data_scope_get(eo_member, EFL_CANVAS_OBJECT_CLASS);
   Evas_Object_Protected_Data *other = efl_data_scope_get(eo_other, EFL_CANVAS_OBJECT_CLASS);
   o = efl_data_scope_get(member->smart.parent, MY_CLASS);
   o->contained = eina_inlist_remove(o->contained, EINA_INLIST_GET(member));
   o->contained = eina_inlist_append_relative(o->contained, EINA_INLIST_GET(member), EINA_INLIST_GET(other));
}

void
evas_object_smart_member_stack_below(Evas_Object *eo_member, Evas_Object *eo_other)
{
   Evas_Smart_Data *o;
   Evas_Object_Protected_Data *member = efl_data_scope_get(eo_member, EFL_CANVAS_OBJECT_CLASS);
   Evas_Object_Protected_Data *other = efl_data_scope_get(eo_other, EFL_CANVAS_OBJECT_CLASS);
   o = efl_data_scope_get(member->smart.parent, MY_CLASS);
   o->contained = eina_inlist_remove(o->contained, EINA_INLIST_GET(member));
   o->contained = eina_inlist_prepend_relative(o->contained, EINA_INLIST_GET(member), EINA_INLIST_GET(other));
}

void
evas_object_smart_need_bounding_box_update(Evas_Smart_Data *o, Evas_Object_Protected_Data *obj)
{
   if (o->update_boundingbox_needed) return;
   o->update_boundingbox_needed = EINA_TRUE;
   if (!obj->cur->cache.clip.dirty)
     {
        EINA_COW_STATE_WRITE_BEGIN(obj, state_write, cur)
          state_write->cache.clip.dirty = EINA_TRUE;
        EINA_COW_STATE_WRITE_END(obj, state_write, cur);
     }

   if (obj->smart.parent)
     evas_object_smart_need_bounding_box_update(obj->smart.parent_data,
                                                obj->smart.parent_object_data);
}

void
evas_object_smart_bounding_box_update(Evas_Object_Protected_Data *obj)
{
   Evas_Smart_Data *os;
   Eina_Inlist *list;
   Evas_Object_Protected_Data *o;
   Evas_Coord minx = 0x7fffffff;
   Evas_Coord miny = 0x7fffffff;
   Evas_Coord maxx = 0x80000000;
   Evas_Coord maxy = 0x80000000;
   Evas_Coord tx1, ty1, tx2, ty2;
   Eina_Bool none = EINA_TRUE;

   os = obj->private_data;

   if (!os->update_boundingbox_needed) return;
   os->update_boundingbox_needed = EINA_FALSE;

   list = os->contained;
   EINA_INLIST_FOREACH(list, o)
     {
        if (o == obj) continue ;
        if (o->clip.clipees || o->is_static_clip) continue ;
        if (!o->cur->visible) continue;

        none = EINA_FALSE;

        if (o->is_smart)
          {
             Evas_Smart_Data *s = o->private_data;

             evas_object_smart_bounding_box_update(o);

             tx1 = s->cur.bounding_box.x;
             ty1 = s->cur.bounding_box.y;
             tx2 = tx1 + s->cur.bounding_box.w;
             ty2 = ty1 + s->cur.bounding_box.h;
          }
        else
          {
             tx1 = o->cur->geometry.x;
             ty1 = o->cur->geometry.y;
             tx2 = tx1 + o->cur->geometry.w;
             ty2 = ty1 + o->cur->geometry.h;
          }

        if (tx1 < minx) minx = tx1;
        if (ty1 < miny) miny = ty1;
        if (tx2 > maxx) maxx = tx2;
        if (ty2 > maxy) maxy = ty2;
     }
   if (none)
     {
        minx = obj->cur->geometry.x;
        miny = obj->cur->geometry.y;
        maxx = obj->cur->geometry.x + obj->cur->geometry.w;
        maxy = obj->cur->geometry.y + obj->cur->geometry.h;
     }

   os->cur.bounding_box.x = minx;
   os->cur.bounding_box.y = miny;
   os->cur.bounding_box.w = maxx - minx;
   os->cur.bounding_box.h = maxy - miny;

   EINA_COW_STATE_WRITE_BEGIN(obj, state_write, cur)
     {
        state_write->cache.clip.dirty = EINA_TRUE;
     }
   EINA_COW_STATE_WRITE_END(obj, state_write, cur);
   evas_object_clip_recalc(obj);
   if (obj->cur->clipper) evas_object_clip_recalc(obj->cur->clipper);
}

/* all nice and private */
static void
evas_object_smart_render(Evas_Object *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj EINA_UNUSED,  void *type_private_data EINA_UNUSED, void *engine EINA_UNUSED, void *output EINA_UNUSED, void *context EINA_UNUSED, void *surface EINA_UNUSED, int x EINA_UNUSED, int y EINA_UNUSED, Eina_Bool do_async EINA_UNUSED)
{
   return;
}

static void
evas_object_smart_render_pre(Evas_Object *eo_obj,
			     Evas_Object_Protected_Data *obj,
			     void *type_private_data EINA_UNUSED)
{
   if (obj->pre_render_done) return;

   if (obj->changed_map || obj->changed_src_visible)
     evas_object_render_pre_prev_cur_add(&obj->layer->evas->clip_changes,
                                         eo_obj, obj);

   obj->pre_render_done = EINA_TRUE;
}

static void
evas_object_smart_render_post(Evas_Object *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj, void *type_private_data)
{
   Evas_Smart_Data *o = type_private_data;
   evas_object_cur_prev(obj);
   o->prev = o->cur;
}

static void *evas_object_smart_engine_data_get(Evas_Object *eo_obj)
{
   Evas_Smart_Data *o = efl_data_scope_get(eo_obj, MY_CLASS);
   if (!o) return NULL;
   return o->engine_data;
}

static void
_efl_canvas_group_class_constructor(Efl_Class *klass EINA_UNUSED)
{
   _evas_smart_class_names_hash_table = eina_hash_string_small_new(NULL);
   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}

static void
_efl_canvas_group_class_destructor(Efl_Class *klass EINA_UNUSED)
{
   eina_hash_free(_evas_smart_class_names_hash_table);
}

static void
_efl_canvas_group_group_paragraph_direction_set_internal(Eo *eo_obj,
                                                    Evas_BiDi_Direction dir)
{
   Evas_Object_Protected_Data *o;
   Evas_Smart_Data *member_o;

   EINA_INLIST_FOREACH(evas_object_smart_members_get_direct(eo_obj), o)
     {
        evas_object_change(o->object, o);

        if (o->is_smart)
          {
             member_o = efl_data_scope_get(o->object, MY_CLASS);

             if ((member_o->inherit_paragraph_direction) &&
                 (member_o->paragraph_direction != dir))
               {
                  member_o->paragraph_direction = dir;
                  _efl_canvas_group_group_paragraph_direction_set_internal(o->object, dir);
               }
          }
     }
}

EOLIAN static void
_efl_canvas_group_efl_canvas_object_paragraph_direction_set(Eo *eo_obj, Evas_Smart_Data *o,
                                                            Evas_BiDi_Direction dir)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   Evas_Smart_Data *parent;

   if ((!(o->inherit_paragraph_direction) && (o->paragraph_direction == dir)) ||
       (o->inherit_paragraph_direction && (dir == EVAS_BIDI_DIRECTION_INHERIT)))
     return;

   if (dir == EVAS_BIDI_DIRECTION_INHERIT)
     {
        o->inherit_paragraph_direction = EINA_TRUE;
        Evas_BiDi_Direction parent_dir = EVAS_BIDI_DIRECTION_NEUTRAL;

        if (obj->smart.parent)
          {
             parent = efl_data_scope_get(obj->smart.parent, MY_CLASS);

             if (parent)
               parent_dir = parent->paragraph_direction;
          }

        if (parent_dir != o->paragraph_direction)
          {
             o->paragraph_direction = parent_dir;
             evas_object_change(eo_obj, obj);
          }
     }
   else
     {
        o->inherit_paragraph_direction = EINA_FALSE;
        o->paragraph_direction = dir;
        evas_object_change(eo_obj, obj);
     }

   _efl_canvas_group_group_paragraph_direction_set_internal(eo_obj, o->paragraph_direction);
}

EOLIAN static Evas_BiDi_Direction
_efl_canvas_group_efl_canvas_object_paragraph_direction_get(const Eo *eo_obj EINA_UNUSED, Evas_Smart_Data *o)
{
   return o->paragraph_direction;
}

EOLIAN static const Eo *
_efl_canvas_group_group_clipper_get(const Eo *eo_obj EINA_UNUSED, Evas_Smart_Data *o)
{
   // NOTE: This may be NULL until all EO smart objects are clipped!
   return _smart_clipper_get(o);
}

/* Internal EO */
static void
_efl_canvas_group_group_clipped_set(Eo *eo_obj EINA_UNUSED, Evas_Smart_Data *sd, Eina_Bool clipped)
{
   // We must call this function BEFORE the constructor (yes, it's hacky)
   EINA_SAFETY_ON_FALSE_RETURN(!sd->object);
   sd->clipped = !!clipped;
}

/* Internal EO APIs */

EOAPI EFL_VOID_FUNC_BODY(efl_canvas_group_add)
EOAPI EFL_VOID_FUNC_BODY(efl_canvas_group_del)
EOAPI EFL_VOID_FUNC_BODYV(efl_canvas_group_clipped_set, EFL_FUNC_CALL(enable), Eina_Bool enable)

#define EFL_CANVAS_GROUP_EXTRA_OPS \
   EFL_OBJECT_OP_FUNC(efl_canvas_group_add, _efl_canvas_group_group_add), \
   EFL_OBJECT_OP_FUNC(efl_canvas_group_del, _efl_canvas_group_group_del), \
   EFL_OBJECT_OP_FUNC(efl_canvas_group_clipped_set, _efl_canvas_group_group_clipped_set)

#include "canvas/efl_canvas_group.eo.c"
