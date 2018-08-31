#include "efl_ui_win.eo.legacy.h"

/**
 * @brief In some environments, like phones, you may have an indicator that
 * shows battery status, reception, time etc. This is the indicator.
 *
 * Sometimes you don't want it because you provide the same functionality
 * inside your app, so this will request that the indicator is hidden in this
 * circumstance. The default is depend on the environments. For example, like
 * phones, default is to have the indicator shown. But like TV, default is to
 * have the indicator hidden.
 *
 * @ingroup Elm_Win
 */
typedef enum
{
  ELM_WIN_INDICATOR_UNKNOWN = 0, /** Unknown indicator state */
  ELM_WIN_INDICATOR_HIDE, /** Hides the indicator */
  ELM_WIN_INDICATOR_SHOW /** Shows the indicator */
} Elm_Win_Indicator_Mode;

/** Defines the opacity modes of indicator that can be shown
 *
 * @ingroup Elm_Win
 */
typedef enum
{
  ELM_WIN_INDICATOR_OPACITY_UNKNOWN = 0, /** Unknown indicator opacity mode */
  ELM_WIN_INDICATOR_OPAQUE, /** Opacifies the indicator */
  ELM_WIN_INDICATOR_TRANSLUCENT, /** Be translucent the indicator */
  ELM_WIN_INDICATOR_TRANSPARENT /** Transparentizes the indicator */
} Elm_Win_Indicator_Opacity_Mode;

/**
 * @brief Available commands that can be sent to the Illume manager.
 *
 * When running under an Illume session, a window may send commands to the
 * Illume manager to perform different actions.
 *
 * @ingroup Elm
 */
typedef enum
{
  ELM_ILLUME_COMMAND_FOCUS_BACK = 0, /** Reverts focus to the previous window
                                      */
  ELM_ILLUME_COMMAND_FOCUS_FORWARD, /** Sends focus to the next window in the
                                     * list */
  ELM_ILLUME_COMMAND_FOCUS_HOME, /** Hides all windows to show the Home screen
                                  */
  ELM_ILLUME_COMMAND_CLOSE /** Closes the currently active window */
} Elm_Illume_Command;

typedef Efl_Ui_Win_Type       Elm_Win_Type;

#define ELM_WIN_UNKNOWN              EFL_UI_WIN_UNKNOWN
#define ELM_WIN_BASIC                EFL_UI_WIN_BASIC
#define ELM_WIN_DIALOG_BASIC         EFL_UI_WIN_DIALOG_BASIC
#define ELM_WIN_DESKTOP              EFL_UI_WIN_DESKTOP
#define ELM_WIN_DOCK                 EFL_UI_WIN_DOCK
#define ELM_WIN_TOOLBAR              EFL_UI_WIN_TOOLBAR
#define ELM_WIN_MENU                 EFL_UI_WIN_MENU
#define ELM_WIN_UTILITY              EFL_UI_WIN_UTILITY
#define ELM_WIN_SPLASH               EFL_UI_WIN_SPLASH
#define ELM_WIN_DROPDOWN_MENU        EFL_UI_WIN_DROPDOWN_MENU
#define ELM_WIN_POPUP_MENU           EFL_UI_WIN_POPUP_MENU
#define ELM_WIN_TOOLTIP              EFL_UI_WIN_TOOLTIP
#define ELM_WIN_NOTIFICATION         EFL_UI_WIN_NOTIFICATION
#define ELM_WIN_COMBO                EFL_UI_WIN_COMBO
/**
 * Used to indicate the window is a representation of an object being
 * dragged across different windows, or even applications. Typically
 * used with elm_win_override_set().
 */
#define ELM_WIN_DND                  13
#define ELM_WIN_INLINED_IMAGE        14
#define ELM_WIN_SOCKET_IMAGE         15
#define ELM_WIN_FAKE                 16 /**< See elm_win_fake_add(). @since 1.13 */
#define ELM_WIN_NAVIFRAME_BASIC      EFL_UI_WIN_NAVIFRAME_BASIC

typedef Efl_Ui_Win_Keyboard_Mode        Elm_Win_Keyboard_Mode;

#define ELM_WIN_KEYBOARD_UNKNOWN        EFL_UI_WIN_KEYBOARD_UNKNOWN
#define ELM_WIN_KEYBOARD_OFF            EFL_UI_WIN_KEYBOARD_OFF
#define ELM_WIN_KEYBOARD_ON             EFL_UI_WIN_KEYBOARD_ON
#define ELM_WIN_KEYBOARD_ALPHA          EFL_UI_WIN_KEYBOARD_ALPHA
#define ELM_WIN_KEYBOARD_NUMERIC        EFL_UI_WIN_KEYBOARD_NUMERIC
#define ELM_WIN_KEYBOARD_PIN            EFL_UI_WIN_KEYBOARD_PIN
#define ELM_WIN_KEYBOARD_PHONE_NUMBER   EFL_UI_WIN_KEYBOARD_PHONE_NUMBER
#define ELM_WIN_KEYBOARD_HEX            EFL_UI_WIN_KEYBOARD_HEX
#define ELM_WIN_KEYBOARD_TERMINAL       EFL_UI_WIN_KEYBOARD_TERMINAL
#define ELM_WIN_KEYBOARD_PASSWORD       EFL_UI_WIN_KEYBOARD_PASSWORD
#define ELM_WIN_KEYBOARD_IP             EFL_UI_WIN_KEYBOARD_IP
#define ELM_WIN_KEYBOARD_HOST           EFL_UI_WIN_KEYBOARD_HOST
#define ELM_WIN_KEYBOARD_FILE           EFL_UI_WIN_KEYBOARD_FILE
#define ELM_WIN_KEYBOARD_URL            EFL_UI_WIN_KEYBOARD_URL
#define ELM_WIN_KEYBOARD_KEYPAD         EFL_UI_WIN_KEYBOARD_KEYPAD
#define ELM_WIN_KEYBOARD_J2ME           EFL_UI_WIN_KEYBOARD_J2ME

