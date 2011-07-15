/* EINA - EFL data type library
 * Copyright (C) 2011 Cedric Bail
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library;
 * if not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <string.h>

#include "eina_private.h"
#include "eina_inlist.h"
#include "eina_rbtree.h"
#include "eina_mempool.h"
#include "eina_trash.h"
#include "eina_log.h"
#include "eina_stringshare.h"
#include "eina_lock.h"

#include "eina_object.h"

/*============================================================================*
 *                                  Local                                     *
 *============================================================================*/

/**
 * @cond LOCAL
 */

/* If we are on a 64bits computer user bigger generation and ID */
/* FIXME: make it GCC independent */
/* FIXME: maybe having 2^32 objects doesn't make sense and 2^24 are enough
   so instead of increasing the object count, we could just add a magic
   to first check if the pointer is valid at all (and maybe use a pointer
   that will always trigger a segv if we try to use it directly).
*/
#ifdef _LP64
typedef unsigned long Eina_Object_ID;
typedef unsigned short Eina_Object_Generation;
# define EINA_GEN_OFFSET 48
# define EINA_ID_STR "%lu"
#elif _WIN64
typedef unsigned __int64 Eina_Object_ID;
typedef unsigned short Eina_Object_Generation;
# define EINA_GEN_OFFSET 48
# define EINA_ID_STR "%I64u"
#else
typedef unsigned int Eina_Object_ID;
typedef unsigned char Eina_Object_Generation;
# define EINA_GEN_OFFSET 24
# define EINA_ID_STR "%u"
#endif

typedef struct _Eina_Class_Range Eina_Class_Range;
typedef struct _Eina_Object_Item Eina_Object_Item;
typedef struct _Eina_Range Eina_Range;
typedef struct _Eina_Class_Top Eina_Class_Top;

struct _Eina_Range
{
  EINA_INLIST;

  unsigned int start;
  unsigned int end;
};

struct _Eina_Object_Item
{
  EINA_INLIST;
  Eina_Class_Range *range;

  Eina_Object_Item *parent;
  Eina_Inlist *link;

  unsigned int index;
};

struct _Eina_Class_Range
{
  EINA_INLIST;
  EINA_RBTREE;

  unsigned int start;
  unsigned int end;
  unsigned int current;
  unsigned int empty_count;

  Eina_Class *type;
  Eina_Trash *empty;

  Eina_Object_Item **pointer_array;
  Eina_Object_Generation generation_array[1];
};

struct _Eina_Class_Top
{
  Eina_Class *top_parent;
  Eina_Rbtree *range;
  Eina_Inlist *available;

  unsigned int upper_limit;
};

struct _Eina_Class
{
  EINA_INLIST;

  const char *name;

  Eina_Class_Top *top;

  Eina_Class *parent;
  Eina_Inlist *childs;

  Eina_Class_Callback constructor;
  Eina_Class_Callback destructor;
  void *data;

  Eina_Inlist *allocated_range;

  Eina_Mempool *mempool;
  unsigned int class_size;
  unsigned int object_size;
  unsigned int pool_size;

  Eina_Bool repack_needed : 1;

#ifdef EINA_HAVE_DEBUG_THREADS
  pthread_t self;
#endif
  Eina_Lock mutex;

  EINA_MAGIC;
};

static const char EINA_MAGIC_CLASS_STR[] = "Eina Class";

static Eina_Mempool *_eina_class_mp = NULL;
static Eina_Mempool *_eina_top_mp = NULL;
static Eina_Mempool *_eina_range_mp = NULL;
static int _eina_object_log_dom = -1;
static unsigned int _eina_object_item_size = 0;

#ifdef ERR
#undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_eina_object_log_dom, __VA_ARGS__)

#ifdef DBG
#undef DBG
#endif
#define DBG(...) EINA_LOG_DOM_DBG(_eina_object_log_dom, __VA_ARGS__)

