/* EINA - EFL data type library
 * Copyright (C) 2013 Cedric Bail
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

#include "eina_config.h"
#include "eina_mempool.h"
#include "eina_types.h"
#include "eina_safety_checks.h"
#include "eina_list.h"
#include "eina_hash.h"

#include "eina_cow.h"

#define EINA_COW_MAGIC 0xDEADBEEF

typedef struct _Eina_Cow_Ptr Eina_Cow_Ptr;
typedef struct _Eina_Cow_GC Eina_Cow_GC;

struct _Eina_Cow_Ptr
{
   int refcount;

   Eina_Bool hashed;
   Eina_Bool togc;
};

struct _Eina_Cow_GC
{
   Eina_List *togc;
   Eina_Cow_Ptr *ref;
   const void * const *dst;
};

struct _Eina_Cow
{
   EINA_MAGIC;

   Eina_List *togc;
   Eina_Hash *match;

   Eina_Mempool *pool;
   const void *default_value;

   unsigned int struct_size;
};

typedef int (*Eina_Cow_Hash)(const void *, int);

#define EINA_COW_MAGIC_CHECK(d)                         \
  do {                                                  \
     if (!EINA_MAGIC_CHECK((d), EINA_COW_MAGIC))        \
       EINA_MAGIC_FAIL((d), EINA_COW_MAGIC);            \
  } while (0);

static Eina_Mempool *gc_pool = NULL;
  
static inline int
_eina_cow_hash_gen(const void *key, int key_length,
                   Eina_Cow_Hash hash,
                   int size)
{
   const unsigned char *walk = key;
   int r = 0xDEADBEEF;

   while (key_length > 0)
     {
        r ^= hash(walk, size);

        walk += size;
        key_length -= size;
     }

   return r;
}

#ifdef __LP64__
static int
_eina_cow_hash64(const void *key, int key_length)
{
   return _eina_cow_hash_gen(key, key_length,
                             (Eina_Cow_Hash) eina_hash_int64, sizeof (unsigned long int));
}
#else
static int
_eina_cow_hash32(const void *key, int key_length)
{
   return _eina_cow_hash_gen(key, key_length,
                             (Eina_Cow_Hash) eina_hash_int32, sizeof (int));
}
#endif

static int current_cow_size = 0;

static unsigned int
_eina_cow_length(const void *key EINA_UNUSED)
{
   /* nasty hack, has only gc need to access the hash, he will be in charge
      of that global. access to the hash should be considered global. so a
      lock will be needed to make multiple gc run at the same safely.
    */
   return current_cow_size;
}

static int
_eina_cow_cmp(const void *key1, int key1_length,
              const void *key2, int key2_length EINA_UNUSED)
{
   return memcmp(key1, key2, key1_length);
}

static void
_eina_cow_hash_del(Eina_Cow *cow,
                   const void *data,
                   Eina_Cow_Ptr *ref)
{
   /* if eina_cow_gc is supposed to be thread safe, lock the cow here */
   if (ref->hashed)
     {
        current_cow_size = cow->struct_size;
        eina_hash_del(cow->match, data, ref);
        ref->hashed = EINA_FALSE;
     }
}

static void
_eina_cow_togc_del(Eina_Cow *cow, Eina_Cow_Ptr *ref)
{
   /* if eina_cow_gc is supposed to be thread safe, lock the cow here */
   if (ref->togc)
     {
        Eina_Cow_GC *gc;
        Eina_List *l;

        EINA_LIST_FOREACH(cow->togc, l, gc)
          if (gc->ref == ref)
            {
               cow->togc = eina_list_remove_list(cow->togc, l);
               break;
            }
        ref->togc = EINA_FALSE;
     }
}

Eina_Bool
eina_cow_init(void)
{
   const char *choice, *tmp;

#ifdef EINA_DEFAULT_MEMPOOL
   choice = "pass_through";
#else
   choice = "chained_mempool";
#endif
   tmp = getenv("EINA_MEMPOOL");
   if (tmp && tmp[0])
     choice = tmp;

   gc_pool = eina_mempool_add(choice, "gc", NULL, sizeof (Eina_Cow_GC), 32);
   if (!gc_pool)
     {
        /* ERR("ERROR: Mempool for cow '%s' cannot be allocated in eina_cow_new.", name); */
        return EINA_FALSE;
     }
   return EINA_TRUE;
}

Eina_Bool
eina_cow_shutdown(void)
{
   eina_mempool_del(gc_pool);
   return EINA_TRUE;
}

EAPI Eina_Cow *
eina_cow_add(const char *name, unsigned int struct_size, unsigned int step, const void *default_value)
{
   const char *choice, *tmp;
   Eina_Cow *cow;

   EINA_SAFETY_ON_NULL_RETURN_VAL(default_value, NULL);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(struct_size, NULL);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(step, NULL);

   cow = malloc(sizeof (Eina_Cow));
   if (!cow) return NULL;

#ifdef EINA_DEFAULT_MEMPOOL
   choice = "pass_through";
#else
   choice = "chained_mempool";
#endif
   tmp = getenv("EINA_MEMPOOL");
   if (tmp && tmp[0])
     choice = tmp;

   cow->pool = eina_mempool_add(choice, name,
                                NULL,
                                struct_size + sizeof (Eina_Cow_Ptr), step);
   if (!cow->pool)
     {
        /* ERR("ERROR: Mempool for cow '%s' cannot be allocated in eina_cow_new.", name); */
        goto on_error;
     }

#ifdef __LP64__
   cow->match = eina_hash_new(_eina_cow_length,
                              _eina_cow_cmp,
                              _eina_cow_hash64,
                              NULL,
                              6);
#else
   cow->match = eina_hash_new(_eina_cow_length,
                              _eina_cow_cmp,
                              _eina_cow_hash32,
                              NULL,
                              6);   
#endif

   cow->togc = NULL;
   cow->default_value = default_value;
   cow->struct_size = struct_size;

   EINA_MAGIC_SET(cow, EINA_COW_MAGIC);

   return cow;

 on_error:
   free(cow);
   return NULL;
}

