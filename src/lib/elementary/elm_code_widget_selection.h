#ifndef ELM_CODE_WIDGET_SELECTION_H_
# define ELM_CODE_WIDGET_SELECTION_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Selection handling functions.
 * @defgroup Managing the complexities of selecting text across separate lines.
 *
 * @{
 *
 * Functions for selection handling
 *
 */

/**
 * @brief Start the selection of widget contents.
 *
 * @param[in] widget @c The elm_widget object.
 * @param[in] line @c The line index of the widget.
 * @param[in] widget @c The column index of the widget.
 *
 * @return None
 *
 */
EAPI void elm_code_widget_selection_start(Evas_Object *widget, unsigned int line, unsigned int col);

/**
 * @brief End the selection of widget contents.
 *
 * @param[in] widget @c The elm_widget object.
 * @param[in] line @c The line index of the widget.
 * @param[in] widget @c The column index of the widget.
 *
 * @return None
 *
 */
EAPI void elm_code_widget_selection_end(Evas_Object *widget, unsigned int line, unsigned int col);

/**
 * @brief Clear selection of widget contents.
 *
 * @param[in] widget @c The elm_widget object.
 *
 * @return None
 *
 */
EAPI void elm_code_widget_selection_clear(Evas_Object *widget);

/**
 * @brief Delete the selected contents of widget contents.
 *
 * @param[in] widget @c The elm_widget object.
 *
 * @return None
 *
 */
EAPI void elm_code_widget_selection_delete(Evas_Object *widget);

/**
 * @brief Select the entire line from the widget.
 *
 * @param[in] widget @c The elm_widget object.
 * @param[in] line @c The line index of the widget.
 *
 * @return None
 *
 */
EAPI void elm_code_widget_selection_select_line(Evas_Object *widget, unsigned int line);

/**
 * @brief Select a word from the widget.
 *
 * @param[in] widget @c The elm_widget object.
 * @param[in] line @c The line index of the widget.
 * @param[in] col @c The column index of the widget.
 *
 * @return None
 *
 */
EAPI void elm_code_widget_selection_select_word(Evas_Object *widget, unsigned int line, unsigned int col);

/**
 * @brief Get the selected widget content.
 *
 * @param[in] widget @c The elm_widget object.
 *
 * @return char pointer to text contents.
 *
 */
EAPI char *elm_code_widget_selection_text_get(Evas_Object *widget);

/**
 * @brief Cut the selected widget content.
 *
 * @param[in] widget @c The elm_widget object.
 *
 * @return None
 *
 */
EAPI void elm_code_widget_selection_cut(Evas_Object *widget);

/**
 * @brief Copy the selected widget content to clipboard.
 *
 * @param[in] widget @c The elm_widget object.
 *
 * @return None
 *
 */
EAPI void elm_code_widget_selection_copy(Evas_Object *widget);

/**
 * @brief Paste the copied widget content from clipboard.
 *
 * @param[in] widget @c The elm_widget object.
 *
 * @return None
 *
 */

EAPI void elm_code_widget_selection_paste(Evas_Object *widget);

/**
 * @brief check if the widget selection is empty.
 *
 * @param[in] widget @c The elm_widget object.
 *
 * @return true is empty, false otherwise
 *
 */
EAPI Eina_Bool elm_code_widget_selection_is_empty(Evas_Object *widget);

/**
 * @brief Select the entire widget contents.
 *
 * @param[in] widget @c The elm_widget object.
 *
 * @return None
 *
 */
EAPI void elm_code_widget_selection_select_all(Evas_Object *widget);


/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* ELM_CODE_WIDGET_SELECTION_H_ */
