#include <Eina.h>
#include <Ecore_File.h>

#include "Eolian.h"
#include "legacy_generator.h"
#include "eo1_generator.h"

#define EO_SUFFIX ".eo"

static int eo_version = 0;

static Eina_Bool
_generate_h_file(char *filename, char *classname, Eina_Bool append)
{
   Eina_Bool ret = EINA_FALSE;
   Eina_Strbuf *hfile = eina_strbuf_new();
   if (append)
     {
        Eina_File *fn = eina_file_open(filename, EINA_FALSE);
        if (!fn)
          {
            printf ("Cant open file \"%s\" for updating.\n", filename);
            goto end;
          }

        eina_strbuf_append(hfile, (char*)eina_file_map_all(fn, EINA_FILE_SEQUENTIAL));
        eina_file_close(fn);

        legacy_header_append(classname, eo_version, hfile);
     }
   else
     {
        eo1_header_generate(classname, hfile);
     }
   const char *htext = eina_strbuf_string_get(hfile);

   FILE* fd = fopen(filename, "w");
   if (!fd)
     {
        printf ("Couldn't open file %s for writing\n", filename);
        goto end;
     }
   if (htext) fputs(htext, fd);
   fclose(fd);

   ret = EINA_TRUE;
end:
   eina_strbuf_free(hfile);

   return ret;
}

static Eina_Bool
_generate_c_file(char *filename, char *classname, Eina_Bool append)
{
   Eina_Bool ret = EINA_FALSE;

   Eina_Strbuf *cfile = eina_strbuf_new();
   legacy_source_generate(classname, eo_version, cfile);

   FILE* fd = fopen(filename, (append) ? "a" : "w");
   if (!fd)
     {
        printf ("Couldnt open file %s for writing\n", filename);
        goto end;
     }
   const char *ctext = eina_strbuf_string_get(cfile);
   if (ctext) fputs(ctext, fd);
   fclose(fd);

   ret = EINA_TRUE;
end:
   eina_strbuf_free(cfile);
   return ret;
}

// TODO join with header gen.
static Eina_Bool
_generate_legacy_header_file(char *filename, char *classname, Eina_Bool append)
{
   Eina_Bool ret = EINA_FALSE;

   Eina_Strbuf *lfile = eina_strbuf_new();

   if (append)
     {
        Eina_File *fn = eina_file_open(filename, EINA_FALSE);
        if (!fn)
          {
            printf ("Cant open file \"%s\" for updating.\n", filename);
            goto end;
          }
        eina_strbuf_append(lfile, (char*)eina_file_map_all(fn, EINA_FILE_SEQUENTIAL));
        eina_file_close(fn);

        legacy_header_append(classname, eo_version, lfile);
     }
   else
     {
        legacy_header_generate(classname, eo_version, lfile);
     }

   FILE* fd = fopen(filename, "w");
   if (!fd)
     {
        printf ("Couldnt open file %s for writing\n", filename);
        goto end;
     }
   const char *ltext = eina_strbuf_string_get(lfile);
   if (ltext) fputs(ltext, fd);
   fclose(fd);

   ret = EINA_TRUE;
end:
   eina_strbuf_free(lfile);
   return ret;
}

static Eina_Bool
_generate_eo_and_legacy_h_file(char *filename, char *classname)
{
   Eina_Bool ret = EINA_FALSE;

   Eina_Strbuf *hfile = eina_strbuf_new();

   FILE* fd = fopen(filename, "w");
   if (!fd)
     {
        printf ("Couldnt open file %s for writing\n", filename);
        goto end;
     }

   eo1_header_generate(classname, hfile);
   legacy_header_generate(classname, eo_version, hfile);
   const char *htext = eina_strbuf_string_get(hfile);
   if (htext) fputs(htext, fd);

   fclose(fd);

   ret = EINA_TRUE;
end:
   eina_strbuf_free(hfile);
   return ret;
}