#define EINA_MAGIC_CHECK_CLASS(d, ...)				 \
  do {								 \
    if (!EINA_MAGIC_CHECK(d, EINA_MAGIC_CLASS))                  \
      {								 \
	EINA_MAGIC_FAIL(d, EINA_MAGIC_CLASS);			 \
	return __VA_ARGS__;					 \
      }								 \
  } while(0)

static int
_eina_rbtree_cmp_range(const Eina_Rbtree *node, const void *key,
		       __UNUSED__ int length, __UNUSED__ void *data)
{
  Eina_Class_Range *range;
  Eina_Object_ID id;

  range = EINA_RBTREE_CONTAINER_GET(node, Eina_Class_Range);
  id = (Eina_Object_ID) key;

  if (id < range->start) return -1;
  else if (id >= range->end) return 1;
  return 0;
}

static Eina_Rbtree_Direction
_eina_class_direction_range(const Eina_Rbtree *left,
			    const Eina_Rbtree *right,
			    __UNUSED__ void *data)
{
  Eina_Class_Range *rl;
  Eina_Class_Range *rr;

  rl = EINA_RBTREE_CONTAINER_GET(left, Eina_Class_Range);
  rr = EINA_RBTREE_CONTAINER_GET(right, Eina_Class_Range);

  if (rl->start < rr->start) return EINA_RBTREE_LEFT;
  return EINA_RBTREE_RIGHT;
}

/* really destroying a range and handling that case is a complex
   problem to solve. Not handling it right now, exposing myself to
   DoS. */
static void
_eina_range_cleanup(Eina_Class_Range *range)
{
  range->current = 0;
  range->empty = NULL;
}

static void
_eina_object_constructor_call(Eina_Class *class, void *object)
{
  if (class->parent) _eina_object_constructor_call(class->parent, object);

  if (class->constructor) class->constructor(class, object, class->data);
}

static void
_eina_object_destructor_call(Eina_Class *class, void *object)
{
  if (class->destructor) class->destructor(class, object, class->data);

  if (class->parent) _eina_object_destructor_call(class->parent, object);
}

static Eina_Object*
_eina_object_get(Eina_Object_Item *item)
{
  Eina_Object_Generation gen;
  Eina_Object_ID id;

  if (!item) return NULL;

  gen = item->range->generation_array[item->index];

  id = ((Eina_Object_ID) gen << EINA_GEN_OFFSET) + item->range->start + item->index;

  return (Eina_Object *) id;
}

static Eina_Object_Item *
_eina_object_find_item(Eina_Class *class, Eina_Object *object)
{
  Eina_Class_Range *matched;
  Eina_Object_Item *item;
  Eina_Class *search;
  Eina_Rbtree *match;
  Eina_Object_Generation generation;
  Eina_Object_ID id;
  Eina_Object_ID idx;

  id = (Eina_Object_ID) object;
  idx = id & (((Eina_Object_ID) 1 << EINA_GEN_OFFSET) - 1);
  generation = id & !(((Eina_Object_ID) 1 << EINA_GEN_OFFSET) - 1);

  /* Try to find the ID */
  match = eina_rbtree_inline_lookup(class->top->range,
				    (void*) idx, sizeof (Eina_Object_ID),
				    _eina_rbtree_cmp_range, NULL);
  /* ID not found, invalid pointer ! */
  if (!match)
    {
      ERR("%p: ID ["EINA_ID_STR"] not found in class hiearchy of [%s].",
	  object, idx, class->name);
      return NULL;
    }
  matched = EINA_RBTREE_CONTAINER_GET(match, Eina_Class_Range);

  /* generation mismatch, invalid pointer ! */
  if (generation != matched->generation_array[idx - matched->start])
    {
      ERR("%p: generation mismatch [%i] vs [%i].",
	  object, generation, matched->generation_array[idx - matched->start]);
      return NULL;
    }

  /* does it belong to the right class ? */
  for (search = matched->type; search && search != class; search = search->parent)
    ;

  /* class match request or invalid pointer ! */
  if (search != class)
    {
      ERR("%p: from class [%s] is not in the hierarchie of [%s].",
	  object, matched->type->name, class->name);
      return NULL;
    }

  /* retrieve pointer */
  item = matched->pointer_array[id - matched->start];
  /* allocated or invalid pointer ? */
  if (!item)
    {
      ERR("%p: ID is not allocated.", object);
      return NULL;
    }

  /* be aware, that because we use eina_trash to store empty pointer after first use,
     pointer could be != NULL and still be empty. Need another pool of data somewhere
     to store the state of the allocation... Is it needed ?
   */

  return item;
}

