#include "evas_common.h"
#include "evas_private.h"

int _evas_alloc_error = 0;
static int _evas_debug_init = 0;
static int _evas_debug_show = 0;
static int _evas_debug_abort = 0;

/**
 * Return if any allocation errors have occured during the prior function
 * @return The allocation error flag
 *
 * This function will return if any memory allocation errors occured during,
 * and what kind they were. The return value will be one of
 * EVAS_ALLOC_ERROR_NONE, EVAS_ALLOC_ERROR_FATAL or EVAS_ALLOC_ERROR_RECOVERED
 * with each meaning something different.
 *
 * EVAS_ALLOC_ERROR_NONE means that no errors occured at all and the function
 * worked as expected.
 *
 * EVAS_ALLOC_ERROR_FATAL means the function was completely unable to perform
 * its job and will  have  exited as cleanly as possible. The programmer
 * should consider this as a sign of very low memory and should try and safely
 * recover from the prior functions failure (or try free up memory elsewhere
 * and try again after more memory is freed).
 *
 * EVAS_ALLOC_ERROR_RECOVERED means that an allocation error occured, but was
 * recovered from by evas finding memory of its own it has allocated and
 * freeing what it sees as not really usefully allocated memory. What is freed
 * may vary. Evas may reduce the resolution of images, free cached images or
 * fonts, trhow out pre-rendered data, reduce the complexity of change lists
 * etc. Evas and the program will function as per normal after this, but this
 * is a sign of low memory, and it is suggested that the program try and
 * identify memory it doesn't need, and free it.
 *
 * Example:
 * @code
 * extern Evas_Object *object;
 * void callback (void *data, Evas *e, Evas_Object *obj, void *event_info);
 *
 * evas_object_event_callback_add(object, EVAS_CALLBACK_MOUSE_DOWN, callback, NULL);
 * if (evas_alloc_error() == EVAS_ALLOC_ERROR_FATAL)
 *   {
 *     fprintf(stderr, "ERROR: Completely unable to attach callabck. Must\n");
 *     fprintf(stderr, "       destroy object now as it cannot be used.\n");
 *     evas_object_del(object);
 *     object = NULL;
 *     fprintf(stderr, "WARNING: Memory is really low. Cleaning out RAM.\n");
 *     my_memory_cleanup();
 *   }
 * if (evas_alloc_error() == EVAS_ALLOC_ERROR_RECOVERED)
 *   {
 *     fprintf(stderr, "WARNING: Memory is really low. Cleaning out RAM.\n");
 *     my_memory_cleanup();
 *   }
 * @endcode
 */
int
evas_alloc_error(void)
{
   return _evas_alloc_error;
}

/* free cached items only in ram for speed reasons. return 0 if cant free */
int
evas_mem_free(int mem_required)
{
   return 0;
}

/* start reducing quality of images etc. return 0 if cant free anything */
int
evas_mem_degrade(int mem_required)
{
   return 0;
}

void *
evas_mem_calloc(int size)
{
   void *ptr;

   ptr = calloc(1, size);
   if (ptr) return ptr;
   MERR_BAD();
   while ((!ptr) && (evas_mem_free(size))) ptr = calloc(1, size);
   if (ptr) return ptr;
   while ((!ptr) && (evas_mem_degrade(size))) ptr = calloc(1, size);
   if (ptr) return ptr;
   MERR_FATAL();
   return NULL;
}

void
evas_debug_error(void)
{
   if (!_evas_debug_init)
     {
	if (getenv("EVAS_DEBUG_SHOW")) _evas_debug_show = 1;
	if (getenv("EVAS_DEBUG_ABORT")) _evas_debug_abort = 1;
	_evas_debug_init = 1;
     }
   if (_evas_debug_show)
     fprintf(stderr,
	     "*** EVAS ERROR: Evas Magic Check Failed!!!\n");
}

