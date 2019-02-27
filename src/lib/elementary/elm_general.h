/**
 * @defgroup Elm_General General
 * @ingroup Elementary
 *
 * @brief General Elementary API. Functions that don't relate to
 * Elementary objects specifically.
 *
 * Here are documented functions which init/shutdown the library,
 * that apply to generic Elementary objects, that deal with
 * configuration, et cetera.
 *
 * @ref general_functions_example_page "This" example contemplates
 * some of these functions.
 */

/**
 * @addtogroup Elm_General
 * @{
 */

// Legacy types
#include "elm_general.eot.h"

// EO types. Defined for legacy-only builds as legacy uses typedef of EO types.
#include "efl_ui.eot.h"

#define ELM_FOCUS_PREVIOUS EFL_UI_FOCUS_DIRECTION_PREVIOUS
#define ELM_FOCUS_NEXT EFL_UI_FOCUS_DIRECTION_NEXT
#define ELM_FOCUS_UP EFL_UI_FOCUS_DIRECTION_UP
#define ELM_FOCUS_DOWN EFL_UI_FOCUS_DIRECTION_DOWN
#define ELM_FOCUS_RIGHT EFL_UI_FOCUS_DIRECTION_RIGHT
#define ELM_FOCUS_LEFT EFL_UI_FOCUS_DIRECTION_LEFT
typedef Efl_Ui_Focus_Direction Elm_Focus_Direction;

/**
 * Defines couple of standard Evas_Object layers to be used
 * with evas_object_layer_set().
 *
 * @note whenever extending with new values, try to keep some padding
 *       to siblings so there is room for further extensions.
 */
typedef enum
{
   ELM_OBJECT_LAYER_BACKGROUND = EVAS_LAYER_MIN + 64, /**< where to place backgrounds */
   ELM_OBJECT_LAYER_DEFAULT = 0, /**< Evas_Object default layer (and thus for Elementary) */
   ELM_OBJECT_LAYER_FOCUS = EVAS_LAYER_MAX - 128, /**< where focus object visualization is */
   ELM_OBJECT_LAYER_TOOLTIP = EVAS_LAYER_MAX - 64, /**< where to show tooltips */
   ELM_OBJECT_LAYER_CURSOR = EVAS_LAYER_MAX - 32, /**< where to show cursors */
   ELM_OBJECT_LAYER_LAST /**< last layer known by Elementary */
} Elm_Object_Layer;

/** How the focus region should be calculated (not related to input focus). */
typedef enum
{
  ELM_FOCUS_REGION_SHOW_WIDGET = 0, /**< As a widget. */
  ELM_FOCUS_REGION_SHOW_ITEM /**< As an item. */
} Elm_Focus_Region_Show_Mode;


/**************************************************************************/
EAPI extern int ELM_ECORE_EVENT_ETHUMB_CONNECT;

/**
 * Emitted when the application has reconfigured elementary settings due
 * to an external configuration tool asking it to.
 */
EAPI extern int ELM_EVENT_CONFIG_ALL_CHANGED;

/**
 * Emitted when any Elementary's policy value is changed.
 */
EAPI extern int ELM_EVENT_POLICY_CHANGED;

/**
 * Emitted when nothing is visible and the process as a whole should go into
 * a background state.
 * @since 1.12
 */
EAPI extern int ELM_EVENT_PROCESS_BACKGROUND;

/**
 * Emitted when going from nothing being visible to at least one window
 * being visible.
 * @since 1.12
 */
EAPI extern int ELM_EVENT_PROCESS_FOREGROUND;

typedef Eina_Bool             (*Elm_Event_Cb)(void *data, Evas_Object *obj, Evas_Object *src, Evas_Callback_Type type, void *event_info); /**< Function prototype definition for callbacks on input events happening on Elementary widgets. @a data will receive the user data pointer passed to elm_object_event_callback_add(). @a src will be a pointer to the widget on which the input event took place. @a type will get the type of this event and @a event_info, the struct with details on this event. */

extern EAPI double _elm_startup_time;

#ifndef ELM_LIB_QUICKLAUNCH
#define ELM_MAIN() int main(int argc, char **argv) { int ret__; _elm_startup_time = ecore_time_unix_get(); elm_init(argc, argv); ret__ = elm_main(argc, argv); elm_shutdown(); return ret__; } /**< macro to be used after the elm_main() function */
#else
/** @deprecated macro to be used after the elm_main() function.
 * Do not define ELM_LIB_QUICKLAUNCH
 * Compile your programs with -fpie and -pie -rdynamic instead, to generate a single binary (linkable executable).
 */
#define ELM_MAIN() int main(int argc, char **argv) { int ret__; _elm_startup_time = ecore_time_unix_get(); ret__ = elm_quicklaunch_fallback(argc, argv); elm_shutdown(); return ret__; }
#endif

#define __EFL_UI_IS_REQUIRED

#include "efl_general.h"

#ifndef __EFL_UI_IS_DEFINED
# error "You have an old efl_general.h installed in your local include/elementary-1/ remove it first."
#endif

