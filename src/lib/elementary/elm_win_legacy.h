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


/**
 * @brief Defines the types of window that can be created
 *
 * These are hints set on a window so that a running Window Manager knows how
 * the window should be handled and/or what kind of decorations it should have.
 *
 * Currently, only the X11 backed engines use them.
 *
 * @ingroup Elm_Win
 */
typedef enum
{
  ELM_WIN_UNKNOWN = -1 /* +1 */, /**< Default, unknown, type */
  ELM_WIN_BASIC, /**< A normal window. Indicates a normal, top-level window.
                     * Almost every window will be created with this type. */
  ELM_WIN_DIALOG_BASIC, /**< Used for simple dialog windows. */
  ELM_WIN_DESKTOP, /**< For special desktop windows, like a background window
                       * holding desktop icons. */
  ELM_WIN_DOCK, /**< The window is used as a dock or panel. Usually would be
                    * kept on top of any other window by the Window Manager. */
  ELM_WIN_TOOLBAR, /**< The window is used to hold a floating toolbar, or
                       * similar. */
  ELM_WIN_MENU, /**< Similar to @.toolbar. */
  ELM_WIN_UTILITY, /**< A persistent utility window, like a toolbox or
                       * palette. */
  ELM_WIN_SPLASH, /**< Splash window for a starting up application. */
  ELM_WIN_DROPDOWN_MENU, /**< The window is a dropdown menu, as when an
                             *  entry in a menubar is clicked. This hint exists
                             * for completion only, as the EFL way of
                             * implementing a menu would not normally use a
                             * separate window for its contents. */
  ELM_WIN_POPUP_MENU, /**< Like @.dropdown_menu, but for the menu triggered
                          * by right-clicking an object. */
  ELM_WIN_TOOLTIP, /**< The window is a tooltip. A short piece of explanatory
                       * text that typically appear after the mouse cursor
                       * hovers over an object for a while. Typically not very
                       * commonly used in the EFL. */
  ELM_WIN_NOTIFICATION, /**< A notification window, like a warning about
                            * battery life or a new E-Mail received. */
  ELM_WIN_COMBO, /**< A window holding the contents of a combo box. Not
                     * usually used in the EFL. */

/**
* Used to indicate the window is a representation of an object being
* dragged across different windows, or even applications. Typically
* used with elm_win_override_set().
*/
  ELM_WIN_DND,
  ELM_WIN_INLINED_IMAGE,
  ELM_WIN_SOCKET_IMAGE,
  ELM_WIN_FAKE, /**< See elm_win_fake_add(). @since 1.13 */

  ELM_WIN_NAVIFRAME_BASIC, /**< Used for naviframe style replacement with
                                   * a back button instead of a close button.
                                   *
                                   * @since 1.19 */
} Elm_Win_Type;

/**
 * @brief The different layouts that can be requested for the virtual keyboard.
 *
 * When the application window is being managed by Illume it may request any of
 * the following layouts for the virtual keyboard.
 *
 * @ingroup Elm_Win
 */
typedef enum
{
  ELM_WIN_KEYBOARD_UNKNOWN = 0, /**< Unknown keyboard state */
  ELM_WIN_KEYBOARD_OFF, /**< Request to deactivate the keyboard */
  ELM_WIN_KEYBOARD_ON, /**< Enable keyboard with default layout */
  ELM_WIN_KEYBOARD_ALPHA, /**< Alpha (a-z) keyboard layout */
  ELM_WIN_KEYBOARD_NUMERIC, /**< Numeric keyboard layout */
  ELM_WIN_KEYBOARD_PIN, /**< PIN keyboard layout */
  ELM_WIN_KEYBOARD_PHONE_NUMBER, /**< Phone keyboard layout */
  ELM_WIN_KEYBOARD_HEX, /**< Hexadecimal numeric keyboard layout */
  ELM_WIN_KEYBOARD_TERMINAL, /**< Full (QWERTY) keyboard layout */
  ELM_WIN_KEYBOARD_PASSWORD, /**< Password keyboard layout */
  ELM_WIN_KEYBOARD_IP, /**< IP keyboard layout */
  ELM_WIN_KEYBOARD_HOST, /**< Host keyboard layout */
  ELM_WIN_KEYBOARD_FILE, /**< File keyboard layout */
  ELM_WIN_KEYBOARD_URL, /**< URL keyboard layout */
  ELM_WIN_KEYBOARD_KEYPAD, /**< Keypad layout */
  ELM_WIN_KEYBOARD_J2ME /**< J2ME keyboard layout */
} Elm_Win_Keyboard_Mode;

