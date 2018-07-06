/* Eina - EFL data type library
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

#ifdef HAVE_BACKTRACE
# include <execinfo.h>
#endif

#ifndef NVALGRIND
# include <memcheck.h>
#endif

#include "eina_config.h"
#include "eina_private.h"
#include "eina_log.h"
#include "eina_mempool.h"
#include "eina_types.h"
#include "eina_safety_checks.h"
#include "eina_list.h"
#include "eina_hash.h"

#include "eina_cow.h"

#ifdef EINA_COW_MAGIC_ON
#define EINA_COW_MAGIC 0xDEADBEEF

# define EINA_COW_PTR_MAGIC 0xBEEFE00
#endif

typedef struct _Eina_Cow_Ptr Eina_Cow_Ptr;
typedef struct _Eina_Cow_GC Eina_Cow_GC;

#ifdef HAVE_BACKTRACE
#define EINA_DEBUG_BT_NUM 64
typedef void (*Eina_Bt_Func) ();
#endif

struct _Eina_Cow_Ptr
{
#ifdef EINA_COW_MAGIC_ON
   EINA_MAGIC;
# ifdef HAVE_BACKTRACE
   Eina_Bt_Func writer_bt[EINA_DEBUG_BT_NUM];
   int  writer_bt_num;
# endif
#endif
   int refcount;

   Eina_Bool hashed : 1;
   Eina_Bool togc : 1;
#ifdef EINA_COW_MAGIC_ON
   Eina_Bool writing : 1;
#endif
};

struct _Eina_Cow_GC
{
#ifdef EINA_COW_MAGIC_ON
   EINA_MAGIC;
#endif

   Eina_Cow_Ptr *ref;
   const void **dst;
};
struct _Eina_Cow
{
#ifdef EINA_COW_MAGIC_ON
   EINA_MAGIC;
#endif

   Eina_Hash *togc;
   Eina_Hash *match;

   Eina_Mempool *pool;
   const Eina_Cow_Data *default_value;

   unsigned int struct_size;
   unsigned int total_size;
};

typedef int (*Eina_Cow_Hash)(const void *, int);

#ifdef EINA_COW_MAGIC_ON
# define EINA_COW_MAGIC_CHECK(d)                        \
  do {                                                  \
     if (!EINA_MAGIC_CHECK((d), EINA_COW_MAGIC))        \
       EINA_MAGIC_FAIL((d), EINA_COW_MAGIC);            \
  } while (0);

# define EINA_COW_PTR_MAGIC_CHECK(d)                    \
  do {                                                  \
     if (!EINA_MAGIC_CHECK((d), EINA_COW_PTR_MAGIC))    \
       EINA_MAGIC_FAIL((d), EINA_COW_PTR_MAGIC);        \
  } while (0);
#else
# define EINA_COW_MAGIC_CHECK(d)
# define EINA_COW_PTR_MAGIC_CHECK(d)
#endif

#define EINA_COW_PTR_SIZE                       \
  eina_mempool_alignof(sizeof (Eina_Cow_Ptr))

#define EINA_COW_PTR_GET(d)				\
  (((Eina_Cow_Ptr *)d) - 1)

#define EINA_COW_DATA_GET(d)                    \
  (((Eina_Cow_Ptr *)d) + 1)

static int _eina_cow_log_dom = -1;

#ifdef ERR
#undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_eina_cow_log_dom, __VA_ARGS__)

#ifdef INF
#undef INF
#endif
#define INF(...) EINA_LOG_DOM_INFO(_eina_cow_log_dom, __VA_ARGS__)

#ifdef DBG
#undef DBG
#endif
#define DBG(...) EINA_LOG_DOM_DBG(_eina_cow_log_dom, __VA_ARGS__)


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

#ifdef EFL64
static int
_eina_cow_hash64(const void *key, int key_length)
{
   return _eina_cow_hash_gen(key, key_length,
                             (Eina_Cow_Hash) eina_hash_int64, sizeof (unsigned long long int));
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
   /* nasty hack, since only gc needs to access the hash, it will be in charge
      of that global. access to the hash should be considered global.
    */
   return current_cow_size;
}

