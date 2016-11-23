#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Eina.h>
#include <fnmatch.h>

#include "Eo.h"
#include "eo_ptr_indirection.h"
#include "eo_private.h"

#define EFL_EVENT_SPECIAL_SKIP 1

static int event_freeze_count = 0;

typedef struct _Eo_Callback_Description Eo_Callback_Description;

typedef struct
{
   const char                *name;
   const char                *comment;
   Eo                        *composite_parent;
   Eina_Inlist               *generic_data;
   Eo                      ***wrefs;
   Eina_List                 *futures;
} Efl_Object_Extension;

typedef struct
{
   Eina_Inlist               *children;
   Eo                        *parent;

   Efl_Object_Extension      *ext;

   Eina_Inlist               *current;

   Eo_Callback_Description  **callbacks;
   unsigned int               callbacks_count;

   unsigned short             walking_list;
   unsigned short             event_freeze_count;
#ifdef EFL_EVENT_SPECIAL_SKIP
   unsigned short             event_cb_efl_event_callback_add_count;
   unsigned short             event_cb_efl_event_callback_del_count;
   unsigned short             event_cb_efl_event_del_count;
#endif
   Eina_Bool                  deletions_waiting : 1;
   Eina_Bool                  callback_stopped : 1;
   Eina_Bool                  parent_sunk : 1; // If parent ref has already been settled (parent has been set, or we are in add_ref mode
} Efl_Object_Data;

typedef enum {
     DATA_PTR,
     DATA_OBJ,
     DATA_OBJ_WEAK,
     DATA_VAL
} Eo_Generic_Data_Node_Type;

typedef struct
{
   EINA_INLIST;
   const Eo                  *obj;
   Eina_Stringshare          *key;
   union {
        Eina_Value           *val;
        Eo                   *obj;
        void                 *ptr;
   } d;
   Eo_Generic_Data_Node_Type  d_type;
} Eo_Generic_Data_Node;

typedef struct
{
   EINA_INLIST;
   const Efl_Event_Description *desc;
   unsigned int current;
} Eo_Current_Callback_Description;

static inline void
_efl_object_extension_free(Efl_Object_Extension *ext)
{
   eina_freeq_ptr_main_add(ext, free, sizeof(*ext));
}

static inline Efl_Object_Extension *
_efl_object_extension_need(Efl_Object_Data *pd)
{
   if (!pd->ext) pd->ext = calloc(1, sizeof(Efl_Object_Extension));
   return pd->ext;
}

static inline void
_efl_object_extension_noneed(Efl_Object_Data *pd)
{
   Efl_Object_Extension *ext = pd->ext;
   if ((!ext) ||
       (ext->name) ||
       (ext->comment) ||
       (ext->generic_data) ||
       (ext->wrefs) ||
       (ext->composite_parent) ||
       (ext->futures)) return;
   _efl_object_extension_free(pd->ext);
   pd->ext = NULL;
}

static void _key_generic_cb_del(void *data, const Efl_Event *event);

static void
_eo_generic_data_node_free(Eo_Generic_Data_Node *node)
{
   switch (node->d_type)
     {
      case DATA_PTR:
        break;
      case DATA_OBJ:
        efl_event_callback_del(node->d.obj, EFL_EVENT_DEL, _key_generic_cb_del, node);
        efl_unref(node->d.obj);
        break;
      case DATA_OBJ_WEAK:
        efl_event_callback_del(node->d.obj, EFL_EVENT_DEL, _key_generic_cb_del, node);
        break;
      case DATA_VAL:
        eina_value_free(node->d.val);
        break;
     }
   eina_stringshare_del(node->key);
   eina_freeq_ptr_main_add(node, free, sizeof(*node));
}

static void
_eo_generic_data_del_all(Eo *obj EINA_UNUSED, Efl_Object_Data *pd)
{
   Eo_Generic_Data_Node *node;
   Efl_Object_Extension *ext = pd->ext;

   if (!ext) return;

   while (ext->generic_data)
     {
        node = (Eo_Generic_Data_Node *)ext->generic_data;
        ext->generic_data = eina_inlist_remove(ext->generic_data,
              EINA_INLIST_GET(node));

        _eo_generic_data_node_free(node);
     }
}

static void
_eo_key_generic_direct_del(Efl_Object_Data *pd, Eo_Generic_Data_Node *node, Eina_Bool call_free)
{
   Efl_Object_Extension *ext = pd->ext;

   ext->generic_data = eina_inlist_remove
     (ext->generic_data, EINA_INLIST_GET(node));
   if (call_free) _eo_generic_data_node_free(node);
}

static void
_eo_key_generic_del(const Eo *obj EINA_UNUSED, Efl_Object_Data *pd, const char *key, Eina_Bool call_free)
{
   Eo_Generic_Data_Node *node;
   Efl_Object_Extension *ext = pd->ext;

   EINA_INLIST_FOREACH(ext->generic_data, node)
     {
        if (!strcmp(node->key, key))
          {
             ext->generic_data = eina_inlist_remove
               (ext->generic_data, EINA_INLIST_GET(node));
             if (call_free) _eo_generic_data_node_free(node);
             return;
          }
     }
}

/* Return TRUE if the object was newly added. */
static Eo_Generic_Data_Node *
_key_generic_set(const Eo *obj, Efl_Object_Data *pd, const char *key, const void *data, Eo_Generic_Data_Node_Type d_type, Eina_Bool call_free)
{
   Eo_Generic_Data_Node *node;
   Efl_Object_Extension *ext = pd->ext;

   if (!key) return NULL;
   if (ext)
     {
        if (!data)
          {
             _eo_key_generic_del(obj, pd, key, call_free);
             return NULL;
          }
        EINA_INLIST_FOREACH(ext->generic_data, node)
          {
             if (!strcmp(node->key, key))
               {
                  if ((node->d_type == d_type) && (node->d.ptr == data))
                     return NULL;
                  ext->generic_data = eina_inlist_remove
                    (ext->generic_data, EINA_INLIST_GET(node));
                  _eo_generic_data_node_free(node);
                  break;
               }
          }
     }

   ext = _efl_object_extension_need(pd);
   if (ext)
     {
        node = calloc(1, sizeof(Eo_Generic_Data_Node));
        if (!node) return NULL;
        node->obj = obj;
        node->key = eina_stringshare_add(key);
        node->d.ptr = (void *) data;
        node->d_type = d_type;
        ext->generic_data = eina_inlist_prepend
          (ext->generic_data, EINA_INLIST_GET(node));
        return node;
     }

   return NULL;
}

static void *
_key_generic_get(const Eo *obj, Efl_Object_Data *pd, const char *key, Eo_Generic_Data_Node_Type d_type)
{
   Eo_Generic_Data_Node *node;
   Efl_Object_Extension *ext = pd->ext;

   if (!key) return NULL;
   if (!ext) return NULL;
   EINA_INLIST_FOREACH(ext->generic_data, node)
     {
        if (!strcmp(node->key, key))
          {
             if (node->d_type == d_type)
               {
                  ext->generic_data = eina_inlist_promote
                    (ext->generic_data, EINA_INLIST_GET(node));
                  return node->d.ptr;
               }
             else
               {
                  ERR("Object %p key '%s' asked for %d but is %d'",
                      obj, key, d_type, node->d_type);
                  return NULL;
               }
          }
     }
   return NULL;
}

