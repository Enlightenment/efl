#include <unistd.h>
#include <libgen.h>

#include "main.h"
#include "types.h"
#include "headers.h"
#include "sources.h"

int _eolian_gen_log_dom = -1;

enum
{
   GEN_H        = 1 << 0,
   GEN_H_LEGACY = 1 << 1,
   GEN_H_STUB   = 1 << 2,
   GEN_C        = 1 << 3,
   GEN_C_IMPL   = 1 << 4
};

static const char *_dexts[5] =
{
  ".eo.h", ".eo.legacy.h", ".eo.stub.h", ".eo.c", ".eo.c"
};

static int
_get_bit_pos(int flag)
{
   int pos = 0;
   for (; !(flag & 1); flag >>= 1)
     ++pos;
   return pos;
}

static void
_print_usage(const char *progn, FILE *outf)
{
   fprintf(outf, "Usage: %s [options] [input]\n", progn);
   fprintf(outf, "Options:\n"
                 "  -I inc        include path \"inc\"\n"
                 "  -g type       generate file of type \"type\"\n"
                 "  -o name       specify the base name for output\n"
                 "  -o type:name  specify a particular output filename\n"
                 "  -h            print this message and exit\n"
                 "  -v            print version and exit\n"
                 "\n"
                 "Available types:\n"
                 "  h: C header file (.eo.h)\n"
                 "  l: Legacy C header file (.eo.legacy.h)\n"
                 "  s: Stub C header file (.eo.stub.h)\n"
                 "  c: C source file (.eo.c)\n"
                 "  i: Implementation file (added into .eo.c)\n"
                 "\n"
                 "By default, the 'hc' set is used.\n"
                 "Output filenames are determined from input .eo filename.\n");
}

static void
_print_version(FILE *outf)
{
   fprintf(outf, "Eolian C generator version: " PACKAGE_VERSION "\n");
}

static Eina_Bool
_try_set_out(char t, char **outs, const char *val)
{
   int pos = -1;
   switch (t)
     {
      case 'h':
        pos = _get_bit_pos(GEN_H);
        break;
      case 'l':
        pos = _get_bit_pos(GEN_H_LEGACY);
        break;
      case 's':
        pos = _get_bit_pos(GEN_H_STUB);
        break;
      case 'c':
        pos = _get_bit_pos(GEN_C);
        break;
      case 'i':
        pos = _get_bit_pos(GEN_C_IMPL);
        break;
     }
   if (pos < 0)
     return EINA_FALSE;
   if (outs[pos])
     free(outs[pos]);
   outs[pos] = strdup(val);
   return EINA_TRUE;
}

static void _fill_all_outs(char **outs, const char *val)
{
   size_t vlen = strlen(val);
   for (size_t i = 0; i < (sizeof(_dexts) / sizeof(char *)); ++i)
   {
      if (outs[i])
        continue;
      size_t dlen = strlen(_dexts[i]);
      char *str = malloc(vlen + dlen + 1);
      memcpy(str, val, vlen);
      memcpy(str + vlen, _dexts[i], dlen);
      str[vlen + dlen] = '\0';
      outs[i] = str;
   }
}

static Eina_Strbuf *
_include_guard(const char *fname, const char *gname, Eina_Strbuf *buf)
{
   if (!buf || !eina_strbuf_length_get(buf))
     return buf;

   if (!gname)
     gname = "";

   char iname[256] = {0};
   strncpy(iname, fname, sizeof(iname) - 1);
   char *inamep = iname;
   eina_str_toupper(&inamep);

   Eina_Strbuf *g = eina_strbuf_new();
   eina_strbuf_append_printf(g, "#ifndef _%s_%s\n", iname, gname);
   eina_strbuf_append_printf(g, "#define _%s_%s\n\n", iname, gname);

   eina_strbuf_replace_all(g, ".", "_");
   eina_strbuf_append(g, eina_strbuf_string_get(buf));
   eina_strbuf_append(g, "\n#endif\n");
   eina_strbuf_free(buf);
   return g;
}

static const char *
_get_filename(const char *path)
{
   if (!path)
     return NULL;
   const char *ret1 = strrchr(path, '/');
   const char *ret2 = strrchr(path, '\\');
   if (!ret1 && !ret2)
     return path;
   if (ret1 && ret2)
     {
        if (ret1 > ret2)
          return ret1 + 1;
        else
          return ret2 + 1;
     }
   if (ret1)
     return ret1 + 1;
   return ret2 + 1;
}

