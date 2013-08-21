/**
 * Structure used to report load errors.
 *
 * Load errors are reported as signal by elm_web. All the strings are
 * temporary references and should @b not be used after the signal
 * callback returns. If it's required, make copies with strdup() or
 * eina_stringshare_add() (they are not even guaranteed to be
 * stringshared, so must use eina_stringshare_add() and not
 * eina_stringshare_ref()).
 */
typedef struct _Elm_Web_Frame_Load_Error Elm_Web_Frame_Load_Error;

/**
 * Structure used to report load errors.
 *
 * Load errors are reported as signal by elm_web. All the strings are
 * temporary references and should @b not be used after the signal
 * callback returns. If it's required, make copies with strdup() or
 * eina_stringshare_add() (they are not even guaranteed to be
 * stringshared, so must use eina_stringshare_add() and not
 * eina_stringshare_ref()).
 */
struct _Elm_Web_Frame_Load_Error
{
   int          code; /**< Numeric error code */
   Eina_Bool    is_cancellation; /**< Error produced by canceling a request */
   const char  *domain; /**< Error domain name */
   const char  *description; /**< Error description (already localized) */
   const char  *failing_url; /**< The URL that failed to load */
   Evas_Object *frame; /**< Frame object that produced the error */
};

/**
 * The possibles types that the items in a menu can be
 */
typedef enum
{
   ELM_WEB_MENU_SEPARATOR,
   ELM_WEB_MENU_GROUP,
   ELM_WEB_MENU_OPTION
} Elm_Web_Menu_Item_Type;

/**
 * Structure describing the items in a menu
 */
typedef struct _Elm_Web_Menu_Item Elm_Web_Menu_Item;

/**
 * Structure describing the items in a menu
 */
struct _Elm_Web_Menu_Item
{
   const char            *text; /**< The text for the item */
   Elm_Web_Menu_Item_Type type; /**< The type of the item */
};

/**
 * Structure describing the menu of a popup
 *
 * This structure will be passed as the @c event_info for the "popup,create"
 * signal, which is emitted when a dropdown menu is opened. Users wanting
 * to handle these popups by themselves should listen to this signal and
 * set the @c handled property of the struct to @c EINA_TRUE. Leaving this
 * property as @c EINA_FALSE means that the user will not handle the popup
 * and the default implementation will be used.
 *
 * When the popup is ready to be dismissed, a "popup,willdelete" signal
 * will be emitted to notify the user that it can destroy any objects and
 * free all data related to it.
 *
 * @see elm_web_popup_selected_set()
 * @see elm_web_popup_destroy()
 *
 * @ingroup Web
 */
typedef struct _Elm_Web_Menu Elm_Web_Menu;

/**
 * Structure describing the menu of a popup
 *
 * This structure will be passed as the @c event_info for the "popup,create"
 * signal, which is emitted when a dropdown menu is opened. Users wanting
 * to handle these popups by themselves should listen to this signal and
 * set the @c handled property of the struct to @c EINA_TRUE. Leaving this
 * property as @c EINA_FALSE means that the user will not handle the popup
 * and the default implementation will be used.
 *
 * When the popup is ready to be dismissed, a "popup,willdelete" signal
 * will be emitted to notify the user that it can destroy any objects and
 * free all data related to it.
 *
 * @see elm_web_popup_selected_set()
 * @see elm_web_popup_destroy()
 *
 * @ingroup Web
 */
struct _Elm_Web_Menu
{
   Eina_List *items; /**< List of #Elm_Web_Menu_Item */
   int        x; /**< The X position of the popup, relative to the elm_web object */
   int        y; /**< The Y position of the popup, relative to the elm_web object */
   int        width; /**< Width of the popup menu */
   int        height; /**< Height of the popup menu */

   Eina_Bool  handled : 1; /**< Set to @c EINA_TRUE by the user to indicate that the popup has been handled and the default implementation should be ignored. Leave as @c EINA_FALSE otherwise. */
};

typedef struct _Elm_Web_Download Elm_Web_Download;
struct _Elm_Web_Download
{
   const char *url;
};

/**
 * Types of zoom available.
 */
typedef enum
{
   ELM_WEB_ZOOM_MODE_MANUAL = 0, /**< Zoom controlled normally by elm_web_zoom_set */
   ELM_WEB_ZOOM_MODE_AUTO_FIT, /**< Zoom until content fits in web object */
   ELM_WEB_ZOOM_MODE_AUTO_FILL, /**< Zoom until content fills web object */
   ELM_WEB_ZOOM_MODE_LAST /**< Sentinel value to indicate end */
} Elm_Web_Zoom_Mode;

