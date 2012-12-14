typedef struct _Eina_Barrier Eina_Barrier;

struct _Eina_Barrier
{
   int needed, called;
   Eina_Lock cond_lock;
   Eina_Condition cond;
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