static void
_key_generic_cb_del(void *data, const Efl_Event *event EINA_UNUSED)
{
   Eo_Generic_Data_Node *node = data;
   Efl_Object_Data *pd = efl_data_scope_get(node->obj, EFL_OBJECT_CLASS);
   _eo_key_generic_direct_del(pd, node, EINA_FALSE);
}

EOLIAN static void
_efl_object_key_data_set(Eo *obj, Efl_Object_Data *pd, const char *key, const void *data)
{
   _key_generic_set(obj, pd, key, data, DATA_PTR, EINA_TRUE);
}

EOLIAN static void *
_efl_object_key_data_get(Eo *obj, Efl_Object_Data *pd, const char *key)
{
   return _key_generic_get(obj, pd, key, DATA_PTR);
}

EOLIAN static void
_efl_object_key_ref_set(Eo *obj EINA_UNUSED, Efl_Object_Data *pd, const char *key, const Eo *objdata)
{
   Eo_Generic_Data_Node *node;

   if (!_eo_id_domain_compatible(obj, objdata)) return;
   node = _key_generic_set(obj, pd, key, objdata, DATA_OBJ, EINA_TRUE);
   if (node)
     {
        efl_ref(objdata);
        efl_event_callback_add((Eo *)objdata, EFL_EVENT_DEL, _key_generic_cb_del, node);
     }
}

EOLIAN static Eo *
_efl_object_key_ref_get(Eo *obj, Efl_Object_Data *pd, const char *key)
{
   return _key_generic_get(obj, pd, key, DATA_OBJ);
}

EOLIAN static void
_efl_object_key_wref_set(Eo *obj, Efl_Object_Data *pd, const char * key, const Efl_Object *objdata)
{
   Eo_Generic_Data_Node *node;

   if (!_eo_id_domain_compatible(obj, objdata)) return;
   node = _key_generic_set(obj, pd, key, objdata, DATA_OBJ_WEAK, EINA_TRUE);
   if (node)
     {
        efl_event_callback_add((Eo *)objdata, EFL_EVENT_DEL, _key_generic_cb_del, node);
     }
}

EOLIAN static Eo *
_efl_object_key_wref_get(Eo *obj, Efl_Object_Data *pd, const char * key)
{
   return _key_generic_get(obj, pd, key, DATA_OBJ_WEAK);
}

EOLIAN static void
_efl_object_key_value_set(Eo *obj EINA_UNUSED, Efl_Object_Data *pd, const char *key, Eina_Value *value)
{
   _key_generic_set(obj, pd, key, value, DATA_VAL, EINA_TRUE);
}

EOLIAN static Eina_Value *
_efl_object_key_value_get(Eo *obj, Efl_Object_Data *pd, const char *key)
{
   return _key_generic_get(obj, pd, key, DATA_VAL);
}

EOLIAN static void
_efl_object_name_set(Eo *obj EINA_UNUSED, Efl_Object_Data *pd, const char *name)
{
   if ((name) && (!name[0])) name = NULL;
   if (name)
     {
        _efl_object_extension_need(pd);
        if (pd->ext) eina_stringshare_replace(&(pd->ext->name), name);
     }
   else
     {
        if (!pd->ext) return;
        if (pd->ext->name)
          {
             eina_stringshare_replace(&(pd->ext->name), name);
             _efl_object_extension_noneed(pd);
          }
     }
}

EOLIAN static const char *
_efl_object_name_get(Eo *obj EINA_UNUSED, Efl_Object_Data *pd)
{
   if (!pd->ext) return NULL;
   return pd->ext->name;
}

static inline Eina_Bool
_name_match(const char *match, Eina_Bool is_glob, const char *str)
{
   if (str)
     {
        if (is_glob)
          {
             // if match string is empty - then it matches - same as "*"
             if (!match[0]) return EINA_TRUE;
             // if match string is "*" special case it and match
             if ((match[0] == '*') && (match[1] == 0)) return EINA_TRUE;
             // actual compare
             if (!fnmatch(match, str, 0)) return EINA_TRUE;
          }
        else
          {
             // if match string is empty - then it matches - same as "*"
             if (!match[0]) return EINA_TRUE;
             // if pointers are the same they must be the same
             if (match == str) return EINA_TRUE;
             // actual compare
             if (!strcmp(match, str)) return EINA_TRUE;
          }
     }
   return EINA_FALSE;
}

static inline Eina_Bool
_matchall(const char *match)
{
   if ((match[0] == 0) || ((match[0] == '*') && (match[1] == 0)))
     return EINA_TRUE;
   return EINA_FALSE;
}

static Eina_Bool
_hasglob(const char *match)
{
   if (strpbrk(match, "*?[")) return EINA_TRUE;
   return EINA_FALSE;
}

static Eina_Bool
_ismultiglob(const char *match)
{
   if ((match[0] == '*') && (match[1] == '*') && (match[2] == 0))
     return EINA_TRUE;
   if ((match[0] == '*') && (match[1] == '*') && (match[2] == '/'))
     return EINA_TRUE;
   if ((match[0] == '/') && (match[1] == '*') && (match[2] == '*') && (match[3] == 0))
     return EINA_TRUE;
   if ((match[0] == '/') && (match[1] == '*') && (match[2] == '*') && (match[3] == '/'))
     return EINA_TRUE;
   return EINA_FALSE;
}

EOLIAN static Efl_Object *
_efl_object_name_find(Eo *obj EINA_UNUSED, Efl_Object_Data *pd, const char *search)
{
   Eo *child;
   _Eo_Object *child_eo;
   const char *name, *p, *klass_name;

   // notes:
   // if search contains NO "/" char, then its just a name search.
   // if there is one or more "/" chars, then these are explicitly object
   // delimiters.
   // a name of "**" means 0 or more objects in the heirachy chain
   // if the string has no "/" char at the start, it implies "/**/"
   // a name can be a name or the form "class:name" where the object must
   // be of class named "class" and name "name". if "name" is empty like:
   // "class:" then an object of any name will match like "class:*". an
   // empty class like ":name" is the sanme as "*:name" which is the same
   // as "name". class ane name of course can be basic globs but not **

   // search string NULL or "" is invalid
   if (!search) return NULL;
   if (!search[0]) return NULL;

   if (strchr(search, '/'))
     {
        ERR("Looking up object by path '%s' is not supported", search);
        return NULL;
     }
   else
     {
        // if this is a multi glob - "**" then we don't have a name or
        // class to match at all so just don't look
        if (_ismultiglob(search)) return NULL;
        // check if this is "class:name" or just "name"
        if ((p = strchr(search, ':')))
          {
             // "class:name"
             char *klass;
             char *search_name;
             size_t colon_location = p - search;
             Eina_Bool klass_glob = EINA_FALSE;
             Eina_Bool name_glob = EINA_FALSE;

             // split class:name into 2 strings dropping :
             klass = alloca(strlen(search) + 1);
             strcpy(klass, search);
             klass[colon_location] = '\0';
             search_name = klass + colon_location + 1;

             // figure out if class or name are globs
             klass_glob = _hasglob(klass);
             name_glob = _hasglob(search_name);
             EINA_INLIST_FOREACH(pd->children, child_eo)
               {
                  child = _eo_obj_id_get(child_eo);
                  name = efl_name_get(child);
                  klass_name = efl_class_name_get(efl_class_get(child));
                  if (_name_match(klass, klass_glob, klass_name) &&
                      (((!_matchall(klass)) && (!name) && (_matchall(search_name))) ||
                       ((name) && _name_match(search_name, name_glob, name))))
                    return child;
                  child = efl_name_find(child, search);
                  if (child) return child;
               }
          }
        else
          {
             if (_hasglob(search))
               {
                  // we have a glob - fnmatch
                  EINA_INLIST_FOREACH(pd->children, child_eo)
                    {
                       child = _eo_obj_id_get(child_eo);
                       name = efl_name_get(child);
                       if ((name) && (_name_match(search, EINA_TRUE, name)))
                         return child;
                       child = efl_name_find(child, search);
                       if (child) return child;
                    }
               }
             else
               {
                  // fast path for simple "name"
                  EINA_INLIST_FOREACH(pd->children, child_eo)
                    {
                       child = _eo_obj_id_get(child_eo);
                       name = efl_name_get(child);
                       if ((name) && (_name_match(search, EINA_FALSE, name)))
                         return child;
                       child = efl_name_find(child, search);
                       if (child) return child;
                    }
               }
          }
     }
   return NULL;
}