/**
 * Opaque handler containing the features (such as statusbar, menubar, etc)
 * that are to be set on a newly requested window.
 */
typedef struct _Elm_Web_Window_Features Elm_Web_Window_Features;


/**
 * Definitions of web window features.
 *
 */
typedef enum
{
   ELM_WEB_WINDOW_FEATURE_TOOLBAR,
   ELM_WEB_WINDOW_FEATURE_STATUSBAR,
   ELM_WEB_WINDOW_FEATURE_SCROLLBARS,
   ELM_WEB_WINDOW_FEATURE_MENUBAR,
   ELM_WEB_WINDOW_FEATURE_LOCATIONBAR,
   ELM_WEB_WINDOW_FEATURE_FULLSCREEN
} Elm_Web_Window_Feature_Flag;

/**
 * Callback type for the create_window hook.
 *
 * @param data User data pointer set when setting the hook function.
 * @param obj The elm_web object requesting the new window.
 * @param js Set to @c EINA_TRUE if the request was originated from
 * JavaScript. @c EINA_FALSE otherwise.
 * @param window_features A pointer of #Elm_Web_Window_Features indicating
 * the features requested for the new window.
 *
 * @return The @c elm_web widget where the request will be loaded. That is,
 * if a new window or tab is created, the elm_web widget in it should be
 * returned, and @b NOT the window object. Returning @c NULL should cancel
 * the request.
 *
 * @see elm_web_window_create_hook_set()
 *
 * @ingroup Web
 */
typedef Evas_Object *(*Elm_Web_Window_Open)(void *data, Evas_Object *obj, Eina_Bool js, const Elm_Web_Window_Features *window_features);

/**
 * Callback type for the JS alert hook.
 *
 * @param data User data pointer set when setting the hook function.
 * @param obj The elm_web object requesting the new window.
 * @param message The message to show in the alert dialog.
 *
 * @return The object representing the alert dialog.
 * Elm_Web will run a second main loop to handle the dialog and normal
 * flow of the application will be restored when the object is deleted, so
 * the user should handle the popup properly in order to delete the object
 * when the action is finished.
 * If the function returns @c NULL the popup will be ignored.
 *
 * @see elm_web_dialog_alert_hook_set()
 *
 * @ingroup Web
 */
typedef Evas_Object *(*Elm_Web_Dialog_Alert)(void *data, Evas_Object *obj, const char *message);

/**
 * Callback type for the JS confirm hook.
 *
 * @param data User data pointer set when setting the hook function.
 * @param obj The elm_web object requesting the new window.
 * @param message The message to show in the confirm dialog.
 * @param ret Pointer to store the user selection. @c EINA_TRUE if
 * the user selected @c Ok, @c EINA_FALSE otherwise.
 *
 * @return The object representing the confirm dialog.
 * Elm_Web will run a second main loop to handle the dialog and normal
 * flow of the application will be restored when the object is deleted, so
 * the user should handle the popup properly in order to delete the object
 * when the action is finished.
 * If the function returns @c NULL the popup will be ignored.
 *
 * @see elm_web_dialog_confirm_hook_set()
 *
 * @ingroup Web
 */
typedef Evas_Object *(*Elm_Web_Dialog_Confirm)(void *data, Evas_Object *obj, const char *message, Eina_Bool *ret);

/**
 * Callback type for the JS prompt hook.
 *
 * @param data User data pointer set when setting the hook function.
 * @param obj The elm_web object requesting the new window.
 * @param message The message to show in the prompt dialog.
 * @param def_value The default value to present the user in the entry
 * @param value Pointer to store the value given by the user. Must
 * be a malloc'ed string or @c NULL if the user canceled the popup.
 * @param ret Pointer to store the user selection. @c EINA_TRUE if
 * the user selected @c Ok, @c EINA_FALSE otherwise.
 *
 * @return The object representing the prompt dialog.
 * Elm_Web will run a second main loop to handle the dialog and normal
 * flow of the application will be restored when the object is deleted, so
 * the user should handle the popup properly in order to delete the object
 * when the action is finished.
 * If the function returns @c NULL the popup will be ignored.
 *
 * @see elm_web_dialog_prompt_hook_set()
 *
 * @ingroup Web
 */
typedef Evas_Object *(*Elm_Web_Dialog_Prompt)(void *data, Evas_Object *obj, const char *message, const char *def_value, const char **value, Eina_Bool *ret);

