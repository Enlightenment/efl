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

#include "efl_canvas_layout_internal.eo.c"
