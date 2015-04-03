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
 * Raster operations at pixel level
 */
typedef enum _Ector_Rop
{
  ECTOR_ROP_BLEND, /**< D = S + D(1 - Sa) */
  ECTOR_ROP_COPY, /**< D = S */
  ECTOR_ROP_LAST
} Ector_Rop;

/**
 * Quality values
 */
typedef enum _Ector_Quality
{
  ECTOR_QUALITY_BEST, /**< Best quality */
  ECTOR_QUALITY_GOOD, /**< Good quality */
  ECTOR_QUALITY_FAST, /**< Lower quality, fastest */
  ECTOR_QUALITY_LAST
} Ector_Quality;

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
