#ifndef ELM_CODE_WIDGET_TEXT_H_
# define ELM_CODE_WIDGET_TEXT_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Text layout handling functions.
 * @defgroup Managing the complexities of layout out text in an Elm_Code_Widget
 *
 * @{
 *
 * Functions for text layout handling
 *
 */

EAPI int elm_code_widget_text_left_gutter_width_get(Elm_Code_Widget *widget);

EAPI int elm_code_widget_text_line_number_width_get(Elm_Code_Widget *widget);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* ELM_CODE_WIDGET_TEXT_H_ */
