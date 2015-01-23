#ifndef _ELM_CODE_WIDGET2_EO_H_
#define _ELM_CODE_WIDGET2_EO_H_

#ifndef _ELM_CODE_WIDGET2_EO_CLASS_TYPE
#define _ELM_CODE_WIDGET2_EO_CLASS_TYPE

typedef Eo Elm_Code_Widget2;

#endif

#ifndef _ELM_CODE_WIDGET2_EO_TYPES
#define _ELM_CODE_WIDGET2_EO_TYPES


#endif
#define ELM_CODE_WIDGET2_CLASS elm_code_widget2_class_get()

const Eo_Class *elm_code_widget2_class_get(void) EINA_CONST;

/**
 *
 * No description supplied.
 *
 * @param[in] font_size No description supplied.
 *
 */
EOAPI void  elm_code_widget2_font_size_set(Evas_Font_Size font_size);

/**
 *
 * No description supplied.
 *
 *
 */
EOAPI Evas_Font_Size  elm_code_widget2_font_size_get(void);

EOAPI extern const Eo_Event_Description _ELM_CODE_WIDGET2_EVENT_FOCUSED;
EOAPI extern const Eo_Event_Description _ELM_CODE_WIDGET2_EVENT_UNFOCUSED;

/**
 * No description
 */
#define ELM_CODE_WIDGET2_EVENT_FOCUSED (&(_ELM_CODE_WIDGET2_EVENT_FOCUSED))

/**
 * No description
 */
#define ELM_CODE_WIDGET2_EVENT_UNFOCUSED (&(_ELM_CODE_WIDGET2_EVENT_UNFOCUSED))

#endif
