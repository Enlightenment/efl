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

static void
_elua_errmsg(const char *pname, const char *msg)
{
   ERR("%s%s%s", pname ? pname : "", pname ? ": " : "", msg);
}

EAPI int
elua_report_error(lua_State *L, const char *pname, int status)
{
   if (status && !lua_isnil(L, -1))
     {
        const char *msg = lua_tostring(L, -1);
        _elua_errmsg(pname, msg ? msg : "(non-string error)");
        lua_pop(L, 1);
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
elua_state_setup_i18n(lua_State *L)
{
#ifdef ENABLE_NLS
   char *(*dgettextp)(const char*, const char*) = dgettext;
   char *(*dngettextp)(const char*, const char*, const char*, unsigned long)
      = dngettext;
#endif
   lua_createtable(L, 0, 0);
   luaL_register(L, NULL, gettextlib);
#ifdef ENABLE_NLS
   lua_pushlightuserdata(L, *((void**)&dgettextp));
   lua_setfield(L, -2, "dgettext");
   lua_pushlightuserdata(L, *((void**)&dngettextp));
   lua_setfield(L, -2, "dngettext");
#endif
}
