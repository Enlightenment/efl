/**
@brief Edje Graphical Design Library

These routines are used for Edje.

@mainpage Edje Library Documentation
@image html  e.png
@version 1.1
@author Carsten Haitzler <raster@@rasterman.com>
@author Tilman Sauerbeck (tilman at code-monkey de)
@author ZigsMcKenzie <zigsmckenzie@@gmail.com>
@author Cedric BAIL <cedric.bail@@free.fr>
@author Brian Mattern <rephorm@@rephorm.com>
@author Mathieu Taillefumier <mathieu.taillefumier@@free.fr>
@author Tristan <blunderer@@gmail.com>
@author Gustavo Lima Chaves <glima@@profusion.mobi>
@author Bruno Dilly <bdilly@@profusion.mobi>
@author Fabiano Fidêncio <fidencio@@profusion.mobi>
@author Jihoon Kim <jihoon48.kim@@samsung.com>
@author Tiago Falcão <tiago@@profusion.mobi>
@author Davide Andreoli <dave@@gurumeditation.it>
@author Sebastian Dransfeld <sd@@tango.flipp.net>
@author Tom Hacohen <tom@@stosb.com>
@author Aharon Hillel <a.hillel@@partner.samsung.com>
@date 2003-2011










@section intro What is Edje?

Edje is a complex graphical design & layout library.

It doesn't pretend to do containing and regular layout like a widget
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

For details of Edje's history, see the \ref history section.








@section requirements What does Edje require?

Edje requires fairly little on your system. to use the Edje runtime library
you need:

  - Evas (library)
  - Ecore (library)
  - Eet (library)
  - Embryo (library)
  - Eina (library)

Evas needs to be build with the JPEG, PNG and EET image loaders enabled at a
minimum. Edje uses X for the test program, so you will need the SOFTWARE_X11
engine built into Evas as well. A suggested configure list is below in the
"cheat sheet" for Evas.

Ecore needs the ECORE, ECORE_EVAS and ECORE_X modules built at a minimum.
It's suggested to build all the Ecore modules, but the ECORE_FB modules is
definitely optional.

Eina, Eet and Embryo have no interesting options so just build and
install them.

It is suggested right now that you get the latest SVN versions of the
required libraries. You also need to build them in the right order and make
sure the right options are enabled in the required libraries. Here is a
quick "cheat sheet" on how to get started.

@verbatim
1. You need Eina from the trunk svn branch.

  svn co http://svn.enlightenment.org/svn/e/trunk/eina/
  cd eina
  ./autogen.sh
  ./configure
  make
  sudo make install
  cd

2. You need Eet from the trunk svn branch.

  svn co http://svn.enlightenment.org/svn/e/trunk/eet/
  cd eet
  ./autogen.sh
  ./configure
  make
  sudo make install
  cd

3. You need Evas from the trunk svn branch built with eet, png and jpeg loader support.

  svn co http://svn.enlightenment.org/svn/e/trunk/evas/
  cd evas
  ./autogen.sh
  ./configure --enable-image-loader-eet --enable-font-loader-eet --enable-image-loader-jpeg --enable-image-loader-png --enable-buffer
  make
  sudo make install
  cd

4. You need Ecore from the trunk svn branch built with ecore-x and ecore-evas.

  svn co http://svn.enlightenment.org/svn/e/trunk/ecore/
  cd ecore
  ./autogen.sh
  ./configure --enable-ecore-x --enable-ecore-evas --enable-ecore-evas-software-buffer --enable-ecore-evas-software-x11 --enable-ecore-evas-software-buffer
  make
  sudo make install
  cd

5. You need embryo from the trunk svn branch

  svn co http://svn.enlightenment.org/svn/e/trunk/embryo/
  cd embryo
  ./autogen.sh
  ./configure
  make
  sudo make install
  cd

@endverbatim









@section compiling How to compile and test Edje

Now you need to compile and install Edje.

@verbatim
  ./configure
  make
  sudo make install
@endverbatim

You now have it installed and ready to go, but you need input
data. There are lots of examples in SVN, the best one is
Enlightenment's own theme file.

You may use different tools to edit and view the generated ".edj"
files, for instance:

  - editje (http://trac.enlightenment.org/e/wiki/Editje)
  - edje_viewer (http://trac.enlightenment.org/e/wiki/Edje_Viewer)










@section details So how does this all work?

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
part of the given group. See the following annotated example:

@code

#include <Eina.h>
#include <Evas.h>
#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Edje.h>

#define WIDTH 320
#define HEIGHT 240

static Evas_Object *create_my_group(Evas *canvas, const char *text)
{
   Evas_Object *edje;

   edje = edje_object_add(canvas);
   if (!edje)
     {
	EINA_LOG_CRIT("could not create edje object!");
	return NULL;
     }

   if (!edje_object_file_set(edje, "edje_example.edj", "my_group"))
     {
	int err = edje_object_load_error_get(edje);
	const char *errmsg = edje_load_error_str(err);
	EINA_LOG_ERR("could not load 'my_group' from edje_example.edj: %s",
		     errmsg);

	evas_object_del(edje);
	return NULL;
     }

   if (text)
     {
	if (!edje_object_part_text_set(edje, "text", text))
	  {
	     EINA_LOG_WARN("could not set the text. "
			   "Maybe part 'text' does not exist?");
	  }
     }

   evas_object_move(edje, 0, 0);
   evas_object_resize(edje, WIDTH, HEIGHT);
   evas_object_show(edje);
   return edje;
}

int main(int argc, char *argv[])
{
   Ecore_Evas *window;
   Evas *canvas;
   Evas_Object *edje;
   const char *text;

   eina_init();
   evas_init();
   ecore_init();
   ecore_evas_init();
   edje_init();

   window = ecore_evas_new(NULL, 0, 0, WIDTH, HEIGHT, NULL);
   if (!window)
     {
	EINA_LOG_CRIT("could not create window.");
	return -1;
     }
   canvas = ecore_evas_get(window);

   text = (argc > 1) ? argv[1] : NULL;

   edje = create_my_group(canvas, text);
   if (!edje)
     return -2;

   ecore_evas_show(window);
   ecore_main_loop_begin();

   evas_object_del(edje);
   ecore_evas_free(window);

   return 0;
}
@endcode

It requires the following source Edje file:
@code
// compile: edje_cc edje_example.edc
collections {
   group {
      name: "my_group"; // must be the same as in edje_example.c

      parts {
         part {
            name: "background";
            type: RECT; // plain boring rectangle
            mouse_events: 0; // we don't need any mouse event on the background

            // just one state "default"
            description {
               state: "default" 0.0; // must always exist
               color: 255 255 255 255; // white

               // define part coordinates:

               rel1 { // top-left point at (0, 0) [WIDTH * 0 + 0, HEIGHT * 0 + 0]
                  relative: 0.0 0.0;
                  offset: 0 0;
               }
               rel2 { // bottom-right point at (WIDTH * 1.0 - 1, HEIGHT * 1.0 - 1)
                  relative: 1.0 1.0;
                  offset: -1 -1;
               }
            }
         }

         part {
            name: "text";
            type: TEXT;
            mouse_events: 1; // we want to change the color on mouse-over

            // 2 states, one "default" and another "over" to be used
            // on mouse over effect

            description {
               state: "default" 0.0;
               color: 255 0 0 255; // red

               // define part coordinates:

               rel1 { // top-left at (WIDTH * 0.1 + 5, HEIGHT * 0.2 + 10)
                  relative: 0.1 0.2;
                  offset: 5 10;
               }
               rel2 { // bottom-right at (WIDTH * 0.9 - 6, HEIGHT * 0.8 - 11)
                  relative: 0.9 0.8;
                  offset: -6 -11;
               }

               // define text specific state details
               text {
                  font: "Sans"; // using fontconfig name!
                  size: 10;
                  text: "hello world";
               }
            }

            description {
               state: "over" 0.0;
               inherit: "default" 0.0; // copy everything from "default" at this point

               color: 0 255 0 255; // override color, now it is green
            }
         }

         // do programs to change color on text mouse in/out (over)
         programs {
            program {
               // what triggers this program:
               signal: "mouse,in";
               source: "text";

               // what this program does:
               action: STATE_SET "over" 0.0;
               target: "text";

               // do the state-set in a nice interpolation animation
               // using linear time in 0.1 second
               transition: LINEAR 0.1;
            }

            program {
               // what triggers this program:
               signal: "mouse,out";
               source: "text";

               // what this program does:
               action: STATE_SET "default" 0.0;
               target: "text";

               // do the state-set in a nice interpolation animation
               // using linear time in 0.1 second
               transition: LINEAR 0.1;
            }
         }
      }
   }
}
@endcode


One should save these files as edje_example.c and edje_example.edc then:
@verbatim
gcc -o edje_example edje_example.c `pkg-config --cflags --libs eina evas ecore ecore-evas edje`
edje_cc edje_example.edc

./edje_example "some text"
@endverbatim

Although simple, this example illustrates that animations and state
changes can be done from the Edje file itself without any requirement
in the C application.

Before digging into changing or creating your own Edje source (edc)
files, read the \ref edcref.



@section history Edje History

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



@section Edje_Examples Examples on Edje's usage

What follows is a list with varios commented examples, covering a great
part of Edje's API:

- @ref Example_Edje_Basics
- @ref tutorial_edje_swallow
- @ref tutorial_edje_table
- @ref tutorial_edje_box
- @ref tutorial_edje_box2
- @ref Example_Edje_Signals_Messages




@todo Complete documentation of API
@todo Bytecode language for extending programs... but what/how?

*/


/**

@example embryo_custom_state.edc
This example show how to create a custom state from embryo. Clicking on the
3 labels will rotate the object in the given direction.

@example embryo_pong.edc
Super-simple Pong implementation in pure embryo.

@example embryo_run_program.edc
This example show how to run an edje program from embryo code.

@example embryo_set_state.edc
This example show how to change the state of a part from embryo code.

@example embryo_set_text.edc
This example show how to set the text in TEXT part from embryo code.

@example embryo_timer.edc
This example show the usage of timers in embryo.

@example external_elm_anchorblock.edc
This example use an elementary anchorblock and a button to animate the text.

@example external_elm_button.edc
This example create some elementary buttons and do some actions on user click.

@example external_elm_check.edc
This example show EXTERNAL checkbox in action.

@example external_elm_panes.edc
This example show EXTERNAL elementary panes in action.

@example external_emotion_elm.edc
Super-concise video player example using Edje/Emotion/Elementary.

@example lua_script.edc
This example show the usage of lua scripting to create and animate some
objects in the canvas.

@example toggle_using_filter.edc
This example show how to toggle the state of a part using the 'filter'
param in edje programs

*/

/**
 * @page luaref Edje Lua scripting
 *
 * @section intro Introduction
 *
 * Lua is intended for script-only objects at this point (with embryo left
 * for augmenting standard programs). Since script-only objects effectively
 * define objects entirely via Lua script (resize handling, event handling
 * etc. etc.) this places many more demands on them, and thus a more powerful
 * language is in order. Lua is that language.
 *
 * To get you started, here's an example:
 * @include lua_script.edc
 *
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

#define EDJE_VERSION_MAJOR 1
#define EDJE_VERSION_MINOR 0

   typedef struct _Edje_Version
     {
        int major;
        int minor;
        int micro;
        int revision;
     } Edje_Version;

   EAPI extern Edje_Version *edje_version;

/**
 * @file Edje.h
 * @brief Edje Graphical Design Library
 *
 * These routines are used for Edje.
 */

/**
 * Identifiers of Edje message types, which can be sent back and forth
 * code and a given Edje object's theme file/group.
 *
 * @see edje_object_message_send()
 * @see edje_object_message_handler_set()
 */
typedef enum _Edje_Message_Type
{
   EDJE_MESSAGE_NONE = 0,

   EDJE_MESSAGE_SIGNAL = 1, /* DONT USE THIS */

   EDJE_MESSAGE_STRING = 2, /**< A message with a string as value. Use #Edje_Message_String structs as message body, for this type. */
   EDJE_MESSAGE_INT = 3, /**< A message with an integer number as value. Use #Edje_Message_Int structs as message body, for this type. */
   EDJE_MESSAGE_FLOAT = 4, /**< A message with a floating pointer number as value. Use #Edje_Message_Float structs as message body, for this type. */

   EDJE_MESSAGE_STRING_SET = 5, /**< A message with a list of strings as value. Use #Edje_Message_String_Set structs as message body, for this type. */
   EDJE_MESSAGE_INT_SET = 6, /**< A message with a list of integer numbers as value. Use #Edje_Message_Int_Set structs as message body, for this type. */
   EDJE_MESSAGE_FLOAT_SET = 7, /**< A message with a list of floating point numbers as value. Use #Edje_Message_Float_Set structs as message body, for this type. */

   EDJE_MESSAGE_STRING_INT = 8, /**< A message with a struct containing a string and an integer number as value. Use #Edje_Message_String_Int structs as message body, for this type. */
   EDJE_MESSAGE_STRING_FLOAT = 9, /**< A message with a struct containing a string and a floating point number as value. Use #Edje_Message_String_Float structs as message body, for this type. */

   EDJE_MESSAGE_STRING_INT_SET = 10, /**< A message with a struct containing a string and list of integer numbers as value. Use #Edje_Message_String_Int_Set structs as message body, for this type. */
   EDJE_MESSAGE_STRING_FLOAT_SET = 11 /**< A message with a struct containing a string and list of floating point numbers as value. Use #Edje_Message_String_Float_Set structs as message body, for this type. */
} Edje_Message_Type;

typedef enum _Edje_Aspect_Control
{
   EDJE_ASPECT_CONTROL_NONE = 0,
   EDJE_ASPECT_CONTROL_NEITHER = 1,
   EDJE_ASPECT_CONTROL_HORIZONTAL = 2,
   EDJE_ASPECT_CONTROL_VERTICAL = 3,
   EDJE_ASPECT_CONTROL_BOTH = 4
} Edje_Aspect_Control;

typedef enum _Edje_Object_Table_Homogeneous_Mode
{
   EDJE_OBJECT_TABLE_HOMOGENEOUS_NONE = 0,
   EDJE_OBJECT_TABLE_HOMOGENEOUS_TABLE = 1,
   EDJE_OBJECT_TABLE_HOMOGENEOUS_ITEM = 2
} Edje_Object_Table_Homogeneous_Mode;

typedef enum _Edje_Part_Type
{
   EDJE_PART_TYPE_NONE      = 0,
   EDJE_PART_TYPE_RECTANGLE = 1,
   EDJE_PART_TYPE_TEXT      = 2,
   EDJE_PART_TYPE_IMAGE     = 3,
   EDJE_PART_TYPE_SWALLOW   = 4,
   EDJE_PART_TYPE_TEXTBLOCK = 5,
   EDJE_PART_TYPE_GRADIENT  = 6,
   EDJE_PART_TYPE_GROUP     = 7,
   EDJE_PART_TYPE_BOX       = 8,
   EDJE_PART_TYPE_TABLE     = 9,
   EDJE_PART_TYPE_EXTERNAL  = 10,
   EDJE_PART_TYPE_PROXY     = 11,
   EDJE_PART_TYPE_LAST      = 12
} Edje_Part_Type;

