/**
 * @addtogroup Actionslider Actionslider
 * @ingroup Elementary
 *
 * @image html actionslider_inheritance_tree.png
 * @image latex actionslider_inheritance_tree.eps
 *
 * @image html img/widget/actionslider/preview-00.png
 * @image latex img/widget/actionslider/preview-00.eps
 *
 * An actionslider is a switcher for 2 or 3 labels with customizable magnet
 * properties. The user drags and releases the indicator, to choose a label.
 *
 * Labels occupy the following positions.
 * a. Left
 * b. Right
 * c. Center
 *
 * Positions can be enabled or disabled.
 *
 * Magnets can be set on the above positions.
 *
 * When the indicator is released, it will move to its nearest
 * "enabled and magnetized" position.
 *
 * @note By default all positions are set as enabled.
 *
 * This widget inherits from the @ref Layout one, so that all the
 * functions acting on it also work for actionslider objects.
 *
 * This widget emits the following signals, besides the ones sent from
 * @ref Layout:
 * @li @c "selected" - when user selects an enabled position (the
 *              label is passed as event info).
 * @li @c "pos_changed" - when the indicator reaches any of the
 *                 positions("left", "right" or "center").
 *
 * Default text parts of the actionslider widget that you can use for are:
 * @li "indicator" - An indicator label of the actionslider
 * @li "left" - A left label of the actionslider
 * @li "right" - A right label of the actionslider
 * @li "center" - A center label of the actionslider
 *
 * Supported elm_object common APIs.
 * @li @ref elm_object_part_text_set
 * @li @ref elm_object_part_text_get
 *
 * See an example of actionslider usage @ref actionslider_example_page "here"
 * @{
 */

#define ELM_OBJ_ACTIONSLIDER_CLASS elm_obj_actionslider_class_get()

const Eo_Class *elm_obj_actionslider_class_get(void) EINA_CONST;

extern EAPI Eo_Op ELM_OBJ_ACTIONSLIDER_BASE_ID;

enum
{
   ELM_OBJ_ACTIONSLIDER_SUB_ID_INDICATOR_POS_SET,
   ELM_OBJ_ACTIONSLIDER_SUB_ID_INDICATOR_POS_GET,
   ELM_OBJ_ACTIONSLIDER_SUB_ID_MAGNET_POS_SET,
   ELM_OBJ_ACTIONSLIDER_SUB_ID_MAGNET_POS_GET,
   ELM_OBJ_ACTIONSLIDER_SUB_ID_ENABLED_POS_SET,
   ELM_OBJ_ACTIONSLIDER_SUB_ID_ENABLED_POS_GET,
   ELM_OBJ_ACTIONSLIDER_SUB_ID_SELECTED_LABEL_GET,
   ELM_OBJ_ACTIONSLIDER_SUB_ID_LAST
};

#define ELM_OBJ_ACTIONSLIDER_ID(sub_id) (ELM_OBJ_ACTIONSLIDER_BASE_ID + sub_id)


/**
 * @def elm_obj_actionslider_indicator_pos_set
 * @since 1.8
 *
 * Set actionslider indicator position.
 *
 * @param[in] pos
 *
 * @see elm_actionslider_indicator_pos_set
 */
#define elm_obj_actionslider_indicator_pos_set(pos) ELM_OBJ_ACTIONSLIDER_ID(ELM_OBJ_ACTIONSLIDER_SUB_ID_INDICATOR_POS_SET), EO_TYPECHECK(Elm_Actionslider_Pos, pos)

/**
 * @def elm_obj_actionslider_indicator_pos_get
 * @since 1.8
 *
 * Get actionslider indicator position.
 *
 * @param[out] ret
 *
 * @see elm_actionslider_indicator_pos_get
 */
#define elm_obj_actionslider_indicator_pos_get(ret) ELM_OBJ_ACTIONSLIDER_ID(ELM_OBJ_ACTIONSLIDER_SUB_ID_INDICATOR_POS_GET), EO_TYPECHECK(Elm_Actionslider_Pos *, ret)

/**
 * @def elm_obj_actionslider_magnet_pos_set
 * @since 1.8
 *
 * Set actionslider magnet position. To make multiple positions magnets or
 * them together(e.g.: ELM_ACTIONSLIDER_LEFT | ELM_ACTIONSLIDER_RIGHT)
 *
 * @param[in] pos
 *
 * @see elm_actionslider_magnet_pos_set
 */
#define elm_obj_actionslider_magnet_pos_set(pos) ELM_OBJ_ACTIONSLIDER_ID(ELM_OBJ_ACTIONSLIDER_SUB_ID_MAGNET_POS_SET), EO_TYPECHECK(Elm_Actionslider_Pos, pos)

/**
 * @def elm_obj_actionslider_magnet_pos_get
 * @since 1.8
 *
 * Get actionslider magnet position.
 *
 * @param[out] ret
 *
 * @see elm_actionslider_magnet_pos_get
 */
