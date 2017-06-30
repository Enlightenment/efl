#ifdef EFL_BETA_API_SUPPORT

#ifndef EFL_WL_H
# define EFL_WL_H
#include <Evas.h>
#include <Ecore.h>

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef EFL_EINA_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif /* ! DLL_EXPORT */
# else
#  define EAPI __declspec(dllimport)
# endif /* ! EFL_EINA_BUILD */
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
#endif

/**
 * @defgroup Efl_Wl_Group EFL Wayland
 *
 * A multiseat Wayland compositor in an Evas object.
 * All toplevel windows will be sized to the size of the compositor object.
 * @since 1.20
 * @{
 */

/**
 * @typedef Efl_Wl_Rotation
 * The rotation to apply to the compositor's internal wl_output
 */
typedef enum
{
   EFL_WL_ROTATION_0,
   EFL_WL_ROTATION_90,
   EFL_WL_ROTATION_180,
   EFL_WL_ROTATION_270
} Efl_Wl_Rotation;

/**
 * Add a compositor widget to the given canvas.
 *
 * @param e The canvas
 * @return The compositor object, @c NULL on failure
 */
EAPI Evas_Object *efl_wl_add(Evas *e);

/**
 * Run a command in the compositor widget.
 *
 * @note If GL is available, the ELM_ACCEL environment variable will be
 * set to "gl" while executing the command.
 *
 * @param obj The compositor widget
 * @param cmd The command to run
 * @return The Ecore_Exe from the executed process, @c NULL on failure
 */
EAPI Ecore_Exe *efl_wl_run(Evas_Object *obj, const char *cmd);

/**
 * Put the bottom-most toplevel window on top and apply focus to it
 *
 * @param obj The compositor widget
 * @return EINA_TRUE if the window stacking was changed
 */
EAPI Eina_Bool efl_wl_next(Evas_Object *obj);

/**
 * Put the second top-most toplevel window on top and apply focus to it
 *
 * @param obj The compositor widget
 * @return EINA_TRUE if the window stacking was changed
 */
EAPI Eina_Bool efl_wl_prev(Evas_Object *obj);

/**
 * Set rotation and flip for the compositor's output
 *
 * @param obj The compositor widget
 * @param rot The rotation to apply
 * @param rtl If set, the output will apply a flip around the Y axis
 * @note rtl is equivalent to WL_OUTPUT_TRANSFORM_FLIPPED and rotations are applied
 * on top
 */
EAPI void efl_wl_rotate(Evas_Object *obj, Efl_Wl_Rotation rot, Eina_Bool rtl);

/**
 * Set the scale factor for the compositor's output
 *
 * @param obj The compositor widget
 * @param scale The scale factor to set
 */
EAPI void efl_wl_scale_set(Evas_Object *obj, double scale);
#endif

#endif
