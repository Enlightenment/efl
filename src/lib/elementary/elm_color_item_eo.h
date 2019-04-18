#ifndef _ELM_COLOR_ITEM_EO_H_
#define _ELM_COLOR_ITEM_EO_H_

#ifndef _ELM_COLOR_ITEM_EO_CLASS_TYPE
#define _ELM_COLOR_ITEM_EO_CLASS_TYPE

typedef Eo Elm_Color_Item;

#endif

#ifndef _ELM_COLOR_ITEM_EO_TYPES
#define _ELM_COLOR_ITEM_EO_TYPES


#endif
/** Elementary color item class
 *
 * @ingroup Elm_Color_Item
 */
#define ELM_COLOR_ITEM_CLASS elm_color_item_class_get()

EWAPI const Efl_Class *elm_color_item_class_get(void);

/**
 * @brief Set Palette item's color.
 *
 * @param[in] obj The object.
 * @param[in] r Red-value of color
 * @param[in] g Green-value of color
 * @param[in] b Blue-value of color
 * @param[in] a Alpha-value of color
 *
 * @ingroup Elm_Color_Item
 */
EOAPI void elm_obj_color_item_color_set(Eo *obj, int r, int g, int b, int a);

/**
 * @brief Get Palette item's color.
 *
 * @param[in] obj The object.
 * @param[out] r Red-value of color
 * @param[out] g Green-value of color
 * @param[out] b Blue-value of color
 * @param[out] a Alpha-value of color
 *
 * @ingroup Elm_Color_Item
 */
EOAPI void elm_obj_color_item_color_get(const Eo *obj, int *r, int *g, int *b, int *a);

/**
 * @brief Set the selected state of color palette item
 *
 * @param[in] obj The object.
 * @param[in] selected @c true if selected @c false otherwise
 *
 * @since 1.9
 *
 * @ingroup Elm_Color_Item
 */
EOAPI void elm_obj_color_item_selected_set(Eo *obj, Eina_Bool selected);

/**
 * @brief Get the selected state of color palette item
 *
 * @param[in] obj The object.
 *
 * @return @c true if selected @c false otherwise
 *
 * @since 1.9
 *
 * @ingroup Elm_Color_Item
 */
EOAPI Eina_Bool elm_obj_color_item_selected_get(const Eo *obj);

#endif
