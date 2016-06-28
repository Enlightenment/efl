#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Eina.h>
#include <fnmatch.h>

#include "Eo.h"
#include "eo_ptr_indirection.h"
#include "eo_private.h"

static int event_freeze_count = 0;

typedef struct _Eo_Callback_Description Eo_Callback_Description;

typedef struct
{
   const char                *name;
   const char                *comment;
   Eo                        *composite_parent;
   Eina_Inlist               *generic_data;
   Eo                      ***wrefs;
} Eo_Base_Extension;

typedef struct
{
   Eina_Inlist               *children;
   Eo                        *parent;

   Eo_Base_Extension         *ext;
   Eo_Callback_Description   *callbacks;

   Eina_Inlist               *current;

   unsigned short             walking_list;
   unsigned short             event_freeze_count;
   Eina_Bool                  deletions_waiting : 1;
   Eina_Bool                  callback_stopped : 1;
} Eo_Base_Data;

typedef enum {
     DATA_PTR,
     DATA_OBJ,
     DATA_VAL
} Eo_Generic_Data_Node_Type;

typedef struct
{
   EINA_INLIST;
   Eina_Stringshare          *key;
   union {
        Eina_Value *val;
        Eo *obj;
        void *ptr;
   } d;
   Eo_Generic_Data_Node_Type  d_type;
} Eo_Generic_Data_Node;

typedef struct
{
   EINA_INLIST;
   const Eo_Event_Description *desc;
   Eo_Callback_Description *current;
} Eo_Current_Callback_Description;

static inline void
_eo_base_extension_free(Eo_Base_Extension *ext)
{
   free(ext);
}

static inline void
_eo_base_extension_need(Eo_Base_Data *pd)
{
   if (pd->ext) return;
   pd->ext = calloc(1, sizeof(Eo_Base_Extension));
}

static inline void
_eo_base_extension_noneed(Eo_Base_Data *pd)
{
   Eo_Base_Extension *ext = pd->ext;
   if ((!ext) || (ext->name) || (ext->comment) || (ext->generic_data) ||
       (ext->wrefs) || (ext->composite_parent)) return;
   _eo_base_extension_free(pd->ext);
   pd->ext = NULL;
}




static void
_eo_generic_data_node_free(Eo_Generic_Data_Node *node)
{
   switch (node->d_type)
     {
      case DATA_OBJ:
         eo_unref(node->d.obj);
         break;
      case DATA_VAL:
         eina_value_free(node->d.val);
         break;
      case DATA_PTR:
         break;
     }
   eina_stringshare_del(node->key);
   free(node);
}

