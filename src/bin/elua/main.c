#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#ifdef ENABLE_NLS
# include <locale.h>
# include <libintl.h>
# define _(x) dgettext(PACKAGE, x)
#else
# define _(x) (x)
#endif

#include <Eina.h>
#include <Ecore.h>

#include <Elua.h>

static int _el_log_domain = -1;

#define INF(...) EINA_LOG_DOM_INFO(_el_log_domain, __VA_ARGS__)
#define ERR(...) EINA_LOG_DOM_ERR(_el_log_domain, __VA_ARGS__)

struct Main_Data
{
   Elua_State  *es;
   int          argc;
   char       **argv;
   int          status;
};

static void
elua_print_help(const char *pname, FILE *stream)
{
   fprintf(stream, "Usage: %s [OPTIONS] [SCRIPT [ARGS]]\n\n"
                   "A main entry for all EFL/LuaJIT powered applications.\n\n"
                   "The following options are supported:\n\n"
                   ""
                   "  -h          Show this message.\n"
                   "  -l          Show a license message.\n"
                   "  -C[COREDIR] Elua core directory path.\n"
                   "  -M[MODDIR]  Elua modules directory path.\n"
                   "  -A[APPDIR]  Elua applications directory path.\n"
                   "  -l[LIBRARY] Require library 'library'.\n"
                   "  -I[DIR],    Append an additional require path.\n"
                   "  -E,         Ignore environment variables.\n", pname);
}

/* protected main */
static int
elua_main(lua_State *L)
{
   Eina_Bool   noenv   = EINA_FALSE;
   const char *coredir = NULL, *moddir = NULL, *appsdir = NULL;

   struct Main_Data *m  = (struct Main_Data*)lua_touserdata(L, 1);
   Elua_State       *es = m->es;

   int    argc = m->argc;
   char **argv = m->argv;

   int ch;

   while ((ch = getopt(argc, argv, "+LhC:M:A:l:I:E")) != -1)
     switch (ch)
       {
        case 'h':
          elua_print_help(elua_state_prog_name_get(es), stdout); return 0;
        case 'C':
          coredir = optarg; break;
        case 'M':
          moddir  = optarg; break;
        case 'A':
          appsdir = optarg; break;
        case 'l':
        case 'I':
          if (!optarg[0]) continue;
          if (ch == 'l')
            elua_util_require(es, optarg);
          else
            elua_state_include_path_add(es, optarg);
          break;
        case 'E':
          noenv = EINA_TRUE; break;
       }

   INF("arguments parsed");

   lua_gc(L, LUA_GCSTOP, 0);

   elua_state_dirs_set(es, coredir, moddir, appsdir);
   elua_state_dirs_fill(es, noenv);

   if (!elua_state_setup(es))
     {
        m->status = 1;
        return 0;
     }

   lua_gc(L, LUA_GCRESTART, 0);

   INF("elua lua state initialized");

   if (optind < argc)
     {
        int quit = 0;
        if (!elua_util_script_run(es, argc, argv, optind, &quit))
          {
             m->status = 1;
             return 0;
          }
        if (quit)
          return 0;
     }
   else
     {
        ERR("nothing to run");
        m->status = 1;
        return 0;
     }

   ecore_main_loop_begin();

   return 0;
}

void
elua_bin_shutdown(Elua_State *es, int c)
{
   INF("elua shutdown");
   if (es) elua_state_free(es);
   if (_el_log_domain != EINA_LOG_DOMAIN_GLOBAL)
     eina_log_domain_unregister(_el_log_domain);
   elua_shutdown();
   exit(c);
}

#if LUA_VERSION_NUM < 502
#  define elua_cpcall(L, f, u) lua_cpcall(L, f, u)
#else
#  define elua_cpcall(L, f, u) \
      (lua_pushcfunction(L, f), lua_pushlightuserdata(L, u), lua_pcall(L, 1, 0, 0))
#endif

int
main(int argc, char **argv)
{
   struct Main_Data m;
   Elua_State *es = NULL;

#ifdef ENABLE_NLS
   setlocale(LC_ALL, "");
   bindtextdomain(PACKAGE, LOCALE_DIR);
   bind_textdomain_codeset(PACKAGE, "UTF-8");
   textdomain(PACKAGE);
#endif

   elua_init();

   if (!(_el_log_domain = eina_log_domain_register("elua", EINA_COLOR_ORANGE)))
     {
        printf("cannot set elua log domain\n");
        ERR("could not set elua log domain.");
        _el_log_domain = EINA_LOG_DOMAIN_GLOBAL;
     }

   INF("elua logging initialized: %d", _el_log_domain);

   if (!(es = elua_state_new((argv[0] && argv[0][0]) ? argv[0] : "elua")))
     {
        ERR("could not initialize elua state.");
        elua_bin_shutdown(es, 1);
     }

   INF("elua lua state created");

   m.es     = es;
   m.argc   = argc;
   m.argv   = argv;
   m.status = 0;

   elua_bin_shutdown(es, !!(elua_cpcall(elua_state_lua_state_get(es), elua_main, &m) || m.status));

   return 0; /* never gets here */
}
