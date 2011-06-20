/**
@mainpage Evas

@image html  e_big.png

@version 1.0.0
@author Carsten Haitzler <raster@@rasterman.com>
@author Till Adam <till@@adam-lilienthal.de>
@author Steve Ireland <sireland@@pobox.com>
@author Brett Nash <nash@@nash.id.au>
@author Tilman Sauerbeck <tilman@@code-monkey.de>
@author Corey Donohoe <atmos@@atmos.org>
@author Yuri Hudobin <glassy_ape@@users.sourceforge.net>
@author Nathan Ingersoll <ningerso@@d.umn.edu>
@author Willem Monsuwe <willem@@stack.nl>
@author Jose O Gonzalez <jose_ogp@@juno.com>
@author Bernhard Nemec <Bernhard.Nemec@@viasyshc.com>
@author Jorge Luis Zapata Muga <jorgeluis.zapata@@gmail.com>
@author Cedric Bail <cedric.bail@@free.fr>
@author Gustavo Sverzut Barbieri <barbieri@@profusion.mobi>
@author Vincent Torri <vtorri@@univ-evry.fr>
@author Tim Horton <hortont424@@gmail.com>
@author Tom Hacohen <tom@@stosb.com>
@author Mathieu Taillefumier <mathieu.taillefumier@@free.fr>
@author Iván Briano <ivan@@profusion.mobi>
@author Gustavo Lima Chaves <glima@@profusion.mobi>
@author Samsung Electronics <tbd>
@author Samsung SAIT <tbd>
@author Sung W. Park <sungwoo@@gmail.com>
@author Jiyoun Park <jy0703.park@@samsung.com>
@author Myoungwoon Roy Kim(roy_kim) <myoungwoon.kim@@samsung.com> <myoungwoon@@gmail.com>
@author Thierry el Borgi <thierry@@substantiel.fr>
@author ChunEon Park <hermet@@hermet.pe.kr>
@date 2000-2011

@section toc Table of Contents

@li @ref intro
@li @ref work
@li @ref compiling
@li @ref install
@li @ref next_steps
@li @ref intro_example


@section intro What is Evas?

Evas is a clean display canvas API for several target display systems that
can draw anti-aliased text, smooth super and sub-sampled scaled images,
alpha-blend objects much and more.

It abstracts any need to know much about what the characteristics of your
display system are or what graphics calls are used to draw them and how. It
deals on an object level where all you do is create and manipulate objects
in a canvas, set their properties, and the rest is done for you.

Evas optimises the rendering pipeline to minimise effort in redrawing changes
made to the canvas and so takes this work out of the programmers hand,
saving a lot of time and energy.

It's small and lean, designed to work on embedded systems all the way to
large and powerful multi-cpu workstations. It can be compiled to only have
the features you need for your target platform if you so wish, thus keeping
it small and lean. It has several display back-ends, letting it display on
several display systems, making it portable for cross-device and
cross-platform development.

@subsection intro_not_evas What Evas is not?

Evas is not a widget set or widget toolkit, however it is their
base. See Elementary (http://docs.enlightenment.org/auto/elementary/)
for a toolkit based on Evas, Edje, Ecore and other Enlightenment
technologies.

It is not dependent or aware of main loops, input or output
systems. Input should be polled from various sources and feed them to
Evas. Similarly, it will not create windows or report windows updates
to your system, rather just drawing the pixels and reporting to the
user the areas that were changed. Of course these operations are quite
common and thus they are ready to use in Ecore, particularly in
Ecore_Evas (http://docs.enlightenment.org/auto/ecore/).


@section work How does Evas work?

Evas is a canvas display library. This is markedly different from most
display and windowing systems as a Canvas is structural and is also a state
engine, whereas most display and windowing systems are immediate mode display
targets. Evas handles the logic between a structural display via its' state
engine, and controls the target windowing system in order to produce
rendered results of the current canvases state on the display.

Immediate mode display systems retain very little, or no state. A program
will execute a series of commands, as in the pseudo code:

@verbatim
draw line from position (0, 0) to position (100, 200);

draw rectangle from position (10, 30) to position (50, 500);

bitmap_handle = create_bitmap();
scale bitmap_handle to size 100 x 100;
draw image bitmap_handle at position (10, 30);
@endverbatim

The series of commands is executed by the windowing system and the results
are displayed on the screen (normally). Once the commands are executed the
display system has little or no idea of how to reproduce this image again,
and so has to be instructed by the application how to redraw sections of the
screen whenever needed. Each successive command will be executed as
instructed by the application and either emulated by software or sent to the
graphics hardware on the device to be performed.

The advantage of such a system is that it is simple, and gives a program
tight control over how something looks and is drawn. Given the increasing
complexity of displays and demands by users to have better looking
interfaces, more and more work is needing to be done at this level by the
internals of widget sets, custom display widgets and other programs. This
means more and more logic and display rendering code needs to be written
time and time again, each time the application needs to figure out how to
minimise redraws so that display is fast and interactive, and keep track of
redraw logic. The power comes at a high-price, lots of extra code and work.
Programmers not very familiar with graphics programming will often make
mistakes at this level and produce code that is sub optimal. Those familiar
with this kind of programming will simply get bored by writing the same code
again and again.

For example, if in the above scene, the windowing system requires the
application to redraw the area from 0, 0 to 50, 50 (also referred as
"expose event"), then the programmer must calculate manually the
updates and repaint it again:

@verbatim
Redraw from position (0, 0) to position (50, 50):

// what was in area (0, 0, 50, 50)?

// 1. intersection part of line (0, 0) to (100, 200)?
      draw line from position (0, 0) to position (25, 50);

// 2. intersection part of rectangle (10, 30) to (50, 500)?
      draw rectangle from position (10, 30) to position (50, 50)

// 3. intersection part of image at (10, 30), size 100 x 100?
      bitmap_subimage = subregion from position (0, 0) to position (40, 20)
      draw image bitmap_subimage at position (10, 30);
@endverbatim

The clever reader might have noticed that, if all elements in the
above scene are opaque, then the system is doing useless paints: part
of the line is behind the rectangle, and part of the rectangle is
behind the image. These useless paints tends to be very costly, as
pixels tend to be 4 bytes in size, thus an overlapping region of 100 x
100 pixels is around 40000 useless writes! The developer could write
code to calculate the overlapping areas and avoid painting then, but
then it should be mixed with the "expose event" handling mentioned
above and quickly one realizes the initially simpler method became
really complex.

Evas is a structural system in which the programmer creates and manages
display objects and their properties, and as a result of this higher level
state management, the canvas is able to redraw the set of objects when
needed to represent the current state of the canvas.

For example, the pseudo code:

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

This may look longer, but when the display needs to be refreshed or updated,
the programmer only moves, resizes, shows, hides etc. the objects that they
need to change. The programmer simply thinks at the object logic level, and
the canvas software does the rest of the work for them, figuring out what
actually changed in the canvas since it was last drawn, how to most
efficiently redraw he canvas and its contents to reflect the current state,
and then it can go off and do the actual drawing of the canvas.

This lets the programmer think in a more natural way when dealing with a
display, and saves time and effort of working out how to load and display
images, render given the current display system etc. Since Evas also is
portable across different display systems, this also gives the programmer
the ability to have their code ported and display on different display
systems with very little work.

Evas can be seen as a display system that stands somewhere between a widget
set and an immediate mode display system. It retains basic display logic,
but does very little high-level logic such as scrollbars, sliders, push
buttons etc.


@section compiling How to compile using Evas ?

Evas is a library your application links to. The procedure for this is very
simple. You simply have to compile your application with the appropriate
compiler flags that the @p pkg-config script outputs. For example:

Compiling C or C++ files into object files:

@verbatim
gcc -c -o main.o main.c `pkg-config --cflags evas`
@endverbatim

Linking object files into a binary executable:

@verbatim
gcc -o my_application main.o `pkg-config --libs evas`
@endverbatim

You simply have to make sure that pkg-config is in your shell's PATH (see
the manual page for your appropriate shell) and evas.pc in /usr/lib/pkgconfig
or its path is in the PKG_CONFIG_PATH environment variable. It's that simple
to link and use Evas once you have written your code to use it.

Since the program is linked to Evas, it is now able to use any advertised
API calls to display graphics in a canvas managed by Evas, as well as use
the API calls provided to manage data as well.

You should make sure you add any extra compile and link flags to your
compile commands that your application may need as well. The above example
is only guaranteed to make Evas add it's own requirements.


@section install How is it installed?

Simple:

@verbatim
./configure
make
su -
...
make install
@endverbatim

@section next_steps Next Steps

After you understood what Evas is and installed it in your system you
should proceed understanding the programming interface for all
objects, then see the specific for the most used elements. We'd
recommend you to take a while to learn Ecore
(http://docs.enlightenment.org/auto/ecore/) and Edje
(http://docs.enlightenment.org/auto/edje/) as they will likely save
you tons of work compared to using just Evas directly.

Recommended reading:

@li @ref Evas_Object_Group
@li @ref Evas_Object_Rectangle
@li @ref Evas_Object_Image
@li @ref Evas_Object_Text
@li @ref Evas_Smart_Object_Group and @ref Evas_Smart_Group to define
    an object that provides custom functions to handle clipping,
    hiding, moving, resizing, setting the color and more. These could
    be as simple as a group of objects that move together (see @ref
    Evas_Smart_Object_Clipped). These smart objects can implement what
    ends to be a widget, providing some intelligence (thus the name),
    like a button or check box.

@section intro_example Introductory Example

@include evas-buffer-simple.c
*/

#ifndef _EVAS_H
#define _EVAS_H

#include <time.h>

#include <Eina.h>

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef EFL_EVAS_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif /* ! DLL_EXPORT */
# else
#  define EAPI __declspec(dllimport)
# endif /* ! EFL_EVAS_BUILD */
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

#define EVAS_VERSION_MAJOR 1
#define EVAS_VERSION_MINOR 0

typedef struct _Evas_Version
{
   int major;
   int minor;
   int micro;
   int revision;
} Evas_Version;

EAPI extern Evas_Version *evas_version;

/**
 * @file
 * @brief These routines are used for Evas library interaction.
 *
 * @todo check boolean return values and convert to Eina_Bool
 * @todo change all api to use EINA_SAFETY_*
 * @todo finish api documentation
 */

/* BiDi exposed stuff */
   /*FIXME: document */
typedef enum _Evas_BiDi_Direction
{
   EVAS_BIDI_DIRECTION_NATURAL,
   EVAS_BIDI_DIRECTION_LTR,
   EVAS_BIDI_DIRECTION_RTL
} Evas_BiDi_Direction;

/**
 * Identifier of callbacks to be used with object or canvas.
 *
 * @see evas_object_event_callback_add()
 * @see evas_event_callback_add()
 */
typedef enum _Evas_Callback_Type
{
   /*
    * The following events are only for use with objects
    * evas_object_event_callback_add():
    */
   EVAS_CALLBACK_MOUSE_IN, /**< Mouse In Event */
   EVAS_CALLBACK_MOUSE_OUT, /**< Mouse Out Event */
   EVAS_CALLBACK_MOUSE_DOWN, /**< Mouse Button Down Event */
   EVAS_CALLBACK_MOUSE_UP, /**< Mouse Button Up Event */
   EVAS_CALLBACK_MOUSE_MOVE, /**< Mouse Move Event */
   EVAS_CALLBACK_MOUSE_WHEEL, /**< Mouse Wheel Event */
   EVAS_CALLBACK_MULTI_DOWN, /**< Multi-touch Down Event */
   EVAS_CALLBACK_MULTI_UP, /**< Multi-touch Up Event */
   EVAS_CALLBACK_MULTI_MOVE, /**< Multi-touch Move Event */
   EVAS_CALLBACK_FREE, /**< Object Being Freed (Called after Del) */
   EVAS_CALLBACK_KEY_DOWN, /**< Key Press Event */
   EVAS_CALLBACK_KEY_UP, /**< Key Release Event */
   EVAS_CALLBACK_FOCUS_IN, /**< Focus In Event */
   EVAS_CALLBACK_FOCUS_OUT, /**< Focus Out Event */
   EVAS_CALLBACK_SHOW, /**< Show Event */
   EVAS_CALLBACK_HIDE, /**< Hide Event */
   EVAS_CALLBACK_MOVE, /**< Move Event */
   EVAS_CALLBACK_RESIZE, /**< Resize Event */
   EVAS_CALLBACK_RESTACK, /**< Restack Event */
   EVAS_CALLBACK_DEL, /**< Object Being Deleted (called before Free) */
   EVAS_CALLBACK_HOLD, /**< Events go on/off hold */
   EVAS_CALLBACK_CHANGED_SIZE_HINTS, /**< Size hints changed event */
   EVAS_CALLBACK_IMAGE_PRELOADED, /**< Image as been preloaded */

   /*
    * The following events are only for use with canvas
    * evas_event_callback_add():
    */
   EVAS_CALLBACK_CANVAS_FOCUS_IN, /**< Canvas got focus as a whole */
   EVAS_CALLBACK_CANVAS_FOCUS_OUT, /**< Canvas lost focus as a whole */
   EVAS_CALLBACK_RENDER_FLUSH_PRE, /**< Called just before rendering is updated on the canvas target */
   EVAS_CALLBACK_RENDER_FLUSH_POST, /**< Called just after rendering is updated on the canvas target */
   EVAS_CALLBACK_CANVAS_OBJECT_FOCUS_IN, /**< Canvas object got focus */
   EVAS_CALLBACK_CANVAS_OBJECT_FOCUS_OUT, /**< Canvas object lost focus */

   /*
    * More object event types - see evas_object_event_callback_add():
    */
   EVAS_CALLBACK_IMAGE_UNLOADED, /**< Image data has been unloaded (by some mechanims in evas that throws out original image data) */

   /* the following id no event number, but a sentinel: */
   EVAS_CALLBACK_LAST /**< keep as last element/sentinel -- not really an event */
} Evas_Callback_Type; /**< The type of event to trigger the callback */

/**
 * Flags for Mouse Button events
 */
typedef enum _Evas_Button_Flags
{
   EVAS_BUTTON_NONE = 0, /**< No extra mouse button data */
   EVAS_BUTTON_DOUBLE_CLICK = (1 << 0), /**< This mouse button press was the 2nd press of a double click */
   EVAS_BUTTON_TRIPLE_CLICK = (1 << 1) /**< This mouse button press was the 3rd press of a triple click */
} Evas_Button_Flags; /**< Flags for Mouse Button events */

/**
 * Flags for Events
 */
typedef enum _Evas_Event_Flags
{
   EVAS_EVENT_FLAG_NONE = 0, /**< No fancy flags set */
   EVAS_EVENT_FLAG_ON_HOLD = (1 << 0), /**< This event is being delivered but should be put "on hold" until the on hold flag is unset. the event should be used for informational purposes and maybe some indications visually, but not actually perform anything */
   EVAS_EVENT_FLAG_ON_SCROLL = (1 << 1) /**< This event flag indicates the event occurs while scrolling; for exameple, DOWN event occurs during scrolling; the event should be used for informational purposes and maybe some indications visually, but not actually perform anything */
} Evas_Event_Flags; /**< Flags for Events */

/**
 * Flags for Font Hinting
 * @ingroup Evas_Font_Group
 */
typedef enum _Evas_Font_Hinting_Flags
{
   EVAS_FONT_HINTING_NONE, /**< No font hinting */
   EVAS_FONT_HINTING_AUTO, /**< Automatic font hinting */
   EVAS_FONT_HINTING_BYTECODE /**< Bytecode font hinting */
} Evas_Font_Hinting_Flags; /**< Flags for Font Hinting */

/**
 * Colorspaces for pixel data supported by Evas
 * @ingroup Evas_Object_Image
 */
typedef enum _Evas_Colorspace
{
   EVAS_COLORSPACE_ARGB8888, /**< ARGB 32 bits per pixel, high-byte is Alpha, accessed 1 32bit word at a time */
     /* these are not currently supported - but planned for the future */
   EVAS_COLORSPACE_YCBCR422P601_PL, /**< YCbCr 4:2:2 Planar, ITU.BT-601 specifications. The data poitned to is just an array of row pointer, pointing to the Y rows, then the Cb, then Cr rows */
   EVAS_COLORSPACE_YCBCR422P709_PL,/**< YCbCr 4:2:2 Planar, ITU.BT-709 specifications. The data poitned to is just an array of row pointer, pointing to the Y rows, then the Cb, then Cr rows */
   EVAS_COLORSPACE_RGB565_A5P, /**< 16bit rgb565 + Alpha plane at end - 5 bits of the 8 being used per alpha byte */
   EVAS_COLORSPACE_GRY8 /**< 8bit grayscale */
} Evas_Colorspace; /**< Colorspaces for pixel data supported by Evas */

/**
 * How to pack items into cells in a table.
 * @ingroup Evas_Object_Table
 */
typedef enum _Evas_Object_Table_Homogeneous_Mode
{
  EVAS_OBJECT_TABLE_HOMOGENEOUS_NONE = 0,
  EVAS_OBJECT_TABLE_HOMOGENEOUS_TABLE = 1,
  EVAS_OBJECT_TABLE_HOMOGENEOUS_ITEM = 2
} Evas_Object_Table_Homogeneous_Mode; /**< Table cell pack mode. */

typedef struct _Evas_Coord_Rectangle  Evas_Coord_Rectangle; /**< A generic rectangle handle */
typedef struct _Evas_Point                   Evas_Point; /**< integer point */

typedef struct _Evas_Coord_Point             Evas_Coord_Point;  /**< Evas_Coord point */
typedef struct _Evas_Coord_Precision_Point   Evas_Coord_Precision_Point; /**< Evas_Coord point with sub-pixel precision */

typedef struct _Evas_Position                Evas_Position; /**< associates given point in Canvas and Output */
typedef struct _Evas_Precision_Position      Evas_Precision_Position; /**< associates given point in Canvas and Output, with sub-pixel precision */

/**
 * @typedef Evas_Smart_Class
 * A smart object base class
 * @ingroup Evas_Smart_Group
 */
typedef struct _Evas_Smart_Class             Evas_Smart_Class;

/**
 * @typedef Evas_Smart_Cb_Description
 * A smart object callback description, used to provide introspection
 * @ingroup Evas_Smart_Group
 */
typedef struct _Evas_Smart_Cb_Description    Evas_Smart_Cb_Description;

/**
 * @typedef Evas_Map
 * An opaque handle to map points
 * @see evas_map_new()
 * @see evas_map_free()
 * @see evas_map_dup()
 * @ingroup Evas_Object_Group_Map
 */
typedef struct _Evas_Map            Evas_Map;

/**
 * @typedef Evas
 * An opaque handle to an Evas canvas.
 * @see evas_new()
 * @see evas_free()
 * @ingroup Evas_Canvas
 */
typedef struct _Evas                Evas;

/**
 * @typedef Evas_Object
 * An Evas Object handle.
 * @ingroup Evas_Object_Group
 */
typedef struct _Evas_Object         Evas_Object;

typedef void                        Evas_Performance; /**< An Evas Performance handle */
typedef struct _Evas_Modifier       Evas_Modifier; /**< An opaque type containing information on which modifier keys are registered in an Evas canvas */
typedef struct _Evas_Lock           Evas_Lock; /**< An opaque type containing information on which lock keys are registered in an Evas canvas */
typedef struct _Evas_Smart          Evas_Smart; /**< An Evas Smart Object handle */
typedef struct _Evas_Native_Surface Evas_Native_Surface; /**< A generic datatype for engine specific native surface information */
typedef unsigned long long          Evas_Modifier_Mask; /**< An Evas modifier mask type */

typedef int                         Evas_Coord;
typedef int                         Evas_Font_Size;
typedef int                         Evas_Angle;

struct _Evas_Coord_Rectangle /**< A rectangle in Evas_Coord */
{
   Evas_Coord x; /**< top-left x co-ordinate of rectangle */
   Evas_Coord y; /**< top-left y co-ordinate of rectangle */
   Evas_Coord w; /**< width of rectangle */
   Evas_Coord h; /**< height of rectangle */
};

struct _Evas_Point
{
   int x, y;
};

struct _Evas_Coord_Point
{
   Evas_Coord x, y;
};

struct _Evas_Coord_Precision_Point
{
   Evas_Coord x, y;
   double xsub, ysub;
};

struct _Evas_Position
{
    Evas_Point output;
    Evas_Coord_Point canvas;
};

struct _Evas_Precision_Position
{
    Evas_Point output;
    Evas_Coord_Precision_Point canvas;
};

typedef enum _Evas_Aspect_Control
{
   EVAS_ASPECT_CONTROL_NONE = 0, /**< Preference on scaling unset */
   EVAS_ASPECT_CONTROL_NEITHER = 1, /**< Same effect as unset preference on scaling */
   EVAS_ASPECT_CONTROL_HORIZONTAL = 2, /**< Use all horizontal container space to place an object, using the given aspect */
   EVAS_ASPECT_CONTROL_VERTICAL = 3, /**< Use all vertical container space to place an object, using the given aspect */
   EVAS_ASPECT_CONTROL_BOTH = 4 /**< Use all horizontal @b and vertical container spaces to place an object (never growing it out of those bounds), using the given aspect */
} Evas_Aspect_Control; /**< Aspect types/policies for scaling size hints, used for evas_object_size_hint_aspect_set() */

typedef struct _Evas_Pixel_Import_Source Evas_Pixel_Import_Source; /**< A source description of pixels for importing pixels */
typedef struct _Evas_Engine_Info      Evas_Engine_Info; /**< A generic Evas Engine information structure */
typedef struct _Evas_Device           Evas_Device; /**< A source device handle - where the event came from */
typedef struct _Evas_Event_Mouse_Down Evas_Event_Mouse_Down; /**< Event structure for #EVAS_CALLBACK_MOUSE_DOWN event callbacks */
typedef struct _Evas_Event_Mouse_Up   Evas_Event_Mouse_Up; /**< Event structure for #EVAS_CALLBACK_MOUSE_UP event callbacks */
typedef struct _Evas_Event_Mouse_In   Evas_Event_Mouse_In; /**< Event structure for #EVAS_CALLBACK_MOUSE_IN event callbacks */
typedef struct _Evas_Event_Mouse_Out  Evas_Event_Mouse_Out; /**< Event structure for #EVAS_CALLBACK_MOUSE_OUT event callbacks */
typedef struct _Evas_Event_Mouse_Move Evas_Event_Mouse_Move; /**< Event structure for #EVAS_CALLBACK_MOUSE_MOVE event callbacks */
typedef struct _Evas_Event_Mouse_Wheel Evas_Event_Mouse_Wheel; /**< Event structure for #EVAS_CALLBACK_MOUSE_WHEEL event callbacks */
typedef struct _Evas_Event_Multi_Down Evas_Event_Multi_Down; /**< Event structure for #EVAS_CALLBACK_MULTI_DOWN event callbacks */
typedef struct _Evas_Event_Multi_Up   Evas_Event_Multi_Up; /**< Event structure for #EVAS_CALLBACK_MULTI_UP event callbacks */
typedef struct _Evas_Event_Multi_Move Evas_Event_Multi_Move; /**< Event structure for #EVAS_CALLBACK_MULTI_MOVE event callbacks */
typedef struct _Evas_Event_Key_Down   Evas_Event_Key_Down; /**< Event structure for #EVAS_CALLBACK_KEY_DOWN event callbacks */
typedef struct _Evas_Event_Key_Up     Evas_Event_Key_Up; /**< Event structure for #EVAS_CALLBACK_KEY_UP event callbacks */
typedef struct _Evas_Event_Hold       Evas_Event_Hold; /**< Event structure for #EVAS_CALLBACK_HOLD event callbacks */

typedef enum _Evas_Load_Error
{
   EVAS_LOAD_ERROR_NONE = 0, /**< No error on load */
   EVAS_LOAD_ERROR_GENERIC = 1, /**< A non-specific error occurred */
   EVAS_LOAD_ERROR_DOES_NOT_EXIST = 2, /**< File (or file path) does not exist */
   EVAS_LOAD_ERROR_PERMISSION_DENIED = 3, /**< Permission deinied to an existing file (or path) */
   EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED = 4, /**< Allocation of resources failure prevented load */
   EVAS_LOAD_ERROR_CORRUPT_FILE = 5, /**< File corrupt (but was detected as a known format) */
   EVAS_LOAD_ERROR_UNKNOWN_FORMAT = 6 /**< File is not a known format */
} Evas_Load_Error; /**< Evas image load error codes one can get - see evas_load_error_str() too. */


typedef enum _Evas_Alloc_Error
{
   EVAS_ALLOC_ERROR_NONE = 0, /**< No allocation error */
   EVAS_ALLOC_ERROR_FATAL = 1, /**< Allocation failed despite attempts to free up memory */
   EVAS_ALLOC_ERROR_RECOVERED = 2 /**< Allocation succeeded, but extra memory had to be found by freeing up speculative resources */
} Evas_Alloc_Error; /**< Possible allocation errors returned by evas_alloc_error() */

typedef enum _Evas_Fill_Spread
{
   EVAS_TEXTURE_REFLECT = 0, /**< image fill tiling mode - tiling reflects */
   EVAS_TEXTURE_REPEAT = 1, /**< tiling repeats */
   EVAS_TEXTURE_RESTRICT = 2, /**< tiling clamps - range offset ignored */
   EVAS_TEXTURE_RESTRICT_REFLECT = 3, /**< tiling clamps and any range offset reflects */
   EVAS_TEXTURE_RESTRICT_REPEAT = 4, /**< tiling clamps and any range offset repeats */
   EVAS_TEXTURE_PAD = 5 /**< tiling extends with end values */
} Evas_Fill_Spread; /**< Fill types used for evas_object_image_fill_spread_set() */

typedef enum _Evas_Pixel_Import_Pixel_Format
{
   EVAS_PIXEL_FORMAT_NONE = 0, /**< No pixel format */
   EVAS_PIXEL_FORMAT_ARGB32 = 1, /**< ARGB 32bit pixel format with A in the high byte per 32bit pixel word */
   EVAS_PIXEL_FORMAT_YUV420P_601 = 2 /**< YUV 420 Planar format with CCIR 601 color encoding wuth contiguous planes in the order Y, U and V */
} Evas_Pixel_Import_Pixel_Format; /**< Pixel format for import call. See evas_object_image_pixels_import() */

struct _Evas_Pixel_Import_Source
{
   Evas_Pixel_Import_Pixel_Format format; /**< pixel format type ie ARGB32, YUV420P_601 etc. */
   int w, h; /**< width and height of source in pixels */
   void **rows; /**< an array of pointers (size depends on format) pointing to left edge of each scanline */
};

/* magic version number to know what the native surf struct looks like */
#define EVAS_NATIVE_SURFACE_VERSION 2

typedef enum _Evas_Native_Surface_Type
{
   EVAS_NATIVE_SURFACE_NONE,
   EVAS_NATIVE_SURFACE_X11,
   EVAS_NATIVE_SURFACE_OPENGL
} Evas_Native_Surface_Type;

struct _Evas_Native_Surface
{
   int                         version;
   Evas_Native_Surface_Type    type;
   union {
     struct {
       void          *visual; /**< visual of the pixmap to use (Visual) */
       unsigned long  pixmap; /**< pixmap id to use (Pixmap) */
     } x11;
     struct {
       unsigned int   texture_id; /**< opengl texture id to use from glGenTextures() */
       unsigned int   framebuffer_id; /**< 0 if not a FBO, FBO id otherwise from glGenFramebuffers() */
       unsigned int   internal_format; /**< same as 'internalFormat' for glTexImage2D() */
       unsigned int   format; /**< same as 'format' for glTexImage2D() */
       unsigned int   x, y, w, h; /**< region inside the texture to use (image size is assumed as texture size, with 0, 0 being the top-left and co-ordinates working down to the right and bottom being positive) */
     } opengl;
   } data;
};

#define EVAS_LAYER_MIN -32768 /**< bottom-most layer number */
#define EVAS_LAYER_MAX 32767  /**< top-most layer number */

#define EVAS_COLOR_SPACE_ARGB 0 /**< Not used for anything */
#define EVAS_COLOR_SPACE_AHSV 1 /**< Not used for anything */
#define EVAS_TEXT_INVALID -1 /**< Not used for anything */
#define EVAS_TEXT_SPECIAL -2 /**< Not used for anything */

#define EVAS_HINT_EXPAND  1.0 /**< Use with evas_object_size_hint_weight_set(), evas_object_size_hint_weight_get(), evas_object_size_hint_expand_set(), evas_object_size_hint_expand_get() */
#define EVAS_HINT_FILL   -1.0 /**< Use with evas_object_size_hint_align_set(), evas_object_size_hint_align_get(), evas_object_size_hint_fill_set(), evas_object_size_hint_fill_get() */
#define evas_object_size_hint_fill_set evas_object_size_hint_align_set /**< Convenience macro to make it easier to understand that align is also used for fill properties (as fill is mutually exclusive to align) */
#define evas_object_size_hint_fill_get evas_object_size_hint_align_get /**< Convenience macro to make it easier to understand that align is also used for fill properties (as fill is mutually exclusive to align) */
#define evas_object_size_hint_expand_set evas_object_size_hint_weight_set /**< Convenience macro to make it easier to understand that weight is also used for expand properties */
#define evas_object_size_hint_expand_get evas_object_size_hint_weight_get /**< Convenience macro to make it easier to understand that weight is also used for expand properties */

/**
 * How the object should be rendered to output.
 * @ingroup Evas_Object_Group_Extras
 */
typedef enum _Evas_Render_Op
{
   EVAS_RENDER_BLEND = 0, /**< default op: d = d*(1-sa) + s */
   EVAS_RENDER_BLEND_REL = 1, /**< d = d*(1 - sa) + s*da */
   EVAS_RENDER_COPY = 2, /**< d = s */
   EVAS_RENDER_COPY_REL = 3, /**< d = s*da */
   EVAS_RENDER_ADD = 4, /**< d = d + s */
   EVAS_RENDER_ADD_REL = 5, /**< d = d + s*da */
   EVAS_RENDER_SUB = 6, /**< d = d - s */
   EVAS_RENDER_SUB_REL = 7, /**< d = d - s*da */
   EVAS_RENDER_TINT = 8, /**< d = d*s + d*(1 - sa) + s*(1 - da) */
   EVAS_RENDER_TINT_REL = 9, /**< d = d*(1 - sa + s) */
   EVAS_RENDER_MASK = 10, /**< d = d*sa */
   EVAS_RENDER_MUL = 11 /**< d = d*s */
} Evas_Render_Op; /**< How the object should be rendered to output. */

typedef enum _Evas_Border_Fill_Mode
{
   EVAS_BORDER_FILL_NONE = 0,
   EVAS_BORDER_FILL_DEFAULT = 1,
   EVAS_BORDER_FILL_SOLID = 2
} Evas_Border_Fill_Mode;

typedef enum _Evas_Image_Scale_Hint
{
   EVAS_IMAGE_SCALE_HINT_NONE = 0,
   EVAS_IMAGE_SCALE_HINT_DYNAMIC = 1,
   EVAS_IMAGE_SCALE_HINT_STATIC = 2
} Evas_Image_Scale_Hint;

typedef enum _Evas_Engine_Render_Mode
{
   EVAS_RENDER_MODE_BLOCKING = 0,
   EVAS_RENDER_MODE_NONBLOCKING = 1,
} Evas_Engine_Render_Mode;

typedef enum _Evas_Image_Content_Hint
{
   EVAS_IMAGE_CONTENT_HINT_NONE = 0,
   EVAS_IMAGE_CONTENT_HINT_DYNAMIC = 1,
   EVAS_IMAGE_CONTENT_HINT_STATIC = 2
} Evas_Image_Content_Hint;

struct _Evas_Engine_Info /** Generic engine information. Generic info is useless */
{
   int magic; /**< Magic number */
};

struct _Evas_Event_Mouse_Down /** Mouse button press event */
{
   int button; /**< Mouse button number that went down (1 - 32) */

   Evas_Point output;
   Evas_Coord_Point canvas;

   void          *data;
   Evas_Modifier *modifiers;
   Evas_Lock     *locks;

   Evas_Button_Flags flags;
   unsigned int      timestamp;
   Evas_Event_Flags  event_flags;
   Evas_Device      *dev;
};

struct _Evas_Event_Mouse_Up /** Mouse button release event */
{
   int button; /**< Mouse button number that was raised (1 - 32) */

   Evas_Point output;
   Evas_Coord_Point canvas;

   void          *data;
   Evas_Modifier *modifiers;
   Evas_Lock     *locks;

   Evas_Button_Flags flags;
   unsigned int      timestamp;
   Evas_Event_Flags  event_flags;
   Evas_Device      *dev;
};

struct _Evas_Event_Mouse_In /** Mouse enter event */
{
   int buttons; /**< Button pressed mask, Bits set to 1 are buttons currently pressed (bit 0 = mouse button 1, bit 1 = mouse button 2 etc.) */

   Evas_Point output;
   Evas_Coord_Point canvas;

   void          *data;
   Evas_Modifier *modifiers;
   Evas_Lock     *locks;
   unsigned int   timestamp;
   Evas_Event_Flags  event_flags;
   Evas_Device      *dev;
};

struct _Evas_Event_Mouse_Out /** Mouse leave event */
{
   int buttons; /**< Button pressed mask, Bits set to 1 are buttons currently pressed (bit 0 = mouse button 1, bit 1 = mouse button 2 etc.) */


   Evas_Point output;
   Evas_Coord_Point canvas;

   void          *data;
   Evas_Modifier *modifiers;
   Evas_Lock     *locks;
   unsigned int   timestamp;
   Evas_Event_Flags  event_flags;
   Evas_Device      *dev;
};

struct _Evas_Event_Mouse_Move /** Mouse button down event */
{
   int buttons; /**< Button pressed mask, Bits set to 1 are buttons currently pressed (bit 0 = mouse button 1, bit 1 = mouse button 2 etc.) */

   Evas_Position cur, prev;

   void          *data;
   Evas_Modifier *modifiers;
   Evas_Lock     *locks;
   unsigned int   timestamp;
   Evas_Event_Flags  event_flags;
   Evas_Device      *dev;
};

struct _Evas_Event_Mouse_Wheel /** Wheel event */
{
   int direction; /* 0 = default up/down wheel FIXME: more wheel types */
   int z; /* ...,-2,-1 = down, 1,2,... = up */

   Evas_Point output;
   Evas_Coord_Point canvas;

   void          *data;
   Evas_Modifier *modifiers;
   Evas_Lock     *locks;
   unsigned int   timestamp;
   Evas_Event_Flags  event_flags;
   Evas_Device      *dev;
};

struct _Evas_Event_Multi_Down /** Multi button press event */
{
   int device; /**< Multi device number that went down (1 or more for extra touches) */
   double radius, radius_x, radius_y;
   double pressure, angle;

   Evas_Point output;
   Evas_Coord_Precision_Point canvas;

   void          *data;
   Evas_Modifier *modifiers;
   Evas_Lock     *locks;

   Evas_Button_Flags flags;
   unsigned int      timestamp;
   Evas_Event_Flags  event_flags;
   Evas_Device      *dev;
};

struct _Evas_Event_Multi_Up /** Multi button release event */
{
   int device; /**< Multi device number that went up (1 or more for extra touches) */
   double radius, radius_x, radius_y;
   double pressure, angle;

   Evas_Point output;
   Evas_Coord_Precision_Point canvas;

   void          *data;
   Evas_Modifier *modifiers;
   Evas_Lock     *locks;

   Evas_Button_Flags flags;
   unsigned int      timestamp;
   Evas_Event_Flags  event_flags;
   Evas_Device      *dev;
};

struct _Evas_Event_Multi_Move /** Multi button down event */
{
   int device; /**< Multi device number that moved (1 or more for extra touches) */
   double radius, radius_x, radius_y;
   double pressure, angle;

   Evas_Precision_Position cur;

   void          *data;
   Evas_Modifier *modifiers;
   Evas_Lock     *locks;
   unsigned int   timestamp;
   Evas_Event_Flags  event_flags;
   Evas_Device      *dev;
};

struct _Evas_Event_Key_Down /** Key press event */
{
   char          *keyname; /**< The string name of the key pressed */
   void          *data;
   Evas_Modifier *modifiers;
   Evas_Lock     *locks;

   const char    *key; /**< The logical key : (eg shift+1 == exclamation) */
   const char    *string; /**< A UTF8 string if this keystroke has produced a visible string to be ADDED */
   const char    *compose; /**< A UTF8 string if this keystroke has modified a string in the middle of being composed - this string replaces the previous one */
   unsigned int   timestamp;
   Evas_Event_Flags  event_flags;
   Evas_Device      *dev;
};

struct _Evas_Event_Key_Up /** Key release event */
{
   char          *keyname; /**< The string name of the key released */
   void          *data;
   Evas_Modifier *modifiers;
   Evas_Lock     *locks;

   const char    *key; /**< The logical key : (eg shift+1 == exclamation) */
   const char    *string; /**< A UTF8 string if this keystroke has produced a visible string to be ADDED */
   const char    *compose; /**< A UTF8 string if this keystroke has modified a string in the middle of being composed - this string replaces the previous one */
   unsigned int   timestamp;
   Evas_Event_Flags  event_flags;
   Evas_Device      *dev;
};

struct _Evas_Event_Hold /** Hold change event */
{
   int            hold; /**< The hold flag */
   void          *data;

   unsigned int   timestamp;
   Evas_Event_Flags  event_flags;
   Evas_Device      *dev;
};

/**
 * How mouse pointer should be handled by Evas.
 *
 * If #EVAS_OBJECT_POINTER_MODE_AUTOGRAB, then when mouse is down an
 * object, then moves outside of it, the pointer still behaves as
 * being bound to the object, albeit out of its drawing region. On
 * mouse up, the event will be feed to the object, that may check if
 * the final position is over or not and do something about it.
 *
 * @ingroup Evas_Object_Group_Extras
 */
typedef enum _Evas_Object_Pointer_Mode
{
   EVAS_OBJECT_POINTER_MODE_AUTOGRAB, /**< default, X11-like */
   EVAS_OBJECT_POINTER_MODE_NOGRAB
} Evas_Object_Pointer_Mode; /**< How mouse pointer should be handled by Evas. */

typedef void      (*Evas_Smart_Cb) (void *data, Evas_Object *obj, void *event_info);
typedef void      (*Evas_Event_Cb) (void *data, Evas *e, void *event_info); /**< Evas event callback function signature */
typedef Eina_Bool (*Evas_Object_Event_Post_Cb) (void *data, Evas *e);
typedef void      (*Evas_Object_Event_Cb) (void *data, Evas *e, Evas_Object *obj, void *event_info);
typedef void      (*Evas_Async_Events_Put_Cb)(void *target, Evas_Callback_Type type, void *event_info);

/**
 * @defgroup Evas_Group Top Level Functions
 *
 * Functions that affect Evas as a whole.
 */

/**
 * Initialize Evas
 *
 * @return The init counter value.
 *
 * This function initializes Evas and increments a counter of the
 * number of calls to it. It returs the new counter's value.
 *
 * @see evas_shutdown().
 *
 * Most EFL users wouldn't be using this function directly, because
 * they wouldn't access Evas directly by themselves. Instead, they
 * would be using higher level helpers, like @c ecore_evas_init().
 * See http://docs.enlightenment.org/auto/ecore/.
 *
 * You should be using this if your use is something like the
 * following. The buffer engine is just one of the many ones Evas
 * provides.
 *
 * @dontinclude evas-buffer-simple.c
 * @skip int main
 * @until return -1;
 * And being the canvas creation something like:
 * @skip static Evas *create_canvas
 * @until    evas_output_viewport_set(canvas,
 *
 * Note that this is code creating an Evas canvas with no usage of
 * Ecore helpers at all -- no linkage with Ecore on this scenario,
 * thus. Again, this wouldn't be on Evas common usage for most
 * developers. See the full @ref Example_Evas_Buffer_Simple "example".
 *
 * @ingroup Evas_Group
 */
EAPI int               evas_init                         (void);

/**
 * Shutdown Evas
 *
 * @return Evas' init counter value.
 *
 * This function finalizes Evas, decrementing the counter of the
 * number of calls to the function evas_init(). This new value for the
 * counter is returned.
 *
 * @see evas_init().
 *
 * If you were the sole user of Evas, by means of evas_init(), you can
 * check if it's being properly shut down by expecting a return value
 * of 0.
 *
 * Example code follows.
 * @dontinclude evas-buffer-simple.c
 * @skip // NOTE: use ecore_evas_buffer_new
 * @until evas_shutdown
 * Where that function would contain:
 * @skip   evas_free(canvas)
 * @until   evas_free(canvas)
 *
 * Most users would be using ecore_evas_shutdown() instead, like told
 * in evas_init(). See the full @ref Example_Evas_Buffer_Simple
 * "example".
 *
 * @ingroup Evas_Group
 */
EAPI int               evas_shutdown                     (void);


/**
 * Return if any allocation errors have occurred during the prior function
 * @return The allocation error flag
 *
 * This function will return if any memory allocation errors occurred during,
 * and what kind they were. The return value will be one of
 * EVAS_ALLOC_ERROR_NONE, EVAS_ALLOC_ERROR_FATAL or EVAS_ALLOC_ERROR_RECOVERED
 * with each meaning something different.
 *
 * EVAS_ALLOC_ERROR_NONE means that no errors occurred at all and the function
 * worked as expected.
 *
 * EVAS_ALLOC_ERROR_FATAL means the function was completely unable to perform
 * its job and will  have  exited as cleanly as possible. The programmer
 * should consider this as a sign of very low memory and should try and safely
 * recover from the prior functions failure (or try free up memory elsewhere
 * and try again after more memory is freed).
 *
 * EVAS_ALLOC_ERROR_RECOVERED means that an allocation error occurred, but was
 * recovered from by evas finding memory of its own it has allocated and
 * freeing what it sees as not really usefully allocated memory. What is freed
 * may vary. Evas may reduce the resolution of images, free cached images or
 * fonts, trhow out pre-rendered data, reduce the complexity of change lists
 * etc. Evas and the program will function as per normal after this, but this
 * is a sign of low memory, and it is suggested that the program try and
 * identify memory it doesn't need, and free it.
 *
 * Example:
 * @code
 * extern Evas_Object *object;
 * void callback (void *data, Evas *e, Evas_Object *obj, void *event_info);
 *
 * evas_object_event_callback_add(object, EVAS_CALLBACK_MOUSE_DOWN, callback, NULL);
 * if (evas_alloc_error() == EVAS_ALLOC_ERROR_FATAL)
 *   {
 *     fprintf(stderr, "ERROR: Completely unable to attach callback. Must\n");
 *     fprintf(stderr, "       destroy object now as it cannot be used.\n");
 *     evas_object_del(object);
 *     object = NULL;
 *     fprintf(stderr, "WARNING: Memory is really low. Cleaning out RAM.\n");
 *     my_memory_cleanup();
 *   }
 * if (evas_alloc_error() == EVAS_ALLOC_ERROR_RECOVERED)
 *   {
 *     fprintf(stderr, "WARNING: Memory is really low. Cleaning out RAM.\n");
 *     my_memory_cleanup();
 *   }
 * @endcode
 *
 * @ingroup Evas_Group
 */
EAPI Evas_Alloc_Error  evas_alloc_error                  (void);


/**
 * @brief Get evas' internal asynchronous events read file descriptor.
 *
 * @return The canvas' asynchronous events read file descriptor.
 *
 * Evas' asynchronous events are meant to be dealt with internally,
 * i. e., when building stuff to be glued together into the EFL
 * infrastructure -- a module, for example. The context which demands
 * its use is when calculations need to be done out of the main
 * thread, asynchronously, and some action must be performed after
 * that.
 *
 * An example of actual use of this API is for image asynchronous
 * preload inside evas. If the canvas was instantiated through
 * ecore-evas usage, ecore itself will take care of calling those
 * events' processing.
 *
 * This function returns the read file descriptor where to get the
 * asynchronous events of the canvas. Naturally, other mainloops,
 * apart from ecore, may make use of it.
 *
 * @ingroup Evas_Group
 */
EAPI int               evas_async_events_fd_get          (void) EINA_WARN_UNUSED_RESULT EINA_PURE;

/**
 * @brief Trigger the processing of all events waiting on the file
 * descriptor returned by evas_async_events_fd_get().
 *
 * @return The number of events processed.
 *
 * All asynchronous events queued up by evas_async_events_put() are
 * processed here. More precisely, the callback functions, informed
 * together with other event parameters, when queued, get called (with
 * those parameters), in that order.
 *
 * @ingroup Evas_Group
 */
EAPI int               evas_async_events_process         (void);

/**
* Insert asynchronous events on the canvas.
 *
 * @param target The target to be affected by the events.
 * @param type The type of callback function.
 * @param event_info Information about the event.
 * @param func The callback function pointer.
 *
 * This is the way, for a routine running outside evas' main thread,
 * to report an asynchronous event. A callback function is informed,
 * whose call is to happen after evas_async_events_process() is
 * called.
 *
 * @ingroup Evas_Group
 */
EAPI Eina_Bool         evas_async_events_put             (const void *target, Evas_Callback_Type type, void *event_info, Evas_Async_Events_Put_Cb func) EINA_ARG_NONNULL(1, 4);

/**
 * @defgroup Evas_Canvas Canvas Functions
 *
 * Low level Evas canvas functions. Sub groups will present more high
 * level ones, though.
 *
 * Most of these functions deal with low level Evas actions, like:
 * @li create/destroy raw canvases, not bound to any displaying engine
 * @li tell a canvas i got focused (in a windowing context, for example)
 * @li tell a canvas a region should not be calculated anymore in rendering
 * @li tell a canvas to render its contents, immediately
 *
 * Most users will be using Evas by means of the @c Ecore_Evas
 * wrapper, which deals with all the above mentioned issues
 * automatically for them. Thus, you'll be looking at this section
 * only if you're building low level stuff.
 *
 * The groups within present you functions that deal with the canvas
 * directly, too, and not yet with its @b objects. They are the
 * functions you need to use at a minimum to get a working canvas.
 */

/**
 * Creates a new empty evas.
 *
 * Note that before you can use the evas, you will to at a minimum:
 * @li Set its render method with @ref evas_output_method_set .
 * @li Set its viewport size with @ref evas_output_viewport_set .
 * @li Set its size of the canvas with @ref evas_output_size_set .
 * @li Ensure that the render engine is given the correct settings
 *     with @ref evas_engine_info_set .
 *
 * This function should only fail if the memory allocation fails
 *
 * @note this function is very low level. Instead of using it
 *       directly, consider using the high level functions in
 *       Ecore_Evas such as @c ecore_evas_new(). See
 *       http://docs.enlightenment.org/auto/ecore/.
 *
 * @attention it is recommended that one calls evas_init() before
 *       creating new canvas.
 *
 * @return A new uninitialised Evas canvas on success.  Otherwise, @c
 * NULL.
 * @ingroup Evas_Canvas
 */
EAPI Evas             *evas_new                          (void) EINA_WARN_UNUSED_RESULT EINA_MALLOC;

/**
 * Frees the given evas and any objects created on it.
 *
 * Any objects with 'free' callbacks will have those callbacks called
 * in this function.
 *
 * @param   e The given evas.
 *
 * @ingroup Evas_Canvas
 */
EAPI void              evas_free                         (Evas *e)  EINA_ARG_NONNULL(1);


/**
 * Inform to the evas that it got the focus.
 *
 * @param e The evas to change information.
 * @ingroup Evas_Canvas
 */
EAPI void              evas_focus_in                     (Evas *e);

/**
 * Inform to the evas that it lost the focus.
 *
 * @param e The evas to change information.
 * @ingroup Evas_Canvas
 */
EAPI void              evas_focus_out                    (Evas *e);

/**
 * Get the focus state known by the given evas
 *
 * @param e The evas to query information.
 * @ingroup Evas_Canvas
 */
EAPI Eina_Bool         evas_focus_state_get              (const Evas *e) EINA_PURE;

/**
 * Push the nochange flag up 1
 *
 * This tells evas, that while the nochange flag is greater than 0, do not
 * mark objects as "changed" when making changes.
 *
 * @param e The evas to change information.
 * @ingroup Evas_Canvas
 */
EAPI void              evas_nochange_push                (Evas *e);

/**
 * Pop the nochange flag down 1
 *
 * This tells evas, that while the nochange flag is greater than 0, do not
 * mark objects as "changed" when making changes.
 *
 * @param e The evas to change information.
 * @ingroup Evas_Canvas
 */
EAPI void              evas_nochange_pop                 (Evas *e);


/**
 * Attaches a specific pointer to the evas for fetching later
 *
 * @param e The canvas to attach the pointer to
 * @param data The pointer to attach
 * @ingroup Evas_Canvas
 */
EAPI void              evas_data_attach_set              (Evas *e, void *data) EINA_ARG_NONNULL(1);

/**
 * Returns the pointer attached by evas_data_attach_set()
 *
 * @param e The canvas to attach the pointer to
 * @return The pointer attached
 * @ingroup Evas_Canvas
 */
EAPI void             *evas_data_attach_get              (const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;


/**
 * Add a damage rectangle.
 *
 * @param e The given canvas pointer.
 * @param x The rectangle's left position.
 * @param y The rectangle's top position.
 * @param w The rectangle's width.
 * @param h The rectangle's height.
 *
 * This is the function by which one tells evas that a part of the
 * canvas has to be repainted.
 *
 * @ingroup Evas_Canvas
 */
EAPI void              evas_damage_rectangle_add         (Evas *e, int x, int y, int w, int h) EINA_ARG_NONNULL(1);

/**
 * Add an "obscured region" to an Evas canvas.
 *
 * @param e The given canvas pointer.
 * @param x The rectangle's top left corner's horizontal coordinate.
 * @param y The rectangle's top left corner's vertical coordinate
 * @param w The rectangle's width.
 * @param h The rectangle's height.
 *
 * This is the function by which one tells an Evas canvas that a part
 * of it <b>must not</b> be repainted. The region must be
 * rectangular and its coordinates inside the canvas viewport are
 * passed in the call. After this call, the region specified won't
 * participate in any form in Evas' calculations and actions during
 * its rendering updates, having its displaying content frozen as it
 * was just after this function took place.
 *
 * We call it "obscured region" because the most common use case for
 * this rendering (partial) freeze is something else (most problaby
 * other canvas) being on top of the specified rectangular region,
 * thus shading it completely from the user's final scene in a
 * display. To avoid unecessary processing, one should indicate to the
 * obscured canvas not to bother about the non-important area.
 *
 * The majority of users won't have to worry about this funcion, as
 * they'll be using just one canvas in their applications, with
 * nothing inset or on top of it in any form.
 *
 * To make this region one that @b has to be repainted again, call the
 * function evas_obscured_clear().
 *
 * @note This is a <b>very low level function</b>, which most of
 * Evas' users wouldn't care about.
 *
 * @note This function does @b not flag the canvas as having its state
 * changed. If you want to re-render it afterwards expecting new
 * contents, you have to add "damage" regions yourself (see
 * evas_damage_rectangle_add()).
 *
 * @see evas_obscured_clear()
 * @see evas_render_updates()
 *
 * Example code follows.
 * @dontinclude evas-events.c
 * @skip add an obscured
 * @until evas_obscured_clear(evas);
 *
 * In that example, pressing the "Ctrl" and "o" keys will impose or
 * remove an obscured region in the middle of the canvas. You'll get
 * the same contents at the time the key was pressed, if toggling it
 * on, until you toggle it off again (make sure the animation is
 * running on to get the idea better). See the full @ref
 * Example_Evas_Events "example".
 *
 * @ingroup Evas_Canvas
 */
EAPI void              evas_obscured_rectangle_add       (Evas *e, int x, int y, int w, int h) EINA_ARG_NONNULL(1);

/**
 * Remove all "obscured regions" from an Evas canvas.
 *
 * @param e The given canvas pointer.
 *
 * This function removes all the rectangles from the obscured regions
 * list of the canvas @p e. It takes obscured areas added with
 * evas_obscured_rectangle_add() and make them again a regions that @b
 * have to be repainted on rendering updates.
 *
 * @note This is a <b>very low level function</b>, which most of
 * Evas' users wouldn't care about.
 *
 * @note This function does @b not flag the canvas as having its state
 * changed. If you want to re-render it afterwards expecting new
 * contents, you have to add "damage" regions yourself (see
 * evas_damage_rectangle_add()).
 *
 * @see evas_obscured_rectangle_add() for an example
 * @see evas_render_updates()
 *
 * @ingroup Evas_Canvas
 */
EAPI void              evas_obscured_clear               (Evas *e) EINA_ARG_NONNULL(1);

/**
 * Force immediate renderization of the given Evas canvas.
 *
 * @param e The given canvas pointer.
 * @return A newly allocated list of updated rectangles of the canvas
 *        (@c Eina_Rectangle structs). Free this list with
 *        evas_render_updates_free().
 *
 * This function forces an immediate renderization update of the given
 * canvas @e.
 *
 * @note This is a <b>very low level function</b>, which most of
 * Evas' users wouldn't care about. One would use it, for example, to
 * grab an Evas' canvas update regions and paint them back, using the
 * canvas' pixmap, on a displaying system working below Evas.
 *
 * @note Evas is a stateful canvas. If no operations changing its
 * state took place since the last rendering action, you won't see no
 * changes and this call will be a no-op.
 *
 * Example code follows.
 * @dontinclude evas-events.c
 * @skip add an obscured
 * @until d.obscured = !d.obscured;
 *
 * See the full @ref Example_Evas_Events "example".
 *
 * @ingroup Evas_Canvas
 */
EAPI Eina_List        *evas_render_updates               (Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Free the rectangles returned by evas_render_updates().
 *
 * @param updates The list of updated rectangles of the canvas.
 *
 * This function removes the region from the render updates list. It
 * makes the region doesn't be render updated anymore.
 *
 * @see evas_render_updates() for an example
 *
 * @ingroup Evas_Canvas
 */
EAPI void              evas_render_updates_free          (Eina_List *updates);

/**
 * Force renderization of the given canvas.
 *
 * @param e The given canvas pointer.
 *
 * @ingroup Evas_Canvas
 */
EAPI void              evas_render                       (Evas *e) EINA_ARG_NONNULL(1);

/**
 * Update the canvas internal objects but not triggering immediate
 * renderization.
 *
 * @param e The given canvas pointer.
 *
 * This function updates the canvas internal objects not triggering
 * renderization. To force renderization function evas_render() should
 * be used.
 *
 * @see evas_render.
 *
 * @ingroup Evas_Canvas
 */
EAPI void              evas_norender                     (Evas *e) EINA_ARG_NONNULL(1);

/**
 * Make the canvas discard internally cached data used for rendering.
 *
 * @param e The given canvas pointer.
 *
 * This function flushes the arrays of delete, active and render objects.
 * Other things it may also discard are: shared memory segments,
 * temporary scratch buffers, cached data to avoid re-compute of that data etc.
 *
 * @ingroup Evas_Canvas
 */
EAPI void              evas_render_idle_flush            (Evas *e) EINA_ARG_NONNULL(1);

/**
 * Make the canvas discard as much data as possible used by the engine at
 * runtime.
 *
 * @param e The given canvas pointer.
 *
 * This function will unload images, delete textures and much more, where
 * possible. You may also want to call evas_render_idle_flush() immediately
 * prior to this to perhaps discard a little more, though evas_render_dump()
 * should implicitly delete most of what evas_render_idle_flush() might
 * discard too.
 *
 * @ingroup Evas_Canvas
 */
EAPI void              evas_render_dump                  (Evas *e) EINA_ARG_NONNULL(1);

/**
 * @defgroup Evas_Output_Method Render Engine Functions
 *
 * Functions that are used to set the render engine for a given
 * function, and then get that engine working.
 *
 * The following code snippet shows how they can be used to
 * initialise an evas that uses the X11 software engine:
 * @code
 * Evas *evas;
 * Evas_Engine_Info_Software_X11 *einfo;
 * extern Display *display;
 * extern Window win;
 *
 * evas_init();
 *
 * evas = evas_new();
 * evas_output_method_set(evas, evas_render_method_lookup("software_x11"));
 * evas_output_size_set(evas, 640, 480);
 * evas_output_viewport_set(evas, 0, 0, 640, 480);
 * einfo = (Evas_Engine_Info_Software_X11 *)evas_engine_info_get(evas);
 * einfo->info.display = display;
 * einfo->info.visual = DefaultVisual(display, DefaultScreen(display));
 * einfo->info.colormap = DefaultColormap(display, DefaultScreen(display));
 * einfo->info.drawable = win;
 * einfo->info.depth = DefaultDepth(display, DefaultScreen(display));
 * evas_engine_info_set(evas, (Evas_Engine_Info *)einfo);
 * @endcode
 *
 * @ingroup Evas_Canvas
 */

/**
 * Look up a numeric ID from a string name of a rendering engine.
 *
 * @param name The string name of an engine
 * @return A numeric (opaque) ID for the rendering engine
 * @ingroup Evas_Output_Method
 *
 * This function looks up a numeric return value for the named engine
 * in the string @p name. This is a normal C string, NUL byte
 * terminated. The name is case sensitive. If the rendering engine is
 * available, a numeric ID for that engine is returned that is not
 * 0. If the engine is not available, 0 is returned, indicating an
 * invalid engine.
 *
 * The programmer should NEVER rely on the numeric ID of an engine
 * unless it is returned by this function. Programs should NOT be
 * written accessing render method ID's directly, without first
 * obtaining it from this function.
 *
 * @attention it is mandatory that one calls evas_init() before
 *       looking up the render method.
 *
 * Example:
 * @code
 * int engine_id;
 * Evas *evas;
 *
 * evas_init();
 *
 * evas = evas_new();
 * if (!evas)
 *   {
 *     fprintf(stderr, "ERROR: Canvas creation failed. Fatal error.\n");
 *     exit(-1);
 *   }
 * engine_id = evas_render_method_lookup("software_x11");
 * if (!engine_id)
 *   {
 *     fprintf(stderr, "ERROR: Requested rendering engine is absent.\n");
 *     exit(-1);
 *   }
 * evas_output_method_set(evas, engine_id);
 * @endcode
 */
EAPI int               evas_render_method_lookup         (const char *name) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * List all the rendering engines compiled into the copy of the Evas library
 *
 * @return A linked list whose data members are C strings of engine names
 * @ingroup Evas_Output_Method
 *
 * Calling this will return a handle (pointer) to an Evas linked
 * list. Each node in the linked list will have the data pointer be a
 * (char *) pointer to the string name of the rendering engine
 * available. The strings should never be modified, neither should the
 * list be modified. This list should be cleaned up as soon as the
 * program no longer needs it using evas_render_method_list_free(). If
 * no engines are available from Evas, NULL will be returned.
 *
 * Example:
 * @code
 * Eina_List *engine_list, *l;
 * char *engine_name;
 *
 * engine_list = evas_render_method_list();
 * if (!engine_list)
 *   {
 *     fprintf(stderr, "ERROR: Evas supports no engines! Exit.\n");
 *     exit(-1);
 *   }
 * printf("Available Evas Engines:\n");
 * EINA_LIST_FOREACH(engine_list, l, engine_name)
 *     printf("%s\n", engine_name);
 * evas_render_method_list_free(engine_list);
 * @endcode
 */
EAPI Eina_List        *evas_render_method_list           (void) EINA_WARN_UNUSED_RESULT;

/**
 * This function should be called to free a list of engine names
 *
 * @param list The Eina_List base pointer for the engine list to be freed
 * @ingroup Evas_Output_Method
 *
 * When this function is called it will free the engine list passed in
 * as @p list. The list should only be a list of engines generated by
 * calling evas_render_method_list(). If @p list is NULL, nothing will
 * happen.
 *
 * Example:
 * @code
 * Eina_List *engine_list, *l;
 * char *engine_name;
 *
 * engine_list = evas_render_method_list();
 * if (!engine_list)
 *   {
 *     fprintf(stderr, "ERROR: Evas supports no engines! Exit.\n");
 *     exit(-1);
 *   }
 * printf("Available Evas Engines:\n");
 * EINA_LIST_FOREACH(engine_list, l, engine_name)
 *     printf("%s\n", engine_name);
 * evas_render_method_list_free(engine_list);
 * @endcode
 */
EAPI void              evas_render_method_list_free      (Eina_List *list);


/**
 * Sets the output engine for the given evas.
 *
 * Once the output engine for an evas is set, any attempt to change it
 * will be ignored.  The value for @p render_method can be found using
 * @ref evas_render_method_lookup .
 *
 * @param   e             The given evas.
 * @param   render_method The numeric engine value to use.
 *
 * @attention it is mandatory that one calls evas_init() before
 *       setting the output method.
 *
 * @ingroup Evas_Output_Method
 */
EAPI void              evas_output_method_set            (Evas *e, int render_method) EINA_ARG_NONNULL(1);

/**
 * Retrieves the number of the output engine used for the given evas.
 * @param   e The given evas.
 * @return  The ID number of the output engine being used.  @c 0 is
 *          returned if there is an error.
 * @ingroup Evas_Output_Method
 */
EAPI int               evas_output_method_get            (const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;


/**
 * Retrieves the current render engine info struct from the given evas.
 *
 * The returned structure is publicly modifiable.  The contents are
 * valid until either @ref evas_engine_info_set or @ref evas_render
 * are called.
 *
 * This structure does not need to be freed by the caller.
 *
 * @param   e The given evas.
 * @return  A pointer to the Engine Info structure.  @c NULL is returned if
 *          an engine has not yet been assigned.
 * @ingroup Evas_Output_Method
 */
EAPI Evas_Engine_Info *evas_engine_info_get              (const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

/**
 * Applies the engine settings for the given evas from the given @c
 * Evas_Engine_Info structure.
 *
 * To get the Evas_Engine_Info structure to use, call @ref
 * evas_engine_info_get .  Do not try to obtain a pointer to an
 * @c Evas_Engine_Info structure in any other way.
 *
 * You will need to call this function at least once before you can
 * create objects on an evas or render that evas.  Some engines allow
 * their settings to be changed more than once.
 *
 * Once called, the @p info pointer should be considered invalid.
 *
 * @param   e    The pointer to the Evas Canvas
 * @param   info The pointer to the Engine Info to use
 * @return  1 if no error occurred, 0 otherwise
 * @ingroup Evas_Output_Method
 */
EAPI Eina_Bool         evas_engine_info_set              (Evas *e, Evas_Engine_Info *info) EINA_ARG_NONNULL(1);

/**
 * @defgroup Evas_Output_Size Output and Viewport Resizing Functions
 *
 * Functions that set and retrieve the output and viewport size of an
 * evas.
 *
 * @ingroup Evas_Canvas
 */

/**
 * Sets the output size of the render engine of the given evas.
 *
 * The evas will render to a rectangle of the given size once this
 * function is called.  The output size is independent of the viewport
 * size.  The viewport will be stretched to fill the given rectangle.
 *
 * The units used for @p w and @p h depend on the engine used by the
 * evas.
 *
 * @param   e The given evas.
 * @param   w The width in output units, usually pixels.
 * @param   h The height in output units, usually pixels.
 * @ingroup Evas_Output_Size
 */
EAPI void              evas_output_size_set              (Evas *e, int w, int h) EINA_ARG_NONNULL(1);

/**
 * Retrieve the output size of the render engine of the given evas.
 *
 * The output size is given in whatever the output units are for the
 * engine.
 *
 * If either @p w or @p h is @c NULL, then it is ignored.  If @p e is
 * invalid, the returned results are undefined.
 *
 * @param   e The given evas.
 * @param   w The pointer to an integer to store the width in.
 * @param   h The pointer to an integer to store the height in.
 * @ingroup Evas_Output_Size
 */
EAPI void              evas_output_size_get              (const Evas *e, int *w, int *h) EINA_ARG_NONNULL(1);

/**
 * Sets the output viewport of the given evas in evas units.
 *
 * The output viewport is the area of the evas that will be visible to
 * the viewer.  The viewport will be stretched to fit the output
 * target of the evas when rendering is performed.
 *
 * @note The coordinate values do not have to map 1-to-1 with the output
 *       target.  However, it is generally advised that it is done for ease
 *       of use.
 *
 * @param   e The given evas.
 * @param   x The top-left corner x value of the viewport.
 * @param   y The top-left corner y value of the viewport.
 * @param   w The width of the viewport.  Must be greater than 0.
 * @param   h The height of the viewport.  Must be greater than 0.
 * @ingroup Evas_Output_Size
 */
EAPI void              evas_output_viewport_set          (Evas *e, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h) EINA_ARG_NONNULL(1);

/**
 * Get the render engine's output viewport co-ordinates in canvas units.
 * @param e The pointer to the Evas Canvas
 * @param x The pointer to a x variable to be filled in
 * @param y The pointer to a y variable to be filled in
 * @param w The pointer to a width variable to be filled in
 * @param h The pointer to a height variable to be filled in
 * @ingroup Evas_Output_Size
 *
 * Calling this function writes the current canvas output viewport
 * size and location values into the variables pointed to by @p x, @p
 * y, @p w and @p h.  On success the variables have the output
 * location and size values written to them in canvas units. Any of @p
 * x, @p y, @p w or @p h that are NULL will not be written to. If @p e
 * is invalid, the results are undefined.
 *
 * Example:
 * @code
 * extern Evas *evas;
 * Evas_Coord x, y, width, height;
 *
 * evas_output_viewport_get(evas, &x, &y, &w, &h);
 * @endcode
 */
EAPI void              evas_output_viewport_get          (const Evas *e, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h) EINA_ARG_NONNULL(1);

/**
 * @defgroup Evas_Coord_Mapping_Group Coordinate Mapping Functions
 *
 * Functions that are used to map coordinates from the canvas to the
 * screen or the screen to the canvas.
 *
 * @ingroup Evas_Canvas
 */

/**
 * Convert/scale an ouput screen co-ordinate into canvas co-ordinates
 *
 * @param e The pointer to the Evas Canvas
 * @param x The screen/output x co-ordinate
 * @return The screen co-ordinate translated to canvas unit co-ordinates
 * @ingroup Evas_Coord_Mapping_Group
 *
 * This function takes in a horizontal co-ordinate as the @p x
 * parameter and converts it into canvas units, accounting for output
 * size, viewport size and location, returning it as the function
 * return value. If @p e is invalid, the results are undefined.
 *
 * Example:
 * @code
 * extern Evas *evas;
 * extern int screen_x;
 * Evas_Coord canvas_x;
 *
 * canvas_x = evas_coord_screen_x_to_world(evas, screen_x);
 * @endcode
 */
EAPI Evas_Coord        evas_coord_screen_x_to_world      (const Evas *e, int x) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Convert/scale an ouput screen co-ordinate into canvas co-ordinates
 *
 * @param e The pointer to the Evas Canvas
 * @param y The screen/output y co-ordinate
 * @return The screen co-ordinate translated to canvas unit co-ordinates
 * @ingroup Evas_Coord_Mapping_Group
 *
 * This function takes in a vertical co-ordinate as the @p y parameter
 * and converts it into canvas units, accounting for output size,
 * viewport size and location, returning it as the function return
 * value. If @p e is invalid, the results are undefined.
 *
 * Example:
 * @code
 * extern Evas *evas;
 * extern int screen_y;
 * Evas_Coord canvas_y;
 *
 * canvas_y = evas_coord_screen_y_to_world(evas, screen_y);
 * @endcode
 */
EAPI Evas_Coord        evas_coord_screen_y_to_world      (const Evas *e, int y) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Convert/scale a canvas co-ordinate into output screen co-ordinates
 *
 * @param e The pointer to the Evas Canvas
 * @param x The canvas x co-ordinate
 * @return The output/screen co-ordinate translated to output co-ordinates
 * @ingroup Evas_Coord_Mapping_Group
 *
 * This function takes in a horizontal co-ordinate as the @p x
 * parameter and converts it into output units, accounting for output
 * size, viewport size and location, returning it as the function
 * return value. If @p e is invalid, the results are undefined.
 *
 * Example:
 * @code
 * extern Evas *evas;
 * int screen_x;
 * extern Evas_Coord canvas_x;
 *
 * screen_x = evas_coord_world_x_to_screen(evas, canvas_x);
 * @endcode
 */
EAPI int               evas_coord_world_x_to_screen      (const Evas *e, Evas_Coord x) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Convert/scale a canvas co-ordinate into output screen co-ordinates
 *
 * @param e The pointer to the Evas Canvas
 * @param y The canvas y co-ordinate
 * @return The output/screen co-ordinate translated to output co-ordinates
 * @ingroup Evas_Coord_Mapping_Group
 *
 * This function takes in a vertical co-ordinate as the @p x parameter
 * and converts it into output units, accounting for output size,
 * viewport size and location, returning it as the function return
 * value. If @p e is invalid, the results are undefined.
 *
 * Example:
 * @code
 * extern Evas *evas;
 * int screen_y;
 * extern Evas_Coord canvas_y;
 *
 * screen_y = evas_coord_world_y_to_screen(evas, canvas_y);
 * @endcode
 */
EAPI int               evas_coord_world_y_to_screen      (const Evas *e, Evas_Coord y) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @defgroup Evas_Pointer_Group Pointer (Mouse) Functions
 *
 * Functions that deal with the status of the pointer (mouse cursor).
 *
 * @ingroup Evas_Canvas
 */

/**
 * This function returns the current known pointer co-ordinates
 *
 * @param e The pointer to the Evas Canvas
 * @param x The pointer to an integer to be filled in
 * @param y The pointer to an integer to be filled in
 * @ingroup Evas_Pointer_Group
 *
 * This function returns the current known screen/output co-ordinates
 * of the mouse pointer and sets the contents of the integers pointed
 * to by @p x and @p y to contain these co-ordinates. If @p e is not a
 * valid canvas the results of this function are undefined.
 *
 * Example:
 * @code
 * extern Evas *evas;
 * int mouse_x, mouse_y;
 *
 * evas_pointer_output_xy_get(evas, &mouse_x, &mouse_y);
 * printf("Mouse is at screen position %i, %i\n", mouse_x, mouse_y);
 * @endcode
 */
EAPI void              evas_pointer_output_xy_get        (const Evas *e, int *x, int *y) EINA_ARG_NONNULL(1);

/**
 * This function returns the current known pointer co-ordinates
 *
 * @param e The pointer to the Evas Canvas
 * @param x The pointer to a Evas_Coord to be filled in
 * @param y The pointer to a Evas_Coord to be filled in
 * @ingroup Evas_Pointer_Group
 *
 * This function returns the current known canvas unit co-ordinates of
 * the mouse pointer and sets the contents of the Evas_Coords pointed
 * to by @p x and @p y to contain these co-ordinates. If @p e is not a
 * valid canvas the results of this function are undefined.
 *
 * Example:
 * @code
 * extern Evas *evas;
 * Evas_Coord mouse_x, mouse_y;
 *
 * evas_pointer_output_xy_get(evas, &mouse_x, &mouse_y);
 * printf("Mouse is at canvas position %f, %f\n", mouse_x, mouse_y);
 * @endcode
 */
EAPI void              evas_pointer_canvas_xy_get        (const Evas *e, Evas_Coord *x, Evas_Coord *y) EINA_ARG_NONNULL(1);

/**
 * Returns a bitmask with the mouse buttons currently pressed, set to 1
 *
 * @param e The pointer to the Evas Canvas
 * @return A bitmask of the currently depressed buttons on the cavas
 * @ingroup Evas_Pointer_Group
 *
 * Calling this function will return a 32-bit integer with the
 * appropriate bits set to 1 that correspond to a mouse button being
 * depressed. This limits Evas to a mouse devices with a maximum of 32
 * buttons, but that is generally in excess of any host system's
 * pointing device abilities.
 *
 * A canvas by default begins with no mouse buttons being pressed and
 * only calls to evas_event_feed_mouse_down(),
 * evas_event_feed_mouse_down_data(), evas_event_feed_mouse_up() and
 * evas_event_feed_mouse_up_data() will alter that.
 *
 * The least significant bit corresponds to the first mouse button
 * (button 1) and the most significant bit corresponds to the last
 * mouse button (button 32).
 *
 * If @p e is not a valid canvas, the return value is undefined.
 *
 * Example:
 * @code
 * extern Evas *evas;
 * int button_mask, i;
 *
 * button_mask = evas_pointer_button_down_mask_get(evas);
 * printf("Buttons currently pressed:\n");
 * for (i = 0; i < 32; i++)
 *   {
 *     if ((button_mask & (1 << i)) != 0) printf("Button %i\n", i + 1);
 *   }
 * @endcode
 */
EAPI int               evas_pointer_button_down_mask_get (const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Returns whether the mouse pointer is logically inside the canvas
 *
 * @param e The pointer to the Evas Canvas
 * @return An integer that is 1 if the mouse is inside the canvas, 0 otherwise
 * @ingroup Evas_Pointer_Group
 *
 * When this function is called it will return a value of either 0 or
 * 1, depending on if evas_event_feed_mouse_in(),
 * evas_event_feed_mouse_in_data(), or evas_event_feed_mouse_out(),
 * evas_event_feed_mouse_out_data() have been called to feed in a
 * mouse enter event into the canvas.
 *
 * A return value of 1 indicates the mouse is logically inside the
 * canvas, and 0 implies it is logically outside the canvas.
 *
 * A canvas begins with the mouse being assumed outside (0).
 *
 * If @p e is not a valid canvas, the return value is undefined.
 *
 * Example:
 * @code
 * extern Evas *evas;
 *
 * if (evas_pointer_inside_get(evas)) printf("Mouse is in!\n");
 * else printf("Mouse is out!\n");
 * @endcode
 */
EAPI Eina_Bool         evas_pointer_inside_get           (const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);
   EAPI void              evas_sync(Evas *e) EINA_ARG_NONNULL(1);

/**
 * @defgroup Evas_Canvas_Events Canvas Events
 *
 * Functions relating to canvas events, be they input (mice,
 * keyboards, etc) or output ones (internal states changing, etc).
 *
 * @ingroup Evas_Canvas
 */

/**
 * @addtogroup Evas_Canvas_Events
 * @{
 */

/**
 * Add (register) a callback function to a given canvas event.
 *
 * @param e Canvas to attach a callback to
 * @param type The type of event that will trigger the callback
 * @param func The (callback) function to be called when the event is
 *        triggered
 * @param data The data pointer to be passed to @p func
 *
 * This function adds a function callback to the canvas @p e when the
 * event of type @p type occurs on it. The function pointer is @p
 * func.
 *
 * In the event of a memory allocation error during the addition of
 * the callback to the canvas, evas_alloc_error() should be used to
 * determine the nature of the error, if any, and the program should
 * sensibly try and recover.
 *
 * A callback function must have the ::Evas_Event_Cb prototype
 * definition. The first parameter (@p data) in this definition will
 * have the same value passed to evas_event_callback_add() as the @p
 * data parameter, at runtime. The second parameter @p e is the canvas
 * pointer on which the event occurred. The third parameter @p
 * event_info is a pointer to a data structure that may or may not be
 * passed to the callback, depending on the event type that triggered
 * the callback. This is so because some events don't carry extra
 * context with them, but others do.
 *
 * The event type @p type to trigger the function may be one of
 * #EVAS_CALLBACK_RENDER_FLUSH_PRE, #EVAS_CALLBACK_RENDER_FLUSH_POST,
 * #EVAS_CALLBACK_CANVAS_FOCUS_IN, #EVAS_CALLBACK_CANVAS_FOCUS_OUT,
 * #EVAS_CALLBACK_CANVAS_OBJECT_FOCUS_IN and
 * #EVAS_CALLBACK_CANVAS_OBJECT_FOCUS_OUT. This determines the kind of
 * event that will trigger the callback to be called. Only the last
 * two of the event types listed here provide useful event information
 * data -- a pointer to the recently focused Evas object. For the
 * others the @p event_info pointer is going to be @c NULL.
 *
 * Example:
 * @dontinclude evas-events.c
 * @skip evas_event_callback_add(d.canvas, EVAS_CALLBACK_RENDER_FLUSH_PRE
 * @until two canvas event callbacks
 *
 * Looking to the callbacks registered above,
 * @dontinclude evas-events.c
 * @skip called when our rectangle gets focus
 * @until let's have our events back
 *
 * we see that the canvas flushes its rendering pipeline
 * (#EVAS_CALLBACK_RENDER_FLUSH_PRE) whenever the @c _resize_cb
 * routine takes place: it has to redraw that image at a different
 * size. Also, the callback on an object being focused comes just
 * after we focus it explicitly, on code.
 *
 * See the full @ref Example_Evas_Events "example".
 *
 * @note Be careful not to add the same callback multiple times, if
 * that's not what you want, because Evas won't check if a callback
 * existed before exactly as the one being registered (and thus, call
 * it more than once on the event, in this case). This would make
 * sense if you passed different functions and/or callback data, only.
 */
EAPI void              evas_event_callback_add              (Evas *e, Evas_Callback_Type type, Evas_Event_Cb func, const void *data) EINA_ARG_NONNULL(1, 3);

/**
 * Delete a callback function from the canvas.
 *
 * @param e Canvas to remove a callback from
 * @param type The type of event that was triggering the callback
 * @param func The function that was to be called when the event was triggered
 * @return The data pointer that was to be passed to the callback
 *
 * This function removes the most recently added callback from the
 * canvas @p e which was triggered by the event type @p type and was
 * calling the function @p func when triggered. If the removal is
 * successful it will also return the data pointer that was passed to
 * evas_event_callback_add() when the callback was added to the
 * canvas. If not successful NULL will be returned.
 *
 * Example:
 * @code
 * extern Evas *e;
 * void *my_data;
 * void focus_in_callback(void *data, Evas *e, void *event_info);
 *
 * my_data = evas_event_callback_del(ebject, EVAS_CALLBACK_CANVAS_FOCUS_IN, focus_in_callback);
 * @endcode
 */
EAPI void             *evas_event_callback_del              (Evas *e, Evas_Callback_Type type, Evas_Event_Cb func) EINA_ARG_NONNULL(1, 3);

/**
 * Delete (unregister) a callback function registered to a given
 * canvas event.
 *
 * @param e Canvas to remove an event callback from
 * @param type The type of event that was triggering the callback
 * @param func The function that was to be called when the event was
 *        triggered
 * @param data The data pointer that was to be passed to the callback
 * @return The data pointer that was to be passed to the callback
 *
 * This function removes <b>the first</b> added callback from the
 * canvas @p e matching the event type @p type, the registered
 * function pointer @p func and the callback data pointer @p data. If
 * the removal is successful it will also return the data pointer that
 * was passed to evas_event_callback_add() (that will be the same as
 * the parameter) when the callback(s) was(were) added to the
 * canvas. If not successful @c NULL will be returned. A common use
 * would be to remove an exact match of a callback.
 *
 * Example:
 * @dontinclude evas-events.c
 * @skip evas_event_callback_del_full(evas, EVAS_CALLBACK_RENDER_FLUSH_PRE,
 * @until _object_focus_in_cb, NULL);
 *
 * See the full @ref Example_Evas_Events "example".
 *
 * @note For deletion of canvas events callbacks filtering by just
 * type and function pointer, user evas_event_callback_del().
 */
EAPI void             *evas_event_callback_del_full         (Evas *e, Evas_Callback_Type type, Evas_Event_Cb func, const void *data) EINA_ARG_NONNULL(1, 3);

/**
 * Push a callback on the post-event callback stack
 *
 * @param e Canvas to push the callback on
 * @param func The function that to be called when the stack is unwound
 * @param data The data pointer to be passed to the callback
 *
 * Evas has a stack of callbacks that get called after all the callbacks for
 * an event have triggered (all the objects it triggers on and al the callbacks
 * in each object triggered). When all these have been called, the stack is
 * unwond from most recently to least recently pushed item and removed from the
 * stack calling the callback set for it.
 *
 * This is intended for doing reverse logic-like processing, example - when a
 * child object that happens to get the event later is meant to be able to
 * "steal" functions from a parent and thus on unwind of this stack hav its
 * function called first, thus being able to set flags, or return 0 from the
 * post-callback that stops all other post-callbacks in the current stack from
 * being called (thus basically allowing a child to take control, if the event
 * callback prepares information ready for taking action, but the post callback
 * actually does the action).
 *
 */
EAPI void              evas_post_event_callback_push        (Evas *e, Evas_Object_Event_Post_Cb func, const void *data);

/**
 * Remove a callback from the post-event callback stack
 *
 * @param e Canvas to push the callback on
 * @param func The function that to be called when the stack is unwound
 *
 * This removes a callback from the stack added with
 * evas_post_event_callback_push(). The first instance of the function in
 * the callback stack is removed from being executed when the stack is
 * unwound. Further instances may still be run on unwind.
 */
EAPI void              evas_post_event_callback_remove      (Evas *e, Evas_Object_Event_Post_Cb func);

/**
 * Remove a callback from the post-event callback stack
 *
 * @param e Canvas to push the callback on
 * @param func The function that to be called when the stack is unwound
 * @param data The data pointer to be passed to the callback
 *
 * This removes a callback from the stack added with
 * evas_post_event_callback_push(). The first instance of the function and data
 * in the callback stack is removed from being executed when the stack is
 * unwound. Further instances may still be run on unwind.
 */
EAPI void              evas_post_event_callback_remove_full (Evas *e, Evas_Object_Event_Post_Cb func, const void *data);

/**
 * @defgroup Evas_Event_Freezing_Group Input Events Freezing Functions
 *
 * Functions that deal with the freezing of input event processing of
 * an Evas canvas.
 *
 * There might be scenarios during a graphical user interface
 * program's use when the developer whishes the users wouldn't be able
 * to deliver input events to this application. It may, for example,
 * be the time for it to populate a view or to change some
 * layout. Assuming proper behavior with user interaction during this
 * exact time would be hard, as things are in a changing state. The
 * programmer can then tell the canvas to ignore input events,
 * bringing it back to normal behavior when he/she wants.
 *
 * @ingroup Evas_Canvas_Events
 */

/**
 * @addtogroup Evas_Event_Freezing_Group
 * @{
 */

/**
 * Freeze all input events processing.
 *
 * @param e The canvas to freeze input events processing on.
 *
 * This function will indicate to Evas that the canvas @p e is to have
 * all input event processing frozen until a matching
 * evas_event_thaw() function is called on the same canvas. All events
 * of this kind during the freeze will get @b discarded. Every freeze
 * call must be matched by a thaw call in order to completely thaw out
 * a canvas (i.e. these calls may be nested). The most common use is
 * when you don't want the user to interect with your user interface
 * when you're populating a view or changing the layout.
 *
 * Example:
 * @dontinclude evas-events.c
 * @skip freeze input for 3 seconds
 * @until }
 * @dontinclude evas-events.c
 * @skip let's have our events back
 * @until }
 *
 * See the full @ref Example_Evas_Events "example".
 *
 * If you run that example, you'll see the canvas ignoring all input
 * events for 3 seconds, when the "f" key is pressed. In a more
 * realistic code we would be freezing while a toolkit or Edje was
 * doing some UI changes, thawing it back afterwards.
 */
EAPI void              evas_event_freeze                 (Evas *e) EINA_ARG_NONNULL(1);

/**
 * Thaw a canvas out after freezing (for input events).
 *
 * @param e The canvas to thaw out.
 *
 * This will thaw out a canvas after a matching evas_event_freeze()
 * call. If this call completely thaws out a canvas, i.e., there's no
 * other unbalanced call to evas_event_freeze(), events will start to
 * be processed again, but any "missed" events will @b not be
 * evaluated.
 *
 * See evas_event_freeze() for an example.
 */
EAPI void              evas_event_thaw                   (Evas *e) EINA_ARG_NONNULL(1);

/**
 * Return the freeze count on input events of a given canvas.
 *
 * @param e The canvas to fetch the freeze count from.
 *
 * This returns the number of times the canvas has been told to freeze
 * input events. It is possible to call evas_event_freeze() multiple
 * times, and these must be matched by evas_event_thaw() calls. This
 * call allows the program to discover just how many times things have
 * been frozen in case it may want to break out of a deep freeze state
 * where the count is high.
 *
 * Example:
 * @code
 * extern Evas *evas;
 *
 * while (evas_event_freeze_get(evas) > 0) evas_event_thaw(evas);
 * @endcode
 *
 */
EAPI int               evas_event_freeze_get             (const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

/**
 * After thaw of a canvas, re-evaluate the state of objects and call callbacks
 *
 * @param e The canvas to evaluate after a thaw
 *
 * This is normally called after evas_event_thaw() to re-evaluate mouse
 * containment and other states and thus also call callbacks for mouse in and
 * out on new objects if the state change demands it.
 */
EAPI void              evas_event_thaw_eval              (Evas *e) EINA_ARG_NONNULL(1);

/**
 * @}
 */

/**
 * @defgroup Evas_Event_Feeding_Group Input Events Feeding Functions
 *
 * Functions to tell Evas that input events happened and should be
 * processed.
 *
 * As explained in @ref intro_not_evas, Evas does not know how to poll
 * for input events, so the developer should do it and then feed such
 * events to the canvas to be processed. This is only required if
 * operating Evas directly. Modules such as Ecore_Evas do that for
 * you.
 *
 * @ingroup Evas_Canvas_Events
 */

/**
 * @addtogroup Evas_Event_Feeding_Group
 * @{
 */

/**
 * Mouse down event feed.
 *
 * @param e The given canvas pointer.
 * @param b The button number.
 * @param flags The evas button flags.
 * @param timestamp The timestamp of the mouse down event.
 * @param data The data for canvas.
 *
 * This function will set some evas properties that is necessary when
 * the mouse button is pressed. It prepares information to be treated
 * by the callback function.
 *
 */
EAPI void              evas_event_feed_mouse_down        (Evas *e, int b, Evas_Button_Flags flags, unsigned int timestamp, const void *data) EINA_ARG_NONNULL(1);

/**
 * Mouse up event feed.
 *
 * @param e The given canvas pointer.
 * @param b The button number.
 * @param flags evas button flags.
 * @param timestamp The timestamp of the mouse up event.
 * @param data The data for canvas.
 *
 * This function will set some evas properties that is necessary when
 * the mouse button is released. It prepares information to be treated
 * by the callback function.
 *
 */
EAPI void              evas_event_feed_mouse_up          (Evas *e, int b, Evas_Button_Flags flags, unsigned int timestamp, const void *data) EINA_ARG_NONNULL(1);

/**
 * Mouse move event feed.
 *
 * @param e The given canvas pointer.
 * @param x The horizontal position of the mouse pointer.
 * @param y The vertical position of the mouse pointer.
 * @param timestamp The timestamp of the mouse up event.
 * @param data The data for canvas.
 *
 * This function will set some evas properties that is necessary when
 * the mouse is moved from its last position. It prepares information
 * to be treated by the callback function.
 *
 */
EAPI void              evas_event_feed_mouse_move        (Evas *e, int x, int y, unsigned int timestamp, const void *data) EINA_ARG_NONNULL(1);

/**
 * Mouse in event feed.
 *
 * @param e The given canvas pointer.
 * @param timestamp The timestamp of the mouse up event.
 * @param data The data for canvas.
 *
 * This function will set some evas properties that is necessary when
 * the mouse in event happens. It prepares information to be treated
 * by the callback function.
 *
 */
EAPI void              evas_event_feed_mouse_in          (Evas *e, unsigned int timestamp, const void *data) EINA_ARG_NONNULL(1);

/**
 * Mouse out event feed.
 *
 * @param e The given canvas pointer.
 * @param timestamp Timestamp of the mouse up event.
 * @param data The data for canvas.
 *
 * This function will set some evas properties that is necessary when
 * the mouse out event happens. It prepares information to be treated
 * by the callback function.
 *
 */
EAPI void              evas_event_feed_mouse_out         (Evas *e, unsigned int timestamp, const void *data) EINA_ARG_NONNULL(1);
   EAPI void              evas_event_feed_multi_down        (Evas *e, int d, int x, int y, double rad, double radx, double rady, double pres, double ang, double fx, double fy, Evas_Button_Flags flags, unsigned int timestamp, const void *data);
   EAPI void              evas_event_feed_multi_up          (Evas *e, int d, int x, int y, double rad, double radx, double rady, double pres, double ang, double fx, double fy, Evas_Button_Flags flags, unsigned int timestamp, const void *data);
   EAPI void              evas_event_feed_multi_move        (Evas *e, int d, int x, int y, double rad, double radx, double rady, double pres, double ang, double fx, double fy, unsigned int timestamp, const void *data);

/**
 * Mouse cancel event feed.
 *
 * @param e The given canvas pointer.
 * @param timestamp The timestamp of the mouse up event.
 * @param data The data for canvas.
 *
 * This function will call evas_event_feed_mouse_up() when a
 * mouse cancel event happens.
 *
 */
EAPI void              evas_event_feed_mouse_cancel      (Evas *e, unsigned int timestamp, const void *data) EINA_ARG_NONNULL(1);

/**
 * Mouse wheel event feed.
 *
 * @param e The given canvas pointer.
 * @param direction The wheel mouse direction.
 * @param z How much mouse wheel was scrolled up or down.
 * @param timestamp The timestamp of the mouse up event.
 * @param data The data for canvas.
 *
 * This function will set some evas properties that is necessary when
 * the mouse wheel is scrolled up or down. It prepares information to
 * be treated by the callback function.
 *
 */
EAPI void              evas_event_feed_mouse_wheel       (Evas *e, int direction, int z, unsigned int timestamp, const void *data) EINA_ARG_NONNULL(1);

/**
 * Key down event feed
 *
 * @param e The canvas to thaw out
 * @param keyname  Name of the key
 * @param key The key pressed.
 * @param string A String
 * @param compose The compose string
 * @param timestamp Timestamp of the mouse up event
 * @param data Data for canvas.
 *
 * This function will set some evas properties that is necessary when
 * a key is pressed. It prepares information to be treated by the
 * callback function.
 *
 */
EAPI void              evas_event_feed_key_down          (Evas *e, const char *keyname, const char *key, const char *string, const char *compose, unsigned int timestamp, const void *data) EINA_ARG_NONNULL(1);

/**
 * Key up event feed
 *
 * @param e The canvas to thaw out
 * @param keyname  Name of the key
 * @param key The key released.
 * @param string string
 * @param compose compose
 * @param timestamp Timestamp of the mouse up event
 * @param data Data for canvas.
 *
 * This function will set some evas properties that is necessary when
 * a key is released. It prepares information to be treated by the
 * callback function.
 *
 */
EAPI void              evas_event_feed_key_up            (Evas *e, const char *keyname, const char *key, const char *string, const char *compose, unsigned int timestamp, const void *data) EINA_ARG_NONNULL(1);

/**
 * Hold event feed
 *
 * @param e The given canvas pointer.
 * @param hold The hold.
 * @param timestamp The timestamp of the mouse up event.
 * @param data The data for canvas.
 *
 * This function makes the object to stop sending events.
 *
 */
EAPI void              evas_event_feed_hold              (Evas *e, int hold, unsigned int timestamp, const void *data) EINA_ARG_NONNULL(1);

/**
 * @}
 */

/**
 * @}
 */

/**
 * @defgroup Evas_Image_Group Image Functions
 *
 * Functions that deals with images at canvas level.
 *
 * @ingroup Evas_Canvas
 */

/**
 * Flush the image cache of the canvas.
 *
 * @param e The given evas pointer.
 *
 * This function flushes image cache of canvas.
 *
 */
EAPI void              evas_image_cache_flush            (Evas *e) EINA_ARG_NONNULL(1);

/**
 * Reload the image cache
 *
 * @param e The given evas pointer.
 *
 * This function reloads the image cache of canvas.
 *
 */
EAPI void              evas_image_cache_reload           (Evas *e) EINA_ARG_NONNULL(1);

/**
 * Set the image cache.
 *
 * @param e The given evas pointer.
 * @param size The cache size.
 *
 * This function sets the image cache of canvas.
 *
 */
EAPI void              evas_image_cache_set              (Evas *e, int size) EINA_ARG_NONNULL(1);

/**
 * Set the image cache
 *
 * @param e The given evas pointer.
 *
 * This function returns the image cache of canvas.
 *
 */
EAPI int               evas_image_cache_get              (const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

/**
 * @defgroup Evas_Font_Group Font Functions
 *
 * Functions that deals with fonts.
 *
 * @ingroup Evas_Canvas
 */

/**
 * Changes the font hinting for the given evas.
 *
 * @param e The given evas.
 * @param hinting The hinting to use, one of #EVAS_FONT_HINTING_NONE,
 *        #EVAS_FONT_HINTING_AUTO, #EVAS_FONT_HINTING_BYTECODE.
 * @ingroup Evas_Font_Group
 */
EAPI void                     evas_font_hinting_set        (Evas *e, Evas_Font_Hinting_Flags hinting) EINA_ARG_NONNULL(1);

/**
 * Retrieves the font hinting used by the given evas.
 *
 * @param e The given evas to query.
 * @return The hinting in use, one of #EVAS_FONT_HINTING_NONE,
 *         #EVAS_FONT_HINTING_AUTO, #EVAS_FONT_HINTING_BYTECODE.
 * @ingroup Evas_Font_Group
 */
EAPI Evas_Font_Hinting_Flags  evas_font_hinting_get        (const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

/**
 * Checks if the font hinting is supported by the given evas.
 *
 * @param e The given evas to query.
 * @param hinting The hinting to use, one of #EVAS_FONT_HINTING_NONE,
 *        #EVAS_FONT_HINTING_AUTO, #EVAS_FONT_HINTING_BYTECODE.
 * @return @c EINA_TRUE if it is supported, @c EINA_FALSE otherwise.
 * @ingroup Evas_Font_Group
 */
EAPI Eina_Bool                evas_font_hinting_can_hint   (const Evas *e, Evas_Font_Hinting_Flags hinting) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;


/**
 * Force the given evas and associated engine to flush its font cache.
 *
 * @param e The given evas to flush font cache.
 * @ingroup Evas_Font_Group
 */
EAPI void                     evas_font_cache_flush        (Evas *e) EINA_ARG_NONNULL(1);

/**
 * Changes the size of font cache of the given evas.
 *
 * @param e The given evas to flush font cache.
 * @param size The size, in bytes.
 *
 * @ingroup Evas_Font_Group
 */
EAPI void                     evas_font_cache_set          (Evas *e, int size) EINA_ARG_NONNULL(1);

/**
 * Changes the size of font cache of the given evas.
 *
 * @param e The given evas to flush font cache.
 * @return The size, in bytes.
 *
 * @ingroup Evas_Font_Group
 */
EAPI int                      evas_font_cache_get          (const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;


/**
 * List of available font descriptions known or found by this evas.
 *
 * The list depends on Evas compile time configuration, such as
 * fontconfig support, and the paths provided at runtime as explained
 * in @ref Evas_Font_Path_Group.
 *
 * @param e The evas instance to query.
 * @return a newly allocated list of strings. Do not change the
 *         strings.  Be sure to call evas_font_available_list_free()
 *         after you're done.
 *
 * @ingroup Evas_Font_Group
 */
EAPI Eina_List               *evas_font_available_list     (const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

/**
 * Free list of font descriptions returned by evas_font_dir_available_list().
 *
 * @param e The evas instance that returned such list.
 * @param available the list returned by evas_font_dir_available_list().
 *
 * @ingroup Evas_Font_Group
 */
EAPI void                     evas_font_available_list_free(Evas *e, Eina_List *available) EINA_ARG_NONNULL(1);

/**
 * @defgroup Evas_Font_Path_Group Font Path Functions
 *
 * Functions that edit the paths being used to load fonts.
 *
 * @ingroup Evas_Font_Group
 */

/**
 * Removes all font paths loaded into memory for the given evas.
 * @param   e The given evas.
 * @ingroup Evas_Font_Path_Group
 */
EAPI void              evas_font_path_clear              (Evas *e) EINA_ARG_NONNULL(1);

/**
 * Appends a font path to the list of font paths used by the given evas.
 * @param   e    The given evas.
 * @param   path The new font path.
 * @ingroup Evas_Font_Path_Group
 */
EAPI void              evas_font_path_append             (Evas *e, const char *path) EINA_ARG_NONNULL(1, 2);

/**
 * Prepends a font path to the list of font paths used by the given evas.
 * @param   e The given evas.
 * @param   path The new font path.
 * @ingroup Evas_Font_Path_Group
 */
EAPI void              evas_font_path_prepend            (Evas *e, const char *path) EINA_ARG_NONNULL(1, 2);

/**
 * Retrieves the list of font paths used by the given evas.
 * @param   e The given evas.
 * @return  The list of font paths used.
 * @ingroup Evas_Font_Path_Group
 */
EAPI const Eina_List  *evas_font_path_list               (const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

/**
 * @defgroup Evas_Object_Group Generic Object Functions
 *
 * Functions that manipulate generic Evas objects.
 *
 * All Evas displaying units are Evas objects. One handles them all by
 * means of the handle ::Evas_Object. Besides Evas treats their
 * objects equally, they have @b types, which define their specific
 * behavior (and individual API).
 *
 * Evas comes with a set of built-in object types:
 *   - rectangle,
 *   - line,
 *   - polygon,
 *   - text,
 *   - textblock and
 *   - image.
 *
 * These functions apply to @b any Evas object, whichever type thay
 * may have.
 *
 * @note The built-in types which are most used are rectangles, text
 * and images. In fact, with these ones one can create 2D interfaces
 * of arbitrary complexity and EFL makes it easy.
 */

/**
 * @defgroup Evas_Object_Group_Basic Basic Object Manipulation
 *
 * Methods that are broadly used, like those that change the color,
 * clippers and geometry of an Evas object.
 *
 * @ingroup Evas_Object_Group
 */

/**
 * @addtogroup Evas_Object_Group_Basic
 * @{
 */

/**
 * Clip one object to another.
 *
 * @param obj The object to be clipped
 * @param clip The object to clip @p obj by
 *
 * This function will clip the object @p obj to the area occupied by
 * the object @p clip. This means the object @p obj will only be
 * visible within the area occupied by the clipping object (@p clip).
 *
 * The color of the object being clipped will be multiplied by the
 * color of the clipping one, so the resulting color for the former
 * will be <code>RESULT = (OBJ * CLIP) / (255 * 255)</code>, per color
 * element (red, green, blue and alpha).
 *
 * Clipping is recursive, so clipping objects may be clipped by
 * others, and their color will in term be multiplied. You may @b not
 * set up circular clipping lists (i.e. object 1 clips object 2, which
 * clips object 1): the behavior of Evas is undefined in this case.
 *
 * Objects which do not clip others are visible in the canvas as
 * normal; <b>those that clip one or more objects become invisible
 * themselves</b>, only affecting what they clip. If an object ceases
 * to have other objects being clipped by it, it will become visible
 * again.
 *
 * The visibility of an object affects the objects that are clipped by
 * it, so if the object clipping others is not shown (as in
 * evas_object_show()), the objects clipped by it will not be shown
 * either.
 *
 * If @p obj was being clipped by another object when this function is
 * called, it gets implicitly removed from the old clipper's domain
 * and is made now to be clipped by its new clipper.
 *
 * @note At the moment the <b>only objects that can validly be used to
 * clip other objects are rectangle objects</b>. All other object
 * types are invalid and the result of using them is undefined. The
 * clip object @p clip must be a valid object, but can also be @c
 * NULL, in which case the effect of this function is the same as
 * calling evas_object_clip_unset() on the @p obj object.
 *
 * Example:
 * @dontinclude evas-object-manipulation.c
 * @skip solid white clipper (note that it's the default color for a
 * @until evas_object_show(d.clipper);
 *
 * See the full @ref Example_Evas_Object_Manipulation "example".
 */
EAPI void              evas_object_clip_set              (Evas_Object *obj, Evas_Object *clip) EINA_ARG_NONNULL(1, 2);

/**
 * Get the object clipping @p obj (if any).
 *
 * @param obj The object to get the clipper from
 *
 * This function returns the object clipping @p obj. If @p obj is
 * not being clipped at all, @c NULL is returned. The object @p obj
 * must be a valid ::Evas_Object.
 *
 * See also evas_object_clip_set(), evas_object_clip_unset() and
 * evas_object_clipees_get().
 *
 * Example:
 * @dontinclude evas-object-manipulation.c
 * @skip if (evas_object_clip_get(d.img) == d.clipper)
 * @until return
 *
 * See the full @ref Example_Evas_Object_Manipulation "example".
 */
EAPI Evas_Object      *evas_object_clip_get              (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

/**
 * Disable/cease clipping on a clipped @p obj object.
 *
 * @param obj The object to cease clipping on
 *
 * This function disables clipping for the object @p obj, if it was
 * already clipped, i.e., its visibility and color get detached from
 * the previous clipper. If it wasn't, this has no effect. The object
 * @p obj must be a valid ::Evas_Object.
 *
 * See also evas_object_clip_set() (for an example),
 * evas_object_clipees_get() and evas_object_clip_get().
 *
 */
EAPI void              evas_object_clip_unset            (Evas_Object *obj);

/**
 * Return a list of objects currently clipped by @p obj.
 *
 * @param obj The object to get a list of clippees from
 * @return a list of objects being clipped by @p obj
 *
 * This returns the internal list handle that contains all objects
 * clipped by the object @p obj. If none are clipped by it, the call
 * returns @c NULL. This list is only valid until the clip list is
 * changed and should be fetched again with another call to
 * evas_object_clipees_get() if any objects being clipped by this
 * object are unclipped, clipped by a new object, deleted or get the
 * clipper deleted. These operations will invalidate the list
 * returned, so it should not be used anymore after that point. Any
 * use of the list after this may have undefined results, possibly
 * leading to crashes. The object @p obj must be a valid
 * ::Evas_Object.
 *
 * See also evas_object_clip_set(), evas_object_clip_unset() and
 * evas_object_clip_get().
 *
 * Example:
 * @code
 * extern Evas_Object *obj;
 * Evas_Object *clipper;
 *
 * clipper = evas_object_clip_get(obj);
 * if (clipper)
 *   {
 *     Eina_List *clippees, *l;
 *     Evas_Object *obj_tmp;
 *
 *     clippees = evas_object_clipees_get(clipper);
 *     printf("Clipper clips %i objects\n", eina_list_count(clippees));
 *     EINA_LIST_FOREACH(clippees, l, obj_tmp)
 *         evas_object_show(obj_tmp);
 *   }
 * @endcode
 */
EAPI const Eina_List  *evas_object_clipees_get           (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;


/**
 * Sets focus to the given object.
 *
 * @param obj The object to be focused or unfocused.
 * @param focus set or remove focus to the object.
 *
 * Changing focus only affects where key events go.  There can be only
 * one object focused at any time.  <p> If the parameter (@p focus) is
 * set, the passed object will be set as the currently focused object.
 * It will receive all keyboard events that are not exclusive key
 * grabs on other objects.
 *
 * @see evas_object_focus_get
 * @see evas_focus_get
 * @see evas_object_key_grab
 * @see evas_object_key_ungrab
 */
EAPI void              evas_object_focus_set             (Evas_Object *obj, Eina_Bool focus) EINA_ARG_NONNULL(1);

/**
 * Test if the object has focus.
 *
 * @param obj The object to be tested.
 *
 * If the passed object is the currently focused object 1 is returned,
 * 0 otherwise.
 *
 * @see evas_object_focus_set
 * @see evas_focus_get
 * @see evas_object_key_grab
 * @see evas_object_key_ungrab
 *
 * @return 1 if the object has the focus, 0 otherwise.
 */
EAPI Eina_Bool         evas_object_focus_get             (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;


/**
 * Sets the layer of the evas that the given object will be part of.
 *
 * It is not possible to change the layer of a smart object's child.
 *
 * @param   obj The given Evas object.
 * @param   l   The number of the layer to place the object on.
 *          Between #EVAS_LAYER_MIN and #EVAS_LAYER_MAX.
 */
EAPI void              evas_object_layer_set             (Evas_Object *obj, short l) EINA_ARG_NONNULL(1);

/**
 * Retrieves the layer of the evas that the given object is part of.
 *
 * Be careful, it doesn't make sense to change the layer of smart object's
 * child. So the returned value could be wrong in some case. Don't rely on
 * it's accuracy.
 *
 * @param   obj The given Evas object.
 * @return  Number of the layer.
 */
EAPI short             evas_object_layer_get             (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;


/**
 * Sets the name of the given Evas object to the given name.
 *
 * @param   obj  The given object.
 * @param   name The given name.
 *
 * There might be ocasions where one would like to name his/her
 * objects.
 *
 * Example:
 * @dontinclude evas-events.c
 * @skip d.bg = evas_object_rectangle_add(d.canvas);
 * @until evas_object_name_set(d.bg, "our dear rectangle");
 *
 * See the full @ref Example_Evas_Events "example".
 */
EAPI void              evas_object_name_set              (Evas_Object *obj, const char *name) EINA_ARG_NONNULL(1);

/**
 * Retrieves the name of the given Evas object.
 *
 * @param   obj The given object.
 * @return  The name of the object or @c NULL, if no name has been given
 *          to it.
 *
 * Example:
 * @dontinclude evas-events.c
 * @skip fprintf(stdout, "An object got focused: %s\n",
 * @until evas_focus_get
 *
 * See the full @ref Example_Evas_Events "example".
 */
EAPI const char       *evas_object_name_get              (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;


/**
 * Increments object reference count to defer its deletion.
 *
 * @param obj The given Evas object to reference
 *
 * This increments the reference count of an object, which if greater
 * than 0 will defer deletion by evas_object_del() until all
 * references are released back (counter back to 0). References cannot
 * go below 0 and unreferencing past that will result in the reference
 * count being limited to 0. References are limited to <c>2^32 - 1</c>
 * for an object. Referencing it more than this will result in it
 * being limited to this value.
 *
 * @see evas_object_unref()
 * @see evas_object_del()
 *
 * @note This is a <b>very simple<b> reference counting mechanism! For
 * instance, Evas is not ready to check for pending references on a
 * canvas deletion, or things like that. This is useful on scenarios
 * where, inside a code block, callbacks exist which would possibly
 * delete an object we are operating on afterwards. Then, one would
 * evas_object_ref() it on the beginning of the block and
 * evas_object_unref() it on the end. I would then be deleted at this
 * point, if it should be.
 *
 * Example:
 * @code
 *  evas_object_ref(obj);
 *
 *  // action here...
 *  evas_object_smart_callback_call(obj, SIG_SELECTED, NULL);
 *  // more action here...
 *  evas_object_unref(obj);
 * @endcode
 *
 * @ingroup Evas_Object_Group_Basic
 * @since 1.1.0
 */
EAPI void              evas_object_ref                   (Evas_Object *obj);

/**
 * Decrements object reference count.
 *
 * @param obj The given Evas object to unreference
 *
 * This decrements the reference count of an object. If the object has
 * had evas_object_del() called on it while references were more than
 * 0, it will be deleted at the time this function is called and puts
 * the counter back to 0. See evas_object_ref() for more information.
 *
 * @see evas_object_ref() (for an example)
 * @see evas_object_del()
 *
 * @ingroup Evas_Object_Group_Basic
 * @since 1.1.0
 */
EAPI void              evas_object_unref                 (Evas_Object *obj);


/**
 * Marks the given Evas object for deletion (when Evas will free its
 * memory).
 *
 * @param obj The given Evas object.
 *
 * This call will mark @p obj for deletion, which will take place
 * whenever it has no more references to it (see evas_object_ref() and
 * evas_object_unref()).
 *
 * At actual deletion time, which may or may not be just after this
 * call, ::EVAS_CALLBACK_DEL and ::EVAS_CALLBACK_FREE callbacks will
 * be called. If the object currently had the focus, its
 * ::EVAS_CALLBACK_FOCUS_OUT callback will also be called.
 *
 * @see evas_object_ref()
 * @see evas_object_unref()
 *
 * @ingroup Evas_Object_Group_Basic
 */
EAPI void              evas_object_del                   (Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * Move the given Evas object to the given location inside its
 * canvas' viewport.
 *
 * @param obj The given Evas object.
 * @param x   X position to move the object to, in canvas units.
 * @param y   Y position to move the object to, in canvas units.
 *
 * Besides being moved, the object's ::EVAS_CALLBACK_MOVE callback
 * will be called.
 *
 * @note Naturally, newly created objects are placed at the canvas'
 * origin: <code>0, 0</code>.
 *
 * Example:
 * @dontinclude evas-object-manipulation.c
 * @skip evas_object_image_border_set(d.clipper_border, 3, 3, 3, 3);
 * @until evas_object_show
 *
 * See the full @ref Example_Evas_Object_Manipulation "example".
 *
 * @ingroup Evas_Object_Group_Basic
 */
EAPI void              evas_object_move                  (Evas_Object *obj, Evas_Coord x, Evas_Coord y) EINA_ARG_NONNULL(1);

/**
 * Changes the size of the given Evas object.
 *
 * @param obj The given Evas object.
 * @param w   The new width of the Evas object.
 * @param h   The new height of the Evas object.
 *
 * Besides being resized, the object's ::EVAS_CALLBACK_RESIZE callback
 * will be called.
 *
 * @note Newly created objects have zeroed dimensions. Then, you most
 * probably want to use evas_object_resize() on them after they are
 * created.
 *
 * @note Be aware that resizing an object changes its drawing area,
 * but that does imply the object is rescaled! For instance, images
 * are filled inside their drawing area using the specifications of
 * evas_object_image_fill_set(). Thus to scale the image to match
 * exactly your drawing area, you need to change the
 * evas_object_image_fill_set() as well.
 *
 * @note This is more evident in images, but text, textblock, lines
 * and polygons will behave similarly. Check their specific APIs to
 * know how to achieve your desired behavior. Consider the following
 * example:
 *
 * @code
 * // rescale image to fill exactly its area without tiling:
 * evas_object_resize(img, w, h);
 * evas_object_image_fill_set(img, 0, 0, w, h);
 * @endcode
 *
 * @ingroup Evas_Object_Group_Basic
 */
EAPI void              evas_object_resize                (Evas_Object *obj, Evas_Coord w, Evas_Coord h) EINA_ARG_NONNULL(1);

/**
 * Retrieves the position and (rectangular) size of the given Evas
 * object.
 *
 * @param obj The given Evas object.
 * @param x Pointer to an integer in which to store the X coordinate
 *          of the object.
 * @param y Pointer to an integer in which to store the Y coordinate
 *          of the object.
 * @param w Pointer to an integer in which to store the width of the
 *          object.
 * @param h Pointer to an integer in which to store the height of the
 *          object.
 *
 * The position, naturally, will be relative to the top left corner of
 * the canvas' viewport.
 *
 * @note Use @c NULL pointers on the geometry components you're not
 * interested in: they'll be ignored by the function.
 *
 * Example:
 * @dontinclude evas-events.c
 * @skip int w, h, cw, ch;
 * @until return
 *
 * See the full @ref Example_Evas_Events "example".
 *
 * @ingroup Evas_Object_Group_Basic
 */
EAPI void              evas_object_geometry_get          (const Evas_Object *obj, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h) EINA_ARG_NONNULL(1);


/**
 * Makes the given Evas object visible.
 *
 * @param obj The given Evas object.
 *
 * Besides becoming visible, the object's ::EVAS_CALLBACK_SHOW
 * callback will be called.
 *
 * @see evas_object_hide() for more on object visibility.
 * @see evas_object_visible_get()
 *
 * @ingroup Evas_Object_Group_Basic
 */
EAPI void              evas_object_show                  (Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * Makes the given Evas object invisible.
 *
 * @param obj The given Evas object.
 *
 * Hidden objects, besides not being shown at all in your canvas,
 * won't be checked for changes on the canvas rendering
 * process. Furthermore, they will not catch input events. Thus, they
 * are much ligher (in processing needs) than an object that is
 * invisible due to indirect causes, such as being clipped or out of
 * the canvas' viewport.
 *
 * Besides becoming hidden, @p obj object's ::EVAS_CALLBACK_SHOW
 * callback will be called.
 *
 * @note All objects are created in the hidden state! If you want them
 * shown, use evas_object_show() after their creation.
 *
 * @see evas_object_show()
 * @see evas_object_visible_get()
 *
 * Example:
 * @dontinclude evas-object-manipulation.c
 * @skip if (evas_object_visible_get(d.clipper))
 * @until return
 *
 * See the full @ref Example_Evas_Object_Manipulation "example".
 *
 * @ingroup Evas_Object_Group_Basic
 */
EAPI void              evas_object_hide                  (Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * Retrieves whether or not the given Evas object is visible.
 *
 * @param   obj The given Evas object.
 * @return @c EINA_TRUE if the object is visible, @c EINA_FALSE
 * otherwise.
 *
 * This retrieves an object's visibily as the one enforced by
 * evas_object_show() and evas_object_hide().
 *
 * @note The value returned isn't, by any means, influenced by
 * clippers covering @obj, it being out of its canvas' viewport or
 * stacked below other object.
 *
 * @see evas_object_show()
 * @see evas_object_hide() (for an example)
 *
 * @ingroup Evas_Object_Group_Basic
 */
EAPI Eina_Bool         evas_object_visible_get           (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;


/**
 * Sets the general/main color of the given Evas object to the given
 * one.
 *
 * @param obj The given Evas object.
 * @param r   The red component of the given color.
 * @param g   The green component of the given color.
 * @param b   The blue component of the given color.
 * @param a   The alpha component of the given color.
 *
 * @see evas_object_color_get() (for an example)
 *
 * @ingroup Evas_Object_Group_Basic
 */
EAPI void              evas_object_color_set             (Evas_Object *obj, int r, int g, int b, int a) EINA_ARG_NONNULL(1);

/**
 * Retrieves the general/main color of the given Evas object.
 *
 * @param obj The given Evas object to retrieve color from.
 * @param r Pointer to an integer in which to store the red component
 *          of the color.
 * @param g Pointer to an integer in which to store the green
 *          component of the color.
 * @param b Pointer to an integer in which to store the blue component
 *          of the color.
 * @param a Pointer to an integer in which to store the alpha
 *          component of the color.
 *
 * Retrieves the “main” color's RGB component (and alpha channel)
 * values, <b>which range from 0 to 255</b>. For the alpha channel,
 * which defines the object's transparency level, the former value
 * means totally trasparent, while the latter means opaque.
 *
 * Usually you’ll use this attribute for text and rectangle objects,
 * where the “main” color is their unique one. If set for objects
 * which themselves have colors, like the images one, those colors get
 * modulated by this one.
 *
 * @note All newly created Evas rectangles get the default color
 * values of <code>255 255 255 255</code> (opaque white).
 *
 * @note Use @c NULL pointers on the components you're not interested
 * in: they'll be ignored by the function.
 *
 * Example:
 * @dontinclude evas-object-manipulation.c
 * @skip int alpha, r, g, b;
 * @until return
 *
 * See the full @ref Example_Evas_Object_Manipulation "example".
 *
 * @ingroup Evas_Object_Group_Basic
 */
EAPI void              evas_object_color_get             (const Evas_Object *obj, int *r, int *g, int *b, int *a) EINA_ARG_NONNULL(1);


/**
 * Retrieves the Evas canvas that the given object lives on.
 *
 * @param   obj The given Evas object.
 * @return  A pointer to the canvas where the object is on.
 *
 * This function is most useful at code contexts where you need to
 * operate on the canvas but have only the object pointer.
 *
 * @ingroup Evas_Object_Group_Basic
 */
EAPI Evas             *evas_object_evas_get              (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

/**
 * Retrieves the type of the given Evas object.
 *
 * @param obj The given object.
 * @return The type of the object.
 *
 * For Evas' builtin types, the return strings will be one of:
 *   - <c>"rectangle"</c>,
 *   - <c>"line"</c>,
 *   - <c>"polygon"</c>,
 *   - <c>"text"</c>,
 *   - <c>"textblock"</c> and
 *   - <c>"image"</c>.
 *
 * For Evas smart objects (see @ref Evas_Smart_Group), the name of the
 * smart class itself is returned on this call. For the built-in smart
 * objects, these names are:
 *   - <c>"EvasObjectSmartClipped"</c>, for the clipped smart object
 *   - <c>"Evas_Object_Box"</c>, for the box object and
 *   - <c>"Evas_Object_Table"</c>, for the table object.
 *
 * Example:
 * @dontinclude evas-object-manipulation.c
 * @skip d.img = evas_object_image_filled_add(d.canvas);
 * @until border on the
 *
 * See the full @ref Example_Evas_Object_Manipulation "example".
 */
EAPI const char       *evas_object_type_get              (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

/**
 * Raise @p obj to the top of its layer.
 *
 * @param obj the object to raise
 */
EAPI void              evas_object_raise                 (Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * Lower @p obj to the bottom of its layer.
 *
 * @param obj the object to lower
 */
EAPI void              evas_object_lower                 (Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * Stack @p obj immediately above @p above
 *
 * If @p obj is a member of a smart object, then @p above must also be
 * a member of the same smart object.
 *
 * Similarly, if @p obj is not a member of smart object, @p above may
 * not either.
 *
 * @param obj the object to stack
 * @param above the object above which to stack
 */
EAPI void              evas_object_stack_above           (Evas_Object *obj, Evas_Object *above) EINA_ARG_NONNULL(1, 2);

/**
 * Stack @p obj immediately below @p below
 *
 * If @p obj is a member of a smart object, then @p below must also be
 * a member of the same smart object.
 *
 * Similarly, if @p obj is not a member of smart object, @p below may
 * not either.
 *
 * @param obj the object to stack
 * @param below the object below which to stack
 */
EAPI void              evas_object_stack_below           (Evas_Object *obj, Evas_Object *below) EINA_ARG_NONNULL(1, 2);

/**
 * Get the Evas object above @p obj
 *
 * @param obj an Evas_Object
 * @return the Evas_Object directly above
 */
EAPI Evas_Object      *evas_object_above_get             (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

/**
 * Get the Evas object below @p obj
 *
 * @param obj an Evas_Object
 * @return the Evas_Object directly below
 */
EAPI Evas_Object      *evas_object_below_get             (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

/**
 * @}
 */

/**
 * @defgroup Evas_Object_Group_Events Object Events
 *
 * Objects generates events when they are moved, resized, when their
 * visibility change, when they are deleted and so on. These methods
 * will allow one to handle such events.
 *
 * The events can be those from keyboard and mouse, if the object
 * accepts these events.
 *
 * @ingroup Evas_Object_Group
 */

/**
 * @addtogroup Evas_Object_Group_Events
 * @{
 */

/**
 * Add a callback function to an object
 *
 * @param obj Object to attach a callback to
 * @param type The type of event that will trigger the callback
 * @param func The function to be called when the event is triggered
 * @param data The data pointer to be passed to @p func
 *
 * This function adds a function callback to an object when the event
 * of type @p type occurs on object @p obj. The function is @p func.
 *
 * In the event of a memory allocation error during addition of the
 * callback to the object, evas_alloc_error() should be used to
 * determine the nature of the error, if any, and the program should
 * sensibly try and recover.
 *
 * The function will be passed the pointer @p data when it is
 * called. A callback function must look like this:
 *
 * @code
 * void callback (void *data, Evas *e, Evas_Object *obj, void *event_info);
 * @endcode
 *
 * The first parameter @p data in this function will be the same value
 * passed to evas_object_event_callback_add() as the @p data
 * parameter. The second parameter is a convenience for the programmer
 * to know what evas canvas the event occurred on. The third parameter
 * @p obj is the Object handle on which the event occurred. The foruth
 * parameter @p event_info is a pointer to a data structure that may
 * or may not be passed to the callback, depending on the event type
 * that triggered the callback.
 *
 * The event type @p type to trigger the function may be one of
 * #EVAS_CALLBACK_MOUSE_IN, #EVAS_CALLBACK_MOUSE_OUT,
 * #EVAS_CALLBACK_MOUSE_DOWN, #EVAS_CALLBACK_MOUSE_UP,
 * #EVAS_CALLBACK_MOUSE_MOVE, #EVAS_CALLBACK_MOUSE_WHEEL,
 * #EVAS_CALLBACK_FREE, #EVAS_CALLBACK_KEY_DOWN, #EVAS_CALLBACK_KEY_UP,
 * #EVAS_CALLBACK_FOCUS_IN, #EVAS_CALLBACK_FOCUS_OUT,
 * #EVAS_CALLBACK_SHOW, #EVAS_CALLBACK_HIDE, #EVAS_CALLBACK_MOVE,
 * #EVAS_CALLBACK_RESIZE or #EVAS_CALLBACK_RESTACK.
 * This determines the kind of event that will trigger the callback to
 * be called.  The @p event_info pointer passed to the callback will
 * be one of the following, depending on the event triggering it:
 *
 * #EVAS_CALLBACK_MOUSE_IN: event_info = pointer to Evas_Event_Mouse_In
 *
 * This event is triggered when the mouse pointer enters the region of
 * the object @p obj. This may occur by the mouse pointer being moved
 * by evas_event_feed_mouse_move() or
 * evas_event_feed_mouse_move_data() calls, or by the object being
 * shown, raised, moved, resized, or other objects being moved out of
 * the way, hidden, lowered or moved out of the way.
 *
 * #EVAS_CALLBACK_MOUSE_OUT: event_info = pointer to Evas_Event_Mouse_Out
 *
 * This event is triggered exactly like #EVAS_CALLBACK_MOUSE_IN is, but
 * occurs when the mouse pointer exits an object. Note that no out
 * events will be reported if the mouse pointer is implicitly grabbed
 * to an object (the mouse buttons are down at all and any were
 * pressed on that object). An out event will be reported as soon as
 * the mouse is no longer grabbed (no mouse buttons are
 * depressed). Out events will be reported once all buttons are
 * released, if the mouse has left the object.
 *
 * #EVAS_CALLBACK_MOUSE_DOWN: event_info = pointer to
 * Evas_Event_Mouse_Down
 *
 * This event is triggered by a mouse button being depressed while
 * over an object. If pointermode is EVAS_OBJECT_POINTER_MODE_AUTOGRAB
 * (default) this causes this object to passively grab the mouse until
 * all mouse buttons have been released.  That means if this mouse
 * button is the first to be pressed, all future mouse events will be
 * reported to only this object until no buttons are down. That
 * includes mouse move events, in and out events, and further button
 * presses. When all buttons are released, event propagation occurs as
 * normal.
 *
 * #EVAS_CALLBACK_MOUSE_UP: event_info = pointer to Evas_Event_Mouse_Up
 *
 * This event is triggered by a mouse button being released while over
 * an object or when passively grabbed to an object. If this is the
 * last mouse button to be raised on an object then the passive grab
 * is released and event processing will continue as normal.
 *
 * #EVAS_CALLBACK_MOUSE_MOVE: event_info = pointer to Evas_Event_Mouse_Move
 *
 * This event is triggered by the mouse pointer moving while over an
 * object or passively grabbed to an object.
 *
 * #EVAS_CALLBACK_MOUSE_WHEEL: event_info = pointer to
 * Evas_Event_Mouse_Wheel
 *
 * This event is triggered by the mouse wheel being rolled while over
 * an object or passively grabbed to an object.
 *
 * #EVAS_CALLBACK_FREE: event_info = NULL
 *
 * This event is triggered just before Evas is about to free all
 * memory used by an object and remove all references to it. This is
 * useful for programs to use if they attached data to an object and
 * want to free it when the object is deleted. The object is still
 * valid when this callback is called, but after this callback
 * returns, there is no guarantee on the object's validity.
 *
 * #EVAS_CALLBACK_KEY_DOWN: event_info = pointer to Evas_Event_Key_Down
 *
 * This callback is called when a key is pressed and the focus is on
 * the object, or a key has been grabbed to a particular object which
 * wants to intercept the key press regardless of what object has the
 * focus.
 *
 * #EVAS_CALLBACK_KEY_UP: event_info = pointer to Evas_Event_Key_Up
 *
 * This callback is called when a key is released and the focus is on
 * the object, or a key has been grabbed to a particular object which
 * wants to intercept the key release regardless of what object has
 * the focus.
 *
 * #EVAS_CALLBACK_FOCUS_IN: event_info = NULL
 *
 * This event is called when an object gains the focus. When the
 * callback is called the object has already gained the focus.
 *
 * #EVAS_CALLBACK_FOCUS_OUT: event_info = NULL
 *
 * This event is triggered by an object losing the focus. When the
 * callback is called the object has already lost the focus.
 *
 * #EVAS_CALLBACK_SHOW: event_info = NULL
 *
 * This event is triggered by the object being shown by
 * evas_object_show().
 *
 * #EVAS_CALLBACK_HIDE: event_info = NULL
 *
 * This event is triggered by an object being hidden by
 * evas_object_hide().
 *
 * #EVAS_CALLBACK_MOVE: event_info = NULL
 *
 * This event is triggered by an object being
 * moved. evas_object_move() can trigger this, as can any
 * object-specific manipulations that would mean the object's origin
 * could move.
 *
 * #EVAS_CALLBACK_RESIZE: event_info = NULL
 *
 * This event is triggered by an object being resized. Resizes can be
 * triggered by evas_object_resize() or by any object-specific calls
 * that may cause the object to resize.
 *
 * Example:
 * @code
 * extern Evas_Object *object;
 * extern void *my_data;
 * void down_callback(void *data, Evas *e, Evas_Object *obj, void *event_info);
 * void up_callback(void *data, Evas *e, Evas_Object *obj, void *event_info);
 *
 * evas_object_event_callback_add(object, EVAS_CALLBACK_MOUSE_UP, up_callback, my_data);
 * if (evas_alloc_error() != EVAS_ALLOC_ERROR_NONE)
 *   {
 *     fprintf(stderr, "ERROR: Callback registering failed! Abort!\n");
 *     exit(-1);
 *   }
 * evas_object_event_callback_add(object, EVAS_CALLBACK_MOUSE_DOWN, down_callback, my_data);
 * if (evas_alloc_error() != EVAS_ALLOC_ERROR_NONE)
 *   {
 *     fprintf(stderr, "ERROR: Callback registering failed! Abort!\n");
 *     exit(-1);
 *   }
 * @endcode
 */
   EAPI void              evas_object_event_callback_add     (Evas_Object *obj, Evas_Callback_Type type, Evas_Object_Event_Cb func, const void *data) EINA_ARG_NONNULL(1, 3);

/**
 * Delete a callback function from an object
 *
 * @param obj Object to remove a callback from
 * @param type The type of event that was triggering the callback
 * @param func The function that was to be called when the event was triggered
 * @return The data pointer that was to be passed to the callback
 *
 * This function removes the most recently added callback from the
 * object @p obj which was triggered by the event type @p type and was
 * calling the function @p func when triggered. If the removal is
 * successful it will also return the data pointer that was passed to
 * evas_object_event_callback_add() when the callback was added to the
 * object. If not successful NULL will be returned.
 *
 * Example:
 * @code
 * extern Evas_Object *object;
 * void *my_data;
 * void up_callback(void *data, Evas *e, Evas_Object *obj, void *event_info);
 *
 * my_data = evas_object_event_callback_del(object, EVAS_CALLBACK_MOUSE_UP, up_callback);
 * @endcode
 */
EAPI void             *evas_object_event_callback_del     (Evas_Object *obj, Evas_Callback_Type type, Evas_Object_Event_Cb func) EINA_ARG_NONNULL(1, 3);

/**
 * Delete a callback function from an object
 *
 * @param obj Object to remove a callback from
 * @param type The type of event that was triggering the callback
 * @param func The function that was to be called when the event was triggered
 * @param data The data pointer that was to be passed to the callback
 * @return The data pointer that was to be passed to the callback
 *
 * This function removes the most recently added callback from the
 * object @p obj which was triggered by the event type @p type and was
 * calling the function @p func with data @p data when triggered. If
 * the removal is successful it will also return the data pointer that
 * was passed to evas_object_event_callback_add() (that will be the
 * same as the parameter) when the callback was added to the
 * object. If not successful NULL will be returned.
 *
 * Example:
 * @code
 * extern Evas_Object *object;
 * void *my_data;
 * void up_callback(void *data, Evas *e, Evas_Object *obj, void *event_info);
 *
 * my_data = evas_object_event_callback_del_full(object, EVAS_CALLBACK_MOUSE_UP, up_callback, data);
 * @endcode
 */
EAPI void             *evas_object_event_callback_del_full(Evas_Object *obj, Evas_Callback_Type type, Evas_Object_Event_Cb func, const void *data) EINA_ARG_NONNULL(1, 3);


/**
 * Set an object's pass events state.
 * @param obj the Evas object
 * @param pass whether to pass events or not
 *
 * If @p pass is true, this will cause events on @p obj to be ignored.
 * They will be triggered on the next lower object (that is not set to
 * pass events) instead.
 *
 * If @p pass is false, events will be processed as normal.
 */
EAPI void              evas_object_pass_events_set        (Evas_Object *obj, Eina_Bool pass) EINA_ARG_NONNULL(1);

/**
 * Determine whether an object is set to pass events.
 * @param obj
 * @return pass events state
 */
EAPI Eina_Bool         evas_object_pass_events_get        (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

/**
 * Set an object's repeat events state.
 * @param obj the object
 * @param repeat wheter to repeat events or not
 *
 * If @p repeat is true, this will cause events on @p obj to trigger
 * callbacks, but also to be repeated on the next lower object in the
 * stack.
 *
 * If @p repeat is false, events occurring on @p obj will be processed
 * normally.
 */
EAPI void              evas_object_repeat_events_set      (Evas_Object *obj, Eina_Bool repeat) EINA_ARG_NONNULL(1);

/**
 * Determine whether an object is set to repeat events.
 * @param obj
 * @return repeat events state
 */
EAPI Eina_Bool         evas_object_repeat_events_get      (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

/**
 * Set whether events on a smart member object should propagate to its
 * parent.
 *
 * @param obj the smart member object
 * @param prop wheter to propagate events or not
 *
 * This function has no effect if @p obj is not a member of a smart
 * object.
 *
 * If @p prop is true, events occurring on this object will propagate on
 * to the smart object of which @p obj is a member.
 *
 * If @p prop is false, events for which callbacks are set on the member
 * object, @p obj, will not be passed on to the parent smart object.
 *
 * The default value is true.
 */
EAPI void              evas_object_propagate_events_set   (Evas_Object *obj, Eina_Bool prop) EINA_ARG_NONNULL(1);

/**
 * Determine whether an object is set to propagate events.
 * @param obj
 * @return propagate events state
 */
EAPI Eina_Bool         evas_object_propagate_events_get   (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

/**
 * @}
 */

/**
 * @defgroup Evas_Object_Group_Map UV Mapping (Rotation, Perspecitve, 3D...)
 *
 * Evas allows different transformations to be applied to all kinds of
 * objects. These are applied by means of UV mapping.
 *
 * With UV mapping, one maps points in the source object to a 3D space
 * positioning at target. This allows rotation, perspective, scale and
 * lots of other effects, depending on the map that is used.
 *
 * Each map point may carry a multiplier color. If properly
 * calculated, these can do shading effects on the object, producing
 * 3D effects.
 *
 * As usual, Evas provides both the raw and easy to use methods. The
 * raw methods allow developer to create its maps somewhere else,
 * maybe load them from some file format. The easy to use methods,
 * calculate the points given some high-level parameters, such as
 * rotation angle, ambient light and so on.
 *
 * @note applying mapping will reduce performance, so use with
 *       care. The impact on performance depends on engine in
 *       use. Software is quite optimized, but not as fast as OpenGL.
 *
 * @ingroup Evas_Object_Group
 */

/**
 * Enable or disable the map that is set.
 *
 * This enables the map that is set or disables it. On enable, the object
 * geometry will be saved, and the new geometry will change (position and
 * size) to reflect the map geometry set. If none is set yet, this may be
 * an undefined geometry, unless you have already set the map with
 * evas_object_map_set(). It is suggested you first set a map with
 * evas_object_map_set() with valid useful coordinates then enable and
 * disable the map with evas_object_map_enable_set() as needed.
 *
 * @param obj object to enable the map on
 * @param enabled enabled state
 */
EAPI void              evas_object_map_enable_set        (Evas_Object *obj, Eina_Bool enabled);

/**
 * Get the map enabled state
 *
 * This returns the currently enabled state of the map on the object indicated.
 * The default map enable state is off. You can enable and disable it with
 * evas_object_map_enable_set().
 *
 * @param obj object to get the map enabled state from
 * @return the map enabled state
 */
EAPI Eina_Bool         evas_object_map_enable_get        (const Evas_Object *obj);

/**
 * Set the map source object
 *
 * This sets the object from which the map is taken - can be any object that
 * has map enabled on it.
 *
 * Currently not implemented. for future use.
 *
 * @param obj object to set the map source of
 * @param src the source object from which the map is taken
 */
EAPI void              evas_object_map_source_set        (Evas_Object *obj, Evas_Object *src);

/**
 * Get the map source object
 *
 * See evas_object_map_source_set()
 *
 * @param obj object to set the map source of
 * @return the object set as the source
 */
EAPI Evas_Object      *evas_object_map_source_get        (const Evas_Object *obj);

/**
 * Set current object transformation map.
 *
 * This sets the map on a given object. It is copied from the @p map pointer,
 * so there is no need to keep the @p map object if you don't need it anymore.
 *
 * A map is a set of 4 points which have canvas x, y coordinates per point,
 * with an optional z point value as a hint for perspective correction, if it
 * is available. As well each point has u and v coordinates. These are like
 * "texture coordinates" in OpenGL in that they define a point in the source
 * image that is mapped to that map vertex/point. The u corresponds to the x
 * coordinate of this mapped point and v, the y coordinate. Note that these
 * coordinates describe a bounding region to sample. If you have a 200x100
 * source image and want to display it at 200x100 with proper pixel
 * precision, then do:
 *
 * @code
 * Evas_Map *m = evas_map_new(4);
 * evas_map_point_coord_set(m, 0,   0,   0, 0);
 * evas_map_point_coord_set(m, 1, 200,   0, 0);
 * evas_map_point_coord_set(m, 2, 200, 100, 0);
 * evas_map_point_coord_set(m, 3,   0, 100, 0);
 * evas_map_point_image_uv_set(m, 0,   0,   0);
 * evas_map_point_image_uv_set(m, 1, 200,   0);
 * evas_map_point_image_uv_set(m, 2, 200, 100);
 * evas_map_point_image_uv_set(m, 3,   0, 100);
 * evas_object_map_set(obj, m);
 * evas_map_free(m);
 * @endcode
 *
 * Note that the map points a uv coordinates match the image geometry. If
 * the @p map parameter is NULL, the stored map will be freed and geometry
 * prior to enabling/setting a map will be restored.
 *
 * @param obj object to change transformation map
 * @param map new map to use
 *
 * @see evas_map_new()
 */
EAPI void              evas_object_map_set               (Evas_Object *obj, const Evas_Map *map);

/**
 * Get current object transformation map.
 *
 * This returns the current internal map set on the indicated object. It is
 * intended for read-only acces and is only valid as long as the object is
 * not deleted or the map on the object is not changed. If you wish to modify
 * the map and set it back do the following:
 *
 * @code
 * const Evas_Map *m = evas_object_map_get(obj);
 * Evas_Map *m2 = evas_map_dup(m);
 * evas_map_util_rotate(m2, 30.0, 0, 0);
 * evas_object_map_set(obj);
 * evas_map_free(m2);
 * @endcode
 *
 * @param obj object to query transformation map.
 * @return map reference to map in use. This is an internal data structure, so
 * do not modify it.
 *
 * @see evas_object_map_set()
 */
EAPI const Evas_Map   *evas_object_map_get               (const Evas_Object *obj);


/**
 * Populate source and destination map points to match exactly object.
 *
 * Usually one initialize map of an object to match it's original
 * position and size, then transform these with evas_map_util_*
 * functions, such as evas_map_util_rotate() or
 * evas_map_util_3d_rotate(). The original set is done by this
 * function, avoiding code duplication all around.
 *
 * @param m map to change all 4 points (must be of size 4).
 * @param obj object to use unmapped geometry to populate map coordinates.
 * @param z Point Z Coordinate hint (pre-perspective transform). This value
 *        will be used for all four points.
 *
 * @see evas_map_util_points_populate_from_object()
 * @see evas_map_point_coord_set()
 * @see evas_map_point_image_uv_set()
 */
EAPI void              evas_map_util_points_populate_from_object_full(Evas_Map *m, const Evas_Object *obj, Evas_Coord z);

/**
 * Populate source and destination map points to match exactly object.
 *
 * Usually one initialize map of an object to match it's original
 * position and size, then transform these with evas_map_util_*
 * functions, such as evas_map_util_rotate() or
 * evas_map_util_3d_rotate(). The original set is done by this
 * function, avoiding code duplication all around.
 *
 * Z Point coordinate is assumed as 0 (zero).
 *
 * @param m map to change all 4 points (must be of size 4).
 * @param obj object to use unmapped geometry to populate map coordinates.
 *
 * @see evas_map_util_points_populate_from_object_full()
 * @see evas_map_util_points_populate_from_geometry()
 * @see evas_map_point_coord_set()
 * @see evas_map_point_image_uv_set()
 */
EAPI void              evas_map_util_points_populate_from_object     (Evas_Map *m, const Evas_Object *obj);

/**
 * Populate source and destination map points to match given geometry.
 *
 * Similar to evas_map_util_points_populate_from_object_full(), this
 * call takes raw values instead of querying object's unmapped
 * geometry. The given width will be used to calculate destination
 * points (evas_map_point_coord_set()) and set the image uv
 * (evas_map_point_image_uv_set()).
 *
 * @param m map to change all 4 points (must be of size 4).
 * @param x Point X Coordinate
 * @param y Point Y Coordinate
 * @param w width to use to calculate second and third points.
 * @param h height to use to calculate third and fourth points.
 * @param z Point Z Coordinate hint (pre-perspective transform). This value
 *        will be used for all four points.
 *
 * @see evas_map_util_points_populate_from_object()
 * @see evas_map_point_coord_set()
 * @see evas_map_point_image_uv_set()
 */
EAPI void              evas_map_util_points_populate_from_geometry   (Evas_Map *m, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h, Evas_Coord z);

/**
 * Set color of all points to given color.
 *
 * This call is useful to reuse maps after they had 3d lightning or
 * any other colorization applied before.
 *
 * @param m map to change the color of.
 * @param r red (0 - 255)
 * @param g green (0 - 255)
 * @param b blue (0 - 255)
 * @param a alpha (0 - 255)
 *
 * @see evas_map_point_color_set()
 */
EAPI void              evas_map_util_points_color_set                (Evas_Map *m, int r, int g, int b, int a);

/**
 * Change the map to apply the given rotation.
 *
 * This rotates the indicated map's coordinates around the center coordinate
 * given by @p cx and @p cy as the rotation center. The points will have their
 * X and Y coordinates rotated clockwise by @p degrees degress (360.0 is a
 * full rotation). Negative values for degrees will rotate counter-clockwise
 * by that amount. All coordinates are canvas global coordinates.
 *
 * @param m map to change.
 * @param degrees amount of degrees from 0.0 to 360.0 to rotate.
 * @param cx rotation's center horizontal position.
 * @param cy rotation's center vertical position.
 *
 * @see evas_map_point_coord_set()
 * @see evas_map_util_zoom()
 */
EAPI void              evas_map_util_rotate                          (Evas_Map *m, double degrees, Evas_Coord cx, Evas_Coord cy);

/**
 * Change the map to apply the given zooming.
 *
 * Like evas_map_util_rotate(), this zooms the points of the map from a center
 * point. That center is defined by @p cx and @p cy. The @p zoomx and @p zoomy
 * parameters specific how much to zoom in the X and Y direction respectively.
 * A value of 1.0 means "don't zoom". 2.0 means "dobule the size". 0.5 is
 * "half the size" etc. All coordinates are canvas global coordinates.
 *
 * @param m map to change.
 * @param zoomx horizontal zoom to use.
 * @param zoomy vertical zoom to use.
 * @param cx zooming center horizontal position.
 * @param cy zooming center vertical position.
 *
 * @see evas_map_point_coord_set()
 * @see evas_map_util_rotate()
 */
EAPI void              evas_map_util_zoom                            (Evas_Map *m, double zoomx, double zoomy, Evas_Coord cx, Evas_Coord cy);

/**
 * Rotate the map around 3 axes in 3D
 *
 * This will rotate not just around the "Z" axis as in evas_map_util_rotate()
 * (which is a convenience call for those only wanting 2D). This will rotate
 * around the X, Y and Z axes. The Z axis points "into" the screen with low
 * values at the screen and higher values further away. The X axis runs from
 * left to right on the screen and the Y axis from top to bottom. Like with
 * evas_map_util_rotate(0 you provide a center point to rotate around (in 3D).
 *
 * @param m map to change.
 * @param dx amount of degrees from 0.0 to 360.0 to rotate arount X axis.
 * @param dy amount of degrees from 0.0 to 360.0 to rotate arount Y axis.
 * @param dz amount of degrees from 0.0 to 360.0 to rotate arount Z axis.
 * @param cx rotation's center horizontal position.
 * @param cy rotation's center vertical position.
 * @param cz rotation's center vertical position.
 */
EAPI void              evas_map_util_3d_rotate                       (Evas_Map *m, double dx, double dy, double dz, Evas_Coord cx, Evas_Coord cy, Evas_Coord cz);

/**
 * Perform lighting calculations on the given Map
 *
 * This is used to apply lighting calculations (from a single light source)
 * to a given map. The R, G and B values of each vertex will be modified to
 * reflect the lighting based on the lixth point coordinates, the light
 * color and the ambient color, and at what angle the map is facing the
 * light source. A surface should have its points be declared in a
 * clockwise fashion if the face is "facing" towards you (as opposed to
 * away from you) as faces have a "logical" side for lighting.
 *
 * @param m map to change.
 * @param lx X coordinate in space of light point
 * @param ly Y coordinate in space of light point
 * @param lz Z coordinate in space of light point
 * @param lr light red value (0 - 255)
 * @param lg light green value (0 - 255)
 * @param lb light blue value (0 - 255)
 * @param ar ambient color red value (0 - 255)
 * @param ag ambient color green value (0 - 255)
 * @param ab ambient color blue value (0 - 255)
 */
EAPI void              evas_map_util_3d_lighting                     (Evas_Map *m, Evas_Coord lx, Evas_Coord ly, Evas_Coord lz, int lr, int lg, int lb, int ar, int ag, int ab);

/**
 * Apply a perspective transform to the map
 *
 * This applies a given perspective (3D) to the map coordinates. X, Y and Z
 * values are used. The px and py points specify the "infinite distance" point
 * in the 3D conversion (where all lines converge to like when artists draw
 * 3D by hand). The @p z0 value specifis the z value at which there is a 1:1
 * mapping between spatial coorinates and screen coordinates. Any points
 * on this z value will not have their X and Y values modified in the transform.
 * Those further away (Z value higher) will shrink into the distance, and
 * those less than this value will expand and become bigger. The @p foc value
 * determines the "focal length" of the camera. This is in reality the distance
 * between the camera lens plane itself (at or closer than this rendering
 * results are undefined) and the "z0" z value. This allows for some "depth"
 * control and @p foc must be greater than 0.
 *
 * @param m map to change.
 * @param px The pespective distance X coordinate
 * @param py The pespective distance Y coordinate
 * @param z0 The "0" z plane value
 * @param foc The focal distance
 */
EAPI void              evas_map_util_3d_perspective                  (Evas_Map *m, Evas_Coord px, Evas_Coord py, Evas_Coord z0, Evas_Coord foc);

/**
 * Get the clockwise state of a map
 *
 * This determines if the output points (X and Y. Z is not used) are
 * clockwise or anti-clockwise. This can be used for "back-face culling". This
 * is where you hide objects that "face away" from you. In this case objects
 * that are not clockwise.
 *
 * @param m map to query.
 * @return 1 if clockwise, 0 otherwise
 */
EAPI Eina_Bool         evas_map_util_clockwise_get                   (Evas_Map *m);


/**
 * Create map of transformation points to be later used with an Evas object.
 *
 * This creates a set of points (currently only 4 is supported. no other
 * number for @p count will work). That is empty and ready to be modified
 * with evas_map calls.
 *
 * @param count number of points in the map. *
 * @return a newly allocated map or @c NULL on errors.
 *
 * @see evas_map_free()
 * @see evas_map_dup()
 * @see evas_map_point_coord_set()
 * @see evas_map_point_image_uv_set()
 * @see evas_map_util_points_populate_from_object_full()
 * @see evas_map_util_points_populate_from_object()
 *
 * @see evas_object_map_set()
 */
EAPI Evas_Map         *evas_map_new                      (int count);

/**
 * Set the smoothing for map rendering
 *
 * This sets smoothing for map rendering. If the object is a type that has
 * its own smoothing settings, then both the smooth settings for this object
 * and the map must be turned off. By default smooth maps are enabled.
 *
 * @param m map to modify. Must not be NULL.
 * @param enabled enable or disable smooth map rendering
 */
EAPI void              evas_map_smooth_set               (Evas_Map *m, Eina_Bool enabled);

/**
 * get the smoothing for map rendering
 *
 * This gets smoothing for map rendering.
 *
 * @param m map to get the smooth from. Must not be NULL.
 */
EAPI Eina_Bool         evas_map_smooth_get               (const Evas_Map *m);

/**
 * Set the alpha flag for map rendering
 *
 * This sets alpha flag for map rendering. If the object is a type that has
 * its own alpha settings, then this will take precedence. Only image objects
 * have this currently. Fits stops alpha blending of the map area, and is
 * useful if you know the object and/or all sub-objects is 100% solid.
 *
 * @param m map to modify. Must not be NULL.
 * @param enabled enable or disable alpha map rendering
 */
EAPI void              evas_map_alpha_set                (Evas_Map *m, Eina_Bool enabled);

/**
 * get the alpha flag for map rendering
 *
 * This gets the alph flag for map rendering.
 *
 * @param m map to get the alpha from. Must not be NULL.
 */
EAPI Eina_Bool         evas_map_alpha_get                (const Evas_Map *m);

/**
 * Copy a previously allocated map.
 *
 * This makes a duplicate of the @p m object and returns it.
 *
 * @param m map to copy. Must not be NULL.
 * @return newly allocated map with the same count and contents as @p m.
 */
EAPI Evas_Map         *evas_map_dup                      (const Evas_Map *m);

/**
 * Free a previously allocated map.
 *
 * This frees a givem map @p m and all memory associated with it. You must NOT
 * free a map returned by evas_object_map_get() as this is internal.
 *
 * @param m map to free.
 */
EAPI void              evas_map_free                     (Evas_Map *m);

/**
 * Get a maps size.
 *
 * Returns the number of points in a map.  Should be at least 4.
 *
 * @param m map to get size.
 * @return -1 on error, points otherwise.
 */
EAPI int               evas_map_count_get               (const Evas_Map *m) EINA_CONST;

/**
 * Change the map point's coordinate.
 *
 * This sets the fixen point's coordinate in the map. Note that points
 * describe the outline of a quadrangle and are ordered either clockwise
 * or anit-clock-wise. It is suggested to keep your quadrangles concave and
 * non-complex, though these polygon modes may work, they may not render
 * a desired set of output. The quadrangle will use points 0 and 1 , 1 and 2,
 * 2 and 3, and 3 and 0 to describe the edges of the quandrangle.
 *
 * The X and Y and Z coordinates are in canvas units. Z is optional and may
 * or may not be honored in drawing. Z is a hint and does not affect the
 * X and Y rendered coordinates. It may be used for calculating fills with
 * perspective correct rendering.
 *
 * Remember all coordinates are canvas global ones like with move and reize
 * in evas.
 *
 * @param m map to change point. Must not be @c NULL.
 * @param idx index of point to change. Must be smaller than map size.
 * @param x Point X Coordinate
 * @param y Point Y Coordinate
 * @param z Point Z Coordinate hint (pre-perspective transform)
 *
 * @see evas_map_util_rotate()
 * @see evas_map_util_zoom()
 * @see evas_map_util_points_populate_from_object_full()
 * @see evas_map_util_points_populate_from_object()
 */
EAPI void              evas_map_point_coord_set          (Evas_Map *m, int idx, Evas_Coord x, Evas_Coord y, Evas_Coord z);

/**
 * Get the map point's coordinate.
 *
 * This returns the coordinates of the given point in the map.
 *
 * @param m map to query point.
 * @param idx index of point to query. Must be smaller than map size.
 * @param x where to return the X coordinate.
 * @param y where to return the Y coordinate.
 * @param z where to return the Z coordinate.
 */
EAPI void              evas_map_point_coord_get          (const Evas_Map *m, int idx, Evas_Coord *x, Evas_Coord *y, Evas_Coord *z);

/**
 * Change the map point's U and V texture source point
 *
 * This sets the U and V coordinates for the point. This determines which
 * coordinate in the source image is mapped to the given point, much like
 * OpenGL and textures. Notes that these points do select the pixel, but
 * are double floating point values to allow for accuracy and sub-pixel
 * selection.
 *
 * @param m map to change the point of.
 * @param idx index of point to change. Must be smaller than map size.
 * @param u the X coordinate within the image/texture source
 * @param v the Y coordinate within the image/texture source
 *
 * @see evas_map_point_coord_set()
 * @see evas_object_map_set()
 * @see evas_map_util_points_populate_from_object_full()
 * @see evas_map_util_points_populate_from_object()
 */
EAPI void              evas_map_point_image_uv_set       (Evas_Map *m, int idx, double u, double v);

/**
 * Get the map point's U and V texture source points
 *
 * This returns the texture points set by evas_map_point_image_uv_set().
 *
 * @param m map to query point.
 * @param idx index of point to query. Must be smaller than map size.
 * @param u where to write the X coordinate within the image/texture source
 * @param v where to write the Y coordinate within the image/texture source
 */
EAPI void              evas_map_point_image_uv_get       (const Evas_Map *m, int idx, double *u, double *v);

/**
 * Set the color of a vertex in the map
 *
 * This sets the color of the vertex in the map. Colors will be linearly
 * interpolated between vertex points through the map. Color will multiply
 * the "texture" pixels (like GL_MODULATE in OpenGL). The default color of
 * a vertex in a map is white solid (255, 255, 255, 255) which means it will
 * have no affect on modifying the texture pixels.
 *
 * @param m map to change the color of.
 * @param idx index of point to change. Must be smaller than map size.
 * @param r red (0 - 255)
 * @param g green (0 - 255)
 * @param b blue (0 - 255)
 * @param a alpha (0 - 255)
 *
 * @see evas_map_util_points_color_set()
 * @see evas_map_point_coord_set()
 * @see evas_object_map_set()
 */
EAPI void              evas_map_point_color_set          (Evas_Map *m, int idx, int r, int g, int b, int a);

/**
 * Get the color set on a vertex in the map
 *
 * This gets the color set by evas_map_point_color_set() on the given vertex
 * of the map.
 *
 * @param m map to get the color of the vertex from.
 * @param idx index of point get. Must be smaller than map size.
 * @param r pointer to red return
 * @param g pointer to green return
 * @param b pointer to blue return
 * @param a pointer to alpha return (0 - 255)
 *
 * @see evas_map_point_coord_set()
 * @see evas_object_map_set()
 */
EAPI void              evas_map_point_color_get          (const Evas_Map *m, int idx, int *r, int *g, int *b, int *a);

/**
 * @defgroup Evas_Object_Group_Size_Hints Size Hints
 *
 * Objects may carry hints, so that another object that acts as a
 * manager (see @ref Evas_Smart_Object_Group) may know how to properly
 * position and resize its subordinate objects. The Size Hints provide
 * a common interface that is recommended as the protocol for such
 * information.
 *
 * For example, box objects use alignment hints to align its
 * lines/columns inside its container, padding hints to set the
 * padding between each individual child, etc.
 *
 * @ingroup Evas_Object_Group
 */

/**
 * @addtogroup Evas_Object_Group_Size_Hints
 * @{
 */

/**
 * Retrieves the hints for an object's minimum size.
 *
 * @param obj The given Evas object to query hints from.
 * @param w Pointer to an integer in which to store the minimum width.
 * @param h Pointer to an integer in which to store the minimum height.
 *
 * These are hints on the minimim sizes @p obj should have. This is
 * not a size enforcement in any way, it's just a hint that should be
 * used whenever appropriate.
 *
 * @note Use @c NULL pointers on the hint components you're not
 * interested in: they'll be ignored by the function.
 *
 * @see evas_object_size_hint_min_set() for an example
 */
EAPI void              evas_object_size_hint_min_get     (const Evas_Object *obj, Evas_Coord *w, Evas_Coord *h) EINA_ARG_NONNULL(1);

/**
 * Sets the hints for an object's minimum size.
 *
 * @param obj The given Evas object to query hints from.
 * @param w Integer to use as the minimum width hint.
 * @param h Integer to use as the minimum height hint.
 *
 * This is not a size enforcement in any way, it's just a hint that
 * should be used whenever appropriate.
 *
 * Values @c 0 will be treated as unset hint components, when queried
 * by managers.
 *
 * Example:
 * @dontinclude evas-hints.c
 * @skip evas_object_size_hint_min_set
 * @until return
 *
 * In this example the minimum size hints change de behavior of an
 * Evas box when layouting its children. See the full @ref
 * Example_Evas_Size_Hints "example".
 *
 * @see evas_object_size_hint_min_get()
 */
EAPI void              evas_object_size_hint_min_set     (Evas_Object *obj, Evas_Coord w, Evas_Coord h) EINA_ARG_NONNULL(1);

/**
 * Retrieves the hints for an object's maximum size.
 *
 * @param obj The given Evas object to query hints from.
 * @param w Pointer to an integer in which to store the maximum width.
 * @param h Pointer to an integer in which to store the maximum height.
 *
 * These are hints on the maximum sizes @p obj should have. This is
 * not a size enforcement in any way, it's just a hint that should be
 * used whenever appropriate.
 *
 * @note Use @c NULL pointers on the hint components you're not
 * interested in: they'll be ignored by the function.
 *
 * @see evas_object_size_hint_max_set()
 */
EAPI void              evas_object_size_hint_max_get     (const Evas_Object *obj, Evas_Coord *w, Evas_Coord *h) EINA_ARG_NONNULL(1);

/**
 * Sets the hints for an object's maximum size.
 *
 * @param obj The given Evas object to query hints from.
 * @param w Integer to use as the maximum width hint.
 * @param h Integer to use as the maximum height hint.
 *
 * This is not a size enforcement in any way, it's just a hint that
 * should be used whenever appropriate.
 *
 * Values @c -1 will be treated as unset hint components, when queried
 * by managers.
 *
 * Example:
 * @dontinclude evas-hints.c
 * @skip evas_object_size_hint_max_set
 * @until return
 *
 * In this example the maximum size hints change de behavior of an
 * Evas box when layouting its children. See the full @ref
 * Example_Evas_Size_Hints "example".
 *
 * @see evas_object_size_hint_max_get()
 */
EAPI void              evas_object_size_hint_max_set     (Evas_Object *obj, Evas_Coord w, Evas_Coord h) EINA_ARG_NONNULL(1);

/**
 * Retrieves the hints for an object's optimum size.
 *
 * @param obj The given Evas object to query hints from.
 * @param w Pointer to an integer in which to store the requested width.
 * @param h Pointer to an integer in which to store the requested height.
 *
 * These are hints on the optimum sizes @p obj should have. This is
 * not a size enforcement in any way, it's just a hint that should be
 * used whenever appropriate.
 *
 * @note Use @c NULL pointers on the hint components you're not
 * interested in: they'll be ignored by the function.
 *
 * @see evas_object_size_hint_request_set()
 */
EAPI void              evas_object_size_hint_request_get (const Evas_Object *obj, Evas_Coord *w, Evas_Coord *h) EINA_ARG_NONNULL(1);

/**
 * Sets the hints for an object's optimum size.
 *
 * @param obj The given Evas object to query hints from.
 * @param w Integer to use as the preferred width hint.
 * @param h Integer to use as the preferred height hint.
 *
 * This is not a size enforcement in any way, it's just a hint that
 * should be used whenever appropriate.
 *
 * Values @c 0 will be treated as unset hint components, when queried
 * by managers.
 *
 * @see evas_object_size_hint_request_get()
 */
EAPI void              evas_object_size_hint_request_set (Evas_Object *obj, Evas_Coord w, Evas_Coord h) EINA_ARG_NONNULL(1);

/**
 * Retrieves the hints for an object's aspect ratio.
 *
 * @param obj The given Evas object to query hints from.
 * @param aspect Returns the policy/type of aspect ratio applied to @p obj.
 * @param w Pointer to an integer in which to store the aspect's width
 * ratio term.
 * @param h Pointer to an integer in which to store the aspect's
 * height ratio term.
 *
 * The different aspect ratio policies are documented in the
 * #Evas_Aspect_Control type. A container respecting these size hints
 * would @b scale its children accordingly to those policies.
 *
 * For any policy, if any of the given aspect ratio terms are @c 0,
 * the object's container should ignore the aspect and scale @p obj to
 * occupy the whole available area. If they are both positive
 * integers, that proportion will be respected, under each scaling
 * policy.
 *
 * These images illustrate some of the #Evas_Aspect_Control policies:
 *
 * @image html any-policy.png
 * @image rtf any-policy.png
 * @image latex any-policy.eps
 *
 * @image html aspect-control-none-neither.png
 * @image rtf aspect-control-none-neither.png
 * @image latex aspect-control-none-neither.eps
 *
 * @image html aspect-control-both.png
 * @image rtf aspect-control-both.png
 * @image latex aspect-control-both.eps
 *
 * @image html aspect-control-horizontal.png
 * @image rtf aspect-control-horizontal.png
 * @image latex aspect-control-horizontal.eps
 *
 * This is not a size enforcement in any way, it's just a hint that
 * should be used whenever appropriate.
 *
 * @note Use @c NULL pointers on the hint components you're not
 * interested in: they'll be ignored by the function.
 *
 * Example:
 * @dontinclude evas-aspect-hints.c
 * @skip if (strcmp(ev->keyname, "c") == 0)
 * @until }
 *
 * See the full @ref Example_Evas_Aspect_Hints "example".
 *
 * @see evas_object_size_hint_aspect_set()
 */
EAPI void              evas_object_size_hint_aspect_get  (const Evas_Object *obj, Evas_Aspect_Control *aspect, Evas_Coord *w, Evas_Coord *h) EINA_ARG_NONNULL(1);

/**
 * Sets the hints for an object's aspect ratio.
 *
 * @param obj The given Evas object to query hints from.
 * @param aspect The policy/type of aspect ratio to apply to @p obj.
 * @param w Integer to use as aspect width ratio term.
 * @param h Integer to use as aspect height ratio term.
 *
 * This is not a size enforcement in any way, it's just a hint that should
 * be used whenever appropriate.
 *
 * If any of the given aspect ratio terms are @c 0,
 * the object's container will ignore the aspect and scale @p obj to
 * occupy the whole available area, for any given policy.
 *
 * @see evas_object_size_hint_aspect_get() for more information.
 */
EAPI void              evas_object_size_hint_aspect_set  (Evas_Object *obj, Evas_Aspect_Control aspect, Evas_Coord w, Evas_Coord h) EINA_ARG_NONNULL(1);

/**
 * Retrieves the hints for on object's alignment.
 *
 * @param obj The given Evas object to query hints from.
 * @param x Pointer to a double in which to store the horizontal
 * alignment hint.
 * @param y Pointer to a double in which to store the vertical
 * alignment hint.
 *
 * This is not a size enforcement in any way, it's just a hint that
 * should be used whenever appropriate.
 *
 * @note Use @c NULL pointers on the hint components you're not
 * interested in: they'll be ignored by the function.
 *
 * @see evas_object_size_hint_align_set() for more information
 */
EAPI void              evas_object_size_hint_align_get   (const Evas_Object *obj, double *x, double *y) EINA_ARG_NONNULL(1);

/**
 * Sets the hints for an object's alignment.
 *
 * @param obj The given Evas object to query hints from.
 * @param x Double, ranging from @c 0.0 to @c 1.0 or with the
 * special value #EVAS_HINT_FILL, to use as horizontal alignment hint.
 * @param y Double, ranging from @c 0.0 to @c 1.0 or with the
 * special value #EVAS_HINT_FILL, to use as vertical alignment hint.
 *
 * These are hints on how to align an object <b>inside the boundaries
 * of a container/manager</b>. Accepted values are in the @c 0.0 to @c
 * 1.0 range, with the special value #EVAS_HINT_FILL used to specify
 * "justify" or "fill" by some users. See documentation of possible
 * users: in Evas, they are the @ref Evas_Object_Box "box" and @ref
 * Evas_Object_Table "table" smart objects.
 *
 * For the horizontal component, @c 0.0 means to the left, @c 1.0
 * means to the right. Analogously, for the vertical component, @c 0.0
 * to the top, @c 1.0 means to the bottom.
 *
 * See the following figure:
 * @image html alignment-hints.png
 * @image rtf alignment-hints.png
 * @image latex alignment-hints.eps
 *
 * This is not a size enforcement in any way, it's just a hint that
 * should be used whenever appropriate.
 *
 * Example:
 * @dontinclude evas-hints.c
 * @skip evas_object_size_hint_align_set
 * @until return
 *
 * In this example the alignment hints change de behavior of an Evas
 * box when layouting its children. See the full @ref
 * Example_Evas_Size_Hints "example".
 *
 * @see evas_object_size_hint_align_get()
 */
EAPI void              evas_object_size_hint_align_set   (Evas_Object *obj, double x, double y) EINA_ARG_NONNULL(1);

/**
 * Retrieves the hints for an object's weight.
 *
 * @param obj The given Evas object to query hints from.
 * @param x Pointer to a double in which to store the horizontal weight.
 * @param y Pointer to a double in which to store the vertical weight.
 *
 * Accepted values are zero or positive values. Some users might use
 * this hint as a boolean, but some might consider it as a @b
 * proportion, see documentation of possible users, which in Evas are
 * the @ref Evas_Object_Box "box" and @ref Evas_Object_Table "table"
 * smart objects.
 *
 * This is not a size enforcement in any way, it's just a hint that
 * should be used whenever appropriate.
 *
 * @note Use @c NULL pointers on the hint components you're not
 * interested in: they'll be ignored by the function.
 *
 * @see evas_object_size_hint_weight_set() for an example
 */
EAPI void              evas_object_size_hint_weight_get  (const Evas_Object *obj, double *x, double *y) EINA_ARG_NONNULL(1);

/**
 * Sets the hints for an object's weight.
 *
 * @param obj The given Evas object to query hints from.
 * @param x Double (@c 0.0-1.0) to use as horizontal weight hint.
 * @param y Double (@c 0.0-1.0) to use as vertical weight hint.
 *
 * This is not a size enforcement in any way, it's just a hint that should
 * be used whenever appropriate.
 *
 * Example:
 * @dontinclude evas-hints.c
 * @skip evas_object_size_hint_weight_set
 * @until return
 *
 * In this example the weight hints change de behavior of an Evas box
 * when layouting its children. See the full @ref
 * Example_Evas_Size_Hints "example".
 *
 * @see evas_object_size_hint_weight_get() for more information
 */
EAPI void              evas_object_size_hint_weight_set  (Evas_Object *obj, double x, double y) EINA_ARG_NONNULL(1);

/**
 * Retrieves the hints for an object's padding space.
 *
 * @param obj The given Evas object to query hints from.
 * @param l Pointer to an integer in which to store left padding.
 * @param r Pointer to an integer in which to store right padding.
 * @param t Pointer to an integer in which to store top padding.
 * @param b Pointer to an integer in which to store bottom padding.
 *
 * Padding is extra space an object takes on each of its delimiting
 * rectangle sides, in canvas units. This space will be rendered
 * transparent, naturally.
 *
 * This is not a size enforcement in any way, it's just a hint that
 * should be used whenever appropriate.
 *
 * @note Use @c NULL pointers on the hint components you're not
 * interested in: they'll be ignored by the function.
 *
 * Example:
 * @dontinclude evas-hints.c
 * @skip evas_object_size_hint_padding_set
 * @until return
 *
 * In this example the padding hints change de behavior of an Evas box
 * when layouting its children. See the full @ref
 * Example_Evas_Size_Hints "example".
 *
 * @see evas_object_size_hint_padding_set()
 */
EAPI void              evas_object_size_hint_padding_get (const Evas_Object *obj, Evas_Coord *l, Evas_Coord *r, Evas_Coord *t, Evas_Coord *b) EINA_ARG_NONNULL(1);

/**
 * Sets the hints for an object's padding space.
 *
 * @param obj The given Evas object to query hints from.
 * @param l Integer to specify left padding.
 * @param r Integer to specify right padding.
 * @param t Integer to specify top padding.
 * @param b Integer to specify bottom padding.
 *
 * This is not a size enforcement in any way, it's just a hint that
 * should be used whenever appropriate.
 *
 * @see evas_object_size_hint_padding_get() for more information
 */
EAPI void              evas_object_size_hint_padding_set (Evas_Object *obj, Evas_Coord l, Evas_Coord r, Evas_Coord t, Evas_Coord b) EINA_ARG_NONNULL(1);

/**
 * @}
 */

/**
 * @defgroup Evas_Object_Group_Extras Extra Object Manipulation
 *
 * Miscellaneous functions that also apply to any object, but are less
 * used or not implemented by all objects.
 *
 * @ingroup Evas_Object_Group
 */

/**
 * @addtogroup Evas_Object_Group_Extras
 * @{
 */

/**
 * Set an attached data pointer to an object with a given string key.
 *
 * @param obj The object to attach the data pointer to
 * @param key The string key for the data to access it
 * @param data The ponter to the data to be attached
 *
 * This attaches the pointer @p data to the object @p obj, given the
 * access string @p key. This pointer will stay "hooked" to the object
 * until a new pointer with the same string key is attached with
 * evas_object_data_set() or it is deleted with
 * evas_object_data_del(). On deletion of the object @p obj, the
 * pointers will not be accessible from the object anymore.
 *
 * You can find the pointer attached under a string key using
 * evas_object_data_get(). It is the job of the calling application to
 * free any data pointed to by @p data when it is no longer required.
 *
 * If @p data is @c NULL, the old value stored at @p key will be
 * removed but no new value will be stored. This is synonymous with
 * calling evas_object_data_del() with @p obj and @p key.
 *
 * @note This function is very handy when you have data associated
 * specifically to an Evas object, being of use only when dealing with
 * it. Than you don't have the burden to a pointer to it elsewhere,
 * using this family of functions.
 *
 * Example:
 *
 * @code
 * int *my_data;
 * extern Evas_Object *obj;
 *
 * my_data = malloc(500);
 * evas_object_data_set(obj, "name_of_data", my_data);
 * printf("The data that was attached was %p\n", evas_object_data_get(obj, "name_of_data"));
 * @endcode
 */
EAPI void                      evas_object_data_set             (Evas_Object *obj, const char *key, const void *data) EINA_ARG_NONNULL(1, 2);

/**
 * Return an attached data pointer on an Evas object by its given
 * string key.
 *
 * @param obj The object to which the data was attached
 * @param key The string key the data was stored under
 * @return The data pointer stored, or @c NULL if none was stored
 *
 * This function will return the data pointer attached to the object
 * @p obj, stored using the string key @p key. If the object is valid
 * and a data pointer was stored under the given key, that pointer
 * will be returned. If this is not the case, @c NULL will be
 * returned, signifying an invalid object or a non-existent key. It is
 * possible that a @c NULL pointer was stored given that key, but this
 * situation is non-sensical and thus can be considered an error as
 * well. @c NULL pointers are never stored as this is the return value
 * if an error occurs.
 *
 * Example:
 *
 * @code
 * int *my_data;
 * extern Evas_Object *obj;
 *
 * my_data = evas_object_data_get(obj, "name_of_my_data");
 * if (my_data) printf("Data stored was %p\n", my_data);
 * else printf("No data was stored on the object\n");
 * @endcode
 */
EAPI void                     *evas_object_data_get             (const Evas_Object *obj, const char *key) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1, 2) EINA_PURE;

/**
 * Delete an attached data pointer from an object.
 *
 * @param obj The object to delete the data pointer from
 * @param key The string key the data was stored under
 * @return The original data pointer stored at @p key on @p obj
 *
 * This will remove the stored data pointer from @p obj stored under
 * @p key and return this same pointer, if actually there was data
 * there, or @c NULL, if nothing was stored under that key.
 *
 * Example:
 *
 * @code
 * int *my_data;
 * extern Evas_Object *obj;
 *
 * my_data = evas_object_data_del(obj, "name_of_my_data");
 * @endcode
 */
EAPI void                     *evas_object_data_del             (Evas_Object *obj, const char *key) EINA_ARG_NONNULL(1, 2);


/**
 * Set pointer behavior.
 *
 * @param obj
 * @param setting desired behavior.
 *
 * This function has direct effect on event callbacks related to
 * mouse.
 *
 * If @p setting is EVAS_OBJECT_POINTER_MODE_AUTOGRAB, then when mouse
 * is down at this object, events will be restricted to it as source,
 * mouse moves, for example, will be emitted even if outside this
 * object area.
 *
 * If @p setting is EVAS_OBJECT_POINTER_MODE_NOGRAB, then events will
 * be emitted just when inside this object area.
 *
 * The default value is EVAS_OBJECT_POINTER_MODE_AUTOGRAB.
 *
 * @ingroup Evas_Object_Group_Extras
 */
EAPI void                      evas_object_pointer_mode_set     (Evas_Object *obj, Evas_Object_Pointer_Mode setting) EINA_ARG_NONNULL(1);

/**
 * Determine how pointer will behave.
 * @param obj
 * @return pointer behavior.
 * @ingroup Evas_Object_Group_Extras
 */
EAPI Evas_Object_Pointer_Mode  evas_object_pointer_mode_get     (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;


/**
 * Sets whether or not the given Evas object is to be drawn anti-aliased.
 *
 * @param   obj The given Evas object.
 * @param   anti_alias 1 if the object is to be anti_aliased, 0 otherwise.
 * @ingroup Evas_Object_Group_Extras
 */
EAPI void                      evas_object_anti_alias_set       (Evas_Object *obj, Eina_Bool antialias) EINA_ARG_NONNULL(1);

/**
 * Retrieves whether or not the given Evas object is to be drawn anti_aliased.
 * @param   obj The given Evas object.
 * @return  @c 1 if the object is to be anti_aliased.  @c 0 otherwise.
 * @ingroup Evas_Object_Group_Extras
 */
EAPI Eina_Bool                 evas_object_anti_alias_get       (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;


/**
 * Sets the scaling factor for an Evas object. Does not affect all
 * objects.
 *
 * @param obj The given Evas object.
 * @param scale The scaling factor. <c>1.0</c> means no scaling,
 *        default size.
 *
 * This will multiply the object's dimension by the given factor, thus
 * altering its geometry (width and height). Useful when you want
 * scalable UI elements, possibly at run time.
 *
 * @note Only text and textblock objects have scaling change
 * handlers. Other objects won't change visually on this call.
 *
 * @see evas_object_scale_get()
 *
 * @ingroup Evas_Object_Group_Extras
 */
EAPI void                      evas_object_scale_set            (Evas_Object *obj, double scale) EINA_ARG_NONNULL(1);

/**
 * Retrieves the scaling factor for the given Evas object.
 *
 * @param   obj The given Evas object.
 * @return  The scaling factor.
 *
 * @ingroup Evas_Object_Group_Extras
 *
 * @see evas_object_scale_set()
 */
EAPI double                    evas_object_scale_get            (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;


/**
 * Sets the render_op to be used for rendering the Evas object.
 * @param   obj The given Evas object.
 * @param   render_op one of the Evas_Render_Op values.
 * @ingroup Evas_Object_Group_Extras
 */
EAPI void                      evas_object_render_op_set        (Evas_Object *obj, Evas_Render_Op op) EINA_ARG_NONNULL(1);

/**
 * Retrieves the current value of the operation used for rendering the Evas object.
 * @param   obj The given Evas object.
 * @return  one of the enumerated values in Evas_Render_Op.
 * @ingroup Evas_Object_Group_Extras
 */
EAPI Evas_Render_Op            evas_object_render_op_get        (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

   EAPI void                      evas_object_precise_is_inside_set(Evas_Object *obj, Eina_Bool precise) EINA_ARG_NONNULL(1);
   EAPI Eina_Bool                 evas_object_precise_is_inside_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

   EAPI void                      evas_object_static_clip_set      (Evas_Object *obj, Eina_Bool is_static_clip) EINA_ARG_NONNULL(1);
   EAPI Eina_Bool                 evas_object_static_clip_get      (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

/**
 * @}
 */

/**
 * @defgroup Evas_Object_Group_Find Finding Objects
 *
 * Functions that allows finding objects by their position, name or
 * other properties.
 *
 * @ingroup Evas_Object_Group
 */

/**
 * Retrieve the object that currently has focus.
 *
 * @param e The Evas canvas to query for focused object on.
 * @return The object that has focus or @c NULL if there is not one.
 *
 * Evas can have (at most) one of its objects focused at a time.
 * Focused objects will be the ones having <b>key events</b> delivered
 * to, which the programmer can act upon by means of
 * evas_object_event_callback_add() usage.
 *
 * @note Most users wouldn't be dealing directly with Evas' focused
 * objects. Instead, they would be using a higher level library for
 * that (like a toolkit, as Elementary) to handle focus and who's
 * receiving input for them.
 *
 * This call returns the object that currently has focus on the canvas
 * @p e or @c NULL, if none.
 *
 * @see evas_object_focus_set
 * @see evas_object_focus_get
 * @see evas_object_key_grab
 * @see evas_object_key_ungrab
 *
 * Example:
 * @dontinclude evas-events.c
 * @skip evas_event_callback_add(d.canvas, EVAS_CALLBACK_CANVAS_OBJECT_FOCUS_IN,
 * @until evas_object_focus_set(d.bg, EINA_TRUE);
 * @dontinclude evas-events.c
 * @skip called when our rectangle gets focus
 * @until }
 *
 * In this example the @c event_info is exactly a pointer to that
 * focused rectangle. See the full @ref Example_Evas_Events "example".
 *
 * @ingroup Evas_Object_Group_Find
 */
EAPI Evas_Object      *evas_focus_get                    (const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;


/**
 * Retrieves the object on the given evas with the given name.
 * @param   e    The given evas.
 * @param   name The given name.
 * @return  If successful, the Evas object with the given name.  Otherwise,
 *          @c NULL.
 * @ingroup Evas_Object_Group_Find
 */
EAPI Evas_Object      *evas_object_name_find             (const Evas *e, const char *name) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;


/**
 * Retrieves the top object at the given position (x,y)
 * @param   e The given Evas object.
 * @param   x The horizontal coordinate
 * @param   y The vertical coordinate
 * @param   include_pass_events_objects Boolean Flag to include or not
 * pass events objects
 * @param   include_hidden_objects Boolean Flag to include or not hidden objects
 * @return  The Evas object that is over all others objects at the given position.
 */
EAPI Evas_Object      *evas_object_top_at_xy_get         (const Evas *e, Evas_Coord x, Evas_Coord y, Eina_Bool include_pass_events_objects, Eina_Bool include_hidden_objects) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

/**
 * Retrieves the top object at mouse pointer position
 * @param   e The given Evas object.
 * @return The Evas object that is over all others objects at the
 * pointer position.
 */
EAPI Evas_Object      *evas_object_top_at_pointer_get    (const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

/**
 * Retrieves the top object in the given rectangle region
 * @param   e The given Evas object.
 * @param   x The horizontal coordinate.
 * @param   y The vertical coordinate.
 * @param   w The width size.
 * @param   h The height size.
 * @param   include_pass_events_objects Boolean Flag to include or not pass events objects
 * @param   include_hidden_objects Boolean Flag to include or not hidden objects
 * @return  The Evas object that is over all others objects at the pointer position.
 *
 */
EAPI Evas_Object      *evas_object_top_in_rectangle_get  (const Evas *e, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h, Eina_Bool include_pass_events_objects, Eina_Bool include_hidden_objects) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;


/**
 * Retrieves the objects at the given position
 * @param   e The given Evas object.
 * @param   x The horizontal coordinate.
 * @param   y The vertical coordinate.
 * @param include_pass_events_objects Boolean Flag to include or not
 * pass events objects
 * @param   include_hidden_objects Boolean Flag to include or not hidden objects
 * @return  The list of Evas objects at the pointer position.
 *
 */
EAPI Eina_List        *evas_objects_at_xy_get            (const Evas *e, Evas_Coord x, Evas_Coord y, Eina_Bool include_pass_events_objects, Eina_Bool include_hidden_objects) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI Eina_List        *evas_objects_in_rectangle_get     (const Evas *e, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h, Eina_Bool include_pass_events_objects, Eina_Bool include_hidden_objects) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;


/**
 * Get the lowest Evas object on the Evas @p e
 *
 * @param e an Evas
 * @return the lowest object
 */
EAPI Evas_Object      *evas_object_bottom_get            (const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

/**
 * Get the highest Evas object on the Evas @p e
 *
 * @param e an Evas
 * @return the highest object
 */
EAPI Evas_Object      *evas_object_top_get               (const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

/**
 * @defgroup Evas_Object_Group_Interceptors Object Method Interceptors
 *
 * Evas provides a way to intercept method calls. The interceptor
 * callback may opt to completely deny the call, or may check and
 * change the parameters before continuing. The continuation of an
 * intercepted call is done by calling the intercepted call again,
 * from inside the interceptor callback.
 *
 * @ingroup Evas_Object_Group
 */
typedef void (*Evas_Object_Intercept_Show_Cb) (void *data, Evas_Object *obj);
typedef void (*Evas_Object_Intercept_Hide_Cb) (void *data, Evas_Object *obj);
typedef void (*Evas_Object_Intercept_Move_Cb) (void *data, Evas_Object *obj, Evas_Coord x, Evas_Coord y);
typedef void (*Evas_Object_Intercept_Resize_Cb) (void *data, Evas_Object *obj, Evas_Coord w, Evas_Coord h);
typedef void (*Evas_Object_Intercept_Raise_Cb) (void *data, Evas_Object *obj);
typedef void (*Evas_Object_Intercept_Lower_Cb) (void *data, Evas_Object *obj);
typedef void (*Evas_Object_Intercept_Stack_Above_Cb) (void *data, Evas_Object *obj, Evas_Object *above);
typedef void (*Evas_Object_Intercept_Stack_Below_Cb) (void *data, Evas_Object *obj, Evas_Object *above);
typedef void (*Evas_Object_Intercept_Layer_Set_Cb) (void *data, Evas_Object *obj, int l);
typedef void (*Evas_Object_Intercept_Color_Set_Cb) (void *data, Evas_Object *obj, int r, int g, int b, int a);
typedef void (*Evas_Object_Intercept_Clip_Set_Cb) (void *data, Evas_Object *obj, Evas_Object *clip);
typedef void (*Evas_Object_Intercept_Clip_Unset_Cb) (void *data, Evas_Object *obj);


/**
 * Set the callback function that intercepts a show event of a object.
 *
 * @param obj The given canvas object pointer.
 * @param func The given function to be the callback function.
 * @param data The data passed to the callback function.
 *
 * This function sets a callback function to intercepts a show event
 * of a canvas object.
 *
 * @see evas_object_intercept_show_callback_del().
 *
 */
EAPI void              evas_object_intercept_show_callback_add        (Evas_Object *obj, Evas_Object_Intercept_Show_Cb func, const void *data) EINA_ARG_NONNULL(1, 2);

/**
 * Unset the callback function that intercepts a show event of a
 * object.
 *
 * @param obj The given canvas object pointer.
 * @param func The given callback function.
 *
 * This function sets a callback function to intercepts a show event
 * of a canvas object.
 *
 * @see evas_object_intercept_show_callback_add().
 *
 */
EAPI void             *evas_object_intercept_show_callback_del        (Evas_Object *obj, Evas_Object_Intercept_Show_Cb func) EINA_ARG_NONNULL(1, 2);

/**
 * Set the callback function that intercepts a hide event of a object.
 *
 * @param obj The given canvas object pointer.
 * @param func The given function to be the callback function.
 * @param data The data passed to the callback function.
 *
 * This function sets a callback function to intercepts a hide event
 * of a canvas object.
 *
 * @see evas_object_intercept_hide_callback_del().
 *
 */
EAPI void              evas_object_intercept_hide_callback_add        (Evas_Object *obj, Evas_Object_Intercept_Hide_Cb func, const void *data) EINA_ARG_NONNULL(1, 2);

/**
 * Unset the callback function that intercepts a hide event of a
 * object.
 *
 * @param obj The given canvas object pointer.
 * @param func The given callback function.
 *
 * This function sets a callback function to intercepts a hide event
 * of a canvas object.
 *
 * @see evas_object_intercept_hide_callback_add().
 *
 */
EAPI void             *evas_object_intercept_hide_callback_del        (Evas_Object *obj, Evas_Object_Intercept_Hide_Cb func) EINA_ARG_NONNULL(1, 2);

/**
 * Set the callback function that intercepts a move event of a object.
 *
 * @param obj The given canvas object pointer.
 * @param func The given function to be the callback function.
 * @param data The data passed to the callback function.
 *
 * This function sets a callback function to intercepts a move event
 * of a canvas object.
 *
 * @see evas_object_intercept_move_callback_del().
 *
 */
EAPI void              evas_object_intercept_move_callback_add        (Evas_Object *obj, Evas_Object_Intercept_Move_Cb func, const void *data) EINA_ARG_NONNULL(1, 2);

/**
 * Unset the callback function that intercepts a move event of a
 * object.
 *
 * @param obj The given canvas object pointer.
 * @param func The given callback function.
 *
 * This function sets a callback function to intercepts a move event
 * of a canvas object.
 *
 * @see evas_object_intercept_move_callback_add().
 *
 */
EAPI void             *evas_object_intercept_move_callback_del        (Evas_Object *obj, Evas_Object_Intercept_Move_Cb func) EINA_ARG_NONNULL(1, 2);

   EAPI void              evas_object_intercept_resize_callback_add      (Evas_Object *obj, Evas_Object_Intercept_Resize_Cb func, const void *data) EINA_ARG_NONNULL(1, 2);
   EAPI void             *evas_object_intercept_resize_callback_del      (Evas_Object *obj, Evas_Object_Intercept_Resize_Cb func) EINA_ARG_NONNULL(1, 2);
   EAPI void              evas_object_intercept_raise_callback_add       (Evas_Object *obj, Evas_Object_Intercept_Raise_Cb func, const void *data) EINA_ARG_NONNULL(1, 2);
   EAPI void             *evas_object_intercept_raise_callback_del       (Evas_Object *obj, Evas_Object_Intercept_Raise_Cb func) EINA_ARG_NONNULL(1, 2);
   EAPI void              evas_object_intercept_lower_callback_add       (Evas_Object *obj, Evas_Object_Intercept_Lower_Cb func, const void *data) EINA_ARG_NONNULL(1, 2);
   EAPI void             *evas_object_intercept_lower_callback_del       (Evas_Object *obj, Evas_Object_Intercept_Lower_Cb func) EINA_ARG_NONNULL(1, 2);
   EAPI void              evas_object_intercept_stack_above_callback_add (Evas_Object *obj, Evas_Object_Intercept_Stack_Above_Cb func, const void *data) EINA_ARG_NONNULL(1, 2);
   EAPI void             *evas_object_intercept_stack_above_callback_del (Evas_Object *obj, Evas_Object_Intercept_Stack_Above_Cb func) EINA_ARG_NONNULL(1, 2);
   EAPI void              evas_object_intercept_stack_below_callback_add (Evas_Object *obj, Evas_Object_Intercept_Stack_Below_Cb func, const void *data) EINA_ARG_NONNULL(1, 2);
   EAPI void             *evas_object_intercept_stack_below_callback_del (Evas_Object *obj, Evas_Object_Intercept_Stack_Below_Cb func) EINA_ARG_NONNULL(1, 2);
   EAPI void              evas_object_intercept_layer_set_callback_add   (Evas_Object *obj, Evas_Object_Intercept_Layer_Set_Cb func, const void *data) EINA_ARG_NONNULL(1, 2);
   EAPI void             *evas_object_intercept_layer_set_callback_del   (Evas_Object *obj, Evas_Object_Intercept_Layer_Set_Cb func) EINA_ARG_NONNULL(1, 2);
   EAPI void              evas_object_intercept_color_set_callback_add   (Evas_Object *obj, Evas_Object_Intercept_Color_Set_Cb func, const void *data) EINA_ARG_NONNULL(1, 2);
   EAPI void             *evas_object_intercept_color_set_callback_del   (Evas_Object *obj, Evas_Object_Intercept_Color_Set_Cb func) EINA_ARG_NONNULL(1, 2);
   EAPI void              evas_object_intercept_clip_set_callback_add    (Evas_Object *obj, Evas_Object_Intercept_Clip_Set_Cb func, const void *data) EINA_ARG_NONNULL(1, 2);
   EAPI void             *evas_object_intercept_clip_set_callback_del    (Evas_Object *obj, Evas_Object_Intercept_Clip_Set_Cb func) EINA_ARG_NONNULL(1, 2);
   EAPI void              evas_object_intercept_clip_unset_callback_add  (Evas_Object *obj, Evas_Object_Intercept_Clip_Unset_Cb func, const void *data) EINA_ARG_NONNULL(1, 2);
   EAPI void             *evas_object_intercept_clip_unset_callback_del  (Evas_Object *obj, Evas_Object_Intercept_Clip_Unset_Cb func) EINA_ARG_NONNULL(1, 2);

/**
 * @defgroup Evas_Object_Specific Specific Object Functions
 *
 * Functions that work on specific objects.
 *
 */

/**
 * @defgroup Evas_Object_Rectangle Rectangle Object Functions
 *
 * Functions that operate on evas rectangle objects.
 *
 * @ingroup Evas_Object_Specific
 */

/**
 * Adds a rectangle to the given evas.
 * @param   e The given evas.
 * @return  The new rectangle object.
 */
EAPI Evas_Object      *evas_object_rectangle_add         (Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * @defgroup Evas_Object_Image Image Object Functions
 *
 * Functions used to create and manipulate image objects.
 *
 * Note - Image objects may return or accept "image data" in multiple
 * formats.  This is based on the colorspace of an object. Here is a
 * rundown on formats:
 *
 * EVAS_COLORSPACE_ARGB8888:
 *
 * This pixel format is a linear block of pixels, starting at the
 * top-left row by row until the bottom right of the image or pixel
 * region. All pixels are 32-bit unsigned int's with the high-byte
 * being alpha and the low byte being blue in the format ARGB. Alpha
 * may or may not be used by evas depending on the alpha flag of the
 * image, but if not used, should be set to 0xff anyway.
 *
 * This colorspace uses premultiplied alpha. That means that R, G and
 * B cannot exceed A in value. The conversion from non-premultiplied
 * colorspace is:
 *
 * R = (r * a) / 255; G = (g * a) / 255; B = (b * a) / 255;
 *
 * So 50% transparent blue will be: 0x80000080. This will not be
 * "dark" - just 50% transparent. Values are 0 == black, 255 == solid
 * or full red, green or blue.
 *
 * EVAS_COLORSPACE_YCBCR422P601_PL:
 *
 * This is a pointer-list indirected set of YUV (YCbCr) pixel
 * data. This means that the data returned or set is not actual pixel
 * data, but pointers TO lines of pixel data. The list of pointers
 * will first be N rows of pointers to the Y plane - pointing to the
 * first pixel at the start of each row in the Y plane. N is the
 * height of the image data in pixels. Each pixel in the Y, U and V
 * planes is 1 byte exactly, packed. The next N / 2 pointers will
 * point to rows in the U plane, and the next N / 2 pointers will
 * point to the V plane rows. U and V planes are half the horizontal
 * and vertical resolution of the Y plane.
 *
 * Row order is top to bottom and row pixels are stored left to right.
 *
 * There is a limitation that these images MUST be a multiple of 2
 * pixels in size horizontally or vertically. This is due to the U and
 * V planes being half resolution. Also note that this assumes the
 * itu601 YUV colorspace specification. This is defined for standard
 * television and mpeg streams.  HDTV may use the itu709
 * specification.
 *
 * Values are 0 to 255, indicating full or no signal in that plane
 * respectively.
 *
 * EVAS_COLORSPACE_YCBCR422P709_PL:
 *
 * Not implemented yet.
 *
 * EVAS_COLORSPACE_RGB565_A5P:
 *
 * In the process of being implemented in 1 engine only. This may change.
 *
 * This is a pointer to image data for 16-bit half-word pixel data in
 * 16bpp RGB 565 format (5 bits red, 6 bits green, 5 bits blue), with
 * the high-byte containing red and the low byte containing blue, per
 * pixel. This data is packed row by row from the top-left to the
 * bottom right.
 *
 * If the image has an alpha channel enabled there will be an extra
 * alpha plane after the color pixel plane. If not, then this data
 * will not exist and should not be accessed in any way. This plane is
 * a set of pixels with 1 byte per pixel defining the alpha values of
 * all pixels in the image from the top-left to the bottom right of
 * the image, row by row. Even though the values of the alpha pixels
 * can be 0 to 255, only values 0 through to 32 are used, 32 being
 * solid and 0 being transparent.
 *
 * RGB values can be 0 to 31 for red and blue and 0 to 63 for green,
 * with 0 being black and 31 or 63 being full red, green or blue
 * respectively. This colorspace is also pre-multiplied like
 * EVAS_COLORSPACE_ARGB8888 so:
 *
 * R = (r * a) / 32; G = (g * a) / 32; B = (b * a) / 32;
 *
 * EVAS_COLORSPACE_A8:
 *
 * The image is just a alpha mask (8 bit's per pixel).  This is used for alpha
 * masking.
 *
 * @ingroup Evas_Object_Specific
 */
typedef void (*Evas_Object_Image_Pixels_Get_Cb) (void *data, Evas_Object *o);


/**
 * Creates a new image object on the given evas.
 *
 * @param e The given evas.
 * @return The created image object.
 */
EAPI Evas_Object             *evas_object_image_add                    (Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * Creates a new image object that automatically scales on the given evas.
 *
 * This is a helper around evas_object_image_add() and
 * evas_object_image_filled_set(), it will track object resizes and apply
 * evas_object_image_fill_set() with the new geometry.
 *
 * @see evas_object_image_add()
 * @see evas_object_image_filled_set()
 * @see evas_object_image_fill_set()
 */
EAPI Evas_Object             *evas_object_image_filled_add             (Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;


/**
 * Sets the data for an image from memory to be loaded
 *
 * This is the same as evas_object_image_file_set() but the file to be loaded
 * may exist at an address in memory (the data for the file, not the filename
 * itself). The @p data at the address is copied and stored for future use, so
 * no @p data needs to be kept after this call is made. It will be managed and
 * freed for you when no longer needed. The @p size is limited to 2 gigabytes
 * in size, and must be greater than 0. A NULL @p data pointer is also invalid.
 * Set the filename to NULL to reset to empty state and have the image file
 * data freed from memory using evas_object_image_file_set().
 *
 * The @p format is optional (pass NULL if you don't need/use it). It is used
 * to help Evas guess better which loader to use for the data. It may simply
 * be the "extension" of the file as it would normally be on disk such as
 * "jpg" or "png" or "gif" etc.
 *
 * @param obj The given image object.
 * @param data The image file data address
 * @param size The size of the image file data in bytes
 * @param format The format of the file (optional), or @c NULL if not needed
 * @param key The image key in file, or @c NULL.
 */
EAPI void                     evas_object_image_memfile_set            (Evas_Object *obj, void *data, int size, char *format, char *key) EINA_ARG_NONNULL(1, 2);

/**
 * Sets the filename and key of the given image object.
 *
 * If the file supports multiple data stored in it as eet, you can
 * specify the key to be used as the index of the image in this file.
 *
 * @param obj The given image object.
 * @param file The image filename.
 * @param key The image key in file, or @c NULL.
 */
EAPI void                     evas_object_image_file_set               (Evas_Object *obj, const char *file, const char *key) EINA_ARG_NONNULL(1);

/**
 * Retrieves the filename and key of the given image object.
 *
 * @param obj The given image object.
 * @param file Location to store the image filename, or @c NULL.
 * @param key Location to store the image key, or @c NULL.
 */
EAPI void                     evas_object_image_file_get               (const Evas_Object *obj, const char **file, const char **key) EINA_ARG_NONNULL(1, 2);

/**
 * Sets how much of each border of the given image object is not
 * to be scaled.
 *
 * When rendering, the image may be scaled to fit the size of the
 * image object. This function sets what area around the border of the
 * image is not to be scaled. This sort of function is useful for
 * widget theming, where, for example, buttons may be of varying
 * sizes, but the border size must remain constant.
 *
 * The units used for @p l, @p r, @p t and @p b are output units.
 *
 * @param obj The given image object.
 * @param l Distance of the left border that is not to be stretched.
 * @param r Distance of the right border that is not to be stretched.
 * @param t Distance of the top border that is not to be stretched.
 * @param b Distance of the bottom border that is not to be stretched.
 */
EAPI void                     evas_object_image_border_set             (Evas_Object *obj, int l, int r, int t, int b) EINA_ARG_NONNULL(1);

/**
 * Retrieves how much of each border of the given image object is not
 * to be scaled.
 *
 * See @ref evas_object_image_border_set for more details.
 *
 * @param obj The given image object.
 * @param l Location to store the left border width in, or @c NULL.
 * @param r Location to store the right border width in, or @c NULL.
 * @param t Location to store the top border width in, or @c NULL.
 * @param b Location to store the bottom border width in, or @c NULL.
 */
EAPI void                     evas_object_image_border_get             (const Evas_Object *obj, int *l, int *r, int *t, int *b) EINA_ARG_NONNULL(1);

/**
 * Sets if the center part of the given image object (not the border)
 * should be drawn.
 *
 * When rendering, the image may be scaled to fit the size of the
 * image object. This function sets if the center part of the scaled
 * image is to be drawn or left completely blank, or forced to be
 * solid. Very useful for frames and decorations.
 *
 * @param obj The given image object.
 * @param fill Fill mode of the middle.
 */
EAPI void                     evas_object_image_border_center_fill_set (Evas_Object *obj, Evas_Border_Fill_Mode fill) EINA_ARG_NONNULL(1);

/**
 * Retrieves if the center of the given image object is to be drawn or
 * not.
 *
 * See @ref evas_object_image_fill_set for more details.
 *
 * @param obj The given image object.
 * @return Fill mode of the  center.
 */
EAPI Evas_Border_Fill_Mode    evas_object_image_border_center_fill_get (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

/**
 * Sets if image fill property should track object size.
 *
 * If set to true, then every evas_object_resize() will automatically
 * trigger call to evas_object_image_fill_set() with the new size so
 * image will fill the whole object area.
 *
 * @param obj The given image object.
 * @param setting whether to follow object size.
 *
 * @see evas_object_image_filled_add()
 * @see evas_object_image_fill_set()
 */
EAPI void                     evas_object_image_filled_set             (Evas_Object *obj, Eina_Bool setting) EINA_ARG_NONNULL(1);

/**
 * Retrieves if image fill property is tracking object size.
 *
 * @param obj The given image object.
 * @return 1 if it is tracking, 0 if not and evas_object_fill_set()
 * must be called manually.
 */
EAPI Eina_Bool                evas_object_image_filled_get             (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

/**
 * Sets a scale factor (multiplier) for the borders of an image
 *
 * @param obj The given image object.
 * @param scale The scale factor (default is 1.0 - i.e. no scale)
 */
EAPI void                     evas_object_image_border_scale_set       (Evas_Object *obj, double scale);

/**
 * Retrieves the border scale factor
 *
 * See evas_object_image_border_scale_set()
 *
 * @param obj The given image object.
 * @return The scale factor
 */
EAPI double                   evas_object_image_border_scale_get       (const Evas_Object *obj);

/**
 * Sets the rectangle of the given image object that the image will be
 * drawn to.
 *
 * Note that the image will be tiled around this one rectangle. To
 * have only one copy of the image drawn, @p x and @p y must be 0 and
 * @p w and @p h need to be the width and height of the image object
 * respectively.
 *
 * The default values for the fill parameters is @p x = 0, @p y = 0,
 * @p w = 32 and @p h = 32.
 *
 * @param obj The given image object.
 * @param x The X coordinate for the top left corner of the image.
 * @param y The Y coordinate for the top left corner of the image.
 * @param w The width of the image.
 * @param h The height of the image.
 */
EAPI void                     evas_object_image_fill_set               (Evas_Object *obj, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h) EINA_ARG_NONNULL(1);

/**
 * Retrieves the dimensions of the rectangle of the given image object
 * that the image will be drawn to.
 *
 * See @ref evas_object_image_fill_set for more details.
 *
 * @param obj The given image object.
 * @param x Location to store the X coordinate for the top left corner of the image in, or @c NULL.
 * @param y Location to store the Y coordinate for the top left corner of the image in, or @c NULL.
 * @param w Location to store the width of the image in, or @c NULL.
 * @param h Location to store the height of the image in, or @c NULL.
 */
EAPI void                     evas_object_image_fill_get               (const Evas_Object *obj, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h) EINA_ARG_NONNULL(1);

/**
 * Sets the tiling mode for the given evas image object's fill.
 * @param   obj   The given evas image object.
 * @param   spread One of EVAS_TEXTURE_REFLECT, EVAS_TEXTURE_REPEAT,
 * EVAS_TEXTURE_RESTRICT, or EVAS_TEXTURE_PAD.
 */
EAPI void                     evas_object_image_fill_spread_set        (Evas_Object *obj, Evas_Fill_Spread spread) EINA_ARG_NONNULL(1);

/**
 * Retrieves the spread (tiling mode) for the given image object's
 * fill.
 *
 * @param   obj The given evas image object.
 * @return  The current spread mode of the image object.
 */
EAPI Evas_Fill_Spread         evas_object_image_fill_spread_get        (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

/**
 * Sets the size of the given image object.
 *
 * This function will scale down or crop the image so that it is
 * treated as if it were at the given size. If the size given is
 * smaller than the image, it will be cropped. If the size given is
 * larger, then the image will be treated as if it were in the upper
 * left hand corner of a larger image that is otherwise transparent.
 *
 * @param obj The given image object.
 * @param w The new width of the image.
 * @param h The new height of the image.
 */
EAPI void                     evas_object_image_size_set               (Evas_Object *obj, int w, int h) EINA_ARG_NONNULL(1);

/**
 * Retrieves the size of the given image object.
 *
 * See @ref evas_object_image_size_set for more details.
 *
 * @param obj The given image object.
 * @param w Location to store the width of the image in, or @c NULL.
 * @param h Location to store the height of the image in, or @c NULL.
 */
EAPI void                     evas_object_image_size_get               (const Evas_Object *obj, int *w, int *h) EINA_ARG_NONNULL(1);

/**
 * Retrieves the row stride of the given image object,
 *
 * The row stride is the number of units between the start of a
 * row and the start of the next row.
 *
 * @param obj The given image object.
 * @return The stride of the image.
 */
EAPI int                      evas_object_image_stride_get             (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

/**
 * Retrieves a number representing any error that occurred during the last
 * load of the given image object.
 *
 * @param obj The given image object.
 * @return A value giving the last error that occurred. It should be one of
 *         the @c EVAS_LOAD_ERROR_* values.  @c EVAS_LOAD_ERROR_NONE is
 *         returned if there was no error.
 */
EAPI Evas_Load_Error          evas_object_image_load_error_get         (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

/**
 * Sets the raw image data of the given image object.
 *
 * Note that the raw data must be of the same size and colorspace of
 * the image. If data is NULL the current image data will be freed.
 *
 * @param obj The given image object.
 * @param data The raw data, or @c NULL.
 */
EAPI void                     evas_object_image_data_set               (Evas_Object *obj, void *data) EINA_ARG_NONNULL(1);

/**
 * Converts the raw image data of the given image object to the
 * specified colorspace.
 *
 * Note that this function does not modify the raw image data.  If the
 * requested colorspace is the same as the image colorspace nothing is
 * done and NULL is returned. You should use
 * evas_object_image_colorspace_get() to check the current image
 * colorspace.
 *
 * See @ref evas_object_image_colorspace_get.
 *
 * @param obj The given image object.
 * @param to_cspace The colorspace to which the image raw data will be converted.
 * @return data A newly allocated data in the format specified by to_cspace.
 */
EAPI void                    *evas_object_image_data_convert           (Evas_Object *obj, Evas_Colorspace to_cspace) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

/**
 * Get a pointer to the raw image data of the given image object.
 *
 * This function returns a pointer to an image object's internal pixel
 * buffer, for reading only or read/write. If you request it for
 * writing, the image will be marked dirty so that it gets redrawn at
 * the next update.
 *
 * This is best suited when you want to modify an existing image,
 * without changing its dimensions.
 *
 * @param obj The given image object.
 * @param for_writing Whether the data being retrieved will be modified.
 * @return The raw image data.
 */
EAPI void                    *evas_object_image_data_get               (const Evas_Object *obj, Eina_Bool for_writing) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

/**
 * Replaces the raw image data of the given image object.
 *
 * This function lets the application replace an image object's
 * internal pixel buffer with a user-allocated one. For best results,
 * you should generally first call evas_object_image_size_set() with
 * the width and height for the new buffer.
 *
 * This call is best suited for when you will be using image data with
 * different dimensions than the existing image data, if any. If you
 * only need to modify the existing image in some fashion, then using
 * evas_object_image_data_get() is probably what you are after.
 *
 * Note that the caller is responsible for freeing the buffer when
 * finished with it, as user-set image data will not be automatically
 * freed when the image object is deleted.
 *
 * See @ref evas_object_image_data_get for more details.
 *
 * @param obj The given image object.
 * @param data The raw data.
 */
EAPI void                     evas_object_image_data_copy_set          (Evas_Object *obj, void *data) EINA_ARG_NONNULL(1);

/**
 * Mark a sub-region of the given image object to be redrawn.
 *
 * This function schedules a particular rectangular region of an image
 * object to be updated (redrawn) at the next render.
 *
 * @param obj The given image object.
 * @param x X-offset of the region to be updated.
 * @param y Y-offset of the region to be updated.
 * @param w Width of the region to be updated.
 * @param h Height of the region to be updated.
 */
EAPI void                     evas_object_image_data_update_add        (Evas_Object *obj, int x, int y, int w, int h) EINA_ARG_NONNULL(1);

/**
 * Enable or disable alpha channel of the given image object.
 *
 * This function sets a flag on an image object indicating whether or
 * not to use alpha channel data. A value of 1 indicates to use alpha
 * channel data, and 0 indicates to ignore any alpha channel
 * data. Note that this has nothing to do with an object's color as
 * manipulated by evas_object_color_set().
 *
 * @param obj The given image object.
 * @param has_alpha Whether to use alpha channel data or not.
 */
EAPI void                     evas_object_image_alpha_set              (Evas_Object *obj, Eina_Bool has_alpha) EINA_ARG_NONNULL(1);

/**
 * @brief Retrieves the alpha channel setting of the given image object.
 *
 * @param obj The given image object.
 * @return Whether the alpha channel data is being used.
 *
 * This function returns 1 if the image object's alpha channel is
 * being used, or 0 otherwise.
 *
 * See @ref evas_object_image_alpha_set for more details.
 */
EAPI Eina_Bool                evas_object_image_alpha_get              (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

/**
 * Sets whether to use of high-quality image scaling algorithm
 * of the given image object.
 *
 * When enabled, a higher quality image scaling algorithm is used when
 * scaling images to sizes other than the source image. This gives
 * better results but is more computationally expensive.
 *
 * @param obj The given image object.
 * @param smooth_scale Whether to use smooth scale or not.
 */
EAPI void                     evas_object_image_smooth_scale_set       (Evas_Object *obj, Eina_Bool smooth_scale) EINA_ARG_NONNULL(1);

/**
 * Retrieves whether the given image object is using use a
 * high-quality image scaling algorithm.
 *
 * See @ref evas_object_image_smooth_scale_set for more details.
 *
 * @param obj The given image object.
 * @return Whether smooth scale is being used.
 */
EAPI Eina_Bool                evas_object_image_smooth_scale_get       (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

/**
 * Preload image in the background
 *
 * This function request the preload of the data image in the
 * background. The worked is queued before being processed.
 *
 * If image data is already loaded, it will callback
 * EVAS_CALLBACK_IMAGE_PRELOADED immediately and do nothing else.
 *
 * If cancel is set, it will remove the image from the workqueue.
 *
 * @param obj The given image object.
 * @param cancel 0 means add to the workqueue, 1 remove it.
 */
EAPI void                     evas_object_image_preload                (Evas_Object *obj, Eina_Bool cancel) EINA_ARG_NONNULL(1);

/**
 * Reload a image of the canvas.
 *
 * @param obj The given image object pointer.
 *
 * This function reloads a image of the given canvas.
 *
 */
EAPI void                     evas_object_image_reload                 (Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * Save the given image object to a file.
 *
 * Note that you should pass the filename extension when saving.  If
 * the file supports multiple data stored in it as eet, you can
 * specify the key to be used as the index of the image in this file.
 *
 * You can specify some flags when saving the image.  Currently
 * acceptable flags are quality and compress.  Eg.: "quality=100
 * compress=9"
 *
 * @param obj The given image object.
 * @param file The filename to be used to save the image.
 * @param key The image key in file, or @c NULL.
 * @param flags String containing the flags to be used.
 */
EAPI Eina_Bool                evas_object_image_save                   (const Evas_Object *obj, const char *file, const char *key, const char *flags)  EINA_ARG_NONNULL(1, 2);

/**
 * Import pixels from given source to a given canvas image object.
 *
 * @param obj The given canvas object.
 * @param pixels The pixel's source to be imported.
 *
 * This function imports pixels from a given source to a given canvas image.
 *
 */
EAPI Eina_Bool                evas_object_image_pixels_import          (Evas_Object *obj, Evas_Pixel_Import_Source *pixels) EINA_ARG_NONNULL(1, 2);

/**
 * Set the callback function to get pixels from a canva's image.
 *
 * @param obj The given canvas pointer.
 * @param func The callback function.
 * @param data The data pointer to be passed to @a func.
 *
 * This functions sets a function to be the callback function that get
 * pixes from a image of the canvas.
 *
 */
EAPI void                     evas_object_image_pixels_get_callback_set(Evas_Object *obj, Evas_Object_Image_Pixels_Get_Cb func, void *data) EINA_ARG_NONNULL(1, 2);

/**
 * Mark whether the given image object is dirty (needs to be redrawn).
 *
 * @param obj The given image object.
 * @param dirty Whether the image is dirty.
 */
EAPI void                     evas_object_image_pixels_dirty_set       (Evas_Object *obj, Eina_Bool dirty) EINA_ARG_NONNULL(1);

/**
 * Retrieves whether the given image object is dirty (needs to be redrawn).
 *
 * @param obj The given image object.
 * @return Whether the image is dirty.
 */
EAPI Eina_Bool                evas_object_image_pixels_dirty_get       (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

/**
 * Set the dpi resolution of a loaded image of the  canvas.
 *
 * @param obj The given canvas pointer.
 * @param dpi The new dpi resolution.
 *
 * This function set the dpi resolution of a given loaded canvas image.
 *
 */
EAPI void                     evas_object_image_load_dpi_set           (Evas_Object *obj, double dpi) EINA_ARG_NONNULL(1);

/**
 * Get the dpi resolution of a loaded image of the canvas.
 *
 * @param obj The given canvas pointer.
 * @return The dpi resolution of the given canvas image.
 *
 * This function returns the dpi resolution of given canvas image.
 *
 */
EAPI double                   evas_object_image_load_dpi_get           (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

/**
 * Set the size of a loaded image of the canvas.
 *
 * @param obj The given canvas object.
 * @param w The new width of the canvas image given.
 * @param h The new height of the canvas image given.
 *
 * This function sets a new size for the given canvas image.
 *
 */
EAPI void                     evas_object_image_load_size_set          (Evas_Object *obj, int w, int h) EINA_ARG_NONNULL(1);

/**
 * Get the size of a loaded image of the canvas.
 *
 * @param obj The given image object.
 * @param w The width of the canvas image given.
 * @param h The height of the canvas image given.
 *
 * This function get the size of the given canvas image.
 *
 */
EAPI void                     evas_object_image_load_size_get          (const Evas_Object *obj, int *w, int *h) EINA_ARG_NONNULL(1);

/**
 * Set the scale down of a loaded image of the canvas.
 *
 * @param obj The given image object pointer.
 * @param scale_down The scale to down value.
 *
 * This function sets the scale down of a given canvas image.
 *
 */
EAPI void                     evas_object_image_load_scale_down_set    (Evas_Object *obj, int scale_down) EINA_ARG_NONNULL(1);

/**
 * Get the scale down value of given image of the canvas.
 *
 * @param obj The given image object pointer.
 *
 * This function returns the scale down value of a given canvas image.
 *
 */
EAPI int                      evas_object_image_load_scale_down_get    (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI void                     evas_object_image_load_region_set        (Evas_Object *obj, int x, int y, int w, int h) EINA_ARG_NONNULL(1);
   EAPI void                     evas_object_image_load_region_get        (const Evas_Object *obj, int *x, int *y, int *w, int *h) EINA_ARG_NONNULL(1);

/**
 * Define if the orientation information in the image file should be honored.
 *
 * @param obj The given image object pointer.
 * @param enable @p EINA_TRUE means that it should honor the orientation information
 * @since 1.1
 */
EAPI void                     evas_object_image_load_orientation_set        (Evas_Object *obj, Eina_Bool enable) EINA_ARG_NONNULL(1);

/**
 * Get if the orientation information in the image file should be honored.
 *
 * @param obj The given image object pointer.
 * @since 1.1
 */
EAPI Eina_Bool                evas_object_image_load_orientation_get        (const Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * Set the colorspace of a given image of the canvas.
 *
 * @param obj The given image object pointer.
 * @param cspace The new color space.
 *
 * This function sets the colorspace of given canvas image.
 *
 */
EAPI void                     evas_object_image_colorspace_set         (Evas_Object *obj, Evas_Colorspace cspace) EINA_ARG_NONNULL(1);

/**
 * Get the colorspace of a given image of the canvas.
 *
 * @param obj The given image object pointer.
 * @return The colorspace of the image.
 *
 * This function returns the colorspace of given canvas image.
 *
 */
EAPI Evas_Colorspace          evas_object_image_colorspace_get         (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

/**
 * Set the native surface of a given image of the canvas
 *
 * @param obj The given canvas pointer.
 * @param surf The new native surface.
 *
 * This function sets a native surface of a given canvas image.
 *
 */
EAPI void                     evas_object_image_native_surface_set     (Evas_Object *obj, Evas_Native_Surface *surf) EINA_ARG_NONNULL(1, 2);

/**
 * Get the native surface of a given image of the canvas
 *
 * @param obj The given canvas pointer.
 * @return The native surface of the given canvas image.
 *
 * This function returns the native surface of a given canvas image.
 *
 */
EAPI Evas_Native_Surface     *evas_object_image_native_surface_get     (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

/**
 * Set the scale hint of a given image of the canvas.
 *
 * @param obj The given canvas pointer.
 * @param hint The scale hint value.
 *
 * This function sets the scale hint value of the given image of the canvas.
 *
 */
EAPI void                     evas_object_image_scale_hint_set         (Evas_Object *obj, Evas_Image_Scale_Hint hint) EINA_ARG_NONNULL(1);

/**
 * Get the scale hint of a given image of the canvas.
 *
 * @param obj The given canvas pointer.
 *
 * This function returns the scale hint value of the given image of the canvas.
 *
 */
EAPI Evas_Image_Scale_Hint    evas_object_image_scale_hint_get         (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

/**
 * Set the content hint of a given image of the canvas.
 *
 * @param obj The given canvas pointer.
 * @param hint The content hint value.
 *
 * This function sets the content hint value of the given image of the canvas.
 *
 */
EAPI void                     evas_object_image_content_hint_set       (Evas_Object *obj, Evas_Image_Content_Hint hint) EINA_ARG_NONNULL(1);

/**
 * Get the content hint of a given image of the canvas.
 *
 * @param obj The given canvas pointer.
 *
 * This function returns the content hint value of the given image of the canvas.
 *
 */
EAPI Evas_Image_Content_Hint  evas_object_image_content_hint_get       (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;


/**
 * Enable an image to be used as an alpha mask.
 *
 * This will set any flags, and discard any excess image data not used as an
 * alpha mask.
 *
 * Note there is little point in using a image as alpha mask unless it has an
 * alpha channel.
 *
 * @param obj Object to use as an alpha mask.
 * @param ismask Use image as alphamask, must be true.
 */
EAPI void                     evas_object_image_alpha_mask_set         (Evas_Object *obj, Eina_Bool ismask) EINA_ARG_NONNULL(1);

/**
 * Set the source object on a proxy object.
 *
 * The source must be another object.  The proxy will have the same base
 * appearance of the source object.  Obviously other effects may be applied to
 * the proxy, such as a map to create a reflection of the original object.
 *
 * Any existing source object will be removed.  Setting the src to NULL clears
 * the proxy object.
 *
 * You cannot set a proxy on a proxy.
 *
 * @param obj Proxy object.
 * @param src Source of the proxy.
 * @return EINA_TRUE on success, EINA_FALSE on error.
 */
EAPI Eina_Bool                evas_object_image_source_set             (Evas_Object *obj, Evas_Object *src) EINA_ARG_NONNULL(1);

/**
 * Get the current source object of an image.
 *
 * @param obj Image object
 * @return Source object, or @c NULL on error.
 */
EAPI Evas_Object             *evas_object_image_source_get             (Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * Clear the source on a proxy image.
 *
 * This is equivalent to calling evas_object_image_source_set with a NULL
 * source.
 *
 * @param obj Image object to clear source of.
 * @return EINA_TRUE on success, EINA_FALSE on error.
 */
EAPI Eina_Bool                evas_object_image_source_unset           (Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * Check if a file extention may be supported by @ref Evas_Object_Image.
 *
 * @param file The file to check
 * @return EINA_TRUE if we may be able to opeen it, EINA_FALSE if it's unlikely.
 * @since 1.1.0
 *
 * If file is a Eina_Stringshare, use directly @ref evas_object_image_extension_can_load_fast_get.
 *
 * This functions is threadsafe.
 */
EAPI Eina_Bool evas_object_image_extension_can_load_get(const char *file);

/**
 * Check if a file extention may be supported by @ref Evas_Object_Image.
 *
 * @param file The file to check, it should be an Eina_Stringshare.
 * @return EINA_TRUE if we may be able to opeen it, EINA_FALSE if it's unlikely.
 * @since 1.1.0
 *
 * This functions is threadsafe.
 */
EAPI Eina_Bool evas_object_image_extension_can_load_fast_get(const char *file);

/**
 * @defgroup Evas_Object_Text Text Object Functions
 *
 * Functions that operate on single line, single style text objects.
 *
 * For multiline and multiple style text, see @ref Evas_Object_Textblock.
 *
 * @ingroup Evas_Object_Specific
 */
   typedef enum _Evas_Text_Style_Type
     {
        /* basic styles (4 bits allocatedm use 0->10 now, 5 left) */
#define EVAS_TEXT_STYLE_MASK_BASIC 0xf
#define EVAS_TEXT_STYLE_BASIC_SET(x, s) \
   do { x = ((x) & ~EVAS_TEXT_STYLE_MASK_BASIC) | (s); } while (0)
	EVAS_TEXT_STYLE_PLAIN,
	EVAS_TEXT_STYLE_SHADOW,
	EVAS_TEXT_STYLE_OUTLINE,
	EVAS_TEXT_STYLE_SOFT_OUTLINE,
	EVAS_TEXT_STYLE_GLOW,
	EVAS_TEXT_STYLE_OUTLINE_SHADOW,
	EVAS_TEXT_STYLE_FAR_SHADOW,
	EVAS_TEXT_STYLE_OUTLINE_SOFT_SHADOW,
	EVAS_TEXT_STYLE_SOFT_SHADOW,
	EVAS_TEXT_STYLE_FAR_SOFT_SHADOW,

#define EVAS_TEXT_STYLE_MASK_SHADOW_DIRECTION (0x7 << 4)
#define EVAS_TEXT_STYLE_SHADOW_DIRECTION_SET(x, s) \
   do { x = ((x) & ~EVAS_TEXT_STYLE_MASK_SHADOW_DIRECTION) | (s); } while (0)
        /* OR these to modify shadow direction (3 bits needed) */
	EVAS_TEXT_STYLE_SHADOW_DIRECTION_BOTTOM_RIGHT = (0x0 << 4),
	EVAS_TEXT_STYLE_SHADOW_DIRECTION_BOTTOM       = (0x1 << 4),
	EVAS_TEXT_STYLE_SHADOW_DIRECTION_BOTTOM_LEFT  = (0x2 << 4),
	EVAS_TEXT_STYLE_SHADOW_DIRECTION_LEFT         = (0x3 << 4),
	EVAS_TEXT_STYLE_SHADOW_DIRECTION_TOP_LEFT     = (0x4 << 4),
	EVAS_TEXT_STYLE_SHADOW_DIRECTION_TOP          = (0x5 << 4),
	EVAS_TEXT_STYLE_SHADOW_DIRECTION_TOP_RIGHT    = (0x6 << 4),
	EVAS_TEXT_STYLE_SHADOW_DIRECTION_RIGHT        = (0x7 << 4)
     } Evas_Text_Style_Type;

/**
 * Creates a new text @c Evas_Object on the provided @c Evas canvas.
 *
 * @param e The @c Evas canvas to create the text object upon.
 *
 * @see evas_object_text_font_source_set
 * @see evas_object_text_font_set
 * @see evas_object_text_text_set
 *
 * @returns NULL on error, A pointer to a new @c Evas_Object on success.
 */
EAPI Evas_Object      *evas_object_text_add              (Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

   EAPI void              evas_object_text_font_source_set  (Evas_Object *obj, const char *font) EINA_ARG_NONNULL(1);
   EAPI const char       *evas_object_text_font_source_get  (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI void              evas_object_text_font_set         (Evas_Object *obj, const char *font, Evas_Font_Size size) EINA_ARG_NONNULL(1);

/**
 * Query evas for font information of a text @c Evas_Object.
 *
 * This function allows the font name and size of a text @c Evas_Object as
 * created with evas_object_text_add() to be queried. Be aware that the font
 * name string is still owned by Evas and should NOT have free() called on
 * it by the caller of the function.
 *
 * @param obj	The evas text object to query for font information.
 * @param font	A pointer to the location to store the font name in (may be NULL).
 * @param size	A pointer to the location to store the font size in (may be NULL).
 */
EAPI void              evas_object_text_font_get         (const Evas_Object *obj, const char **font, Evas_Font_Size *size) EINA_ARG_NONNULL(1, 2);

/**
 * Sets the text to be displayed by the given evas text object.
 * @param obj  Evas text object.
 * @param text Text to display.
 */
EAPI void              evas_object_text_text_set         (Evas_Object *obj, const char *text) EINA_ARG_NONNULL(1);

/**
 * @brief Sets the BiDi delimiters used in the textblock.
 *
 * BiDi delimiters are use for in-paragraph separation of bidi segments. This
 * is useful for example in recipients fields of e-mail clients where bidi
 * oddities can occur when mixing rtl and ltr.
 *
 * @param obj The given text object.
 * @param delim A null terminated string of delimiters, e.g ",|".
 * @since 1.1.0
 */
EAPI void              evas_object_text_bidi_delimiters_set(Evas_Object *obj, const char *delim);

/**
 * @brief Gets the BiDi delimiters used in the textblock.
 *
 * BiDi delimiters are use for in-paragraph separation of bidi segments. This
 * is useful for example in recipients fields of e-mail clients where bidi
 * oddities can occur when mixing rtl and ltr.
 *
 * @param obj The given text object.
 * @return A null terminated string of delimiters, e.g ",|". If empty, returns NULL.
 * @since 1.1.0
 */
EAPI const char       *evas_object_text_bidi_delimiters_get(const Evas_Object *obj);

/**
 * Retrieves the text currently being displayed by the given evas text object.
 * @param  obj The given evas text object.
 * @return The text currently being displayed.  Do not free it.
 */
EAPI const char       *evas_object_text_text_get         (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI Evas_Coord        evas_object_text_ascent_get       (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI Evas_Coord        evas_object_text_descent_get      (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI Evas_Coord        evas_object_text_max_ascent_get   (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI Evas_Coord        evas_object_text_max_descent_get  (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI Evas_Coord        evas_object_text_horiz_advance_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI Evas_Coord        evas_object_text_vert_advance_get (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI Evas_Coord        evas_object_text_inset_get        (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

/**
 * Retrieve position and dimension information of a character within a text @c Evas_Object.
 *
 * This function is used to obtain the X, Y, width and height of a the character
 * located at @p pos within the @c Evas_Object @p obj. @p obj must be a text object
 * as created with evas_object_text_add(). Any of the @c Evas_Coord parameters (@p cx,
 * @p cy, @p cw, @p ch) may be NULL in which case no value will be assigned to that
 * parameter.
 *
 * @param obj	The text object to retrieve position information for.
 * @param pos	The character position to request co-ordinates for.
 * @param cx	A pointer to an @c Evas_Coord to store the X value in (can be NULL).
 * @param cy	A pointer to an @c Evas_Coord to store the Y value in (can be NULL).
 * @param cw	A pointer to an @c Evas_Coord to store the Width value in (can be NULL).
 * @param ch	A pointer to an @c Evas_Coord to store the Height value in (can be NULL).
 *
 * @returns EINA_FALSE on error, EINA_TRUE on success.
 */
EAPI Eina_Bool         evas_object_text_char_pos_get     (const Evas_Object *obj, int pos, Evas_Coord *cx, Evas_Coord *cy, Evas_Coord *cw, Evas_Coord *ch) EINA_ARG_NONNULL(1);
   EAPI int               evas_object_text_char_coords_get  (const Evas_Object *obj, Evas_Coord x, Evas_Coord y, Evas_Coord *cx, Evas_Coord *cy, Evas_Coord *cw, Evas_Coord *ch) EINA_ARG_NONNULL(1);

/**
 * Returns the logical position of the last char in the text
 * up to the pos given. this is NOT the position of the last char
 * because of the possibility of RTL in the text.
 */
EAPI int               evas_object_text_last_up_to_pos   (const Evas_Object *obj, Evas_Coord x, Evas_Coord y) EINA_ARG_NONNULL(1);
   EAPI Evas_Text_Style_Type evas_object_text_style_get     (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI void              evas_object_text_style_set        (Evas_Object *obj, Evas_Text_Style_Type type) EINA_ARG_NONNULL(1);
   EAPI void              evas_object_text_shadow_color_set (Evas_Object *obj, int r, int g, int b, int a) EINA_ARG_NONNULL(1);
   EAPI void              evas_object_text_shadow_color_get (const Evas_Object *obj, int *r, int *g, int *b, int *a) EINA_ARG_NONNULL(1);
   EAPI void              evas_object_text_glow_color_set   (Evas_Object *obj, int r, int g, int b, int a) EINA_ARG_NONNULL(1);
   EAPI void              evas_object_text_glow_color_get   (const Evas_Object *obj, int *r, int *g, int *b, int *a) EINA_ARG_NONNULL(1);
   EAPI void              evas_object_text_glow2_color_set  (Evas_Object *obj, int r, int g, int b, int a) EINA_ARG_NONNULL(1);
   EAPI void              evas_object_text_glow2_color_get  (const Evas_Object *obj, int *r, int *g, int *b, int *a) EINA_ARG_NONNULL(1);
   EAPI void              evas_object_text_outline_color_set(Evas_Object *obj, int r, int g, int b, int a) EINA_ARG_NONNULL(1);
   EAPI void              evas_object_text_outline_color_get(const Evas_Object *obj, int *r, int *g, int *b, int *a) EINA_ARG_NONNULL(1);

/**
 * Gets the text style pad of a text object.
 *
 * @param obj The given text object.
 * @param l The left pad (or @c NULL).
 * @param r The right pad (or @c NULL).
 * @param t The top pad (or @c NULL).
 * @param b The bottom pad (or @c NULL).
 *
 */
EAPI void              evas_object_text_style_pad_get    (const Evas_Object *obj, int *l, int *r, int *t, int *b) EINA_ARG_NONNULL(1);

/**
 * Retrieves the direction of the text currently being displayed in the
 * text object.
 * @param  obj The given evas text object.
 * @return the direction of the text
 */
EAPI Evas_BiDi_Direction evas_object_text_direction_get  (const Evas_Object *obj) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

/**
 * @defgroup Evas_Object_Textblock Textblock Object Functions
 *
 * Functions used to create and manipulate textblock objects. Unlike
 * @ref Evas_Object_Text, these handle complex text, doing multiple
 * styles and multiline text based on HTML-like tags. Of these extra
 * features will be heavier on memory and processing cost.
 *
 * @todo put here some usage examples
 *
 * @ingroup Evas_Object_Specific
 */

/**
 * @section Evas_Object_Textblock_Tutorial Textblock Object Tutorial
 *
 * This part explains about the textblock object's API and proper usage.
 * If you want to develop textblock, you should also refer to @ref Evas_Object_Textblock_Internal.
 * The main user of the textblock object is the edje entry object in Edje, so
 * that's a good place to learn from, but I think this document is more than
 * enough, if it's not, please request for me info and I'll update it.
 *
 * @subsection textblock_intro Introduction
 * The textblock objects is, as implied, an object that can show big chunks of
 * text. Textblock supports many features including: Text formatting, automatic
 * and manual text alignment, embedding items (for example icons) and more.
 * Textblock has three important parts, the text paragraphs, the format nodes
 * and the cursors.
 *
 * @subsection textblock_cursors Textblock Object Cursors
 * A textblock Cursor @ref Evas_Textblock_Cursor is data type that represents
 * a position in a textblock. Each cursor contains information about the
 * paragraph it points to, the position in that paragraph and the object itself.
 * Cursors register to textblock objects upon creation, this means that once
 * you created a cursor, it belongs to a specific obj and you can't for example
 * copy a cursor "into" a cursor of a different object. Registered cursors
 * also have the added benefit of updating automatically upon textblock changes,
 * this means that if you have a cursor pointing to a specific character, it'll
 * still point to it even after you change the whole object completely (as long
 * as the char was not deleted), this is not possible without updating, because
 * as mentioned, each cursor holds a character position. There are many
 * functions that handle cursors, just check out the evas_textblock_cursor*
 * functions. For creation and deletion of cursors check out:
 * @see evas_object_textblock_cursor_new()
 * @see evas_textblock_cursor_free()
 * @note Cursors are generally the correct way to handle text in the textblock object, and there are enough functions to do everything you need with them (no need to get big chunks of text and processing them yourself).
 *
 * @subsection textblock_paragraphs Textblock Object Paragraphs
 * The textblock object is made out of text splitted to paragraphs (delimited
 * by the paragraph separation character). Each paragraph has many (or none)
 * format nodes associated with it which are responsible for the formatting
 * of that paragraph.
 *
 * @subsection textblock_format_nodes Textblock Object Format Nodes
 * As explained in @ref textblock_paragraphs each one of the format nodes
 * is associated with a paragraph.
 * There are two types of format nodes, visible and invisible:
 * Visible: formats that a cursor can point to, i.e formats that
 * occupy space, for example: newlines, tabs, items and etc. Some visible items
 * are made of two parts, in this case, only the opening tag is visible.
 * A closing tag (i.e a </tag> tag) should NEVER be visible.
 * Invisible: formats that don't occupy space, for example: bold and underline.
 * Being able to access format nodes is very important for some uses. For
 * example, edje uses the "<a>" format to create links in the text (and pop
 * popups above them when clicked). For the textblock object a is just a
 * formatting instruction (how to color the text), but edje utilizes the access
 * to the format nodes to make it do more.
 * For more information, take a look at all the evas_textblock_node_format_*
 * functions.
 * The translation of "<tag>" tags to actual format is done according to the
 * tags defined in the style, see @ref evas_textblock_style_set
 *
 * @subsection textblock_special_formats Special Formats
 * This section is not yet written. If you want some info about styles/formats
 * and how to use them, expedite's textblock_basic test is a great start.
 * @todo Write @textblock_special_formats
 */
   typedef struct _Evas_Textblock_Style                 Evas_Textblock_Style;
   typedef struct _Evas_Textblock_Cursor                Evas_Textblock_Cursor;
   /**
    * @typedef Evas_Object_Textblock_Node_Format
    * A format node.
    */
   typedef struct _Evas_Object_Textblock_Node_Format    Evas_Object_Textblock_Node_Format;
   typedef struct _Evas_Textblock_Rectangle             Evas_Textblock_Rectangle;

   struct _Evas_Textblock_Rectangle
     {
	Evas_Coord x, y, w, h;
     };

   typedef enum _Evas_Textblock_Text_Type
     {
	EVAS_TEXTBLOCK_TEXT_RAW,
	EVAS_TEXTBLOCK_TEXT_PLAIN,
	EVAS_TEXTBLOCK_TEXT_MARKUP
     } Evas_Textblock_Text_Type;

   typedef enum _Evas_Textblock_Cursor_Type
     {
	EVAS_TEXTBLOCK_CURSOR_UNDER,
	EVAS_TEXTBLOCK_CURSOR_BEFORE
     } Evas_Textblock_Cursor_Type;


/**
 * Adds a textblock to the given evas.
 * @param   e The given evas.
 * @return  The new textblock object.
 */
EAPI Evas_Object                 *evas_object_textblock_add(Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;


/**
 * Returns the unescaped version of escape.
 * @param escape the string to be escaped
 * @return the unescaped version of escape
 */
EAPI const char                  *evas_textblock_escape_string_get(const char *escape) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

/**
 * Returns the escaped version of the string.
 * @param string to escape
 * @param len_ret the len of the new escape
 * @return the escaped string.
 */
EAPI const char                  *evas_textblock_string_escape_get(const char *string, int *len_ret) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

/**
 * Return the unescaped version of the string between start and end.
 *
 * @param escape_start the start of the string.
 * @param escape_end the end of the string.
 * @return the unescaped version of the range
 */
EAPI const char                  *evas_textblock_escape_string_range_get(const char *escape_start, const char *escape_end) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1, 2) EINA_PURE;


/**
 * Creates a new textblock style.
 * @return  The new textblock style.
 */
EAPI Evas_Textblock_Style        *evas_textblock_style_new(void) EINA_WARN_UNUSED_RESULT EINA_MALLOC;

/**
 * Destroys a textblock style.
 * @param ts The textblock style to free.
 */
EAPI void                         evas_textblock_style_free(Evas_Textblock_Style *ts) EINA_ARG_NONNULL(1);

/**
 * Sets the style ts to the style passed as text by text.
 * Expected a string consisting of many (or none) tag='format' pairs.
 *
 * @param ts  the style to set.
 * @param text the text to parse - NOT NULL.
 * @return Returns no value.
 */
EAPI void                         evas_textblock_style_set(Evas_Textblock_Style *ts, const char *text) EINA_ARG_NONNULL(1);

/**
 * Return the text of the style ts.
 * @param ts  the style to get it's text.
 * @return the text of the style or null on error.
 */
EAPI const char                  *evas_textblock_style_get(const Evas_Textblock_Style *ts) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;


/**
 * Set the objects style to ts.
 * @param obj the Evas object to set the style to.
 * @param ts  the style to set.
 * @return Returns no value.
 */
EAPI void                         evas_object_textblock_style_set(Evas_Object *obj, Evas_Textblock_Style *ts) EINA_ARG_NONNULL(1);

/**
 * Return the style of an object.
 * @param obj  the object to get the style from.
 * @return the style of the object.
 */
EAPI const Evas_Textblock_Style  *evas_object_textblock_style_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

/**
 * @brief Set the "replacement character" to use for the given textblock object.
 *
 * @param obj The given textblock object.
 * @param ch The charset name.
 */
EAPI void                         evas_object_textblock_replace_char_set(Evas_Object *obj, const char *ch) EINA_ARG_NONNULL(1);

/**
 * @brief Get the "replacement character" for given textblock object. Returns
 * NULL if no replacement character is in use.
 *
 * @param obj The given textblock object
 * @return replacement character or @c NULL
 */
EAPI const char                  *evas_object_textblock_replace_char_get(Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

/**
 * @brief Sets the vertical alignment of text within the textblock object
 * as a whole.
 *
 * Normally alignment is 0.0 (top of object). Values given should be
 * between 0.0 and 1.0 (1.0 bottom of object, 0.5 being vertically centered
 * etc.).
 *
 * @param obj The given textblock object.
 * @param align A value between 0.0 and 1.0
 * @since 1.1.0
 */
EAPI void                         evas_object_textblock_valign_set(Evas_Object *obj, double align);

/**
 * @brief Gets the vertical alignment of a textblock
 *
 * @param obj The given textblock object.
 * @return The elignment set for the object
 * @since 1.1.0
 */
EAPI double                       evas_object_textblock_valign_get(const Evas_Object *obj);

/**
 * @brief Sets the BiDi delimiters used in the textblock.
 *
 * BiDi delimiters are use for in-paragraph separation of bidi segments. This
 * is useful for example in recipients fields of e-mail clients where bidi
 * oddities can occur when mixing rtl and ltr.
 *
 * @param obj The given textblock object.
 * @param delim A null terminated string of delimiters, e.g ",|".
 * @since 1.1.0
 */
EAPI void                         evas_object_textblock_bidi_delimiters_set(Evas_Object *obj, const char *delim);

/**
 * @brief Gets the BiDi delimiters used in the textblock.
 *
 * BiDi delimiters are use for in-paragraph separation of bidi segments. This
 * is useful for example in recipients fields of e-mail clients where bidi
 * oddities can occur when mixing rtl and ltr.
 *
 * @param obj The given textblock object.
 * @return A null terminated string of delimiters, e.g ",|". If empty, returns NULL.
 * @since 1.1.0
 */
EAPI const char                  *evas_object_textblock_bidi_delimiters_get(const Evas_Object *obj);

/**
 * @brief Sets newline mode. When true, newline character will behave
 * as a paragraph separator.
 *
 * @param obj The given textblock object.
 * @param mode EINA_TRUE for PS mode, EINA_FALSE otherwise.
 * @since 1.1.0
 */
EAPI void                         evas_object_textblock_newline_mode_set(Evas_Object *obj, Eina_Bool mode) EINA_ARG_NONNULL(1);

/**
 * @brief Gets newline mode. When true, newline character behaves
 * as a paragraph separator.
 *
 * @param obj The given textblock object.
 * @return EINA_TRUE if in PS mode, EINA_FALSE otherwise.
 * @since 1.1.0
 */
EAPI Eina_Bool                    evas_object_textblock_newline_mode_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;


/**
 * Sets the tetxblock's text to the markup text.
 *
 * @note assumes text does not include the unicode object replacement char (0xFFFC)
 *
 * @param obj  the textblock object.
 * @param text the markup text to use.
 * @return Return no value.
 */
EAPI void                         evas_object_textblock_text_markup_set(Evas_Object *obj, const char *text) EINA_ARG_NONNULL(1);

/**
 * Prepends markup to the cursor cur.
 *
 * @note assumes text does not include the unicode object replacement char (0xFFFC)
 *
 * @param cur  the cursor to prepend to.
 * @param text the markup text to prepend.
 * @return Return no value.
 */
EAPI void                         evas_object_textblock_text_markup_prepend(Evas_Textblock_Cursor *cur, const char *text) EINA_ARG_NONNULL(1, 2);

/**
 * Return the markup of the object.
 *
 * @param obj the Evas object.
 * @return the markup text of the object.
 */
EAPI const char                  *evas_object_textblock_text_markup_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);


/**
 * Return the object's main cursor.
 *
 * @param obj the object.
 * @return the obj's main cursor.
 */
EAPI const Evas_Textblock_Cursor *evas_object_textblock_cursor_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

/**
 * Create a new cursor, associate it to the obj and init it to point
 * to the start of the textblock. Association to the object means the cursor
 * will be updated when the object will change.
 *
 * @note if you need speed and you know what you are doing, it's slightly faster to just allocate the cursor yourself and not associate it. (only people developing the actual object, and not users of the object).
 *
 * @param obj the object to associate to.
 * @return the new cursor.
 */
EAPI Evas_Textblock_Cursor       *evas_object_textblock_cursor_new(Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;


/**
 * Free the cursor and unassociate it from the object.
 * @note do not use it to free unassociated cursors.
 *
 * @param cur the cursor to free.
 * @return Returns no value.
 */
EAPI void                         evas_textblock_cursor_free(Evas_Textblock_Cursor *cur) EINA_ARG_NONNULL(1);


/**
 * Sets the cursor to the start of the first text node.
 *
 * @param cur the cursor to update.
 * @return Returns no value.
 */
EAPI void                         evas_textblock_cursor_paragraph_first(Evas_Textblock_Cursor *cur) EINA_ARG_NONNULL(1);

/**
 * sets the cursor to the end of the last text node.
 *
 * @param cur the cursor to set.
 * @return Returns no value.
 */
EAPI void                         evas_textblock_cursor_paragraph_last(Evas_Textblock_Cursor *cur) EINA_ARG_NONNULL(1);

/**
 * Advances to the start of the next text node
 *
 * @param cur the cursor to update
 * @return #EINA_TRUE if it managed to advance a paragraph, #EINA_FALSE otherwise.
 */
EAPI Eina_Bool                    evas_textblock_cursor_paragraph_next(Evas_Textblock_Cursor *cur) EINA_ARG_NONNULL(1);

/**
 * Advances to the end of the previous text node
 *
 * @param cur the cursor to update
 * @return #EINA_TRUE if it managed to advance a paragraph, #EINA_FALSE otherwise.
 */
EAPI Eina_Bool                    evas_textblock_cursor_paragraph_prev(Evas_Textblock_Cursor *cur) EINA_ARG_NONNULL(1);

/**
 * Returns the
 *
 * @param obj The evas, must not be NULL.
 * @param anchor the anchor name to get
 * @return Returns the list format node corresponding to the anchor, may be null if there are none.
 */
EAPI const Eina_List             *evas_textblock_node_format_list_get(const Evas_Object *obj, const char *anchor) EINA_ARG_NONNULL(1, 2);

/**
 * Returns the first format node.
 *
 * @param obj The evas, must not be NULL.
 * @return Returns the first format node, may be null if there are none.
 */
EAPI const Evas_Object_Textblock_Node_Format *evas_textblock_node_format_first_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * Returns the last format node.
 *
 * @param obj The evas textblock, must not be NULL.
 * @return Returns the first format node, may be null if there are none.
 */
EAPI const Evas_Object_Textblock_Node_Format *evas_textblock_node_format_last_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * Returns the next format node (after n)
 *
 * @param n the current format node - not null.
 * @return Returns the next format node, may be null.
 */
EAPI const Evas_Object_Textblock_Node_Format *evas_textblock_node_format_next_get(const Evas_Object_Textblock_Node_Format *n) EINA_ARG_NONNULL(1);

/**
 * Returns the prev format node (after n)
 *
 * @param n the current format node - not null.
 * @return Returns the prev format node, may be null.
 */
EAPI const Evas_Object_Textblock_Node_Format *evas_textblock_node_format_prev_get(const Evas_Object_Textblock_Node_Format *n) EINA_ARG_NONNULL(1);

/**
 * Remove a format node and it's match. i.e, removes a <tag> </tag> pair.
 * Assumes the node is the first part of <tag> i.e, this won't work if
 * n is a closing tag.
 *
 * @param obj the Evas object of the textblock - not null.
 * @param n the current format node - not null.
 */
EAPI void                         evas_textblock_node_format_remove_pair(Evas_Object *obj, Evas_Object_Textblock_Node_Format *n) EINA_ARG_NONNULL(1, 2);

/**
 * Sets the cursor to point to the place where format points to.
 *
 * @param cur the cursor to update.
 * @param n the format node to update according.
 */
EAPI void                         evas_textblock_cursor_set_at_format(Evas_Textblock_Cursor *cur, const Evas_Object_Textblock_Node_Format *n) EINA_ARG_NONNULL(1, 2);

/**
 * Return the format node at the position pointed by cur.
 *
 * @param cur the position to look at.
 * @return the format node if found, NULL otherwise.
 * @see evas_textblock_cursor_format_is_visible_get()
 */
EAPI const Evas_Object_Textblock_Node_Format *evas_textblock_cursor_format_get(const Evas_Textblock_Cursor *cur) EINA_ARG_NONNULL(1);

/**
 * Get the text format representation of the format node.
 *
 * @param fmt the format node.
 * @return the textual format of the format node.
 */
EAPI const char                  *evas_textblock_node_format_text_get(const Evas_Object_Textblock_Node_Format *cur) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Set the cursor to point to the position of fmt.
 *
 * @param cur the cursor to update
 * @param fmt the format to update according to.
 */
EAPI void                         evas_textblock_cursor_at_format_set(Evas_Textblock_Cursor *cur, const Evas_Object_Textblock_Node_Format *fmt) EINA_ARG_NONNULL(1, 2);

/**
 * Check if the current cursor position is a visible format. This way is more
 * efficient than evas_textblock_cursor_format_get() to check for the existence
 * of a visible format.
 *
 * @param cur the cursor to look at.
 * @return #EINA_TRUE if the cursor points to a visible format, #EINA_FALSE otherwise.
 * @see evas_textblock_cursor_format_get()
 */
EAPI Eina_Bool                    evas_textblock_cursor_format_is_visible_get(const Evas_Textblock_Cursor *cur) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

/**
 * Advances to the next format node
 *
 * @param cur the cursor to be updated.
 * @return #EINA_TRUE on success #EINA_FALSE otherwise.
 */
EAPI Eina_Bool                    evas_textblock_cursor_format_next(Evas_Textblock_Cursor *cur) EINA_ARG_NONNULL(1);

/**
 * Advances to the previous format node.
 *
 * @param cur the cursor to update.
 * @return #EINA_TRUE on success #EINA_FALSE otherwise.
 */
EAPI Eina_Bool                    evas_textblock_cursor_format_prev(Evas_Textblock_Cursor *cur) EINA_ARG_NONNULL(1);

/**
 * Returns true if the cursor points to a format.
 *
 * @param cur the cursor to check.
 * @return Returns #EINA_TRUE if a cursor points to a format #EINA_FALSE otherwise.
 */
EAPI Eina_Bool                    evas_textblock_cursor_is_format(const Evas_Textblock_Cursor *cur) EINA_ARG_NONNULL(1);

/**
 * Advances 1 char forward.
 *
 * @param cur the cursor to advance.
 * @return #EINA_TRUE on success #EINA_FALSE otherwise.
 */
EAPI Eina_Bool                    evas_textblock_cursor_char_next(Evas_Textblock_Cursor *cur) EINA_ARG_NONNULL(1);

/**
 * Advances 1 char backward.
 *
 * @param cur the cursor to advance.
 * @return #EINA_TRUE on success #EINA_FALSE otherwise.
 */
EAPI Eina_Bool                    evas_textblock_cursor_char_prev(Evas_Textblock_Cursor *cur) EINA_ARG_NONNULL(1);

/**
 * Go to the first char in the node the cursor is pointing on.
 *
 * @param cur the cursor to update.
 * @return Returns no value.
 */
EAPI void                         evas_textblock_cursor_paragraph_char_first(Evas_Textblock_Cursor *cur) EINA_ARG_NONNULL(1);

/**
 * Go to the last char in a text node.
 *
 * @param cur the cursor to update.
 * @return Returns no value.
 */
EAPI void                         evas_textblock_cursor_paragraph_char_last(Evas_Textblock_Cursor *cur) EINA_ARG_NONNULL(1);

/**
 * Go to the start of the current line
 *
 * @param cur the cursor to update.
 * @return Returns no value.
 */
EAPI void                         evas_textblock_cursor_line_char_first(Evas_Textblock_Cursor *cur) EINA_ARG_NONNULL(1);

/**
 * Go to the end of the current line.
 *
 * @param cur the cursor to update.
 * @return Returns no value.
 */
EAPI void                         evas_textblock_cursor_line_char_last(Evas_Textblock_Cursor *cur) EINA_ARG_NONNULL(1);

/**
 * Return the current cursor pos.
 *
 * @param cur the cursor to take the position from.
 * @return the position or -1 on error
 */
EAPI int                          evas_textblock_cursor_pos_get(const Evas_Textblock_Cursor *cur) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

/**
 * Set the cursor pos.
 *
 * @param cur the cursor to be set.
 * @param pos the pos to set.
 */
EAPI void                         evas_textblock_cursor_pos_set(Evas_Textblock_Cursor *cur, int pos) EINA_ARG_NONNULL(1);

/**
 * Go to the start of the line passed
 *
 * @param cur cursor to update.
 * @param line numer to set.
 * @return #EINA_TRUE on success, #EINA_FALSE on error.
 */
EAPI Eina_Bool                    evas_textblock_cursor_line_set(Evas_Textblock_Cursor *cur, int line) EINA_ARG_NONNULL(1);

/**
 * Compare two cursors.
 *
 * @param cur1 the first cursor.
 * @param cur2 the second cursor.
 * @return -1 if cur1 < cur2, 0 if cur1 == cur2 and 1 otherwise.
 */
EAPI int                          evas_textblock_cursor_compare(const Evas_Textblock_Cursor *cur1, const Evas_Textblock_Cursor *cur2) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1, 2) EINA_PURE;

/**
 * Make cur_dest point to the same place as cur. Does not work if they don't
 * point to the same object.
 *
 * @param cur the source cursor.
 * @param cur_dest destination cursor.
 * @return Returns no value.
 */
EAPI void                         evas_textblock_cursor_copy(const Evas_Textblock_Cursor *cur, Evas_Textblock_Cursor *cur_dest) EINA_ARG_NONNULL(1, 2);


/**
 * Adds text to the current cursor position and set the cursor to *before*
 * the start of the text just added.
 *
 * @param cur the cursor to where to add text at.
 * @param _text the text to add.
 * @return Returns the len of the text added.
 * @see evas_textblock_cursor_text_prepend()
 */
EAPI int                          evas_textblock_cursor_text_append(Evas_Textblock_Cursor *cur, const char *text) EINA_ARG_NONNULL(1, 2);

/**
 * Adds text to the current cursor position and set the cursor to *after*
 * the start of the text just added.
 *
 * @param cur the cursor to where to add text at.
 * @param _text the text to add.
 * @return Returns the len of the text added.
 * @see evas_textblock_cursor_text_append()
 */
EAPI int                          evas_textblock_cursor_text_prepend(Evas_Textblock_Cursor *cur, const char *text) EINA_ARG_NONNULL(1, 2);


/**
 * Adds format to the current cursor position. If the format being added is a
 * visible format, add it *before* the cursor position, otherwise, add it after.
 * This behavior is because visible formats are like characters and invisible
 * should be stacked in a way that the last one is added last.
 *
 * This function works with native formats, that means that style defined
 * tags like <br> won't work here. For those kind of things use markup prepend.
 *
 * @param cur the cursor to where to add format at.
 * @param format the format to add.
 * @return Returns true if a visible format was added, false otherwise.
 * @see evas_textblock_cursor_format_prepend()
 */

/**
 * Check if the current cursor position points to the terminating null of the
 * last paragraph. (shouldn't be allowed to point to the terminating null of
 * any previous paragraph anyway.
 *
 * @param cur the cursor to look at.
 * @return #EINA_TRUE if the cursor points to the terminating null, #EINA_FALSE otherwise.
 */
EAPI Eina_Bool                    evas_textblock_cursor_format_append(Evas_Textblock_Cursor *cur, const char *format) EINA_ARG_NONNULL(1, 2);

/**
 * Adds format to the current cursor position. If the format being added is a
 * visible format, add it *before* the cursor position, otherwise, add it after.
 * This behavior is because visible formats are like characters and invisible
 * should be stacked in a way that the last one is added last.
 * If the format is visible the cursor is advanced after it.
 *
 * This function works with native formats, that means that style defined
 * tags like <br> won't work here. For those kind of things use markup prepend.
 *
 * @param cur the cursor to where to add format at.
 * @param format the format to add.
 * @return Returns true if a visible format was added, false otherwise.
 * @see evas_textblock_cursor_format_prepend()
 */
EAPI Eina_Bool                    evas_textblock_cursor_format_prepend(Evas_Textblock_Cursor *cur, const char *format) EINA_ARG_NONNULL(1, 2);

/**
 * Delete the character at the location of the cursor. If there's a format
 * pointing to this position, delete it as well.
 *
 * @param cur the cursor pointing to the current location.
 * @return Returns no value.
 */
EAPI void                         evas_textblock_cursor_char_delete(Evas_Textblock_Cursor *cur) EINA_ARG_NONNULL(1);

/**
 * Delete the range between cur1 and cur2.
 *
 * @param cur1 one side of the range.
 * @param cur2 the second side of the range
 * @return Returns no value.
 */
EAPI void                         evas_textblock_cursor_range_delete(Evas_Textblock_Cursor *cur1, Evas_Textblock_Cursor *cur2) EINA_ARG_NONNULL(1, 2);


/**
 * Return the text of the paragraph cur points to - returns the text in markup..
 *
 * @param cur the cursor pointing to the paragraph.
 * @return the text on success, NULL otherwise.
 */
EAPI const char                  *evas_textblock_cursor_paragraph_text_get(const Evas_Textblock_Cursor *cur) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

/**
 * Return the length of the paragraph, cheaper the eina_unicode_strlen()
 *
 * @param cur the position of the paragraph.
 * @return the length of the paragraph on success, -1 otehrwise.
 */
EAPI int                          evas_textblock_cursor_paragraph_text_length_get(const Evas_Textblock_Cursor *cur) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

/**
 * Return the text in the range between cur1 and cur2
 *
 * FIXME: format is currently unused, you always get markup back.
 *
 * @param cur1 one side of the range.
 * @param cur2 the other side of the range
 * @param format to be documented
 * @return the text in the range
 * @see elm_entry_markup_to_utf8()
 */
EAPI char                        *evas_textblock_cursor_range_text_get(const Evas_Textblock_Cursor *cur1, const Evas_Textblock_Cursor *cur2, Evas_Textblock_Text_Type format) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1, 2) EINA_PURE;

/**
 * Return the content of the cursor.
 *
 * @param cur the cursor
 * @return the text in the range
 */
EAPI char                        *evas_textblock_cursor_content_get(const Evas_Textblock_Cursor *cur) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;


/**
 * Returns the geometry of the cursor. Depends on the type of cursor requested.
 * This should be used instead of char_geometry_get because there are weird
 * special cases with BiDi text.
 * in '_' cursor mode (i.e a line below the char) it's the same as char_geometry
 * get, except for the case of the last char of a line which depends on the
 * paragraph direction.
 *
 * in '|' cursor mode (i.e a line between two chars) it is very varyable.
 * For example consider the following visual string:
 * "abcCBA" (ABC are rtl chars), a cursor pointing on A should actually draw
 * a '|' between the c and the C.
 *
 * @param cur the cursor.
 * @param cx the x of the cursor
 * @param cy the y of the cursor
 * @param cw the width of the cursor
 * @param ch the height of the cursor
 * @param dir the direction of the cursor, can be NULL.
 * @param ctype the type of the cursor.
 * @return line number of the char on success, -1 on error.
 */
EAPI int                          evas_textblock_cursor_geometry_get(const Evas_Textblock_Cursor *cur, Evas_Coord *cx, Evas_Coord *cy, Evas_Coord *cw, Evas_Coord *ch, Evas_BiDi_Direction *dir, Evas_Textblock_Cursor_Type ctype) EINA_ARG_NONNULL(1);

/**
 * Returns the geometry of the char at cur.
 *
 * @param cur the position of the char.
 * @param cx the x of the char.
 * @param cy the y of the char.
 * @param cw the w of the char.
 * @param ch the h of the char.
 * @return line number of the char on success, -1 on error.
 */
EAPI int                          evas_textblock_cursor_char_geometry_get(const Evas_Textblock_Cursor *cur, Evas_Coord *cx, Evas_Coord *cy, Evas_Coord *cw, Evas_Coord *ch) EINA_ARG_NONNULL(1);

/**
 * Returns the geometry of the pen at cur.
 *
 * @param cur the position of the char.
 * @param cpen_x the pen_x of the char.
 * @param cy the y of the char.
 * @param cadv the adv of the char.
 * @param ch the h of the char.
 * @return line number of the char on success, -1 on error.
 */
EAPI int                          evas_textblock_cursor_pen_geometry_get(const Evas_Textblock_Cursor *cur, Evas_Coord *cpen_x, Evas_Coord *cy, Evas_Coord *cadv, Evas_Coord *ch) EINA_ARG_NONNULL(1);

/**
 * Returns the geometry of the line at cur.
 *
 * @param cur the position of the line.
 * @param cx the x of the line.
 * @param cy the y of the line.
 * @param cw the width of the line.
 * @param ch the height of the line.
 * @return line number of the line on success, -1 on error.
 */
EAPI int                          evas_textblock_cursor_line_geometry_get(const Evas_Textblock_Cursor *cur, Evas_Coord *cx, Evas_Coord *cy, Evas_Coord *cw, Evas_Coord *ch) EINA_ARG_NONNULL(1);

/**
 * Set the position of the cursor according to the X and Y coordinates.
 *
 * @param cur the cursor to set.
 * @param x coord to set by.
 * @param y coord to set by.
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 */
EAPI Eina_Bool                    evas_textblock_cursor_char_coord_set(Evas_Textblock_Cursor *cur, Evas_Coord x, Evas_Coord y) EINA_ARG_NONNULL(1);

/**
 * Set the cursor position according to the y coord.
 *
 * @param cur the cur to be set.
 * @param y the coord to set by.
 * @return the line number found, -1 on error.
 */
EAPI int                          evas_textblock_cursor_line_coord_set(Evas_Textblock_Cursor *cur, Evas_Coord y) EINA_ARG_NONNULL(1);

/**
 * Get the geometry of a range.
 *
 * @param cur1 one side of the range.
 * @param cur2 other side of the range.
 * @return a list of Rectangles representing the geometry of the range.
 */
EAPI Eina_List                   *evas_textblock_cursor_range_geometry_get(const Evas_Textblock_Cursor *cur1, const Evas_Textblock_Cursor *cur2) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1, 2) EINA_PURE;
   EAPI Eina_Bool                    evas_textblock_cursor_format_item_geometry_get(const Evas_Textblock_Cursor *cur, Evas_Coord *cx, Evas_Coord *cy, Evas_Coord *cw, Evas_Coord *ch) EINA_ARG_NONNULL(1);


/**
 * Checks if the cursor points to the end of the line.
 *
 * @param cur the cursor to check.
 * @return #EINA_TRUE if true, #EINA_FALSE otherwise.
 */
EAPI Eina_Bool                    evas_textblock_cursor_eol_get(const Evas_Textblock_Cursor *cur) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;


/**
 * Get the geometry of a line number.
 *
 * @param obj the object.
 * @param line the line number.
 * @param cx x coord of the line.
 * @param cy y coord of the line.
 * @param cw w coord of the line.
 * @param ch h coord of the line.
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 */
EAPI Eina_Bool                    evas_object_textblock_line_number_geometry_get(const Evas_Object *obj, int line, Evas_Coord *cx, Evas_Coord *cy, Evas_Coord *cw, Evas_Coord *ch) EINA_ARG_NONNULL(1);

/**
 * Clear the textblock object.
 * @note Does *NOT* free the Evas object itself.
 *
 * @param obj the object to clear.
 * @return nothing.
 */
EAPI void                         evas_object_textblock_clear(Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * Get the formatted width and height. This calculates the actual size after restricting
 * the textblock to the current size of the object.
 * The main difference between this and @ref evas_object_textblock_size_native_get
 * is that the "native" function does not wrapping into account
 * it just calculates the real width of the object if it was placed on an
 * infinite canvas, while this function gives the size after wrapping
 * according to the size restrictions of the object.
 *
 * For example for a textblock containing the text: "You shall not pass!"
 * with no margins or padding and assuming a monospace font and a size of
 * 7x10 char widths (for simplicity) has a native size of 19x1
 * and a formatted size of 5x4.
 *
 *
 * @param obj the Evas object.
 * @param w[out] the width of the object.
 * @param h[out] the height of the object
 * @return Returns no value.
 * @see evas_object_textblock_size_native_get
 */
EAPI void                         evas_object_textblock_size_formatted_get(const Evas_Object *obj, Evas_Coord *w, Evas_Coord *h) EINA_ARG_NONNULL(1);

/**
 * Get the native width and height. This calculates the actual size without taking account
 * the current size of the object.
 * The main difference between this and @ref evas_object_textblock_size_formatted_get
 * is that the "native" function does not take wrapping into account
 * it just calculates the real width of the object if it was placed on an
 * infinite canvas, while the "formatted" function gives the size after
 * wrapping text according to the size restrictions of the object.
 *
 * For example for a textblock containing the text: "You shall not pass!"
 * with no margins or padding and assuming a monospace font and a size of
 * 7x10 char widths (for simplicity) has a native size of 19x1
 * and a formatted size of 5x4.
 *
 * @param obj the Evas object of the textblock
 * @param w[out] the width returned
 * @param h[out] the height returned
 * @return Returns no value.
 */
EAPI void                         evas_object_textblock_size_native_get(const Evas_Object *obj, Evas_Coord *w, Evas_Coord *h) EINA_ARG_NONNULL(1);
   EAPI void                         evas_object_textblock_style_insets_get(const Evas_Object *obj, Evas_Coord *l, Evas_Coord *r, Evas_Coord *t, Evas_Coord *b) EINA_ARG_NONNULL(1);

/**
 * @defgroup Evas_Line_Group Line Object Functions
 *
 * Functions used to deal with evas line objects.
 *
 * @ingroup Evas_Object_Specific
 */

/**
 * Adds a new evas line object to the given evas.
 * @param   e The given evas.
 * @return  The new evas line object.
 */
EAPI Evas_Object      *evas_object_line_add              (Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * Sets the coordinates of the end points of the given evas line object.
 * @param   obj The given evas line object.
 * @param   x1  The X coordinate of the first point.
 * @param   y1  The Y coordinate of the first point.
 * @param   x2  The X coordinate of the second point.
 * @param   y2  The Y coordinate of the second point.
 */
EAPI void              evas_object_line_xy_set           (Evas_Object *obj, Evas_Coord x1, Evas_Coord y1, Evas_Coord x2, Evas_Coord y2);

/**
 * Retrieves the coordinates of the end points of the given evas line object.
 * @param obj The given line object.
 * @param x1  Pointer to an integer in which to store the X coordinate of the
 *            first end point.
 * @param y1  Pointer to an integer in which to store the Y coordinate of the
 *            first end point.
 * @param x2  Pointer to an integer in which to store the X coordinate of the
 *            second end point.
 * @param y2  Pointer to an integer in which to store the Y coordinate of the
 *            second end point.
 */
EAPI void              evas_object_line_xy_get           (const Evas_Object *obj, Evas_Coord *x1, Evas_Coord *y1, Evas_Coord *x2, Evas_Coord *y2);

/**
 * @defgroup Evas_Object_Polygon Polygon Object Functions
 *
 * Functions that operate on evas polygon objects.
 *
 * Hint: as evas does not provide ellipse, smooth paths or circle, one
 * can calculate points and convert these to a polygon.
 *
 * @ingroup Evas_Object_Specific
 */

/**
 * Adds a new evas polygon object to the given evas.
 * @param   e The given evas.
 * @return  A new evas polygon object.
 */
EAPI Evas_Object      *evas_object_polygon_add           (Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * Adds the given point to the given evas polygon object.
 * @param obj The given evas polygon object.
 * @param x   The X coordinate of the given point.
 * @param y   The Y coordinate of the given point.
 * @ingroup Evas_Polygon_Group
 */
EAPI void              evas_object_polygon_point_add     (Evas_Object *obj, Evas_Coord x, Evas_Coord y) EINA_ARG_NONNULL(1);

/**
 * Removes all of the points from the given evas polygon object.
 * @param   obj The given polygon object.
 */
EAPI void              evas_object_polygon_points_clear  (Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * @defgroup Evas_Smart_Group Smart Functions
 *
 * Functions that deal with Evas_Smart's, creating definition
 * (classes) of objects that will have customized behavior for methods
 * like evas_object_move(), evas_object_resize(),
 * evas_object_clip_set() and others.
 *
 * These objects will accept the generic methods defined in @ref
 * Evas_Object_Group and the extensions defined in @ref
 * Evas_Smart_Object_Group. There are couple of existent smart objects
 * in Evas itself, see @ref Evas_Object_Box, @ref Evas_Object_Table
 * and @ref Evas_Smart_Object_Clipped.
 */

/**
 * @def EVAS_SMART_CLASS_VERSION
 * The version you have to put into the version field in the smart
 * class struct
 * @ingroup Evas_Smart_Group
 */
#define EVAS_SMART_CLASS_VERSION 4
/**
 * @struct _Evas_Smart_Class
 * a smart object class
 * @ingroup Evas_Smart_Group
 */
struct _Evas_Smart_Class
{
   const char *name; /**< the string name of the class */
   int         version;
   void  (*add)         (Evas_Object *o);
   void  (*del)         (Evas_Object *o);
   void  (*move)        (Evas_Object *o, Evas_Coord x, Evas_Coord y);
   void  (*resize)      (Evas_Object *o, Evas_Coord w, Evas_Coord h);
   void  (*show)        (Evas_Object *o);
   void  (*hide)        (Evas_Object *o);
   void  (*color_set)   (Evas_Object *o, int r, int g, int b, int a);
   void  (*clip_set)    (Evas_Object *o, Evas_Object *clip);
   void  (*clip_unset)  (Evas_Object *o);
   void  (*calculate)   (Evas_Object *o);
   void  (*member_add)  (Evas_Object *o, Evas_Object *child);
   void  (*member_del)  (Evas_Object *o, Evas_Object *child);

   const Evas_Smart_Class          *parent; /**< this class inherits from this parent */
   const Evas_Smart_Cb_Description *callbacks; /**< callbacks at this level, NULL terminated */
   void                            *interfaces; /**< to be used in a future near you */
   const void                      *data;
};

/**
 * @struct _Evas_Smart_Cb_Description
 *
 * Describes a callback used by a smart class
 * evas_object_smart_callback_call(), particularly useful to explain
 * to user and its code (ie: introspection) what the parameter @c
 * event_info will contain.
 *
 * @ingroup Evas_Smart_Group
 */
struct _Evas_Smart_Cb_Description
{
   const char *name; /**< callback name, ie: "changed" */

   /**
    * @brief Hint type of @c event_info parameter of Evas_Smart_Cb.
    *
    * The type string uses the pattern similar to
    *
    * http://dbus.freedesktop.org/doc/dbus-specification.html#message-protocol-signatures
    *
    * but extended to optionally include variable names within
    * brackets preceding types. Example:
    *
    * @li Structure with two integers:
    *     @c "(ii)"
    *
    * @li Structure called 'x' with two integers named 'a' and 'b':
    *     @c "[x]([a]i[b]i)"
    *
    * @li Array of integers:
    *     @c "ai"
    *
    * @li Array called 'x' of struct with two integers:
    *     @c "[x]a(ii)"
    *
    * @note This type string is used as a hint and is @b not validated
    *       or enforced anyhow. Implementors should make the best use
    *       of it to help bindings, documentation and other users of
    *       introspection features.
    */
   const char *type;
};

/**
 * @def EVAS_SMART_CLASS_INIT_NULL
 * Initializer to zero a whole Evas_Smart_Class structure.
 *
 * @see EVAS_SMART_CLASS_INIT_VERSION
 * @see EVAS_SMART_CLASS_INIT_NAME_VERSION
 * @see EVAS_SMART_CLASS_INIT_NAME_VERSION_PARENT
 * @see EVAS_SMART_CLASS_INIT_NAME_VERSION_PARENT_CALLBACKS
 * @ingroup Evas_Smart_Group
 */
#define EVAS_SMART_CLASS_INIT_NULL {NULL, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}

/**
 * @def EVAS_SMART_CLASS_INIT_VERSION
 * Initializer to zero a whole Evas_Smart_Class structure and set version.
 *
 * Similar to EVAS_SMART_CLASS_INIT_NULL, but will set version field to
 * latest EVAS_SMART_CLASS_VERSION.
 *
 * @see EVAS_SMART_CLASS_INIT_NULL
 * @see EVAS_SMART_CLASS_INIT_NAME_VERSION
 * @see EVAS_SMART_CLASS_INIT_NAME_VERSION_PARENT
 * @see EVAS_SMART_CLASS_INIT_NAME_VERSION_PARENT_CALLBACKS
 * @ingroup Evas_Smart_Group
 */
#define EVAS_SMART_CLASS_INIT_VERSION {NULL, EVAS_SMART_CLASS_VERSION, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}

/**
 * @def EVAS_SMART_CLASS_INIT_NAME_VERSION
 * Initializer to zero a whole Evas_Smart_Class structure and set name
 * and version.
 *
 * Similar to EVAS_SMART_CLASS_INIT_NULL, but will set version field to
 * latest EVAS_SMART_CLASS_VERSION and name to the specified value.
 *
 * It will keep a reference to name field as a "const char *", that is,
 * name must be available while the structure is used (hint: static or global!)
 * and will not be modified.
 *
 * @see EVAS_SMART_CLASS_INIT_NULL
 * @see EVAS_SMART_CLASS_INIT_VERSION
 * @see EVAS_SMART_CLASS_INIT_NAME_VERSION_PARENT
 * @see EVAS_SMART_CLASS_INIT_NAME_VERSION_PARENT_CALLBACKS
 * @ingroup Evas_Smart_Group
 */
#define EVAS_SMART_CLASS_INIT_NAME_VERSION(name) {name, EVAS_SMART_CLASS_VERSION, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}

/**
 * @def EVAS_SMART_CLASS_INIT_NAME_VERSION_PARENT
 * Initializer to zero a whole Evas_Smart_Class structure and set name,
 * version and parent class.
 *
 * Similar to EVAS_SMART_CLASS_INIT_NULL, but will set version field to
 * latest EVAS_SMART_CLASS_VERSION, name to the specified value and
 * parent class.
 *
 * It will keep a reference to name field as a "const char *", that is,
 * name must be available while the structure is used (hint: static or global!)
 * and will not be modified. Similarly, parent reference will be kept.
 *
 * @see EVAS_SMART_CLASS_INIT_NULL
 * @see EVAS_SMART_CLASS_INIT_VERSION
 * @see EVAS_SMART_CLASS_INIT_NAME_VERSION
 * @see EVAS_SMART_CLASS_INIT_NAME_VERSION_PARENT_CALLBACKS
 * @ingroup Evas_Smart_Group
 */
#define EVAS_SMART_CLASS_INIT_NAME_VERSION_PARENT(name, parent) {name, EVAS_SMART_CLASS_VERSION, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, parent, NULL, NULL}

/**
 * @def EVAS_SMART_CLASS_INIT_NAME_VERSION_PARENT_CALLBACKS
 * Initializer to zero a whole Evas_Smart_Class structure and set name,
 * version, parent class and callbacks definition.
 *
 * Similar to EVAS_SMART_CLASS_INIT_NULL, but will set version field to
 * latest EVAS_SMART_CLASS_VERSION, name to the specified value, parent
 * class and callbacks at this level.
 *
 * It will keep a reference to name field as a "const char *", that is,
 * name must be available while the structure is used (hint: static or global!)
 * and will not be modified. Similarly, parent and callbacks reference
 * will be kept.
 *
 * @see EVAS_SMART_CLASS_INIT_NULL
 * @see EVAS_SMART_CLASS_INIT_VERSION
 * @see EVAS_SMART_CLASS_INIT_NAME_VERSION
 * @see EVAS_SMART_CLASS_INIT_NAME_VERSION_PARENT
 * @ingroup Evas_Smart_Group
 */
#define EVAS_SMART_CLASS_INIT_NAME_VERSION_PARENT_CALLBACKS(name, parent, callbacks) {name, EVAS_SMART_CLASS_VERSION, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, parent, callbacks, NULL}

/**
 * @def EVAS_SMART_SUBCLASS_NEW
 *
 * Convenience macro to subclass a Smart Class.
 *
 * This macro saves some typing when writing a Smart Class derived from
 * another one. In order to work, the user needs to provide some functions
 * adhering to the following guidelines.
 *  - @<prefix@>_smart_set_user(): the internal _smart_set function will call
 *    this one provided by the user after inheriting everything from the
 *    parent, which should take care of setting the right member functions
 *    for the class.
 *  - @<prefix@>_parent_sc: pointer to the smart class of the parent. When calling
 *    parent functions from overloaded ones, use this global variable.
 *  - @<prefix@>_smart_class_new(): this function returns the Evas_Smart needed
 *    to create smart objects with this class, should be called by the public
 *    _add() function.
 *  - If this new class should be subclassable as well, a public _smart_set()
 *    function is desirable to fill the class used as parent by the children.
 *    It's up to the user to provide this interface, which will most likely
 *    call @<prefix@>_smart_set() to get the job done.
 *
 * @param smart_name The name used for the Smart Class. e.g: "Evas_Object_Box".
 * @param prefix Prefix used for all variables and functions defined.
 * @param api_type Type of the structure used as API for the Smart Class. Either Evas_Smart_Class or something derived from it.
 * @param parent_type Type of the parent class API.
 * @param parent_func Function that gets the parent class. e.g: evas_object_box_smart_class_get().
 * @param cb_desc Array of callback descriptions for this Smart Class.
 *
 * @ingroup Evas_Smart_Group
 */
#define EVAS_SMART_SUBCLASS_NEW(smart_name, prefix, api_type, parent_type, parent_func, cb_desc) \
  static const parent_type * prefix##_parent_sc = NULL;			\
  static void prefix##_smart_set_user(api_type *api);			\
  static void prefix##_smart_set(api_type *api)				\
  {									\
     Evas_Smart_Class *sc;						\
     if (!(sc = (Evas_Smart_Class *)api))				\
       return;								\
     if (!prefix##_parent_sc)						\
       prefix##_parent_sc = parent_func();				\
     evas_smart_class_inherit(sc, (const Evas_Smart_Class *)prefix##_parent_sc); \
     prefix##_smart_set_user(api);					\
  }									\
  static Evas_Smart * prefix##_smart_class_new(void)			\
  {									\
     static Evas_Smart *smart = NULL;					\
     static api_type api;						\
     if (!smart)							\
       {								\
	  Evas_Smart_Class *sc = (Evas_Smart_Class *)&api;		\
	  memset(&api, 0, sizeof(api_type));				\
	  sc->version = EVAS_SMART_CLASS_VERSION;			\
	  sc->name = smart_name;					\
	  sc->callbacks = cb_desc;					\
	  prefix##_smart_set(&api);					\
	  smart = evas_smart_class_new(sc);				\
       }								\
     return smart;							\
  }

/**
 * @def EVAS_SMART_DATA_ALLOC
 * Convenience macro to allocate smart data only if needed.
 *
 * When writing a subclassable smart object, the .add function will need
 * to check if the smart private data was already allocated by some child
 * object or not. This macro makes it easier to do it.
 *
 * @param o Evas object passed to the .add function
 * @param priv_type The type of the data to allocate
 * @ingroup Evas_Smart_Group
 */
#define EVAS_SMART_DATA_ALLOC(o, priv_type) \
   priv_type *priv; \
   priv = evas_object_smart_data_get(o); \
   if (!priv) { \
      priv = (priv_type *)calloc(1, sizeof(priv_type)); \
      if (!priv) return; \
      evas_object_smart_data_set(o, priv); \
   }


/**
 * Free an Evas_Smart
 *
 * If this smart was created using evas_smart_class_new(), the associated
 * Evas_Smart_Class will not be freed.
 *
 * @param s the Evas_Smart to free
 *
 */
EAPI void                             evas_smart_free                     (Evas_Smart *s) EINA_ARG_NONNULL(1);

/**
 * Creates an Evas_Smart from an Evas_Smart_Class.
 *
 * @param sc the smart class definition
 * @return an Evas_Smart
 */
EAPI Evas_Smart                      *evas_smart_class_new                (const Evas_Smart_Class *sc) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * Get the Evas_Smart_Class of an Evas_Smart
 *
 * @param s the Evas_Smart
 * @return the Evas_Smart_Class
 */
EAPI const Evas_Smart_Class          *evas_smart_class_get                (const Evas_Smart *s) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;


/**
 * @brief Get the data pointer set on an Evas_Smart.
 *
 * @param s Evas_Smart
 *
 * This data pointer is set either as the final parameter to
 * evas_smart_new or as the data field in the Evas_Smart_Class passed
 * in to evas_smart_class_new
 */
EAPI void                            *evas_smart_data_get                 (const Evas_Smart *s) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

/**
 * Get the callbacks known by this Evas_Smart.
 *
 * This is likely different from Evas_Smart_Class::callbacks as it
 * will contain the callbacks of all class hierarchy sorted, while the
 * direct smart class member refers only to that specific class and
 * should not include parent's.
 *
 * If no callbacks are known, this function returns @c NULL.
 *
 * The array elements and thus their contents will be reference to
 * original values given to evas_smart_new() as
 * Evas_Smart_Class::callbacks.
 *
 * The array is sorted by name. The last array element is the @c NULL
 * pointer and is not counted in @a count. Loop iterations can check
 * any of these cases.
 *
 * @param s the Evas_Smart.
 * @param count returns the number of elements in returned array.
 * @return the array with callback descriptions known by this class,
 *         its size is returned in @a count parameter. It should not
 *         be modified anyhow. If no callbacks are known, @c NULL is
 *         returned. The array is sorted by name and elements refer to
 *         the original value given to evas_smart_new().
 *
 * @note objects may provide per-instance callbacks, use
 *       evas_object_smart_callbacks_descriptions_get() to get those
 *       as well.
 * @see evas_object_smart_callbacks_descriptions_get()
 */
EAPI const Evas_Smart_Cb_Description **evas_smart_callbacks_descriptions_get(const Evas_Smart *s, unsigned int *count) EINA_ARG_NONNULL(1, 1);


/**
 * Find callback description for callback called @a name.
 *
 * @param s the Evas_Smart.
 * @param name name of desired callback, must @b not be @c NULL.  The
 *        search have a special case for @a name being the same
 *        pointer as registered with Evas_Smart_Cb_Description, one
 *        can use it to avoid excessive use of strcmp().
 * @return reference to description if found, @c NULL if not found.
 */
EAPI const Evas_Smart_Cb_Description *evas_smart_callback_description_find(const Evas_Smart *s, const char *name) EINA_ARG_NONNULL(1, 2) EINA_PURE;


/**
 * Sets one class to inherit from the other.
 *
 * Copy all function pointers, set @c parent to @a parent_sc and copy
 * everything after sizeof(Evas_Smart_Class) present in @a parent_sc,
 * using @a parent_sc_size as reference.
 *
 * This is recommended instead of a single memcpy() since it will take
 * care to not modify @a sc name, version, callbacks and possible
 * other members.
 *
 * @param sc child class.
 * @param parent_sc parent class, will provide attributes.
 * @param parent_sc_size size of parent_sc structure, child should be at least
 *        this size. Everything after @c Evas_Smart_Class size is copied
 *        using regular memcpy().
 */
EAPI Eina_Bool                        evas_smart_class_inherit_full       (Evas_Smart_Class *sc, const Evas_Smart_Class *parent_sc, unsigned int parent_sc_size) EINA_ARG_NONNULL(1, 2);

/**
 * Get the number of users of the smart instance
 * 
 * @param s The Evas_Smart to get the usage count of
 * @return The number of uses of the smart instance
 * 
 * This function tells you how many more uses of the smart instance are in
 * existence. This should be used before freeing/clearing any of the
 * Evas_Smart_Class that was used to create the smart instance. The smart
 * instance will refer to data in the Evas_Smart_Class used to create it and
 * thus you cannot remove the original data until all users of it are gone.
 * When the usage count goes to 0, you can evas_smart_free() the smart
 * instance @p s and remove from memory any of the Evas_Smart_Class that
 * was used to create the smart instance, if you desire. Removing it from
 * memory without doing this will cause problems (crashes, undefined
 * behavior etc. etc.), so either never remove the original 
 * Evas_Smart_Class data from memory (have it be a constant structure and
 * data), or use this API call and be very careful.
 */
EAPI int                              evas_smart_usage_get(const Evas_Smart *s);
         
  /**
   * @def evas_smart_class_inherit
   * Easy to use version of evas_smart_class_inherit_full().
   *
   * This version will use sizeof(parent_sc), copying everything.
   *
   * @param sc child class, will have methods copied from @a parent_sc
   * @param parent_sc parent class, will provide contents to be copied.
   * @return 1 on success, 0 on failure.
   * @ingroup Evas_Smart_Group
   */
#define evas_smart_class_inherit(sc, parent_sc) evas_smart_class_inherit_full(sc, parent_sc, sizeof(*parent_sc))

/**
 * @defgroup Evas_Smart_Object_Group Smart Object Functions
 *
 * Functions dealing with evas smart objects (instances).
 *
 * Smart objects are groupings of primitive Evas objects that behave as a
 * cohesive group. For instance, a file manager icon may be a smart object
 * composed of an image object, a text label and two rectangles that appear
 * behind the image and text when the icon is selected. As a smart object,
 * the normal evas api could be used on the icon object.
 *
 * @see @ref Evas_Smart_Group for class definitions.
 */

/**
 * Instantiates a new smart object described by @p s.
 *
 * @param e the evas on which to add the object
 * @param s the Evas_Smart describing the smart object
 * @return a new Evas_Object
 * @ingroup Evas_Smart_Object_Group
 */
EAPI Evas_Object      *evas_object_smart_add             (Evas *e, Evas_Smart *s) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1, 2) EINA_MALLOC;

/**
 * Set an Evas object as a member of a smart object.
 *
 * @param obj The member object
 * @param smart_obj The smart object
 *
 * Members will automatically be stacked and layered with the smart object.
 * The various stacking function will operate on members relative to the
 * other members instead of the entire canvas.
 *
 * Non-member objects can not interleave a smart object's members.
 *
 * @ingroup Evas_Smart_Object_Group
 */
EAPI void              evas_object_smart_member_add      (Evas_Object *obj, Evas_Object *smart_obj) EINA_ARG_NONNULL(1, 2);

/**
 * Removes a member object from a smart object.
 *
 * @param obj the member object
 * @ingroup Evas_Smart_Object_Group
 *
 * This removes a member object from a smart object. The object will still
 * be on the canvas, but no longer associated with whichever smart object
 * it was associated with.
 *
 */
EAPI void              evas_object_smart_member_del      (Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * Gets the smart parent of an Evas_Object
 * @param obj the Evas_Object you want to get the parent
 * @return Returns the smart parent of @a obj, or @c NULL if @a obj is not a smart member of another Evas_Object
 * @ingroup Evas_Smart_Object_Group
 */
EAPI Evas_Object      *evas_object_smart_parent_get      (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

/**
 * Checks the Smart type of the object and its parents
 * @param obj the Evas_Object to check the type of
 * @param type the type to check for
 * @return EINA_TRUE if @a obj or any of its parents if of type @a type, EINA_FALSE otherwise
 * @ingroup Evas_Smart_Object_Group
 */
EAPI Eina_Bool         evas_object_smart_type_check      (const Evas_Object *obj, const char *type) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1, 2) EINA_PURE;

/**
 * Checks the Smart type of the object and its parents using pointer comparison
 * @param obj the Evas_Object to check the type of
 * @param type the type to check for. Must be the name pointer in the smart class used to create the object
 * @return EINA_TRUE if @a obj or any of its parents if of type @a type, EINA_FALSE otherwise
 * @ingroup Evas_Smart_Object_Group
 */
EAPI Eina_Bool         evas_object_smart_type_check_ptr  (const Evas_Object *obj, const char *type) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1, 2) EINA_PURE;

/**
 * Gets the list of the member objects of an Evas_Object
 * @param obj the Evas_Object you want to get the list of member objects
 * @return Returns the list of the member objects of @a obj.
 * The returned list should be freed with eina_list_free() when you no longer need it
 */
EAPI Eina_List        *evas_object_smart_members_get     (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

/**
 * Get the Evas_Smart from which @p obj was created.
 *
 * @param obj a smart object
 * @return the Evas_Smart
 * @ingroup Evas_Smart_Object_Group
 */
EAPI Evas_Smart       *evas_object_smart_smart_get       (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

/**
 * Retrieve user data stored on a smart object.
 *
 * @param obj The smart object
 * @return A pointer to data stored using evas_object_smart_data_set(), or
 *         NULL if none has been set.
 * @ingroup Evas_Smart_Object_Group
 */
EAPI void             *evas_object_smart_data_get        (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

/**
 * Store a pointer to user data for a smart object.
 *
 * @param obj The smart object
 * @param data A pointer to user data
 * @ingroup Evas_Smart_Object_Group
 */
EAPI void              evas_object_smart_data_set        (Evas_Object *obj, void *data) EINA_ARG_NONNULL(1);

/**
 * Add a callback for the smart event specified by @p event.
 *
 * @param obj a smart object
 * @param event the event name
 * @param func the callback function
 * @param data user data to be passed to the callback function
 * @ingroup Evas_Smart_Object_Group
 */
EAPI void              evas_object_smart_callback_add    (Evas_Object *obj, const char *event, Evas_Smart_Cb func, const void *data) EINA_ARG_NONNULL(1, 2, 3);

/**
 * Remove a smart callback
 *
 * Removes a callback that was added by evas_object_smart_callback_add()
 *
 * @param obj a smart object
 * @param event the event name
 * @param func the callback function
 * @return the data pointer
 * @ingroup Evas_Smart_Object_Group
 */
EAPI void             *evas_object_smart_callback_del    (Evas_Object *obj, const char *event, Evas_Smart_Cb func) EINA_ARG_NONNULL(1, 2, 3);

/**
 * Call any smart callbacks on @p obj for @p event.
 *
 * @param obj the smart object
 * @param event the event name
 * @param event_info an event specific struct of info to pass to the callback
 *
 * This should be called internally in the smart object when some specific
 * event has occurred. The documentation for the smart object should include
 * a list of possible events and what type of @p event_info to expect.
 *
 * @ingroup Evas_Smart_Object_Group
 */
EAPI void              evas_object_smart_callback_call   (Evas_Object *obj, const char *event, void *event_info) EINA_ARG_NONNULL(1, 2);


/**
 * Set smart object instance callbacks descriptions.
 *
 * These descriptions are hints to be used by introspection and are
 * not enforced in any way.
 *
 * It will not be checked if instance callbacks descriptions have the
 * same name as another in class. Both are kept in different arrays
 * and users of evas_object_smart_callbacks_descriptions_get() should
 * handle this case as they wish.
 *
 * @param obj The smart object
 * @param descriptions NULL terminated (name != NULL) array with
 *        descriptions.  Array elements will not be modified, but
 *        reference to them and their contents will be made, so this
 *        array should be kept alive during object lifetime.
 * @return 1 on success, 0 on failure.
 * @ingroup Evas_Smart_Object_Group
 *
 * @note while instance callbacks descriptions are possible, they are
 *       not recommended. Use class callbacks descriptions instead as they
 *       make user's life simpler and will use less memory as descriptions
 *       and arrays will be shared among all instances.
 */
EAPI Eina_Bool         evas_object_smart_callbacks_descriptions_set(Evas_Object *obj, const Evas_Smart_Cb_Description *descriptions) EINA_ARG_NONNULL(1);

/**
 * Get the callbacks descriptions known by this smart object.
 *
 * This call retrieves processed callbacks descriptions for both
 * instance and class. These arrays are sorted by description's name
 * and are @c NULL terminated, so both @a class_count and
 * @a instance_count can be ignored, the terminator @c NULL is not
 * counted in these values.
 *
 * @param obj the smart object.
 * @param class_descriptions where to store class callbacks
 *        descriptions array, if any is known. If no descriptions are
 *        known, @c NULL is returned. This parameter may be @c NULL if
 *        it is not of interest.
 * @param class_count returns how many class callbacks descriptions
 *        are known.
 * @param instance_descriptions where to store instance callbacks
 *        descriptions array, if any is known. If no descriptions are
 *        known, @c NULL is returned. This parameter may be @c NULL if
 *        it is not of interest.
 * @param instance_count returns how many instance callbacks
 *        descriptions are known.
 *
 * @note if just class descriptions are of interest, try
 *       evas_smart_callbacks_descriptions_get() instead.
 *
 * @see evas_smart_callbacks_descriptions_get()
 * @ingroup Evas_Smart_Object_Group
 */
EAPI void              evas_object_smart_callbacks_descriptions_get(const Evas_Object *obj, const Evas_Smart_Cb_Description ***class_descriptions, unsigned int *class_count, const Evas_Smart_Cb_Description ***instance_descriptions, unsigned int *instance_count) EINA_ARG_NONNULL(1);

/**
 * Find callback description for callback called @a name.
 *
 * @param obj the smart object.
 * @param name name of desired callback, must @b not be @c NULL.  The
 *        search have a special case for @a name being the same
 *        pointer as registered with Evas_Smart_Cb_Description, one
 *        can use it to avoid excessive use of strcmp().
 * @param class_description pointer to return class description or @c
 *        NULL if not found. If parameter is @c NULL, no search will
 *        be done on class descriptions.
 * @param instance_description pointer to return instance description
 *        or @c NULL if not found. If parameter is @c NULL, no search
 *        will be done on instance descriptions.
 * @return reference to description if found, @c NULL if not found.
 */
EAPI void              evas_object_smart_callback_description_find(const Evas_Object *obj, const char *name, const Evas_Smart_Cb_Description **class_description, const Evas_Smart_Cb_Description **instance_description) EINA_ARG_NONNULL(1, 2);


/**
 * Mark smart object as changed, dirty.
 *
 * This will inform the scene that it changed and needs to be redraw, also
 * setting need_recalculate on the given object.
 *
 * @see evas_object_smart_need_recalculate_set().
 *
 * @ingroup Evas_Smart_Object_Group
 */
EAPI void              evas_object_smart_changed         (Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * Set the need_recalculate flag of given smart object.
 *
 * If this flag is set then calculate() callback (method) of the given
 * smart object will be called, if one is provided, during render phase
 * usually evas_render(). After this step, this flag will be automatically
 * unset.
 *
 * If no calculate() is provided, this flag will be left unchanged.
 *
 * @note just setting this flag will not make scene dirty and evas_render()
 *       will have no effect. To do that, use evas_object_smart_changed(),
 *       that will automatically call this function with 1 as parameter.
 *
 * @param obj the smart object
 * @param value if one want to set or unset the need_recalculate flag.
 *
 * @ingroup Evas_Smart_Object_Group
 */
EAPI void              evas_object_smart_need_recalculate_set(Evas_Object *obj, Eina_Bool value) EINA_ARG_NONNULL(1);

/**
 * Get the current value of need_recalculate flag.
 *
 * @note this flag will be unset during the render phase, after calculate()
 *       is called if one is provided.  If no calculate() is provided, then
 *       the flag will be left unchanged after render phase.
 *
 * @param obj the smart object
 * @return if flag is set or not.
 *
 * @ingroup Evas_Smart_Object_Group
 */
EAPI Eina_Bool         evas_object_smart_need_recalculate_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

/**
 * Call user provided calculate() and unset need_calculate.
 *
 * @param obj the smart object
 *
 * @ingroup Evas_Smart_Object_Group
 */
EAPI void              evas_object_smart_calculate       (Evas_Object *obj) EINA_ARG_NONNULL(1);


/**
 * Call user provided calculate() and unset need_calculate on all objects.
 *
 * @param e The canvas to calculate all objects in
 *
 * @ingroup Evas_Smart_Object_Group
 */
EAPI void              evas_smart_objects_calculate      (Evas *e);

/**
 * @defgroup Evas_Smart_Object_Clipped Clipped Smart Object
 *
 * Clipped smart object is a base to construct other smart objects
 * that based on the concept of having an internal clipper that is
 * applied to all its other children. This clipper will control the
 * visibility, clipping and color of sibling objects (remember that
 * the clipping is recursive, and clipper color modulates the color of
 * its clippees). By default, this base will also move children
 * relatively to the parent, and delete them when parent is
 * deleted. In other words, it is the base for simple object grouping.
 *
 * @see evas_object_smart_clipped_smart_set()
 *
 * @ingroup Evas_Smart_Object_Group
 */

/**
 * Every subclass should provide this at the beginning of their own
 * data set with evas_object_smart_data_set().
 */
  typedef struct _Evas_Object_Smart_Clipped_Data Evas_Object_Smart_Clipped_Data;
  struct _Evas_Object_Smart_Clipped_Data
  {
     Evas_Object *clipper;
     Evas        *evas;
  };


/**
 * Get the clipper object for the given clipped smart object.
 *
 * @param obj the clipped smart object to retrieve the associated clipper.
 * @return the clipper object.
 *
 * @see evas_object_smart_clipped_smart_add()
 */
EAPI Evas_Object            *evas_object_smart_clipped_clipper_get   (Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

/**
 * Set smart class callbacks so it implements the "Clipped Smart Object".
 *
 * This call will assign all the required methods of Evas_Smart_Class,
 * if one wants to "subclass" it, call this function and later
 * override values, if one wants to call the original method, save it
 * somewhere, example:
 *
 * @code
 * static Evas_Smart_Class parent_sc = EVAS_SMART_CLASS_INIT_NULL;
 *
 * static void my_class_smart_add(Evas_Object *o)
 * {
 *    parent_sc.add(o);
 *    evas_object_color_set(evas_object_smart_clipped_clipper_get(o),
 *                          255, 0, 0, 255);
 * }
 *
 * Evas_Smart_Class *my_class_new(void)
 * {
 *    static Evas_Smart_Class sc = EVAS_SMART_CLASS_INIT_NAME_VERSION("MyClass");
 *    if (!parent_sc.name)
 *      {
 *         evas_object_smart_clipped_smart_set(&sc);
 *         parent_sc = sc;
 *         sc.add = my_class_smart_add;
 *      }
 *    return &sc;
 * }
 * @endcode
 *
 * Default behavior is:
 *  - add: creates a hidden clipper with "infinite" size;
 *  - del: delete all children objects;
 *  - move: move all objects relative relatively;
 *  - resize: not defined;
 *  - show: if there are children objects, show clipper;
 *  - hide: hides clipper;
 *  - color_set: set the color of clipper;
 *  - clip_set: set clipper of clipper;
 *  - clip_unset: unset the clipper of clipper;
 */
EAPI void                    evas_object_smart_clipped_smart_set     (Evas_Smart_Class *sc) EINA_ARG_NONNULL(1);

/**
 * Get a pointer to the Clipped Smart Class to use for proper inheritance
 */
EAPI const Evas_Smart_Class *evas_object_smart_clipped_class_get     (void) EINA_CONST;


/**
 * Moves all children objects relative to given offset.
 *
 * @param obj the smart Evas object to use.
 * @param dx horizontal offset.
 * @param dy vertical offset.
 */
EAPI void                    evas_object_smart_move_children_relative(Evas_Object *obj, Evas_Coord dx, Evas_Coord dy) EINA_ARG_NONNULL(1);

/**
 * @defgroup Evas_Object_Box Box (Sequence) Smart Object.
 *
 * Convenience smart object that packs children as a sequence using
 * a layout function specified by user. There are a couple of helper
 * layout functions, all of them using children size hints to define
 * their size and alignment inside their cell space.
 *
 * @see @ref Evas_Object_Group_Size_Hints
 *
 * @ingroup Evas_Smart_Object_Group
 */
/**
 * @typedef Evas_Object_Box_Api
 * Smart Class extension providing extra box requirements.
 * @ingroup Evas_Object_Box
 */
   typedef struct _Evas_Object_Box_Api        Evas_Object_Box_Api;
/**
 * @typedef Evas_Object_Box_Data
 * Smart instance data providing box requirements.
 * @ingroup Evas_Object_Box
 */
   typedef struct _Evas_Object_Box_Data       Evas_Object_Box_Data;
/**
 * @typedef Evas_Object_Box_Option
 * The base structure for a box option.
 * @ingroup Evas_Object_Box
 */
   typedef struct _Evas_Object_Box_Option     Evas_Object_Box_Option;
   typedef void (*Evas_Object_Box_Layout) (Evas_Object *o, Evas_Object_Box_Data *priv, void *user_data);

/**
 * @def EVAS_OBJECT_BOX_API_VERSION
 * @ingroup Evas_Object_Box
 */
#define EVAS_OBJECT_BOX_API_VERSION 1
/**
 * @struct _Evas_Object_Box_Api
 *
 * This structure should be used by any class that wants to inherit
 * from box to provide custom behavior not allowed only by providing a
 * layout function with evas_object_box_layout_set().
 *
 * @extends Evas_Smart_Class
 * @ingroup Evas_Object_Box
 */
   struct _Evas_Object_Box_Api
   {
      Evas_Smart_Class          base;
      int                       version;
      Evas_Object_Box_Option *(*append)           (Evas_Object *o, Evas_Object_Box_Data *priv, Evas_Object *child);
      Evas_Object_Box_Option *(*prepend)          (Evas_Object *o, Evas_Object_Box_Data *priv, Evas_Object *child);
      Evas_Object_Box_Option *(*insert_before)    (Evas_Object *o, Evas_Object_Box_Data *priv, Evas_Object *child, const Evas_Object *reference);
      Evas_Object_Box_Option *(*insert_after)     (Evas_Object *o, Evas_Object_Box_Data *priv, Evas_Object *child, const Evas_Object *reference);
      Evas_Object_Box_Option *(*insert_at)        (Evas_Object *o, Evas_Object_Box_Data *priv, Evas_Object *child, unsigned int pos);
      Evas_Object            *(*remove)           (Evas_Object *o, Evas_Object_Box_Data *priv, Evas_Object *child);
      Evas_Object            *(*remove_at)        (Evas_Object *o, Evas_Object_Box_Data *priv, unsigned int pos);
      Eina_Bool               (*property_set)     (Evas_Object *o, Evas_Object_Box_Option *opt, int property, va_list args);
      Eina_Bool               (*property_get)     (Evas_Object *o, Evas_Object_Box_Option *opt, int property, va_list args);
      const char             *(*property_name_get)(Evas_Object *o, int property);
      int                     (*property_id_get)  (Evas_Object *o, const char *name);
      Evas_Object_Box_Option *(*option_new)       (Evas_Object *o, Evas_Object_Box_Data *priv, Evas_Object *child);
      void                    (*option_free)      (Evas_Object *o, Evas_Object_Box_Data *priv, Evas_Object_Box_Option *opt);
   };

/**
 * @def EVAS_OBJECT_BOX_API_INIT
 * Initializer for whole Evas_Object_Box_Api structure.
 *
 * @param smart_class_init initializer to use for the "base" field
 * (Evas_Smart_Class).
 *
 * @see EVAS_SMART_CLASS_INIT_NULL
 * @see EVAS_SMART_CLASS_INIT_VERSION
 * @see EVAS_SMART_CLASS_INIT_NAME_VERSION
 * @see EVAS_OBJECT_BOX_API_INIT_NULL
 * @see EVAS_OBJECT_BOX_API_INIT_VERSION
 * @see EVAS_OBJECT_BOX_API_INIT_NAME_VERSION
 * @ingroup Evas_Object_Box
 */
#define EVAS_OBJECT_BOX_API_INIT(smart_class_init) {smart_class_init, EVAS_OBJECT_BOX_API_VERSION, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}

/**
 * @def EVAS_OBJECT_BOX_API_INIT_NULL
 * Initializer to zero a whole Evas_Object_Box_Api structure.
 *
 * @see EVAS_OBJECT_BOX_API_INIT_VERSION
 * @see EVAS_OBJECT_BOX_API_INIT_NAME_VERSION
 * @see EVAS_OBJECT_BOX_API_INIT
 * @ingroup Evas_Object_Box
 */
#define EVAS_OBJECT_BOX_API_INIT_NULL EVAS_OBJECT_BOX_API_INIT(EVAS_SMART_CLASS_INIT_NULL)

/**
 * @def EVAS_OBJECT_BOX_API_INIT_VERSION
 * Initializer to zero a whole Evas_Object_Box_Api structure and set version.
 *
 * Similar to EVAS_OBJECT_BOX_API_INIT_NULL, but will set version field of
 * Evas_Smart_Class (base field) to latest EVAS_SMART_CLASS_VERSION
 *
 * @see EVAS_OBJECT_BOX_API_INIT_NULL
 * @see EVAS_OBJECT_BOX_API_INIT_NAME_VERSION
 * @see EVAS_OBJECT_BOX_API_INIT
 * @ingroup Evas_Object_Box
 */
#define EVAS_OBJECT_BOX_API_INIT_VERSION EVAS_OBJECT_BOX_API_INIT(EVAS_SMART_CLASS_INIT_VERSION)

/**
 * @def EVAS_OBJECT_BOX_API_INIT_NAME_VERSION
 * Initializer to zero a whole Evas_Object_Box_Api structure and set
 * name and version.
 *
 * Similar to EVAS_OBJECT_BOX_API_INIT_NULL, but will set version field of
 * Evas_Smart_Class (base field) to latest EVAS_SMART_CLASS_VERSION and name
 * to the specific value.
 *
 * It will keep a reference to name field as a "const char *", that is,
 * name must be available while the structure is used (hint: static or global!)
 * and will not be modified.
 *
 * @see EVAS_OBJECT_BOX_API_INIT_NULL
 * @see EVAS_OBJECT_BOX_API_INIT_VERSION
 * @see EVAS_OBJECT_BOX_API_INIT
 * @ingroup Evas_Object_Box
 */
#define EVAS_OBJECT_BOX_API_INIT_NAME_VERSION(name) EVAS_OBJECT_BOX_API_INIT(EVAS_SMART_CLASS_INIT_NAME_VERSION(name))

/**
 * @struct _Evas_Object_Box_Data
 *
 * This structure augments clipped smart object's instance data,
 * providing extra members required by generic box implementation. If
 * a subclass inherits from #Evas_Object_Box_Api then it may augment
 * #Evas_Object_Box_Data to fit its own needs.
 *
 * @extends Evas_Object_Smart_Clipped_Data
 * @ingroup Evas_Object_Box
 */
   struct _Evas_Object_Box_Data
   {
      Evas_Object_Smart_Clipped_Data   base;
      const Evas_Object_Box_Api       *api;
      struct {
	 double                        h, v;
      } align;
      struct {
	 Evas_Coord                    h, v;
      } pad;
      Eina_List                       *children;
      struct {
	 Evas_Object_Box_Layout        cb;
	 void                         *data;
	 void                        (*free_data)(void *data);
      } layout;
      Eina_Bool                        layouting : 1;
      Eina_Bool                        children_changed : 1;
   };

   struct _Evas_Object_Box_Option
   {
      Evas_Object *obj;
      Eina_Bool    max_reached:1;
      Eina_Bool    min_reached:1;
      Evas_Coord   alloc_size;
   };


/**
 * Set the default box @a api struct (Evas_Object_Box_Api)
 * with the default values. May be used to extend that API.
 */
EAPI void                       evas_object_box_smart_set                             (Evas_Object_Box_Api *api) EINA_ARG_NONNULL(1);

/**
 * Get Box Smart Class for inheritance purposes
 */
EAPI const Evas_Object_Box_Api *evas_object_box_smart_class_get                       (void) EINA_CONST;

/**
 * Set a 'calculate' callback (@a cb) to the @a o box's smart class,
 * which here defines its genre (horizontal, vertical, homogeneous,
 * etc.).
 */
EAPI void                       evas_object_box_layout_set                            (Evas_Object *o, Evas_Object_Box_Layout cb, const void *data, void (*free_data)(void *data)) EINA_ARG_NONNULL(1, 2);


/**
 * Create a new box.
 *
 * Its layout function must be set via evas_object_box_layout_set()
 * (defaults to evas_object_box_layout_horizontal()).  The other
 * properties of the box must be set/retrieved via
 * evas_object_box_{h,v}_{align,padding}_{get,set)().
 */
EAPI Evas_Object               *evas_object_box_add                                   (Evas *evas) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * Create a box that is child of a given element @a parent.
 *
 * @see evas_object_box_add()
 */
EAPI Evas_Object               *evas_object_box_add_to                                (Evas_Object *parent) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;


/**
 * Layout function which sets the box @a o to a (basic) horizontal
 * box.  @a priv must be the smart data of the box.
 *
 * The object's overall behavior is controlled by its properties,
 * which are set by the evas_object_box_{h,v}_{align,padding}_set()
 * family of functions.  The properties of the elements in the box --
 * set by evas_object_size_hint_{align,padding,weight}_set() functions
 * -- also control the way this function works.
 *
 * \par box's properties:
 * @c align_h controls the horizontal alignment of the child objects
 * relative to the containing box. When set to 0, children are aligned
 * to the left. A value of 1 lets them aligned to the right border.
 * Values in between align them proportionally.  Note that if the size
 * required by the children, which is given by their widths and the @c
 * padding_h property of the box, is bigger than the container width,
 * the children will be displayed out of its bounds.  A negative value
 * of @c align_h makes the box to *justify* its children. The padding
 * between them, in this case, is corrected so that the leftmost one
 * touches the left border and the rightmost one touches the right
 * border (even if they must overlap).  The @c align_v and @c
 * padding_v properties of the box don't contribute to its behaviour
 * when this layout is chosen.
 *
 * \par Child element's properties:
 * @c align_x does not influence the box's behavior.  @c padding_l and
 * @c padding_r sum up to the container's horizontal padding between
 * elements.  The child's @c padding_t, @c padding_b and @c align_y
 * properties apply for padding/positioning relative to the overall
 * height of the box. Finally, there is the @c weight_x property,
 * which, if set to a non-zero value, tells the container that the
 * child width is not pre-defined.  If the container can't accommodate
 * all its children, it sets the widths of the children *with weights*
 * to sizes as small as they can all fit into it.  If the size
 * required by the children is less than the available, the box
 * increases its children's (which have weights) widths as to fit the
 * remaining space.  The @c weight_x property, besides telling the
 * element is resizable, gives a *weight* for the resizing process.
 * The parent box will try to distribute (or take off) widths
 * accordingly to the *normalized* list of weigths: most weighted
 * children remain/get larger in this process than the least ones.
 * @c weight_y does not influence the layout.
 *
 * If one desires that, besides having weights, child elements must be
 * resized bounded to a minimum or maximum size, their size hint
 * properties must be set (by the
 * evas_object_size_hint_{min,max}_set() functions.
 */
EAPI void                       evas_object_box_layout_horizontal                     (Evas_Object *o, Evas_Object_Box_Data *priv, void *data) EINA_ARG_NONNULL(1, 2);

/**
 * Layout function which sets the box @a o to a (basic) vertical box.
 * @a priv must be the smart data of the box.
 *
 * This function behaves analogously to
 * evas_object_box_layout_horizontal().  The description of its
 * behaviour can be derived from that function's documentation.
 */
EAPI void                       evas_object_box_layout_vertical                       (Evas_Object *o, Evas_Object_Box_Data *priv, void *data) EINA_ARG_NONNULL(1, 2);

/**
 * Layout function which sets the box @a o to a *homogeneous* vertical
 * box.  @a priv must be the smart data of the box.
 *
 * This function behaves analogously to
 * evas_object_box_layout_homogeneous_horizontal().  The description
 * of its behaviour can be derived from that function's documentation.
 */
EAPI void                       evas_object_box_layout_homogeneous_vertical           (Evas_Object *o, Evas_Object_Box_Data *priv, void *data) EINA_ARG_NONNULL(1, 2);

/**
 * Layout function which sets the box @a o to a *homogeneous*
 * horizontal box.  @a priv must be the smart data of the box.
 *
 * In a homogeneous horizontal box, its width is divided equally
 * between the contained objects.  The box's overall behavior is
 * controlled by its properties, which are set by the
 * evas_object_box_{h,v}_{align,padding}_set() family of functions.
 * The properties of the elements in the box -- set by
 * evas_object_size_hint_{align,padding,weight}_set() functions --
 * also control the way this function works.
 *
 * \par box's properties:
 * @c align_h has no influence on the box for this layout.  @c
 * padding_h tells the box to draw empty spaces of that size, in
 * pixels, between the (still equal) child objects's cells.  The @c
 * align_v and @c padding_v properties of the box don't contribute to
 * its behaviour when this layout is chosen.
 *
 * \par Child element's properties:
 * @c padding_l and @c padding_r sum up to the required width of the
 * child element.  The @c align_x property tells the relative position
 * of this overall child width in its allocated cell (0 to extreme
 * left, 1 to extreme right).  A value of -1.0 to @c align_x makes the
 * box try to resize this child element to the exact width of its cell
 * (respecting the min and max hints on the child's width *and*
 * accounting its horizontal padding properties).  The child's @c
 * padding_t, @c padding_b and @c align_y properties apply for
 * padding/positioning relative to the overall height of the box. A
 * value of -1.0 to @c align_y makes the box try to resize this child
 * element to the exact height of its parent (respecting the max hint
 * on the child's height).
 */
EAPI void                       evas_object_box_layout_homogeneous_horizontal         (Evas_Object *o, Evas_Object_Box_Data *priv, void *data) EINA_ARG_NONNULL(1, 2);

/**
 * Layout function which sets the box @a o to a *max size-homogeneous*
 * horizontal box.  @a priv must be the smart data of the box.
 *
 * In a max size-homogeneous horizontal box, the equal sized cells
 * reserved for the child objects have the width of the space required
 * by the largest child (in width). The box's overall behavior is
 * controlled by its properties, which are set by the
 * evas_object_box_{h,v}_{align,padding}_set() family of functions.
 * The properties of the elements in the box -- set by
 * evas_object_size_hint_{align,padding,weight}_set() functions --
 * also control the way this function works.
 *
 * \par box's properties:
 * @c padding_h tells the box to draw empty spaces of that size, in
 * pixels, between the child objects's cells.  @c align_h controls the
 * horizontal alignment of the child objects relative to the
 * containing box. When set to 0, children are aligned to the left. A
 * value of 1 lets them aligned to the right border.  Values in
 * between align them proportionally. A negative value of @c align_h
 * makes the box to *justify* its children cells. The padding between
 * them, in this case, is corrected so that the leftmost one touches
 * the left border and the rightmost one touches the right border
 * (even if they must overlap).  The @c align_v and @c padding_v
 * properties of the box don't contribute to its behaviour when this
 * layout is chosen.
 *
 * \par Child element's properties:
 * @c padding_l and @c padding_r sum up to the required width of the
 * child element. The @c align_x property tells the relative position
 * of this overall child width in its allocated cell (0 to extreme
 * left, 1 to extreme right).  A value of -1.0 to @c align_x makes the
 * box try to resize this child element to the exact width of its cell
 * (respecting the min and max hints on the child's width *and*
 * accounting its horizontal padding properties).  The child's @c
 * padding_t, @c padding_b and @c align_y properties apply for
 * padding/positioning relative to the overall height of the box. A
 * value of -1.0 to @c align_y makes the box try to resize this child
 * element to the exact height of its parent (respecting the max hint
 * on the child's height).
 */
EAPI void                       evas_object_box_layout_homogeneous_max_size_horizontal(Evas_Object *o, Evas_Object_Box_Data *priv, void *data) EINA_ARG_NONNULL(1, 2);

/**
 * Layout function which sets the box @a o to a *max size-homogeneous*
 * vertical box.  @a priv must be the smart data of the box.
 *
 * This function behaves analogously to
 * evas_object_box_layout_homogeneous_max_size_horizontal().  The
 * description of its behaviour can be derived from that function's
 * documentation.
 */
EAPI void                       evas_object_box_layout_homogeneous_max_size_vertical  (Evas_Object *o, Evas_Object_Box_Data *priv, void *data) EINA_ARG_NONNULL(1, 2);

/**
 * Layout function which sets the box @a o to a *flow* horizontal box.
 * @a priv must be the smart data of the box.
 *
 * In a flow horizontal box, the box's child elements are placed in
 * rows (think of text as an analogy). A row has as much elements as
 * can fit into the box's width.  The box's overall behavior is
 * controlled by its properties, which are set by the
 * evas_object_box_{h,v}_{align,padding}_set() family of functions.
 * The properties of the elements in the box -- set by
 * evas_object_size_hint_{align,padding,weight}_set() functions --
 * also control the way this function works.
 *
 * \par box's properties:
 * @c padding_h tells the box to draw empty spaces of that size, in
 * pixels, between the child objects's cells.  @c align_h dictates the
 * horizontal alignment of the rows (0 to left align them, 1 to right
 * align).  A value of -1.0 to @c align_h lets the rows *justified*
 * horizontally.  @c align_v controls the vertical alignment of the
 * entire set of rows (0 to top, 1 to bottom).  A value of -1.0 to @c
 * align_v makes the box to *justify* the rows vertically. The padding
 * between them, in this case, is corrected so that the first row
 * touches the top border and the last one touches the bottom border
 * (even if they must overlap). @c padding_v has no influence on the
 * layout.
 *
 * \par Child element's properties:
 * @c padding_l and @c padding_r sum up to the required width of the
 * child element.  The @c align_x property has no influence on the
 * layout. The child's @c padding_t and @c padding_b sum up to the
 * required height of the child element and is the only means (besides
 * row justifying) of setting space between rows.  Note, however, that
 * @c align_y dictates positioning relative to the *largest height*
 * required by a child object in the actual row.
 */
EAPI void                       evas_object_box_layout_flow_horizontal                (Evas_Object *o, Evas_Object_Box_Data *priv, void *data) EINA_ARG_NONNULL(1, 2);

/**
 * Layout function which sets the box @a o to a *flow* vertical box.
 * @a priv must be the smart data of the box.
 *
 * This function behaves analogously to
 * evas_object_box_layout_flow_horizontal().  The description of its
 * behaviour can be derived from that function's documentation.
 */
EAPI void                       evas_object_box_layout_flow_vertical                  (Evas_Object *o, Evas_Object_Box_Data *priv, void *data) EINA_ARG_NONNULL(1, 2);

/**
 * Layout function which sets the box @a o to set all children to the
 * size of the object.  @a priv must be the smart data of the box.
 *
 * In a stack box, all children will be given the same size and they
 * will be stacked on above the other, so the first object will be the
 * bottom most.
 *
 * \par box's properties:
 * No box option is used.
 *
 * \par Child  element's   properties:
 * @c padding_l and @c padding_r sum up to the required width of the
 * child element.  The @c align_x property tells the relative position
 * of this overall child width in its allocated cell (0 to extreme
 * left, 1 to extreme right).  A value of -1.0 to @c align_x makes the
 * box try to resize this child element to the exact width of its cell
 * (respecting the min and max hints on the child's width *and*
 * accounting its horizontal padding properties).  Same applies to
 * vertical axis.
 */
EAPI void                       evas_object_box_layout_stack                          (Evas_Object *o, Evas_Object_Box_Data *priv, void *data) EINA_ARG_NONNULL(1, 2);


/**
 * Set the alignment of the whole bounding box of contents.
 */
EAPI void                       evas_object_box_align_set                             (Evas_Object *o, double horizontal, double vertical) EINA_ARG_NONNULL(1);

/**
 * Get alignment of the whole bounding box of contents.
 */
EAPI void                       evas_object_box_align_get                             (const Evas_Object *o, double *horizontal, double *vertical) EINA_ARG_NONNULL(1);

/**
 * Set the space (padding) between cells.
 */
EAPI void                       evas_object_box_padding_set                           (Evas_Object *o, Evas_Coord horizontal, Evas_Coord vertical) EINA_ARG_NONNULL(1);

/**
 * Get the (space) padding between cells.
 */
EAPI void                       evas_object_box_padding_get                           (const Evas_Object *o, Evas_Coord *horizontal, Evas_Coord *vertical) EINA_ARG_NONNULL(1);


/**
 * Append a new object @a child to the box @a o. On error, @c NULL is
 * returned.
 */
EAPI Evas_Object_Box_Option    *evas_object_box_append                                (Evas_Object *o, Evas_Object *child) EINA_ARG_NONNULL(1, 2);

/**
 * Prepend a new object @a child to the box @a o. On error, @c NULL is
 * returned.
 */
EAPI Evas_Object_Box_Option    *evas_object_box_prepend                               (Evas_Object *o, Evas_Object *child) EINA_ARG_NONNULL(1, 2);

/**
 * Prepend a new object @a child to the box @c o relative to element @a
 * reference. If @a reference is not contained in the box or any other
 * error occurs, @c NULL is returned.
 */
EAPI Evas_Object_Box_Option    *evas_object_box_insert_before                         (Evas_Object *o, Evas_Object *child, const Evas_Object *reference) EINA_ARG_NONNULL(1, 2, 3);

/**
 * Append a new object @a child to the box @c o relative to element @a
 * reference. If @a reference is not contained in the box or any other
 * error occurs, @c NULL is returend.
 */
EAPI Evas_Object_Box_Option    *evas_object_box_insert_after                          (Evas_Object *o, Evas_Object *child, const Evas_Object *referente) EINA_ARG_NONNULL(1, 2, 3);

/**
 * Insert a new object @a child to the box @a o at position @a pos. On
 * error, @c NULL is returned.
 */
EAPI Evas_Object_Box_Option    *evas_object_box_insert_at                             (Evas_Object *o, Evas_Object *child, unsigned int pos) EINA_ARG_NONNULL(1, 2);

/**
 * Remove an object @a child from the box @a o. On error, @c 0 is
 * returned.
 */
EAPI Eina_Bool                  evas_object_box_remove                                (Evas_Object *o, Evas_Object *child) EINA_ARG_NONNULL(1, 2);

/**
 * Remove an object from the box @a o which occupies position @a
 * pos. On error, @c 0 is returned.
 */
EAPI Eina_Bool                  evas_object_box_remove_at                             (Evas_Object *o, unsigned int pos) EINA_ARG_NONNULL(1);

/**
 * Remove all child objects.
 * @return 0 on errors
 */
EAPI Eina_Bool                  evas_object_box_remove_all                            (Evas_Object *o, Eina_Bool clear) EINA_ARG_NONNULL(1);

/**
 * Get an iterator to walk the list of children for the box.
 *
 * @note Do not remove or delete objects while walking the list.
 */
EAPI Eina_Iterator             *evas_object_box_iterator_new                          (const Evas_Object *o) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * Get an accessor to get random access to the list of children for the box.
 *
 * @note Do not remove or delete objects while walking the list.
 */
EAPI Eina_Accessor             *evas_object_box_accessor_new                          (const Evas_Object *o) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * Get the list of children for the box.
 *
 * @note This is a duplicate of the list kept by the box internally.
 *       It's up to the user to destroy it when it no longer needs it.
 *       It's possible to remove objects from the box when walking this
 *       list, but these removals won't be reflected on it.
 */
EAPI Eina_List                 *evas_object_box_children_get                          (const Evas_Object *o) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;


/**
 * Get the name of the property of the child elements of the box @a o
 * whose id is @a property. On error, @c NULL is returned.
 */
EAPI const char                *evas_object_box_option_property_name_get              (Evas_Object *o, int property) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

/**
 * Get the id of the property of the child elements of the box @a o
 * whose name is @a name. On error, @c -1 is returned.
 */
EAPI int                        evas_object_box_option_property_id_get                (Evas_Object *o, const char *name) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1, 2) EINA_PURE;

/**
 * Set the property (with id @a property) of the child element of the
 * box @a o whose property struct is @a opt. The property's values
 * must be the last arguments and their type *must* match that of the
 * property itself. On error, @c 0 is returned.
 */
EAPI Eina_Bool                  evas_object_box_option_property_set                   (Evas_Object *o, Evas_Object_Box_Option *opt, int property, ...) EINA_ARG_NONNULL(1, 2);

/**
 * Set the property (with id @a property) of the child element of the
 * box @a o whose property struct is @a opt. The property's values
 * must be the args which the va_list @a args is initialized with and
 * their type *must* match that of the property itself. On error, @c 0
 * is returned.
 */
EAPI Eina_Bool                  evas_object_box_option_property_vset                  (Evas_Object *o, Evas_Object_Box_Option *opt, int property, va_list args) EINA_ARG_NONNULL(1, 2);

/**
 * Get the property (with id @a property) of the child element of the
 * box @a o whose property struct is @a opt. The last arguments must
 * be addresses of variables with the same type of that property. On
 * error, @c 0 is returned.
 */
EAPI Eina_Bool                  evas_object_box_option_property_get                   (Evas_Object *o, Evas_Object_Box_Option *opt, int property, ...) EINA_ARG_NONNULL(1, 2);

/**
 * Get the property (with id @a property) of the child element of the
 * box @a o whose property struct is @a opt. The args which the
 * va_list @a args is initialized with must be addresses of variables
 * with the same type of that property. On error, @c 0 is returned.
 */
EAPI Eina_Bool                  evas_object_box_option_property_vget                  (Evas_Object *o, Evas_Object_Box_Option *opt, int property, va_list args) EINA_ARG_NONNULL(1, 2);

/**
 * @defgroup Evas_Object_Table Table Smart Object.
 *
 * Convenience smart object that packs children using a tabular
 * layout using children size hints to define their size and
 * alignment inside their cell space.
 *
 * @see @ref Evas_Object_Group_Size_Hints
 *
 * @ingroup Evas_Smart_Object_Group
 */

/**
 * Create a new table.
 *
 * It's set to non-homogeneous by default, add children with
 * evas_object_table_pack().
 */
EAPI Evas_Object                        *evas_object_table_add             (Evas *evas) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * Create a table that is child of a given element @a parent.
 *
 * @see evas_object_table_add()
 */
EAPI Evas_Object                        *evas_object_table_add_to          (Evas_Object *parent) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * Set how this table should layout children.
 *
 * @todo consider aspect hint and respect it.
 *
 * @par EVAS_OBJECT_TABLE_HOMOGENEOUS_NONE
 * If table does not use homogeneous mode then columns and rows will
 * be calculated based on hints of individual cells. This operation
 * mode is more flexible, but more complex and heavy to calculate as
 * well. @b Weight properties are handled as a boolean
 * expand. Negative alignment will be considered as 0.5.
 *
 * @todo @c EVAS_OBJECT_TABLE_HOMOGENEOUS_NONE should balance weight.
 *
 * @par EVAS_OBJECT_TABLE_HOMOGENEOUS_TABLE
 * When homogeneous is relative to table the own table size is divided
 * equally among children, filling the whole table area. That is, if
 * table has @c WIDTH and @c COLUMNS, each cell will get <tt>WIDTH /
 * COLUMNS</tt> pixels. If children have minimum size that is larger
 * than this amount (including padding), then it will overflow and be
 * aligned respecting the alignment hint, possible overlapping sibling
 * cells. @b Weight hint is used as a boolean, if greater than zero it
 * will make the child expand in that axis, taking as much space as
 * possible (bounded to maximum size hint). Negative alignment will be
 * considered as 0.5.
 *
 * @par EVAS_OBJECT_TABLE_HOMOGENEOUS_ITEM
 * When homogeneous is relative to item it means the greatest minimum
 * cell size will be used. That is, if no element is set to expand,
 * the table will have its contents to a minimum size, the bounding
 * box of all these children will be aligned relatively to the table
 * object using evas_object_table_align_get(). If the table area is
 * too small to hold this minimum bounding box, then the objects will
 * keep their size and the bounding box will overflow the box area,
 * still respecting the alignment. @b Weight hint is used as a
 * boolean, if greater than zero it will make that cell expand in that
 * axis, toggling the <b>expand mode</b>, which makes the table behave
 * much like @b EVAS_OBJECT_TABLE_HOMOGENEOUS_TABLE, except that the
 * bounding box will overflow and items will not overlap siblings. If
 * no minimum size is provided at all then the table will fallback to
 * expand mode as well.
 */
EAPI void                                evas_object_table_homogeneous_set (Evas_Object *o, Evas_Object_Table_Homogeneous_Mode homogeneous) EINA_ARG_NONNULL(1);

/**
 * Get the current layout homogeneous mode.
 *
 * @see evas_object_table_homogeneous_set()
 */
EAPI Evas_Object_Table_Homogeneous_Mode  evas_object_table_homogeneous_get (const Evas_Object *o) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

/**
 * Set padding between cells.
 */
EAPI void                                evas_object_table_padding_set     (Evas_Object *o, Evas_Coord horizontal, Evas_Coord vertical) EINA_ARG_NONNULL(1);

/**
 * Get padding between cells.
 */
EAPI void                                evas_object_table_padding_get     (const Evas_Object *o, Evas_Coord *horizontal, Evas_Coord *vertical) EINA_ARG_NONNULL(1);

/**
 * Set the alignment of the whole bounding box of contents.
 */
EAPI void                                evas_object_table_align_set       (Evas_Object *o, double horizontal, double vertical) EINA_ARG_NONNULL(1);

/**
 * Get alignment of the whole bounding box of contents.
 */
EAPI void                                evas_object_table_align_get       (const Evas_Object *o, double *horizontal, double *vertical) EINA_ARG_NONNULL(1);

/**
 * Sets the mirrored mode of the table. In mirrored mode the table items go
 * from right to left instead of left to right. That is, 1,1 is top right, not
 * to left.
 *
 * @param obj The table object.
 * @param mirrored the mirrored mode to set
 * @since 1.1.0
 */
EAPI void                                evas_object_table_mirrored_set    (Evas_Object *o, Eina_Bool mirrored) EINA_ARG_NONNULL(1);

/**
 * Gets the mirrored mode of the table. In mirrored mode the table items go
 * from right to left instead of left to right. That is, 1,1 is top right, not
 * to left.
 *
 * @param obj The table object.
 * @return EINA_TRUE if it's a mirrored table, EINA_FALSE otherwise.
 * @since 1.1.0
 */

/**
 * Get a child from the table using its coordinates
 *
 * @note This does not take into account col/row spanning
 */
EAPI Eina_Bool                           evas_object_table_mirrored_get    (const Evas_Object *o) EINA_ARG_NONNULL(1);


/**
 * Get packing location of a child of table
 *
 * @param o The given table object.
 * @param child The child object to add.
 * @param col pointer to store relative-horizontal position to place child.
 * @param row pointer to store relative-vertical position to place child.
 * @param colspan pointer to store how many relative-horizontal position to use for this child.
 * @param rowspan pointer to store how many relative-vertical position to use for this child.
 *
 * @return 1 on success, 0 on failure.
 * @since 1.1.0
 */
EAPI Eina_Bool                           evas_object_table_pack_get(Evas_Object *o, Evas_Object *child, unsigned short *col, unsigned short *row, unsigned short *colspan, unsigned short *rowspan);
         
/**
 * Add a new child to a table object or set its current packing.
 *
 * @param o The given table object.
 * @param child The child object to add.
 * @param col relative-horizontal position to place child.
 * @param row relative-vertical position to place child.
 * @param colspan how many relative-horizontal position to use for this child.
 * @param rowspan how many relative-vertical position to use for this child.
 *
 * @return 1 on success, 0 on failure.
 */
EAPI Eina_Bool                           evas_object_table_pack            (Evas_Object *o, Evas_Object *child, unsigned short col, unsigned short row, unsigned short colspan, unsigned short rowspan) EINA_ARG_NONNULL(1, 2);

/**
 * Remove child from table.
 *
 * @note removing a child will immediately call a walk over children in order
 *       to recalculate numbers of columns and rows. If you plan to remove
 *       all children, use evas_object_table_clear() instead.
 *
 * @return 1 on success, 0 on failure.
 */
EAPI Eina_Bool                           evas_object_table_unpack          (Evas_Object *o, Evas_Object *child) EINA_ARG_NONNULL(1, 2);

/**
 * Faster way to remove all child objects from a table object.
 *
 * @param o The given table object.
 * @param clear if true, it will delete just removed children.
 */
EAPI void                                evas_object_table_clear           (Evas_Object *o, Eina_Bool clear) EINA_ARG_NONNULL(1);


/**
 * Get the number of columns and rows this table takes.
 *
 * @note columns and rows are virtual entities, one can specify a table
 *       with a single object that takes 4 columns and 5 rows. The only
 *       difference for a single cell table is that paddings will be
 *       accounted proportionally.
 */
EAPI void                                evas_object_table_col_row_size_get(const Evas_Object *o, int *cols, int *rows) EINA_ARG_NONNULL(1);

/**
 * Get an iterator to walk the list of children for the table.
 *
 * @note Do not remove or delete objects while walking the list.
 */
EAPI Eina_Iterator                      *evas_object_table_iterator_new    (const Evas_Object *o) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * Get an accessor to get random access to the list of children for the table.
 *
 * @note Do not remove or delete objects while walking the list.
 */
EAPI Eina_Accessor                      *evas_object_table_accessor_new    (const Evas_Object *o) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * Get the list of children for the table.
 *
 * @note This is a duplicate of the list kept by the table internally.
 *       It's up to the user to destroy it when it no longer needs it.
 *       It's possible to remove objects from the table when walking this
 *       list, but these removals won't be reflected on it.
 */
EAPI Eina_List                          *evas_object_table_children_get    (const Evas_Object *o) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;
   EAPI Evas_Object                        *evas_object_table_child_get       (const Evas_Object *o, unsigned short col, unsigned short row) EINA_ARG_NONNULL(1);
   
/**
 * @defgroup Evas_Object_Grid Grid Smart Object.
 *
 * Convenience smart object that packs children using a regular grid
 * layout using Their virtual grid location and size to determine
 * position inside the grid object
 *
 * @ingroup Evas_Smart_Object_Group
 * @since 1.1.0
 */

/**
 * Create a new grid.
 *
 * It's set to a virtual size of 1x1 by default and add children with
 * evas_object_grid_pack().
 * @since 1.1.0
 */
EAPI Evas_Object                        *evas_object_grid_add             (Evas *evas) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * Create a grid that is child of a given element @a parent.
 *
 * @see evas_object_grid_add()
 * @since 1.1.0
 */
EAPI Evas_Object                        *evas_object_grid_add_to          (Evas_Object *parent) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * Set the virtual resolution for the grid
 *
 * @param o The grid object to modify
 * @param w The virtual horizontal size (resolution) in integer units
 * @param h The virtual vertical size (resolution) in integer units
 * @since 1.1.0
 */
EAPI void                                evas_object_grid_size_set        (Evas_Object *o, int w, int h) EINA_ARG_NONNULL(1);

/**
 * Get the current virtual resolution
 *
 * @param o The grid object to query
 * @param w A pointer to an integer to store the virtual width
 * @param h A pointer to an integer to store the virtual height
 * @see evas_object_grid_size_set()
 * @since 1.1.0
 */
EAPI void                                evas_object_grid_size_get        (const Evas_Object *o, int *w, int *h) EINA_ARG_NONNULL(1) EINA_PURE;

/**
 * Sets the mirrored mode of the grid. In mirrored mode the grid items go
 * from right to left instead of left to right. That is, 0,0 is top right, not
 * to left.
 *
 * @param obj The grid object.
 * @param mirrored the mirrored mode to set
 * @since 1.1.0
 */
EAPI void                                evas_object_grid_mirrored_set    (Evas_Object *o, Eina_Bool mirrored) EINA_ARG_NONNULL(1);

/**
 * Gets the mirrored mode of the grid.
 *
 * @param obj The grid object.
 * @return EINA_TRUE if it's a mirrored grid, EINA_FALSE otherwise.
 * @see evas_object_grid_mirrored_set()
 * @since 1.1.0
 */
EAPI Eina_Bool                           evas_object_grid_mirrored_get    (const Evas_Object *o) EINA_ARG_NONNULL(1);

/**
 * Add a new child to a grid object.
 *
 * @param o The given grid object.
 * @param child The child object to add.
 * @param x The virtual x coordinate of the child
 * @param y The virtual y coordinate of the child
 * @param w The virtual width of the child
 * @param h The virtual height of the child
 * @return 1 on success, 0 on failure.
 * @since 1.1.0
 */
EAPI Eina_Bool                           evas_object_grid_pack            (Evas_Object *o, Evas_Object *child, int x, int y, int w, int h) EINA_ARG_NONNULL(1, 2);

/**
 * Remove child from grid.
 *
 * @note removing a child will immediately call a walk over children in order
 *       to recalculate numbers of columns and rows. If you plan to remove
 *       all children, use evas_object_grid_clear() instead.
 *
 * @return 1 on success, 0 on failure.
 * @since 1.1.0
 */
EAPI Eina_Bool                           evas_object_grid_unpack          (Evas_Object *o, Evas_Object *child) EINA_ARG_NONNULL(1, 2);

/**
 * Faster way to remove all child objects from a grid object.
 *
 * @param o The given grid object.
 * @param clear if true, it will delete just removed children.
 * @since 1.1.0
 */
EAPI void                                evas_object_grid_clear           (Evas_Object *o, Eina_Bool clear) EINA_ARG_NONNULL(1);

/**
 * Get the pack options for a grid child
 * 
 * Get the pack x, y, width and height in virtual coordinates set by
 * evas_object_grid_pack()
 * @param o The grid object
 * @param child The grid child to query for coordinates
 * @param x The pointer to where the x coordinate will be returned
 * @param y The pointer to where the y coordinate will be returned
 * @param w The pointer to where the width will be returned
 * @param h The pointer to where the height will be returned
 * @return 1 on success, 0 on failure.
 * @since 1.1.0
 */
EAPI Eina_Bool                           evas_object_grid_pack_get        (Evas_Object *o, Evas_Object *child, int *x, int *y, int *w, int *h);
         
/**
 * Get an iterator to walk the list of children for the grid.
 *
 * @note Do not remove or delete objects while walking the list.
 * @since 1.1.0
 */
EAPI Eina_Iterator                      *evas_object_grid_iterator_new    (const Evas_Object *o) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * Get an accessor to get random access to the list of children for the grid.
 *
 * @note Do not remove or delete objects while walking the list.
 * @since 1.1.0
 */
EAPI Eina_Accessor                      *evas_object_grid_accessor_new    (const Evas_Object *o) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * Get the list of children for the grid.
 *
 * @note This is a duplicate of the list kept by the grid internally.
 *       It's up to the user to destroy it when it no longer needs it.
 *       It's possible to remove objects from the grid when walking this
 *       list, but these removals won't be reflected on it.
 * @since 1.1.0
 */
EAPI Eina_List                          *evas_object_grid_children_get    (const Evas_Object *o) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;
         
/**
 * @defgroup Evas_Cserve Shared Image Cache Server
 *
 * Provides client-server infrastructure to share bitmaps across
 * multiple processes, saving data and processing power.
 */
   typedef struct _Evas_Cserve_Stats       Evas_Cserve_Stats;
   typedef struct _Evas_Cserve_Image_Cache Evas_Cserve_Image_Cache;
   typedef struct _Evas_Cserve_Image       Evas_Cserve_Image;
   typedef struct _Evas_Cserve_Config      Evas_Cserve_Config;

/**
 * Statistics about server that shares cached bitmaps.
 * @ingroup Evas_Cserve
 */
   struct _Evas_Cserve_Stats
     {
        int    saved_memory; /**< current saved memory, in bytes */
        int    wasted_memory; /**< current wasted memory, in bytes */
        int    saved_memory_peak; /**< peak of saved memory, in bytes */
        int    wasted_memory_peak; /**< peak of wasted memory, in bytes */
        double saved_time_image_header_load; /**< time, in seconds, saved in header loads by sharing cached loads instead */
        double saved_time_image_data_load; /**< time, in seconds, saved in data loads by sharing cached loads instead */
     };

/**
 * Cache of images shared by server.
 * @ingroup Evas_Cserve
 */
   struct _Evas_Cserve_Image_Cache
     {
        struct {
           int     mem_total;
           int     count;
        } active, cached;
        Eina_List *images;
     };

/**
 * An image shared by the server.
 * @ingroup Evas_Cserve
 */
   struct _Evas_Cserve_Image
     {
        const char *file, *key;
        int         w, h;
        time_t      file_mod_time;
        time_t      file_checked_time;
        time_t      cached_time;
        int         refcount;
        int         data_refcount;
        int         memory_footprint;
        double      head_load_time;
        double      data_load_time;
        Eina_Bool   alpha : 1;
        Eina_Bool   data_loaded : 1;
        Eina_Bool   active : 1;
        Eina_Bool   dead : 1;
        Eina_Bool   useless : 1;
     };

/**
 * Configuration that controls the server that shares cached bitmaps.
 * @ingroup Evas_Cserve
 */
    struct _Evas_Cserve_Config
     {
        int cache_max_usage;
        int cache_item_timeout;
        int cache_item_timeout_check;
     };


/**
 * Retrieves if the system wants to share bitmaps using the server.
 * @return @c EINA_TRUE if wants, @c EINA_FALSE otherwise.
 * @ingroup Evas_Cserve
 */
EAPI Eina_Bool         evas_cserve_want_get                   (void) EINA_WARN_UNUSED_RESULT EINA_PURE;

/**
 * Retrieves if the system is connected to the server used to shae bitmaps.
 * @return @c EINA_TRUE if connected, @c EINA_FALSE otherwise.
 * @ingroup Evas_Cserve
 */
EAPI Eina_Bool         evas_cserve_connected_get              (void) EINA_WARN_UNUSED_RESULT;

/**
 * Retrieves if the system wants to share bitmaps using the server.
 * @param stats pointer to structure to fill with statistics about
 *        cache server.
 * @return @c EINA_TRUE if @p stats were filled with data,
 *         @c EINA_FALSE otherwise and @p stats is untouched.
 * @ingroup Evas_Cserve
 */
EAPI Eina_Bool         evas_cserve_stats_get                  (Evas_Cserve_Stats *stats) EINA_WARN_UNUSED_RESULT;
   EAPI void              evas_cserve_image_cache_contents_clean (Evas_Cserve_Image_Cache *cache) EINA_PURE;

/**
 * Retrieves the current configuration of the server.
 * @param config where to store current server configuration.
 * @return @c EINA_TRUE if @p config were filled with data,
 *         @c EINA_FALSE otherwise and @p config is untouched.
 * @ingroup Evas_Cserve
 */
EAPI Eina_Bool         evas_cserve_config_get                 (Evas_Cserve_Config *config) EINA_WARN_UNUSED_RESULT EINA_PURE;

/**
 * Changes the configuration of the server.
 * @param config where to store current server configuration.
 * @return @c EINA_TRUE if @p config were successfully applied,
 *         @c EINA_FALSE otherwise.
 * @ingroup Evas_Cserve
 */
EAPI Eina_Bool         evas_cserve_config_set                 (const Evas_Cserve_Config *config) EINA_WARN_UNUSED_RESULT EINA_PURE;

/**
 * Force system to disconnect from cache server.
 * @ingroup Evas_Cserve
 */
EAPI void              evas_cserve_disconnect                 (void);

/**
 * @defgroup Evas_Utils General Utilities
 *
 * Some functions that are handy but are not specific of canvas or
 * objects.
 */

/**
 * Converts the given Evas image load error code into a string
 * describing it in english.
 *
 * @param error the error code, a value in ::Evas_Load_Error.
 * @return Always returns a valid string. If the given @p error is not
 *         supported, <code>"Unknown error"</code> is returned.
 *
 * Mostly evas_object_image_file_set() would be the function setting
 * that error value afterwards, but also evas_object_image_load(),
 * evas_object_image_save(), evas_object_image_data_get(),
 * evas_object_image_data_convert(), evas_object_image_pixels_import()
 * and evas_object_image_is_inside(). This function is meant to be
 * used in conjunction with evas_object_image_load_error_get(), as in:
 *
 * Example code:
 * @dontinclude evas-load-error-str.c
 * @skip img1 =
 * @until ecore_main_loop_begin(
 *
 * Here, being @c valid_path the path to a valid image and @c
 * bogus_path a path to a file which does not exist, the two outputs
 * of evas_load_error_str() would be (if no other errors occur):
 * <code>"No error on load"</code> and <code>"File (or file path) does
 * not exist"</code>, respectively. See the full @ref
 * Example_Evas_Load_Error_Str "example".
 *
 * @ingroup Evas_Utils
 */
EAPI const char       *evas_load_error_str               (Evas_Load_Error error);

/* Evas utility routines for color space conversions */
/* hsv color space has h in the range 0.0 to 360.0, and s,v in the range 0.0 to 1.0 */
/* rgb color space has r,g,b in the range 0 to 255 */

/**
 * Convert a given color from HSV to RGB format.
 *
 * @param h The Hue component of the color.
 * @param s The Saturation component of the color.
 * @param v The Value component of the color.
 * @param r The Red component of the color.
 * @param g The Green component of the color.
 * @param b The Blue component of the color.
 *
 * This function converts a given color in HSV color format to RGB
 * color format.
 *
 * @ingroup Evas_Utils
 **/
EAPI void              evas_color_hsv_to_rgb             (float h, float s, float v, int *r, int *g, int *b);

/**
 * Convert a given color from RGB to HSV format.
 *
 * @param r The Red component of the color.
 * @param g The Green component of the color.
 * @param b The Blue component of the color.
 * @param h The Hue component of the color.
 * @param s The Saturation component of the color.
 * @param v The Value component of the color.
 *
 * This function converts a given color in RGB color format to HSV
 * color format.
 *
 * @ingroup Evas_Utils
 **/
EAPI void              evas_color_rgb_to_hsv             (int r, int g, int b, float *h, float *s, float *v);

/* argb color space has a,r,g,b in the range 0 to 255 */

/**
 * Pre-multiplies a rgb triplet by an alpha factor.
 *
 * @param a The alpha factor.
 * @param r The Red component of the color.
 * @param g The Green component of the color.
 * @param b The Blue component of the color.
 *
 * This function pre-multiplies a given rbg triplet by an alpha
 * factor. Alpha factor is used to define transparency.
 *
 * @ingroup Evas_Utils
 **/
EAPI void              evas_color_argb_premul            (int a, int *r, int *g, int *b);

/**
 * Undo pre-multiplication of a rgb triplet by an alpha factor.
 *
 * @param a The alpha factor.
 * @param r The Red component of the color.
 * @param g The Green component of the color.
 * @param b The Blue component of the color.
 *
 * This function undoes pre-multiplication a given rbg triplet by an
 * alpha factor. Alpha factor is used to define transparency.
 *
 * @see evas_color_argb_premul().
 *
 * @ingroup Evas_Utils
 **/
EAPI void              evas_color_argb_unpremul          (int a, int *r, int *g, int *b);


/**
 * Pre-multiplies data by an alpha factor.
 *
 * @param data The data value.
 * @param len  The length value.
 *
 * This function pre-multiplies a given data by an alpha
 * factor. Alpha factor is used to define transparency.
 *
 * @ingroup Evas_Utils
 **/
EAPI void              evas_data_argb_premul             (unsigned int *data, unsigned int len);

/**
 * Undo pre-multiplication data by an alpha factor.
 *
 * @param data The data value.
 * @param len  The length value.
 *
 * This function undoes pre-multiplication of a given data by an alpha
 * factor. Alpha factor is used to define transparency.
 *
 * @ingroup Evas_Utils
 **/
EAPI void              evas_data_argb_unpremul           (unsigned int *data, unsigned int len);

/* string and font handling */

/**
 * Gets the next character in the string
 *
 * Given the UTF-8 string in @p str, and starting byte position in @p pos,
 * this function will place in @p decoded the decoded code point at @p pos
 * and return the byte index for the next character in the string.
 *
 * The only boundary check done is that @p pos must be >= 0. Other than that,
 * no checks are performed, so passing an index value that's not within the
 * length of the string will result in undefined behavior.
 *
 * @param str The UTF-8 string
 * @param pos The byte index where to start
 * @param decoded Address where to store the decoded code point. Optional.
 *
 * @return The byte index of the next character
 *
 * @ingroup Evas_Utils
 */
EAPI int               evas_string_char_next_get         (const char *str, int pos, int *decoded) EINA_ARG_NONNULL(1);

/**
 * Gets the previous character in the string
 *
 * Given the UTF-8 string in @p str, and starting byte position in @p pos,
 * this function will place in @p decoded the decoded code point at @p pos
 * and return the byte index for the previous character in the string.
 *
 * The only boundary check done is that @p pos must be >= 1. Other than that,
 * no checks are performed, so passing an index value that's not within the
 * length of the string will result in undefined behavior.
 *
 * @param str The UTF-8 string
 * @param pos The byte index where to start
 * @param decoded Address where to store the decoded code point. Optional.
 *
 * @return The byte index of the previous character
 *
 * @ingroup Evas_Utils
 */
EAPI int               evas_string_char_prev_get         (const char *str, int pos, int *decoded) EINA_ARG_NONNULL(1);

/**
 * Get the length in characters of the string.
 * @param  str The string to get the length of.
 * @return The length in characters (not bytes)
 * @ingroup Evas_Utils
 */
EAPI int               evas_string_char_len_get          (const char *str) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

/**
 * @defgroup Evas_Keys Key Input Functions
 *
 * Functions which feed key events to the canvas.
 *
 * As explained in @ref intro_not_evas, Evas is @b not aware of input
 * systems at all. Then, the user, if using it crudely (evas_new()),
 * will have to feed it with input events, so that it can react
 * somehow. If, however, the user creates a canvas by means of the
 * Ecore_Evas wrapper, it will automatically bind the chosen display
 * engine's input events to the canvas, for you.
 *
 * This group presents the functions dealing with the feeding of key
 * events to the canvas. On most of them, one has to reference a given
 * key by a name (<code>keyname</code> argument). Those are
 * <b>platform dependent</b> symbolic names for the keys. Sometimes
 * you'll get the right <code>keyname</code> by simply using an ASCII
 * value of the key name, but it won't be like that always.
 *
 * Typical platforms are Linux frame buffer (Ecore_FB) and X server
 * (Ecore_X) when using Evas with Ecore and Ecore_Evas. Please refer
 * to your display engine's documentation when using evas through an
 * Ecore helper wrapper when you need the <code>keyname</code>s.
 *
 * Example:
 * @dontinclude evas-events.c
 * @skip mods = evas_key_modifier_get(evas);
 * @until {
 *
 * All the other @c evas_key functions behave on the same manner. See
 * the full @ref Example_Evas_Events "example".
 *
 * @ingroup Evas_Canvas
 */

/**
 * @addtogroup Evas_Keys
 * @{
 */

/**
 * Returns a handle to the list of modifier keys registered in the
 * canvas @p e. This is required to check for which modifiers are set
 * at a given time with the evas_key_modifier_is_set() function.
 *
 * @param e The pointer to the Evas canvas
 *
 * @see evas_key_modifier_add
 * @see evas_key_modifier_del
 * @see evas_key_modifier_on
 * @see evas_key_modifier_off
 * @see evas_key_modifier_is_set
 *
 * @return An ::Evas_Modifier handle to query Evas' keys subsystem
 *	with evas_key_modifier_is_set(), or @c NULL on error.
 */
EAPI const Evas_Modifier *evas_key_modifier_get          (const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

/**
 * Returns a handle to the list of lock keys registered in the canvas
 * @p e. This is required to check for which locks are set at a given
 * time with the evas_key_lock_is_set() function.
 *
 * @param e The pointer to the Evas canvas
 *
 * @see evas_key_lock_add
 * @see evas_key_lock_del
 * @see evas_key_lock_on
 * @see evas_key_lock_off
 * @see evas_key_lock_is_set
 *
 * @return An ::Evas_Lock handle to query Evas' keys subsystem with
 *	evas_key_lock_is_set(), or @c NULL on error.
 */
EAPI const Evas_Lock     *evas_key_lock_get              (const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;


/**
 * Checks the state of a given modifier key, at the time of the
 * call. If the modifier is set, such as shift being pressed, this
 * function returns @c Eina_True.
 *
 * @param m The current modifiers set, as returned by
 *        evas_key_modifier_get().
 * @param keyname The name of the modifier key to check status for.
 *
 * @return @c Eina_True if the modifier key named @p keyname is on, @c
 *         Eina_False otherwise.
 *
 * @see evas_key_modifier_add
 * @see evas_key_modifier_del
 * @see evas_key_modifier_get
 * @see evas_key_modifier_on
 * @see evas_key_modifier_off
 */
EAPI Eina_Bool            evas_key_modifier_is_set       (const Evas_Modifier *m, const char *keyname) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1, 2) EINA_PURE;


/**
 * Checks the state of a given lock key, at the time of the call. If
 * the lock is set, such as caps lock, this function returns @c
 * Eina_True.
 *
 * @param l The current locks set, as returned by evas_key_lock_get().
 * @param keyname The name of the lock key to check status for.
 *
 * @return @c Eina_True if the @p keyname lock key is set, @c
 *        Eina_False otherwise.
 *
 * @see evas_key_lock_get
 * @see evas_key_lock_add
 * @see evas_key_lock_del
 * @see evas_key_lock_on
 * @see evas_key_lock_off
 */
EAPI Eina_Bool            evas_key_lock_is_set           (const Evas_Lock *l, const char *keyname) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1, 2) EINA_PURE;


/**
 * Adds the @p keyname key to the current list of modifier keys.
 *
 * @param e The pointer to the Evas canvas
 * @param keyname The name of the modifier key to add to the list of
 *        Evas modifiers.
 *
 * Modifiers are keys like shift, alt and ctrl, i.e., keys which are
 * meant to be pressed together with others, altering the behavior of
 * the secondly pressed keys somehow. Evas is so that these keys can
 * be user defined.
 *
 * This call allows custom modifiers to be added to the Evas system at
 * run time. It is then possible to set and unset modifier keys
 * programmatically for other parts of the program to check and act
 * on. Programmers using Evas would check for modifier keys on key
 * event callbacks using evas_key_modifier_is_set().
 *
 * @see evas_key_modifier_del
 * @see evas_key_modifier_get
 * @see evas_key_modifier_on
 * @see evas_key_modifier_off
 * @see evas_key_modifier_is_set
 *
 * @note If the programmer instantiates the canvas by means of the
 *       ecore_evas_new() family of helper functions, Ecore will take
 *       care of registering on it all standard modifiers: "Shift",
 *       "Control", "Alt", "Meta", "Hyper", "Super".
 */
EAPI void                 evas_key_modifier_add          (Evas *e, const char *keyname) EINA_ARG_NONNULL(1, 2);

/**
 * Removes the @p keyname key from the current list of modifier keys
 * on canvas @e.
 *
 * @param e The pointer to the Evas canvas
 * @param keyname The name of the key to remove from the modifiers list.
 *
 * @see evas_key_modifier_add
 * @see evas_key_modifier_get
 * @see evas_key_modifier_on
 * @see evas_key_modifier_off
 * @see evas_key_modifier_is_set
 */
EAPI void                 evas_key_modifier_del          (Evas *e, const char *keyname) EINA_ARG_NONNULL(1, 2);

/**
 * Adds the @p keyname key to the current list of lock keys.
 *
 * @param e The pointer to the Evas canvas
 * @param keyname The name of the key to add to the locks list.
 *
 * Locks are keys like caps lock, num lock or scroll lock, i.e., keys
 * which are meant to be pressed once -- toggling a binary state which
 * is bound to it -- and thus altering the behavior of all
 * subsequently pressed keys somehow, depending on its state. Evas is
 * so that these keys can be defined by the user.
 *
 * This allows custom locks to be added to the evas system at run
 * time. It is then possible to set and unset lock keys
 * programmatically for other parts of the program to check and act
 * on. Programmers using Evas would check for lock keys on key event
 * callbacks using evas_key_lock_is_set().
 *
 * @see evas_key_lock_get
 * @see evas_key_lock_del
 * @see evas_key_lock_on
 * @see evas_key_lock_off
 * @see evas_key_lock_is_set
 *
 * @note If the programmer instantiates the canvas by means of the
 *       ecore_evas_new() family of helper functions, Ecore will take
 *       care of registering on it all standard lock keys: "Caps_Lock",
 *       "Num_Lock", "Scroll_Lock".
 */
EAPI void                 evas_key_lock_add              (Evas *e, const char *keyname) EINA_ARG_NONNULL(1, 2);

/**
 * Removes the @p keyname key from the current list of lock keys on
 * canvas @e.
 *
 * @param e The pointer to the Evas canvas
 * @param keyname The name of the key to remove from the locks list.
 *
 * @see evas_key_lock_get
 * @see evas_key_lock_add
 * @see evas_key_lock_on
 * @see evas_key_lock_off
 */
EAPI void                 evas_key_lock_del              (Evas *e, const char *keyname) EINA_ARG_NONNULL(1, 2);


/**
 * Enables or turns on programmatically the modifier key with name @p
 * keyname.
 *
 * @param e The pointer to the Evas canvas
 * @param keyname The name of the modifier to enable.
 *
 * The effect will be as if the key was pressed for the whole time
 * between this call and a matching evas_key_modifier_off().
 *
 * @see evas_key_modifier_add
 * @see evas_key_modifier_get
 * @see evas_key_modifier_off
 * @see evas_key_modifier_is_set
 */
EAPI void                 evas_key_modifier_on           (Evas *e, const char *keyname) EINA_ARG_NONNULL(1, 2);

/**
 * Disables or turns off programmatically the modifier key with name
 * @p keyname.
 *
 * @param e The pointer to the Evas canvas
 * @param keyname The name of the modifier to disable.
 *
 * @see evas_key_modifier_add
 * @see evas_key_modifier_get
 * @see evas_key_modifier_on
 * @see evas_key_modifier_is_set
 */
EAPI void                 evas_key_modifier_off          (Evas *e, const char *keyname) EINA_ARG_NONNULL(1, 2);

/**
 * Enables or turns on programmatically the lock key with name @p
 * keyname.
 *
 * @param e The pointer to the Evas canvas
 * @param keyname The name of the lock to enable.
 *
 * The effect will be as if the key was put on its active state after
 * this call.
 *
 * @see evas_key_lock_get
 * @see evas_key_lock_add
 * @see evas_key_lock_del
 * @see evas_key_lock_off
 */
EAPI void                 evas_key_lock_on               (Evas *e, const char *keyname) EINA_ARG_NONNULL(1, 2);

/**
 * Disables or turns off programmatically the lock key with name @p
 * keyname.
 *
 * @param e The pointer to the Evas canvas
 * @param keyname The name of the lock to disable.
 *
 * The effect will be as if the key was put on its inactive state
 * after this call.
 *
 * @see evas_key_lock_get
 * @see evas_key_lock_add
 * @see evas_key_lock_del
 * @see evas_key_lock_on
 */
EAPI void                 evas_key_lock_off              (Evas *e, const char *keyname) EINA_ARG_NONNULL(1, 2);


/**
 * Creates a bit mask from the @p keyname @b modifier key. Values
 * returned from different calls to it may be ORed together,
 * naturally.
 *
 * @param e The canvas whom to query the bit mask from.
 * @param keyname The name of the modifier key to create the mask for.
 *
 * @returns the bit mask or 0 if the @p keyname key wasn't registered as a
 *          modifier for canvas @p e.
 *
 * This function is meant to be using in conjunction with
 * evas_object_key_grab()/evas_object_key_ungrab(). Go check their
 * documentation for more information.
 *
 * @see evas_key_modifier_add
 * @see evas_key_modifier_get
 * @see evas_key_modifier_on
 * @see evas_key_modifier_off
 * @see evas_key_modifier_is_set
 * @see evas_object_key_grab
 * @see evas_object_key_ungrab
 */
EAPI Evas_Modifier_Mask   evas_key_modifier_mask_get     (const Evas *e, const char *keyname) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1, 2) EINA_PURE;


/**
 * Requests @p keyname key events be directed to @p obj.
 *
 * Key grabs allow an object to receive key events for specific key
 * strokes even if another object has focus.  If the grab is
 * non-exclusive then all objects that have grabs on the key will get
 * the event, however if the grab is exclusive, no other object can
 * get a grab on the key and only that object will get the event.
 *
 * @p keyname is a platform dependent symbolic name for the key
 * pressed.  It is sometimes possible to convert the string to an
 * ASCII value of the key, but not always for example the enter key
 * may be returned as the string 'Enter'.
 *
 * Typical platforms are Linux frame buffer (Ecore_FB) and X server
 * (Ecore_X) when using Evas with Ecore and Ecore_Evas.
 *
 * For a list of keynames for the Linux frame buffer, please refer to
 * the Ecore_FB documentation.
 *
 * @p modifiers and @p not_modifiers are bit masks of all the
 * modifiers that are required and not required respectively for the
 * new grab.  Modifiers can be things such as shift and ctrl as well
 * as user defigned types via evas_key_modifier_add.
 *
 * @see evas_object_key_ungrab
 * @see evas_object_focus_set
 * @see evas_object_focus_get
 * @see evas_focus_get
 * @see evas_key_modifier_add
 *
 * @param obj the object to direct @p keyname events to.
 * @param keyname the key to request events for.
 * @param modifiers a mask of modifiers that should be present to
 * trigger the event.
 * @param not_modifiers a mask of modifiers that should not be present
 * to trigger the event.
 * @param exclusive request that the @p obj is the only object
 * receiving the @p keyname events.
 * @return Boolean indicating whether the grab succeeded
 */
EAPI Eina_Bool            evas_object_key_grab           (Evas_Object *obj, const char *keyname, Evas_Modifier_Mask modifiers, Evas_Modifier_Mask not_modifiers, Eina_Bool exclusive) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1, 2);

/**
 * Request that the grab on @p obj be removed.
 *
 * Removes the grab on @p obj if @p keyname, @p modifiers, and @p not_modifiers
 * match.
 *
 * @see evas_object_key_grab
 * @see evas_object_focus_set
 * @see evas_object_focus_get
 * @see evas_focus_get
 *
 * @param obj the object that has an existing grab.
 * @param keyname the key the grab is for.
 * @param modifiers a mask of modifiers that should be present to
 * trigger the event.
 * @param not_modifiers a mask of modifiers that should not be present
 * to trigger the event.
 */
EAPI void                 evas_object_key_ungrab         (Evas_Object *obj, const char *keyname, Evas_Modifier_Mask modifiers, Evas_Modifier_Mask not_modifiers) EINA_ARG_NONNULL(1, 2);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif
