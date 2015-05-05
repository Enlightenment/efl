#ifndef ELM_CODE_TEXT_H_
# define ELM_CODE_TEXT_H_

#define ELM_CODE_TEXT_NOT_FOUND -1

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file
 * @brief These routines are used for interacting with the textual content of files/lines in Elm Code.
 */

/**
 * @brief Line text handling functions.
 * @defgroup Text access and manipulation within lines
 *
 * @{
 *
 * Functions for handling content of lines within elm code.
 *
 */

EAPI const char *elm_code_line_text_get(Elm_Code_Line *line, unsigned int *length);

EAPI void elm_code_line_text_set(Elm_Code_Line *line, const char *chars, unsigned int length);

EAPI int elm_code_line_text_strpos(Elm_Code_Line *line, const char *search, int offset);

EAPI Eina_Bool elm_code_line_text_contains(Elm_Code_Line *line, const char *search);

EAPI char *elm_code_line_text_substr(Elm_Code_Line *line, unsigned int position, int length);

EAPI void elm_code_line_text_insert(Elm_Code_Line *line, unsigned int position, const char *string, int length);

EAPI void elm_code_line_text_remove(Elm_Code_Line *line, unsigned int position, int length);

/**
 * @}
 *
 * @brief Generic text handling functions.
 * @defgroup Text helper functions
 *
 * @{
 *
 * Functions for managing unicode text.
 *
 */

EAPI unsigned int elm_code_text_tabwidth_at_position(unsigned int position, unsigned int tabstop);

EAPI int elm_code_text_strnpos(const char *text, unsigned int length, const char *search, int offset);

EAPI int elm_code_text_newlinenpos(const char *text, unsigned int length, short *nllen);

EAPI unsigned int elm_code_line_text_column_width_to_position(Elm_Code_Line *line, unsigned int length, unsigned int tabstop);

EAPI unsigned int elm_code_line_text_column_width(Elm_Code_Line *line, unsigned int tabstop);

EAPI unsigned int elm_code_line_text_position_for_column_get(Elm_Code_Line *line, unsigned int column, unsigned int tabstop);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* ELM_CODE_TEXT_H_ */
