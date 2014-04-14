/* The Lua runtime component of the EFL */

#include "main.h"

static Eina_List *modlist = NULL;
static int require_ref = LUA_REFNIL;

int el_log_domain = -1;

enum {
    ARG_CODE = 0, ARG_LIBRARY, ARG_LIBDIR
};

typedef struct Arg_Data {
    int type;
    const char *value;
} Arg_Data;

static Eina_Bool append_cb(const Ecore_Getopt      *parser EINA_UNUSED,
                           const Ecore_Getopt_Desc *desc   EINA_UNUSED,
                           const char              *str,
                           void                    *data,
                           Ecore_Getopt_Value      *val)
{
    Eina_List **l = val->listp;
    Arg_Data   *v = malloc(sizeof(Arg_Data));
    v->type       = (size_t)data;
    v->value      = str;
    *l = eina_list_append(*l, v);
    return EINA_TRUE;
}

static Ecore_Getopt opt = {
    "elua", "%prog [options] [script [args]]", "0.0.1", "See COPYING",
    "See COPYING", "A main entry for all EFL/LuaJIT powered applications.",
    0, {
        ECORE_GETOPT_LICENSE('L', "license"),
        ECORE_GETOPT_COPYRIGHT('c', "copyright"),
        ECORE_GETOPT_VERSION('v', "version"),
        ECORE_GETOPT_HELP('h', "help"),

        ECORE_GETOPT_STORE_STR('C', "core-dir", "Elua core directory path."),
        ECORE_GETOPT_STORE_STR('M', "modules-dir", "Elua modules directory path."),

        ECORE_GETOPT_CALLBACK_ARGS('e', "execute", "Execute string "
            "'code'.", "CODE", append_cb, (void*)ARG_CODE),
        ECORE_GETOPT_CALLBACK_ARGS('l', "library", "Require library 'library'.",
            "LIBRARY", append_cb, (void*)ARG_LIBRARY),
        ECORE_GETOPT_CALLBACK_ARGS('L', "lib-dir", "Append an additional "
            "require path 'LIBDIR'.", "LIBDIR", append_cb, (void*)ARG_LIBDIR),
        ECORE_GETOPT_STORE_TRUE('E', "noenv", "Ignore environment vars."),

        ECORE_GETOPT_SENTINEL
    }
};

static void errmsg(const char *pname, const char *msg) {
    ERR("%s%s%s", pname ? pname : "", pname ? ": " : "", msg);
}