typedef Efl_Ui_Win_Keygrab_Mode         Elm_Win_Keygrab_Mode;

#define ELM_WIN_KEYGRAB_UNKNOWN              EFL_UI_WIN_KEYGRAB_UNKNOWN
#define ELM_WIN_KEYGRAB_SHARED               EFL_UI_WIN_KEYGRAB_SHARED
#define ELM_WIN_KEYGRAB_TOPMOST              EFL_UI_WIN_KEYGRAB_TOPMOST
#define ELM_WIN_KEYGRAB_EXCLUSIVE            EFL_UI_WIN_KEYGRAB_EXCLUSIVE
#define ELM_WIN_KEYGRAB_OVERRIDE_EXCLUSIVE   EFL_UI_WIN_KEYGRAB_OVERRIDE_EXCLUSIVE

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
 * @ingroup Elm_Win
 */
EAPI Evas_Object          *elm_win_add(Evas_Object *parent, const char *name, Elm_Win_Type type);

typedef Evas_Object Elm_Win;

/**
 * Creates a fake window object using a pre-existing canvas.
 *
 * @param ee The Ecore_Evas to use
 *
 * The returned window widget will not manage or modify the canvas;
 * this canvas must continue to be managed externally.
 *
 * Do not use this function if you are not writing a window manager.
 * @warning Exact behaviors of this function are not guaranteed.
 *
 * @return The created object, or @c NULL on failure
 *
 * @ingroup Elm_Win
 *
 * @since 1.13
 */
EAPI Evas_Object          *elm_win_fake_add(Ecore_Evas *ee);

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
 * @ingroup Elm_Win
 */
EAPI Evas_Object          *elm_win_util_standard_add(const char *name, const char *title);

/**
 * Adds a window object with dialog setup
 *
 * @param parent The parent window
 * @param name The name of the window
 * @param title The title for the window
 *
 * This creates a window like elm_win_add() but also puts in a standard
 * background with elm_bg_add(), as well as setting the window title to
 * @p title. The window type created is of type ELM_WIN_DIALOG_BASIC.
 * This tipe of window will be handled in special mode by window managers
 * with regards of it's @p parent window.
 *
 * @return The created object, or @c NULL on failure
 *
 * @see elm_win_add()
 *
 * @ingroup Elm_Win
 * @since 1.13
 */
EAPI Evas_Object          *elm_win_util_dialog_add(Evas_Object *parent, const char *name, const char *title);

/**
 * @brief Set the window's autodel state.
 *
 * When closing the window in any way outside of the program control, like
 * pressing the X button in the titlebar or using a command from the Window
 * Manager, a "delete,request" signal is emitted to indicate that this event
 * occurred and the developer can take any action, which may include, or not,
 * destroying the window object.
 *
 * When the @c autodel parameter is set, the window will be automatically
 * destroyed when this event occurs, after the signal is emitted. If @c autodel
 * is @c false, then the window will not be destroyed and is up to the program
 * to do so when it's required.
 *
 * @param[in] obj The object.
 * @param[in] autodel If @c true, the window will automatically delete itself
 * when closed.
 *
 * @ingroup Elm_Win
 */
EAPI void elm_win_autodel_set(Evas_Object *obj, Eina_Bool autodel);

/**
 * @brief Get the window's autodel state.
 *
 * @param[in] obj The object.
 *
 * @return If @c true, the window will automatically delete itself when closed.
 *
 * @ingroup Elm_Win
 */
EAPI Eina_Bool elm_win_autodel_get(const Evas_Object *obj);

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
 * @ingroup Elm_Win
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
 * @ingroup Elm_Win
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
 * @ingroup Elm_Win
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
 * @ingroup Elm_Win
 * @since 1.7
 */
EAPI void                  elm_win_norender_pop(Evas_Object *obj);

/**
 * The returns how many times norender has been pushed on the window
 * @param obj The window object
 * @return The number of times norender has been pushed
 *
 * @see elm_win_norender_push()
 * @see elm_win_norender_pop()
 * @see elm_win_render()
 * @ingroup Elm_Win
 * @since 1.7
 */
EAPI int                   elm_win_norender_get(const Evas_Object *obj);

/**
 * This manually asks evas to render the window now
 *
 * @param obj The window object
 *
 * You should NEVER call this unless you really know what you are doing and
 * why. Never call this unless you are asking for performance degradation
 * and possibly weird behavior. Windows get automatically rendered when the
 * application goes into the idle enter state so there is never a need to call
 * this UNLESS you have enabled "norender" mode.
 *
 * @see elm_win_norender_push()
 * @see elm_win_norender_pop()
 * @see elm_win_norender_get()
 * @ingroup Elm_Win
 * @since 1.7
 */
