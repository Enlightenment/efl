#ifndef _EO_PRIVATE_H
#define _EO_PRIVATE_H

#include <Eo.h>
#include <Eina.h>

#define EO_EINA_MAGIC 0xa186bc32
#define EO_EINA_MAGIC_STR "Eo"
#define EO_FREED_EINA_MAGIC 0xa186bb32
#define EO_FREED_EINA_MAGIC_STR "Eo - Freed object"
#define EO_CLASS_EINA_MAGIC 0xa186ba32
#define EO_CLASS_EINA_MAGIC_STR "Efl Class"

#define EO_MAGIC_RETURN_VAL(d, magic, ret) \
   do { \
        if (!EINA_MAGIC_CHECK(d, magic)) \
          { \
             EINA_MAGIC_FAIL(d, magic); \
             return ret; \
          } \
   } while (0)

#define EO_MAGIC_RETURN(d, magic) \
   do { \
        if (!EINA_MAGIC_CHECK(d, magic)) \
          { \
             EINA_MAGIC_FAIL(d, magic); \
             return; \
          } \
   } while (0)


extern int _eo_log_dom;

#ifdef CRI
#undef CRI
#endif
#define CRI(...) EINA_LOG_DOM_CRIT(_eo_log_dom, __VA_ARGS__)

#ifdef ERR
#undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_eo_log_dom, __VA_ARGS__)

#ifdef WRN
#undef WRN
#endif
#define WRN(...) EINA_LOG_DOM_WARN(_eo_log_dom, __VA_ARGS__)

#ifdef INF
#undef INF
#endif
#define INF(...) EINA_LOG_DOM_INFO(_eo_log_dom, __VA_ARGS__)

#ifdef DBG
#undef DBG
#endif
#define DBG(...) EINA_LOG_DOM_DBG(_eo_log_dom, __VA_ARGS__)

typedef uintptr_t Eo_Id;
typedef struct _Efl_Class _Efl_Class;
typedef struct _Eo_Header Eo_Header;
typedef struct _Efl_Object_Optional Efl_Object_Optional;

/* Allocates an entry for the given object */
static inline Eo_Id _eo_id_allocate(const _Eo_Object *obj, const Eo *parent_id);

/* Releases an entry by the object id */
static inline void _eo_id_release(const Eo_Id obj_id);

void _eo_condtor_done(Eo *obj);

typedef struct _Dich_Chain1 Dich_Chain1;

typedef struct _Eo_Vtable
{
   Dich_Chain1 *chain;
   unsigned int size;
} Eo_Vtable;

/* Clean the vtable. */
void _vtable_func_clean_all(Eo_Vtable *vtable);

struct _Eo_Header
{
     Eo_Id id;
};

struct _Efl_Object_Optional
{
   Eo_Vtable          *vtable;
   Eina_List          *composite_objects;
   Efl_Del_Intercept   del_intercept;
};

struct _Eo_Object
{
     Eo_Header header;
     EINA_INLIST;
     const _Efl_Class *klass;
#ifdef EO_DEBUG
     Eina_Inlist *xrefs;
     Eina_Inlist *data_xrefs;
#endif

     const Efl_Object_Optional *opt; // eina cow
     _Efl_Class *cur_klass;

     short refcount;
     short user_refcount;
#ifdef EO_DEBUG
     short datarefcount;
#endif

     Eina_Bool condtor_done:1;
     Eina_Bool finalized:1;
     Eina_Bool super:1;
     Eina_Bool invalidate:1;
     Eina_Bool is_invalidating:1;
     Eina_Bool parent : 1;
     Eina_Bool unref_compensate : 1;
     Eina_Bool allow_parent_unref : 1;

     Eina_Bool noref_event : 1;
     Eina_Bool del_triggered:1;
     Eina_Bool destructed:1;
     Eina_Bool manual_free:1;
     unsigned char auto_unref : 1; // unref after 1 call - hack for parts
};

/* How we search and store the implementations in classes. */
#define DICH_CHAIN_LAST_BITS 5
#define DICH_CHAIN_LAST_SIZE (1 << DICH_CHAIN_LAST_BITS)
#define DICH_CHAIN1(x) ((x) >> DICH_CHAIN_LAST_BITS)
#define DICH_CHAIN_LAST(x) ((x) & ((1 << DICH_CHAIN_LAST_BITS) - 1))

extern Eina_Cow *efl_object_optional_cow;
#define EO_OPTIONAL_COW_WRITE(_obj) ({ Efl_Object_Optional *_cow = eina_cow_write(efl_object_optional_cow, (const Eina_Cow_Data**)&(_obj->opt)); _cow; })
#define EO_OPTIONAL_COW_END(_cow, _obj) eina_cow_done(efl_object_optional_cow, (const Eina_Cow_Data**)&(_obj->opt), _cow, EINA_TRUE)
#define EO_OPTIONAL_COW_SET(_obj, _field, _value) do { \
   typeof (_value) _val = _value; \
   if (_obj->opt->_field != _val) {\
      Efl_Object_Optional *_obj##_cow = EO_OPTIONAL_COW_WRITE(_obj); \
      _obj##_cow->_field = _val; \
      EO_OPTIONAL_COW_END(_obj##_cow, _obj); \
   }} while (0)
