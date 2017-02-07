#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Eina.h"
#include "eina_private.h"

#ifdef HAVE_VALGRIND
# include <valgrind.h>
# include <memcheck.h>
#endif

// ========================================================================= //

#define ITEM_FILLPAT_MAX 0
#define ITEM_TOTAL_MAX   (  16 * 1024)
#define ITEM_MEM_MAX     (1024 * 1024)
#define ITEM_BLOCK_COUNT 42

// ========================================================================= //

typedef struct _Eina_FreeQ_Item Eina_FreeQ_Item;
typedef struct _Eina_FreeQ_Block Eina_FreeQ_Block;

// ========================================================================= //

struct _Eina_FreeQ_Item
{
   void    *ptr;
   void   (*free_func) (void *ptr);
   size_t  size;
};

struct _Eina_FreeQ_Block
{
   int start;
   int end;
   Eina_FreeQ_Block *next;
   Eina_FreeQ_Item items[ITEM_BLOCK_COUNT];
};

struct _Eina_FreeQ
{
   Eina_Lock lock; // recursive lock, unused for postponed queues (thread-local)
   int count; // number of item slots used
   int count_max; // maximum number of slots allowed to be used or -1
   size_t mem_max; // the maximum amount of memory allowed to be used
   size_t mem_total; // current total memory known about in the queue
   Eina_FreeQ_Block *blocks; // the list of blocks of free items
   Eina_FreeQ_Block *block_last; // the last block to append items to
   Eina_Bool bypass; // 0 if not to bypass, 1 if we should bypass
   Eina_Bool postponed; // 1 if postponed type of freeq (eg. temp strings)
   Eina_Bool unlocked; // 0 by default, 1 if thread-local (lock not used)
};

// ========================================================================= //

static Eina_FreeQ    *_eina_freeq_main              = NULL;
static int            _eina_freeq_bypass            = -1;
static unsigned int   _eina_freeq_fillpat_max       = ITEM_FILLPAT_MAX;
static unsigned char  _eina_freeq_fillpat_val       = 0x55;
static unsigned char  _eina_freeq_fillpat_freed_val = 0x77;
static int            _eina_freeq_total_max         = ITEM_TOTAL_MAX;
static size_t         _eina_freeq_mem_max           = ITEM_MEM_MAX;

// ========================================================================= //

#define LOCK_FQ(fq); do { \
   if (!fq->unlocked) eina_lock_take(&(fq->lock)); } while(0)
#define UNLOCK_FQ(fq); do { \
   if (!fq->unlocked) eina_lock_release(&(fq->lock)); } while(0)

// ========================================================================= //

static inline void
_eina_freeq_fill_do(void *ptr, size_t size)
{
   if (ptr) memset(ptr, _eina_freeq_fillpat_val, size);
}

static inline void
_eina_freeq_freed_fill_do(void *ptr, size_t size)
{
   if (_eina_freeq_fillpat_freed_val == 0) return;
   if (ptr) memset(ptr, _eina_freeq_fillpat_freed_val, size);
}

static void
_eina_freeq_fill_check(void *ptr, void (*free_func) (void *ptr), size_t size)
{
   unsigned char *p0 = ptr, *p = p0, *pe = p + size;
   for (; p < pe; p++)
     {
        if (*p != _eina_freeq_fillpat_val) goto err;
     }
   return;
err:
   EINA_LOG_ERR("Pointer %p size %lu freed by %p has fill error %x != %x @ %lu",
                p0, (unsigned long)size, free_func,
                (unsigned int)*p, (unsigned int)_eina_freeq_fillpat_val,
                (unsigned long)(p - p0));
}

static void
_eina_freeq_free_do(void *ptr,
                    void (*free_func) (void *ptr),
                    size_t size EINA_UNUSED)
{
   if ((size < _eina_freeq_fillpat_max) && (size > 0))
     {
        _eina_freeq_fill_check(ptr, free_func, size);
        _eina_freeq_freed_fill_do(ptr, size);
     }
   free_func(ptr);
   return;
}