static void
_eo_generic_data_del_all(Eo *obj EINA_UNUSED, Eo_Base_Data *pd)
{
   Eo_Generic_Data_Node *node;
   Eo_Base_Extension *ext = pd->ext;

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
_eo_key_generic_del(const Eo *obj EINA_UNUSED, Eo_Base_Data *pd, const char *key)
{
   Eo_Generic_Data_Node *node;
   Eo_Base_Extension *ext = pd->ext;

   EINA_INLIST_FOREACH(ext->generic_data, node)
     {
        if (!strcmp(node->key, key))
          {
             ext->generic_data = eina_inlist_remove
               (ext->generic_data, EINA_INLIST_GET(node));
             _eo_generic_data_node_free(node);
             return;
          }
     }
}

/* Return TRUE if the object was newly added. */
static Eina_Bool
_key_generic_set(const Eo *obj, Eo_Base_Data *pd, const char *key, const void *data, Eo_Generic_Data_Node_Type d_type)
{
   Eo_Generic_Data_Node *node;
   Eo_Base_Extension *ext = pd->ext;

   if (!key) return EINA_FALSE;
   if (ext)
     {
        if (!data)
          {
             _eo_key_generic_del(obj, pd, key);
             return EINA_TRUE;
          }
        EINA_INLIST_FOREACH(ext->generic_data, node)
          {
             if (!strcmp(node->key, key))
               {
                  if ((node->d_type == d_type) && (node->d.ptr == data))
                     return EINA_FALSE;
                  ext->generic_data = eina_inlist_remove
                    (ext->generic_data, EINA_INLIST_GET(node));
                  _eo_generic_data_node_free(node);
                  break;
               }
          }
     }

   _eo_base_extension_need(pd);
   ext = pd->ext;
   if (ext)
     {
        node = calloc(1, sizeof(Eo_Generic_Data_Node));
        if (!node) return EINA_FALSE;
        node->key = eina_stringshare_add(key);
        node->d.ptr = (void *) data;
        node->d_type = d_type;
        ext->generic_data = eina_inlist_prepend
          (ext->generic_data, EINA_INLIST_GET(node));
        return EINA_TRUE;
     }

   return EINA_FALSE;
}

static void *
_key_generic_get(const Eo *obj, Eo_Base_Data *pd, const char *key, Eo_Generic_Data_Node_Type d_type)
{
   Eo_Generic_Data_Node *node;
   Eo_Base_Extension *ext = pd->ext;

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

EOLIAN static void
_eo_base_key_data_set(Eo *obj, Eo_Base_Data *pd, const char *key, const void *data)
{
   _key_generic_set(obj, pd, key, data, DATA_PTR);
}

EOLIAN static void *
_eo_base_key_data_get(Eo *obj, Eo_Base_Data *pd, const char *key)
{
   return _key_generic_get(obj, pd, key, DATA_PTR);
}

EOLIAN static void
_eo_base_key_obj_set(Eo *obj EINA_UNUSED, Eo_Base_Data *pd, const char *key, const Eo *objdata)
{
   if (_key_generic_set(obj, pd, key, objdata, DATA_OBJ))
      eo_ref(objdata);
}

EOLIAN static Eo *
_eo_base_key_obj_get(Eo *obj, Eo_Base_Data *pd, const char *key)
{
   return _key_generic_get(obj, pd, key, DATA_OBJ);
}

EOLIAN static void
_eo_base_key_value_set(Eo *obj EINA_UNUSED, Eo_Base_Data *pd, const char *key, Eina_Value *value)
{
   _key_generic_set(obj, pd, key, value, DATA_VAL);
}

EOLIAN static Eina_Value *
_eo_base_key_value_get(Eo *obj, Eo_Base_Data *pd, const char *key)
{
   return _key_generic_get(obj, pd, key, DATA_VAL);
}

EOLIAN static void
_eo_base_name_set(Eo *obj EINA_UNUSED, Eo_Base_Data *pd, const char *name)
{
   if ((name) && (!name[0])) name = NULL;
   if (name)
     {
        _eo_base_extension_need(pd);
        if (pd->ext) eina_stringshare_replace(&(pd->ext->name), name);
     }
   else
     {
        if (!pd->ext) return;
        if (pd->ext->name)
          {
             eina_stringshare_replace(&(pd->ext->name), name);
             _eo_base_extension_noneed(pd);
          }
     }
}

EOLIAN static const char *
_eo_base_name_get(Eo *obj EINA_UNUSED, Eo_Base_Data *pd)
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

EOLIAN static Eo_Base *
_eo_base_name_find(Eo *obj EINA_UNUSED, Eo_Base_Data *pd, const char *search)
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
                  name = eo_name_get(child);
                  klass_name = eo_class_name_get(eo_class_get(child));
                  if (_name_match(klass, klass_glob, klass_name) &&
                      (((!_matchall(klass)) && (!name) && (_matchall(search_name))) ||
                       ((name) && _name_match(search_name, name_glob, name))))
                    return child;
                  child = eo_name_find(child, search);
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
                       name = eo_name_get(child);
                       if ((name) && (_name_match(search, EINA_TRUE, name)))
                         return child;
                       child = eo_name_find(child, search);
                       if (child) return child;
                    }
               }
             else
               {
                  // fast path for simple "name"
                  EINA_INLIST_FOREACH(pd->children, child_eo)
                    {
                       child = _eo_obj_id_get(child_eo);
                       name = eo_name_get(child);
                       if ((name) && (_name_match(search, EINA_FALSE, name)))
                         return child;
                       child = eo_name_find(child, search);
                       if (child) return child;
                    }
               }
          }
     }
   return NULL;
}

EOLIAN static void
_eo_base_comment_set(Eo *obj EINA_UNUSED, Eo_Base_Data *pd, const char *comment)
{
   if ((comment) && (!comment[0])) comment = NULL;
   if (comment)
     {
        _eo_base_extension_need(pd);
        if (pd->ext) eina_stringshare_replace(&(pd->ext->comment), comment);
     }
   else
     {
        if (!pd->ext) return;
        if (pd->ext->comment)
          {
             eina_stringshare_replace(&(pd->ext->comment), comment);
             _eo_base_extension_noneed(pd);
          }
     }
}

EOLIAN static const char *
_eo_base_comment_get(Eo *obj EINA_UNUSED, Eo_Base_Data *pd)
{
   if (!pd->ext) return NULL;
   return pd->ext->comment;
}

EOLIAN static void
_eo_base_del(const Eo *obj, Eo_Base_Data *pd EINA_UNUSED)
{
   if (eo_parent_get((Eo *) obj))
     {
        eo_parent_set((Eo *) obj, NULL);
     }
   else
     {
        eo_unref(obj);
     }
}