EAPI void                  elm_win_render(Evas_Object *obj);

/* Wayland specific call - returns NULL on non-Wayland engines */
/**
 * Get the Ecore_Wl_Window of an Evas_Object
 *
 * Do not use this function if you'd like your application/library be portable.
 * You have been warned.
 *
 * @param obj the object
 *
 * @return The Ecore_Wl_Window of @p obj
 *
 * @ingroup Elm_Win
 */
EAPI Ecore_Wl2_Window *elm_win_wl_window_get(const Evas_Object *obj);

/* Windows specific call - returns NULL on non-Windows engines */
/**
 * Get the Ecore_Win32_Window of an Evas_Object
 *
 * Do not use this function if you'd like your application/library be portable.
 * You have been warned.
 *
 * @param obj the object
 *
 * @return The Ecore_Win32_Window of @p obj
 *
 * @ingroup Elm_Win
 *
 * @since 1.16
 */
EAPI Ecore_Win32_Window *elm_win_win32_window_get(const Evas_Object *obj);

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
 * ingroup Elm_Win
 * @since 1.9
 */
EAPI void                  elm_win_wm_rotation_preferred_rotation_set(Evas_Object *obj, int rotation);

/**
 * @brief Add @c subobj as a resize object of window @c obj.
 *
 * Setting an object as a resize object of the window means that the @c subobj
 * child's size and position will be controlled by the window directly. That
 * is, the object will be resized to match the window size and should never be
 * moved or resized manually by the developer.
 *
 * In addition, resize objects of the window control what the minimum size of
 * it will be, as well as whether it can or not be resized by the user.
 *
 * For the end user to be able to resize a window by dragging the handles or
 * borders provided by the Window Manager, or using any other similar
 * mechanism, all of the resize objects in the window should have their @ref
 * evas_object_size_hint_weight_set set to EVAS_HINT_EXPAND.
 *
 * Also notice that the window can get resized to the current size of the
 * object if the EVAS_HINT_EXPAND is set after the call to this. So if the
 * object should get resized to the size of the window, set this hint before
 * adding it as a resize object (this happens because the size of the window
 * and the object are evaluated as soon as the object is added to the window).
 *
 * @param[in] subobj The resize object to add.
 *
 * @ingroup Elm_Win
 */
EAPI void elm_win_resize_object_add(Evas_Object *obj, Evas_Object *subobj);

/**
 * @brief Delete @c subobj as a resize object of window @c obj.
 *
 * This function removes the object @c subobj from the resize objects of the
 * window @c obj. It will not delete the object itself, which will be left
 * unmanaged and should be deleted by the developer, manually handled or set as
 * child of some other container.
 *
 * @param[in] subobj The resize object to add.
 *
 * @ingroup Elm_Win
 */
EAPI void elm_win_resize_object_del(Evas_Object *obj, Evas_Object *subobj);

/** Get the Ecore_X_Window of an Evas_Object.
 *
 * @ingroup Elm_Win
 */
EAPI Ecore_X_Window elm_win_xwindow_get(const Evas_Object *obj);

/**
 * @brief Get the Ecore_Wl2_Window of an Evas_Object.
 *
 * @return The Ecore_Wl2_Window of @c obj.
 *
 * @ingroup Elm_Win
 */
EAPI Ecore_Wl2_Window *elm_win_wl_window_get(const Evas_Object *obj);


/**
 * @brief Get the Ecore_Win32_Window of an Evas_Object
 *
 * @return The Ecore_Win32_Window of @c obj.
 *
 * @since 1.17
 *
 * @ingroup Elm_Win
 */
EAPI Ecore_Win32_Window *elm_win_win32_window_get(const Evas_Object *obj);

/**
 * @brief Get the Ecore_Cocoa_Window of an Efl.Canvas.Object.
 *
 * @return The Ecore_Cocoa_Window of @c obj.
 *
 * @since 1.17
 *
 * @ingroup Elm_Win
 */
EAPI Ecore_Cocoa_Window *elm_win_cocoa_window_get(const Evas_Object *obj);

/**
 * @brief Get the trap data associated with a window.
 *
 * @return The trap data of the window.
 *
 * @since 1.12
 *
 * @ingroup Elm_Win
 */
EAPI void *elm_win_trap_data_get(const Evas_Object *obj);

/**
 * @brief Set the override state of a window.
 *
 * A window with @c override set to true will not be managed by the Window
 * Manager. This means that no decorations of any kind will be shown for it,
 * moving and resizing must be handled by the application, as well as the
 * window visibility.
 *
 * This should not be used for normal windows, and even for not so normal ones,
 * it should only be used when there's a good reason and with a lot of care.
 * Mishandling override windows may result situations that disrupt the normal
 * workflow of the end user.
 *
 * @param[in] override If true, the window is overridden.
 *
 * @ingroup Elm_Win
 */
EAPI void elm_win_override_set(Evas_Object *obj, Eina_Bool override);

/**
 * @brief Get the override state of a window.
 *
 * @return If true, the window is overridden.
 *
 * @ingroup Elm_Win
 */
