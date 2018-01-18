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
# ifdef EFL_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif
# else
#  define EAPI __declspec(dllimport)
# endif
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
#endif

#ifdef __cplusplus
extern "C" {
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
 * Also, all the paths will be sanitized before setting by calling
 * @ref eina_file_path_sanitize on them.
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
 * Also, all the paths will be sanitized before setting by calling
 * @ref eina_file_path_sanitize on them.
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
 * The path will be sanitized using @ref eina_file_path_sanitize.
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
 * @brief Set up the Elua state.
 *
 * This API function sets up 3 things, module system, i18n and I/O. After that
 * it requires all modules not yet required (i.e. those queued in before the
 * state was fully initialized).
 *
 * This function sets up correct i18n for an Elua state. That means loading
 * the gettext bindings and making Lua aware of them. This also works when
 * i18n support is disabled at compilation time, so you can just call it
 * unconditionally.
 *
 * This also loads the Elua module system and makes Lua aware of it. It also
 * registers the Elua C utility library module.
 *
 * Finally, Elua provides its own loadfile based around mmap to replace the
 * less efficient Lua version. This function takes care of the setup.
 *
 * @param[in] es The Elua state.
 * @return EINA_TRUE on success, EINA_FALSE on failure.
 *
 * @ingroup Elua
 */
EAPI Eina_Bool elua_state_setup(Elua_State *es);

/**
 * @brief Loads a file using Elua's own mmap-based IO.
 *
 * This function behaves identically to luaL_loadfile when it comes to
 * semantics. The loaded file remains on the Lua stack. If the input
 * state is NULL, the return value is -1 and nothing is left on the stack.
 * On any different error, the error object is left on the stack and this
 * returns a value larger than zero (LUA_ERR*). On success, zero is returned.
 *
 * @param[in] es The Elua state.
 * @param[in] fname The file name.
 * @return 0 for no errors, a non-zero value for errors (-1 for NULL es).
 *
 * @ingroup Elua
 */
EAPI int elua_io_loadfile(const Elua_State *es, const char *fname);

/**
 * @brief Requires a module.
 *
 * Requires a Lua module. Leaves the Lua stack clean.
 *
 * @param[in] es The Elua state.
 * @param[in] libname The library name.
 * @return EINA_TRUE on success, EINA_FALSE on failure.
 *
 * @ingroup Elua
 */
EAPI Eina_Bool elua_util_require(Elua_State *es, const char *libname);

/**
 * @brief Runs a file.
 *
 * Runs a file. Uses the Elua mmapped file IO to load the file.
 *
 * @param[in] es The Elua state.
 * @param[in] fname The file name.
 * @return EINA_TRUE on success, EINA_FALSE on failure.
 *
 * @ingroup Elua
 */
EAPI Eina_Bool elua_util_file_run(Elua_State *es, const char *fname);

/**
 * @brief Runs a string.
 *
 * Runs a string.
 *
 * @param[in] es The Elua state.
 * @param[in] chunk The string to run.
 * @param[in] chname The chunk name to use for traceback/debug.
 * @return EINA_TRUE on success, EINA_FALSE on failure.
 *
 * @ingroup Elua
 */
EAPI Eina_Bool elua_util_string_run(Elua_State *es, const char *chunk,
                                    const char *chname);

/**
 * @brief Loads an application.
 *
 * This loads an app, respecting the app path set on state initialization.
 * Actually runs the app. If the input state is NULL, the return value is -1
 * nd nothing is left on the stack. On any different error, the error object
 * is left on the stack and this returns 1. On success, zero is returned
 * (and the return value from the app is left on the stack).
 *
 * @param[in] es The Elua state.
 * @param[in] appname The application name.
 * @return 0 for no errors, 1 on errors, -1 on null input.
 *
 * @ingroup Elua
 */
EAPI int elua_util_app_load(Elua_State *es, const char *appname);

/**
 * @brief Runs a script.
 *
 * This is a more complicated function that runs a script. It's a combination
 * of the previously mentioned util functions. It takes argc and argv, which
 * are typically given to the program, and an index of the first positional
 * arg in argv (i.e. not options). The value on this index is then used as
 * the potential name.
 *
 * If this name is either a dash or empty, the script is loaded from stdin.
 * If it's a value and a file with this name exists, the script is loaded from
 * the file. Otherwise, the name is treated to be an application name, and
 * is loaded from the application path.
 *
 * If all succeeds, this is then run, and a quit value is written into the
 * quit arg; if it's true (1), it means the app wants to exit immediately.
 * If it's false (0), it means the app likely wants to execute a main loop.
 *
 * @param[in] es The Elua state.
 * @param[in] argc The argument count.
 * @param[in] argv The arguments.
 * @param[in] n The index of the first positional argt.
 * @param[out] quit Whether to quit or run a main loop.
 * @return EINA_TRUE on success, EINA_FALSE on failure.
 *
 * @ingroup Elua
 */
EAPI Eina_Bool elua_util_script_run(Elua_State *es, int argc, char **argv,
                                    int n, int *quit);

/**
 * @brief Reports an error using Eina logging.
 *
 * If the given status is 0, this function just returns it. Otherwise, it takes
 * the topmost item on the Lua stack, converts it to string (if it cannot be
 * converted, a "(non-string error)" placeholder is used) and logs it out
 * as an error, together with the program name set on Elua state init.
 *
 * @param[in] es The Elua state.
 * @param[in] status The status code.
 * @return The status code.
 *
 * @ingroup Elua
 */
EAPI int elua_util_error_report(const Elua_State *es, int status);

#endif

#ifdef __cplusplus
} // extern "C" {
#endif

#undef EAPI
#define EAPI

#endif
