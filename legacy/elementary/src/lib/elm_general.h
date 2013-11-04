/**
 * @defgroup General General
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
 * @addtogroup General
 * @{
 */

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
 * @typedef Elm_Event_Policy_Changed
 *
 * Data on the event when an Elementary policy has changed
 */
typedef struct _Elm_Event_Policy_Changed Elm_Event_Policy_Changed;

/**
 * @struct _Elm_Event_Policy_Changed
 *
 * Data on the event when an Elementary policy has changed
 */
struct _Elm_Event_Policy_Changed
{
   unsigned int policy; /**< the policy identifier */
   int          new_value; /**< value the policy had before the change */
   int          old_value; /**< new value the policy got */
};

/**
 * Policy identifiers.
 */
typedef enum
{
   ELM_POLICY_QUIT, /**< under which circumstances the application
                     * should quit automatically. @see
                     * Elm_Policy_Quit.
                     */
   ELM_POLICY_EXIT, /**< defines elm_exit() behaviour. @see Elm_Policy_Exit.
                     * @since 1.8
                     */
   ELM_POLICY_THROTTLE, /**< defines how throttling should work @see Elm_Policy_Throttle
                         * @since 1.8
                         */
   ELM_POLICY_LAST
} Elm_Policy; /**< Elementary policy identifiers/groups enumeration.  @see elm_policy_set() */

/**
 * Possible values for the #ELM_POLICY_QUIT policy
 */
typedef enum
{
   ELM_POLICY_QUIT_NONE = 0, /**< never quit the application
                              * automatically */
   ELM_POLICY_QUIT_LAST_WINDOW_CLOSED /**< quit when the
                                       * application's last
                                       * window is closed */
} Elm_Policy_Quit;

/**
 * Possible values for the #ELM_POLICY_EXIT policy.
 * @since 1.8
 */
typedef enum
{
   ELM_POLICY_EXIT_NONE = 0, /**< just quit the main loop on elm_exit() */
   ELM_POLICY_EXIT_WINDOWS_DEL /**< delete all the windows after quitting
                                * the main loop */
} Elm_Policy_Exit;

/**
 * Possible values for the #ELM_POLICY_THROTTLE policy.
 * @since 1.8
 */
typedef enum
{
   ELM_POLICY_THROTTLE_CONFIG = 0, /**< do whatever elementary config is configured to do */
   ELM_POLICY_THROTTLE_HIDDEN_ALWAYS, /**< always throttle when all windows are no longer visible */
   ELM_POLICY_THROTTLE_NEVER /**< never throttle when windows are all hidden, regardless of config settings */
} Elm_Policy_Throttle;

typedef enum
{
   ELM_OBJECT_SELECT_MODE_DEFAULT = 0, /**< default select mode */
   ELM_OBJECT_SELECT_MODE_ALWAYS, /**< always select mode */
   ELM_OBJECT_SELECT_MODE_NONE, /**< no select mode */
   ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY, /**< no select mode with no finger size rule*/
   ELM_OBJECT_SELECT_MODE_MAX
} Elm_Object_Select_Mode;

typedef enum
{
   ELM_OBJECT_MULTI_SELECT_MODE_DEFAULT = 0, /**< default multiple select mode */
   ELM_OBJECT_MULTI_SELECT_MODE_WITH_CONTROL, /**< disallow mutiple selection when clicked without control key pressed */
   ELM_OBJECT_MULTI_SELECT_MODE_MAX
} Elm_Object_Multi_Select_Mode;

typedef Eina_Bool             (*Elm_Event_Cb)(void *data, Evas_Object *obj, Evas_Object *src, Evas_Callback_Type type, void *event_info); /**< Function prototype definition for callbacks on input events happening on Elementary widgets. @a data will receive the user data pointer passed to elm_object_event_callback_add(). @a src will be a pointer to the widget on which the input event took place. @a type will get the type of this event and @a event_info, the struct with details on this event. */

extern EAPI double _elm_startup_time;

#ifndef ELM_LIB_QUICKLAUNCH
#define ELM_MAIN() int main(int argc, char **argv) { int ret; _elm_startup_time = ecore_time_unix_get(); elm_init(argc, argv); ret = elm_main(argc, argv); return ret; } /**< macro to be used after the elm_main() function */
#else
/** @deprecated macro to be used after the elm_main() function.
 * Do not define ELM_LIB_QUICKLAUNCH
 * Compile your programs with -fpie and -pie -rdynamic instead, to generate a single binary (linkable executable).
 */
#define ELM_MAIN() int main(int argc, char **argv) { _elm_startup_time = ecore_time_unix_get(); return elm_quicklaunch_fallback(argc, argv); }
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
 * @ingroup General
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
 * @ingroup General
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
 * @see elm_init() for an example
 *
 * @ingroup General
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
 * @ingroup General
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
EAPI Eina_Bool elm_quicklaunch_fork(int argc, char **argv, char *cwd, void (postfork_func) (void *data), void *postfork_data);

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
 * @ingroup General
 */
EAPI Eina_Bool elm_policy_set(unsigned int policy, int value);

/**
 * Get the policy value for given policy identifier.
 *
 * @param policy policy identifier, as in #Elm_Policy.
 * @return The currently set policy value, for that
 * identifier. Will be @c 0 if @p policy passed is invalid.
 *
 * @ingroup General
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
 * @ingroup General
 */
EAPI void      elm_language_set(const char *lang);

/**
 * @}
 */