EOLIAN static void
_efl_object_comment_set(Eo *obj EINA_UNUSED, Efl_Object_Data *pd, const char *comment)
{
   if ((comment) && (!comment[0])) comment = NULL;
   if (comment)
     {
        _efl_object_extension_need(pd);
        if (pd->ext) eina_stringshare_replace(&(pd->ext->comment), comment);
     }
   else
     {
        if (!pd->ext) return;
        if (pd->ext->comment)
          {
             eina_stringshare_replace(&(pd->ext->comment), comment);
             _efl_object_extension_noneed(pd);
          }
     }
}

EOLIAN static const char *
_efl_object_comment_get(Eo *obj EINA_UNUSED, Efl_Object_Data *pd)
{
   if (!pd->ext) return NULL;
   return pd->ext->comment;
}

EOLIAN static void
_efl_object_del(const Eo *obj, Efl_Object_Data *pd EINA_UNUSED)
{
   if (efl_parent_get((Eo *) obj))
     {
        efl_parent_set((Eo *) obj, NULL);
     }
   else
     {
        efl_unref(obj);
     }
}

void
_efl_object_parent_sink_set(Eo *obj, Eina_Bool sink)
{
   Efl_Object_Data *pd = efl_data_scope_get(obj, EFL_OBJECT_CLASS);
   pd->parent_sunk = sink;
}

EOLIAN static void
_efl_object_parent_set(Eo *obj, Efl_Object_Data *pd, Eo *parent_id)
{
   Eo *prev_parent = pd->parent;
   if ((pd->parent == parent_id) ||
       ((parent_id) && (!_eo_id_domain_compatible(parent_id, obj))))
     return;

   EO_OBJ_POINTER(obj, eo_obj);
   if (pd->parent)
     {
        Efl_Object_Data *old_parent_pd = efl_data_scope_get(pd->parent,
                                                            EFL_OBJECT_CLASS);

        if (old_parent_pd)
          old_parent_pd->children = eina_inlist_remove(old_parent_pd->children,
                                                       EINA_INLIST_GET(eo_obj));
        // this error is highly unlikely so move it out of the normal
        // instruction path to avoid l1 cache pollution
        else goto err_impossible;
     }

   /* Set new parent */
   if (parent_id)
     {
        Efl_Object_Data *parent_pd = efl_data_scope_get(parent_id,
                                                        EFL_OBJECT_CLASS);

        if (EINA_LIKELY(parent_pd != NULL))
          {
             pd->parent = parent_id;
             parent_pd->children = eina_inlist_append(parent_pd->children,
                                                      EINA_INLIST_GET(eo_obj));
             if (!prev_parent && pd->parent_sunk) efl_ref(obj);
             pd->parent_sunk = EINA_TRUE;
          }
        else
          {
             pd->parent = NULL;
             if (prev_parent) efl_unref(obj);
             // unlikely this error happens, so move it out of execution path
             // to improve l1 cache efficiency
             goto err_parent;
          }
     }
   else
     {
        pd->parent = NULL;
        if (prev_parent && !eo_obj->del_triggered) efl_unref(obj);
     }

   EO_OBJ_DONE(obj);
   return;

err_parent:
   ERR("New parent %p for object %p is not a valid Eo object.",
       parent_id, obj);
   EO_OBJ_DONE(obj);
   return;

err_impossible:
   ERR("CONTACT DEVS!!! SHOULD NEVER HAPPEN!!! Old parent %p for object %p is not a valid Eo object.",
       pd->parent, obj);
   EO_OBJ_DONE(obj);
}

EOLIAN static Eo *
_efl_object_parent_get(Eo *obj EINA_UNUSED, Efl_Object_Data *pd)
{
   return pd->parent;
}

EOLIAN static Eina_Bool
_efl_object_finalized_get(Eo *obj_id, Efl_Object_Data *pd EINA_UNUSED)
{
   Eina_Bool finalized;
   EO_OBJ_POINTER_RETURN_VAL(obj_id, obj, EINA_FALSE);
   finalized = obj->finalized;
   EO_OBJ_DONE(obj_id);
   return finalized;
}

EOLIAN static Efl_Object *
_efl_object_provider_find(Eo *obj EINA_UNUSED, Efl_Object_Data *pd, const Efl_Object *klass)
{
   if (pd->parent) return efl_provider_find(pd->parent, klass);
   return NULL;
}


/* Children accessor */
typedef struct _Eo_Children_Iterator Eo_Children_Iterator;
struct _Eo_Children_Iterator
{
   Eina_Iterator iterator;
   Eina_Inlist *current;
   _Eo_Object *obj;
   Eo *obj_id;
};

static Eina_Bool
_efl_children_iterator_next(Eo_Children_Iterator *it, void **data)
{
   if (!it->current) return EINA_FALSE;

   if (data)
     {
        _Eo_Object *eo_obj = EINA_INLIST_CONTAINER_GET(it->current, _Eo_Object);
        *data = _eo_obj_id_get(eo_obj);
     }
   it->current = it->current->next;

   return EINA_TRUE;
}

static Eo *
_efl_children_iterator_container(Eo_Children_Iterator *it)
{
   return it->obj_id;
}

static void
_efl_children_iterator_free(Eo_Children_Iterator *it)
{
   _Efl_Class *klass;
   _Eo_Object *obj;

   klass = (_Efl_Class*) it->obj->klass;
   obj = it->obj;

   eina_spinlock_take(&klass->iterators.trash_lock);
   if (klass->iterators.trash_count < 8)
     {
        klass->iterators.trash_count++;
        eina_trash_push(&klass->iterators.trash, it);
     }
   else
     {
        eina_freeq_ptr_main_add(it, free, sizeof(*it));
     }
   eina_spinlock_release(&klass->iterators.trash_lock);

   _efl_unref(obj);
}