int main(int argc, char **argv)
{
   eina_init();
   int i, ret = 0;
   Eina_Bool help = EINA_FALSE, show = EINA_FALSE;
   Eina_List *files = NULL, *itr;
   Eina_List *classes = NULL;
   char *h_filename = NULL, *c_filename = NULL,
        *classname = NULL, *leg_filename = NULL,
        *eoleg_filename = NULL;

   Eina_Bool happend = EINA_FALSE;
   Eina_Bool lappend = EINA_FALSE;

   for(i = 1; i < argc; i++)
     {
        if (!strcmp(argv[i], "-eo1"))
          {
             eo_version = 1;
             continue;
          }
        if (!strcmp(argv[i], "-eo2"))
          {
             eo_version = 2;
             continue;
          }
        if (!strcmp(argv[i], "-gh") && (i < (argc-1)))
          {
             h_filename = argv[i + 1];
             continue;
          }
        if (!strcmp(argv[i], "-gc") && (i < (argc-1)))
          {
             c_filename = argv[i + 1];
             continue;
          }
        if (!strcmp(argv[i], "-gl") && (i < (argc-1)))
          {
             leg_filename = argv[i + 1];
             continue;
          }
        if (!strcmp(argv[i], "-gle") && (i < (argc-1)))
          {
             eoleg_filename = argv[i + 1];
             continue;
          }
        if (!strcmp(argv[i], "-ah") && (i < (argc-1)))
          {
             h_filename = argv[i + 1];
             happend = EINA_TRUE;
             continue;
          }
        if (!strcmp(argv[i], "-al") && (i < (argc-1)))
          {
             leg_filename = argv[i + 1];
             lappend = EINA_TRUE;
             continue;
          }
        if (!strcmp(argv[i], "--class") && (i < (argc-1)))
          {
             classes = eina_list_append(classes, argv[i + 1]);
             continue;
          }
        if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help"))
          {
             help = EINA_TRUE;
             continue;
          }
        if (!strcmp(argv[i], "--show"))
          {
             show = EINA_TRUE;
             continue;
          }
        /* Directory parameter found. */
        if ((!strcmp(argv[i], "-d") || !strcmp(argv[i], "--dir")) && (i < (argc-1)))
          {
             i++;
             char *dir = ecore_file_realpath(argv[i]);
             if (strlen(dir) != 0)
               {
                  if (ecore_file_is_dir(dir))
                    {
                       Eina_List *dir_files;
                       char *file;
                       /* Get all files from directory. Not recursively!!!*/
                       dir_files = ecore_file_ls(dir);
                       EINA_LIST_FREE(dir_files, file)
                         {
                            char *filepath = malloc(strlen(dir) + 1 + strlen(file) + 1);
                            sprintf(filepath, "%s/%s", dir, file);
                            if ((!ecore_file_is_dir(filepath)) && eina_str_has_suffix(filepath, EO_SUFFIX))
                              {
                                 /* Allocated string will be freed during deletion of "files" list. */
                                 files = eina_list_append(files, strdup(filepath));
                              }
                            free(filepath);
                            free(file);
                         }
                    }
               }
             free(dir);
             continue;
          }
        if ((!strcmp(argv[i], "-f") || !strcmp(argv[i], "--file")) && (i < (argc-1)))
          {
             i++;
             char *realpath = ecore_file_realpath(argv[i]);
             if (strlen(realpath) != 0)
               {
                  if (!ecore_file_is_dir(realpath))
                    {
                       if (eina_str_has_suffix(realpath, EO_SUFFIX))
                         files = eina_list_append(files, strdup(realpath));
                    }
               }
             free(realpath);
             continue;
          }
     }

   if (eina_list_count(classes)) classname = eina_list_data_get(classes);

   if (!files || help || !classname)
     {
        printf("Usage: %s [-h/--help] [--show] [-d/--dir input_dir] [-f/--file input_file] [-gh|-gc|-ah] filename [--class] classname \n", argv[0]);
        printf("       -eo1/-eo2 Set generator to eo1/eo2 mode. Must be specified\n");
        printf("       -gh Generate c header file [.h] for eo class specified by classname\n");
        printf("       -gc Generate c source file [.c] for eo class specified by classname\n");
        printf("       -ah Append eo class definitions to an existing c header file [.h]\n");
        printf("       -al Append legacy function definitions to an existing c header file [.h]\n");
        printf("       -gle Generate eo and legacy file [.h]\n");
        return 0;
     }

   eolian_init();
   const char *filename;
   EINA_LIST_FOREACH(files, itr, filename)
     {
        if (!eolian_eo_file_parse(filename))
          {
             printf("Error during parsing file %s\n", filename);
             goto end;
          }
     }

   if (show) eolian_show(classname);

   if (!eo_version)
     {
        printf("No eo version specified (use -eo1 or -eo2). Aborting eo generation.\n");
        ret = 1;
        goto end;
     }

   if (h_filename)
     {
        printf("%s header file %s\n", (happend) ? "Appending" : "Generating", h_filename);
        _generate_h_file(h_filename, classname, happend);
     }

   if (c_filename)
     {
        printf("Generating source file %s\n", c_filename);
        Eina_List *l = NULL;
        char *cname = NULL;
        EINA_LIST_FOREACH(classes,l,cname)
          _generate_c_file(c_filename, cname, (classes != l));
     }

   if (leg_filename)
     {
        printf("%s legacy file %s\n", (lappend) ? "Appending" : "Generating", leg_filename);
        _generate_legacy_header_file(leg_filename, classname, lappend);
     }

   if (eoleg_filename)
     {
        printf("Generating eo and legacy header file %s\n", eoleg_filename);
        _generate_eo_and_legacy_h_file(eoleg_filename, classname);
     }

end:
   EINA_LIST_FREE(files, filename)
      free((char *)filename);
   eina_list_free(classes);
   eolian_shutdown();
   eina_shutdown();
   return ret;
}