static Eina_Bool
_write_file(const char *fname, const Eina_Strbuf *buf, Eina_Bool append)
{
   FILE *f = fopen(fname, append ? "ab" : "wb");
   if (!f)
     {
        fprintf(stderr, "eolian: could not open '%s' (%s)\n",
                fname, strerror(errno));
        return EINA_FALSE;
     }

   Eina_Bool fret = EINA_TRUE;

   size_t bl = eina_strbuf_length_get(buf);
   if (!bl)
     goto end;

   if (fwrite(eina_strbuf_string_get(buf), 1, bl, f) != bl)
     {
        fprintf(stderr, "eolian: could not write '%s' (%s)\n",
                fname, strerror(errno));
        fret = EINA_FALSE;
     }

end:
   fclose(f);
   return fret;
}

static Eina_Strbuf *
_read_file(const char *fname)
{
   FILE *f = fopen(fname, "rb");
   if (!f)
     return NULL;

   fseek(f, 0, SEEK_END);
   long fs = ftell(f);
   if (fs < 0)
     {
        fprintf(stderr, "eolian: could not get length of '%s'\n", fname);
        fclose(f);
        return NULL;
     }
   fseek(f, 0, SEEK_SET);

   char *cont = malloc(fs + 1);
   if (!cont)
     {
        fprintf(stderr, "eolian: could not allocate memory for '%s'\n", fname);
        fclose(f);
        return NULL;
     }

   long as = fread(cont, 1, fs, f);
   if (as != fs)
     {
        fprintf(stderr, "eolian: could not read %ld bytes from '%s' (got %ld)\n",
                fs, fname, as);
        free(cont);
        fclose(f);
        return NULL;
     }

   cont[fs] = '\0';
   fclose(f);
   return eina_strbuf_manage_new_length(cont, fs);
}

char *eo_gen_c_full_name_get(const char *nm)
{
   if (!nm)
     return NULL;
   char *buf = strdup(nm);
   if (!buf)
     return NULL;
   for (char *p = strchr(buf, '.'); p; p = strchr(p, '.'))
     *p = '_';
   return buf;
}

Eina_Bool eo_gen_class_names_get(const Eolian_Class *cl, char **cname,
                                 char **cnameu, char **cnamel)
{
   char *cn = NULL, *cnu = NULL, *cnl = NULL;
   cn = eo_gen_c_full_name_get(eolian_class_full_name_get(cl));
   if (!cn)
     return EINA_FALSE;
   if (cname)
     *cname = cn;

   if (cnameu)
     {
        cnu = strdup(cn);
        if (!cnu)
          {
             free(cn);
             return EINA_FALSE;
          }
        eina_str_toupper(&cnu);
        *cnameu = cnu;
     }

   if (cnamel)
     {
        cnl = strdup(cn);
        if (!cnl)
          {
             free(cn);
             free(cnu);
             return EINA_FALSE;
          }
        eina_str_tolower(&cnl);
        *cnamel = cnl;
     }

   if (!cname)
     free(cn);

   return EINA_TRUE;
}

static Eina_Bool
_write_header(const char *ofname, const char *ifname, Eina_Bool legacy)
{
   INF("generating header: %s (legacy: %d)", ofname, legacy);
   Eina_Strbuf *buf = eina_strbuf_new();

   eo_gen_types_header_gen(ifname, buf, EINA_TRUE, legacy);
   buf = _include_guard(ifname, "TYPES", buf);

   Eina_Strbuf *cltd = eo_gen_class_typedef_gen(ifname);
   if (cltd)
     {
        cltd = _include_guard(ifname, "CLASS_TYPE", cltd);
        eina_strbuf_prepend_char(buf, '\n');
        eina_strbuf_prepend(buf, eina_strbuf_string_get(cltd));
        eina_strbuf_free(cltd);
     }

   const Eolian_Class *cl = eolian_class_get_by_file(ifname);
   eo_gen_header_gen(cl, buf, legacy);
   if (cl || !legacy)
     {
        buf = _include_guard(_get_filename(ofname), NULL, buf);
        if (_write_file(ofname, buf, EINA_FALSE))
          {
             eina_strbuf_free(buf);
             return EINA_TRUE;
          }
     }

   eina_strbuf_free(buf);
   return EINA_FALSE;
}

static Eina_Bool
_write_stub_header(const char *ofname, const char *ifname)
{
   INF("generating stuv header: %s", ofname);
   Eina_Strbuf *buf = eina_strbuf_new();

   eo_gen_types_header_gen(ifname, buf, EINA_FALSE, EINA_FALSE);

   Eina_Strbuf *cltd = eo_gen_class_typedef_gen(ifname);
   if (cltd)
     {
        eina_strbuf_prepend_char(buf, '\n');
        eina_strbuf_prepend(buf, eina_strbuf_string_get(cltd));
        eina_strbuf_free(cltd);
     }

   buf = _include_guard(ifname, "STUBS", buf);

   Eina_Bool ret = _write_file(ofname, buf, EINA_FALSE);
   eina_strbuf_free(buf);
   return ret;
}

