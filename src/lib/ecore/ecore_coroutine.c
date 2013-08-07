#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>
#include <sys/time.h>
#include <assert.h>
#include <sys/types.h>
#include <unistd.h>

#if defined(USE_UCONTEXT)
# include <sys/time.h>
# include <sys/resource.h>
# include <ucontext.h>
#elif defined(USE_SETJMP)
# include <sys/time.h>
# if HAVE_SYS_RESOURCE_H
#  include <sys/resource.h>
# endif
# include <setjmp.h>
#endif

#ifdef HAVE_EVIL
# include <Evil.h>
#endif

#ifdef _WIN32
# define USE_FIBERS
# undef USE_UCONTEXT
# undef USE_SETJMP
#endif

#include "Ecore.h"
#include "ecore_private.h"

typedef struct _Ecore_Coroutine_Defer Ecore_Coroutine_Defer;
struct _Ecore_Coroutine_Defer
{
   Eina_Free_Cb func;
   void *data;
};

struct _Ecore_Coroutine
{
#if defined(USE_FIBERS)
   void *context;
#elif defined(USE_UCONTEXT)
   ucontext_t context;
#elif defined(USE_SETJMP)
   jmp_buf context;
#else
# error "No coroutine implementation !"
#endif

   Eina_Inarray defer;

   Ecore_Coroutine_Cb func;
   void *data;

   Ecore_Coroutine_State state;

#ifdef USE_VALGRIND
   int vg_stack_id;
#endif
   int yield_value;

   unsigned char stack[1];
};

#ifdef __x86_64__
static const int _ecore_coroutine_default_stack_size = 16 * 1024;
#else
static const int _ecore_coroutine_default_stack_size = 12 * 1024;
#endif

static void
_ecore_coroutine_finish(Ecore_Coroutine *coro)
{
   int return_value = coro->func(coro->data, coro);

   coro->state = ECORE_COROUTINE_FINISHED;
   ecore_coroutine_yield(coro, return_value);
}

#if defined(USE_UCONTEXT)
# ifdef __x86_64__
union ptr_splitter {
   void *ptr;
   uint32_t part[sizeof(void *) / sizeof(uint32_t)];
};

static void
_ecore_coroutine_entry_point(uint32_t part0, uint32_t part1)
{
   union ptr_splitter p = {
     .part = { part0, part1 }
   };
   Ecore_Coroutine *coro = p.ptr;

   _ecore_coroutine_finish(coro);
}
# else
static void
_ecore_coroutine_entry_point(Ecore_Coroutine *coro)
{
   _ecore_coroutine_finish(coro);
}
# endif
#else
static void
_ecore_coroutine_entry_point(Ecore_Coroutine *coro)
{
   _ecore_coroutine_finish(coro);
}
# if defined(USE_SETJMP)
static void
_ecore_coroutine_setjmp(Ecore_Coroutine *coro)
{
   setjmp(coro->context);

   /* The idea of this trick come from libcoroutine */
   /* __jmpbuf[6] == stack pointer */
   /* __jmpbuf[7] == program counter */
   coro->context->env[0].__jmpbuf[6] = ((uintptr_t)(&coro->stack));
   coro->context->env[0].__jmpbuf[7] = ((uintptr_t)_ecore_coroutine_entry_point);
}
# endif
#endif

#if defined(USE_FIBERS)
static void *caller;
#elif defined(USE_UCONTEXT)
static ucontext_t caller;
static ucontext_t callee;
#elif defined(USE_SETJMP)
static jmp_buf caller;
static jmp_buf callee;
#endif

void
_ecore_coroutine_init(void)
{
#if defined(USE_FIBERS)
   caller = GetCurrentFiber();
   if (caller == (LPVOID) 0x1e00)
     {
        caller = ConvertThreadToFiber(NULL);
     }
#else
   memset(&caller, 0, sizeof (caller));
   memset(&callee, 0, sizeof (callee));
#endif
}

void
_ecore_coroutine_shutdown(void)
{
#ifdef USE_FIBERS
   ConvertFiberToThread();
#endif

   // FIXME: should we track lost coroutine ?
}