static int report(lua_State *L, int status) {
    if (status && !lua_isnil(L, -1)) {
        const char *msg = lua_tostring(L, -1);
        errmsg(opt.prog, msg ? msg : "(non-string error)");
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

static int docall(lua_State *L, int narg) {
    int status;
    int bs = lua_gettop(L) - narg;
    lua_pushcfunction(L, traceback);
    lua_insert(L, bs);
    status = lua_pcall(L, narg, 0, bs);
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
        lua_pushstring(L, argv[i]);
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
    Eina_List  *largs    = lua_touserdata(L, lua_upvalueindex(3)), *l = NULL;
    Arg_Data   *data     = NULL;
    int n = 2;
    lua_pushvalue(L, 1);
    require_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    if (!corepath) {
        if (!(corepath = getenv("ELUA_CORE_DIR")) || !corepath[0])
              corepath = ELUA_CORE_DIR;
    }
    if (!modpath) {
        if (!(modpath = getenv("ELUA_MODULES_DIR")) || !modpath[0])
              modpath = ELUA_MODULES_DIR;
    }
    lua_pushfstring(L, "%s/?.lua;", corepath);
    EINA_LIST_FOREACH(largs, l, data) {
        if (data->type != ARG_LIBDIR) continue;
        lua_pushstring(L, data->value);
        ++n;
    }
    lua_pushfstring(L, "%s/?.lua;", modpath);
    lua_pushvalue(L, 2);
    lua_concat(L, n + 1);
    return 1;
}

static int dolib(lua_State *L, const char *libname) {
    lua_rawgeti(L, LUA_REGISTRYINDEX, require_ref);
    lua_pushstring(L, libname);
    return report(L, lua_pcall(L, 1, 0, 0));
}

static int dofile(lua_State *L, const char *fname) {
    return report(L, elua_loadfile(L, fname) || docall(L, 0));
}

static int dostr(lua_State *L, const char *chunk, const char *chname) {
    return report(L, luaL_loadbuffer(L, chunk, strlen(chunk), chname)
        || docall(L, 0));
}

static int doscript(lua_State *L, int argc, char **argv, int n) {
    int status;
    const char *fname = argv[n];
    int narg = getargs(L, argc, argv, n);
    lua_setglobal(L, "arg");
    if (fname[0] == '-' && !fname[1]) {
        fname = NULL;
    }
    status = elua_loadfile(L, fname);
    lua_insert(L, -(narg + 1));
    if (!status) {
         status = docall(L, narg);
    } else {
        lua_pop(L, narg);
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

struct Main_Data {
    int    argc;
    char **argv;
    int    status;
};

const luaL_reg cutillib[] = {
    { "init_module", init_module },
    { "register_callbacks", register_callbacks },
    { NULL, NULL }
};

/* protected main */
static int lua_main(lua_State *L) {
    Eina_Bool     quit    = EINA_FALSE,
                 noenv    = EINA_FALSE,
                 hasexec  = EINA_FALSE;
    Eina_List  *largs     = NULL, *l = NULL;
    Arg_Data   *data      = NULL;
    char       *coredir   = NULL, *moddir = NULL;

    int nonopt;

    struct Main_Data *m = (struct Main_Data*)lua_touserdata(L, 1);

    int    argc = m->argc;
    char **argv = m->argv;

    if (argv[0] && argv[0][0]) opt.prog = argv[0];

    nonopt = ecore_getopt_parse(&opt, (Ecore_Getopt_Value[]){
        ECORE_GETOPT_VALUE_BOOL(quit), /* license */
        ECORE_GETOPT_VALUE_BOOL(quit), /* copyright */
        ECORE_GETOPT_VALUE_BOOL(quit), /* version */
        ECORE_GETOPT_VALUE_BOOL(quit), /* help */

        ECORE_GETOPT_VALUE_STR(coredir),
        ECORE_GETOPT_VALUE_STR( moddir),

        ECORE_GETOPT_VALUE_LIST(largs),
        ECORE_GETOPT_VALUE_LIST(largs),
        ECORE_GETOPT_VALUE_LIST(largs),
        ECORE_GETOPT_VALUE_LIST(largs),
        ECORE_GETOPT_VALUE_BOOL(noenv)
    }, argc, argv);

    INF("arguments parsed");

    lua_gc(L, LUA_GCSTOP, 0);

    luaL_openlibs(L);

    if (report(L, elua_loadfile(L, ELUA_DATA_DIR "/core/module.lua"))) {
        m->status = 1;
        return 0;
    }
    lua_pushlightuserdata(L, coredir);
    lua_pushlightuserdata(L, moddir);
    lua_pushlightuserdata(L, largs);
    lua_pushcclosure(L, register_require, 3);
    lua_createtable(L, 0, 0);
    luaL_register(L, NULL, cutillib);
    lua_call(L, 2, 0);

    elua_register_cache(L);
    lua_gc(L, LUA_GCRESTART, 0);

    INF("elua lua state initialized");

    if (quit) return 0;

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
    if (nonopt >= 0 && nonopt < argc) {
        if ((m->status = doscript(L, argc, argv, nonopt))) return 0;
    } else if (!hasexec) {
        dofile(L, NULL);
    }

    ecore_main_loop_begin();

    return 0;
}

int main(int argc, char **argv) {
    struct Main_Data m;
    lua_State *L = NULL;

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