EAPI Eina_Bool elm_win_override_get(const Evas_Object *obj);

/**
 * @brief Lower a window object.
 *
 * Places the window pointed by @c obj at the bottom of the stack, so that no
 * other window is covered by it.
 *
 * If @ref elm_win_override_set is not set, the Window Manager may ignore this
 * request.
 *
 * @ingroup Elm_Win
 */
EAPI void elm_win_lower(Evas_Object *obj);

/**
 * @brief Set a window to be an illume quickpanel window.
 *
 * By default window objects are not quickpanel windows.
 *
 * @param[in] quickpanel The quickpanel flag.
 *
 * @ingroup Elm_Win
 */
EAPI void elm_win_quickpanel_set(Evas_Object *obj, Eina_Bool quickpanel);

/**
 * @brief Get if this window is a quickpanel or not.
 *
 * @return The quickpanel flag.
 *
 * @ingroup Elm_Win
 */
EAPI Eina_Bool elm_win_quickpanel_get(const Evas_Object *obj);
/**
 * @brief Set which zone this quickpanel should appear in.
 *
 * @param[in] zone The requested zone for this quickpanel.
 *
 * @ingroup Elm_Win
 */
EAPI void elm_win_quickpanel_zone_set(Evas_Object *obj, int zone);

/**
 * @brief Get which zone this quickpanel should appear in.
 *
 * @return The requested zone for this quickpanel.
 *
 * @ingroup Elm_Win
 */
EAPI int elm_win_quickpanel_zone_get(const Evas_Object *obj);
/**
 * @brief Set the major priority of a quickpanel window.
 *
 * @param[in] priority The major priority for this quickpanel or -1.
 *
 * @ingroup Elm_Win
 */
EAPI void elm_win_quickpanel_priority_major_set(Evas_Object *obj, int priority);

/**
 * @brief Get the major priority of a quickpanel window.
 *
 * @return The major priority for this quickpanel or -1.
 *
 * @ingroup Elm_Win
 */
EAPI int elm_win_quickpanel_priority_major_get(const Evas_Object *obj);

/**
 * @brief Set the minor priority of a quickpanel window.
 *
 * @param[in] priority The minor priority for this quickpanel.
 *
 * @ingroup Elm_Win
 */
EAPI void elm_win_quickpanel_priority_minor_set(Evas_Object *obj, int priority);

/**
 * @brief Get the minor priority of a quickpanel window.
 *
 * @return The minor priority for this quickpanel.
 *
 * @ingroup Elm_Win
 */
EAPI int elm_win_quickpanel_priority_minor_get(const Evas_Object *obj);

/**
 * @brief Sets the indicator mode of the window.
 *
 * @param[in] mode The mode, one of #Elm_Win_Indicator_Mode.
 *
 * @ingroup Elm_Win
 */
EAPI void elm_win_indicator_mode_set(Evas_Object *obj, Elm_Win_Indicator_Mode mode);

/**
 * @brief Get the indicator mode of the window.
 *
 * @return The mode, one of #Elm_Win_Indicator_Mode.
 *
 * @ingroup Elm_Win
 */
EAPI Elm_Win_Indicator_Mode elm_win_indicator_mode_get(const Evas_Object *obj);

/**
 * @brief Sets the indicator opacity mode of the window.
 *
 * @param[in] mode The mode, one of #Elm_Win_Indicator_Opacity_Mode.
 *
 * @ingroup Elm_Win
 */
EAPI void elm_win_indicator_opacity_set(Evas_Object *obj, Elm_Win_Indicator_Opacity_Mode mode);

/**
 * @brief Get the indicator opacity mode of the window.
 *
 * @return The mode, one of #Elm_Win_Indicator_Opacity_Mode.
 *
 * @ingroup Elm_Win
 */
EAPI Elm_Win_Indicator_Opacity_Mode elm_win_indicator_opacity_get(const Evas_Object *obj);

/**
 * @brief Sets whether the window is a keyboard.
 *
 * @param[in] mode If true, the window is a virtual keyboard..
 *
 * @ingroup Elm_Win
 */
EAPI void elm_win_keyboard_win_set(Evas_Object *obj, Eina_Bool is_keyboard);

/**
 * @brief Get whether the window is a keyboard
 *
 * @return The mode, If true, the window is a virtual keyboard..
 *
 * @ingroup Elm_Win
 */
EAPI Eina_Bool elm_win_keyboard_win_get(const Evas_Object *obj);

/**
 * @brief Set if this window is an illume conformant window.
 *
 * @param[in] conformant The conformant flag.
 *
 * @ingroup Elm_Win
 */
EAPI void elm_win_conformant_set(Evas_Object *obj, Eina_Bool conformant);

/**
 * @brief Get if this window is an illume conformant window.
 *
 * @return The conformant flag.
 *
 * @ingroup Elm_Win
 */
EAPI Eina_Bool elm_win_conformant_get(const Evas_Object *obj);

/**
 * @brief Set the manual rotation done mode.
 *
 * This function is used to set or reset the manual rotation done mode. the
 * message of rotation done is sent to WM after rendering its canvas in
 * Ecore_Evas. but if set the manual rotation done mode, it's disabled and user
 * should call this explicitly to sends the message.
 *
 * @param[in] set @c true to set manual rotation done mode, @c false otherwise.
 *
 * @since 1.9
 *
 * @ingroup Elm_Win
 */