static Eina_Bool
_write_source(const char *ofname, const char *ifname)
{
   INF("generating source: %s", ofname);
   Eina_Strbuf *buf = eina_strbuf_new();

   const Eolian_Class *cl = eolian_class_get_by_file(ifname);
   eo_gen_source_gen(cl, buf);
   if (cl)
     {
        if (_write_file(ofname, buf, EINA_FALSE))
          {
             eina_strbuf_free(buf);
             return EINA_TRUE;
          }
     }

   eina_strbuf_free(buf);
   return EINA_FALSE;
}

static Eina_Bool
_write_impl(const char *ofname, const char *ifname)
{
   INF("generating impl: %s", ofname);
   Eina_Strbuf *buf = eina_strbuf_new();

   Eina_Bool ret = _write_file(ofname, buf, EINA_FALSE);
   eina_strbuf_free(buf);
   return ret;
}

int
main(int argc, char **argv)
{
   int pret = 1;

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

   char *outs[5] = { NULL, NULL, NULL, NULL, NULL };

   int gen_what = 0;
   for (int opt; (opt = getopt(argc, argv, "I:g:o:hvV")) != -1;)
     switch (opt)
       {
        case 0:
          break;
        case 'I':
          if (!eolian_directory_scan(optarg))
            {
               fprintf(stderr, "eolian: could not scan '%s'\n", optarg);
               goto end;
            }
          break;
        case 'g':
          for (const char *wstr = optarg; *wstr; ++wstr)
            switch (*wstr)
              {
               case 'h':
                 gen_what |= GEN_H;
                 break;
               case 'l':
                 gen_what |= GEN_H_LEGACY;
                 break;
               case 's':
                 gen_what |= GEN_H_STUB;
                 break;
               case 'c':
                 gen_what |= GEN_C;
                 break;
               case 'i':
                 gen_what |= GEN_C_IMPL;
                 break;
               default:
                 fprintf(stderr, "unknown type: '%c'\n", *wstr);
                 goto end;
              }
          break;
        case 'o':
          if (strchr(optarg, ':'))
            {
               const char *abeg = optarg;
               const char *cpos = strchr(abeg, ':');
               if (((cpos - abeg) != 1) || !_try_set_out(*abeg, outs, cpos + 1))
                 {
                    char *oa = strdup(abeg);
                    oa[cpos - abeg] = '\0';
                    fprintf(stderr, "unknown type: '%s'\n", oa);
                    free(oa);
                    goto end;
                 }
            }
          else _fill_all_outs(outs, optarg);
          break;
        case 'h':
          _print_usage(argv[0], stdout);
          goto end;
        case 'v':
          _print_version(stdout);
          goto end;
        default:
          _print_usage(argv[0], stderr);
          goto end;
       }

   const char *input = argv[optind];
   if (!input)
     {
        fprintf(stderr, "eolian: no input file\n");
        goto end;
     }

   const char *ext = strrchr(input, '.');
   if (!ext || strcmp(ext, ".eo"))
     {
        fprintf(stderr, "eolian: invalid input file '%s'\n", input);
        goto end;
     }

   if (!eolian_file_parse(input))
     {
        fprintf(stderr, "eolian: could not parse file '%s'\n", input);
        goto end;
     }

   char *inoext = strdup(input);
   inoext[ext - input] = '\0';
   _fill_all_outs(outs, inoext);
   free(inoext);

   const char *eobn = _get_filename(input);

   if (!gen_what)
     gen_what = GEN_H | GEN_C;

   Eina_Bool succ = EINA_TRUE;
   if (gen_what & GEN_H)
     succ = _write_header(outs[_get_bit_pos(GEN_H)], eobn, EINA_FALSE);
   if (succ && (gen_what & GEN_H_LEGACY))
     succ = _write_header(outs[_get_bit_pos(GEN_H_LEGACY)], eobn, EINA_TRUE);
   if (succ && (gen_what & GEN_H_STUB))
     succ = _write_stub_header(outs[_get_bit_pos(GEN_H_STUB)], eobn);
   if (succ && (gen_what & GEN_C))
     succ = _write_source(outs[_get_bit_pos(GEN_C)], eobn);
   if (succ && (gen_what & GEN_C_IMPL))
     succ = _write_impl(outs[_get_bit_pos(GEN_C_IMPL)], eobn);

   free(inoext);

   if (!succ)
     goto end;

   pret = 0;
end:
   eina_log_timing(_eolian_gen_log_dom, EINA_LOG_STATE_START, EINA_LOG_STATE_SHUTDOWN);
   eina_log_domain_unregister(_eolian_gen_log_dom);
   for (size_t i = 0; i < (sizeof(_dexts) / sizeof(char *)); ++i)
     free(outs[i]);
   eolian_shutdown();
   eina_shutdown();

   return pret;
}
