#ifndef ECTOR_H_
#define ECTOR_H_

#include <Eina.h>
#include <Eo.h>
#include <Efl.h>

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

#ifdef EFL_BETA_API_SUPPORT

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

/**
 * @typedef Ector_Colorspace
 * The definiton of colorspace.
 */
  // FIXME: Enable that when we have merged Emile
/* typedef Evas_Colorspace Ector_Colorspace; */

/**
 * Priorities
 */
typedef enum _Ector_Priority
{
  ECTOR_PRIORITY_NONE = 0,
  ECTOR_PRIORITY_MARGINAL = 64,
  ECTOR_PRIORITY_SECONDARY = 128,
  ECTOR_PRIORITY_PRIMARY = 256,
} Ector_Priority;

/**
 * What kind of update is being pushed
 */
typedef enum _Ector_Update_Type
{
  ECTOR_UPDATE_BACKGROUND = 1, /* All the previous state in that area is reset to the new updated profile */
  ECTOR_UPDATE_EMPTY = 2, /* Pushing empty area (no visible pixels at all, no need to read this surface to render it) */
  ECTOR_UPDATE_ALPHA = 4, /* Pushing some transparent pixels (this impact the under layer and will require to read back the surface where this surface is blitted) */
  ECTOR_UPDATE_OPAQUE = 8 /* Pushing some opaque pixels (this means that their is no need to read the under layer when blitting this surface) */
} Ector_Update_Type;

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

/**
 * @brief Registers OpenGL API calls with the internal Ector_GL_API.
 *
 * @param glsym Function to use for looking up dynamically loaded symbols
 * @param lib Dynamically loaded shared object, or RTLD_DEFAULT or RTLD_NEXT
 * @return EINA_TRUE if call succeeded, EINA_FALSE if glsym was undefined or an error occurred
 *
 * The RTLD_DEFAULT and RTLD_NEXT pseudo-handles can be passed as lib to
 * look up the first or next occurance of the desired symbol in the dynamic
 * library search order.
 *
 * @see dlsym()
 */
EAPI Eina_Bool ector_glsym_set(void *(*glsym)(void *lib, const char *name), void *lib);

/* Avoid redefinition of types */
#define _ECTOR_SURFACE_EO_CLASS_TYPE
#define _ECTOR_RENDERER_EO_CLASS_TYPE

#include "ector_surface.h"
#include "ector_renderer.h"
#include "ector_util.h"

#endif

/**
 * @}
 */


#ifdef __cplusplus
}
#endif

#endif