EAPI void
eina_cow_del(Eina_Cow *cow)
{
   Eina_Cow_GC *gc;

   EINA_COW_MAGIC_CHECK(cow);

   eina_mempool_del(cow->pool);

   EINA_LIST_FREE(cow->togc, gc)
     eina_mempool_free(gc_pool, gc);
   free(cow);
}

EAPI const void *
eina_cow_alloc(Eina_Cow *cow)
{
   EINA_COW_MAGIC_CHECK(cow);

   return cow->default_value;
}

EAPI void
eina_cow_free(Eina_Cow *cow, const void *data)
{
   Eina_Cow_Ptr *ref;

   EINA_COW_MAGIC_CHECK(cow);

   if (!data) return ;
   if (cow->default_value == data) return ;

   ref = (Eina_Cow_Ptr*)(((unsigned char*) data) + cow->struct_size);
   ref->refcount--;

   if (ref->refcount > 0) return ;

   _eina_cow_hash_del(cow, data, ref);
   _eina_cow_togc_del(cow, ref);
   eina_mempool_free(cow->pool, (void*) data);
}

EAPI void *
eina_cow_write(Eina_Cow *cow, const void * const *data)
{
   Eina_Cow_Ptr *ref;
   const void *src;
   void *r;

   EINA_COW_MAGIC_CHECK(cow);

   if (!*data) return NULL; /* cow pointer is always != NULL */
   if (*data == cow->default_value)
     {
        src = cow->default_value;
        goto allocate;
     }

   ref = (Eina_Cow_Ptr*)(((unsigned char*) *data) + cow->struct_size);

   if (ref->refcount == 1)
     {
        _eina_cow_hash_del(cow, *data, ref);
        return (void *) *data;
     }

   src = *data;

 allocate:
   r = eina_mempool_malloc(cow->pool,
                           cow->struct_size + sizeof (Eina_Cow_Ptr));
   memcpy(r, src, cow->struct_size);

   ref = (Eina_Cow_Ptr*)(((unsigned char*) r) + cow->struct_size);
   ref->refcount = 1;
   ref->hashed = EINA_FALSE;
   ref->togc = EINA_FALSE;

   *((void**) data) = r;

   return r;
}

EAPI void
eina_cow_commit(Eina_Cow *cow, const void * const * dst, const void *data)
{
   Eina_Cow_Ptr *ref;

   EINA_COW_MAGIC_CHECK(cow);

   ref = (Eina_Cow_Ptr*)(((unsigned char*) data) + cow->struct_size);

   /* needed if we want to make cow gc safe */
   if (!ref->togc)
     {
        Eina_Cow_GC *gc;

        gc = eina_mempool_malloc(gc_pool, sizeof (Eina_Cow_GC));
        if (!gc) return ; /* That one will not get gced this time */

        gc->ref = ref;
        gc->dst = dst;
        cow->togc = gc->togc = eina_list_prepend(cow->togc, gc);
        ref->togc = EINA_TRUE;
     }
}

EAPI void
eina_cow_memcpy(Eina_Cow *cow, const void * const *dst, const void *src)
{
   Eina_Cow_Ptr *ref;

   EINA_COW_MAGIC_CHECK(cow);

   eina_cow_free(cow, *dst);

   ref = (Eina_Cow_Ptr*)(((unsigned char*) src) + cow->struct_size);
   ref->refcount++;

   *((const void**)dst) = src;
}

EAPI Eina_Bool
eina_cow_gc(Eina_Cow *cow)
{
   Eina_Cow_Ptr *ref;
   Eina_Cow_GC *gc;
   void *data;
   void *match;

   EINA_COW_MAGIC_CHECK(cow);

   if (!cow->togc) return EINA_FALSE; /* Nothing more to do */

   /* Do handle hash and all funky merge think here */
   gc = eina_list_data_get(eina_list_last(cow->togc));

   data = ((unsigned char*) gc->ref) - cow->struct_size;

   gc->ref->togc = EINA_FALSE;
   cow->togc = eina_list_remove_list(cow->togc, eina_list_last(cow->togc));

   current_cow_size = cow->struct_size;
   match = eina_hash_find(cow->match, data);
   if (match)
     {
        eina_cow_free(cow, data);

        ref = (Eina_Cow_Ptr*)(((unsigned char*) match) + cow->struct_size);
        *((void**)gc->dst) = match;
        ref->refcount++;
     }
   else
     {
        eina_hash_direct_add(cow->match, data, data);
        gc->ref->hashed = EINA_TRUE;
     }

   eina_mempool_free(gc_pool, gc);

   return EINA_TRUE;
}

