/* The Lua runtime component of the EFL */

#include <getopt.h>
#ifndef _WIN32
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#define pipe(x, mode) pipe(x)
#else
#include <io.h>
int fork_win(void);
#define fork fork_win
#define fdopen _fdopen
#define execv _execv
#define close _close
#define dup2 _dup2
#define pipe(x, mode) _pipe(x, 4096, ((mode && mode[0] && mode[1] == 'b') \
    ? _O_BINARY : _O_TEXT) | _NO_NOINHERIT)
#endif

#include "main.h"

typedef struct Arg_Data {
    int type;
    const char *value;
} Arg_Data;

enum {
    ARG_CODE = 0, ARG_LIBRARY, ARG_LIBDIR
};

static Eina_List *modlist = NULL;
static int require_ref = LUA_REFNIL;
static int appload_ref = LUA_REFNIL;
static const char *progname = NULL;

int el_log_domain = -1;

static void errmsg(const char *pname, const char *msg) {
    ERR("%s%s%s", pname ? pname : "", pname ? ": " : "", msg);
}

static int report(lua_State *L, int status) {
    if (status && !lua_isnil(L, -1)) {
        const char *msg = lua_tostring(L, -1);
        errmsg(progname, msg ? msg : "(non-string error)");
        lua_pop(L, 1);
    }
    return status;
}

static int traceback(lua_State *L) {
    lua_getglobal(L, "debug");
    if (!lua_istable(L, -1)) {
        lua_pop(L, 1);
        return 1;
    }
    lua_getfield(L, -1, "traceback");
    if (!lua_isfunction(L, -1)) {
        lua_pop(L, 2);
        return 1;
    }
    lua_pushvalue(L, 1);
    lua_pushinteger(L, 2);
    lua_call(L, 2, 1);
    return 1;
}

static int docall(lua_State *L, int narg, int nret) {
    int status;
    int bs = lua_gettop(L) - narg;
    lua_pushcfunction(L, traceback);
    lua_insert(L, bs);
    status = lua_pcall(L, narg, nret, bs);
    lua_remove(L, bs);
    if (status) lua_gc(L, LUA_GCCOLLECT, 0);
    return status;
}

static int getargs(lua_State *L, int argc, char **argv, int n) {
    int i;
    int narg = argc - (n + 1);
    luaL_checkstack(L, narg + 3, "too many arguments to script");
    for (i = n + 1; i < argc; ++i) lua_pushstring(L, argv[i]);
    lua_createtable(L, narg, n + 1);
    for (i = 0; i < argc; ++i) {
        if (!(i - n) && argv[i][0] == ':') {
            lua_pushstring(L, &argv[i][1]);
        } else {
            lua_pushstring(L, argv[i]);
        }
        lua_rawseti(L, -2, i - n);
    }
    return narg;
}

static int init_module(lua_State *L) {
    if (!lua_isnoneornil(L, 1)) {
        lua_pushvalue(L, 1);
        lua_call(L, 0, 0);
    }
    if (!lua_isnoneornil(L, 2)) {
        lua_pushvalue(L, 2);
        modlist = eina_list_append(modlist,
            (void*)(size_t)luaL_ref(L, LUA_REGISTRYINDEX));
    }
    return 0;
}

static int register_require(lua_State *L) {
    const char *corepath = lua_touserdata(L, lua_upvalueindex(1));
    const char *modpath  = lua_touserdata(L, lua_upvalueindex(2));
    const char *appspath = lua_touserdata(L, lua_upvalueindex(3));
    Eina_List  *largs    = lua_touserdata(L, lua_upvalueindex(4)), *l = NULL;
    Eina_Bool   noenv    = lua_toboolean (L, lua_upvalueindex(5));
    Arg_Data   *data     = NULL;
    int n = 2;
    lua_pushvalue(L, 1);
    require_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    lua_pushvalue(L, 2);
    appload_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    if (getenv("EFL_RUN_IN_TREE")) {
        corepath = PACKAGE_BUILD_DIR "/src/bin/elua/core";
        modpath  = PACKAGE_BUILD_DIR "/src/bin/elua/modules";
        appspath = PACKAGE_BUILD_DIR "/src/bin/elua/apps";
    } else {
        if (!corepath) {
            if (noenv || !(corepath = getenv("ELUA_CORE_DIR")) || !corepath[0])
                corepath = ELUA_CORE_DIR;
        }
        if (!modpath) {
            if (noenv || !(modpath = getenv("ELUA_MODULES_DIR")) || !modpath[0])
                modpath = ELUA_MODULES_DIR;
        }
        if (!appspath) {
            if (noenv || !(appspath = getenv("ELUA_APPS_DIR")) || !appspath[0])
                appspath = ELUA_APPS_DIR;
        }
    }
    lua_pushfstring(L, "%s/?.lua;", corepath);
    EINA_LIST_FOREACH(largs, l, data) {
        if (data->type != ARG_LIBDIR) continue;
        lua_pushfstring(L, "%s/?.lua;", data->value);
        ++n;
    }
    lua_pushfstring(L, "%s/?.lua;", modpath);
    lua_pushvalue(L, 3);
    lua_concat(L, n + 1);
    lua_pushfstring(L, "%s/?.lua;", appspath);
    lua_pushvalue(L, 4);
    lua_concat(L, 2);
    return 2;
}

