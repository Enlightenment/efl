#ifndef _EFL_H
#define _EFL_H

#if defined ( __cplusplus )
extern "C" {
#endif

#include <Eo.h>

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef EFL_EFL_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif /* ! DLL_EXPORT */
# else
#  define EAPI __declspec(dllimport)
# endif /* ! EFL_EFL_BUILD */
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EAPI __attribute__ ((visibility("default")))
#  else
#   define EAPI
#  endif
# else
#  define EAPI
# endif
#endif /* ! _WIN32 */



#ifdef EFL_BETA_API_SUPPORT

#include <Efl_Model_Common.h>

/* Data types */
#include "interfaces/efl_gfx_types.eot.h"
typedef Efl_Gfx_Path_Command_Type Efl_Gfx_Path_Command;

/* Interfaces */
#include "interfaces/efl_control.eo.h"
#include "interfaces/efl_file.eo.h"
#include "interfaces/efl_image.eo.h"
#include "interfaces/efl_player.eo.h"
#include "interfaces/efl_text.eo.h"
#include "interfaces/efl_text_properties.eo.h"

/* Core interface */
#include "interfaces/efl_core_animator.eo.h"

EAPI extern const Eo_Event_Description _EFL_GFX_CHANGED;
EAPI extern const Eo_Event_Description _EFL_GFX_PATH_CHANGED;

#define EFL_GFX_CHANGED (&(_EFL_GFX_CHANGED))
#define EFL_GFX_PATH_CHANGED (&(_EFL_GFX_PATH_CHANGED))

#include "interfaces/efl_gfx_base.eo.h"
#include "interfaces/efl_gfx_stack.eo.h"
#include "interfaces/efl_gfx_fill.eo.h"
#include "interfaces/efl_gfx_view.eo.h"
#include "interfaces/efl_gfx_shape.eo.h"
#include "interfaces/efl_gfx_gradient_base.eo.h"
#include "interfaces/efl_gfx_gradient_linear.eo.h"
#include "interfaces/efl_gfx_gradient_radial.eo.h"
#include "interfaces/efl_gfx_filter.eo.h"

#define EFL_GFX_COLOR_SET(value) (value << 8)
#define EFL_GFX_COLOR16_SET(value) (value)

static inline void efl_gfx_color_type_set(Efl_Gfx_Color *color,
                                          unsigned char r, unsigned char g,
                                          unsigned char b, unsigned char a);

static inline void efl_gfx_color16_type_set(Efl_Gfx_Color *color,
                                            unsigned short r, unsigned short g,
                                            unsigned short b, unsigned short a);

#include "interfaces/efl_gfx.x"

#else

#ifndef EFL_NOLEGACY_API_SUPPORT
#include "interfaces/efl_gfx_types.eot.h"
#include "interfaces/efl_gfx_fill.eo.legacy.h"
#include "interfaces/efl_gfx_base.eo.legacy.h"
#endif

typedef Efl_Gfx_Path_Command_Type Efl_Gfx_Path_Command;

#endif

#if defined ( __cplusplus )
}
#endif

#undef EAPI
#define EAPI

#endif
