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

/** Data on the event when an Elementary policy has changed
 *
 * @ingroup Elm_Event
 */
typedef struct _Elm_Event_Policy_Changed
{
  unsigned int policy; /**< the policy identifier */
  int new_value; /**< value the policy had before the change */
  int old_value; /**< new value the policy got */
} Elm_Event_Policy_Changed;

/** Possible values for the @ref ELM_POLICY_EXIT policy.
 *
 * @since 1.8
 *
 * @ingroup Elm
 */
typedef enum
{
  ELM_POLICY_EXIT_NONE = 0, /**< just quit the main loop on elm_exit() */
  ELM_POLICY_EXIT_WINDOWS_DEL /**< delete all the windows after quitting the
                               * main loop */
} Elm_Policy_Exit;

/** Possible values for the @ref ELM_POLICY_THROTTLE policy.
 *
 * @since 1.8
 *
 * @ingroup Elm
 */
typedef enum
{
  ELM_POLICY_THROTTLE_CONFIG = 0, /**< do whatever elementary config is
                                   * configured to do */
  ELM_POLICY_THROTTLE_HIDDEN_ALWAYS, /**< always throttle when all windows are
                                      * no longer visible */
  ELM_POLICY_THROTTLE_NEVER /**< never throttle when windows are all hidden,
                             * regardless of config settings */
} Elm_Policy_Throttle;

/** Possible values for the #ELM_OBJECT_MULTI_SELECT_MODE policy.
 *
 * @since 1.8
 *
 * @ingroup Elm_Object
 */
typedef enum
{
  ELM_OBJECT_MULTI_SELECT_MODE_DEFAULT = 0, /**< default multiple select mode */
  ELM_OBJECT_MULTI_SELECT_MODE_WITH_CONTROL, /**< disallow mutiple selection
                                              * when clicked without control
                                              * key pressed */
  ELM_OBJECT_MULTI_SELECT_MODE_MAX /**< canary value: any value greater or equal
                                    * to ELM_OBJECT_MULTI_SELECT_MODE_MAX is
                                    * forbidden. */
} Elm_Object_Multi_Select_Mode;

/**
 * @brief Line wrapping types. Type of word or character wrapping to use.
 *
 * See also @ref elm_entry_line_wrap_set, @ref
 * elm_popup_content_text_wrap_type_set, @ref elm_label_line_wrap_set.
 *
 * @ingroup Elm_Wrap
 */
typedef enum
{
  ELM_WRAP_NONE = 0, /**< No wrap - value is zero. */
  ELM_WRAP_CHAR, /**< Char wrap - wrap between characters. */
  ELM_WRAP_WORD, /**< Word wrap - wrap in allowed wrapping points (as defined in
                  * the unicode standard). */
  ELM_WRAP_MIXED, /**< Mixed wrap - Word wrap, and if that fails, char wrap. */
  ELM_WRAP_LAST /**< Sentinel value to indicate last enum field during iteration
                 */
} Elm_Wrap_Type;

/** Text Format types.
 *
 * @ingroup Elm
 */
typedef enum
{
  ELM_TEXT_FORMAT_PLAIN_UTF8 = 0, /**< Plain UTF8 type */
  ELM_TEXT_FORMAT_MARKUP_UTF8 /**< Markup UTF8 type */
} Elm_Text_Format;

/** Enum of entry's copy & paste policy.
 *
 * @ingroup Elm
 */
typedef enum
{
  ELM_CNP_MODE_MARKUP = 0, /**< copy & paste text with markup tag */
  ELM_CNP_MODE_NO_IMAGE, /**< copy & paste text without item(image) tag */
  ELM_CNP_MODE_PLAINTEXT /**< copy & paste text without markup tag */
} Elm_Cnp_Mode;

/** Defines if the item is of any special type (has subitems or it's the index
 * of a group), or is just a simple item.
 *
 * @ingroup Elm_Genlist_Item
 */
