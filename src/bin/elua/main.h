#ifndef LUAR_MAIN_H
#define LUAR_MAIN_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#if ENABLE_NLS
# include <locale.h>
# include <libintl.h>
# define _(x) dgettext(PACKAGE, x)
#else
# define _(x) (x)
#endif

#include <stdio.h>
#include <stdlib.h>

#include <Eina.h>
#include <Ecore.h>

#ifdef HAVE_EVIL
#include <Evil.h>
#endif

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

extern int el_log_domain;

#define DBG(...) EINA_LOG_DOM_DBG(el_log_domain, __VA_ARGS__)
#define INF(...) EINA_LOG_DOM_INFO(el_log_domain, __VA_ARGS__)
#define WRN(...) EINA_LOG_DOM_WARN(el_log_domain, __VA_ARGS__)
#define ERR(...) EINA_LOG_DOM_ERR(el_log_domain, __VA_ARGS__)
#define CRT(...) EINA_LOG_DOM_CRITICAL(el_log_domain, __VA_ARGS__)

int elua_loadfilex(lua_State *L, const char *fname, const char *mode);
int elua_loadfile(lua_State *L, const char *fname);
void elua_register_cache(lua_State *L);

#endif
