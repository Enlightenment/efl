#include <getopt.h>

#include <Eina.h>
#include <Ecore_File.h>

#include "Eolian.h"
#include "legacy_generator.h"
#include "eo1_generator.h"

#define EO_SUFFIX ".eo"

static int eo_version = 0;
static Eina_Bool legacy_support = EINA_FALSE;

static Eina_Bool
_generate_h_file(char *filename, const char *classname, Eina_Bool append)
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
_generate_c_file(char *filename, const char *classname, Eina_Bool append)
{
   Eina_Bool ret = EINA_FALSE;

   Eina_Strbuf *cfile = eina_strbuf_new();
   legacy_source_generate(classname, legacy_support, eo_version, cfile);

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
_generate_legacy_header_file(char *filename, const char *classname, Eina_Bool append)
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
_generate_eo_and_legacy_h_file(char *filename, const char *classname)
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
   int ret = 0;
   Eina_Bool help = EINA_FALSE, show = EINA_FALSE;
   Eina_List *included_files = NULL, *itr;
   Eina_List *files4gen = NULL;
   const char *classname;
   char *h_filename = NULL, *c_filename = NULL,
        *leg_filename = NULL, *eoleg_filename = NULL;

   Eina_Bool happend = EINA_FALSE;
   Eina_Bool lappend = EINA_FALSE;

   static struct option long_options[] =
     {
        /* These options set a flag. */
          {"eo1",        no_argument,         &eo_version, 1},
          {"eo2",        no_argument,         &eo_version, 2},
          {"verbose",    no_argument,         0, 'V'},
          {"help",       no_argument,         0, 'h'},
          {"gh",         required_argument,   0, 1},
          {"gc",         required_argument,   0, 2},
          {"ah",         required_argument,   0, 3},
          {"al",         required_argument,   0, 4},
          {"gle",        required_argument,   0, 5},
          {"legacy",     no_argument,         0, 6},
          {"include",    required_argument,   0, 'I'},
          {0, 0, 0, 0}
     };
   int long_index =0, opt;
   while ((opt = getopt_long(argc, argv,"Vho:I:", long_options, &long_index )) != -1)
     {
        switch (opt) {
           case 0: break;
           case 1: h_filename = optarg; break;
           case 2: c_filename = optarg; break;
           case 3: h_filename = optarg; happend = EINA_TRUE; break;
           case 4: leg_filename = optarg; lappend = EINA_TRUE; break;
           case 5: eoleg_filename = optarg; break;
           case 6: legacy_support = EINA_TRUE; break;
           case 'V': show = EINA_TRUE; break;
           case 'h': help = EINA_TRUE; break;
           case 'I':
                     {
                        printf("%s\n", optarg);
                        const char *dir = optarg;
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
                                       /* Allocated string will be freed during deletion of "included_files" list. */
                                       included_files = eina_list_append(included_files, strdup(filepath));
                                    }
                                  free(filepath);
                                  free(file);
                               }
                          }
                        break;
                     }
           default: help = EINA_TRUE;
        }
     }
   while (optind < argc)
      files4gen = eina_list_append(files4gen, argv[optind++]);

   if (!included_files || help || !files4gen)
     {
        printf("Usage: %s [-h/--help] [-V/--verbose] [-I/--include input_dir] [--legacy] [--gh|--gc|--ah filename] eo_file... \n", argv[0]);
        printf("       --eo1/--eo2 Set generator to eo1/eo2 mode. Must be specified\n");
        printf("       --gh Generate c header file [.h]\n");
        printf("       --gc Generate c source file [.c]\n");
        printf("       --ah Append eo class definitions to an existing c header file [.h]\n");
        printf("       --al Append legacy function definitions to an existing c header file [.h]\n");
        printf("       --gle Generate eo and legacy file [.h]\n");
        printf("       --legacy Generate legacy\n");
        return 0;
     }

   eolian_init();
   const char *filename;
   EINA_LIST_FOREACH(included_files, itr, filename)
     {
        if (!eolian_eo_file_parse(filename))
          {
             printf("Error during parsing file %s\n", filename);
             goto end;
          }
     }

   EINA_LIST_FOREACH(files4gen, itr, filename)
     {
        if (!eolian_eo_file_parse(filename))
          {
             printf("Error during parsing file %s\n", filename);
             goto end;
          }
     }

   if (show)
     {
        EINA_LIST_FOREACH(files4gen, itr, filename)
          {
             const char *cname = eolian_class_find_by_file(filename);
             if (cname) eolian_show(cname);
          }
     }

   if (!eo_version)
     {
        printf("No eo version specified (use --eo1 or --eo2). Aborting eo generation.\n");
        ret = 1;
        goto end;
     }

   classname = eolian_class_find_by_file(eina_list_data_get(files4gen));

   if (h_filename)
     {
        printf("%s header file %s\n", (happend) ? "Appending" : "Generating", h_filename);
        _generate_h_file(h_filename, classname, happend);
     }

   if (c_filename)
     {
        printf("Generating source file %s\n", c_filename);
        const char *cname;
        EINA_LIST_FOREACH(files4gen, itr, filename)
          {
             cname = eolian_class_find_by_file(filename);
             _generate_c_file(c_filename, cname, (files4gen != itr));
          }
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
   EINA_LIST_FREE(included_files, filename)
      free((char *)filename);
   eina_list_free(files4gen);
   eolian_shutdown();
   eina_shutdown();
   return ret;
}
