#ifndef _ELM_COMBOBOX_EO_CLASS_TYPE
#define _ELM_COMBOBOX_EO_CLASS_TYPE

typedef Eo Elm_Combobox;

#endif

/**
 * @brief Add a new Combobox object
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * @ingroup Elm_Combobox
 */
EAPI Evas_Object                 *elm_combobox_add(Evas_Object *parent);

/**
 * @brief Returns whether the combobox is expanded.
 *
 * This will return EINA_TRUE if the combobox is expanded or EINA_FALSE if it
 * is not expanded.
 *
 * @param[in] obj The object.
 *
 * @return @c true if combobox is expenaded, @c false otherwise
 *
 * @since 1.17
 *
 * @ingroup Elm_Combobox_Group
 */
EAPI Eina_Bool elm_combobox_expanded_get(const Elm_Combobox *obj);

/** This triggers the combobox popup from code, the same as if the user had
 * clicked the button.
 *
 * @since 1.17
 *
 * @ingroup Elm_Combobox_Group
 */
EAPI void elm_combobox_hover_begin(Elm_Combobox *obj);

/** This dismisses the combobox popup as if the user had clicked outside the
 * hover.
 *
 * @since 1.17
 *
 * @ingroup Elm_Combobox_Group
 */
EAPI void elm_combobox_hover_end(Elm_Combobox *obj);

#include "elm_combobox.eo.legacy.h"
