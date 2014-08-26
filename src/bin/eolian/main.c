#include <getopt.h>
#include <libgen.h>

#include <Eina.h>

#include "Eolian.h"
#include "legacy_generator.h"
#include "eo_generator.h"
#include "impl_generator.h"
#include "types_generator.h"
#include "common_funcs.h"

static Eina_Strbuf *
_include_guard_enclose(const char *fname, const char *suffix, Eina_Strbuf *fbody)
{
   char incname[255];
   if (!fbody || !eina_strbuf_string_get(fbody)) return fbody;
   memset(incname, 0, sizeof(incname));
   strncpy (incname, fname, sizeof(incname) - 1);
   char *p = incname;
   eina_str_toupper(&p);

   Eina_Strbuf *incguard = eina_strbuf_new();
   eina_strbuf_append_printf(incguard,
         "#ifndef _%s_%s\n#define _%s_%s\n\n",
         incname, suffix?suffix:"",
         incname, suffix?suffix:"");
   eina_strbuf_replace_all(incguard, ".", "_");
   eina_strbuf_append(incguard, eina_strbuf_string_get(fbody));
   eina_strbuf_append(incguard, "\n#endif\n");
   eina_strbuf_free(fbody);
   return incguard;
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
_read_file(char *filename, Eina_Strbuf *buffer)
{
   Eina_Bool ret = EINA_FALSE;
   long file_size = 0;
   eina_strbuf_reset(buffer);

   FILE *fd = fopen(filename, "rb");
   if (fd)
     {
        fseek(fd, 0, SEEK_END);
        file_size = ftell(fd);
        if (file_size <= 0)
          {
             ERR("Couldnt determine length for file %s", filename);
             goto end;
          }
        fseek(fd, 0, SEEK_SET);
        char *content = malloc(file_size + 1);
        if (!content)
          {
             ERR("Couldnt allocate memory for file %s", filename);
             goto end;
          }
        long actual_size = (long)fread(content, 1, file_size, fd);
        if (actual_size != file_size)
          {
             ERR("Couldnt read the %ld bytes of file %s (read %ld bytes)",
                 file_size, filename, actual_size);
             free(content);
             goto end;
          }

        content[file_size] = '\0';

        eina_strbuf_append(buffer, content);
        free(content);
#ifdef _WIN32
        eina_strbuf_replace_all(buffer, "\r\n", "\n");
#endif
     }

   ret = EINA_TRUE;
end:
   if (fd) fclose(fd);
   return ret;
}

static Eina_Bool
_write_file(char *filename, const Eina_Strbuf *buffer, Eina_Bool append)
{
   const char *data = eina_strbuf_string_get(buffer);

   FILE* fd = fopen(filename, append ? "ab" : "wb");
   if (!fd)
     {
        const char *err = strerror(errno);
        ERR ("Couldn't open file %s for writing. Reason: '%s'", filename, err);
        return EINA_FALSE;
     }

   if (data) fputs(data, fd);
   fclose(fd);
   return EINA_TRUE;
}

static Eina_Bool
_generate_eo_header_file(char *filename, const char *eo_filename)
{
   Eina_Bool ret = EINA_FALSE;

   Eina_Strbuf *buffer = eina_strbuf_new();

   if (!types_header_generate(eo_filename, buffer, EINA_TRUE))
     {
        ERR("Failed to generate types of file %s", eo_filename);
        goto end;
     }
   else
     {
        buffer = _include_guard_enclose(eo_filename, "TYPES", buffer);
     }

   Eina_Strbuf *ctbuf = eina_strbuf_new();
   if (types_class_typedef_generate(eo_filename, ctbuf))
     {
        ctbuf = _include_guard_enclose(eo_filename, "CLASS_TYPE", ctbuf);
        eina_strbuf_append_char(ctbuf, '\n');
        eina_strbuf_prepend(buffer, eina_strbuf_string_get(ctbuf));
     }
   eina_strbuf_free(ctbuf);

   const Eolian_Class *class = eolian_class_get_by_file(eo_filename);
   if (class)
     {
        if (!eo_header_generate(class, buffer))
          {
             ERR("Failed to generate header for %s", eolian_class_name_get(class));
             goto end;
          }
     }

   buffer = _include_guard_enclose(_filename_get(filename), NULL, buffer);
   if (_write_file(filename, buffer, EINA_FALSE))
      ret = EINA_TRUE;
end:
   eina_strbuf_free(buffer);

   return ret;
}

static Eina_Bool
_generate_stub_header_file(char *filename, const char *eo_filename)
{
   Eina_Bool ret = EINA_FALSE;

   Eina_Strbuf *buffer = eina_strbuf_new();

   if (!types_header_generate(eo_filename, buffer, EINA_FALSE))
     {
        ERR("Failed to generate types of file %s", eo_filename);
        goto end;
     }

   Eina_Strbuf *ctbuf = eina_strbuf_new();
   if (types_class_typedef_generate(eo_filename, ctbuf))
     {
        eina_strbuf_append_char(ctbuf, '\n');
        eina_strbuf_prepend(buffer, eina_strbuf_string_get(ctbuf));
     }
   eina_strbuf_free(ctbuf);

   buffer = _include_guard_enclose(_filename_get(filename), "STUBS", buffer);
   if (_write_file(filename, buffer, EINA_FALSE))
      ret = EINA_TRUE;
end:
   eina_strbuf_free(buffer);

   return ret;
}

static Eina_Bool
_generate_c_file(char *filename, const char *eo_filename, Eina_Bool legacy_support)
{
   Eina_Bool ret = EINA_FALSE;

   Eina_Strbuf *eo_buf = eina_strbuf_new();
   Eina_Strbuf *legacy_buf = eina_strbuf_new();

   const Eolian_Class *class = eolian_class_get_by_file(eo_filename);
   if (class)
     {
        if (!eo_source_generate(class, eo_buf))
          {
             ERR("Failed to generate source for %s", eolian_class_name_get(class));
             goto end;
          }

        if (legacy_support)
           if (!legacy_source_generate(class, legacy_buf))
             {
                ERR("Failed to generate source for %s", eolian_class_name_get(class));
                goto end;
             }
     }

   if (_write_file(filename, eo_buf, EINA_FALSE) &&
         _write_file(filename, legacy_buf, EINA_TRUE))
      ret = EINA_TRUE;
end:
   eina_strbuf_free(legacy_buf);
   eina_strbuf_free(eo_buf);
   return ret;
}

static Eina_Bool
_generate_impl_c_file(char *filename, const char *eo_filename)
{
   Eina_Bool ret = EINA_FALSE;
   Eina_Strbuf *buffer = eina_strbuf_new();

   const Eolian_Class *class = eolian_class_get_by_file(eo_filename);
   if (class)
     {
        if (!_read_file(filename, buffer)) goto end;

        if (!impl_source_generate(class, buffer))
          {
             ERR("Failed to generate source for %s", eolian_class_name_get(class));
             goto end;
          }

        if (_write_file(filename, buffer, EINA_FALSE))
           ret = EINA_TRUE;
     }
end:
   eina_strbuf_free(buffer);
   return ret;
}

// TODO join with header gen.
static Eina_Bool
_generate_legacy_header_file(char *filename, const char *eo_filename)
{
   Eina_Bool ret = EINA_FALSE;

   Eina_Strbuf *buffer = eina_strbuf_new();

   if (!types_header_generate(eo_filename, buffer, EINA_TRUE))
     {
        ERR("Failed to generate types of file %s", eo_filename);
        goto end;
     }
   else
     {
        buffer = _include_guard_enclose(eo_filename, "TYPES", buffer);
     }

   Eina_Strbuf *ctbuf = eina_strbuf_new();
   if (types_class_typedef_generate(eo_filename, ctbuf))
     {
        ctbuf = _include_guard_enclose(eo_filename, "CLASS_TYPE", ctbuf);
        eina_strbuf_append_char(ctbuf, '\n');
        eina_strbuf_prepend(buffer, eina_strbuf_string_get(ctbuf));
     }
   eina_strbuf_free(ctbuf);

   const Eolian_Class *class = eolian_class_get_by_file(eo_filename);
   if (class)
     {
        if (!legacy_header_generate(class, buffer))
          {
             ERR("Failed to generate header for %s", eolian_class_name_get(class));
             goto end;
          }

        buffer = _include_guard_enclose(_filename_get(filename), NULL, buffer);
        if (_write_file(filename, buffer, EINA_FALSE))
           ret = EINA_TRUE;
     }
end:
   eina_strbuf_free(buffer);
   return ret;
}

enum
{
   NO_WAY_GEN,
   H_GEN,
   H_STUB_GEN,
   C_GEN,
   C_IMPL_GEN
};
static int gen_opt = NO_WAY_GEN;
static int eo_needed = 0;
static int legacy_support = 0;

int main(int argc, char **argv)
{
   int ret = 1;
   Eina_Bool help = EINA_FALSE, show = EINA_FALSE;
   const char *eo_filename = NULL;
   char *output_filename = NULL; /* if NULL, have to generate, otherwise use the name stored there */
   char *eo_filename_copy = NULL, *eo_file_basename;

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
          {"eo",         no_argument,         &eo_needed, 1},
          {"verbose",    no_argument,         0, 'v'},
          {"help",       no_argument,         0, 'h'},
          {"gh",         no_argument,         &gen_opt, H_GEN},
          {"gc",         no_argument,         &gen_opt, C_GEN},
          {"gi",         no_argument,         &gen_opt, C_IMPL_GEN},
          {"gs",         no_argument,         &gen_opt, H_STUB_GEN},
          {"output",     required_argument,   0, 'o'},
          {"legacy",     no_argument,         &legacy_support, 1},
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
                        if (!eolian_directory_scan(dir))
                          {
                             ERR("Failed to scan %s", dir);
                             goto end;
                          }
                        break;
                     }
           default: help = EINA_TRUE;
        }
     }
   eo_filename = argv[optind++];

   if (help)
     {
        printf("Usage: %s [-h/--help] [-v/--verbose] [-I/--include input_dir] [--legacy] [--gh|--gc|--gi] [--output/-o outfile] file.eo ... \n", argv[0]);
        printf("       --help/-h Print that help\n");
        printf("       --include/-I Include 'input_dir' as directory to search .eo files into\n");
        printf("       --output/-o Force output filename to 'outfile'\n");
        printf("       --eo Set generator to eo mode. Must be specified\n");
        printf("       --gh Generate C header file [.h]\n");
        printf("       --gs Generate C type stubs [.h]\n");
        printf("       --gc Generate C source file [.c]\n");
        printf("       --gi Generate C implementation source file [.c]. The output will be a series of functions that have to be filled.\n");
        printf("       --legacy Generate legacy\n");
        ret = 0;
        goto end;
     }

   if (!eo_filename)
     {
        ERR("No input file specified.\nTerminating.\n");
        goto end;
     }

   eolian_all_eot_files_parse();

   if (!eolian_eo_file_parse(eo_filename))
     {
        ERR("Error during parsing file %s\n", eo_filename);
        goto end;
     }

   if (!eolian_database_validate())
     {
        ERR("Error validating database.\n");
        goto end;
     }

   eo_filename_copy = strdup(eo_filename);
   eo_file_basename = basename(eo_filename_copy);
   if (show)
     {
        const Eolian_Class *class = eolian_class_get_by_file(eo_file_basename);
        if (class) eolian_show_class(class);
     }

   if (!eo_needed && !(gen_opt == H_GEN && legacy_support))
     {
        ERR("Eo flag is not specified (use --eo). Aborting eo generation.\n");
        goto end;
     }

   if (gen_opt)
     {
        if (!output_filename)
          {
             ERR("You must use -o argument for files generation.");
             goto end;
          }
        switch (gen_opt)
          {
           case H_GEN:
                {
                   INF("Generating header file %s\n", output_filename);
                   if (legacy_support)
                     ret = ( _generate_legacy_header_file(output_filename, eo_file_basename) ? 0 : 1 );
                   else
                     ret = ( _generate_eo_header_file(output_filename, eo_file_basename) ? 0 : 1 );
                   break;
                }
           case H_STUB_GEN:
                {
                   INF("Generating stubs header file %s\n", output_filename);
                   ret = _generate_stub_header_file(output_filename, eo_file_basename) ? 0 : 1;
                   break;
                }
           case C_GEN:
                {
                   INF("Generating source file %s\n", output_filename);
                   ret = _generate_c_file(output_filename, eo_file_basename, !!legacy_support)?0:1;
                   break;
                }
           case C_IMPL_GEN:
                {
                   INF("Generating user source file %s\n", output_filename);
                   ret = _generate_impl_c_file(output_filename, eo_file_basename) ? 0 : 1;
                   break;
                }
           default:
              ERR("Bad generation option\n");
              break;
          }
     }
   else ret = 0;

end:
   free(eo_filename_copy);
   free(output_filename);

   eina_log_timing(_eolian_gen_log_dom,
         EINA_LOG_STATE_START,
         EINA_LOG_STATE_SHUTDOWN);
   eina_log_domain_unregister(_eolian_gen_log_dom);
   _eolian_gen_log_dom = -1;

   eolian_shutdown();
   eina_shutdown();
   return ret;
}