/**
 * Callback type for the JS file selector hook.
 *
 * @param data User data pointer set when setting the hook function.
 * @param obj The elm_web object requesting the new window.
 * @param allows_multiple @c EINA_TRUE if multiple files can be selected.
 * @param accept_types Mime types accepted.
 * @param selected Pointer to store the list of malloc'ed strings
 * containing the path to each file selected. Must be @c NULL if the file
 * dialog is canceled.
 * @param ret Pointer to store the user selection. @c EINA_TRUE if
 * the user selected @c Ok, @c EINA_FALSE otherwise.
 *
 * @return The object representing the file selector dialog.
 * Elm_Web will run a second main loop to handle the dialog and normal
 * flow of the application will be restored when the object is deleted, so
 * the user should handle the popup properly in order to delete the object
 * when the action is finished.
 * If the function returns @c NULL the popup will be ignored.
 *
 * @see elm_web_dialog_file selector_hook_set()
 *
 * @ingroup Web
 */
typedef Evas_Object *(*Elm_Web_Dialog_File_Selector)(void *data, Evas_Object *obj, Eina_Bool allows_multiple, Eina_List *accept_types, Eina_List **selected, Eina_Bool *ret);

/**
 * Callback type for the JS console message hook.
 *
 * When a console message is added from JavaScript, any set function to the
 * console message hook will be called for the user to handle. There is no
 * default implementation of this hook.
 *
 * @param data User data pointer set when setting the hook function.
 * @param obj The elm_web object that originated the message.
 * @param message The message sent.
 * @param line_number The line number.
 * @param source_id Source id.
 *
 * @see elm_web_console_message_hook_set()
 *
 * @ingroup Web
 */
typedef void (*Elm_Web_Console_Message)(void *data, Evas_Object *obj, const char *message, unsigned int line_number, const char *source_id);

/**
 * Add a new web object to the parent.
 *
 * @param parent The parent object.
 * @return The new object or @c NULL if it cannot be created.
 *
 * @see elm_web_uri_set()
 * @see elm_web_webkit_view_get()
 *
 * @ingroup Web
 */
EAPI Evas_Object      *elm_web_add(Evas_Object *parent);

/**
 * Sets the zoom mode to use
 *
 * The modes can be any of those defined in ::Elm_Web_Zoom_Mode, except
 * ::ELM_WEB_ZOOM_MODE_LAST. The default is ::ELM_WEB_ZOOM_MODE_MANUAL.
 *
 * ::ELM_WEB_ZOOM_MODE_MANUAL means the zoom level will be controlled
 * with the elm_web_zoom_set() function.
 * ::ELM_WEB_ZOOM_MODE_AUTO_FIT will calculate the needed zoom level to
 * make sure the entirety of the web object's contents are shown.
 * ::ELM_WEB_ZOOM_MODE_AUTO_FILL will calculate the needed zoom level to
 * fit the contents in the web object's size, without leaving any space
 * unused.
 *
 * @param obj The web object
 * @param mode The mode to set
 *
 * @ingroup Web
 */
EAPI void              elm_web_zoom_mode_set(Evas_Object *obj, Elm_Web_Zoom_Mode mode);

/**
 * Get the currently set zoom mode
 *
 * @param obj The web object
 *
 * @return The current zoom mode set for the object,
 * or ::ELM_WEB_ZOOM_MODE_LAST on error
 *
 * @ingroup Web
 */
EAPI Elm_Web_Zoom_Mode elm_web_zoom_mode_get(const Evas_Object *obj);

/**
 * Get boolean properties from Elm_Web_Window_Features
 * (such as statusbar, menubar, etc) that are on a window.
 *
 * @param wf The web window features object
 * @param flag The web window feature flag whose value is required.
 *
 * @return @c EINA_TRUE if the flag is set, @c EINA_FALSE otherwise
 *
 * @ingroup Web
 */
EAPI Eina_Bool              elm_web_window_features_property_get(const Elm_Web_Window_Features *wf, Elm_Web_Window_Feature_Flag flag);

/**
 *
 * TODO : Add documentation.
 *
 * @param wf The web window features object
 * @param x, y, w, h - the co-ordinates of the web view window.
 *
 * @ingroup Web
 */
EAPI void              elm_web_window_features_region_get(const Elm_Web_Window_Features *wf, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h);

EAPI void              elm_web_window_features_ref(Elm_Web_Window_Features *wf);
EAPI void              elm_web_window_features_unref(Elm_Web_Window_Features *wf);
