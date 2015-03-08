#ifndef ELM_CODE_TEXT_H_
# define ELM_CODE_TEXT_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file
 * @brief These routines are used for interacting with the textual content of files/lines in Elm Code.
 */

/**
 * @brief Text handling functions.
 * @defgroup Text access and manipulation
 *
 * @{
 *
 * Functions for handling content of lines within elm code.
 *
 */

EAPI const char *elm_code_line_text_get(Elm_Code_Line *line, unsigned int *length);

EAPI void elm_code_line_text_set(Elm_Code_Line *line, const char *chars, unsigned int length);

EAPI void elm_code_line_text_insert(Elm_Code_Line *line, unsigned int position, const char *string, int length);

EAPI void elm_code_line_text_remove(Elm_Code_Line *line, unsigned int position, int length);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* ELM_CODE_TEXT_H_ */
