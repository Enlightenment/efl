#ifndef EINA_FILE_H_
#define EINA_FILE_H_

/**
 * @defgroup File_Group Memory File
 * @{
 */

typedef void (*Eina_File_Dir_List_Cb)(const char *name, const char *path, void *data);

EAPI void eina_file_dir_list(const char *dir, int recursive, Eina_File_Dir_List_Cb cb, void *data);
EAPI void eina_file_path_nth_get(const char *path, int n, char **left, char **right);

/** @} */

#endif /*EINA_FILE_H_*/
