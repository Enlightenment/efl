/**
 * Adds a window object. If this is the first window created, pass NULL as
 * @p parent.
 *
 * @param parent Parent object to add the window to, or NULL
 * @param name The name of the window
 * @param type The window type, one of #Elm_Win_Type.
 *
 * The @p parent parameter can be @c NULL for every window @p type
 * except #ELM_WIN_INLINED_IMAGE, which needs a parent to retrieve the
 * canvas on which the image object will be created.
 *
 * @return The created object, or @c NULL on failure
 *
 * @ingroup Win
 */
EAPI Evas_Object          *elm_win_add(Evas_Object *parent, const char *name, Elm_Win_Type type);

/**
 * Adds a window object with standard setup
 *
 * @param name The name of the window
 * @param title The title for the window
 *
 * This creates a window like elm_win_add() but also puts in a standard
 * background with elm_bg_add(), as well as setting the window title to
 * @p title. The window type created is of type ELM_WIN_BASIC, with @c NULL
 * as the parent widget.
 *
 * @return The created object, or @c NULL on failure
 *
 * @see elm_win_add()
 *
 * @ingroup Win
 */
EAPI Evas_Object          *elm_win_util_standard_add(const char *name, const char *title);

/**
 * Set the floating mode of a window.
 *
 * @param obj The window object
 * @param floating If true, the window is floating mode
 *
 * The floating mode can be used on mobile environment. For example, if the
 * video-player window sets the floating mode, then e (enlightenment window
 * manager) changes its geometry and handles it like a popup. This is similar to
 * a multi window concept in a mobile phone. The way of handling floating mode
 * window is decided by enlightenment window manager.
 *
 * @ingroup Win
 * @see elm_win_floating_mode_get()
 * @since 1.8
 */
EAPI void                  elm_win_floating_mode_set(Evas_Object *obj, Eina_Bool floating);

/**
 * Get the floating mode of a window.
 *
 * @param obj The window object
 * @return If true, the window is floating mode
 *
 * @ingroup Win
 * @see elm_win_floating_mode_set()
 * @since 1.8
 */
EAPI Eina_Bool             elm_win_floating_mode_get(const Evas_Object *obj);

/**
 * This pushes (increments) the norender counter on the window
 *
 * @param obj The window object
 *
 * There are some occasions where you wish to suspend rendering on a window.
 * You may be "sleeping" and have nothing to update and do not want animations
 * or other theme side-effects causing rendering to the window while "asleep".
 * You can push (and pop) the norender mode to have this work.
 *
 * If combined with evas_render_dump(), evas_image_cache_flush() and
 * evas_font_cache_flush() (and maybe edje_file_cache_flush() and
 * edje_collection_cache_flush()), you can minimize memory footprint
 * significantly while "asleep", and the pausing of rendering ensures
 * evas does not re-load data into memory until needed. When rendering is
 * resumed, data will be re-loaded as needed, which may result in some
 * lag, but does save memory.
 *
 * @see elm_win_norender_pop()
 * @see elm_win_norender_get()
 * @see elm_win_render()
 * @ingroup Win
 * @since 1.7
 */
EAPI void                  elm_win_norender_push(Evas_Object *obj);

/**
 * This pops (decrements) the norender counter on the window
 *
 * @param obj The window object
 *
 * Once norender has gone back to 0, then automatic rendering will continue
 * in the given window. If it is already at 0, this will have no effect.
 *
 * @see elm_win_norender_push()
 * @see elm_win_norender_get()
 * @see elm_win_render()
 * @ingroup Win
 * @since 1.7
 */
EAPI void                  elm_win_norender_pop(Evas_Object *obj);

/**
 * The retruns how many times norender has been pushed on the window
 * @param obj The window object
 * @return The number of times norender has been pushed
 *
 * @see elm_win_norender_push()
 * @see elm_win_norender_pop()
 * @see elm_win_render()
 * @ingroup Win
 * @since 1.7
 */
EAPI int                   elm_win_norender_get(Evas_Object *obj);

/**
 * This manually asks evas to render the window now
 *
 * @param obj The window object
 *
 * You should NEVER call this unless you really know what you are doing and
 * why. Never call this unless you are asking for performance degredation
 * and possibly weird behavior. Windows get automatically rendered when the
 * application goes idle so there is never a need to call this UNLESS you
 * have enabled "norender" mode.
 *
 * @see elm_win_norender_push()
 * @see elm_win_norender_pop()
 * @see elm_win_norender_get()
 * @ingroup Win
 * @since 1.7
 */
EAPI void                  elm_win_render(Evas_Object *obj);

/* Wayland specific call - returns NULL on non-Wayland engines */
/**
 * Get the Ecore_Wl_Window of an Evas_Object
 *
 * @param obj the object
 *
 * @return The Ecore_Wl_Window of @p obj
 *
 * @ingroup Win
 */
EAPI Ecore_Wl_Window *elm_win_wl_window_get(const Evas_Object *obj);

/**
 * Set the preferred rotation value.
 *
 * This function is used to set the orientation of window @p obj to spicific angle fixed.
 *
 * @param obj The window object
 * @param rotation The preferred rotation of the window in degrees (0-360),
 * counter-clockwise.
 *
 * @see elm_win_wm_rotation_preferred_rotation_get()
 *
 * ingroup Win
 * @since 1.9
 */
EAPI void                  elm_win_wm_rotation_preferred_rotation_set(const Evas_Object *obj, int rotation);

/**
 * Get the Ecore_Window of an Evas_Object
 *
 * When Elementary is using a Wayland engine, this function will return the surface id of the elm window's surface.
 *
 * @param obj The window object
 * @return The Ecore_Window of an Evas_Object
 *
 * @ingroup Win
 * @since 1.8
 */
EAPI Ecore_Window          elm_win_window_id_get(const Evas_Object *obj);

#include "elm_win.eo.legacy.h"