EOLIAN static Eina_Iterator *
_efl_object_children_iterator_new(Eo *obj_id, Efl_Object_Data *pd)
{
   _Efl_Class *klass;
   Eo_Children_Iterator *it = NULL;

   EO_OBJ_POINTER_RETURN_VAL(obj_id, obj, NULL);

   if (pd->children)
     {
        klass = (_Efl_Class *)obj->klass;

        eina_spinlock_take(&klass->iterators.trash_lock);
        it = eina_trash_pop(&klass->iterators.trash);
        if (it)
          {
             klass->iterators.trash_count--;
             memset(it, 0, sizeof (Eo_Children_Iterator));
          }
        else it = calloc(1, sizeof (Eo_Children_Iterator));
        eina_spinlock_release(&klass->iterators.trash_lock);
        // very unlikely to not allocate the iterator to move this error
        // handling out of l1 instruction cache
        if (!it) goto done;

        EINA_MAGIC_SET(&it->iterator, EINA_MAGIC_ITERATOR);
        it->current = pd->children;
        it->obj = _efl_ref(obj);
        it->obj_id = obj_id;
        it->iterator.next = FUNC_ITERATOR_NEXT(_efl_children_iterator_next);
        it->iterator.get_container = FUNC_ITERATOR_GET_CONTAINER(_efl_children_iterator_container);
        it->iterator.free = FUNC_ITERATOR_FREE(_efl_children_iterator_free);
     }
done:
   EO_OBJ_DONE(obj_id);
   return (Eina_Iterator *)it;
}

EOLIAN static void
_efl_object_dbg_info_get(Eo *obj EINA_UNUSED, Efl_Object_Data *pd EINA_UNUSED, Efl_Dbg_Info *root_node EINA_UNUSED)
{  /* No info required in the meantime */
   return;
}

/* Weak reference. */

static inline size_t
_wref_count(Efl_Object_Data *pd)
{
   Eo ***itr;
   size_t count = 0;
   Efl_Object_Extension *ext = pd->ext;

   if ((!ext) || (!ext->wrefs)) return 0;
   for (itr = ext->wrefs; *itr; itr++) count++;

   return count;
}

EOLIAN static void
_efl_object_wref_add(Eo *obj, Efl_Object_Data *pd, Eo **wref)
{
   size_t count;
   Eo ***tmp;
   Efl_Object_Extension *ext;

   count = _wref_count(pd);
   count += 1; /* New wref. */

   ext = _efl_object_extension_need(pd);
   if (ext)
     {
        tmp = realloc(ext->wrefs, sizeof(*ext->wrefs) * (count + 1));
        if (tmp)
          {
             ext->wrefs = tmp;
             ext->wrefs[count - 1] = wref;
             ext->wrefs[count] = NULL;
             *wref = obj;
          }
     }
}

EOLIAN static void
_efl_object_wref_del(Eo *obj, Efl_Object_Data *pd, Eo **wref)
{
   size_t count;
   Efl_Object_Extension *ext = pd->ext;

   // very unlikely so improve l1 instr cache by using goto
   if (*wref != obj) goto err_wref_not_obj;

   // very unlikely so improve l1 instr cache by using goto
   if ((!ext) || (!ext->wrefs)) goto err_wref_none;

   /* Move the last item in the array instead of the current wref. */
   count = _wref_count(pd);

     {
        Eo ***itr;
        for (itr = ext->wrefs; *itr; itr++)
          {
             if (*itr == wref)
               {
                  *itr = ext->wrefs[count - 1];
                  break;
               }
          }
        // very unlikely so improve l1 instr cache by using goto
        if (!*itr) goto err_noiter;
     }

   if (count > 1)
     {
        Eo ***tmp;
        // No count--; because of the NULL that is not included in the count
        tmp = realloc(ext->wrefs, sizeof(*ext->wrefs) * count);
        if (!tmp) return;
        ext->wrefs = tmp;
        ext->wrefs[count - 1] = NULL;
        *wref = NULL;
        return;
     }
   else
     {
        eina_freeq_ptr_main_add(ext->wrefs, free, 0);
        ext->wrefs = NULL;
        _efl_object_extension_noneed(pd);
     }

   *wref = NULL;
   return;

err_noiter:
   ERR("Wref %p is not associated with object %p", wref, obj);
   goto err_null;
err_wref_none:
   ERR("There are no weak refs for object %p", obj);
err_null:
   *wref = NULL;
   return;
err_wref_not_obj:
   ERR("Wref is a weak ref to %p, while this function was called on %p.",
       *wref, obj);
   return;
}

static inline void
_wref_destruct(Efl_Object_Data *pd)
{
   Eo ***itr;
   Efl_Object_Extension *ext = pd->ext;

   if ((!ext) || (!ext->wrefs)) return;
   for (itr = ext->wrefs; *itr; itr++) **itr = NULL;
   eina_freeq_ptr_main_add(ext->wrefs, free, 0);
   ext->wrefs = NULL;
}

/* EOF Weak reference. */

/* Event callbacks */

/* Callbacks */

/* XXX: Legacy support, remove when legacy is dead. */
static Eina_Hash *_legacy_events_hash = NULL;

EAPI const Efl_Event_Description *
efl_object_legacy_only_event_description_get(const char *_event_name)
{
   Eina_Stringshare *event_name = eina_stringshare_add(_event_name);
   Efl_Event_Description *event_desc = eina_hash_find(_legacy_events_hash, event_name);
   if (!event_desc)
     {
        event_desc = calloc(1, sizeof(Efl_Event_Description));
        event_desc->name = event_name;
        event_desc->legacy_is = EINA_TRUE;
        eina_hash_add(_legacy_events_hash, event_name, event_desc);
     }
   else
     {
        eina_stringshare_del(event_name);
     }

   return event_desc;
}

static inline Eina_Bool
_legacy_event_desc_is(const Efl_Event_Description *desc)
{
   return desc->legacy_is;
}

static void
_legacy_events_hash_free_cb(void *_desc)
{
   Efl_Event_Description *desc = _desc;
   eina_stringshare_del(desc->name);
   eina_freeq_ptr_main_add(desc, free, sizeof(*desc));
}

/* EOF Legacy */

struct _Eo_Callback_Description
{
   union
     {
        Efl_Callback_Array_Item item;
        const Efl_Callback_Array_Item *item_array;
     } items;

   void *func_data;
   Efl_Callback_Priority priority;

   Eina_Bool delete_me : 1;
   Eina_Bool func_array : 1;
};

static int _eo_callbacks                  = 0;
static Eina_Mempool *_eo_callback_mempool = NULL;

static void
_eo_callback_free(Eo_Callback_Description *cb)
{
   if (!cb) return;
   eina_mempool_free(_eo_callback_mempool, cb);
   _eo_callbacks--;
   if (_eo_callbacks == 0)
     {
        eina_mempool_del(_eo_callback_mempool);
        _eo_callback_mempool = NULL;
     }
}

static Eo_Callback_Description *
_eo_callback_new(void)
{
   Eo_Callback_Description *cb;
   // very unlikely  that the mempool isnt initted, so take all the init code
   // and move it out of l1 instruction cache space so we dont pollute the
   // l1 cache with unused code 99% of the time
   if (!_eo_callback_mempool) goto init_mempool;
init_mempool_back:

   cb = eina_mempool_calloc(_eo_callback_mempool,
                            sizeof(Eo_Callback_Description));
   if (cb)
     {
        _eo_callbacks++;
        return cb;
     }
   if (_eo_callbacks != 0) return NULL;
   eina_mempool_del(_eo_callback_mempool);
   _eo_callback_mempool = NULL;
   return NULL;
init_mempool:
   _eo_callback_mempool = eina_mempool_add
   ("chained_mempool", NULL, NULL, sizeof(Eo_Callback_Description), 256);
   if (!_eo_callback_mempool) return NULL;
   goto init_mempool_back;
}

#ifdef EFL_EVENT_SPECIAL_SKIP

#define CB_COUNT_INC(cnt) do { if ((cnt) != 0xffff) (cnt)++; } while(0)
#define CB_COUNT_DEC(cnt) do { if ((cnt) != 0xffff) (cnt)--; } while(0)

