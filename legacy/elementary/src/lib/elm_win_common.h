/**
 * Defines the types of window that can be created
 *
 * These are hints set on the window so that a running Window Manager knows
 * how the window should be handled and/or what kind of decorations it
 * should have.
 *
 * Currently, only the X11 backed engines use them.
 */
typedef enum
{
   ELM_WIN_UNKNOWN = -1, /**< Unknown window type. */
   ELM_WIN_BASIC, /**< A normal window. Indicates a normal, top-level
                     window. Almost every window will be created with this
                     type. */
   ELM_WIN_DIALOG_BASIC, /**< Used for simple dialog windows/ */
   ELM_WIN_DESKTOP, /**< For special desktop windows, like a background
                       window holding desktop icons. */
   ELM_WIN_DOCK, /**< The window is used as a dock or panel. Usually would
                    be kept on top of any other window by the Window
                    Manager. */
   ELM_WIN_TOOLBAR, /**< The window is used to hold a floating toolbar, or
                       similar. */
   ELM_WIN_MENU, /**< Similar to #ELM_WIN_TOOLBAR. */
   ELM_WIN_UTILITY, /**< A persistent utility window, like a toolbox or
                       palette. */
   ELM_WIN_SPLASH, /**< Splash window for a starting up application. */
   ELM_WIN_DROPDOWN_MENU, /**< The window is a dropdown menu, as when an
                             entry in a menubar is clicked. Typically used
                             with elm_win_override_set(). This hint exists
                             for completion only, as the EFL way of
                             implementing a menu would not normally use a
                             separate window for its contents. */
   ELM_WIN_POPUP_MENU, /**< Like #ELM_WIN_DROPDOWN_MENU, but for the menu
                          triggered by right-clicking an object. */
   ELM_WIN_TOOLTIP, /**< The window is a tooltip. A short piece of
                       explanatory text that typically appear after the
                       mouse cursor hovers over an object for a while.
                       Typically used with elm_win_override_set() and also
                       not very commonly used in the EFL. */
   ELM_WIN_NOTIFICATION, /**< A notification window, like a warning about
                            battery life or a new E-Mail received. */
   ELM_WIN_COMBO, /**< A window holding the contents of a combo box. Not
                     usually used in the EFL. */
   ELM_WIN_DND, /**< Used to indicate the window is a representation of an
                   object being dragged across different windows, or even
                   applications. Typically used with
                   elm_win_override_set(). */
   ELM_WIN_INLINED_IMAGE, /**< The window is rendered onto an image
                             buffer. No actual window is created for this
                             type, instead the window and all of its
                             contents will be rendered to an image buffer.
                             This allows to have children window inside a
                             parent one just like any other object would
                             be, and do other things like applying @c
                             Evas_Map effects to it. This is the only type
                             of window that requires the @c parent
                             parameter of elm_win_add() to be a valid @c
                             Evas_Object. */
   ELM_WIN_SOCKET_IMAGE,/**< The window is rendered onto an image buffer
			     and can be shown other process's plug image object.
			     No actual window is created for this type,
			     instead the window and all of its contents will be
			     rendered to an image buffer and can be shown
			     other process's plug image object*/
} Elm_Win_Type;

/**
 * The different layouts that can be requested for the virtual keyboard.
 *
 * When the application window is being managed by Illume, it may request
 * any of the following layouts for the virtual keyboard.
 */
