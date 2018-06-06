/* EINA - EFL data type library
 * Copyright (C) 2017 ProFUSION embedded systems
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

#include <stdlib.h>
#include <limits.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/resource.h>

#include "eina_config.h"
#include "eina_private.h"
#include "eina_log.h"
#include "eina_mempool.h"
#include "eina_lock.h"
#if USE_CORO_THREAD
#include "eina_thread.h"
#endif
#include "eina_inarray.h"
#include "eina_promise.h"

/* undefs EINA_ARG_NONULL() so NULL checks are not compiled out! */
#include "eina_safety_checks.h"

#include "eina_coro.h"
#include "eina_value.h"
#include "eina_value_util.h"

#if HAVE_VALGRIND
#include <valgrind/valgrind.h>
#endif

typedef void* fcontext_t;

/**
 * Structure used to transfer data between contexts.
 */
typedef struct {
    /* The continuation that called this continuation. Usually you jump_context to this continuation
       in order to make the current continuation 'return' control to it.
     */
    fcontext_t ctx;
    /* Data received from the caller of jump_fcontext. */
    void *data;
} transfer_t;

/**
 * @brief Saves the current context and jumps into another.
 *
 * Saves the current continuation and changes into the continuation
 * pointed by to, passing vp as the data. The current continuation
 * is passed to the callback associated with to through a transfer_t
 * instance, either as argument if the new context is starting or
 * return from a previous jump_fcontext call. This call @b will block
 * @b until this newly-created continuation is jumped into.
 *
 * @param to The continuation to jump into. Must not be #NULL.
 * @param vp User data to pass to the transfer_t structure.
 *
 * @return A context transfer struct with the continuation that called
 *         jump_fcontext returning to this call.
 */
transfer_t ostd_jump_fcontext(
    fcontext_t const to, void *vp
);

/**
 * @brief Initializes a context with the given stack and context function.
 *
 *
 *
 * @param sp Pointer to the @b base @b of the stack.
 * @param size The size of the stack for the new continuation.
// @param fn Callback that will be the starting point of the new continuation.
 */
fcontext_t ostd_make_fcontext(
    void *sp, size_t size, void (*fn)(transfer_t)
);

static Eina_Mempool *_eina_coro_mp = NULL;
static Eina_Lock _eina_coro_lock;

static int _eina_coro_log_dom = -1;
static int _eina_coro_usage = 0;

static int _eina_coro_hooks_walking = 0;
static Eina_Inarray _eina_coro_hooks;

#ifdef CRIT
#undef CRIT
#endif
#define CRIT(...) EINA_LOG_DOM_CRIT(_eina_coro_log_dom, __VA_ARGS__)

#ifdef ERR
#undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_eina_coro_log_dom, __VA_ARGS__)

#ifdef INF
#undef INF
#endif
#define INF(...) EINA_LOG_DOM_INFO(_eina_coro_log_dom, __VA_ARGS__)

#ifdef DBG
#undef DBG
#endif
#define DBG(...) EINA_LOG_DOM_DBG(_eina_coro_log_dom, __VA_ARGS__)

typedef enum _Eina_Coro_Turn {
  EINA_CORO_TURN_MAIN = 0,
  EINA_CORO_TURN_COROUTINE
} Eina_Coro_Turn;

struct _Eina_Coro {
   Eina_Coro_Cb func;
   const void *data;
   Eina_Future *awaiting;
   Eina_Lock lock;
#if USE_CORO_THREAD
   Eina_Condition condition;
   Eina_Thread main;
   Eina_Thread coroutine;
#elif USE_CORO_FCONTEXT
   fcontext_t main;
   fcontext_t coroutine;
   unsigned char* stack;
   size_t stack_size;
   void *result;
#if HAVE_VALGRIND
   int valgrind_stack_id;
#endif
#endif
   Eina_Bool finished;
   Eina_Bool canceled;
   Eina_Coro_Turn turn;
};