static inline void
_special_event_count_inc(Efl_Object_Data *pd, const Efl_Callback_Array_Item *it)
{
   if      (it->desc == EFL_EVENT_CALLBACK_ADD)
     CB_COUNT_INC(pd->event_cb_efl_event_callback_add_count);
   else if (it->desc == EFL_EVENT_CALLBACK_DEL)
     CB_COUNT_INC(pd->event_cb_efl_event_callback_del_count);
   else if (it->desc == EFL_EVENT_DEL)
     CB_COUNT_INC(pd->event_cb_efl_event_del_count);
}

static inline void
_special_event_count_dec(Efl_Object_Data *pd, const Efl_Callback_Array_Item *it)
{
   if      (it->desc == EFL_EVENT_CALLBACK_ADD)
     CB_COUNT_DEC(pd->event_cb_efl_event_callback_add_count);
   else if (it->desc == EFL_EVENT_CALLBACK_DEL)
     CB_COUNT_DEC(pd->event_cb_efl_event_callback_del_count);
   else if (it->desc == EFL_EVENT_DEL)
     CB_COUNT_DEC(pd->event_cb_efl_event_del_count);
}
#endif

/* Actually remove, doesn't care about walking list, or delete_me */
static void
_eo_callback_remove(Efl_Object_Data *pd, Eo_Callback_Description **cb)
{
   unsigned int length;
#ifdef EFL_EVENT_SPECIAL_SKIP
   const Efl_Callback_Array_Item *it;

   if ((*cb)->func_array)
     {
        for (it = (*cb)->items.item_array; it->func; it++)
          _special_event_count_dec(pd, it);
     }
   else _special_event_count_dec(pd, &((*cb)->items.item));
#endif

   _eo_callback_free(*cb);

   length = pd->callbacks_count - (cb - pd->callbacks);
   if (length > 1) memmove(cb, cb + 1, (length - 1) * sizeof (Eo_Callback_Description*));
   pd->callbacks_count--;
}

/* Actually remove, doesn't care about walking list, or delete_me */
static void
_eo_callback_remove_all(Efl_Object_Data *pd)
{
   unsigned int i;

   for (i = 0; i < pd->callbacks_count; i++)
     _eo_callback_free(pd->callbacks[i]);

   eina_freeq_ptr_main_add(pd->callbacks, free, 0);
   pd->callbacks = NULL;
   pd->callbacks_count = 0;
#ifdef EFL_EVENT_SPECIAL_SKIP
   pd->event_cb_efl_event_callback_add_count = 0;
   pd->event_cb_efl_event_callback_del_count = 0;
   pd->event_cb_efl_event_del_count = 0;
#endif
}

static void
_eo_callbacks_clear(Efl_Object_Data *pd)
{
   unsigned int i = 0;

   /* If there are no deletions waiting. */
   if (!pd->deletions_waiting) return;

   /* Abort if we are currently walking the list. */
   if (pd->walking_list > 0) return;

   pd->deletions_waiting = EINA_FALSE;

   while (i < pd->callbacks_count)
     {
        Eo_Callback_Description **itr;

        itr = pd->callbacks + i;

        if ((*itr)->delete_me) _eo_callback_remove(pd, itr);
        else i++;
     }
}

static inline unsigned int
_eo_callback_search_sorted_near(const Efl_Object_Data *pd, const Eo_Callback_Description *look)
{
   unsigned int start, last, middle;
   const Eo_Callback_Description *p;
   int cmp;

   if (pd->callbacks_count == 0) return 0;
   else if (pd->callbacks_count == 1) return 0;

   start = 0;
   last = pd->callbacks_count - 1;
   do
     {
        middle = start + (last - start) / 2;
        p = pd->callbacks[middle];

        cmp = p->priority - look->priority;
        if (cmp == 0) return middle;
        else if (cmp > 0) start = middle + 1;
        else if (middle > 0) last = middle - 1;
        else break;
     }
   while (start <= last);
   return middle;
}

static void
_eo_callbacks_sorted_insert(Efl_Object_Data *pd, Eo_Callback_Description *cb)
{
   Eo_Callback_Description **itr;
   unsigned int length, j;

   // Do a dichotomic searh
   j = _eo_callback_search_sorted_near(pd, cb);

   // Adjust for both case of length == 0 and when priority is equal.
   while (j < pd->callbacks_count &&
          pd->callbacks[j]->priority >= cb->priority)
     j++;


   // Increase the callbacks storage by 16 entries at a time
   if ((pd->callbacks_count & 0xF) == 0x0)
     {
        Eo_Callback_Description **tmp;
        unsigned int new_len = (pd->callbacks_count | 0xF) + 1;

        tmp = realloc(pd->callbacks, new_len * sizeof (Eo_Callback_Description*));
        if (!tmp) return ;
        pd->callbacks = tmp;
     }

   // FIXME: Potential improvement, merge single callback description of the same priority
   // into an array when possible

   itr = pd->callbacks + j;
   length = pd->callbacks_count - j;
   if (length > 0) memmove(itr + 1, itr, length * sizeof (Eo_Callback_Description*));

   *itr = cb;

   pd->callbacks_count++;
}

EOLIAN static Eina_Bool
_efl_object_event_callback_priority_add(Eo *obj, Efl_Object_Data *pd,
                                        const Efl_Event_Description *desc,
                                        Efl_Callback_Priority priority,
                                        Efl_Event_Cb func,
                                        const void *user_data)
{
   const Efl_Callback_Array_Item arr[] = { {desc, func}, {NULL, NULL}};
   Eo_Callback_Description *cb = _eo_callback_new();

   // very unlikely so improve l1 instr cache by using goto
   if (!cb || !desc || !func) goto err;
   cb->items.item.desc = desc;
   cb->items.item.func = func;
   cb->func_data = (void *) user_data;
   cb->priority = priority;
   _eo_callbacks_sorted_insert(pd, cb);
#ifdef EFL_EVENT_SPECIAL_SKIP
   _special_event_count_inc(pd, &(cb->items.item));
#endif

   efl_event_callback_call(obj, EFL_EVENT_CALLBACK_ADD, (void *)arr);

   return EINA_TRUE;

err:
   ERR("Tried adding callback with invalid values: cb: %p desc: %p func: %p\n", cb, desc, func);
   _eo_callback_free(cb);
   return EINA_FALSE;
}

static void
_efl_object_event_callback_clean(Eo *obj, Efl_Object_Data *pd,
                                 const Efl_Callback_Array_Item *array,
                                 Eo_Callback_Description **cb)
{
   (*cb)->delete_me = EINA_TRUE;
   if (pd->walking_list > 0)
     pd->deletions_waiting = EINA_TRUE;
   else
     _eo_callback_remove(pd, cb);

   efl_event_callback_call(obj, EFL_EVENT_CALLBACK_DEL, (void *)array);
}

EOLIAN static Eina_Bool
_efl_object_event_callback_del(Eo *obj, Efl_Object_Data *pd,
                    const Efl_Event_Description *desc,
                    Efl_Event_Cb func,
                    const void *user_data)
{
   Eo_Callback_Description **cb;
   unsigned int i;

   for (cb = pd->callbacks, i = 0;
        i < pd->callbacks_count;
        cb++, i++)
     {
        if (!(*cb)->delete_me &&
            ((*cb)->items.item.desc == desc) &&
            ((*cb)->items.item.func == func) &&
            ((*cb)->func_data == user_data))
          {
             const Efl_Callback_Array_Item arr[] = { {desc, func}, {NULL, NULL}};

             _efl_object_event_callback_clean(obj, pd, arr, cb);
             return EINA_TRUE;
          }
     }

   DBG("Callback of object %p with function %p and data %p not found.", obj, func, user_data);
   return EINA_FALSE;
}

