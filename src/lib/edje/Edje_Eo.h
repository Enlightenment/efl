#include <Efl.h>

#include "efl_canvas_layout_calc.eo.h"
#include "efl_canvas_layout_signal.eo.h"
#include "efl_canvas_layout_group.eo.h"
#include "edje_object.eo.h"
#include "edje_edit.eo.h"

#include "efl_canvas_layout_part.eo.h"
#include "efl_canvas_layout_part_box.eo.h"
#include "efl_canvas_layout_part_table.eo.h"
#include "efl_canvas_layout_part_swallow.eo.h"
#include "efl_canvas_layout_part_text.eo.h"
#include "efl_canvas_layout_part_external.eo.h"

#ifdef __cplusplus
extern "C" {
#endif

/** A helper for efl_canvas_layout_signal_cb_add for C. */
static inline Eina_Bool
efl_canvas_layout_signal_callback_add(Eo *obj,
                                      const char *emission, const char *source,
                                      Efl_Signal_Cb func, void *data)
{
   Efl_Signal_Cb_Connection connection =
         efl_canvas_layout_signal_cb_add(obj, emission, source, data, func, NULL);
   return (connection.func_ptr != NULL);
}

/** A helper for efl_canvas_layout_signal_cb_del for C. */
static inline Eina_Bool
efl_canvas_layout_signal_callback_del(Eo *obj,
                                      const char *emission, const char *source,
                                      Efl_Signal_Cb func, void *data)
{
   const Efl_Signal_Cb_Connection connection = { emission, source,
                                                 (const void *) func,
                                                 (const void *) data };
   return efl_canvas_layout_signal_cb_del(obj, &connection);
}

#ifdef __cplusplus
}
#endif
