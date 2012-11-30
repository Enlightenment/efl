#ifndef ELM_PREFS_H
#define ELM_PREFS_H

#define ELM_OBJ_PREFS_CLASS elm_obj_prefs_class_get()

const Eo_Class *elm_obj_prefs_class_get(void) EINA_CONST;

extern EAPI Eo_Op ELM_OBJ_PREFS_BASE_ID;

enum
{
   ELM_OBJ_PREFS_SUB_ID_FILE_SET,
   ELM_OBJ_PREFS_SUB_ID_FILE_GET,
   ELM_OBJ_PREFS_SUB_ID_DATA_SET,
   ELM_OBJ_PREFS_SUB_ID_DATA_GET,
   ELM_OBJ_PREFS_SUB_ID_AUTOSAVE_SET,
   ELM_OBJ_PREFS_SUB_ID_AUTOSAVE_GET,
   ELM_OBJ_PREFS_SUB_ID_RESET,
   ELM_OBJ_PREFS_SUB_ID_ITEM_VALUE_SET,
   ELM_OBJ_PREFS_SUB_ID_ITEM_VALUE_GET,
   ELM_OBJ_PREFS_SUB_ID_ITEM_OBJECT_GET,
   ELM_OBJ_PREFS_SUB_ID_ITEM_VISIBLE_SET,
   ELM_OBJ_PREFS_SUB_ID_ITEM_VISIBLE_GET,
   ELM_OBJ_PREFS_SUB_ID_ITEM_DISABLED_SET,
   ELM_OBJ_PREFS_SUB_ID_ITEM_DISABLED_GET,
   ELM_OBJ_PREFS_SUB_ID_ITEM_EDITABLE_SET,
   ELM_OBJ_PREFS_SUB_ID_ITEM_EDITABLE_GET,
   ELM_OBJ_PREFS_SUB_ID_ITEM_SWALLOW,
   ELM_OBJ_PREFS_SUB_ID_ITEM_UNSWALLOW,
   ELM_OBJ_PREFS_SUB_ID_LAST
};

#define ELM_OBJ_PREFS_ID(sub_id) (ELM_OBJ_PREFS_BASE_ID + sub_id)

/**
 * @def elm_obj_prefs_file_set
 *
 * Set file and page to populate a given prefs widget's interface.
 *
 * @param[in] file
 * @param[in] page
 * @param[out] ret
 *
 * @see elm_prefs_file_set()
 *
 * @since 1.8
 */
#define elm_obj_prefs_file_set(file, page, ret)          \
        ELM_OBJ_PREFS_ID(ELM_OBJ_PREFS_SUB_ID_FILE_SET), \
        EO_TYPECHECK(const char *, file),                \
        EO_TYPECHECK(const char *, page),                \
        EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_prefs_file_get
 *
 * Retrieve file and page bound to a given prefs widget.
 *
 * @param[out] file
 * @param[out] page
 * @param[out] ret
 *
 * @see elm_prefs_file_get()
 *
 * @since 1.8
 */
#define elm_obj_prefs_file_get(file, page, ret)          \
        ELM_OBJ_PREFS_ID(ELM_OBJ_PREFS_SUB_ID_FILE_GET), \
        EO_TYPECHECK(const char **, file),               \
        EO_TYPECHECK(const char **, page),               \
        EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_prefs_data_set
 *
 * Set user data for a given prefs widget
 *
 * @param[in] data
 * @param[out] ret
 *
 * @see elm_prefs_data_set()
 *
 * @since 1.8
 */
#define elm_obj_prefs_data_set(data, ret)                \
        ELM_OBJ_PREFS_ID(ELM_OBJ_PREFS_SUB_ID_DATA_SET), \
        EO_TYPECHECK(Elm_Prefs_Data *, data),            \
        EO_TYPECHECK(Eina_Bool *, ret)
//FIXME: prefs_data == eobject?

/**
 * @def elm_obj_prefs_data_get
 *
 * Retrieve user data for a given prefs widget
 *
 * @param[out] ret
 *
 * @see elm_prefs_data_get()
 *
 * @since 1.8
 */
#define elm_obj_prefs_data_get(ret)                      \
        ELM_OBJ_PREFS_ID(ELM_OBJ_PREFS_SUB_ID_DATA_GET), \
        EO_TYPECHECK(Elm_Prefs_Data **, ret)

/**
 * @def elm_obj_prefs_autosave_set
 *
 * Set whether a given prefs widget should save its values back (on
 * the user data file, if set) automatically on every UI element
 * changes.
 *
 * @param[in] autosave
 *
 * @see elm_prefs_autosave_get()
 *
 * @since 1.8
 */
#define elm_obj_prefs_autosave_set(autosave)                 \
        ELM_OBJ_PREFS_ID(ELM_OBJ_PREFS_SUB_ID_AUTOSAVE_SET), \
        EO_TYPECHECK(Eina_Bool, autosave)

/**
 * @def elm_obj_prefs_autosave_get
 *
 * Get whether a given prefs widget is saving its values back
 * automatically on changes.
 *
 * @param[out] ret
 *
 * @see elm_prefs_autosave_set()
 *
 * @since 1.8
 */
