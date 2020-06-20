#ifndef ELM_CODE_PARSE_H_
# define ELM_CODE_PARSE_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file
 * @brief These routines are used for handling the parsing of Elm Code content.
 */

typedef struct _Elm_Code_Parser Elm_Code_Parser;

EAPI extern Elm_Code_Parser *ELM_CODE_PARSER_STANDARD_SYNTAX; /**< A provided parser to provide syntax highlighting */
EAPI extern Elm_Code_Parser *ELM_CODE_PARSER_STANDARD_DIFF; /**< A provided parser that will mark up diff text */
EAPI extern Elm_Code_Parser *ELM_CODE_PARSER_STANDARD_TODO; /**< A provided parser that will highlight TODO and FIXME lines */

/**
 * @brief Parser helper functions.
 * @defgroup Parser  Hooking in and launching parsers
 *
 * @{
 *
 * Parser functions for marking up elm code.
 *
 */

EAPI void elm_code_parser_add(Elm_Code *code, void (*parse_line)(Elm_Code_Line *, void *),
                              void (*parse_file)(Elm_Code_File *, void *), void *data);

EAPI void elm_code_parser_standard_add(Elm_Code *code, Elm_Code_Parser *parser);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* ELM_CODE_PARSE_H_ */
