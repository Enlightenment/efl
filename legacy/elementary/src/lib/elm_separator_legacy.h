/**
 * @brief Add a separator object to @p parent
 *
 * @param parent The parent object
 *
 * @return The separator object, or NULL upon failure
 *
 * @ingroup Separator
 */
EAPI Evas_Object *elm_separator_add(Evas_Object *parent);

/**
 * @brief Set the horizontal mode of a separator object
 *
 * @param obj The separator object
 * @param horizontal If true, the separator is horizontal
 *
 * @ingroup Separator
 */
EAPI void      elm_separator_horizontal_set(Evas_Object *obj, Eina_Bool horizontal);

/**
 * @brief Get the horizontal mode of a separator object
 *
 * @param obj The separator object
 * @return If true, the separator is horizontal
 *
 * @see elm_separator_horizontal_set()
 *
 * @ingroup Separator
 */
EAPI Eina_Bool elm_separator_horizontal_get(const Evas_Object *obj);