#define elm_obj_prefs_autosave_get(ret)                      \
        ELM_OBJ_PREFS_ID(ELM_OBJ_PREFS_SUB_ID_AUTOSAVE_GET), \
        EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_prefs_reset
 *
 * Reset the values of a given prefs widget to a previous state.
 *
 * @param[in] mode
 *
 * @since 1.8
 */
#define elm_obj_prefs_reset(mode)                     \
        ELM_OBJ_PREFS_ID(ELM_OBJ_PREFS_SUB_ID_RESET), \
        EO_TYPECHECK(Elm_Prefs_Reset_Mode, mode)

/**
 * @def elm_obj_prefs_item_value_set
 *
 * Set the value on a given prefs widget's item.
 *
 * @param[in] name
 * @param[in] value
 * @param[out] ret
 *
 * @see elm_prefs_item_value_get()
 *
 * @since 1.8
 */
#define elm_obj_prefs_item_value_set(name, value, ret)         \
        ELM_OBJ_PREFS_ID(ELM_OBJ_PREFS_SUB_ID_ITEM_VALUE_SET), \
        EO_TYPECHECK(const char *, name),                      \
        EO_TYPECHECK(const Eina_Value *, value),               \
        EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_prefs_item_value_get
 *
 * Retrieve the value of a given prefs widget's item.
 *
 * @param[in] name
 * @param[out] value
 * @param[out] ret
 *
 * @see elm_prefs_item_value_set()
 *
 * @since 1.8
 */
#define elm_obj_prefs_item_value_get(name, value, ret)         \
        ELM_OBJ_PREFS_ID(ELM_OBJ_PREFS_SUB_ID_ITEM_VALUE_GET), \
        EO_TYPECHECK(const char *, name),                      \
        EO_TYPECHECK(Eina_Value *, value),                     \
        EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_prefs_item_object_get
 *
 * Retrieve the Elementary widget bound to a given prefs widget's
 * item.
 *
 * @param[in] name
 * @param[out] ret
 *
 * @see elm_prefs_item_value_get()
 *
 * @since 1.8
 */
#define elm_obj_prefs_item_object_get(name, ret)                \
        ELM_OBJ_PREFS_ID(ELM_OBJ_PREFS_SUB_ID_ITEM_OBJECT_GET), \
        EO_TYPECHECK(const char *, name),                       \
        EO_TYPECHECK(const Evas_Object **, ret) //FIXME: return EObject?

/**
 * @def elm_obj_prefs_item_visible_set
 *
 * Set whether the widget bound to given prefs widget's item should be
 * visible or not.
 *
 * @param[in] name
 * @param[in] visible
 *
 * @see elm_prefs_item_visible_set()
 *
 * @since 1.8
 */
#define elm_obj_prefs_item_visible_set(name, visible)            \
        ELM_OBJ_PREFS_ID(ELM_OBJ_PREFS_SUB_ID_ITEM_VISIBLE_SET), \
        EO_TYPECHECK(const char *, name),                        \
        EO_TYPECHECK(Eina_Bool, visible)

/**
 * @def elm_obj_prefs_item_visible_get
 *
 * Retrieve whether the widget bound to a given prefs widget's item is
 * visible or not.
 *
 * @param[in] name
 * @param[out] ret
 *
 * @see elm_prefs_item_visible_get()
 *
 * @since 1.8
 */
#define elm_obj_prefs_item_visible_get(name, ret)                \
        ELM_OBJ_PREFS_ID(ELM_OBJ_PREFS_SUB_ID_ITEM_VISIBLE_GET), \
        EO_TYPECHECK(const char *,name),                         \
        EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_prefs_item_disabled_set
 *
 * Set whether the widget bound to a given prefs widget's item is
 * disabled or not.
 *
 * @param[in] name
 * @param[in] disabled
 *
 * @see elm_prefs_item_disabled_set()
 *
 * @since 1.8
 */
#define elm_obj_prefs_item_disabled_set(name, disabled)           \
        ELM_OBJ_PREFS_ID(ELM_OBJ_PREFS_SUB_ID_ITEM_DISABLED_SET), \
        EO_TYPECHECK(const char *, name),                         \
        EO_TYPECHECK(Eina_Bool, disabled)

/**
 * @def elm_obj_prefs_item_disabled_get
 *
 * Retrieve whether the widget bound to a given prefs widget's item is
 * disabled or not.
 *
 * @param[in] name
 * @param[out] ret
 *
 * @see elm_prefs_item_disabled_get()
 *
 * @since 1.8
 */
#define elm_obj_prefs_item_disabled_get(name, ret)                \
        ELM_OBJ_PREFS_ID(ELM_OBJ_PREFS_SUB_ID_ITEM_DISABLED_GET), \
        EO_TYPECHECK(const char *, name),                         \
        EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_prefs_item_editable_set
 *
 * Set whether the widget bound to a given prefs widget's item is
 * editable or not.
 *
 * @param[in] name
 * @param[in] editable
 *
 * @see elm_prefs_item_editable_set()
 *
 * @since 1.8
 */
#define elm_obj_prefs_item_editable_set(name, editable)           \
        ELM_OBJ_PREFS_ID(ELM_OBJ_PREFS_SUB_ID_ITEM_EDITABLE_SET), \
        EO_TYPECHECK(const char *, name),                         \
        EO_TYPECHECK(Eina_Bool, editable)