static Eina_Bool
_eina_freeq_block_append(Eina_FreeQ *fq)
{
   Eina_FreeQ_Block *fb = malloc(sizeof(Eina_FreeQ_Block));
   if (!fb) return EINA_FALSE;
   fb->start = 0;
   fb->end = 0;
   fb->next = NULL;
   if (!fq->blocks) fq->blocks = fb;
   else fq->block_last->next = fb;
   fq->block_last = fb;
   return EINA_TRUE;
}

static void
_eina_freeq_process(Eina_FreeQ *fq)
{
   Eina_FreeQ_Block *fb = fq->blocks;
   if (!fb) return;
   _eina_freeq_free_do(fb->items[fb->start].ptr,
                       fb->items[fb->start].free_func,
                       fb->items[fb->start].size);
   fq->mem_total -= fb->items[fb->start].size;
   fb->start++;
   fq->count--;
   if (fb->start == fb->end)
     {
        fq->blocks = fb->next;
        if (!fq->blocks) fq->block_last = NULL;
        free(fb);
     }
}

static void
_eina_freeq_flush_nolock(Eina_FreeQ *fq)
{
   if (fq->postponed) return;

   while ((fq->count > fq->count_max) || (fq->mem_total > fq->mem_max))
     _eina_freeq_process(fq);
}

// ========================================================================= //

static Eina_FreeQ *
_eina_freeq_new_default(void)
{
   Eina_FreeQ *fq;

   if (EINA_UNLIKELY(_eina_freeq_bypass == -1))
     {
        const char *s;
        int v;

        s = getenv("EINA_FREEQ_BYPASS");
        if (s)
          {
             v = atoi(s);
             if (v == 0) _eina_freeq_bypass = 0;
             else _eina_freeq_bypass = 1;
          }
        if (_eina_freeq_bypass == -1)
          {
#ifdef HAVE_VALGRIND
             if (RUNNING_ON_VALGRIND) _eina_freeq_bypass = 1;
             else
#endif
               _eina_freeq_bypass = 0;
          }
        s = getenv("EINA_FREEQ_FILL_MAX");
        if (s) _eina_freeq_fillpat_max = atoi(s);
        s = getenv("EINA_FREEQ_TOTAL_MAX");
        if (s) _eina_freeq_total_max = atoi(s);
        s = getenv("EINA_FREEQ_MEM_MAX");
        if (s) _eina_freeq_mem_max = atoi(s) * 1024;
        s = getenv("EINA_FREEQ_FILL");
        if (s) _eina_freeq_fillpat_val = atoi(s);
        s = getenv("EINA_FREEQ_FILL_FREED");
        if (s) _eina_freeq_fillpat_freed_val = atoi(s);
     }
   fq = calloc(1, sizeof(Eina_FreeQ));
   if (!fq) return NULL;
   eina_lock_recursive_new(&(fq->lock));
   fq->count_max = _eina_freeq_total_max;
   fq->mem_max = _eina_freeq_mem_max;
   fq->bypass = _eina_freeq_bypass;
   return fq;
}

static Eina_FreeQ *
_eina_freeq_new_postponed(void)
{
   Eina_FreeQ *fq;

   fq= calloc(1, sizeof(*fq));
   if (!fq) return NULL;
   fq->mem_max = 0;
   fq->count_max = -1;
   fq->postponed = EINA_TRUE;
   fq->unlocked = EINA_TRUE;
   return fq;
}

EAPI Eina_FreeQ *
eina_freeq_new(Eina_FreeQ_Type type)
{
   switch (type)
     {
      case EINA_FREEQ_DEFAULT:
        return _eina_freeq_new_default();
      case EINA_FREEQ_POSTPONED:
        return _eina_freeq_new_postponed();
      default:
        return NULL;
     }
}

EAPI void
eina_freeq_free(Eina_FreeQ *fq)
{
   if (!fq) return;
   if (fq == _eina_freeq_main) _eina_freeq_main = NULL;
   eina_freeq_clear(fq);
   if (!fq->unlocked) eina_lock_free(&(fq->lock));
   free(fq);
}

