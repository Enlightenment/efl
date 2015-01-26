#ifndef _ELM_CODE_WIDGET_EO_H_
#define _ELM_CODE_WIDGET_EO_H_

#ifndef _ELM_CODE_WIDGET_EO_CLASS_TYPE
#define _ELM_CODE_WIDGET_EO_CLASS_TYPE

typedef Eo Elm_Code_Widget;

#endif

#ifndef _ELM_CODE_WIDGET_EO_TYPES
#define _ELM_CODE_WIDGET_EO_TYPES


#endif
#define ELM_CODE_WIDGET_CLASS elm_code_widget_class_get()

const Eo_Class *elm_code_widget_class_get(void) EINA_CONST;

/**
 *
 * No description supplied.
 *
 * @param[in] code No description supplied.
 *
 */
EOAPI void  elm_code_widget_code_set(Elm_Code *code);

/**
 *
 * No description supplied.
 *
 *
 */
EOAPI Elm_Code * elm_code_widget_code_get(void);

/**
 *
 * No description supplied.
 *
 * @param[in] font_size No description supplied.
 *
 */
EOAPI void  elm_code_widget_font_size_set(Evas_Font_Size font_size);

/**
 *
 * No description supplied.
 *
 *
 */
EOAPI Evas_Font_Size  elm_code_widget_font_size_get(void);

/**
 *
 * No description supplied.
 *
 * @param[in] editable No description supplied.
 *
 */
EOAPI void  elm_code_widget_editable_set(Eina_Bool editable);

/**
 *
 * No description supplied.
 *
 *
 */
EOAPI Eina_Bool  elm_code_widget_editable_get(void);

EOAPI extern const Eo_Event_Description _ELM_CODE_WIDGET_EVENT_LINE_CLICKED;

/**
 * No description
 */
#define ELM_CODE_WIDGET_EVENT_LINE_CLICKED (&(_ELM_CODE_WIDGET_EVENT_LINE_CLICKED))

#endif
