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
   Eina_Bool continues;

   Elm_Code_Token_Type type;

} Elm_Code_Token;

typedef struct _Elm_Code_Line
{
   Elm_Code_File *file;

   const char *content;
   unsigned int length;
   unsigned int number;
   char *modified;

   Elm_Code_Status_Type status;
   Eina_List *tokens;

   void *data;
   char *status_text;
} Elm_Code_Line;

EAPI void elm_code_line_free(Elm_Code_Line *line);

/**
 * @brief Line manipulation functions.
 * @defgroup Content
 * @{
 *
 * Functions for changing the content of lines in an Elm_Code_File
 */

EAPI void elm_code_line_split_at(Elm_Code_Line *line, unsigned int position);

/**
 * @}
 *
 * @brief Line markup functions.
 * @defgroup Highlighting
 *
 * @{
 *
 * Functions for handling styling and marking up lines within elm code.
 *
 */

EAPI void elm_code_line_status_set(Elm_Code_Line *line, Elm_Code_Status_Type status);

EAPI void elm_code_line_status_text_set(Elm_Code_Line *line, const char *text);

EAPI void elm_code_line_status_clear(Elm_Code_Line *line);

EAPI void elm_code_line_token_add(Elm_Code_Line *line, int start, int end, int lines, Elm_Code_Token_Type type);

EAPI void elm_code_line_tokens_clear(Elm_Code_Line *line);

EAPI Eina_Bool elm_code_line_contains_widget_cursor(Elm_Code_Line *line);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* ELM_CODE_LINE_H_ */