EOLIAN static void
_eo_base_parent_set(Eo *obj, Eo_Base_Data *pd, Eo *parent_id)
{
   if (pd->parent == parent_id)
     return;

   EO_OBJ_POINTER(obj, eo_obj);

   if (pd->parent)
     {
        Eo_Base_Data *old_parent_pd;

        old_parent_pd = eo_data_scope_get(pd->parent, EO_BASE_CLASS);
        if (old_parent_pd)
          {
             old_parent_pd->children = eina_inlist_remove(old_parent_pd->children,
                   EINA_INLIST_GET(eo_obj));
          }
        else
          {
             ERR("CONTACT DEVS!!! SHOULD NEVER HAPPEN!!! Old parent %p for object %p is not a valid Eo object.",
                 pd->parent, obj);
          }

        /* Only unref if we don't have a new parent instead and we are not at
         * the process of deleting the object.*/
        if (!parent_id && !eo_obj->del_triggered)
          {
             _eo_unref(eo_obj);
          }
     }

   /* Set new parent */
   if (parent_id)
     {
        Eo_Base_Data *parent_pd = NULL;
        parent_pd = eo_data_scope_get(parent_id, EO_BASE_CLASS);

        if (EINA_LIKELY(parent_pd != NULL))
          {
             pd->parent = parent_id;
             parent_pd->children = eina_inlist_append(parent_pd->children,
                   EINA_INLIST_GET(eo_obj));
          }
        else
          {
             pd->parent = NULL;
             ERR("New parent %p for object %p is not a valid Eo object.",
                 parent_id, obj);
          }
     }
   else
     {
        pd->parent = NULL;
     }
}

EOLIAN static Eo *
_eo_base_parent_get(Eo *obj EINA_UNUSED, Eo_Base_Data *pd)
{
   return pd->parent;
}

EOLIAN static Eina_Bool
_eo_base_finalized_get(Eo *obj_id, Eo_Base_Data *pd EINA_UNUSED)
{
   EO_OBJ_POINTER_RETURN_VAL(obj_id, obj, EINA_FALSE);

   return obj->finalized;
}

EOLIAN static Eo_Base *
_eo_base_provider_find(Eo *obj EINA_UNUSED, Eo_Base_Data *pd, const Eo_Base *klass)
{
   if (pd->parent) return eo_provider_find(pd->parent, klass);
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
_eo_children_iterator_next(Eo_Children_Iterator *it, void **data)
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
_eo_children_iterator_container(Eo_Children_Iterator *it)
{
   return it->obj_id;
}

static void
_eo_children_iterator_free(Eo_Children_Iterator *it)
{
   _Eo_Class *klass;
   _Eo_Object *obj;

   klass = (_Eo_Class*) it->obj->klass;
   obj = it->obj;

   eina_spinlock_take(&klass->iterators.trash_lock);
   if (klass->iterators.trash_count < 8)
     {
        klass->iterators.trash_count++;
        eina_trash_push(&klass->iterators.trash, it);
     }
   else
     {
        free(it);
     }
   eina_spinlock_release(&klass->iterators.trash_lock);

   _eo_unref(obj);
}

EOLIAN static Eina_Iterator *
_eo_base_children_iterator_new(Eo *obj_id, Eo_Base_Data *pd)
{
   _Eo_Class *klass;
   Eo_Children_Iterator *it;

   EO_OBJ_POINTER_RETURN_VAL(obj_id, obj, NULL);

   if (!pd->children) return NULL;

   klass = (_Eo_Class *) obj->klass;

   eina_spinlock_take(&klass->iterators.trash_lock);
   it = eina_trash_pop(&klass->iterators.trash);
   if (it)
     {
        klass->iterators.trash_count--;
        memset(it, 0, sizeof (Eo_Children_Iterator));
     }
   else
     {
        it = calloc(1, sizeof (Eo_Children_Iterator));
     }
   eina_spinlock_release(&klass->iterators.trash_lock);
   if (!it) return NULL;

   EINA_MAGIC_SET(&it->iterator, EINA_MAGIC_ITERATOR);
   it->current = pd->children;
   it->obj = _eo_ref(obj);
   it->obj_id = obj_id;

   it->iterator.next = FUNC_ITERATOR_NEXT(_eo_children_iterator_next);
   it->iterator.get_container = FUNC_ITERATOR_GET_CONTAINER(_eo_children_iterator_container);
   it->iterator.free = FUNC_ITERATOR_FREE(_eo_children_iterator_free);

   return (Eina_Iterator *)it;
}

EOLIAN static void
_eo_base_dbg_info_get(Eo *obj EINA_UNUSED, Eo_Base_Data *pd EINA_UNUSED, Eo_Dbg_Info *root_node EINA_UNUSED)
{  /* No info required in the meantime */
   return;
}

/* Weak reference. */

static inline size_t
_wref_count(Eo_Base_Data *pd)
{
   Eo ***itr;
   size_t count = 0;
   Eo_Base_Extension *ext = pd->ext;

   if ((!ext) || (!ext->wrefs)) return 0;
   for (itr = ext->wrefs; *itr; itr++) count++;

   return count;
}

EOLIAN static void
_eo_base_wref_add(Eo *obj, Eo_Base_Data *pd, Eo **wref)
{
   size_t count;
   Eo ***tmp;
   Eo_Base_Extension *ext;

   count = _wref_count(pd);
   count += 1; /* New wref. */

   _eo_base_extension_need(pd);
   ext = pd->ext;
   if (!ext) return;

   tmp = realloc(ext->wrefs, sizeof(*ext->wrefs) * (count + 1));
   if (!tmp) return;
   ext->wrefs = tmp;

   ext->wrefs[count - 1] = wref;
   ext->wrefs[count] = NULL;
   *wref = obj;
}

EOLIAN static void
_eo_base_wref_del(Eo *obj, Eo_Base_Data *pd, Eo **wref)
{
   size_t count;
   Eo_Base_Extension *ext = pd->ext;

   if (*wref != obj)
     {
        ERR("Wref is a weak ref to %p, while this function was called on %p.",
            *wref, obj);
        return;
     }

   if ((!ext) || (!ext->wrefs))
     {
        ERR("There are no weak refs for object %p", obj);
        *wref = NULL;
        return;
     }

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

        if (!*itr)
          {
             ERR("Wref %p is not associated with object %p", wref, obj);
             *wref = NULL;
             return;
          }
     }

   if (count > 1)
     {
        Eo ***tmp;
        // No count--; because of the NULL that is not included in the count
        tmp = realloc(ext->wrefs, sizeof(*ext->wrefs) * count);
        if (!tmp) return;
        ext->wrefs = tmp;
        ext->wrefs[count - 1] = NULL;
     }
   else
     {
        free(ext->wrefs);
        ext->wrefs = NULL;
        _eo_base_extension_noneed(pd);
     }

   *wref = NULL;
}