typedef enum _Edje_Text_Effect
{
#define EDJE_TEXT_EFFECT_MASK_BASIC 0xf
#define EDJE_TEXT_EFFECT_BASIC_SET(x, s) \
   do { x = ((x) & ~EDJE_TEXT_EFFECT_MASK_BASIC) | (s); } while (0)
   EDJE_TEXT_EFFECT_NONE                = 0,
   EDJE_TEXT_EFFECT_PLAIN               = 1,
   EDJE_TEXT_EFFECT_OUTLINE             = 2,
   EDJE_TEXT_EFFECT_SOFT_OUTLINE        = 3,
   EDJE_TEXT_EFFECT_SHADOW              = 4,
   EDJE_TEXT_EFFECT_SOFT_SHADOW         = 5,
   EDJE_TEXT_EFFECT_OUTLINE_SHADOW      = 6,
   EDJE_TEXT_EFFECT_OUTLINE_SOFT_SHADOW = 7,
   EDJE_TEXT_EFFECT_FAR_SHADOW          = 8,
   EDJE_TEXT_EFFECT_FAR_SOFT_SHADOW     = 9,
   EDJE_TEXT_EFFECT_GLOW                = 10,

   EDJE_TEXT_EFFECT_LAST                = 11,

#define EDJE_TEXT_EFFECT_MASK_SHADOW_DIRECTION (0x7 << 4)
#define EDJE_TEXT_EFFECT_SHADOW_DIRECTION_SET(x, s) \
   do { x = ((x) & ~EDJE_TEXT_EFFECT_MASK_SHADOW_DIRECTION) | (s); } while (0)
   EDJE_TEXT_EFFECT_SHADOW_DIRECTION_BOTTOM_RIGHT = (0x0 << 4),
   EDJE_TEXT_EFFECT_SHADOW_DIRECTION_BOTTOM       = (0x1 << 4),
   EDJE_TEXT_EFFECT_SHADOW_DIRECTION_BOTTOM_LEFT  = (0x2 << 4),
   EDJE_TEXT_EFFECT_SHADOW_DIRECTION_LEFT         = (0x3 << 4),
   EDJE_TEXT_EFFECT_SHADOW_DIRECTION_TOP_LEFT     = (0x4 << 4),
   EDJE_TEXT_EFFECT_SHADOW_DIRECTION_TOP          = (0x5 << 4),
   EDJE_TEXT_EFFECT_SHADOW_DIRECTION_TOP_RIGHT    = (0x6 << 4),
   EDJE_TEXT_EFFECT_SHADOW_DIRECTION_RIGHT        = (0x7 << 4)
} Edje_Text_Effect;

typedef enum _Edje_Action_Type
{
   EDJE_ACTION_TYPE_NONE          = 0,
   EDJE_ACTION_TYPE_STATE_SET     = 1,
   EDJE_ACTION_TYPE_ACTION_STOP   = 2,
   EDJE_ACTION_TYPE_SIGNAL_EMIT   = 3,
   EDJE_ACTION_TYPE_DRAG_VAL_SET  = 4,
   EDJE_ACTION_TYPE_DRAG_VAL_STEP = 5,
   EDJE_ACTION_TYPE_DRAG_VAL_PAGE = 6,
   EDJE_ACTION_TYPE_SCRIPT        = 7,
   EDJE_ACTION_TYPE_FOCUS_SET     = 8,
   EDJE_ACTION_TYPE_RESERVED00    = 9,
   EDJE_ACTION_TYPE_FOCUS_OBJECT  = 10,
   EDJE_ACTION_TYPE_PARAM_COPY    = 11,
   EDJE_ACTION_TYPE_PARAM_SET     = 12,
   EDJE_ACTION_TYPE_LAST          = 13
} Edje_Action_Type;

typedef enum _Edje_Tween_Mode
{
   EDJE_TWEEN_MODE_NONE              = 0,
   EDJE_TWEEN_MODE_LINEAR            = 1,
   EDJE_TWEEN_MODE_SINUSOIDAL        = 2,
   EDJE_TWEEN_MODE_ACCELERATE        = 3,
   EDJE_TWEEN_MODE_DECELERATE        = 4,
   EDJE_TWEEN_MODE_ACCELERATE_FACTOR = 5,
   EDJE_TWEEN_MODE_DECELERATE_FACTOR = 6,
   EDJE_TWEEN_MODE_SINUSOIDAL_FACTOR = 7,
   EDJE_TWEEN_MODE_DIVISOR_INTERP    = 8,
   EDJE_TWEEN_MODE_BOUNCE            = 9,
   EDJE_TWEEN_MODE_SPRING            = 10,
   EDJE_TWEEN_MODE_LAST              = 11
} Edje_Tween_Mode;

typedef enum _Edje_Cursor
{
   EDJE_CURSOR_MAIN,
   EDJE_CURSOR_SELECTION_BEGIN,
   EDJE_CURSOR_SELECTION_END,
   EDJE_CURSOR_PREEDIT_START,
   EDJE_CURSOR_PREEDIT_END,
   EDJE_CURSOR_USER,
   EDJE_CURSOR_USER_EXTRA,
   // more later
} Edje_Cursor;

typedef struct _Edje_Message_String           Edje_Message_String;
typedef struct _Edje_Message_Int              Edje_Message_Int;
typedef struct _Edje_Message_Float            Edje_Message_Float;
typedef struct _Edje_Message_String_Set       Edje_Message_String_Set;
typedef struct _Edje_Message_Int_Set          Edje_Message_Int_Set;
typedef struct _Edje_Message_Float_Set        Edje_Message_Float_Set;
typedef struct _Edje_Message_String_Int       Edje_Message_String_Int;
typedef struct _Edje_Message_String_Float     Edje_Message_String_Float;
typedef struct _Edje_Message_String_Int_Set   Edje_Message_String_Int_Set;
typedef struct _Edje_Message_String_Float_Set Edje_Message_String_Float_Set;

struct _Edje_Message_String
{
   char *str; /**< The message's string pointer */
}; /**< Structure passed as value on #EDJE_MESSAGE_STRING messages. The string in it is automatically freed be Edje */

struct _Edje_Message_Int
{
   int val; /**< The message's value */
}; /**< Structure passed as value on #EDJE_MESSAGE_INT messages */

struct _Edje_Message_Float
{
   double val; /**< The message's value */
}; /**< Structure passed as value on #EDJE_MESSAGE_FLOAT messages */

struct _Edje_Message_String_Set
{
   int count; /**< The size of the message's array (may be greater than 1) */
   char *str[1]; /**< The message's @b array of string pointers */
}; /**< Structure passed as value on #EDJE_MESSAGE_STRING_SET messages. The array in it is automatically freed be Edje */

struct _Edje_Message_Int_Set
{
   int count; /**< The size of the message's array (may be greater than 1) */
   int val[1]; /**< The message's @b array of integers */
}; /**< Structure passed as value on #EDJE_MESSAGE_INT_SET messages. The array in it is automatically freed be Edje */

struct _Edje_Message_Float_Set
{
   int count; /**< The size of the message's array (may be greater than 1) */
   double val[1]; /**< The message's @b array of floats */
}; /**< Structure passed as value on #EDJE_MESSAGE_FLOAT_SET messages. The array in it is automatically freed be Edje */

struct _Edje_Message_String_Int
{
   char *str; /**< The message's string value */
   int val; /**< The message's integer value */
}; /**< Structure passed as value on #EDJE_MESSAGE_STRING_INT messages */

struct _Edje_Message_String_Float
{
   char *str; /**< The message's string value */
   double val; /**< The message's float value */
}; /**< Structure passed as value on #EDJE_MESSAGE_STRING_FLOAT messages */

struct _Edje_Message_String_Int_Set
{
   char *str; /**< The message's string value */
   int count; /**< The size of the message's array (may be greater than 1) */
   int val[1]; /**< The message's @b array of integers */
}; /**< Structure passed as value on #EDJE_MESSAGE_STRING_INT_SET messages */

struct _Edje_Message_String_Float_Set
{
   char *str; /**< The message's string value */
   int count; /**< The size of the message's array (may be greater than 1) */
   double val[1]; /**< The message's @b array of floats */
}; /**< Structure passed as value on #EDJE_MESSAGE_STRING_FLOAT_SET messages */

typedef enum _Edje_Drag_Dir
{
   EDJE_DRAG_DIR_NONE = 0,
   EDJE_DRAG_DIR_X = 1,
   EDJE_DRAG_DIR_Y = 2,
   EDJE_DRAG_DIR_XY = 3
} Edje_Drag_Dir;

typedef enum _Edje_Load_Error
{
   EDJE_LOAD_ERROR_NONE = 0, /**< No error happened, the loading was successful */
   EDJE_LOAD_ERROR_GENERIC = 1, /**< A generic error happened during the loading */
   EDJE_LOAD_ERROR_DOES_NOT_EXIST = 2, /**< The file pointed to did not exist */
   EDJE_LOAD_ERROR_PERMISSION_DENIED = 3, /**< Permission to read the given file was denied */
   EDJE_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED = 4, /**< Resource allocation failed during the loading */
   EDJE_LOAD_ERROR_CORRUPT_FILE = 5, /**< The file pointed to was corrupt */
   EDJE_LOAD_ERROR_UNKNOWN_FORMAT = 6, /**< The file pointed to had an unknown format */
   EDJE_LOAD_ERROR_INCOMPATIBLE_FILE = 7, /**< The file pointed to is incompatible, i.e., it doesn't match the library's current version's format */
   EDJE_LOAD_ERROR_UNKNOWN_COLLECTION = 8, /**< The group/collection set to load from was @b not found in the file */
   EDJE_LOAD_ERROR_RECURSIVE_REFERENCE = 9 /**< The group/collection set to load from had <b>recursive references</b> on its components */
} Edje_Load_Error; /**< Edje file loading error codes one can get - see edje_load_error_str() too. */

typedef enum _Edje_Text_Filter_Type
{
   EDJE_TEXT_FILTER_TEXT = 0,
   EDJE_TEXT_FILTER_FORMAT = 1,
   EDJE_TEXT_FILTER_MARKUP = 2
} Edje_Text_Filter_Type;

typedef enum _Edje_External_Param_Type
{
   EDJE_EXTERNAL_PARAM_TYPE_INT,
   EDJE_EXTERNAL_PARAM_TYPE_DOUBLE,
   EDJE_EXTERNAL_PARAM_TYPE_STRING,
   EDJE_EXTERNAL_PARAM_TYPE_BOOL,
   EDJE_EXTERNAL_PARAM_TYPE_CHOICE,
   EDJE_EXTERNAL_PARAM_TYPE_MAX
} Edje_External_Param_Type;

/**
 * @typedef Edje_External_Param_Flags flags that determines the
 * behavior of a parameter.
 *
 * @var EDJE_EXTERNAL_PARAM_FLAGS_NONE property is incapable of
 *      operations, this is used to catch bogus flags.
 * @var EDJE_EXTERNAL_PARAM_FLAGS_GET property can be read/get
 * @var EDJE_EXTERNAL_PARAM_FLAGS_SET property can be written/set.
 *      This only enables edje_object_part_external_param_set() and
 *      Embryo scripts. To enable parameter being set from state
 *      description whenever it changes state, use
 *      #EDJE_EXTERNAL_PARAM_FLAGS_STATE.
 * @var EDJE_EXTERNAL_PARAM_FLAGS_STATE property can be set from state
 *      description.
 * @var EDJE_EXTERNAL_PARAM_FLAGS_CONSTRUCTOR this property is only
 *      set once when object is constructed using its value from
 *      "default" 0.0 state description. Setting this overrides
 *      #EDJE_EXTERNAL_PARAM_FLAGS_STATE.
 * @var EDJE_EXTERNAL_PARAM_FLAGS_REGULAR convenience flag that sets
 *      property as GET, SET and STATE.
 */
typedef enum _Edje_External_Param_Flags
{
   EDJE_EXTERNAL_PARAM_FLAGS_NONE        = 0,
   EDJE_EXTERNAL_PARAM_FLAGS_GET         = (1 << 0),
   EDJE_EXTERNAL_PARAM_FLAGS_SET         = (1 << 1),
   EDJE_EXTERNAL_PARAM_FLAGS_STATE       = (1 << 2),
   EDJE_EXTERNAL_PARAM_FLAGS_CONSTRUCTOR = (1 << 3),
   EDJE_EXTERNAL_PARAM_FLAGS_REGULAR     = (EDJE_EXTERNAL_PARAM_FLAGS_GET |
                                            EDJE_EXTERNAL_PARAM_FLAGS_SET |
                                            EDJE_EXTERNAL_PARAM_FLAGS_STATE)
} Edje_External_Param_Flags;


/**
 * @brief Converts type identifier to string nicer representation.
 *
 * This may be used to debug or other informational purposes.
 *
 * @param type the identifier to convert.
 * @return the string with the string representation, or @c "(unknown)".
 */
EAPI const char *edje_external_param_type_str(Edje_External_Param_Type type) EINA_PURE;

struct _Edje_External_Param
{
   const char               *name;
   Edje_External_Param_Type  type;
   // XXX these could be in a union, but eet doesn't support them (or does it?)
   int                       i; /**< used by both integer and boolean */
   double                    d;
   const char               *s; /**< used by both string and choice */
};
typedef struct _Edje_External_Param Edje_External_Param;

#define EDJE_EXTERNAL_INT_UNSET INT_MAX
#define EDJE_EXTERNAL_DOUBLE_UNSET DBL_MAX

typedef struct _Edje_External_Param_Info Edje_External_Param_Info;
struct _Edje_External_Param_Info
{
   const char               *name;
   Edje_External_Param_Type  type;
   Edje_External_Param_Flags flags;
   union {
      struct {
         int                 def, min, max, step;
      } i;
      struct {
         double              def, min, max, step;
      } d;
      struct {
         const char         *def;
         const char         *accept_fmt;
         const char         *deny_fmt;
      } s;
      struct {
         int                 def;
         const char         *false_str;
         const char         *true_str;
      } b;
      struct {
         const char         *def;
         const char        **choices; /* NULL terminated array */
         char               *(*def_get)(void *data, const Edje_External_Param_Info *info); /* return malloc() memory with the default choice, should be used if def is NULL. First parameter is Edje_External_Type::data */
         char              **(*query)(void *data, const Edje_External_Param_Info *info); /* NULL terminated array of strings, memory is dynamically allocated and should be freed with free() for array and each element. First parameter is Edje_External_Type::data */
      } c;
   } info;
};

