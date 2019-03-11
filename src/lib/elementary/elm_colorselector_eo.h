#ifndef _ELM_COLORSELECTOR_EO_H_
#define _ELM_COLORSELECTOR_EO_H_

#ifndef _ELM_COLORSELECTOR_EO_CLASS_TYPE
#define _ELM_COLORSELECTOR_EO_CLASS_TYPE

typedef Eo Elm_Colorselector;

#endif

#ifndef _ELM_COLORSELECTOR_EO_TYPES
#define _ELM_COLORSELECTOR_EO_TYPES

/**
 * @brief Different modes supported by Colorselector
 *
 * See also @ref elm_obj_colorselector_mode_set,
 * @ref elm_obj_colorselector_mode_get.
 *
 * @ingroup Elm_Colorselector
 */
typedef enum
{
  ELM_COLORSELECTOR_PALETTE = 0, /**< Only color palette is displayed. */
  ELM_COLORSELECTOR_COMPONENTS, /**< Only color selector is displayed. */
  ELM_COLORSELECTOR_BOTH, /**< Both Palette and selector is displayed, default.
                           */
  ELM_COLORSELECTOR_PICKER, /**< Only color picker is displayed. */
  ELM_COLORSELECTOR_ALL /**< All possible color selector is displayed. */
} Elm_Colorselector_Mode;


#endif
/** Elementary colorselector class
 *
 * @ingroup Elm_Colorselector
 */
#define ELM_COLORSELECTOR_CLASS elm_colorselector_class_get()

EWAPI const Efl_Class *elm_colorselector_class_get(void);

/**
 * @brief Set color to colorselector.
 *
 * @param[in] obj The object.
 * @param[in] r Red value of color
 * @param[in] g Green value of color
 * @param[in] b Blue value of color
 * @param[in] a Alpha value of color
 *
 * @ingroup Elm_Colorselector
 */
EOAPI void elm_obj_colorselector_picked_color_set(Eo *obj, int r, int g, int b, int a);

/**
 * @brief Get current color from colorselector.
 *
 * @param[in] obj The object.
 * @param[out] r Red value of color
 * @param[out] g Green value of color
 * @param[out] b Blue value of color
 * @param[out] a Alpha value of color
 *
 * @ingroup Elm_Colorselector
 */
EOAPI void elm_obj_colorselector_picked_color_get(const Eo *obj, int *r, int *g, int *b, int *a);

/**
 * @brief Set current palette's name
 *
 * When colorpalette name is set, colors will be loaded from and saved to
 * config using the set name. If no name is set then colors will be loaded from
 * or saved to "default" config.
 *
 * @param[in] obj The object.
 * @param[in] palette_name Name of palette
 *
 * @ingroup Elm_Colorselector
 */
EOAPI void elm_obj_colorselector_palette_name_set(Eo *obj, const char *palette_name);

/**
 * @brief Get current palette's name
 *
 * Returns the currently set palette name using which colors will be
 * saved/loaded in to config.
 *
 * @param[in] obj The object.
 *
 * @return Name of palette
 *
 * @ingroup Elm_Colorselector
 */
EOAPI const char *elm_obj_colorselector_palette_name_get(const Eo *obj);

/**
 * @brief Set Colorselector's mode.
 *
 * Colorselector supports three modes palette only, selector only and both.
 *
 * @param[in] obj The object.
 * @param[in] mode Elm_Colorselector_Mode
 *
 * @ingroup Elm_Colorselector
 */
EOAPI void elm_obj_colorselector_mode_set(Eo *obj, Elm_Colorselector_Mode mode);

/**
 * @brief Get Colorselector's mode.
 *
 * @param[in] obj The object.
 *
 * @return Elm_Colorselector_Mode
 *
 * @ingroup Elm_Colorselector
 */
EOAPI Elm_Colorselector_Mode elm_obj_colorselector_mode_get(const Eo *obj);

/**
 * @brief Get list of palette items.
 *
 * Note That palette item list is internally managed by colorselector widget
 * and it should not be freed/modified by application.
 *
 * @param[in] obj The object.
 *
 * @return The list of color palette items.
 *
 * @since 1.9
 *
 * @ingroup Elm_Colorselector
 */
EOAPI const Eina_List *elm_obj_colorselector_palette_items_get(const Eo *obj);

/**
 * @brief Get the selected item in colorselector palette.
 *
 * @param[in] obj The object.
 *
 * @return The selected item, or @c null if none selected.
 *
 * @since 1.9
 *
 * @ingroup Elm_Colorselector
 */
EOAPI Elm_Widget_Item *elm_obj_colorselector_palette_selected_item_get(const Eo *obj);

/**
 * @brief Add a new color item to palette.
 *
 * @param[in] obj The object.
 * @param[in] r Red value of color
 * @param[in] g Green value of color
 * @param[in] b Blue value of color
 * @param[in] a Alpha value of color
 *
 * @return A new color palette Item.
 *
 * @ingroup Elm_Colorselector
 */
EOAPI Elm_Widget_Item *elm_obj_colorselector_palette_color_add(Eo *obj, int r, int g, int b, int a);

/** Clear the palette items.
 *
 * @ingroup Elm_Colorselector
 */
EOAPI void elm_obj_colorselector_palette_clear(Eo *obj);

EWAPI extern const Efl_Event_Description _ELM_COLORSELECTOR_EVENT_COLOR_ITEM_SELECTED;

/** Called when color item was selected
 * @return Efl_Object *
 *
 * @ingroup Elm_Colorselector
 */
#define ELM_COLORSELECTOR_EVENT_COLOR_ITEM_SELECTED (&(_ELM_COLORSELECTOR_EVENT_COLOR_ITEM_SELECTED))

EWAPI extern const Efl_Event_Description _ELM_COLORSELECTOR_EVENT_COLOR_ITEM_LONGPRESSED;

/** Called when color item got a long press
 * @return Efl_Object *
 *
 * @ingroup Elm_Colorselector
 */
#define ELM_COLORSELECTOR_EVENT_COLOR_ITEM_LONGPRESSED (&(_ELM_COLORSELECTOR_EVENT_COLOR_ITEM_LONGPRESSED))

EWAPI extern const Efl_Event_Description _ELM_COLORSELECTOR_EVENT_CHANGED;

/** Called when colorselector changed
 *
 * @ingroup Elm_Colorselector
 */
#define ELM_COLORSELECTOR_EVENT_CHANGED (&(_ELM_COLORSELECTOR_EVENT_CHANGED))

EWAPI extern const Efl_Event_Description _ELM_COLORSELECTOR_EVENT_CHANGED_USER;

/** Called when the object changed due to user interaction
 *
 * @ingroup Elm_Colorselector
 */
#define ELM_COLORSELECTOR_EVENT_CHANGED_USER (&(_ELM_COLORSELECTOR_EVENT_CHANGED_USER))

#endif