void
evas_debug_input_null(void)
{
   if (!_evas_debug_init)
     {
	if (getenv("EVAS_DEBUG_SHOW")) _evas_debug_show = 1;
	if (getenv("EVAS_DEBUG_ABORT")) _evas_debug_abort = 1;
	_evas_debug_init = 1;
     }
   if (_evas_debug_show)
     fprintf(stderr,
	     "  Input object pointer is NULL!\n");
   if (_evas_debug_abort) abort();
}

void
evas_debug_magic_null(void)
{
   if (!_evas_debug_init)
     {
	if (getenv("EVAS_DEBUG_SHOW")) _evas_debug_show = 1;
	if (getenv("EVAS_DEBUG_ABORT")) _evas_debug_abort = 1;
	_evas_debug_init = 1;
     }
   if (_evas_debug_show)
     fprintf(stderr,
	     "  Input object is zero'ed out (maybe a freed object or zero-filled RAM)!\n");
   if (_evas_debug_abort) abort();
}

void
evas_debug_magic_wrong(DATA32 expected, DATA32 supplied)
{
   if (!_evas_debug_init)
     {
	if (getenv("EVAS_DEBUG_SHOW")) _evas_debug_show = 1;
	if (getenv("EVAS_DEBUG_ABORT")) _evas_debug_abort = 1;
	_evas_debug_init = 1;
     }
   if (_evas_debug_show)
     fprintf(stderr,
	     "  Input object is wrong type\n"
	     "    Expected: %08x - %s\n"
	     "    Supplied: %08x - %s\n",
	     expected, evas_debug_magic_string_get(expected),
	     supplied, evas_debug_magic_string_get(supplied));
   if (_evas_debug_abort) abort();
}

void
evas_debug_generic(const char *str)
{
   if (!_evas_debug_init)
     {
	if (getenv("EVAS_DEBUG_SHOW")) _evas_debug_show = 1;
	if (getenv("EVAS_DEBUG_ABORT")) _evas_debug_abort = 1;
	_evas_debug_init = 1;
     }
   if (_evas_debug_show)
     fprintf(stderr,
	     "*** EVAS ERROR:\n"
	     "%s", (char *)str);
   if (_evas_debug_abort) abort();
}

char *
evas_debug_magic_string_get(DATA32 magic)
{
   switch (magic)
     {
      case MAGIC_EVAS:
	return "Evas";
	break;
      case MAGIC_OBJ:
	return "Evas_Object";
	break;
      case MAGIC_OBJ_RECTANGLE:
	return "Evas_Object (Rectangle)";
	break;
      case MAGIC_OBJ_LINE:
	return "Evas_Object (Line)";
	break;
      case MAGIC_OBJ_GRADIENT:
	return "Evas_Object (Gradient)";
	break;
      case MAGIC_OBJ_POLYGON:
	return "Evas_Object (Polygon)";
	break;
      case MAGIC_OBJ_IMAGE:
	return "Evas_Object (Image)";
	break;
      case MAGIC_OBJ_TEXT:
	return "Evas_Object (Text)";
	break;
      case MAGIC_OBJ_SMART:
	return "Evas_Object (Smart)";
	break;
      default:
	return "<UNKNOWN>";
     };
   return "<UNKNOWN>";
}

typedef struct _Evas_Mempool1 Evas_Mempool1;
typedef struct _Evas_Mempool2 Evas_Mempool2;
typedef unsigned int Evas_Mempool_Bitmask;

struct _Evas_Mempool1 /* used if pool size <= 32 */
{
   Evas_Mempool_Bitmask  allocated;
   Evas_Mempool1        *next;
   unsigned char        *mem;
};

struct _Evas_Mempool2 /* used if pool size > 32 */
{
   Evas_Mempool_Bitmask  allocated, filled;
   Evas_Mempool_Bitmask  allocated_list[32];
   Evas_Mempool2        *next;
   unsigned char        *mem;
};

