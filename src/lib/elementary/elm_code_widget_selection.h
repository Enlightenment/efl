#ifndef ELM_CODE_WIDGET_SELECTION_H_
# define ELM_CODE_WIDGET_SELECTION_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Selection handling functions.
 * @defgroup Managing the complexities of selecting text across seperate lines.
 *
 * @{
 *
 * Functions for selection handling
 *
 */

EAPI void elm_code_widget_selection_start(Evas_Object *widget, unsigned int line, unsigned int col);

EAPI void elm_code_widget_selection_end(Evas_Object *widget, unsigned int line, unsigned int col);

EAPI void elm_code_widget_selection_clear(Evas_Object *widget);

EAPI void elm_code_widget_selection_delete(Evas_Object *widget);

EAPI void elm_code_widget_selection_select_line(Evas_Object *widget, unsigned int line);

EAPI void elm_code_widget_selection_select_word(Evas_Object *widget, unsigned int line, unsigned int col);

EAPI char *elm_code_widget_selection_text_get(Evas_Object *widget);

EAPI void elm_code_widget_selection_cut(Evas_Object *widget);
EAPI void elm_code_widget_selection_copy(Evas_Object *widget);
EAPI void elm_code_widget_selection_paste(Evas_Object *widget);

EAPI Eina_Bool elm_code_widget_selection_is_empty(Evas_Object *widget);

EAPI void elm_code_widget_selection_select_all(Evas_Object *widget);


/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* ELM_CODE_WIDGET_SELECTION_H_ */