typedef enum
{
  ELM_GENLIST_ITEM_NONE = 0, /**< Simple item. */
  ELM_GENLIST_ITEM_TREE = 1 /* 1 >> 0 */, /**< This may be expanded and have
                                           * child items. */
  ELM_GENLIST_ITEM_GROUP = 2 /* 1 >> 1 */, /**< An index item of a group of
                                            * items. this item can have child
                                            * items. */
  ELM_GENLIST_ITEM_MAX = 4 /* 1 >> 2 */ /**< Sentinel value to indicate last
                                         * enum field during iteration */
} Elm_Genlist_Item_Type;

/** Defines the type of the item part Used while updating item's parts It can
 * be used at updating multi fields.
 *
 * @ingroup Elm_Genlist_Item
 */
typedef enum
{
  ELM_GENLIST_ITEM_FIELD_ALL = 0, /**< Type all */
  ELM_GENLIST_ITEM_FIELD_TEXT = 1 /* 1 >> 0 */, /**< Type text */
  ELM_GENLIST_ITEM_FIELD_CONTENT = 2 /* 1 >> 1 */, /**< Type content */
  ELM_GENLIST_ITEM_FIELD_STATE = 4 /* 1 >> 2 */ /**< Type state */
} Elm_Genlist_Item_Field_Type;

/** Defines where to position the item in the genlist.
 *
 * @ingroup Elm_Genlist_Item
 */
typedef enum
{
  ELM_GENLIST_ITEM_SCROLLTO_NONE = 0, /**< Nothing will happen, Don't use this
                                       * value. */
  ELM_GENLIST_ITEM_SCROLLTO_IN = 1 /* 1 >> 0 */, /**< To the nearest viewport.
                                                  */
  ELM_GENLIST_ITEM_SCROLLTO_TOP = 2 /* 1 >> 1 */, /**< To the top of viewport.
                                                   */
  ELM_GENLIST_ITEM_SCROLLTO_MIDDLE = 4 /* 1 >> 2 */, /**< To the middle of
                                                      * viewport. */
  ELM_GENLIST_ITEM_SCROLLTO_BOTTOM = 8 /* 1 >> 3 */ /**< To the bottom of
                                                     * viewport. */
} Elm_Genlist_Item_Scrollto_Type;

/** Defines where to position the item in the genlist.
 *
 * @ingroup Elm_Gengrid_Item
 */
typedef enum
{
  ELM_GENGRID_ITEM_SCROLLTO_NONE = 0, /**< No scrollto. */
  ELM_GENGRID_ITEM_SCROLLTO_IN = 1 /* 1 >> 0 */, /**< To the nearest viewport.
                                                  */
  ELM_GENGRID_ITEM_SCROLLTO_TOP = 2 /* 1 >> 1 */, /**< To the top of viewport.
                                                   */
  ELM_GENGRID_ITEM_SCROLLTO_MIDDLE = 4 /* 1 >> 2 */, /**< To the middle of
                                                      * viewport. */
  ELM_GENGRID_ITEM_SCROLLTO_BOTTOM = 8 /* 1 >> 3 */ /**< To the bottom of
                                                     * viewport. */
} Elm_Gengrid_Item_Scrollto_Type;

/** Defines the type of the item part Used while updating item's parts. It can
 * be used at updating multi fields.
 *
 * @ingroup Elm_Gengrid_Item
 */
typedef enum
{
  ELM_GENGRID_ITEM_FIELD_ALL = 0, /**< Type all */
  ELM_GENGRID_ITEM_FIELD_TEXT = 1 /* 1 >> 0 */, /**< Type text */
  ELM_GENGRID_ITEM_FIELD_CONTENT = 2 /* 1 >> 1 */, /**< Type content */
  ELM_GENGRID_ITEM_FIELD_STATE = 4 /* 1 >> 2 */ /**< Type state */
} Elm_Gengrid_Item_Field_Type;

/**
 * @brief Set list's resizing behavior, transverse axis scrolling and items
 * cropping. See each mode's description for more details.
 *
 * @note Default value is @ref ELM_LIST_SCROLL.
 *
 * Values here don't work as bitmasks -- only one can be chosen at a time.
 *
 * @ingroup Elm_List
 */