#define EDJE_EXTERNAL_PARAM_INFO_INT_FULL_FLAGS(name, def, min, max, step, flags) \
  {name, EDJE_EXTERNAL_PARAM_TYPE_INT, flags, {.i = {def, min, max, step}}}
#define EDJE_EXTERNAL_PARAM_INFO_DOUBLE_FULL_FLAGS(name, def, min, max, step, flags) \
  {name, EDJE_EXTERNAL_PARAM_TYPE_DOUBLE, flags, {.d = {def, min, max, step}}}
#define EDJE_EXTERNAL_PARAM_INFO_STRING_FULL_FLAGS(name, def, accept, deny, flags) \
  {name, EDJE_EXTERNAL_PARAM_TYPE_STRING, flags, {.s = {def, accept, deny}}}
#define EDJE_EXTERNAL_PARAM_INFO_BOOL_FULL_FLAGS(name, def, false_str, true_str, flags) \
  {name, EDJE_EXTERNAL_PARAM_TYPE_BOOL, flags, {.b = {def, false_str, true_str}}}
#define EDJE_EXTERNAL_PARAM_INFO_CHOICE_FULL_FLAGS(name, def, choices, flags) \
  {name, EDJE_EXTERNAL_PARAM_TYPE_CHOICE, flags, {.c = {def, choices, NULL, NULL}}}
#define EDJE_EXTERNAL_PARAM_INFO_CHOICE_DYNAMIC_FULL_FLAGS(name, def_get, query, flags) \
  {name, EDJE_EXTERNAL_PARAM_TYPE_CHOICE, flags, {.c = {NULL, NULL, def_get, query}}}

#define EDJE_EXTERNAL_PARAM_INFO_INT_FULL(name, def, min, max, step) \
  EDJE_EXTERNAL_PARAM_INFO_INT_FULL_FLAGS(name, def, min, max, step, EDJE_EXTERNAL_PARAM_FLAGS_REGULAR)
#define EDJE_EXTERNAL_PARAM_INFO_DOUBLE_FULL(name, def, min, max, step) \
  EDJE_EXTERNAL_PARAM_INFO_DOUBLE_FULL_FLAGS(name, def, min, max, step, EDJE_EXTERNAL_PARAM_FLAGS_REGULAR)
#define EDJE_EXTERNAL_PARAM_INFO_STRING_FULL(name, def, accept, deny) \
  EDJE_EXTERNAL_PARAM_INFO_STRING_FULL_FLAGS(name, def, accept, deny, EDJE_EXTERNAL_PARAM_FLAGS_REGULAR)
#define EDJE_EXTERNAL_PARAM_INFO_BOOL_FULL(name, def, false_str, true_str) \
  EDJE_EXTERNAL_PARAM_INFO_BOOL_FULL_FLAGS(name, def, false_str, true_str, EDJE_EXTERNAL_PARAM_FLAGS_REGULAR)
#define EDJE_EXTERNAL_PARAM_INFO_CHOICE_FULL(name, def, choices) \
  EDJE_EXTERNAL_PARAM_INFO_CHOICE_FULL_FLAGS(name, def, choices, EDJE_EXTERNAL_PARAM_FLAGS_REGULAR)
#define EDJE_EXTERNAL_PARAM_INFO_CHOICE_DYNAMIC_FULL(name, def_get, query) \
  EDJE_EXTERNAL_PARAM_INFO_CHOICE_DYNAMIC_FULL_FLAGS(name, def_get, query, EDJE_EXTERNAL_PARAM_FLAGS_REGULAR)

#define EDJE_EXTERNAL_PARAM_INFO_INT_DEFAULT(name, def) \
   EDJE_EXTERNAL_PARAM_INFO_INT_FULL(name, def, EDJE_EXTERNAL_INT_UNSET, EDJE_EXTERNAL_INT_UNSET, EDJE_EXTERNAL_INT_UNSET)
#define EDJE_EXTERNAL_PARAM_INFO_DOUBLE_DEFAULT(name, def) \
   EDJE_EXTERNAL_PARAM_INFO_DOUBLE_FULL(name, def, EDJE_EXTERNAL_DOUBLE_UNSET, EDJE_EXTERNAL_DOUBLE_UNSET, EDJE_EXTERNAL_DOUBLE_UNSET)
#define EDJE_EXTERNAL_PARAM_INFO_STRING_DEFAULT(name, def) \
   EDJE_EXTERNAL_PARAM_INFO_STRING_FULL(name, def, NULL, NULL)
#define EDJE_EXTERNAL_PARAM_INFO_BOOL_DEFAULT(name, def) \
   EDJE_EXTERNAL_PARAM_INFO_BOOL_FULL(name, def, "false", "true")

#define EDJE_EXTERNAL_PARAM_INFO_INT_DEFAULT_FLAGS(name, def, flags)    \
  EDJE_EXTERNAL_PARAM_INFO_INT_FULL_FLAGS(name, def, EDJE_EXTERNAL_INT_UNSET, EDJE_EXTERNAL_INT_UNSET, EDJE_EXTERNAL_INT_UNSET, flags)
#define EDJE_EXTERNAL_PARAM_INFO_DOUBLE_DEFAULT_FLAGS(name, def, flags) \
  EDJE_EXTERNAL_PARAM_INFO_DOUBLE_FULL_FLAGS(name, def, EDJE_EXTERNAL_DOUBLE_UNSET, EDJE_EXTERNAL_DOUBLE_UNSET, EDJE_EXTERNAL_DOUBLE_UNSET, flags)
#define EDJE_EXTERNAL_PARAM_INFO_STRING_DEFAULT_FLAGS(name, def, flags) \
  EDJE_EXTERNAL_PARAM_INFO_STRING_FULL_FLAGS(name, def, NULL, NULL, flags)
#define EDJE_EXTERNAL_PARAM_INFO_BOOL_DEFAULT_FLAGS(name, def, flags)   \
  EDJE_EXTERNAL_PARAM_INFO_BOOL_FULL_FLAGS(name, def, "false", "true", flags)

#define EDJE_EXTERNAL_PARAM_INFO_INT(name) \
   EDJE_EXTERNAL_PARAM_INFO_INT_DEFAULT(name, 0)
#define EDJE_EXTERNAL_PARAM_INFO_DOUBLE(name) \
   EDJE_EXTERNAL_PARAM_INFO_DOUBLE_DEFAULT(name, 0.0)
#define EDJE_EXTERNAL_PARAM_INFO_STRING(name) \
   EDJE_EXTERNAL_PARAM_INFO_STRING_DEFAULT(name, NULL)
#define EDJE_EXTERNAL_PARAM_INFO_BOOL(name) \
   EDJE_EXTERNAL_PARAM_INFO_BOOL_DEFAULT(name, 0)

#define EDJE_EXTERNAL_PARAM_INFO_INT_FLAGS(name, flags) \
   EDJE_EXTERNAL_PARAM_INFO_INT_DEFAULT_FLAGS(name, 0, flags)
#define EDJE_EXTERNAL_PARAM_INFO_DOUBLE_FLAGS(name, flags) \
   EDJE_EXTERNAL_PARAM_INFO_DOUBLE_DEFAULT_FLAGS(name, 0.0, flags)
#define EDJE_EXTERNAL_PARAM_INFO_STRING_FLAGS(name, flags) \
   EDJE_EXTERNAL_PARAM_INFO_STRING_DEFAULT_FLAGS(name, NULL, flags)
#define EDJE_EXTERNAL_PARAM_INFO_BOOL_FLAGS(name, flags) \
   EDJE_EXTERNAL_PARAM_INFO_BOOL_DEFAULT_FLAGS(name, 0, flags)

#define EDJE_EXTERNAL_PARAM_INFO_SENTINEL {NULL, 0, 0, {.s = {NULL, NULL, NULL}}}

/**
 * @struct _Edje_External_Type
 *
 * @brief Information about an external type to be used.
 *
 * This structure provides information on how to display and modify a
 * third party Evas_Object in Edje.
 *
 * Some function pointers are not really used by Edje, but provide
 * means for Edje users to better interact with such objects. For
 * instance, an editor may use label_get() and icon_get() to list all
 * registered external types.
 *
 * @note The function pointers provided in this structure must check
 *       for errors and invalid or out-of-range values as for
 *       performance reasons Edje will not enforce hints provided as
 *       #Edje_External_Param_Info in the member parameters_info.
 */
struct _Edje_External_Type
{
#define EDJE_EXTERNAL_TYPE_ABI_VERSION (3)
  unsigned int  abi_version; /**< always use:
                              *  - #EDJE_EXTERNAL_TYPE_ABI_VERSION to declare.
                              *  - edje_external_type_abi_version_get() to check.
                              */
  const char    *module;
  const char    *module_name;
  Evas_Object *(*add) (void *data, Evas *evas, Evas_Object *parent, const Eina_List *params, const char *part_name); /**< creates the object to be used by Edje as the part */
  void         (*state_set) (void *data, Evas_Object *obj, const void *from_params, const void *to_params, float pos); /**< called upon state changes, including the initial "default" 0.0 state. Parameters are the value returned by params_parse() */
  void         (*signal_emit) (void *data, Evas_Object *obj, const char *emission, const char *source); /**< Feed a signal emitted with emission originally set as part_name:signal to this object (without the "part_name:" prefix) */
  Eina_Bool    (*param_set) (void *data, Evas_Object *obj, const Edje_External_Param *param); /**< dynamically change a parameter of this external, called by scripts and user code. Returns @c EINA_TRUE on success */
  Eina_Bool    (*param_get) (void *data, const Evas_Object *obj, Edje_External_Param *param); /**< dynamically fetch a parameter of this external, called by scripts and user code. Returns @c EINA_TRUE on success. (Must check parameter name and type!) */
  Evas_Object *(*content_get) (void *data, const Evas_Object *obj, const char *content); /**< dynamically fetch a sub object of this external, called by scripts and user code. Returns @c Evas_Object * on success. (Must check parameter name and type!) */
  void        *(*params_parse) (void *data, Evas_Object *obj, const Eina_List *params); /**< parses the list of parameters, converting into a friendly representation. Used with state_set() */
  void         (*params_free) (void *params); /**< free parameters parsed with params_parse() */

  /* The following callbacks aren't used by Edje itself, but by UI design
     tools instead */
  const char  *(*label_get) (void *data);
  const char  *(*description_get) (void *data);
  Evas_Object *(*icon_add) (void *data, Evas *e);
  Evas_Object *(*preview_add) (void *data, Evas *e);
  const char  *(*translate) (void *data, const char *orig); /**< called to translate parameters_info name properties for use in user interfaces that support internationalization (i18n) */

  Edje_External_Param_Info *parameters_info;
  void                     *data;
};
typedef struct _Edje_External_Type Edje_External_Type;

struct _Edje_External_Type_Info
{
   const char               *name;
   const Edje_External_Type *info;
};
typedef struct _Edje_External_Type_Info Edje_External_Type_Info;

typedef void         (*Edje_Signal_Cb)          (void *data, Evas_Object *obj, const char *emission, const char *source); /**< Edje signal callback functions's prototype definition. @c data will have the auxiliary data pointer set at the time the callback registration. @c obj will be a pointer the Edje object where the signal comes from. @c emission will identify the exact signal's emission string and @c source the exact signal's source one. */
typedef void         (*Edje_Text_Change_Cb)     (void *data, Evas_Object *obj, const char *part);
typedef void         (*Edje_Message_Handler_Cb) (void *data, Evas_Object *obj, Edje_Message_Type type, int id, void *msg); /**< Edje message handler callback functions's prototype definition. @c data will have the auxiliary data pointer set at the time the callback registration. @c obj will be a pointer the Edje object where the message comes from. @c type will identify the type of the given message and @c msg will be a pointer the message's contents, de facto, which depend on @c type. */
typedef void         (*Edje_Text_Filter_Cb)     (void *data, Evas_Object *obj, const char *part, Edje_Text_Filter_Type type, char **text);
typedef Evas_Object *(*Edje_Item_Provider_Cb)   (void *data, Evas_Object *obj, const char *part, const char *item);

/**
 * @brief Initialize the Edje library.
 *
 * @return The new init count. The initial value is zero.
 *
 * This function initializes the Ejde library, making the proper calls
 * to internal initialization functions. It will also initialize its
 * @b dependencies, making calls to @c eina_init(), @c ecore_init(),
 * @c embryo_init() and @c eet_init(). So, there is no need to call
 * those functions again, in your code. To shutdown Edje there is the
 * function edje_shutdown().
 *
 * @see edje_shutdown()
 * @see eina_init()
 * @see ecore_init()
 * @see embryo_init()
 * @see eet_init()
 *
 */
EAPI int          edje_init                       (void);

/**
 * @brief Shutdown the Edje library.
 *
 * @return The number of times the library has been initialised
 *         without being shutdown.
 *
 * This function shuts down the Edje library. It will also call the
 * shutdown functions of its @b dependencies, which are @c
 * eina_shutdown(), @c ecore_shutdown(), @c embryo_shutdown() and @c
 * eet_shutdown(), so there is no need to call these functions again,
 * in your code.
 *
 * @see edje_init()
 * @see eina_shutdown()
 * @see ecore_shutdown()
 * @see embryo_shutdown()
 * @see eet_shutdown()
 *
 */
EAPI int          edje_shutdown                   (void);

/**
 * @brief Set edje trasitions' frame time.
 *
 * @param t The frame time, in seconds. Default value is 1/30.
 *
 * This function sets the edje built-in animations' frame time (thus,
 * affecting their resolution) by calling
 * ecore_animator_frametime_set(). This frame time can be retrieved
 * with edje_frametime_get().
 *
 * @see edje_frametime_get()
 *
 */
EAPI void         edje_frametime_set              (double t);

/**
 * @brief Get edje trasitions' frame time.
 *
 * @return The frame time, in seconds.
 *
 * This function returns the edje frame time set by
 * edje_frametime_set().
 *
 * @see edje_frametime_set()
 *
 */
EAPI double       edje_frametime_get              (void);

/**
 * @brief Freeze Edje objects.
 *
 * This function freezes every edje objects in the current process.
 *
 * See edje_object_freeze().
 *
 */
EAPI void         edje_freeze                     (void);

/**
 * @brief Thaw edje objects.
 *
 * This function thaw all edje object in the current process.
 *
 * See edje_object_thaw().
 *
 */
EAPI void         edje_thaw                       (void);

/**
 * @brief Set the edje append fontset.
 *
 * @param fonts The fontset to append.
 *
 * This function sets the edje append fontset.
 *
 */
EAPI void         edje_fontset_append_set         (const char *fonts);

/**
 * @brief Get the edje append fontset.
 *
 * @return The edje append fontset.
 *
 * This function returns the edje append fontset set by
 * edje_fontset_append_set() function.
 *
 * @see edje_fontset_append_set().
 *
 */
EAPI const char  *edje_fontset_append_get         (void);