EOLIAN static Eina_Bool
_efl_object_event_callback_array_priority_add(Eo *obj, Efl_Object_Data *pd,
                                              const Efl_Callback_Array_Item *array,
                                              Efl_Callback_Priority priority,
                                              const void *user_data)
{
   Eo_Callback_Description *cb = _eo_callback_new();
#if  defined(EFL_EVENT_SPECIAL_SKIP) ||  defined(EO_DEBUG)
   const Efl_Callback_Array_Item *it;
#endif
#ifdef EO_DEBUG
   const Efl_Callback_Array_Item *prev;
#endif

   // very unlikely so improve l1 instr cache by using goto
   if (!cb || !array) goto err;
#ifdef EO_DEBUG
   prev = array;
   for (it = prev + 1; prev->func && it->func; it++, prev++)
     {
        if (efl_callbacks_cmp(prev, it) > 0)
          {
             ERR("Trying to insert a non sorted array callbacks (%p).", array);
             _eo_callback_free(cb);
             return EINA_FALSE;
          }
     }
#endif

   cb->func_data = (void *) user_data;
   cb->priority = priority;
   cb->items.item_array = array;
   cb->func_array = EINA_TRUE;
   _eo_callbacks_sorted_insert(pd, cb);
#ifdef EFL_EVENT_SPECIAL_SKIP
   for (it = cb->items.item_array; it->func; it++)
     _special_event_count_inc(pd, it);
#endif

   efl_event_callback_call(obj, EFL_EVENT_CALLBACK_ADD, (void *)array);

   return EINA_TRUE;

err:
   ERR("Tried adding array of callbacks with invalid values: cb: %p array: %p.", cb, array);
   _eo_callback_free(cb);
   return EINA_FALSE;
}

EOLIAN static Eina_Bool
_efl_object_event_callback_array_del(Eo *obj, Efl_Object_Data *pd,
                                     const Efl_Callback_Array_Item *array,
                                     const void *user_data)
{
   Eo_Callback_Description **cb;
   unsigned int i;

   for (cb = pd->callbacks, i = 0;
        i < pd->callbacks_count;
        cb++, i++)
     {
        if (!(*cb)->delete_me &&
            ((*cb)->items.item_array == array) &&
            ((*cb)->func_data == user_data))
          {
             _efl_object_event_callback_clean(obj, pd, array, cb);
             return EINA_TRUE;
          }
     }

   DBG("Callback of object %p with function array %p and data %p not found.", obj, array, user_data);
   return EINA_FALSE;
}

static Eina_Bool
_cb_desc_match(const Efl_Event_Description *a, const Efl_Event_Description *b, Eina_Bool legacy_compare)
{
   /* If one is legacy and the other is not, strcmp. Else, pointer compare. */
   if (!EINA_UNLIKELY(legacy_compare && (_legacy_event_desc_is(a) != _legacy_event_desc_is(b))))
     return (a == b);
   return !strcmp(a->name, b->name);
}

static inline Eina_Bool
_event_callback_call(Eo *obj_id, Efl_Object_Data *pd,
                     const Efl_Event_Description *desc,
                     void *event_info,
                     Eina_Bool legacy_compare)
{
   Eo_Callback_Description **cb;
   Eo_Current_Callback_Description *lookup, saved;
   Efl_Event ev;
   unsigned int idx;
   Eina_Bool callback_already_stopped, ret;

   if (pd->callbacks_count == 0) return EINA_FALSE;
#ifdef EFL_EVENT_SPECIAL_SKIP
   else if ((desc == EFL_EVENT_CALLBACK_ADD) &&
            (pd->event_cb_efl_event_callback_add_count == 0)) return EINA_FALSE;
   else if ((desc == EFL_EVENT_CALLBACK_DEL) &&
            (pd->event_cb_efl_event_callback_del_count == 0)) return EINA_FALSE;
   else if ((desc == EFL_EVENT_DEL) &&
            (pd->event_cb_efl_event_del_count == 0)) return EINA_FALSE;
#endif

   lookup = NULL;
   callback_already_stopped = pd->callback_stopped;
   ret = EINA_TRUE;

   ev.object = obj_id;
   ev.desc = desc;
   ev.info = event_info;

   pd->walking_list++;

   // Handle event that require to restart where we were in the nested list walking
   // relatively unlikely so improve l1 instr cache by using goto
   if (desc->restart) goto restart;
   else idx = pd->callbacks_count;
restart_back:

   for (; idx > 0; idx--)
     {
        cb = pd->callbacks + idx - 1;
        if (!(*cb)->delete_me)
          {
             if ((*cb)->func_array)
               {
                  const Efl_Callback_Array_Item *it;

                  for (it = (*cb)->items.item_array; it->func; it++)
                    {
                       // Array callbacks are sorted, break if we are getting to high.
                       if (!legacy_compare &&
                           ((const unsigned char *) desc - (const unsigned char *) it->desc) < 0)
                         break;
                       if (!_cb_desc_match(it->desc, desc, legacy_compare))
                          continue;
                       if (!it->desc->unfreezable &&
                           (event_freeze_count || pd->event_freeze_count))
                          continue;

                       // Handle nested restart of walking list
                       if (lookup) lookup->current = idx - 1;
                       it->func((void *) (*cb)->func_data, &ev);
                       /* Abort callback calling if the func says so. */
                       if (pd->callback_stopped)
                         {
                            ret = EINA_FALSE;
                            goto end;
                         }
                       // We have actually walked this list during a nested call
                       if (lookup &&
                           lookup->current == 0)
                         goto end;
                    }
               }
             else
               {
                  if (!_cb_desc_match((*cb)->items.item.desc, desc, legacy_compare))
                    continue;
                  if (!(*cb)->items.item.desc->unfreezable &&
                      (event_freeze_count || pd->event_freeze_count))
                    continue;

                  // Handle nested restart of walking list
                  if (lookup) lookup->current = idx - 1;
                  (*cb)->items.item.func((void *) (*cb)->func_data, &ev);
                  /* Abort callback calling if the func says so. */
                  if (pd->callback_stopped)
                    {
                       ret = EINA_FALSE;
                       goto end;
                    }
                  // We have actually walked this list during a nested call
                  if (lookup &&
                      lookup->current == 0)
                    goto end;
               }
          }
     }

end:
   // Handling restarting list walking complete exit.
   if (lookup) lookup->current = 0;
   if (lookup == &saved)
     {
        pd->current = eina_inlist_remove(pd->current, EINA_INLIST_GET(lookup));
     }

   pd->walking_list--;
   _eo_callbacks_clear(pd);

   pd->callback_stopped = callback_already_stopped;

   return ret;
restart:
   EINA_INLIST_FOREACH(pd->current, lookup)
     {
        if (lookup->desc == desc) break;
     }

   // This is the first event to trigger it, so register it here
   if (!lookup)
     {
        // This following trick get us a zero allocation list
        saved.desc = desc;
        saved.current = 0;
        lookup = &saved;
        // Ideally there will most of the time be only one item in this list
        // But just to speed up things, prepend so we find it fast at the end
        // of this function
        pd->current = eina_inlist_prepend(pd->current, EINA_INLIST_GET(lookup));
     }

   if (!lookup->current) lookup->current = pd->callbacks_count;
   idx = lookup->current;
   goto restart_back;
}

