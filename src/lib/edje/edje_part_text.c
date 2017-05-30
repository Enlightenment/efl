#include "edje_private.h"
#include "edje_part_helper.h"
#include "efl_canvas_layout_internal_text.eo.h"
#define MY_CLASS EFL_CANVAS_LAYOUT_INTERNAL_TEXT_CLASS

PROXY_IMPLEMENTATION(text, INTERNAL_TEXT, EINA_FALSE)
#undef PROXY_IMPLEMENTATION

EOLIAN static void
_efl_canvas_layout_internal_text_efl_text_text_set(Eo *obj,
      void *_pd EINA_UNUSED, const char *text)
{
   PROXY_DATA_GET(obj, pd);
   _edje_efl_text_set(obj, pd->ed, pd->part, text);
}

EOLIAN static const char *
_efl_canvas_layout_internal_text_efl_text_text_get(Eo *obj,
      void *_pd EINA_UNUSED)
{
   PROXY_DATA_GET(obj, pd);
   RETURN_VAL(_edje_efl_text_get(obj, pd->ed, pd->part));
}

#include "efl_canvas_layout_internal_text.eo.c"

