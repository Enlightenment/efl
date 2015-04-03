#ifndef ECTOR_H_
#define ECTOR_H_

#include <Eina.h>
#include <Eo.h>

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef EFL_ECTOR_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif /* ! DLL_EXPORT */
# else
#  define EAPI __declspec(dllimport)
# endif /* ! EFL_EO_BUILD */
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

/**
 * @page ector_main Ector
 *
 * @date 2014 (created)
 *
 * @section toc Table of Contents
 *
 * @li @ref ector_main_intro
 * @li @ref ector_main_compiling
 * @li @ref ector_main_next_steps
 * @li @ref ector_main_intro_example
 *
 * @section ector_main_intro Introduction
 *
 * Ector is a retained mode drawing library that is designed to work
 * for and with an scenegraph like Evas.
 *
 * @section ector_main_compiling How to compile
 *
 * Ector is a library your application links to. The procedure for this is
 * very simple. You simply have to compile your application with the
 * appropriate compiler flags that the @c pkg-config script outputs. For
 * example:
 *
 * Compiling C or C++ files into object files:
 *
 * @verbatim
 gcc -c -o main.o main.c `pkg-config --cflags ector`
 @endverbatim
 *
 * Linking object files into a binary executable:
 *
 * @verbatim
 gcc -o my_application main.o `pkg-config --libs ector`
 @endverbatim
 *
 * See @ref pkgconfig
 *
 * @section ector_main_next_steps Next Steps
 *
 * After you understood what Ector is and installed it in your system
 * you should proceed understanding the programming interface.
 *
 * Recommended reading:
 *
 * @li @ref Ector_Surface
 * @li @ref Ector_Renderer
 *
 * @section ector_main_intro_example Introductory Example
 *
 * @ref Ector_Tutorial
 *
 *
 * @addtogroup Ector
 * @{
 */

/**
 * @typedef Ector_Surface
 * The base type to render content into.
 */
typedef Eo Ector_Surface;

/**
 * @typedef Ector_Renderer
 * The base type describing what to render.
 */
typedef Eo Ector_Renderer;

#ifdef EFL_BETA_API_SUPPORT

/**
 * @brief Init the ector subsystem
 * @return @c EINA_TRUE on success.
 *
 * @see ector_shutfown()
 */
EAPI int ector_init(void);

/**
 * @brief Shutdown the ector subsystem
 * @return @c EINA_TRUE on success.
 *
 * @see ector_init()
 */
EAPI int ector_shutdown(void);

#endif

/**
 * @}
 */


#ifdef __cplusplus
}
#endif

#endif
