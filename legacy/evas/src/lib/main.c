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









typedef struct _Pool Pool;

struct _Pool
{
   int   usage;
   void *base;
   Pool *prev, *next;
};

Pool *
_evas_mp_pool_new(Evas_Mempool *pool)
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

void
_evas_mp_pool_free(Pool *p)
{
   free(p);
}

//#define NOPOOL
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
	if ((ptr >= pmem) && (ptr < (pmem + psize))) // is it in pool mem?
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
{
   void *mem;
   
   mem = evas_mempool_malloc(pool, size);
   memset(mem, 0, size);
   return mem;
}