static int dolib(lua_State *L, const char *libname) {
    lua_rawgeti(L, LUA_REGISTRYINDEX, require_ref);
    lua_pushstring(L, libname);
    return report(L, lua_pcall(L, 1, 0, 0));
}

static int dofile(lua_State *L, const char *fname) {
    return report(L, elua_loadfile(L, fname) || docall(L, 0, 1));
}

static int dostr(lua_State *L, const char *chunk, const char *chname) {
    return report(L, luaL_loadbuffer(L, chunk, strlen(chunk), chname)
        || docall(L, 0, 0));
}

static Eina_Bool loadapp(lua_State *L, const char *appname) {
    lua_rawgeti(L, LUA_REGISTRYINDEX, appload_ref);
    lua_pushstring(L, appname);
    lua_call(L, 1, 2);
    if (lua_isnil(L, -2)) {
        lua_remove(L, -2);
        return EINA_FALSE;
    }
    lua_pop(L, 1);
    return EINA_TRUE;
}

static int doscript(lua_State *L, int argc, char **argv, int n, int *quit) {
    int status;
    const char *fname = argv[n];
    int narg = getargs(L, argc, argv, n);
    lua_setglobal(L, "arg");
    if (fname[0] == '-' && !fname[1]) {
        fname = NULL;
    }
    if (fname && fname[0] == ':') {
        status = !loadapp(L, fname + 1);
    } else {
        status = elua_loadfile(L, fname);
    }
    lua_insert(L, -(narg + 1));
    if (!status) {
         status = docall(L, narg, 1);
    } else {
        lua_pop(L, narg);
    }
    if (!status) {
        *quit = lua_toboolean(L, -1);
        lua_pop(L, 1);
    }
    return report(L, status);
}

void shutdown(lua_State *L, int c) {
    void *data;
    INF("elua shutdown");

    EINA_LIST_FREE(modlist, data) {
        lua_rawgeti(L, LUA_REGISTRYINDEX, (size_t)data);
        lua_call(L, 0, 0);
    }

    if (L) lua_close(L);
    eina_shutdown();
    exit(c);
}

static int cb_ref = LUA_REFNIL;
static lua_State *LL = NULL;

static void smart_cb_wrapper(void *data, void *obj EINA_UNUSED,
void *einfo EINA_UNUSED) {
    int idx = (size_t)data;
    lua_rawgeti(LL, LUA_REGISTRYINDEX, cb_ref);
    lua_rawgeti(LL, -1, idx);
    lua_call(LL, 0, 0);
    lua_pop(LL, 1);
}

static int register_callbacks(lua_State *L) {
    union { void (*fptr)(void*, void*, void*); void *ptr; } u;
    lua_pushvalue(L, 1);
    cb_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    u.fptr = smart_cb_wrapper;
    lua_pushlightuserdata(L, u.ptr);
    return 1;
}

static FILE *elua_popen(const char *path, const char *argv[], const char *mode) {
    int read   = (!mode || mode[0] == 'r');
    int binary = mode && mode[0] && mode[1] == 'b';
    pid_t pid;

    int des[2];
    if (pipe(des, mode)) return NULL;

    pid = fork();
    if (!pid) {
        /* if read, stdout (1) is still open here
         * (parent can read, child can write) */
        close(des[!read]);
        dup2(des[read], read ? STDOUT_FILENO : STDIN_FILENO);
        execv(path, (char * const *)argv);
        return NULL;
    } else {
        /* if read, stdin (0) is still open here
         * (child can read, parent can write) */
        close(des[read]);
        return fdopen(des[!read], read ? (binary ? "rb" : "r")
            : (binary ? "wb" : "w"));
    }
}