/**************************************************************************/
/* General calls */

/**
 * Initialize Elementary
 *
 * @param[in] argc System's argument count value
 * @param[in] argv System's pointer to array of argument strings
 * @return The init counter value.
 *
 * This function initializes Elementary and increments a counter of
 * the number of calls to it. It returns the new counter's value.
 *
 * @warning This call is exported only for use by the @c ELM_MAIN()
 * macro. There is no need to use this if you use this macro (which
 * is highly advisable). An elm_main() should contain the entry
 * point code for your application, having the same prototype as
 * elm_init(), and @b not being static (putting the @c EAPI_MAIN symbol
 * in front of its type declaration is advisable). The @c
 * ELM_MAIN() call should be placed just after it.
 *
 * Example:
 * @dontinclude bg_example_01.c
 * @skip static void
 * @until ELM_MAIN
 *
 * See the full @ref bg_example_01_c "example".
 *
 * @see elm_shutdown().
 * @ingroup Elm_General
 */
EAPI int       elm_init(int argc, char **argv);

/**
 * Shut down Elementary
 *
 * @return The init counter value.
 *
 * This should be called at the end of your application, just
 * before it ceases to do any more processing. This will clean up
 * any permanent resources your application may have allocated via
 * Elementary that would otherwise persist.
 *
 * @see elm_init() for an example
 *
 * @note elm_shutdown() will iterate main loop until all ecore_evas are freed.
 * There is a possibility to call your ecore callbacks(timer, animator, event,
 * job, and etc.) in elm_shutdown()
 *
 * @ingroup Elm_General
 */
EAPI int       elm_shutdown(void);

/**
 * Run Elementary's main loop
 *
 * This call should be issued just after all initialization is
 * completed. This function will not return until elm_exit() is
 * called. It will keep looping, running the main
 * (event/processing) loop for Elementary.
 *
 * This function should be called once only from the same thread that
 * initted elementary, (elm_init(), eina_init(), ...) and should never
 * be nested. Never call it from within an instance of itself.
 *
 * @see elm_init() for an example
 *
 * @ingroup Elm_General
 */
EAPI void      elm_run(void);

/**
 * Ask to exit Elementary's main loop
 *
 * If this call is issued, it will flag the main loop to cease
 * processing and return back to its parent function (usually your
 * elm_main() function). This does not mean the main loop instantly quits.
 * So your ecore callbacks(timer, animator, event, job, and etc.) have chances
 * to be called even after elm_exit().
 *
 * @see elm_init() for an example. There, just after a request to
 * close the window comes, the main loop will be left.
 *
 * @note By using the appropriate #ELM_POLICY_QUIT on your Elementary
 * applications, you'll be able to get this function called automatically for you.
 *
 * @ingroup Elm_General
 */
EAPI void      elm_exit(void);

/**
 * Exposed symbol used only by macros and should not be used by apps
 */
EAPI void      elm_quicklaunch_mode_set(Eina_Bool ql_on);

/**
 * Exposed symbol used only by macros and should not be used by apps
 */
EAPI Eina_Bool elm_quicklaunch_mode_get(void);

/**
 * Exposed symbol used only by macros and should not be used by apps
 */
EAPI int       elm_quicklaunch_init(int argc, char **argv);

/**
 * Exposed symbol used only by macros and should not be used by apps
 */
EAPI int       elm_quicklaunch_sub_init(int argc, char **argv);

/**
 * Exposed symbol used only by macros and should not be used by apps
 */
EAPI int       elm_quicklaunch_sub_shutdown(void);

/**
 * Exposed symbol used only by macros and should not be used by apps
 */
EAPI int       elm_quicklaunch_shutdown(void);

/**
 * Exposed symbol used only by macros and should not be used by apps
 */
EAPI void      elm_quicklaunch_seed(void);

/**
 * Exposed symbol used only by macros and should not be used by apps
 */
EAPI Eina_Bool elm_quicklaunch_prepare(int argc, char **argv, const char *cwd);

/**
 * Exposed symbol used only by macros and should not be used by apps
 */
EAPI Eina_Bool efl_quicklaunch_prepare(int argc, char **argv, const char *cwd);

/**
 * Exposed symbol used only by macros and should not be used by apps
 */
EAPI int elm_quicklaunch_fork(int argc, char **argv, char *cwd, void (*postfork_func) (void *data), void *postfork_data);

/**
 * Exposed symbol used only by macros and should not be used by apps
 */
EAPI void      elm_quicklaunch_cleanup(void);

/**
 * Exposed symbol used only by macros and should not be used by apps
 */
EAPI int       elm_quicklaunch_fallback(int argc, char **argv);

/**
 * Exposed symbol used only by macros and should not be used by apps
 */
EAPI char     *elm_quicklaunch_exe_path_get(const char *exe, const char *cwd);

