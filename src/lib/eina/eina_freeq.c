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
#define ITEM_TOTAL_MAX   ( 256 * 1024)
#define ITEM_MEM_MAX     (  32 * 1024 * 1024)
#if __WORDSIZE == 32
// 4kb blocks
//# define ITEM_BLOCK_COUNT 340
// 64k blocks
# define ITEM_BLOCK_COUNT 5459
// 256k blocks
//# define ITEM_BLOCK_COUNT 21844
#else // __WORDSIZE == 64
// 4kb blocks
//# define ITEM_BLOCK_COUNT 170
// 64k blocks
# define ITEM_BLOCK_COUNT 2730
// 256k blocks
//# define ITEM_BLOCK_COUNT 10922
#endif

// ========================================================================= //

typedef struct _Eina_FreeQ_Item Eina_FreeQ_Item;
typedef struct _Eina_FreeQ_Block Eina_FreeQ_Block;

// ========================================================================= //

// these items are highly compressable. here is a dump of F: ptr freefunc size
// ...
// F: 0xaaab0454dd00 0xffff8b83b628 0x10
// F: 0xaaab0454bd00 0xffff8b83b648 0x20
// F: 0xaaab0454dd10 0xffff8b83b628 0x10
// F: 0xaaab0454bd20 0xffff8b83b648 0x20
// F: 0xaaab0454dd20 0xffff8b83b628 0x10
// F: 0xaaab0454bd40 0xffff8b83b648 0x20
// F: 0xaaab0454dd30 0xffff8b83b628 0x10
// F: 0xaaab0454bd60 0xffff8b83b648 0x20
// F: 0xaaab0454bda0 0xffff8b83b648 0x20
// F: 0xaaab0454bdc0 0xffff8b83b648 0x20
// ...
// F: 0xaaab049176d0 0xffff8b83b648 0x20
// F: 0xaaab04917750 0xffff8b83b648 0x20
// F: 0xaaab04917770 0xffff8b83b648 0x20
// F: 0xaaab04917330 0xffff8b83b648 0x20
// F: 0xaaab0481b3c0 0xffff8b83b628 0x10
// F: 0xaaab049177f0 0xffff8b83b648 0x20
// F: 0xaaab049259a0 0xffff8af1c638 0x38
// F: 0xaaab049172d0 0xffff8b83b648 0x20
// F: 0xaaab049172f0 0xffff8b83b648 0x20
// F: 0xaaab04925c00 0xffff8af1c638 0x38
// F: 0xaaab04925c40 0xffff8af1c638 0x38
// F: 0xaaab0481b3b0 0xffff8b83b628 0x10
// F: 0xaaab04917310 0xffff8b83b648 0x20
// F: 0xaaab04925bc0 0xffff8af1c638 0x38
// F: 0xaaab0491d240 0xffff8af1c638 0x40
// F: 0xaaab0492b3a0 0xffff8af1c638 0x38
// F: 0xaaab049225c0 0xffff8af1c638 (nil)
// F: 0xaaab04917730 0xffff8b83b648 0x20
// F: 0xaaab04917790 0xffff8b83b648 0x20
// F: 0xaaab049177b0 0xffff8b83b648 0x20
// F: 0xaaab0492b710 0xffff8af1c638 0x38
// F: 0xaaab0492b750 0xffff8af1c638 0x38
// F: 0xaaab0481b420 0xffff8b83b628 0x10
// F: 0xaaab049177d0 0xffff8b83b648 0x20
// F: 0xaaab0492b2d0 0xffff8af1c638 0x38
// F: 0xaaab0481b410 0xffff8b83b628 0x10
// F: 0xaaab04917710 0xffff8b83b648 0x20
// F: 0xaaab0492b280 0xffff8af1c638 0x40
// F: 0xaaab0481b430 0xffff8b83b628 0x10
// F: 0xaaab04917810 0xffff8b83b648 0x20
// F: 0xaaab0492b6d0 0xffff8af1c638 0x38
// F: 0xaaab0491ca80 0xffff8af1c638 (nil)
// F: 0xaaab0492b350 0xffff8af1c638 0x40
// F: 0xaaab0490fef0 0xffff8af1c638 0x38
// F: 0xaaab0481b320 0xffff8b83b628 0x10
// F: 0xaaab04916ff0 0xffff8b83b648 0x20
// F: 0xaaab0481b330 0xffff8b83b628 0x10
// F: 0xaaab04917030 0xffff8b83b648 0x20
// F: 0xaaab04920560 0xffff8af1c638 0x38
// F: 0xaaab0481b350 0xffff8b83b628 0x10
// F: 0xaaab049170f0 0xffff8b83b648 0x20
// F: 0xaaab04917050 0xffff8b83b648 0x20
// F: 0xaaab04920510 0xffff8af1c638 0x40
// F: 0xaaab04920850 0xffff8af1c638 0x38
// F: 0xaaab04917070 0xffff8b83b648 0x20
// F: 0xaaab04920800 0xffff8af1c638 0x40
// F: 0xaaab04920c30 0xffff8af1c638 0x38
// F: 0xaaab04917090 0xffff8b83b648 0x20
// F: 0xaaab04920be0 0xffff8af1c638 0x40
// F: 0xaaab04920e60 0xffff8af1c638 0x38
// F: 0xaaab04920e10 0xffff8af1c638 0x40
// F: 0xaaab049212b0 0xffff8af1c638 0x38
// F: 0xaaab0481b340 0xffff8b83b628 0x10
// F: 0xaaab049170d0 0xffff8b83b648 0x20
// F: 0xaaab04921260 0xffff8af1c638 0x40
// F: 0xaaab0481b1f0 0xffff8b83b628 0x10
// F: 0xaaab04875d50 0xffff8b83b648 0x20
// F: 0xaaab0490fea0 0xffff8af1c638 0x40
// F: 0xaaab049102e0 0xffff8af1c638 0x38
// F: 0xaaab04917010 0xffff8b83b648 0x20
// F: 0xaaab0490b4f0 0xffff8af1c638 (nil)
// F: 0xaaab04910290 0xffff8af1c638 0x40
// F: 0xaaab0481b460 0xffff8b83b628 0x10
// F: 0xaaab049178b0 0xffff8b83b648 0x20
// F: 0xaaab0481b450 0xffff8b83b628 0x10
// F: 0xaaab04917870 0xffff8b83b648 0x20
// F: 0xaaab0481b490 0xffff8b83b628 0x10
// F: 0xaaab049e56f0 0xffff8b83b648 0x20
// F: 0xaaab0481b4a0 0xffff8b83b628 0x10
// F: 0xaaab049e5710 0xffff8b83b648 0x20
// F: 0xaaab0481b4b0 0xffff8b83b628 0x10
// F: 0xaaab049e5730 0xffff8b83b648 0x20
// F: 0xaaab0481b4c0 0xffff8b83b628 0x10
// F: 0xaaab049e5750 0xffff8b83b648 0x20
// F: 0xaaab0481b4f0 0xffff8b83b628 0x10
// F: 0xaaab049e57d0 0xffff8b83b648 0x20
// F: 0xaaab049e5990 0xffff8b83b648 0x20
// F: 0xaaab049e5770 0xffff8b83b648 0x20
// F: 0xaaab0481b4d0 0xffff8b83b628 0x10
// F: 0xaaab049e5790 0xffff8b83b648 0x20
// F: 0xaaab049e5a50 0xffff8b83b648 0x20
// F: 0xaaab049e57b0 0xffff8b83b648 0x20
// ...
// F: 0xaaab04d9f330 0xffff8b83b648 0x20
// F: 0xaaab04b18920 0xffff8b83b628 0x10
// F: 0xaaab04d9f350 0xffff8b83b648 0x20
// F: 0xaaab04d4d000 0xffff8b83b648 0x20
// F: 0xaaab04d9f370 0xffff8b83b648 0x20
// F: 0xaaab04d9f390 0xffff8b83b648 0x20
// F: 0xaaab04d9f3b0 0xffff8b83b648 0x20
// F: 0xaaab04b18930 0xffff8b83b628 0x10
// F: 0xaaab04d9f3d0 0xffff8b83b648 0x20
// F: 0xaaab04d9f3f0 0xffff8b83b648 0x20
// F: 0xaaab04d9f410 0xffff8b83b648 0x20
// F: 0xaaab04d9f430 0xffff8b83b648 0x20
// F: 0xaaab04b18940 0xffff8b83b628 0x10
// F: 0xaaab04d9f450 0xffff8b83b648 0x20
// F: 0xaaab04d4d020 0xffff8b83b648 0x20
// F: 0xaaab04d9f470 0xffff8b83b648 0x20
// F: 0xaaab04d9f490 0xffff8b83b648 0x20
// F: 0xaaab04d9f4b0 0xffff8b83b648 0x20
// F: 0xaaab04b18950 0xffff8b83b628 0x10
// F: 0xaaab04d9f4d0 0xffff8b83b648 0x20
// F: 0xaaab04d9f4f0 0xffff8b83b648 0x20
// F: 0xaaab04d9f510 0xffff8b83b648 0x20
// F: 0xaaab04d9f530 0xffff8b83b648 0x20
// F: 0xaaab04b18960 0xffff8b83b628 0x10
// F: 0xaaab04d9f550 0xffff8b83b648 0x20
// F: 0xaaab04640ce0 0xffff8b83b628 0x10
// F: 0xaaab04d9ee10 0xffff8b83b648 0x20
// F: 0xaaab04577e50 0xffff8af1c638 (nil)
// F: 0xaaab04571570 0xffff8af1c638 (nil)
// F: 0xaaab04577ee0 0xffff8af1c638 0x40
// F: 0xaaab0457af50 0xffff8af1c638 (nil)
// F: 0xaaab04571590 0xffff8af1c638 (nil)
// F: 0xaaab0457afe0 0xffff8af1c638 0x40
// F: 0xaaab0457e0c0 0xffff8af1c638 (nil)
// F: 0xaaab0457b360 0xffff8af1c638 (nil)
// F: 0xaaab0457e150 0xffff8af1c638 0x40
// F: 0xaaab04581860 0xffff8af1c638 (nil)
// F: 0xaaab04581330 0xffff8af1c638 (nil)
// F: 0xaaab045818f0 0xffff8af1c638 0x40
// F: 0xaaab0490ed00 0xffff8af1c638 0xc0
// F: 0xaaab0490f090 0xffff8af1c638 0xc0
// F: 0xaaab04922760 0xffff8af1c638 0xc0
// F: 0xaaab04922880 0xffff8af1c638 0xc0
// F: 0xaaab04922a20 0xffff8af1c638 0xc0
// F: 0xaaab0491cb80 0xffff8af1c638 0xc0
// F: 0xaaab0492b8e0 0xffff8af1c638 0xc0
// F: 0xaaab0492c4b0 0xffff8af1c638 0xc0
// F: 0xaaab0492c5c0 0xffff8af1c638 0xc0
// F: 0xaaab0492c750 0xffff8af1c638 0xc0
// F: 0xaaab04926230 0xffff8af1c638 0xc0
// F: 0xaaab04920d00 0xffff8af1c638 0xc0
// F: 0xaaab04920aa0 0xffff8af1c638 0xc0
// F: 0xaaab04a05280 0xffff8af1c638 0xc0
// F: 0xaaab04a053d0 0xffff8af1c638 0xc0
// F: 0xaaab04a05520 0xffff8af1c638 0xc0
// F: 0xaaab049f3860 0xffff8af1c638 0xc0
// F: 0xaaab049f3a30 0xffff8af1c638 0xc0
// F: 0xaaab04a06e60 0xffff8af1c638 0xc0
// F: 0xaaab04a25490 0xffff8af1c638 0xc0
// F: 0xaaab04a55170 0xffff8af1c638 0xc0
// F: 0xaaab04a55ca0 0xffff8af1c638 0xc0
// ...
// so in future maybe create delta compression. keep a "start value" in the
// Eina_FreeQ_Block block for each to begin from (and update these as we
// march blcok->start forward (or at least update them when we finish a run
// of processing items at the end of the processing.
//
// we can store things as DELTAS from the preview value. ptr, func, size all
// are ptr sized values so we can compress them with deltas and thus encode
// them in variable runs of bytes depending on the size of the delta. e.g.
// use LEB128 maybe or PrefixVariant.
//
// after some playng leb128 seems to be the best from simplicity (so fast
// encode which matters and decode needs to be good too) and size. i saw
// a reduction to 24% of the original data size this way based on the sample
// data i collected like above. is it worth the extra cycles? don't know.
//
// when looking at the deltas i noticed that func and sie delats are very
// often 0 for long runs. this means we can probably use RLE effectively
// if we split this into 3 streams wahc delta compressed then RLE compressed
// per stream. walking is more complex and filling the block means taking
// a guess at pre-allocating offsets per stream so it may not fill the blocks
// as affectively then. again - is it worth it? need to measure if RLE helps
// a lot or not in keeping size down in addition to delta + leb128.
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

