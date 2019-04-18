#ifndef _EFL_FILE_H
# define _EFL_FILE_H

/* add doc note about needing ref/unref when passing efl_part to these functions */
EAPI Eina_Bool efl_file_simple_load(Eo *obj, const char *file, const char *key);
EAPI Eina_Bool efl_file_simple_mmap_load(Eo *obj, const Eina_File *file, const char *key);
EAPI void efl_file_simple_get(const Eo *obj, const char **file, const char **key);
EAPI void efl_file_simple_mmap_get(const Eo *obj, const Eina_File **file, const char **key);

#endif
