#include <getopt.h>

#include <Eina.h>

#include "Eolian.h"
#include "legacy_generator.h"
#include "eo1_generator.h"
#include "common_funcs.h"

#define EO_SUFFIX ".eo"

static int eo_version = 0;
static Eina_Bool legacy_support = EINA_FALSE;

static char*
_include_guard_enclose(const char *fname, const char *fbody)
{
   char incname[255];
   memset(incname, 0, sizeof(incname));
   strncpy (incname, fname, sizeof(incname) - 1);
   char *p = incname;
   eina_str_toupper(&p);

   Eina_Strbuf *incguard = eina_strbuf_new();
   eina_strbuf_append_printf(incguard,
         "#ifndef _%s_\n#define _%s_\n\n_code_\n#endif",
         incname,
         incname);
   eina_strbuf_replace_all(incguard, ".", "_");
   eina_strbuf_replace_all(incguard, "_code_", fbody);
   char *ret = eina_strbuf_string_steal(incguard);
   eina_strbuf_free(incguard);
   return ret;
}

static const char *
_filename_get(const char *path)
{
   char *result = NULL;

   if (!path) return NULL;
   if ((result = strrchr(path, '/'))) result++;
   else result = (char *)path;
   return result;
}

static Eina_Bool
_generate_eo_h_file(char *filename, const char *classname)
{
   Eina_Bool ret = EINA_FALSE;
   Eina_Strbuf *hfile = eina_strbuf_new();
   if (!eo_header_generate(classname, hfile))
     {
        ERR("Failed to generate header for %s", classname);
        goto end;
     }

   const char *htext = eina_strbuf_string_get(hfile);

   FILE* fd = fopen(filename, "wb");
   if (!fd)
     {
        const char *err = strerror(errno);
        ERR ("Couldn't open file %s for writing. Reason: '%s'", filename, err);
        goto end;
     }

   if (htext)
     {
        char *fcontent = _include_guard_enclose(_filename_get(filename), htext);
        fputs(fcontent, fd);
        free(fcontent);
     }

   fclose(fd);

   ret = EINA_TRUE;
end:
   eina_strbuf_free(hfile);

   return ret;
}

static Eina_Bool
_generate_c_file(char *filename, const char *classname)
{
   Eina_Bool ret = EINA_FALSE;

   Eina_Strbuf *eo_buf = eina_strbuf_new();
   Eina_Strbuf *legacy_buf = eina_strbuf_new();

   if (!eo_source_generate(classname, eo_version, eo_buf))
     {
        ERR("Failed to generate source for %s", classname);
        goto end;
     }

   if (legacy_support)
      if (!legacy_source_generate(classname, eo_version, legacy_buf))
        {
           ERR("Failed to generate source for %s", classname);
           goto end;
        }

   FILE* fd = fopen(filename, "wb");
   if (!fd)
     {
        ERR("Couldnt open file %s for writing", filename);
        goto end;
     }

   const char *text = eina_strbuf_string_get(eo_buf);
   if (text) fputs(text, fd);
   text = eina_strbuf_string_get(legacy_buf);
   if (text) fputs(text, fd);

   fclose(fd);

   ret = EINA_TRUE;
end:
   eina_strbuf_free(legacy_buf);
   eina_strbuf_free(eo_buf);
   return ret;
}

// TODO join with header gen.
static Eina_Bool
_generate_legacy_header_file(char *filename, const char *classname)
{
   Eina_Bool ret = EINA_FALSE;

   Eina_Strbuf *lfile = eina_strbuf_new();

   if (!legacy_header_generate(classname, eo_version, lfile))
     {
        ERR("Failed to generate header for %s", classname);
        goto end;
     }

   FILE* fd = fopen(filename, "wb");
   if (!fd)
     {
        ERR ("Couldnt open file %s for writing", filename);
        goto end;
     }

   const char *ltext = eina_strbuf_string_get(lfile);

   if (ltext)
     {
        char *fcontent = _include_guard_enclose(_filename_get(filename), ltext);
        fputs(fcontent, fd);
        free(fcontent);
     }

   fclose(fd);

   ret = EINA_TRUE;
end:
   eina_strbuf_free(lfile);
   return ret;
}

enum
{
   NO_WAY_GEN,
   H_GEN,
   C_GEN
};
int gen_opt = NO_WAY_GEN;