typedef enum
{
   ELM_WIN_KEYBOARD_UNKNOWN, /**< Unknown keyboard state */
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

/**
 * In some environments, like phones, you may have an indicator that
 * shows battery status, reception, time etc. This is the indicator.
 *
 * Sometimes you don't want it because you provide the same functionality
 * inside your app, so this will request that the indicator is hidden in
 * this circumstance if you use ELM_ILLUME_INDICATOR_HIDE. The default
 * is to have the indicator shown.
 */
typedef enum
{
   ELM_WIN_INDICATOR_UNKNOWN, /**< Unknown indicator state */
   ELM_WIN_INDICATOR_HIDE, /**< Hides the indicator */
   ELM_WIN_INDICATOR_SHOW /**< Shows the indicator */
} Elm_Win_Indicator_Mode;

/**
 * Defines the opacity modes of indicator that can be shown
 */

typedef enum
{
   ELM_WIN_INDICATOR_OPACITY_UNKNOWN, /**< Unknown indicator opacity mode */
   ELM_WIN_INDICATOR_OPAQUE, /**< Opacifies the indicator */
   ELM_WIN_INDICATOR_TRANSLUCENT, /**< Be translucent the indicator */
   ELM_WIN_INDICATOR_TRANSPARENT /**< Transparentizes the indicator */
} Elm_Win_Indicator_Opacity_Mode;

/**
 * Available commands that can be sent to the Illume manager.
 *
 * When running under an Illume session, a window may send commands to the
 * Illume manager to perform different actions.
 */
typedef enum
{
   ELM_ILLUME_COMMAND_FOCUS_BACK, /**< Reverts focus to the previous window */
   ELM_ILLUME_COMMAND_FOCUS_FORWARD, /**< Sends focus to the next window in the list */
   ELM_ILLUME_COMMAND_FOCUS_HOME, /**< Hides all windows to show the Home screen */
   ELM_ILLUME_COMMAND_CLOSE, /**< Closes the currently active window */
} Elm_Illume_Command;

/**
 * @typedef Elm_Win_Trap
 *
 * Trap can be set with elm_win_trap_set() and will intercept the
 * calls to internal ecore_evas with the same name and parameters. If
 * there is a trap and it returns @c EINA_TRUE then the call will be
 * allowed, otherwise it will be ignored.
 *
 * @since 1.7
 */
typedef struct _Elm_Win_Trap Elm_Win_Trap;
struct _Elm_Win_Trap
{
#define ELM_WIN_TRAP_VERSION (1UL)
   unsigned long version;
   void *(*add)(Evas_Object *o); /**< object was just added. The returned pointer will be handled to every other trap call. */
   void (*del)(void *data, Evas_Object *o); /**< object will be deleted. */
   Eina_Bool (*hide)(void *data, Evas_Object *o);
   Eina_Bool (*show)(void *data, Evas_Object *o);
   Eina_Bool (*move)(void *data, Evas_Object *o, int x, int y);
   Eina_Bool (*resize)(void *data, Evas_Object *o, int w, int h);
   Eina_Bool (*center)(void *data, Evas_Object *o); /* not in ecore_evas, but nice to trap */
   Eina_Bool (*lower)(void *data, Evas_Object *o);
   Eina_Bool (*raise)(void *data, Evas_Object *o);
   Eina_Bool (*activate)(void *data, Evas_Object *o);
   Eina_Bool (*alpha_set)(void *data, Evas_Object *o, Eina_Bool alpha);
   Eina_Bool (*aspect_set)(void *data, Evas_Object *o, double aspect);
   Eina_Bool (*avoid_damage_set)(void *data, Evas_Object *o, Ecore_Evas_Avoid_Damage_Type on);
   Eina_Bool (*borderless_set)(void *data, Evas_Object *o, Eina_Bool on);
   Eina_Bool (*demand_attention_set)(void *data, Evas_Object *o, Eina_Bool on);
   Eina_Bool (*focus_skip_set)(void *data, Evas_Object *o, Eina_Bool skip);
   Eina_Bool (*fullscreen_set)(void *data, Evas_Object *o, Eina_Bool on);
   Eina_Bool (*iconified_set)(void *data, Evas_Object *o, Eina_Bool on);
   Eina_Bool (*layer_set)(void *data, Evas_Object *o, int layer);
   Eina_Bool (*manual_render_set)(void *data, Evas_Object *o, Eina_Bool manual_render);
   Eina_Bool (*maximized_set)(void *data, Evas_Object *o, Eina_Bool on);
   Eina_Bool (*modal_set)(void *data, Evas_Object *o, Eina_Bool on);
   Eina_Bool (*name_class_set)(void *data, Evas_Object *o, const char *n, const char *c);
   Eina_Bool (*object_cursor_set)(void *data, Evas_Object *o, Evas_Object *obj, int layer, int hot_x, int hot_y);
   Eina_Bool (*override_set)(void *data, Evas_Object *o, Eina_Bool on);
   Eina_Bool (*rotation_set)(void *data, Evas_Object *o, int rot);
   Eina_Bool (*rotation_with_resize_set)(void *data, Evas_Object *o, int rot);
   Eina_Bool (*shaped_set)(void *data, Evas_Object *o, Eina_Bool shaped);
   Eina_Bool (*size_base_set)(void *data, Evas_Object *o, int w, int h);
   Eina_Bool (*size_step_set)(void *data, Evas_Object *o, int w, int h);
   Eina_Bool (*size_min_set)(void *data, Evas_Object *o, int w, int h);
   Eina_Bool (*size_max_set)(void *data, Evas_Object *o, int w, int h);
   Eina_Bool (*sticky_set)(void *data, Evas_Object *o, Eina_Bool sticky);
   Eina_Bool (*title_set)(void *data, Evas_Object *o, const char *t);
   Eina_Bool (*urgent_set)(void *data, Evas_Object *o, Eina_Bool urgent);
   Eina_Bool (*withdrawn_set)(void *data, Evas_Object *o, Eina_Bool withdrawn);
};

/**
 * Sets the trap to be used for internal @c Ecore_Evas management.
 *
 * @param trap the trap to be used or @c NULL to remove traps. Pointer
 *        is not modified or copied, keep it alive.
 * @return @c EINA_TRUE on success, @c EINA_FALSE if there was a
 *         problem, such as invalid version number.
 *
 * @warning this is an advanced feature that you should avoid using.
 *
 * @since 1.7
 */
EAPI Eina_Bool elm_win_trap_set(const Elm_Win_Trap *trap);