/**
 * @def elm_obj_prefs_item_editable_get
 *
 * Retrieve whether the widget bound to a given prefs widget's item is
 * editable or not.
 *
 * @param[in] name
 * @param[out] ret
 *
 * @see elm_prefs_item_editable_get()
 *
 * @since 1.8
 */
#define elm_obj_prefs_item_editable_get(name, ret)                \
        ELM_OBJ_PREFS_ID(ELM_OBJ_PREFS_SUB_ID_ITEM_EDITABLE_GET), \
        EO_TYPECHECK(const char *, name),                         \
        EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_prefs_item_swallow
 *
 * "Swallows" an object into a SWALLOW item of a prefs widget.
 *
 * @param[in] name
 * @param[in] child
 * @param[out] ret
 *
 * @see elm_prefs_item_swallow()
 *
 * @since 1.8
 */
#define elm_obj_prefs_item_swallow(name, child, ret)         \
        ELM_OBJ_PREFS_ID(ELM_OBJ_PREFS_SUB_ID_ITEM_SWALLOW), \
        EO_TYPECHECK(const char *, name),                    \
        EO_TYPECHECK(Evas_Object *, child),                  \
        EO_TYPECHECK(Eina_Bool *, ret)
//FIXME: child == EObject?

/**
 * @def elm_obj_prefs_item_unswallow
 *
 * Unswallow an object from a SWALLOW item of a prefs widget.
 *
 * @param[in] name
 * @param[out] ret
 *
 * @see elm_prefs_item_unswallow()
 *
 * @since 1.8
 */
#define elm_obj_prefs_item_unswallow(name, ret)                \
        ELM_OBJ_PREFS_ID(ELM_OBJ_PREFS_SUB_ID_ITEM_UNSWALLOW), \
        EO_TYPECHECK(const char *,name),                       \
        EO_TYPECHECK(Evas_Object **, ret)
//FIXME: ret == EObject?

/**
 * @addtogroup Prefs
 * @{
 *
 * @section elm-prefs-widget Prefs
 *
 * @image html img/widget/prefs/preview-00.png
 * @image latex img/widget/prefs/preview-00.eps width=\textwidth
 *
 * @image html prefs_inheritance_tree.png
 * @image latex prefs_inheritance_tree.eps
 *
 * Prefs is a special widget inside Elementary. It is mainly targeted
 * to configuration dialogs. Its viewport is blank until you associate
 * a definition file to it. That file (usually with @b .epb
 * extension), is a binary format (Eet) one, coming from a
 * human-readable textual declaration. This textual form, an
 * <b>Elementary prefs collection</b> (usually with @b .epc
 * extension), is translated to the binary form by means of the
 * @b prefs_cc compiler.
 *
 * On it one declares UI elements to populate prefs widgets, which are
 * contained in @b pages (a prefs widget is always bound to one page).
 * Those elements get properties like:
 * - default value,
 * - maximum and minumum ranges,
 * - acceptance and denial regular expressions, for strings,
 * - visibility,
 * - persistence,
 * - editability, etc.
 *
 * Once a prefs widget is created, after elm_prefs_file_set() is
 * issued on it, all of its UI elements will get default values,
 * declared on that file. To fetch an user's own, personal set of
 * those values, one gets to pair an <b>@ref elm-prefs-data</b> handle
 * to the prefs widget.
 *
 * Personal, prefs data values with keys matching the ones present on
 * the file passed on elm_prefs_file_set() to will have their values
 * applied to the respective UI elements of the widget.
 *
 * When a prefs widget dies, the values of the elements declared on
 * its @b .epb file marked as permanent <b>will be written back</b> to
 * the user prefs data handle, if it is writable. One is also able to
 * make this writing event to take place automatically after each UI
 * element modification by using elm_prefs_autosave_set().
 *
 * This widget emits the following signals:
 * - @c "page,changed": a given page has changed (event info is the
 *                      page's name)
 * - @c "page,saved": a given page has had its values saved back on
 *                    user config file (event info is the page's
 *                    name)
 * - @c "page,reset": a given page has had its values reset (event
 *                    info is the page's name)
 * - @c "page,loaded": a given page has been loaded (event info is
 *                     the page's name)
 * - @c "item,changed": a given page's item has changed (event info is
 *                      the item's full hierarchical name, in the form
 *                      @c "page:subpage:subsubpage:item")
 * - @c "action": an action item has been triggered (event info is the
 *                item's full hierarchical name, in the form @c
 *                "page:subpage:subsubpage:item")
 *
 * @li @ref epcref
 *
 * The following are examples on how to use prefs:
 * @li @ref prefs_example_01
 * @li @ref prefs_example_02
 * @li @ref prefs_example_03
 */

/**
 * Elm Prefs item types.
 *
 * @since 1.8
 */
