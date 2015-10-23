#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

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
_read_file(const char *filename, Eina_Strbuf **buf)
{
   FILE *fd = fopen(filename, "rb");
   if (!fd)
     {
        *buf = eina_strbuf_new();
        return EINA_TRUE;
     }

   fseek(fd, 0, SEEK_END);
   long file_size = ftell(fd);
   if (file_size <= 0)
     {
        fprintf(stderr, "eolian: could not get length of '%s'\n", filename);
        fclose(fd);
        return EINA_FALSE;
     }
   fseek(fd, 0, SEEK_SET);

   char *content = malloc(file_size + 1);
   if (!content)
     {
        fprintf(stderr, "eolian: could not allocate memory for '%s'\n", filename);
        fclose(fd);
        return EINA_FALSE;
     }

   long actual_size = (long)fread(content, 1, file_size, fd);
   if (actual_size != file_size)
     {
        fprintf(stderr, "eolian: could not read %ld bytes from '%s' (read %ld bytes)\n",
            file_size, filename, actual_size);
        free(content);
        fclose(fd);
        return EINA_FALSE;
     }

   content[file_size] = '\0';
   fclose(fd);
   *buf = eina_strbuf_manage_new_length(content, file_size);
   return EINA_TRUE;
}

static Eina_Bool
_write_file(const char *filename, const Eina_Strbuf *buffer, Eina_Bool append)
{
   FILE *fd = fopen(filename, append ? "ab" : "wb");
   if (!fd)
     {
        fprintf(stderr, "eolian: could not open '%s' for writing (%s)\n",
                filename, strerror(errno));
        return EINA_FALSE;
     }

   if (eina_strbuf_length_get(buffer))
     fwrite(eina_strbuf_string_get(buffer), 1, eina_strbuf_length_get(buffer), fd);
   fclose(fd);
   return EINA_TRUE;
}

static Eina_Bool
_generate_header_file(const char *filename, const char *eo_filename, Eina_Bool legacy)
{
   Eina_Bool ret = EINA_FALSE;

   Eina_Strbuf *buffer = eina_strbuf_new();

   if (!types_header_generate(eo_filename, buffer, EINA_TRUE, legacy))
     {
        fprintf(stderr, "eolian: could not generate types of '%s'\n", eo_filename);
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
        Eina_Bool gret = legacy ? legacy_header_generate(class, buffer)
                                : eo_header_generate(class, buffer);
        if (!gret)
          {
             fprintf(stderr, "eolian: could not generate header for '%s'\n",
                     eolian_class_name_get(class));
             goto end;
          }
     }

   if (class || !legacy)
     {
        buffer = _include_guard_enclose(_filename_get(filename), NULL, buffer);
        if (_write_file(filename, buffer, EINA_FALSE))
           ret = EINA_TRUE;
     }
end:
   eina_strbuf_free(buffer);

   return ret;
}

