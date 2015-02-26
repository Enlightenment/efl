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
   void (*parse_line)(Elm_Code_Line *); 

   void (*parse_file)(Elm_Code_File *);
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

EAPI void elm_code_parser_add(Elm_Code *code, void (*parse_line)(Elm_Code_Line *),
                              void (*parse_file)(Elm_Code_File *));

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* ELM_CODE_PARSE_H_ */
