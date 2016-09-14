#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <unistd.h>

#include "main.h"

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
                 "By default, the 'hcl' set is used.\n"
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
          printf("including file: '%s'\n", optarg);
          if (!eolian_directory_scan(optarg))
            {
               fprintf(stderr, "eolian: could not scan '%s'\n", optarg);
               goto end;
            }
          printf("include: %s\n", optarg);
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

   char *inoext = strdup(input);
   inoext[ext - input] = '\0';
   _fill_all_outs(outs, inoext);
   free(inoext);

   if (!gen_what)
     gen_what = GEN_H | GEN_H_LEGACY | GEN_C;

   printf("generating\n");
   if (gen_what & GEN_H)
     printf("generating header: %s\n", outs[_get_bit_pos(GEN_H)]);
   if (gen_what & GEN_H_LEGACY)
     printf("generating legacy header: %s\n", outs[_get_bit_pos(GEN_H_LEGACY)]);
   if (gen_what & GEN_H_STUB)
     printf("generating header stub: %s\n", outs[_get_bit_pos(GEN_H_STUB)]);
   if (gen_what & GEN_C)
     printf("generating source: %s\n", outs[_get_bit_pos(GEN_C)]);
   if (gen_what & GEN_C_IMPL)
     printf("generating impl: %s\n", outs[_get_bit_pos(GEN_C_IMPL)]);

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
