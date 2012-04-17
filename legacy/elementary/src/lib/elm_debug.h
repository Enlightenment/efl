/**
 * @defgroup Debug Debug
 * @ingroup Elementary
 * Don't use it unless you are sure.
 *
 * @{
 */

/**
 * Print Tree object hierarchy in stdout
 *
 * @param top The root object
 * @ingroup Debug
 */
EAPI void elm_object_tree_dump(const Evas_Object *top);

/**
 * Print Elm Objects tree hierarchy in file as dot(graphviz) syntax.
 *
 * @param top The root object
 * @param file The path of output file
 * @ingroup Debug
 */
EAPI void elm_object_tree_dot_dump(const Evas_Object *top, const char *file);

/**
 * @}
 */