/**
 * @brief Set Edje's global scaling factor.
 *
 * @param scale The global scaling factor (the default value is @c 1.0)
 *
 * Edje allows one to build scalable interfaces. Scaling factors,
 * which are set to neutral (@c 1.0) values by default (no scaling,
 * actual sizes), are of two types: @b global and @b individual.
 * Edje's global scaling factor will affect all its objects which
 * hadn't their individual scaling factors altered from the default
 * value (which is zero). If they had it set differently, by
 * edje_object_scale_set(), that factor will @b override the global
 * one.
 *
 * Scaling affects the values of mininum/maximum @b part sizes, which
 * are @b multiplied by it. Font sizes are scaled, too.
 *
 * @warning Only parts which, at EDC level, had the @c "scale"
 * property set to @c 1, will be affected by this function. Check the
 * complete @ref edcref "syntax reference" for EDC files.
 *
 * @see edje_scale_get().
 */
EAPI void         edje_scale_set                  (double scale);

/**
 * @brief Retrieve Edje's global scaling factor.
 *
 * @return The global scaling factor
 *
 * This function returns Edje's global scaling factor.
 *
 * @see edje_scale_set() for more details
 *
 */
EAPI double       edje_scale_get                  (void);

/**
 * @brief Show last character in password mode.
 *
 * @param password_show_last If TRUE enable last character show in password mode.
 *
 * This function enables last input to be visible when in password mode for few seconds
 * or until the next input is entered.
 *
 * The time out value is obtained by edje_password_show_last_timeout_set function.
 *
 * @see edje_password_show_last_timeout_set().
 */
EAPI void edje_password_show_last_set(Eina_Bool password_show_last);

/**
 * @brief Set's the timeout value in last show password mode.
 *
 * @param password_show_last_timeout The timeout value.
 *
 * This functions sets the time out value for which the last input entered in password
 * mode will be visible.
 *
 * This value can be used only when last show mode is set in password mode.
 *
 * @see edje_password_show_last_set().
 *
 */
EAPI void edje_password_show_last_timeout_set(double password_show_last_timeout);

/**
 * @brief Set the scaling factor for a given Edje object.
 *
 * @param obj A handle to an Edje object
 * @param scale The scaling factor (the default value is @c 0.0,
 * meaning indivinual scaling @b not set)
 *
 * This function sets an @b individual scaling factor on the @a obj
 * Edje object. This property (or Edje's global scaling factor, when
 * applicable), will affect this object's part sizes. If @p scale is
 * not zero, than the individual scaling will @b override any global
 * scaling set, for the object @p obj's parts. Put it back to zero to
 * get the effects of the global scaling again.
 *
 * @warning Only parts which, at EDC level, had the @c "scale"
 * property set to @c 1, will be affected by this function. Check the
 * complete @ref edcref "syntax reference" for EDC files.
 *
 * @see edje_object_scale_get()
 * @see edje_scale_get() for more details
 */
EAPI Eina_Bool    edje_object_scale_set           (Evas_Object *obj, double scale);

/**
 * @brief Get a given Edje object's scaling factor.
 *
 * @param obj A handle to an Edje object
 *
 * This function returns the @c individual scaling factor set on the
 * @a obj Edje object.
 *
 * @see edje_object_scale_set() for more details
 *
 */
EAPI double       edje_object_scale_get           (const Evas_Object *obj);

/**
 * Set the RTL orientation for this object.
 *
 * @param obj the smart object
 * @rtl new value of flag EINA_TRUE/EINA_FALSE
 * @since 1.1.0
 */
EAPI void         edje_object_mirrored_set        (Evas_Object *obj, Eina_Bool rtl);

/**
 * Get the RTL orientation for this object.
 *
 * You can RTL orientation explicitly with edje_object_mirrored_set.
 *
 * @param obj the smart object
 * @return if flag is set or not.
 * @since 1.1.0
 */
EAPI Eina_Bool    edje_object_mirrored_get        (const Evas_Object *obj);

/**
 * Get a list of groups in an edje file
 * @param file The path to the edje file
 *
 * @return The Eina_List of group names (char *)
 *
 * Note: the list must be freed using edje_file_collection_list_free()
 * when you are done with it.
 */
EAPI Eina_List   *edje_file_collection_list       (const char *file);

/**
 * Free file collection list
 * @param lst The Eina_List of groups
 *
 * Frees the list returned by edje_file_collection_list().
 */
EAPI void         edje_file_collection_list_free  (Eina_List *lst);

/**
 * Determine whether a group matching glob exists in an edje file.
 * @param file The file path
 * @param glob A glob to match on
 *
 * @return 1 if a match is found, 0 otherwise
 */
EAPI Eina_Bool    edje_file_group_exists          (const char *file, const char *glob);

/**
 * Get data from the file level data block of an edje file
 * @param file The path to the .edj file
 * @param key The data key
 * @return The string value of the data
 *
 * If an edje file is built from the following edc:
 *
 * data {
 *   item: "key1" "value1";
 *   item: "key2" "value2";
 * }
 * collections { ... }
 *
 * Then, edje_file_data_get("key1") will return "value1"
 */
EAPI char        *edje_file_data_get              (const char *file, const char *key);

/**
 * @brief Set the file cache size.
 *
 * @param count The file cache size in edje file units. Default is 16.
 *
 * This function sets the file cache size. Edje keeps this cache in
 * order to prevent duplicates of edje file entries in memory. The
 * file cache size can be retrieved with edje_file_cache_get().
 *
 * @see edje_file_cache_get()
 * @see edje_file_cache_flush()
 *
 */
EAPI void         edje_file_cache_set             (int count);

/**
 * @brief Return the file cache size.
 *
 * @return The file cache size in edje file units. Default is 16.
 *
 * This function returns the file cache size set by
 * edje_file_cache_set().
 *
 * @see edje_file_cache_set()
 * @see edje_file_cache_flush()
 *
 */
EAPI int          edje_file_cache_get             (void);

/**
 * @brief Clean the file cache.
 *
 * This function cleans the file cache entries, but keeps this cache's
 * size to the last value set.
 *
 * @see edje_file_cache_set()
 * @see edje_file_cache_get()
 *
 */
EAPI void         edje_file_cache_flush           (void);

/**
 * @brief Set the collection cache size.
 *
 * @param count The collection cache size, in edje object units. Default is 16.
 *
 * This function sets the collection cache size. Edje keeps this cache
 * in order to prevent duplicates of edje {collection,group,part}
 * entries in memory. The collection cache size can be retrieved with
 * edje_collection_cache_get().
 *
 * @see edje_collection_cache_get()
 * @see edje_collection_cache_flush()
 *
 */
EAPI void         edje_collection_cache_set       (int count);

/**
 * @brief Return the collection cache size.
 *
 * @return The collection cache size, in edje object units. Default is 16.
 *
 * This function returns the collection cache size set by
 * edje_collection_cache_set().
 *
 * @see edje_collection_cache_set()
 * @see edje_collection_cache_flush()
 *
 */
EAPI int          edje_collection_cache_get       (void);

/**
 * @brief Clean the collection cache.
 *
 * This function cleans the collection cache, but keeps this cache's
 * size to the last value set.
 *
 * @see edje_collection_cache_set()
 * @see edje_collection_cache_get()
 *
 */
EAPI void         edje_collection_cache_flush     (void);

/**
 * @brief Set Edje color class.
 *
 * @param color_class
 * @param r Object Red value
 * @param g Object Green value
 * @param b Object Blue value
 * @param a Object Alpha value
 * @param r2 Outline Red value
 * @param g2 Outline Green value
 * @param b2 Outline Blue value
 * @param a2 Outline Alpha value
 * @param r3 Shadow Red value
 * @param g3 Shadow Green value
 * @param b3 Shadow Blue value
 * @param a3 Shadow Alpha value
 *
 * This function sets the color values for a process level color
 * class.  This will cause all edje parts in the current process that
 * have the specified color class to have their colors multiplied by
 * these values.  (Object level color classes set by
 * edje_object_color_class_set() will override the values set by this
 * function).
 *
 * The first color is the object, the second is the text outline, and
 * the third is the text shadow. (Note that the second two only apply
 * to text parts).
 *
 * Setting color emits a signal "color_class,set" with source being
 * the given color class in all objects.
 *
 * @see edje_color_class_set().
 *
 * @note unlike Evas, Edje colors are @b not pre-multiplied. That is,
 *       half-transparent white is 255 255 255 128.
 */
EAPI Eina_Bool    edje_color_class_set            (const char *color_class, int r, int g, int b, int a, int r2, int g2, int b2, int a2, int r3, int g3, int b3, int a3);

/**
 * @brief Get Edje color class.
 *
 * @param color_class
 * @param r Object Red value
 * @param g Object Green value
 * @param b Object Blue value
 * @param a Object Alpha value
 * @param r2 Outline Red value
 * @param g2 Outline Green value
 * @param b2 Outline Blue value
 * @param a2 Outline Alpha value
 * @param r3 Shadow Red value
 * @param g3 Shadow Green value
 * @param b3 Shadow Blue value
 * @param a3 Shadow Alpha value
 *
 * @return EINA_TRUE if found or EINA_FALSE if not found and all
 *         values are zeroed.
 *
 * This function gets the color values for a process level color
 * class. This value is the globally set and not per-object, that is,
 * the value that would be used by objects if they did not override with
 * edje_object_color_class_set().
 *
 * The first color is the object, the second is the text outline, and
 * the third is the text shadow. (Note that the second two only apply
 * to text parts).
 *
 * @see edje_color_class_set().
 *
 * @note unlike Evas, Edje colors are @b not pre-multiplied. That is,
 *       half-transparent white is 255 255 255 128.
 */
EAPI Eina_Bool    edje_color_class_get            (const char *color_class, int *r, int *g, int *b, int *a, int *r2, int *g2, int *b2, int *a2, int *r3, int *g3, int *b3, int *a3);

/**
 * @brief Delete the object color class.
 *
 * @param obj The edje object's reference.
 * @param color_class The color class to be deleted.
 *
 * This function deletes any values at the object level for the
 * specified object and color class.
 *
 * Deleting color emits a signal "color_class,del" with source being
 * the given color.
 */
EAPI void         edje_color_class_del            (const char *color_class);
   EAPI Eina_List   *edje_color_class_list           (void);

/**
 * @brief Set the Edje text class.
 *
 * @param text_class The text class name
 * @param font The font name
 * @param size The font size
 *
 * This function sets updates all edje members which belong to this
 * text class with the new font attributes.
 *
 * @see edje_text_class_get().
 *
 */
EAPI Eina_Bool    edje_text_class_set             (const char *text_class, const char *font, Evas_Font_Size size);

/**
 * @brief Delete the text class.
 *
 * @param text_class The text class name string
 *
 * This function deletes any values at the process level for the
 * specified text class.
 *
 */
EAPI void         edje_text_class_del             (const char *text_class);

/**
 * @brief List text classes.
 *
 * @return A list of text class names (strings). These strings are
 * stringshares and the list must be free()'d by the caller.
 *
 * This function lists all text classes known about by the current
 * process.
 *
 */
EAPI Eina_List   *edje_text_class_list            (void);

/**
 * @brief Set the object minimum size.
 *
 * @param obj A valid Evas_Object handle
 * @param minw The minimum width
 * @param minh The minimum height
 *
 * This sets the minimum size restriction for the object.
 */
EAPI void         edje_extern_object_min_size_set (Evas_Object *obj, Evas_Coord minw, Evas_Coord minh);

/**
 * @brief Set the object maximum size.
 *
 * @param obj A valid Evas_Object handle
 * @param maxw The maximum width
 * @param maxh The maximum height
 *
 * This sets the maximum size restriction for the object.
 */
EAPI void         edje_extern_object_max_size_set (Evas_Object *obj, Evas_Coord maxw, Evas_Coord maxh);

/**
 * @brief Set the object aspect size.
 *
 * @param obj A valid Evas_Object handle
 * @param aspect The aspect control axes
 * @param aw The aspect radio width
 * @param ah The aspect ratio height
 *
 * This sets the desired aspect ratio to keep an object that will be
 * swallowed by Edje. The width and height define a preferred size
 * ASPECT and the object may be scaled to be larger or smaller, but
 * retaining the relative scale of both aspect width and height.
 */
EAPI void         edje_extern_object_aspect_set   (Evas_Object *obj, Edje_Aspect_Control aspect, Evas_Coord aw, Evas_Coord ah);

/**
 * @brief Registers a custom layout to be used in edje boxes.
 *
 * @param name The name of the layout
 * @param func The function defining the layout
 * @param layout_data_get This function gets the custom data pointer
 * for func
 * @param layout_data_free Passed to func to free its private data
 * when needed
 * @param free_data Frees data
 * @param data Private pointer passed to layout_data_get
 *
 * This function registers custom layouts that can be referred from
 * themes by the registered name. The Evas_Object_Box_Layout
 * functions receive two pointers for internal use, one being private
 * data, and the other the function to free that data when it's not
 * longer needed. From Edje, this private data will be retrieved by
 * calling layout_data_get, and layout_data_free will be the free
 * function passed to func. layout_data_get will be called with data
 * as its parameter, and this one will be freed by free_data whenever
 * the layout is unregistered from Edje.
 */
EAPI void         edje_box_layout_register        (const char *name, Evas_Object_Box_Layout func, void *(*layout_data_get)(void *), void (*layout_data_free)(void *), void (*free_data)(void *), void *data);

/**
 * @brief Instantiate a new Edje object
 *
 * @param evas A valid Evas handle, the canvas to place the new object
 * in
 * @return A handle to the new object created or @c NULL, on errors.
 *
 * This function creates a new Edje smart object, returning its @c
 * Evas_Object handle. An Edje object is useless without a (source)
 * file set to it, so you'd most probably call edje_object_file_set()
 * afterwards, like in:
 * @code
 * Evas_Object *edje;
 *
 * edje = edje_object_add(canvas);
 * if (!edje)
 *   {
 *      fprintf(stderr, "could not create edje object!\n");
 *      return NULL;
 *   }
 *
 * if (!edje_object_file_set(edje, "theme.edj", "group_name"))
 *   {
 *      int err = edje_object_load_error_get(edje);
 *      const char *errmsg = edje_load_error_str(err);
 *      fprintf(stderr, "could not load 'group_name' from theme.edj: %s",
 *      	errmsg);
 *
 *      evas_object_del(edje);
 *      return NULL;
 *   }
 *
 * @endcode
 *
 * @note before creating the first Edje object in your code, remember
 * to initialize the library, with edje_init(), or unexpected behavior
 * might occur.
 */
EAPI Evas_Object *edje_object_add                 (Evas *evas);