#define CORO_TURN_STR(turn) \
  (((turn) == EINA_CORO_TURN_MAIN) ? "MAIN" : "COROUTINE")

#define CORO_FMT "coro=%p {func=%p data=%p turn=%s threads={%p%c %p%c} awaiting=%p}"

#if USE_CORO_THREAD
#define CORO_EXP(coro) \
  coro, coro->func, coro->data, \
    CORO_TURN_STR(coro->turn), \
    (void *)coro->coroutine, \
    eina_thread_self() == coro->coroutine ? '*' : 0, \
    (void *)coro->main, \
    eina_thread_self() == coro->main ? '*' : 0, \
    coro->awaiting
#elif USE_CORO_FCONTEXT
#define CORO_EXP(coro) \
  coro, coro->func, coro->data, \
    CORO_TURN_STR(coro->turn), \
    &coro->coroutine, \
    '*', \
    &coro->main, \
    '*', \
    coro->awaiting
#endif

#if USE_CORO_THREAD
#define IS_CORO(coro) ((coro)->coroutine == eina_thread_self())
#define IS_MAIN(coro) ((coro)->main == eina_thread_self())
#elif USE_CORO_FCONTEXT
#define IS_CORO(coro) ((coro)->turn == EINA_CORO_TURN_COROUTINE)
#define IS_MAIN(coro) ((coro)->turn == EINA_CORO_TURN_MAIN)
#endif

