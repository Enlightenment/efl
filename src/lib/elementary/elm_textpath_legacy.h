typedef Eo Elm_Textpath;

/**
 * @brief Add a new textpath to the parent
 *
 * @param[in] parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * @ingroup Elm_Textpath_Group
 *
 * @since 1.22
 */
EAPI Evas_Object                 *elm_textpath_add(Evas_Object *parent);

/**
 * @brief Set the user text style
 *
 * @param[in] obj The textpath object
 * @param[in] style The user text style. If the sytle is $null, the default style will be applied
 *
 * @note ellipsis in the style will be ignored since textpath supports ellipsis API.
 * @see elm_textpath_ellipsis_set()
 *
 * @ingroup Elm_Textpath_Group
 *
 * @since 1.25
 */
EAPI void                        elm_textpath_text_user_style_set(Evas_Object *obj, const char *style);

#include "efl_ui_textpath_eo.legacy.h"
