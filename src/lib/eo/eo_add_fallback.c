#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef HAVE_VALGRIND
# include <valgrind.h>
# include <memcheck.h>
#endif

#if defined HAVE_DLADDR && ! defined _WIN32
# include <dlfcn.h>
#endif

#include <Eina.h>

#include "eo_ptr_indirection.h"
#include "eo_private.h"

#include "eo_add_fallback.h"

// 1024 entries == 16k or 32k (32 or 64bit) for eo call stack. that's 1023
// imbricated/recursive calls it can handle before barfing. i'd say that's ok
#define EFL_OBJECT_CALL_STACK_DEPTH_MIN 1024

typedef struct _Efl_Object_Call_Stack {
   Eo_Stack_Frame *frames;
   Eo_Stack_Frame *frame_ptr;
} Efl_Object_Call_Stack;

#define EFL_OBJECT_CALL_STACK_SIZE (EFL_OBJECT_CALL_STACK_DEPTH_MIN * sizeof(Eo_Stack_Frame))

static Eina_TLS _eo_call_stack_key = 0;

#define MEM_PAGE_SIZE 4096

static void *
_eo_call_stack_mem_alloc(size_t size)
{
#ifdef HAVE_MMAP
# ifdef HAVE_VALGRIND
   if (RUNNING_ON_VALGRIND) return calloc(1, size);
   else
# endif
     {
        // allocate eo call stack via mmped anon segment if on linux - more
        // secure and safe. also gives page aligned memory allowing madvise
        void *ptr;
        size_t newsize;
        newsize = MEM_PAGE_SIZE * ((size + MEM_PAGE_SIZE - 1) /
                                   MEM_PAGE_SIZE);
        ptr = mmap(NULL, newsize, PROT_READ | PROT_WRITE,
                   MAP_PRIVATE | MAP_ANON, -1, 0);
        if (ptr == MAP_FAILED)
          {
             ERR("eo call stack mmap failed.");
             return NULL;
          }
        return ptr;
     }
#else
   //in regular cases just use malloc
   return calloc(1, size);
#endif
}

static void
_eo_call_stack_mem_free(void *ptr, size_t size)
{
#ifdef HAVE_MMAP
# ifdef HAVE_VALGRIND
   if (RUNNING_ON_VALGRIND) free(ptr);
   else
# endif
   munmap(ptr, size);
#else
   (void) size;
   free(ptr);
#endif
}

static Efl_Object_Call_Stack *
_eo_call_stack_create(void)
{
   Efl_Object_Call_Stack *stack;

   stack = calloc(1, sizeof(Efl_Object_Call_Stack));
   if (!stack)
     return NULL;

   stack->frames = _eo_call_stack_mem_alloc(EFL_OBJECT_CALL_STACK_SIZE);
   if (!stack->frames)
     {
        free(stack);
        return NULL;
     }

   // first frame is never used
   stack->frame_ptr = stack->frames;

   return stack;
}

static void
_eo_call_stack_free(void *ptr)
{
   Efl_Object_Call_Stack *stack = (Efl_Object_Call_Stack *) ptr;

   if (!stack) return;

   if (stack->frames)
     _eo_call_stack_mem_free(stack->frames, EFL_OBJECT_CALL_STACK_SIZE);

   free(stack);
}

static Efl_Object_Call_Stack *main_loop_stack = NULL;

#define _EFL_OBJECT_CALL_STACK_GET() ((EINA_LIKELY(eina_main_loop_is())) ? main_loop_stack : _eo_call_stack_get_thread())

static inline Efl_Object_Call_Stack *
_eo_call_stack_get_thread(void)
{
   Efl_Object_Call_Stack *stack = eina_tls_get(_eo_call_stack_key);

   if (stack) return stack;

   stack = _eo_call_stack_create();
   eina_tls_set(_eo_call_stack_key, stack);

   return stack;
}

EAPI Eo *
_efl_added_get(void)
{
   return _EFL_OBJECT_CALL_STACK_GET()->frame_ptr->obj;
}

Eo_Stack_Frame *
_efl_add_fallback_stack_push(Eo *obj)
{
   Efl_Object_Call_Stack *stack = _EFL_OBJECT_CALL_STACK_GET();
   if (stack->frame_ptr == (stack->frames + EFL_OBJECT_CALL_STACK_DEPTH_MIN))
     {
        CRI("efl_add fallback stack overflow.");
     }

   stack->frame_ptr++;
   stack->frame_ptr->obj = obj;

   return stack->frame_ptr;
}

Eo_Stack_Frame *
_efl_add_fallback_stack_pop(void)
{
   Efl_Object_Call_Stack *stack = _EFL_OBJECT_CALL_STACK_GET();
   if (stack->frame_ptr == stack->frames)
     {
        CRI("efl_add fallback stack underflow.");
     }

   stack->frame_ptr--;

   return stack->frame_ptr;
}

Eina_Bool
_efl_add_fallback_init(void)
{
   if (_eo_call_stack_key != 0)
     WRN("_eo_call_stack_key already set, this should not happen.");
   else
     {
        if (!eina_tls_cb_new(&_eo_call_stack_key, _eo_call_stack_free))
          {
             EINA_LOG_ERR("Could not create TLS key for call stack.");
             return EINA_FALSE;

          }
     }

   main_loop_stack = _eo_call_stack_create();
   if (!main_loop_stack)
     {
        EINA_LOG_ERR("Could not alloc eo call stack.");
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

Eina_Bool
_efl_add_fallback_shutdown(void)
{
   if (_eo_call_stack_key != 0)
     {
        eina_tls_free(_eo_call_stack_key);
        _eo_call_stack_key = 0;
     }

   _eo_call_stack_free(main_loop_stack);
   main_loop_stack = NULL;

   return EINA_TRUE;
}
