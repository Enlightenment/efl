#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Efl.h>

#include "interfaces/efl_control.eo.c"
#include "interfaces/efl_file.eo.c"
#include "interfaces/efl_image.eo.c"
#include "interfaces/efl_image_animated.eo.c"
#include "interfaces/efl_image_load.eo.c"
#include "interfaces/efl_part.eo.c"
#include "interfaces/efl_player.eo.c"
#include "interfaces/efl_text.eo.c"
#include "interfaces/efl_text_properties.eo.c"

#include "interfaces/efl_gfx.eo.c"
#include "interfaces/efl_gfx_buffer.eo.c"
#include "interfaces/efl_gfx_stack.eo.c"
#include "interfaces/efl_gfx_fill.eo.c"
#include "interfaces/efl_gfx_view.eo.c"

#include "interfaces/efl_gfx_gradient.eo.c"
#include "interfaces/efl_gfx_gradient_linear.eo.c"
#include "interfaces/efl_gfx_gradient_radial.eo.c"

#include "interfaces/efl_gfx_filter.eo.c"

#include "interfaces/efl_vpath.eo.c"

EAPI const Eo_Event_Description _EFL_GFX_CHANGED =
  EO_EVENT_DESCRIPTION("Graphics changed");

EAPI const Eo_Event_Description _EFL_GFX_PATH_CHANGED =
  EO_EVENT_DESCRIPTION("Graphics path changed");

#include "interfaces/efl_container.eo.c"
#include "interfaces/efl_pack.eo.c"
#include "interfaces/efl_pack_layout.eo.c"
#include "interfaces/efl_pack_linear.eo.c"
#include "interfaces/efl_pack_grid.eo.c"

#include "interfaces/efl_model.eo.c"
#include "interfaces/efl_animator.eo.c"
#include "interfaces/efl_orientation.eo.c"
#include "interfaces/efl_flipable.eo.c"
#include "interfaces/efl_ui_spin.eo.c"
#include "interfaces/efl_ui_progress.eo.c"

EAPI void
__efl_internal_init(void)
{
   efl_model_init();
}
