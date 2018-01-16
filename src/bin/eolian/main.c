#include <stdlib.h>
#include <unistd.h>

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
  ".h", ".legacy.h", ".stub.h", ".c", ".c"
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
                 "  -S            do not scan system dir for eo files\n"
                 "  -g type       generate file of type \"type\"\n"
                 "  -o name       specify the base name for output\n"
                 "  -o type:name  specify a particular output filename\n"
                 "  -h            print this message and exit\n"
                 "  -v            print version and exit\n"
                 "\n"
                 "Available types:\n"
                 "  h: C header file (.eo.h/.eot.h)\n"
                 "  l: Legacy C header file (.eo.legacy.h/.eot.legacy.h)\n"
                 "  s: Stub C header file (.eo.stub.h/.eot.stub.h)\n"
                 "  c: C source file (.eo.c)\n"
                 "  i: Implementation file (.c, merged with existing)\n"
                 "\n"
                 "By default, the 'hc' set is used ('h' for .eot files).\n\n"
                 "The system-wide Eolian directory is scanned for eo files\n"
                 "by default, together with all specified '-I' flags.\n\n"
                 "Output filenames are determined from input .eo filename.\n"
                 "Default output path is where the input file is.\n\n"
                 "Also, specifying a type-dependent input file automatically\n"
                 "adds it to generated files, so if you specify those, you\n"
                 "don't need to explicitly specify -g for those types anymore.\n\n"
                 "Explicit output base name is without extension. The extension\n"
                 "is determined from the input file name. If that is not possible\n"
                 "for some reason, it defaults to \".eo\". Obviously, this does not\n"
                 "affect specific filenames (-o x:y) as these are full names.\n"
                 "Implementation files are a special case (no \".eo\" added).\n");
}

static void
_print_version(FILE *outf)
{
   fprintf(outf, "Eolian C generator version: " PACKAGE_VERSION "\n");
}

static Eina_Bool
_try_set_out(char t, char **outs, const char *val, int *what)
{
   int pos = -1;
   switch (t)
     {
      case 'h':
        pos = _get_bit_pos(GEN_H);
        *what |= GEN_H;
        break;
      case 'l':
        pos = _get_bit_pos(GEN_H_LEGACY);
        *what |= GEN_H_LEGACY;
        break;
      case 's':
        pos = _get_bit_pos(GEN_H_STUB);
        *what |= GEN_H_STUB;
        break;
      case 'c':
        pos = _get_bit_pos(GEN_C);
        *what |= GEN_C;
        break;
      case 'i':
        pos = _get_bit_pos(GEN_C_IMPL);
        *what |= GEN_C_IMPL;
        break;
     }
   if (pos < 0)
     return EINA_FALSE;
   if (outs[pos])
     free(outs[pos]);
   outs[pos] = strdup(val);
   return EINA_TRUE;
}

static void _fill_all_outs(char **outs, const char *val, char *base)
{
   const char *ext = strrchr(val, '.');
   if (!ext)
     ext = ".eo";

   char *basen = base;
   if (!basen)
     {
        basen = strdup(val);
        char *p = strrchr(basen, '.');
        if (p) *p = '\0';
     }

   size_t blen = strlen(basen),
          elen = strlen(ext);

   for (size_t i = 0; i < (sizeof(_dexts) / sizeof(char *)); ++i)
     {
        if (outs[i])
          continue;
        size_t dlen = strlen(_dexts[i]);
        char *str = malloc(blen + elen + dlen + 1);
        char *p = str;
        memcpy(p, basen, blen);
        p += blen;
        if ((1 << i) != GEN_C_IMPL)
          {
             memcpy(p, ext, elen);
             p += elen;
          }
        memcpy(p, _dexts[i], dlen);
        p[dlen] = '\0';
        outs[i] = str;
     }

   if (!base)
     free(basen);
}

