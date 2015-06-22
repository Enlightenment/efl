#include <Ecore_Buffer_Queue.h>
#include "ecore_buffer_private.h"
#include "ecore_buffer_con.h"

int _ecore_buffer_queue_log_dom = -1;
static int _ecore_buffer_queue_init_count = 0;

EAPI int
ecore_buffer_queue_init(void)
{
   if (++_ecore_buffer_queue_init_count != 1)
     return _ecore_buffer_queue_init_count;

   _ecore_buffer_queue_log_dom =
      eina_log_domain_register("ecore_buffer_queue", EINA_COLOR_GREEN);

   if (_ecore_buffer_queue_log_dom < 0)
     {
        EINA_LOG_ERR("Could not register log domain: ecore_buffer_queue");
        goto err;
     }

#ifdef DEBUG
   eina_log_abort_on_critical_level_set(EINA_LOG_LEVEL_ERR);
   eina_log_abort_on_critical_set(EINA_TRUE);
#endif

   DBG("Ecore_Buffer_Queue Init");

   if (!_ecore_buffer_con_init())
     {
        eina_log_domain_unregister(_ecore_buffer_queue_log_dom);
        _ecore_buffer_queue_log_dom = -1;
        goto err;
     }

   return _ecore_buffer_queue_init_count;
err:
   return --_ecore_buffer_queue_init_count;
}

EAPI int
ecore_buffer_queue_shutdown(void)
{
   if (--_ecore_buffer_queue_init_count != 0)
     return _ecore_buffer_queue_init_count;

   DBG("Ecore_Buffer_Queue Shutdown");
   _ecore_buffer_con_shutdown();
   eina_log_domain_unregister(_ecore_buffer_queue_log_dom);
   _ecore_buffer_queue_log_dom = -1;

   return _ecore_buffer_queue_init_count;
}