/** Define the keygrab modes of window. A window may send commands to the
 * Keyrouter according this mode, and perform different actions.
 *
 * @ingroup Elm_Win
 */
typedef enum
{
  ELM_WIN_KEYGRAB_UNKNOWN = 0, /**< Unknown keygrab mode */
  ELM_WIN_KEYGRAB_SHARED = 256 /* 1 >> 8 */, /**< Get the grabbed-key
                                                 * together with the other
                                                 * client windows */
  ELM_WIN_KEYGRAB_TOPMOST = 512 /* 1 >> 9 */, /**< Get the grabbed-key only
                                                  * when window is top of the
                                                  * stack */
  ELM_WIN_KEYGRAB_EXCLUSIVE = 1024 /* 1 >> 10 */, /**< Get the grabbed-key
                                                      * exclusively regardless
                                                      * of window's position */
  ELM_WIN_KEYGRAB_OVERRIDE_EXCLUSIVE = 2048 /* 1 >> 11 */ /**< Get the grabbed-key exclusively
                                                              * regardless of window's position.
                                                              * This is overrided by grabs from the
                                                              * other client window */
} Elm_Win_Keygrab_Mode;


/**
 * @brief Define the move or resize mode of window.
 *
 * The user can request the display server to start moving or resizing the
 * window by combining these modes. However only limited combinations are
 * allowed.
 *
 * Currently, only the following 9 combinations are permitted. More
 * combinations may be added in future: 1. move, 2. top, 3. bottom, 4. left, 5.
 * right, 6. top | left, 7. top | right, 8. bottom | left, 9. bottom | right.
 *
 * @since 1.19
 *
 * @ingroup Elm_Win
 */
typedef enum
{
  ELM_WIN_MOVE_RESIZE_MOVE = 1, /**< Start moving window */
  ELM_WIN_MOVE_RESIZE_TOP = 2 /* 1 >> 1 */, /**< Start resizing window to the
                                                * top */
  ELM_WIN_MOVE_RESIZE_BOTTOM = 4 /* 1 >> 2 */, /**< Start resizing window to
                                                   * the bottom */
  ELM_WIN_MOVE_RESIZE_LEFT = 8 /* 1 >> 3 */, /**< Start resizing window to
                                                 * the left */
  ELM_WIN_MOVE_RESIZE_RIGHT = 16 /* 1 >> 4 */ /**< Start resizing window to
                                                  * the right */
} Elm_Win_Move_Resize_Mode;


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
 * This type of window will be handled in special mode by window managers
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
 * This function is used to set the orientation of window @p obj to specific angle fixed.
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


/**
 * @brief Constrain the maximum width and height of a window to the width and
 * height of the screen.
 *
 * When @c constrain is @c true, @c obj will never resize larger than the
 * screen.
 *
 * @param[in] obj The object.
 * @param[in] constrain @c true to restrict the window's maximum size.
 *
 * @ingroup Elm_Win_Group
 */
EAPI void elm_win_screen_constrain_set(Evas_Object *obj, Eina_Bool constrain);

/**
 * @brief Get the constraints on the maximum width and height of a window
 * relative to the width and height of the screen.
 *
 * When this function returns @c true, @c obj will never resize larger than the
 * screen.
 *
 * @param[in] obj The object.
 *
 * @return @c true to restrict the window's maximum size.
 *
 * @ingroup Elm_Win_Group
 */