int main(int argc, char **argv)
{
   int ret = 1;
   Eina_Bool help = EINA_FALSE, show = EINA_FALSE;
   Eina_List *included_files = NULL, *itr;
   Eina_List *files4gen = NULL;
   const char *classname;
   char *output_filename = NULL; /* if NULL, have to generate, otherwise use the name stored there */

   eina_init();
   eolian_init();

   const char *log_dom = "eolian_gen";
   _eolian_gen_log_dom = eina_log_domain_register(log_dom, EINA_COLOR_GREEN);
   if (_eolian_gen_log_dom < 0)
     {
        EINA_LOG_ERR("Could not register log domain: %s", log_dom);
        goto end;
     }

   eina_log_timing(_eolian_gen_log_dom,
                   EINA_LOG_STATE_STOP,
                   EINA_LOG_STATE_INIT);

   static struct option long_options[] =
     {
        /* These options set a flag. */
          {"eo1",        no_argument,         &eo_version, 1},
          {"eo",        no_argument,         &eo_version, 2},
          {"verbose",    no_argument,         0, 'v'},
          {"help",       no_argument,         0, 'h'},
          {"gh",         no_argument,         &gen_opt, H_GEN},
          {"gc",         no_argument,         &gen_opt, C_GEN},
          {"output",     required_argument,   0, 'o'},
          {"legacy",     no_argument,         (int *)&legacy_support, EINA_TRUE},
          {"include",    required_argument,   0, 'I'},
          {0, 0, 0, 0}
     };
   int long_index =0, opt;
   while ((opt = getopt_long(argc, argv,"vho:I:", long_options, &long_index )) != -1)
     {
        switch (opt) {
           case 0: break;
           case 'o':
                   {
                      output_filename = strdup(optarg);
                      break;
                   }
           case 'v': show = EINA_TRUE; break;
           case 'h': help = EINA_TRUE; break;
           case 'I':
                     {
                        const char *dir = optarg;
                        Eina_Iterator *dir_files;
                        char *file;
                        /* Get all files from directory. Not recursively!!! */
                        dir_files = eina_file_ls(dir);
                        EINA_ITERATOR_FOREACH(dir_files, file)
                          {
                             if (eina_str_has_suffix(file, EO_SUFFIX))
                               {
                                  /* Allocated string will be freed during deletion of "included_files" list. */
                                  included_files = eina_list_append(included_files, strdup(file));
                               }
                          }
                        eina_iterator_free(dir_files);
                        break;
                     }
           default: help = EINA_TRUE;
        }
     }
   while (optind < argc)
      files4gen = eina_list_append(files4gen, argv[optind++]);

   if (help)
     {
        printf("Usage: %s [-h/--help] [-v/--verbose] [-I/--include input_dir] [--legacy] [--gh|--gc|--ah] [--output/-o outfile] file.eo ... \n", argv[0]);
        printf("       --help/-h Print that help\n");
        printf("       --include/-I Include 'input_dir' as directory to search .eo files into\n");
        printf("       --output/-o Force output filename to 'outfile'\n");
        printf("       --eo Set generator to eo mode. Must be specified\n");
        printf("       --gh Generate c header file [.h]\n");
        printf("       --gc Generate c source file [.c]\n");
        printf("       --ah Append eo class definitions to an existing c header file [.h]\n");
        printf("       --al Append legacy function definitions to an existing c header file [.h]\n");
        printf("       --legacy Generate legacy\n");
        ret = 0;
        goto end;
     }

   if (!files4gen)
     {
        ERR("No input files specified.\nTerminating.\n");
        goto end;
     }

   const char *filename;
   EINA_LIST_FOREACH(included_files, itr, filename)
     {
        if (!eolian_eo_file_parse(filename))
          {
             ERR("Error during parsing file %s\n", filename);
             goto end;
          }
     }

   EINA_LIST_FOREACH(files4gen, itr, filename)
     {
        if (!eolian_eo_file_parse(filename))
          {
             ERR("Error during parsing file %s\n", filename);
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

   if (!eo_version && !(gen_opt==H_GEN && legacy_support))
     {
        ERR("No eo version specified (use --eo). Aborting eo generation.\n");
        goto end;
     }

   classname = eolian_class_find_by_file(eina_list_data_get(files4gen));

   if (gen_opt)
     {
        if (!output_filename)
          {
             output_filename = malloc(strlen(eina_list_data_get(files4gen)) + 5);
             strcpy(output_filename, eina_list_data_get(files4gen));
             if (C_GEN == gen_opt) strcat(output_filename, ".c");
             else strcat(output_filename, ".h");
          }
        switch (gen_opt)
          {
           case H_GEN:
                {
                   INF("Generating header file %s\n", output_filename);
                   if (legacy_support)
                     ret = ( _generate_legacy_header_file(output_filename, classname) ? 0 : 1 );
                   else
                     ret = ( _generate_eo_h_file(output_filename, classname) ? 0 : 1 );
                   break;
                }
           case C_GEN:
                {
                   INF("Generating source file %s\n", output_filename);
                   ret = _generate_c_file(output_filename, classname)?0:1;
                   break;
                }
           default:
              ERR("Bad generation option\n");
              break;
          }
        free(output_filename);
     }
   else ret = 0;

end:
   EINA_LIST_FREE(included_files, filename)
      free((char *)filename);
   eina_list_free(files4gen);

   eina_log_timing(_eolian_gen_log_dom,
         EINA_LOG_STATE_START,
         EINA_LOG_STATE_SHUTDOWN);
   eina_log_domain_unregister(_eolian_gen_log_dom);
   _eolian_gen_log_dom = -1;

   eolian_shutdown();
   eina_shutdown();
   return ret;
}