static Eina_Strbuf *
_include_guard(const char *fname, const char *gname, Eina_Strbuf *buf)
{
   if (!buf)
     return NULL;

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
_write_file(const char *fname, const Eina_Strbuf *buf)
{
   FILE *f = fopen(fname, "wb");
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

static Eina_Bool
_read_file(const char *fname, Eina_Strbuf **buf)
{
   FILE *f = fopen(fname, "rb");
   if (!f)
     {
        *buf = eina_strbuf_new();
        return EINA_TRUE;
     }

   fseek(f, 0, SEEK_END);
   long fs = ftell(f);
   if (fs < 0)
     {
        fprintf(stderr, "eolian: could not get length of '%s'\n", fname);
        fclose(f);
        return EINA_FALSE;
     }
   fseek(f, 0, SEEK_SET);

   char *cont = malloc(fs + 1);
   if (!cont)
     {
        fprintf(stderr, "eolian: could not allocate memory for '%s'\n", fname);
        fclose(f);
        return EINA_FALSE;
     }

   long as = fread(cont, 1, fs, f);
   if (as != fs)
     {
        fprintf(stderr, "eolian: could not read %ld bytes from '%s' (got %ld)\n",
                fs, fname, as);
        free(cont);
        fclose(f);
        return EINA_FALSE;
     }

   cont[fs] = '\0';
   fclose(f);
   *buf = eina_strbuf_manage_new_length(cont, fs);
   return EINA_TRUE;
}

char *eo_gen_c_full_name_get(const char *nm)
{
   if (!nm)
     return NULL;
   char *buf = strdup(nm);
   if (!buf)
     abort();
   for (char *p = strchr(buf, '.'); p; p = strchr(p, '.'))
     *p = '_';
   return buf;
}

void eo_gen_class_names_get(const Eolian_Class *cl, char **cname,
                            char **cnameu, char **cnamel)
{
   char *cn = NULL, *cnu = NULL, *cnl = NULL;
   cn = eo_gen_c_full_name_get(eolian_class_full_name_get(cl));
   if (!cn)
     abort();
   if (cname)
     *cname = cn;

   if (cnameu)
     {
        cnu = strdup(cn);
        if (!cnu)
          {
             free(cn);
             abort();
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
             abort();
          }
        eina_str_tolower(&cnl);
        *cnamel = cnl;
     }

   if (!cname)
     free(cn);
}

static Eina_Bool
_write_header(const Eolian *eos, const Eolian_Unit *src, const char *ofname,
              const char *ifname, Eina_Bool legacy)
{
   INF("generating header: %s (legacy: %d)", ofname, legacy);
   Eina_Strbuf *buf = eina_strbuf_new();

   eo_gen_types_header_gen(src, eolian_declarations_get_by_file(eos, ifname),
                           buf, EINA_TRUE, legacy);
   buf = _include_guard(ifname, "TYPES", buf);

   Eina_Strbuf *cltd = eo_gen_class_typedef_gen(src, ifname);
   if (cltd)
     {
        cltd = _include_guard(ifname, "CLASS_TYPE", cltd);
        eina_strbuf_prepend_char(buf, '\n');
        eina_strbuf_prepend(buf, eina_strbuf_string_get(cltd));
        eina_strbuf_free(cltd);
     }

   const Eolian_Class *cl = eolian_class_get_by_file(src, ifname);
   eo_gen_header_gen(src, cl, buf, legacy);
   if (cl || !legacy)
     {
        buf = _include_guard(_get_filename(ofname), NULL, buf);
        if (_write_file(ofname, buf))
          {
             eina_strbuf_free(buf);
             return EINA_TRUE;
          }
     }

   eina_strbuf_free(buf);
   return EINA_FALSE;
}

static Eina_Bool
_write_stub_header(const Eolian *eos, const Eolian_Unit *src, const char *ofname,
                   const char *ifname)
{
   INF("generating stub header: %s", ofname);
   Eina_Strbuf *buf = eina_strbuf_new();

   eo_gen_types_header_gen(src, eolian_declarations_get_by_file(eos, ifname),
                           buf, EINA_FALSE, EINA_FALSE);

   Eina_Strbuf *cltd = eo_gen_class_typedef_gen(src, ifname);
   if (cltd)
     {
        eina_strbuf_prepend_char(buf, '\n');
        eina_strbuf_prepend(buf, eina_strbuf_string_get(cltd));
        eina_strbuf_free(cltd);
     }

   buf = _include_guard(_get_filename(ofname), "STUBS", buf);

   Eina_Bool ret = _write_file(ofname, buf);
   eina_strbuf_free(buf);
   return ret;
}

static Eina_Bool
_write_source(const Eolian *eos, const Eolian_Unit *src, const char *ofname,
              const char *ifname, Eina_Bool eot)
{
   INF("generating source: %s", ofname);
   Eina_Strbuf *buf = eina_strbuf_new();

   const Eolian_Class *cl = eolian_class_get_by_file(src, ifname);
   eo_gen_types_source_gen(eolian_declarations_get_by_file(eos, ifname), buf);
   eo_gen_source_gen(cl, buf);
   if (cl || (eot && eina_strbuf_length_get(buf)))
     {
        if (_write_file(ofname, buf))
          {
             eina_strbuf_free(buf);
             return EINA_TRUE;
          }
     }

   eina_strbuf_free(buf);
   return EINA_FALSE;
}

static Eina_Bool
_write_impl(const Eolian_Unit *src, const char *ofname, const char *ifname)
{
   INF("generating impl: %s", ofname);

   const Eolian_Class *cl = eolian_class_get_by_file(src, ifname);
   if (!cl)
     return EINA_FALSE;

   Eina_Strbuf *buf;
   if (!_read_file(ofname, &buf))
     return EINA_FALSE;

   eo_gen_impl_gen(cl, buf);
   Eina_Bool ret = _write_file(ofname, buf);
   eina_strbuf_free(buf);
   return ret;
}

int
main(int argc, char **argv)
{
   int pret = 1;

   char *outs[5] = { NULL, NULL, NULL, NULL, NULL };
   char *basen = NULL;
   Eina_List *includes = NULL;

   eina_init();
   eolian_init();

   Eolian *eos = eolian_new();

   const char *dom = "eolian_gen";
   _eolian_gen_log_dom = eina_log_domain_register(dom, EINA_COLOR_GREEN);
   if (_eolian_gen_log_dom < 0)
     {
        EINA_LOG_ERR("Could not register log domain: %s", dom);
        goto end;
     }

   eina_log_timing(_eolian_gen_log_dom, EINA_LOG_STATE_STOP, EINA_LOG_STATE_INIT);

   int gen_what = 0;
   Eina_Bool scan_system = EINA_TRUE;

   for (int opt; (opt = getopt(argc, argv, "SI:g:o:hv")) != -1;)
     switch (opt)
       {
        case 0:
          break;
        case 'S':
          scan_system = EINA_FALSE;
          break;
        case 'I':
          /* just a pointer to argv contents, so it persists */
          includes = eina_list_append(includes, optarg);
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
               if (((cpos - abeg) != 1) ||
                   !_try_set_out(*abeg, outs, cpos + 1, &gen_what))
                 {
                    char *oa = strdup(abeg);
                    oa[cpos - abeg] = '\0';
                    fprintf(stderr, "unknown type: '%s'\n", oa);
                    free(oa);
                    goto end;
                 }
            }
          else
            {
               if (basen)
                 free(basen);
               basen = strdup(optarg);
            }
          break;
        case 'h':
          _print_usage(argv[0], stdout);
          pret = 0;
          goto end;
        case 'v':
          _print_version(stdout);
          pret = 0;
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
   if (!ext || (strcmp(ext, ".eo") && strcmp(ext, ".eot")))
     {
        fprintf(stderr, "eolian: invalid input file '%s'\n", input);
        goto end;
     }

   if (scan_system)
     {
        if (!eolian_system_directory_scan(eos))
          {
             fprintf(stderr, "eolian: could not scan system directory\n");
             goto end;
          }
     }

   const char *inc;
   EINA_LIST_FREE(includes, inc)
     {
        if (!eolian_directory_scan(eos, inc))
          {
             fprintf(stderr, "eolian: could not scan '%s'\n", inc);
             goto end;
          }
     }

   const Eolian_Unit *src = eolian_file_parse(eos, input);
   if (!src)
     {
        fprintf(stderr, "eolian: could not parse file '%s'\n", input);
        goto end;
     }

   _fill_all_outs(outs, input, basen);

   const char *eobn = _get_filename(input);

   if (!gen_what)
     gen_what = GEN_H | GEN_C;

   Eina_Bool succ = EINA_TRUE;
   if (gen_what & GEN_H)
     succ = _write_header(eos, src, outs[_get_bit_pos(GEN_H)], eobn, EINA_FALSE);
   if (succ && (gen_what & GEN_H_LEGACY))
     succ = _write_header(eos, src, outs[_get_bit_pos(GEN_H_LEGACY)], eobn, EINA_TRUE);
   if (succ && (gen_what & GEN_H_STUB))
     succ = _write_stub_header(eos, src, outs[_get_bit_pos(GEN_H_STUB)], eobn);
   if (succ && (gen_what & GEN_C))
     succ = _write_source(eos, src, outs[_get_bit_pos(GEN_C)], eobn, !strcmp(ext, ".eot"));
   if (succ && (gen_what & GEN_C_IMPL))
     succ = _write_impl(src, outs[_get_bit_pos(GEN_C_IMPL)], eobn);

   if (!succ)
     goto end;

   pret = 0;
end:
   if (_eolian_gen_log_dom >= 0)
     {
        eina_log_timing(_eolian_gen_log_dom, EINA_LOG_STATE_START, EINA_LOG_STATE_SHUTDOWN);
        eina_log_domain_unregister(_eolian_gen_log_dom);
     }

   eina_list_free(includes);
   for (size_t i = 0; i < (sizeof(_dexts) / sizeof(char *)); ++i)
     free(outs[i]);
   free(basen);

   eolian_free(eos);
   eolian_shutdown();
   eina_shutdown();

   return pret;
}
