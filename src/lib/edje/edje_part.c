#include "edje_private.h"
#include "edje_part_helper.h"

PROXY_INIT(box)
PROXY_INIT(table)
PROXY_INIT(swallow)

void
_edje_internal_proxy_shutdown(void)
{
   _box_shutdown();
   _table_shutdown();
   _swallow_shutdown();
}

/* Internal EO API */

EOAPI EFL_VOID_FUNC_BODYV(_efl_canvas_layout_internal_real_part_set, EFL_FUNC_CALL(ed, rp, part), void *ed, void *rp, const char *part)

#define EFL_CANVAS_LAYOUT_INTERNAL_EXTRA_OPS \
      EFL_OBJECT_OP_FUNC(_efl_canvas_layout_internal_real_part_set, NULL)

#include "efl_canvas_layout_internal.eo.c"