static Eina_Bool
_generate_stub_header_file(const char *filename, const char *eo_filename)
{
   Eina_Bool ret = EINA_FALSE;

   Eina_Strbuf *buffer = eina_strbuf_new();

   if (!types_header_generate(eo_filename, buffer, EINA_FALSE, EINA_FALSE))
     {
        fprintf(stderr, "eolian: could not generate types of '%s'\n", eo_filename);
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
_generate_c_file(const char *filename, const char *eo_filename, Eina_Bool legacy_support)
{
   Eina_Bool ret = EINA_FALSE;

   Eina_Strbuf *eo_buf = eina_strbuf_new();
   Eina_Strbuf *legacy_buf = eina_strbuf_new();

   const Eolian_Class *class = eolian_class_get_by_file(eo_filename);
   if (class)
     {
        if (!eo_source_generate(class, eo_buf))
          {
             fprintf(stderr, "eolian: could not generate source for '%s'\n",
                     eolian_class_name_get(class));
             goto end;
          }

        if (legacy_support)
           if (!legacy_source_generate(class, legacy_buf))
             {
                fprintf(stderr, "eolian: could not generate source for '%s'\n",
                        eolian_class_name_get(class));
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
_generate_impl_c_file(const char *filename, const char *eo_filename)
{
   const Eolian_Class *class = eolian_class_get_by_file(eo_filename);
   if (!class)
     return EINA_FALSE;

   Eina_Strbuf *buffer = NULL;
   if (!_read_file(filename, &buffer))
     return EINA_FALSE;

   if (!impl_source_generate(class, buffer))
     {
        fprintf(stderr, "eolian: could not generate source for '%s'\n",
                eolian_class_name_get(class));
        eina_strbuf_free(buffer);
        return EINA_FALSE;
     }

   Eina_Bool ret = _write_file(filename, buffer, EINA_FALSE);
   eina_strbuf_free(buffer);
   return ret;
}

enum
{
   GEN_NOTHING = 0,
   GEN_H,
   GEN_H_STUB,
   GEN_C,
   GEN_C_IMPL
};

int
main(int argc, char **argv)
{
   int gen_what = GEN_NOTHING, do_legacy = 0, ret = 1;
   Eina_Bool help = EINA_FALSE;
   const char *outf = NULL;

   eina_init();
   eolian_init();

   const char *dom = "eolian_gen";
   _eolian_gen_log_dom = eina_log_domain_register(dom, EINA_COLOR_GREEN);
   if (_eolian_gen_log_dom < 0)
     {
        EINA_LOG_ERR("Could not register log domain: %s", dom);
        goto end;
     }

   eina_log_timing(_eolian_gen_log_dom, EINA_LOG_STATE_STOP, EINA_LOG_STATE_INIT);

   struct option opts[] = {
      { "help",    no_argument,       NULL,       'h'        },
      { "gh",      no_argument,       &gen_what,  GEN_H      },
      { "gc",      no_argument,       &gen_what,  GEN_C      },
      { "gi",      no_argument,       &gen_what,  GEN_C_IMPL },
      { "gs",      no_argument,       &gen_what,  GEN_H_STUB },
      { "output",  required_argument, NULL,       'o'        },
      { "legacy",  no_argument,       &do_legacy, 1          },
      { "include", required_argument, NULL,       'I'        },
      { NULL, 0, NULL, 0 }
   };

   for (int opt; (opt = getopt_long(argc, argv, "vho:I:", opts, NULL)) != -1; )
     switch (opt)
       {
        case 0: break;
        case 'o':
          outf = optarg;
          break;
        case 'h':
          help = EINA_TRUE;
          break;
        case 'I':
          if (!eolian_directory_scan(optarg))
            {
               fprintf(stderr, "eolian: could not scan '%s'\n", optarg);
               goto end;
            }
          break;
        default:
          help = EINA_TRUE;
          break;
       }

   if (help)
     {
        printf("Usage: %s [-h/--help] [-I/--include input_dir] [--legacy] [--gh|--gc|--gi] [--output/-o outfile] file.eo ... \n", argv[0]);
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

   const char *eof = argv[optind++];
   if (!eof)
     {
        fprintf(stderr, "eolian: no input file\n");
        goto end;
     }

   if (!eolian_file_parse(eof))
     {
        fprintf(stderr, "eolian: could not parse file '%s'\n", eof);
        goto end;
     }

   if (!eolian_database_validate())
     {
        fprintf(stderr, "eolian: error validating database\n");
        goto end;
     }

   char *eofc = strdup(eof);
   char *eobn = basename(eofc);

   if (gen_what)
     {
        if (!outf)
          {
             fprintf(stderr, "eolian: no output file\n");
             free(eofc);
             goto end;
          }
        switch (gen_what)
          {
           case GEN_H:
             INF("Generating header file %s\n", outf);
             ret = !_generate_header_file(outf, eobn, do_legacy);
             break;
           case GEN_H_STUB:
             INF("Generating stub header file %s\n", outf);
             ret = !_generate_stub_header_file(outf, eobn);
             break;
           case GEN_C:
             INF("Generating source file %s\n", outf);
             ret = !_generate_c_file(outf, eobn, do_legacy);
             break;
           case GEN_C_IMPL:
             INF("Generating user source file %s\n", outf);
             ret = !_generate_impl_c_file(outf, eobn);
             break;
           default:
             ERR("Wrong generation option\n");
             break;
          }
     }
   else
     ret = 0;

   free(eofc);

end:
   eina_log_timing(_eolian_gen_log_dom, EINA_LOG_STATE_START, EINA_LOG_STATE_SHUTDOWN);
   eina_log_domain_unregister(_eolian_gen_log_dom);
   eolian_shutdown();
   eina_shutdown();
   return ret;
}