typedef enum _Elm_Prefs_Item_Type
{
   ELM_PREFS_TYPE_UNKNOWN = 0,
   ELM_PREFS_TYPE_ACTION, /***< action type, bound to button-like UI elements */
   ELM_PREFS_TYPE_BOOL, /**< boolean type, bound to checkbox-like UI elements */
   ELM_PREFS_TYPE_INT, /**< integer type, bound to spinner-like UI elements */
   ELM_PREFS_TYPE_FLOAT, /**< floating point type, bound to spinner-like UI elements */
   ELM_PREFS_TYPE_LABEL, /**< label type, bound to read-only, label UI elements */
   ELM_PREFS_TYPE_DATE, /**< date type, bound to date selection UI elements */
   ELM_PREFS_TYPE_TEXT, /**< text type, bound to single-line, text entry UI elements */
   ELM_PREFS_TYPE_TEXTAREA, /**< text area type, bound to multi-line, text entry UI elements */
   ELM_PREFS_TYPE_PAGE, /**< page type, used to nest pages */
   ELM_PREFS_TYPE_RESET, /**< values resetting trigger type, bound to button-like UI elements */
   ELM_PREFS_TYPE_SAVE, /**< saving trigger type, bound to button-like UI elements */
   ELM_PREFS_TYPE_SEPARATOR, /**< separator type, bound to separator-like UI elements */
   ELM_PREFS_TYPE_SWALLOW /**< swallow type, bound to an empty 'spot' on the UI meant to receive and display external content */
} Elm_Prefs_Item_Type;

/**
 * Modes of resetting a prefs widget.
 *
 * @see elm_prefs_reset()
 *
 * @since 1.8
 */
typedef enum
{
   ELM_PREFS_RESET_DEFAULTS, /** reset to default values, declared in its @b .epb file */
   ELM_PREFS_RESET_LAST /** prefs will keep a small history of changes, so this  resets back to last batch of changes on the UI elements. it does @b not work on autosave mode */
} Elm_Prefs_Reset_Mode;

/**
 * @brief Add a new prefs widget
 *
 * @param parent The parent widget to hold the new one
 * @return The new object or @c NULL, on errors
 *
 * @since 1.8
 */
EAPI Evas_Object       *elm_prefs_add(Evas_Object *parent);

/**
 * Set file and page to populate a given prefs widget's interface.
 *
 * @param o A prefs widget handle
 * @param file The @b .epb (binary) file to get contents from
 * @param page The page, inside @a file, where to get item contents from
 * @return @c EINA_TRUE, on success, @c EINA_FALSE otherwise
 *
 * Elm prefs widgets start blank, with no child widgets. It's meant to
 * have its viewport populated with child elements coming from a
 * declaration file. That file (usually with @b .epb extension), is a
 * binary format (Eet) one, coming from a human-readable textual
 * declaration. This textual form (usually with @b .epc extension) is
 * translated to the binary form by means of the @b prefs_cc compiler.
 *
 * With this function, one thus populates a prefs widget with UI
 * elements.
 *
 * If @a file is @c NULL, "elm_app_data_dir_get()/preferences.epb"
 * will be used, by default. If @a file is a @b relative path, the
 * prefix "elm_app_data_dir_get()/" will be implicitly used with it.
 * If @a page is @c NULL, it is considered "main", as default.
 *
 * @warning If your binary is not properly installed and
 * elm_app_data_dir_get() can't be figured out, a fallback value of
 * "." will be tryed, instead.
 *
 * @see elm_prefs_file_get()
 *
 * @since 1.8
 */
EAPI Eina_Bool          elm_prefs_file_set(Evas_Object *o,
                                           const char *file,
                                           const char *page);

/**
 * Retrieve file and page bound to a given prefs widget.
 *
 * @param o A prefs widget handle
 * @param file Where to store the file @a o is bound to
 * @param page Where to store the page @a o is bound to
 * @return @c EINA_TRUE, on success, @c EINA_FALSE otherwise
 *
 * @note Use @c NULL pointers on the components you're not
 * interested in: they'll be ignored by the function.
 *
 * @see elm_prefs_file_set() for more information
 *
 * @since 1.8
 */
EAPI Eina_Bool          elm_prefs_file_get(const Evas_Object *o,
                                           const char **file,
                                           const char **page);
/**
 * Set user data for a given prefs widget
 *
 * @param obj A prefs widget handle
 * @param prefs_data A valid prefs_data handle
 * @return @c EINA_TRUE, on success, @c EINA_FALSE otherwise
 *
 * Once a prefs widget is created, after elm_prefs_file_set() is
 * issued on it, all of its UI elements will get default values, when
 * declared on that file. To fetch an user's own, personal set of
 * those values, one gets to pair a <b>prefs data</b> handle to the
 * prefs widget. This is what this call is intended for.
 *
 * Prefs data values from @a prefs_data with keys matching the ones
 * present on the file passed on elm_prefs_file_set() to @a obj will
 * have their values applied to the respective UI elements of the
 * widget.
 *
 * When @a obj dies, the values of the elements declared on its @b
 * .epb file (the one set on elm_prefs_file_set()) marked as permanent
 * <b>will be written back</b> to @a prefs_data, if it is writable.
 * One is also able to make this writing event to take place
 * automatically after each UI element modification by using
 * elm_prefs_autosave_set().
 *
 * @note @a obj will keep a reference of its own for @a prefs_data,
 * but you should still unreference it by yourself, after the widget
 * is gone.
 *
 * @see elm_prefs_data_set()
 *
 * @since 1.8
 */
EAPI Eina_Bool          elm_prefs_data_set(Evas_Object *obj,
                                           Elm_Prefs_Data *data);

/**
 * Retrieve user data for a given prefs widget
 *
 * @param obj A prefs widget handle
 * @param prefs_data A valid prefs_data handle
 * @return @c EINA_TRUE, on success, @c EINA_FALSE otherwise
 *
 * @see elm_prefs_data_set() for more details
 *
 * @since 1.8
 */
