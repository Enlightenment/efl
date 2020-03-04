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
   unsigned int scope;

   void *data;
   char *status_text;
} Elm_Code_Line;

EAPI void elm_code_line_free(Elm_Code_Line *line);

/**
 * @brief Line manipulation functions.
 * @defgroup Elm_Code_Line_Content Elementary Code Line
 * @{
 *
 * Functions for changing the content of lines in an Elm_Code_File
 */

/**
 * Split the given line into two at the specified character position.
 * The additional line will be inserted into the file immediately below the specified line.
 *
 * @param line The line to split
 * @param position The character position to split at
 *
 * @ingroup Content
 */
EAPI void elm_code_line_split_at(Elm_Code_Line *line, unsigned int position);

/**
 * Merge the specified line with the line above.
 * The content of the specified line will be added to the end of the previous line.
 * The specified line will then be removed from the file.
 *
 * If there is no previous line this method does nothing.
 *
 * @param line The line to merge with the previous line.
 *
 * @ingroup Content
 */
EAPI void elm_code_line_merge_up(Elm_Code_Line *line);

/**
 * Merge the specified line with the line below.
 * The content of the specified line will have the contents of the next line added to the end.
 * The next line will then be removed from the file.
 *
 * If there is no next line this method does nothing.
 *
 * @param line The line to merge with the next line.
 *
 * @ingroup Content
 */
EAPI void elm_code_line_merge_down(Elm_Code_Line *line);

/**
 * @}
 *
 * @brief Line markup functions.
 * @defgroup Highlighting Elementary Code Highlighting
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

EAPI unsigned int elm_code_line_scope_get(Elm_Code_Line *line);

EAPI Eina_Bool elm_code_line_contains_widget_cursor(Elm_Code_Line *line);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* ELM_CODE_LINE_H_ */