static void
_eina_object_item_del(Eina_Object_Item *item)
{
  Eina_Class *class;

  class = item->range->type;
  _eina_object_destructor_call(class,
			       (unsigned char*) item + _eina_object_item_size);

  eina_trash_push(&item->range->empty, item->range->pointer_array + item->index);
  item->range->generation_array[item->index]++;
  item->range->empty_count++;

  if (item->range->empty_count == item->range->end - item->range->start)
    _eina_range_cleanup(item->range);
  item->range = NULL;

  if (item->parent)
    item->parent->link = eina_inlist_remove(item->parent->link, EINA_INLIST_GET(item));
  item->parent = NULL;

  while (item->link)
    {
      Eina_Object_Item *child;

      child = EINA_INLIST_CONTAINER_GET(item->link, Eina_Object_Item);
      _eina_object_item_del(child);
    }

  eina_mempool_free(class->mempool, item);
}

static Eina_Class_Range *
_eina_class_empty_range_get(Eina_Inlist *list)
{
  while (list)
    {
      Eina_Class_Range *range;

      range = EINA_INLIST_CONTAINER_GET(list, Eina_Class_Range);
      if (range->empty_count > 0)
	return range;

      list = list->next;
    }

  return NULL;
}

static Eina_Class_Range *
_eina_class_range_add(Eina_Class *class)
{
  Eina_Class_Range *range;
  unsigned char *tmp;
  Eina_Range *av = NULL;

  range = malloc(sizeof (Eina_Class_Range)
		 + sizeof (Eina_Object_Item*) * class->pool_size
		 + sizeof (Eina_Object_Generation) * (class->pool_size - 1));
  if (!range) return NULL;

  tmp = (void*) (range + 1);
  tmp += sizeof (Eina_Object_Generation) * (class->pool_size - 1);

  range->pointer_array = (Eina_Object_Item**) tmp;

  /* no need to fix generation to a specific value as random value should be just fine */
  memset(range->pointer_array, 0, sizeof (Eina_Object_Item*) * class->pool_size);

  range->current = 0;
  range->type = class;
  range->empty = NULL;

  /* and now find an empty block */
  EINA_INLIST_FOREACH(class->top->available, av)
    if (av->end - av->start == class->pool_size)
      break;

  if (av)
    {
      range->start = av->start;
      range->end = av->end;

      class->top->available = eina_inlist_remove(class->top->available,
						 EINA_INLIST_GET(av));
      eina_mempool_free(_eina_range_mp, av);
    }
  else
    {
      range->start = class->top->upper_limit;
      range->end = range->start + class->pool_size;
      class->top->upper_limit = range->end;
    }

  return range;
}

static void
_eina_class_range_del(Eina_Class_Range *range)
{
  Eina_Class_Top *top;
  Eina_Range *keep;

  top = range->type->top;

  keep = eina_mempool_malloc(_eina_range_mp, sizeof (Eina_Range));
  if (!keep)
    {
      ERR("Not enougth memory to keep track of allocated range.");
      goto delete_class_range;
    }

  keep->start = range->start;
  keep->end = range->end;

  top->available = eina_inlist_prepend(top->available,
				       EINA_INLIST_GET(keep));

 delete_class_range:
  top->range = eina_rbtree_inline_remove(top->range, EINA_RBTREE_GET(range),
					 _eina_class_direction_range, NULL);
  range->type->allocated_range = eina_inlist_remove(range->type->allocated_range,
						    EINA_INLIST_GET(range));
  free(range);
}

