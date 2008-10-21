#ifndef _EVAS_PATH_H
#define _EVAS_PATH_H


int        evas_file_path_is_full_path (const char *path);
char      *evas_file_path_join         (const char *path, const char *end);
int        evas_file_path_exists       (const char *path);
int        evas_file_path_is_file      (const char *path);
int        evas_file_path_is_dir       (const char *path);
Eina_List *evas_file_path_list         (char *path, const char *match, int match_case);
DATA64     evas_file_modified_time     (const char *file);
char      *evas_file_path_resolve      (const char *file);


#endif /* _EVAS_PATH_H */