/**
 * @brief Retrive an <b>EDC data field's value</b> from a given Edje
 * object's group.
 *
 * @param obj A handle to an Edje object
 * @param key The data field's key string
 * @return The data's value string
 *
 * This function fetches an EDC data field's value, which is declared
 * on the objects building EDC file, <b>under its group</b>. EDC data
 * blocks are most commonly used to pass arbitrary parameters from an
 * application's theme to its code.
 *
 * They look like the following:
 *
 * @code
 * collections {
 *   group {
 *     name: "a_group";
 *     data {
 *	 item: "key1" "value1";
 *	 item: "key2" "value2";
 *     }
 *   }
 * }
 * @endcode
 *
 * EDC data fields always hold @b strings as values, hence the return
 * type of this function. Check the complete @ref edcref "syntax reference"
 * for EDC files.
 *
 * @warning Do not confuse this call with edje_file_data_get(), which
 * queries for a @b global EDC data field on an EDC declaration file.
 *
 * @see edje_object_file_set()
 */
EAPI const char  *edje_object_data_get            (const Evas_Object *obj, const char *key);

/**
 * @brief Sets the @b EDJ file (and group within it) to load an Edje
 * object's contents from
 *
 * @param obj A handle to an Edje object
 * @param file The path to the EDJ file to load @p from
 * @param group The name of the group, in @p file, which implements an
 * Edje object
 * @return @c EINA_TRUE, on success or @c EINA_FALSE, on errors (check
 * edje_object_load_error_get() after this call to get errors causes)
 *
 * Edje expects EDJ files, which are theming objects' descriptions and
 * resources packed together in an EET file, to read Edje object
 * definitions from. They usually are created with the @c .edj
 * extension. EDJ files, in turn, are assembled from @b textual object
 * description files, where one describes Edje objects declaratively
 * -- the EDC files (see @ref edcref "the syntax" for those files).
 *
 * Those description files were designed so that many Edje object
 * definitions -- also called @b groups (or collections) -- could be
 * packed together <b>in the same EDJ file</b>, so that a whole
 * application's theme could be packed in one file only. This is the
 * reason for the @p group argument.
 *
 * Use this function after you instantiate a new Edje object, so that
 * you can "give him life", telling where to get its contents from.
 *
 * @see edje_object_add()
 * @see edje_object_file_get()
 */
EAPI Eina_Bool        edje_object_file_set        (Evas_Object *obj, const char *file, const char *group);

/**
 * @brief Get the file and group name that a given Edje object is bound to
 *
 * @param obj A handle to an Edje object
 * @param file A pointer to a variable whero to store the <b>file's
 * path</b>
 * @param group A pointer to a variable where to store the <b>group
 * name</b> in
 *
 * This gets the EDJ file's path, with the respective group set for
 * the given Edje object. If @a obj is either not an Edje file, or has
 * not had its file/group set previously, by edje_object_file_set(),
 * then both @p file and @p group will be set to @c NULL, indicating
 * an error.
 *
 * @see edje_object_file_set()
 *
 * @note Use @c NULL pointers on the file/group components you're not
 * interested in: they'll be ignored by the function.
 */
EAPI void             edje_object_file_get        (const Evas_Object *obj, const char **file, const char **group);

/**
 * @brief Gets the (last) file loading error for a given Edje object
 *
 * @param obj A handlet to an Edje object
 *
 * @return The Edje loading error, one of:
 * - #EDJE_LOAD_ERROR_NONE
 * - #EDJE_LOAD_ERROR_GENERIC
 * - #EDJE_LOAD_ERROR_DOES_NOT_EXIST
 * - #EDJE_LOAD_ERROR_PERMISSION_DENIED
 * - #EDJE_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED
 * - #EDJE_LOAD_ERROR_CORRUPT_FILE
 * - #EDJE_LOAD_ERROR_UNKNOWN_FORMAT
 * - #EDJE_LOAD_ERROR_INCOMPATIBLE_FILE
 * - #EDJE_LOAD_ERROR_UNKNOWN_COLLECTION
 * - #EDJE_LOAD_ERROR_RECURSIVE_REFERENCE
 *
 * This function is meant to be used after an Edje EDJ <b>file
 * loading</b>, what takes place with the edje_object_file_set()
 * function. If that function does not return @c EINA_TRUE, one should
 * check for the reason of failure with this one.
 *
 * @see edje_load_error_str()
 */
EAPI Edje_Load_Error  edje_object_load_error_get  (const Evas_Object *obj);

/**
 * Converts the given Edje file load error code into a string
 * describing it in English.
 *
 * @param error the error code, a value in ::Edje_Load_Error.
 * @return Always returns a valid string. If the given @p error is not
 *         supported, <code>"Unknown error"</code> is returned.
 *
 * edje_object_file_set() is a function which sets an error value,
 * afterwards, which can be fetched with
 * edje_object_load_error_get(). The function in question is meant
 * to be used in conjunction with the latter, for pretty-printing any
 * possible error cause.
 */
EAPI const char      *edje_load_error_str         (Edje_Load_Error error);

EAPI Eina_Bool        edje_object_preload         (Evas_Object *obj, Eina_Bool cancel);

/**
 * @brief Add a callback for an arriving Edje signal, emitted by
 * a given Ejde object.
 *
 * @param obj A handle to an Edje object
 * @param emission The signal's "emission" string
 * @param source The signal's "source" string
 * @param func The callback function to be executed when the signal is
 * emitted.
 * @param data A pointer to data to pass in to @p func.
 *
 * Edje signals are one of the communication interfaces between
 * @b code and a given Edje object's @b theme. With signals, one can
 * communicate two string values at a time, which are:
 * - "emission" value: the name of the signal, in general
 * - "source" value: a name for the signal's context, in general
 *
 * Though there are those common uses for the two strings, one is free
 * to use them however they like.
 *
 * This function adds a callback function to a signal emitted by @a obj, to
 * be issued every time an EDC program like the following
 * @code
 * program {
 *   name: "emit_example";
 *   action: SIGNAL_EMIT "a_signal" "a_source";
 * }
 * @endcode
 * is run, if @p emission and @p source are given those same values,
 * here.
 *
 * Signal callback registration is powerful, in the way that @b blobs
 * may be used to match <b>multiple signals at once</b>. All the @c
 * "*?[\" set of @c fnmatch() operators can be used, both for @p
 * emission and @p source.
 *
 * Edje has @b internal signals it will emit, automatically, on
 * various actions taking place on group parts. For example, the mouse
 * cursor being moved, pressed, released, etc., over a given part's
 * area, all generate individual signals.
 *
 * By using something like
 * @code
 * edje_object_signal_callback_add(obj, "mouse,down,*", "button.*",
 *                                 signal_cb, NULL);
 * @endcode
 * being @c "button.*" the pattern for the names of parts implementing
 * buttons on an interface, you'd be registering for notifications on
 * events of mouse buttons being pressed down on either of those parts
 * (those events all have the @c "mouse,down," common prefix on their
 * names, with a suffix giving the button number). The actual emisson
 * and source strings of an event will be passed in as the @a emission
 * and @a source parameters of the callback function (e.g. @c
 * "mouse,down,2" and @c "button.close"), for each of those events.
 *
 * @note See @ref edcref "the syntax" for EDC files
 * @see edje_object_signal_emit() on how to emits Edje signals from
 * code to a an object
 * @see edje_object_signal_callback_del_full()
 */
EAPI void         edje_object_signal_callback_add (Evas_Object *obj, const char *emission, const char *source, Edje_Signal_Cb func, void *data);

/**
 * @brief Remove a signal-triggered callback from an object.
 *
 * @param obj A valid Evas_Object handle.
 * @param emission The emission string.
 * @param source The source string.
 * @param func The callback function.
 * @return The data pointer
 *
 * This function removes a callback, previously attached to the
 * emittion of a signal, from the object @a obj. The parameters @a
 * emission, @a source and @a func must match exactly those passed to
 * a previous call to edje_object_signal_callback_add(). The data
 * pointer that was passed to this call will be returned.
 *
 * @see edje_object_signal_callback_add().
 * @see edje_object_signal_callback_del_full().
 *
 */
EAPI void        *edje_object_signal_callback_del (Evas_Object *obj, const char *emission, const char *source, Edje_Signal_Cb func);

/**
 * @brief Unregister/delete a callback set for an arriving Edje
 * signal, emitted by a given Ejde object.
 *
 * @param obj A handle to an Edje object
 * @param emission The signal's "emission" string
 * @param source The signal's "source" string
 * @param func The callback function passed on the callback's
 * registration
 * @param data The pointer given to be passed as data to @p func
 * @return @p data, on success or @c NULL, on errors (or if @p data
 * had this value)
 *
 * This function removes a callback, previously attached to the
 * emittion of a signal, from the object @a obj. The parameters
 * @a emission, @a source, @a func and @a data must match exactly those
 * passed to a previous call to edje_object_signal_callback_add(). The
 * data pointer that was passed to this call will be returned.
 *
 * @see edje_object_signal_callback_add().
 * @see edje_object_signal_callback_del().
 *
 */
EAPI void        *edje_object_signal_callback_del_full(Evas_Object *obj, const char *emission, const char *source, Edje_Signal_Cb func, void *data);

/**
 * @brief Send/emit an Edje signal to a given Edje object
 *
 * @param obj A handle to an Edje object
 * @param emission The signal's "emission" string
 * @param source The signal's "source" string
 *
 * This function sends a signal to the object @a obj. An Edje program,
 * at @p obj's EDC specification level, can respond to a signal by
 * having declared matching @c 'signal' and @c 'source' fields on its
 * block (see @ref edcref "the syntax" for EDC files).
 *
 * As an example,
 * @code
 * edje_object_signal_emit(obj, "a_signal", "");
 * @endcode
 * would trigger a program which had an EDC declaration block like
 * @code
 * program {
 *  name: "a_program";
 *  signal: "a_signal";
 *  source: "";
 *  action: ...
 * }
 * @endcode
 *
 * @see edje_object_signal_callback_add() for more on Edje signals.
 */
EAPI void         edje_object_signal_emit         (Evas_Object *obj, const char *emission, const char *source);

/**
 * @brief Set the edje object to playing or paused states.
 *
 * @param obj A valid Evas_Object handle.
 * @param play Object state (1 to playing, 0 to pauseed).
 *
 * This function sets the edje object @a obj to playing or paused
 * states, depending on the parameter @a play.  This has no effect if
 * the object was already at that state.
 *
 * @see edje_object_play_get().
 *
 */
EAPI void         edje_object_play_set            (Evas_Object *obj, Eina_Bool play);

/**
 * @brief Get the edje object's play/pause state.
 *
 * @param obj A valid Evas_Object handle.
 * @return @c EINA_FALSE if the object is not connected, its @c delete_me flag
 * is set, or it is at paused state; @c EINA_TRUE if the object is at playing
 * state.
 *
 * This function tells if an edje object is playing or not. This state
 * is set by edje_object_play_set().
 *
 * @see edje_object_play_set().
 *
 */
EAPI Eina_Bool    edje_object_play_get            (const Evas_Object *obj);

/**
 * @brief Set the object's animation state.
 *
 * @param obj A valid Evas_Object handle.
 * @param on Animation State.
 *
 * This function starts or stops an edje object's animation. The
 * information if it's runnig can be retrieved by
 * edje_object_animation_get().
 *
 * @see edje_object_animation_get()
 *
 */
EAPI void         edje_object_animation_set       (Evas_Object *obj, Eina_Bool on);

/**
 * @brief Get the edje object's animation state.
 *
 * @param obj A valid Evas_Object handle.
 * @return @c EINA_FALSE on error or if object is not animated;
 * @c EINA_TRUE if animated.
 *
 * This function returns if the animation is playing or not. The
 * animation state is set by edje_object_play_set().
 *
 * @see edje_object_animation_set().
 *
 */
EAPI Eina_Bool    edje_object_animation_get       (const Evas_Object *obj);

/**
 * @brief Freeze object.
 *
 * @param obj A valid Evas_Object handle
 * @return The frozen state or 0 on Error
 *
 * This function puts all changes on hold. Successive freezes will
 * nest, requiring an equal number of thaws.
 *
 */
EAPI int          edje_object_freeze                  (Evas_Object *obj);

/**
 * @brief Thaw object.
 *
 * @param obj A valid Evas_Object handle
 * @return The frozen state or 0 on Error
 *
 * This allows frozen changes to occur.
 *
 */
EAPI int          edje_object_thaw                    (Evas_Object *obj);

/**
 * @brief Sets the object color class.
 *
 * @param obj A valid Evas_Object handle
 * @param color_class
 * @param r Object Red value
 * @param g Object Green value
 * @param b Object Blue value
 * @param a Object Alpha value
 * @param r2 Outline Red value
 * @param g2 Outline Green value
 * @param b2 Outline Blue value
 * @param a2 Outline Alpha value
 * @param r3 Shadow Red value
 * @param g3 Shadow Green value
 * @param b3 Shadow Blue value
 * @param a3 Shadow Alpha value
 *
 * This function sets the color values for an object level color
 * class. This will cause all edje parts in the specified object that
 * have the specified color class to have their colors multiplied by
 * these values.
 *
 * The first color is the object, the second is the text outline, and
 * the third is the text shadow. (Note that the second two only apply
 * to text parts).
 *
 * Setting color emits a signal "color_class,set" with source being
 * the given color.
 *
 * @note unlike Evas, Edje colors are @b not pre-multiplied. That is,
 *       half-transparent white is 255 255 255 128.
 */

/**
 * @brief Lists color classes.
 *
 * @return A list of color class names (strings). These strings and
 * the list must be free()'d by the caller.
 *
 * This function lists all color classes known about by the current
 * process.
 *
 */

/**
 * @brief Delete edje color class.
 *
 * @param color_class
 *
 * This function deletes any values at the process level for the
 * specified color class.
 *
 * Deleting color emits a signal "color_class,del" with source being
 * the given color class in all objects.
 */
EAPI Eina_Bool    edje_object_color_class_set         (Evas_Object *obj, const char *color_class, int r, int g, int b, int a, int r2, int g2, int b2, int a2, int r3, int g3, int b3, int a3);

/**
 * @brief Gets the object color class.
 *
 * @param obj A valid Evas_Object handle
 * @param color_class
 * @param r Object Red value
 * @param g Object Green value
 * @param b Object Blue value
 * @param a Object Alpha value
 * @param r2 Outline Red value
 * @param g2 Outline Green value
 * @param b2 Outline Blue value
 * @param a2 Outline Alpha value
 * @param r3 Shadow Red value
 * @param g3 Shadow Green value
 * @param b3 Shadow Blue value
 * @param a3 Shadow Alpha value
 *
 * @return EINA_TRUE if found or EINA_FALSE if not found and all
 *         values are zeroed.
 *
 * This function gets the color values for an object level color
 * class. If no explicit object color is set, then global values will
 * be used.
 *
 * The first color is the object, the second is the text outline, and
 * the third is the text shadow. (Note that the second two only apply
 * to text parts).
 *
 * @note unlike Evas, Edje colors are @b not pre-multiplied. That is,
 *       half-transparent white is 255 255 255 128.
 */