static inline void
_wref_destruct(Eo_Base_Data *pd)
{
   Eo ***itr;
   Eo_Base_Extension *ext = pd->ext;

   if ((!ext) || (!ext->wrefs)) return;
   for (itr = ext->wrefs; *itr; itr++) **itr = NULL;
   free(ext->wrefs);
   ext->wrefs = NULL;
}

/* EOF Weak reference. */

/* Event callbacks */

/* Callbacks */

/* XXX: Legacy support, remove when legacy is dead. */
static Eina_Hash *_legacy_events_hash = NULL;

EAPI const Eo_Event_Description *
eo_base_legacy_only_event_description_get(const char *_event_name)
{
   Eina_Stringshare *event_name = eina_stringshare_add(_event_name);
   Eo_Event_Description *event_desc = eina_hash_find(_legacy_events_hash, event_name);
   if (!event_desc)
     {
        event_desc = calloc(1, sizeof(Eo_Event_Description));
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
_legacy_event_desc_is(const Eo_Event_Description *desc)
{
   return desc->legacy_is;
}

static void
_legacy_events_hash_free_cb(void *_desc)
{
   Eo_Event_Description *desc = _desc;
   eina_stringshare_del(desc->name);
   free(desc);
}

/* EOF Legacy */

struct _Eo_Callback_Description
{
   Eo_Callback_Description *next;

   union
     {
        Eo_Callback_Array_Item item;
        const Eo_Callback_Array_Item *item_array;
     } items;

   void *func_data;
   Eo_Callback_Priority priority;

   Eina_Bool delete_me : 1;
   Eina_Bool func_array : 1;
};

/* Actually remove, doesn't care about walking list, or delete_me */
static void
_eo_callback_remove(Eo_Base_Data *pd, Eo_Callback_Description *cb)
{
   Eo_Callback_Description *itr, *pitr = NULL;

   itr = pd->callbacks;

   for ( ; itr; )
     {
        Eo_Callback_Description *titr = itr;
        itr = itr->next;

        if (titr == cb)
          {
             if (pitr)
               {
                  pitr->next = titr->next;
               }
             else
               {
                  pd->callbacks = titr->next;
               }
             free(titr);
          }
        else
          {
             pitr = titr;
          }
     }
}

/* Actually remove, doesn't care about walking list, or delete_me */
static void
_eo_callback_remove_all(Eo_Base_Data *pd)
{
   while (pd->callbacks)
     {
        Eo_Callback_Description *next = pd->callbacks->next;
        free(pd->callbacks);
        pd->callbacks = next;
     }
}

static void
_eo_callbacks_clear(Eo_Base_Data *pd)
{
   Eo_Callback_Description *cb = NULL;

   /* If there are no deletions waiting. */
   if (!pd->deletions_waiting)
      return;

   /* Abort if we are currently walking the list. */
   if (pd->walking_list > 0)
      return;

   pd->deletions_waiting = EINA_FALSE;

   for (cb = pd->callbacks; cb; )
     {
        Eo_Callback_Description *titr = cb;
        cb = cb->next;

        if (titr->delete_me)
          {
             _eo_callback_remove(pd, titr);
          }
     }
}

static void
_eo_callbacks_sorted_insert(Eo_Base_Data *pd, Eo_Callback_Description *cb)
{
   Eo_Callback_Description *itr, *itrp = NULL;
   for (itr = pd->callbacks; itr && (itr->priority < cb->priority);
         itr = itr->next)
     {
        itrp = itr;
     }

   if (itrp)
     {
        cb->next = itrp->next;
        itrp->next = cb;
     }
   else
     {
        cb->next = pd->callbacks;
        pd->callbacks = cb;
     }
}

EOLIAN static Eina_Bool
_eo_base_event_callback_priority_add(Eo *obj, Eo_Base_Data *pd,
                    const Eo_Event_Description *desc,
                    Eo_Callback_Priority priority,
                    Eo_Event_Cb func,
                    const void *user_data)
{
   const Eo_Callback_Array_Item arr[] = { {desc, func}, {NULL, NULL}};
   Eo_Callback_Description *cb;

   cb = calloc(1, sizeof(*cb));
   if (!cb || !desc || !func)
     {
        ERR("Tried adding callback with invalid values: cb: %p desc: %p func: %p\n", cb, desc, func);
        free(cb);
        return EINA_FALSE;
     }
   cb->items.item.desc = desc;
   cb->items.item.func = func;
   cb->func_data = (void *) user_data;
   cb->priority = priority;
   _eo_callbacks_sorted_insert(pd, cb);

   eo_event_callback_call(obj, EO_EVENT_CALLBACK_ADD, (void *)arr);

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_eo_base_event_callback_del(Eo *obj, Eo_Base_Data *pd,
                    const Eo_Event_Description *desc,
                    Eo_Event_Cb func,
                    const void *user_data)
{
   Eo_Callback_Description *cb;

   for (cb = pd->callbacks; cb; cb = cb->next)
     {
        if (!cb->delete_me && (cb->items.item.desc == desc) &&
              (cb->items.item.func == func) && (cb->func_data == user_data))
          {
             const Eo_Callback_Array_Item arr[] = { {desc, func}, {NULL, NULL}};

             cb->delete_me = EINA_TRUE;
             pd->deletions_waiting = EINA_TRUE;
             _eo_callbacks_clear(pd);
             eo_event_callback_call(obj, EO_EVENT_CALLBACK_DEL, (void *)arr);
             return EINA_TRUE;
          }
     }

   DBG("Callback of object %p with function %p and data %p not found.", obj, func, user_data);
   return EINA_FALSE;
}

EOLIAN static Eina_Bool
_eo_base_event_callback_array_priority_add(Eo *obj, Eo_Base_Data *pd,
                          const Eo_Callback_Array_Item *array,
                          Eo_Callback_Priority priority,
                          const void *user_data)
{
   Eo_Callback_Description *cb;

   cb = calloc(1, sizeof(*cb));
   if (!cb || !array)
     {
        ERR("Tried adding array of callbacks with invalid values: cb: %p array: %p\n", cb, array);
        free(cb);
        return EINA_FALSE;
     }
   cb->func_data = (void *) user_data;
   cb->priority = priority;
   cb->items.item_array = array;
   cb->func_array = EINA_TRUE;
   _eo_callbacks_sorted_insert(pd, cb);

   eo_event_callback_call(obj, EO_EVENT_CALLBACK_ADD, (void *)array);

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_eo_base_event_callback_array_del(Eo *obj, Eo_Base_Data *pd,
                 const Eo_Callback_Array_Item *array,
                 const void *user_data)
{
   Eo_Callback_Description *cb;

   for (cb = pd->callbacks; cb; cb = cb->next)
     {
        if (!cb->delete_me &&
              (cb->items.item_array == array) && (cb->func_data == user_data))
          {
             cb->delete_me = EINA_TRUE;
             pd->deletions_waiting = EINA_TRUE;
             _eo_callbacks_clear(pd);

             eo_event_callback_call(obj, EO_EVENT_CALLBACK_DEL, (void *)array);
             return EINA_TRUE;
          }
     }

   DBG("Callback of object %p with function array %p and data %p not found.", obj, array, user_data);
   return EINA_FALSE;
}

static Eina_Bool
_cb_desc_match(const Eo_Event_Description *a, const Eo_Event_Description *b)
{
   /* If one is legacy and the other is not, strcmp. Else, pointer compare. */
   if (EINA_UNLIKELY(_legacy_event_desc_is(a) != _legacy_event_desc_is(b)))
     {
        return !strcmp(a->name, b->name);
     }

   return (a == b);
}

EOLIAN static Eina_Bool
_eo_base_event_callback_call(Eo *obj_id, Eo_Base_Data *pd,
            const Eo_Event_Description *desc,
            void *event_info)
{
   Eina_Bool callback_already_stopped = pd->callback_stopped;
   Eina_Bool ret = EINA_TRUE;
   Eo_Callback_Description *cb;
   Eo_Current_Callback_Description *lookup = NULL;
   Eo_Current_Callback_Description saved;
   Eo_Event ev;

   ev.object = obj_id;
   ev.desc = desc;
   ev.info = event_info;

   pd->walking_list++;

   // Handle event that require to restart where we were in the nested list walking
   if (desc->restart)
     {
        EINA_INLIST_FOREACH(pd->current, lookup)
          if (lookup->desc == desc)
            break;

        // This is the first event to trigger it, so register it here
        if (!lookup)
          {
             // This following trick get us a zero allocation list
             saved.desc = desc;
             saved.current = NULL;
             lookup = &saved;
             // Ideally there will most of the time be only one item in this list
             // But just to speed up things, prepend so we find it fast at the end
             // of this function
             pd->current = eina_inlist_prepend(pd->current, EINA_INLIST_GET(lookup));
          }

        if (!lookup->current) lookup->current = pd->callbacks;
        cb = lookup->current;
     }
   else
     {
        cb = pd->callbacks;
     }

   for (; cb; cb = cb->next)
     {
        if (!cb->delete_me)
          {
             if (cb->func_array)
               {
                  const Eo_Callback_Array_Item *it;

                  for (it = cb->items.item_array; it->func; it++)
                    {
                       if (!_cb_desc_match(it->desc, desc))
                          continue;
                       if (!it->desc->unfreezable &&
                           (event_freeze_count || pd->event_freeze_count))
                          continue;

                       // Handle nested restart of walking list
                       if (lookup) lookup->current = cb->next;
                       it->func((void *) cb->func_data, &ev);
                       /* Abort callback calling if the func says so. */
                       if (pd->callback_stopped)
                         {
                            ret = EINA_FALSE;
                            goto end;
                         }
                       // We have actually walked this list during a nested call
                       if (lookup &&
                           lookup->current == NULL)
                         goto end;
                    }
               }
             else
               {
                  if (!_cb_desc_match(cb->items.item.desc, desc))
                    continue;
                  if (!cb->items.item.desc->unfreezable &&
                      (event_freeze_count || pd->event_freeze_count))
                    continue;

                  // Handle nested restart of walking list
                  if (lookup) lookup->current = cb->next;
                  cb->items.item.func((void *) cb->func_data, &ev);
                  /* Abort callback calling if the func says so. */
                  if (pd->callback_stopped)
                    {
                       ret = EINA_FALSE;
                       goto end;
                    }
                  // We have actually walked this list during a nested call
                  if (lookup &&
                      lookup->current == NULL)
                    goto end;
               }
          }
     }

end:
   // Handling restarting list walking complete exit.
   if (lookup) lookup->current = NULL;
   if (lookup == &saved)
     {
        pd->current = eina_inlist_remove(pd->current, EINA_INLIST_GET(lookup));
     }

   pd->walking_list--;
   _eo_callbacks_clear(pd);

   pd->callback_stopped = callback_already_stopped;

   return ret;
}

EOLIAN static void
_eo_base_event_callback_stop(Eo *obj EINA_UNUSED, Eo_Base_Data *pd)
{
   pd->callback_stopped = EINA_TRUE;
}

static void
_eo_event_forwarder_callback(void *data, const Eo_Event *event)
{
   Eo *new_obj = (Eo *) data;
   Eina_Bool ret = EINA_FALSE;

   ret = eo_event_callback_call(new_obj, event->desc, event->info);

   if (!ret)
     {
        eo_event_callback_stop(event->object);
     }
}

/* FIXME: Change default priority? Maybe call later? */
EOLIAN static void
_eo_base_event_callback_forwarder_add(Eo *obj, Eo_Base_Data *pd EINA_UNUSED,
                     const Eo_Event_Description *desc,
                     Eo *new_obj)
{

   /* FIXME: Add it EO_MAGIC_RETURN(new_obj, EO_EINA_MAGIC); */

   eo_event_callback_add(obj, desc, _eo_event_forwarder_callback, new_obj);
}

EOLIAN static void
_eo_base_event_callback_forwarder_del(Eo *obj, Eo_Base_Data *pd EINA_UNUSED,
                     const Eo_Event_Description *desc,
                     Eo *new_obj)
{

   /* FIXME: Add it EO_MAGIC_RETURN(new_obj, EO_EINA_MAGIC); */

   eo_event_callback_del(obj, desc, _eo_event_forwarder_callback, new_obj);
}

EOLIAN static void
_eo_base_event_freeze(Eo *obj EINA_UNUSED, Eo_Base_Data *pd)
{
   pd->event_freeze_count++;
}

EOLIAN static void
_eo_base_event_thaw(Eo *obj, Eo_Base_Data *pd)
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
_eo_base_event_freeze_count_get(Eo *obj EINA_UNUSED, Eo_Base_Data *pd)
{
   return pd->event_freeze_count;
}

EOLIAN static void
_eo_base_event_global_freeze(Eo *klass EINA_UNUSED, void *pd EINA_UNUSED)
{
   event_freeze_count++;
}

EOLIAN static void
_eo_base_event_global_thaw(Eo *klass EINA_UNUSED, void *pd EINA_UNUSED)
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
_eo_base_event_global_freeze_count_get(Eo *klass EINA_UNUSED, void *pd EINA_UNUSED)
{
   return event_freeze_count;
}

EOLIAN static Eina_Bool
_eo_base_composite_attach(Eo *parent_id, Eo_Base_Data *pd EINA_UNUSED, Eo *comp_obj_id)
{
   EO_OBJ_POINTER_RETURN_VAL(comp_obj_id, comp_obj, EINA_FALSE);
   EO_OBJ_POINTER_RETURN_VAL(parent_id, parent, EINA_FALSE);

   Eo_Base_Data *comp_pd = eo_data_scope_get(comp_obj_id, EO_BASE_CLASS);
   /* Don't composite if we already have a composite object of this type */
     {
        Eina_List *itr;
        Eo *emb_obj_id;
        EINA_LIST_FOREACH(parent->composite_objects, itr, emb_obj_id)
          {
             EO_OBJ_POINTER_RETURN_VAL(emb_obj_id, emb_obj, EINA_FALSE);
             if (emb_obj->klass == comp_obj->klass)
               return EINA_FALSE;
          }
     }

   if (eo_composite_part_is(comp_obj_id))
     {
        eo_composite_detach(comp_pd->ext->composite_parent, comp_obj_id);
     }

   /* Set the parent comp on the child. */
   _eo_base_extension_need(comp_pd);
   comp_pd->ext->composite_parent = parent_id;

   parent->composite_objects = eina_list_prepend(parent->composite_objects, comp_obj_id);

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_eo_base_composite_detach(Eo *parent_id, Eo_Base_Data *pd EINA_UNUSED, Eo *comp_obj_id)
{
   EO_OBJ_POINTER_RETURN_VAL(comp_obj_id, comp_obj, EINA_FALSE);
   EO_OBJ_POINTER_RETURN_VAL(parent_id, parent, EINA_FALSE);

   if (!eo_composite_part_is(comp_obj_id))
      return EINA_FALSE;

   parent->composite_objects = eina_list_remove(parent->composite_objects, comp_obj_id);
   /* Clear the comp parent on the child. */
     {
        Eo_Base_Data *comp_pd = eo_data_scope_get(comp_obj_id, EO_BASE_CLASS);
        comp_pd->ext->composite_parent = NULL;

        _eo_base_extension_noneed(comp_pd);
     }

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_eo_base_composite_part_is(Eo *comp_obj_id EINA_UNUSED, Eo_Base_Data *pd)
{
   return pd->ext && pd->ext->composite_parent;
}

/* Eo_Dbg */
EAPI void
eo_dbg_info_free(Eo_Dbg_Info *info)
{
   eina_value_flush(&(info->value));
   free(info);
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
   Eo_Dbg_Info *mem = *(Eo_Dbg_Info **) _mem;
   eina_stringshare_del(mem->name);
   eina_value_flush(&(mem->value));
   free(mem);
   return EINA_TRUE;
}

static Eina_Bool
_eo_dbg_info_copy(const Eina_Value_Type *type EINA_UNUSED, const void *_src, void *_dst)
{
   const Eo_Dbg_Info **src = (const Eo_Dbg_Info **) _src;
   Eo_Dbg_Info **dst = _dst;

   *dst = calloc(1, sizeof(Eo_Dbg_Info));
   if (!*dst) return EINA_FALSE;
   (*dst)->name = eina_stringshare_ref((*src)->name);
   eina_value_copy(&((*src)->value), &((*dst)->value));
   return EINA_TRUE;
}

static Eina_Bool
_eo_dbg_info_convert_to(const Eina_Value_Type *type EINA_UNUSED, const Eina_Value_Type *convert, const void *type_mem, void *convert_mem)
{
   /* FIXME: For the meanwhile, just use the inner type for the value. */
   const Eo_Dbg_Info **src = (const Eo_Dbg_Info **) type_mem;
   if (convert == EINA_VALUE_TYPE_STRINGSHARE ||
       convert == EINA_VALUE_TYPE_STRING)
     {
        Eina_Bool ret;
        const char *other_mem;
        char *inner_val = eina_value_to_string(&(*src)->value);
        other_mem = inner_val;
        ret = eina_value_type_pset(convert, convert_mem, &other_mem);
        free(inner_val);
        return ret;
     }

   eina_error_set(EINA_ERROR_VALUE_FAILED);
   return EINA_FALSE;
}

static Eina_Bool
_eo_dbg_info_pset(const Eina_Value_Type *type EINA_UNUSED, void *_mem, const void *_ptr)
{
   Eo_Dbg_Info **mem = _mem;
   if (*mem)
     free(*mem);
   *mem = (void *) _ptr;
   return EINA_TRUE;
}

static Eina_Bool
_eo_dbg_info_pget(const Eina_Value_Type *type EINA_UNUSED, const void *_mem, void *_ptr)
{
   Eo_Dbg_Info **ptr = _ptr;
   *ptr = (void *) _mem;
   return EINA_TRUE;
}

static const Eina_Value_Type _EO_DBG_INFO_TYPE = {
   EINA_VALUE_TYPE_VERSION,
   sizeof(Eo_Dbg_Info *),
   "Eo_Dbg_Info_Ptr",
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

EAPI const Eina_Value_Type *EO_DBG_INFO_TYPE = &_EO_DBG_INFO_TYPE;


/* EOF event callbacks */

/* EO_BASE_CLASS stuff */
#define MY_CLASS EO_BASE_CLASS

EOLIAN static Eo *
_eo_base_constructor(Eo *obj, Eo_Base_Data *pd EINA_UNUSED)
{
   DBG("%p - %s.", obj, eo_class_name_get(obj));

   _eo_condtor_done(obj);

   return obj;
}

EOLIAN static void
_eo_base_destructor(Eo *obj, Eo_Base_Data *pd)
{
   Eo *child;
   Eo_Base_Extension *ext;

   DBG("%p - %s.", obj, eo_class_name_get(obj));

   // special removal - remove from children list by hand after getting
   // child handle in case unparent method is overridden and does
   // extra things like removes other children too later on in the list
   while (pd->children)
     {
        child = _eo_obj_id_get(EINA_INLIST_CONTAINER_GET(pd->children, _Eo_Object));
        eo_parent_set(child, NULL);
     }

   /* If we are a composite object, detach children */
     {
        EO_OBJ_POINTER_RETURN(obj, obj_data);
        Eina_List *itr, *next;
        Eo *emb_obj_id;
        EINA_LIST_FOREACH_SAFE(obj_data->composite_objects, itr, next, emb_obj_id)
          {
             eo_composite_detach(obj, emb_obj_id);
          }
     }

   if (pd->ext && pd->ext->composite_parent)
     {
        eo_composite_detach(pd->ext->composite_parent, obj);
     }

   if (pd->parent)
     {
        ERR("Object '%p' still has a parent at the time of destruction.", obj);
        eo_parent_set(obj, NULL);
     }

   _eo_generic_data_del_all(obj, pd);
   _wref_destruct(pd);
   _eo_callback_remove_all(pd);

   ext = pd->ext;
   if (ext)
     {
        eina_stringshare_del(ext->name);
        ext->name = NULL;
        eina_stringshare_del(ext->comment);
        ext->comment = NULL;
        _eo_base_extension_free(ext);
        pd->ext = NULL;
     }

   _eo_condtor_done(obj);
}

EOLIAN static Eo *
_eo_base_finalize(Eo *obj, Eo_Base_Data *pd EINA_UNUSED)
{
   return obj;
}

EOLIAN static void
_eo_base_class_constructor(Eo_Class *klass EINA_UNUSED)
{
   event_freeze_count = 0;
   _legacy_events_hash = eina_hash_stringshared_new(_legacy_events_hash_free_cb);
}

EOLIAN static void
_eo_base_class_destructor(Eo_Class *klass EINA_UNUSED)
{
   eina_hash_free(_legacy_events_hash);
}

#include "eo_base.eo.c"
