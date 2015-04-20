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

/**
 * @page elua_main Elua library (BETA)
 *
 * @date 2015 (created)
 *
 * @section toc Table of Contents
 *
 * @li @ref elua_main_intro
 * @li @ref elua_main_compiling
 * @li @ref elua_main_next_steps
 *
 * @section elua_main_intro Introduction
 *
 * The Elua library provides all necessary infrastructure required to set up
 * a fully functional Lua state able of running Elua scripts. This is provided
 * as a library in order to encourage reuse from different libraries and apps.
 *
 * @section elua_main_compiling How to compile
 *
 * As Elua is a library, compiling is very simple.
 *
 * Compiling C or C++ files into object files:
 *
 * @verbatim
   gcc -c -o main.o main.c `pkg-config --cflags elua`
   @endverbatim
 *
 * Linking object files into a binary executable:
 *
 * @verbatim
   gcc -o my_application main.o `pkg-config --libs elua`
   @endverbatim
 *
 * See @ref pkgconfig
 *
 * @section elua_main_next_steps Next Steps
 *
 * There is a comperehensive API reference available that should get you up
 * and running.
 *
 * @addtogroup Elua
 * @{
 */

#ifdef EFL_BETA_API_SUPPORT

#include <Eina.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

/** Opaque Elua state
 *
 * @ingroup Elua
 */
typedef struct _Elua_State Elua_State;

/**
 * @brief Initialize the Elua library.
 *
 * This initializes the Elua library for usage. It maintains an internal
 * counter so that multiple calls will only increment/decrement correctly.
 *
 * @see elua_shutdown
 *
 * @ingroup Elua
 */
EAPI int elua_init(void);

/**
 * @brief Shutdown the Elua library.
 *
 * Depending on the internal initialization counter, this either decrements
 * or completely shuts down the Elua library. In any case, call this once for
 * each init call.
 *
 * @see elua_init
 *
 * @ingroup Elua
 */
EAPI int elua_shutdown(void);

/**
 * @brief Create a new Elua state.
 *
 * This creates a new Elua state. An Elua state is externally opaque, but
 * it contains a LuaJIT state as well as some additional information that
 * is mostly initialized by other APIs.
 *
 * @param[in] progname The program name that holds the Elua state. This will
 * be used for stuff like error reporting. Typically the same as the binary
 * name of the application (argv[0]).
 * @return A new Elua state or NULL.
 *
 * @ingroup Elua
 */
EAPI Elua_State *elua_state_new(const char *progname);

/**
 * @brief Retrieve an Elua state from a Lua state.
 *
 * This doesn't create a new Elua state. Instead it just retrieves an existing
 * Elua state given a Lua state. If no Elua state could be found (for example
 * when the Lua state was created independently of Elua), this function returns
 * NULL.
 *
 * @param[in] L The Lua state.
 * @return An Elua state or NULL.
 *
 * @ingroup Elua
 */
EAPI Elua_State *elua_state_from_lua_state_get(lua_State *L);

/**
 * @brief Destroy an Elua state.
 *
 * Given an Elua state, this destroys its internal Lua state as well as all
 * other data its holding and then frees the Elua state itself.
 *
 * @param[in] es The Elua state.
 *
 * @ingroup Elua
 */
EAPI void elua_state_free(Elua_State *es);

/**
 * @brief Set the Elua directory paths.
 *
 * Every Elua state needs three paths - the core script path, the module
 * path and the apps path. The core path refers to from where core scripts
 * will be loaded (such as the module system), the module path refers to from
 * where extra modules will be loaded and the apps path refers to from where
 * Elua applications will be loaded (this is not a module path).
 *
 * If you provide NULL for any path, it will not be set. This allows you to
 * split the setting into multiple calls. By the time of state use all need
 * to be set.
 *
 * @param[in] es The Elua state.
 * @param[in] core The core path.
 * @param[in] mods The modules path.
 * @param[in] apps The apps path.
 *
 * @see elua_state_core_dir_get
 * @see elua_state_mod_dir_get
 * @see elua_state_apps_dir_get
 *
 * @ingroup Elua
 */
EAPI void elua_state_dirs_set(Elua_State *es, const char *core,
                              const char *mods, const char *apps);