EAPI Eina_Bool    edje_object_color_class_get         (const Evas_Object *o, const char *color_class, int *r, int *g, int *b, int *a, int *r2, int *g2, int *b2, int *a2, int *r3, int *g3, int *b3, int *a3);
   EAPI void         edje_object_color_class_del         (Evas_Object *obj, const char *color_class);

/**
 * @brief Sets Edje text class.
 *
 * @param obj A valid Evas_Object handle
 * @param text_class The text class name
 * @param font Font name
 * @param size Font Size
 *
 * This function sets the text class for the Edje.
 *
 */
EAPI Eina_Bool    edje_object_text_class_set          (Evas_Object *obj, const char *text_class, const char *font, Evas_Font_Size size);

/**
 * @brief Get the minimum size specified -- as an EDC property -- for a
 * given Edje object
 *
 * @param obj A handle to an Edje object
 * @param minw Pointer to a variable where to store the minimum width
 * @param minh Pointer to a variable where to store the minimum height
 *
 * This function retrieves the @p obj object's minimum size values,
 * <b>as declared in its EDC group definition</b>. Minimum size of
 * groups have the following syntax
 * @code
 * collections {
 *   group {
 *     name: "a_group";
 *     min: 100 100;
 *   }
 * }
 * @endcode
 *
 * where one declares a minimum size of 100 pixels both for width and
 * height. Those are (hint) values which should be respected when the
 * given object/group is to be controlled by a given container object
 * (e.g. an Edje object being "swallowed" into a given @c SWALLOW
 * typed part, as in edje_object_part_swallow()). Check the complete
 * @ref edcref "syntax reference" for EDC files.
 *
 * @note If the @c min EDC property was not declared for @p obj, this
 * call will return the value 0, for each axis.
 *
 * @note On failure, this function will make all non-@c NULL size
 * pointers' pointed variables be set to zero.
 *
 * @see edje_object_size_max_get()
 */
EAPI void         edje_object_size_min_get            (const Evas_Object *obj, Evas_Coord *minw, Evas_Coord *minh);

/**
 * @brief Get the maximum size specified -- as an EDC property -- for a
 * given Edje object
 *
 * @param obj A handle to an Edje object
 * @param maxw Pointer to a variable where to store the maximum width
 * @param maxh Pointer to a variable where to store the maximum height
 *
 * This function retrieves the @p obj object's maximum size values,
 * <b>as declared in its EDC group definition</b>. Maximum size of
 * groups have the following syntax
 * @code
 * collections {
 *   group {
 *     name: "a_group";
 *     max: 100 100;
 *   }
 * }
 * @endcode
 *
 * where one declares a maximum size of 100 pixels both for width and
 * height. Those are (hint) values which should be respected when the
 * given object/group is to be controlled by a given container object
 * (e.g. an Edje object being "swallowed" into a given @c SWALLOW
 * typed part, as in edje_object_part_swallow()). Check the complete
 * @ref edcref "syntax reference" for EDC files.
 *
 * @note If the @c max EDC property was not declared for @p obj, this
 * call will return the maximum size a given Edje object may have, for
 * each axis.
 *
 * @note On failure, this function will make all non-@c NULL size
 * pointers' pointed variables be set to zero.
 *
 * @see edje_object_size_min_get()
 */
EAPI void         edje_object_size_max_get            (const Evas_Object *obj, Evas_Coord *maxw, Evas_Coord *maxh);

/**
 * @brief Force a Size/Geometry calculation.
 *
 * @param obj A valid Evas_Object handle
 *
 * Forces the object @p obj to recalculation layout regardless of
 * freeze/thaw.
 */
EAPI void         edje_object_calc_force              (Evas_Object *obj);

/**
 * @brief Calculate the minimum required size for a given Edje object.
 *
 * @param obj A handle to an Edje object
 * @param minw Pointer to a variable where to store the minimum
 * required width
 * @param minh Pointer to a variable where to store the minimum
 * required height
 *
 * This call works exactly as edje_object_size_min_restricted_calc(),
 * with the last two arguments set to 0. Please refer to its
 * documentation, then.
 */
EAPI void         edje_object_size_min_calc           (Evas_Object *obj, Evas_Coord *minw, Evas_Coord *minh);

/**
 * Calculate the geometry of the region, relative to a given Edje
 * object's area, <b>occupied by all parts in the object</b>
 *
 * @param obj A handle to an Edje object
 * @param part The Edje part's name
 * @param x A pointer to a variable where to store the parts region's
 * x coordinate
 * @param y A pointer to a variable where to store the parts region's
 * y coordinate
 * @param w A pointer to a variable where to store the parts region's
 * width
 * @param h A pointer to a variable where to store the parts region's
 * height
 *
 * This function gets the geometry of the rectangle equal to the area
 * required to group all parts in @p obj's group/collection. The @p x
 * and @p y coordinates are relative to the top left corner of the
 * whole @p obj object's area. Parts placed out of the group's
 * boundaries will also be taken in account, so that @p x and @p y
 * <b>may be negative</b>.
 *
 * @note Use @c NULL pointers on the geometry components you're not
 * interested in: they'll be ignored by the function.
 *
 * @note On failure, this function will make all non-@c NULL geometry
 * pointers' pointed variables be set to zero.
 */
EAPI Eina_Bool    edje_object_parts_extends_calc      (Evas_Object *obj, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h);

/**
 * @brief Calculate the minimum required size for a given Edje object.
 *
 * @param obj A handle to an Edje object
 * @param minw Pointer to a variable where to store the minimum
 * required width
 * @param minh Pointer to a variable where to store the minimum
 * required height
 * @param restrictedw Do not allow object's calculated (minimum) width
 * to be less than this value
 * @param restrictedh Do not allow object's calculated (minimum)
 * height to be less than this value
 *
 * This call will trigger an internal recalculation of all parts of
 * the @p obj object, in order to return its minimum required
 * dimensions for width and height. The user might choose to @b impose
 * those minimum sizes, making the resulting calculation to get to values
 * equal or bigger than @p restrictedw and @p restrictedh, for width and
 * height, respectively.
 *
 * @note At the end of this call, @p obj @b won't be automatically
 * resized to new dimensions, but just return the calculated
 * sizes. The caller is the one up to change its geometry or not.
 *
 * @warning Be advised that invisible parts in @p obj @b will be taken
 * into account in this calculation.
 */
EAPI void         edje_object_size_min_restricted_calc(Evas_Object *obj, Evas_Coord *minw, Evas_Coord *minh, Evas_Coord restrictedw, Evas_Coord restrictedh);

/**
 * @brief Check if an Edje part exists in a given Edje object's group
 * definition..
 *
 * @param obj A handle to an Edje object
 * @param part The part's name to check for existence in @p obj's
 * group
 * @return @c EINA_TRUE, if the Edje part exists in @p obj's group or
 * @c EINA_FALSE, otherwise (and on errors)
 *
 * This function returns if a given part exists in the Edje group
 * bound to object @p obj (with edje_object_file_set()).
 *
 * This call is useful, for example, when one could expect or not a
 * given GUI element, depending on the @b theme applied to @p obj.
 */
EAPI Eina_Bool    edje_object_part_exists             (const Evas_Object *obj, const char *part);

/**
 * @brief Get a handle to the Evas object implementing a given Edje
 * part, in an Edje object.
 *
 * @param obj A handle to an Edje object
 * @param part The Edje part's name
 * @return A pointer to the Evas object implementing the given part,
 * or @c NULL on failure (e.g. the given part doesn't exist)
 *
 * This function gets a pointer the Evas object corresponding to a
 * given part in the @p obj object's group.
 *
 * You should @b never modify the state of the returned object (with
 * @c evas_object_move() or @c evas_object_hide() for example),
 * because it's meant to be managed be Edje, solely. You are safe to
 * query information about its current state (with @c
 * evas_object_visible_get() or @c evas_object_color_get() for
 * example), though.
 */
EAPI const Evas_Object *edje_object_part_object_get   (const Evas_Object *obj, const char *part);

/**
 * @brief Retrieve the geometry of a given Edje part, in a given Edje
 * object's group definition, <b>relative to the object's area</b>
 *
 * @param obj A handle to an Edje object
 * @param part The Edje part's name
 * @param x A pointer to a variable where to store the part's x
 * coordinate
 * @param y A pointer to a variable where to store the part's y
 * coordinate
 * @param w A pointer to a variable where to store the part's width
 * @param h A pointer to a variable where to store the part's height
 *
 * This function gets the geometry of an Edje part within its
 * group. The @p x and @p y coordinates are relative to the top left
 * corner of the whole @p obj object's area.
 *
 * @note Use @c NULL pointers on the geometry components you're not
 * interested in: they'll be ignored by the function.
 *
 * @note On failure, this function will make all non-@c NULL geometry
 * pointers' pointed variables be set to zero.
 */
EAPI Eina_Bool    edje_object_part_geometry_get       (const Evas_Object *obj, const char *part, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h);


/**
 * @brief Set the function that provides item objects for named items in an edje entry text
 *
 * @param obj A valid Evas Object handle
 * @param func The function to call (or NULL to disable) to get item objects
 * @param data The data pointer to pass to the @p func callback
 *
 * Item objects may be deleted any time by Edje, and will be deleted when the
 * Edje object is deleted (or file is set to a new file).
 */
EAPI void         edje_object_item_provider_set       (Evas_Object *obj, Edje_Item_Provider_Cb func, void *data);


/**
 * @brief Set the object text callback.
 *
 * @param obj A valid Evas_Object handle
 * @param func The callback function to handle the text change
 * @param data The data associated to the callback function.
 *
 * This function sets the callback to be called when the text changes.
 */
EAPI void         edje_object_text_change_cb_set      (Evas_Object *obj, Edje_Text_Change_Cb func, void *data);

/**
 * @brief Sets the text for an object part
 *
 * @param obj A valid Evas Object handle
 * @param part The part name
 * @param text The text string
 */
EAPI Eina_Bool    edje_object_part_text_set           (Evas_Object *obj, const char *part, const char *text);

/**
 * @brief Return the text of the object part.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 *
 * @return The text string
 *
 * This function returns the text associated to the object part.
 *
 * @see edje_object_part_text_set().
 */
EAPI const char  *edje_object_part_text_get           (const Evas_Object *obj, const char *part);

/**
 * @brief Sets the raw (non escaped) text for an object part.
 *
 * @param obj A valid Evas Object handle
 * @param part The part name
 * @param text_to_escape The text string
 *
 * This funciton will not do escape for you if it is a TEXTBLOCK part, that is,
 * if text contain tags, these tags will not be interpreted/parsed by TEXTBLOCK.
 *
 * @see edje_object_part_text_unescaped_get().
 */
EAPI Eina_Bool    edje_object_part_text_unescaped_set (Evas_Object *obj, const char *part, const char *text_to_escape);

/**
 * @brief Returns the text of the object part, without escaping.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @return The @b allocated text string without escaping, or NULL on
 * problems.
 *
 * This function is the counterpart of
 * edje_object_part_text_unescaped_set(). Please notice that the
 * result is newly allocated memory and should be released with free()
 * when done.
 *
 * @see edje_object_part_text_unescaped_set().
 */
EAPI char        *edje_object_part_text_unescaped_get (const Evas_Object *obj, const char *part);

/**
 * @brief Return the selection text of the object part.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @return The text string
 *
 * This function returns selection text of the object part.
 *
 * @see edje_object_part_text_select_all()
 * @see edje_object_part_text_select_none()
 */
EAPI const char      *edje_object_part_text_selection_get           (const Evas_Object *obj, const char *part);

/**
 * @brief Set the selection to be none.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 *
 * This function sets the selection text to be none.
 */
EAPI void             edje_object_part_text_select_none             (const Evas_Object *obj, const char *part);

/**
 * @brief Set the selection to be everything.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 *
 * This function selects all text of the object of the part.
 */
EAPI void             edje_object_part_text_select_all              (const Evas_Object *obj, const char *part);

/**
 * @brief Insert text for an object part.
 *
 * @param obj A valid Evas Object handle
 * @param part The part name
 * @param text The text string
 *
 * This function inserts the text for an object part just before the
 * cursor position.
 *
 */
EAPI void             edje_object_part_text_insert                  (Evas_Object *obj, const char *part, const char *text);

/**
 * @brief Insert text for an object part.
 *
 * @param obj A valid Evas Object handle
 * @param part The part name
 * @param text The text string
 *
 * This function inserts the text for an object part at the end; It does not
 * move the cursor.
 *
 * @since 1.1
 */
EAPI void             edje_object_part_text_append(Evas_Object *obj, const char *part, const char *text);

/**
 * @brief Return a list of char anchor names.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 *
 * @return The list of anchors (const char *), do not modify!
 *
 * This function returns a list of char anchor names.
 *
 */
EAPI const Eina_List *edje_object_part_text_anchor_list_get         (const Evas_Object *obj, const char *part);

/**
 * @brief Return a list of Evas_Textblock_Rectangle anchor rectangles.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param anchor The anchor name
 *
 * @return The list of anchor rects (const Evas_Textblock_Rectangle
 * *), do not modify! Geometry is relative to entry part.
 *
 * This function return a list of Evas_Textblock_Rectangle anchor
 * rectangles.
 *
 */
EAPI const Eina_List *edje_object_part_text_anchor_geometry_get     (const Evas_Object *obj, const char *part, const char *anchor);

/**
 * @brief Return a list of char item names.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 *
 * @return The list of items (const char *), do not modify!
 *
 * This function returns a list of char item names.
 *
 */
EAPI const Eina_List *edje_object_part_text_item_list_get           (const Evas_Object *obj, const char *part);

/**
 * @brief Return item geometry.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param item The item name
 * @param cx Item x return (relative to entry part)
 * @param cy Item y return (relative to entry part)
 * @param cw Item width return
 * @param ch Item height return
 *
 * @return 1 if item exists, 0 if not
 *
 * This function return a list of Evas_Textblock_Rectangle item
 * rectangles.
 *
 */
EAPI Eina_Bool        edje_object_part_text_item_geometry_get       (const Evas_Object *obj, const char *part, const char *item, Evas_Coord *cx, Evas_Coord *cy, Evas_Coord *cw, Evas_Coord *ch);

/**
 * @brief Returns the cursor geometry of the part relative to the edje
 * object.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param x Cursor X position
 * @param y Cursor Y position
 * @param w Cursor width
 * @param h Cursor height
 *
 */
EAPI void             edje_object_part_text_cursor_geometry_get     (const Evas_Object *obj, const char *part, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h);

/**
 * @brief Enables selection if the entry is an EXPLICIT selection mode
 * type.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param allow EINA_TRUE to enable, EINA_FALSE otherwise
 *
 * The default is to @b not allow selection. This function only affects user
 * selection, functions such as edje_object_part_text_select_all() and
 * edje_object_part_text_select_none() are not affected.
 */
