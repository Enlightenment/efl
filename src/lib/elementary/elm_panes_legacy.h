typedef Eo Elm_Panes;

/**
 * Add a new panes widget to the given parent Elementary
 * (container) object.
 *
 * @param parent The parent object.
 * @return a new panes widget handle or @c NULL, on errors.
 *
 * This function inserts a new panes widget on the canvas.
 *
 * @ingroup Elm_Panes
 */
EAPI Evas_Object                 *elm_panes_add(Evas_Object *parent);

/**
 * @brief Set how to split and dispose each content.
 *
 * Use this function to change how your panes is to be disposed: vertically or
 * horizontally. Horizontal panes have "top" and "bottom" contents, vertical
 * panes have "left" and "right" contents.
 *
 * By default panes is in a vertical mode.
 *
 * @param[in] horizontal Use @c true to make @c obj to split panes horizontally
 * ("top" and "bottom" contents). @c false to make it vertically ("left" and
 * "right" contents).
 *
 * @ingroup Elm_Panes
 */
EAPI void elm_panes_horizontal_set(Evas_Object *obj, Eina_Bool horizontal);

/**
 * @brief Get how to split and dispose each content.
 *
 * Use this function to change how your panes is to be disposed: vertically or
 * horizontally. Horizontal panes have "top" and "bottom" contents, vertical
 * panes have "left" and "right" contents.
 *
 * By default panes is in a vertical mode.
 *
 * @return Use @c true to make @c obj to split panes horizontally ("top" and
 * "bottom" contents). @c false to make it vertically ("left" and "right"
 * contents).
 *
 * @ingroup Elm_Panes
 */
EAPI Eina_Bool elm_panes_horizontal_get(const Evas_Object *obj);

/**
 * @brief Set the absolute minimum size of panes widget's left side.
 *
 * @note If displayed vertically, left content is displayed at top.
 *
 * @param[in] size Value representing minimum size of left side in pixels.
 *
 * @ingroup Efl_Ui_Panes
 */
EAPI void elm_panes_content_left_min_size_set(Evas_Object *obj, int size);

/**
 * @brief Get the absolute minimum size of panes widget's left side.
 *
 * @note If displayed vertically, left content is displayed at top.
 *
 * @return Value representing minimum size of left side in pixels.
 *
 * @ingroup Efl_Ui_Panes
 */
EAPI int elm_panes_content_left_min_size_get(const Evas_Object *obj);

/**
 * @brief Control the absolute minimum size of panes widget's right side.
 *
 * @note If displayed vertically, right content is displayed at bottom.
 *
 * @param[in] size Value representing minimum size of right side in pixels.
 *
 * @ingroup Efl_Ui_Panes
 */
EAPI void elm_panes_content_right_min_size_set(Evas_Object *obj, int size);

/**
 * @brief Get the absolute minimum size of panes widget's right side.
 *
 * @note If displayed vertically, right content is displayed at bottom.
 *
 * @return Value representing minimum size of right side in pixels.
 *
 * @ingroup Efl_Ui_Panes
 */
EAPI int elm_panes_content_right_min_size_get(const Evas_Object *obj);

/**
 * @brief Set the size proportion of panes widget's right side.
 *
 * By default it's homogeneous, i.e., both sides have the same size.
 *
 * If something different is required, it can be set with this function. For
 * example, if the right content should be displayed over 75% of the panes
 * size, @c size should be passed as 0.75. This way, left content will be
 * resized to 25% of panes size.
 *
 * If displayed vertically, left content is displayed at top, and right content
 * at bottom.
 *
 * @note This proportion will change when user drags the panes bar.
 *
 * @param[in] size Value between 0.0 and 1.0 representing size proportion of
 * right side.
 *
 * @ingroup Efl_Ui_Panes
 */
EAPI void elm_panes_content_right_size_set(Evas_Object *obj, double size);

/**
 * @brief Get the size proportion of panes widget's right side.
 *
 * By default it's homogeneous, i.e., both sides have the same size.
 *
 * If displayed vertically, left content is displayed at top, and right content
 * at bottom.
 *
 * @note This proportion will change when user drags the panes bar.
 *
 * @return Value between 0.0 and 1.0 representing size proportion of right
 * side.
 *
 * @ingroup Efl_Ui_Panes
 */
EAPI double elm_panes_content_right_size_get(const Evas_Object *obj);

/**
 * @brief Set the size proportion of panes widget's left side.
 *
 * By default it's homogeneous, i.e., both sides have the same size.
 *
 * If something different is required, it can be set with this function. For
 * example, if the left content should be displayed over 75% of the panes size,
 * @c size should be passed as 0.75. This way, right content will be resized to
 * 25% of panes size.
 *
 * If displayed vertically, left content is displayed at top, and right content
 * at bottom.
 *
 * @note This proportion will change when user drags the panes bar.
 *
 * @param[in] size Value between 0.0 and 1.0 representing size proportion of
 * left side.
 *
 * @ingroup Efl_Ui_Panes
 */
EAPI void elm_panes_content_left_size_set(Evas_Object *obj, double size);

/**
 * @brief Get the size proportion of panes widget's left side.
 *
 * By default it's homogeneous, i.e., both sides have the same size.
 *
 * If displayed vertically, left content is displayed at top, and right content
 * at bottom.
 *
 * @note This proportion will change when user drags the panes bar.
 *
 * @return Value between 0.0 and 1.0 representing size proportion of left side.
 *
 * @ingroup Efl_Ui_Panes
 */
EAPI double elm_panes_content_left_size_get(const Evas_Object *obj);

/**
 * @brief Controls the relative minimum size of panes widget's left side.
 *
 * proportion of minimum size of left side.
 *
 * @note If displayed vertically, left content is displayed at top.
 *
 * @param[in] size Value between 0.0 and 1.0 representing size proportion of
 * minimum size of left side.
 *
 * @ingroup Efl_Ui_Panes
 */
EAPI void elm_panes_content_left_min_relative_size_set(Evas_Object *obj, double size);

/**
 * @brief Get the relative minimum size of panes widget's left side.
 *
 * proportion of minimum size of left side.
 *
 * @note If displayed vertically, left content is displayed at top.
 *
 * @return Value between 0.0 and 1.0 representing size proportion of minimum
 * size of left side.
 *
 * @ingroup Efl_Ui_Panes
 */
EAPI double elm_panes_content_left_min_relative_size_get(const Evas_Object *obj);

/**
 * @brief Set the relative minimum size of panes widget's right side.
 *
 * @note If displayed vertically, right content is displayed at bottom.
 *
 * @param[in] size Value between 0.0 and 1.0 representing size proportion of
 * minimum size of right side.
 *
 * @ingroup Efl_Ui_Panes
 */
EAPI void elm_panes_content_right_min_relative_size_set(Evas_Object *obj, double size);

/**
 * @brief Get the relative minimum size of panes widget's right side.
 *
 * @note If displayed vertically, right content is displayed at bottom.
 *
 * @return Value between 0.0 and 1.0 representing size proportion of minimum
 * size of right side.
 *
 * @ingroup Efl_Ui_Panes
 */
EAPI double elm_panes_content_right_min_relative_size_get(const Evas_Object *obj);

#include "efl_ui_panes.eo.legacy.h"