/**
 * @brief Fill the currently unset Elua dirs.
 *
 * This checks if any of the three main paths are unset and tries to fill
 * them from the environment. It first tries environment variables to fill
 * them (ELUA_CORE_DIR, ELUA_MODULES_DIR, ELUA_APPS_DIR) unless the ignore_env
 * param is EINA_TRUE. If it is (or if the environment vars weren't set right)
 * it uses eina prefix of the library to determine the paths. In that case
 * they will expand to DATADIR/core, DATADIR/modules and DATADIR/apps, where
 * DATADIR is typically something like /usr/share/elua.
 *
 * @param[in] es The Elua state.
 * @param[in] ignore_env If set to EINA_TRUE, this ignores the env vars.
 *
 * @ingroup Elua
 */
EAPI void elua_state_dirs_fill(Elua_State *es, Eina_Bool ignore_env);

/**
 * @brief Retrieve the Elua core dir.
 *
 * @param[in] es The Elua state.
 * @return The path.
 *
 * @ingroup Elua
 */
EAPI Eina_Stringshare *elua_state_core_dir_get(const Elua_State *es);

/**
 * @brief Retrieve the Elua module dir.
 *
 * @param[in] es The Elua state.
 * @return The path.
 *
 * @ingroup Elua
 */
EAPI Eina_Stringshare *elua_state_mod_dir_get(const Elua_State *es);

/**
 * @brief Retrieve the Elua apps dir.
 *
 * @param[in] es The Elua state.
 * @return The path.
 *
 * @ingroup Elua
 */
EAPI Eina_Stringshare *elua_state_apps_dir_get(const Elua_State *es);

/**
 * @brief Retrieve the prog name set on state creation.
 *
 * @param[in] es The Elua state.
 * @return The name.
 *
 * @ingroup Elua
 */
EAPI Eina_Stringshare *elua_state_prog_name_get(const Elua_State *es);

/**
 * @brief Add another path to look up modules in to the state.
 *
 * @param[in] es The Elua state.
 *
 * @ingroup Elua
 */
EAPI void elua_state_include_path_add(Elua_State *es, const char *path);

/**
 * @brief Push the Elua "require" function onto the Lua stack.
 *
 * @param[in] es The Elua state.
 * @return EINA_TRUE if the push was successful, EINA_FALSE otherwise.
 *
 * @ingroup Elua
 */
EAPI Eina_Bool elua_state_require_ref_push(Elua_State *es);

/**
 * @brief Push the Elua app loader function onto the Lua stack.
 *
 * @param[in] es The Elua state.
 * @return EINA_TRUE if the push was successful, EINA_FALSE otherwise.
 *
 * @ingroup Elua
 */
EAPI Eina_Bool elua_state_appload_ref_push(Elua_State *es);

/**
 * @brief Retrieve the Lua state from an Elua state.
 *
 * This function retrieves the Lua state from a valid Elua state. As an
 * Elua state is always initialized, this will return a valid state, unless
 * the given Elua state is NULL, in which case it will also return NULL.
 *
 * @param[in] es The Elua state.
 * @return The Lua state or NULL.
 *
 * @ingroup Elua
 */
EAPI lua_State *elua_state_lua_state_get(const Elua_State *es);

/**
 * @brief Set up internationalization support for an Elua state.
 *
 * This function sets up correct i18n for an Elua state. That means loading
 * the gettext bindings and making Lua aware of them. This also works when
 * i18n support is disabled at compilation time, so you can just call it
 * unconditionally.
 *
 * @param[in] es The Elua state.
 * @return EINA_TRUE on success, EINA_FALSE on failure.
 *
 * @ingroup Elua
 */
EAPI Eina_Bool elua_state_i18n_setup(const Elua_State *es);

/**
 * @brief Set up module support for an Elua state.
 *
 * This loads the Elua module system and makes Lua aware of it. It also
 * registers the Elua C utility library module.
 *
 * @param[in] es The Elua state.
 * @return EINA_TRUE on success, EINA_FALSE on failure.
 *
 * @ingroup Elua
 */
EAPI Eina_Bool elua_state_modules_setup(const Elua_State *es);
EAPI Eina_Bool elua_state_io_setup(const Elua_State *es);

EAPI int elua_io_loadfile(const Elua_State *es, const char *fname);

EAPI int elua_util_require(Elua_State *es, const char *libname);
EAPI int elua_util_file_run(Elua_State *es, const char *fname);
EAPI int elua_util_string_run(Elua_State *es, const char *chunk,
                              const char *chname);
EAPI Eina_Bool elua_util_app_load(Elua_State *es, const char *appname);
EAPI int elua_util_script_run(Elua_State *es, int argc, char **argv, int n,
                              int *quit);

EAPI int elua_util_error_report(const Elua_State *es, const char *pname,
                                int status);

#endif

#ifdef __cplusplus
} // extern "C" {
#endif

#endif
