#include "Elua.h"
#include "elua_private.h"

static Eina_Prefix *_elua_pfx = NULL;

static int _elua_init_counter = 0;
int _elua_log_dom = -1;

EAPI int
elua_init(void)
{
   const char *dom = "elua";
   if (_elua_init_counter > 0) return ++_elua_init_counter;

   eina_init();
   _elua_log_dom = eina_log_domain_register(dom, EINA_COLOR_LIGHTBLUE);
   if (_elua_log_dom < 0)
     {
        EINA_LOG_ERR("Could not register log domain: %s", dom);
        return EINA_FALSE;
     }

   eina_log_timing(_elua_log_dom, EINA_LOG_STATE_STOP, EINA_LOG_STATE_INIT);
   INF("elua init");

   _elua_pfx = eina_prefix_new(NULL, elua_init, "ELUA", "elua", "checkme",
                               PACKAGE_BIN_DIR, "", PACKAGE_DATA_DIR,
                               LOCALE_DIR);

   if (!_elua_pfx)
     {
        ERR("coul not find elua prefix");
        return EINA_FALSE;
     }

   return ++_elua_init_counter;
}

EAPI int
elua_shutdown(void)
{
   if (_elua_init_counter <= 0)
     {
        EINA_LOG_ERR("Init count not greater than 0 in shutdown.");
        return EINA_FALSE;
     }
   --_elua_init_counter;

   if (_elua_init_counter > 0)
     return _elua_init_counter;

   INF("shutdown");
   eina_log_timing(_elua_log_dom, EINA_LOG_STATE_START, EINA_LOG_STATE_SHUTDOWN);

   eina_prefix_free(_elua_pfx);
   _elua_pfx = NULL;

   eina_log_domain_unregister(_elua_log_dom);
   _elua_log_dom = -1;

   eina_shutdown();
   return _elua_init_counter;
}

EAPI Elua_State *
elua_state_new(void)
{
   Elua_State *ret = NULL;
   lua_State *L = luaL_newstate();
   if (!L)
     return NULL;
   ret = calloc(1, sizeof(Elua_State));
   ret->luastate = L;
   luaL_openlibs(L);
   lua_pushlightuserdata(L, ret);
   lua_setfield(L, LUA_REGISTRYINDEX, "elua_ptr");
   return ret;
}

EAPI void
elua_state_free(Elua_State *es)
{
   if (!es) return;
   if (es->luastate) lua_close(es->luastate);
   eina_stringshare_del(es->coredir);
   eina_stringshare_del(es->moddir);
   eina_stringshare_del(es->appsdir);
   free(es);
}

EAPI void
elua_state_dirs_set(Elua_State *es, const char *core, const char *mods,
                    const char *apps)
{
   EINA_SAFETY_ON_NULL_RETURN(es);
   if (core) es->coredir = eina_stringshare_add(core);
   if (mods) es->moddir  = eina_stringshare_add(mods);
   if (apps) es->appsdir = eina_stringshare_add(apps);
}

EAPI Eina_Stringshare *
elua_state_core_dir_get(Elua_State *es)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(es, NULL);
   return es->coredir;
}

EAPI Eina_Stringshare *
elua_state_mod_dir_get(Elua_State *es)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(es, NULL);
   return es->moddir;
}

EAPI Eina_Stringshare *
elua_state_apps_dir_get(Elua_State *es)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(es, NULL);
   return es->moddir;
}

EAPI Elua_State *
elua_state_from_lua_get(lua_State *L)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(L, NULL);
   lua_getfield(L, LUA_REGISTRYINDEX, "elua_ptr");
   if (!lua_isnil(L, -1))
     {
        void *st = lua_touserdata(L, -1);
        lua_pop(L, 1);
        return (Elua_State *)st;
     }
   lua_pop(L, 1);
   return NULL;
}

static void
_elua_errmsg(const char *pname, const char *msg)
{
   ERR("%s%s%s", pname ? pname : "", pname ? ": " : "", msg);
}

EAPI int
elua_report_error(Elua_State *es, const char *pname, int status)
{
   EINA_SAFETY_ON_FALSE_RETURN_VAL(es && es->luastate, status);
   if (status && !lua_isnil(es->luastate, -1))
     {
        const char *msg = lua_tostring(es->luastate, -1);
        _elua_errmsg(pname, msg ? msg : "(non-string error)");
        lua_pop(es->luastate, 1);
     }
   return status;
}

static int
_elua_gettext_bind_textdomain(lua_State *L)
{
#ifdef ENABLE_NLS
   const char *textdomain = luaL_checkstring(L, 1);
   const char *dirname    = luaL_checkstring(L, 2);
   const char *ret;
   if (!textdomain[0] || !strcmp(textdomain, PACKAGE))
     {
        lua_pushnil(L);
        lua_pushliteral(L, "invalid textdomain");
        return 2;
     }
   if (!(ret = bindtextdomain(textdomain, dirname)))
     {
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

const luaL_reg gettextlib[] =
{
   { "bind_textdomain", _elua_gettext_bind_textdomain },
   { NULL, NULL }
};

EAPI void
elua_state_setup_i18n(Elua_State *es)
{
#ifdef ENABLE_NLS
   char *(*dgettextp)(const char*, const char*) = dgettext;
   char *(*dngettextp)(const char*, const char*, const char*, unsigned long)
      = dngettext;
#endif
   lua_createtable(es->luastate, 0, 0);
   luaL_register(es->luastate, NULL, gettextlib);
#ifdef ENABLE_NLS
   lua_pushlightuserdata(es->luastate, *((void**)&dgettextp));
   lua_setfield(es->luastate, -2, "dgettext");
   lua_pushlightuserdata(es->luastate, *((void**)&dngettextp));
   lua_setfield(es->luastate, -2, "dngettext");
#endif
}