EAPI Elm_Prefs_Data    *elm_prefs_data_get(const Evas_Object *o);

/**
 * Set whether a given prefs widget should save its values back (on
 * the user data file, if set) automatically on every UI element
 * changes.
 *
 * @param prefs A valid prefs widget handle
 * @param autosave @c EINA_TRUE to save automatically, @c EINA_FALSE
 *                 otherwise.
 *
 * If @a autosave is @c EINA_TRUE, every call to
 * elm_prefs_item_value_set(), every
 * Elm_Prefs_Data_Event_Type::ELM_PREFS_DATA_EVENT_ITEM_CHANGED event
 * coming for its prefs data and every UI element direct value
 * changing will implicitly make the prefs values to be flushed back
 * to it prefs data. If a prefs data handle with no writing
 * permissions or no prefs data is set on @a prefs, naturally nothing
 * will happen.
 *
 * @see elm_prefs_autosave_get()
 *
 * @since 1.8
 */
EAPI void               elm_prefs_autosave_set(Evas_Object *prefs,
                                               Eina_Bool autosave);

/**
 * Get whether a given prefs widget is saving its values back
 * automatically on changes.
 *
 * @param prefs A valid prefs widget handle
 * @return @c EINA_TRUE if @a prefs is saving automatically,
 *         @c EINA_FALSE otherwise.
 *
 * @see elm_prefs_autosave_set(), for more details
 *
 * @since 1.8
 */
EAPI Eina_Bool          elm_prefs_autosave_get(const Evas_Object *prefs);

/**
 * Reset the values of a given prefs widget to a previous state.
 *
 * @param prefs A valid prefs widget handle
 * @param mode The reset mode to apply on @a prefs
 *
 * As can be seen on #Elm_Prefs_Reset_Mode, there are two possible
 * actions to be taken by this call -- either to reset @a prefs'
 * values to the defaults (declared on the @c .epb file it is bound
 * to) or to reset to the state they were before the last modification
 * it got.
 *
 * @since 1.8
 */
EAPI void               elm_prefs_reset(Evas_Object *prefs,
                                        Elm_Prefs_Reset_Mode mode);

/**
 * Set the value on a given prefs widget's item.
 *
 * @param prefs A valid prefs widget handle
 * @param name The name of the item (as declared in the prefs
 *             collection)
 * @param value The value to set on the item. It should be typed as
 *              the item expects, preferably, or a conversion will
 *              take place
 * @return @c EINA_TRUE, on success, @c EINA_FALSE otherwise
 *
 * This will change the value of item named @a name programatically.
 *
 * @see elm_prefs_item_value_get()
 *
 * @since 1.8
 */
EAPI Eina_Bool          elm_prefs_item_value_set(Evas_Object *prefs,
                                                 const char *name,
                                                 const Eina_Value *value);

/**
 * Retrieve the value of a given prefs widget's item.
 *
 * @param prefs A valid prefs widget handle
 * @param name The name of the item (as declared in the prefs
 *             collection) to get value from
 * @param value Where to store the value of the item. It will be
 *              overwritten and setup with the type the item
 *              is bound to
 * @return @c EINA_TRUE, on success, @c EINA_FALSE otherwise
 *
 * This will retrieve the value of item named @a name.
 *
 * @see elm_prefs_item_value_set()
 *
 * @since 1.8
 */
EAPI Eina_Bool          elm_prefs_item_value_get(const Evas_Object *prefs,
                                                 const char *name,
                                                 Eina_Value *value);
/**
 * Retrieve the Elementary widget bound to a given prefs widget's
 * item.
 *
 * @param prefs A valid prefs widget handle
 * @param name The name of the item (as declared in the prefs
 *             collection) to get object from
 * @return A valid widget handle, on success, or @c NULL, otherwise
 *
 * This will retrieve a handle to the real widget implementing a given
 * item of @a prefs, <b>for read-only</b> actions.
 *
 * @warning You should @b never modify the state of the returned
 * widget, because it's meant to be managed by @a prefs, solely.
 *
 * @see elm_prefs_item_value_set()
 *
 * @since 1.8
 */
EAPI const Evas_Object *elm_prefs_item_object_get(Evas_Object *prefs,
                                                  const char *name);

/**
 * Set whether the widget bound to given prefs widget's item should be
 * visible or not.
 *
 * @param prefs A valid prefs widget handle
 * @param name The name of the item (as declared in the prefs
 *             collection) to change visibility of
 * @param visible @c EINA_TRUE, to make it visible, @c EINA_FALSE
 *                otherwise
 *
 * Each prefs item may have a default visibility state, declared on
 * the @c .epb @a prefs it was loaded with. By this call one may alter
 * that state, programatically.
 *
 * @see elm_prefs_item_visible_get()
 *
 * @since 1.8
 */
EAPI void               elm_prefs_item_visible_set(Evas_Object *prefs,
                                                   const char *name,
                                                   Eina_Bool visible);

/**
 * Retrieve whether the widget bound to a given prefs widget's item is
 * visible or not.
 *
 * @param prefs A valid prefs widget handle
 * @param name The name of the item (as declared in the prefs
 *             collection) to get visibility state from
 * @return @c EINA_TRUE, if it is visible, @c EINA_FALSE
 *         otherwise
 *
 * @see elm_prefs_item_visible_set() for more details
 *
 * @since 1.8
 */
