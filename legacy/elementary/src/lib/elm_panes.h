/**
 * @defgroup Panes Panes
 * @ingroup Elementary
 *
 * @image html panes_inheritance_tree.png
 * @image latex panes_inheritance_tree.eps
 *
 * @image html img/widget/panes/preview-00.png
 * @image latex img/widget/panes/preview-00.eps width=\textwidth
 *
 * @image html img/panes.png
 * @image latex img/panes.eps width=\textwidth
 *
 * The panes widget adds a draggable bar between two contents. When dragged
 * this bar will resize contents' size.
 *
 * Panes can be split vertically or horizontally, and contents
 * size proportion can be customized (homogeneous by default).
 *
 * This widget inherits from the @ref Layout one, so that all the
 * functions acting on it also work for panes objects.
 *
 * This widget emits the following signals, besides the ones sent from
 * @ref Layout:
 * - @c "press" - The panes has been pressed (button wasn't released yet).
 * - @c "unpressed" - The panes was released after being pressed.
 * - @c "clicked" - The panes has been clicked>
 * - @c "clicked,double" - The panes has been double clicked
 *
 * Available styles for it:
 * - @c "default"
 *
 * Default content parts of the panes widget that you can use are:
 * @li "left" - A leftside content of the panes
 * @li "right" - A rightside content of the panes
 * @li "top" - A top content of the vertical panes
 * @li "bottom" - A bottom content of the vertical panes
 *
 * If panes are displayed vertically, left content will be displayed on
 * top.
 *
 * Supported elm_object common APIs.
 * @li @ref elm_object_part_content_set
 * @li @ref elm_object_part_content_get
 * @li @ref elm_object_part_content_unset
 *
 * Here is an example on its usage:
 * @li @ref panes_example
 */

#define ELM_OBJ_PANES_CLASS elm_obj_panes_class_get()

const Eo_Class *elm_obj_panes_class_get(void) EINA_CONST;

extern EAPI Eo_Op ELM_OBJ_PANES_BASE_ID;

enum
{
   ELM_OBJ_PANES_SUB_ID_CONTENT_LEFT_SIZE_GET,
   ELM_OBJ_PANES_SUB_ID_CONTENT_LEFT_SIZE_SET,
   ELM_OBJ_PANES_SUB_ID_CONTENT_RIGHT_SIZE_GET,
   ELM_OBJ_PANES_SUB_ID_CONTENT_RIGHT_SIZE_SET,
   ELM_OBJ_PANES_SUB_ID_HORIZONTAL_SET,
   ELM_OBJ_PANES_SUB_ID_HORIZONTAL_GET,
   ELM_OBJ_PANES_SUB_ID_FIXED_SET,
   ELM_OBJ_PANES_SUB_ID_FIXED_GET,
   ELM_OBJ_PANES_SUB_ID_LAST
};

#define ELM_OBJ_PANES_ID(sub_id) (ELM_OBJ_PANES_BASE_ID + sub_id)


/**
 * @def elm_obj_panes_content_left_size_get
 * @since 1.8
 *
 * Get the size proportion of panes widget's left side.
 *
 * @param[out] ret
 *
 * @see elm_panes_content_left_size_get
 */
#define elm_obj_panes_content_left_size_get(ret) ELM_OBJ_PANES_ID(ELM_OBJ_PANES_SUB_ID_CONTENT_LEFT_SIZE_GET), EO_TYPECHECK(double *, ret)

/**
 * @def elm_obj_panes_content_left_size_set
 * @since 1.8
 *
 * Set the size proportion of panes widget's left side.
 *
 * @param[in] size
 *
 * @see elm_panes_content_left_size_set
 */
#define elm_obj_panes_content_left_size_set(size) ELM_OBJ_PANES_ID(ELM_OBJ_PANES_SUB_ID_CONTENT_LEFT_SIZE_SET), EO_TYPECHECK(double, size)

