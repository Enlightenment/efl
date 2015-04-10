#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

/* The Lua runtime component of the EFL */

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

#include "Elua.h"

typedef struct Arg_Data
{
   int type;
   const char *value;
} Arg_Data;

enum
{
   ARG_CODE = 0,
   ARG_LIBRARY
};

static const char *elua_progname = NULL;
static Elua_State *elua_state    = NULL;

static int _el_log_domain = -1;

#define DBG(...) EINA_LOG_DOM_DBG(_el_log_domain, __VA_ARGS__)
#define INF(...) EINA_LOG_DOM_INFO(_el_log_domain, __VA_ARGS__)
#define WRN(...) EINA_LOG_DOM_WARN(_el_log_domain, __VA_ARGS__)
#define ERR(...) EINA_LOG_DOM_ERR(_el_log_domain, __VA_ARGS__)
#define CRT(...) EINA_LOG_DOM_CRITICAL(_el_log_domain, __VA_ARGS__)

static int
elua_traceback(lua_State *L)
{
   lua_getglobal(L, "debug");
   if (!lua_istable(L, -1))
     {
        lua_pop(L, 1);
        return 1;
     }
   lua_getfield(L, -1, "traceback");
   if (!lua_isfunction(L, -1))
     {
        lua_pop(L, 2);
        return 1;
     }
   lua_pushvalue(L, 1);
   lua_pushinteger(L, 2);
   lua_call(L, 2, 1);
   return 1;
}

static int
elua_docall(Elua_State *es, int narg, int nret)
{
   int status;
   lua_State *L = elua_state_lua_state_get(es);
   int bs = lua_gettop(L) - narg;
   lua_pushcfunction(L, elua_traceback);
   lua_insert(L, bs);
   status = lua_pcall(L, narg, nret, bs);
   lua_remove(L, bs);
   if (status)
      lua_gc(L, LUA_GCCOLLECT, 0);
   return status;
}

static int
elua_getargs(Elua_State *es, int argc, char **argv, int n)
{
   lua_State *L = elua_state_lua_state_get(es);
   int i;
   int narg = argc - (n + 1);
   luaL_checkstack(L, narg + 3, "too many arguments to script");
   for (i = n + 1; i < argc; ++i)
     {
        lua_pushstring(L, argv[i]);
     }
   lua_createtable(L, narg, n + 1);
   for (i = 0; i < argc; ++i)
     {
        lua_pushstring(L, argv[i]);
        lua_rawseti(L, -2, i - n);
     }
   return narg;
}

static int
elua_dolib(Elua_State *es, const char *libname)
{
   lua_State *L = elua_state_lua_state_get(es);
   elua_state_require_ref_push(es);
   lua_pushstring(L, libname);
   return elua_report_error(es, elua_progname, lua_pcall(L, 1, 0, 0));
}

static int
elua_dofile(Elua_State *es, const char *fname)
{
   return elua_report_error(es, elua_progname,
                            elua_io_loadfile(es, fname)
                            || elua_docall(es, 0, 1));
}

static int
elua_dostr(Elua_State *es, const char *chunk, const char *chname)
{
   return elua_report_error(es, elua_progname,
                            luaL_loadbuffer(elua_state_lua_state_get(es), chunk, strlen(chunk),
                                            chname)
                            || elua_docall(es, 0, 0));
}

static Eina_Bool
elua_loadapp(Elua_State *es, const char *appname)
{
   lua_State *L = elua_state_lua_state_get(es);
   elua_state_appload_ref_push(es);
   lua_pushstring(L, appname);
   lua_call(L, 1, 2);
   if (lua_isnil(L, -2))
     {
        lua_remove(L, -2);
        return EINA_FALSE;
     }
   lua_pop(L, 1);
   return EINA_TRUE;
}