// debgging/tuning info to enable in future when gathering stats
#if 0
static int            _max_seen = 0;
# define FQMAX(fq) \
   if (fq == _eina_freeq_main) { \
      if (fq->count > _max_seen) { \
         _max_seen = fq->count; \
         printf("FQ max: %i\n", _max_seen); \
      } \
   }
#else
# define FQMAX(fq)
#endif

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
                    size_t size)
{
   if (EINA_LIKELY((size > 0) && (size < _eina_freeq_fillpat_max)))
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

   FQMAX(fq);
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

EINA_API Eina_FreeQ *
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

EINA_API void
eina_freeq_free(Eina_FreeQ *fq)
{
   if (!fq) return;
   if (fq == _eina_freeq_main) _eina_freeq_main = NULL;
   eina_freeq_clear(fq);
   if (!fq->unlocked) eina_lock_free(&(fq->lock));
   free(fq);
}

EINA_API Eina_FreeQ_Type
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

EINA_API Eina_FreeQ *
eina_freeq_main_get(void)
{
   return _eina_freeq_main;
}

EINA_API void
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

EINA_API int
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

EINA_API void
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

EINA_API size_t
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

EINA_API void
eina_freeq_clear(Eina_FreeQ *fq)
{
   if (!fq) return;
   LOCK_FQ(fq);
   FQMAX(fq);
   while (fq->count > 0) _eina_freeq_process(fq);
   UNLOCK_FQ(fq);
}

EINA_API void
eina_freeq_reduce(Eina_FreeQ *fq, int count)
{
   if (!fq) return;
   LOCK_FQ(fq);
   FQMAX(fq);
   while ((fq->count > 0) && (count > 0))
     {
        _eina_freeq_process(fq);
        count--;
     }
   UNLOCK_FQ(fq);
}

EINA_API Eina_Bool
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

EINA_API void
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