static int
_eina_cow_cmp(const void *key1, int key1_length,
              const void *key2, int key2_length EINA_UNUSED)
{
   return memcmp(key1, key2, key1_length);
}

static inline void
_eina_cow_hash_del(Eina_Cow *cow,
                   const void *data,
                   Eina_Cow_Ptr *ref)
{
   /* eina_cow_gc is not supposed to be thread safe */
   if (!ref->hashed) return;

   current_cow_size = cow->struct_size;
   eina_hash_del(cow->match, data, data);
   ref->hashed = EINA_FALSE;
}

static void
_eina_cow_gc_free(void *data)
{
   eina_mempool_free(gc_pool, data);
}

static inline void
_eina_cow_togc_del(Eina_Cow *cow, Eina_Cow_Ptr *ref)
{
   /* eina_cow_gc is not supposed to be thread safe */
   if (!ref->togc) return;
   eina_hash_del(cow->togc, &ref, NULL);
   ref->togc = EINA_FALSE;
}

static void
_eina_cow_togc_add(Eina_Cow *cow,
                   Eina_Cow_Ptr *ref,
                   const Eina_Cow_Data ** dst)
{
   Eina_Cow_GC *gc;

   /* needed if we want to make cow gc safe */
   if (ref->togc) return;
#ifndef NVALGRIND
   VALGRIND_MAKE_MEM_NOACCESS(ref, sizeof (*ref));
#endif

   gc = eina_mempool_malloc(gc_pool, sizeof (Eina_Cow_GC));
   if (!gc) return; /* That one will not get gced this time */

   gc->ref = ref;
   gc->dst = dst;
   eina_hash_direct_add(cow->togc, &gc->ref, gc);
#ifndef NVALGRIND
   VALGRIND_MAKE_MEM_DEFINED(ref, sizeof (*ref));
#endif
   ref->togc = EINA_TRUE;
#ifndef NVALGRIND
   VALGRIND_MAKE_MEM_NOACCESS(ref, sizeof (*ref));
#endif
}

static void
_eina_cow_gc(Eina_Cow *cow, Eina_Cow_GC *gc)
{
   Eina_Cow_Data *data;
   Eina_Cow_Data *match;

   data = EINA_COW_DATA_GET(gc->ref);

   current_cow_size = cow->struct_size;
   match = eina_hash_find(cow->match, data);
   if (match)
     {
        Eina_Cow_Ptr *ref = EINA_COW_PTR_GET(match);
#ifndef NVALGRIND
        VALGRIND_MAKE_MEM_DEFINED(ref, sizeof (*ref));
#endif
        ref->refcount += gc->ref->refcount;

        *gc->dst = match;
        eina_cow_free(cow, (const Eina_Cow_Data**) &data);

#ifndef NVALGRIND
        VALGRIND_MAKE_MEM_NOACCESS(ref, sizeof (*ref));
#endif
     }
   else
     {
        eina_hash_direct_add(cow->match, data, data);
        gc->ref->hashed = EINA_TRUE;
        gc->ref->togc = EINA_FALSE;
        eina_hash_del(cow->togc, &gc->ref, gc);
     }
}

Eina_Bool
eina_cow_init(void)
{
   const char *choice, *tmp;

   _eina_cow_log_dom = eina_log_domain_register("eina_cow", EINA_LOG_COLOR_DEFAULT);
   if (_eina_cow_log_dom < 0)
     {
        EINA_LOG_ERR("Could not register log domain: eina_cow");
        return EINA_FALSE;
     }

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
        ERR("Mempool for cow gc cannot be allocated.");
        return EINA_FALSE;
     }
   return EINA_TRUE;
}

Eina_Bool
eina_cow_shutdown(void)
{
   eina_log_domain_unregister(_eina_cow_log_dom);
   eina_mempool_del(gc_pool);
   return EINA_TRUE;
}

