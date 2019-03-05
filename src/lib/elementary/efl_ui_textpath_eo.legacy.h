#ifndef _EFL_UI_TEXTPATH_EO_LEGACY_H_
#define _EFL_UI_TEXTPATH_EO_LEGACY_H_

#ifndef _EFL_UI_TEXTPATH_EO_CLASS_TYPE
#define _EFL_UI_TEXTPATH_EO_CLASS_TYPE

typedef Eo Efl_Ui_Textpath;

#endif

#ifndef _EFL_UI_TEXTPATH_EO_TYPES
#define _EFL_UI_TEXTPATH_EO_TYPES

/** Textpath direction
 *
 * @ingroup Efl_Ui
 */
typedef enum
{
  EFL_UI_TEXTPATH_DIRECTION_CW = 0, /**< Clockwise */
  EFL_UI_TEXTPATH_DIRECTION_CCW /**< Counter-clockwise */
} Efl_Ui_Textpath_Direction;


#endif

/**
 * @brief Set a circle with given center, radius, and start angle.
 *
 * @param[in] obj The object.
 * @param[in] x X coordinate of center
 * @param[in] y Y coordinate of center
 * @param[in] radius Radius of the circle
 * @param[in] start_angle Start angle of the circle
 * @param[in] direction Textpath direction
 *
 * @ingroup Elm_Textpath_Group
 */
EAPI void elm_textpath_circle_set(Efl_Ui_Textpath *obj, double x, double y, double radius, double start_angle, Efl_Ui_Textpath_Direction direction);

/**
 * @brief The number of slices. The larger the number of slice_num is, The
 * better the text follows the path.
 *
 * @param[in] obj The object.
 * @param[in] slice_no Number of slices
 *
 * @ingroup Elm_Textpath_Group
 */
EAPI void elm_textpath_slice_number_set(Efl_Ui_Textpath *obj, int slice_no);

/**
 * @brief The number of slices. The larger the number of slice_num is, The
 * better the text follows the path.
 *
 * @param[in] obj The object.
 *
 * @return Number of slices
 *
 * @ingroup Elm_Textpath_Group
 */
EAPI int elm_textpath_slice_number_get(const Efl_Ui_Textpath *obj);

/**
 * @brief Control the ellipsis behavior of the textpath.
 *
 * @param[in] obj The object.
 * @param[in] ellipsis To ellipsis text or not
 *
 * @ingroup Elm_Textpath_Group
 */
EAPI void elm_textpath_ellipsis_set(Efl_Ui_Textpath *obj, Eina_Bool ellipsis);

/**
 * @brief Control the ellipsis behavior of the textpath.
 *
 * @param[in] obj The object.
 *
 * @return To ellipsis text or not
 *
 * @ingroup Elm_Textpath_Group
 */
EAPI Eina_Bool elm_textpath_ellipsis_get(const Efl_Ui_Textpath *obj);

#endif
