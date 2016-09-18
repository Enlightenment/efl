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