EAPI Eina_Cow *
eina_cow_add(const char *name, unsigned int struct_size, unsigned int step, const void *default_value, Eina_Bool gc)
{
   const char *choice, *tmp;
   Eina_Cow *cow;
   unsigned int total_size;

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

   INF("Creating Cow '%s' with mempool of type '%s'", name, choice);
   total_size = eina_mempool_alignof(struct_size + EINA_COW_PTR_SIZE);
   cow->pool = eina_mempool_add(choice, name, NULL, total_size, step);
   if (!cow->pool)
     {
        ERR("Mempool for cow '%s' cannot be allocated.", name);
        goto on_error;
     }

#ifdef EFL64
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
   if (gc)
     cow->togc = eina_hash_pointer_new(_eina_cow_gc_free);
   else
     cow->togc = NULL;
   cow->default_value = default_value;
   cow->struct_size = struct_size;
   cow->total_size = total_size;

#ifdef EINA_COW_MAGIC_ON
   EINA_MAGIC_SET(cow, EINA_COW_MAGIC);
#endif

   return cow;

 on_error:
   free(cow);
   return NULL;
}

EAPI void
eina_cow_del(Eina_Cow *cow)
{
   if (!cow) return;

#ifdef EINA_COW_MAGIC_ON
   EINA_COW_MAGIC_CHECK(cow);
#endif

   eina_mempool_del(cow->pool);
   eina_hash_free(cow->match);
   if (cow->togc) eina_hash_free(cow->togc);
   free(cow);
}

EAPI const Eina_Cow_Data *
eina_cow_alloc(Eina_Cow *cow)
{
#ifdef EINA_COW_MAGIC_ON
   EINA_COW_MAGIC_CHECK(cow);
#endif

   return cow->default_value;
}

EAPI void
eina_cow_free(Eina_Cow *cow, const Eina_Cow_Data **data)
{
   Eina_Cow_Ptr *ref;

#ifdef EINA_COW_MAGIC_ON
   EINA_COW_MAGIC_CHECK(cow);
#endif

   if (!data || !*data) return;
   if (cow->default_value == *data) return;

   ref = EINA_COW_PTR_GET(*data);
#ifndef NVALGRIND
   VALGRIND_MAKE_MEM_DEFINED(ref, sizeof (*ref));
#endif
   ref->refcount--;

   if (ref->refcount == 0) _eina_cow_hash_del(cow, *data, ref);
   *data = (Eina_Cow_Data*) cow->default_value;

   if (ref->refcount > 0)
     {
#ifndef NVALGRIND
       VALGRIND_MAKE_MEM_NOACCESS(ref, sizeof (*ref));
#endif
       return;
     }

#ifdef EINA_COW_MAGIC_ON
   EINA_MAGIC_SET(ref, EINA_MAGIC_NONE);
#endif
   _eina_cow_togc_del(cow, ref);
   eina_mempool_free(cow->pool, (void*) ref);
}

EAPI void *
eina_cow_write(Eina_Cow *cow,
	       const Eina_Cow_Data * const *data)
{
   Eina_Cow_Ptr *ref;
   Eina_Cow_Data *r;

#ifdef EINA_COW_MAGIC_ON
   EINA_COW_MAGIC_CHECK(cow);
#endif

   if (!*data) return NULL; /* cow pointer is always != NULL */
   if (*data == cow->default_value)
     goto allocate;

   ref = EINA_COW_PTR_GET(*data);

#ifndef NVALGRIND
   VALGRIND_MAKE_MEM_DEFINED(ref, sizeof (*ref));
#endif
   if (ref->refcount == 1)
     {
#ifdef EINA_COW_MAGIC_ON
        EINA_COW_PTR_MAGIC_CHECK(ref);

        if (ref->writing)
          {
             ERR("Request writing on an pointer that is already in a writing process %p\n", data);
#ifdef HAVE_BACKTRACE
             backtrace_symbols_fd((void **) ref->writer_bt,
                                  ref->writer_bt_num, 1);
#endif
             return NULL;
          }
#endif

        if (cow->togc)
          _eina_cow_hash_del(cow, *data, ref);

#ifndef NVALGRIND
        VALGRIND_MAKE_MEM_NOACCESS(ref, sizeof (*ref));
#endif
        goto end;
     }
   ref->refcount--;

 allocate:
   ref = eina_mempool_malloc(cow->pool, cow->total_size);
   ref->refcount = 1;
   ref->hashed = EINA_FALSE;
   ref->togc = EINA_FALSE;
#ifdef EINA_COW_MAGIC_ON
   EINA_MAGIC_SET(ref, EINA_COW_PTR_MAGIC);
#endif

#ifndef NVALGRIND
   VALGRIND_MAKE_MEM_NOACCESS(ref, sizeof (*ref));
#endif

   r = EINA_COW_DATA_GET(ref);
   memcpy(r, *data, cow->struct_size);
   *((Eina_Cow_Data**) data) = r;

 end:
#ifndef NVALGRIND
   VALGRIND_MAKE_MEM_DEFINED(ref, sizeof (*ref));
#endif
#ifdef EINA_COW_MAGIC_ON
# ifdef HAVE_BACKTRACE
   ref->writer_bt_num = backtrace((void **)(ref->writer_bt),
                                  EINA_DEBUG_BT_NUM);
# endif
   ref->writing = EINA_TRUE;
#endif
#ifndef NVALGRIND
   VALGRIND_MAKE_MEM_NOACCESS(ref, sizeof (*ref));
#endif

   return (void *) *data;
}