EAPI void             edje_object_part_text_select_allow_set        (const Evas_Object *obj, const char *part, Eina_Bool allow);

/**
 * @brief Aborts any selection action on a part.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 */
EAPI void             edje_object_part_text_select_abort            (const Evas_Object *obj, const char *part);

/**
 * @brief Starts selecting at current cursor position
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 */
EAPI void             edje_object_part_text_select_begin            (const Evas_Object *obj, const char *part);

/**
 * @brief Extends the current selection to the current cursor position
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 */
EAPI void             edje_object_part_text_select_extend           (const Evas_Object *obj, const char *part);


/**
 * @brief Advances the cursor to the next cursor position.
 * @see evas_textblock_cursor_char_next
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param cur The edje cursor to advance
 */
EAPI Eina_Bool        edje_object_part_text_cursor_next                 (Evas_Object *obj, const char *part, Edje_Cursor cur);

/**
 * @brief Moves the cursor to the previous char
 * @see evas_textblock_cursor_char_prev
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param cur the edje cursor to work on
 */
EAPI Eina_Bool        edje_object_part_text_cursor_prev                 (Evas_Object *obj, const char *part, Edje_Cursor cur);

/**
 * @brief Move the cursor to the char above the current cursor position.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param cur the edje cursor to work on
 */
EAPI Eina_Bool        edje_object_part_text_cursor_up                   (Evas_Object *obj, const char *part, Edje_Cursor cur);

/**
 * @brief Moves the cursor to the char below the current cursor position.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param cur the edje cursor to work on
 */
EAPI Eina_Bool        edje_object_part_text_cursor_down                 (Evas_Object *obj, const char *part, Edje_Cursor cur);

/**
 * @brief Moves the cursor to the beginning of the text part
 * @see evas_textblock_cursor_paragraph_first
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param cur the edje cursor to work on
 */
EAPI void             edje_object_part_text_cursor_begin_set            (Evas_Object *obj, const char *part, Edje_Cursor cur);

/**
 * @brief Moves the cursor to the end of the text part.
 * @see evas_textblock_cursor_paragraph_last
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param cur the edje cursor to work on
 */
EAPI void             edje_object_part_text_cursor_end_set              (Evas_Object *obj, const char *part, Edje_Cursor cur);

/**
 * @brief Copy the cursor to another cursor.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param sry the cursor to copy from
 * @param dst the cursor to copy to
 */
EAPI void             edje_object_part_text_cursor_copy                 (Evas_Object *obj, const char *part, Edje_Cursor src, Edje_Cursor dst);

/**
 * @brief Move the cursor to the beginning of the line.
 * @see evas_textblock_cursor_line_char_first
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param cur the edje cursor to work on
 */
EAPI void             edje_object_part_text_cursor_line_begin_set       (Evas_Object *obj, const char *part, Edje_Cursor cur);

/**
 * @brief Move the cursor to the end of the line.
 * @see evas_textblock_cursor_line_char_last
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param cur the edje cursor to work on
 */
EAPI void             edje_object_part_text_cursor_line_end_set         (Evas_Object *obj, const char *part, Edje_Cursor cur);

/**
 * Position the given cursor to a X,Y position.
 *
 * This is frequently used with the user cursor.
 *
 * @param obj An Edje object.
 * @param part The part containing the object.
 * @param cur The cursor to adjust.
 * @param x X Coordinate.
 * @param y Y Coordinate.
 * @return True on success, false on error.
 */
EAPI Eina_Bool        edje_object_part_text_cursor_coord_set            (Evas_Object *obj, const char *part, Edje_Cursor cur, Evas_Coord x, Evas_Coord y);

/**
 * @brief Returns whether the cursor points to a format.
 * @see evas_textblock_cursor_is_format
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param cur The cursor to adjust.
 * @return EINA_TRUE if it's true, EINA_FALSE otherwise.
 */
EAPI Eina_Bool        edje_object_part_text_cursor_is_format_get        (const Evas_Object *obj, const char *part, Edje_Cursor cur);

/**
 * @brief Return true if the cursor points to a visible format
 * For example \t, \n, item and etc.
 * @see  evas_textblock_cursor_format_is_visible_get
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param cur The cursor to adjust.
 */
EAPI Eina_Bool        edje_object_part_text_cursor_is_visible_format_get(const Evas_Object *obj, const char *part, Edje_Cursor cur);

/**
 * @brief Returns the content (char) at the cursor position.
 * @see evas_textblock_cursor_content_get
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param cur The cursor to use
 */
EAPI const char      *edje_object_part_text_cursor_content_get          (const Evas_Object *obj, const char *part, Edje_Cursor cur);

/**
 * @brief Sets the cursor position to the given value
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param cur The cursor to move
 * @param pos the position of the cursor
 * @since 1.1.0
 */
EAPI void             edje_object_part_text_cursor_pos_set              (Evas_Object *obj, const char *part, Edje_Cursor cur, int pos);

/**
 * @brief Retrieves the current position of the cursor
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param cur The cursor to get the position
 * @return The cursor position
 * @since 1.1.0
 */
EAPI int              edje_object_part_text_cursor_pos_get              (const Evas_Object *obj, const char *part, Edje_Cursor cur);

/**
 * Add a filter function for newly inserted text.
 *
 * Whenever text is inserted (not the same as set) into the given @p part,
 * the list of filter functions will be called to decide if and how the new
 * text will be accepted.
 * There are three types of filters, EDJE_TEXT_FILTER_TEXT,
 * EDJE_TEXT_FILTER_FORMAT and EDJE_TEXT_FILTER_MARKUP.
 * The text parameter in the @p func filter can be modified by the user and
 * it's up to him to free the one passed if he's to change the pointer. If
 * doing so, the newly set text should be malloc'ed, as once all the filters
 * are called Edje will free it.
 * If the text is to be rejected, freeing it and setting the pointer to NULL
 * will make Edje break out of the filter cycle and reject the inserted
 * text.
 *
 * @see edje_object_text_insert_filter_callback_del
 * @see edje_object_text_insert_filter_callback_del_full
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param func The callback function that will act as filter
 * @param data User provided data to pass to the filter function
 */
EAPI void             edje_object_text_insert_filter_callback_add       (Evas_Object *obj, const char *part, Edje_Text_Filter_Cb func, void *data);

/**
 * Delete a function from the filter list.
 *
 * Delete the given @p func filter from the list in @p part. Returns
 * the user data pointer given when added.
 *
 * @see edje_object_text_insert_filter_callback_add
 * @see edje_object_text_insert_filter_callback_del_full
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param func The function callback to remove
 *
 * @return The user data pointer if succesful, or NULL otherwise
 */
EAPI void            *edje_object_text_insert_filter_callback_del       (Evas_Object *obj, const char *part, Edje_Text_Filter_Cb func);

/**
 * Delete a function and matching user data from the filter list.
 *
 * Delete the given @p func filter and @p data user data from the list
 * in @p part.
 * Returns the user data pointer given when added.
 *
 * @see edje_object_text_insert_filter_callback_add
 * @see edje_object_text_insert_filter_callback_del
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param func The function callback to remove
 * @param data The data passed to the callback function
 *
 * @return The same data pointer if succesful, or NULL otherwise
 */
EAPI void            *edje_object_text_insert_filter_callback_del_full  (Evas_Object *obj, const char *part, Edje_Text_Filter_Cb func, void *data);

/**
 * @brief Swallows an object into the edje.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param obj_swallow The object to swallow
 *
 * Swallows the object into the edje part so that all geometry changes
 * for the part affect the swallowed object. (e.g. resize, move, show,
 * raise/lower, etc.).
 *
 * If an object has already been swallowed into this part, then it
 * will first be unswallowed before the new object is swallowed.
 */
EAPI Eina_Bool        edje_object_part_swallow        (Evas_Object *obj, const char *part, Evas_Object *obj_swallow);

/**
 * @brief Unswallow an object.
 *
 * @param obj A valid Evas_Object handle
 * @param obj_swallow The swallowed object
 *
 * Causes the edje to regurgitate a previously swallowed object. :)
 *
 * @note @p obj_swallow will @b not be deleted.
 */
EAPI void             edje_object_part_unswallow      (Evas_Object *obj, Evas_Object *obj_swallow);

/**
 * @brief Get the object currently swallowed by a part.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @return The swallowed object, or NULL if there is none.
 */
EAPI Evas_Object     *edje_object_part_swallow_get    (const Evas_Object *obj, const char *part);

/**
 * @brief Returns the state of the Edje part.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param val_ret
 *
 * @return The part state:\n
 * "default" for the default state\n
 * "" for other states
 */
EAPI const char      *edje_object_part_state_get      (const Evas_Object *obj, const char *part, double *val_ret);

/**
 * @brief Determine dragable directions.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 *
 * @return 0: Not dragable\n
 * 1: Dragable in X direction\n
 * 2: Dragable in Y direction\n
 * 3: Dragable in X & Y directions
 */
EAPI Edje_Drag_Dir    edje_object_part_drag_dir_get   (const Evas_Object *obj, const char *part);

/**
 * @brief Set the dragable object location.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param dx The x value
 * @param dy The y value
 *
 * Places the dragable object at the given location.
 */
EAPI Eina_Bool        edje_object_part_drag_value_set (Evas_Object *obj, const char *part, double dx, double dy);

/**
 * @brief Get the dragable object location.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param dx The X value pointer
 * @param dy The Y value pointer
 *
 * Gets the drag location values.
 */
EAPI Eina_Bool        edje_object_part_drag_value_get (const Evas_Object *obj, const char *part, double *dx, double *dy);

/**
 * @brief Set the dragable object size.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param dw The drag width
 * @param dh The drag height
 *
 * Sets the size of the dragable object.
 */
EAPI Eina_Bool        edje_object_part_drag_size_set  (Evas_Object *obj, const char *part, double dw, double dh);

/**
 * @brief Get the dragable object size.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param dw The drag width pointer
 * @param dh The drag height pointer
 *
 * Gets the dragable object size.
 */
EAPI Eina_Bool        edje_object_part_drag_size_get  (const Evas_Object *obj, const char *part, double *dw, double *dh);

/**
 * @brief Sets the drag step increment.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param dx The x step amount
 * @param dy The y step amount
 *
 * Sets the x,y step increments for a dragable object.
 */
EAPI Eina_Bool        edje_object_part_drag_step_set  (Evas_Object *obj, const char *part, double dx, double dy);

/**
 * @brief Gets the drag step increment values.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part
 * @param dx The x step increment pointer
 * @param dy The y step increment pointer
 *
 * Gets the x and y step increments for the dragable object.
 */
EAPI Eina_Bool        edje_object_part_drag_step_get  (const Evas_Object *obj, const char *part, double *dx, double *dy);

/**
 * @brief Sets the page step increments.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param dx The x page step increment
 * @param dy The y page step increment
 *
 * Sets the x,y page step increment values.
 */
EAPI Eina_Bool        edje_object_part_drag_page_set  (Evas_Object *obj, const char *part, double dx, double dy);

/**
 * @brief Gets the page step increments.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param dx The dx page increment pointer
 * @param dy The dy page increment pointer
 *
 * Gets the x,y page step increments for the dragable object.
 */
EAPI Eina_Bool        edje_object_part_drag_page_get  (const Evas_Object *obj, const char *part, double *dx, double *dy);

/**
 * @brief Steps the dragable x,y steps.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param dx The x step
 * @param dy The y step
 *
 * Steps x,y where the step increment is the amount set by
 * edje_object_part_drag_step_set.
 */
EAPI Eina_Bool        edje_object_part_drag_step      (Evas_Object *obj, const char *part, double dx, double dy);

/**
 * @brief Pages x,y steps.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param dx The x step
 * @param dy The y step
 *
 * Pages x,y where the increment is defined by
 * edje_object_part_drag_page_set.\n WARNING: Paging is bugged!
 */
EAPI Eina_Bool        edje_object_part_drag_page      (Evas_Object *obj, const char *part, double dx, double dy);


/**
 * @brief Get the object created by this external part.
 *
 * Parts of type external creates the part object using information
 * provided by external plugins. It's somehow like "swallow"
 * (edje_object_part_swallow()), but it's all set automatically.
 *
 * This function returns the part created by such external plugins and
 * being currently managed by this Edje.
 *
 * @note Almost all swallow rules apply: you should not move, resize,
 *       hide, show, set the color or clipper of such part. It's a bit
 *       more restrictive as one must @b never delete this object!
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @return The externally created object, or NULL if there is none or
 *         part is not an external.
 */
EAPI Evas_Object              *edje_object_part_external_object_get     (const Evas_Object *obj, const char *part);

/**
 * @brief Set the parameter for the external part.
 *
 * Parts of type external may carry extra properties that have
 * meanings defined by the external plugin. For instance, it may be a
 * string that defines a button label and setting this property will
 * change that label on the fly.
 *
 * @note external parts have parameters set when they change
 *       states. Those parameters will never be changed by this
 *       function. The interpretation of how state_set parameters and
 *       param_set will interact is up to the external plugin.
 *
 * @note this function will not check if parameter value is valid
 *       using #Edje_External_Param_Info minimum, maximum, valid
 *       choices and others. However these should be checked by the
 *       underlying implementation provided by the external
 *       plugin. This is done for performance reasons.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param param the parameter details, including its name, type and
 *        actual value. This pointer should be valid, and the
 *        parameter must exist in
 *        #Edje_External_Type::parameters_info, with the exact type,
 *        otherwise the operation will fail and @c EINA_FALSE will be
 *        returned.
 *
 * @return @c EINA_TRUE if everything went fine, @c EINA_FALSE on errors.
 */
EAPI Eina_Bool                 edje_object_part_external_param_set      (Evas_Object *obj, const char *part, const Edje_External_Param *param);

/**
 * @brief Get the parameter for the external part.
 *
 * Parts of type external may carry extra properties that have
 * meanings defined by the external plugin. For instance, it may be a
 * string that defines a button label. This property can be modifed by
 * state parameters, by explicit calls to
 * edje_object_part_external_param_set() or getting the actual object
 * with edje_object_part_external_object_get() and calling native
 * functions.
 *
 * This function asks the external plugin what is the current value,
 * independent on how it was set.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name

 * @param param the parameter details. It is used as both input and
 *        output variable. This pointer should be valid, and the
 *        parameter must exist in
 *        #Edje_External_Type::parameters_info, with the exact type,
 *        otherwise the operation will fail and @c EINA_FALSE will be
 *        returned.
 *
 * @return @c EINA_TRUE if everything went fine and @p param members
 *         are filled with information, @c EINA_FALSE on errors and @p
 *         param member values are not set or valid.
 */
EAPI Eina_Bool                 edje_object_part_external_param_get      (const Evas_Object *obj, const char *part, Edje_External_Param *param);

EAPI Evas_Object              *edje_object_part_external_content_get    (const Evas_Object *obj, const char *part, const char *content);

