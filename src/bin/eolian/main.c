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
   if (!fbody || !eina_strbuf_string_get(fbody))
     return fbody;

   if (!suffix)
     suffix = "";

   char incname[255];
   memset(incname, 0, sizeof(incname));
   strncpy (incname, fname, sizeof(incname) - 1);
   char *p = incname;
   eina_str_toupper(&p);

   Eina_Strbuf *incguard = eina_strbuf_new();
   eina_strbuf_append_printf(incguard, "#ifndef _%s_%s\n#define _%s_%s\n\n",
                             incname, suffix, incname, suffix);
   eina_strbuf_replace_all(incguard, ".", "_");
   eina_strbuf_append(incguard, eina_strbuf_string_get(fbody));
   eina_strbuf_append(incguard, "\n#endif\n");
   eina_strbuf_free(fbody);
   return incguard;
}

static const char *
_filename_get(const char *path)
{
   if (!path)
     return NULL;
   const char *ret = strrchr(path, '/');
   if (!ret)
     return path;
   return ret + 1;
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
   if (file_size < 0)
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

   Eina_Bool ret = EINA_TRUE;
   size_t blen = eina_strbuf_length_get(buffer);

   if (!blen)
     goto end;

   if (fwrite(eina_strbuf_string_get(buffer), 1, blen, fd) != blen)
     {
        fprintf(stderr, "eolian: could not write '%s' (%s)\n",
                filename, strerror(errno));
        ret = EINA_FALSE;
     }

end:
   fclose(fd);
   return ret;
}

static Eina_Bool
_generate_header(const char *outf, const char *inf, Eina_Bool legacy)
{
   Eina_Strbuf *buf = eina_strbuf_new();

   if (!types_header_generate(inf, buf, EINA_TRUE, legacy))
     {
        fprintf(stderr, "eolian: could not generate types of '%s'\n", inf);
        eina_strbuf_free(buf);
        return EINA_FALSE;
     }

   buf = _include_guard_enclose(inf, "TYPES", buf);

   Eina_Strbuf *ctbuf = eina_strbuf_new();
   if (types_class_typedef_generate(inf, ctbuf))
     {
        ctbuf = _include_guard_enclose(inf, "CLASS_TYPE", ctbuf);
        eina_strbuf_append_char(ctbuf, '\n');
        eina_strbuf_prepend(buf, eina_strbuf_string_get(ctbuf));
     }
   eina_strbuf_free(ctbuf);

   const Eolian_Class *cl = eolian_class_get_by_file(inf);
   if (cl)
     {
        Eina_Bool gret = legacy ? legacy_header_generate(cl, buf)
                                : eo_header_generate(cl, buf);
        if (!gret)
          {
             fprintf(stderr, "eolian: could not generate header for '%s'\n",
                     eolian_class_name_get(cl));
             eina_strbuf_free(buf);
             return EINA_FALSE;
          }
     }

   if (cl || !legacy)
     {
        buf = _include_guard_enclose(_filename_get(outf), NULL, buf);
        if (_write_file(outf, buf, EINA_FALSE))
          {
             eina_strbuf_free(buf);
             return EINA_TRUE;
          }
     }

   eina_strbuf_free(buf);
   return EINA_FALSE;
}

static Eina_Bool
_generate_stub_header(const char *outf, const char *inf)
{
   Eina_Strbuf *buf = eina_strbuf_new();

   if (!types_header_generate(inf, buf, EINA_FALSE, EINA_FALSE))
     {
        fprintf(stderr, "eolian: could not generate types of '%s'\n", inf);
        eina_strbuf_free(buf);
        return EINA_FALSE;
     }

   Eina_Strbuf *ctbuf = eina_strbuf_new();
   if (types_class_typedef_generate(inf, ctbuf))
     {
        eina_strbuf_append_char(ctbuf, '\n');
        eina_strbuf_prepend(buf, eina_strbuf_string_get(ctbuf));
     }
   eina_strbuf_free(ctbuf);

   buf = _include_guard_enclose(_filename_get(outf), "STUBS", buf);

   Eina_Bool ret = _write_file(outf, buf, EINA_FALSE);
   eina_strbuf_free(buf);
   return ret;
}

static Eina_Bool
_generate_c(const char *outf, const char *inf, Eina_Bool legacy)
{
   Eina_Strbuf *eobuf = eina_strbuf_new(),
               *lgbuf = eina_strbuf_new();

   const Eolian_Class *cl = eolian_class_get_by_file(inf);
   if (cl)
     {
        if (!eo_source_generate(cl, eobuf))
          {
             fprintf(stderr, "eolian: could not generate source for '%s'\n",
                     eolian_class_name_get(cl));
             eina_strbuf_free(eobuf);
             eina_strbuf_free(lgbuf);
             return EINA_FALSE;
          }

        if (legacy && !legacy_source_generate(cl, lgbuf))
          {
             fprintf(stderr, "eolian: could not generate source for '%s'\n",
                     eolian_class_name_get(cl));
             eina_strbuf_free(eobuf);
             eina_strbuf_free(lgbuf);
             return EINA_FALSE;
          }
     }

   Eina_Bool ret = _write_file(outf, eobuf, EINA_FALSE) &&
                   _write_file(outf, lgbuf, EINA_TRUE);
   eina_strbuf_free(eobuf);
   eina_strbuf_free(lgbuf);
   return ret;
}

static Eina_Bool
_generate_impl(const char *outf, const char *inf)
{
   const Eolian_Class *cl = eolian_class_get_by_file(inf);
   if (!cl)
     return EINA_FALSE;

   Eina_Strbuf *buf = NULL;
   if (!_read_file(outf, &buf))
     return EINA_FALSE;

   if (!impl_source_generate(cl, buf))
     {
        fprintf(stderr, "eolian: could not generate source for '%s'\n",
                eolian_class_name_get(cl));
        eina_strbuf_free(buf);
        return EINA_FALSE;
     }

   Eina_Bool ret = _write_file(outf, buf, EINA_FALSE);
   eina_strbuf_free(buf);
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
   int gen_what = GEN_NOTHING, do_legacy = 0, ret = 1, silent_types = 0;
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
      { "silent-types", no_argument,  &silent_types, 1       },
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
        printf("Usage: %s [-h/--help] [-I/--include input_dir] [--legacy] [--gh|--gs|--gc|--gi] [--output/-o outfile] file.eo ... \n", argv[0]);
        printf("       --help/-h Print that help\n");
        printf("       --include/-I Include 'input_dir' as directory to search .eo files into\n");
        printf("       --output/-o Force output filename to 'outfile'\n");
        printf("       --gh Generate C header file [.h]\n");
        printf("       --gs Generate C type stubs [.h]\n");
        printf("       --gc Generate C source file [.c]\n");
        printf("       --gi Generate C implementation source file [.c]. The output will be a series of functions that have to be filled.\n");
        printf("       --legacy Generate legacy\n");
        printf("       --silent-types Silence type validation\n");
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

   if (!eolian_database_validate(silent_types))
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
             ret = !_generate_header(outf, eobn, do_legacy);
             break;
           case GEN_H_STUB:
             INF("Generating stub header file %s\n", outf);
             ret = !_generate_stub_header(outf, eobn);
             break;
           case GEN_C:
             INF("Generating source file %s\n", outf);
             ret = !_generate_c(outf, eobn, do_legacy);
             break;
           case GEN_C_IMPL:
             INF("Generating user source file %s\n", outf);
             ret = !_generate_impl(outf, eobn);
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