EAPI Eina_Bool elm_win_screen_constrain_get(const Evas_Object *obj);

/**
 * @brief Set the window to be skipped by keyboard focus.
 *
 * This sets the window to be skipped by normal keyboard input. This means a
 * window manager will be asked not to focus this window as well as omit it
 * from things like the taskbar, pager, "alt-tab" list etc. etc.
 *
 * Call this and enable it on a window BEFORE you show it for the first time,
 * otherwise it may have no effect.
 *
 * Use this for windows that have only output information or might only be
 * interacted with by the mouse or touchscreen, never for typing. This may have
 * side-effects like making the window non-accessible in some cases unless the
 * window is specially handled. Use this with care.
 *
 * @param[in] obj The object.
 * @param[in] skip The skip flag state ($true if it is to be skipped).
 *
 * @ingroup Elm_Win_Group
 */
EAPI void elm_win_prop_focus_skip_set(Evas_Object *obj, Eina_Bool skip);

/**
 * @brief Window's autohide state.
 *
 * When closing the window in any way outside of the program control, like
 * pressing the X button in the titlebar or using a command from the Window
 * Manager, a "delete,request" signal is emitted to indicate that this event
 * occurred and the developer can take any action, which may include, or not,
 * destroying the window object.
 *
 * When this property is set to @c true, the window will be automatically
 * hidden when this event occurs, after the signal is emitted. If this property
 * is @c false nothing will happen, beyond the event emission.
 *
 * C applications can use this option along with the quit policy
 * @c ELM_POLICY_QUIT_LAST_WINDOW_HIDDEN which allows exiting EFL's main loop
 * when all the windows are hidden.
 *
 * @note @c autodel and @c autohide are not mutually exclusive. The window will
 * be deleted if both are set to @c true.
 *
 * @param[in] obj The object.
 * @param[in] autohide If @c true, the window will automatically hide itself
 * when closed.
 *
 * @ingroup Elm_Win_Group
 */
EAPI void elm_win_autohide_set(Evas_Object *obj, Eina_Bool autohide);

/**
 * @brief Window's autohide state.
 *
 * When closing the window in any way outside of the program control, like
 * pressing the X button in the titlebar or using a command from the Window
 * Manager, a "delete,request" signal is emitted to indicate that this event
 * occurred and the developer can take any action, which may include, or not,
 * destroying the window object.
 *
 * When this property is set to @c true, the window will be automatically
 * hidden when this event occurs, after the signal is emitted. If this property
 * is @c false nothing will happen, beyond the event emission.
 *
 * C applications can use this option along with the quit policy
 * @c ELM_POLICY_QUIT_LAST_WINDOW_HIDDEN which allows exiting EFL's main loop
 * when all the windows are hidden.
 *
 * @note @c autodel and @c autohide are not mutually exclusive. The window will
 * be deleted if both are set to @c true.
 *
 * @param[in] obj The object.
 *
 * @return If @c true, the window will automatically hide itself when closed.
 *
 * @ingroup Elm_Win_Group
 */
EAPI Eina_Bool elm_win_autohide_get(const Evas_Object *obj);

/**
 * @brief Set a window object's icon.
 *
 * This sets an image to be used as the icon for the given window, in the
 * window manager decoration part. The exact pixel dimensions of the object
 * (not object size) will be used and the image pixels will be used as-is when
 * this function is called. If the image object has been updated, then call
 * this function again to source the image pixels and place them in the
 * window's icon. Note that only objects of type @ref Efl_Canvas_Image or
 * @ref Efl_Ui_Image are allowed.
 *
 * @param[in] obj The object.
 * @param[in] icon The image object to use for an icon.
 *
 * @ingroup Elm_Win_Group
 */
EAPI void elm_win_icon_object_set(Evas_Object *obj, Evas_Object *icon);

