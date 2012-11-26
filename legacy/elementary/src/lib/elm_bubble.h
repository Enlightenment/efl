/**
 * @defgroup Bubble Bubble
 * @ingroup Elementary
 *
 * @image html bubble_inheritance_tree.png
 * @image latex bubble_inheritance_tree.eps
 *
 * @image html img/widget/bubble/preview-00.png
 * @image latex img/widget/bubble/preview-00.eps
 * @image html img/widget/bubble/preview-01.png
 * @image latex img/widget/bubble/preview-01.eps
 * @image html img/widget/bubble/preview-02.png
 * @image latex img/widget/bubble/preview-02.eps
 *
 * @brief The Bubble is a widget to show text similar to how speech is
 * represented in comics.
 *
 * The bubble widget contains 5 important visual elements:
 * @li The frame is a rectangle with rounded edjes and an "arrow".
 * @li The @p icon is an image to which the frame's arrow points to.
 * @li The @p label is a text which appears to the right of the icon if the
 * corner is "top_left" or "bottom_left" and is right aligned to the frame
 * otherwise.
 * @li The @p info is a text which appears to the right of the label. Info's
 * font is of a lighter color than label.
 * @li The @p content is an evas object that is shown inside the frame.
 *
 * The position of the arrow, icon, label and info depends on which corner is
 * selected. The four available corners are:
 * @li "top_left" - Default
 * @li "top_right"
 * @li "bottom_left"
 * @li "bottom_right"
 *
 * This widget inherits from the @ref Layout one, so that all the
 * functions acting on it also work for bubble objects.
 *
 * This widget emits the following signals, besides the ones sent from
 * @ref Layout:
 * @li @c "clicked" - This is called when a user has clicked the bubble.
 *
 * Default content parts of the bubble that you can use for are:
 * @li "default" - A content of the bubble
 * @li "icon" - An icon of the bubble
 *
 * Default text parts of the button widget that you can use for are:
 * @li "default" - Label of the bubble
 * @li "info" - info of the bubble
 *
 * Supported elm_object common APIs.
 * @li @ref elm_object_part_text_set
 * @li @ref elm_object_part_text_get
 * @li @ref elm_object_part_content_set
 * @li @ref elm_object_part_content_get
 * @li @ref elm_object_part_content_unset
 *
 * For an example of using a bubble see @ref bubble_01_example_page "this".
 *
 * @{
 */

#define ELM_OBJ_BUBBLE_CLASS elm_obj_bubble_class_get()

const Eo_Class *elm_obj_bubble_class_get(void) EINA_CONST;

extern EAPI Eo_Op ELM_OBJ_BUBBLE_BASE_ID;

enum
{
   ELM_OBJ_BUBBLE_SUB_ID_POS_SET,
   ELM_OBJ_BUBBLE_SUB_ID_POS_GET,
   ELM_OBJ_BUBBLE_SUB_ID_LAST
};

#define ELM_OBJ_BUBBLE_ID(sub_id) (ELM_OBJ_BUBBLE_BASE_ID + sub_id)


/**
 * @def elm_obj_bubble_pos_set
 * @since 1.8
 *
 * Set the corner of the bubble
 *
 * @param[in] pos
 *
 * @see elm_bubble_pos_set
 */
#define elm_obj_bubble_pos_set(pos) ELM_OBJ_BUBBLE_ID(ELM_OBJ_BUBBLE_SUB_ID_POS_SET), EO_TYPECHECK(Elm_Bubble_Pos, pos)

/**
 * @def elm_obj_bubble_pos_get
 * @since 1.8
 *
 * Get the corner of the bubble
 *
 * @param[out] ret
 *
 * @see elm_bubble_pos_get
 */
#define elm_obj_bubble_pos_get(ret) ELM_OBJ_BUBBLE_ID(ELM_OBJ_BUBBLE_SUB_ID_POS_GET), EO_TYPECHECK(Elm_Bubble_Pos *, ret)


/**
 * Defines the corner values for a bubble.
 *
 * The corner will be used to determine where the arrow of the
 * bubble points to.
 */
typedef enum
{
  ELM_BUBBLE_POS_INVALID = -1, /**< invalid corner */
  ELM_BUBBLE_POS_TOP_LEFT, /**< the arrow of the bubble points to the top left corner. */
  ELM_BUBBLE_POS_TOP_RIGHT, /**< the arrow of the bubble points to the top right corner. */
  ELM_BUBBLE_POS_BOTTOM_LEFT, /**< the arrow of the bubble points to the bottom left corner. */
  ELM_BUBBLE_POS_BOTTOM_RIGHT, /**< the arrow of the bubble points to the bottom right corner. */
} Elm_Bubble_Pos;

/**
 * Add a new bubble to the parent
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * This function adds a text bubble to the given parent evas object.
 *
 * @ingroup Bubble
 */
EAPI Evas_Object                 *elm_bubble_add(Evas_Object *parent);

/**
 * Set the corner of the bubble
 *
 * @param obj The bubble object.
 * @param pos The given corner for the bubble.
 *
 * This function sets the corner of the bubble. The corner will be used to
 * determine where the arrow in the frame points to and where label, icon and
 * info are shown.
 *
 *
 * @ingroup Bubble
 */
EAPI void  elm_bubble_pos_set(Evas_Object *obj, Elm_Bubble_Pos pos);

/**
 * Get the corner of the bubble
 *
 * @param obj The bubble object.
 * @return The given corner for the bubble.
 *
 * This function gets the selected corner of the bubble.
 *
 * @ingroup Bubble
 */
EAPI Elm_Bubble_Pos elm_bubble_pos_get(const Evas_Object *obj);

/**
 * @}
 */
