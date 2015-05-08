#include "eina_debug.h"
#include "eina_types.h"

#ifdef EINA_HAVE_DEBUG

extern pthread_t     _eina_debug_thread_mainloop;
extern pthread_t    *_eina_debug_thread_active;
extern int           _eina_debug_thread_active_num;


// yes - a global debug spinlock. i expect contention to be low for now, and
// when needed we can split this up into mroe locks to reduce contention when
// and if that day comes
Eina_Spinlock _eina_debug_lock;

static Eina_Bool _inited = EINA_FALSE;

Eina_Bool
eina_debug_init(void)
{
   pthread_t self;

   if (_inited)
     {
        eina_spinlock_release(&_eina_debug_thread_lock);
        return EINA_TRUE;
     }
   _inited = EINA_TRUE;
   eina_spinlock_new(&_eina_debug_lock);
   eina_spinlock_new(&_eina_debug_thread_lock);
   eina_semaphore_new(&_eina_debug_monitor_return_sem, 0);
   self = pthread_self();
   _eina_debug_thread_mainloop_set(&self);
#if defined(HAVE_GETUID) && defined(HAVE_GETEUID)
   // if we are setuid - don't debug!
   if (getuid() != geteuid()) return EINA_TRUE;
#endif
   if (getenv("EFL_NODEBUG")) return EINA_TRUE;
   _eina_debug_monitor_service_connect();
   if (_eina_debug_monitor_service_fd >= 0)
     {
        _eina_debug_monitor_service_greet();
        _eina_debug_monitor_signal_init();
        _eina_debug_monitor_thread_start();
     }
   return EINA_TRUE;
}

Eina_Bool
eina_debug_shutdown(void)
{
   eina_spinlock_take(&_eina_debug_thread_lock);
   // yes - we never free on shutdown - this is because the monitor thread
   // never exits. this is not a leak - we intend to never free up any
   // resources here because they are allocated once only ever.
   return EINA_TRUE;
}
#else
Eina_Bool
eina_debug_init(void)
{
   return EINA_TRUE;
}

Eina_Bool
eina_debug_shutdown(void)
{
   return EINA_TRUE;
}
#endif
