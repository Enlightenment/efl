#ifndef _EVAS_SCRIPT_UTILS
#define _EVAS_SCRIPT_UTILS

#include <Eina.h>
#include "evas_bidi_utils.h"

int
evas_common_script_end_of_run_get(const Eina_Unicode *str, const Evas_BiDi_Paragraph_Props *bidi_props, size_t start, int len);

#endif

