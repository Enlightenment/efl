#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "eina_config.h"
#include "eina_types.h"
#include <evil_private.h>

void free_thread(void);

BOOL WINAPI
DllMain(HINSTANCE inst EINA_UNUSED, WORD reason, PVOID reserved EINA_UNUSED)
{
   if (DLL_THREAD_DETACH == reason)
      free_thread();

   return TRUE;
}
