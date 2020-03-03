#ifndef ELM_COLOR_CLASS_H
# define ELM_COLOR_CLASS_H

/**
 * @defgroup Elm_Color_Class_Group Color Class Editor
 * @brief This group provides a UI for editing color classes in applications.
 *
 * @{
 */

//#define ELM_COLOR_CLASS_METHOD_BASE "org.elementary.colorclass"

/**
 * @typedef Elm_Color_Class_Name_Cb
 * @brief A callback used to translate color class descriptions
 * @since 1.14
 */
typedef char *(*Elm_Color_Class_Name_Cb)(char *);

/**
 * @typedef Elm_Color_Class_List_Cb
 * @brief A callback used to provide a list of allocated Edje_Color_Class structs used by an application
 * @since 1.14
 *
 * The list and its members will be freed internally.
 */
typedef Eina_List *(*Elm_Color_Class_List_Cb)(void);

/**
 * @brief Create a new color class editor
 * @param obj The parent object
 *
 * A color class editor is a visual representation of the color schemes in an application.
 * Values changed in the editor are stored in Elementary's config and will remain until they
 * are reset or the config is cleared. By default, the editor will load only the currently active
 * color classes in an application.
 *
 * @since 1.14
 */
EAPI Evas_Object *elm_color_class_editor_add(Evas_Object *obj);

/**
 * @brief Set a callback to provide translations for color class descriptions
 * @param cb The callback to use
 *
 * This callback will be called globally by the application to translate any available color class
 * description strings from the theme's color classes.
 *
 * @since 1.14
 **/
EAPI void elm_color_class_translate_cb_set(Elm_Color_Class_Name_Cb cb);

/**
 * @brief Set a callback to provide a list of supplementary color classes
 * @param cb The callback to use
 *
 * This callback will be called globally by the application to provide extra color classes
 * that an application may use but which may not be currently loaded.
 *
 * @see Elm_Color_Class_List_Cb
 *
 * @since 1.14
 **/
EAPI void elm_color_class_list_cb_set(Elm_Color_Class_List_Cb cb);

/**
 * @brief Create an allocated list of allocated Edje_Color_Class structs from an open edje file
 * @param f The file to list color classes from
 * @return The list of color classes present in the file
 *
 * This is a helper function to create a list for use with #Elm_Color_Class_List_Cb callbacks.
 *
 * @see Elm_Color_Class_List_Cb
 *
 * @since 1.14
 **/
EAPI Eina_List *elm_color_class_util_edje_file_list(Eina_File *f);
/** @} */

#endif