EAPI Eina_Bool          elm_prefs_item_visible_get(const Evas_Object *prefs,
                                                   const char *name);

/**
 * Set whether the widget bound to a given prefs widget's item is
 * disabled or not.
 *
 * @param prefs A valid prefs widget handle
 * @param name The name of the item (as declared in the prefs
 *             collection) to act on
 * @param disabled @c EINA_TRUE, to make it disabled, @c EINA_FALSE
 *                 otherwise
 *
 * @see elm_prefs_item_disabled_get()
 *
 * @since 1.8
 */
EAPI void               elm_prefs_item_disabled_set(Evas_Object *prefs,
                                                    const char *name,
                                                    Eina_Bool disabled);

/**
 * Retrieve whether the widget bound to a given prefs widget's item is
 * disabled or not.
 *
 * @param prefs A valid prefs widget handle
 * @param name The name of the item (as declared in the prefs
 *             collection) to get disabled state from
 * @return @c EINA_TRUE, if it is disabled, @c EINA_FALSE
 *         otherwise
 *
 * @see elm_prefs_item_disabled_set()
 *
 * @since 1.8
 */
EAPI Eina_Bool          elm_prefs_item_disabled_get(const Evas_Object *prefs,
                                                    const char *name);

/**
 * Set whether the widget bound to a given prefs widget's item is
 * editable or not.
 *
 * @param prefs A valid prefs widget handle
 * @param name The name of the item (as declared in the prefs
 *             collection) to act on
 * @param editable @c EINA_TRUE, to make it editable, @c EINA_FALSE
 *                 otherwise
 *
 * @note Only @c TEXT or @c TEXTAREA items' default widgets implement
 * the 'editable' property. Custom registered widgets may as well
 * implement them.
 *
 * @see elm_prefs_item_editable_get()
 *
 * @since 1.8
 */
EAPI void               elm_prefs_item_editable_set(Evas_Object *prefs,
                                                    const char *name,
                                                    Eina_Bool editable);

/**
 * Retrieve whether the widget bound to a given prefs widget's item is
 * editable or not.
 *
 * @param prefs A valid prefs widget handle
 * @param name The name of the item (as declared in the prefs
 *             collection) to get editable state from
 * @return @c EINA_TRUE, if it is editable, @c EINA_FALSE
 *         otherwise
 *
 * @see elm_prefs_item_editable_set() for more details
 *
 * @since 1.8
 */
EAPI Eina_Bool          elm_prefs_item_editable_get(const Evas_Object *prefs,
                                                    const char *name);

/**
 * "Swallows" an object into a SWALLOW item of a prefs widget.
 *
 * @param obj A valid prefs widget handle
 * @param name the name of the SWALLOW item (as declared in the prefs
 *             collection)
 * @param child The object to occupy the item
 * @return @c EINA_TRUE, on success, @c EINA_FALSE otherwise
 *
 * @see elm_prefs_item_swallow() for more details
 *
 * @since 1.8
 */
EAPI Eina_Bool          elm_prefs_item_swallow(Evas_Object *obj,
                                               const char *name,
                                               Evas_Object *child);

/**
 * Unswallow an object from a SWALLOW item of a prefs widget.
 *
 * @param obj A valid prefs widget handle
 * @param name the name of the SWALLOW item (as declared in the prefs
 *             collection)
 * @return The unswallowed object, or NULL on errors
 *
 * @see elm_prefs_item_unswallow() for more details
 *
 * @since 1.8
 */
EAPI Evas_Object       *elm_prefs_item_unswallow(Evas_Object *obj,
                                                 const char *name);

/* API for prefs interface modules, from now on */

typedef struct _Elm_Prefs_Item_Spec_Void
{
} Elm_Prefs_Item_Spec_Void;

typedef struct _Elm_Prefs_Item_Spec_Bool
{
   Eina_Bool def;
} Elm_Prefs_Item_Spec_Bool;

typedef struct _Elm_Prefs_Item_Spec_Int
{
   int def, min, max;
} Elm_Prefs_Item_Spec_Int;

typedef struct _Elm_Prefs_Item_Spec_Float
{
   float def, min, max;
} Elm_Prefs_Item_Spec_Float;

typedef struct _Elm_Prefs_Item_Spec_String
{
   const char *def;
   const char *placeholder;
   const char *accept;
   const char *deny;

   struct
   {
      int min, max;
   } length;
} Elm_Prefs_Item_Spec_String;

typedef struct _Elm_Prefs_Item_Spec_Date
{
   struct
   {
      unsigned char  d; /* day*/
      unsigned char  m; /* month*/
      unsigned short y; /* year */
   } def;

   struct
   {
      unsigned char  d; /* day*/
      unsigned char  m; /* month*/
      unsigned short y; /* year */
   } min;

   struct
   {
      unsigned char  d; /* day*/
      unsigned char  m; /* month*/
      unsigned short y; /* year */
   } max;
} Elm_Prefs_Item_Spec_Date;

typedef struct _Elm_Prefs_Item_Spec_Page
{
   const char *source;
} Elm_Prefs_Item_Spec_Page;