static void
_eina_class_range_repack(void *dst, void *src, void *data)
{
  Eina_Object_Item *di = dst;
  Eina_Object_Item *si = src;
  Eina_Class *class = data;

  (void) class;

  si->range->pointer_array[si->index] = di;

  /* FIXME: We could just lock the right Eina_Class_Range 
     here instead of locking all the Class */
}

/**
 * @endcond
 */

Eina_Bool
eina_object_init(void)
{
  _eina_object_log_dom = eina_log_domain_register("eina_object",
						  EINA_LOG_COLOR_DEFAULT);
  if (_eina_object_log_dom < 0)
    {
      EINA_LOG_ERR("Could not register log domain: eina_list");
      return EINA_FALSE;
    }

  _eina_class_mp = eina_mempool_add("chained_mempool", "class", 
				    NULL, sizeof (Eina_Class), 64);
  if (!_eina_class_mp)
    {
      ERR("ERROR: Mempool for Eina_Class cannot be allocated in object init.");
      goto on_init_fail;
    }

  _eina_top_mp = eina_mempool_add("chained_mempool", "top",
				  NULL, sizeof (Eina_Class_Top), 64);
  if (!_eina_top_mp)
    {
      ERR("ERROR: Mempool for Eina_Class_Top cannot be allocated in object init.");
      goto on_init_fail;
    }

  _eina_range_mp = eina_mempool_add("chained_mempool", "range",
				    NULL, sizeof (Eina_Range), 64);
  if (!_eina_range_mp)
    {
      ERR("ERROR: Mempool for Eina_Class_Top cannot be allocated in object init.");
      goto on_init_fail;
    }

#define EMS(n) eina_magic_string_static_set(n, n ## _STR)
  EMS(EINA_MAGIC_CLASS);
#undef EMS

  _eina_object_item_size = eina_mempool_alignof(sizeof (Eina_Object_Item));

  return EINA_TRUE;

 on_init_fail:
  eina_log_domain_unregister(_eina_object_log_dom);
  _eina_object_log_dom = -1;

  if (_eina_top_mp)
    {
      eina_mempool_del(_eina_top_mp);
      _eina_top_mp = NULL;
    }

  if (_eina_class_mp)
    {
      eina_mempool_del(_eina_class_mp);
      _eina_class_mp = NULL;
    }

  return EINA_FALSE;
}

Eina_Bool
eina_object_shutdown(void)
{
  eina_mempool_del(_eina_class_mp);
  eina_mempool_del(_eina_top_mp);

  eina_log_domain_unregister(_eina_object_log_dom);
  _eina_object_log_dom = -1;
  return EINA_TRUE;
}

Eina_Class *
eina_class_new(const char *name,
	       unsigned int class_size,
	       unsigned int pool_size,
	       Eina_Class_Callback constructor,
	       Eina_Class_Callback destructor,
	       Eina_Class *parent,
	       void *data)
{
  Eina_Class *c;
  unsigned int object_size = class_size;

  if (parent) EINA_MAGIC_CHECK_CLASS(parent, NULL);

  c = eina_mempool_malloc(_eina_class_mp, sizeof (Eina_Class));
  if (!c) return NULL;

  c->parent = parent;
  if (parent)
    {
      parent->childs = eina_inlist_append(parent->childs,
					  EINA_INLIST_GET(c));
      c->top = parent->top;
    }
  else
    {
      c->top = eina_mempool_malloc(_eina_top_mp, sizeof (Eina_Class_Top));

      c->top->top_parent = c;
      c->top->range = NULL;
      c->top->available = NULL;
      c->top->upper_limit = 0;
    }

  /* Build complete object size and find top parent */
  if (parent) object_size += parent->object_size;

  c->name = eina_stringshare_add(name);
  c->class_size = class_size;
  c->pool_size = pool_size;
  c->object_size = object_size;
  c->mempool = eina_mempool_add("chained_mempool", "range",
				NULL, 
				_eina_object_item_size + object_size, pool_size); 

  c->constructor = constructor;
  c->destructor = destructor;
  c->data = data;

  c->allocated_range = NULL;
  c->childs = NULL;

#ifdef EINA_HAVE_DEBUG_THREADS
  c->self = pthread_self();
#endif
  eina_lock_new(&c->mutex);

  EINA_MAGIC_SET(c, EINA_MAGIC_CLASS);

  return c;
}