EAPI void elm_win_wm_rotation_manual_rotation_done_set(Evas_Object *obj, Eina_Bool set);

/**
 * @brief Get the state of manual rotation done mode.
 *
 * This function is used to get the state of manual rotation done mode.
 *
 * @return @c true to set manual rotation done mode, @c false otherwise.
 *
 * @since 1.9
 *
 * @ingroup Elm_Win
 */
EAPI Eina_Bool elm_win_wm_rotation_manual_rotation_done_get(const Evas_Object *obj);

/**
 * @brief To notify the rotation done to WM manually.
 *
 * This function is used to notify the rotation done to WM manually.
 *
 * @since 1.9
 *
 * @ingroup Elm_Win
 */
EAPI void elm_win_wm_rotation_manual_rotation_done(Evas_Object *obj);

/**
 * @brief Set the rotation of the window.
 *
 * Most engines only work with multiples of 90.
 *
 * This function is used to set the orientation of the window @c obj to match
 * that of the screen. The window itself will be resized to adjust to the new
 * geometry of its contents. If you want to keep the window size, see
 * @ref elm_win_rotation_with_resize_set.
 *
 * @param[in] rotation The rotation of the window, in degrees (0-360),
 * counter-clockwise.
 *
 * @ingroup Elm_Win
 */
EAPI void elm_win_rotation_set(Evas_Object *obj, int rotation);

/**
 * @brief Get the rotation of the window.
 *
 * @return The rotation of the window, in degrees (0-360), counter-clockwise.
 *
 * @ingroup Elm_Win
 */
EAPI int elm_win_rotation_get(const Evas_Object *obj);

/**
 * @brief Rotates the window and resizes it.
 *
 * Like @ref elm_win_rotation_set, but it also resizes the window's contents so
 * that they fit inside the current window geometry.
 *
 * @param[in] rotation The rotation of the window in degrees (0-360),
 * counter-clockwise.
 *
 * @ingroup Elm_Win
 */
EAPI void elm_win_rotation_with_resize_set(Evas_Object *obj, int rotation);

/**
 * @brief Query whether window manager supports window rotation or not.
 *
 * The window manager rotation allows the WM to controls the rotation of
 * application windows. It is designed to support synchronized rotation for the
 * multiple application windows at same time.
 *
 * See also @ref elm_win_wm_rotation_preferred_rotation_get,
 * @ref elm_win_wm_rotation_available_rotations_get.
 *
 * @since 1.9
 *
 * @ingroup Elm_Win
 */
EAPI Eina_Bool elm_win_wm_rotation_supported_get(const Evas_Object *obj);

/**
 * @brief Get the preferred rotation value.
 *
 * This function is used to get the preferred rotation value.
 *
 * @return The preferred rotation of the window in degrees (0-360),
 * counter-clockwise.
 *
 * @since 1.9
 *
 * @ingroup Elm_Win
 */
EAPI int elm_win_wm_rotation_preferred_rotation_get(const Evas_Object *obj);


/**
 * @brief Get the screen position of a window.
 *
 * @param[out] x The int to store the x coordinate to.
 * @param[out] y The int to store the y coordinate to.
 *
 * @ingroup Elm_Win
 */
EAPI void elm_win_screen_position_get(const Evas_Object *obj, int *x, int *y);

/**
 * @brief Get screen geometry details for the screen that a window is on.
 *
 * @param[out] x Where to return the horizontal offset value. May be @c null.
 * @param[out] y Where to return the vertical offset value. May be @c null.
 * @param[out] w Where to return the width value. May be @c null.
 * @param[out] h Where to return the height value. May be @c null.
 *
 * @ingroup Elm_Win
 */
EAPI void elm_win_screen_size_get(const Evas_Object *obj, int *x, int *y, int *w, int *h);

/**
 * @brief Get screen dpi for the screen that a window is on.
 *
 * @param[out] xdpi Pointer to value to store return horizontal dpi. May be
 * @c null.
 * @param[out] ydpi Pointer to value to store return vertical dpi. May be
 * @c null.
 *
 * @since 1.7
 *
 * @ingroup Elm_Win
 */
EAPI void elm_win_screen_dpi_get(const Evas_Object *obj, int *xdpi, int *ydpi);

/**
 * @brief Set the icon name of the window.
 *
 * @param[in] icon_name The icon name to set.
 *
 * @ingroup Elm_Win
 */
EAPI void elm_win_icon_name_set(Evas_Object *obj, const char *icon_name);

/**
 * @brief Get the icon name of the window.
 *
 * The returned string is an internal one and should not be freed or modified.
 * It will also be invalid if a new icon name is set or if the window is
 * destroyed.
 *
 * @return The icon name to set.
 *
 * @ingroup Elm_Win
 */
EAPI const char *elm_win_icon_name_get(const Evas_Object *obj);

/**
 * @brief Set the withdrawn state of a window.
 *
 * @param[in] withdrawn If true, the window is withdrawn.
 *
 * @ingroup Elm_Win
 */
EAPI void elm_win_withdrawn_set(Evas_Object *obj, Eina_Bool withdrawn);

