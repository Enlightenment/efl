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
 * @ingroup Efl_Geometry_Shape
 */
typedef enum _Efl_Geometry_Path_Command
{
  EFL_GEOMETRY_PATH_COMMAND_TYPE_END = 0, /**< End of the stream of command */
  EFL_GEOMETRY_PATH_COMMAND_TYPE_MOVE_TO, /**< A move command type */
  EFL_GEOMETRY_PATH_COMMAND_TYPE_LINE_TO, /**< A line command type */
  EFL_GEOMETRY_PATH_COMMAND_TYPE_QUADRATIC_TO, /**< A quadratic command type */
  EFL_GEOMETRY_PATH_COMMAND_TYPE_SQUADRATIC_TO, /**< A smooth quadratic command type */
  EFL_GEOMETRY_PATH_COMMAND_TYPE_CUBIC_TO, /**< A cubic command type */
  EFL_GEOMETRY_PATH_COMMAND_TYPE_SCUBIC_TO, /**< A smooth cubic command type */
  EFL_GEOMETRY_PATH_COMMAND_TYPE_ARC_TO, /**< An arc command type */
  EFL_GEOMETRY_PATH_COMMAND_TYPE_CLOSE, /**< A close command type */
  EFL_GEOMETRY_PATH_COMMAND_TYPE_LAST, /**< Not a valid command, but last one according to this version header */
} Efl_Geometry_Path_Command;

/**
 * Type describing dash
 * @since 1.13
 */
typedef struct _Efl_Geometry_Dash Efl_Geometry_Dash;
struct _Efl_Geometry_Dash
{
   double length;
   double gap;
};

/**
 * Type defining how a line end.
 * @since 1.13
 */
typedef enum _Efl_Geometry_Cap
{
  EFL_GEOMETRY_CAP_BUTT,
  EFL_GEOMETRY_CAP_ROUND,
  EFL_GEOMETRY_CAP_SQUARE,
  EFL_GEOMETRY_CAP_LAST
} Efl_Geometry_Cap;

/**
 * Type defining how join between path are drawn.
 * @since 1.13
 */
typedef enum _Efl_Geometry_Join
{
  EFL_GEOMETRY_JOIN_MITER,
  EFL_GEOMETRY_JOIN_ROUND,
  EFL_GEOMETRY_JOIN_BEVEL,
  EFL_GEOMETRY_JOIN_LAST
} Efl_Geometry_Join;

#ifdef EFL_BETA_API_SUPPORT

/* Interfaces */
#include "interfaces/efl_control.eo.h"
#include "interfaces/efl_file.eo.h"
#include "interfaces/efl_image.eo.h"
#include "interfaces/efl_player.eo.h"
#include "interfaces/efl_text.eo.h"
#include "interfaces/efl_text_properties.eo.h"

#include "interfaces/efl_geometry_shape.eo.h"

#endif

#if defined ( __cplusplus )
}
#endif

#endif