static int elua_exec(lua_State *L) {
#ifndef _WIN32
    pid_t cpid = fork();
    if (cpid < 0) {
        lua_pushnil(L);
        lua_pushstring(L, strerror(errno));
        return 2;
    }
    if (!cpid) {
        const char **argv = alloca((lua_gettop(L) + 1) * sizeof(char*));
        int  i;
        for (i = 0; i < lua_gettop(L); ++i) {
            argv[i] = lua_tostring(L, i + 1);
        }
        argv[lua_gettop(L)] = NULL;
        execv(argv[0], (char*const*)argv);
        exit(1);
    } else {
        int status;
        if (waitpid(cpid, &status, 0) < 0) {
            lua_pushnil(L);
            lua_pushstring(L, strerror(errno));
            return 2;
        }
        lua_pushinteger(L, status);
        return 1;
    }
#else
    char *buf, *cptr;
    size_t buflen = 1;
    int i;

    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&si, sizeof(pi));

    for (i = 1; i < lua_gettop(L); ++i) {
        buflen += lua_objlen(L, i + 1) + 2;
    }

    buf = alloca(buflen + 1);
    cptr = buf;

    for (i = 1; i < lua_gettop(L); ++i) {
        size_t l;
        const char *arg = lua_tolstring(L, i + 1, &l);
        *(cptr++) = '"';
        memcpy(cptr, l + 1, arg);
        cptr += l;
        *(cptr++) = '"'
        if (i != (lua_gettop(L) - 1)) {
            *(cptr++) = ' ';
        } else {
            cptr[0] = '\0';
        }
    }

    if (!CreateProcess(lua_tostring(L, 1), buf, NULL, NULL, FALSE, 0, NULL,
    NULL, &si, &pi)) {
        LPVOID msgbuf;
        DWORD dw = GetLastError();
        FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER
            | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL, dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&msgbuf,
            0, NULL);
        lua_pushnil(L);
        lua_pushstring(L, (const char*)msgbuf);
        LocalFree(msgbuf);
        return 2;
    }

    int status;
    if (!GetExitCodeProcess(pi.hProcess, &status)) status = 0;
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    lua_pushinteger(L, status);
    return 1;
#endif
}

static int elua_build_args(lua_State *L) {
    /* todo: will probably need adjustments for Windows */
    int nargs = lua_gettop(L), n = 0, i;
    for (i = 1; i <= nargs; ++i) {
        lua_pushliteral(L, "'");     ++n;
        lua_pushvalue(L, i);         ++n;
        lua_pushliteral(L, "'");     ++n;
        if (i != nargs) {
            lua_pushliteral(L, " "); ++n;
        }
    }
    lua_concat(L, n);
    return 1;
}

struct Main_Data {
    int    argc;
    char **argv;
    int    status;
};

const luaL_reg cutillib[] = {
    { "init_module"       , init_module        },
    { "register_callbacks", register_callbacks },
    { "exec"              , elua_exec          },
    { "build_args"        , elua_build_args    },
    { NULL                , NULL               }
};

static int gettext_bind_textdomain(lua_State *L) {
#if ENABLE_NLS
    const char *textdomain = luaL_checkstring(L, 1);
    const char *dirname    = luaL_checkstring(L, 2);
    const char *ret;
    if (!textdomain[0] || !strcmp(textdomain, PACKAGE)) {
        lua_pushnil(L);
        lua_pushliteral(L, "invalid textdomain");
        return 2;
    }
    if (!(ret = bindtextdomain(textdomain, dirname))) {
        lua_pushnil(L);
        lua_pushstring(L, strerror(errno));
        return 2;
    }
    bind_textdomain_codeset(textdomain, "UTF-8");
    lua_pushstring(L, ret);
    return 1;
#else
    lua_pushliteral(L, "");
    return 1;
#endif
}

const luaL_reg gettextlib[] = {
    { "bind_textdomain", gettext_bind_textdomain },
    { NULL, NULL }
};

static void print_help(const char *progname, FILE *stream) {
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
                    "  -E,          --noenv                Ignore environment variables.\n", progname);
}

