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

static Elm_Code_Line *_elm_code_blank_create(int line)
{
   Elm_Code_Line *ecl;

   ecl = calloc(1, sizeof(Elm_Code_Line));
   if (!ecl) return NULL;

   ecl->number = line;
   return ecl;
}

EAPI Elm_Code_File *elm_code_open(const char *path)
{
   Elm_Code_File *ret;
   Eina_File *file;
   Eina_File_Line *line;
   Eina_Iterator *it;
   unsigned int lastindex;

   file = eina_file_open(path, EINA_FALSE);
   ret = calloc(1, sizeof(Elm_Code_File));
   ret->file = file;
   lastindex = 1;

   it = eina_file_map_lines(file);
   EINA_ITERATOR_FOREACH(it, line)
     {
        Elm_Code_Line *ecl;

        /* Working around the issue that eina_file_map_lines does not trigger an item for empty lines */
        while (lastindex < line->index - 1)
          {
             ecl = _elm_code_blank_create(++lastindex);
             if (!ecl) continue;

             ret->lines = eina_list_append(ret->lines, ecl);
          }

        ecl = _elm_code_blank_create(lastindex = line->index);
        if (!ecl) continue;

        ecl->content = malloc(sizeof(char) * (line->length + 1));
        strncpy(ecl->content, line->start, line->length);
        ecl->content[line->length] = 0;

        ret->lines = eina_list_append(ret->lines, ecl);
     }
   eina_iterator_free(it);

   return ret;
}

EAPI void elm_code_close(Elm_Code_File *file)
{
   Elm_Code_Line *l;

   EINA_LIST_FREE(file->lines, l)
     {
        if (l->content)
          free(l->content);
        free(l);
     }

   eina_file_close(file->file);
   free(file);
}

EAPI const char *elm_code_filename_get(Elm_Code_File *file)
{
   return basename((char *)eina_file_filename_get(file->file));
}

EAPI const char *elm_code_path_get(Elm_Code_File *file)
{
   return eina_file_filename_get(file->file);
}

EAPI unsigned int elm_code_lines_get(Elm_Code_File *file)
{
   return eina_list_count(file->lines);
}

EAPI char *elm_code_line_content_get(Elm_Code_File *file, int number)
{
   Elm_Code_Line *line;

   line = eina_list_nth(file->lines, number);
   return line->content;
}
