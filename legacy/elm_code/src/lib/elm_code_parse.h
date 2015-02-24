#ifndef ELM_CODE_PARSE_H_
# define ELM_CODE_PARSE_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file
 * @brief These routines are used for handling the parsing of Elm Code content.
 */

typedef struct _Elm_Code_Parser
{
   void (*parse_line)(Elm_Code_Line *, void *);

   void (*parse_file)(Elm_Code_File *, void *);

   void *data;
} Elm_Code_Parser;

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

EAPI void elm_code_parse_line(Elm_Code *code, Elm_Code_Line *line);

EAPI void elm_code_parse_file(Elm_Code *code, Elm_Code_File *file);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* ELM_CODE_PARSE_H_ */
