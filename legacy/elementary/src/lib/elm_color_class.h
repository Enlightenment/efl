/**
 * @defgroup Elm_Color_Class_Group Color Class Editor
 * @ingroup Elm_Color_Class_Group
 * @brief This group provides a UI for editing color classes in applications.
 *
 * @{
 */

/**
 * @typedef Elm_Color_Class_Name_Cb
 * @brief A callback used to translate color class descriptions
 * @since 1.14
 */
typedef char *(*Elm_Color_Class_Name_Cb)(char *);
/**
 * @brief Create a new color class editor
 * @param obj The parent object
 * @param cb The translation callback to use
 *
 * A color class editor is a visual representation of the color schemes in an application.
 * Values changed in the editor are stored in Elementary's config and will remain until they
 * are reset or the config is cleared.
 *
 * Passing a @c NULL param for @p cb will cause the editor to work in remote mode, using dbus
 * signals to message between the target application.
 * @since 1.14
 */
EAPI Evas_Object *elm_color_class_editor_add(Evas_Object *obj, Elm_Color_Class_Name_Cb cb);

/** }@ */
