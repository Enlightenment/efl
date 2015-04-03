#ifndef _EFL_H
#define _EFL_H

#if defined ( __cplusplus )
extern "C"
{
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

/**
 * Path command enum.
 *
 * @since 1.13
 * @ingroup Efl_Gfx_Shape
 */
typedef enum _Efl_Gfx_Path_Command
{
  EFL_GFX_PATH_COMMAND_TYPE_END = 0, /**< End of the stream of command */
  EFL_GFX_PATH_COMMAND_TYPE_MOVE_TO, /**< A move command type */
  EFL_GFX_PATH_COMMAND_TYPE_LINE_TO, /**< A line command type */
  EFL_GFX_PATH_COMMAND_TYPE_CUBIC_TO, /**< A cubic command type */
  EFL_GFX_PATH_COMMAND_TYPE_CLOSE, /**< A close command type */
  EFL_GFX_PATH_COMMAND_TYPE_LAST, /**< Not a valid command, but last one according to this version header */
} Efl_Gfx_Path_Command;

/**
 * Type describing dash
 * @since 1.13
 */
typedef struct _Efl_Gfx_Dash Efl_Gfx_Dash;
struct _Efl_Gfx_Dash
{
   double length;
   double gap;
};

/**
 * Type defining how a line end.
 * @since 1.13
 */
typedef enum _Efl_Gfx_Cap
{
  EFL_GFX_CAP_BUTT,
  EFL_GFX_CAP_ROUND,
  EFL_GFX_CAP_SQUARE,
  EFL_GFX_CAP_LAST
} Efl_Gfx_Cap;

/**
 * Type defining how join between path are drawn.
 * @since 1.13
 */
typedef enum _Efl_Gfx_Join
{
  EFL_GFX_JOIN_MITER,
  EFL_GFX_JOIN_ROUND,
  EFL_GFX_JOIN_BEVEL,
  EFL_GFX_JOIN_LAST
} Efl_Gfx_Join;

/**
 * Type defining gradient stop.
 * @since 1.13
 */
typedef struct _Efl_Gfx_Gradient_Stop Efl_Gfx_Gradient_Stop;
struct _Efl_Gfx_Gradient_Stop
{
   double offset;
   int r;
   int g;
   int b;
   int a;
};

/**
 * Type defining how the gradient spread after its limit.
 * @since 1.13
 */
typedef enum _Efl_Gfx_Gradient_Spread
{
  EFL_GFX_GRADIENT_SPREAD_PAD,
  EFL_GFX_GRADIENT_SPREAD_REFLECT,
  EFL_GFX_GRADIENT_SPREAD_REPEAT,
  EFL_GFX_GRADIENT_SPREAD_LAST
} Efl_Gfx_Gradient_Spread;

/**
 * Type defining how an image content get filled.
 * @since 1.13
 */
typedef enum _Efl_Gfx_Fill_Spread
{
  EFL_GFX_FILL_REFLECT = 0, /**< image fill tiling mode - tiling reflects */
  EFL_GFX_FILL_REPEAT = 1,  /**< tiling repeats */
  EFL_GFX_FILL_RESTRICT = 2, /**< tiling clamps - range offset ignored */
  EFL_GFX_FILL_RESTRICT_REFLECT = 3, /**< tiling clamps and any range offset reflects */
  EFL_GFX_FILL_RESTRICT_REPEAT = 4, /**< tiling clamps and any range offset repeats */
  EFL_GFX_FILL_PAD = 5 /**< tiling extends with end values */
} Efl_Gfx_Fill_Spread;

#ifdef EFL_BETA_API_SUPPORT

/* Interfaces */
#include "interfaces/efl_control.eo.h"
#include "interfaces/efl_file.eo.h"
#include "interfaces/efl_image.eo.h"
#include "interfaces/efl_player.eo.h"
#include "interfaces/efl_text.eo.h"
#include "interfaces/efl_text_properties.eo.h"

#include "interfaces/efl_gfx_utils.h"

#include "interfaces/efl_gfx_base.eo.h"
#include "interfaces/efl_gfx_stack.eo.h"
#include "interfaces/efl_gfx_fill.eo.h"
#include "interfaces/efl_gfx_view.eo.h"
#include "interfaces/efl_gfx_shape.eo.h"
#include "interfaces/efl_gfx_gradient_base.eo.h"
#include "interfaces/efl_gfx_gradient_linear.eo.h"
#include "interfaces/efl_gfx_gradient_radial.eo.h"

#endif

#if defined ( __cplusplus )
}
#endif

#endif
