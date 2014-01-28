/**
@brief Edje Graphical Design Library

These routines are used for Edje.

@page edje_main Edje

@date 2003 (created)

@section toc Table of Contents

@li @ref edje_main_intro
@li @ref edje_main_work
@li @ref edje_main_compiling
@li @ref edje_main_next_steps
@li @ref edje_main_intro_example

@section edje_main_intro Introduction

Edje is a complex graphical design & layout library.

It doesn't intend to do containing and regular layout like a widget
set, but it is the base for such components. Based on the requirements
of Enlightenment 0.17, Edje should serve all the purposes of creating
visual elements (borders of windows, buttons, scrollbars, etc.) and
allow the designer the ability to animate, layout and control the look
and feel of any program using Edje as its basic GUI constructor. This
library allows for multiple collections of Layouts in one file,
sharing the same image and font database and thus allowing a whole
theme to be conveniently packaged into 1 file and shipped around.

Edje separates the layout and behavior logic. Edje files ship with an
image and font database, used by all the parts in all the collections
to source graphical data. It has a directory of logical part names
pointing to the part collection entry ID in the file (thus allowing
for multiple logical names to point to the same part collection,
allowing for the sharing of data between display elements). Each part
collection consists of a list of visual parts, as well as a list of
programs. A program is a conditionally run program that if a
particular event occurs (a button is pressed, a mouse enters or leaves
a part) will trigger an action that may affect other parts. In this
way a part collection can be "programmed" via its file as to hilight
buttons when the mouse passes over them or show hidden parts when a
button is clicked somewhere etc. The actions performed in changing
from one state to another are also allowed to transition over a period
of time, allowing animation. Programs and animations can be run in
"parallel".

This separation and simplistic event driven style of programming can produce
almost any look and feel one could want for basic visual elements. Anything
more complex is likely the domain of an application or widget set that may
use Edje as a convenient way of being able to configure parts of the display.

For details of Edje's history, see the @ref edje_history section.

@subsection edje_main_work So how does this all work?

Edje internally holds a geometry state machine and state graph of what is
visible, not, where, at what size, with what colors etc. This is described
to Edje from an Edje .edj file containing this information. These files can
be produced by using edje_cc to take a text file (a .edc file) and "compile"
an output .edj file that contains this information, images and any other
data needed.

The application using Edje will then create an object in its Evas
canvas and set the bundle file to use, specifying the @b group name to
use. Edje will load such information and create all the required
children objects with the specified properties as defined in each @b
part of the given group. See the example at @ref edje_main_intro_example.

Although simple, this example illustrates that animations and state
changes can be done from the Edje file itself without any requirement
in the C application.

Before digging into changing or creating your own Edje source (edc)
files, read the @ref edcref.

@subsection edje_history Edje History

It's a sequel to "Ebits" which has serviced the needs of Enlightenment
development for early version 0.17. The original design parameters under
which Ebits came about were a lot more restricted than the resulting
use of them, thus Edje was born.

Edje is a more complex layout engine compared to Ebits. It doesn't
pretend to do containing and regular layout like a widget set. It
still inherits the more simplistic layout ideas behind Ebits, but it
now does them a lot more cleanly, allowing for easy expansion, and the
ability to cover much more ground than Ebits ever could. For the
purposes of Enlightenment 0.17, Edje was conceived to serve all the
purposes of creating visual elements (borders of windows, buttons,
scrollbars, etc.) and allow the designer the ability to animate,
layout and control the look and feel of any program using Edje as its
basic GUI constructor.

Unlike Ebits, Edje separates the layout and behavior logic.

@section edje_main_compiling How to compile

Edje is a library your application links to. The procedure for this is
very simple. You simply have to compile your application with the
appropriate compiler flags that the @c pkg-config script outputs. For
example:

Compiling C or C++ files into object files:

@verbatim
gcc -c -o main.o main.c `pkg-config --cflags edje`
@endverbatim

Linking object files into a binary executable:

@verbatim
gcc -o my_application main.o `pkg-config --libs edje`
@endverbatim

See @ref pkgconfig

@section edje_main_next_steps Next Steps

After you understood what Edje is and installed it in your system you
should proceed understanding the programming interface for all
objects, then see the specific for the most used elements. We'd
recommend you to take a while to learn @ref Ecore, @ref Evas, @ref Eo
and @ref Eina as they are the building blocks for Edje. There is a
widget set built on top of Edje providing high level elements such as
buttons, lists and selectors called Elementary
(http://docs.enlightenment.org/auto/elementary/) as they will likely
save you tons of work compared to using just Evas directly.

Recommended reading:
@li @ref edcref
@li @ref Edje_General_Group
@li @ref Edje_Object_Group
@li @ref Edje_External_Group
@li @ref luaref

@section edje_main_intro_example Introductory Example

What follows is a list with various commented examples, covering a great
part of Edje's API:

@include edje_example.c

The above example requires the following annotated source Edje file:
@include edje_example.edc


More examples can be found at @ref edje_examples.
*/

/**

@example embryo_custom_state.edc
This example shows how to create a custom state from embryo. Clicking on the
3 labels will rotate the object in the given direction.

@example embryo_pong.edc
Super-simple Pong implementation in pure embryo.

@example embryo_run_program.edc
This example shows how to run an edje program from embryo code.

@example embryo_set_state.edc
This example shows how to change the state of a part from embryo code.

@example embryo_set_text.edc
This example shows how to set the text in TEXT part from embryo code.

@example embryo_timer.edc
This example shows the usage of timers in embryo.

@example external_elm_anchorblock.edc
This example use an elementary anchorblock and a button to animate the text.

@example external_elm_button.edc
This example create some elementary buttons and do some actions on user click.

@example external_elm_check.edc
This example shows EXTERNAL checkbox in action.

@example external_elm_panes.edc
This example shows EXTERNAL elementary panes in action.

@example external_emotion_elm.edc
Super-concise video player example using Edje/Emotion/Elementary.

@example lua_script.edc
This example shows the usage of lua scripting to create and animate some
objects in the canvas.

@example toggle_using_filter.edc
This example shows how to toggle the state of a part using the 'filter'
param in edje programs

*/

/**
 * @file Edje.h
 * @brief Edje Graphical Design Library
 *
 * These routines are used for Edje.
 */

#ifndef _EDJE_H
#define _EDJE_H

#ifndef _MSC_VER
# include <stdint.h>
#endif
#include <math.h>
#include <float.h>
#include <limits.h>

#include <Evas.h>
#include <Eo.h>
#include <Efl_Config.h>

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef EFL_EDJE_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif /* ! DLL_EXPORT */
# else
#  define EAPI __declspec(dllimport)
# endif /* ! EFL_EDJE_BUILD */
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

#define EDJE_VERSION_MAJOR EFL_VERSION_MAJOR
#define EDJE_VERSION_MINOR EFL_VERSION_MINOR

#include "Edje_Common.h"
#ifndef EFL_NOLEGACY_API_SUPPORT
#include "Edje_Legacy.h"
#endif
#ifdef EFL_EO_API_SUPPORT
#include "Edje_Eo.h"
#endif

#ifdef __cplusplus
}
#endif

#endif