/**
 * @brief Get the withdrawn state of a window.
 *
 * @return If true, the window is withdrawn.
 *
 * @ingroup Elm_Win
 */
EAPI Eina_Bool elm_win_withdrawn_get(const Evas_Object *obj);

/**
 * @brief Set the urgent state of a window.
 *
 * @param[in] urgent If true, the window is urgent.
 *
 * @ingroup Elm_Win
 */
EAPI void elm_win_urgent_set(Evas_Object *obj, Eina_Bool urgent);

/**
 * @brief Get the urgent state of a window.
 *
 * @return If true, the window is urgent.
 *
 * @ingroup Elm_Win
 */
EAPI Eina_Bool elm_win_urgent_get(const Evas_Object *obj);

/**
 * @brief Set the demand_attention state of a window.
 *
 * @param[in] demand_attention If true, the window is demand_attention.
 *
 * @ingroup Elm_Win
 */
EAPI void elm_win_demand_attention_set(Evas_Object *obj, Eina_Bool demand_attention);

/**
 * @brief Get the demand_attention state of a window.
 *
 * @return If true, the window is demand_attention.
 *
 * @ingroup Elm_Win
 */
EAPI Eina_Bool elm_win_demand_attention_get(const Evas_Object *obj);

/**
 * @brief Set the modal state of a window.
 *
 * @param[in] modal If true, the window is modal.
 *
 * @ingroup Elm_Win
 */
EAPI void elm_win_modal_set(Evas_Object *obj, Eina_Bool modal);

/**
 * @brief Get the modal state of a window.
 *
 * @return If true, the window is modal.
 *
 * @ingroup Elm_Win
 */
EAPI Eina_Bool elm_win_modal_get(const Evas_Object *obj);

/**
 * @brief Set the shaped state of a window.
 *
 * Shaped windows, when supported, will render the parts of the window that has
 * no content, transparent.
 *
 * If @c shaped is false, then it is strongly advised to have some background
 * object or cover the entire window in any other way, or the parts of the
 * canvas that have no data will show framebuffer artifacts.
 *
 * @param[in] shaped If @c true, the window is shaped.
 *
 * @ingroup Elm_Win
 */
EAPI void elm_win_shaped_set(Evas_Object *obj, Eina_Bool shaped);

/**
 * @brief Get the shaped state of a window.
 *
 * @return If @c true, the window is shaped.
 *
 * @ingroup Elm_Win
 */
EAPI Eina_Bool elm_win_shaped_get(const Evas_Object *obj);

/**
 * @brief Set the title of the window.
 *
 * @param[in] title The title.
 *
 * @ingroup Elm_Win
 */
EAPI void elm_win_title_set(Evas_Object *obj, const char *title);

/**
 * @brief Get the title of the window.
 *
 * The returned string is an internal one and should not be freed or modified.
 * It will also be invalid if a new title is set or if the window is destroyed.
 *
 * @return The title.
 *
 * @ingroup Elm_Win
 */
EAPI const char *elm_win_title_get(const Evas_Object *obj);

/**
 * @brief Set the base window size used with stepping calculation
 *
 * Base size + stepping is what is calculated for window sizing restrictions.
 *
 * @param[in] w The base width.
 * @param[in] h The base height.
 *
 * @since 1.7
 *
 * @ingroup Elm_Win
 */
EAPI void elm_win_size_base_set(Evas_Object *obj, int w, int h);

/**
 * @brief Get the base size of a window.
 *
 * @param[out] w The base width.
 * @param[out] h The base height.
 *
 * @since 1.7
 *
 * @ingroup Elm_Win
 */
EAPI void elm_win_size_base_get(const Evas_Object *obj, int *w, int *h);

/**
 * @brief Set the window stepping used with sizing calculation.
 *
 * Base size + stepping is what is calculated for window sizing restrictions.
 *
 * @param[in] w The stepping width (0 disables).
 * @param[in] h The stepping height (0 disables).
 *
 * @since 1.7
 *
 * @ingroup Elm_Win
 */
EAPI void elm_win_size_step_set(Evas_Object *obj, int w, int h);

/**
 * @brief Get the stepping of a window.
 *
 * @param[out] w The stepping width (0 disables).
 * @param[out] h The stepping height (0 disables).
 *
 * @since 1.7
 *
 * @ingroup Elm_Win
 */
EAPI void elm_win_size_step_get(const Evas_Object *obj, int *w, int *h);

/**
 * @brief Send a command to the windowing environment
 *
 * This is intended to work in touchscreen or small screen device environments
 * where there is a more simplistic window management policy in place. This
 * uses the window object indicated to select which part of the environment to
 * control (the part that this window lives in), and provides a command and an
 * optional parameter structure (use NULL for this if not needed).
 *
 * @param[in] params Optional parameters for the command.
 *
 * @ingroup Elm_Win
 */
EAPI void elm_win_illume_command_send(Evas_Object *obj, Elm_Illume_Command command, void *params);

/**
 * @brief Set the profile of a window.
 *
 * @param[in] profile The string value of a window profile.
 *
 * @since 1.8
 *
 * @ingroup Elm_Win
 */
EAPI void elm_win_profile_set(Evas_Object *obj, const char *profile);

