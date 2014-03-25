typedef struct _Eina_Barrier Eina_Barrier;

/** @private */
struct _Eina_Barrier
{
   int needed;           /**< The number of waiting threads that will cause the barrier to signal and reset. */ 
   int called;           /**< The number of threads that are waiting on this barrier. */
   Eina_Lock cond_lock;  /**< The lock for the barrier */
   Eina_Condition cond;  /**< The condition variable for the barrier */
};

static inline Eina_Bool
eina_barrier_new(Eina_Barrier *barrier, int needed)
{
   barrier->needed = needed;
   barrier->called = 0;
   if (!eina_lock_new(&(barrier->cond_lock)))
     return EINA_FALSE;
   if (!eina_condition_new(&(barrier->cond), &(barrier->cond_lock)))
     return EINA_FALSE;
   return EINA_TRUE;
}

static inline void
eina_barrier_free(Eina_Barrier *barrier)
{
   eina_condition_free(&(barrier->cond));
   eina_lock_free(&(barrier->cond_lock));
   barrier->needed = 0;
   barrier->called = 0;
}

static inline Eina_Bool
eina_barrier_wait(Eina_Barrier *barrier)
{
   eina_lock_take(&(barrier->cond_lock));
   barrier->called++;
   if (barrier->called == barrier->needed)
     {
        barrier->called = 0;
        eina_condition_broadcast(&(barrier->cond));
     }
   else
     eina_condition_wait(&(barrier->cond));
   eina_lock_release(&(barrier->cond_lock));
   return EINA_TRUE;
}