/**
 * @brief Get the icon object used for the window.
 *
 * The object returns is the one marked by @ref elm_win_icon_object_set as the
 * object to use for the window icon.
 *
 * @param[in] obj The object.
 *
 * @return The Evas image object to use for an icon.
 *
 * @ingroup Elm_Win_Group
 */
EAPI const Evas_Object *elm_win_icon_object_get(const Evas_Object *obj);

/**
 * @brief Set the iconified state of a window.
 *
 * @param[in] obj The object.
 * @param[in] iconified If @c true, the window is iconified.
 *
 * @ingroup Elm_Win_Group
 */
EAPI void elm_win_iconified_set(Evas_Object *obj, Eina_Bool iconified);

/**
 * @brief Get the iconified state of a window.
 *
 * @param[in] obj The object.
 *
 * @return If @c true, the window is iconified.
 *
 * @ingroup Elm_Win_Group
 */
EAPI Eina_Bool elm_win_iconified_get(const Evas_Object *obj);

/**
 * @brief Set the maximized state of a window.
 *
 * @param[in] obj The object.
 * @param[in] maximized If @c true, the window is maximized.
 *
 * @ingroup Elm_Win_Group
 */
EAPI void elm_win_maximized_set(Evas_Object *obj, Eina_Bool maximized);

/**
 * @brief Get the maximized state of a window.
 *
 * @param[in] obj The object.
 *
 * @return If @c true, the window is maximized.
 *
 * @ingroup Elm_Win_Group
 */
EAPI Eina_Bool elm_win_maximized_get(const Evas_Object *obj);

/**
 * @brief Set the fullscreen state of a window.
 *
 * @param[in] obj The object.
 * @param[in] fullscreen If @c true, the window is fullscreen.
 *
 * @ingroup Elm_Win_Group
 */
EAPI void elm_win_fullscreen_set(Evas_Object *obj, Eina_Bool fullscreen);

/**
 * @brief Get the fullscreen state of a window.
 *
 * @param[in] obj The object.
 *
 * @return If @c true, the window is fullscreen.
 *
 * @ingroup Elm_Win_Group
 */
EAPI Eina_Bool elm_win_fullscreen_get(const Evas_Object *obj);

/**
 * @brief Set the sticky state of the window.
 *
 * Hints the Window Manager that the window in @c obj should be left fixed at
 * its position even when the virtual desktop it's on moves or changes.
 *
 * @param[in] obj The object.
 * @param[in] sticky If @c true, the window's sticky state is enabled.
 *
 * @ingroup Elm_Win_Group
 */
EAPI void elm_win_sticky_set(Evas_Object *obj, Eina_Bool sticky);

/**
 * @brief Get the sticky state of the window.
 *
 * @param[in] obj The object.
 *
 * @return If @c true, the window's sticky state is enabled.
 *
 * @ingroup Elm_Win_Group
 */
EAPI Eina_Bool elm_win_sticky_get(const Evas_Object *obj);





/**
 * @brief Set the noblank property of a window.
 *
 * The "noblank" property is a way to request the display on which the window
 * is shown does not blank, go to screensaver or otherwise hide or obscure the
 * window. It is intended for uses such as media playback on a television where
 * a user may not want to be interrupted by an idle screen. The noblank
 * property may have no effect if the window is iconified/minimized or hidden.
 *
 * @param[in] obj The object.
 * @param[in] noblank If @c true, the window is set to noblank.
 *
 * @since 1.11
 *
 * @ingroup Elm_Win_Group
 */
EAPI void elm_win_noblank_set(Evas_Object *obj, Eina_Bool noblank);

/**
 * @brief Get the noblank property of a window.
 *
 * @param[in] obj The object.
 *
 * @return If @c true, the window is set to noblank.
 *
 * @since 1.11
 *
 * @ingroup Elm_Win_Group
 */
EAPI Eina_Bool elm_win_noblank_get(const Evas_Object *obj);