typedef enum
{
  ELM_LIST_COMPRESS = 0, /**< The list won't set any of its size hints to inform
                          * how a possible container should resize it. Then, if
                          * it's not created as a "resize object", it might end
                          * with zeroed dimensions. The list will respect the
                          * container's geometry and, if any of its items won't
                          * fit into its transverse axis, one won't be able to
                          * scroll it in that direction. */
  ELM_LIST_SCROLL, /**< Default value. This is the same as #ELM_LIST_COMPRESS,
                    * with the exception that if any of its items won't fit
                    * into its transverse axis, one will be able to scroll it
                    * in that direction. */
  ELM_LIST_LIMIT, /**< Sets a minimum size hint on the list object, so that
                   * containers may respect it (and resize itself to fit the
                   * child properly). More specifically, a minimum size hint
                   * will be set for its transverse axis, so that the largest
                   * item in that direction fits well. This is naturally bound
                   * by the list object's maximum size hints, set externally.
                   */
  ELM_LIST_EXPAND, /**< Besides setting a minimum size on the transverse axis,
                    * just like on @ref ELM_LIST_LIMIT, the list will set a
                    * minimum size on the longitudinal axis, trying to reserve
                    * space to all its children to be visible at a time. . This
                    * is naturally bound by the list object's maximum size
                    * hints, set externally. */
  ELM_LIST_LAST /**< Indicates error if returned by elm_list_mode_get(). */
} Elm_List_Mode;

typedef enum
{
  ELM_FOCUS_PREVIOUS = 0, /**< previous direction */
  ELM_FOCUS_NEXT = 1, /**< next direction */
  ELM_FOCUS_UP = 2, /**< up direction */
  ELM_FOCUS_DOWN = 3, /**< down direction */
  ELM_FOCUS_RIGHT = 4, /**< right direction */
  ELM_FOCUS_LEFT = 5, /**< left direction */
  ELM_FOCUS_LAST = 6 /**< last direction */
} Elm_Focus_Direction;

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

typedef enum
{
  ELM_FOCUS_AUTOSCROLL_MODE_SHOW = 0, /**< Directly show the focused region
                                          * or item automatically. */
  ELM_FOCUS_AUTOSCROLL_MODE_NONE, /**< Do not show the focused region or item
                                      * automatically. */
  ELM_FOCUS_AUTOSCROLL_MODE_BRING_IN /**< Bring in the focused region or item
                                         * automatically which might invole the
                                         * scrolling. */
} Elm_Focus_Autoscroll_Mode;

typedef enum
{
  ELM_SOFTCURSOR_MODE_AUTO = 0, /**< Auto-detect if a software cursor should
                                    * be used (default). */
  ELM_SOFTCURSOR_MODE_ON, /**< Always use a softcursor. */
  ELM_SOFTCURSOR_MODE_OFF /**< Never use a softcursor. */
} Elm_Softcursor_Mode;

typedef enum
{
  ELM_SLIDER_INDICATOR_VISIBLE_MODE_DEFAULT = 0, /**< show indicator on mouse
                                                     * down or change in slider
                                                     * value */
  ELM_SLIDER_INDICATOR_VISIBLE_MODE_ALWAYS, /**< Always show the indicator.
                                                */
  ELM_SLIDER_INDICATOR_VISIBLE_MODE_ON_FOCUS, /**< Show the indicator on
                                                  * focus */
  ELM_SLIDER_INDICATOR_VISIBLE_MODE_NONE /**< Never show the indicator */
} Elm_Slider_Indicator_Visible_Mode;

typedef enum
{
  ELM_FOCUS_MOVE_POLICY_CLICK = 0, /**< Move focus by mouse click or touch.
                                       * Elementary focus is set on mouse click
                                       * and this is checked at mouse up time.
                                       * (default) */
  ELM_FOCUS_MOVE_POLICY_IN, /**< Move focus by mouse in. Elementary focus is
                                * set on mouse move when the mouse pointer is
                                * moved into an object. */
  ELM_FOCUS_MOVE_POLICY_KEY_ONLY /**< Move focus by key. Elementary focus is
                                     * set on key input like Left, Right, Up,
                                     * Down, Tab, or Shift+Tab. */
} Elm_Focus_Move_Policy;

/**
 * @}
 */