#define EO_VTABLE(_obj) ((_obj)->opt->vtable ?: &((_obj)->klass->vtable))

typedef void (*Eo_Op_Func_Type)(Eo *, void *class_data);

typedef struct
{
   Eo_Op_Func_Type func;
   const _Efl_Class *src;
} op_type_funcs;

typedef struct _Dich_Chain2
{
   op_type_funcs funcs[DICH_CHAIN_LAST_SIZE];
   unsigned short refcount;
} Dich_Chain2;

struct _Dich_Chain1
{
   Dich_Chain2 *chain2;
};

typedef struct
{
   const _Efl_Class *klass;
   size_t offset;
} Eo_Extension_Data_Offset;

struct _Efl_Class
{
   Eo_Header header;

   const _Efl_Class *parent;
   const Efl_Class_Description *desc;
   Eo_Vtable vtable;

   const _Efl_Class **extensions;

   Eo_Extension_Data_Offset *extn_data_off;

   const _Efl_Class **mro;

   const Efl_Object_Property_Reflection_Ops *reflection;

   /* cached object for faster allocation */
   struct {
      Eina_Trash  *trash;
      Eina_Spinlock    trash_lock;
      unsigned int trash_count;
   } objects;

   /* cached iterator for faster allocation cycle */
   struct {
      Eina_Trash   *trash;
      Eina_Spinlock trash_lock;
      unsigned int  trash_count;
   } iterators;

   unsigned int obj_size; /**< size of an object of this class */
   unsigned int base_id;
   unsigned int data_offset; /* < Offset of the data within object data. */
   unsigned int ops_count; /* < Offset of the data within object data. */

   Eina_Thread construction_thread; /** < the thread which called the class constructor */

   Eina_Bool constructed : 1;
   Eina_Bool functions_set : 1;
   /* [extensions*] + NULL */
   /* [mro*] + NULL */
   /* [extensions data offset] + NULL */
};

typedef struct
{
   EINA_INLIST;
   const Eo *ref_obj;
   const char *data_klass;
   const char *file;
   int line;
} Eo_Xref_Node;

/* provide valgrind-like tracking of object allocationg and deletion */
void _eo_log_obj_report(const Eo_Id id, int log_level, const char *func_name, const char *file, int line);

void _efl_object_reuse(_Eo_Object *obj);

static inline
Eo *_eo_header_id_get(const Eo_Header *header)
{
   return (Eo *) header->id;
}

/* Retrieves the pointer to the object from the id */
_Eo_Object *_eo_obj_pointer_get(const Eo_Id obj_id, const char *func_name, const char *file, int line);

static inline
Efl_Class *_eo_class_id_get(const _Efl_Class *klass)
{
   return _eo_header_id_get((Eo_Header *) klass);
}

static inline
Eo *_eo_obj_id_get(const _Eo_Object *obj)
{
   return _eo_header_id_get((Eo_Header *) obj);
}

static inline void
_eo_condtor_reset(_Eo_Object *obj)
{
   obj->condtor_done = EINA_FALSE;
}

typedef struct _Efl_Object_Data Efl_Object_Data;

EOLIAN void _efl_object_parent_set(Eo *obj, Efl_Object_Data *pd, Eo *parent_id);
void _efl_invalidate(_Eo_Object *obj);

static inline void
_efl_del_internal(_Eo_Object *obj, const char *func_name, const char *file, int line)
{
   /* We need that for the event callbacks that may ref/unref. */
   obj->refcount++;

   const _Efl_Class *klass = obj->klass;

   // If the object hasn't been invalidated yet, time to do it
   // before any destructor kick in. This can happen when
   // the object has no parent and get deleted by efl_unref.
   if (obj->parent)
     {
        Eo *parent = efl_parent_get(_eo_obj_id_get(obj));

        ERR("Destructor path being taken while object [%s] still has a parent [%s] in state %i:%i.",
            efl_debug_name_get(_eo_obj_id_get(obj)),
            efl_debug_name_get(parent),
            obj->is_invalidating, obj->invalidate);

        efl_parent_set(_eo_obj_id_get(obj), NULL);

        if (obj->parent)
          {
             CRI("Something is preventing [%s] from disconnecting from its parent, bypassing.",
                 efl_debug_name_get(_eo_obj_id_get(obj)));
             _efl_object_parent_set(_eo_obj_id_get(obj), efl_data_scope_get(_eo_obj_id_get(obj), EFL_OBJECT_CLASS), NULL);
          }
     }
   else if (!obj->invalidate || !obj->is_invalidating)
     {
        _efl_invalidate(obj);
     }

   efl_event_callback_call(_eo_obj_id_get(obj), EFL_EVENT_DEL, NULL);

   _eo_condtor_reset(obj);

   efl_destructor(_eo_obj_id_get(obj));

   if (!obj->condtor_done)
     {
        ERR("in %s:%d: func '%s' Object of class '%s' - Not all of the object destructors have been executed.",
            file, line, func_name, klass->desc->name);
     }
   /*FIXME: add eo_class_unref(klass) ? - just to clear the caches. */

     {
        Eina_List *itr, *itr_n;
        Eo *emb_obj;
        EINA_LIST_FOREACH_SAFE(obj->opt->composite_objects, itr, itr_n, emb_obj)
          {
             efl_composite_detach(_eo_obj_id_get(obj), emb_obj);
          }
     }

   obj->destructed = EINA_TRUE;
   obj->refcount--;
}

