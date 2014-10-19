#ifdef HAVE_CONFIG
# include "config.h"
#endif

#include "Elm_Code.h"

#include "elm_code_private.h"

static int _elm_code_init = 0;
int _elm_code_lib_log_dom = -1;

EAPI int
elm_code_init(void)
{
   _elm_code_init++;
   if (_elm_code_init > 1) return _elm_code_init;

   eina_init();

   _elm_code_lib_log_dom = eina_log_domain_register("elm_code", EINA_COLOR_CYAN);
   if (_elm_code_lib_log_dom < 0)
     {
        EINA_LOG_ERR("Elm Code can not create its log domain.");
        goto shutdown_eina;
     }

   // Put here your initialization logic of your library

   eina_log_timing(_elm_code_lib_log_dom, EINA_LOG_STATE_STOP, EINA_LOG_STATE_INIT);

   return _elm_code_init;

   shutdown_eina:
   eina_shutdown();
   _elm_code_init--;

   return _elm_code_init;
}

EAPI int
elm_code_shutdown(void)
{
   _elm_code_init--;
   if (_elm_code_init != 0) return _elm_code_init;

   eina_log_timing(_elm_code_lib_log_dom,
                   EINA_LOG_STATE_START,
                   EINA_LOG_STATE_SHUTDOWN);

   // Put here your shutdown logic

   eina_log_domain_unregister(_elm_code_lib_log_dom);
   _elm_code_lib_log_dom = -1;

   eina_shutdown();

   return _elm_code_init;
}

EAPI Elm_Code_File *elm_code_open(const char *path)
{
   Elm_Code_File *ret;
   Eina_File *file;
   Eina_File_Line *line;
   Eina_Iterator *it;
   void *map;

   file = eina_file_open(path, EINA_FALSE);
   map = eina_file_map_all(file, EINA_FILE_WILLNEED);
   ret = calloc(1, sizeof(Elm_Code_File));
   ret->file = file;
   ret->map = map;

   it = eina_file_map_lines(file);
   EINA_ITERATOR_FOREACH(it, line)
     {
        Elm_Code_Line *ecl;

        ecl = calloc(1, sizeof(Elm_Code_Line));
        if (!ecl) continue;

        ecl->content = *line;
        ret->lines = eina_list_append(ret->lines, ecl);
     }
   eina_iterator_free(it);

   return ret;
}

EAPI void elm_code_close(Elm_Code_File *file)
{
   Elm_Code_Line *l;

   EINA_LIST_FREE(file->lines, l)
     free(l);

   eina_file_map_free(file->file, file->map);
   eina_file_close(file->file);
   free(file);
}

EAPI const char *elm_code_filename_get(Elm_Code_File *file)
{
   return basename(eina_file_filename_get(file->file));
}

EAPI const char *elm_code_path_get(Elm_Code_File *file)
{
   return eina_file_filename_get(file->file);
}

EAPI int elm_code_lines_get(Elm_Code_File *file)
{
   return eina_list_count(file->lines);
}
