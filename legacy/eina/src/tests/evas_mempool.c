/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>
#include <string.h>

#include "Evas_Data.h"
#include "evas_mempool.h"

//#define NOPOOL

typedef struct _Pool Pool;

struct _Pool
{
   int   usage;
   void *base;
   Pool *prev, *next;
};


Pool *
_evas_mp_pool_new(Evas_Mempool *pool)
#ifdef NOPOOL
{
   static Pool thepool;
   return &thepool;
}
#else
{
   Pool *p;
   void **ptr;
   int item_alloc, i;

   item_alloc = ((pool->item_size + sizeof(void *) - 1) / sizeof(void *)) * sizeof(void *);
   p = malloc(sizeof(Pool) + (pool->pool_size * item_alloc));
   ptr = (void **)(((unsigned char *)p) + sizeof(Pool));
   p->usage = 0;
   p->base = ptr;
   for (i = 0; i < pool->pool_size - 1; i++)
     {
	*ptr = (void **)(((unsigned char *)ptr) + item_alloc);
	ptr = *ptr;
     }
   *ptr = NULL;
   return p;
}
#endif

void
_evas_mp_pool_free(Pool *p)
#ifdef NOPOOL
{
}
#else
{
   free(p);
}
#endif

void *
evas_mempool_malloc(Evas_Mempool *pool, int size)
#ifdef NOPOOL
{
   return malloc(size);
}
#else
{
   Pool *p;
   void *mem;

   for (p = pool->first; p; p = p->next) // look 4 pool from 2nd bucket on
     {
	if (p->base) // base is not NULL - has a free slot
	  {
	     if (p->prev)
	       {
		  if (pool->last == p) pool->last = p->prev;
		  p->prev->next = p->next;
		  p->prev = NULL;
		  p->next = pool->first;
		  p->next->prev = p;
		  pool->first = p;
	       }
	     break;
	  }
     }
   if (!p) // we have reached the end of the list - no free pools
     {
	p = _evas_mp_pool_new(pool);
	if (!p) return NULL;
	p->prev = NULL;
	p->next = pool->first;
	if (p->next) p->next->prev = p;
	if (!pool->last) pool->last = p;
	pool->first = p;
     }
   mem = p->base; // this points to the next free block - so take it
   p->base = *((void **)mem); // base now points to the next free block
   if (!p->base) // move to end - it just filled up
     {
	if (p->next)
	  {
	     if (p->prev) p->prev->next = p->next;
	     else pool->first = p->next;
	     p->next->prev = p->prev;
	     ((Pool *)pool->last)->next = p;
	     p->prev = pool->last;
	     p->next = NULL;
	     pool->last = p;
	  }
     }
   p->usage++;
   pool->usage++;
   return mem;
}
#endif

void
evas_mempool_free(Evas_Mempool *pool, void *ptr)
#ifdef NOPOOL
{
   free(ptr);
}
#else
{
   Pool *p;
   void *pmem;
   int item_alloc, psize;

   item_alloc = ((pool->item_size + sizeof(void *) - 1) / sizeof(void *)) * sizeof(void *);
   psize = item_alloc * pool->pool_size;
   for (p = (Pool *)(pool->first); p; p = p->next) // look 4 pool
     {
	pmem = (void *)(((unsigned char *)p) + sizeof(Pool)); // pool mem base
	if ((ptr >= pmem) && ((unsigned char *)ptr < (((unsigned char *)pmem) + psize))) // is it in pool mem?
	  {
	     *((void **)ptr) = p->base; // freed node points to prev free node
	     p->base = ptr; // next free node is now the one we freed
	     p->usage--;
	     pool->usage--;
	     if (p->usage == 0) // free bucket
	       {
		  if (p->prev) p->prev->next = p->next;
		  if (p->next) p->next->prev = p->prev;
		  if (pool->last == p) pool->last = p->prev;
		  if (pool->first == p) pool->first = p->next;
		  _evas_mp_pool_free(p);
	       }
	     else
	       {
		  if (p->prev) // if not the first - move to front
		    {
		       p->prev->next = p->next;
		       if (p->next) p->next->prev = p->prev;
		       if (pool->last == p) pool->last = p->prev;
		       p->prev = NULL;
		       p->next = pool->first;
		       p->next->prev = p;
		       pool->first = p;
		    }
	       }
	     break;
	  }
     }
}
#endif

void *
evas_mempool_calloc(Evas_Mempool *pool, int size)
#ifdef NOPOOL
{
   return calloc(1, size);
}
#else
{
   void *mem;

   mem = evas_mempool_malloc(pool, size);
   memset(mem, 0, size);
   return mem;
}
#endif