static inline Eina_Bool
_obj_is_override(_Eo_Object *obj)
{
   return obj->opt->vtable != NULL;
}

void _eo_free(_Eo_Object *obj, Eina_Bool manual_free);

static inline _Eo_Object *
_efl_ref(_Eo_Object *obj)
{
   obj->refcount++;
   return obj;
}

#define _efl_unref(obj) _efl_unref_internal(obj, __FUNCTION__, __FILE__, __LINE__)
static inline void
_efl_unref_internal(_Eo_Object *obj, const char *func_name, const char *file, int line)
{
   --(obj->refcount);
   if (EINA_UNLIKELY(obj->refcount <= 0))
     {
        if (obj->user_refcount > 0)
          {
             ERR("Object %p is still refcounted %i by users, but internal refcount reached 0. This should never happen. Please report a bug and send a backtrace to EFL developer.", (Eo*) obj->header.id, obj->user_refcount);
             _eo_log_obj_report((Eo_Id)_eo_obj_id_get(obj), EINA_LOG_LEVEL_ERR, __FUNCTION__, __FILE__, __LINE__);
             return;
          }
        if (obj->refcount < 0)
          {
             ERR("in %s:%d: func '%s' Obj:%p. Refcount (%d) < 0. Too many unrefs.", file, line, func_name, obj, obj->refcount);
             _eo_log_obj_report((Eo_Id)_eo_obj_id_get(obj), EINA_LOG_LEVEL_ERR, __FUNCTION__, __FILE__, __LINE__);
             return;
          }

        if (obj->destructed)
          {
             ERR("in %s:%d: func '%s' Object %p already destructed.", file, line, func_name, _eo_obj_id_get(obj));
             _eo_log_obj_report((Eo_Id)_eo_obj_id_get(obj), EINA_LOG_LEVEL_ERR, __FUNCTION__, __FILE__, __LINE__);
             return;
          }

        if (obj->del_triggered)
          {
             ERR("in %s:%d: func '%s' Object %p deletion already triggered. You wrongly call efl_unref() within a destructor.", file, line, func_name, _eo_obj_id_get(obj));
             _eo_log_obj_report((Eo_Id)_eo_obj_id_get(obj), EINA_LOG_LEVEL_ERR, __FUNCTION__, __FILE__, __LINE__);
             return;
          }

        if (obj->opt->del_intercept)
          {
             Eo *obj_id = _eo_obj_id_get(obj);
             efl_ref(obj_id);
             obj->opt->del_intercept(obj_id);
             return;
          }

        obj->del_triggered = EINA_TRUE;

        _efl_del_internal(obj, func_name, file, line);

        if (EINA_LIKELY(!obj->manual_free))
          {
#ifdef EO_DEBUG
             /* If for some reason it's not empty, clear it. */
             Eo *obj_id = _eo_obj_id_get(obj);
             while (obj->xrefs)
               {
                  Eina_Inlist *nitr = obj->xrefs->next;
                  Eo_Xref_Node *xref = EINA_INLIST_CONTAINER_GET(obj->xrefs, Eo_Xref_Node);
                  ERR("in %s:%d: func '%s' Object %p is still referenced by object %p. Origin: %s:%d",
                      file, line, func_name, obj_id, xref->ref_obj, xref->file, xref->line);
                  eina_freeq_ptr_main_add(xref, free, sizeof(*xref));
                  obj->xrefs = nitr;
               }
             while (obj->data_xrefs)
               {
                  Eina_Inlist *nitr = obj->data_xrefs->next;
                  Eo_Xref_Node *xref = EINA_INLIST_CONTAINER_GET(obj->data_xrefs, Eo_Xref_Node);
                  if (obj_id == xref->ref_obj)
                    {
                       WRN("in %s:%d: func '%s' Object %p still has a reference to its own data (subclass: %s). Origin: %s:%d",
                           file, line, func_name, obj_id, xref->data_klass, xref->file, xref->line);
                    }
                  else
                    {
                       ERR("in %s:%d: func '%s' Data of object %p (subclass: %s) is still referenced by object %p. Origin: %s:%d",
                           file, line, func_name, obj_id, xref->data_klass, xref->ref_obj, xref->file, xref->line);
                    }

                  eina_freeq_ptr_main_add(xref, free, sizeof(*xref));
                  obj->data_xrefs = nitr;
               }
#endif

             _eo_free(obj, EINA_FALSE);
          }
        else
          _efl_ref(obj); /* If we manual free, we keep a phantom ref. */
     }
}

#endif