/**
 * @def elm_obj_panes_content_right_size_get
 * @since 1.8
 *
 * Get the size proportion of panes widget's right side.
 *
 * @param[out] ret
 *
 * @see elm_panes_content_right_size_get
 */
#define elm_obj_panes_content_right_size_get(ret) ELM_OBJ_PANES_ID(ELM_OBJ_PANES_SUB_ID_CONTENT_RIGHT_SIZE_GET), EO_TYPECHECK(double *, ret)

/**
 * @def elm_obj_panes_content_right_size_set
 * @since 1.8
 *
 * Set the size proportion of panes widget's right side.
 *
 * @param[in] size
 *
 * @see elm_panes_content_right_size_set
 */
#define elm_obj_panes_content_right_size_set(size) ELM_OBJ_PANES_ID(ELM_OBJ_PANES_SUB_ID_CONTENT_RIGHT_SIZE_SET), EO_TYPECHECK(double, size)

/**
 * @def elm_obj_panes_horizontal_set
 * @since 1.8
 *
 * Set how to split and dispose each content.
 *
 * @param[in] horizontal
 *
 * @see elm_panes_horizontal_set
 */
#define elm_obj_panes_horizontal_set(horizontal) ELM_OBJ_PANES_ID(ELM_OBJ_PANES_SUB_ID_HORIZONTAL_SET), EO_TYPECHECK(Eina_Bool, horizontal)

/**
 * @def elm_obj_panes_horizontal_get
 * @since 1.8
 *
 * Retrieve the split direction of a given panes widget.
 *
 * @param[out] ret
 *
 * @see elm_panes_horizontal_get
 */
#define elm_obj_panes_horizontal_get(ret) ELM_OBJ_PANES_ID(ELM_OBJ_PANES_SUB_ID_HORIZONTAL_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_panes_fixed_set
 * @since 1.8
 *
 * Set whether the left and right panes can be resized by user interaction.
 *
 * @param[in] fixed
 *
 * @see elm_panes_fixed_set
 */
#define elm_obj_panes_fixed_set(fixed) ELM_OBJ_PANES_ID(ELM_OBJ_PANES_SUB_ID_FIXED_SET), EO_TYPECHECK(Eina_Bool, fixed)

/**
 * @def elm_obj_panes_fixed_get
 * @since 1.8
 *
 * Retrieve the resize mode for the panes of a given panes widget.
 *
 * @param[out] ret
 *
 * @see elm_panes_fixed_get
 */
#define elm_obj_panes_fixed_get(ret) ELM_OBJ_PANES_ID(ELM_OBJ_PANES_SUB_ID_FIXED_GET), EO_TYPECHECK(Eina_Bool *, ret)
/**
 * @addtogroup Panes
 * @{
 */

/**
 * Add a new panes widget to the given parent Elementary
 * (container) object.
 *
 * @param parent The parent object.
 * @return a new panes widget handle or @c NULL, on errors.
 *
 * This function inserts a new panes widget on the canvas.
 *
 * @ingroup Panes
 */
EAPI Evas_Object                 *elm_panes_add(Evas_Object *parent);

/**
 * Set whether the left and right panes can be resized by user interaction.
 *
 * @param obj The panes object.
 * @param fixed Use @c EINA_TRUE to fix the left and right panes sizes and make
 * them not to be resized by user interaction. Use @c EINA_FALSE to make them
 * resizable.
 *
 * By default panes' contents are resizable by user interaction.
 *
 * @see elm_panes_fixed_get()
 * @see elm_panes_content_left_size_set()
 * @see elm_panes_content_right_size_set()
 *
 * @ingroup Panes
 */
EAPI void                         elm_panes_fixed_set(Evas_Object *obj, Eina_Bool fixed);

