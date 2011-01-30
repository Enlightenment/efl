/**
 * @internal
 * @addtogroup Evas_Utils
 *
 * @{
 */
/**
 * @internal
 * @defgroup Evas_Script Evas Script (language) utility functions
 *
 * This set of functions and types helps evas handle scripts correctly.
 * @todo Document types, structures and macros.
 *
 * @{
 */
#include <Eina.h>

#include "evas_script_utils.h"
#include "evas_bidi_utils.h" /* Used for fallback. */
int
evas_common_script_end_of_run_get(const Eina_Unicode *str,
      const Evas_BiDi_Paragraph_Props *bidi_props, size_t start, int len)
{
   /* FIXME: Currently we fall back to bidi runs, should fix */
   (void) str;
#ifdef BIDI_SUPPORT
   return evas_bidi_end_of_run_get(bidi_props, start, len);
#else
   (void) bidi_props;
   (void) start;
   (void) len;
   return 0;
#endif
}

/*
 * @}
 */
/*
 * @}
 */