/**
 * @brief Set the borderless state of a window.
 *
 * This function requests the Window Manager not to draw any decoration around
 * the window.
 *
 * @param[in] obj The object.
 * @param[in] borderless If @c true, the window is borderless.
 *
 * @ingroup Elm_Win_Group
 */
EAPI void elm_win_borderless_set(Evas_Object *obj, Eina_Bool borderless);

/**
 * @brief Get the borderless state of a window.
 *
 * @param[in] obj The object.
 *
 * @return If @c true, the window is borderless.
 *
 * @ingroup Elm_Win_Group
 */
EAPI Eina_Bool elm_win_borderless_get(const Evas_Object *obj);

/**
 * @brief The role of the window.
 *
 * It is a hint of how the Window Manager should handle it. Unlike
 * @ref elm_win_type_get and @ref elm_win_name_get this can be changed at
 * runtime.
 *
 * Set the role of the window.
 *
 * @param[in] obj The object.
 * @param[in] role The role to set.
 *
 * @ingroup Elm_Win_Group
 */
EAPI void elm_win_role_set(Evas_Object *obj, const char *role);

/**
 * @brief The role of the window.
 *
 * It is a hint of how the Window Manager should handle it. Unlike
 * @ref elm_win_type_get and @ref elm_win_name_get this can be changed at
 * runtime.
 *
 * Get the role of the window.
 *
 * The returned string is an internal one and should not be freed or modified.
 * It will also be invalid if a new role is set or if the window is destroyed.
 *
 * @param[in] obj The object.
 *
 * @return The role to set.
 *
 * @ingroup Elm_Win_Group
 */
EAPI const char *elm_win_role_get(const Evas_Object *obj);


/**
 * @brief The window name.
 *
 * The meaning of name depends on the underlying windowing system.
 *
 * The window name is a construction property that can only be set at creation
 * time, before finalize. In C this means inside @c efl_add().
 *
 * @note Once set, it cannot be modified afterwards.
 *
 * No description supplied.
 *
 * @param[in] obj The object.
 *
 * @return Window name
 *
 * @since 1.18
 *
 * @ingroup Elm_Win_Group
 */
EAPI const char *elm_win_name_get(const Evas_Object *obj);


/**
 * @brief The type of the window.
 *
 * It is a hint of how the Window Manager should handle it.
 *
 * The window type is a construction property that can only be set at creation
 * time, before finalize. In C this means inside @c efl_add().
 *
 * @note Once set, it cannot be modified afterward.
 *
 * If the object is not window object, returns @c unknown.
 *
 * @param[in] obj The object.
 *
 * @return Window type
 *
 * @ingroup Elm_Win_Group
 */
EAPI Elm_Win_Type elm_win_type_get(const Evas_Object *obj);


/**
 * @brief The hardware acceleration preference for this window.
 *
 * This is a constructor function and can only be called before
 * @ref Efl.Object.finalize.
 *
 * This property overrides the global EFL configuration option
 * "accel_preference" for this single window, and accepts the same syntax.
 *
 * The @c accel string is a freeform C string that indicates what kind of
 * acceleration is preferred. Here "acceleration" generally refers to rendering
 * and the hardware with which the unit application renders GUIs. This may or
 * may not be honored but a best attempt will be made. Known strings are as
 * follows:
 *
 * "gl", "opengl" - try use OpenGL. "3d" - try to use a 3d acceleration unit.
 * "hw", "hardware", "accel" - try any acceleration unit (best possible) "none"
 * - use no acceleration but software instead (since 1.16)
 *
 * Since 1.14, it is also possible to specify some GL properties for the GL
 * window surface. This allows applications to use GLView with depth, stencil
 * and MSAA buffers with direct rendering. The new accel preference string
 * format is thus "{HW Accel}[:depth{value}[:stencil{value}[:msaa{str}$]$]$]".
 *
 * Accepted values for depth are for instance "depth", "depth16", "depth24".
 * Accepted values for stencil are "stencil", "stencil1", "stencil8". For MSAA,
 * only predefined strings are accepted: "msaa", "msaa_low", "msaa_mid" and
 * "msaa_high". The selected configuration is not guaranteed and is only valid
 * in case of GL acceleration. Only the base acceleration string will be saved
 * (e.g. "gl" or "hw").
 *
 * Full examples include:
 *
 * "gl", - try to use OpenGL "hw:depth:stencil", - use HW acceleration with
 * default depth and stencil buffers "opengl:depth24:stencil8:msaa_mid" - use
 * OpenGL with 24-bit depth, 8-bit stencil and a medium number of MSAA samples
 * in the backbuffer.
 *
 * Note that this option may be overriden by environment variables or the
 * configuration option "accel_preference_override".
 *
 * This will return the value of "accel_preference" when the window was
 * created.
 *
 * @param[in] obj The object.
 *
 * @return Acceleration
 *
 * @since 1.18
 *
 * @ingroup Elm_Win_Group
 */
