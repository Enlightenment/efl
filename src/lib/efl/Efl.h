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

/**
 * These values determine how the points are interpreted in a stream of points.
 *
 * @since 1.14
 */
typedef enum _Efl_Gfx_Path_Command
{
  EFL_GFX_PATH_COMMAND_TYPE_END = 0, /**< The end of stream , no more points to process. */
  EFL_GFX_PATH_COMMAND_TYPE_MOVE_TO, /**< The next point is the start point of a sub path */
  EFL_GFX_PATH_COMMAND_TYPE_LINE_TO, /**< The next point is used to draw a line from current point */
  EFL_GFX_PATH_COMMAND_TYPE_CUBIC_TO, /**< The next three point is used to draw a cubic bezier curve from current point */
  EFL_GFX_PATH_COMMAND_TYPE_CLOSE, /**< Close the curent subpath by drawing a line between current point and the first point of current subpath */
  EFL_GFX_PATH_COMMAND_TYPE_LAST, /**< Not a valid command, but last one according to this version header */
} Efl_Gfx_Path_Command;

/**
 * Type describing dash
 *
 * @see efl_gfx_shape_stroke_dash_set()
 *
 * @since 1.14
 */
typedef struct _Efl_Gfx_Dash Efl_Gfx_Dash;
struct _Efl_Gfx_Dash
{
   double length; /**< dash drawing length */
   double gap; /**< distance bettwen two dashes */
};

/**
 * These values determine how the end of opened sub-paths are rendered in a
 * stroke.
 *
 * @see efl_gfx_shape_stroke_cap_set()
 *
 * @since 1.14
 */
typedef enum _Efl_Gfx_Cap
{
  EFL_GFX_CAP_BUTT = 0, /**< The end of lines is rendered as a full stop on the last point itself */
  EFL_GFX_CAP_ROUND, /**< The end of lines is rendered as a half-circle around the last point */
  EFL_GFX_CAP_SQUARE, /**< The end of lines is rendered as a square around the last point */
  EFL_GFX_CAP_LAST /**< End of enum value */
} Efl_Gfx_Cap;

/**
 * These values determine how two joining lines are rendered in a stroker.
 *
 * @see efl_gfx_shape_stroke_join_set()
 *
 * @since 1.14
 */
typedef enum _Efl_Gfx_Join
{
  EFL_GFX_JOIN_MITER = 0, /**< Used to render rounded line joins. Circular arcs are used to join two lines smoothly. */
  EFL_GFX_JOIN_ROUND, /**< Used to render beveled line joins. The outer corner of the joined lines is filled by enclosing the triangular region of the corner with a straight line between the outer corners of each stroke. */
  EFL_GFX_JOIN_BEVEL, /**< Used to render mitered line joins. The intersection of the strokes is clipped at a line perpendicular to the bisector of the angle between the strokes, at the distance from the intersection of the segments equal to the product of the miter limit value and the border radius.  This prevents long spikes being created. */
  EFL_GFX_JOIN_LAST /**< End of enum value */
} Efl_Gfx_Join;

/**
 * Type defining gradient stop.
 * @since 1.14
 */
typedef struct _Efl_Gfx_Gradient_Stop Efl_Gfx_Gradient_Stop;
struct _Efl_Gfx_Gradient_Stop
{
   double offset; /**< The location of the gradient stop within the gradient vector*/
   int r; /**< The component R color of the gradient stop */
   int g; /**< The component G color of the gradient stop */
   int b; /**< The component B color of the graident stop */
   int a; /**< The component A color of the graident stop */
};

/**
 * Specifies how the area outside the gradient area should be filled.
 *
 * @see efl_gfx_gradient_spread_set()
 *
 * @since 1.14
 */
typedef enum _Efl_Gfx_Gradient_Spread
{
  EFL_GFX_GRADIENT_SPREAD_PAD, /**< The area is filled with the closest stop color. This is the default. */
  EFL_GFX_GRADIENT_SPREAD_REFLECT, /**< The gradient is reflected outside the gradient area. */
  EFL_GFX_GRADIENT_SPREAD_REPEAT, /**< The gradient is repeated outside the gradient area. */
  EFL_GFX_GRADIENT_SPREAD_LAST /**< End of enum value */
} Efl_Gfx_Gradient_Spread;

/**
 * Type defining how an image content get filled.
 * @since 1.14
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

#include <Efl_Model_Common.h>
  
/* Interfaces */
#include "interfaces/efl_control.eo.h"
#include "interfaces/efl_file.eo.h"
#include "interfaces/efl_image.eo.h"
#include "interfaces/efl_player.eo.h"
#include "interfaces/efl_text.eo.h"
#include "interfaces/efl_text_properties.eo.h"
#include "interfaces/efl_model_base.eo.h"

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

#endif

#if defined ( __cplusplus )
}
#endif

#undef EAPI
#define EAPI

#endif