#define elm_obj_actionslider_magnet_pos_get(ret) ELM_OBJ_ACTIONSLIDER_ID(ELM_OBJ_ACTIONSLIDER_SUB_ID_MAGNET_POS_GET), EO_TYPECHECK(Elm_Actionslider_Pos *, ret)

/**
 * @def elm_obj_actionslider_enabled_pos_set
 * @since 1.8
 *
 * Set actionslider enabled position. To set multiple positions as enabled or
 * them together(e.g.: ELM_ACTIONSLIDER_LEFT | ELM_ACTIONSLIDER_RIGHT).
 *
 * @param[in] pos
 *
 * @see elm_actionslider_enabled_pos_set
 */
#define elm_obj_actionslider_enabled_pos_set(pos) ELM_OBJ_ACTIONSLIDER_ID(ELM_OBJ_ACTIONSLIDER_SUB_ID_ENABLED_POS_SET), EO_TYPECHECK(Elm_Actionslider_Pos, pos)

/**
 * @def elm_obj_actionslider_enabled_pos_get
 * @since 1.8
 *
 * Get actionslider enabled position.
 *
 * @param[out] ret
 *
 * @see elm_actionslider_enabled_pos_get
 */
#define elm_obj_actionslider_enabled_pos_get(ret) ELM_OBJ_ACTIONSLIDER_ID(ELM_OBJ_ACTIONSLIDER_SUB_ID_ENABLED_POS_GET), EO_TYPECHECK(Elm_Actionslider_Pos *, ret)

/**
 * @def elm_obj_actionslider_selected_label_get
 * @since 1.8
 *
 * Get actionslider selected label.
 *
 * @param[out] ret
 *
 * @see elm_actionslider_selected_label_get
 */
#define elm_obj_actionslider_selected_label_get(ret) ELM_OBJ_ACTIONSLIDER_ID(ELM_OBJ_ACTIONSLIDER_SUB_ID_SELECTED_LABEL_GET), EO_TYPECHECK(const char **, ret)


typedef enum
{
   ELM_ACTIONSLIDER_NONE = 0,
   ELM_ACTIONSLIDER_LEFT = 1 << 0,
   ELM_ACTIONSLIDER_CENTER = 1 << 1,
   ELM_ACTIONSLIDER_RIGHT = 1 << 2,
   ELM_ACTIONSLIDER_ALL = (1 << 3) - 1
} Elm_Actionslider_Pos;

/**
 * Add a new actionslider to the parent.
 *
 * @param parent The parent object
 * @return The new actionslider object or NULL if it cannot be created
 *
 * @ingroup Actionslider
 */
EAPI Evas_Object                *elm_actionslider_add(Evas_Object *parent);

/**
 * Get actionslider selected label.
 *
 * @param obj The actionslider object
 * @return The selected label
 *
 * @ingroup Actionslider
 */
EAPI const char                 *elm_actionslider_selected_label_get(const Evas_Object *obj);

/**
 * Set actionslider indicator position.
 *
 * @param obj The actionslider object.
 * @param pos The position of the indicator.
 *
 * @ingroup Actionslider
 */
EAPI void                        elm_actionslider_indicator_pos_set(Evas_Object *obj, Elm_Actionslider_Pos pos);

/**
 * Get actionslider indicator position.
 *
 * @param obj The actionslider object.
 * @return The position of the indicator.
 *
 * @ingroup Actionslider
 */
EAPI Elm_Actionslider_Pos        elm_actionslider_indicator_pos_get(const Evas_Object *obj);

/**
 * Set actionslider magnet position. To make multiple positions magnets @c or
 * them together(e.g.: ELM_ACTIONSLIDER_LEFT | ELM_ACTIONSLIDER_RIGHT)
 *
 * @param obj The actionslider object.
 * @param pos Bit mask indicating the magnet positions.
 *
 * @ingroup Actionslider
 */
EAPI void                        elm_actionslider_magnet_pos_set(Evas_Object *obj, Elm_Actionslider_Pos pos);

/**
 * Get actionslider magnet position.
 *
 * @param obj The actionslider object.
 * @return The positions with magnet property.
 *
 * @ingroup Actionslider
 */
EAPI Elm_Actionslider_Pos        elm_actionslider_magnet_pos_get(const Evas_Object *obj);

/**
 * Set actionslider enabled position. To set multiple positions as enabled @c or
 * them together(e.g.: ELM_ACTIONSLIDER_LEFT | ELM_ACTIONSLIDER_RIGHT).
 *
 * @note All the positions are enabled by default.
 *
 * @param obj The actionslider object.
 * @param pos Bit mask indicating the enabled positions.
 *
 * @ingroup Actionslider
 */
EAPI void                        elm_actionslider_enabled_pos_set(Evas_Object *obj, Elm_Actionslider_Pos pos);

/**
 * Get actionslider enabled position.
 *
 * @param obj The actionslider object.
 * @return The enabled positions.
 *
 * @ingroup Actionslider
 */
EAPI Elm_Actionslider_Pos        elm_actionslider_enabled_pos_get(const Evas_Object *obj);

/**
 * @}
 */
