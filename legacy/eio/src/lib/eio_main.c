#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "eio_private.h"

#include "Eio.h"

static int _eio_count = 0;

EAPI int
eio_init(void)
{
   _eio_count++;

   if (_eio_count > 1) return _eio_count;

   eina_init();
   ecore_init();

   return _eio_count;
}

EAPI int
eio_shutdown(void)
{
   _eio_count--;

   if (_eio_count > 0) return _eio_count;

   ecore_shutdown();
   eina_shutdown();
   return _eio_count;
}
