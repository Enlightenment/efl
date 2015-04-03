#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Efl.h>

#include "interfaces/efl_control.eo.c"
#include "interfaces/efl_file.eo.c"
#include "interfaces/efl_image.eo.c"
#include "interfaces/efl_player.eo.c"
#include "interfaces/efl_text.eo.c"
#include "interfaces/efl_text_properties.eo.c"

#include "interfaces/efl_gfx_base.eo.c"
#include "interfaces/efl_gfx_stack.eo.c"
#include "interfaces/efl_gfx_fill.eo.c"
#include "interfaces/efl_gfx_view.eo.c"

#include "interfaces/efl_gfx_gradient_base.eo.c"
#include "interfaces/efl_gfx_gradient_linear.eo.c"
#include "interfaces/efl_gfx_gradient_radial.eo.c"

EAPI const Eo_Event_Description _EFL_GFX_CHANGED =
  EO_EVENT_DESCRIPTION("Graphics changed", "The visual representation of the object changed");
