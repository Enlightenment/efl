#ifdef EFL_BETA_API_SUPPORT

#ifndef EFL_WL_H
# define EFL_WL_H
#include <Evas.h>
#include <Ecore.h>

#ifdef EAPI
# undef EAPI
#endif

#ifdef __GNUC__
# if __GNUC__ >= 4
#  define EAPI __attribute__ ((visibility("default")))
# else
#  define EAPI
# endif
#else
# define EAPI
#endif

/**
 * @defgroup Efl_Wl_Group EFL Wayland
 *
 * A multiseat xdg-shell compliant Wayland compositor in an Evas object.
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
 * The following smart callbacks will trigger on the compositor object:
 * "child_added" - A toplevel surface with a parent has been added; event info is Evas_Object *surface
 * "popup_added" - A popup surface has been added; event info is Evas_Object *surface
 * "seat_added" - A compositor seat has been added; event info is Eo *dev
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
 * Run a command in the compositor widget with specified flags.
 *
 * @note If GL is available, the ELM_ACCEL environment variable will be
 * set to "gl" while executing the command.
 *
 * @param obj The compositor widget
 * @param cmd The command to run
 * @param flags The flags to use
 * @return The Ecore_Exe from the executed process, @c NULL on failure
 */
Ecore_Exe *efl_wl_flags_run(Evas_Object *obj, const char *cmd, Ecore_Exe_Flags flags);

/**
 * Add a process to the list of allowed clients for the compositor widget
 *
 * @param obj The compositor widget
 * @param pid The process to allow
 * @since 1.21
 */
EAPI void efl_wl_pid_add(Evas_Object *obj, int32_t pid);

/**
 * Remove a process from the list of allowed clients for the compositor widget
 *
 * @param obj The compositor widget
 * @param pid The process to deny
 * @since 1.21
 */
EAPI void efl_wl_pid_del(Evas_Object *obj, int32_t pid);

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

/**
 * Transfer aspect hints from top-most surface onto the efl_wl object
 *
 * @param obj The compositor widget
 * @param set Whether to enable aspect setting
 * @since 1.21
 */
EAPI void efl_wl_aspect_set(Evas_Object *obj, Eina_Bool set);

/**
 * Transfer min/max hints from top-most surface onto the efl_wl object
 *
 * @param obj The compositor widget
 * @param set Whether to enable min/max setting
 * @since 1.21
 */
EAPI void efl_wl_minmax_set(Evas_Object *obj, Eina_Bool set);

/**
 * Add an externally-managed global to the compositor
 * @note The external implementation is expected to restrict access to authorized
 * clients
 * @see wl_global_create() docs
 * @since 1.21
 */
EAPI void *efl_wl_global_add(Evas_Object *obj, const void *interface, uint32_t version, void *data, void *bind_cb);

/**
 * Extract a child surface from the compositor
 *
 * An extracted surface can be freely manipulated by external code.
 * @note size hints must be respected, and the extracted object must not be externally deleted
 *
 * @param surface The surface to extract
 * @return True if the surface was successfully extracted
 * @since 1.21
 */
EAPI Eina_Bool efl_wl_surface_extract(Evas_Object *surface);

/**
 * Get the Evas_Object for an extracted wl_surface resource created by an efl_wl object
 *
 * @note Passing anything other than a valid wl_surface resource from an efl_wl object will guarantee a crash.
 *
 * @param surface_resource The wl_resource for a wl_surface
 * @return The Evas_Object of the surface, NULL on failure
 * @since 1.21
 */
EAPI Evas_Object *efl_wl_extracted_surface_object_find(void *surface_resource);

/**
 * Get the Evas_Object for an extracted surface's parent, or NULL if the parent is not extracted
 *
 * @note Passing anything other than a valid, extracted surface guarantees a crash.
 *
 * @param surface The extracted surface for a wl_surface
 * @return The Evas_Object of the parent surface, NULL on failure or if there is no parent
 * @since 1.21
 */
EAPI Evas_Object *efl_wl_extracted_surface_extracted_parent_get(Evas_Object *surface);

/**
 * Set external xkbcommon resources to be used read-only by the compositor object
 *
 * Use this function if you have available the necessary xkbcommon objects which are used
 * to handle keyboard states in a compositor. The passed objects will not be modified or copied,
 * so this function must be called again in the case that the compositor widget outlives the
 * lifetime of any of the passed pointers.
 *
 * @param obj The compositor widget
 * @param seat The seat to set the keymap for, NULL to set the keymap for all seats
 * @param keymap The xkb_keymap object to use
 * @param state The xkb_state object to use
 * @param str The string containing the keymap
 * @param wl_key_array A pointer to the wl_array in which keys are stored
 * @since 1.21
 */
EAPI void efl_wl_seat_keymap_set(Evas_Object *obj, Eo *seat, void *state, char *str, void *wl_key_array);

/**
 * Set the key repeat rate for a seat in the compositor
 *
 * @param obj The compositor widget
 * @since 1.21
 */
EAPI void efl_wl_seat_key_repeat_set(Evas_Object *obj, Eo *seat, int repeat_rate, int repeat_delay);
#endif

#endif