EAPI Ecore_Coroutine *
ecore_coroutine_add(int stack_size, Ecore_Coroutine_Cb func, void *data)
{
   Ecore_Coroutine *coro;
#ifndef USE_FIBERS
   unsigned char *stack;
#endif

   if (stack_size <= 0)
     {
#if defined(USE_UCONTEXT) || defined(USE_SETJMP)
        struct rlimit check;

        if (getrlimit(RLIMIT_STACK, &check))
          check.rlim_cur = _ecore_coroutine_default_stack_size;
        stack_size = check.rlim_cur;
#elif defined(USE_FIBERS)
        stack_size = _ecore_coroutine_default_stack_size;
#endif
        if (stack_size < _ecore_coroutine_default_stack_size)
          stack_size = _ecore_coroutine_default_stack_size;
     }

   coro = malloc(sizeof (Ecore_Coroutine) + stack_size - 1);
   if (!coro) return NULL;

#ifndef USE_FIBERS
   stack = coro->stack;
#endif

#ifdef USE_VALGRIND
   coro->vg_stack_id = VALGRIND_STACK_REGISTER(stack, stack + stack_size);
#endif

   coro->state = ECORE_COROUTINE_NEW;
   coro->func = func;
   coro->data = data;
   eina_inarray_step_set(&coro->defer,
                         sizeof (Eina_Inarray), sizeof (Ecore_Coroutine_Defer),
                         8);

#if defined(USE_UCONTEXT)
   getcontext(&coro->context);

   coro->context.uc_stack.ss_sp = stack;
   coro->context.uc_stack.ss_size = stack_size;
   coro->context.uc_stack.ss_flags = 0;
   coro->context.uc_link = NULL;

# ifdef __x86_64__
   union ptr_splitter p = { .ptr = coro };
   makecontext(&coro->context, (void (*)())_ecore_coroutine_entry_point,
               2, p.part[0], p.part[1]);
# else
   makecontext(&coro->context, (void (*)())_ecore_coroutine_entry_point,
               1, coro);
# endif
#elif defined(USE_FIBERS)
   coro->context = CreateFiber(stack_size,
                               (LPFIBER_START_ROUTINE)_ecore_coroutine_entry_point,
                               coro);
   if (!coro->context)
     {
        free(coro);
        return NULL;
     }
#elif defined(USE_SETJMP)
   /* We use an intermediate function call to setup the stack with the right arguments */
   _ecore_coroutine_setjmp(coro);
#endif

   return coro;
}

EAPI void *
ecore_coroutine_del(Ecore_Coroutine *coro)
{
   void *data;

   data = coro->data;

   while (eina_inarray_count(&coro->defer))
     {
        Ecore_Coroutine_Defer *defer;

        defer = eina_inarray_pop(&coro->defer);
        defer->func(defer->data);
     }
   eina_inarray_flush(&coro->defer);

#ifdef USE_VALGRIND
   VALGRIND_STACK_DEREGISTER(coro->vg_stack_id);
#endif

#ifdef USE_FIBERS
   DeleteFiber(coro->context);
#endif

   free(coro);
   return data;
}

EAPI int
ecore_coroutine_resume(Ecore_Coroutine *coro)
{
#if defined(USE_FIBERS)
   void *prev_caller;
#elif defined(USE_UCONTEXT)
   ucontext_t prev_caller;
#elif defined(USE_SETJMP)
   jmp_buf prev_caller;
#endif

   if (coro->state == ECORE_COROUTINE_FINISHED)
     return 0;
   coro->state = ECORE_COROUTINE_RUNNING;

   prev_caller = caller;
#if defined(USE_FIBERS)
   SwitchToFiber(coro->context);
#elif defined(USE_UCONTEXT)
   swapcontext(&caller, &coro->context);
#elif defined(USE_SETJMP)
   setjmp(caller);
   longjmp(coro->context);
#endif
#ifndef USE_FIBERS
   // As fiber do handle the callee stack for us, no need here
   coro->context = callee;
#endif
   caller = prev_caller;

   return coro->yield_value;
}

EAPI void
ecore_coroutine_yield(Ecore_Coroutine *coro, int value)
{
   coro->yield_value = value;
#if defined(USE_FIBERS)
   SwitchToFiber(caller);
#elif defined(USE_UCONTEXT)
   swapcontext(&callee, &caller);
#elif defined(USE_SETJMP)
   setjmp(callee);
   longjmp(caller);
#endif
}

EAPI void *
ecore_coroutine_data_get(Ecore_Coroutine *coro)
{
   return coro->data;
}

EAPI Ecore_Coroutine_State
ecore_coroutine_state_get(Ecore_Coroutine *coro)
{
   return coro->state;
}

EAPI void
ecore_coroutine_defer(Ecore_Coroutine *coro, Eina_Free_Cb func, void *data)
{
   Ecore_Coroutine_Defer *defer;

   defer = eina_inarray_grow(&coro->defer, 1);
   if (!defer) return;
   defer->func = func;
   defer->data = data;
}

EAPI void *
ecore_coroutine_alloc(Ecore_Coroutine *coro, size_t size)
{
   void *data;

   data = malloc(size);
   ecore_coroutine_defer(coro, free, data);
   return data;
}


