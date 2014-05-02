
#ifndef EOLIAN_CXX_EOLIAN_HELPER_H
#define EOLIAN_CXX_EOLIAN_HELPER_H

#include <Eina.h>
#include <Eolian.h>
#include <assert.h>

#define EO_SUFFIX ".eo"

inline Eina_List*
_list_dir(const char *dir, const char *suffix, Eina_Bool recurse)
{
   Eina_List *files = NULL;
   Eina_Iterator *ls;
   Eina_File_Direct_Info *info;

   ls = eina_file_direct_ls(dir);
   if(ls == NULL) return NULL;

   EINA_ITERATOR_FOREACH (ls, info)
     {
        assert(info && info->path);
        if (info->type == EINA_FILE_DIR && recurse)
          {
             files = eina_list_merge
               (files, _list_dir(info->path, suffix, recurse));
          }
        else if (eina_str_has_suffix(&info->path[info->name_start], suffix))
          {
             files = eina_list_append(files, strdup(info->path));
          }
     }
   eina_iterator_free(ls);
   return eina_list_sort
     (files, eina_list_count(files), EINA_COMPARE_CB(strcoll));
}

inline Eina_List*
eolian_read_from_fs(const char *path, Eina_Bool recurse)
{
   if (eina_str_has_suffix(path, EO_SUFFIX))
     {
        if(!eolian_eo_file_parse(path))
          {
             /* XXX: fprintf? */
             fprintf(stderr, "Couldn't load input file: %s\n", path);
             return NULL;
          }
     }
   else
     {
        if (!eolian_directory_scan(path))
          {
             /* XXX: fprintf? */
             fprintf(stderr, "Error scanning directory: %s\n", path);
          }
     }
   return eina_list_clone(eolian_class_names_list_get());
}

#endif /* EOLIAN_CXX_EOLIAN_HELPER_H */
