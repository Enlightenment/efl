#ifndef _ELM_VIEW_FORM_EO_H_
#define _ELM_VIEW_FORM_EO_H_

#ifndef _ELM_VIEW_FORM_EO_CLASS_TYPE
#define _ELM_VIEW_FORM_EO_CLASS_TYPE

typedef Eo Elm_View_Form;

#endif

#ifndef _ELM_VIEW_FORM_EO_TYPES
#define _ELM_VIEW_FORM_EO_TYPES


#endif
/** Elementary view form class
 *
 * @ingroup Elm_View_Form
 */
#define ELM_VIEW_FORM_CLASS elm_view_form_class_get()

EWAPI const Efl_Class *elm_view_form_class_get(void);

/**
 * @brief Set model
 *
 * @param[in] obj The object.
 * @param[in] model Emodel object
 *
 * @since 1.11
 *
 * @ingroup Elm_View_Form
 */
EOAPI void elm_view_form_model_set(Eo *obj, Efl_Model *model);

/**
 * @brief Add new widget
 *
 * @param[in] obj The object.
 * @param[in] propname Property name
 * @param[in] evas Evas widget
 *
 * @since 1.11
 *
 * @ingroup Elm_View_Form
 */
EOAPI void elm_view_form_widget_add(Eo *obj, const char *propname, Efl_Canvas_Object *evas);

#endif
