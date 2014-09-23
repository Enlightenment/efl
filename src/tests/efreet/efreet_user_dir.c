#include "config.h" 

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Efreet.h>

static void
_print_data_home(void)
{
   const char *s = efreet_data_home_get();
   if (!s) return;
   fputs(s, stdout);
}

static void
_print_data_dirs(void)
{
   const Eina_List *itr, *lst = efreet_data_dirs_get();
   const char *s;
   Eina_Bool first = EINA_TRUE;

   EINA_LIST_FOREACH(lst, itr, s)
     {
        if (first)
          first = EINA_FALSE;
        else
          putchar(':');
        fputs(s, stdout);
     }
}

static void
_print_config_home(void)
{
   const char *s = efreet_config_home_get();
   if (!s) return;
   fputs(s, stdout);
}

static void
_print_config_dirs(void)
{
   const Eina_List *itr, *lst = efreet_config_dirs_get();
   const char *s;
   Eina_Bool first = EINA_TRUE;

   EINA_LIST_FOREACH(lst, itr, s)
     {
        if (first)
          first = EINA_FALSE;
        else
          putchar(':');
        fputs(s, stdout);
     }
}

static void
_print_cache_home(void)
{
   const char *s = efreet_cache_home_get();
   if (!s) return;
   fputs(s, stdout);
}

static void
_print_runtime_dir(void)
{
   const char *s = efreet_runtime_dir_get();
   if (!s) return;
   fputs(s, stdout);
}

static void
_print_desktop_dir(void)
{
   const char *s = efreet_desktop_dir_get();
   if (!s) return;
   fputs(s, stdout);
}

static void
_print_download_dir(void)
{
   const char *s = efreet_download_dir_get();
   if (!s) return;
   fputs(s, stdout);
}

static void
_print_templates_dir(void)
{
   const char *s = efreet_templates_dir_get();
   if (!s) return;
   fputs(s, stdout);
}

static void
_print_public_share_dir(void)
{
   const char *s = efreet_public_share_dir_get();
   if (!s) return;
   fputs(s, stdout);
}

static void
_print_documents_dir(void)
{
   const char *s = efreet_documents_dir_get();
   if (!s) return;
   fputs(s, stdout);
}

static void
_print_music_dir(void)
{
   const char *s = efreet_music_dir_get();
   if (!s) return;
   fputs(s, stdout);
}

static void
_print_pictures_dir(void)
{
   const char *s = efreet_pictures_dir_get();
   if (!s) return;
   fputs(s, stdout);
}

static void
_print_videos_dir(void)
{
   const char *s = efreet_videos_dir_get();
   if (!s) return;
   fputs(s, stdout);
}

struct var_alias {
   const char *alias;
   const char *name;
};

struct var_print {
   const char *name;
   void (*print)(void);
};

static const struct var_alias aliases[] = {
  {"DATA_HOME", "XDG_DATA_HOME"},
  {"DATA_DIRS", "XDG_DATA_DIRS"},
  {"CONFIG_HOME", "XDG_CONFIG_HOME"},
  {"CONFIG_DIRS", "XDG_CONFIG_DIRS"},
  {"CACHE_HOME", "XDG_CACHE_HOME"},
  {"RUNTIME_DIR", "XDG_RUNTIME_DIR"},
  {"DESKTOP", "XDG_DESKTOP_DIR"},
  {"DOWNLOAD", "XDG_DOWNLOAD_DIR"},
  {"TEMPLATES", "XDG_TEMPLATES_DIR"},
  {"PUBLICSHARE", "XDG_PUBLICSHARE_DIR"},
  {"DOCUMENTS", "XDG_DOCUMENTS_DIR"},
  {"MUSIC", "XDG_MUSIC_DIR"},
  {"PICTURES", "XDG_PICTURES_DIR"},
  {"VIDEOS", "XDG_VIDEOS_DIR"},
  {NULL, NULL}
};

static const struct var_print printers[] = {
  {"XDG_DATA_HOME", _print_data_home},
  {"XDG_DATA_DIRS", _print_data_dirs},
  {"XDG_CONFIG_HOME", _print_config_home},
  {"XDG_CONFIG_DIRS", _print_config_dirs},
  {"XDG_CACHE_HOME", _print_cache_home},
  {"XDG_RUNTIME_DIR", _print_runtime_dir},
  {"XDG_DESKTOP_DIR", _print_desktop_dir},
  {"XDG_DOWNLOAD_DIR", _print_download_dir},
  {"XDG_TEMPLATES_DIR", _print_templates_dir},
  {"XDG_PUBLICSHARE_DIR", _print_public_share_dir},
  {"XDG_DOCUMENTS_DIR", _print_documents_dir},
  {"XDG_MUSIC_DIR", _print_music_dir},
  {"XDG_PICTURES_DIR", _print_pictures_dir},
  {"XDG_VIDEOS_DIR", _print_videos_dir},
  {NULL, NULL}
};

static void
_print_do(const struct var_print *itr)
{
   printf("%s=", itr->name);
   itr->print();
   putchar('\n');
}

static const struct var_print *
_resolve_printer(const char *alias)
{
   const struct var_print *p = printers;
   const struct var_alias *a;

   for (; p->name != NULL; p++)
     if (strcmp(p->name, alias) == 0)
       return p;

   for (a = aliases; a->alias != NULL; a++)
     if (strcmp(a->alias, alias) == 0)
       {
          for (p = printers; p->name != NULL; p++)
            if (strcmp(p->name, a->name) == 0)
              return p;
          return NULL;
       }

   return NULL;
}

static void
_print(const char *alias)
{
   const struct var_print *itr = _resolve_printer(alias);

   if (!itr)
     {
        fprintf(stderr, "ERROR: unknown variable %s\n", alias);
        return;
     }

   _print_do(itr);
}

static void
_print_all(void)
{
   const struct var_print *itr = printers;
   for (; itr->name != NULL; itr++)
     _print_do(itr);
}

static void
_help(const char *prog)
{
   const struct var_print *itr = printers;

   printf("Usage:\n\n"
          "\t%s [varname1] ... [varnameN]\n\n"
          "Where varnameN is one of:\n",
          prog);

   for (; itr->name != NULL; itr++)
     printf("\t%s\n", itr->name);
   putchar('\n');
}

int
main(int argc, char *argv[])
{
   if (!efreet_init())
     {
        fputs("ERROR: Could not initialize efreet!\n", stderr);
        return EXIT_FAILURE;
     }

   if (argc < 2)
     _print_all();
   else
     {
        int i;

        for (i = 1; i < argc; i++)
          if ((strcmp(argv[i], "-h") == 0) ||
              (strcmp(argv[i], "--help") == 0))
            {
               _help(argv[0]);
               goto end;
            }

        for (i = 1; i < argc; i++)
          _print(argv[i]);
     }

 end:
   efreet_shutdown();
   return EXIT_SUCCESS;
}