static int
elua_doscript(Elua_State *es, int argc, char **argv, int n, int *quit)
{
   int status;
   const char *fname = argv[n];
   int narg = elua_getargs(es, argc, argv, n);
   lua_setglobal(elua_state_lua_state_get(es), "arg");
   if (fname[0] == '-' && !fname[1])
     {
        fname = NULL;
     }
   if (fname)
     {
        /* check if there is a file of that name */
        FILE *f = fopen(fname, "r");
        if (f)
          {
             fclose(f);
             status = elua_io_loadfile(es, fname);
          }
        else
          status = !elua_loadapp(es, fname);
     }
   else
     {
        status = elua_io_loadfile(es, fname);
     }
   lua_insert(elua_state_lua_state_get(es), -(narg + 1));
   if (!status)
     {
         status = elua_docall(es, narg, 1);
     }
   else
     {
        lua_pop(elua_state_lua_state_get(es), narg);
     }
   if (!status)
     {
        *quit = lua_toboolean(elua_state_lua_state_get(es), -1);
        lua_pop(elua_state_lua_state_get(es), 1);
     }
   return elua_report_error(es, elua_progname, status);
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

struct Main_Data
{
   Elua_State  *es;
   int          argc;
   char       **argv;
   int          status;
};

const luaL_reg cutillib[] =
{
   { "init_module", elua_module_init },
   { "popenv"     , elua_io_popen    },
   { NULL         , NULL             }
};

static void
elua_print_help(const char *pname, FILE *stream)
{
   fprintf(stream, "Usage: %s [OPTIONS] [SCRIPT [ARGS]]\n\n"
                   "A main entry for all EFL/LuaJIT powered applications.\n\n"
                   "The following options are supported:\n\n"
                   ""
                   "  -h,          --help                 Show this message.\n"
                   "  -l,          --license              Show a license message.\n"
                   "  -C[COREDIR], --core-dir=[COREDIR]   Elua core directory path.\n"
                   "  -M[MODDIR],  --modules-dir=[MODDIR] Elua modules directory path.\n"
                   "  -A[APPDIR],  --apps-dir=[APPDIR]    Elua applications directory path.\n"
                   "  -e[CODE],    --execute=[CODE]       Execute string 'code'.\n"
                   "  -l[LIBRARY], --library=[LIBRARY]    Require library 'library'.\n"
                   "  -I[DIR],     --lib-dir=[DIR]        Append an additional require path.\n"
                   "  -E,          --noenv                Ignore environment variables.\n", pname);
}

static struct option lopt[] =
{
   { "help"       , no_argument      , NULL, 'h' },

   { "core-dir"   , required_argument, NULL, 'C' },
   { "modules-dir", required_argument, NULL, 'M' },
   { "apps-dir"   , required_argument, NULL, 'A' },

   { "execute"    , required_argument, NULL, 'e' },
   { "library"    , required_argument, NULL, 'l' },
   { "lib-dir"    , required_argument, NULL, 'I' },
   { "noenv"      , no_argument      , NULL, 'E' },
   { NULL         , 0                , NULL,   0 }
};

/* protected main */
static int
elua_main(lua_State *L)
{
   Eina_Bool   noenv   = EINA_FALSE,
               hasexec = EINA_FALSE;
   Eina_List  *largs   = NULL, *l = NULL;
   Arg_Data   *data    = NULL;
   const char *coredir = NULL, *moddir = NULL, *appsdir = NULL;
   char        modfile[PATH_MAX];

   int ch;

   struct Main_Data *m = (struct Main_Data*)lua_touserdata(L, 1);
   Elua_State *es = m->es;

   int    argc = m->argc;
   char **argv = m->argv;

   elua_progname = (argv[0] && argv[0][0]) ? argv[0] : "elua";

   while ((ch = getopt_long(argc, argv, "+LhC:M:A:e:l:I:E", lopt, NULL)) != -1)
     {
        switch (ch)
          {
           case 'h':
             elua_print_help(elua_progname, stdout);
             return 0;
           case 'C':
             coredir = optarg;
             break;
           case 'M':
             moddir = optarg;
             break;
           case 'A':
             appsdir = optarg;
             break;
           case 'e':
           case 'l':
             {
                Arg_Data *v = malloc(sizeof(Arg_Data));
                v->type = (ch == 'e') ? ARG_CODE : ARG_LIBRARY;
                v->value = optarg;
                largs = eina_list_append(largs, v);
                break;
             }
           case 'I':
             elua_state_include_path_add(es, optarg);
             break;
           case 'E':
             noenv = EINA_TRUE;
          }
     }

   INF("arguments parsed");

   lua_gc(L, LUA_GCSTOP, 0);

   elua_state_dirs_set(es, coredir, moddir, appsdir);
   elua_state_dirs_fill(es, noenv);

   coredir = elua_state_core_dir_get(es);
   moddir  = elua_state_mod_dir_get(es);
   appsdir = elua_state_apps_dir_get(es);

   if (!coredir || !moddir || !appsdir)
     {
        ERR("could not set one or more script directories");
        m->status = 1;
        return 0;
     }

   snprintf(modfile, sizeof(modfile), "%s/module.lua", coredir);
   if (elua_report_error(es, elua_progname, elua_io_loadfile(es, modfile)))
     {
        m->status = 1;
        return 0;
     }
   lua_pushcfunction(L, elua_module_system_init);
   lua_createtable(L, 0, 0);
   luaL_register(L, NULL, cutillib);
   lua_call(L, 2, 0);

   snprintf(modfile, sizeof(modfile), "%s/gettext.lua", coredir);
   if (elua_report_error(es, elua_progname, elua_io_loadfile(es, modfile)))
     {
        m->status = 1;
        return 0;
     }
   elua_state_setup_i18n(es);
   lua_call(L, 1, 0);

   elua_io_register(es);
   lua_gc(L, LUA_GCRESTART, 0);

   INF("elua lua state initialized");

   /* load all the things */
   EINA_LIST_FOREACH(largs, l, data)
     {
        switch (data->type)
          {
             case ARG_CODE:
                if (!hasexec) hasexec = EINA_TRUE;
                if (elua_dostr(es, data->value, "=(command line)"))
                  {
                     m->status = 1;
                     return 0;
                  }
                break;
             case ARG_LIBRARY:
                if (elua_dolib(es, data->value))
                  {
                     m->status = 1;
                     return 0;
                  }
                break;
             default:
                break;
          }
     }

   /* cleanup */
   EINA_LIST_FREE(largs, data) free(data);

   /* run script or execute sdin as file */
   if (optind < argc)
     {
        int quit = 0;
        if ((m->status = elua_doscript(es, argc, argv, optind, &quit))) return 0;
        if (quit) return 0;
     }
   else if (!hasexec)
     {
        int quit;
        if ((m->status = elua_dofile(es, NULL))) return 0;
        quit = lua_toboolean(L, -1);
        lua_pop(L, 1);
        if (quit) return 0;
     }

   ecore_main_loop_begin();

   return 0;
}

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

   if (!(es = elua_state_new()))
     {
        ERR("could not initialize elua state.");
        elua_bin_shutdown(es, 1);
     }

   elua_state = es;

   INF("elua lua state created");

   m.es     = es;
   m.argc   = argc;
   m.argv   = argv;
   m.status = 0;

   elua_bin_shutdown(es, !!(lua_cpcall(elua_state_lua_state_get(es), elua_main, &m) || m.status));

   return 0; /* never gets here */
}