const char *
eina_class_name_get(Eina_Class *class)
{
  EINA_MAGIC_CHECK_CLASS(class, NULL);

  return class->name;  
}

unsigned int
eina_class_size_get(Eina_Class *class)
{
  EINA_MAGIC_CHECK_CLASS(class, 0);

  return class->class_size;
}

unsigned int
eina_class_object_size_get(Eina_Class *class)
{
  EINA_MAGIC_CHECK_CLASS(class, 0);

  return class->object_size;
}

void
eina_class_del(Eina_Class *class)
{
  EINA_MAGIC_CHECK_CLASS(class);

  EINA_MAGIC_SET(class, 0);

  while (class->allocated_range)
    _eina_class_range_del(EINA_INLIST_CONTAINER_GET(class->allocated_range,
						    Eina_Class_Range));

  while (class->childs)
    {
      Eina_Class *child;

      child = EINA_INLIST_CONTAINER_GET(class->childs, Eina_Class);
      eina_class_del(child);
    }

  if (class->parent)
    {
      class->parent->childs = eina_inlist_remove(class->parent->childs,
						 EINA_INLIST_GET(class));
    }
  else
    {
      while (class->top->available)
	{
	  Eina_Range *range;

	  range = EINA_INLIST_CONTAINER_GET(class->top->available, Eina_Range);
	  class->top->available = eina_inlist_remove(class->top->available,
						     class->top->available);
	  eina_mempool_free(_eina_range_mp, range);
	}
    }

#ifdef EINA_HAVE_DEBUG_THREADS
  assert(pthread_equal(class->self, pthread_self()));
#endif
  eina_lock_free(&class->mutex);

  eina_mempool_del(class->mempool);
  eina_mempool_free(_eina_class_mp, class);
}

void
eina_class_repack(Eina_Class *class)
{
  Eina_Class *child;

  EINA_MAGIC_CHECK_CLASS(class);

  if (!eina_lock_take(&class->mutex))
    {
#ifdef EINA_HAVE_DEBUG_THREADS
      assert(pthread_equal(class->self, pthread_self()));
#endif
    }

  eina_mempool_repack(class->mempool, _eina_class_range_repack, class);

  eina_lock_release(&class->mutex);

  EINA_INLIST_FOREACH(class->childs, child)
    eina_class_repack(child);
}

Eina_Object *
eina_object_add(Eina_Class *class)
{
  Eina_Class_Range *range;
  Eina_Object_Item **object;
  int localid;

  EINA_MAGIC_CHECK_CLASS(class, NULL);

  /* No need to lock the class as we don't access/modify the pointer inside
   * this function. */

  range = _eina_class_empty_range_get(class->allocated_range);
  if (!range) range = _eina_class_range_add(class);

  if (range->empty_count == 0)
    {
      ERR("The impossible happen, range is empty when it should not !");
      return NULL;
    }

  range->empty_count--;

  object = eina_trash_pop(&range->empty);
  if (!object) object = range->pointer_array + range->current++;

  localid = object - range->pointer_array;

  *object = eina_mempool_malloc(class->mempool, 
				class->object_size + _eina_object_item_size);

  (*object)->index = localid;
  (*object)->link = NULL;
  (*object)->range = range;

  _eina_object_constructor_call(class,
				((unsigned char*)(*object)) + _eina_object_item_size);

  if (!(++range->generation_array[localid]))
    ++range->generation_array[localid];

  return _eina_object_get(*object);
}

