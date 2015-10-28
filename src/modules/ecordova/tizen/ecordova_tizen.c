#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Ecore.h>

static Eina_Bool
_ecordova_tizen_init(void)
{
  return EINA_TRUE;
}

static void
_ecordova_tizen_shutdown(void)
{
}

EINA_MODULE_INIT(_ecordova_system_tizen_init);
EINA_MODULE_SHUTDOWN(_ecordova_system_tizen_shutdown);
