#ifndef ELM_CODE_FILE_H_
# define ELM_CODE_FILE_H_

#include <Eina.h>

#include "elm_code_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file
 * @brief These routines are used for interacting with files using Elm Code.
 */

typedef struct _Elm_Code_Line
{
   char *content;
   unsigned int number;

   Elm_Code_Status_Type status;

} Elm_Code_Line;

typedef struct _Elm_Code_File
{
   Eina_List *lines;
   Eina_File *file;

} Elm_Code_File;

/**
 * @brief File handling functions.
 * @defgroup File  I/O at a file level
 *
 * @{
 *
 * Functions for file handling within elm code.
 *
 */

EAPI Elm_Code_File *elm_code_file_new();

EAPI Elm_Code_File *elm_code_file_open(const char *path);

EAPI void elm_code_file_free(Elm_Code_File *file);

EAPI void elm_code_file_close(Elm_Code_File *file);

EAPI const char *elm_code_file_filename_get(Elm_Code_File *file);

EAPI const char *elm_code_file_path_get(Elm_Code_File *file);

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

EAPI unsigned int elm_code_file_lines_get(Elm_Code_File *file);

EAPI void elm_code_file_line_append(Elm_Code_File *file, const char *line);

EAPI Elm_Code_Line *elm_code_file_line_get(Elm_Code_File *file, unsigned int line);

EAPI char *elm_code_file_line_content_get(Elm_Code_File *file, unsigned int line);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* ELM_CODE_FILE_H_ */