typedef union _Elm_Prefs_Item_Spec
{
   Elm_Prefs_Item_Spec_Void   v;
   Elm_Prefs_Item_Spec_Bool   b;
   Elm_Prefs_Item_Spec_Int    i;
   Elm_Prefs_Item_Spec_Float  f;
   Elm_Prefs_Item_Spec_String s;
   Elm_Prefs_Item_Spec_Date   d;
   Elm_Prefs_Item_Spec_Page   p;
} Elm_Prefs_Item_Spec;

typedef struct _Elm_Prefs_Item_Iface Elm_Prefs_Item_Iface;

/**
 * Convenience struct used to mass-register widgets implementing
 * prefs @b items interfaces.
 *
 * To be used with elm_prefs_item_iface_register() and
 * elm_prefs_item_iface_unregister().
 */
struct _Elm_Prefs_Item_Iface_Info
{
   const char                 *widget_name; /**< The name of the widget implementing the interface, to be exposed on the prefs collections language. */
   const Elm_Prefs_Item_Iface *info; /**< The type interface's implementation. */
};
typedef struct _Elm_Prefs_Item_Iface_Info Elm_Prefs_Item_Iface_Info;

typedef void                            (*Elm_Prefs_Item_Changed_Cb)(Evas_Object *it_obj);   /**< Elementary prefs' item widget changed function signature */

/**
 * @struct _Elm_Prefs_Item_Iface
 *
 * @brief Interface between items of the @ref elm-prefs-widget widget
 *        and the real widgets implementing them.
 *
 * This structure defines the interface between the
 * @ref elm-prefs-widget widget's items (all item types in Elementary prefs
 * collections but the @c PAGE one) and Elementary widgets
 * implementing them. @c add() is the only mandatory function an
 * item widget has to implement.
 *
 * @note For items of type @c PAGE, refer to #Elm_Prefs_Page_Iface.
 *
 * @since 1.8
 */
struct _Elm_Prefs_Item_Iface
{
#define ELM_PREFS_ITEM_IFACE_ABI_VERSION (1)
   unsigned int               abi_version; /**< always use:
                                            *  - #ELM_PREFS_ITEM_IFACE_ABI_VERSION to declare.
                                            *  - elm_prefs_widget_iface_abi_version_get() to check.
                                            */

   const Elm_Prefs_Item_Type *types;  /**< types of prefs items supported by the widget, #ELM_PREFS_TYPE_UNKNOWN terminated */

   Evas_Object              * (*add)(const Elm_Prefs_Item_Iface * iface,
                                     Evas_Object * prefs,
                                     const Elm_Prefs_Item_Type type,
                                     const Elm_Prefs_Item_Spec spec,
                                     Elm_Prefs_Item_Changed_Cb it_changed_cb); /**< Function to instantiate the item widget. It must return the widget handle, which should be the @c obj argument on the functions which follow. That object argument, by the way, should always have their respective #Elm_Prefs_Item_Node handle accessible via a @c "prefs_item" Evas object data value. The return value of the following functions should be @c EINA_TRUE, on success or @c EINA_FALSE, otherwise. */

   Eina_Bool                  (*value_set)(Evas_Object *obj,
                                           Eina_Value *value); /**< Function to set the value on the item widget. Note that for items of type #ELM_PREFS_TYPE_SEPARATOR, this function has a special meaning of making the separator widget a @b horizontal one */

   Eina_Bool                  (*value_get)(Evas_Object *obj,
                                           Eina_Value *value); /**< Function to set the value on the item widget. Note that for items of type #ELM_PREFS_TYPE_SEPARATOR, this function has a special meaning of making the separator widget a @b vertical one */

   Eina_Bool                  (*value_validate)(Evas_Object *obj); /** < Function to validate the value from the item widget before saving it. The return value of the following function should be @c EINA_TRUE, if the value conforms with the expected or @c EINA_FALSE, otherwise. */

   Eina_Bool                  (*label_set)(Evas_Object *obj,
                                           const char *label); /**< function to set a label on the item widget */

   Eina_Bool                  (*icon_set)(Evas_Object *obj,
                                          const char *icon); /**< function to set an icon on the item widget */

   Eina_Bool                  (*editable_set)(Evas_Object *obj,
                                              Eina_Bool val); /**< function to set an item widget as editable or not */

   Eina_Bool                  (*editable_get)(Evas_Object *obj); /**< function to retrieve whether an item widget is editable or not */

   Eina_Bool                  (*expand_want)(Evas_Object *obj); /**< function to get wether the item implementation needs to be expanded in the page's longitudinal axis or not */
};

/**
 * Mass-register widgets implementing prefs @b items interfaces.
 *
 * @param array An array of #Elm_Prefs_Iface_Info structs, @c NULL
 * terminated.
 *
 * This will register all item interfaces declared on @a array in
 * Elementary, so that the prefs widget will recognize them on @c .epc
 * files @c 'widget:' (item) declarations.
 *
 * @see elm_prefs_item_iface_unregister()
 *
 * @since 1.8
 */
EAPI void      elm_prefs_item_iface_register(const Elm_Prefs_Item_Iface_Info *array);

/**
 * Mass-unregister widgets implementing prefs @b items interfaces.
 *
 * @param array An array of #Elm_Prefs_Iface_Info structs, @c NULL
 * terminated.
 *
 * This will unregister all item interfaces declared on @a array in
 * Elementary, given they had been previously registered.
 *
 * @see elm_prefs_item_iface_register() for more details
 *
 * @since 1.8
 */