EAPI const char *elm_win_accel_preference_get(const Evas_Object *obj);

/**
 * @brief Set the alpha channel state of a window.
 *
 * If @c alpha is true, the alpha channel of the canvas will be enabled
 * possibly making parts of the window completely or partially transparent.
 * This is also subject to the underlying system supporting it, for example a
 * system using a compositing manager.
 *
 * @note Alpha window can be enabled automatically by window theme style's
 * property. If "alpha" data.item is "1" or "true" in window style(eg.
 * elm/win/base/default), the window is switched to alpha automatically without
 * the explicit api call.
 *
 * @param[in] obj The object.
 * @param[in] alpha @c true if the window alpha channel is enabled, @c false
 * otherwise.
 *
 * @ingroup Elm_Win_Group
 */
EAPI void elm_win_alpha_set(Evas_Object *obj, Eina_Bool alpha);

/**
 * @brief Get the alpha channel state of a window.
 *
 * @param[in] obj The object.
 *
 * @return @c true if the window alpha channel is enabled, @c false otherwise.
 *
 * @ingroup Elm_Win_Group
 */
EAPI Eina_Bool elm_win_alpha_get(const Evas_Object *obj);

/**
 * @brief Activate a window object.
 *
 * This function sends a request to the Window Manager to activate the window
 * pointed by @c obj. If honored by the WM, the window will receive the
 * keyboard focus.
 *
 * @note This is just a request that a Window Manager may ignore, so calling
 * this function does not ensure in any way that the window will be the active
 * one afterwards.
 * @param[in] obj The object.
 *
 * @ingroup Elm_Win_Group
 */
EAPI void elm_win_activate(Evas_Object *obj);

/**
 * @brief Center a window on the screen.
 *
 * This function centers window @c obj horizontally and/or vertically based on
 * the values of @c h and @c v.
 *
 * @note This is just a request that a Window Manager may ignore, so calling
 * this function does not ensure in any way that the window will be centered
 * afterwards.
 *
 * @param[in] obj The object.
 * @param[in] h If @c true, center horizontally. If @c false, do not change
 * horizontal location.
 * @param[in] v If @c true, center vertically. If @c false, do not change
 * vertical location.
 *
 * @ingroup Elm_Win_Group
 */
EAPI void elm_win_center(Evas_Object *obj, Eina_Bool h, Eina_Bool v);



