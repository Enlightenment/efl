#undef EAPI_MAIN
#undef __EFL_UI
#undef __EFL_NET

#ifdef _WIN32
// There is no support for quicklaunch on windows, so no needs
// to export the efl_main symbol
# define EAPI_MAIN
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EAPI_MAIN __attribute__ ((visibility("default")))
#  else
#   define EAPI_MAIN
#  endif
# else
#  define EAPI_MAIN
# endif
#endif /* ! _WIN32 */

#ifdef __EFL_UI_IS_REQUIRED
# define __EFL_UI(...) __VA_ARGS__
# define __EFL_UI_IS_DEFINED
#else
# define __EFL_UI(...)
#endif

#ifdef __EFL_NET_IS_REQUIRED
# define __EFL_NET(...) __VA_ARGS__
#else
# define __EFL_NET(...)
#endif

#ifdef EFL_BETA_API_SUPPORT

// This file is designed to be included again and again
// so cleanup last inclusion before generating this one.
#undef _EFL_VERSION_MICRO
#undef _EFL_VERSION_REVISION
#undef _EFL_VERSION_FLAVOR
#undef _EFL_BUILD_ID
#undef _EFL_APP_VERSION_SET
#undef __EFL_MAIN_CONSTRUCTOR
#undef __EFL_MAIN_DESTRUCTOR
#undef EFL_MAIN
#undef EFL_MAIN_EX

#ifdef EFL_VERSION_MICRO
# define _EFL_VERSION_MICRO EFL_VERSION_MICRO
#else
# define _EFL_VERSION_MICRO 0
#endif

#ifdef EFL_VERSION_REVISION
# define _EFL_VERSION_REVISION EFL_VERSION_REVISION
#else
# define _EFL_VERSION_REVISION 0
#endif

#ifdef EFL_VERSION_FLAVOR
# define _EFL_VERSION_FLAVOR EFL_VERSION_FLAVOR
#else
# define _EFL_VERSION_FLAVOR NULL
#endif

#ifdef EFL_BUILD_ID
# define _EFL_BUILD_ID EFL_BUILD_ID
#else
# define _EFL_BUILD_ID NULL
#endif

#define __EFL_MAIN_CONSTRUCTOR                  \
  __EFL_NET(ecore_con_init();)                  \
  __EFL_NET(ecore_con_url_init();)              \
  __EFL_UI(elm_init(argc, argv);)

#define __EFL_MAIN_DESTRUCTOR                   \
  __EFL_UI(elm_shutdown();)                     \
  __EFL_NET(ecore_con_url_shutdown();)          \
  __EFL_NET(ecore_con_shutdown();)

#define _EFL_APP_VERSION_SET()                                          \
  do {                                                                  \
     if (efl_build_version_set)                                         \
       efl_build_version_set(EFL_VERSION_MAJOR, EFL_VERSION_MINOR, _EFL_VERSION_MICRO, \
                             _EFL_VERSION_REVISION, _EFL_VERSION_FLAVOR, _EFL_BUILD_ID); \
  } while (0)

#define EFL_MAIN() int main(int argc, char **argv)                      \
  {                                                                     \
     Eina_Value *ret__;                                                 \
     int real__;                                                        \
     _efl_startup_time = ecore_time_unix_get();                         \
     _EFL_APP_VERSION_SET();                                            \
     ecore_init();                                                      \
     efl_event_callback_add(efl_main_loop_get(), EFL_LOOP_EVENT_ARGUMENTS, efl_main, NULL); \
     ecore_init_ex(argc, argv);                                         \
     __EFL_MAIN_CONSTRUCTOR;                                            \
     ret__ = efl_loop_begin(efl_main_loop_get());                     \
     real__ = efl_loop_exit_code_process(ret__);                        \
     __EFL_MAIN_DESTRUCTOR;                                             \
     ecore_shutdown_ex();                                               \
     ecore_shutdown();                                                  \
     return real__;                                                     \
  }

#define EFL_MAIN_EX()                                                   \
  EFL_CALLBACKS_ARRAY_DEFINE(_efl_main_ex,                              \
                             { EFL_LOOP_EVENT_ARGUMENTS, efl_main },    \
                             { EFL_APP_EVENT_PAUSE, efl_pause },       \
                             { EFL_APP_EVENT_RESUME, efl_resume },     \
                             { EFL_APP_EVENT_TERMINATE, efl_terminate });         \
  int main(int argc, char **argv)                                       \
  {                                                                     \
     Eina_Value *ret__;                                                 \
     int real__;                                                        \
     _efl_startup_time = ecore_time_unix_get();                         \
     _EFL_APP_VERSION_SET();                                            \
     ecore_init();                                                      \
     efl_event_callback_array_add(efl_main_loop_get(), _efl_main_ex(), NULL); \
     ecore_init_ex(argc, argv);                                         \
     __EFL_MAIN_CONSTRUCTOR;                                            \
     ret__ = efl_loop_begin(efl_main_loop_get());                     \
     real__ = efl_loop_exit_code_process(ret__);                        \
     __EFL_MAIN_DESTRUCTOR;                                             \
     ecore_shutdown_ex();                                               \
     ecore_shutdown();                                                  \
     return real__;                                                     \
  }

#endif /* EFL_BETA_API_SUPPORT */