EAPI Eina_FreeQ_Type
eina_freeq_type_get(Eina_FreeQ *fq)
{
   if (fq && fq->postponed)
     return EINA_FREEQ_POSTPONED;
   return EINA_FREEQ_DEFAULT;
}

void
eina_freeq_main_set(Eina_FreeQ *fq)
{
   if (!fq) return;
   _eina_freeq_main = fq;
}

EAPI Eina_FreeQ *
eina_freeq_main_get(void)
{
   return _eina_freeq_main;
}

EAPI void
eina_freeq_count_max_set(Eina_FreeQ *fq, int count)
{
   if (!fq) return;
   if (fq->postponed) return;
   if (count < 0) count = -1;
   LOCK_FQ(fq);
   fq->bypass = 0;
   fq->count_max = count;
   _eina_freeq_flush_nolock(fq);
   UNLOCK_FQ(fq);
}

EAPI int
eina_freeq_count_max_get(Eina_FreeQ *fq)
{
   int count;

   if (!fq) return 0;
   LOCK_FQ(fq);
   if (fq->bypass) count = 0;
   else count = fq->count_max;
   UNLOCK_FQ(fq);
   return count;
}

EAPI void
eina_freeq_mem_max_set(Eina_FreeQ *fq, size_t mem)
{
   if (!fq) return;
   if (fq->postponed) return;
   LOCK_FQ(fq);
   fq->bypass = 0;
   fq->mem_max = mem;
   _eina_freeq_flush_nolock(fq);
   UNLOCK_FQ(fq);
}

EAPI size_t
eina_freeq_mem_max_get(Eina_FreeQ *fq)
{
   size_t mem;

   if (!fq) return 0;
   LOCK_FQ(fq);
   if (fq->bypass) mem = 0;
   else mem = fq->mem_max;
   UNLOCK_FQ(fq);
   return mem;
}

EAPI void
eina_freeq_clear(Eina_FreeQ *fq)
{
   if (!fq) return;
   LOCK_FQ(fq);
   while (fq->count > 0) _eina_freeq_process(fq);
   UNLOCK_FQ(fq);
}

EAPI void
eina_freeq_reduce(Eina_FreeQ *fq, int count)
{
   if (!fq) return;
   LOCK_FQ(fq);
   while ((fq->count > 0) && (count > 0))
     {
        _eina_freeq_process(fq);
        count--;
     }
   UNLOCK_FQ(fq);
}

EAPI Eina_Bool
eina_freeq_ptr_pending(Eina_FreeQ *fq)
{
   Eina_Bool pending;

   if (!fq) return EINA_FALSE;
   LOCK_FQ(fq);
   if (fq->blocks) pending = EINA_TRUE;
   else pending = EINA_FALSE;
   UNLOCK_FQ(fq);
   return pending;
}

EAPI void
eina_freeq_ptr_add(Eina_FreeQ *fq,
                   void *ptr,
                   void (*free_func) (void *ptr),
                   size_t size)
{
   Eina_FreeQ_Block *fb;

   if (!ptr) return;
   if (!free_func) free_func = free;
   if ((((fq) && !fq->postponed) || (!fq)) &&
       (size < _eina_freeq_fillpat_max) && (size > 0))
     _eina_freeq_fill_do(ptr, size);

   if (!fq || fq->bypass)
     {
        _eina_freeq_free_do(ptr, free_func, size);
        return;
     }

   LOCK_FQ(fq);
   if ((!fq->block_last) || (fq->block_last->end == ITEM_BLOCK_COUNT))
     {
        if (!_eina_freeq_block_append(fq))
          {
             UNLOCK_FQ(fq);
             if (!fq->postponed)
               _eina_freeq_free_do(ptr, free_func, size);
             else
               EINA_LOG_ERR("Could not add a pointer to the free queue! This "
                            "program will leak resources!");
             return;
          }
     }

   fb = fq->block_last;
   fb->items[fb->end].ptr = ptr;
   fb->items[fb->end].free_func = free_func;
   fb->items[fb->end].size = size;
   fb->end++;
   fq->count++;
   fq->mem_total += size;
   _eina_freeq_flush_nolock(fq);
   UNLOCK_FQ(fq);
}
