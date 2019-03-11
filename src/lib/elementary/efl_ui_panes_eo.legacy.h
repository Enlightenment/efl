#ifndef _EFL_UI_PANES_EO_LEGACY_H_
#define _EFL_UI_PANES_EO_LEGACY_H_

#ifndef _EFL_UI_PANES_EO_CLASS_TYPE
#define _EFL_UI_PANES_EO_CLASS_TYPE

typedef Eo Efl_Ui_Panes;

#endif

#ifndef _EFL_UI_PANES_EO_TYPES
#define _EFL_UI_PANES_EO_TYPES


#endif



/**
 * @brief Set whether the left and right panes can be resized by user
 * interaction.
 *
 * By default panes' contents are resizable by user interaction.
 *
 * @param[in] obj The object.
 * @param[in] fixed Use @c true to fix the left and right panes sizes and make
 * them not to be resized by user interaction. Use @c false to make them
 * resizable.
 *
 * @ingroup Elm_Panes_Group
 */
EAPI void elm_panes_fixed_set(Efl_Ui_Panes *obj, Eina_Bool fixed);

/**
 * @brief Set whether the left and right panes can be resized by user
 * interaction.
 *
 * By default panes' contents are resizable by user interaction.
 *
 * @param[in] obj The object.
 *
 * @return Use @c true to fix the left and right panes sizes and make them not
 * to be resized by user interaction. Use @c false to make them resizable.
 *
 * @ingroup Elm_Panes_Group
 */
EAPI Eina_Bool elm_panes_fixed_get(const Efl_Ui_Panes *obj);

#endif