/**
 * @brief Get the profile of a window.
 *
 * @return The string value of a window profile.
 *
 * @since 1.8
 *
 * @ingroup Elm_Win
 */
EAPI const char *elm_win_profile_get(const Evas_Object *obj);

/**
 * @brief Set the layer of the window.
 *
 * What this means exactly will depend on the underlying engine used.
 *
 * In the case of X11 backed engines, the value in @c layer has the following
 * meanings - less than 3 means that the window will be placed below all
 * others, more than 5 means that the window will be placed above all others,
 * and anything else means that the window will be placed in the default layer.
 *
 * @param[in] layer The layer of the window.
 *
 * @ingroup Elm_Win
 */
EAPI void elm_win_layer_set(Evas_Object *obj, int layer);

/**
 * @brief Get the layer of the window.
 *
 * @return The layer of the window.
 *
 * @ingroup Elm_Win
 */
EAPI int elm_win_layer_get(const Evas_Object *obj);

/**
 * @brief Get the inlined image object handle
 *
 * When you create a window with elm_win_add() of type #ELM_WIN_INLINED_IMAGE,
 * then the window is in fact an evas image object inlined in the parent
 * canvas. You can get this object (be careful to not manipulate it as it is
 * under control of elementary), and use it to do things like get pixel data,
 * save the image to a file, etc.
 *
 * @return The inlined image object or @c null if none exists.
 *
 * @ingroup Elm_Win
 */
EAPI Evas_Object *elm_win_inlined_image_object_get(const Evas_Object *obj);

/**
 * @brief Internal. Used to complete the fake window type.
 *
 * @param[in] oee
 *
 * @ingroup Elm_Win
 *
 * @deprecated
 */
EAPI void elm_win_fake_canvas_set(Evas_Object *obj, Ecore_Evas *oee) EINA_DEPRECATED;

/**
 * Get the Ecore_Window of an Evas_Object
 *
 * When Elementary is using a Wayland engine, this function will return the surface id of the elm window's surface.
 *
 * @param obj The window object
 * @return The Ecore_Window of an Evas_Object
 *
 * @ingroup Elm_Win
 * @since 1.8
 * @note Unless you are getting the window id for the purpose of communicating between client<->compositor over dbus,
 * this is definitely not the function you are looking for.
 */
EAPI Ecore_Window          elm_win_window_id_get(const Evas_Object *obj);

/**
 * @brief Get the Main Menu of a window.
 *
 * @return The Main Menu of the window ($null on error).
 *
 * @ingroup Elm_Win
 */
EAPI Evas_Object *elm_win_main_menu_get(Evas_Object *obj);

/**
 * @brief Sets the keyboard mode of the window.
 *
 * @param[in] mode The mode, one of #Elm_Win_Keyboard_Mode.
 *
 * @ingroup Elm_Win
 */
EAPI void elm_win_keyboard_mode_set(Elm_Win *obj, Elm_Win_Keyboard_Mode mode);

/**
 * @brief Get the keyboard mode of the window.
 *
 * @return The mode, one of #Elm_Win_Keyboard_Mode.
 *
 * @ingroup Elm_Win
 */
EAPI Elm_Win_Keyboard_Mode elm_win_keyboard_mode_get(const Elm_Win *obj);

/**
 * @brief Set the aspect ratio of a window.
 *
 * @param[in] aspect If 0, the window has no aspect limits, otherwise it is
 * width divided by height.
 */
EAPI void elm_win_aspect_set(Elm_Win *obj, double aspect);

/**
 * @brief Get the aspect ratio of a window.
 *
 * @return If 0, the window has no aspect limits, otherwise it is width divided
 * by height.
 */
EAPI double elm_win_aspect_get(const Elm_Win *obj);

/**
 * @brief Set keygrab value of the window
 *
 * This function grabs the @c key of window using @c grab_mode.
 *
 * @param[in] key This string is the keyname to grab.
 * @param[in] modifiers A combination of modifier keys that must be present to
 * trigger the event. Not supported yet.
 * @param[in] not_modifiers A combination of modifier keys that must not be
 * present to trigger the event. Not supported yet.
 * @param[in] priority Not supported yet.
 * @param[in] grab_mode Describes how the key should be grabbed, wrt. focus and
 * stacking.
 *
 * @return @c true on success, @c false otherwise
 */
EAPI Eina_Bool elm_win_keygrab_set(Elm_Win *obj, const char *key, Evas_Modifier_Mask modifiers, Evas_Modifier_Mask not_modifiers, int priority, Elm_Win_Keygrab_Mode grab_mode);

/**
 * @brief Unset keygrab value of the window
 *
 * This function unset keygrab value. Ungrab @c key of window.
 *
 * @param[in] key This string is the keyname to grab.
 * @param[in] modifiers A combination of modifier keys that must be present to
 * trigger the event. Not supported yet.
 * @param[in] not_modifiers A combination of modifier keys that must not be
 * present to trigger the event. Not supported yet.
 *
 * @return @c true on success, @c false otherwise
 */
EAPI Eina_Bool elm_win_keygrab_unset(Elm_Win *obj, const char *key, Evas_Modifier_Mask modifiers, Evas_Modifier_Mask not_modifiers);