/**
 * Set a new policy's value (for a given policy group/identifier).
 *
 * @param policy policy identifier, as in @ref Elm_Policy.
 * @param value policy value, which depends on the identifier
 *
 * @return @c EINA_TRUE on success or @c EINA_FALSE, on error.
 *
 * Elementary policies define applications' behavior,
 * somehow. These behaviors are divided in policy groups
 * (see #Elm_Policy enumeration). This call will emit the Ecore
 * event #ELM_EVENT_POLICY_CHANGED, which can be hooked at with
 * handlers. An #Elm_Event_Policy_Changed struct will be passed,
 * then.
 *
 * @note Currently, we have only one policy identifier/group
 * (#ELM_POLICY_QUIT), which has two possible values.
 *
 * @ingroup Elm_General
 */
EAPI Eina_Bool elm_policy_set(unsigned int policy, int value);

/**
 * Get the policy value for given policy identifier.
 *
 * @param policy policy identifier, as in #Elm_Policy.
 * @return The currently set policy value, for that
 * identifier. Will be @c 0 if @p policy passed is invalid.
 *
 * @ingroup Elm_General
 */
EAPI int       elm_policy_get(unsigned int policy);

/**
 * Change the language of the current application
 *
 * The @p lang passed must be the full name of the locale to use, for
 * example "en_US.utf8" or "es_ES@euro".
 *
 * Changing language with this function will make Elementary run through
 * all its widgets, translating strings set with
 * elm_object_domain_translatable_part_text_set(). This way, an entire
 * UI can have its language changed without having to restart the program.
 *
 * For more complex cases, like having formatted strings that need
 * translation, widgets will also emit a "language,changed" signal that
 * the user can listen to and manually translate the text.
 *
 * @param lang Language to set, must be the full name of the locale
 *
 * @ingroup Elm_General
 */
EAPI void      elm_language_set(const char *lang);

typedef enum _Elm_Process_State
{
   ELM_PROCESS_STATE_FOREGROUND, /*< The process is in a foreground/active/running state - work as normal. @since 1.12 */
   ELM_PROCESS_STATE_BACKGROUND /*< The process is in the bacgkround, so you may want to stop animating, fetching data as often etc. @since 1.12 */
} Elm_Process_State; /** The state of the process as a whole. @since 1.12 */

/**
 * Get the process state as a while
 *
 * @return The current process state
 *
 * The process may logically be some runnable state. a "foreground" application
 * runs as normal and may be user-visible or "active" in some way. A
 * background application is not user-visible or otherwise important and
 * likely should release resources and not wake up often or process much.
 *
 * @ingroup Elm_General
 * @since 1.12
 */
EAPI Elm_Process_State  elm_process_state_get(void);


/* legacy to eo translation */

typedef Efl_Ui_Focus_Autoscroll_Mode       Elm_Focus_Autoscroll_Mode;
#define ELM_FOCUS_AUTOSCROLL_MODE_SHOW     EFL_UI_FOCUS_AUTOSCROLL_MODE_SHOW
#define ELM_FOCUS_AUTOSCROLL_MODE_NONE     EFL_UI_FOCUS_AUTOSCROLL_MODE_NONE
#define ELM_FOCUS_AUTOSCROLL_MODE_BRING_IN EFL_UI_FOCUS_AUTOSCROLL_MODE_BRING_IN

typedef Efl_Ui_Softcursor_Mode             Elm_Softcursor_Mode;
#define ELM_SOFTCURSOR_MODE_AUTO           EFL_UI_SOFTCURSOR_MODE_AUTO
#define ELM_SOFTCURSOR_MODE_ON             EFL_UI_SOFTCURSOR_MODE_ON
#define ELM_SOFTCURSOR_MODE_OFF            EFL_UI_SOFTCURSOR_MODE_OFF

typedef Efl_Ui_Slider_Indicator_Visible_Mode       Elm_Slider_Indicator_Visible_Mode;
#define ELM_SLIDER_INDICATOR_VISIBLE_MODE_DEFAULT  EFL_UI_SLIDER_INDICATOR_VISIBLE_MODE_ON_DRAG
#define ELM_SLIDER_INDICATOR_VISIBLE_MODE_ALWAYS   EFL_UI_SLIDER_INDICATOR_VISIBLE_MODE_ALWAYS
#define ELM_SLIDER_INDICATOR_VISIBLE_MODE_ON_FOCUS EFL_UI_SLIDER_INDICATOR_VISIBLE_MODE_ON_FOCUS
#define ELM_SLIDER_INDICATOR_VISIBLE_MODE_NONE     EFL_UI_SLIDER_INDICATOR_VISIBLE_MODE_NONE

typedef Efl_Ui_Focus_Move_Policy           Elm_Focus_Move_Policy;
#define ELM_FOCUS_MOVE_POLICY_CLICK        EFL_UI_FOCUS_MOVE_POLICY_CLICK
#define ELM_FOCUS_MOVE_POLICY_IN           EFL_UI_FOCUS_MOVE_POLICY_IN
#define ELM_FOCUS_MOVE_POLICY_KEY_ONLY     EFL_UI_FOCUS_MOVE_POLICY_KEY_ONLY

/**
 * @}
 */
