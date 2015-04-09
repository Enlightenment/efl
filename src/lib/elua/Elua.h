/**
 * @file Elua.h
 * @brief Elua Library
 *
 * @defgroup Elua Elua
 */

/**
 *
 * @section intro Elua library
 *
 * The Elua library was created to ease integration of EFL Lua into other EFL
 * libraries or applications. Using the Elua library you can easily create a
 * Lua state that is fully set up for running EFL Lua bindings.
 *
 * You can find the API documentation at @ref Elua
*/
#ifndef _ELUA_H
#define _ELUA_H

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef EFL_ELUA_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif /* ! DLL_EXPORT */
# else
#  define EAPI __declspec(dllimport)
# endif /* ! EFL_ELUA_BUILD */
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EAPI __attribute__ ((visibility("default")))
#  else
#   define EAPI
#  endif
# else
#  define EAPI
# endif
#endif /* ! _WIN32 */

#ifdef __cplusplus
extern "C" {
#endif

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef EFL_BETA_API_SUPPORT

#include <Eina.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

typedef struct _Elua_State
{
   lua_State *luastate;
   Eina_Stringshare *coredir;
   Eina_Stringshare *moddir;
   Eina_Stringshare *appsdir;
} Elua_State;

EAPI int elua_init(void);
EAPI int elua_shutdown(void);

EAPI Elua_State *elua_state_new(void);
EAPI Elua_State *elua_state_from_lua_get(lua_State *L);

EAPI void elua_state_free(Elua_State *es);

EAPI void elua_state_dirs_set(Elua_State *es, const char *core,
                              const char *mods, const char *apps);

EAPI Eina_Stringshare *elua_state_core_dir_get(const Elua_State *es);
EAPI Eina_Stringshare *elua_state_mod_dir_get(const Elua_State *es);
EAPI Eina_Stringshare *elua_state_apps_dir_get(const Elua_State *es);

EAPI int elua_report_error(const Elua_State *es, const char *pname, int status);

EAPI void elua_state_setup_i18n(const Elua_State *es);

EAPI int elua_io_loadfile(const Elua_State *es, const char *fname);
EAPI void elua_io_register(const Elua_State *es);

EAPI int elua_io_popen(lua_State *L);

#endif

#ifdef __cplusplus
} // extern "C" {
#endif

#endif