static struct option lopt[] = {
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
static int lua_main(lua_State *L) {
    Eina_Bool   noenv   = EINA_FALSE,
                hasexec = EINA_FALSE;
    Eina_List  *largs   = NULL, *l = NULL;
    Arg_Data   *data    = NULL;
    const char *coref   = NULL;
    char       *coredir = NULL, *moddir = NULL, *appsdir = NULL;
    char        modfile[1024];

    int ch;

    struct Main_Data *m = (struct Main_Data*)lua_touserdata(L, 1);

    int    argc = m->argc;
    char **argv = m->argv;

#if ENABLE_NLS
    char *(*dgettextp)(const char*, const char*) = dgettext;
    char *(*dngettextp)(const char*, const char*, const char*, unsigned long)
        = dngettext;
#endif

    progname = (argv[0] && argv[0][0]) ? argv[0] : "elua";

    while ((ch = getopt_long(argc, argv, "+LhC:M:A:e:l:I:E", lopt, NULL)) != -1) {
        switch (ch) {
            case 'h':
                print_help(progname, stdout);
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
            case 'I': {
                Arg_Data *v = malloc(sizeof(Arg_Data));
                v->type = (ch == 'e') ? ARG_CODE : ((ch == 'l')
                    ? ARG_LIBRARY : ARG_LIBDIR);
                v->value = optarg;
                largs = eina_list_append(largs, v);
                break;
            }
        }
    }

    INF("arguments parsed");

    lua_gc(L, LUA_GCSTOP, 0);

    luaL_openlibs(L);

    if (getenv("EFL_RUN_IN_TREE")) {
        Arg_Data *v = malloc(sizeof(Arg_Data));
        v->type     = ARG_LIBDIR;
        v->value    = PACKAGE_BUILD_DIR "/src/bindings/luajit";
        largs       = eina_list_append(largs, v);
        coref       = PACKAGE_BUILD_DIR "/src/bin/elua/core";
    } else if (!(coref = coredir)) {
        if (noenv || !(coref = getenv("ELUA_CORE_DIR")) || !coref[0]) {
            coref = ELUA_CORE_DIR;
        }
    }
    snprintf(modfile, sizeof(modfile), "%s/module.lua", coref);
    if (report(L, elua_loadfile(L, modfile))) {
        m->status = 1;
        return 0;
    }
    lua_pushlightuserdata(L, coredir);
    lua_pushlightuserdata(L, moddir);
    lua_pushlightuserdata(L, appsdir);
    lua_pushlightuserdata(L, largs);
    lua_pushboolean      (L, noenv);
    lua_pushcclosure(L, register_require, 5);
    lua_createtable(L, 0, 0);
    luaL_register(L, NULL, cutillib);
    lua_call(L, 2, 0);

    snprintf(modfile, sizeof(modfile), "%s/gettext.lua", coref);
    if (report(L, elua_loadfile(L, modfile))) {
        m->status = 1;
        return 0;
    }
    lua_createtable(L, 0, 0);
    luaL_register(L, NULL, gettextlib);
#if ENABLE_NLS
    lua_pushlightuserdata(L, *((void**)&dgettextp));
    lua_setfield(L, -2, "dgettext");
    lua_pushlightuserdata(L, *((void**)&dngettextp));
    lua_setfield(L, -2, "dngettext");
#endif
    lua_call(L, 1, 0);

    elua_register_cache(L);
    lua_gc(L, LUA_GCRESTART, 0);

    INF("elua lua state initialized");

    /* load all the things */
    EINA_LIST_FOREACH(largs, l, data) {
        switch (data->type) {
            case ARG_CODE:
                if (!hasexec) hasexec = EINA_TRUE;
                if (dostr(L, data->value, "=(command line)")) {
                    m->status = 1;
                    return 0;
                }
                break;
            case ARG_LIBRARY:
                if (dolib(L, data->value)) {
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
    if (optind < argc) {
        int quit = 0;
        if ((m->status = doscript(L, argc, argv, optind, &quit))) return 0;
        if (quit) return 0;
    } else if (!hasexec) {
        int quit;
        if ((m->status = dofile(L, NULL))) return 0;
        quit = lua_toboolean(L, -1);
        lua_pop(L, 1);
        if (quit) return 0;
    }

    ecore_main_loop_begin();

    return 0;
}

int main(int argc, char **argv) {
    struct Main_Data m;
    lua_State *L = NULL;

#if ENABLE_NLS
    setlocale(LC_ALL, "");
    bindtextdomain(PACKAGE, LOCALE_DIR);
    bind_textdomain_codeset(PACKAGE, "UTF-8");
    textdomain(PACKAGE);
#endif

    eina_init();

    if (!(el_log_domain = eina_log_domain_register("elua",
    EINA_COLOR_ORANGE))) {
        printf("cannot set elua log domain\n");
        ERR("could not set elua log domain.");
        el_log_domain = EINA_LOG_DOMAIN_GLOBAL;
    }

    INF("elua logging initialized: %d", el_log_domain);

    if (!(L = luaL_newstate())) {
        ERR("could not initialize elua state.");
        shutdown(L, 1);
    }

    LL = L;

    INF("elua lua state created");

    m.argc   = argc;
    m.argv   = argv;
    m.status = 0;

    shutdown(L, !!(lua_cpcall(L, lua_main, &m) || m.status));

    return 0; /* never gets here */
}