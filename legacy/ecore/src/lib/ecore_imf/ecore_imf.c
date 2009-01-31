/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Ecore.h>

#include "Ecore_IMF.h"
#include "ecore_imf_private.h"

EAPI int ECORE_IMF_EVENT_PREEDIT_START = 0;
EAPI int ECORE_IMF_EVENT_PREEDIT_END = 0;
EAPI int ECORE_IMF_EVENT_PREEDIT_CHANGED = 0;
EAPI int ECORE_IMF_EVENT_COMMIT = 0;
EAPI int ECORE_IMF_EVENT_DELETE_SURROUNDING = 0;

static int init_count = 0;

/**
 * @defgroup Ecore_IMF_Lib_Group Ecore Input Method Library Functions
 *
 * Utility functions that set up and shut down the Ecore Input Method
 * library.
 */

/**
 * Initialises the Ecore_IMF library.
 * @return  Number of times the library has been initialised without being
 *          shut down.
 * @ingroup Ecore_IMF_Lib_Group
 */
EAPI int
ecore_imf_init(void)
{
   if (++init_count != 1) return init_count;

   ecore_init();
   ecore_imf_module_init();

   ECORE_IMF_EVENT_PREEDIT_START = ecore_event_type_new();
   ECORE_IMF_EVENT_PREEDIT_END = ecore_event_type_new();
   ECORE_IMF_EVENT_PREEDIT_CHANGED = ecore_event_type_new();
   ECORE_IMF_EVENT_COMMIT = ecore_event_type_new();
   ECORE_IMF_EVENT_DELETE_SURROUNDING = ecore_event_type_new();

   return init_count;
}

/**
 * Shuts down the Ecore_IMF library.
 * @return  Number of times the library has been initialised without being
 *          shut down.
 * @ingroup Ecore_IMF_Lib_Group
 */
EAPI int
ecore_imf_shutdown(void)
{
   if (--init_count != 0) return init_count;

   ecore_shutdown();
   ecore_imf_module_shutdown();

   return init_count;
}