void *
eina_object_pointer_get(Eina_Class *class,
			Eina_Object *object)
{
  Eina_Object_Item *item;
  unsigned char *mem = NULL;

  if (!object) return NULL;
  EINA_MAGIC_CHECK_CLASS(class, NULL);

  if (!eina_lock_take(&class->mutex))
    {
#ifdef EINA_HAVE_DEBUG_THREADS
      assert(pthread_equal(class->self, pthread_self()));
#endif
    }

  item = _eina_object_find_item(class, object);
  if (!item) goto on_error;

  mem = (unsigned char*) item + _eina_object_item_size;

 on_error:
  eina_lock_release(&class->mutex);

  return mem;
}

void
eina_object_del(Eina_Class *class,
		Eina_Object *object)
{
  Eina_Object_Item *item;

  if (!object) return ;
  EINA_MAGIC_CHECK_CLASS(class);

  if (!eina_lock_take(&class->mutex))
    {
#ifdef EINA_HAVE_DEBUG_THREADS
      assert(pthread_equal(class->self, pthread_self()));
#endif
    }

  item = _eina_object_find_item(class, object);
  if (!item) goto on_error;

  _eina_object_item_del(item);

 on_error:
  eina_lock_release(&class->mutex);
}

Eina_Bool
eina_object_parent_set(Eina_Class *parent_class, Eina_Object *parent,
		       Eina_Class *object_class, Eina_Object *object)
{
  Eina_Object_Item *parent_item;
  Eina_Object_Item *object_item;

  if (!parent) return EINA_FALSE;
  if (!object) return EINA_FALSE;
  EINA_MAGIC_CHECK_CLASS(parent_class, EINA_FALSE);
  EINA_MAGIC_CHECK_CLASS(object_class, EINA_FALSE);

  if (!eina_lock_take(&parent_class->mutex))
    {
#ifdef EINA_HAVE_DEBUG_THREADS
      assert(pthread_equal(parent_class->self, pthread_self()));
#endif
    }

  if (!eina_lock_take(&object_class->mutex))
    {
#ifdef EINA_HAVE_DEBUG_THREADS
      assert(pthread_equal(object_class->self, pthread_self()));
#endif
    }

  parent_item = _eina_object_find_item(parent_class, parent);
  if (!parent_item) return EINA_FALSE;

  object_item = _eina_object_find_item(object_class, object);
  if (!object_item) return EINA_FALSE;

  if (object_item->parent)
    object_item->parent->link = eina_inlist_remove(object_item->parent->link,
						   EINA_INLIST_GET(object_item));

  object_item->parent = parent_item;
  parent_item->link = eina_inlist_append(parent_item->link,
					 EINA_INLIST_GET(object_item));

  eina_lock_release(&parent_class->mutex);
  eina_lock_release(&object_class->mutex);

  return EINA_TRUE;
}

Eina_Object *
eina_object_parent_get(Eina_Class *class, Eina_Object *object)
{
  Eina_Object_Item *object_item;
  Eina_Object *or = NULL;

  if (!object) return EINA_FALSE;
  EINA_MAGIC_CHECK_CLASS(class, EINA_FALSE);

  if (!eina_lock_take(&class->mutex))
    {
#ifdef EINA_HAVE_DEBUG_THREADS
      assert(pthread_equal(class->self, pthread_self()));
#endif
    }

  object_item = _eina_object_find_item(class, object);
  if (object_item)
    or = _eina_object_get(object_item->parent);

  eina_lock_release(&class->mutex);

  return or;
}