/**
 * @brief Start moving or resizing the window.
 *
 * The user can request the display server to start moving or resizing the
 * window by combining modes from @ref Efl_Ui_Win_Move_Resize_Mode. This API
 * can only be called if none of the following conditions is true:
 *
 * 1. Called in the absence of a pointer down event, 2. Called more than once
 * before a pointer up event, 3. Called when the window is already being
 * resized or moved, 4. Called with an unsupported combination of modes.
 *
 * Right usage: 1. Pointer (mouse or touch) down event, 2.
 * @ref elm_win_move_resize_start called only once with a supported mode, 3.
 * Pointer (mouse or touch) up event.
 *
 * If a pointer up event occurs after calling the function, it automatically
 * ends the window move and resize operation.
 *
 * Currently, only the following 9 combinations are allowed, and possibly more
 * combinations may be added in the future: 1. @ref ELM_WIN_MOVE_RESIZE_MOVE
 * 2. @ref ELM_WIN_MOVE_RESIZE_TOP 3. @ref ELM_WIN_MOVE_RESIZE_BOTTOM 4.
 * @ref ELM_WIN_MOVE_RESIZE_LEFT 5. @ref ELM_WIN_MOVE_RESIZE_RIGHT 6.
 * @ref ELM_WIN_MOVE_RESIZE_TOP | @ref ELM_WIN_MOVE_RESIZE_LEFT 7.
 * @ref ELM_WIN_MOVE_RESIZE_TOP | @ref ELM_WIN_MOVE_RESIZE_RIGHT 8.
 * @ref ELM_WIN_MOVE_RESIZE_BOTTOM | @ref ELM_WIN_MOVE_RESIZE_LEFT 9.
 * @ref ELM_WIN_MOVE_RESIZE_BOTTOM | @ref ELM_WIN_MOVE_RESIZE_RIGHT
 *
 * In particular move and resize cannot happen simultaneously.
 *
 * @note the result of this API can only guarantee that the request has been
 * forwarded to the server, but there is no guarantee that the request can be
 * processed by the display server.
 *
 * @param[in] obj The object.
 * @param[in] mode The requested move or resize mode.
 *
 * @return @c true if the request was successfully sent to the display server,
 * @c false in case of error.
 *
 * @since 1.19
 *
 * @ingroup Elm_Win_Group
 */
EAPI Eina_Bool elm_win_move_resize_start(Evas_Object *obj, Elm_Win_Move_Resize_Mode mode);

/**
 * @brief Set the animate status for the focus highlight for this window.
 *
 * This function will enable or disable the animation of focus highlight only
 * for the given window, regardless of the global setting for it.
 *
 * @param[in] animate The enabled value for the highlight animation.
 *
 * @ingroup Elm_Win
 */
EAPI void elm_win_focus_highlight_animate_set(Elm_Win *obj, Eina_Bool animate);

/**
 * @brief Get the animate value of the focus highlight for this window.
 *
 * @return The enabled value for the highlight animation.
 *
 * @ingroup Elm_Win
 */
EAPI Eina_Bool elm_win_focus_highlight_animate_get(const Elm_Win *obj);

/**
 * @brief Set the enabled status for the focus highlight in a window.
 *
 * This function will enable or disable the focus highlight only for the given
 * window, regardless of the global setting for it.
 *
 * @param[in] enabled The enabled value for the highlight.
 *
 * @ingroup Elm_Win
 */
EAPI void elm_win_focus_highlight_enabled_set(Elm_Win *obj, Eina_Bool enabled);

/**
 * @brief Get the enabled value of the focus highlight for this window.
 *
 * @return The enabled value for the highlight.
 *
 * @ingroup Elm_Win
 */
EAPI Eina_Bool elm_win_focus_highlight_enabled_get(const Elm_Win *obj);

/**
 * @brief Set the style for the focus highlight on this window.
 *
 * Sets the style to use for theming the highlight of focused objects on the
 * given window. If @c style is NULL, the default will be used.
 *
 * @param[in] style The style or @c null if none.
 *
 * @ingroup Elm_Win
 */
EAPI Eina_Bool elm_win_focus_highlight_style_set(Elm_Win *obj, const char *style);

/**
 * @brief Get the style set for the focus highlight object.
 *
 * @return The style or @c null if none.
 *
 * @ingroup Elm_Win
 */
EAPI const char *elm_win_focus_highlight_style_get(const Elm_Win *obj);