/**
 * @brief Get the elm_win object from any child object
 * 
 * @return The elm_win, or @c NULL on failure
 * @since 1.20
 */
EAPI Evas_Object *elm_win_get(Evas_Object *obj);

/**
 * @brief Create a socket to provide the service for Plug widget.
 *
 * @param[in] svcname The name of the service to be advertised. Eensure that it
 * is unique (when combined with @c svcnum) otherwise creation may fail.
 * @param[in] svcnum A number (any value, 0 being the common default) to
 * differentiate multiple instances of services with the same name.
 * @param[in] svcsys A boolean that if true, specifies to create a system-wide
 * service all users can connect to, otherwise the service is private to the
 * user id that created the service.
 *
 * @return @c true on success, @c false otherwise
 *
 * @ingroup Efl_Ui_Win
 */
EAPI Eina_Bool elm_win_socket_listen(Elm_Win *obj, const char *svcname, int svcnum, Eina_Bool svcsys);

/**
 * @brief Get whether a window has focus.
 *
 * @return @c true if window has focus, @c false otherwise
 *
 * @ingroup Efl_Ui_Win
 */
EAPI Eina_Bool elm_win_focus_get(const Elm_Win *obj);

/**
 * @brief Set the style for the focus highlight on this window.
 *
 * Sets the style to use for theming the highlight of focused objects on the
 * given window. If @c style is NULL, the default will be used.
 *
 * @param[in] style The style or @c null if none.
 *
 * @ingroup Efl_Ui_Win
 */
EAPI void elm_win_focus_highlight_style_set(Elm_Win *obj, const char *style);

/**
 * @brief Get the style set for the focus highlight object.
 *
 * @return The style or @c null if none.
 *
 * @ingroup Efl_Ui_Win
 */
EAPI const char *elm_win_focus_highlight_style_get(const Elm_Win *obj);

/**
 * @brief Set the enabled status for the focus highlight in a window.
 *
 * This function will enable or disable the focus highlight only for the given
 * window, regardless of the global setting for it.
 *
 * @param[in] enabled The enabled value for the highlight.
 *
 * @ingroup Efl_Ui_Win
 */
EAPI void elm_win_focus_highlight_enabled_set(Elm_Win *obj, Eina_Bool enabled);

/**
 * @brief Get the enabled value of the focus highlight for this window.
 *
 * @return The enabled value for the highlight.
 *
 * @ingroup Efl_Ui_Win
 */
EAPI Eina_Bool elm_win_focus_highlight_enabled_get(const Elm_Win *obj);

/**
 * @brief Set the animate status for the focus highlight for this window.
 *
 * This function will enable or disable the animation of focus highlight only
 * for the given window, rof the global setting for it.
 *
 * @param[in] animate The enabled value for the highlight animation.
 *
 * @ingroup Efl_Ui_Win
 */
EAPI void elm_win_focus_highlight_animate_set(Elm_Win *obj, Eina_Bool animate);

/**
 * @brief Get the animate value of the focus highlight for this window.
 *
 * @return The enabled value for the highlight animation.
 *
 * @ingroup Efl_Ui_Win
 */
EAPI Eina_Bool elm_win_focus_highlight_animate_get(const Elm_Win *obj);

/**
 * @brief Raise a window object.
 *
 * Places the window pointed by @c obj at the top of the stack, so that it's
 * not covered by any other window.
 *
 * @ingroup Efl_Ui_Win
 */
EAPI void elm_win_raise(Elm_Win *obj);

/**
 * @brief Set the array of available profiles to a window.
 *
 * @param[in] profiles The string array of available profiles.
 * @param[in] count The number of members in profiles.
 *
 * @since 1.8
 *
 * @ingroup Efl_Ui_Win
 */
EAPI void elm_win_available_profiles_set(Elm_Win *obj, const char **profiles, unsigned int count);

/**
 * @brief Get the array of available profiles of a window.
 *
 * @param[out] profiles The string array of available profiles.
 * @param[out] count The number of members in profiles.
 *
 * @return @c true on success, @c false otherwise
 *
 * @since 1.8
 *
 * @ingroup Efl_Ui_Win
 */
EAPI Eina_Bool elm_win_available_profiles_get(const Elm_Win *obj, char ***profiles, unsigned int *count);

/**
 * @brief Set the array of available window rotations.
 *
 * This function is used to set the available rotations to give the hints to
 * WM. WM will refer this hints and set the orientation window properly.
 *
 * @param[in] obj The object.
 * @param[in] rotations The array of rotation value.
 * @param[in] count The size of the rotations array.
 *
 * @since 1.9
 *
 * @ingroup Efl_Ui_Win
 */
EAPI void elm_win_wm_rotation_available_rotations_set(Elm_Win *obj, const int *rotations, unsigned int count);

/**
 * @brief Get the array of available window rotations.
 *
 * This function is used to get the available rotations.
 *
 * @param[in] obj The object.
 * @param[out] rotations The array of rotation value.
 * @param[out] count The size of the rotations array.
 *
 * @return @c true on success, @c false otherwise
 *
 * @since 1.9
 *
 * @ingroup Efl_Ui_Win
 */
EAPI Eina_Bool elm_win_wm_rotation_available_rotations_get(const Elm_Win *obj, int **rotations, unsigned int *count);