EAPI void      elm_prefs_item_iface_unregister(const Elm_Prefs_Item_Iface_Info *array);

EAPI Eina_Bool elm_prefs_item_widget_common_add(Evas_Object *prefs,
                                                Evas_Object *obj);

typedef struct _Elm_Prefs_Page_Iface Elm_Prefs_Page_Iface;

/**
 * Convenience struct used to mass-register widgets implementing
 * prefs @b pages interfaces.
 *
 * To be used with elm_prefs_page_iface_register() and
 * elm_prefs_page_iface_unregister().
 */
struct _Elm_Prefs_Page_Iface_Info
{
   const char                 *widget_name; /**< The name of the widget implementing the interface, to be exposed on the prefs collections language. */
   const Elm_Prefs_Page_Iface *info; /**< The interface's implementation. */
};
typedef struct _Elm_Prefs_Page_Iface_Info Elm_Prefs_Page_Iface_Info;

/**
 * @struct _Elm_Prefs_Page_Iface
 *
 * @brief Interface between pages of the @ref elm-prefs-widget widget
 *        and the real widgets implementing them.
 *
 * This structure defines the interface between the
 * @ref elm-prefs-widget widget's pages and Elementary widgets
 * implementing them.
 *
 * It is @b mandatory that the following functions be implemented, at
 * least, for a page widget:
 *
 * - #Elm_Prefs_Page_Iface::add
 * - #Elm_Prefs_Page_Iface::item_pack
 * - #Elm_Prefs_Page_Iface::item_unpack
 * - #Elm_Prefs_Page_Iface::item_pack_before
 * - #Elm_Prefs_Page_Iface::item_pack_after
 *
 * @note For regular, non-page prefs items, refer to #Elm_Prefs_Item_Iface.
 *
 * @since 1.8
 */
struct _Elm_Prefs_Page_Iface
{
#define ELM_PREFS_PAGE_IFACE_ABI_VERSION (1)
   unsigned int  abi_version; /**< always use:
                               *  - #ELM_PREFS_PAGE_IFACE_ABI_VERSION to declare.
                               *  - elm_prefs_widget_iface_abi_version_get() to check.
                               */

   Evas_Object * (*add)(const Elm_Prefs_Page_Iface * iface,
                        Evas_Object * prefs); /**< Function to instantiate the page widget. It must return the widget handle, which should be the @c obj argument on the functions which follow. That object argument, by the way, should always have their respective #Elm_Prefs_Page_Node handle accessible via a @c "prefs_page" Evas object data value. The return value of the following functions should be @c EINA_TRUE, on success or @c EINA_FALSE, otherwise. */

   Eina_Bool     (*title_set)(Evas_Object *obj,
                              const char *title); /**< function to set a title on the page widget */

   Eina_Bool     (*sub_title_set)(Evas_Object *obj,
                                  const char *sub_title); /**< function to set a sub-title on the page widget */

   Eina_Bool     (*icon_set)(Evas_Object *obj,
                             const char *icon); /**< function to set an icon on the page widget */

   Eina_Bool     (*item_pack)(Evas_Object *obj,
                              Evas_Object *it,
                              const Elm_Prefs_Item_Type type,
                              const Elm_Prefs_Item_Iface *iface); /**< function to pack an item (widget) on the page widget */


   Eina_Bool     (*item_unpack)(Evas_Object *obj,
                                Evas_Object *it); /**< function to unpack an item (widget) on the page widget */

   Eina_Bool     (*item_pack_before)(Evas_Object *obj,
                                     Evas_Object *it,
                                     Evas_Object *it_before,
                                     const Elm_Prefs_Item_Type type,
                                     const Elm_Prefs_Item_Iface *iface); /**< function to pack an item (widget) on the page widget, before a pre-existing, referential, packed one */

   Eina_Bool     (*item_pack_after)(Evas_Object *obj,
                                    Evas_Object *it,
                                    Evas_Object *it_after, /**< function to pack an item (widget) on the page widget, after a pre-existing, referential, packed one */
                                    const Elm_Prefs_Item_Type type,
                                    const Elm_Prefs_Item_Iface *iface);
};

/**
 * Mass-register widgets implementing prefs @b pages interfaces.
 *
 * @param array An array of #Elm_Prefs_Iface_Info structs, @c NULL
 * terminated.
 *
 * This will register all page interfaces declared on @a array in
 * Elementary, so that the prefs widget will recognize them on @c .epc
 * files @c 'widget:' (page) declarations.
 *
 * @see elm_prefs_page_iface_unregister()
 *
 * @since 1.8
 */
EAPI void      elm_prefs_page_iface_register(const Elm_Prefs_Page_Iface_Info *array);

/**
 * Mass-unregister widgets implementing prefs @b pages interfaces.
 *
 * @param array An array of #Elm_Prefs_Iface_Info structs, @c NULL
 * terminated.
 *
 * This will unregister all page interfaces declared on @a array in
 * Elementary, given they had been previously registered.
 *
 * @see elm_prefs_page_iface_register() for more details
 *
 * @since 1.8
 */
EAPI void      elm_prefs_page_iface_unregister(const Elm_Prefs_Page_Iface_Info *array);

EAPI Eina_Bool elm_prefs_page_widget_common_add(Evas_Object *prefs,
                                                Evas_Object *obj);

/**
 * @}
 */
#endif
