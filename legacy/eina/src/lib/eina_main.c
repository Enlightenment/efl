#include "Eina.h"

EAPI int
eina_init(void)
{
   int r;

   r = eina_error_init();
   r += eina_hash_init();
   r += eina_stringshare_init();

   return r;
}

EAPI int
eina_shutdown(void)
{
   int r;

   r = eina_stringshare_shutdown();
   r += eina_hash_shutdown();
   r += eina_error_shutdown();

   return r;
}