EAPI void
eina_cow_done(Eina_Cow *cow,
	      const Eina_Cow_Data * const * dst,
	      const void *data,
              Eina_Bool needed_gc)
{
   Eina_Cow_Ptr *ref;

   EINA_COW_MAGIC_CHECK(cow);

   ref = EINA_COW_PTR_GET(data);
#ifndef NVALGRIND
   VALGRIND_MAKE_MEM_DEFINED(ref, sizeof (*ref));
#endif
   EINA_COW_PTR_MAGIC_CHECK(ref);
#ifdef EINA_COW_MAGIC_ON
   if (!ref->writing)
     ERR("Pointer %p is not in a writable state !", dst);

   ref->writing = EINA_FALSE;
#endif
#ifndef NVALGRIND
   VALGRIND_MAKE_MEM_NOACCESS(ref, sizeof (*ref));
#endif

   if (!cow->togc || !needed_gc) return;

#ifndef NVALGRIND
   VALGRIND_MAKE_MEM_DEFINED(ref, sizeof (*ref));
#endif

   _eina_cow_togc_add(cow, ref, (const Eina_Cow_Data **) dst);
}

EAPI void
eina_cow_memcpy(Eina_Cow *cow,
		const Eina_Cow_Data * const *dst,
		const Eina_Cow_Data *src)
{
   Eina_Cow_Ptr *ref;

   EINA_COW_MAGIC_CHECK(cow);

   if (*dst == src) return;

   if (src != cow->default_value)
     {
       ref = EINA_COW_PTR_GET(src);
#ifndef NVALGRIND
       VALGRIND_MAKE_MEM_DEFINED(ref, sizeof (*ref));
#endif

       EINA_COW_PTR_MAGIC_CHECK(ref);
       ref->refcount++;

       if (cow->togc)
         _eina_cow_togc_del(cow, ref);

#ifndef NVALGRIND
       VALGRIND_MAKE_MEM_NOACCESS(ref, sizeof (*ref));
#endif
     }

   eina_cow_free(cow, (const Eina_Cow_Data**) dst);

   *((const void**)dst) = src;
}

EAPI Eina_Bool
eina_cow_gc(Eina_Cow *cow)
{
   Eina_Cow_GC *gc;
   Eina_Iterator *it;
   Eina_Bool r;
#ifndef NVALGRIND
   Eina_Cow_Ptr *ref;
#endif

   EINA_COW_MAGIC_CHECK(cow);

   if (!cow->togc || !eina_hash_population(cow->togc))
     return EINA_FALSE;

   it = eina_hash_iterator_data_new(cow->togc);
   r = eina_iterator_next(it, (void**) &gc);
   eina_iterator_free(it);

   if (!r) return EINA_FALSE; /* Something did go wrong here */

#ifndef NVALGRIND
   /* Do handle hash and all funky merge thing here */
   ref = gc->ref;

   VALGRIND_MAKE_MEM_DEFINED(ref, sizeof (*ref));
#endif
   _eina_cow_gc(cow, gc);
#ifndef NVALGRIND
   VALGRIND_MAKE_MEM_NOACCESS(ref, sizeof (*ref));
#endif

   return EINA_TRUE;
}