static Evas_Mempool1 *
_evas_mempoool1_new(Evas_Mempool *pool)
{
   Evas_Mempool1 *mp;
   
   if (pool->pool_size <= 32)
     mp = malloc(sizeof(Evas_Mempool1) + (pool->item_size * pool->pool_size));
   else
     mp = malloc(sizeof(Evas_Mempool1) + (pool->item_size * 32));
   mp->allocated = 0;
   mp->next = NULL;
   mp->mem = (unsigned char *)mp + sizeof(Evas_Mempool1);
   return mp;
}

static void
_evas_mempool1_free(Evas_Mempool1 *mp)
{
   free(mp);
}

static Evas_Mempool1 *
_evas_mempool1_free_find(Evas_Mempool *pool, int *slot, Evas_Mempool1 **pmp)
{
   Evas_Mempool1 *mp;
   int i, psize;
   Evas_Mempool_Bitmask allocated;
   
   psize = pool->pool_size;
   if (psize > 32) psize = 32;
   for (mp = (Evas_Mempool1 *)pool->first; mp; mp = mp->next)
     {
	allocated = mp->allocated;
	if (allocated != 0xffffffff)
	  {
	     for (i = 0; i < psize; i++)
	       {
		  if ((allocated & (1 << i)) == 0)
		    {
		       *slot = i;
		       return mp;
		    }
	       }
	  }
	*pmp = mp;
	if (!mp->next) mp->next = _evas_mempoool1_new(pool);
     }
   return NULL;
}

static Evas_Mempool1 *
_evas_mempool1_pointer_find(Evas_Mempool *pool, int *slot, Evas_Mempool1 **pmp, unsigned char *ptr)
{
   Evas_Mempool1 *mp;
   int i, psize, isize;
   unsigned char *mem;
   
   psize = pool->pool_size;
   if (psize > 32) psize = 32;
   isize = pool->item_size;
   for (mp = (Evas_Mempool1 *)pool->first; mp; mp = mp->next)
     {
	mem = mp->mem;
	if (ptr >= mem)
	  {
	     i = (ptr - mem) / isize;
	     if (i < psize)
	       {
		  *slot = i;
		  return mp;
	       }
	  }
	*pmp = mp;
     }
   return NULL;
}

static void
_evas_mempool1_slot_set(Evas_Mempool1 *mp, int slot)
{
   mp->allocated |= (1 << slot);
}

static void
_evas_mempool1_slot_unset(Evas_Mempool1 *mp, int slot)
{
   mp->allocated &= ~(1 << slot);
}

/*
static void
_evas_mempool1_debug(Evas_Mempool *pool)
{
   Evas_Mempool1 *mp;
   int psize, isize, i, j, bits, space, allocated, nodes;
   
   psize = pool->pool_size;
   if (psize > 32) psize = 32;
   isize = pool->item_size;
   nodes = allocated = space = 0;
   for (i = 0, mp = (Evas_Mempool1 *)pool->first; mp; mp = mp->next, i++)
     {
	bits = 0;
	
	for (j = 0; j < 32; j++)
	  {
	     if ((mp->allocated & (1 << j)) != 0) bits++;
	  }
	allocated += bits * isize;
	space += psize * isize;
	nodes++;
//	printf("pool %i, alloc %08x, full %i/%i\n",
//	       i, mp->allocated, bits, 32);
     }
   printf("pool[0-32] %p usage (%i @ %i, %i nodes) %3.1f%%\n",
	  pool, pool->usage, psize, nodes, 
	  100.0 * (double)allocated / (double)space);
}
*/



static Evas_Mempool2 *
_evas_mempoool2_new(Evas_Mempool *pool)
{
   Evas_Mempool2 *mp;

   if (pool->pool_size <= 1024)
     mp = malloc(sizeof(Evas_Mempool2) + (pool->item_size * pool->pool_size));
   else
     mp = malloc(sizeof(Evas_Mempool2) + (pool->item_size * 1024));
   mp->allocated = 0;
   mp->filled = 0;
   memset(mp->allocated_list, 0, sizeof(int) * 32);
   mp->next = NULL;
   mp->mem = (unsigned char *)mp + sizeof(Evas_Mempool2);
   return mp;
}

static void
_evas_mempool2_free(Evas_Mempool2 *mp)
{
   free(mp);
}

static Evas_Mempool2 *
_evas_mempool2_free_find(Evas_Mempool *pool, int *slot, Evas_Mempool2 **pmp)
{
   Evas_Mempool2 *mp;
   int i, j, psize, ps, bsize;
   Evas_Mempool_Bitmask allocated, filled;
   
   psize = pool->pool_size;
   if (psize > 1024) psize = 1024;
   bsize = (psize + 31) / 32;
   for (mp = (Evas_Mempool2 *)pool->first; mp; mp = mp->next)
     {
	filled = mp->filled;
	if (filled != 0xffffffff)
	  {
	     for (j = 0; j < bsize; j++)
	       {
		  if ((filled & (1 << j)) == 0)
		    {
		       if (j == bsize - 1)
			 ps = psize - (j * 32);
		       else
			 ps = 32;
		       allocated = mp->allocated_list[j];
		       for (i = 0; i < ps; i++)
			 {
			    if ((allocated & (1 << i)) == 0)
			      {
				 *slot = (j * 32) + i;
				 return mp;
			      }
			 }
		    }
	       }
	  }
	*pmp = mp;
	if (!mp->next) mp->next = _evas_mempoool2_new(pool);
     }
   return NULL;
}

static Evas_Mempool2 *
_evas_mempool2_pointer_find(Evas_Mempool *pool, int *slot, Evas_Mempool2 **pmp, unsigned char *ptr)
{
   Evas_Mempool2 *mp;
   int i, psize, isize;
   unsigned char *mem;
   
   psize = pool->pool_size;
   if (psize > 1024) psize = 1024;
   isize = pool->item_size;
   for (mp = (Evas_Mempool2 *)pool->first; mp; mp = mp->next)
     {
	mem = mp->mem;
	if (ptr >= mem)
	  {
	     i = (ptr - mem) / isize;
	     if (i < psize)
	       {
		  *slot = i;
		  return mp;
	       }
	  }
	*pmp = mp;
     }
   return NULL;
}

static void
_evas_mempool2_slot_set(Evas_Mempool2 *mp, int slot)
{
   int bucket;
   
   bucket = slot / 32;
   mp->allocated_list[bucket] |= (1 << (slot - (bucket * 32)));
   mp->allocated |= (1 << bucket);
   if (mp->allocated_list[bucket] == 0xffffffff)
     mp->filled |= (1 << bucket);
}

static void
_evas_mempool2_slot_unset(Evas_Mempool2 *mp, int slot)
{
   int bucket;
   
   bucket = slot / 32;
   mp->allocated_list[bucket] &= ~(1 << (slot - (bucket * 32)));
   mp->filled &= ~(1 << bucket);
   if (mp->allocated_list[bucket] == 0)
     mp->allocated &= ~(1 << bucket);
}

/*
static void
_evas_mempool2_debug(Evas_Mempool *pool)
{
   Evas_Mempool2 *mp;
   int psize, bsize, isize, i, j, ps, bits, allocated, space, nodes;
   
   psize = pool->pool_size;
   if (psize > 1024) psize = 1024;
   bsize = (psize + 31) / 32;
   isize = pool->item_size;
   nodes = allocated = space = 0;
   for (i = 0, mp = (Evas_Mempool2 *)pool->first; mp; mp = mp->next, i++)
     {
	for (i = 0; i < bsize; i++)
	  {
	     bits = 0;
	     if (i == bsize - 1)
	       ps = psize - (i * 32);
	     else
	       ps = 32;
	     for (j = 0; j < ps; j++)
	       {
		  if ((mp->allocated_list[i] & (1 << j)) != 0) bits++;
	       }
//	     printf("pool %i, alloc %08x, full %i/%i\n",
//		    i, mp->allocated, bits, 32);
	     allocated += bits * isize;
	  }
	space += psize * isize;
	nodes++;
     }
   printf("pool[32-1024] %p usage (%i @ %i, %i nodes) %3.1f%%\n",
	  pool, pool->usage, psize, nodes, 
	  100.0 * (double)allocated / (double)space);
}
*/

/*#define NOPOOL 1*/

void *
evas_mempool_malloc(Evas_Mempool *pool, int size)
#ifdef NOPOOL
{
   return malloc(size);
}
#else
{
   if (pool->pool_size <= 32)
     {
	Evas_Mempool1 *mp, *pmp = NULL;
	int freeslot;
	
	mp = pool->first;
	if (!mp)
	  {
	     mp = _evas_mempoool1_new(pool);
	     pool->first = mp;
	     freeslot = 0;
	  }
	else mp = _evas_mempool1_free_find(pool, &freeslot, &pmp);
	if (!mp) return NULL;
	pool->usage++;
	_evas_mempool1_slot_set(mp, freeslot);
	if (mp->allocated == 0xffffffff)
	  {
	     if (mp->next)
	       {
		  if (pool->first == mp) pool->first = mp->next;
		  else pmp->next = mp;
		  mp->next = NULL;
	       }
	  }
/*	_evas_mempool1_debug(pool);*/
	return mp->mem + (freeslot * pool->item_size);
     }
   else
     {
	Evas_Mempool2 *mp, *pmp = NULL;
	int freeslot;
	
	mp = pool->first;
	if (!mp)
	  {
	     mp = _evas_mempoool2_new(pool);
	     pool->first = mp;
	     freeslot = 0;
	  }
	else mp = _evas_mempool2_free_find(pool, &freeslot, &pmp);
	if (!mp) return NULL;
	pool->usage++;
	_evas_mempool2_slot_set(mp, freeslot);
	if (mp->allocated == 0xffffffff)
	  {
	     if (mp->next)
	       {
		  if (pool->first == mp) pool->first = mp->next;
		  else pmp->next = mp;
		  mp->next = NULL;
	       }
	  }
/*	_evas_mempool2_debug(pool);*/
	return mp->mem + (freeslot * pool->item_size);
     }
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
   if (pool->pool_size <= 32)
     {
	Evas_Mempool1 *mp, *pmp = NULL;
	int allocslot;
	
	mp = _evas_mempool1_pointer_find(pool, &allocslot, &pmp, (unsigned char*)ptr);
	if (!mp) return;
	_evas_mempool1_slot_unset(mp, allocslot);
	if (mp->allocated == 0)
	  {
	     if (pool->first == mp) pool->first = mp->next;
	     else pmp->next = mp->next;
	     _evas_mempool1_free(mp);
	  }
	else
	  {
	     if (pool->first != mp)
	       {
		  pmp->next = mp->next;
		  mp->next = pool->first;
		  pool->first = mp;
	       }
	  }
	pool->usage--;
     }
   else
     {
	Evas_Mempool2 *mp, *pmp = NULL;
	int allocslot;
	
	mp = _evas_mempool2_pointer_find(pool, &allocslot, &pmp, (unsigned char*)ptr);
	if (!mp) return;
	_evas_mempool2_slot_unset(mp, allocslot);
	if (mp->allocated == 0)
	  {
	     if (pool->first == mp) pool->first = mp->next;
	     else pmp->next = mp->next;
	     _evas_mempool2_free(mp);
	  }
	else
	  {
	     if (pool->first != mp)
	       {
		  pmp->next = mp->next;
		  mp->next = pool->first;
		  pool->first = mp;
	       }
	  }
	pool->usage--;
     }
}
#endif

void *
evas_mempool_calloc(Evas_Mempool *pool, int size)
{
   void *mem;
   
   mem = evas_mempool_malloc(pool, size);
   memset(mem, 0, size);
   return mem;
}
