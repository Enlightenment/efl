#ifndef ELM_CODE_LINE_H_
# define ELM_CODE_LINE_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file
 * @brief These routines are used for interacting with lines of content using Elm Code.
 */

typedef struct _Elm_Code_Token
{
   int start, end;

   Elm_Code_Token_Type type;

} Elm_Code_Token;

typedef struct _Elm_Code_Line
{
   const char *content;
   int length;
   unsigned int number;
   char *modified;

   Elm_Code_Status_Type status;
   Eina_List *tokens;

   void *data;
} Elm_Code_Line;

/**
 * @brief Line handling functions.
 * @defgroup Line and content manipulation 
 *
 * @{
 *
 * Functions for handling content and styling of lines within elm code.
 *
 */

EAPI const char *elm_code_line_content_get(Elm_Code_Line *line, int *length);

EAPI void elm_code_line_status_set(Elm_Code_Line *line, Elm_Code_Status_Type status);

EAPI void elm_code_line_token_add(Elm_Code_Line *line, int start, int end, Elm_Code_Token_Type type);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* ELM_CODE_LINE_H_ */
