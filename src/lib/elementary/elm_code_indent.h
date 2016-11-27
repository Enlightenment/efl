#ifndef ELM_CODE_INDENT_H_
# define ELM_CODE_INDENT_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file
 * @brief These routines are used for calculating text indentation of lines in Elm Code.
 */

/**
 * @brief Line indent handling functions.
 * @defgroup Indent management of indentation of lines
 *
 * @{
 *
 * Functions for calculation indentation of lines within elm code.
 *
 */

EAPI char *elm_code_line_indent_get(const char *prevtext, unsigned int prevlength);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* ELM_CODE_INDENT_H_ */
