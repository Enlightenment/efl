#ifndef ELM_CODE_FILE_H_
# define ELM_CODE_FILE_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file
 * @brief These routines are used for interacting with files using Elm Code.
 */

typedef enum {
   ELM_CODE_FILE_LINE_ENDING_UNIX = 0,
   ELM_CODE_FILE_LINE_ENDING_WINDOWS
} Elm_Code_File_Line_Ending;

struct _Elm_Code_File
{
   void *parent;

   Eina_List *lines;
   Eina_File *file;
   void *map;
   const char *mime;

   Elm_Code_File_Line_Ending line_ending;
};

/**
 * @brief File handling functions.
 * @defgroup File  I/O at a file level
 *
 * @{
 *
 * Functions for file handling within elm code.
 *
 */

EAPI Elm_Code_File *elm_code_file_new(Elm_Code *code);

EAPI Elm_Code_File *elm_code_file_open(Elm_Code *code, const char *path);

EAPI void elm_code_file_save(Elm_Code_File *file);

EAPI void elm_code_file_free(Elm_Code_File *file);

EAPI void elm_code_file_close(Elm_Code_File *file);

/**
 * Get the filename for the file specified.
 *
 * @return the filename or NULL if it is an in-memory file
 */
EAPI const char *elm_code_file_filename_get(Elm_Code_File *file);

/**
 * Get the file path for the file specified.
 *
 * @return the file's path or NULL if it is an in-memory file
 */
EAPI const char *elm_code_file_path_get(Elm_Code_File *file);

EAPI Elm_Code_File_Line_Ending elm_code_file_line_ending_get(Elm_Code_File *file);

EAPI const char *elm_code_file_line_ending_chars_get(Elm_Code_File *file, short *length);

/**
 * @}
 *
 * @brief Content functions.
 * @defgroup Content  Functions for accessing file content
 *
 * @{
 *
 * File content handling functions.
 *
 */

EAPI void elm_code_file_clear(Elm_Code_File *file);

EAPI unsigned int elm_code_file_lines_get(Elm_Code_File *file);

EAPI void elm_code_file_line_append(Elm_Code_File *file, const char *line, int length, void *data);

EAPI void elm_code_file_line_insert(Elm_Code_File *file, unsigned int row, const char *line, int length, void *data);

EAPI void elm_code_file_line_remove(Elm_Code_File *file, unsigned int row);

EAPI Elm_Code_Line *elm_code_file_line_get(Elm_Code_File *file, unsigned int line);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* ELM_CODE_FILE_H_ */
