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
   unsigned int end_line;

   Elm_Code_Token_Type type;

} Elm_Code_Token;

typedef struct _Elm_Code_Line
{
   Elm_Code_File *file;

   const char *content;
   unsigned int length, unicode_length;
   unsigned int number;
   char *modified;

   Elm_Code_Status_Type status;
   Eina_List *tokens;

   void *data;
   const char *status_text;
} Elm_Code_Line;

EAPI void elm_code_line_free(Elm_Code_Line *line);

EAPI unsigned int elm_code_line_utf8_length_get(Elm_Code_Line *line);

/**
 * @brief Line markup functions.
 * @defgroup Line highlighting and status manipulation
 *
 * @{
 *
 * Functions for handling styling and marking up lines within elm code.
 *
 */

EAPI void elm_code_line_status_set(Elm_Code_Line *line, Elm_Code_Status_Type status);

EAPI void elm_code_line_token_add(Elm_Code_Line *line, int start, int end, int lines, Elm_Code_Token_Type type);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* ELM_CODE_LINE_H_ */