EOLIAN static Eina_Bool
_efl_object_event_callback_call(Eo *obj_id, Efl_Object_Data *pd,
            const Efl_Event_Description *desc,
            void *event_info)
{
   return _event_callback_call(obj_id, pd, desc, event_info, EINA_FALSE);
}

EOLIAN static Eina_Bool
_efl_object_event_callback_legacy_call(Eo *obj_id, Efl_Object_Data *pd,
            const Efl_Event_Description *desc,
            void *event_info)
{
   return _event_callback_call(obj_id, pd, desc, event_info, EINA_TRUE);
}

EOLIAN static void
_efl_object_event_callback_stop(Eo *obj EINA_UNUSED, Efl_Object_Data *pd)
{
   pd->callback_stopped = EINA_TRUE;
}

static void
_efl_event_forwarder_callback(void *data, const Efl_Event *event)
{
   Eo *new_obj = (Eo *) data;
   Eina_Bool ret = EINA_FALSE;

   ret = efl_event_callback_call(new_obj, event->desc, event->info);

   if (!ret)
     {
        efl_event_callback_stop(event->object);
     }
}

/* FIXME: Change default priority? Maybe call later? */
EOLIAN static void
_efl_object_event_callback_forwarder_add(Eo *obj, Efl_Object_Data *pd EINA_UNUSED,
                     const Efl_Event_Description *desc,
                     Eo *new_obj)
{

   /* FIXME: Add it EO_MAGIC_RETURN(new_obj, EO_EINA_MAGIC); */

   efl_event_callback_add(obj, desc, _efl_event_forwarder_callback, new_obj);
}

EOLIAN static void
_efl_object_event_callback_forwarder_del(Eo *obj, Efl_Object_Data *pd EINA_UNUSED,
                     const Efl_Event_Description *desc,
                     Eo *new_obj)
{

   /* FIXME: Add it EO_MAGIC_RETURN(new_obj, EO_EINA_MAGIC); */

   efl_event_callback_del(obj, desc, _efl_event_forwarder_callback, new_obj);
}

EOLIAN static void
_efl_object_event_freeze(Eo *obj EINA_UNUSED, Efl_Object_Data *pd)
{
   pd->event_freeze_count++;
}

EOLIAN static void
_efl_object_event_thaw(Eo *obj, Efl_Object_Data *pd)
{
   if (pd->event_freeze_count > 0)
     {
        pd->event_freeze_count--;
     }
   else
     {
        ERR("Events for object %p have already been thawed.", obj);
     }
}

EOLIAN static int
_efl_object_event_freeze_count_get(Eo *obj EINA_UNUSED, Efl_Object_Data *pd)
{
   return pd->event_freeze_count;
}

EOLIAN static void
_efl_object_event_global_freeze(Eo *klass EINA_UNUSED, void *pd EINA_UNUSED)
{
   event_freeze_count++;
}

EOLIAN static void
_efl_object_event_global_thaw(Eo *klass EINA_UNUSED, void *pd EINA_UNUSED)
{
   if (event_freeze_count > 0)
     {
        event_freeze_count--;
     }
   else
     {
        ERR("Global events have already been thawed.");
     }
}

EOLIAN static int
_efl_object_event_global_freeze_count_get(Eo *klass EINA_UNUSED, void *pd EINA_UNUSED)
{
   return event_freeze_count;
}

EOLIAN static Eina_Bool
_efl_object_composite_attach(Eo *parent_id, Efl_Object_Data *pd EINA_UNUSED, Eo *comp_obj_id)
{
   Eo *emb_obj_id = NULL;

   EO_OBJ_POINTER_RETURN_VAL(comp_obj_id, comp_obj, EINA_FALSE);
   EO_OBJ_POINTER_GOTO(parent_id, parent, err_parent);

   /* FIXME: composite should fail if domains are different */

   /* Don't composite if we already have a composite object of this type */
     {
        Eina_List *itr;
        EINA_LIST_FOREACH(parent->composite_objects, itr, emb_obj_id)
          {
             EO_OBJ_POINTER_GOTO(emb_obj_id, emb_obj, err_klass);
             if (EINA_UNLIKELY(emb_obj->klass == comp_obj->klass)) goto err_klass;
          }
        emb_obj_id = NULL;
     }

   Efl_Object_Data *comp_pd = efl_data_scope_get(comp_obj_id, EFL_OBJECT_CLASS);

   if (efl_composite_part_is(comp_obj_id))
     efl_composite_detach(comp_pd->ext->composite_parent, comp_obj_id);

   /* Set the parent comp on the child. */
   _efl_object_extension_need(comp_pd);
   comp_pd->ext->composite_parent = parent_id;

   parent->composite_objects = eina_list_prepend(parent->composite_objects, comp_obj_id);

   if (emb_obj_id) EO_OBJ_DONE(emb_obj_id);
   EO_OBJ_DONE(parent_id);
   EO_OBJ_DONE(comp_obj_id);
   return EINA_TRUE;

err_klass:
   if (emb_obj_id) EO_OBJ_DONE(emb_obj_id);
   EO_OBJ_DONE(parent_id);
err_parent:
   EO_OBJ_DONE(comp_obj_id);
   return EINA_FALSE;
}

EOLIAN static Eina_Bool
_efl_object_composite_detach(Eo *parent_id, Efl_Object_Data *pd EINA_UNUSED, Eo *comp_obj_id)
{
   EO_OBJ_POINTER_RETURN_VAL(comp_obj_id, comp_obj, EINA_FALSE);
   EO_OBJ_POINTER_GOTO(parent_id, parent, err_parent);

   // unlikely so improve l1 instr cache by using goto
   if (!efl_composite_part_is(comp_obj_id)) goto err_part;

   parent->composite_objects = eina_list_remove(parent->composite_objects, comp_obj_id);
   /* Clear the comp parent on the child. */
     {
        Efl_Object_Data *comp_pd = efl_data_scope_get(comp_obj_id, EFL_OBJECT_CLASS);
        comp_pd->ext->composite_parent = NULL;
        _efl_object_extension_noneed(comp_pd);
     }

   EO_OBJ_DONE(parent_id);
   EO_OBJ_DONE(comp_obj_id);
   return EINA_TRUE;

err_part:
   EO_OBJ_DONE(parent_id);
err_parent:
   EO_OBJ_DONE(comp_obj_id);
   return EINA_FALSE;
}

EOLIAN static Eina_Bool
_efl_object_composite_part_is(Eo *comp_obj_id EINA_UNUSED, Efl_Object_Data *pd)
{
   return pd->ext && pd->ext->composite_parent;
}

/* Eo_Dbg */
EAPI void
efl_dbg_info_free(Efl_Dbg_Info *info)
{
   eina_value_flush(&(info->value));
   eina_freeq_ptr_main_add(info, free, sizeof(*info));
}

static Eina_Bool
_eo_dbg_info_setup(const Eina_Value_Type *type, void *mem)
{
   memset(mem, 0, type->value_size);
   return EINA_TRUE;
}

static Eina_Bool
_eo_dbg_info_flush(const Eina_Value_Type *type EINA_UNUSED, void *_mem)
{
   Efl_Dbg_Info *mem = *(Efl_Dbg_Info **) _mem;
   eina_stringshare_del(mem->name);
   eina_value_flush(&(mem->value));
   eina_freeq_ptr_main_add(mem, free, sizeof(*mem));
   return EINA_TRUE;
}