/**
 * Facility to query the type of the given parameter of the given part.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param param the parameter name to use.
 *
 * @return @c EDJE_EXTERNAL_PARAM_TYPE_MAX on errors, or another value
 *         from #Edje_External_Param_Type on success.
 */
EAPI Edje_External_Param_Type  edje_object_part_external_param_type_get (const Evas_Object *obj, const char *part, const char *param);


/**
 * @brief Appends an object to the box.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param child The object to append
 *
 * @return @c EINA_TRUE: Successfully added.\n
 * @c EINA_FALSE: An error occurred.
 *
 * Appends child to the box indicated by part.
 *
 * @see edje_object_part_box_prepend()
 * @see edje_object_part_box_insert_before()
 * @see edje_object_part_box_insert_at()
 */
EAPI Eina_Bool    edje_object_part_box_append             (Evas_Object *obj, const char *part, Evas_Object *child);

/**
 * @brief Prepends an object to the box.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param child The object to prepend
 *
 * @return @c EINA_TRUE: Successfully added.\n
 * @c EINA_FALSE: An error occurred.
 *
 * Prepends child to the box indicated by part.
 *
 * @see edje_object_part_box_append()
 * @see edje_object_part_box_insert_before()
 * @see edje_object_part_box_insert_at()
 */
EAPI Eina_Bool    edje_object_part_box_prepend            (Evas_Object *obj, const char *part, Evas_Object *child);

/**
 * @brief Adds an object to the box.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param child The object to insert
 * @param reference The object to be used as reference
 *
 * @return @c EINA_TRUE: Successfully added.\n
 * @c EINA_FALSE: An error occurred.
 *
 * Inserts child in the box given by part, in the position marked by
 * reference.
 *
 * @see edje_object_part_box_append()
 * @see edje_object_part_box_prepend()
 * @see edje_object_part_box_insert_at()
 */
EAPI Eina_Bool    edje_object_part_box_insert_before      (Evas_Object *obj, const char *part, Evas_Object *child, const Evas_Object *reference);

/**
 * @brief Inserts an object to the box.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param child The object to insert
 * @param pos The position where to insert child
 *
 * @return @c EINA_TRUE: Successfully added.\n
 * @c EINA_FALSE: An error occurred.
 *
 * Adds child to the box indicated by part, in the position given by
 * pos.
 *
 * @see edje_object_part_box_append()
 * @see edje_object_part_box_prepend()
 * @see edje_object_part_box_insert_before()
 */
EAPI Eina_Bool    edje_object_part_box_insert_at          (Evas_Object *obj, const char *part, Evas_Object *child, unsigned int pos);

/**
 * @brief Removes an object from the box.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param child The object to remove
 *
 * @return Pointer to the object removed, or @c NULL.
 *
 * Removes child from the box indicated by part.
 *
 * @see edje_object_part_box_remove_at()
 * @see edje_object_part_box_remove_all()
 */
EAPI Evas_Object *edje_object_part_box_remove             (Evas_Object *obj, const char *part, Evas_Object *child);

/**
 * @brief Removes an object from the box.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param pos The position index of the object (starts counting from 0)
 *
 * @return Pointer to the object removed, or @c NULL.
 *
 * Removes from the box indicated by part, the object in the position
 * pos.
 *
 * @see edje_object_part_box_remove()
 * @see edje_object_part_box_remove_all()
 */
EAPI Evas_Object *edje_object_part_box_remove_at          (Evas_Object *obj, const char *part, unsigned int pos);

/**
 * @brief Removes all elements from the box.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param clear Delete objects on removal
 *
 * @return 1: Successfully cleared.\n
 * 0: An error occurred.
 *
 * Removes all the external objects from the box indicated by part.
 * Elements created from the theme will not be removed.
 *
 * @see edje_object_part_box_remove()
 * @see edje_object_part_box_remove_at()
 */
EAPI Eina_Bool    edje_object_part_box_remove_all         (Evas_Object *obj, const char *part, Eina_Bool clear);

/**
 * @brief Retrieve a child from a table
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param col The column of the child to get
 * @param row The row of the child to get
 * @return The child Evas_Object
 */
EAPI Evas_Object *edje_object_part_table_child_get        (Evas_Object *obj, const char *part, unsigned int col, unsigned int row);

/**
 * @brief Packs an object into the table.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param child_obj The object to pack in
 * @param col The column to place it in
 * @param row The row to place it in
 * @param colspan Columns the child will take
 * @param rowspan Rows the child will take
 *
 * @return @c EINA_TRUE object was added, @c EINA_FALSE on failure
 *
 * Packs an object into the table indicated by part.
 */
EAPI Eina_Bool    edje_object_part_table_pack             (Evas_Object *obj, const char *part, Evas_Object *child_obj, unsigned short col, unsigned short row, unsigned short colspan, unsigned short rowspan);

/**
 * @brief Removes an object from the table.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param child_obj The object to pack in
 *
 * @return @c EINA_TRUE object removed, @c EINA_FALSE on failure
 *
 * Removes an object from the table indicated by part.
 */
EAPI Eina_Bool    edje_object_part_table_unpack           (Evas_Object *obj, const char *part, Evas_Object *child_obj);

/**
 * @brief Gets the number of columns and rows the table has.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param cols Pointer where to store number of columns (can be NULL)
 * @param rows Pointer where to store number of rows (can be NULL)
 *
 * @return @c EINA_TRUE get some data, @c EINA_FALSE on failure
 *
 * Retrieves the size of the table in number of columns and rows.
 */
EAPI Eina_Bool    edje_object_part_table_col_row_size_get (const Evas_Object *obj, const char *part, int *cols, int *rows);

/**
 * @brief Removes all object from the table.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param clear If set, will delete subobjs on remove
 *
 * @return @c EINA_TRUE clear the table, @c EINA_FALSE on failure
 *
 * Removes all object from the table indicated by part, except the
 * internal ones set from the theme.
 */
EAPI Eina_Bool    edje_object_part_table_clear            (Evas_Object *obj, const char *part, Eina_Bool clear);

/**
 * @brief Send an (Edje) message to a given Edje object
 *
 * @param obj A handle to an Edje object
 * @param type The type of message to send to @p obj
 * @param id A identification number for the message to be sent
 * @param msg The message's body, a struct depending on @p type
 *
 * This function sends an Edje message to @p obj and to all of its
 * child objects, if it has any (swallowed objects are one kind of
 * child object). @p type and @p msg @b must be matched accordingly,
 * as documented in #Edje_Message_Type.
 *
 * The @p id argument as a form of code and theme defining a common
 * interface on message communication. One should define the same IDs
 * on both code and EDC declaration (see @ref edcref "the syntax" for
 * EDC files), to individualize messages (binding them to a given
 * context).
 *
 * The function to handle messages arriving @b from @b obj is set with
 * edje_object_message_handler_set().
 */
EAPI void         edje_object_message_send                (Evas_Object *obj, Edje_Message_Type type, int id, void *msg);

/**
 * @brief Set an Edje message handler function for a given Edje object.
 *
 * @param obj A handle to an Edje object
 * @param func The function to handle messages @b coming from @p obj
 * @param data Auxiliary data to be passed to @p func
 *
 * Edje messages are one of the communication interfaces between
 * @b code and a given Edje object's @b theme. With messages, one can
 * communicate values beyond strings (which are the subject of Edje
 * signals -- see edje_object_signal_emit()), like float and integer
 * numbers. Moreover, messages can be identified by integer
 * numbers. See #Edje_Message_Type for the full list of message types.
 *
 * For scriptable programs on an Edje object's defining EDC file which
 * send messages with the @c send_message() primitive, one can attach
 * <b>handler functions</b>, to be called in the code which creates
 * that object (see @ref edcref "the syntax" for EDC files).
 *
 * This function associates a message handler function and the
 * attached data pointer to the object @p obj.
 *
 * @see edje_object_message_send()
 */
EAPI void         edje_object_message_handler_set         (Evas_Object *obj, Edje_Message_Handler_Cb func, void *data);

/**
 * @brief Process an object's message queue.
 *
 * @param obj The edje object reference.
 *
 * This function goes through the object message queue processing the
 * pending messages for *this* specific edje object. Normally they'd
 * be processed only at idle time.
 *
 */
EAPI void         edje_object_message_signal_process      (Evas_Object *obj);


/**
 * @brief Process all queued up edje messages.
 *
 * This function triggers the processing of messages addressed to any
 * (alive) edje objects.
 *
 */
EAPI void         edje_message_signal_process             (void);

/**
 * Register given type name to return the given information.
 *
 * @param type_name name to register and be known by edje's "source:"
 *        parameter of "type: EXTERNAL" parts.
 * @param type_info meta-information describing how to interact with it.
 *
 * @return @c EINA_TRUE on success, @c EINA_FALSE on failure (like
 *         type already registered).
 *
 * @see edje_external_type_array_register()
 */
EAPI Eina_Bool    edje_external_type_register             (const char *type_name, const Edje_External_Type *type_info);

/**
 * Unregister given type name previously registered.
 *
 * @param type_name name to unregister. It should be registered with
 *        edje_external_type_register() before.
 *
 * @return @c EINA_TRUE on success, @c EINA_FALSE on failure (like
 *         type_name did not exist).
 *
 * @see edje_external_type_array_unregister()
 */
EAPI Eina_Bool    edje_external_type_unregister           (const char *type_name);

/**
 * Register a batch of types and their information.
 *
 * This is the recommended function to add information as it's faster
 * than the single version edje_external_type_register().
 *
 * @note the given array is not modified, but the type name strings
 *       are @b not duplicated! That is, all type names must be @b
 *       live until they are unregistered! This was chosen to save
 *       some memory and most people will just define the array as a
 *       global static const type anyway.
 *
 * @param array @c NULL terminated array with type name and
 *        information. Note that type name or information are not
 *        modified by are @b referenced, so they must keep alive after
 *        this function returns!
 *
 * @return @c EINA_TRUE on success, @c EINA_FALSE on failure (like
 *         type already registered).
 *
 * @see edje_external_type_register()
 */
EAPI void         edje_external_type_array_register       (const Edje_External_Type_Info *array);

/**
 * Unregister a batch of given external type previously registered.
 *
 * @param array @c NULL terminated array, should be the same as the
 *        one used to register with edje_external_type_array_register()
 *
 * @see edje_external_type_unregister()
 */
EAPI void         edje_external_type_array_unregister     (const Edje_External_Type_Info *array);

/**
 * Return the current ABI version for Edje_External_Type structure.
 *
 * Always check this number before accessing Edje_External_Type in
 * your own software. If the number is not the same, your software may
 * access invalid memory and crash, or just get garbage values.
 *
 * @warning @b NEVER, EVER define your own Edje_External_Type using the
 *          return of this function as it will change as Edje library
 *          (libedje.so) changes, but your type definition will
 *          not. Instead, use #EDJE_EXTERNAL_TYPE_ABI_VERSION.
 *
 * Summary:
 *   - use edje_external_type_abi_version_get() to check.
 *   - use #EDJE_EXTERNAL_TYPE_ABI_VERSION to define/declare.
 *
 * @return version this edje library was compiled.
 */
EAPI unsigned int edje_external_type_abi_version_get      (void) EINA_CONST;

/**
 * Returns an iterator that emits Eina_Hash_Tuple pointers with key
 * being the name and data being the Edje_External_Type pointer.
 *
 * @code
 * const Eina_Hash_Tuple *tuple;
 * Eina_Iterator *itr;
 * const Eina_List *l, *modules;
 * const char *s;
 *
 * modules = edje_available_modules_get();
 * EINA_LIST_FOREACH(modules, l, s)
 *   {
 *      if (!edje_module_load(s))
 *        printf("Error loading edje module: %s\n", s);
 *   }
 *
 * itr = edje_external_iterator_get();
 * EINA_ITERATOR_FOREACH(itr, tuple)
 *   {
 *      const char *name = tuple->key;
 *      const Edje_External_Type *type = tuple->data;
 *
 *      if ((!type) ||
 *          (type->abi_version != edje_external_type_abi_version_get()))
 *        {
 *           printf("Error: invalid type %p (abi: %d, expected: %d)\n",
 *                   type, type ? type->abi_version : 0,
 *                   edje_external_type_abi_version_get());
 *           continue;
 *        }
 *
 *      printf("%s: %s (%s) label='%s' desc='%s'\n",
 *             name, type->module, type->module_name,
 *             type->label_get ? type->label_get(type->data) : "",
 *             type->description_get ? type->description_get(type->data) : "");
 *   }
 *
 * @endcode
 */
EAPI Eina_Iterator                  *edje_external_iterator_get     (void);

   EAPI Edje_External_Param            *edje_external_param_find       (const Eina_List *params, const char *key);
   EAPI Eina_Bool                       edje_external_param_int_get    (const Eina_List *params, const char *key, int *ret);
   EAPI Eina_Bool                       edje_external_param_double_get (const Eina_List *params, const char *key, double *ret);
   EAPI Eina_Bool                       edje_external_param_string_get (const Eina_List *params, const char *key, const char **ret);
   EAPI Eina_Bool                       edje_external_param_bool_get   (const Eina_List *params, const char *key, Eina_Bool *ret);
   EAPI Eina_Bool                       edje_external_param_choice_get (const Eina_List *params, const char *key, const char **ret);

/**
 * Get the array of parameters information about a type given its name.
 *
 * @note the type names and other strings are static, that means they
 *       @b NOT translated. One must use
 *       Edje_External_Type::translate() to translate those.
 *
 * @return the NULL terminated array, or @c NULL if type is unknown or
 *         it does not have any parameter information.
 *
 * @see edje_external_type_get()
 */
EAPI const Edje_External_Param_Info *edje_external_param_info_get   (const char *type_name);

   EAPI const Edje_External_Type       *edje_external_type_get         (const char *type_name);

   EAPI Eina_Bool               edje_module_load                (const char *module);
   EAPI const Eina_List        *edje_available_modules_get      (void);

   /* perspective info for maps inside edje objects */
   typedef struct _Edje_Perspective Edje_Perspective;

   EAPI Edje_Perspective       *edje_perspective_new            (Evas *e);
   EAPI void                    edje_perspective_free           (Edje_Perspective *ps);
   EAPI void                    edje_perspective_set            (Edje_Perspective *ps, Evas_Coord px, Evas_Coord py, Evas_Coord z0, Evas_Coord foc);
   EAPI void                    edje_perspective_global_set     (Edje_Perspective *ps, Eina_Bool global);
   EAPI Eina_Bool               edje_perspective_global_get     (const Edje_Perspective *ps);
   EAPI const Edje_Perspective *edje_evas_global_perspective_get(const Evas *e);
   EAPI void                    edje_object_perspective_set     (Evas_Object *obj, Edje_Perspective *ps);
   EAPI const Edje_Perspective *edje_object_perspective_get     (const Evas_Object *obj);

#ifdef __cplusplus
}
#endif

#endif
