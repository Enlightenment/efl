#ifndef ELM_CODE_SYNTAX_H_
# define ELM_CODE_SYNTAX_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file
 * @brief These routines are used for handling the parsing of Elm Code content.
 */

typedef struct _Elm_Code_Syntax Elm_Code_Syntax;

/**
 * @brief Syntax highlighting helper functions.
 * @defgroup Syntax Parsing and marking up syntax in files
 *
 * @{
 *
 * Syntax functions for adding syntax highlighting to elm code.
 *
 */

/**
 * Lookup a syntax definition from a mime type.
 * If there is no syntax known NULL will be returned.
 *
 * @param mime The mime type to be looked up for a matching syntax definition
 * @return A syntax definition, if one is found, or NULL
 *
 * @ingroup Syntax
 */
EAPI Elm_Code_Syntax *elm_code_syntax_for_mime_get(const char *mime);

/**
 * Parse a line and apply the syntax definition by inserting Elm_Code_Token into the line.
 *
 * @param syntax The syntax definition to use (from elm_code_syntax_for_mime_get)
 * @param line The line that contains the content to parse and will receive the tokens
 *
 * @ingroup Syntax
 */
EAPI void elm_code_syntax_parse_line(Elm_Code_Syntax *syntax, Elm_Code_Line *line);

/**
 * Parse a file and apply the syntax definition one line at a time.
 *
 * @param syntax The syntax definition to use (from elm_code_syntax_for_mime_get)
 * @param file The file to parse - each line in the file will be processed
 *
 * @ingroup Syntax
 */
EAPI void elm_code_syntax_parse_file(Elm_Code_Syntax *syntax, Elm_Code_File *file);


/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* ELM_CODE_SYNTAX_H_ */