#define EINA_CORO_CHECK(coro, turn, ...)        \
  do \
    { \
       if ((!_eina_coro_mp) || (!eina_mempool_from(_eina_coro_mp, (coro)))) \
         { \
            CRIT(#coro "=%p is invalid.", (coro)); \
            return __VA_ARGS__; \
         } \
       else if ((turn == EINA_CORO_TURN_COROUTINE) && !IS_CORO((coro))) \
         { \
            CRIT("must be called from coroutine! " CORO_FMT, CORO_EXP((coro))); \
            return __VA_ARGS__; \
         } \
       else if ((turn == EINA_CORO_TURN_MAIN) && !IS_MAIN((coro))) \
         { \
            CRIT("must be called from main thread! " CORO_FMT, CORO_EXP((coro))); \
            return __VA_ARGS__; \
         } \
    } \
  while (0)

#define EINA_CORO_CHECK_GOTO(coro, turn, label) \
  do \
    { \
       if ((!_eina_coro_mp) || (!eina_mempool_from(_eina_coro_mp, (coro)))) \
         { \
            CRIT(#coro "=%p is invalid.", (coro)); \
            goto label; \
         } \
       else if ((turn == EINA_CORO_TURN_COROUTINE) && !IS_CORO((coro))) \
         { \
            CRIT("must be called from coroutine! " CORO_FMT, CORO_EXP((coro))); \
            goto label; \
         } \
       else if ((turn == EINA_CORO_TURN_MAIN) && !IS_MAIN((coro))) \
         { \
            CRIT("must be called from main thread! " CORO_FMT, CORO_EXP((coro))); \
            goto label; \
         } \
    } \
  while (0)


typedef struct _Eina_Coro_Hook {
   Eina_Coro_Hook_Coro_Enter_Cb coro_enter;
   Eina_Coro_Hook_Coro_Exit_Cb coro_exit;
   Eina_Coro_Hook_Main_Enter_Cb main_enter;
   Eina_Coro_Hook_Main_Exit_Cb main_exit;
   const void *data;
} Eina_Coro_Hook;

EAPI Eina_Bool
eina_coro_hook_add(Eina_Coro_Hook_Coro_Enter_Cb coro_enter, Eina_Coro_Hook_Coro_Exit_Cb coro_exit, Eina_Coro_Hook_Main_Enter_Cb main_enter, Eina_Coro_Hook_Main_Exit_Cb main_exit, const void *data)
{
   Eina_Coro_Hook hook = { coro_enter, coro_exit, main_enter, main_exit, data };
   int idx;

   eina_lock_take(&_eina_coro_lock);

   EINA_SAFETY_ON_TRUE_GOTO(_eina_coro_hooks_walking > 0, error);

   idx = eina_inarray_push(&_eina_coro_hooks, &hook);
   EINA_SAFETY_ON_TRUE_GOTO(idx < 0, error);

   eina_lock_release(&_eina_coro_lock);
   return EINA_TRUE;

 error:
   eina_lock_release(&_eina_coro_lock);
   return EINA_FALSE;
}

EAPI Eina_Bool
eina_coro_hook_del(Eina_Coro_Hook_Coro_Enter_Cb coro_enter, Eina_Coro_Hook_Coro_Exit_Cb coro_exit, Eina_Coro_Hook_Main_Enter_Cb main_enter, Eina_Coro_Hook_Main_Exit_Cb main_exit, const void *data)
{
   Eina_Coro_Hook hook = { coro_enter, coro_exit, main_enter, main_exit, data };
   int idx;

   eina_lock_take(&_eina_coro_lock);

   EINA_SAFETY_ON_TRUE_GOTO(_eina_coro_hooks_walking > 0, error);

   idx = eina_inarray_remove(&_eina_coro_hooks, &hook);
   EINA_SAFETY_ON_TRUE_GOTO(idx < 0, error);

   eina_lock_release(&_eina_coro_lock);
   return EINA_TRUE;

 error:
   eina_lock_release(&_eina_coro_lock);
   return EINA_FALSE;
}


// opposite of the coro_exit, similar to coro_enter
static void
_eina_coro_hooks_main_exit(Eina_Coro *coro)
{
   const Eina_Coro_Hook *itr;

   eina_lock_take(&_eina_coro_lock);
   _eina_coro_hooks_walking++;
   eina_lock_release(&_eina_coro_lock);

   EINA_INARRAY_FOREACH(&_eina_coro_hooks, itr)
     {
        if (!itr->main_exit) continue;
        if (itr->main_exit((void *)itr->data, coro)) continue;
        coro->canceled = EINA_TRUE;
        ERR("failed hook exit=%p data=%p for main routine " CORO_FMT,
            itr->main_exit, itr->data, CORO_EXP(coro));
     }
}

// opposite of the coro_enter, similar to coro_exit
static void
_eina_coro_hooks_main_enter(Eina_Coro *coro)
{
   const Eina_Coro_Hook *itr;

   EINA_INARRAY_REVERSE_FOREACH(&_eina_coro_hooks, itr)
     {
        if (!itr->main_enter) continue;
        itr->main_enter((void *)itr->data, coro);
     }

   eina_lock_take(&_eina_coro_lock);
   _eina_coro_hooks_walking--;
   eina_lock_release(&_eina_coro_lock);
}

static Eina_Bool
_eina_coro_hooks_coro_enter(Eina_Coro *coro)
{
   const Eina_Coro_Hook *itr;
   Eina_Bool r = EINA_TRUE;

   eina_lock_take(&_eina_coro_lock);
   _eina_coro_hooks_walking++;
   eina_lock_release(&_eina_coro_lock);

   EINA_INARRAY_FOREACH(&_eina_coro_hooks, itr)
     {
        if (!itr->coro_enter) continue;
        if (itr->coro_enter((void *)itr->data, coro)) continue;
        r = EINA_FALSE;
        ERR("failed hook enter=%p data=%p for coroutine " CORO_FMT,
            itr->coro_enter, itr->data, CORO_EXP(coro));
     }

   return r;
}

static void
_eina_coro_hooks_coro_exit(Eina_Coro *coro)
{
   const Eina_Coro_Hook *itr;

   EINA_INARRAY_REVERSE_FOREACH(&_eina_coro_hooks, itr)
     {
        if (!itr->coro_exit) continue;
        itr->coro_exit((void *)itr->data, coro);
     }

   eina_lock_take(&_eina_coro_lock);
   _eina_coro_hooks_walking--;
   eina_lock_release(&_eina_coro_lock);
}

#if USE_CORO_THREAD
static void
_eina_coro_signal(Eina_Coro *coro, Eina_Coro_Turn turn)
{
   DBG("signal turn=%s " CORO_FMT, CORO_TURN_STR(turn), CORO_EXP(coro));

   eina_lock_take(&coro->lock);
   coro->turn = turn;
   eina_condition_signal(&coro->condition);
   eina_lock_release(&coro->lock);
}

static void
_eina_coro_wait(Eina_Coro *coro, Eina_Coro_Turn turn)
{
   DBG("waiting turn=%s " CORO_FMT, CORO_TURN_STR(turn), CORO_EXP(coro));

   eina_lock_take(&coro->lock);
   while (coro->turn != turn)
     eina_condition_wait(&coro->condition);
   eina_lock_release(&coro->lock);

   eina_main_loop_define();

   DBG("wait is over: turn=%s " CORO_FMT, CORO_TURN_STR(turn), CORO_EXP(coro));
}
#endif

static Eina_Bool
_eina_coro_hooks_coro_enter_and_get_canceled(Eina_Coro *coro)
{
   if (!_eina_coro_hooks_coro_enter(coro)) return EINA_TRUE;
   return coro->canceled;
}

#if USE_CORO_THREAD
static void *
_eina_coro_thread(void *data, Eina_Thread t EINA_UNUSED)
{
   Eina_Coro *coro = data;
   void *result = NULL;
   Eina_Bool canceled = EINA_FALSE;

   _eina_coro_wait(coro, EINA_CORO_TURN_COROUTINE);

   canceled = _eina_coro_hooks_coro_enter_and_get_canceled(coro);

   DBG("call (canceled=%hhu) " CORO_FMT, canceled, CORO_EXP(coro));
   result = (void *)coro->func((void *)coro->data, canceled, coro);
   DBG("finished with result=%p " CORO_FMT, result, CORO_EXP(coro));

   _eina_coro_hooks_coro_exit(coro);

   coro->finished = EINA_TRUE;
   _eina_coro_signal(coro, EINA_CORO_TURN_MAIN);

   return result;
}
#elif USE_CORO_FCONTEXT
static void
_eina_coro_coro(transfer_t continuation)
{
   Eina_Coro *coro = (Eina_Coro*) continuation.data;
   Eina_Bool canceled = EINA_FALSE;

   canceled = _eina_coro_hooks_coro_enter_and_get_canceled(coro);

   // Saves the context that called jump_fcontext and started this call.
   coro->main = continuation.ctx;
   coro->turn = EINA_CORO_TURN_COROUTINE;

   coro->result = (void*)coro->func((void *)coro->data, canceled, coro);

   _eina_coro_hooks_coro_exit(coro);

   coro->finished = EINA_TRUE;

   // Jump back to main explicitly.
   // fcontext by default exits after the context function finishes.
   ostd_jump_fcontext(coro->main, coro);
}
#endif

static Eina_Coro *
_eina_coro_alloc(void)
{
   Eina_Coro *coro = NULL;

   eina_lock_take(&_eina_coro_lock);
   if (EINA_UNLIKELY(!_eina_coro_mp))
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

        _eina_coro_mp = eina_mempool_add
          (choice, "coro", NULL, sizeof(Eina_Coro), 16);

        if (!_eina_coro_mp)
          {
             ERR("Mempool for coro cannot be allocated in coro init.");
             goto end;
          }
     }
   coro = eina_mempool_calloc(_eina_coro_mp, sizeof(Eina_Coro));
   if (coro) _eina_coro_usage++;

 end:
   eina_lock_release(&_eina_coro_lock);

   return coro;
}

static void
_eina_coro_free(Eina_Coro *coro)
{
   EINA_SAFETY_ON_NULL_RETURN(coro);

   eina_lock_take(&_eina_coro_lock);

   eina_mempool_free(_eina_coro_mp, coro);
   _eina_coro_usage--;
   if (_eina_coro_usage == 0)
     {
        eina_mempool_del(_eina_coro_mp);
        _eina_coro_mp = NULL;
     }

   eina_lock_release(&_eina_coro_lock);
}

EAPI Eina_Coro *
eina_coro_new(Eina_Coro_Cb func, const void *data, size_t stack_size)
{
   Eina_Coro *coro;
   Eina_Bool r;

   EINA_SAFETY_ON_NULL_RETURN_VAL(func, NULL);

   coro = _eina_coro_alloc();
   EINA_SAFETY_ON_NULL_RETURN_VAL(coro, NULL);

   coro->func = func;
   coro->data = data;
   r = eina_lock_new(&coro->lock);
   EINA_SAFETY_ON_FALSE_GOTO(r, failed_lock);
#if USE_CORO_THREAD
   r = eina_condition_new(&coro->condition, &coro->lock);
   EINA_SAFETY_ON_FALSE_GOTO(r, failed_condition);
   coro->main = eina_thread_self();
   coro->coroutine = 0;

   /* eina_thread_create() doesn't take attributes so we can set stack size */
   if (stack_size)
     DBG("currently stack size is ignored! Using thread default.");
#elif USE_CORO_FCONTEXT
   if (stack_size == 0)
     {
        struct rlimit limit;
        getrlimit(RLIMIT_STACK, &limit);
        stack_size = (size_t)limit.rlim_cur;
        printf("Setting stack size to %lu\n", stack_size);
     }
   // Setup ucontext_t pointers
   void *stack = mmap(NULL, stack_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
   mprotect(stack, getpagesize(), PROT_NONE);
   stack = (unsigned char *)stack + stack_size;
#if HAVE_VALGRIND
   coro->valgrind_stack_id = VALGRIND_STACK_REGISTER(stack - stack_size, stack);
#endif
   coro->stack = stack;
   coro->stack_size = stack_size;
   coro->coroutine = ostd_make_fcontext(stack, stack_size, _eina_coro_coro);
#endif
   coro->finished = EINA_FALSE;
   coro->canceled = EINA_FALSE;
   coro->turn = EINA_CORO_TURN_MAIN;

#if USE_CORO_THREAD
   if (!eina_thread_create(&coro->coroutine,
                           EINA_THREAD_NORMAL, -1,
                           _eina_coro_thread, coro))
     {
        ERR("could not create thread for " CORO_FMT, CORO_EXP(coro));
        goto failed_thread;
     }
#endif

   INF(CORO_FMT, CORO_EXP(coro));
   return coro;

#if USE_CORO_THREAD
 failed_thread:
   eina_condition_free(&coro->condition);
 failed_condition:
   eina_lock_free(&coro->lock);
#endif
 failed_lock:
   _eina_coro_free(coro);
   return NULL;
}

EAPI Eina_Bool
eina_coro_yield(Eina_Coro *coro)
{
   EINA_CORO_CHECK(coro, EINA_CORO_TURN_COROUTINE, EINA_FALSE);

   _eina_coro_hooks_coro_exit(coro);

#if USE_CORO_THREAD
   _eina_coro_signal(coro, EINA_CORO_TURN_MAIN);
   _eina_coro_wait(coro, EINA_CORO_TURN_COROUTINE);
#elif USE_CORO_FCONTEXT
   DBG("Jumping to switch to main context");
   transfer_t continuation = ostd_jump_fcontext(coro->main, coro);
   DBG("Returned to coro context from jump");
   // Save the point that called jump_fcontext to resume this coroutine.
   coro->main = continuation.ctx;
   coro->turn = EINA_CORO_TURN_COROUTINE;
#endif

   return !_eina_coro_hooks_coro_enter_and_get_canceled(coro);
}

EAPI Eina_Bool
eina_coro_run(Eina_Coro **p_coro, void **p_result, Eina_Future **p_awaiting)
{
   Eina_Coro *coro;

   if (p_result) *p_result = NULL;
   if (p_awaiting) *p_awaiting = NULL;

   EINA_SAFETY_ON_NULL_RETURN_VAL(p_coro, EINA_FALSE);
   EINA_CORO_CHECK(*p_coro, EINA_CORO_TURN_MAIN, EINA_FALSE);

   coro = *p_coro;

   _eina_coro_hooks_main_exit(coro);

#if USE_CORO_THREAD
   _eina_coro_signal(coro, EINA_CORO_TURN_COROUTINE);
   _eina_coro_wait(coro, EINA_CORO_TURN_MAIN);
#elif USE_CORO_FCONTEXT
   transfer_t continuation = ostd_jump_fcontext(coro->coroutine, coro);
   // Save the point where we should resume the coroutine.
   coro->coroutine = continuation.ctx;
   coro->turn = EINA_CORO_TURN_MAIN;
#endif

   _eina_coro_hooks_main_enter(coro);

   if (EINA_UNLIKELY(coro->finished)) {
      void *result;

#if USE_CORO_THREAD
      result = eina_thread_join(coro->coroutine);
#elif USE_CORO_FCONTEXT
      result = coro->result;
#if HAVE_VALGRIND
      VALGRIND_STACK_DEREGISTER(coro->valgrind_stack_id);
#endif
      mprotect(coro->stack - coro->stack_size, getpagesize(), PROT_READ|PROT_WRITE);
      munmap(coro->stack - coro->stack_size, coro->stack_size);
#endif
      INF("coroutine finished with result=%p " CORO_FMT,
          result, CORO_EXP(coro));
      if (p_result) *p_result = result;
      if (coro->awaiting) eina_future_cancel(coro->awaiting);
#if USE_CORO_THREAD
      eina_condition_free(&coro->condition);
#endif
      eina_lock_free(&coro->lock);
      _eina_coro_free(coro);
      *p_coro = NULL;
      return EINA_FALSE;
   }

   if (p_awaiting) *p_awaiting = coro->awaiting;

   DBG("coroutine yielded, must run again " CORO_FMT, CORO_EXP(coro));
   return EINA_TRUE;
}

typedef struct _Eina_Coro_Await_Data {
   Eina_Coro *coro;
   Eina_Value *p_value;
   Eina_Bool resolved;
} Eina_Coro_Await_Data;

static Eina_Value
_eina_coro_await_cb(void *data, const Eina_Value value, const Eina_Future *dead_future)
{
   Eina_Coro_Await_Data *d = data;

   DBG("future %p resolved with value type %p (%s) " CORO_FMT,
       dead_future, value.type, value.type ? value.type->name : "EMPTY",
       CORO_EXP(d->coro));

   if (d->p_value)
     {
        // copy is needed as value contents is flushed when this function returns.
        if (!value.type) *d->p_value = value;
        else if (!eina_value_copy(&value, d->p_value))
          {
             ERR("Value cannot be copied - unusable with Eina_Future: %p (%s)", value.type, value.type->name);
             eina_value_setup(d->p_value, EINA_VALUE_TYPE_ERROR);
             eina_value_set(d->p_value, ENOTSUP);
          }
     }

   d->resolved = EINA_TRUE;

   return value;
}

EAPI Eina_Bool
eina_coro_await(Eina_Coro *coro, Eina_Future *f, Eina_Value *p_value)
{
   Eina_Coro_Await_Data data = { coro, p_value, EINA_FALSE };

   if (p_value) *p_value = EINA_VALUE_EMPTY;

   EINA_CORO_CHECK_GOTO(coro, EINA_CORO_TURN_COROUTINE, no_coro);
   EINA_SAFETY_ON_TRUE_GOTO(coro->awaiting != NULL, no_coro);

   // storage will be NULL once future dies...
   f = eina_future_then(f, _eina_coro_await_cb, &data, &coro->awaiting);
   if (!f) return EINA_FALSE;

   INF("await future %p " CORO_FMT, f, CORO_EXP(coro));
   while (eina_coro_yield(coro) && !data.resolved)
     DBG("future %p still pending " CORO_FMT, f, CORO_EXP(coro));

   if (!data.resolved)
     {
        INF("future %p still pending and coroutine was canceled " CORO_FMT,
            f, CORO_EXP(coro));
        if (p_value)
          {
             eina_value_flush(p_value);
             *p_value = eina_value_error_init(ECANCELED);
          }
        return EINA_FALSE;
     }

   INF("future %p resolved! continue coroutine " CORO_FMT, f, CORO_EXP(coro));
   return EINA_TRUE;

 no_coro:
   if (p_value) *p_value = eina_value_error_init(EINVAL);
   eina_future_cancel(f);
   return EINA_FALSE;
}

EAPI void *
eina_coro_cancel(Eina_Coro **p_coro)
{
   void *result = NULL;
   Eina_Coro *coro;

   EINA_SAFETY_ON_NULL_RETURN_VAL(p_coro, NULL);
   EINA_CORO_CHECK(*p_coro, EINA_CORO_TURN_MAIN, NULL);

   coro = *p_coro;

   coro->canceled = EINA_TRUE;

   if (coro->awaiting) eina_future_cancel(coro->awaiting);

   DBG("marked as canceled, run so it can exit... " CORO_FMT, CORO_EXP(coro));
   while (eina_coro_run(p_coro, &result, NULL))
     DBG("did not exited, try running again..." CORO_FMT, CORO_EXP(coro));

   // be careful, coro is dead!
   INF("coroutine %p canceled and returned %p", coro, result);

   return result;
}

/**
 * @internal
 * @brief Initialize the coroutine module.
 *
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * This function sets up the coroutine module of Eina. It is called by
 * eina_init().
 *
 * This function creates mempool to speed up and keep safety of coro
 * handles, using EINA_MEMPOOL environment variable if it is set to
 * choose the memory pool type to use.
 *
 * @see eina_init()
 */
Eina_Bool
eina_coro_init(void)
{
   _eina_coro_log_dom = eina_log_domain_register("eina_coro",
                                                 EINA_LOG_COLOR_DEFAULT);
   if (_eina_coro_log_dom < 0)
     {
        EINA_LOG_ERR("Could not register log domain: eina_coro");
        return EINA_FALSE;
     }

   eina_lock_new(&_eina_coro_lock);
   _eina_coro_usage = 0;
   _eina_coro_hooks_walking = 0;
   eina_inarray_step_set(&_eina_coro_hooks, sizeof(_eina_coro_hooks),
                         sizeof(Eina_Coro_Hook), 1);

   return EINA_TRUE;
}

/**
 * @internal
 * @brief Shut down the coroutine module.
 *
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * This function shuts down the coroutine module set up by
 * eina_coro_init(). It is called by eina_shutdown().
 *
 * @see eina_shutdown()
 */
Eina_Bool
eina_coro_shutdown(void)
{
   eina_lock_take(&_eina_coro_lock);
   EINA_SAFETY_ON_TRUE_GOTO(_eina_coro_usage > 0, in_use);
   eina_lock_release(&_eina_coro_lock);
   eina_lock_free(&_eina_coro_lock);
   eina_inarray_flush(&_eina_coro_hooks);

   eina_log_domain_unregister(_eina_coro_log_dom);
   _eina_coro_log_dom = -1;
   return EINA_TRUE;

 in_use:
   eina_lock_release(&_eina_coro_lock);
   return EINA_FALSE;
}