static Eina_Bool
_eo_dbg_info_copy(const Eina_Value_Type *type EINA_UNUSED, const void *_src, void *_dst)
{
   const Efl_Dbg_Info **src = (const Efl_Dbg_Info **) _src;
   Efl_Dbg_Info **dst = _dst;

   *dst = calloc(1, sizeof(Efl_Dbg_Info));
   if (!*dst) return EINA_FALSE;
   (*dst)->name = eina_stringshare_ref((*src)->name);
   eina_value_copy(&((*src)->value), &((*dst)->value));
   return EINA_TRUE;
}

static Eina_Bool
_eo_dbg_info_convert_to(const Eina_Value_Type *type EINA_UNUSED, const Eina_Value_Type *convert, const void *type_mem, void *convert_mem)
{
   /* FIXME: For the meanwhile, just use the inner type for the value. */
   const Efl_Dbg_Info **src = (const Efl_Dbg_Info **) type_mem;
   if (convert == EINA_VALUE_TYPE_STRINGSHARE ||
       convert == EINA_VALUE_TYPE_STRING)
     {
        Eina_Bool ret;
        const char *other_mem;
        char *inner_val = eina_value_to_string(&(*src)->value);
        other_mem = inner_val;
        ret = eina_value_type_pset(convert, convert_mem, &other_mem);
        eina_freeq_ptr_main_add(inner_val, free, 0);
        return ret;
     }

   eina_error_set(EINA_ERROR_VALUE_FAILED);
   return EINA_FALSE;
}

static Eina_Bool
_eo_dbg_info_pset(const Eina_Value_Type *type EINA_UNUSED, void *_mem, const void *_ptr)
{
   Efl_Dbg_Info **mem = _mem;
   if (*mem) free(*mem);
   *mem = (void *) _ptr;
   return EINA_TRUE;
}

static Eina_Bool
_eo_dbg_info_pget(const Eina_Value_Type *type EINA_UNUSED, const void *_mem, void *_ptr)
{
   Efl_Dbg_Info **ptr = _ptr;
   *ptr = (void *) _mem;
   return EINA_TRUE;
}

static const Eina_Value_Type _EFL_DBG_INFO_TYPE = {
   EINA_VALUE_TYPE_VERSION,
   sizeof(Efl_Dbg_Info *),
   "Efl_Dbg_Info_Ptr",
   _eo_dbg_info_setup,
   _eo_dbg_info_flush,
   _eo_dbg_info_copy,
   NULL,
   _eo_dbg_info_convert_to,
   NULL,
   NULL,
   _eo_dbg_info_pset,
   _eo_dbg_info_pget
};

EAPI const Eina_Value_Type *EFL_DBG_INFO_TYPE = &_EFL_DBG_INFO_TYPE;


/* EOF event callbacks */

/* EFL_OBJECT_CLASS stuff */
#define MY_CLASS EFL_OBJECT_CLASS

EOLIAN static Eo *
_efl_object_constructor(Eo *obj, Efl_Object_Data *pd EINA_UNUSED)
{
   DBG("%p - %s.", obj, efl_class_name_get(obj));

   _eo_condtor_done(obj);

   return obj;
}

EOLIAN static void
_efl_object_destructor(Eo *obj, Efl_Object_Data *pd)
{
   Eo *child;
   Efl_Object_Extension *ext;
   _Eo_Object *obj_data2 = NULL;

   DBG("%p - %s.", obj, efl_class_name_get(obj));

   // special removal - remove from children list by hand after getting
   // child handle in case unparent method is overridden and does
   // extra things like removes other children too later on in the list
   // this is a goto because more often than not objects do not have children
   // so it's unlikely they will need the child cleanup code to so to have
   // better l1 cache instruction coherency, move this to the end
   if (pd->children) goto children;
children_back:

   // If we are a composite object, detach children. it is quite unlikely
   // we are a composite object, so put the core of this handling
   // at the end out of l1 cache prefetch
     {
        EO_OBJ_POINTER_RETURN(obj, obj_data);
        obj_data2 = obj_data;
        if (obj_data->composite_objects) goto composite_obj;
composite_obj_back:
        EO_OBJ_DONE(obj);
     }

   if (pd->ext && pd->ext->composite_parent)
     efl_composite_detach(pd->ext->composite_parent, obj);

   // parent still being here is unlikely, so move error handling out of the
   // code execution path
   if (pd->parent) goto err_parent;
err_parent_back:

   _eo_generic_data_del_all(obj, pd);
   _wref_destruct(pd);
   _eo_callback_remove_all(pd);

   ext = pd->ext;
   // it is rather likely we dont have any extension section for most objects
   // so return immediately here to avoid pulling in more instructions to
   // the 1l cache if we can
   if (!ext)
     {
        _eo_condtor_done(obj);
        return;
     }
   eina_stringshare_del(ext->name);
   ext->name = NULL;
   eina_stringshare_del(ext->comment);
   ext->comment = NULL;
   while (pd->ext && ext->futures)
     efl_future_cancel(eina_list_data_get(ext->futures));
   _efl_object_extension_noneed(pd);
   _eo_condtor_done(obj);
   return;

children:
   while (pd->children)
     {
        child = _eo_obj_id_get(EINA_INLIST_CONTAINER_GET(pd->children, _Eo_Object));
        efl_parent_set(child, NULL);
     }
   goto children_back;

composite_obj:
     {
        Eina_List *itr, *next;
        Eo *emb_obj_id;
        EINA_LIST_FOREACH_SAFE(obj_data2->composite_objects, itr, next, emb_obj_id)
          {
             efl_composite_detach(obj, emb_obj_id);
          }
     }
   goto composite_obj_back;

err_parent:
   ERR("Object '%p' still has a parent at the time of destruction.", obj);
   efl_parent_set(obj, NULL);
   goto err_parent_back;
}

EOLIAN static Eo *
_efl_object_finalize(Eo *obj, Efl_Object_Data *pd EINA_UNUSED)
{
   return obj;
}

EOLIAN static void
_efl_object_class_constructor(Efl_Class *klass EINA_UNUSED)
{
   event_freeze_count = 0;
   _legacy_events_hash = eina_hash_stringshared_new(_legacy_events_hash_free_cb);
}

EOLIAN static void
_efl_object_class_destructor(Efl_Class *klass EINA_UNUSED)
{
   eina_hash_free(_legacy_events_hash);
}

static void
_efl_object_future_link_tracking_end(void *data, const Efl_Event *ev)
{
   Efl_Future *link = ev->object;
   Eo *obj = data;
   Efl_Object_Data *pd = efl_data_scope_get(obj, EFL_OBJECT_CLASS);
   Efl_Object_Extension *ext = _efl_object_extension_need(pd);

   ext->futures = eina_list_remove(ext->futures, link);
   _efl_object_extension_noneed(pd);
}

EOLIAN static Eina_Bool
_efl_object_future_link(Eo *obj EINA_UNUSED, Efl_Object_Data *pd, Efl_Future *link)
{
   Efl_Object_Extension *ext;

   if (!link) return EINA_FALSE;

   ext = _efl_object_extension_need(pd);

   ext->futures = eina_list_append(ext->futures, link);
   return !!efl_future_then(link, _efl_object_future_link_tracking_end, _efl_object_future_link_tracking_end, NULL, obj);
}

#include "efl_object.eo.c"
