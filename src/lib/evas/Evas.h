/**
   @page evas_main Evas

   @date 2000 (created)

   @section toc Table of Contents

   @li @ref evas_main_intro
   @li @ref evas_main_work
   @li @ref evas_main_compiling
   @li @ref evas_main_next_steps
   @li @ref evas_main_intro_example


   @section evas_main_intro Introduction

   Evas is a clean display canvas API for several target display systems
   that can draw anti-aliased text, smooth super- and sub-sampled scaled
   images, alpha-blend objects and more.

   It abstracts the graphics drawing characteristics of the display
   system by implementing a canvas where graphical objects can be
   created, manipulated, and modified.  It then handles the rendering
   pipeline in an optimal way for the underlying device in order to
   minimize redraws, via a programmatically efficient API.

   A design goal for the system is to run well at both small and large
   scale, and be portable from embedded systems to multi-CPU
   workstations.  Architecturally, this is achieved via 'backends' that
   provide the specialized display logic for specific devices.  As well,
   there are various compile options to exclude feature support not
   required for a target platform to help minimize disk and memory
   requirements.

   Evas can serve as a base for widget sets or toolkits
   (e.g. Elementary, http://docs.enlightenment.org/auto/elementary/) by
   handling pixel drawing and regional change reporting, but does not
   manage windows itself, nor deal with input or window update event
   propagation.  In other words, it is intended for use in drawing
   scrollbars, sliders, and push buttons but not for high-level logic of
   how the widget operates and behaves.  Under Enlightenment, window and
   widget management is handled by other software components, including
   @ref Ecore (see @ref Ecore_Evas_Group in particular); however Evas is
   designed to not be dependent on any particular main loop
   architecture, and also strives to be input and output system
   agnostic.

   Evas can be seen as a display system that stands somewhere between a
   widget set and an immediate mode display system. It retains basic
   display logic, but does very little high-level logic such as
   scrollbars, sliders, and push buttons.


   @section evas_main_work How does Evas work?

   The Evas canvas is a 'retained mode' renderer, which differs from the
   more traditional 'immediate mode' display and windowing systems by
   tracking drawing state information of its contained objects.

   In an immediate mode rendering system, each frame is drawn from
   scratch by having each drawing element redraw itself.  Once the
   commands are executed, the display system blits the frame to the
   screen but has no idea how to reproduce the image again, so the
   application has to run through the same sequence of drawing commands
   again.  Very little or no state is kept from one frame draw to the
   next; while this is simple it forces each application to manually
   optimize their graphics code.

   With retained mode systems like Evas, the application does not need
   to implement the display rendering code and associated logic, but
   merely updates the list of objects maintained in the canvas.  Evas is
   then able to optimize the processing and rendering of the visible
   elements, and is better able to avoid redraws due to occlusion or
   opacity.

   Evas is a structural system in which the programmer creates and
   manages display objects and their properties, and as a result of this
   higher level state management, the canvas is able to redraw the set of
   objects when needed to represent the current state of the canvas.

   For example, consider the pseudo code:

   @verbatim
   line_handle = create_line();
   set line_handle from position (0, 0) to position (100, 200);
   show line_handle;

   rectangle_handle = create_rectangle();
   move rectangle_handle to position (10, 30);
   resize rectangle_handle to size 40 x 470;
   show rectangle_handle;

   bitmap_handle = create_bitmap();
   scale bitmap_handle to size 100 x 100;
   move bitmap_handle to position (10, 30);
   show bitmap_handle;

   render scene;
   @endverbatim

   By expressing the drawing as a set of drawable objects, Evas is able
   to internally handle refreshing, updating, moving, resizing, showing,
   and hiding the objects, and to determine to most efficiently redraw
   the canvas and its contents to reflect the current state.  This
   permits the application to focus on the higher level logic, which
   both reduces the amount of coding and allows a more natural way of
   dealing with the display.  Importantly, abstracting the display logic
   like this also simplifies porting the application to different
   display systems, since its own code is less tied into how that system
   works.

   @section evas_main_compiling How to compile the library

   Evas compiles automatically within EFL's build system, and is
   automatically linked with @ref Ecore and other components that need
   it.  But it can also be built and used standalone, by compiling and
   linking your application with the compiler flags indicated by @c
   pkg-config.  For example:

   @verbatim
   gcc -c -o my_main.o my_main.c `pkg-config --cflags evas`

   gcc -o my_application my_main.o `pkg-config --libs evas`
   @endverbatim

   See @ref pkgconfig

   @section evas_main_next_steps Recommended reading

   @li @ref Ecore, @ref Edje, and @ref Elementary that provide higher
   level infrastructure and components for real world usage.
   @li @ref Evas_Object_Group for how to manipulate generic objects on
   an Evas canvas and handle the associated events.
   @li @ref Evas_Object_Rectangle, to learn about the most basic object
    type on Evas -- the rectangle.
   @li @ref Evas_Object_Polygon, to learn how to create polygon elements
    on the canvas.
   @li @ref Evas_Line_Group, to learn how to create line elements on the
    canvas.
   @li @ref Evas_Object_Image, to learn about image objects, over which
    Evas can do a plethora of operations.
   @li @ref Evas_Object_Text, to learn how to create textual elements on
    the canvas.
   @li @ref Evas_Object_Textblock, to learn how to create multiline
    textual elements on the canvas.

   @li @ref Evas_Smart_Object_Group and @ref Evas_Smart_Group, to define
    new objects that provide @b custom functions to handle clipping,
    hiding, moving, resizing, color setting and more. This includes
    simple grouping of objects that move together (see @ref
    Evas_Smart_Object_Clipped) and more complex widget-like intelligent
    behaviors such as buttons and check boxes.

   @section evas_main_intro_example Introductory Example

   @include evas-buffer-simple.c

   More examples can be found at @ref evas_examples.

 */

#ifndef _EVAS_H
#define _EVAS_H

#include <Efl_Config.h>

#include <time.h>

#include <Eina.h>

#include <Eo.h>
#ifdef EFL_BETA_API_SUPPORT
/* This include has been added to support Eo in Evas */
#include <Efl.h>
#endif

#include <Evas_Loader.h>

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

#include <interfaces/efl_gfx_types.eot.h>
#include <interfaces/efl_gfx_path.eo.h>
#include <interfaces/efl_input_types.eot.h>
#include <interfaces/efl_text_types.eot.h>
#include <Evas_Common.h>


#ifndef EFL_NOLEGACY_API_SUPPORT
#include <Evas_Legacy.h>
#endif
#ifdef EFL_BETA_API_SUPPORT
#include <Evas_Eo.h>
#endif

#ifdef __cplusplus
}
#endif

#undef EAPI
#define EAPI

#endif