/**
 * Retrieve the resize mode for the panes of a given panes widget.
 *
 * @param obj The panes object.
 * @return @c EINA_TRUE, if @p obj is set to be resizable by user interaction.
 *
 * @see elm_panes_fixed_set() for more details.
 * @see elm_panes_content_left_size_get()
 * @see elm_panes_content_right_size_get()
 *
 * @ingroup Panes
 */
EAPI Eina_Bool                    elm_panes_fixed_get(const Evas_Object *obj);

/**
 * Get the size proportion of panes widget's left side.
 *
 * @param obj The panes object.
 * @return float value between 0.0 and 1.0 representing size proportion
 * of left side.
 *
 * @see elm_panes_content_left_size_set() for more details.
 *
 * @ingroup Panes
 */
EAPI double                       elm_panes_content_left_size_get(const Evas_Object *obj);

/**
 * Set the size proportion of panes widget's left side.
 *
 * @param obj The panes object.
 * @param size Value between 0.0 and 1.0 representing size proportion
 * of left side.
 *
 * By default it's homogeneous, i.e., both sides have the same size.
 *
 * If something different is required, it can be set with this function.
 * For example, if the left content should be displayed over
 * 75% of the panes size, @p size should be passed as @c 0.75.
 * This way, right content will be resized to 25% of panes size.
 *
 * If displayed vertically, left content is displayed at top, and
 * right content at bottom.
 *
 * @note This proportion will change when user drags the panes bar.
 *
 * @see elm_panes_content_left_size_get()
 *
 * @ingroup Panes
 */
EAPI void                         elm_panes_content_left_size_set(Evas_Object *obj, double size);

/**
 * Get the size proportion of panes widget's right side.
 *
 * @param obj The panes object.
 * @return float value between 0.0 and 1.0 representing size proportion
 * of right side.
 *
 * @see elm_panes_content_right_size_set() for more details.
 *
 * @ingroup Panes
 */
EAPI double                       elm_panes_content_right_size_get(const Evas_Object *obj);

/**
 * Set the size proportion of panes widget's right side.
 *
 * @param obj The panes object.
 * @param size Value between 0.0 and 1.0 representing size proportion
 * of right side.
 *
 * By default it's homogeneous, i.e., both sides have the same size.
 *
 * If something different is required, it can be set with this function.
 * For example, if the right content should be displayed over
 * 75% of the panes size, @p size should be passed as @c 0.75.
 * This way, left content will be resized to 25% of panes size.
 *
 * If displayed vertically, left content is displayed at top, and
 * right content at bottom.
 *
 * @note This proportion will change when user drags the panes bar.
 *
 * @see elm_panes_content_right_size_get()
 *
 * @ingroup Panes
 */
EAPI void                         elm_panes_content_right_size_set(Evas_Object *obj, double size);


/**
 * Set how to split and dispose each content.
 *
 * @param obj The panes object.
 * @param horizontal Use @c EINA_TRUE to make @p obj to split panes
 * horizontally ("top" and  "bottom" contents). @c EINA_FALSE to make it
 * vertically ("left" and "right" contents)
 *
 * Use this function to change how your panes is to be disposed:
 * vertically or horizontally.
 * Horizontal panes have "top" and "bottom" contents, vertical panes have
 * "left" and "right" contents.
 *
 * By default panes is in a vertical mode.
 *
 * @see elm_panes_horizontal_get()
 *
 * @ingroup Panes
 */
EAPI void                         elm_panes_horizontal_set(Evas_Object *obj, Eina_Bool horizontal);

/**
 * Retrieve the split direction of a given panes widget.
 *
 * @param obj The panes object.
 * @return @c EINA_TRUE, if @p obj is set to be @b horizontal,
 * @c EINA_FALSE if it's @b vertical (and on errors).
 *
 * @see elm_panes_horizontal_set() for more details.
 *
 * @ingroup Panes
 */
EAPI Eina_Bool                    elm_panes_horizontal_get(const Evas_Object *obj);

/**
 * @}
 */
