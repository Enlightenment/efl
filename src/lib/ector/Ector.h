#ifndef ECTOR_H_
#define ECTOR_H_

#include <Eina.h>
#include <Eo.h>
#ifdef EFL_BETA_API_SUPPORT
#include <Efl.h>
#endif
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
 * Ector is a retained mode drawing library designed to work
 * for and with a scenegraph such as Evas, which supports several
 * types of rendering surface including software, cairo, and gl.
 *
 * @section ector_main_compiling How to compile the library
 *
 * Ector compiles automatically within EFL's build system, and is
 * automatically linked with other components that need it.  But it can
 * also be built and used standalone, by compiling and linking your
 * application with the compiler flags indicated by @c pkg-config.  For
 * example:
 *
 * @verbatim
 * gcc -c -o my_main.o my_main.c `pkg-config --cflags ector`
 *
 * gcc -o my_application my_main.o `pkg-config --libs ector`
 * @endverbatim
 *
 * See @ref pkgconfig
 *
 * @section ector_main_next_steps Recommended reading:
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
 * The definition of colorspace.
 */
  // FIXME: Enable this when we have merged Emile
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
  ECTOR_UPDATE_ALPHA = 4, /* Pushing some transparent pixels (this impacts the under layer and will require reading back the surface where this surface is blitted) */
  ECTOR_UPDATE_OPAQUE = 8 /* Pushing some opaque pixels (this means that there is no need to read the under layer when blitting this surface) */
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
 * look up the first or next occurrence of the desired symbol in the dynamic
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

#undef EAPI
#define EAPI

#endif
