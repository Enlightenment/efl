/**
   @mainpage Evas

   @version 1.7
   @date 2000-2012

   Please see the @ref authors page for contact details.
   @link Evas.h Evas API @endlink

   @link Evas.h Evas API @endlink

   @section toc Table of Contents

   @li @ref intro
   @li @ref work
   @li @ref compiling
   @li @ref install
   @li @ref next_steps
   @li @ref intro_example


   @section intro What is Evas?

   Evas is a clean display canvas API for several target display systems
   that can draw anti-aliased text, smooth super and sub-sampled scaled
   images, alpha-blend objects and much more.

   It abstracts any need to know much about what the characteristics of
   your display system are or what graphics calls are used to draw them
   and how. It deals on an object level where all you do is create and
   manipulate objects in a canvas, set their properties, and the rest is
   done for you.

   Evas optimises the rendering pipeline to minimise effort in redrawing
   changes made to the canvas and so takes this work out of the
   programmers hand, saving a lot of time and energy.

   It's small and lean, designed to work on embedded systems all the way
   to large and powerful multi-cpu workstations. It can be compiled to
   only have the features you need for your target platform if you so
   wish, thus keeping it small and lean. It has several display
   back-ends, letting it display on several display systems, making it
   portable for cross-device and cross-platform development.

   @subsection intro_not_evas What Evas is not?

   Evas is not a widget set or widget toolkit, however it is their
   base. See Elementary (http://docs.enlightenment.org/auto/elementary/)
   for a toolkit based on Evas, Edje, Ecore and other Enlightenment
   technologies.

   It is not dependent or aware of main loops, input or output
   systems. Input should be polled from various sources and fed to
   Evas. Similarly, it will not create windows or report windows updates
   to your system, rather just drawing the pixels and reporting to the
   user the areas that were changed. Of course these operations are quite
   common and thus they are ready to use in Ecore, particularly in
   Ecore_Evas (http://docs.enlightenment.org/auto/ecore/).


   @section work How does Evas work?

   Evas is a canvas display library. This is markedly different from most
   display and windowing systems as a canvas is structural and is also a
   state engine, whereas most display and windowing systems are immediate
   mode display targets. Evas handles the logic between a structural
   display via its state engine, and controls the target windowing system
   in order to produce rendered results of the current canvas' state on
   the display.

   Immediate mode display systems retain very little, or no state. A
   program will execute a series of commands, as in the pseudo code:

   @verbatim
   draw line from position (0, 0) to position (100, 200);

   draw rectangle from position (10, 30) to position (50, 500);

   bitmap_handle = create_bitmap();
   scale bitmap_handle to size 100 x 100;
   draw image bitmap_handle at position (10, 30);
   @endverbatim

   The series of commands is executed by the windowing system and the
   results are displayed on the screen (normally). Once the commands are
   executed the display system has little or no idea of how to reproduce
   this image again, and so has to be instructed by the application how
   to redraw sections of the screen whenever needed. Each successive
   command will be executed as instructed by the application and either
   emulated by software or sent to the graphics hardware on the device to
   be performed.

   The advantage of such a system is that it is simple, and gives a
   program tight control over how something looks and is drawn. Given the
   increasing complexity of displays and demands by users to have better
   looking interfaces, more and more work is needing to be done at this
   level by the internals of widget sets, custom display widgets and
   other programs. This means more and more logic and display rendering
   code needs to be written time and time again, each time the
   application needs to figure out how to minimise redraws so that
   display is fast and interactive, and keep track of redraw logic. The
   power comes at a high-price, lots of extra code and work.  Programmers
   not very familiar with graphics programming will often make mistakes
   at this level and produce code that is sub optimal. Those familiar
   with this kind of programming will simply get bored by writing the
   same code again and again.

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
   behind the image. These useless paints tend to be very costly, as
   pixels tend to be 4 bytes in size, thus an overlapping region of 100 x
   100 pixels is around 40000 useless writes! The developer could write
   code to calculate the overlapping areas and avoid painting then, but
   then it should be mixed with the "expose event" handling mentioned
   above and quickly one realizes the initially simpler method became
   really complex.

   Evas is a structural system in which the programmer creates and
   manages display objects and their properties, and as a result of this
   higher level state management, the canvas is able to redraw the set of
   objects when needed to represent the current state of the canvas.

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

   This may look longer, but when the display needs to be refreshed or
   updated, the programmer only moves, resizes, shows, hides etc. the
   objects that need to change. The programmer simply thinks at the
   object logic level, and the canvas software does the rest of the work
   for them, figuring out what actually changed in the canvas since it
   was last drawn, how to most efficiently redraw the canvas and its
   contents to reflect the current state, and then it can go off and do
   the actual drawing of the canvas.

   This lets the programmer think in a more natural way when dealing with
   a display, and saves time and effort of working out how to load and
   display images, render given the current display system etc. Since
   Evas also is portable across different display systems, this also
   gives the programmer the ability to have their code ported and
   displayed on different display systems with very little work.

   Evas can be seen as a display system that stands somewhere between a
   widget set and an immediate mode display system. It retains basic
   display logic, but does very little high-level logic such as
   scrollbars, sliders, push buttons etc.


   @section compiling How to compile using Evas ?

   Evas is a library your application links to. The procedure for this is
   very simple. You simply have to compile your application with the
   appropriate compiler flags that the @c pkg-config script outputs. For
   example:

   Compiling C or C++ files into object files:

   @verbatim
   gcc -c -o main.o main.c `pkg-config --cflags evas`
   @endverbatim

   Linking object files into a binary executable:

   @verbatim
   gcc -o my_application main.o `pkg-config --libs evas`
   @endverbatim

   You simply have to make sure that @c pkg-config is in your shell's @c
   PATH (see the manual page for your appropriate shell) and @c evas.pc
   in @c /usr/lib/pkgconfig or its path in the @c PKG_CONFIG_PATH
   environment variable. It's that simple to link and use Evas once you
   have written your code to use it.

   Since the program is linked to Evas, it is now able to use any
   advertised API calls to display graphics in a canvas managed by it, as
   well as use the API calls provided to manage data.

   You should make sure you add any extra compile and link flags to your
   compile commands that your application may need as well. The above
   example is only guaranteed to make Evas add it's own requirements.


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

   @li @ref Evas_Object_Group, where you'll get how to basically
    manipulate generic objects lying on an Evas canvas, handle canvas
    and object events, etc.
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
    hiding, moving, resizing, color setting and more. These could
    be as simple as a group of objects that move together (see @ref
    Evas_Smart_Object_Clipped) up to implementations of what
    ends to be a widget, providing some intelligence (thus the name)
    to Evas objects -- like a button or check box, for example.

   @section intro_example Introductory Example

   @include evas-buffer-simple.c
 */

/**
   @page authors Authors
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
   @author Samsung Electronics
   @author Samsung SAIT
   @author Sung W. Park <sungwoo@@gmail.com>
   @author Jiyoun Park <jy0703.park@@samsung.com>
   @author Myoungwoon Roy Kim(roy_kim) <myoungwoon.kim@@samsung.com> <myoungwoon@@gmail.com>
   @author Thierry el Borgi <thierry@@substantiel.fr>
   @author Shilpa Singh <shilpa.singh@@samsung.com> <shilpasingh.o@@gmail.com>
   @author ChunEon Park <hermet@@hermet.pe.kr>
   @author Christopher 'devilhorns' Michael <cpmichael1@@comcast.net>
   @author Seungsoo Woo <om101.woo@@samsung.com>
   @author Youness Alaoui <kakaroto@@kakaroto.homelinux.net>
   @author Jim Kukunas <james.t.kukunas@@linux.intel.com>
   @author Nicolas Aguirre <aguirre.nicolas@@gmail.com>
   @author Rafal Krypa <r.krypa@@samsung.com>
   @author Hyoyoung Chang <hyoyoung@@gmail.com>
   @author Jérôme Pinot <ngc891@@gmail.com>
   @author Rafael Antognolli <antognolli@@profusion.mobi>

   Please contact <enlightenment-devel@lists.sourceforge.net> to get in
   contact with the developers and maintainers.
 */

#ifndef _EVAS_H
#define _EVAS_H

#include <time.h>

#include <Eina.h>

/* This include has been added to support Eo in Evas */
#include <Eo.h>

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
#define EVAS_VERSION_MINOR 8

typedef struct _Evas_Version
{
   int major;
   int minor;
   int micro;
   int revision;
} Evas_Version;

EAPI extern Evas_Version * evas_version;

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
   EVAS_BIDI_DIRECTION_NEUTRAL = EVAS_BIDI_DIRECTION_NATURAL,
   EVAS_BIDI_DIRECTION_LTR,
   EVAS_BIDI_DIRECTION_RTL
} Evas_BiDi_Direction;

/**
 * Identifier of callbacks to be set for Evas canvases or Evas
 * objects.
 *
 * The following figure illustrates some Evas callbacks:
 *
 * @image html evas-callbacks.png
 * @image rtf evas-callbacks.png
 * @image latex evas-callbacks.eps
 *
 * @see evas_object_event_callback_add()
 * @see evas_event_callback_add()
 */
typedef enum _Evas_Callback_Type
{
   /*
    * The following events are only for use with Evas objects, with
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
   EVAS_CALLBACK_IMAGE_PRELOADED, /**< Image has been preloaded */

   /*
    * The following events are only for use with Evas canvases, with
    * evas_event_callback_add():
    */
   EVAS_CALLBACK_CANVAS_FOCUS_IN, /**< Canvas got focus as a whole */
   EVAS_CALLBACK_CANVAS_FOCUS_OUT, /**< Canvas lost focus as a whole */
   EVAS_CALLBACK_RENDER_FLUSH_PRE, /**< Called just before rendering is updated on the canvas target */
   EVAS_CALLBACK_RENDER_FLUSH_POST, /**< Called just after rendering is updated on the canvas target */
   EVAS_CALLBACK_CANVAS_OBJECT_FOCUS_IN, /**< Canvas object got focus */
   EVAS_CALLBACK_CANVAS_OBJECT_FOCUS_OUT, /**< Canvas object lost focus */

   /*
    * More Evas object event types - see evas_object_event_callback_add():
    */
   EVAS_CALLBACK_IMAGE_UNLOADED, /**< Image data has been unloaded (by some mechanism in Evas that throw out original image data) */

   EVAS_CALLBACK_RENDER_PRE, /**< Called just before rendering starts on the canvas target @since 1.2 */
   EVAS_CALLBACK_RENDER_POST, /**< Called just after rendering stops on the canvas target @since 1.2 */

   EVAS_CALLBACK_IMAGE_RESIZE, /**< Image size is changed @since 1.8 */
   EVAS_CALLBACK_DEVICE_CHANGED, /**< Devices added, removed or changed on canvas @since 1.8 */
   EVAS_CALLBACK_LAST /**< kept as last element/sentinel -- not really an event */
} Evas_Callback_Type; /**< The types of events triggering a callback */

EAPI extern const Eo_Event_Description _EVAS_EVENT_MOUSE_IN;
EAPI extern const Eo_Event_Description _EVAS_EVENT_MOUSE_OUT;
EAPI extern const Eo_Event_Description _EVAS_EVENT_MOUSE_DOWN;
EAPI extern const Eo_Event_Description _EVAS_EVENT_MOUSE_UP;
EAPI extern const Eo_Event_Description _EVAS_EVENT_MOUSE_MOVE;
EAPI extern const Eo_Event_Description _EVAS_EVENT_MOUSE_WHEEL;
EAPI extern const Eo_Event_Description _EVAS_EVENT_MULTI_DOWN;
EAPI extern const Eo_Event_Description _EVAS_EVENT_MULTI_UP;
EAPI extern const Eo_Event_Description _EVAS_EVENT_MULTI_MOVE;
EAPI extern const Eo_Event_Description _EVAS_EVENT_FREE;
EAPI extern const Eo_Event_Description _EVAS_EVENT_KEY_DOWN;
EAPI extern const Eo_Event_Description _EVAS_EVENT_KEY_UP;
EAPI extern const Eo_Event_Description _EVAS_EVENT_FOCUS_IN;
EAPI extern const Eo_Event_Description _EVAS_EVENT_FOCUS_OUT;
EAPI extern const Eo_Event_Description _EVAS_EVENT_SHOW;
EAPI extern const Eo_Event_Description _EVAS_EVENT_HIDE;
EAPI extern const Eo_Event_Description _EVAS_EVENT_MOVE;
EAPI extern const Eo_Event_Description _EVAS_EVENT_RESIZE;
EAPI extern const Eo_Event_Description _EVAS_EVENT_RESTACK;
EAPI extern const Eo_Event_Description _EVAS_EVENT_DEL;
EAPI extern const Eo_Event_Description _EVAS_EVENT_HOLD;
EAPI extern const Eo_Event_Description _EVAS_EVENT_CHANGED_SIZE_HINTS;
EAPI extern const Eo_Event_Description _EVAS_EVENT_IMAGE_PRELOADED;
EAPI extern const Eo_Event_Description _EVAS_EVENT_IMAGE_RESIZE;
EAPI extern const Eo_Event_Description _EVAS_EVENT_CANVAS_FOCUS_IN;
EAPI extern const Eo_Event_Description _EVAS_EVENT_CANVAS_FOCUS_OUT;
EAPI extern const Eo_Event_Description _EVAS_EVENT_RENDER_FLUSH_PRE;
EAPI extern const Eo_Event_Description _EVAS_EVENT_RENDER_FLUSH_POST;
EAPI extern const Eo_Event_Description _EVAS_EVENT_CANVAS_OBJECT_FOCUS_IN;
EAPI extern const Eo_Event_Description _EVAS_EVENT_CANVAS_OBJECT_FOCUS_OUT;
EAPI extern const Eo_Event_Description _EVAS_EVENT_IMAGE_UNLOADED;
EAPI extern const Eo_Event_Description _EVAS_EVENT_RENDER_PRE;
EAPI extern const Eo_Event_Description _EVAS_EVENT_RENDER_POST;

#define EVAS_EVENT_MOUSE_IN (&(_EVAS_EVENT_MOUSE_IN))
#define EVAS_EVENT_MOUSE_OUT (&(_EVAS_EVENT_MOUSE_OUT))
#define EVAS_EVENT_MOUSE_DOWN (&(_EVAS_EVENT_MOUSE_DOWN))
#define EVAS_EVENT_MOUSE_UP (&(_EVAS_EVENT_MOUSE_UP))
#define EVAS_EVENT_MOUSE_MOVE (&(_EVAS_EVENT_MOUSE_MOVE))
#define EVAS_EVENT_MOUSE_WHEEL (&(_EVAS_EVENT_MOUSE_WHEEL))
#define EVAS_EVENT_MULTI_DOWN (&(_EVAS_EVENT_MULTI_DOWN))
#define EVAS_EVENT_MULTI_UP (&(_EVAS_EVENT_MULTI_UP))
#define EVAS_EVENT_MULTI_MOVE (&(_EVAS_EVENT_MULTI_MOVE))
#define EVAS_EVENT_FREE (&(_EVAS_EVENT_FREE))
#define EVAS_EVENT_KEY_DOWN (&(_EVAS_EVENT_KEY_DOWN))
#define EVAS_EVENT_KEY_UP (&(_EVAS_EVENT_KEY_UP))
#define EVAS_EVENT_FOCUS_IN (&(_EVAS_EVENT_FOCUS_IN))
#define EVAS_EVENT_FOCUS_OUT (&(_EVAS_EVENT_FOCUS_OUT))
#define EVAS_EVENT_SHOW (&(_EVAS_EVENT_SHOW))
#define EVAS_EVENT_HIDE (&(_EVAS_EVENT_HIDE))
#define EVAS_EVENT_MOVE (&(_EVAS_EVENT_MOVE))
#define EVAS_EVENT_RESIZE (&(_EVAS_EVENT_RESIZE))
#define EVAS_EVENT_RESTACK (&(_EVAS_EVENT_RESTACK))
#define EVAS_EVENT_DEL (&(_EVAS_EVENT_DEL))
#define EVAS_EVENT_HOLD (&(_EVAS_EVENT_HOLD))
#define EVAS_EVENT_CHANGED_SIZE_HINTS (&(_EVAS_EVENT_CHANGED_SIZE_HINTS))
#define EVAS_EVENT_IMAGE_PRELOADED (&(_EVAS_EVENT_IMAGE_PRELOADED))
#define EVAS_EVENT_IMAGE_RESIZE (&(_EVAS_EVENT_IMAGE_RESIZE))
#define EVAS_EVENT_CANVAS_FOCUS_IN (&(_EVAS_EVENT_CANVAS_FOCUS_IN))
#define EVAS_EVENT_CANVAS_FOCUS_OUT (&(_EVAS_EVENT_CANVAS_FOCUS_OUT))
#define EVAS_EVENT_RENDER_FLUSH_PRE (&(_EVAS_EVENT_RENDER_FLUSH_PRE))
#define EVAS_EVENT_RENDER_FLUSH_POST (&(_EVAS_EVENT_RENDER_FLUSH_POST))
#define EVAS_EVENT_CANVAS_OBJECT_FOCUS_IN (&(_EVAS_EVENT_CANVAS_OBJECT_FOCUS_IN))
#define EVAS_EVENT_CANVAS_OBJECT_FOCUS_OUT (&(_EVAS_EVENT_CANVAS_OBJECT_FOCUS_OUT))
#define EVAS_EVENT_IMAGE_UNLOADED (&(_EVAS_EVENT_IMAGE_UNLOADED))
#define EVAS_EVENT_RENDER_PRE (&(_EVAS_EVENT_RENDER_PRE))
#define EVAS_EVENT_RENDER_POST (&(_EVAS_EVENT_RENDER_POST))

/**
 * @def EVAS_CALLBACK_PRIORITY_BEFORE
 * Slightly more prioritized than default.
 * @since 1.1
 */
#define EVAS_CALLBACK_PRIORITY_BEFORE  -100
/**
 * @def EVAS_CALLBACK_PRIORITY_DEFAULT
 * Default callback priority level
 * @since 1.1
 */
#define EVAS_CALLBACK_PRIORITY_DEFAULT 0
/**
 * @def EVAS_CALLBACK_PRIORITY_AFTER
 * Slightly less prioritized than default.
 * @since 1.1
 */
#define EVAS_CALLBACK_PRIORITY_AFTER   100

/**
 * @typedef Evas_Callback_Priority
 *
 * Callback priority value. Range is -32k - 32k. The lower the number, the
 * bigger the priority.
 *
 * @see EVAS_CALLBACK_PRIORITY_AFTER
 * @see EVAS_CALLBACK_PRIORITY_BEFORE
 * @see EVAS_CALLBACK_PRIORITY_DEFAULT
 *
 * @since 1.1
 */
typedef Eo_Callback_Priority Evas_Callback_Priority;

EAPI extern const Eo_Event_Description _CLICKED_EVENT;
EAPI extern const Eo_Event_Description _CLICKED_DOUBLE_EVENT;
EAPI extern const Eo_Event_Description _CLICKED_TRIPLE_EVENT;
EAPI extern const Eo_Event_Description _PRESSED_EVENT;
EAPI extern const Eo_Event_Description _UNPRESSED_EVENT;
EAPI extern const Eo_Event_Description _LONGPRESSED_EVENT;
EAPI extern const Eo_Event_Description _REPEATED_EVENT;
EAPI extern const Eo_Event_Description _SCROLL_EVENT;
EAPI extern const Eo_Event_Description _SCROLL_ANIM_START_EVENT;
EAPI extern const Eo_Event_Description _SCROLL_ANIM_STOP_EVENT;
EAPI extern const Eo_Event_Description _SCROLL_DRAG_START_EVENT;
EAPI extern const Eo_Event_Description _SCROLL_DRAG_STOP_EVENT;
EAPI extern const Eo_Event_Description _ZOOM_START_EVENT;
EAPI extern const Eo_Event_Description _ZOOM_STOP_EVENT;
EAPI extern const Eo_Event_Description _ZOOM_CHANGE_EVENT;
EAPI extern const Eo_Event_Description _SELECTED_EVENT;
EAPI extern const Eo_Event_Description _UNSELECTED_EVENT;
EAPI extern const Eo_Event_Description _SELECTION_PASTE_EVENT;
EAPI extern const Eo_Event_Description _SELECTION_COPY_EVENT;
EAPI extern const Eo_Event_Description _SELECTION_CUT_EVENT;
EAPI extern const Eo_Event_Description _SELECTION_START_EVENT;
EAPI extern const Eo_Event_Description _SELECTION_CHANGED_EVENT;
EAPI extern const Eo_Event_Description _SELECTION_CLEARED_EVENT;
EAPI extern const Eo_Event_Description _DRAG_EVENT;
EAPI extern const Eo_Event_Description _DRAG_START_EVENT;
EAPI extern const Eo_Event_Description _DRAG_STOP_EVENT;
EAPI extern const Eo_Event_Description _DRAG_END_EVENT;
EAPI extern const Eo_Event_Description _DRAG_START_UP_EVENT;
EAPI extern const Eo_Event_Description _DRAG_START_DOWN_EVENT;
EAPI extern const Eo_Event_Description _DRAG_START_RIGHT_EVENT;
EAPI extern const Eo_Event_Description _DRAG_START_LEFT_EVENT;

#define EVAS_SMART_CLICKED_EVENT (&(_CLICKED_EVENT))
#define EVAS_SMART_CLICKED_DOUBLE_EVENT (&(_CLICKED_DOUBLE_EVENT))
#define EVAS_SMART_CLICKED_TRIPLE_EVENT (&(_CLICKED_TRIPLE_EVENT))
#define EVAS_SMART_PRESSED_EVENT (&(_PRESSED_EVENT))
#define EVAS_SMART_UNPRESSED_EVENT (&(_UNPRESSED_EVENT))
#define EVAS_SMART_LONGPRESSED_EVENT (&(_LONGPRESSED_EVENT))
#define EVAS_SMART_REPEATED_EVENT (&(_REPEATED_EVENT))
#define EVAS_SMART_SCROLL_EVENT (&(_SCROLL_EVENT))
#define EVAS_SMART_SCROLL_ANIM_START_EVENT (&(_SCROLL_ANIM_START_EVENT))
#define EVAS_SMART_SCROLL_ANIM_STOP_EVENT (&(_SCROLL_ANIM_STOP_EVENT))
#define EVAS_SMART_SCROLL_DRAG_START_EVENT (&(_SCROLL_DRAG_START_EVENT))
#define EVAS_SMART_SCROLL_DRAG_STOP_EVENT (&(_SCROLL_DRAG_STOP_EVENT))
#define EVAS_SMART_ZOOM_START_EVENT (&(_ZOOM_START_EVENT))
#define EVAS_SMART_ZOOM_STOP_EVENT (&(_ZOOM_STOP_EVENT))
#define EVAS_SMART_ZOOM_CHANGE_EVENT (&(_ZOOM_CHANGE_EVENT))
#define EVAS_SMART_SELECTED_EVENT (&(_SELECTED_EVENT))
#define EVAS_SMART_UNSELECTED_EVENT (&(_UNSELECTED_EVENT))
#define EVAS_SMART_SELECTION_PASTE_EVENT (&(_SELECTION_PASTE_EVENT))
#define EVAS_SMART_SELECTION_COPY_EVENT (&(_SELECTION_COPY_EVENT))
#define EVAS_SMART_SELECTION_CUT_EVENT (&(_SELECTION_CUT_EVENT))
#define EVAS_SMART_SELECTION_START_EVENT (&(_SELECTION_START_EVENT))
#define EVAS_SMART_SELECTION_CHANGED_EVENT (&(_SELECTION_CHANGED_EVENT))
#define EVAS_SMART_SELECTION_CLEARED_EVENT (&(_SELECTION_CLEARED_EVENT))
#define EVAS_SMART_DRAG_EVENT (&(_DRAG_EVENT))
#define EVAS_SMART_DRAG_START_EVENT (&(_DRAG_START_EVENT))
#define EVAS_SMART_DRAG_STOP_EVENT (&(_DRAG_STOP_EVENT))
#define EVAS_SMART_DRAG_END_EVENT (&(_DRAG_END_EVENT))
#define EVAS_SMART_DRAG_START_UP_EVENT (&(_DRAG_START_UP_EVENT))
#define EVAS_SMART_DRAG_START_DOWN_EVENT (&(_DRAG_START_DOWN_EVENT))
#define EVAS_SMART_DRAG_START_RIGHT_EVENT (&(_DRAG_START_RIGHT_EVENT))
#define EVAS_SMART_DRAG_START_LEFT_EVENT (&(_DRAG_START_LEFT_EVENT))

const Eo_Class *evas_smart_signal_interface_get(void) EINA_CONST;
const Eo_Class *evas_smart_clickable_interface_get(void) EINA_CONST;
const Eo_Class *evas_smart_scrollable_interface_get(void) EINA_CONST;
const Eo_Class *evas_smart_zoomable_interface_get(void) EINA_CONST;
const Eo_Class *evas_smart_selectable_interface_get(void) EINA_CONST;
const Eo_Class *evas_smart_draggable_interface_get(void) EINA_CONST;

#define EVAS_SMART_SIGNAL_INTERFACE evas_smart_signal_interface_get()
#define EVAS_SMART_CLICKABLE_INTERFACE evas_smart_clickable_interface_get()
#define EVAS_SMART_SCROLLABLE_INTERFACE evas_smart_scrollable_interface_get()
#define EVAS_SMART_ZOOMABLE_INTERFACE evas_smart_zoomable_interface_get()
#define EVAS_SMART_SELECTABLE_INTERFACE evas_smart_selectable_interface_get()
#define EVAS_SMART_DRAGGABLE_INTERFACE evas_smart_draggable_interface_get()

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
   EVAS_EVENT_FLAG_ON_SCROLL = (1 << 1) /**< This event flag indicates the event occurs while scrolling; for example, DOWN event occurs during scrolling; the event should be used for informational purposes and maybe some indications visually, but not actually perform anything */
} Evas_Event_Flags; /**< Flags for Events */

/**
 * State of Evas_Coord_Touch_Point
 */
typedef enum _Evas_Touch_Point_State
{
   EVAS_TOUCH_POINT_DOWN, /**< Touch point is pressed down */
   EVAS_TOUCH_POINT_UP, /**< Touch point is released */
   EVAS_TOUCH_POINT_MOVE, /**< Touch point is moved */
   EVAS_TOUCH_POINT_STILL, /**< Touch point is not moved after pressed */
   EVAS_TOUCH_POINT_CANCEL /**< Touch point is cancelled */
} Evas_Touch_Point_State;

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
   EVAS_COLORSPACE_YCBCR422P601_PL, /**< YCbCr 4:2:2 Planar, ITU.BT-601 specifications. The data pointed to is just an array of row pointer, pointing to the Y rows, then the Cb, then Cr rows */
   EVAS_COLORSPACE_YCBCR422P709_PL, /**< YCbCr 4:2:2 Planar, ITU.BT-709 specifications. The data pointed to is just an array of row pointer, pointing to the Y rows, then the Cb, then Cr rows */
   EVAS_COLORSPACE_RGB565_A5P, /**< 16bit rgb565 + Alpha plane at end - 5 bits of the 8 being used per alpha byte */
   EVAS_COLORSPACE_GRY8, /**< 8bit grayscale */
   EVAS_COLORSPACE_YCBCR422601_PL, /**<  YCbCr 4:2:2, ITU.BT-601 specifications. The data pointed to is just an array of row pointer, pointing to line of Y,Cb,Y,Cr bytes */
   EVAS_COLORSPACE_YCBCR420NV12601_PL, /**< YCbCr 4:2:0, ITU.BT-601 specification. The data pointed to is just an array of row pointer, pointing to the Y rows, then the Cb,Cr rows. */
   EVAS_COLORSPACE_YCBCR420TM12601_PL, /**< YCbCr 4:2:0, ITU.BT-601 specification. The data pointed to is just an array of tiled row pointer, pointing to the Y rows, then the Cb,Cr rows. */
} Evas_Colorspace; /**< Colorspaces for pixel data supported by Evas */

/**
 * How to pack items into cells in a table.
 * @ingroup Evas_Object_Table
 *
 * @see evas_object_table_homogeneous_set() for an explanation of the function of
 * each one.
 */
typedef enum _Evas_Object_Table_Homogeneous_Mode
{
   EVAS_OBJECT_TABLE_HOMOGENEOUS_NONE = 0,
   EVAS_OBJECT_TABLE_HOMOGENEOUS_TABLE = 1,
   EVAS_OBJECT_TABLE_HOMOGENEOUS_ITEM = 2
} Evas_Object_Table_Homogeneous_Mode; /**< Table cell pack mode. */

typedef struct _Evas_Coord_Rectangle       Evas_Coord_Rectangle; /**< A generic rectangle handle */
typedef struct _Evas_Point                 Evas_Point;   /**< integer point */

typedef struct _Evas_Coord_Point           Evas_Coord_Point;    /**< Evas_Coord point */
typedef struct _Evas_Coord_Precision_Point Evas_Coord_Precision_Point;   /**< Evas_Coord point with sub-pixel precision */

typedef struct _Evas_Position              Evas_Position;   /**< associates given point in Canvas and Output */
typedef struct _Evas_Precision_Position    Evas_Precision_Position;   /**< associates given point in Canvas and Output, with sub-pixel precision */

/**
 * @typedef Evas_Smart_Class
 *
 * A smart object's @b base class definition
 *
 * @ingroup Evas_Smart_Group
 */
typedef struct _Evas_Smart_Class Evas_Smart_Class;

/**
 * @typedef Evas_Smart_Interface
 *
 * A smart object's @b base interface definition
 *
 * An Evas interface is exactly like the OO-concept: an 'contract' or
 * API a given object is declared to support. A smart object may have
 * more than one interface, thus extending the behavior it gets from
 * sub-classing.
 *
 * @since 1.7
 *
 * @ingroup Evas_Smart_Group
 */
typedef struct _Evas_Smart_Interface         Evas_Smart_Interface;

/**
 * @typedef Evas_Smart_Cb_Description
 *
 * A smart object callback description, used to provide introspection
 *
 * @ingroup Evas_Smart_Group
 */
typedef struct _Evas_Smart_Cb_Description Evas_Smart_Cb_Description;

/**
 * @typedef Evas_Map
 *
 * An opaque handle to map points
 *
 * @see evas_map_new()
 * @see evas_map_free()
 * @see evas_map_dup()
 *
 * @ingroup Evas_Object_Group_Map
 */
typedef struct _Evas_Map Evas_Map;

/**
 * @typedef Evas
 *
 * An opaque handle to an Evas canvas.
 *
 * @see evas_new()
 * @see evas_free()
 *
 * @ingroup Evas_Canvas
 */
typedef Eo                 Evas;

/**
 * @typedef Evas_Public_Data
 * Public data for an Evas.
 * @ingroup Evas_Canvas
 */
typedef struct _Evas_Public_Data  Evas_Public_Data;

/**
 * @typedef Evas_Object
 * An Evas Object handle.
 * @ingroup Evas_Object_Group
 */
typedef Eo                 Evas_Object;

/**
 * @typedef Evas_Object_Protected_Data
 * Protected data for an Evas Object.
 * @ingroup Evas_Object_Group
 */
typedef struct _Evas_Object_Protected_Data Evas_Object_Protected_Data;

typedef void                        Evas_Performance; /**< An Evas Performance handle */
typedef struct _Evas_Modifier       Evas_Modifier; /**< An opaque type containing information on which modifier keys are registered in an Evas canvas */
typedef struct _Evas_Lock           Evas_Lock; /**< An opaque type containing information on which lock keys are registered in an Evas canvas */
typedef struct _Evas_Smart          Evas_Smart; /**< An Evas Smart Object handle */
typedef struct _Evas_Native_Surface Evas_Native_Surface; /**< A generic datatype for engine specific native surface information */

/**
 * @typedef Evas_Video_Surface
 *
 * A generic datatype for video specific surface information
 * @see evas_object_image_video_surface_set
 * @see evas_object_image_video_surface_get
 * @since 1.1
 */
typedef struct _Evas_Video_Surface Evas_Video_Surface;

typedef unsigned long long         Evas_Modifier_Mask;  /**< An Evas modifier mask type */

typedef int                        Evas_Coord;
typedef int                        Evas_Font_Size;
typedef int                        Evas_Angle;

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
   Evas_Coord x; /**< x co-ordinate */
   Evas_Coord y; /**< y co-ordinate */
};

struct _Evas_Coord_Precision_Point
{
   Evas_Coord x, y;
   double     xsub, ysub;
};

struct _Evas_Position
{
   Evas_Point       output;
   Evas_Coord_Point canvas; /**< position on the canvas */
};

struct _Evas_Precision_Position
{
   Evas_Point                 output;
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

typedef enum _Evas_Display_Mode
{
   EVAS_DISPLAY_MODE_NONE = 0, /**<Default mode */
   EVAS_DISPLAY_MODE_INHERIT = 1, /**< Use this mode when object's display mode depend on ancestor's */
   EVAS_DISPLAY_MODE_COMPRESS = 2, /**< Use this mode want to give comppress display mode hint to object */
   EVAS_DISPLAY_MODE_EXPAND = 3, /**< Use this mode want to give expand display mode hint to object */
   EVAS_DISPLAY_MODE_DONT_CHANGE = 4 /**< Use this mode when object should not change display mode */
} Evas_Display_Mode; /**< object's display mode type related with compress/expand or etc mode */

typedef struct _Evas_Pixel_Import_Source Evas_Pixel_Import_Source; /**< A source description of pixels for importing pixels */
typedef struct _Evas_Engine_Info         Evas_Engine_Info; /**< A generic Evas Engine information structure */
typedef struct _Evas_Device              Evas_Device; /**< A source device handle - where the event came from */
typedef struct _Evas_Event_Mouse_Down    Evas_Event_Mouse_Down; /**< Event structure for #EVAS_CALLBACK_MOUSE_DOWN event callbacks */
typedef struct _Evas_Event_Mouse_Up      Evas_Event_Mouse_Up; /**< Event structure for #EVAS_CALLBACK_MOUSE_UP event callbacks */
typedef struct _Evas_Event_Mouse_In      Evas_Event_Mouse_In; /**< Event structure for #EVAS_CALLBACK_MOUSE_IN event callbacks */
typedef struct _Evas_Event_Mouse_Out     Evas_Event_Mouse_Out; /**< Event structure for #EVAS_CALLBACK_MOUSE_OUT event callbacks */
typedef struct _Evas_Event_Mouse_Move    Evas_Event_Mouse_Move; /**< Event structure for #EVAS_CALLBACK_MOUSE_MOVE event callbacks */
typedef struct _Evas_Event_Mouse_Wheel   Evas_Event_Mouse_Wheel; /**< Event structure for #EVAS_CALLBACK_MOUSE_WHEEL event callbacks */
typedef struct _Evas_Event_Multi_Down    Evas_Event_Multi_Down; /**< Event structure for #EVAS_CALLBACK_MULTI_DOWN event callbacks */
typedef struct _Evas_Event_Multi_Up      Evas_Event_Multi_Up; /**< Event structure for #EVAS_CALLBACK_MULTI_UP event callbacks */
typedef struct _Evas_Event_Multi_Move    Evas_Event_Multi_Move; /**< Event structure for #EVAS_CALLBACK_MULTI_MOVE event callbacks */
typedef struct _Evas_Event_Key_Down      Evas_Event_Key_Down; /**< Event structure for #EVAS_CALLBACK_KEY_DOWN event callbacks */
typedef struct _Evas_Event_Key_Up        Evas_Event_Key_Up; /**< Event structure for #EVAS_CALLBACK_KEY_UP event callbacks */
typedef struct _Evas_Event_Hold          Evas_Event_Hold; /**< Event structure for #EVAS_CALLBACK_HOLD event callbacks */

typedef enum _Evas_Load_Error
{
   EVAS_LOAD_ERROR_NONE = 0, /**< No error on load */
   EVAS_LOAD_ERROR_GENERIC = 1, /**< A non-specific error occurred */
   EVAS_LOAD_ERROR_DOES_NOT_EXIST = 2, /**< File (or file path) does not exist */
   EVAS_LOAD_ERROR_PERMISSION_DENIED = 3, /**< Permission denied to an existing file (or path) */
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
   EVAS_PIXEL_FORMAT_YUV420P_601 = 2 /**< YUV 420 Planar format with CCIR 601 color encoding with contiguous planes in the order Y, U and V */
} Evas_Pixel_Import_Pixel_Format; /**< Pixel format for import call. See evas_object_image_pixels_import() */

struct _Evas_Pixel_Import_Source
{
   Evas_Pixel_Import_Pixel_Format format; /**< pixel format type ie ARGB32, YUV420P_601 etc. */
   int                            w, h; /**< width and height of source in pixels */
   void                         **rows; /**< an array of pointers (size depends on format) pointing to left edge of each scanline */
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
   int                      version;
   Evas_Native_Surface_Type type;
   union {
      struct
      {
         void         *visual; /**< visual of the pixmap to use (Visual) */
         unsigned long pixmap; /**< pixmap id to use (Pixmap) */
      } x11;
      struct
      {
         unsigned int texture_id; /**< opengl texture id to use from glGenTextures() */
         unsigned int framebuffer_id; /**< 0 if not a FBO, FBO id otherwise from glGenFramebuffers() */
         unsigned int internal_format; /**< same as 'internalFormat' for glTexImage2D() */
         unsigned int format; /**< same as 'format' for glTexImage2D() */
         unsigned int x, y, w, h; /**< region inside the texture to use (image size is assumed as texture size, with 0, 0 being the top-left and co-ordinates working down to the right and bottom being positive) */
      } opengl;
   } data;
};

/**
 * @def EVAS_VIDEO_SURFACE_VERSION
 * Magic version number to know what the video surf struct looks like
 * @since 1.1
 */
#define EVAS_VIDEO_SURFACE_VERSION 1

typedef void (*Evas_Video_Cb)(void *data, Evas_Object *obj, const Evas_Video_Surface *surface);
typedef void (*Evas_Video_Coord_Cb)(void *data, Evas_Object *obj, const Evas_Video_Surface *surface, Evas_Coord a, Evas_Coord b);

struct _Evas_Video_Surface
{
   int                 version;

   Evas_Video_Coord_Cb move; /**< Move the video surface to this position */
   Evas_Video_Coord_Cb resize; /**< Resize the video surface to that size */
   Evas_Video_Cb       show; /**< Show the video overlay surface */
   Evas_Video_Cb       hide; /**< Hide the video overlay surface */
   Evas_Video_Cb       update_pixels; /**< Please update the Evas_Object_Image pixels when called */

   Evas_Object        *parent;
   void               *data;
};

#define EVAS_LAYER_MIN                   -32768 /**< bottom-most layer number */
#define EVAS_LAYER_MAX                   32767 /**< top-most layer number */

#define EVAS_COLOR_SPACE_ARGB            0 /**< Not used for anything */
#define EVAS_COLOR_SPACE_AHSV            1 /**< Not used for anything */
#define EVAS_TEXT_INVALID                -1 /**< Not used for anything */
#define EVAS_TEXT_SPECIAL                -2 /**< Not used for anything */

#define EVAS_HINT_EXPAND                 1.0 /**< Use with evas_object_size_hint_weight_set(), evas_object_size_hint_weight_get(), evas_object_size_hint_expand_set(), evas_object_size_hint_expand_get() */
#define EVAS_HINT_FILL                   -1.0 /**< Use with evas_object_size_hint_align_set(), evas_object_size_hint_align_get(), evas_object_size_hint_fill_set(), evas_object_size_hint_fill_get() */
#define evas_object_size_hint_fill_set   evas_object_size_hint_align_set /**< Convenience macro to make it easier to understand that align is also used for fill properties (as fill is mutually exclusive to align) */
#define evas_object_size_hint_fill_get   evas_object_size_hint_align_get /**< Convenience macro to make it easier to understand that align is also used for fill properties (as fill is mutually exclusive to align) */
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
   EVAS_RENDER_ADD = 4, /* d = d + s */
   EVAS_RENDER_ADD_REL = 5, /**< d = d + s*da */
   EVAS_RENDER_SUB = 6, /**< d = d - s */
   EVAS_RENDER_SUB_REL = 7, /* d = d - s*da */
   EVAS_RENDER_TINT = 8, /**< d = d*s + d*(1 - sa) + s*(1 - da) */
   EVAS_RENDER_TINT_REL = 9, /**< d = d*(1 - sa + s) */
   EVAS_RENDER_MASK = 10, /**< d = d*sa */
   EVAS_RENDER_MUL = 11 /**< d = d*s */
} Evas_Render_Op; /**< How the object should be rendered to output. */

typedef enum _Evas_Border_Fill_Mode
{
   EVAS_BORDER_FILL_NONE = 0, /**< Image's center region is @b not to be rendered */
   EVAS_BORDER_FILL_DEFAULT = 1, /**< Image's center region is to be @b blended with objects underneath it, if it has transparency. This is the default behavior for image objects */
   EVAS_BORDER_FILL_SOLID = 2 /**< Image's center region is to be made solid, even if it has transparency on it */
} Evas_Border_Fill_Mode; /**< How an image's center region (the complement to the border region) should be rendered by Evas */

typedef enum _Evas_Image_Scale_Hint
{
   EVAS_IMAGE_SCALE_HINT_NONE = 0, /**< No scale hint at all */
   EVAS_IMAGE_SCALE_HINT_DYNAMIC = 1, /**< Image is being re-scaled over time, thus turning scaling cache @b off for its data */
   EVAS_IMAGE_SCALE_HINT_STATIC = 2 /**< Image is not being re-scaled over time, thus turning scaling cache @b on for its data */
} Evas_Image_Scale_Hint; /**< How an image's data is to be treated by Evas, with regard to scaling cache */

typedef enum _Evas_Image_Animated_Loop_Hint
{
   EVAS_IMAGE_ANIMATED_HINT_NONE = 0,
   EVAS_IMAGE_ANIMATED_HINT_LOOP = 1, /**< Image's animation mode is loop like 1->2->3->1->2->3 */
   EVAS_IMAGE_ANIMATED_HINT_PINGPONG = 2 /**< Image's animation mode is pingpong like 1->2->3->2->1-> ... */
} Evas_Image_Animated_Loop_Hint;

typedef enum _Evas_Engine_Render_Mode
{
   EVAS_RENDER_MODE_BLOCKING = 0,
   EVAS_RENDER_MODE_NONBLOCKING = 1,
} Evas_Engine_Render_Mode;

typedef enum _Evas_Image_Content_Hint
{
   EVAS_IMAGE_CONTENT_HINT_NONE = 0, /**< No hint at all */
   EVAS_IMAGE_CONTENT_HINT_DYNAMIC = 1, /**< The contents will change over time */
   EVAS_IMAGE_CONTENT_HINT_STATIC = 2 /**< The contents won't change over time */
} Evas_Image_Content_Hint; /**< How an image's data is to be treated by Evas, for optimization */

typedef enum _Evas_Device_Class
{
   EVAS_DEVICE_CLASS_NONE, /**< Not a device @since 1.8 */
   EVAS_DEVICE_CLASS_SEAT, /**< The user/seat (the user themselves) @since 1.8 */
   EVAS_DEVICE_CLASS_KEYBOARD, /**< A regular keyboard, numberpad or attached buttons @since 1.8 */
   EVAS_DEVICE_CLASS_MOUSE, /**< A mouse, trackball or touchpad relative motion device @since 1.8 */
   EVAS_DEVICE_CLASS_TOUCH, /**< A touchscreen with fingers or stylus @since 1.8 */
   EVAS_DEVICE_CLASS_PEN, /**< A special pen device @since 1.8 */
   EVAS_DEVICE_CLASS_POINTER, /**< A laser pointer, wii-style or "minority report" pointing device @since 1.8 */
   EVAS_DEVICE_CLASS_GAMEPAD /**<  A gamepad controller or joystick @since 1.8 */
} Evas_Device_Class;
   
struct _Evas_Engine_Info /** Generic engine information. Generic info is useless */
{
   int magic; /**< Magic number */
};

struct _Evas_Event_Mouse_Down /** Mouse button press event */
{
   int               button; /**< Mouse button number that went down (1 - 32) */

   Evas_Point        output; /**< The X/Y location of the cursor */
   Evas_Coord_Point  canvas; /**< The X/Y location of the cursor */

   void             *data;
   Evas_Modifier    *modifiers; /**< modifier keys pressed during the event */
   Evas_Lock        *locks;

   Evas_Button_Flags flags; /**< button flags set during the event */
   unsigned int      timestamp;
   Evas_Event_Flags  event_flags;
   Evas_Device      *dev;
};

struct _Evas_Event_Mouse_Up /** Mouse button release event */
{
   int               button; /**< Mouse button number that was raised (1 - 32) */

   Evas_Point        output; /**< The X/Y location of the cursor */
   Evas_Coord_Point  canvas; /**< The X/Y location of the cursor */

   void             *data;
   Evas_Modifier    *modifiers; /**< modifier keys pressed during the event */
   Evas_Lock        *locks;

   Evas_Button_Flags flags; /**< button flags set during the event */
   unsigned int      timestamp;
   Evas_Event_Flags  event_flags;
   Evas_Device      *dev;
};

struct _Evas_Event_Mouse_In /** Mouse enter event */
{
   int              buttons; /**< Button pressed mask, Bits set to 1 are buttons currently pressed (bit 0 = mouse button 1, bit 1 = mouse button 2 etc.) */

   Evas_Point        output; /**< The X/Y location of the cursor */
   Evas_Coord_Point  canvas; /**< The X/Y location of the cursor */

   void            *data;
   Evas_Modifier    *modifiers; /**< modifier keys pressed during the event */
   Evas_Lock       *locks;
   unsigned int     timestamp;
   Evas_Event_Flags event_flags;
   Evas_Device     *dev;
};

struct _Evas_Event_Mouse_Out /** Mouse leave event */
{
   int              buttons; /**< Button pressed mask, Bits set to 1 are buttons currently pressed (bit 0 = mouse button 1, bit 1 = mouse button 2 etc.) */

   Evas_Point        output; /**< The X/Y location of the cursor */
   Evas_Coord_Point  canvas; /**< The X/Y location of the cursor */

   void            *data;
   Evas_Modifier    *modifiers; /**< modifier keys pressed during the event */
   Evas_Lock       *locks;
   unsigned int     timestamp;
   Evas_Event_Flags event_flags;
   Evas_Device     *dev;
};

struct _Evas_Event_Mouse_Move /** Mouse move event */
{
   int              buttons; /**< Button pressed mask, Bits set to 1 are buttons currently pressed (bit 0 = mouse button 1, bit 1 = mouse button 2 etc.) */

   Evas_Position    cur; /**< Current mouse position */
   Evas_Position    prev; /**< Previous mouse position */

   void            *data;
   Evas_Modifier    *modifiers; /**< modifier keys pressed during the event */
   Evas_Lock       *locks;
   unsigned int     timestamp;
   Evas_Event_Flags event_flags;
   Evas_Device     *dev;
};

struct _Evas_Event_Mouse_Wheel /** Wheel event */
{
   int              direction; /* 0 = default up/down wheel FIXME: more wheel types */
   int              z; /* ...,-2,-1 = down, 1,2,... = up */

   Evas_Point        output; /**< The X/Y location of the cursor */
   Evas_Coord_Point  canvas; /**< The X/Y location of the cursor */

   void            *data;
   Evas_Modifier    *modifiers; /**< modifier keys pressed during the event */
   Evas_Lock       *locks;
   unsigned int     timestamp;
   Evas_Event_Flags event_flags;
   Evas_Device     *dev;
};

struct _Evas_Event_Multi_Down /** Multi button press event */
{
   int                        device; /**< Multi device number that went down (1 or more for extra touches) */
   double                     radius, radius_x, radius_y;
   double                     pressure, angle;

   Evas_Point                 output;
   Evas_Coord_Precision_Point canvas;

   void                      *data;
   Evas_Modifier    *modifiers; /**< modifier keys pressed during the event */
   Evas_Lock                 *locks;

   Evas_Button_Flags flags; /**< button flags set during the event */
   unsigned int               timestamp;
   Evas_Event_Flags           event_flags;
   Evas_Device               *dev;
};

struct _Evas_Event_Multi_Up /** Multi button release event */
{
   int                        device; /**< Multi device number that went up (1 or more for extra touches) */
   double                     radius, radius_x, radius_y;
   double                     pressure, angle;

   Evas_Point                 output;
   Evas_Coord_Precision_Point canvas;

   void                      *data;
   Evas_Modifier    *modifiers; /**< modifier keys pressed during the event */
   Evas_Lock                 *locks;

   Evas_Button_Flags flags; /**< button flags set during the event */
   unsigned int               timestamp;
   Evas_Event_Flags           event_flags;
   Evas_Device               *dev;
};

struct _Evas_Event_Multi_Move /** Multi button down event */
{
   int                     device; /**< Multi device number that moved (1 or more for extra touches) */
   double                  radius, radius_x, radius_y;
   double                  pressure, angle;

   Evas_Precision_Position cur;

   void                   *data;
   Evas_Modifier    *modifiers; /**< modifier keys pressed during the event */
   Evas_Lock              *locks;
   unsigned int            timestamp;
   Evas_Event_Flags        event_flags;
   Evas_Device            *dev;
};

struct _Evas_Event_Key_Down /** Key press event */
{
   char            *keyname; /**< the name string of the key pressed */
   void            *data;
   Evas_Modifier    *modifiers; /**< modifier keys pressed during the event */
   Evas_Lock       *locks;

   const char      *key; /**< The logical key : (eg shift+1 == exclamation) */
   const char      *string; /**< A UTF8 string if this keystroke has produced a visible string to be ADDED */
   const char      *compose; /**< A UTF8 string if this keystroke has modified a string in the middle of being composed - this string replaces the previous one */
   unsigned int     timestamp;
   Evas_Event_Flags event_flags;
   Evas_Device     *dev;
};

struct _Evas_Event_Key_Up /** Key release event */
{
   char            *keyname; /**< the name string of the key released */
   void            *data;
   Evas_Modifier    *modifiers; /**< modifier keys pressed during the event */
   Evas_Lock       *locks;

   const char      *key; /**< The logical key : (eg shift+1 == exclamation) */
   const char      *string; /**< A UTF8 string if this keystroke has produced a visible string to be ADDED */
   const char      *compose; /**< A UTF8 string if this keystroke has modified a string in the middle of being composed - this string replaces the previous one */
   unsigned int     timestamp;
   Evas_Event_Flags event_flags;
   Evas_Device     *dev;
};

struct _Evas_Event_Hold /** Hold change event */
{
   int              hold; /**< The hold flag */
   void            *data;

   unsigned int     timestamp;
   Evas_Event_Flags event_flags;
   Evas_Device     *dev;
};

/**
 * How the mouse pointer should be handled by Evas.
 *
 * In the mode #EVAS_OBJECT_POINTER_MODE_AUTOGRAB, when a mouse button
 * is pressed down over an object and held, with the mouse pointer
 * being moved outside of it, the pointer still behaves as being bound
 * to that object, albeit out of its drawing region. When the button
 * is released, the event will be fed to the object, that may check if
 * the final position is over it or not and do something about it.
 *
 * In the mode #EVAS_OBJECT_POINTER_MODE_NOGRAB, the pointer will
 * always be bound to the object right below it.
 *
 * @ingroup Evas_Object_Group_Extras
 */
typedef enum _Evas_Object_Pointer_Mode
{
   EVAS_OBJECT_POINTER_MODE_AUTOGRAB, /**< default, X11-like */
   EVAS_OBJECT_POINTER_MODE_NOGRAB, /**< pointer always bound to the object right below it */
   EVAS_OBJECT_POINTER_MODE_NOGRAB_NO_REPEAT_UPDOWN /**< useful on object with "repeat events" enabled, where mouse/touch up and down events WONT be repeated to objects and these objects wont be auto-grabbed. @since 1.2 */
} Evas_Object_Pointer_Mode; /**< How the mouse pointer should be handled by Evas. */

typedef void      (*Evas_Smart_Cb)(void *data, Evas_Object *obj, void *event_info);  /**< Evas smart objects' "smart callback" function signature */
typedef void      (*Evas_Event_Cb)(void *data, Evas *e, void *event_info);  /**< Evas event callback function signature */
typedef Eina_Bool (*Evas_Object_Event_Post_Cb)(void *data, Evas *e);
typedef void      (*Evas_Object_Event_Cb)(void *data, Evas *e, Evas_Object *obj, void *event_info);  /**< Evas object event callback function signature */
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
 * number of calls to it. It returns the new counter's value.
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
EAPI int               evas_init(void);

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
EAPI int               evas_shutdown(void);

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
EAPI Evas_Alloc_Error  evas_alloc_error(void);

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
EAPI int               evas_async_events_fd_get(void) EINA_WARN_UNUSED_RESULT;

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
EAPI int               evas_async_events_process(void);

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
EAPI Eina_Bool         evas_async_events_put(const void *target, Evas_Callback_Type type, void *event_info, Evas_Async_Events_Put_Cb func) EINA_ARG_NONNULL(1, 4);

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
 *
 * Some of the functions in this group are exemplified @ref
 * Example_Evas_Events "here".
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
 * @return A new uninitialised Evas canvas on success. Otherwise, @c NULL.
 * @ingroup Evas_Canvas
 */
EAPI Evas             *evas_new(void) EINA_WARN_UNUSED_RESULT EINA_MALLOC;

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
EAPI void              evas_free(Evas *e)  EINA_ARG_NONNULL(1);

/**
 * Inform to the evas that it got the focus.
 *
 * @param e The evas to change information.
 * @ingroup Evas_Canvas
 */
EAPI void              evas_focus_in(Evas *e);

/**
 * Inform to the evas that it lost the focus.
 *
 * @param e The evas to change information.
 * @ingroup Evas_Canvas
 */
EAPI void              evas_focus_out(Evas *e);

/**
 * Get the focus state known by the given evas
 *
 * @param e The evas to query information.
 * @ingroup Evas_Canvas
 */
EAPI Eina_Bool         evas_focus_state_get(const Evas *e);

/**
 * Push the nochange flag up 1
 *
 * This tells evas, that while the nochange flag is greater than 0, do not
 * mark objects as "changed" when making changes.
 *
 * @param e The evas to change information.
 * @ingroup Evas_Canvas
 */
EAPI void              evas_nochange_push(Evas *e);

/**
 * Pop the nochange flag down 1
 *
 * This tells evas, that while the nochange flag is greater than 0, do not
 * mark objects as "changed" when making changes.
 *
 * @param e The evas to change information.
 * @ingroup Evas_Canvas
 */
EAPI void              evas_nochange_pop(Evas *e);

/**
 * Attaches a specific pointer to the evas for fetching later
 *
 * @param e The canvas to attach the pointer to
 * @param data The pointer to attach
 * @ingroup Evas_Canvas
 */
EAPI void              evas_data_attach_set(Evas *e, void *data) EINA_ARG_NONNULL(1);

/**
 * Returns the pointer attached by evas_data_attach_set()
 *
 * @param e The canvas to attach the pointer to
 * @return The pointer attached
 * @ingroup Evas_Canvas
 */
EAPI void             *evas_data_attach_get(const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

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
 * @note All newly created Evas rectangles get the default color values of 255 255 255 255 (opaque white).
 *
 * @ingroup Evas_Canvas
 */
EAPI void              evas_damage_rectangle_add(Evas *e, int x, int y, int w, int h) EINA_ARG_NONNULL(1);

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
 * this rendering (partial) freeze is something else (most probably
 * other canvas) being on top of the specified rectangular region,
 * thus shading it completely from the user's final scene in a
 * display. To avoid unnecessary processing, one should indicate to the
 * obscured canvas not to bother about the non-important area.
 *
 * The majority of users won't have to worry about this function, as
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
EAPI void              evas_obscured_rectangle_add(Evas *e, int x, int y, int w, int h) EINA_ARG_NONNULL(1);

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
EAPI void              evas_obscured_clear(Evas *e) EINA_ARG_NONNULL(1);

/**
 * Force immediate renderization of the given Evas canvas.
 *
 * @param e The given canvas pointer.
 * @return A newly allocated list of updated rectangles of the canvas
 *        (@c Eina_Rectangle structs). Free this list with
 *        evas_render_updates_free().
 *
 * This function forces an immediate renderization update of the given
 * canvas @p e.
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
EAPI Eina_List        *evas_render_updates(Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

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
EAPI void              evas_render_updates_free(Eina_List *updates);

/**
 * Force renderization of the given canvas.
 *
 * @param e The given canvas pointer.
 *
 * @ingroup Evas_Canvas
 */
EAPI void              evas_render(Evas *e) EINA_ARG_NONNULL(1);

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
EAPI void              evas_norender(Evas *e) EINA_ARG_NONNULL(1);

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
EAPI void              evas_render_idle_flush(Evas *e) EINA_ARG_NONNULL(1);

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
EAPI void              evas_render_dump(Evas *e) EINA_ARG_NONNULL(1);

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
 * @param name the name string of an engine
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
EAPI int               evas_render_method_lookup(const char *name) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * List all the rendering engines compiled into the copy of the Evas library
 *
 * @return A linked list whose data members are C strings of engine names
 * @ingroup Evas_Output_Method
 *
 * Calling this will return a handle (pointer) to an Evas linked
 * list. Each node in the linked list will have the data pointer be a
 * (char *) pointer to the name string of the rendering engine
 * available. The strings should never be modified, neither should the
 * list be modified. This list should be cleaned up as soon as the
 * program no longer needs it using evas_render_method_list_free(). If
 * no engines are available from Evas, @c NULL will be returned.
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
EAPI Eina_List        *evas_render_method_list(void) EINA_WARN_UNUSED_RESULT;

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
EAPI void              evas_render_method_list_free(Eina_List *list);

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
EAPI void              evas_output_method_set(Evas *e, int render_method) EINA_ARG_NONNULL(1);

/**
 * Retrieves the number of the output engine used for the given evas.
 * @param   e The given evas.
 * @return  The ID number of the output engine being used.  @c 0 is
 *          returned if there is an error.
 * @ingroup Evas_Output_Method
 */
EAPI int               evas_output_method_get(const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

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
EAPI Evas_Engine_Info *evas_engine_info_get(const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

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
 * @return  @c EINA_TRUE if no error occurred, @c EINA_FALSE otherwise.
 * @ingroup Evas_Output_Method
 */
EAPI Eina_Bool         evas_engine_info_set(Evas *e, Evas_Engine_Info *info) EINA_ARG_NONNULL(1);

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
EAPI void              evas_output_size_set(Evas *e, int w, int h) EINA_ARG_NONNULL(1);

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
EAPI void              evas_output_size_get(const Evas *e, int *w, int *h) EINA_ARG_NONNULL(1);

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
EAPI void              evas_output_viewport_set(Evas *e, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h) EINA_ARG_NONNULL(1);

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
 * x, @p y, @p w or @p h that are @c NULL will not be written to. If @p e
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
EAPI void              evas_output_viewport_get(const Evas *e, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h) EINA_ARG_NONNULL(1);

/**
 * Sets the output framespace size of the render engine of the given evas.
 *
 * The framespace size is used in the Wayland engines to denote space where
 * the output is not drawn. This is mainly used in ecore_evas to draw borders
 *
 * The units used for @p w and @p h depend on the engine used by the
 * evas.
 *
 * @param   e The given evas.
 * @param   x The left coordinate in output units, usually pixels.
 * @param   y The top coordinate in output units, usually pixels.
 * @param   w The width in output units, usually pixels.
 * @param   h The height in output units, usually pixels.
 * @ingroup Evas_Output_Size
 * @since 1.1
 */
EAPI void              evas_output_framespace_set(Evas *e, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h);

/**
 * Get the render engine's output framespace co-ordinates in canvas units.
 *
 * @param e The pointer to the Evas Canvas
 * @param x The pointer to a x variable to be filled in
 * @param y The pointer to a y variable to be filled in
 * @param w The pointer to a width variable to be filled in
 * @param h The pointer to a height variable to be filled in
 * @ingroup Evas_Output_Size
 * @since 1.1
 */
EAPI void              evas_output_framespace_get(const Evas *e, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h);

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
EAPI Evas_Coord        evas_coord_screen_x_to_world(const Evas *e, int x) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

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
EAPI Evas_Coord        evas_coord_screen_y_to_world(const Evas *e, int y) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

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
EAPI int               evas_coord_world_x_to_screen(const Evas *e, Evas_Coord x) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

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
EAPI int               evas_coord_world_y_to_screen(const Evas *e, Evas_Coord y) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

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
EAPI void              evas_pointer_output_xy_get(const Evas *e, int *x, int *y) EINA_ARG_NONNULL(1);

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
EAPI void              evas_pointer_canvas_xy_get(const Evas *e, Evas_Coord *x, Evas_Coord *y) EINA_ARG_NONNULL(1);

/**
 * Returns a bitmask with the mouse buttons currently pressed, set to 1
 *
 * @param e The pointer to the Evas Canvas
 * @return A bitmask of the currently depressed buttons on the canvas
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
EAPI int               evas_pointer_button_down_mask_get(const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

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
EAPI Eina_Bool         evas_pointer_inside_get(const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

EAPI void              evas_sync(Evas *e) EINA_ARG_NONNULL(1);

#define EVAS_CLASS evas_class_get()

const Eo_Class *evas_class_get(void) EINA_CONST;

extern EAPI Eo_Op EVAS_CANVAS_BASE_ID;

enum
{
   EVAS_CANVAS_SUB_ID_OUTPUT_METHOD_SET,
   EVAS_CANVAS_SUB_ID_OUTPUT_METHOD_GET,
   EVAS_CANVAS_SUB_ID_ENGINE_INFO_GET,
   EVAS_CANVAS_SUB_ID_ENGINE_INFO_SET,
   EVAS_CANVAS_SUB_ID_OUTPUT_SIZE_SET,
   EVAS_CANVAS_SUB_ID_OUTPUT_SIZE_GET,
   EVAS_CANVAS_SUB_ID_OUTPUT_VIEWPORT_SET,
   EVAS_CANVAS_SUB_ID_OUTPUT_VIEWPORT_GET,
   EVAS_CANVAS_SUB_ID_OUTPUT_FRAMESPACE_SET,
   EVAS_CANVAS_SUB_ID_OUTPUT_FRAMESPACE_GET,
   EVAS_CANVAS_SUB_ID_COORD_SCREEN_X_TO_WORLD,
   EVAS_CANVAS_SUB_ID_COORD_SCREEN_Y_TO_WORLD,
   EVAS_CANVAS_SUB_ID_COORD_WORLD_X_TO_SCREEN,
   EVAS_CANVAS_SUB_ID_COORD_WORLD_Y_TO_SCREEN,
   EVAS_CANVAS_SUB_ID_POINTER_OUTPUT_XY_GET,
   EVAS_CANVAS_SUB_ID_POINTER_CANVAS_XY_GET,
   EVAS_CANVAS_SUB_ID_POINTER_BUTTON_DOWN_MASK_GET,
   EVAS_CANVAS_SUB_ID_POINTER_INSIDE_GET,
   EVAS_CANVAS_SUB_ID_DATA_ATTACH_SET,
   EVAS_CANVAS_SUB_ID_DATA_ATTACH_GET,
   EVAS_CANVAS_SUB_ID_FOCUS_IN,
   EVAS_CANVAS_SUB_ID_FOCUS_OUT,
   EVAS_CANVAS_SUB_ID_FOCUS_STATE_GET,
   EVAS_CANVAS_SUB_ID_NOCHANGE_PUSH,
   EVAS_CANVAS_SUB_ID_NOCHANGE_POP,
   EVAS_CANVAS_SUB_ID_EVENT_DEFAULT_FLAGS_SET,
   EVAS_CANVAS_SUB_ID_EVENT_DEFAULT_FLAGS_GET,
   EVAS_CANVAS_SUB_ID_EVENT_FEED_MOUSE_DOWN,
   EVAS_CANVAS_SUB_ID_EVENT_FEED_MOUSE_UP,
   EVAS_CANVAS_SUB_ID_EVENT_FEED_MOUSE_CANCEL,
   EVAS_CANVAS_SUB_ID_EVENT_FEED_MOUSE_WHEEL,
   EVAS_CANVAS_SUB_ID_EVENT_FEED_MOUSE_MOVE,
   EVAS_CANVAS_SUB_ID_EVENT_FEED_MOUSE_IN,
   EVAS_CANVAS_SUB_ID_EVENT_FEED_MOUSE_OUT,
   EVAS_CANVAS_SUB_ID_EVENT_FEED_MULTI_DOWN,
   EVAS_CANVAS_SUB_ID_EVENT_FEED_MULTI_UP,
   EVAS_CANVAS_SUB_ID_EVENT_FEED_MULTI_MOVE,
   EVAS_CANVAS_SUB_ID_EVENT_FEED_KEY_DOWN,
   EVAS_CANVAS_SUB_ID_EVENT_FEED_KEY_UP,
   EVAS_CANVAS_SUB_ID_EVENT_FEED_HOLD,
   EVAS_CANVAS_SUB_ID_EVENT_REFEED_EVENT,
   EVAS_CANVAS_SUB_ID_EVENT_DOWN_COUNT_GET,
   EVAS_CANVAS_SUB_ID_FOCUS_GET,
   EVAS_CANVAS_SUB_ID_FONT_PATH_CLEAR,
   EVAS_CANVAS_SUB_ID_FONT_PATH_APPEND,
   EVAS_CANVAS_SUB_ID_FONT_PATH_PREPEND,
   EVAS_CANVAS_SUB_ID_FONT_PATH_LIST,
   EVAS_CANVAS_SUB_ID_FONT_HINTING_SET,
   EVAS_CANVAS_SUB_ID_FONT_HINTING_GET,
   EVAS_CANVAS_SUB_ID_FONT_HINTING_CAN_HINT,
   EVAS_CANVAS_SUB_ID_FONT_CACHE_FLUSH,
   EVAS_CANVAS_SUB_ID_FONT_CACHE_SET,
   EVAS_CANVAS_SUB_ID_FONT_CACHE_GET,
   EVAS_CANVAS_SUB_ID_FONT_AVAILABLE_LIST,
   EVAS_CANVAS_SUB_ID_KEY_MODIFIER_GET,
   EVAS_CANVAS_SUB_ID_KEY_LOCK_GET,
   EVAS_CANVAS_SUB_ID_KEY_MODIFIER_ADD,
   EVAS_CANVAS_SUB_ID_KEY_MODIFIER_DEL,
   EVAS_CANVAS_SUB_ID_KEY_LOCK_ADD,
   EVAS_CANVAS_SUB_ID_KEY_LOCK_DEL,
   EVAS_CANVAS_SUB_ID_KEY_MODIFIER_ON,
   EVAS_CANVAS_SUB_ID_KEY_MODIFIER_OFF,
   EVAS_CANVAS_SUB_ID_KEY_LOCK_ON,
   EVAS_CANVAS_SUB_ID_KEY_LOCK_OFF,
   EVAS_CANVAS_SUB_ID_KEY_MODIFIER_MASK_GET,
   EVAS_CANVAS_SUB_ID_DAMAGE_RECTANGLE_ADD,
   EVAS_CANVAS_SUB_ID_OBSCURED_RECTANGLE_ADD,
   EVAS_CANVAS_SUB_ID_OBSCURED_CLEAR,
   EVAS_CANVAS_SUB_ID_RENDER_UPDATES,
   EVAS_CANVAS_SUB_ID_RENDER,
   EVAS_CANVAS_SUB_ID_NORENDER,
   EVAS_CANVAS_SUB_ID_RENDER_IDLE_FLUSH,
   EVAS_CANVAS_SUB_ID_SYNC,
   EVAS_CANVAS_SUB_ID_RENDER_DUMP,
   EVAS_CANVAS_SUB_ID_OBJECT_BOTTOM_GET,
   EVAS_CANVAS_SUB_ID_OBJECT_TOP_GET,
   EVAS_CANVAS_SUB_ID_TOUCH_POINT_LIST_COUNT,
   EVAS_CANVAS_SUB_ID_TOUCH_POINT_LIST_NTH_XY_GET,
   EVAS_CANVAS_SUB_ID_TOUCH_POINT_LIST_NTH_ID_GET,
   EVAS_CANVAS_SUB_ID_TOUCH_POINT_LIST_NTH_STATE_GET,
   EVAS_CANVAS_SUB_ID_IMAGE_CACHE_FLUSH,
   EVAS_CANVAS_SUB_ID_IMAGE_CACHE_RELOAD,
   EVAS_CANVAS_SUB_ID_IMAGE_CACHE_SET,
   EVAS_CANVAS_SUB_ID_IMAGE_CACHE_GET,
   EVAS_CANVAS_SUB_ID_IMAGE_MAX_SIZE_GET,
   EVAS_CANVAS_SUB_ID_OBJECT_NAME_FIND,
   EVAS_CANVAS_SUB_ID_OBJECT_TOP_AT_XY_GET,
   EVAS_CANVAS_SUB_ID_OBJECT_TOP_IN_RECTANGLE_GET,
   EVAS_CANVAS_SUB_ID_OBJECTS_AT_XY_GET,
   EVAS_CANVAS_SUB_ID_OBJECTS_IN_RECTANGLE_GET,
   EVAS_CANVAS_SUB_ID_SMART_OBJECTS_CALCULATE,
   EVAS_CANVAS_SUB_ID_SMART_OBJECTS_CALCULATE_COUNT_GET,
   EVAS_CANVAS_SUB_ID_LAST
};

#define EVAS_CANVAS_ID(sub_id) (EVAS_CANVAS_BASE_ID + sub_id)


/**
 * @def evas_canvas_output_method_set
 * @since 1.8
 *
 * Sets the output engine for the given evas.
 *
 * @param[in] render_method
 *
 * @see evas_output_method_set
 */
#define evas_canvas_output_method_set(render_method) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_OUTPUT_METHOD_SET), EO_TYPECHECK(int, render_method)

/**
 * @def evas_canvas_output_method_get
 * @since 1.8
 *
 * Retrieves the number of the output engine used for the given evas.
 *
 * @param[out] ret
 *
 * @see evas_output_method_get
 */
#define evas_canvas_output_method_get(ret) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_OUTPUT_METHOD_GET), EO_TYPECHECK(int *, ret)

/**
 * @def evas_canvas_engine_info_get
 * @since 1.8
 *
 * Retrieves the current render engine info struct from the given evas.
 *
 * @param[out] ret
 *
 * @see evas_engine_info_get
 */
#define evas_canvas_engine_info_get(ret) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_ENGINE_INFO_GET), EO_TYPECHECK(Evas_Engine_Info **, ret)

/**
 * @def evas_canvas_engine_info_set
 * @since 1.8
 *
 * Applies the engine settings for the given evas from the given @c
 * Evas_Engine_Info structure.
 *
 * @param[in] info
 * @param[out] ret
 *
 * @see evas_engine_info_set
 */
#define evas_canvas_engine_info_set(info, ret) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_ENGINE_INFO_SET), EO_TYPECHECK(Evas_Engine_Info *, info), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def evas_canvas_output_size_set
 * @since 1.8
 *
 * Sets the output size of the render engine of the given evas.
 *
 * @param[in] w
 * @param[in] h
 *
 * @see evas_output_size_set
 */
#define evas_canvas_output_size_set(w, h) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_OUTPUT_SIZE_SET), EO_TYPECHECK(int, w), EO_TYPECHECK(int, h)

/**
 * @def evas_canvas_output_size_get
 * @since 1.8
 *
 * Retrieve the output size of the render engine of the given evas.
 *
 * @param[out] w
 * @param[out] h
 *
 * @see evas_output_size_get
 */
#define evas_canvas_output_size_get(w, h) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_OUTPUT_SIZE_GET), EO_TYPECHECK(int *, w), EO_TYPECHECK(int *, h)

/**
 * @def evas_canvas_output_viewport_set
 * @since 1.8
 *
 * Sets the output viewport of the given evas in evas units.
 *
 * @param[in] x
 * @param[in] y
 * @param[in] w
 * @param[in] h
 *
 * @see evas_output_viewport_set
 */
#define evas_canvas_output_viewport_set(x, y, w, h) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_OUTPUT_VIEWPORT_SET), EO_TYPECHECK(Evas_Coord, x), EO_TYPECHECK(Evas_Coord, y), EO_TYPECHECK(Evas_Coord, w), EO_TYPECHECK(Evas_Coord, h)

/**
 * @def evas_canvas_output_viewport_get
 * @since 1.8
 *
 * Get the render engine's output viewport co-ordinates in canvas units.
 *
 * @param[out] x
 * @param[out] y
 * @param[out] w
 * @param[out] h
 *
 * @see evas_output_viewport_get
 */
#define evas_canvas_output_viewport_get(x, y, w, h) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_OUTPUT_VIEWPORT_GET), EO_TYPECHECK(Evas_Coord *, x), EO_TYPECHECK(Evas_Coord *, y), EO_TYPECHECK(Evas_Coord *, w), EO_TYPECHECK(Evas_Coord *, h)

/**
 * @def evas_canvas_output_framespace_set
 * @since 1.8
 *
 * Sets the output framespace size of the render engine of the given evas.
 *
 * @param[in] x
 * @param[in] y
 * @param[in] w
 * @param[in] h
 *
 * @see evas_output_framespace_set
 */
#define evas_canvas_output_framespace_set(x, y, w, h) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_OUTPUT_FRAMESPACE_SET), EO_TYPECHECK(Evas_Coord, x), EO_TYPECHECK(Evas_Coord, y), EO_TYPECHECK(Evas_Coord, w), EO_TYPECHECK(Evas_Coord, h)

/**
 * @def evas_canvas_output_framespace_get
 * @since 1.8
 *
 * Get the render engine's output framespace co-ordinates in canvas units.
 *
 * @param[out] x
 * @param[out] y
 * @param[out] w
 * @param[out] h
 *
 * @see evas_output_framespace_get
 */
#define evas_canvas_output_framespace_get(x, y, w, h) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_OUTPUT_FRAMESPACE_GET), EO_TYPECHECK(Evas_Coord *, x), EO_TYPECHECK(Evas_Coord *, y), EO_TYPECHECK(Evas_Coord *, w), EO_TYPECHECK(Evas_Coord *, h)

/**
 * @def evas_canvas_coord_screen_x_to_world
 * @since 1.8
 *
 * Convert/scale an ouput screen co-ordinate into canvas co-ordinates
 *
 * @param[in] x
 * @param[out] ret
 *
 * @see evas_coord_screen_x_to_world
 */
#define evas_canvas_coord_screen_x_to_world(x, ret) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_COORD_SCREEN_X_TO_WORLD), EO_TYPECHECK(int, x), EO_TYPECHECK(Evas_Coord *, ret)

/**
 * @def evas_canvas_coord_screen_y_to_world
 * @since 1.8
 *
 * Convert/scale an ouput screen co-ordinate into canvas co-ordinates
 *
 * @param[in] y
 * @param[out] ret
 *
 * @see evas_coord_screen_y_to_world
 */
#define evas_canvas_coord_screen_y_to_world(y, ret) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_COORD_SCREEN_Y_TO_WORLD), EO_TYPECHECK(int, y), EO_TYPECHECK(Evas_Coord *, ret)

/**
 * @def evas_canvas_coord_world_x_to_screen
 * @since 1.8
 *
 * Convert/scale a canvas co-ordinate into output screen co-ordinates
 *
 * @param[in] x
 * @param[out] ret
 *
 * @see evas_coord_world_x_to_screen
 */
#define evas_canvas_coord_world_x_to_screen(x, ret) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_COORD_WORLD_X_TO_SCREEN), EO_TYPECHECK(Evas_Coord, x), EO_TYPECHECK(int *, ret)

/**
 * @def evas_canvas_coord_world_y_to_screen
 * @since 1.8
 *
 * Convert/scale a canvas co-ordinate into output screen co-ordinates
 *
 * @param[in] y
 * @param[out] ret
 *
 * @see evas_coord_world_y_to_screen
 */
#define evas_canvas_coord_world_y_to_screen(y, ret) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_COORD_WORLD_Y_TO_SCREEN), EO_TYPECHECK(Evas_Coord, y), EO_TYPECHECK(int *, ret)

/**
 * @def evas_canvas_pointer_output_xy_get
 * @since 1.8
 *
 * This function returns the current known pointer co-ordinates
 *
 * @param[out] x
 * @param[out] y
 *
 * @see evas_pointer_output_xy_get
 */
#define evas_canvas_pointer_output_xy_get(x, y) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_POINTER_OUTPUT_XY_GET), EO_TYPECHECK(int *, x), EO_TYPECHECK(int *, y)

/**
 * @def evas_canvas_pointer_canvas_xy_get
 * @since 1.8
 *
 * This function returns the current known pointer co-ordinates
 *
 * @param[out] x
 * @param[out] y
 *
 * @see evas_pointer_canvas_xy_get
 */
#define evas_canvas_pointer_canvas_xy_get(x, y) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_POINTER_CANVAS_XY_GET), EO_TYPECHECK(Evas_Coord *, x), EO_TYPECHECK(Evas_Coord *, y)

/**
 * @def evas_canvas_pointer_button_down_mask_get
 * @since 1.8
 *
 * Returns a bitmask with the mouse buttons currently pressed, set to 1
 *
 * @param[out] ret
 *
 * @see evas_pointer_button_down_mask_get
 */
#define evas_canvas_pointer_button_down_mask_get(ret) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_POINTER_BUTTON_DOWN_MASK_GET), EO_TYPECHECK(int *, ret)

/**
 * @def evas_canvas_pointer_inside_get
 * @since 1.8
 *
 * Returns whether the mouse pointer is logically inside the canvas
 *
 * @param[out] ret
 *
 * @see evas_pointer_inside_get
 */
#define evas_canvas_pointer_inside_get(ret) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_POINTER_INSIDE_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def evas_canvas_data_attach_set
 * @since 1.8
 *
 * Attaches a specific pointer to the evas for fetching later
 *
 * @param[in] data
 *
 * @see evas_data_attach_set
 */
#define evas_canvas_data_attach_set(data) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_DATA_ATTACH_SET), EO_TYPECHECK(void *, data)

/**
 * @def evas_canvas_data_attach_get
 * @since 1.8
 *
 * Returns the pointer attached by evas_data_attach_set()
 *
 * @param[out] ret
 *
 * @see evas_data_attach_get
 */
#define evas_canvas_data_attach_get(ret) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_DATA_ATTACH_GET), EO_TYPECHECK(void **, ret)

/**
 * @def evas_canvas_focus_in
 * @since 1.8
 *
 * Inform to the evas that it got the focus.
 *
 *
 * @see evas_focus_in
 */
#define evas_canvas_focus_in() EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_FOCUS_IN)

/**
 * @def evas_canvas_focus_out
 * @since 1.8
 *
 * Inform to the evas that it lost the focus.
 *
 *
 * @see evas_focus_out
 */
#define evas_canvas_focus_out() EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_FOCUS_OUT)

/**
 * @def evas_canvas_focus_state_get
 * @since 1.8
 *
 * Get the focus state known by the given evas
 *
 * @param[out] ret
 *
 * @see evas_focus_state_get
 */
#define evas_canvas_focus_state_get(ret) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_FOCUS_STATE_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def evas_canvas_nochange_push
 * @since 1.8
 *
 * Push the nochange flag up 1
 *
 *
 * @see evas_nochange_push
 */
#define evas_canvas_nochange_push() EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_NOCHANGE_PUSH)

/**
 * @def evas_canvas_nochange_pop
 * @since 1.8
 *
 * Pop the nochange flag down 1
 *
 *
 * @see evas_nochange_pop
 */
#define evas_canvas_nochange_pop() EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_NOCHANGE_POP)


/**
 * @def evas_canvas_event_default_flags_set
 * @since 1.8
 *
 * Set the default set of flags an event begins with
 *
 * @param[in] flags
 *
 * @see evas_event_default_flags_set
 */
#define evas_canvas_event_default_flags_set(flags) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_EVENT_DEFAULT_FLAGS_SET), EO_TYPECHECK(Evas_Event_Flags, flags)

/**
 * @def evas_canvas_event_default_flags_get
 * @since 1.8
 *
 * Get the defaulty set of flags an event begins with
 *
 * @param[out] ret
 *
 * @see evas_event_default_flags_get
 */
#define evas_canvas_event_default_flags_get(ret) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_EVENT_DEFAULT_FLAGS_GET), EO_TYPECHECK(Evas_Event_Flags *, ret)

/**
 * @def evas_canvas_event_freeze
 * @since 1.8
 *
 * Freeze all input events processing.
 *
 *
 * @see evas_event_freeze
 */
#define evas_canvas_event_freeze() EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_EVENT_FREEZE)

/**
 * @def evas_canvas_event_thaw
 * @since 1.8
 *
 * Thaw a canvas out after freezing (for input events).
 *
 *
 * @see evas_event_thaw
 */
#define evas_canvas_event_thaw() EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_EVENT_THAW)

/**
 * @def evas_canvas_event_freeze_get
 * @since 1.8
 *
 * Return the freeze count on input events of a given canvas.
 *
 * @param[out] ret
 *
 * @see evas_event_freeze_get
 */
#define evas_canvas_event_freeze_get(ret) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_EVENT_FREEZE_GET), EO_TYPECHECK(int *, ret)

/**
 * @def evas_canvas_event_thaw_eval
 * @since 1.8
 *
 * After thaw of a canvas, re-evaluate the state of objects and call callbacks
 *
 *
 * @see evas_event_thaw_eval
 */
#define evas_canvas_event_thaw_eval() EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_EVENT_THAW_EVAL)

/**
 * @def evas_canvas_event_feed_mouse_down
 * @since 1.8
 *
 * Mouse down event feed.
 *
 * @param[in] b
 * @param[in] flags
 * @param[in] timestamp
 * @param[in] data
 *
 * @see evas_event_feed_mouse_down
 */
#define evas_canvas_event_feed_mouse_down(b, flags, timestamp, data) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_EVENT_FEED_MOUSE_DOWN), EO_TYPECHECK(int, b), EO_TYPECHECK(Evas_Button_Flags, flags), EO_TYPECHECK(unsigned int, timestamp), EO_TYPECHECK(const void *, data)

/**
 * @def evas_canvas_event_feed_mouse_up
 * @since 1.8
 *
 * Mouse up event feed.
 *
 * @param[in] b
 * @param[in] flags
 * @param[in] timestamp
 * @param[in] data
 *
 * @see evas_event_feed_mouse_up
 */
#define evas_canvas_event_feed_mouse_up(b, flags, timestamp, data) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_EVENT_FEED_MOUSE_UP), EO_TYPECHECK(int, b), EO_TYPECHECK(Evas_Button_Flags, flags), EO_TYPECHECK(unsigned int, timestamp), EO_TYPECHECK(const void *, data)

/**
 * @def evas_canvas_event_feed_mouse_cancel
 * @since 1.8
 *
 * Mouse cancel event feed.
 *
 * @param[in] timestamp
 * @param[in] data
 *
 * @see evas_event_feed_mouse_cancel
 */
#define evas_canvas_event_feed_mouse_cancel(timestamp, data) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_EVENT_FEED_MOUSE_CANCEL), EO_TYPECHECK(unsigned int, timestamp), EO_TYPECHECK(const void *, data)

/**
 * @def evas_canvas_event_feed_mouse_wheel
 * @since 1.8
 *
 * Mouse wheel event feed.
 *
 * @param[in] direction
 * @param[in] z
 * @param[in] timestamp
 * @param[in] data
 *
 * @see evas_event_feed_mouse_wheel
 */
#define evas_canvas_event_feed_mouse_wheel(direction, z, timestamp, data) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_EVENT_FEED_MOUSE_WHEEL), EO_TYPECHECK(int, direction), EO_TYPECHECK(int, z), EO_TYPECHECK(unsigned int, timestamp), EO_TYPECHECK(const void *, data)

/**
 * @def evas_canvas_event_feed_mouse_move
 * @since 1.8
 *
 * Mouse move event feed.
 *
 * @param[in] x
 * @param[in] y
 * @param[in] timestamp
 * @param[in] data
 *
 * @see evas_event_feed_mouse_move
 */
#define evas_canvas_event_feed_mouse_move(x, y, timestamp, data) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_EVENT_FEED_MOUSE_MOVE), EO_TYPECHECK(int, x), EO_TYPECHECK(int, y), EO_TYPECHECK(unsigned int, timestamp), EO_TYPECHECK(const void *, data)

/**
 * @def evas_canvas_event_feed_mouse_in
 * @since 1.8
 *
 * Mouse in event feed.
 *
 * @param[in] timestamp
 * @param[in] data
 *
 * @see evas_event_feed_mouse_in
 */
#define evas_canvas_event_feed_mouse_in(timestamp, data) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_EVENT_FEED_MOUSE_IN), EO_TYPECHECK(unsigned int, timestamp), EO_TYPECHECK(const void *, data)

/**
 * @def evas_canvas_event_feed_mouse_out
 * @since 1.8
 *
 * Mouse out event feed.
 *
 * @param[in] timestamp
 * @param[in] data
 *
 * @see evas_event_feed_mouse_out
 */
#define evas_canvas_event_feed_mouse_out(timestamp, data) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_EVENT_FEED_MOUSE_OUT), EO_TYPECHECK(unsigned int, timestamp), EO_TYPECHECK(const void *, data)
#define evas_canvas_event_feed_multi_down(d, x, y, rad, radx, rady, pres, ang, fx, fy, flags, timestamp, data) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_EVENT_FEED_MULTI_DOWN), EO_TYPECHECK(int, d), EO_TYPECHECK(int, x), EO_TYPECHECK(int, y), EO_TYPECHECK(double, rad), EO_TYPECHECK(double, radx), EO_TYPECHECK(double, rady), EO_TYPECHECK(double, pres), EO_TYPECHECK(double, ang), EO_TYPECHECK(double, fx), EO_TYPECHECK(double, fy), EO_TYPECHECK(Evas_Button_Flags, flags), EO_TYPECHECK(unsigned int, timestamp), EO_TYPECHECK(const void *, data)
#define evas_canvas_event_feed_multi_up(d, x, y, rad, radx, rady, pres, ang, fx, fy, flags, timestamp, data) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_EVENT_FEED_MULTI_UP), EO_TYPECHECK(int, d), EO_TYPECHECK(int, x), EO_TYPECHECK(int, y), EO_TYPECHECK(double, rad), EO_TYPECHECK(double, radx), EO_TYPECHECK(double, rady), EO_TYPECHECK(double, pres), EO_TYPECHECK(double, ang), EO_TYPECHECK(double, fx), EO_TYPECHECK(double, fy), EO_TYPECHECK(Evas_Button_Flags, flags), EO_TYPECHECK(unsigned int, timestamp), EO_TYPECHECK(const void *, data)
#define evas_canvas_event_feed_multi_move(d, x, y, rad, radx, rady, pres, ang, fx, fy, timestamp, data) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_EVENT_FEED_MULTI_MOVE), EO_TYPECHECK(int, d), EO_TYPECHECK(int, x), EO_TYPECHECK(int, y), EO_TYPECHECK(double, rad), EO_TYPECHECK(double, radx), EO_TYPECHECK(double, rady), EO_TYPECHECK(double, pres), EO_TYPECHECK(double, ang), EO_TYPECHECK(double, fx), EO_TYPECHECK(double, fy), EO_TYPECHECK(unsigned int, timestamp), EO_TYPECHECK(const void *, data)

/**
 * @def evas_canvas_event_feed_key_down
 * @since 1.8
 *
 * Key down event feed
 *
 * @param[in] keyname
 * @param[in] key
 * @param[in] string
 * @param[in] compose
 * @param[in] timestamp
 * @param[in] data
 *
 * @see evas_event_feed_key_down
 */
#define evas_canvas_event_feed_key_down(keyname, key, string, compose, timestamp, data) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_EVENT_FEED_KEY_DOWN), EO_TYPECHECK(const char *, keyname), EO_TYPECHECK(const char *, key), EO_TYPECHECK(const char *, string), EO_TYPECHECK(const char *, compose), EO_TYPECHECK(unsigned int, timestamp), EO_TYPECHECK(const void *, data)

/**
 * @def evas_canvas_event_feed_key_up
 * @since 1.8
 *
 * Key up event feed
 *
 * @param[in] keyname
 * @param[in] key
 * @param[in] string
 * @param[in] compose
 * @param[in] timestamp
 * @param[in] data
 *
 * @see evas_event_feed_key_up
 */
#define evas_canvas_event_feed_key_up(keyname, key, string, compose, timestamp, data) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_EVENT_FEED_KEY_UP), EO_TYPECHECK(const char *, keyname), EO_TYPECHECK(const char *, key), EO_TYPECHECK(const char *, string), EO_TYPECHECK(const char *, compose), EO_TYPECHECK(unsigned int, timestamp), EO_TYPECHECK(const void *, data)

/**
 * @def evas_canvas_event_feed_hold
 * @since 1.8
 *
 * Hold event feed
 *
 * @param[in] hold
 * @param[in] timestamp
 * @param[in] data
 *
 * @see evas_event_feed_hold
 */
#define evas_canvas_event_feed_hold(hold, timestamp, data) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_EVENT_FEED_HOLD), EO_TYPECHECK(int, hold), EO_TYPECHECK(unsigned int, timestamp), EO_TYPECHECK(const void *, data)

/**
 * @def evas_canvas_event_refeed_event
 * @since 1.8
 *
 * Re feed event.
 *
 * @param[in] event_copy
 * @param[in] event_type
 *
 * @see evas_event_refeed_event
 */
#define evas_canvas_event_refeed_event(event_copy, event_type) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_EVENT_REFEED_EVENT), EO_TYPECHECK(void *, event_copy), EO_TYPECHECK(Evas_Callback_Type, event_type)

/**
 * @def evas_canvas_event_down_count_get
 * @since 1.8
 *
 * Get the number of mouse or multi presses currently active
 *
 * @param[out] ret
 *
 * @see evas_event_down_count_get
 */
#define evas_canvas_event_down_count_get(ret) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_EVENT_DOWN_COUNT_GET), EO_TYPECHECK(int *, ret)

/**
 * @def evas_canvas_focus_get
 * @since 1.8
 *
 * Retrieve the object that currently has focus.
 *
 * @param[out] ret
 *
 * @see evas_focus_get
 */
#define evas_canvas_focus_get(ret) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_FOCUS_GET), EO_TYPECHECK(Evas_Object **, ret)


/**
 * @def evas_canvas_font_path_clear
 * @since 1.8
 *
 * Removes all font paths loaded into memory for the given evas.
 *
 *
 * @see evas_font_path_clear
 */
#define evas_canvas_font_path_clear() EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_FONT_PATH_CLEAR)

/**
 * @def evas_canvas_font_path_append
 * @since 1.8
 *
 * Appends a font path to the list of font paths used by the given evas.
 *
 * @param[in] path
 *
 * @see evas_font_path_append
 */
#define evas_canvas_font_path_append(path) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_FONT_PATH_APPEND), EO_TYPECHECK(const char *, path)

/**
 * @def evas_canvas_font_path_prepend
 * @since 1.8
 *
 * Prepends a font path to the list of font paths used by the given evas.
 *
 * @param[in] path
 *
 * @see evas_font_path_prepend
 */
#define evas_canvas_font_path_prepend(path) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_FONT_PATH_PREPEND), EO_TYPECHECK(const char *, path)

/**
 * @def evas_canvas_font_path_list
 * @since 1.8
 *
 * Retrieves the list of font paths used by the given evas.
 *
 * @param[out] ret
 *
 * @see evas_font_path_list
 */
#define evas_canvas_font_path_list(ret) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_FONT_PATH_LIST), EO_TYPECHECK(const Eina_List **, ret)

/**
 * @def evas_canvas_font_hinting_set
 * @since 1.8
 *
 * Changes the font hinting for the given evas.
 *
 * @param[in] hinting
 *
 * @see evas_font_hinting_set
 */
#define evas_canvas_font_hinting_set(hinting) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_FONT_HINTING_SET), EO_TYPECHECK(Evas_Font_Hinting_Flags, hinting)

/**
 * @def evas_canvas_font_hinting_get
 * @since 1.8
 *
 * Retrieves the font hinting used by the given evas.
 *
 * @param[out] ret
 *
 * @see evas_font_hinting_get
 */
#define evas_canvas_font_hinting_get(ret) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_FONT_HINTING_GET), EO_TYPECHECK(Evas_Font_Hinting_Flags *, ret)

/**
 * @def evas_canvas_font_hinting_can_hint
 * @since 1.8
 *
 * Checks if the font hinting is supported by the given evas.
 *
 * @param[in] hinting
 * @param[out] ret
 *
 * @see evas_font_hinting_can_hint
 */
#define evas_canvas_font_hinting_can_hint(hinting, ret) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_FONT_HINTING_CAN_HINT), EO_TYPECHECK(Evas_Font_Hinting_Flags, hinting), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def evas_canvas_font_cache_flush
 * @since 1.8
 *
 * Force the given evas and associated engine to flush its font cache.
 *
 *
 * @see evas_font_cache_flush
 */
#define evas_canvas_font_cache_flush() EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_FONT_CACHE_FLUSH)

/**
 * @def evas_canvas_font_cache_set
 * @since 1.8
 *
 * Changes the size of font cache of the given evas.
 *
 * @param[in] size
 *
 * @see evas_font_cache_set
 */
#define evas_canvas_font_cache_set(size) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_FONT_CACHE_SET), EO_TYPECHECK(int, size)

/**
 * @def evas_canvas_font_cache_get
 * @since 1.8
 *
 * Changes the size of font cache of the given evas.
 *
 * @param[out] ret
 *
 * @see evas_font_cache_get
 */
#define evas_canvas_font_cache_get(ret) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_FONT_CACHE_GET), EO_TYPECHECK(int *, ret)

/**
 * @def evas_canvas_font_available_list
 * @since 1.8
 *
 * List of available font descriptions known or found by this evas.
 *
 * @param[out] ret
 *
 * @see evas_font_available_list
 */
#define evas_canvas_font_available_list(ret) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_FONT_AVAILABLE_LIST), EO_TYPECHECK(Eina_List **, ret)


/**
 * @def evas_canvas_key_modifier_get
 * @since 1.8
 *
 * Returns a handle to the list of modifier keys registered in the
 *
 * @param[out] ret
 *
 * @see evas_key_modifier_get
 */
#define evas_canvas_key_modifier_get(ret) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_KEY_MODIFIER_GET), EO_TYPECHECK(const Evas_Modifier **, ret)

/**
 * @def evas_canvas_key_lock_get
 * @since 1.8
 *
 * Returns a handle to the list of lock keys registered in the canvas
 *
 * @param[out] ret
 *
 * @see evas_key_lock_get
 */
#define evas_canvas_key_lock_get(ret) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_KEY_LOCK_GET), EO_TYPECHECK(const Evas_Lock **, ret)

/**
 * @def evas_canvas_key_modifier_add
 * @since 1.8
 *
 * Adds the keyname key to the current list of modifier keys.
 *
 * @param[in] keyname
 *
 * @see evas_key_modifier_add
 */
#define evas_canvas_key_modifier_add(keyname) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_KEY_MODIFIER_ADD), EO_TYPECHECK(const char *, keyname)

/**
 * @def evas_canvas_key_modifier_del
 * @since 1.8
 *
 * Removes the keyname key from the current list of modifier keys
 *
 * @param[in] keyname
 *
 * @see evas_key_modifier_del
 */
#define evas_canvas_key_modifier_del(keyname) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_KEY_MODIFIER_DEL), EO_TYPECHECK(const char *, keyname)

/**
 * @def evas_canvas_key_lock_add
 * @since 1.8
 *
 * Adds the keyname key to the current list of lock keys.
 *
 * @param[in] keyname
 *
 * @see evas_key_lock_add
 */
#define evas_canvas_key_lock_add(keyname) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_KEY_LOCK_ADD), EO_TYPECHECK(const char *, keyname)

/**
 * @def evas_canvas_key_lock_del
 * @since 1.8
 *
 * Removes the keyname key from the current list of lock keys on
 *
 * @param[in] keyname
 *
 * @see evas_key_lock_del
 */
#define evas_canvas_key_lock_del(keyname) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_KEY_LOCK_DEL), EO_TYPECHECK(const char *, keyname)

/**
 * @def evas_canvas_key_modifier_on
 * @since 1.8
 *
 * Enables or turns on programmatically the modifier key with name @p keyname.
 *
 * @param[in] keyname
 *
 * @see evas_key_modifier_on
 */
#define evas_canvas_key_modifier_on(keyname) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_KEY_MODIFIER_ON), EO_TYPECHECK(const char *, keyname)

/**
 * @def evas_canvas_key_modifier_off
 * @since 1.8
 *
 * Disables or turns off programmatically the modifier key with name @p keyname
 *
 * @param[in] keyname
 *
 * @see evas_key_modifier_off
 */
#define evas_canvas_key_modifier_off(keyname) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_KEY_MODIFIER_OFF), EO_TYPECHECK(const char *, keyname)

/**
 * @def evas_canvas_key_lock_on
 * @since 1.8
 *
 * Enables or turns on programmatically the lock key with name @p keyname
 *
 * @param[in] keyname
 *
 * @see evas_key_lock_on
 */
#define evas_canvas_key_lock_on(keyname) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_KEY_LOCK_ON), EO_TYPECHECK(const char *, keyname)

/**
 * @def evas_canvas_key_lock_off
 * @since 1.8
 *
 * Disables or turns off programmatically the lock key with name @p keyname
 *
 * @param[in] keyname
 *
 * @see evas_key_lock_off
 */
#define evas_canvas_key_lock_off(keyname) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_KEY_LOCK_OFF), EO_TYPECHECK(const char *, keyname)

/**
 * @def evas_canvas_key_modifier_mask_get
 * @since 1.8
 *
 * Creates a bit mask from the keyname @b modifier key.
 *
 * @param[in] keyname
 * @param[out] ret
 *
 * @see evas_key_modifier_mask_get
 */
#define evas_canvas_key_modifier_mask_get(keyname, ret) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_KEY_MODIFIER_MASK_GET), EO_TYPECHECK(const char *, keyname), EO_TYPECHECK(Evas_Modifier_Mask *, ret)

/**
 * @def evas_canvas_damage_rectangle_add
 * @since 1.8
 *
 * Add a damage rectangle.
 *
 * @param[in] x
 * @param[in] y
 * @param[in] w
 * @param[in] h
 *
 * @see evas_damage_rectangle_add
 */
#define evas_canvas_damage_rectangle_add(x, y, w, h) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_DAMAGE_RECTANGLE_ADD), EO_TYPECHECK(int, x), EO_TYPECHECK(int, y), EO_TYPECHECK(int, w), EO_TYPECHECK(int, h)

/**
 * @def evas_canvas_obscured_rectangle_add
 * @since 1.8
 *
 * Add an "obscured region" to an Evas canvas.
 *
 * @param[in] x
 * @param[in] y
 * @param[in] w
 * @param[in] h
 *
 * @see evas_obscured_rectangle_add
 */
#define evas_canvas_obscured_rectangle_add(x, y, w, h) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_OBSCURED_RECTANGLE_ADD), EO_TYPECHECK(int, x), EO_TYPECHECK(int, y), EO_TYPECHECK(int, w), EO_TYPECHECK(int, h)

/**
 * @def evas_canvas_obscured_clear
 * @since 1.8
 *
 * Remove all "obscured regions" from an Evas canvas.
 *
 *
 * @see evas_obscured_clear
 */
#define evas_canvas_obscured_clear() EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_OBSCURED_CLEAR)

/**
 * @def evas_canvas_render_updates
 * @since 1.8
 *
 * Force immediate renderization of the given Evas canvas.
 *
 * @param[out] ret
 *
 * @see evas_render_updates
 */
#define evas_canvas_render_updates(ret) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_RENDER_UPDATES), EO_TYPECHECK(Eina_List **, ret)

/**
 * @def evas_canvas_render
 * @since 1.8
 *
 * Force renderization of the given canvas.
 *
 *
 * @see evas_render
 */
#define evas_canvas_render() EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_RENDER)

/**
 * @def evas_canvas_norender
 * @since 1.8
 *
 * Update the canvas internal objects but not triggering immediate
 * renderization.
 *
 *
 * @see evas_norender
 */
#define evas_canvas_norender() EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_NORENDER)

/**
 * @def evas_canvas_render_idle_flush
 * @since 1.8
 *
 * Make the canvas discard internally cached data used for rendering.
 *
 *
 * @see evas_render_idle_flush
 */
#define evas_canvas_render_idle_flush() EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_RENDER_IDLE_FLUSH)
#define evas_canvas_sync() EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_SYNC)

/**
 * @def evas_canvas_render_dump
 * @since 1.8
 *
 * Make the canvas discard as much data as possible used by the engine at
 *
 *
 * @see evas_render_dump
 */
#define evas_canvas_render_dump() EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_RENDER_DUMP)


/**
 * @def evas_canvas_object_bottom_get
 * @since 1.8
 *
 * Get the lowest (stacked) Evas object on the canvas e.
 *
 * @param[out] ret
 *
 * @see evas_object_bottom_get
 */
#define evas_canvas_object_bottom_get(ret) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_OBJECT_BOTTOM_GET), EO_TYPECHECK(Evas_Object **, ret)

/**
 * @def evas_canvas_object_top_get
 * @since 1.8
 *
 * Get the highest (stacked) Evas object on the canvas e.
 *
 * @param[out] ret
 *
 * @see evas_object_top_get
 */
#define evas_canvas_object_top_get(ret) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_OBJECT_TOP_GET), EO_TYPECHECK(Evas_Object **, ret)


/**
 * @def evas_canvas_touch_point_list_count
 * @since 1.8
 *
 * Get the number of touched point in the evas.
 *
 * @param[out] ret
 *
 * @see evas_touch_point_list_count
 */
#define evas_canvas_touch_point_list_count(ret) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_TOUCH_POINT_LIST_COUNT), EO_TYPECHECK(unsigned int *, ret)

/**
 * @def evas_canvas_touch_point_list_nth_xy_get
 * @since 1.8
 *
 * This function returns the nth touch point's co-ordinates.
 *
 * @param[in] n
 * @param[out] x
 * @param[out] y
 *
 * @see evas_touch_point_list_nth_xy_get
 */
#define evas_canvas_touch_point_list_nth_xy_get(n, x, y) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_TOUCH_POINT_LIST_NTH_XY_GET), EO_TYPECHECK(unsigned int, n), EO_TYPECHECK(Evas_Coord *, x), EO_TYPECHECK(Evas_Coord *, y)

/**
 * @def evas_canvas_touch_point_list_nth_id_get
 * @since 1.8
 *
 * This function returns the id of nth touch point.
 *
 * @param[in] n
 * @param[out] ret
 *
 * @see evas_touch_point_list_nth_id_get
 */
#define evas_canvas_touch_point_list_nth_id_get(n, ret) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_TOUCH_POINT_LIST_NTH_ID_GET), EO_TYPECHECK(unsigned int, n), EO_TYPECHECK(int *, ret)

/**
 * @def evas_canvas_touch_point_list_nth_state_get
 * @since 1.8
 *
 * This function returns the state of nth touch point.
 *
 * @param[in] n
 * @param[out] ret
 *
 * @see evas_touch_point_list_nth_state_get
 */
#define evas_canvas_touch_point_list_nth_state_get(n, ret) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_TOUCH_POINT_LIST_NTH_STATE_GET), EO_TYPECHECK(unsigned int, n), EO_TYPECHECK(Evas_Touch_Point_State *, ret)

/**
 * @def evas_canvas_image_cache_flush
 * @since 1.8
 *
 * Flush the image cache of the canvas.
 *
 *
 * @see evas_image_cache_flush
 */
#define evas_canvas_image_cache_flush() EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_IMAGE_CACHE_FLUSH)

/**
 * @def evas_canvas_image_cache_reload
 * @since 1.8
 *
 * Reload the image cache
 *
 *
 * @see evas_image_cache_reload
 */
#define evas_canvas_image_cache_reload() EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_IMAGE_CACHE_RELOAD)

/**
 * @def evas_canvas_image_cache_set
 * @since 1.8
 *
 * Set the image cache.
 *
 * @param[in] size
 *
 * @see evas_image_cache_set
 */
#define evas_canvas_image_cache_set(size) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_IMAGE_CACHE_SET), EO_TYPECHECK(int, size)

/**
 * @def evas_canvas_image_cache_get
 * @since 1.8
 *
 * Get the image cache
 *
 * @param[out] ret
 *
 * @see evas_image_cache_get
 */
#define evas_canvas_image_cache_get(ret) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_IMAGE_CACHE_GET), EO_TYPECHECK(int *, ret)

/**
 * @def evas_canvas_image_max_size_get
 * @since 1.8
 *
 * Get the maximum image size evas can possibly handle
 *
 * @param[out] maxw
 * @param[out] maxh
 * @param[out] ret
 *
 * @see evas_image_max_size_get
 */
#define evas_canvas_image_max_size_get(maxw, maxh, ret) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_IMAGE_MAX_SIZE_GET), EO_TYPECHECK(int *, maxw), EO_TYPECHECK(int *, maxh), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def evas_canvas_object_name_find
 * @since 1.8
 *
 * Retrieves the object on the given evas with the given name.
 *
 * @param[in] name
 * @param[out] ret
 *
 * @see evas_object_name_find
 */
#define evas_canvas_object_name_find(name, ret) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_OBJECT_NAME_FIND),EO_TYPECHECK(const char *, name),  EO_TYPECHECK(Evas_Object **, ret)

/**
 * @def evas_canvas_object_top_at_xy_get
 * @since 1.8
 *
 * Retrieve the Evas object stacked at the top of a given position in
 * a canvas.
 *
 * @param[in] x
 * @param[in] y
 * @param[in] include_pass_events_objects
 * @param[in] include_hidden_objects
 * @param[out] ret
 *
 * @see evas_object_top_at_xy_get
 */
#define evas_canvas_object_top_at_xy_get(x, y, include_pass_events_objects, include_hidden_objects, ret) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_OBJECT_TOP_AT_XY_GET), EO_TYPECHECK(Evas_Coord, x), EO_TYPECHECK(Evas_Coord, y), EO_TYPECHECK(Eina_Bool, include_pass_events_objects), EO_TYPECHECK(Eina_Bool, include_hidden_objects), EO_TYPECHECK(Evas_Object **, ret)

/**
 * @def evas_canvas_object_top_in_rectangle_get
 * @since 1.8
 *
 * Retrieve the Evas object stacked at the top of a given rectangular
 * region in a canvas.
 *
 * @param[in] x
 * @param[in] y
 * @param[in] w
 * @param[in] h
 * @param[in] include_pass_events_objects
 * @param[in] include_hidden_objects
 * @param[out] ret
 *
 * @see evas_object_top_in_rectangle_get
 */
#define evas_canvas_object_top_in_rectangle_get(x, y, w, h, include_pass_events_objects, include_hidden_objects, ret) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_OBJECT_TOP_IN_RECTANGLE_GET), EO_TYPECHECK(Evas_Coord, x), EO_TYPECHECK(Evas_Coord, y), EO_TYPECHECK(Evas_Coord, w), EO_TYPECHECK(Evas_Coord, h), EO_TYPECHECK(Eina_Bool, include_pass_events_objects), EO_TYPECHECK(Eina_Bool, include_hidden_objects), EO_TYPECHECK(Evas_Object **, ret)

/**
 * @def evas_canvas_objects_at_xy_get
 * @since 1.8
 *
 * Retrieve a list of Evas objects lying over a given position in
 * a canvas.
 *
 * @param[in] x
 * @param[in] y
 * @param[in] include_pass_events_objects
 * @param[in] include_hidden_objects
 * @param[out] ret
 *
 * @see evas_objects_at_xy_get
 */
#define evas_canvas_objects_at_xy_get(x, y, include_pass_events_objects, include_hidden_objects, ret) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_OBJECTS_AT_XY_GET), EO_TYPECHECK(Evas_Coord, x), EO_TYPECHECK(Evas_Coord, y), EO_TYPECHECK(Eina_Bool, include_pass_events_objects), EO_TYPECHECK(Eina_Bool, include_hidden_objects), EO_TYPECHECK(Eina_List **, ret)

/**
 * @def evas_canvas_objects_in_rectangle_get
 * @since 1.8
 *
 * @param[in] x
 * @param[in] y
 * @param[in] w
 * @param[in] h
 * @param[in] include_pass_events_objects
 * @param[in] include_hidden_objects
 * @param[out] ret
 *
 * @see evas_objects_in_rectangle_get
 */
#define evas_canvas_objects_in_rectangle_get(x, y, w, h, include_pass_events_objects, include_hidden_objects, ret) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_OBJECTS_IN_RECTANGLE_GET), EO_TYPECHECK(Evas_Coord, x), EO_TYPECHECK(Evas_Coord, y), EO_TYPECHECK(Evas_Coord, w), EO_TYPECHECK(Evas_Coord, h), EO_TYPECHECK(Eina_Bool, include_pass_events_objects), EO_TYPECHECK(Eina_Bool, include_hidden_objects), EO_TYPECHECK(Eina_List **, ret)

/**
 * @def evas_canvas_smart_objects_calculate
 * @since 1.8
 *
 * Call user-provided calculate() smart functions and unset the
 * flag signalling that the object needs to get recalculated to @b all
 * smart objects in the canvas.
 *
 *
 * @see evas_smart_objects_calculate
 */
#define evas_canvas_smart_objects_calculate() EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_SMART_OBJECTS_CALCULATE)

/**
 * @def evas_canvas_smart_objects_calculate_count_get
 * @since 1.8
 *
 * This gets the internal counter that counts the number of smart calculations
 *
 * @param[out] ret
 *
 * @see evas_smart_objects_calculate_count_get
 */
#define evas_canvas_smart_objects_calculate_count_get(ret) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_SMART_OBJECTS_CALCULATE_COUNT_GET), EO_TYPECHECK(int *, ret)




   
/**
 * @defgroup Evas_Canvas_Events Canvas Events
 *
 * Functions relating to canvas events, which are mainly reports on
 * its internal states changing (an object got focused, the rendering
 * is updated, etc).
 *
 * Some of the functions in this group are exemplified @ref
 * Example_Evas_Events "here".
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
EAPI void  evas_event_callback_add(Evas *e, Evas_Callback_Type type, Evas_Event_Cb func, const void *data) EINA_ARG_NONNULL(1, 3);

/**
 * Add (register) a callback function to a given canvas event with a
 * non-default priority set. Except for the priority field, it's exactly the
 * same as @ref evas_event_callback_add
 *
 * @param e Canvas to attach a callback to
 * @param type The type of event that will trigger the callback
 * @param priority The priority of the callback, lower values called first.
 * @param func The (callback) function to be called when the event is
 *        triggered
 * @param data The data pointer to be passed to @p func
 *
 * @see evas_event_callback_add
 * @since 1.1
 */
EAPI void  evas_event_callback_priority_add(Evas *e, Evas_Callback_Type type, Evas_Callback_Priority priority, Evas_Event_Cb func, const void *data) EINA_ARG_NONNULL(1, 4);

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
 * canvas. If not successful @c NULL will be returned.
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
EAPI void *evas_event_callback_del(Evas *e, Evas_Callback_Type type, Evas_Event_Cb func) EINA_ARG_NONNULL(1, 3);

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
EAPI void *evas_event_callback_del_full(Evas *e, Evas_Callback_Type type, Evas_Event_Cb func, const void *data) EINA_ARG_NONNULL(1, 3);

/**
 * Push a callback on the post-event callback stack
 *
 * @param e Canvas to push the callback on
 * @param func The function that to be called when the stack is unwound
 * @param data The data pointer to be passed to the callback
 *
 * Evas has a stack of callbacks that get called after all the callbacks for
 * an event have triggered (all the objects it triggers on and all the callbacks
 * in each object triggered). When all these have been called, the stack is
 * unwond from most recently to least recently pushed item and removed from the
 * stack calling the callback set for it.
 *
 * This is intended for doing reverse logic-like processing, example - when a
 * child object that happens to get the event later is meant to be able to
 * "steal" functions from a parent and thus on unwind of this stack have its
 * function called first, thus being able to set flags, or return 0 from the
 * post-callback that stops all other post-callbacks in the current stack from
 * being called (thus basically allowing a child to take control, if the event
 * callback prepares information ready for taking action, but the post callback
 * actually does the action).
 *
 */
EAPI void  evas_post_event_callback_push(Evas *e, Evas_Object_Event_Post_Cb func, const void *data);

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
EAPI void  evas_post_event_callback_remove(Evas *e, Evas_Object_Event_Post_Cb func);

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
EAPI void  evas_post_event_callback_remove_full(Evas *e, Evas_Object_Event_Post_Cb func, const void *data);

/**
 * @defgroup Evas_Event_Freezing_Group Input Events Freezing Functions
 *
 * Functions that deal with the freezing of input event processing of
 * an Evas canvas.
 *
 * There might be scenarios during a graphical user interface
 * program's use when the developer wishes the users wouldn't be able
 * to deliver input events to this application. It may, for example,
 * be the time for it to populate a view or to change some
 * layout. Assuming proper behavior with user interaction during this
 * exact time would be hard, as things are in a changing state. The
 * programmer can then tell the canvas to ignore input events,
 * bringing it back to normal behavior when he/she wants.
 *
 * Most of the time use of freezing events is done like this:
 * @code
 * evas_event_freeze(my_evas_canvas);
 * function_that_does_work_which_cant_be_interrupted_by_events();
 * evas_event_thaw(my_evas_canvas);
 * @endcode
 *
 * Some of the functions in this group are exemplified @ref
 * Example_Evas_Events "here".
 *
 * @ingroup Evas_Canvas_Events
 */

/**
 * @addtogroup Evas_Event_Freezing_Group
 * @{
 */

/**
 * Set the default set of flags an event begins with
 *
 * @param e The canvas to set the default event flags of
 * @param flags The default flags to use
 *
 * Events in evas can have an event_flags member. This starts out with
 * and initial value (no flags). this lets you set the default flags that
 * an event begins with to be @p flags
 *
 * @since 1.2
 */
EAPI void             evas_event_default_flags_set(Evas *e, Evas_Event_Flags flags) EINA_ARG_NONNULL(1);

/**
 * Get the defaulty set of flags an event begins with
 *
 * @param e The canvas to get the default event flags from
 * @return The default event flags for that canvas
 *
 * This gets the default event flags events are produced with when fed in.
 *
 * @see evas_event_default_flags_set()
 * @since 1.2
 */
EAPI Evas_Event_Flags evas_event_default_flags_get(const Evas *e) EINA_ARG_NONNULL(1);

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
 * when you don't want the user to interact with your user interface
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
EAPI void             evas_event_freeze(Evas *e) EINA_ARG_NONNULL(1);

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
EAPI void             evas_event_thaw(Evas *e) EINA_ARG_NONNULL(1);

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
EAPI int              evas_event_freeze_get(const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * After thaw of a canvas, re-evaluate the state of objects and call callbacks
 *
 * @param e The canvas to evaluate after a thaw
 *
 * This is normally called after evas_event_thaw() to re-evaluate mouse
 * containment and other states and thus also call callbacks for mouse in and
 * out on new objects if the state change demands it.
 */
EAPI void             evas_event_thaw_eval(Evas *e) EINA_ARG_NONNULL(1);

/**
 * @}
 */

/**
 * @defgroup Evas_Event_Feeding_Group Input Events Feeding Functions
 *
 * Functions to tell Evas that input events happened and should be
 * processed.
 *
 * @warning Most of the time these functions are @b not what you're looking for.
 * These functions should only be used if you're not working with ecore evas(or
 * another input handling system). If you're not using ecore evas please
 * consider using it, in most situation it will make life a lot easier.
 *
 * As explained in @ref intro_not_evas, Evas does not know how to poll
 * for input events, so the developer should do it and then feed such
 * events to the canvas to be processed. This is only required if
 * operating Evas directly. Modules such as Ecore_Evas do that for
 * you.
 *
 * Some of the functions in this group are exemplified @ref
 * Example_Evas_Events "here".
 *
 * @ingroup Evas_Canvas_Events
 */

/**
 * @addtogroup Evas_Event_Feeding_Group
 * @{
 */

/**
 * @since 1.8
 */
EAPI Evas_Device *evas_device_new(Evas *e);

/**
 * @since 1.8
 */
EAPI void evas_device_free(Evas_Device *dev);
   
/**
 * @since 1.8
 */
EAPI void evas_device_push(Evas *e, Evas_Device *dev);
   
/**
 * @since 1.8
 */
EAPI void evas_device_pop(Evas *e);
   
/**
 * @since 1.8
 */
EAPI const Eina_List *evas_device_list(Evas *e, const Evas_Device *dev);
   
/**
 * @since 1.8
 */
EAPI void evas_device_name_set(Evas_Device *dev, const char *name);
   
/**
 * @since 1.8
 */
EAPI const char *evas_device_name_get(const Evas_Device *dev);
   
/**
 * @since 1.8
 */
EAPI void evas_device_description_set(Evas_Device *dev, const char *desc);
   
/**
 * @since 1.8
 */
EAPI const char *evas_device_description_get(const Evas_Device *dev);
   
/**
 * @since 1.8
 */
EAPI void evas_device_parent_set(Evas_Device *dev, Evas_Device *parent);
   
/**
 * @since 1.8
 */
EAPI const Evas_Device *evas_device_parent_get(const Evas_Device *dev);
   
/**
 * @since 1.8
 */
EAPI void evas_device_class_set(Evas_Device *dev, Evas_Device_Class clas);
   
/**
 * @since 1.8
 */
EAPI Evas_Device_Class evas_device_class_get(const Evas_Device *dev);
   
/**
 * @since 1.8
 */
EAPI void evas_device_emulation_source_set(Evas_Device *dev, Evas_Device *src);
   
/**
 * @since 1.8
 */
EAPI const Evas_Device *evas_device_emulation_source_get(const Evas_Device *dev);

/**
 * Get the number of mouse or multi presses currently active
 *
 * @p e The given canvas pointer.
 * @return The numer of presses (0 if none active).
 *
 * @since 1.2
 */
EAPI int  evas_event_down_count_get(const Evas *e) EINA_ARG_NONNULL(1);

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
EAPI void evas_event_feed_mouse_down(Evas *e, int b, Evas_Button_Flags flags, unsigned int timestamp, const void *data) EINA_ARG_NONNULL(1);

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
EAPI void evas_event_feed_mouse_up(Evas *e, int b, Evas_Button_Flags flags, unsigned int timestamp, const void *data) EINA_ARG_NONNULL(1);

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
EAPI void evas_event_feed_mouse_move(Evas *e, int x, int y, unsigned int timestamp, const void *data) EINA_ARG_NONNULL(1);

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
EAPI void evas_event_feed_mouse_in(Evas *e, unsigned int timestamp, const void *data) EINA_ARG_NONNULL(1);

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
EAPI void evas_event_feed_mouse_out(Evas *e, unsigned int timestamp, const void *data) EINA_ARG_NONNULL(1);
EAPI void evas_event_feed_multi_down(Evas *e, int d, int x, int y, double rad, double radx, double rady, double pres, double ang, double fx, double fy, Evas_Button_Flags flags, unsigned int timestamp, const void *data);
EAPI void evas_event_feed_multi_up(Evas *e, int d, int x, int y, double rad, double radx, double rady, double pres, double ang, double fx, double fy, Evas_Button_Flags flags, unsigned int timestamp, const void *data);
EAPI void evas_event_feed_multi_move(Evas *e, int d, int x, int y, double rad, double radx, double rady, double pres, double ang, double fx, double fy, unsigned int timestamp, const void *data);

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
EAPI void evas_event_feed_mouse_cancel(Evas *e, unsigned int timestamp, const void *data) EINA_ARG_NONNULL(1);

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
EAPI void evas_event_feed_mouse_wheel(Evas *e, int direction, int z, unsigned int timestamp, const void *data) EINA_ARG_NONNULL(1);

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
EAPI void evas_event_feed_key_down(Evas *e, const char *keyname, const char *key, const char *string, const char *compose, unsigned int timestamp, const void *data) EINA_ARG_NONNULL(1);

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
EAPI void evas_event_feed_key_up(Evas *e, const char *keyname, const char *key, const char *string, const char *compose, unsigned int timestamp, const void *data) EINA_ARG_NONNULL(1);

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
EAPI void evas_event_feed_hold(Evas *e, int hold, unsigned int timestamp, const void *data) EINA_ARG_NONNULL(1);

/**
 * Re feed event.
 *
 * @param e The given canvas pointer.
 * @param event_copy the event to refeed
 * @param event_type Event type
 *
 * This function re-feeds the event pointed by event_copy
 *
 * This function call evas_event_feed_* functions, so it can
 * cause havoc if not used wisely. Please use it responsibly.
 */
EAPI void evas_event_refeed_event(Evas *e, void *event_copy, Evas_Callback_Type event_type) EINA_ARG_NONNULL(1);

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
 * @addtogroup Evas_Image_Group
 * @{
 */

/**
 * Flush the image cache of the canvas.
 *
 * @param e The given evas pointer.
 *
 * This function flushes image cache of canvas.
 *
 */
EAPI void      evas_image_cache_flush(Evas *e) EINA_ARG_NONNULL(1);

/**
 * Reload the image cache
 *
 * @param e The given evas pointer.
 *
 * This function reloads the image cache of canvas.
 *
 */
EAPI void      evas_image_cache_reload(Evas *e) EINA_ARG_NONNULL(1);

/**
 * Set the image cache.
 *
 * @param e The given evas pointer.
 * @param size The cache size.
 *
 * This function sets the image cache of canvas in bytes.
 *
 */
EAPI void      evas_image_cache_set(Evas *e, int size) EINA_ARG_NONNULL(1);

/**
 * Get the image cache
 *
 * @param e The given evas pointer.
 *
 * This function returns the image cache size of canvas in bytes.
 *
 */
EAPI int       evas_image_cache_get(const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Get the maximum image size evas can possibly handle
 *
 * @param e The given evas pointer.
 * @param maxw Pointer to hold the return value in pixels of the maxumum width
 * @param maxh Pointer to hold the return value in pixels of the maximum height
 *
 * This function returns the larges image or surface size that evas can handle
 * in pixels, and if there is one, returns @c EINA_TRUE. It returns
 * @c EINA_FALSE if no extra constraint on maximum image size exists. You still
 * should check the return values of @p maxw and @p maxh as there may still be
 * a limit, just a much higher one.
 *
 * @since 1.1
 */
EAPI Eina_Bool evas_image_max_size_get(const Evas *e, int *maxw, int *maxh) EINA_ARG_NONNULL(1);

/**
 * @}
 */

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
EAPI void                    evas_font_hinting_set(Evas *e, Evas_Font_Hinting_Flags hinting) EINA_ARG_NONNULL(1);

/**
 * Retrieves the font hinting used by the given evas.
 *
 * @param e The given evas to query.
 * @return The hinting in use, one of #EVAS_FONT_HINTING_NONE,
 *         #EVAS_FONT_HINTING_AUTO, #EVAS_FONT_HINTING_BYTECODE.
 * @ingroup Evas_Font_Group
 */
EAPI Evas_Font_Hinting_Flags evas_font_hinting_get(const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Checks if the font hinting is supported by the given evas.
 *
 * @param e The given evas to query.
 * @param hinting The hinting to use, one of #EVAS_FONT_HINTING_NONE,
 *        #EVAS_FONT_HINTING_AUTO, #EVAS_FONT_HINTING_BYTECODE.
 * @return @c EINA_TRUE if it is supported, @c EINA_FALSE otherwise.
 * @ingroup Evas_Font_Group
 */
EAPI Eina_Bool               evas_font_hinting_can_hint(const Evas *e, Evas_Font_Hinting_Flags hinting) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Force the given evas and associated engine to flush its font cache.
 *
 * @param e The given evas to flush font cache.
 * @ingroup Evas_Font_Group
 */
EAPI void                    evas_font_cache_flush(Evas *e) EINA_ARG_NONNULL(1);

/**
 * Changes the size of font cache of the given evas.
 *
 * @param e The given evas to flush font cache.
 * @param size The size, in bytes.
 *
 * @ingroup Evas_Font_Group
 */
EAPI void                    evas_font_cache_set(Evas *e, int size) EINA_ARG_NONNULL(1);

/**
 * Changes the size of font cache of the given evas.
 *
 * @param e The given evas to flush font cache.
 * @return The size, in bytes.
 *
 * @ingroup Evas_Font_Group
 */
EAPI int                     evas_font_cache_get(const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

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
EAPI Eina_List              *evas_font_available_list(const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Free list of font descriptions returned by evas_font_dir_available_list().
 *
 * @param e The evas instance that returned such list.
 * @param available the list returned by evas_font_dir_available_list().
 *
 * @ingroup Evas_Font_Group
 */
EAPI void                    evas_font_available_list_free(Evas *e, Eina_List *available) EINA_ARG_NONNULL(1);

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
EAPI void                    evas_font_path_clear(Evas *e) EINA_ARG_NONNULL(1);

/**
 * Appends a font path to the list of font paths used by the given evas.
 * @param   e    The given evas.
 * @param   path The new font path.
 * @ingroup Evas_Font_Path_Group
 */
EAPI void                    evas_font_path_append(Evas *e, const char *path) EINA_ARG_NONNULL(1, 2);

/**
 * Prepends a font path to the list of font paths used by the given evas.
 * @param   e The given evas.
 * @param   path The new font path.
 * @ingroup Evas_Font_Path_Group
 */
EAPI void                    evas_font_path_prepend(Evas *e, const char *path) EINA_ARG_NONNULL(1, 2);

/**
 * Retrieves the list of font paths used by the given evas.
 * @param   e The given evas.
 * @return  The list of font paths used.
 * @ingroup Evas_Font_Path_Group
 */
EAPI const Eina_List        *evas_font_path_list(const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

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
 * These functions apply to @b any Evas object, whichever type that
 * may have.
 *
 * @note The built-in types which are most used are rectangles, text
 * and images. In fact, with these ones one can create 2D interfaces
 * of arbitrary complexity and EFL makes it easy.
 */

/**
 * @defgroup Evas_Object_Group_Basic Basic Object Manipulation
 *
 * Almost every evas object created will have some generic function used to
 * manipulate it. That's because there are a number of basic actions to be done
 * to objects that are irrespective of the object's type, things like:
 * @li Showing/Hiding
 * @li Setting(and getting) geometry
 * @li Bring up or down a layer
 * @li Color management
 * @li Handling focus
 * @li Clipping
 * @li Reference counting
 *
 * All of this issues are handled through the functions here grouped. Examples
 * of these function can be seen in @ref Example_Evas_Object_Manipulation(which
 * deals with the most common ones) and in @ref Example_Evas_Stacking(which
 * deals with stacking functions).
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
 * The following figure illustrates some clipping in Evas:
 *
 * @image html clipping.png
 * @image rtf clipping.png
 * @image latex clipping.eps
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
EAPI void             evas_object_clip_set(Evas_Object *obj, Evas_Object *clip) EINA_ARG_NONNULL(1, 2);

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
EAPI Evas_Object     *evas_object_clip_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

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
EAPI void             evas_object_clip_unset(Evas_Object *obj);

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
EAPI const Eina_List *evas_object_clipees_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Sets or unsets a given object as the currently focused one on its
 * canvas.
 *
 * @param obj The object to be focused or unfocused.
 * @param focus @c EINA_TRUE, to set it as focused or @c EINA_FALSE,
 * to take away the focus from it.
 *
 * Changing focus only affects where (key) input events go. There can
 * be only one object focused at any time. If @p focus is @c EINA_TRUE,
 * @p obj will be set as the currently focused object and it will
 * receive all keyboard events that are not exclusive key grabs on
 * other objects.
 *
 * Example:
 * @dontinclude evas-events.c
 * @skip evas_object_focus_set
 * @until evas_object_focus_set
 *
 * See the full example @ref Example_Evas_Events "here".
 *
 * @see evas_object_focus_get
 * @see evas_focus_get
 * @see evas_object_key_grab
 * @see evas_object_key_ungrab
 */
EAPI void             evas_object_focus_set(Evas_Object *obj, Eina_Bool focus) EINA_ARG_NONNULL(1);

/**
 * Retrieve whether an object has the focus.
 *
 * @param obj The object to retrieve focus information from.
 * @return @c EINA_TRUE if the object has the focus, @c EINA_FALSE otherwise.
 *
 * If the passed object is the currently focused one, @c EINA_TRUE is
 * returned. @c EINA_FALSE is returned, otherwise.
 *
 * Example:
 * @dontinclude evas-events.c
 * @skip And again
 * @until something is bad
 *
 * See the full example @ref Example_Evas_Events "here".
 *
 * @see evas_object_focus_set
 * @see evas_focus_get
 * @see evas_object_key_grab
 * @see evas_object_key_ungrab
 */
EAPI Eina_Bool        evas_object_focus_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Sets the layer of its canvas that the given object will be part of.
 *
 * @param   obj The given Evas object.
 * @param   l   The number of the layer to place the object on.
 *          Must be between #EVAS_LAYER_MIN and #EVAS_LAYER_MAX.
 *
 * If you don't use this function, you'll be dealing with an @b unique
 * layer of objects, the default one. Additional layers are handy when
 * you don't want a set of objects to interfere with another set with
 * regard to @b stacking. Two layers are completely disjoint in that
 * matter.
 *
 * This is a low-level function, which you'd be using when something
 * should be always on top, for example.
 *
 * @warning Be careful, it doesn't make sense to change the layer of
 * smart objects' children. Smart objects have a layer of their own,
 * which should contain all their children objects.
 *
 * @see evas_object_layer_get()
 */
EAPI void             evas_object_layer_set(Evas_Object *obj, short l) EINA_ARG_NONNULL(1);

/**
 * Retrieves the layer of its canvas that the given object is part of.
 *
 * @param   obj The given Evas object to query layer from
 * @return  Number of its layer
 *
 * @see evas_object_layer_set()
 */
EAPI short            evas_object_layer_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Sets the name of the given Evas object to the given name.
 *
 * @param   obj  The given object.
 * @param   name The given name.
 *
 * There might be occasions where one would like to name his/her
 * objects.
 *
 * Example:
 * @dontinclude evas-events.c
 * @skip d.bg = evas_object_rectangle_add(d.canvas);
 * @until evas_object_name_set(d.bg, "our dear rectangle");
 *
 * See the full @ref Example_Evas_Events "example".
 */
EAPI void             evas_object_name_set(Evas_Object *obj, const char *name) EINA_ARG_NONNULL(1);

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
EAPI const char      *evas_object_name_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

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
 * @note This is a <b>very simple</b> reference counting mechanism! For
 * instance, Evas is not ready to check for pending references on a
 * canvas deletion, or things like that. This is useful on scenarios
 * where, inside a code block, callbacks exist which would possibly
 * delete an object we are operating on afterwards. Then, one would
 * evas_object_ref() it on the beginning of the block and
 * evas_object_unref() it on the end. It would then be deleted at this
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
 * @since 1.1
 */
EAPI void             evas_object_ref(Evas_Object *obj);

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
 * @since 1.1
 */
EAPI void             evas_object_unref(Evas_Object *obj);

/**
 * Get the object reference count.
 *
 * @param obj The given Evas object to query
 *
 * This gets the reference count for an object (normally 0 until it is
 * referenced). Values of 1 or greater mean that someone is holding a
 * reference to this object that needs to be unreffed before it can be
 * deleted.
 *
 * @see evas_object_ref()
 * @see evas_object_unref()
 * @see evas_object_del()
 *
 * @ingroup Evas_Object_Group_Basic
 * @since 1.2
 */
EAPI int              evas_object_ref_get(const Evas_Object *obj);

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
EAPI void             evas_object_del(Evas_Object *obj) EINA_ARG_NONNULL(1);

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
EAPI void             evas_object_move(Evas_Object *obj, Evas_Coord x, Evas_Coord y) EINA_ARG_NONNULL(1);

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
EAPI void             evas_object_resize(Evas_Object *obj, Evas_Coord w, Evas_Coord h) EINA_ARG_NONNULL(1);

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
EAPI void             evas_object_geometry_get(const Evas_Object *obj, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h) EINA_ARG_NONNULL(1);

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
EAPI void             evas_object_show(Evas_Object *obj) EINA_ARG_NONNULL(1);

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
EAPI void             evas_object_hide(Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * Retrieves whether or not the given Evas object is visible.
 *
 * @param   obj The given Evas object.
 * @return @c EINA_TRUE if the object is visible, @c EINA_FALSE
 * otherwise.
 *
 * This retrieves an object's visibility as the one enforced by
 * evas_object_show() and evas_object_hide().
 *
 * @note The value returned isn't, by any means, influenced by
 * clippers covering @p obj, it being out of its canvas' viewport or
 * stacked below other object.
 *
 * @see evas_object_show()
 * @see evas_object_hide() (for an example)
 *
 * @ingroup Evas_Object_Group_Basic
 */
EAPI Eina_Bool        evas_object_visible_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

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
 * @note These color values are expected to be premultiplied by @p a.
 *
 * @ingroup Evas_Object_Group_Basic
 */
EAPI void             evas_object_color_set(Evas_Object *obj, int r, int g, int b, int a) EINA_ARG_NONNULL(1);

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
 * which defines the object's transparency level, 0 means totally
 * transparent, while 255 means opaque. These color values are
 * premultiplied by the alpha value.
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
EAPI void             evas_object_color_get(const Evas_Object *obj, int *r, int *g, int *b, int *a) EINA_ARG_NONNULL(1);

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
EAPI Evas            *evas_object_evas_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

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
EAPI const char      *evas_object_type_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Raise @p obj to the top of its layer.
 *
 * @param obj the object to raise
 *
 * @p obj will, then, be the highest one in the layer it belongs
 * to. Object on other layers won't get touched.
 *
 * @see evas_object_stack_above()
 * @see evas_object_stack_below()
 * @see evas_object_lower()
 */
EAPI void             evas_object_raise(Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * Lower @p obj to the bottom of its layer.
 *
 * @param obj the object to lower
 *
 * @p obj will, then, be the lowest one in the layer it belongs
 * to. Objects on other layers won't get touched.
 *
 * @see evas_object_stack_above()
 * @see evas_object_stack_below()
 * @see evas_object_raise()
 */
EAPI void             evas_object_lower(Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * Stack @p obj immediately above @p above
 *
 * @param obj the object to stack
 * @param above the object above which to stack
 *
 * Objects, in a given canvas, are stacked in the order they get added
 * to it.  This means that, if they overlap, the highest ones will
 * cover the lowest ones, in that order. This function is a way to
 * change the stacking order for the objects.
 *
 * This function is intended to be used with <b>objects belonging to
 * the same layer</b> in a given canvas, otherwise it will fail (and
 * accomplish nothing).
 *
 * If you have smart objects on your canvas and @p obj is a member of
 * one of them, then @p above must also be a member of the same
 * smart object.
 *
 * Similarly, if @p obj is not a member of a smart object, @p above
 * must not be either.
 *
 * @see evas_object_layer_get()
 * @see evas_object_layer_set()
 * @see evas_object_stack_below()
 */
EAPI void             evas_object_stack_above(Evas_Object *obj, Evas_Object *above) EINA_ARG_NONNULL(1, 2);

/**
 * Stack @p obj immediately below @p below
 *
 * @param obj the object to stack
 * @param below the object below which to stack
 *
 * Objects, in a given canvas, are stacked in the order they get added
 * to it.  This means that, if they overlap, the highest ones will
 * cover the lowest ones, in that order. This function is a way to
 * change the stacking order for the objects.
 *
 * This function is intended to be used with <b>objects belonging to
 * the same layer</b> in a given canvas, otherwise it will fail (and
 * accomplish nothing).
 *
 * If you have smart objects on your canvas and @p obj is a member of
 * one of them, then @p below must also be a member of the same
 * smart object.
 *
 * Similarly, if @p obj is not a member of a smart object, @p below
 * must not be either.
 *
 * @see evas_object_layer_get()
 * @see evas_object_layer_set()
 * @see evas_object_stack_below()
 */
EAPI void             evas_object_stack_below(Evas_Object *obj, Evas_Object *below) EINA_ARG_NONNULL(1, 2);

/**
 * Get the Evas object stacked right above @p obj
 *
 * @param obj an #Evas_Object
 * @return the #Evas_Object directly above @p obj, if any, or @c NULL,
 * if none
 *
 * This function will traverse layers in its search, if there are
 * objects on layers above the one @p obj is placed at.
 *
 * @see evas_object_layer_get()
 * @see evas_object_layer_set()
 * @see evas_object_below_get()
 *
 */
EAPI Evas_Object     *evas_object_above_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Get the Evas object stacked right below @p obj
 *
 * @param obj an #Evas_Object
 * @return the #Evas_Object directly below @p obj, if any, or @c NULL,
 * if none
 *
 * This function will traverse layers in its search, if there are
 * objects on layers below the one @p obj is placed at.
 *
 * @see evas_object_layer_get()
 * @see evas_object_layer_set()
 * @see evas_object_below_get()
 */
EAPI Evas_Object     *evas_object_below_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @}
 */

/**
 * @defgroup Evas_Object_Group_Events Object Events
 *
 * Objects generate events when they are moved, resized, when their
 * visibility change, when they are deleted and so on. These methods
 * allow one to be notified about and to handle such events.
 *
 * Objects also generate events on input (keyboard and mouse), if they
 * accept them (are visible, focused, etc).
 *
 * For each of those events, Evas provides a way for one to register
 * callback functions to be issued just after they happen.
 *
 * The following figure illustrates some Evas (event) callbacks:
 *
 * @image html evas-callbacks.png
 * @image rtf evas-callbacks.png
 * @image latex evas-callbacks.eps
 *
 * These events have their values in the #Evas_Callback_Type
 * enumeration, which has also ones happening on the canvas level (see
 * @ref Evas_Canvas_Events ).
 *
 * Examples on this group of functions can be found @ref
 * Example_Evas_Stacking "here" and @ref Example_Evas_Events "here".
 *
 * @ingroup Evas_Object_Group
 */

/**
 * @addtogroup Evas_Object_Group_Events
 * @{
 */

/**
 * Add (register) a callback function to a given Evas object event.
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
 * A callback function must have the ::Evas_Object_Event_Cb prototype
 * definition. The first parameter (@p data) in this definition will
 * have the same value passed to evas_object_event_callback_add() as
 * the @p data parameter, at runtime. The second parameter @p e is the
 * canvas pointer on which the event occurred. The third parameter is
 * a pointer to the object on which event occurred. Finally, the
 * fourth parameter @p event_info is a pointer to a data structure
 * that may or may not be passed to the callback, depending on the
 * event type that triggered the callback. This is so because some
 * events don't carry extra context with them, but others do.
 *
 * The event type @p type to trigger the function may be one of
 * #EVAS_CALLBACK_MOUSE_IN, #EVAS_CALLBACK_MOUSE_OUT,
 * #EVAS_CALLBACK_MOUSE_DOWN, #EVAS_CALLBACK_MOUSE_UP,
 * #EVAS_CALLBACK_MOUSE_MOVE, #EVAS_CALLBACK_MOUSE_WHEEL,
 * #EVAS_CALLBACK_MULTI_DOWN, #EVAS_CALLBACK_MULTI_UP,
 * #EVAS_CALLBACK_MULTI_MOVE, #EVAS_CALLBACK_FREE,
 * #EVAS_CALLBACK_KEY_DOWN, #EVAS_CALLBACK_KEY_UP,
 * #EVAS_CALLBACK_FOCUS_IN, #EVAS_CALLBACK_FOCUS_OUT,
 * #EVAS_CALLBACK_SHOW, #EVAS_CALLBACK_HIDE, #EVAS_CALLBACK_MOVE,
 * #EVAS_CALLBACK_RESIZE, #EVAS_CALLBACK_RESTACK, #EVAS_CALLBACK_DEL,
 * #EVAS_CALLBACK_HOLD, #EVAS_CALLBACK_CHANGED_SIZE_HINTS,
 * #EVAS_CALLBACK_IMAGE_PRELOADED or #EVAS_CALLBACK_IMAGE_UNLOADED.
 *
 * This determines the kind of event that will trigger the callback.
 * What follows is a list explaining better the nature of each type of
 * event, along with their associated @p event_info pointers:
 *
 * - #EVAS_CALLBACK_MOUSE_IN: @p event_info is a pointer to an
 *   #Evas_Event_Mouse_In struct\n\n
 *   This event is triggered when the mouse pointer enters the area
 *   (not shaded by other objects) of the object @p obj. This may
 *   occur by the mouse pointer being moved by
 *   evas_event_feed_mouse_move() calls, or by the object being shown,
 *   raised, moved, resized, or other objects being moved out of the
 *   way, hidden or lowered, whatever may cause the mouse pointer to
 *   get on top of @p obj, having been on top of another object
 *   previously.
 *
 * - #EVAS_CALLBACK_MOUSE_OUT: @p event_info is a pointer to an
 *   #Evas_Event_Mouse_Out struct\n\n
 *   This event is triggered exactly like #EVAS_CALLBACK_MOUSE_IN is,
 *   but it occurs when the mouse pointer exits an object's area. Note
 *   that no mouse out events will be reported if the mouse pointer is
 *   implicitly grabbed to an object (mouse buttons are down, having
 *   been pressed while the pointer was over that object). In these
 *   cases, mouse out events will be reported once all buttons are
 *   released, if the mouse pointer has left the object's area. The
 *   indirect ways of taking off the mouse pointer from an object,
 *   like cited above, for #EVAS_CALLBACK_MOUSE_IN, also apply here,
 *   naturally.
 *
 * - #EVAS_CALLBACK_MOUSE_DOWN: @p event_info is a pointer to an
 *   #Evas_Event_Mouse_Down struct\n\n
 *   This event is triggered by a mouse button being pressed while the
 *   mouse pointer is over an object. If the pointer mode for Evas is
 *   #EVAS_OBJECT_POINTER_MODE_AUTOGRAB (default), this causes this
 *   object to <b>passively grab the mouse</b> until all mouse buttons
 *   have been released: all future mouse events will be reported to
 *   only this object until no buttons are down. That includes mouse
 *   move events, mouse in and mouse out events, and further button
 *   presses. When all buttons are released, event propagation will
 *   occur as normal (see #Evas_Object_Pointer_Mode).
 *
 * - #EVAS_CALLBACK_MOUSE_UP: @p event_info is a pointer to an
 *   #Evas_Event_Mouse_Up struct\n\n
 *   This event is triggered by a mouse button being released while
 *   the mouse pointer is over an object's area (or when passively
 *   grabbed to an object).
 *
 * - #EVAS_CALLBACK_MOUSE_MOVE: @p event_info is a pointer to an
 *   #Evas_Event_Mouse_Move struct\n\n
 *   This event is triggered by the mouse pointer being moved while
 *   over an object's area (or while passively grabbed to an object).
 *
 * - #EVAS_CALLBACK_MOUSE_WHEEL: @p event_info is a pointer to an
 *   #Evas_Event_Mouse_Wheel struct\n\n
 *   This event is triggered by the mouse wheel being rolled while the
 *   mouse pointer is over an object (or passively grabbed to an
 *   object).
 *
 * - #EVAS_CALLBACK_MULTI_DOWN: @p event_info is a pointer to an
 *   #Evas_Event_Multi_Down struct
 *
 * - #EVAS_CALLBACK_MULTI_UP: @p event_info is a pointer to an
 *   #Evas_Event_Multi_Up struct
 *
 * - #EVAS_CALLBACK_MULTI_MOVE: @p event_info is a pointer to an
 *   #Evas_Event_Multi_Move struct
 *
 * - #EVAS_CALLBACK_FREE: @p event_info is @c NULL \n\n
 *   This event is triggered just before Evas is about to free all
 *   memory used by an object and remove all references to it. This is
 *   useful for programs to use if they attached data to an object and
 *   want to free it when the object is deleted. The object is still
 *   valid when this callback is called, but after it returns, there
 *   is no guarantee on the object's validity.
 *
 * - #EVAS_CALLBACK_KEY_DOWN: @p event_info is a pointer to an
 *   #Evas_Event_Key_Down struct\n\n
 *   This callback is called when a key is pressed and the focus is on
 *   the object, or a key has been grabbed to a particular object
 *   which wants to intercept the key press regardless of what object
 *   has the focus.
 *
 * - #EVAS_CALLBACK_KEY_UP: @p event_info is a pointer to an
 *   #Evas_Event_Key_Up struct \n\n
 *   This callback is called when a key is released and the focus is
 *   on the object, or a key has been grabbed to a particular object
 *   which wants to intercept the key release regardless of what
 *   object has the focus.
 *
 * - #EVAS_CALLBACK_FOCUS_IN: @p event_info is @c NULL \n\n
 *   This event is called when an object gains the focus. When it is
 *   called the object has already gained the focus.
 *
 * - #EVAS_CALLBACK_FOCUS_OUT: @p event_info is @c NULL \n\n
 *   This event is triggered when an object loses the focus. When it
 *   is called the object has already lost the focus.
 *
 * - #EVAS_CALLBACK_SHOW: @p event_info is @c NULL \n\n
 *   This event is triggered by the object being shown by
 *   evas_object_show().
 *
 * - #EVAS_CALLBACK_HIDE: @p event_info is @c NULL \n\n
 *   This event is triggered by an object being hidden by
 *   evas_object_hide().
 *
 * - #EVAS_CALLBACK_MOVE: @p event_info is @c NULL \n\n
 *   This event is triggered by an object being
 *   moved. evas_object_move() can trigger this, as can any
 *   object-specific manipulations that would mean the object's origin
 *   could move.
 *
 * - #EVAS_CALLBACK_RESIZE: @p event_info is @c NULL \n\n
 *   This event is triggered by an object being resized. Resizes can
 *   be triggered by evas_object_resize() or by any object-specific
 *   calls that may cause the object to resize.
 *
 * - #EVAS_CALLBACK_RESTACK: @p event_info is @c NULL \n\n
 *   This event is triggered by an object being re-stacked. Stacking
 *   changes can be triggered by
 *   evas_object_stack_below()/evas_object_stack_above() and others.
 *
 * - #EVAS_CALLBACK_DEL: @p event_info is @c NULL.
 *
 * - #EVAS_CALLBACK_HOLD: @p event_info is a pointer to an
 *   #Evas_Event_Hold struct
 *
 * - #EVAS_CALLBACK_CHANGED_SIZE_HINTS: @p event_info is @c NULL.
 *
 * - #EVAS_CALLBACK_IMAGE_PRELOADED: @p event_info is @c NULL.
 *
 * - #EVAS_CALLBACK_IMAGE_UNLOADED: @p event_info is @c NULL.
 *
 * @note Be careful not to add the same callback multiple times, if
 * that's not what you want, because Evas won't check if a callback
 * existed before exactly as the one being registered (and thus, call
 * it more than once on the event, in this case). This would make
 * sense if you passed different functions and/or callback data, only.
 *
 * Example:
 * @dontinclude evas-events.c
 * @skip evas_object_event_callback_add(
 * @until }
 *
 * See the full example @ref Example_Evas_Events "here".
 *
 */
EAPI void      evas_object_event_callback_add(Evas_Object *obj, Evas_Callback_Type type, Evas_Object_Event_Cb func, const void *data) EINA_ARG_NONNULL(1, 3);

/**
 * Add (register) a callback function to a given Evas object event with a
 * non-default priority set. Except for the priority field, it's exactly the
 * same as @ref evas_object_event_callback_add
 *
 * @param obj Object to attach a callback to
 * @param type The type of event that will trigger the callback
 * @param priority The priority of the callback, lower values called first.
 * @param func The function to be called when the event is triggered
 * @param data The data pointer to be passed to @p func
 *
 * @see evas_object_event_callback_add
 * @since 1.1
 */
EAPI void      evas_object_event_callback_priority_add(Evas_Object *obj, Evas_Callback_Type type, Evas_Callback_Priority priority, Evas_Object_Event_Cb func, const void *data) EINA_ARG_NONNULL(1, 4);

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
 * object. If not successful @c NULL will be returned.
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
EAPI void     *evas_object_event_callback_del(Evas_Object *obj, Evas_Callback_Type type, Evas_Object_Event_Cb func) EINA_ARG_NONNULL(1, 3);

/**
 * Delete (unregister) a callback function registered to a given
 * Evas object event.
 *
 * @param obj Object to remove a callback from
 * @param type The type of event that was triggering the callback
 * @param func The function that was to be called when the event was
 * triggered
 * @param data The data pointer that was to be passed to the callback
 * @return The data pointer that was to be passed to the callback
 *
 * This function removes the most recently added callback from the
 * object @p obj, which was triggered by the event type @p type and was
 * calling the function @p func with data @p data, when triggered. If
 * the removal is successful it will also return the data pointer that
 * was passed to evas_object_event_callback_add() (that will be the
 * same as the parameter) when the callback was added to the
 * object. In errors, @c NULL will be returned.
 *
 * @note For deletion of Evas object events callbacks filtering by
 * just type and function pointer, user
 * evas_object_event_callback_del().
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
EAPI void     *evas_object_event_callback_del_full(Evas_Object *obj, Evas_Callback_Type type, Evas_Object_Event_Cb func, const void *data) EINA_ARG_NONNULL(1, 3);

/**
 * Set whether an Evas object is to pass (ignore) events.
 *
 * @param obj the Evas object to operate on
 * @param pass whether @p obj is to pass events (@c EINA_TRUE) or not
 * (@c EINA_FALSE)
 *
 * If @p pass is @c EINA_TRUE, it will make events on @p obj to be @b
 * ignored. They will be triggered on the @b next lower object (that
 * is not set to pass events), instead (see evas_object_below_get()).
 *
 * If @p pass is @c EINA_FALSE, events will be processed on that
 * object as normal.
 *
 * @see evas_object_pass_events_get() for an example
 * @see evas_object_repeat_events_set()
 * @see evas_object_propagate_events_set()
 * @see evas_object_freeze_events_set()
 */
EAPI void      evas_object_pass_events_set(Evas_Object *obj, Eina_Bool pass) EINA_ARG_NONNULL(1);

/**
 * Determine whether an object is set to pass (ignore) events.
 *
 * @param obj the Evas object to get information from.
 * @return pass whether @p obj is set to pass events (@c EINA_TRUE) or not
 * (@c EINA_FALSE)
 *
 * Example:
 * @dontinclude evas-stacking.c
 * @skip if (strcmp(ev->keyname, "p") == 0)
 * @until }
 *
 * See the full @ref Example_Evas_Stacking "example".
 *
 * @see evas_object_pass_events_set()
 * @see evas_object_repeat_events_get()
 * @see evas_object_propagate_events_get()
 * @see evas_object_freeze_events_get()
 */
EAPI Eina_Bool evas_object_pass_events_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Set whether an Evas object is to repeat events.
 *
 * @param obj the Evas object to operate on
 * @param repeat whether @p obj is to repeat events (@c EINA_TRUE) or not
 * (@c EINA_FALSE)
 *
 * If @p repeat is @c EINA_TRUE, it will make events on @p obj to also
 * be repeated for the @b next lower object in the objects' stack (see
 * see evas_object_below_get()).
 *
 * If @p repeat is @c EINA_FALSE, events occurring on @p obj will be
 * processed only on it.
 *
 * Example:
 * @dontinclude evas-stacking.c
 * @skip if (strcmp(ev->keyname, "r") == 0)
 * @until }
 *
 * See the full @ref Example_Evas_Stacking "example".
 *
 * @see evas_object_repeat_events_get()
 * @see evas_object_pass_events_set()
 * @see evas_object_propagate_events_set()
 * @see evas_object_freeze_events_set()
 */
EAPI void      evas_object_repeat_events_set(Evas_Object *obj, Eina_Bool repeat) EINA_ARG_NONNULL(1);

/**
 * Determine whether an object is set to repeat events.
 *
 * @param obj the given Evas object pointer
 * @return whether @p obj is set to repeat events (@c EINA_TRUE)
 * or not (@c EINA_FALSE)
 *
 * @see evas_object_repeat_events_set() for an example
 * @see evas_object_pass_events_get()
 * @see evas_object_propagate_events_get()
 * @see evas_object_freeze_events_get()
 */
EAPI Eina_Bool evas_object_repeat_events_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Set whether events on a smart object's member should get propagated
 * up to its parent.
 *
 * @param obj the smart object's child to operate on
 * @param prop whether to propagate events (@c EINA_TRUE) or not
 * (@c EINA_FALSE)
 *
 * This function has @b no effect if @p obj is not a member of a smart
 * object.
 *
 * If @p prop is @c EINA_TRUE, events occurring on this object will be
 * propagated on to the smart object of which @p obj is a member.  If
 * @p prop is @c EINA_FALSE, events occurring on this object will @b
 * not be propagated on to the smart object of which @p obj is a
 * member.  The default value is @c EINA_TRUE.
 *
 * @see evas_object_propagate_events_get()
 * @see evas_object_repeat_events_set()
 * @see evas_object_pass_events_set()
 * @see evas_object_freeze_events_set()
 */
EAPI void      evas_object_propagate_events_set(Evas_Object *obj, Eina_Bool prop) EINA_ARG_NONNULL(1);

/**
 * Retrieve whether an Evas object is set to propagate events.
 *
 * @param obj the given Evas object pointer
 * @return whether @p obj is set to propagate events (@c EINA_TRUE)
 * or not (@c EINA_FALSE)
 *
 * @see evas_object_propagate_events_set()
 * @see evas_object_repeat_events_get()
 * @see evas_object_pass_events_get()
 * @see evas_object_freeze_events_get()
 */
EAPI Eina_Bool evas_object_propagate_events_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Set whether an Evas object is to freeze (discard) events.
 *
 * @param obj the Evas object to operate on
 * @param freeze pass whether @p obj is to freeze events (@c EINA_TRUE) or not
 * (@c EINA_FALSE)
 *
 * If @p freeze is @c EINA_TRUE, it will make events on @p obj to be @b
 * discarded. Unlike evas_object_pass_events_set(), events will not be
 * passed to @b next lower object. This API can be used for blocking
 * events while @p obj is on transiting.
 *
 * If @p freeze is @c EINA_FALSE, events will be processed on that
 * object as normal.
 *
 * @see evas_object_freeze_events_get()
 * @see evas_object_pass_events_set()
 * @see evas_object_repeat_events_set()
 * @see evas_object_propagate_events_set()
 * @since 1.1
 */
EAPI void      evas_object_freeze_events_set(Evas_Object *obj, Eina_Bool freeze) EINA_ARG_NONNULL(1);

/**
 * Determine whether an object is set to freeze (discard) events.
 *
 * @param obj the Evas object to get information from.
 * @return freeze whether @p obj is set to freeze events (@c EINA_TRUE) or
 * not (@c EINA_FALSE)
 *
 * @see evas_object_freeze_events_set()
 * @see evas_object_pass_events_get()
 * @see evas_object_repeat_events_get()
 * @see evas_object_propagate_events_get()
 * @since 1.1
 */
EAPI Eina_Bool evas_object_freeze_events_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @}
 */

/**
 * @defgroup Evas_Object_Group_Map UV Mapping (Rotation, Perspective, 3D...)
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
 * raw methods allow developers to create their maps somewhere else,
 * possibly loading them from some file format. The easy to use methods
 * calculate the points given some high-level parameters such as
 * rotation angle, ambient light, and so on.
 *
 * @note applying mapping will reduce performance, so use with
 *       care. The impact on performance depends on engine in
 *       use. Software is quite optimized, but not as fast as OpenGL.
 *
 * @section sec-map-points Map points
 * @subsection subsec-rotation Rotation
 *
 * A map consists of a set of points, currently only four are supported. Each
 * of these points contains a set of canvas coordinates @c x and @c y that
 * can be used to alter the geometry of the mapped object, and a @c z
 * coordinate that indicates the depth of that point. This last coordinate
 * does not normally affect the map, but it's used by several of the utility
 * functions to calculate the right position of the point given other
 * parameters.
 *
 * The coordinates for each point are set with evas_map_point_coord_set().
 * The following image shows a map set to match the geometry of an existing
 * object.
 *
 * @image html map-set-map-points-1.png
 * @image rtf map-set-map-points-1.png
 * @image latex map-set-map-points-1.eps
 *
 * This is a common practice, so there are a few functions that help make it
 * easier.
 *
 * evas_map_util_points_populate_from_geometry() sets the coordinates of each
 * point in the given map to match the rectangle defined by the function
 * parameters.
 *
 * evas_map_util_points_populate_from_object() and
 * evas_map_util_points_populate_from_object_full() both take an object and
 * set the map points to match its geometry. The difference between the two
 * is that the first function sets the @c z value of all points to 0, while
 * the latter receives the value to set in said coordinate as a parameter.
 *
 * The following lines of code all produce the same result as in the image
 * above.
 * @code
 * evas_map_util_points_populate_from_geometry(m, 100, 100, 200, 200, 0);
 * // Assuming o is our original object
 * evas_object_move(o, 100, 100);
 * evas_object_resize(o, 200, 200);
 * evas_map_util_points_populate_from_object(m, o);
 * evas_map_util_points_populate_from_object_full(m, o, 0);
 * @endcode
 *
 * Several effects can be applied to an object by simply setting each point
 * of the map to the right coordinates. For example, a simulated perspective
 * could be achieve as follows.
 *
 * @image html map-set-map-points-2.png
 * @image rtf map-set-map-points-2.png
 * @image latex map-set-map-points-2.eps
 *
 * As said before, the @c z coordinate is unused here so when setting points
 * by hand, its value is of no importance.
 *
 * @image html map-set-map-points-3.png
 * @image rtf map-set-map-points-3.png
 * @image latex map-set-map-points-3.eps
 *
 * In all three cases above, setting the map to be used by the object is the
 * same.
 * @code
 * evas_object_map_set(o, m);
 * evas_object_map_enable_set(o, EINA_TRUE);
 * @endcode
 *
 * Doing things this way, however, is a lot of work that can be avoided by
 * using the provided utility functions, as described in the next section.
 *
 * @section map-utils Utility functions
 *
 * Utility functions take an already set up map and alter it to produce a
 * specific effect. For example, to rotate an object around its own center
 * you would need to take the rotation angle, the coordinates of each corner
 * of the object and do all the math to get the new set of coordinates that
 * need to be set in the map.
 *
 * Or you can use this code:
 * @code
 * evas_object_geometry_get(o, &x, &y, &w, &h);
 * m = evas_map_new(4);
 * evas_map_util_points_populate_from_object(m, o);
 * evas_map_util_rotate(m, 45, x + (w / 2), y + (h / 2));
 * evas_object_map_set(o, m);
 * evas_object_map_enable_set(o, EINA_TRUE);
 * evas_map_free(m);
 * @endcode
 *
 * Which will rotate the object around its center point in a 45 degree angle
 * in the clockwise direction, taking it from this
 *
 * @image html map-rotation-2d-1.png
 * @image rtf map-rotation-2d-1.png
 * @image latex map-rotation-2d-1.eps
 *
 * to this
 *
 * @image html map-rotation-2d-2.png
 * @image rtf map-rotation-2d-2.png
 * @image latex map-rotation-2d-2.eps
 *
 * Objects may be rotated around any other point just by setting the last two
 * paramaters of the evas_map_util_rotate() function to the right values. A
 * circle of roughly the diameter of the object overlaid on each image shows
 * where the center of rotation is set for each example.
 *
 * For example, this code
 * @code
 * evas_object_geometry_get(o, &x, &y, &w, &h);
 * m = evas_map_new(4);
 * evas_map_util_points_populate_from_object(m, o);
 * evas_map_util_rotate(m, 45, x + w - 20, y + h - 20);
 * evas_object_map_set(o, m);
 * evas_object_map_enable_set(o, EINA_TRUE);
 * evas_map_free(m);
 * @endcode
 *
 * produces something like
 *
 * @image html map-rotation-2d-3.png
 * @image rtf map-rotation-2d-3.png
 * @image latex map-rotation-2d-3.eps
 *
 * And the following
 * @code
 * evas_output_size_get(evas, &w, &h);
 * m = evas_map_new(4);
 * evas_map_util_points_populate_from_object(m, o);
 * evas_map_util_rotate(m, 45, w, h);
 * evas_object_map_set(o, m);
 * evas_object_map_enable_set(o, EINA_TRUE);
 * evas_map_free(m);
 * @endcode
 *
 * rotates the object around the center of the window
 *
 * @image html map-rotation-2d-4.png
 * @image rtf map-rotation-2d-4.png
 * @image latex map-rotation-2d-4.eps
 *
 * @subsection subsec-3d 3D Maps
 *
 * Maps can also be used to achieve the effect of 3-dimensionality. When doing
 * this, the @c z coordinate of each point counts, with higher values meaning
 * the point is further into the screen, and smaller values (negative, usually)
 * meaning the point is closer towards the user.
 *
 * Thinking in 3D also introduces the concept of back-face of an object. An
 * object is said to be facing the user when all its points are placed in a
 * clockwise fashion. The next image shows this, with each point showing the
 * with which is identified within the map.
 *
 * @image html map-point-order-face.png
 * @image rtf map-point-order-face.png
 * @image latex map-point-order-face.eps
 *
 * Rotating this map around the @c Y axis would leave the order of the points
 * in a counter-clockwise fashion, as seen in the following image.
 *
 * @image html map-point-order-back.png
 * @image rtf map-point-order-back.png
 * @image latex map-point-order-back.eps
 *
 * This way we can say that we are looking at the back face of the object.
 * This will have stronger implications later when we talk about lighting.
 *
 * To know if a map is facing towards the user or not it's enough to use
 * the evas_map_util_clockwise_get() function, but this is normally done
 * after all the other operations are applied on the map.
 *
 * @subsection subsec-3d-rot 3D rotation and perspective
 *
 * Much like evas_map_util_rotate(), there's the function
 * evas_map_util_3d_rotate() that transforms the map to apply a 3D rotation
 * to an object. As in its 2D counterpart, the rotation can be applied around
 * any point in the canvas, this time with a @c z coordinate too. The rotation
 * can also be around any of the 3 axis.
 *
 * Starting from this simple setup
 *
 * @image html map-3d-basic-1.png
 * @image rtf map-3d-basic-1.png
 * @image latex map-3d-basic-1.eps
 *
 * and setting maps so that the blue square to rotate on all axis around a
 * sphere that uses the object as its center, and the red square to rotate
 * around the @c Y axis, we get the following. A simple overlay over the image
 * shows the original geometry of each object and the axis around which they
 * are being rotated, with the @c Z one not appearing due to being orthogonal
 * to the screen.
 *
 * @image html map-3d-basic-2.png
 * @image rtf map-3d-basic-2.png
 * @image latex map-3d-basic-2.eps
 *
 * which doesn't look very real. This can be helped by adding perspective
 * to the transformation, which can be simply done by calling
 * evas_map_util_3d_perspective() on the map after its position has been set.
 * The result in this case, making the vanishing point the center of each
 * object:
 *
 * @image html map-3d-basic-3.png
 * @image rtf map-3d-basic-3.png
 * @image latex map-3d-basic-3.eps
 *
 * @section sec-color Color and lighting
 *
 * Each point in a map can be set to a color, which will be multiplied with
 * the objects own color and linearly interpolated in between adjacent points.
 * This is done with evas_map_point_color_set() for each point of the map,
 * or evas_map_util_points_color_set() to set every point to the same color.
 *
 * When using 3D effects, colors can be used to improve the looks of them by
 * simulating a light source. The evas_map_util_3d_lighting() function makes
 * this task easier by taking the coordinates of the light source and its
 * color, along with the color of the ambient light. Evas then sets the color
 * of each point based on the distance to the light source, the angle with
 * which the object is facing the light and the ambient light. Here, the
 * orientation of each point as explained before, becomes more important.
 * If the map is defined counter-clockwise, the object will be facing away
 * from the user and thus become obscured, since no light would be reflecting
 * from it.
 *
 * @image html map-light.png
 * @image rtf map-light.png
 * @image latex map-light.eps
 * @note Object facing the light source
 *
 * @image html map-light2.png
 * @image rtf map-light2.png
 * @image latex map-light2.eps
 * @note Same object facing away from the user
 *
 * @section Image mapping
 *
 * @image html map-uv-mapping-1.png
 * @image rtf map-uv-mapping-1.png
 * @image latex map-uv-mapping-1.eps
 *
 * Images need some special handling when mapped. Evas can easily take care
 * of objects and do almost anything with them, but it's completely oblivious
 * to the content of images, so each point in the map needs to be told to what
 * pixel in the source image it belongs. Failing to do may sometimes result
 * in the expected behavior, or it may look like a partial work.
 *
 * The next image illustrates one possibility of a map being set to an image
 * object, without setting the right UV mapping for each point. The objects
 * themselves are mapped properly to their new geometry, but the image content
 * may not be displayed correctly within the mapped object.
 *
 * @image html map-uv-mapping-2.png
 * @image rtf map-uv-mapping-2.png
 * @image latex map-uv-mapping-2.eps
 *
 * Once Evas knows how to handle the source image within the map, it will
 * transform it as needed. This is done with evas_map_point_image_uv_set(),
 * which tells the map to which pixel in image it maps.
 *
 * To match our example images to the maps above all we need is the size of
 * each image, which can always be found with evas_object_image_size_get().
 *
 * @code
 * evas_map_point_image_uv_set(m, 0, 0, 0);
 * evas_map_point_image_uv_set(m, 1, 150, 0);
 * evas_map_point_image_uv_set(m, 2, 150, 200);
 * evas_map_point_image_uv_set(m, 3, 0, 200);
 * evas_object_map_set(o, m);
 * evas_object_map_enable_set(o, EINA_TRUE);
 *
 * evas_map_point_image_uv_set(m, 0, 0, 0);
 * evas_map_point_image_uv_set(m, 1, 120, 0);
 * evas_map_point_image_uv_set(m, 2, 120, 160);
 * evas_map_point_image_uv_set(m, 3, 0, 160);
 * evas_object_map_set(o2, m);
 * evas_object_map_enable_set(o2, EINA_TRUE);
 * @endcode
 *
 * To get
 *
 * @image html map-uv-mapping-3.png
 * @image rtf map-uv-mapping-3.png
 * @image latex map-uv-mapping-3.eps
 *
 * Maps can also be set to use part of an image only, or even map them inverted,
 * and combined with evas_object_image_source_set() it can be used to achieve
 * more interesting results.
 *
 * @code
 * evas_object_image_size_get(evas_object_image_source_get(o), &w, &h);
 * evas_map_point_image_uv_set(m, 0, 0, h);
 * evas_map_point_image_uv_set(m, 1, w, h);
 * evas_map_point_image_uv_set(m, 2, w, h / 3);
 * evas_map_point_image_uv_set(m, 3, 0, h / 3);
 * evas_object_map_set(o, m);
 * evas_object_map_enable_set(o, EINA_TRUE);
 * @endcode
 *
 * @image html map-uv-mapping-4.png
 * @image rtf map-uv-mapping-4.png
 * @image latex map-uv-mapping-4.eps
 *
 * Examples:
 * @li @ref Example_Evas_Map_Overview
 *
 * @ingroup Evas_Object_Group
 *
 * @{
 */

/**
 * Enable or disable the map that is set.
 *
 * Enable or disable the use of map for the object @p obj.
 * On enable, the object geometry will be saved, and the new geometry will
 * change (position and size) to reflect the map geometry set.
 *
 * If the object doesn't have a map set (with evas_object_map_set()), the
 * initial geometry will be undefined. It is advised to always set a map
 * to the object first, and then call this function to enable its use.
 *
 * @param obj object to enable the map on
 * @param enabled enabled state
 */
EAPI void            evas_object_map_enable_set(Evas_Object *obj, Eina_Bool enabled);

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
EAPI Eina_Bool       evas_object_map_enable_get(const Evas_Object *obj);

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
EAPI void            evas_object_map_set(Evas_Object *obj, const Evas_Map *map);

/**
 * Get current object transformation map.
 *
 * This returns the current internal map set on the indicated object. It is
 * intended for read-only access and is only valid as long as the object is
 * not deleted or the map on the object is not changed. If you wish to modify
 * the map and set it back do the following:
 *
 * @code
 * const Evas_Map *m = evas_object_map_get(obj);
 * Evas_Map *m2 = evas_map_dup(m);
 * evas_map_util_rotate(m2, 30.0, 0, 0);
 * evas_object_map_set(obj, m2);
 * evas_map_free(m2);
 * @endcode
 *
 * @param obj object to query transformation map.
 * @return map reference to map in use. This is an internal data structure, so
 * do not modify it.
 *
 * @see evas_object_map_set()
 */
EAPI const Evas_Map *evas_object_map_get(const Evas_Object *obj);

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
EAPI void            evas_map_util_points_populate_from_object_full(Evas_Map *m, const Evas_Object *obj, Evas_Coord z);

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
EAPI void            evas_map_util_points_populate_from_object(Evas_Map *m, const Evas_Object *obj);

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
EAPI void            evas_map_util_points_populate_from_geometry(Evas_Map *m, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h, Evas_Coord z);

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
EAPI void            evas_map_util_points_color_set(Evas_Map *m, int r, int g, int b, int a);

/**
 * Change the map to apply the given rotation.
 *
 * This rotates the indicated map's coordinates around the center coordinate
 * given by @p cx and @p cy as the rotation center. The points will have their
 * X and Y coordinates rotated clockwise by @p degrees degrees (360.0 is a
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
EAPI void            evas_map_util_rotate(Evas_Map *m, double degrees, Evas_Coord cx, Evas_Coord cy);

/**
 * Change the map to apply the given zooming.
 *
 * Like evas_map_util_rotate(), this zooms the points of the map from a center
 * point. That center is defined by @p cx and @p cy. The @p zoomx and @p zoomy
 * parameters specify how much to zoom in the X and Y direction respectively.
 * A value of 1.0 means "don't zoom". 2.0 means "double the size". 0.5 is
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
EAPI void            evas_map_util_zoom(Evas_Map *m, double zoomx, double zoomy, Evas_Coord cx, Evas_Coord cy);

/**
 * Rotate the map around 3 axes in 3D
 *
 * This will rotate not just around the "Z" axis as in evas_map_util_rotate()
 * (which is a convenience call for those only wanting 2D). This will rotate
 * around the X, Y and Z axes. The Z axis points "into" the screen with low
 * values at the screen and higher values further away. The X axis runs from
 * left to right on the screen and the Y axis from top to bottom. Like with
 * evas_map_util_rotate() you provide a center point to rotate around (in 3D).
 *
 * @param m map to change.
 * @param dx amount of degrees from 0.0 to 360.0 to rotate around X axis.
 * @param dy amount of degrees from 0.0 to 360.0 to rotate around Y axis.
 * @param dz amount of degrees from 0.0 to 360.0 to rotate around Z axis.
 * @param cx rotation's center horizontal position.
 * @param cy rotation's center vertical position.
 * @param cz rotation's center vertical position.
 */
EAPI void            evas_map_util_3d_rotate(Evas_Map *m, double dx, double dy, double dz, Evas_Coord cx, Evas_Coord cy, Evas_Coord cz);

/**
 * Rotate the map in 3D using a unit quaternion.
 *
 * This will rotate in 3D using a unit quaternion. Like with
 * evas_map_util_3d_rotate() you provide a center point 
 * to rotate around (in 3D).
 *
 * @param m map to change.
 * @param qx the x component of the imaginary part of the quaternion.
 * @param qy the y component of the imaginary part of the quaternion.
 * @param qz the z component of the imaginary part of the quaternion.
 * @param qw the w component of the real part of the quaternion.
 * @param cx rotation's center x.
 * @param cy rotation's center y.
 * @param cz rotation's center z.
 *
 * @warning Rotations can be done using a unit quaternion. Thus, this
 * function expects a unit quaternion (i.e. qx² + qy² + qz² + qw² == 1).
 * If this is not the case the behavior is undefined.
 *
 * @since 1.8
 */
EAPI void            evas_map_util_quat_rotate(Evas_Map *m, double qx, double qy, double qz, double qw, double cx, double cy, double cz);

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
 * @image html map-light3.png
 * @image rtf map-light3.png
 * @image latex map-light3.eps
 * @note Grey object, no lighting used
 *
 * @image html map-light4.png
 * @image rtf map-light4.png
 * @image latex map-light4.eps
 * @note Lights out! Every color set to 0
 *
 * @image html map-light5.png
 * @image rtf map-light5.png
 * @image latex map-light5.eps
 * @note Ambient light to full black, red light coming from close at the
 * bottom-left vertex
 *
 * @image html map-light6.png
 * @image rtf map-light6.png
 * @image latex map-light6.eps
 * @note Same light as before, but not the light is set to 0 and ambient light
 * is cyan
 *
 * @image html map-light7.png
 * @image rtf map-light7.png
 * @image latex map-light7.eps
 * @note Both lights are on
 *
 * @image html map-light8.png
 * @image rtf map-light8.png
 * @image latex map-light8.eps
 * @note Both lights again, but this time both are the same color.
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
EAPI void            evas_map_util_3d_lighting(Evas_Map *m, Evas_Coord lx, Evas_Coord ly, Evas_Coord lz, int lr, int lg, int lb, int ar, int ag, int ab);

/**
 * Apply a perspective transform to the map
 *
 * This applies a given perspective (3D) to the map coordinates. X, Y and Z
 * values are used. The px and py points specify the "infinite distance" point
 * in the 3D conversion (where all lines converge to like when artists draw
 * 3D by hand). The @p z0 value specifies the z value at which there is a 1:1
 * mapping between spatial coordinates and screen coordinates. Any points
 * on this z value will not have their X and Y values modified in the transform.
 * Those further away (Z value higher) will shrink into the distance, and
 * those less than this value will expand and become bigger. The @p foc value
 * determines the "focal length" of the camera. This is in reality the distance
 * between the camera lens plane itself (at or closer than this rendering
 * results are undefined) and the "z0" z value. This allows for some "depth"
 * control and @p foc must be greater than 0.
 *
 * @param m map to change.
 * @param px The perspective distance X coordinate
 * @param py The perspective distance Y coordinate
 * @param z0 The "0" z plane value
 * @param foc The focal distance
 */
EAPI void            evas_map_util_3d_perspective(Evas_Map *m, Evas_Coord px, Evas_Coord py, Evas_Coord z0, Evas_Coord foc);

/**
 * Get the clockwise state of a map
 *
 * This determines if the output points (X and Y. Z is not used) are
 * clockwise or counter-clockwise. This can be used for "back-face culling". This
 * is where you hide objects that "face away" from you. In this case objects
 * that are not clockwise.
 *
 * @param m map to query.
 * @return 1 if clockwise, 0 otherwise
 */
EAPI Eina_Bool       evas_map_util_clockwise_get(Evas_Map *m);

/**
 * Create map of transformation points to be later used with an Evas object.
 *
 * This creates a set of points (currently only 4 is supported. no other
 * number for @p count will work). That is empty and ready to be modified
 * with evas_map calls.
 *
 * @param count number of points in the map.
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
EAPI Evas_Map       *evas_map_new(int count);

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
EAPI void            evas_map_smooth_set(Evas_Map *m, Eina_Bool enabled);

/**
 * get the smoothing for map rendering
 *
 * This gets smoothing for map rendering.
 *
 * @param m map to get the smooth from. Must not be NULL.
 */
EAPI Eina_Bool       evas_map_smooth_get(const Evas_Map *m);

/**
 * Set the alpha flag for map rendering
 *
 * This sets alpha flag for map rendering. If the object is a type that has
 * its own alpha settings, then this will take precedence. Only image objects
 * have this currently.
 * Setting this off stops alpha blending of the map area, and is
 * useful if you know the object and/or all sub-objects is 100% solid.
 *
 * @param m map to modify. Must not be NULL.
 * @param enabled enable or disable alpha map rendering
 */
EAPI void            evas_map_alpha_set(Evas_Map *m, Eina_Bool enabled);

/**
 * get the alpha flag for map rendering
 *
 * This gets the alpha flag for map rendering.
 *
 * @param m map to get the alpha from. Must not be NULL.
 */
EAPI Eina_Bool       evas_map_alpha_get(const Evas_Map *m);

/**
 * Copy a previously allocated map.
 *
 * This makes a duplicate of the @p m object and returns it.
 *
 * @param m map to copy. Must not be NULL.
 * @return newly allocated map with the same count and contents as @p m.
 */
EAPI Evas_Map       *evas_map_dup(const Evas_Map *m);

/**
 * Free a previously allocated map.
 *
 * This frees a givem map @p m and all memory associated with it. You must NOT
 * free a map returned by evas_object_map_get() as this is internal.
 *
 * @param m map to free.
 */
EAPI void            evas_map_free(Evas_Map *m);

/**
 * Get a maps size.
 *
 * Returns the number of points in a map.  Should be at least 4.
 *
 * @param m map to get size.
 * @return -1 on error, points otherwise.
 */
EAPI int             evas_map_count_get(const Evas_Map *m) EINA_CONST;

/**
 * Change the map point's coordinate.
 *
 * This sets the fixed point's coordinate in the map. Note that points
 * describe the outline of a quadrangle and are ordered either clockwise
 * or counter-clockwise. It is suggested to keep your quadrangles concave and
 * non-complex, though these polygon modes may work, they may not render
 * a desired set of output. The quadrangle will use points 0 and 1 , 1 and 2,
 * 2 and 3, and 3 and 0 to describe the edges of the quadrangle.
 *
 * The X and Y and Z coordinates are in canvas units. Z is optional and may
 * or may not be honored in drawing. Z is a hint and does not affect the
 * X and Y rendered coordinates. It may be used for calculating fills with
 * perspective correct rendering.
 *
 * Remember all coordinates are canvas global ones like with move and resize
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
EAPI void            evas_map_point_coord_set(Evas_Map *m, int idx, Evas_Coord x, Evas_Coord y, Evas_Coord z);

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
EAPI void            evas_map_point_coord_get(const Evas_Map *m, int idx, Evas_Coord *x, Evas_Coord *y, Evas_Coord *z);

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
EAPI void            evas_map_point_image_uv_set(Evas_Map *m, int idx, double u, double v);

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
EAPI void            evas_map_point_image_uv_get(const Evas_Map *m, int idx, double *u, double *v);

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
EAPI void            evas_map_point_color_set(Evas_Map *m, int idx, int r, int g, int b, int a);

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
EAPI void            evas_map_point_color_get(const Evas_Map *m, int idx, int *r, int *g, int *b, int *a);
/**
 * @}
 */

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
 * Examples on their usage:
 * - @ref Example_Evas_Size_Hints "evas-hints.c"
 * - @ref Example_Evas_Aspect_Hints "evas-aspect-hints.c"
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
EAPI void evas_object_size_hint_min_get(const Evas_Object *obj, Evas_Coord *w, Evas_Coord *h) EINA_ARG_NONNULL(1);

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
 * In this example the minimum size hints change the behavior of an
 * Evas box when layouting its children. See the full @ref
 * Example_Evas_Size_Hints "example".
 *
 * @see evas_object_size_hint_min_get()
 */
EAPI void evas_object_size_hint_min_set(Evas_Object *obj, Evas_Coord w, Evas_Coord h) EINA_ARG_NONNULL(1);

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
EAPI void evas_object_size_hint_max_get(const Evas_Object *obj, Evas_Coord *w, Evas_Coord *h) EINA_ARG_NONNULL(1);

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
 * In this example the maximum size hints change the behavior of an
 * Evas box when layouting its children. See the full @ref
 * Example_Evas_Size_Hints "example".
 *
 * @see evas_object_size_hint_max_get()
 */
EAPI void evas_object_size_hint_max_set(Evas_Object *obj, Evas_Coord w, Evas_Coord h) EINA_ARG_NONNULL(1);

/**
 * Retrieves the hints for an object's display mode
 *
 * @param obj The given Evas object to query hints from.
 *
 * These are hints on the display mode @p obj. This is
 * not a size enforcement in any way, it's just a hint that can be
 * used whenever appropriate.
 * This mode can be used object's display mode like commpress or expand
 *
 * @see evas_object_size_hint_display_mode_set()
 */
EAPI Evas_Display_Mode evas_object_size_hint_display_mode_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * Sets the hints for an object's disply mode
 *
 * @param obj The given Evas object to query hints from.
 * @param dispmode to use as the display mode hint.
 *
 * This is not a size enforcement in any way, it's just a hint that
 * can be used whenever appropriate.
 *
 * @see evas_object_size_hint_display_mode_get()
 */
EAPI void evas_object_size_hint_display_mode_set(Evas_Object *obj, Evas_Display_Mode dispmode) EINA_ARG_NONNULL(1);

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
EAPI void evas_object_size_hint_request_get(const Evas_Object *obj, Evas_Coord *w, Evas_Coord *h) EINA_ARG_NONNULL(1);

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
EAPI void evas_object_size_hint_request_set(Evas_Object *obj, Evas_Coord w, Evas_Coord h) EINA_ARG_NONNULL(1);

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
 * would @b resize its children accordingly to those policies.
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
EAPI void evas_object_size_hint_aspect_get(const Evas_Object *obj, Evas_Aspect_Control *aspect, Evas_Coord *w, Evas_Coord *h) EINA_ARG_NONNULL(1);

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
EAPI void evas_object_size_hint_aspect_set(Evas_Object *obj, Evas_Aspect_Control aspect, Evas_Coord w, Evas_Coord h) EINA_ARG_NONNULL(1);

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
 * @note If @c obj is invalid, then the hint components will be set with 0.5
 *
 * @see evas_object_size_hint_align_set() for more information
 */
EAPI void evas_object_size_hint_align_get(const Evas_Object *obj, double *x, double *y) EINA_ARG_NONNULL(1);

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
 * "justify" or "fill" by some users. In this case, maximum size hints
 * should be enforced with higher priority, if they are set. Also, any
 * padding hint set on objects should add up to the alignment space on
 * the final scene composition.
 *
 * See documentation of possible users: in Evas, they are the @ref
 * Evas_Object_Box "box" and @ref Evas_Object_Table "table" smart
 * objects.
 *
 * For the horizontal component, @c 0.0 means to the left, @c 1.0
 * means to the right. Analogously, for the vertical component, @c 0.0
 * to the top, @c 1.0 means to the bottom.
 *
 * See the following figure:
 *
 * @image html alignment-hints.png
 * @image rtf alignment-hints.png
 * @image latex alignment-hints.eps
 *
 * This is not a size enforcement in any way, it's just a hint that
 * should be used whenever appropriate.
 *
 * @note Default alignment hint values are 0.5, for both axis.
 *
 * Example:
 * @dontinclude evas-hints.c
 * @skip evas_object_size_hint_align_set
 * @until return
 *
 * In this example the alignment hints change the behavior of an Evas
 * box when layouting its children. See the full @ref
 * Example_Evas_Size_Hints "example".
 *
 * @see evas_object_size_hint_align_get()
 * @see evas_object_size_hint_max_set()
 * @see evas_object_size_hint_padding_set()
 */
EAPI void evas_object_size_hint_align_set(Evas_Object *obj, double x, double y) EINA_ARG_NONNULL(1);

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
 * @note If @c obj is invalid, then the hint components will be set with 0.0
 *
 * @see evas_object_size_hint_weight_set() for an example
 */
EAPI void evas_object_size_hint_weight_get(const Evas_Object *obj, double *x, double *y) EINA_ARG_NONNULL(1);

/**
 * Sets the hints for an object's weight.
 *
 * @param obj The given Evas object to query hints from.
 * @param x Nonnegative double value to use as horizontal weight hint.
 * @param y Nonnegative double value to use as vertical weight hint.
 *
 * This is not a size enforcement in any way, it's just a hint that
 * should be used whenever appropriate.
 *
 * This is a hint on how a container object should @b resize a given
 * child within its area. Containers may adhere to the simpler logic
 * of just expanding the child object's dimensions to fit its own (see
 * the #EVAS_HINT_EXPAND helper weight macro) or the complete one of
 * taking each child's weight hint as real @b weights to how much of
 * its size to allocate for them in each axis. A container is supposed
 * to, after @b normalizing the weights of its children (with weight
 * hints), distribute the space it has to layout them by those factors
 * -- most weighted children get larger in this process than the least
 * ones.
 *
 * Example:
 * @dontinclude evas-hints.c
 * @skip evas_object_size_hint_weight_set
 * @until return
 *
 * In this example the weight hints change the behavior of an Evas box
 * when layouting its children. See the full @ref
 * Example_Evas_Size_Hints "example".
 *
 * @note Default weight hint values are 0.0, for both axis.
 *
 * @see evas_object_size_hint_weight_get() for more information
 */
EAPI void evas_object_size_hint_weight_set(Evas_Object *obj, double x, double y) EINA_ARG_NONNULL(1);

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
 * transparent, naturally, as in the following figure:
 *
 * @image html padding-hints.png
 * @image rtf padding-hints.png
 * @image latex padding-hints.eps
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
 * In this example the padding hints change the behavior of an Evas box
 * when layouting its children. See the full @ref
 * Example_Evas_Size_Hints "example".
 *
 * @see evas_object_size_hint_padding_set()
 */
EAPI void evas_object_size_hint_padding_get(const Evas_Object *obj, Evas_Coord *l, Evas_Coord *r, Evas_Coord *t, Evas_Coord *b) EINA_ARG_NONNULL(1);

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
EAPI void evas_object_size_hint_padding_set(Evas_Object *obj, Evas_Coord l, Evas_Coord r, Evas_Coord t, Evas_Coord b) EINA_ARG_NONNULL(1);

/**
 * @}
 */

/**
 * @defgroup Evas_Object_Group_Extras Extra Object Manipulation
 *
 * Miscellaneous functions that also apply to any object, but are less
 * used or not implemented by all objects.
 *
 * Examples on this group of functions can be found @ref
 * Example_Evas_Stacking "here" and @ref Example_Evas_Events "here".
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
 * @param data The pointer to the data to be attached
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
EAPI void                     evas_object_data_set(Evas_Object *obj, const char *key, const void *data) EINA_ARG_NONNULL(1, 2);

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
EAPI void                    *evas_object_data_get(const Evas_Object *obj, const char *key) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1, 2);

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
EAPI void                    *evas_object_data_del(Evas_Object *obj, const char *key) EINA_ARG_NONNULL(1, 2);

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
EAPI void                     evas_object_pointer_mode_set(Evas_Object *obj, Evas_Object_Pointer_Mode setting) EINA_ARG_NONNULL(1);

/**
 * Determine how pointer will behave.
 * @param obj
 * @return pointer behavior.
 * @ingroup Evas_Object_Group_Extras
 */
EAPI Evas_Object_Pointer_Mode evas_object_pointer_mode_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Sets whether or not the given Evas object is to be drawn anti-aliased.
 *
 * @param   obj The given Evas object.
 * @param   antialias 1 if the object is to be anti_aliased, 0 otherwise.
 * @ingroup Evas_Object_Group_Extras
 */
EAPI void                     evas_object_anti_alias_set(Evas_Object *obj, Eina_Bool antialias) EINA_ARG_NONNULL(1);

/**
 * Retrieves whether or not the given Evas object is to be drawn anti_aliased.
 * @param   obj The given Evas object.
 * @return  @c 1 if the object is to be anti_aliased.  @c 0 otherwise.
 * @ingroup Evas_Object_Group_Extras
 */
EAPI Eina_Bool                evas_object_anti_alias_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

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
EAPI void                     evas_object_scale_set(Evas_Object *obj, double scale) EINA_ARG_NONNULL(1);

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
EAPI double                   evas_object_scale_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Sets the render_op to be used for rendering the Evas object.
 * @param   obj The given Evas object.
 * @param   op one of the Evas_Render_Op values.
 * @ingroup Evas_Object_Group_Extras
 */
EAPI void                     evas_object_render_op_set(Evas_Object *obj, Evas_Render_Op op) EINA_ARG_NONNULL(1);

/**
 * Retrieves the current value of the operation used for rendering the Evas object.
 * @param   obj The given Evas object.
 * @return  one of the enumerated values in Evas_Render_Op.
 * @ingroup Evas_Object_Group_Extras
 */
EAPI Evas_Render_Op           evas_object_render_op_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Set whether to use precise (usually expensive) point collision
 * detection for a given Evas object.
 *
 * @param obj The given object.
 * @param precise Whether to use precise point collision detection or
 * not. The default value is false.
 *
 * Use this function to make Evas treat objects' transparent areas as
 * @b not belonging to it with regard to mouse pointer events. By
 * default, all of the object's boundary rectangle will be taken in
 * account for them.
 *
 * @warning By using precise point collision detection you'll be
 * making Evas more resource intensive.
 *
 * Example code follows.
 * @dontinclude evas-events.c
 * @skip if (strcmp(ev->keyname, "p") == 0)
 * @until }
 *
 * See the full example @ref Example_Evas_Events "here".
 *
 * @see evas_object_precise_is_inside_get()
 * @ingroup Evas_Object_Group_Extras
 */
EAPI void                     evas_object_precise_is_inside_set(Evas_Object *obj, Eina_Bool precise) EINA_ARG_NONNULL(1);

/**
 * Determine whether an object is set to use precise point collision
 * detection.
 *
 * @param obj The given object.
 * @return whether @p obj is set to use precise point collision
 * detection or not The default value is false.
 *
 * @see evas_object_precise_is_inside_set() for an example
 *
 * @ingroup Evas_Object_Group_Extras
 */
EAPI Eina_Bool                evas_object_precise_is_inside_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Set a hint flag on the given Evas object that it's used as a "static
 * clipper".
 *
 * @param obj The given object.
 * @param is_static_clip @c EINA_TRUE if it's to be used as a static
 * clipper, @c EINA_FALSE otherwise.
 *
 * This is a hint to Evas that this object is used as a big static
 * clipper and shouldn't be moved with children and otherwise
 * considered specially. The default value for new objects is
 * @c EINA_FALSE.
 *
 * @see evas_object_static_clip_get()
 *
 * @ingroup Evas_Object_Group_Extras
 */
EAPI void                     evas_object_static_clip_set(Evas_Object *obj, Eina_Bool is_static_clip) EINA_ARG_NONNULL(1);

/**
 * Get the "static clipper" hint flag for a given Evas object.
 *
 * @param obj The given object.
 * @return @c EINA_TRUE if it's set as a static clipper,
 * @c EINA_FALSE otherwise.
 *
 * @see evas_object_static_clip_set() for more details
 *
 * @ingroup Evas_Object_Group_Extras
 */
EAPI Eina_Bool                evas_object_static_clip_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

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
 * @addtogroup Evas_Object_Group_Find
 * @{
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
EAPI Evas_Object *evas_focus_get(const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Retrieves the object on the given evas with the given name.
 * @param   e    The given evas.
 * @param   name The given name.
 * @return  If successful, the Evas object with the given name.  Otherwise,
 *          @c NULL.
 *
 * This looks for the evas object given a name by evas_object_name_set(). If
 * the name is not unique canvas-wide, then which one of the many objects
 * with that name is returned is undefined, so only use this if you can ensure
 * the object name is unique.
 *
 * @ingroup Evas_Object_Group_Find
 */
EAPI Evas_Object *evas_object_name_find(const Evas *e, const char *name) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Retrieves the object from children of the given object with the given name.
 * @param   obj  The parent (smart) object whose children to search.
 * @param   name The given name.
 * @param   recurse Set to the number of child levels to recurse (0 == don't recurse, 1 == only look at the children of @p obj or their immediate children, but no further etc.).
 * @return  If successful, the Evas object with the given name.  Otherwise,
 *          @c NULL.
 *
 * This looks for the evas object given a name by evas_object_name_set(), but
 * it ONLY looks at the children of the object *p obj, and will only recurse
 * into those children if @p recurse is greater than 0. If the name is not
 * unique within immediate children (or the whole child tree) then it is not
 * defined which child object will be returned. If @p recurse is set to -1 then
 * it will recurse without limit.
 *
 * @since 1.2
 *
 * @ingroup Evas_Object_Group_Find
 */
EAPI Evas_Object *evas_object_name_child_find(const Evas_Object *obj, const char *name, int recurse) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Retrieve the Evas object stacked at the top of a given position in
 * a canvas
 *
 * @param   e A handle to the canvas.
 * @param   x The horizontal coordinate of the position
 * @param   y The vertical coordinate of the position
 * @param   include_pass_events_objects Boolean flag to include or not
 * objects which pass events in this calculation
 * @param   include_hidden_objects Boolean flag to include or not hidden
 * objects in this calculation
 * @return  The Evas object that is over all other objects at the given
 * position.
 *
 * This function will traverse all the layers of the given canvas,
 * from top to bottom, querying for objects with areas covering the
 * given position. The user can remove from the query
 * objects which are hidden and/or which are set to pass events.
 *
 * @warning This function will @b skip objects parented by smart
 * objects, acting only on the ones at the "top level", with regard to
 * object parenting.
 */
EAPI Evas_Object *evas_object_top_at_xy_get(const Evas *e, Evas_Coord x, Evas_Coord y, Eina_Bool include_pass_events_objects, Eina_Bool include_hidden_objects) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Retrieve the Evas object stacked at the top at the position of the
 * mouse cursor, over a given canvas
 *
 * @param   e A handle to the canvas.
 * @return  The Evas object that is over all other objects at the mouse
 * pointer's position
 *
 * This function will traverse all the layers of the given canvas,
 * from top to bottom, querying for objects with areas covering the
 * mouse pointer's position, over @p e.
 *
 * @warning This function will @b skip objects parented by smart
 * objects, acting only on the ones at the "top level", with regard to
 * object parenting.
 */
EAPI Evas_Object *evas_object_top_at_pointer_get(const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Retrieve the Evas object stacked at the top of a given rectangular
 * region in a canvas
 *
 * @param   e A handle to the canvas.
 * @param   x The top left corner's horizontal coordinate for the
 * rectangular region
 * @param   y The top left corner's vertical coordinate for the
 * rectangular region
 * @param   w The width of the rectangular region
 * @param   h The height of the rectangular region
 * @param   include_pass_events_objects Boolean flag to include or not
 * objects which pass events in this calculation
 * @param   include_hidden_objects Boolean flag to include or not hidden
 * objects in this calculation
 * @return  The Evas object that is over all other objects at the given
 * rectangular region.
 *
 * This function will traverse all the layers of the given canvas,
 * from top to bottom, querying for objects with areas overlapping
 * with the given rectangular region inside @p e. The user can remove
 * from the query objects which are hidden and/or which are set to
 * pass events.
 *
 * @warning This function will @b skip objects parented by smart
 * objects, acting only on the ones at the "top level", with regard to
 * object parenting.
 */
EAPI Evas_Object *evas_object_top_in_rectangle_get(const Evas *e, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h, Eina_Bool include_pass_events_objects, Eina_Bool include_hidden_objects) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Retrieve a list of Evas objects lying over a given position in
 * a canvas
 *
 * @param   e A handle to the canvas.
 * @param   x The horizontal coordinate of the position
 * @param   y The vertical coordinate of the position
 * @param   include_pass_events_objects Boolean flag to include or not
 * objects which pass events in this calculation
 * @param   include_hidden_objects Boolean flag to include or not hidden
 * objects in this calculation
 * @return  The list of Evas objects that are over the given position
 * in @p e
 *
 * This function will traverse all the layers of the given canvas,
 * from top to bottom, querying for objects with areas covering the
 * given position. The user can remove from query
 * objects which are hidden and/or which are set to pass events.
 *
 * @warning This function will @b skip objects parented by smart
 * objects, acting only on the ones at the "top level", with regard to
 * object parenting.
 */
EAPI Eina_List   *evas_objects_at_xy_get(const Evas *e, Evas_Coord x, Evas_Coord y, Eina_Bool include_pass_events_objects, Eina_Bool include_hidden_objects) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);
EAPI Eina_List   *evas_objects_in_rectangle_get(const Evas *e, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h, Eina_Bool include_pass_events_objects, Eina_Bool include_hidden_objects) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Get the lowest (stacked) Evas object on the canvas @p e.
 *
 * @param e a valid canvas pointer
 * @return a pointer to the lowest object on it, if any, or @c NULL,
 * otherwise
 *
 * This function will take all populated layers in the canvas into
 * account, getting the lowest object for the lowest layer, naturally.
 *
 * @see evas_object_layer_get()
 * @see evas_object_layer_set()
 * @see evas_object_below_get()
 * @see evas_object_above_get()
 *
 * @warning This function will @b skip objects parented by smart
 * objects, acting only on the ones at the "top level", with regard to
 * object parenting.
 */
EAPI Evas_Object *evas_object_bottom_get(const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Get the highest (stacked) Evas object on the canvas @p e.
 *
 * @param e a valid canvas pointer
 * @return a pointer to the highest object on it, if any, or @c NULL,
 * otherwise
 *
 * This function will take all populated layers in the canvas into
 * account, getting the highest object for the highest layer,
 * naturally.
 *
 * @see evas_object_layer_get()
 * @see evas_object_layer_set()
 * @see evas_object_below_get()
 * @see evas_object_above_get()
 *
 * @warning This function will @b skip objects parented by smart
 * objects, acting only on the ones at the "top level", with regard to
 * object parenting.
 */
EAPI Evas_Object *evas_object_top_get(const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @}
 */

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

/**
 * @addtogroup Evas_Object_Group_Interceptors
 * @{
 */

typedef void (*Evas_Object_Intercept_Show_Cb)(void *data, Evas_Object *obj);
typedef void (*Evas_Object_Intercept_Hide_Cb)(void *data, Evas_Object *obj);
typedef void (*Evas_Object_Intercept_Move_Cb)(void *data, Evas_Object *obj, Evas_Coord x, Evas_Coord y);
typedef void (*Evas_Object_Intercept_Resize_Cb)(void *data, Evas_Object *obj, Evas_Coord w, Evas_Coord h);
typedef void (*Evas_Object_Intercept_Raise_Cb)(void *data, Evas_Object *obj);
typedef void (*Evas_Object_Intercept_Lower_Cb)(void *data, Evas_Object *obj);
typedef void (*Evas_Object_Intercept_Stack_Above_Cb)(void *data, Evas_Object *obj, Evas_Object *above);
typedef void (*Evas_Object_Intercept_Stack_Below_Cb)(void *data, Evas_Object *obj, Evas_Object *above);
typedef void (*Evas_Object_Intercept_Layer_Set_Cb)(void *data, Evas_Object *obj, int l);
typedef void (*Evas_Object_Intercept_Color_Set_Cb)(void *data, Evas_Object *obj, int r, int g, int b, int a);
typedef void (*Evas_Object_Intercept_Clip_Set_Cb)(void *data, Evas_Object *obj, Evas_Object *clip);
typedef void (*Evas_Object_Intercept_Clip_Unset_Cb)(void *data, Evas_Object *obj);

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
EAPI void  evas_object_intercept_show_callback_add(Evas_Object *obj, Evas_Object_Intercept_Show_Cb func, const void *data) EINA_ARG_NONNULL(1, 2);

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
EAPI void *evas_object_intercept_show_callback_del(Evas_Object *obj, Evas_Object_Intercept_Show_Cb func) EINA_ARG_NONNULL(1, 2);

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
EAPI void  evas_object_intercept_hide_callback_add(Evas_Object *obj, Evas_Object_Intercept_Hide_Cb func, const void *data) EINA_ARG_NONNULL(1, 2);

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
EAPI void *evas_object_intercept_hide_callback_del(Evas_Object *obj, Evas_Object_Intercept_Hide_Cb func) EINA_ARG_NONNULL(1, 2);

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
EAPI void  evas_object_intercept_move_callback_add(Evas_Object *obj, Evas_Object_Intercept_Move_Cb func, const void *data) EINA_ARG_NONNULL(1, 2);

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
EAPI void *evas_object_intercept_move_callback_del(Evas_Object *obj, Evas_Object_Intercept_Move_Cb func) EINA_ARG_NONNULL(1, 2);

EAPI void  evas_object_intercept_resize_callback_add(Evas_Object *obj, Evas_Object_Intercept_Resize_Cb func, const void *data) EINA_ARG_NONNULL(1, 2);
EAPI void *evas_object_intercept_resize_callback_del(Evas_Object *obj, Evas_Object_Intercept_Resize_Cb func) EINA_ARG_NONNULL(1, 2);
EAPI void  evas_object_intercept_raise_callback_add(Evas_Object *obj, Evas_Object_Intercept_Raise_Cb func, const void *data) EINA_ARG_NONNULL(1, 2);
EAPI void *evas_object_intercept_raise_callback_del(Evas_Object *obj, Evas_Object_Intercept_Raise_Cb func) EINA_ARG_NONNULL(1, 2);
EAPI void  evas_object_intercept_lower_callback_add(Evas_Object *obj, Evas_Object_Intercept_Lower_Cb func, const void *data) EINA_ARG_NONNULL(1, 2);
EAPI void *evas_object_intercept_lower_callback_del(Evas_Object *obj, Evas_Object_Intercept_Lower_Cb func) EINA_ARG_NONNULL(1, 2);
EAPI void  evas_object_intercept_stack_above_callback_add(Evas_Object *obj, Evas_Object_Intercept_Stack_Above_Cb func, const void *data) EINA_ARG_NONNULL(1, 2);
EAPI void *evas_object_intercept_stack_above_callback_del(Evas_Object *obj, Evas_Object_Intercept_Stack_Above_Cb func) EINA_ARG_NONNULL(1, 2);
EAPI void  evas_object_intercept_stack_below_callback_add(Evas_Object *obj, Evas_Object_Intercept_Stack_Below_Cb func, const void *data) EINA_ARG_NONNULL(1, 2);
EAPI void *evas_object_intercept_stack_below_callback_del(Evas_Object *obj, Evas_Object_Intercept_Stack_Below_Cb func) EINA_ARG_NONNULL(1, 2);
EAPI void  evas_object_intercept_layer_set_callback_add(Evas_Object *obj, Evas_Object_Intercept_Layer_Set_Cb func, const void *data) EINA_ARG_NONNULL(1, 2);
EAPI void *evas_object_intercept_layer_set_callback_del(Evas_Object *obj, Evas_Object_Intercept_Layer_Set_Cb func) EINA_ARG_NONNULL(1, 2);
EAPI void  evas_object_intercept_color_set_callback_add(Evas_Object *obj, Evas_Object_Intercept_Color_Set_Cb func, const void *data) EINA_ARG_NONNULL(1, 2);
EAPI void *evas_object_intercept_color_set_callback_del(Evas_Object *obj, Evas_Object_Intercept_Color_Set_Cb func) EINA_ARG_NONNULL(1, 2);
EAPI void  evas_object_intercept_clip_set_callback_add(Evas_Object *obj, Evas_Object_Intercept_Clip_Set_Cb func, const void *data) EINA_ARG_NONNULL(1, 2);
EAPI void *evas_object_intercept_clip_set_callback_del(Evas_Object *obj, Evas_Object_Intercept_Clip_Set_Cb func) EINA_ARG_NONNULL(1, 2);
EAPI void  evas_object_intercept_clip_unset_callback_add(Evas_Object *obj, Evas_Object_Intercept_Clip_Unset_Cb func, const void *data) EINA_ARG_NONNULL(1, 2);
EAPI void *evas_object_intercept_clip_unset_callback_del(Evas_Object *obj, Evas_Object_Intercept_Clip_Unset_Cb func) EINA_ARG_NONNULL(1, 2);

/**
 * @}
 */

/**
 * @defgroup Evas_Object_Specific Specific Object Functions
 *
 * Functions that work on specific objects.
 *
 */

/**
 * @defgroup Evas_Object_Rectangle Rectangle Object Functions
 *
 * @brief Function to create evas rectangle objects.
 *
 * There is only one function to deal with rectangle objects, this may make this
 * function seem useless given there are no functions to manipulate the created
 * rectangle, however the rectangle is actually very useful and should be
 * manipulated using the generic @ref Evas_Object_Group "evas object functions".
 *
 * The evas rectangle serves a number of key functions when working on evas
 * programs:
 * @li Background
 * @li Debugging
 * @li Clipper
 *
 * @section Background
 *
 * One extremely common requirement of evas programs is to have a solid color
 * background, this can be accomplished with the following very simple code:
 * @code
 * Evas_Object *bg = evas_object_rectangle_add(evas_canvas);
 * //Here we set the rectangles red, green, blue and opacity levels
 * evas_object_color_set(bg, 255, 255, 255, 255); // opaque white background
 * evas_object_resize(bg, WIDTH, HEIGHT); // covers full canvas
 * evas_object_show(bg);
 * @endcode
 *
 * This however will have issues if the @c evas_canvas is resized, however most
 * windows are created using ecore evas and that has a solution to using the
 * rectangle as a background:
 * @code
 * Evas_Object *bg = evas_object_rectangle_add(ecore_evas_get(ee));
 * //Here we set the rectangles red, green, blue and opacity levels
 * evas_object_color_set(bg, 255, 255, 255, 255); // opaque white background
 * evas_object_resize(bg, WIDTH, HEIGHT); // covers full canvas
 * evas_object_show(bg);
 * ecore_evas_object_associate(ee, bg, ECORE_EVAS_OBJECT_ASSOCIATE_BASE);
 * @endcode
 * So this gives us a white background to our window that will be resized
 * together with it.
 *
 * @section Debugging
 *
 * Debugging is a major part of any programmers task and when debugging visual
 * issues with evas programs the rectangle is an extremely useful tool. The
 * rectangle's simplicity means that it's easier to pinpoint issues with it than
 * with more complex objects. Therefore a common technique to use when writing
 * an evas program and not getting the desired visual result is to replace the
 * misbehaving object for a solid color rectangle and seeing how it interacts
 * with the other elements, this often allows us to notice clipping, parenting
 * or positioning issues. Once the issues have been identified and corrected the
 * rectangle can be replaced for the original part and in all likelihood any
 * remaining issues will be specific to that object's type.
 *
 * @section clipping Clipping
 *
 * Clipping serves two main functions:
 * @li Limiting visibility(i.e. hiding portions of an object).
 * @li Applying a layer of color to an object.
 *
 * @subsection hiding Limiting visibility
 *
 * It is often necessary to show only parts of an object, while it may be
 * possible to create an object that corresponds only to the part that must be
 * shown(and it isn't always possible) it's usually easier to use a a clipper. A
 * clipper is a rectangle that defines what's visible and what is not. The way
 * to do this is to create a solid white rectangle(which is the default, no need
 * to call evas_object_color_set()) and give it a position and size of what
 * should be visible. The following code exemplifies showing the center half of
 * @c my_evas_object:
 * @code
 * Evas_Object *clipper = evas_object_rectangle_add(evas_canvas);
 * evas_object_move(clipper, my_evas_object_x / 4, my_evas_object_y / 4);
 * evas_object_resize(clipper, my_evas_object_width / 2, my_evas_object_height / 2);
 * evas_object_clip_set(my_evas_object, clipper);
 * evas_object_show(clipper);
 * @endcode
 *
 * @subsection color Layer of color
 *
 * In the @ref clipping section we used a solid white clipper, which produced no
 * change in the color of the clipped object, it just hid what was outside the
 * clippers area. It is however sometimes desirable to change the of color an
 * object, this can be accomplished using a clipper that has a non-white color.
 * Clippers with color work by multiplying the colors of clipped object. The
 * following code will show how to remove all the red from an object:
 * @code
 * Evas_Object *clipper = evas_object_rectangle_add(evas);
 * evas_object_move(clipper, my_evas_object_x, my_evas_object_y);
 * evas_object_resize(clipper, my_evas_object_width, my_evas_object_height);
 * evas_object_color_set(clipper, 0, 255, 255, 255);
 * evas_object_clip_set(obj, clipper);
 * evas_object_show(clipper);
 * @endcode
 *
 * @warning We don't guarantee any proper results if you create a Rectangle
 * object without setting the evas engine.
 *
 * For an example that more fully exercise the use of an evas object rectangle
 * see @ref Example_Evas_Object_Manipulation.
 *
 * @ingroup Evas_Object_Specific
 */

#define EVAS_OBJ_RECTANGLE_CLASS evas_object_rectangle_class_get()

const Eo_Class *evas_object_rectangle_class_get(void) EINA_CONST;

extern EAPI Eo_Op EVAS_OBJ_RECTANGLE_BASE_ID;

enum
{
   EVAS_OBJ_RECTANGLE_SUB_ID_LAST
};

#define EVAS_OBJ_RECTANGLE_ID(sub_id) (EVAS_OBJ_RECTANGLE_BASE_ID + sub_id)

/**
 * Adds a rectangle to the given evas.
 * @param   e The given evas.
 * @return  The new rectangle object.
 *
 * @ingroup Evas_Object_Rectangle
 */
EAPI Evas_Object *evas_object_rectangle_add(Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * @defgroup Evas_Object_Image Image Object Functions
 *
 * Here are grouped together functions used to create and manipulate
 * image objects. They are available to whichever occasion one needs
 * complex imagery on a GUI that could not be achieved by the other
 * Evas' primitive object types, or to make image manipulations.
 *
 * Evas will support whichever image file types it was compiled with
 * support to (its image loaders) -- check your software packager for
 * that information and see
 * evas_object_image_extension_can_load_get().
 *
 * @section Evas_Object_Image_Basics Image object basics
 *
 * The most common use of image objects -- to display an image on the
 * canvas -- is achieved by a common function triplet:
 * @code
 * img = evas_object_image_add(canvas);
 * evas_object_image_file_set(img, "path/to/img", NULL);
 * evas_object_image_fill_set(img, 0, 0, w, h);
 * @endcode
 * The first function, naturally, is creating the image object. Then,
 * one must set an source file on it, so that it knows where to fetch
 * image data from. Next, one must set <b>how to fill the image
 * object's area</b> with that given pixel data. One could use just a
 * sub-region of the original image or even have it tiled repeatedly
 * on the image object. For the common case of having the whole source
 * image to be displayed on the image object, stretched to the
 * destination's size, there's also a function helper, to be used
 * instead of evas_object_image_fill_set():
 * @code
 * evas_object_image_filled_set(img, EINA_TRUE);
 * @endcode
 * See those functions' documentation for more details.
 *
 * @section Evas_Object_Image_Scale Scale and resizing
 *
 * Resizing of image objects will scale their respective source images
 * to their areas, if they are set to "fill" the object's area
 * (evas_object_image_filled_set()). If the user wants any control on
 * the aspect ratio of an image for different sizes, he/she has to
 * take care of that themselves. There are functions to make images to
 * get loaded scaled (up or down) in memory, already, if the user is
 * going to use them at pre-determined sizes and wants to save
 * computations.
 *
 * Evas has even a scale cache, which will take care of caching scaled
 * versions of images with more often usage/hits. Finally, one can
 * have images being rescaled @b smoothly by Evas (more
 * computationally expensive) or not.
 *
 * @section Evas_Object_Image_Performance Performance hints
 *
 * When dealing with image objects, there are some tricks to boost the
 * performance of your application, if it does intense image loading
 * and/or manipulations, as in animations on a UI.
 *
 * @subsection Evas_Object_Image_Load Load hints
 *
 * In image viewer applications, for example, the user will be looking
 * at a given image, at full size, and will desire that the navigation
 * to the adjacent images on his/her album be fluid and fast. Thus,
 * while displaying a given image, the program can be on the
 * background loading the next and previous images already, so that
 * displaying them on the sequence is just a matter of repainting the
 * screen (and not decoding image data).
 *
 * Evas addresses this issue with <b>image pre-loading</b>. The code
 * for the situation above would be something like the following:
 * @code
 * prev = evas_object_image_filled_add(canvas);
 * evas_object_image_file_set(prev, "/path/to/prev", NULL);
 * evas_object_image_preload(prev, EINA_TRUE);
 *
 * next = evas_object_image_filled_add(canvas);
 * evas_object_image_file_set(next, "/path/to/next", NULL);
 * evas_object_image_preload(next, EINA_TRUE);
 * @endcode
 *
 * If you're loading images which are too big, consider setting
 * previously it's loading size to something smaller, in case you
 * won't expose them in real size. It may speed up the loading
 * considerably:
 * @code
 * //to load a scaled down version of the image in memory, if that's
 * //the size you'll be displaying it anyway
 * evas_object_image_load_scale_down_set(img, zoom);
 *
 * //optional: if you know you'll be showing a sub-set of the image's
 * //pixels, you can avoid loading the complementary data
 * evas_object_image_load_region_set(img, x, y, w, h);
 * @endcode
 * Refer to Elementary's Photocam widget for a high level (smart)
 * object which does lots of loading speed-ups for you.
 *
 * @subsection Evas_Object_Image_Animation Animation hints
 *
 * If you want to animate image objects on a UI (what you'd get by
 * concomitant usage of other libraries, like Ecore and Edje), there
 * are also some tips on how to boost the performance of your
 * application. If the animation involves resizing of an image (thus,
 * re-scaling), you'd better turn off smooth scaling on it @b during
 * the animation, turning it back on afterwards, for less
 * computations. Also, in this case you'd better flag the image object
 * in question not to cache scaled versions of it:
 * @code
 * evas_object_image_scale_hint_set(wd->img, EVAS_IMAGE_SCALE_HINT_DYNAMIC);
 *
 * // resizing takes place in between
 *
 * evas_object_image_scale_hint_set(wd->img, EVAS_IMAGE_SCALE_HINT_STATIC);
 * @endcode
 *
 * Finally, movement of opaque images through the canvas is less
 * expensive than of translucid ones, because of blending
 * computations.
 *
 * @section Evas_Object_Image_Borders Borders
 *
 * Evas provides facilities for one to specify an image's region to be
 * treated specially -- as "borders". This will make those regions be
 * treated specially on resizing scales, by keeping their aspect. This
 * makes setting frames around other objects on UIs easy.
 * See the following figures for a visual explanation:\n
 * @htmlonly
 * <img src="image-borders.png" style="max-width: 100%;" />
 * <a href="image-borders.png">Full-size</a>
 * @endhtmlonly
 * @image rtf image-borders.png
 * @image latex image-borders.eps width=\textwidth
 * @htmlonly
 * <img src="border-effect.png" style="max-width: 100%;" />
 * <a href="border-effect.png">Full-size</a>
 * @endhtmlonly
 * @image rtf border-effect.png
 * @image latex border-effect.eps width=\textwidth
 *
 * @section Evas_Object_Image_Manipulation Manipulating pixels
 *
 * Evas image objects can be used to manipulate raw pixels in many
 * ways.  The meaning of the data in the pixel arrays will depend on
 * the image's color space, be warned (see next section). You can set
 * your own data as an image's pixel data, fetch an image's pixel data
 * for saving/altering, convert images between different color spaces
 * and even advanced operations like setting a native surface as image
 * objects' data.
 *
 * @section Evas_Object_Image_Color_Spaces Color spaces
 *
 * Image objects may return or accept "image data" in multiple
 * formats. This is based on the color space of an object. Here is a
 * rundown on formats:
 *
 * - #EVAS_COLORSPACE_ARGB8888:
 *   This pixel format is a linear block of pixels, starting at the
 *   top-left row by row until the bottom right of the image or pixel
 *   region. All pixels are 32-bit unsigned int's with the high-byte
 *   being alpha and the low byte being blue in the format ARGB. Alpha
 *   may or may not be used by evas depending on the alpha flag of the
 *   image, but if not used, should be set to 0xff anyway.
 *   \n\n
 *   This colorspace uses premultiplied alpha. That means that R, G
 *   and B cannot exceed A in value. The conversion from
 *   non-premultiplied colorspace is:
 *   \n\n
 *   R = (r * a) / 255; G = (g * a) / 255; B = (b * a) / 255;
 *   \n\n
 *   So 50% transparent blue will be: 0x80000080. This will not be
 *   "dark" - just 50% transparent. Values are 0 == black, 255 ==
 *   solid or full red, green or blue.
 * .
 * - #EVAS_COLORSPACE_YCBCR422P601_PL:
 *   This is a pointer-list indirected set of YUV (YCbCr) pixel
 *   data. This means that the data returned or set is not actual
 *   pixel data, but pointers TO lines of pixel data. The list of
 *   pointers will first be N rows of pointers to the Y plane -
 *   pointing to the first pixel at the start of each row in the Y
 *   plane. N is the height of the image data in pixels. Each pixel in
 *   the Y, U and V planes is 1 byte exactly, packed. The next N / 2
 *   pointers will point to rows in the U plane, and the next N / 2
 *   pointers will point to the V plane rows. U and V planes are half
 *   the horizontal and vertical resolution of the Y plane.
 *   \n\n
 *   Row order is top to bottom and row pixels are stored left to
 *   right.
 *   \n\n
 *   There is a limitation that these images MUST be a multiple of 2
 *   pixels in size horizontally or vertically. This is due to the U
 *   and V planes being half resolution. Also note that this assumes
 *   the itu601 YUV colorspace specification. This is defined for
 *   standard television and mpeg streams. HDTV may use the itu709
 *   specification.
 *   \n\n
 *   Values are 0 to 255, indicating full or no signal in that plane
 *   respectively.
 * .
 * - #EVAS_COLORSPACE_YCBCR422P709_PL:
 *   Not implemented yet.
 * .
 * - #EVAS_COLORSPACE_RGB565_A5P:
 *   In the process of being implemented in 1 engine only. This may
 *   change.
 *   \n\n
 *   This is a pointer to image data for 16-bit half-word pixel data
 *   in 16bpp RGB 565 format (5 bits red, 6 bits green, 5 bits blue),
 *   with the high-byte containing red and the low byte containing
 *   blue, per pixel. This data is packed row by row from the top-left
 *   to the bottom right.
 *   \n\n
 *   If the image has an alpha channel enabled there will be an extra
 *   alpha plane after the color pixel plane. If not, then this data
 *   will not exist and should not be accessed in any way. This plane
 *   is a set of pixels with 1 byte per pixel defining the alpha
 *   values of all pixels in the image from the top-left to the bottom
 *   right of the image, row by row. Even though the values of the
 *   alpha pixels can be 0 to 255, only values 0 through to 32 are
 *   used, 32 being solid and 0 being transparent.
 *   \n\n
 *   RGB values can be 0 to 31 for red and blue and 0 to 63 for green,
 *   with 0 being black and 31 or 63 being full red, green or blue
 *   respectively. This colorspace is also pre-multiplied like
 *   EVAS_COLORSPACE_ARGB8888 so:
 *   \n\n
 *   R = (r * a) / 32; G = (g * a) / 32; B = (b * a) / 32;
 * .
 * - #EVAS_COLORSPACE_GRY8:
 *   The image is just a alpha mask (8 bit's per pixel). This is used
 *   for alpha masking.
 *
 * @warning We don't guarantee any proper results if you create a Image object
 * without setting the evas engine.
 *
 * Some examples on this group of functions can be found @ref
 * Example_Evas_Images "here".
 *
 * @ingroup Evas_Object_Specific
 */

/**
 * @addtogroup Evas_Object_Image
 * @{
 */

typedef void (*Evas_Object_Image_Pixels_Get_Cb)(void *data, Evas_Object *o);

/**
 * Creates a new image object on the given Evas @p e canvas.
 *
 * @param e The given canvas.
 * @return The created image object handle.
 *
 * @note If you intend to @b display an image somehow in a GUI,
 * besides binding it to a real image file/source (with
 * evas_object_image_file_set(), for example), you'll have to tell
 * this image object how to fill its space with the pixels it can get
 * from the source. See evas_object_image_filled_add(), for a helper
 * on the common case of scaling up an image source to the whole area
 * of the image object.
 *
 * @see evas_object_image_fill_set()
 *
 * Example:
 * @code
 * img = evas_object_image_add(canvas);
 * evas_object_image_file_set(img, "/path/to/img", NULL);
 * @endcode
 */
EAPI Evas_Object                  *evas_object_image_add(Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * Creates a new image object that @b automatically scales its bound
 * image to the object's area, on both axis.
 *
 * @param e The given canvas.
 * @return The created image object handle.
 *
 * This is a helper function around evas_object_image_add() and
 * evas_object_image_filled_set(). It has the same effect of applying
 * those functions in sequence, which is a very common use case.
 *
 * @note Whenever this object gets resized, the bound image will be
 * rescaled, too.
 *
 * @see evas_object_image_add()
 * @see evas_object_image_filled_set()
 * @see evas_object_image_fill_set()
 */
EAPI Evas_Object                  *evas_object_image_filled_add(Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * Sets the data for an image from memory to be loaded
 *
 * This is the same as evas_object_image_file_set() but the file to be loaded
 * may exist at an address in memory (the data for the file, not the filename
 * itself). The @p data at the address is copied and stored for future use, so
 * no @p data needs to be kept after this call is made. It will be managed and
 * freed for you when no longer needed. The @p size is limited to 2 gigabytes
 * in size, and must be greater than 0. A @c NULL @p data pointer is also
 * invalid. Set the filename to @c NULL to reset to empty state and have the
 * image file data freed from memory using evas_object_image_file_set().
 *
 * The @p format is optional (pass @c NULL if you don't need/use it). It is
 * used to help Evas guess better which loader to use for the data. It may
 * simply be the "extension" of the file as it would normally be on disk
 * such as "jpg" or "png" or "gif" etc.
 *
 * @param obj The given image object.
 * @param data The image file data address
 * @param size The size of the image file data in bytes
 * @param format The format of the file (optional), or @c NULL if not needed
 * @param key The image key in file, or @c NULL.
 */
EAPI void                          evas_object_image_memfile_set(Evas_Object *obj, void *data, int size, char *format, char *key) EINA_ARG_NONNULL(1, 2);

/**
 * Set the source file from where an image object must fetch the real
 * image data (it may be an Eet file, besides pure image ones).
 *
 * @param obj The given image object.
 * @param file The image file path.
 * @param key The image key in @p file (if its an Eet one), or @c
 * NULL, otherwise.
 *
 * If the file supports multiple data stored in it (as Eet files do),
 * you can specify the key to be used as the index of the image in
 * this file.
 *
 * Example:
 * @code
 * img = evas_object_image_add(canvas);
 * evas_object_image_file_set(img, "/path/to/img", NULL);
 * err = evas_object_image_load_error_get(img);
 * if (err != EVAS_LOAD_ERROR_NONE)
 *   {
 *      fprintf(stderr, "could not load image '%s'. error string is \"%s\"\n",
 *              valid_path, evas_load_error_str(err));
 *   }
 * else
 *   {
 *      evas_object_image_fill_set(img, 0, 0, w, h);
 *      evas_object_resize(img, w, h);
 *      evas_object_show(img);
 *   }
 * @endcode
 */
EAPI void                          evas_object_image_file_set(Evas_Object *obj, const char *file, const char *key) EINA_ARG_NONNULL(1);

/**
 * Retrieve the source file from where an image object is to fetch the
 * real image data (it may be an Eet file, besides pure image ones).
 *
 * @param obj The given image object.
 * @param file Location to store the image file path.
 * @param key Location to store the image key (if @p file is an Eet
 * one).
 *
 * You must @b not modify the strings on the returned pointers.
 *
 * @note Use @c NULL pointers on the file components you're not
 * interested in: they'll be ignored by the function.
 */
EAPI void                          evas_object_image_file_get(const Evas_Object *obj, const char **file, const char **key) EINA_ARG_NONNULL(1, 2);

/**
 * Set the dimensions for an image object's border, a region which @b
 * won't ever be scaled together with its center.
 *
 * @param obj The given image object.
 * @param l The border's left width.
 * @param r The border's right width.
 * @param t The border's top width.
 * @param b The border's bottom width.
 *
 * When Evas is rendering, an image source may be scaled to fit the
 * size of its image object. This function sets an area from the
 * borders of the image inwards which is @b not to be scaled. This
 * function is useful for making frames and for widget theming, where,
 * for example, buttons may be of varying sizes, but their border size
 * must remain constant.
 *
 * The units used for @p l, @p r, @p t and @p b are canvas units.
 *
 * @note The border region itself @b may be scaled by the
 * evas_object_image_border_scale_set() function.
 *
 * @note By default, image objects have no borders set, i. e. @c l, @c
 * r, @c t and @c b start as @c 0.
 *
 * See the following figures for visual explanation:\n
 * @htmlonly
 * <img src="image-borders.png" style="max-width: 100%;" />
 * <a href="image-borders.png">Full-size</a>
 * @endhtmlonly
 * @image rtf image-borders.png
 * @image latex image-borders.eps width=\textwidth
 * @htmlonly
 * <img src="border-effect.png" style="max-width: 100%;" />
 * <a href="border-effect.png">Full-size</a>
 * @endhtmlonly
 * @image rtf border-effect.png
 * @image latex border-effect.eps width=\textwidth
 *
 * @see evas_object_image_border_get()
 * @see evas_object_image_border_center_fill_set()
 */
EAPI void                          evas_object_image_border_set(Evas_Object *obj, int l, int r, int t, int b) EINA_ARG_NONNULL(1);

/**
 * Retrieve the dimensions for an image object's border, a region
 * which @b won't ever be scaled together with its center.
 *
 * @param obj The given image object.
 * @param l Location to store the border's left width in.
 * @param r Location to store the border's right width in.
 * @param t Location to store the border's top width in.
 * @param b Location to store the border's bottom width in.
 *
 * @note Use @c NULL pointers on the border components you're not
 * interested in: they'll be ignored by the function.
 *
 * See @ref evas_object_image_border_set() for more details.
 */
EAPI void                          evas_object_image_border_get(const Evas_Object *obj, int *l, int *r, int *t, int *b) EINA_ARG_NONNULL(1);

/**
 * Sets @b how the center part of the given image object (not the
 * borders) should be drawn when Evas is rendering it.
 *
 * @param obj The given image object.
 * @param fill Fill mode of the center region of @p obj (a value in
 * #Evas_Border_Fill_Mode).
 *
 * This function sets how the center part of the image object's source
 * image is to be drawn, which must be one of the values in
 * #Evas_Border_Fill_Mode. By center we mean the complementary part of
 * that defined by evas_object_image_border_set(). This one is very
 * useful for making frames and decorations. You would most probably
 * also be using a filled image (as in evas_object_image_filled_set())
 * to use as a frame.
 *
 * @see evas_object_image_border_center_fill_get()
 */
EAPI void                          evas_object_image_border_center_fill_set(Evas_Object *obj, Evas_Border_Fill_Mode fill) EINA_ARG_NONNULL(1);

/**
 * Retrieves @b how the center part of the given image object (not the
 * borders) is to be drawn when Evas is rendering it.
 *
 * @param obj The given image object.
 * @return fill Fill mode of the center region of @p obj (a value in
 * #Evas_Border_Fill_Mode).
 *
 * See @ref evas_object_image_fill_set() for more details.
 */
EAPI Evas_Border_Fill_Mode         evas_object_image_border_center_fill_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Set whether the image object's fill property should track the
 * object's size.
 *
 * @param obj The given image object.
 * @param setting @c EINA_TRUE, to make the fill property follow
 *        object size or @c EINA_FALSE, otherwise.
 *
 * If @p setting is @c EINA_TRUE, then every evas_object_resize() will
 * @b automatically trigger a call to evas_object_image_fill_set()
 * with the that new size (and @c 0, @c 0 as source image's origin),
 * so the bound image will fill the whole object's area.
 *
 * @see evas_object_image_filled_add()
 * @see evas_object_image_fill_get()
 */
EAPI void                          evas_object_image_filled_set(Evas_Object *obj, Eina_Bool setting) EINA_ARG_NONNULL(1);

/**
 * Retrieve whether the image object's fill property should track the
 * object's size.
 *
 * @param obj The given image object.
 * @return @c EINA_TRUE if it is tracking, @c EINA_FALSE, if not (and
 *         evas_object_fill_set() must be called manually).
 *
 * @see evas_object_image_filled_set() for more information
 */
EAPI Eina_Bool                     evas_object_image_filled_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Sets the scaling factor (multiplier) for the borders of an image
 * object.
 *
 * @param obj The given image object.
 * @param scale The scale factor (default is @c 1.0 - i.e. no scaling)
 *
 * @see evas_object_image_border_set()
 * @see evas_object_image_border_scale_get()
 */
EAPI void                          evas_object_image_border_scale_set(Evas_Object *obj, double scale);

/**
 * Retrieves the scaling factor (multiplier) for the borders of an
 * image object.
 *
 * @param obj The given image object.
 * @return The scale factor set for its borders
 *
 * @see evas_object_image_border_set()
 * @see evas_object_image_border_scale_set()
 */
EAPI double                        evas_object_image_border_scale_get(const Evas_Object *obj);

/**
 * Set how to fill an image object's drawing rectangle given the
 * (real) image bound to it.
 *
 * @param obj The given image object to operate on.
 * @param x The x coordinate (from the top left corner of the bound
 *          image) to start drawing from.
 * @param y The y coordinate (from the top left corner of the bound
 *          image) to start drawing from.
 * @param w The width the bound image will be displayed at.
 * @param h The height the bound image will be displayed at.
 *
 * Note that if @p w or @p h are smaller than the dimensions of
 * @p obj, the displayed image will be @b tiled around the object's
 * area. To have only one copy of the bound image drawn, @p x and @p y
 * must be 0 and @p w and @p h need to be the exact width and height
 * of the image object itself, respectively.
 *
 * See the following image to better understand the effects of this
 * call. On this diagram, both image object and original image source
 * have @c a x @c a dimensions and the image itself is a circle, with
 * empty space around it:
 *
 * @image html image-fill.png
 * @image rtf image-fill.png
 * @image latex image-fill.eps
 *
 * @warning The default values for the fill parameters are @p x = 0,
 * @p y = 0, @p w = 0 and @p h = 0. Thus, if you're not using the
 * evas_object_image_filled_add() helper and want your image
 * displayed, you'll have to set valid values with this function on
 * your object.
 *
 * @note evas_object_image_filled_set() is a helper function which
 * will @b override the values set here automatically, for you, in a
 * given way.
 */
EAPI void                          evas_object_image_fill_set(Evas_Object *obj, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h) EINA_ARG_NONNULL(1);

/**
 * Retrieve how an image object is to fill its drawing rectangle,
 * given the (real) image bound to it.
 *
 * @param obj The given image object.
 * @param x Location to store the x coordinate (from the top left
 *          corner of the bound image) to start drawing from.
 * @param y Location to store the y coordinate (from the top left
 *          corner of the bound image) to start drawing from.
 * @param w Location to store the width the bound image is to be
 *          displayed at.
 * @param h Location to store the height the bound image is to be
 *          displayed at.
 *
 * @note Use @c NULL pointers on the fill components you're not
 * interested in: they'll be ignored by the function.
 *
 * See @ref evas_object_image_fill_set() for more details.
 */
EAPI void                          evas_object_image_fill_get(const Evas_Object *obj, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h) EINA_ARG_NONNULL(1);

/**
 * Sets the tiling mode for the given evas image object's fill.
 * @param   obj   The given evas image object.
 * @param   spread One of EVAS_TEXTURE_REFLECT, EVAS_TEXTURE_REPEAT,
 * EVAS_TEXTURE_RESTRICT, or EVAS_TEXTURE_PAD.
 */
EAPI void                          evas_object_image_fill_spread_set(Evas_Object *obj, Evas_Fill_Spread spread) EINA_ARG_NONNULL(1);

/**
 * Retrieves the spread (tiling mode) for the given image object's
 * fill.
 *
 * @param   obj The given evas image object.
 * @return  The current spread mode of the image object.
 */
EAPI Evas_Fill_Spread              evas_object_image_fill_spread_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Sets the size of the given image object.
 *
 * @param obj The given image object.
 * @param w The new width of the image.
 * @param h The new height of the image.
 *
 * This function will scale down or crop the image so that it is
 * treated as if it were at the given size. If the size given is
 * smaller than the image, it will be cropped. If the size given is
 * larger, then the image will be treated as if it were in the upper
 * left hand corner of a larger image that is otherwise transparent.
 */
EAPI void                          evas_object_image_size_set(Evas_Object *obj, int w, int h) EINA_ARG_NONNULL(1);

/**
 * Retrieves the size of the given image object.
 *
 * @param obj The given image object.
 * @param w Location to store the width of the image in, or @c NULL.
 * @param h Location to store the height of the image in, or @c NULL.
 *
 * See @ref evas_object_image_size_set() for more details.
 */
EAPI void                          evas_object_image_size_get(const Evas_Object *obj, int *w, int *h) EINA_ARG_NONNULL(1);

/**
 * Retrieves the row stride of the given image object.
 *
 * @param obj The given image object.
 * @return The stride of the image (<b>in bytes</b>).
 *
 * The row stride is the number of bytes between the start of a row
 * and the start of the next row for image data.
 */
EAPI int                           evas_object_image_stride_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Retrieves a number representing any error that occurred during the
 * last loading of the given image object's source image.
 *
 * @param obj The given image object.
 * @return A value giving the last error that occurred. It should be
 *         one of the #Evas_Load_Error values. #EVAS_LOAD_ERROR_NONE
 *         is returned if there was no error.
 */
EAPI Evas_Load_Error               evas_object_image_load_error_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Sets the raw image data of the given image object.
 *
 * @param obj The given image object.
 * @param data The raw data, or @c NULL.
 *
 * Note that the raw data must be of the same size (see
 * evas_object_image_size_set(), which has to be called @b before this
 * one) and colorspace (see evas_object_image_colorspace_set()) of the
 * image. If data is @c NULL, the current image data will be
 * freed. Naturally, if one does not set an image object's data
 * manually, it will still have one, allocated by Evas.
 *
 * @see evas_object_image_data_get()
 */
EAPI void                          evas_object_image_data_set(Evas_Object *obj, void *data) EINA_ARG_NONNULL(1);

/**
 * Get a pointer to the raw image data of the given image object.
 *
 * @param obj The given image object.
 * @param for_writing Whether the data being retrieved will be
 *        modified (@c EINA_TRUE) or not (@c EINA_FALSE).
 * @return The raw image data.
 *
 * This function returns a pointer to an image object's internal pixel
 * buffer, for reading only or read/write. If you request it for
 * writing, the image will be marked dirty so that it gets redrawn at
 * the next update.
 *
 * Each time you call this function on an image object, its data
 * buffer will have an internal reference counter
 * incremented. Decrement it back by using
 * evas_object_image_data_set(). This is specially important for the
 * directfb Evas engine.
 *
 * This is best suited for when you want to modify an existing image,
 * without changing its dimensions.
 *
 * @note The contents' format returned by it depend on the color
 * space of the given image object.
 *
 * @note You may want to use evas_object_image_data_update_add() to
 * inform data changes, if you did any.
 *
 * @see evas_object_image_data_set()
 */
EAPI void                         *evas_object_image_data_get(const Evas_Object *obj, Eina_Bool for_writing) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Converts the raw image data of the given image object to the
 * specified colorspace.
 *
 * Note that this function does not modify the raw image data.  If the
 * requested colorspace is the same as the image colorspace nothing is
 * done and @c NULL is returned. You should use
 * evas_object_image_colorspace_get() to check the current image
 * colorspace.
 *
 * See @ref evas_object_image_colorspace_get.
 *
 * @param obj The given image object.
 * @param to_cspace The colorspace to which the image raw data will be converted.
 * @return data A newly allocated data in the format specified by to_cspace.
 */
EAPI void                         *evas_object_image_data_convert(Evas_Object *obj, Evas_Colorspace to_cspace) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Replaces the raw image data of the given image object.
 *
 * @param obj The given image object.
 * @param data The raw data to replace.
 *
 * This function lets the application replace an image object's
 * internal pixel buffer with an user-allocated one. For best results,
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
 * See @ref evas_object_image_data_get() for more details.
 *
 */
EAPI void                          evas_object_image_data_copy_set(Evas_Object *obj, void *data) EINA_ARG_NONNULL(1);

/**
 * Mark a sub-region of the given image object to be redrawn.
 *
 * @param obj The given image object.
 * @param x X-offset of the region to be updated.
 * @param y Y-offset of the region to be updated.
 * @param w Width of the region to be updated.
 * @param h Height of the region to be updated.
 *
 * This function schedules a particular rectangular region of an image
 * object to be updated (redrawn) at the next rendering cycle.
 */
EAPI void                          evas_object_image_data_update_add(Evas_Object *obj, int x, int y, int w, int h) EINA_ARG_NONNULL(1);

/**
 * Enable or disable alpha channel usage on the given image object.
 *
 * @param obj The given image object.
 * @param has_alpha Whether to use alpha channel (@c EINA_TRUE) data
 * or not (@c EINA_FALSE).
 *
 * This function sets a flag on an image object indicating whether or
 * not to use alpha channel data. A value of @c EINA_TRUE makes it use
 * alpha channel data, and @c EINA_FALSE makes it ignore that
 * data. Note that this has nothing to do with an object's color as
 * manipulated by evas_object_color_set().
 *
 * @see evas_object_image_alpha_get()
 */
EAPI void                          evas_object_image_alpha_set(Evas_Object *obj, Eina_Bool has_alpha) EINA_ARG_NONNULL(1);

/**
 * Retrieve whether alpha channel data is being used on the given
 * image object.
 *
 * @param obj The given image object.
 * @return Whether the alpha channel data is being used (@c EINA_TRUE)
 * or not (@c EINA_FALSE).
 *
 * This function returns @c EINA_TRUE if the image object's alpha
 * channel is being used, or @c EINA_FALSE otherwise.
 *
 * See @ref evas_object_image_alpha_set() for more details.
 */
EAPI Eina_Bool                     evas_object_image_alpha_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Sets whether to use high-quality image scaling algorithm on the
 * given image object.
 *
 * @param obj The given image object.
 * @param smooth_scale Whether to use smooth scale or not.
 *
 * When enabled, a higher quality image scaling algorithm is used when
 * scaling images to sizes other than the source image's original
 * one. This gives better results but is more computationally
 * expensive.
 *
 * @note Image objects get created originally with smooth scaling @b
 * on.
 *
 * @see evas_object_image_smooth_scale_get()
 */
EAPI void                          evas_object_image_smooth_scale_set(Evas_Object *obj, Eina_Bool smooth_scale) EINA_ARG_NONNULL(1);

/**
 * Retrieves whether the given image object is using high-quality
 * image scaling algorithm.
 *
 * @param obj The given image object.
 * @return Whether smooth scale is being used.
 *
 * See @ref evas_object_image_smooth_scale_set() for more details.
 */
EAPI Eina_Bool                     evas_object_image_smooth_scale_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Preload an image object's image data in the background
 *
 * @param obj The given image object.
 * @param cancel @c EINA_FALSE will add it the preloading work queue,
 *               @c EINA_TRUE will remove it (if it was issued before).
 *
 * This function requests the preload of the data image in the
 * background. The work is queued before being processed (because
 * there might be other pending requests of this type).
 *
 * Whenever the image data gets loaded, Evas will call
 * #EVAS_CALLBACK_IMAGE_PRELOADED registered callbacks on @p obj (what
 * may be immediately, if the data was already preloaded before).
 *
 * Use @c EINA_TRUE for @p cancel on scenarios where you don't need
 * the image data preloaded anymore.
 *
 * @note Any evas_object_show() call after evas_object_image_preload()
 * will make the latter to be @b cancelled, with the loading process
 * now taking place @b synchronously (and, thus, blocking the return
 * of the former until the image is loaded). It is highly advisable,
 * then, that the user preload an image with it being @b hidden, just
 * to be shown on the #EVAS_CALLBACK_IMAGE_PRELOADED event's callback.
 */
EAPI void                          evas_object_image_preload(Evas_Object *obj, Eina_Bool cancel) EINA_ARG_NONNULL(1);

/**
 * Reload an image object's image data.
 *
 * @param obj The given image object pointer.
 *
 * This function reloads the image data bound to image object @p obj.
 */
EAPI void                          evas_object_image_reload(Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * Save the given image object's contents to an (image) file.
 *
 * @param obj The given image object.
 * @param file The filename to be used to save the image (extension
 *        obligatory).
 * @param key The image key in the file (if an Eet one), or @c NULL,
 *        otherwise.
 * @param flags String containing the flags to be used (@c NULL for
 *        none).
 *
 * The extension suffix on @p file will determine which <b>saver
 * module</b> Evas is to use when saving, thus the final file's
 * format. If the file supports multiple data stored in it (Eet ones),
 * you can specify the key to be used as the index of the image in it.
 *
 * You can specify some flags when saving the image.  Currently
 * acceptable flags are @c quality and @c compress. Eg.: @c
 * "quality=100 compress=9"
 */
EAPI Eina_Bool                     evas_object_image_save(const Evas_Object *obj, const char *file, const char *key, const char *flags)  EINA_ARG_NONNULL(1, 2);

/**
 * Import pixels from given source to a given canvas image object.
 *
 * @param obj The given canvas object.
 * @param pixels The pixel's source to be imported.
 *
 * This function imports pixels from a given source to a given canvas image.
 *
 */
EAPI Eina_Bool                     evas_object_image_pixels_import(Evas_Object *obj, Evas_Pixel_Import_Source *pixels) EINA_ARG_NONNULL(1, 2);

/**
 * Set the callback function to get pixels from a canvas' image.
 *
 * @param obj The given canvas pointer.
 * @param func The callback function.
 * @param data The data pointer to be passed to @a func.
 *
 * This functions sets a function to be the callback function that get
 * pixes from a image of the canvas.
 *
 */
EAPI void                          evas_object_image_pixels_get_callback_set(Evas_Object *obj, Evas_Object_Image_Pixels_Get_Cb func, void *data) EINA_ARG_NONNULL(1, 2);

/**
 * Mark whether the given image object is dirty and needs to request its pixels.
 *
 * @param obj The given image object.
 * @param dirty Whether the image is dirty.
 *
 * This function will only properly work if a pixels get callback has been set.
 *
 * @warning use this function if you really know what you are doing.
 *
 * @see evas_object_image_pixels_get_callback_set()
 */
EAPI void                          evas_object_image_pixels_dirty_set(Evas_Object *obj, Eina_Bool dirty) EINA_ARG_NONNULL(1);

/**
 * Retrieves whether the given image object is dirty (needs to be redrawn).
 *
 * @param obj The given image object.
 * @return Whether the image is dirty.
 */
EAPI Eina_Bool                     evas_object_image_pixels_dirty_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Set the DPI resolution of an image object's source image.
 *
 * @param obj The given canvas pointer.
 * @param dpi The new DPI resolution.
 *
 * This function sets the DPI resolution of a given loaded canvas
 * image. Most useful for the SVG image loader.
 *
 * @see evas_object_image_load_dpi_get()
 */
EAPI void                          evas_object_image_load_dpi_set(Evas_Object *obj, double dpi) EINA_ARG_NONNULL(1);

/**
 * Get the DPI resolution of a loaded image object in the canvas.
 *
 * @param obj The given canvas pointer.
 * @return The DPI resolution of the given canvas image.
 *
 * This function returns the DPI resolution of the given canvas image.
 *
 * @see evas_object_image_load_dpi_set() for more details
 */
EAPI double                        evas_object_image_load_dpi_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Set the size of a given image object's source image, when loading
 * it.
 *
 * @param obj The given canvas object.
 * @param w The new width of the image's load size.
 * @param h The new height of the image's load size.
 *
 * This function sets a new (loading) size for the given canvas
 * image.
 *
 * @see evas_object_image_load_size_get()
 */
EAPI void                          evas_object_image_load_size_set(Evas_Object *obj, int w, int h) EINA_ARG_NONNULL(1);

/**
 * Get the size of a given image object's source image, when loading
 * it.
 *
 * @param obj The given image object.
 * @param w Where to store the new width of the image's load size.
 * @param h Where to store the new height of the image's load size.
 *
 * @note Use @c NULL pointers on the size components you're not
 * interested in: they'll be ignored by the function.
 *
 * @see evas_object_image_load_size_set() for more details
 */
EAPI void                          evas_object_image_load_size_get(const Evas_Object *obj, int *w, int *h) EINA_ARG_NONNULL(1);

/**
 * Set the scale down factor of a given image object's source image,
 * when loading it.
 *
 * @param obj The given image object pointer.
 * @param scale_down The scale down factor.
 *
 * This function sets the scale down factor of a given canvas
 * image. Most useful for the SVG image loader.
 *
 * @see evas_object_image_load_scale_down_get()
 */
EAPI void                          evas_object_image_load_scale_down_set(Evas_Object *obj, int scale_down) EINA_ARG_NONNULL(1);

/**
 * get the scale down factor of a given image object's source image,
 * when loading it.
 *
 * @param obj The given image object pointer.
 *
 * @see evas_object_image_load_scale_down_set() for more details
 */
EAPI int                           evas_object_image_load_scale_down_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Inform a given image object to load a selective region of its
 * source image.
 *
 * @param obj The given image object pointer.
 * @param x X-offset of the region to be loaded.
 * @param y Y-offset of the region to be loaded.
 * @param w Width of the region to be loaded.
 * @param h Height of the region to be loaded.
 *
 * This function is useful when one is not showing all of an image's
 * area on its image object.
 *
 * @note The image loader for the image format in question has to
 * support selective region loading in order to this function to take
 * effect.
 *
 * @see evas_object_image_load_region_get()
 */
EAPI void                          evas_object_image_load_region_set(Evas_Object *obj, int x, int y, int w, int h) EINA_ARG_NONNULL(1);

/**
 * Retrieve the coordinates of a given image object's selective
 * (source image) load region.
 *
 * @param obj The given image object pointer.
 * @param x Where to store the X-offset of the region to be loaded.
 * @param y Where to store the Y-offset of the region to be loaded.
 * @param w Where to store the width of the region to be loaded.
 * @param h Where to store the height of the region to be loaded.
 *
 * @note Use @c NULL pointers on the coordinates you're not interested
 * in: they'll be ignored by the function.
 *
 * @see evas_object_image_load_region_get()
 */
EAPI void                          evas_object_image_load_region_get(const Evas_Object *obj, int *x, int *y, int *w, int *h) EINA_ARG_NONNULL(1);

/**
 * Define if the orientation information in the image file should be honored.
 *
 * @param obj The given image object pointer.
 * @param enable @c EINA_TRUE means that it should honor the orientation information
 * @since 1.1
 */
EAPI void                          evas_object_image_load_orientation_set(Evas_Object *obj, Eina_Bool enable) EINA_ARG_NONNULL(1);

/**
 * Get if the orientation information in the image file should be honored.
 *
 * @param obj The given image object pointer.
 * @since 1.1
 */
EAPI Eina_Bool                     evas_object_image_load_orientation_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * Set the colorspace of a given image of the canvas.
 *
 * @param obj The given image object pointer.
 * @param cspace The new color space.
 *
 * This function sets the colorspace of given canvas image.
 *
 */
EAPI void                          evas_object_image_colorspace_set(Evas_Object *obj, Evas_Colorspace cspace) EINA_ARG_NONNULL(1);

/**
 * Get the colorspace of a given image of the canvas.
 *
 * @param obj The given image object pointer.
 * @return The colorspace of the image.
 *
 * This function returns the colorspace of given canvas image.
 *
 */
EAPI Evas_Colorspace               evas_object_image_colorspace_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Get the support state of a given image
 *
 * @param obj The given image object pointer
 * @return The region support state
 * @since 1.2
 *
 * This function returns the state of the region support of given image
 */
EAPI Eina_Bool                     evas_object_image_region_support_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Set the native surface of a given image of the canvas
 *
 * @param obj The given canvas pointer.
 * @param surf The new native surface.
 *
 * This function sets a native surface of a given canvas image.
 *
 */
EAPI void                          evas_object_image_native_surface_set(Evas_Object *obj, Evas_Native_Surface *surf) EINA_ARG_NONNULL(1, 2);

/**
 * Get the native surface of a given image of the canvas
 *
 * @param obj The given canvas pointer.
 * @return The native surface of the given canvas image.
 *
 * This function returns the native surface of a given canvas image.
 *
 */
EAPI Evas_Native_Surface          *evas_object_image_native_surface_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Set the video surface linked to a given image of the canvas
 *
 * @param obj The given canvas pointer.
 * @param surf The new video surface.
 * @since 1.1
 *
 * This function link a video surface to a given canvas image.
 *
 */
EAPI void                          evas_object_image_video_surface_set(Evas_Object *obj, Evas_Video_Surface *surf) EINA_ARG_NONNULL(1);

/**
 * Get the video surface linekd to a given image of the canvas
 *
 * @param obj The given canvas pointer.
 * @return The video surface of the given canvas image.
 * @since 1.1
 *
 * This function returns the video surface linked to a given canvas image.
 *
 */
EAPI const Evas_Video_Surface     *evas_object_image_video_surface_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Set the scale hint of a given image of the canvas.
 *
 * @param obj The given image object pointer.
 * @param hint The scale hint, a value in
 * #Evas_Image_Scale_Hint.
 *
 * This function sets the scale hint value of the given image object
 * in the canvas, which will affect how Evas is to cache scaled
 * versions of its original source image.
 *
 * @see evas_object_image_scale_hint_get()
 */
EAPI void                          evas_object_image_scale_hint_set(Evas_Object *obj, Evas_Image_Scale_Hint hint) EINA_ARG_NONNULL(1);

/**
 * Get the scale hint of a given image of the canvas.
 *
 * @param obj The given image object pointer.
 * @return The scale hint value set on @p obj, a value in
 * #Evas_Image_Scale_Hint.
 *
 * This function returns the scale hint value of the given image
 * object of the canvas.
 *
 * @see evas_object_image_scale_hint_set() for more details.
 */
EAPI Evas_Image_Scale_Hint         evas_object_image_scale_hint_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Set the content hint setting of a given image object of the canvas.
 *
 * @param obj The given canvas pointer.
 * @param hint The content hint value, one of the
 * #Evas_Image_Content_Hint ones.
 *
 * This function sets the content hint value of the given image of the
 * canvas. For example, if you're on the GL engine and your driver
 * implementation supports it, setting this hint to
 * #EVAS_IMAGE_CONTENT_HINT_DYNAMIC will make it need @b zero copies
 * at texture upload time, which is an "expensive" operation.
 *
 * @see evas_object_image_content_hint_get()
 */
EAPI void                          evas_object_image_content_hint_set(Evas_Object *obj, Evas_Image_Content_Hint hint) EINA_ARG_NONNULL(1);

/**
 * Get the content hint setting of a given image object of the canvas.
 *
 * @param obj The given canvas pointer.
 * @return hint The content hint value set on it, one of the
 * #Evas_Image_Content_Hint ones (#EVAS_IMAGE_CONTENT_HINT_NONE means
 * an error).
 *
 * This function returns the content hint value of the given image of
 * the canvas.
 *
 * @see evas_object_image_content_hint_set()
 */
EAPI Evas_Image_Content_Hint       evas_object_image_content_hint_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

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
EAPI void                          evas_object_image_alpha_mask_set(Evas_Object *obj, Eina_Bool ismask) EINA_ARG_NONNULL(1);

/**
 * Set the source object on an image object to used as a @b proxy.
 *
 * @param obj Proxy (image) object.
 * @param src Source object to use for the proxy.
 * @return @c EINA_TRUE on success, @c EINA_FALSE on error.
 *
 * If an image object is set to behave as a @b proxy, it will mirror
 * the rendering contents of a given @b source object in its drawing
 * region, without affecting that source in any way. The source must
 * be another valid Evas object. Other effects may be applied to the
 * proxy, such as a map (see evas_object_map_set()) to create a
 * reflection of the original object (for example).
 *
 * Any existing source object on @p obj will be removed after this
 * call. Setting @p src to @c NULL clears the proxy object (not in
 * "proxy state" anymore).
 *
 * @warning You cannot set a proxy as another proxy's source.
 *
 * @see evas_object_image_source_get()
 * @see evas_object_image_source_unset()
 * @see evas_object_image_source_visible_set()
 */
EAPI Eina_Bool                     evas_object_image_source_set(Evas_Object *obj, Evas_Object *src) EINA_ARG_NONNULL(1);

/**
 * Get the current source object of an image object.
 *
 * @param obj Image object
 * @return Source object (if any), or @c NULL, if not in "proxy mode"
 * (or on errors).
 *
 * @see evas_object_image_source_set() for more details
 */
EAPI Evas_Object                  *evas_object_image_source_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * Clear the source object on a proxy image object.
 *
 * @param obj Image object to clear source of.
 * @return @c EINA_TRUE on success, @c EINA_FALSE on error.
 *
 * This is equivalent to calling evas_object_image_source_set() with a
 * @c NULL source.
 */
EAPI Eina_Bool                     evas_object_image_source_unset(Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * Set the source object to be shown or hidden.
 *
 * @param obj Proxy (image) object.
 * @param visible @c EINA_TRUE is source object to be shown, @c EINA_FALSE 
 * otherwise.
 *
 * If the @p visible set to @c EINA_FALSE, the source object of the proxy(@p obj
 * ) will be hidden.
 *
 * This API works differently with evas_object_show() and evas_object_hide().
 * Once source object is hidden by evas_object_hide() then the proxy object will * be hidden also. Actually in this case both objects are excluded from the
 * Evas internal updation. By this API, instead, you can set only proxy object
 * to be shown. And even if source object is invisible, source object can be 
 *updated for proxy.
 *
 * @see evas_object_image_source_visible_get()
 * @see evas_object_image_source_set()
 * @see evas_object_show()
 * @see evas_object_hide()
 * @since 1.8
 */
EAPI void                          evas_object_image_source_visible_set(Evas_Object *obj, Eina_Bool visible) EINA_ARG_NONNULL(1);

/**
 * Get the state of the source object visibility.
 *
 * @param obj Proxy (image) object.
 * @return @c EINA_TRUE if source object is visible, @c EINA_FALSE otherwise.
 *
 * @see evas_object_image_source_visible_set()
 * @see evas_object_image_source_set()
 * @see evas_object_show()
 * @see evas_object_hide()
 * @since 1.8
 */
EAPI Eina_Bool                     evas_object_image_source_visible_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * Set whether an Evas object is to source events.
 *
 * @param obj Proxy (image) object.
 * @param source whether @p obj is to pass events (@c EINA_TRUE) or not
 * (@c EINA_FALSE)
 *
 * Set whether an Evas object is to repeat events to source.
 *
 * If @p source is @c EINA_TRUE, it will make events on @p obj to also be
 * repeated for the source object (see evas_object_image_source_set()). Even the
 * @p obj and source geometries are different, the event position will be
 * transformed to the source object's space.
 *
 * If @p source is @c EINA_FALSE, events occurring on @p obj will be
 * processed only on it.
 *
 * @see evas_object_image_source_get()
 * @see evas_object_image_source_visible_set()
 * @see evas_object_source_events_set()
 * @since 1.8
 */
EAPI void evas_object_image_source_events_set(Evas_Object *obj, Eina_Bool source) EINA_ARG_NONNULL(1);

/**
 * Determine whether an object is set to source events.
 *
 * @param obj Proxy (image) object.
 * @return source whether @p obj is set to source events (@c EINA_TRUE) or not
 * (@c EINA_FALSE)
 *
 * @see evas_object_image_source_set()
 * @see evas_object_image_source_visible_set()
 * @see evas_object_source_events_set()
 * @since 1.8
 */
EAPI Eina_Bool evas_object_image_source_events_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Check if a file extension may be supported by @ref Evas_Object_Image.
 *
 * @param file The file to check
 * @return @c EINA_TRUE if we may be able to open it, @c EINA_FALSE if it's
 * unlikely.
 * @since 1.1
 *
 * If file is a Eina_Stringshare, use directly @ref evas_object_image_extension_can_load_fast_get.
 *
 * This functions is threadsafe.
 */
EAPI Eina_Bool                     evas_object_image_extension_can_load_get(const char *file);

/**
 * Check if a file extension may be supported by @ref Evas_Object_Image.
 *
 * @param file The file to check, it should be an Eina_Stringshare.
 * @return @c EINA_TRUE if we may be able to open it, @c EINA_FALSE if it's
 * unlikely.
 * @since 1.1
 *
 * This functions is threadsafe.
 */
EAPI Eina_Bool                     evas_object_image_extension_can_load_fast_get(const char *file);

/**
 * Check if an image object can be animated (have multiple frames)
 *
 * @param obj Image object
 * @return whether obj support animation
 *
 * This returns if the image file of an image object is capable of animation
 * such as an animated gif file might. This is only useful to be called once
 * the image object file has been set.
 *
 * Example:
 * @code
 * extern Evas_Object *obj;
 *
 * if (evas_object_image_animated_get(obj))
 *   {
 *     int frame_count;
 *     int loop_count;
 *     Evas_Image_Animated_Loop_Hint loop_type;
 *     double duration;
 *
 *     frame_count = evas_object_image_animated_frame_count_get(obj);
 *     printf("This image has %d frames\n",frame_count);
 *
 *     duration = evas_object_image_animated_frame_duration_get(obj,1,0);
 *     printf("Frame 1's duration is %f. You had better set object's frame to 2 after this duration using timer\n");
 *
 *     loop_count = evas_object_image_animated_loop_count_get(obj);
 *     printf("loop count is %d. You had better run loop %d times\n",loop_count,loop_count);
 *
 *     loop_type = evas_object_image_animated_loop_type_get(obj);
 *     if (loop_type == EVAS_IMAGE_ANIMATED_HINT_LOOP)
 *       printf("You had better set frame like 1->2->3->1->2->3...\n");
 *     else if (loop_type == EVAS_IMAGE_ANIMATED_HINT_PINGPONG)
 *       printf("You had better set frame like 1->2->3->2->1->2...\n");
 *     else
 *       printf("Unknown loop type\n");
 *
 *     evas_object_image_animated_frame_set(obj,1);
 *     printf("You set image object's frame to 1. You can see frame 1\n");
 *   }
 * @endcode
 *
 * @see evas_object_image_animated_get()
 * @see evas_object_image_animated_frame_count_get()
 * @see evas_object_image_animated_loop_type_get()
 * @see evas_object_image_animated_loop_count_get()
 * @see evas_object_image_animated_frame_duration_get()
 * @see evas_object_image_animated_frame_set()
 * @since 1.1
 */
EAPI Eina_Bool                     evas_object_image_animated_get(const Evas_Object *obj);

/**
 * Get the total number of frames of the image object.
 *
 * @param obj Image object
 * @return The number of frames
 *
 * This returns total number of frames the image object supports (if animated)
 *
 * @see evas_object_image_animated_get()
 * @see evas_object_image_animated_frame_count_get()
 * @see evas_object_image_animated_loop_type_get()
 * @see evas_object_image_animated_loop_count_get()
 * @see evas_object_image_animated_frame_duration_get()
 * @see evas_object_image_animated_frame_set()
 * @since 1.1
 */
EAPI int                           evas_object_image_animated_frame_count_get(const Evas_Object *obj);

/**
 * Get the kind of looping the image object does.
 *
 * @param obj Image object
 * @return Loop type of the image object
 *
 * This returns the kind of looping the image object wants to do.
 *
 * If it returns EVAS_IMAGE_ANIMATED_HINT_LOOP, you should display frames in a sequence like:
 * 1->2->3->1->2->3->1...
 * If it returns EVAS_IMAGE_ANIMATED_HINT_PINGPONG, it is better to
 * display frames in a sequence like: 1->2->3->2->1->2->3->1...
 *
 * The default type is EVAS_IMAGE_ANIMATED_HINT_LOOP.
 *
 * @see evas_object_image_animated_get()
 * @see evas_object_image_animated_frame_count_get()
 * @see evas_object_image_animated_loop_type_get()
 * @see evas_object_image_animated_loop_count_get()
 * @see evas_object_image_animated_frame_duration_get()
 * @see evas_object_image_animated_frame_set()
 * @since 1.1
 */
EAPI Evas_Image_Animated_Loop_Hint evas_object_image_animated_loop_type_get(const Evas_Object *obj);

/**
 * Get the number times the animation of the object loops.
 *
 * @param obj Image object
 * @return The number of loop of an animated image object
 *
 * This returns loop count of image. The loop count is the number of times
 * the animation will play fully from first to last frame until the animation
 * should stop (at the final frame).
 *
 * If 0 is returned, then looping should happen indefinitely (no limit to
 * the number of times it loops).
 *
 * @see evas_object_image_animated_get()
 * @see evas_object_image_animated_frame_count_get()
 * @see evas_object_image_animated_loop_type_get()
 * @see evas_object_image_animated_loop_count_get()
 * @see evas_object_image_animated_frame_duration_get()
 * @see evas_object_image_animated_frame_set()
 * @since 1.1
 */
EAPI int                           evas_object_image_animated_loop_count_get(const Evas_Object *obj);

/**
 * Get the duration of a sequence of frames.
 *
 * @param obj Image object
 * @param start_frame The first frame
 * @param fram_num Number of frames in the sequence
 *
 * This returns total duration that the specified sequence of frames should
 * take in seconds.
 *
 * If you set start_frame to 1 and frame_num 0, you get frame 1's duration
 * If you set start_frame to 1 and frame_num 1, you get frame 1's duration +
 * frame2's duration
 *
 * @see evas_object_image_animated_get()
 * @see evas_object_image_animated_frame_count_get()
 * @see evas_object_image_animated_loop_type_get()
 * @see evas_object_image_animated_loop_count_get()
 * @see evas_object_image_animated_frame_duration_get()
 * @see evas_object_image_animated_frame_set()
 * @since 1.1
 */
EAPI double                        evas_object_image_animated_frame_duration_get(const Evas_Object *obj, int start_frame, int fram_num);

/**
 * Set the frame to current frame of an image object
 *
 * @param obj The given image object.
 * @param frame_num The index of current frame
 *
 * This set image object's current frame to frame_num with 1 being the first
 * frame.
 *
 * @see evas_object_image_animated_get()
 * @see evas_object_image_animated_frame_count_get()
 * @see evas_object_image_animated_loop_type_get()
 * @see evas_object_image_animated_loop_count_get()
 * @see evas_object_image_animated_frame_duration_get()
 * @see evas_object_image_animated_frame_set()
 * @since 1.1
 */
EAPI void                          evas_object_image_animated_frame_set(Evas_Object *obj, int frame_num);
/**
 * @}
 */

/**
 * @defgroup Evas_Object_Text Text Object Functions
 *
 * Functions that operate on single line, single style text objects.
 *
 * For multiline and multiple style text, see @ref Evas_Object_Textblock.
 *
 * See some @ref Example_Evas_Text "examples" on this group of functions.
 *
 * @warning We don't guarantee any proper results if you create a Text object
 * without setting the evas engine.
 *
 * @ingroup Evas_Object_Specific
 */

/**
 * @addtogroup Evas_Object_Text
 * @{
 */

/* basic styles (4 bits allocated use 0->10 now, 5 left) */
#define EVAS_TEXT_STYLE_MASK_BASIC 0xf

/**
 * Text style type creation macro. Use style types on the 's'
 * arguments, being 'x' your style variable.
 */
#define EVAS_TEXT_STYLE_BASIC_SET(x, s) \
  do { x = ((x) & ~EVAS_TEXT_STYLE_MASK_BASIC) | (s); } while (0)

#define EVAS_TEXT_STYLE_MASK_SHADOW_DIRECTION (0x7 << 4)

/**
 * Text style type creation macro. This one will impose shadow
 * directions on the style type variable -- use the @c
 * EVAS_TEXT_STYLE_SHADOW_DIRECTION_* values on 's', incrementally.
 */
#define EVAS_TEXT_STYLE_SHADOW_DIRECTION_SET(x, s) \
  do { x = ((x) & ~EVAS_TEXT_STYLE_MASK_SHADOW_DIRECTION) | (s); } while (0)

typedef enum _Evas_Text_Style_Type
{
   EVAS_TEXT_STYLE_PLAIN,      /**< plain, standard text */
   EVAS_TEXT_STYLE_SHADOW,      /**< text with shadow underneath */
   EVAS_TEXT_STYLE_OUTLINE,      /**< text with an outline */
   EVAS_TEXT_STYLE_SOFT_OUTLINE,      /**< text with a soft outline */
   EVAS_TEXT_STYLE_GLOW,      /**< text with a glow effect */
   EVAS_TEXT_STYLE_OUTLINE_SHADOW,      /**< text with both outline and shadow effects */
   EVAS_TEXT_STYLE_FAR_SHADOW,      /**< text with (far) shadow underneath */
   EVAS_TEXT_STYLE_OUTLINE_SOFT_SHADOW,      /**< text with outline and soft shadow effects combined */
   EVAS_TEXT_STYLE_SOFT_SHADOW,      /**< text with (soft) shadow underneath */
   EVAS_TEXT_STYLE_FAR_SOFT_SHADOW,      /**< text with (far soft) shadow underneath */

   /* OR these to modify shadow direction (3 bits needed) */
   EVAS_TEXT_STYLE_SHADOW_DIRECTION_BOTTOM_RIGHT = (0x0 << 4),      /**< shadow growing to bottom right */
   EVAS_TEXT_STYLE_SHADOW_DIRECTION_BOTTOM = (0x1 << 4),            /**< shadow growing to the bottom */
   EVAS_TEXT_STYLE_SHADOW_DIRECTION_BOTTOM_LEFT = (0x2 << 4),       /**< shadow growing to bottom left */
   EVAS_TEXT_STYLE_SHADOW_DIRECTION_LEFT = (0x3 << 4),              /**< shadow growing to the left */
   EVAS_TEXT_STYLE_SHADOW_DIRECTION_TOP_LEFT = (0x4 << 4),          /**< shadow growing to top left */
   EVAS_TEXT_STYLE_SHADOW_DIRECTION_TOP = (0x5 << 4),               /**< shadow growing to the top */
   EVAS_TEXT_STYLE_SHADOW_DIRECTION_TOP_RIGHT = (0x6 << 4),         /**< shadow growing to top right */
   EVAS_TEXT_STYLE_SHADOW_DIRECTION_RIGHT = (0x7 << 4)             /**< shadow growing to the right */
} Evas_Text_Style_Type;      /**< Types of styles to be applied on text objects. The @c EVAS_TEXT_STYLE_SHADOW_DIRECTION_* ones are to be ORed together with others imposing shadow, to change shadow's direction */

#define EVAS_OBJ_TEXT_CLASS evas_object_text_class_get()

const Eo_Class *evas_object_text_class_get(void) EINA_CONST;

extern EAPI Eo_Op EVAS_OBJ_TEXT_BASE_ID;

enum
{
   EVAS_OBJ_TEXT_SUB_ID_FONT_SOURCE_SET,
   EVAS_OBJ_TEXT_SUB_ID_FONT_SOURCE_GET,
   EVAS_OBJ_TEXT_SUB_ID_FONT_SET,
   EVAS_OBJ_TEXT_SUB_ID_FONT_GET,
   EVAS_OBJ_TEXT_SUB_ID_TEXT_SET,
   EVAS_OBJ_TEXT_SUB_ID_BIDI_DELIMITERS_SET,
   EVAS_OBJ_TEXT_SUB_ID_BIDI_DELIMITERS_GET,
   EVAS_OBJ_TEXT_SUB_ID_TEXT_GET,
   EVAS_OBJ_TEXT_SUB_ID_DIRECTION_GET,
   EVAS_OBJ_TEXT_SUB_ID_ASCENT_GET,
   EVAS_OBJ_TEXT_SUB_ID_DESCENT_GET,
   EVAS_OBJ_TEXT_SUB_ID_MAX_ASCENT_GET,
   EVAS_OBJ_TEXT_SUB_ID_MAX_DESCENT_GET,
   EVAS_OBJ_TEXT_SUB_ID_INSET_GET,
   EVAS_OBJ_TEXT_SUB_ID_HORIZ_ADVANCE_GET,
   EVAS_OBJ_TEXT_SUB_ID_VERT_ADVANCE_GET,
   EVAS_OBJ_TEXT_SUB_ID_CHAR_POS_GET,
   EVAS_OBJ_TEXT_SUB_ID_LAST_UP_TO_POS,
   EVAS_OBJ_TEXT_SUB_ID_CHAR_COORDS_GET,
   EVAS_OBJ_TEXT_SUB_ID_STYLE_SET,
   EVAS_OBJ_TEXT_SUB_ID_STYLE_GET,
   EVAS_OBJ_TEXT_SUB_ID_SHADOW_COLOR_SET,
   EVAS_OBJ_TEXT_SUB_ID_SHADOW_COLOR_GET,
   EVAS_OBJ_TEXT_SUB_ID_GLOW_COLOR_SET,
   EVAS_OBJ_TEXT_SUB_ID_GLOW_COLOR_GET,
   EVAS_OBJ_TEXT_SUB_ID_GLOW2_COLOR_SET,
   EVAS_OBJ_TEXT_SUB_ID_GLOW2_COLOR_GET,
   EVAS_OBJ_TEXT_SUB_ID_OUTLINE_COLOR_SET,
   EVAS_OBJ_TEXT_SUB_ID_OUTLINE_COLOR_GET,
   EVAS_OBJ_TEXT_SUB_ID_STYLE_PAD_GET,
   EVAS_OBJ_TEXT_SUB_ID_LAST
};

#define EVAS_OBJ_TEXT_ID(sub_id) (EVAS_OBJ_TEXT_BASE_ID + sub_id)

/**
 * @def evas_obj_text_font_source_set
 * @since 1.8
 *
 * Set the font (source) file to be used on a given text object.
 *
 * @param[in] font_source in
 *
 * @see evas_object_text_font_source_set
 */
#define evas_obj_text_font_source_set(font_source) EVAS_OBJ_TEXT_ID(EVAS_OBJ_TEXT_SUB_ID_FONT_SOURCE_SET), EO_TYPECHECK(const char *, font_source)

/**
 * @def evas_obj_text_font_source_get
 * @since 1.8
 *
 * Get the font file's path which is being used on a given text
 * object.
 *
 * @param[out] font_source out
 *
 * @see evas_object_text_font_source_get
 */
#define evas_obj_text_font_source_get(font_source) EVAS_OBJ_TEXT_ID(EVAS_OBJ_TEXT_SUB_ID_FONT_SOURCE_GET), EO_TYPECHECK(const char **, font_source)

/**
 * @def evas_obj_text_font_set
 * @since 1.8
 *
 * Set the font family and size on a given text object.
 *
 * @param[in] font in
 * @param[in] size in
 *
 * @see evas_object_text_font_set
 */
#define evas_obj_text_font_set(font, size) EVAS_OBJ_TEXT_ID(EVAS_OBJ_TEXT_SUB_ID_FONT_SET), EO_TYPECHECK(const char *, font), EO_TYPECHECK(Evas_Font_Size, size)

/**
 * @def evas_obj_text_font_get
 * @since 1.8
 *
 * Retrieve the font family and size in use on a given text object.
 *
 * @param[out] font out
 * @param[out] size out
 *
 * @see evas_object_text_font_get
 */
#define evas_obj_text_font_get(font, size) EVAS_OBJ_TEXT_ID(EVAS_OBJ_TEXT_SUB_ID_FONT_GET), EO_TYPECHECK(const char **, font), EO_TYPECHECK(Evas_Font_Size *, size)

/**
 * @def evas_obj_text_text_set
 * @since 1.8
 *
 * Sets the text string to be displayed by the given text object.
 *
 * @param[in] text
 *
 * @see evas_object_text_text_set
 */
#define evas_obj_text_text_set(text) EVAS_OBJ_TEXT_ID(EVAS_OBJ_TEXT_SUB_ID_TEXT_SET), EO_TYPECHECK(const char *, text)

/**
 * @def evas_obj_text_text_get
 * @since 1.8
 *
 * Retrieves the text string currently being displayed by the given
 * text object.
 *
 * @param[out] text out
 *
 * @see evas_object_text_text_get
 */
#define evas_obj_text_text_get(text) EVAS_OBJ_TEXT_ID(EVAS_OBJ_TEXT_SUB_ID_TEXT_GET), EO_TYPECHECK(const char **, text)

/**
 * @def evas_obj_text_bidi_delimiters_set
 * @since 1.8
 *
 * Sets the BiDi delimiters used in the textblock.
 *
 * @param[in] delim in
 *
 * @see evas_object_text_bidi_delimiters_set
 */
#define evas_obj_text_bidi_delimiters_set(delim) EVAS_OBJ_TEXT_ID(EVAS_OBJ_TEXT_SUB_ID_BIDI_DELIMITERS_SET), EO_TYPECHECK(const char *, delim)

/**
 * @def evas_obj_text_bidi_delimiters_get
 * @since 1.8
 *
 * Gets the BiDi delimiters used in the textblock.
 *
 * @param[out] delim out
 *
 * @see evas_object_text_bidi_delimiters_get
 */
#define evas_obj_text_bidi_delimiters_get(delim) EVAS_OBJ_TEXT_ID(EVAS_OBJ_TEXT_SUB_ID_BIDI_DELIMITERS_GET), EO_TYPECHECK(const char **, delim)

/**
 * @def evas_obj_text_direction_get
 * @since 1.8
 *
 * Retrieves the direction of the text currently being displayed in the
 * text object.
 *
 * @param[out] bidi_dir out
 *
 * @see evas_object_text_direction_get
 */
#define evas_obj_text_direction_get(bidi_dir) EVAS_OBJ_TEXT_ID(EVAS_OBJ_TEXT_SUB_ID_DIRECTION_GET), EO_TYPECHECK(Evas_BiDi_Direction *, bidi_dir)

/**
 * @def evas_obj_text_ascent_get
 * @since 1.8
 *
 * @param[out] ascent out
 *
 * @see evas_object_text_ascent_get
 */
#define evas_obj_text_ascent_get(ascent) EVAS_OBJ_TEXT_ID(EVAS_OBJ_TEXT_SUB_ID_ASCENT_GET), EO_TYPECHECK(Evas_Coord *, ascent)

/**
 * @def evas_obj_text_descent_get
 * @since 1.8
 *
 * @param[out] descent out
 *
 * @see evas_object_text_descent_get
 */
#define evas_obj_text_descent_get(descent) EVAS_OBJ_TEXT_ID(EVAS_OBJ_TEXT_SUB_ID_DESCENT_GET), EO_TYPECHECK(Evas_Coord *, descent)

/**
 * @def evas_obj_text_max_ascent_get
 * @since 1.8
 *
 * @param[out] max_ascent out
 *
 * @see evas_object_text_max_ascent_get
 */
#define evas_obj_text_max_ascent_get(max_ascent) EVAS_OBJ_TEXT_ID(EVAS_OBJ_TEXT_SUB_ID_MAX_ASCENT_GET), EO_TYPECHECK(Evas_Coord *, max_ascent)

/**
 * @def evas_obj_text_max_descent_get
 * @since 1.8
 *
 * @param[out] max_descent out
 *
 * @see evas_object_text_max_descent_get
 */
#define evas_obj_text_max_descent_get(max_descent) EVAS_OBJ_TEXT_ID(EVAS_OBJ_TEXT_SUB_ID_MAX_DESCENT_GET), EO_TYPECHECK(Evas_Coord *, max_descent)

/**
 * @def evas_obj_text_inset_get
 * @since 1.8
 *
 * @param[out] inset out
 *
 * @see evas_object_text_inset_get
 */
#define evas_obj_text_inset_get(inset) EVAS_OBJ_TEXT_ID(EVAS_OBJ_TEXT_SUB_ID_INSET_GET), EO_TYPECHECK(Evas_Coord *, inset)

/**
 * @def evas_obj_text_horiz_advance_get
 * @since 1.8
 *
 * @param[out] horiz out
 *
 * @see evas_object_text_horiz_advance_get
 */
#define evas_obj_text_horiz_advance_get(horiz) EVAS_OBJ_TEXT_ID(EVAS_OBJ_TEXT_SUB_ID_HORIZ_ADVANCE_GET), EO_TYPECHECK(Evas_Coord *, horiz)

/**
 * @def evas_obj_text_vert_advance_get
 * @since 1.8
 *
 * @param[out] vert out
 *
 * @see evas_object_text_vert_advance_get
 */
#define evas_obj_text_vert_advance_get(vert) EVAS_OBJ_TEXT_ID(EVAS_OBJ_TEXT_SUB_ID_VERT_ADVANCE_GET), EO_TYPECHECK(Evas_Coord *, vert)

/**
 * @def evas_obj_text_char_pos_get
 * @since 1.8
 *
 * Retrieve position and dimension information of a character within a text @c Evas_Object.
 *
 * @param[in] pos in
 * @param[out] cx out
 * @param[out] cy out
 * @param[out] cw out
 * @param[out] ch out
 * @param[out] ret out
 *
 * @see evas_object_text_char_pos_get
 */
#define evas_obj_text_char_pos_get(pos, cx, cy, cw, ch, ret) EVAS_OBJ_TEXT_ID(EVAS_OBJ_TEXT_SUB_ID_CHAR_POS_GET), EO_TYPECHECK(int, pos), EO_TYPECHECK(Evas_Coord *, cx), EO_TYPECHECK(Evas_Coord *, cy), EO_TYPECHECK(Evas_Coord *, cw), EO_TYPECHECK(Evas_Coord *, ch), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def evas_obj_text_last_up_to_pos
 * @since 1.8
 *
 * Returns the logical position of the last char in the text
 * up to the pos given. this is NOT the position of the last char
 * because of the possibility of RTL in the text.
 *
 * @param[in] x in
 * @param[in] y in
 * @param[out] res out
 *
 * @see evas_object_text_last_up_to_pos
 */
#define evas_obj_text_last_up_to_pos(x, y, res) EVAS_OBJ_TEXT_ID(EVAS_OBJ_TEXT_SUB_ID_LAST_UP_TO_POS), EO_TYPECHECK(Evas_Coord, x), EO_TYPECHECK(Evas_Coord, y), EO_TYPECHECK(int *, res)
/**
 * @def evas_obj_text_char_coords_get
 * @since 1.8
 *
 * @param[in] x in
 * @param[in] y in
 * @param[out] cx out
 * @param[out] cy out
 * @param[out] cw out
 * @param[out] ch out
 * @param[out] res out
 *
 * @see evas_object_text_char_coords_get
 */
#define evas_obj_text_char_coords_get(x, y, cx, cy, cw, ch, res) EVAS_OBJ_TEXT_ID(EVAS_OBJ_TEXT_SUB_ID_CHAR_COORDS_GET), EO_TYPECHECK(Evas_Coord, x), EO_TYPECHECK(Evas_Coord, y), EO_TYPECHECK(Evas_Coord *, cx), EO_TYPECHECK(Evas_Coord *, cy), EO_TYPECHECK(Evas_Coord *, cw), EO_TYPECHECK(Evas_Coord *, ch), EO_TYPECHECK(int *, res)

/**
 * @def evas_obj_text_style_set
 * @since 1.8
 *
 * Sets the style to apply on the given text object.
 *
 * @param[in] style in
 *
 * @see evas_object_text_style_set
 */
#define evas_obj_text_style_set(style) EVAS_OBJ_TEXT_ID(EVAS_OBJ_TEXT_SUB_ID_STYLE_SET), EO_TYPECHECK(Evas_Text_Style_Type, style)

/**
 * @def evas_obj_text_style_get
 * @since 1.8
 *
 * Retrieves the style on use on the given text object.
 *
 * @param[out] style out
 *
 * @see evas_object_text_style_get
 */
#define evas_obj_text_style_get(style) EVAS_OBJ_TEXT_ID(EVAS_OBJ_TEXT_SUB_ID_STYLE_GET), EO_TYPECHECK(Evas_Text_Style_Type *, style)

/**
 * @def evas_obj_text_shadow_color_set
 * @since 1.8
 *
 * Sets the shadow color for the given text object.
 *
 * @param[in] r in
 * @param[in] g in
 * @param[in] b in
 * @param[in] a in
 *
 * @see evas_object_text_shadow_color_set
 */
#define evas_obj_text_shadow_color_set(r, g, b, a) EVAS_OBJ_TEXT_ID(EVAS_OBJ_TEXT_SUB_ID_SHADOW_COLOR_SET), EO_TYPECHECK(int, r), EO_TYPECHECK(int, g), EO_TYPECHECK(int, b), EO_TYPECHECK(int, a)

/**
 * @def evas_obj_text_shadow_color_get
 * @since 1.8
 *
 * Retrieves the shadow color for the given text object.
 *
 * @param[out] r out
 * @param[out] g out
 * @param[out] b out
 * @param[out] a out
 *
 * @see evas_object_text_shadow_color_get
 */
#define evas_obj_text_shadow_color_get(r, g, b, a) EVAS_OBJ_TEXT_ID(EVAS_OBJ_TEXT_SUB_ID_SHADOW_COLOR_GET), EO_TYPECHECK(int *, r), EO_TYPECHECK(int *, g), EO_TYPECHECK(int *, b), EO_TYPECHECK(int *, a)

/**
 * @def evas_obj_text_glow_color_set
 * @since 1.8
 *
 * Sets the glow color for the given text object.
 *
 * @param[in] r in
 * @param[in] g in
 * @param[in] b in
 * @param[in] a in
 *
 * @see evas_object_text_glow_color_set
 */
#define evas_obj_text_glow_color_set(r, g, b, a) EVAS_OBJ_TEXT_ID(EVAS_OBJ_TEXT_SUB_ID_GLOW_COLOR_SET), EO_TYPECHECK(int, r), EO_TYPECHECK(int, g), EO_TYPECHECK(int, b), EO_TYPECHECK(int, a)

/**
 * @def evas_obj_text_glow_color_get
 * @since 1.8
 *
 * Retrieves the glow color for the given text object.
 *
 * @param[out] r out
 * @param[out] g out
 * @param[out] b out
 * @param[out] a out
 *
 * @see evas_object_text_glow_color_get
 */
#define evas_obj_text_glow_color_get(r, g, b, a) EVAS_OBJ_TEXT_ID(EVAS_OBJ_TEXT_SUB_ID_GLOW_COLOR_GET), EO_TYPECHECK(int *, r), EO_TYPECHECK(int *, g), EO_TYPECHECK(int *, b), EO_TYPECHECK(int *, a)

/**
 * @def evas_obj_text_glow2_color_set
 * @since 1.8
 *
 * Sets the 'glow 2' color for the given text object.
 *
 * @param[in] r in
 * @param[in] g in
 * @param[in] b in
 * @param[in] a in
 *
 * @see evas_object_text_glow2_color_set
 */
#define evas_obj_text_glow2_color_set(r, g, b, a) EVAS_OBJ_TEXT_ID(EVAS_OBJ_TEXT_SUB_ID_GLOW2_COLOR_SET), EO_TYPECHECK(int, r), EO_TYPECHECK(int, g), EO_TYPECHECK(int, b), EO_TYPECHECK(int, a)

/**
 * @def evas_obj_text_glow2_color_get
 * @since 1.8
 *
 * Retrieves the 'glow 2' color for the given text object.
 *
 * @param[out] r out
 * @param[out] g out
 * @param[out] b out
 * @param[out] a out
 *
 * @see evas_object_text_glow2_color_get
 */
#define evas_obj_text_glow2_color_get(r, g, b, a) EVAS_OBJ_TEXT_ID(EVAS_OBJ_TEXT_SUB_ID_GLOW2_COLOR_GET), EO_TYPECHECK(int *, r), EO_TYPECHECK(int *, g), EO_TYPECHECK(int *, b), EO_TYPECHECK(int *, a)

/**
 * @def evas_obj_text_outline_color_set
 * @since 1.8
 *
 * Sets the outline color for the given text object.
 *
 * @param[in] r in
 * @param[in] g in
 * @param[in] b in
 * @param[in] a in
 *
 * @see evas_object_text_outline_color_set
 */
#define evas_obj_text_outline_color_set(r, g, b, a) EVAS_OBJ_TEXT_ID(EVAS_OBJ_TEXT_SUB_ID_OUTLINE_COLOR_SET), EO_TYPECHECK(int, r), EO_TYPECHECK(int, g), EO_TYPECHECK(int, b), EO_TYPECHECK(int, a)

/**
 * @def evas_obj_text_outline_color_get
 * @since 1.8
 *
 * Retrieves the outline color for the given text object.
 *
 * @param[out] r out
 * @param[out] g out
 * @param[out] b out
 * @param[out] a out
 *
 * @see evas_object_text_outline_color_get
 */
#define evas_obj_text_outline_color_get(r, g, b, a) EVAS_OBJ_TEXT_ID(EVAS_OBJ_TEXT_SUB_ID_OUTLINE_COLOR_GET), EO_TYPECHECK(int *, r), EO_TYPECHECK(int *, g), EO_TYPECHECK(int *, b), EO_TYPECHECK(int *, a)

/**
 * @def evas_obj_text_style_pad_get
 * @since 1.8
 *
 * Gets the text style pad of a text object.
 *
 * @param[out] l out
 * @param[out] r out
 * @param[out] t out
 * @param[out] b out
 *
 * @see evas_object_text_style_pad_get
 */
#define evas_obj_text_style_pad_get(l, r, t, b) EVAS_OBJ_TEXT_ID(EVAS_OBJ_TEXT_SUB_ID_STYLE_PAD_GET), EO_TYPECHECK(int *, l), EO_TYPECHECK(int *, r), EO_TYPECHECK(int *, t), EO_TYPECHECK(int *, b)

/**
 * Creates a new text object on the provided canvas.
 *
 * @param e The canvas to create the text object on.
 * @return @c NULL on error, a pointer to a new text object on
 * success.
 *
 * Text objects are for simple, single line text elements. If you want
 * more elaborated text blocks, see @ref Evas_Object_Textblock.
 *
 * @see evas_object_text_font_source_set()
 * @see evas_object_text_font_set()
 * @see evas_object_text_text_set()
 */
EAPI Evas_Object         *evas_object_text_add(Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * Set the font (source) file to be used on a given text object.
 *
 * @param obj The text object to set font for.
 * @param font The font file's path.
 *
 * This function allows the font file to be explicitly set for a given
 * text object, overriding system lookup, which will first occur in
 * the given file's contents.
 *
 * @see evas_object_text_font_get()
 */
EAPI void                 evas_object_text_font_source_set(Evas_Object *obj, const char *font) EINA_ARG_NONNULL(1);

/**
 * Get the font file's path which is being used on a given text
 * object.
 *
 * @param obj The text object to set font for.
 * @return The font file's path.
 *
 * @see evas_object_text_font_get() for more details
 */
EAPI const char          *evas_object_text_font_source_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Set the font family and size on a given text object.
 *
 * @param obj The text object to set font for.
 * @param font The font (family) name.
 * @param size The font size, in points.
 *
 * This function allows the font name and size of a text object to be
 * set. The @p font string has to follow fontconfig's convention on
 * naming fonts, as it's the underlying library used to query system
 * fonts by Evas (see the @c fc-list command's output, on your system,
 * to get an idea).
 *
 * @see evas_object_text_font_get()
 * @see evas_object_text_font_source_set()
 */
EAPI void                 evas_object_text_font_set(Evas_Object *obj, const char *font, Evas_Font_Size size) EINA_ARG_NONNULL(1);

/**
 * Retrieve the font family and size in use on a given text object.
 *
 * @param obj The evas text object to query for font information.
 * @param font A pointer to the location to store the font name in.
 * @param size A pointer to the location to store the font size in.
 *
 * This function allows the font name and size of a text object to be
 * queried. Be aware that the font name string is still owned by Evas
 * and should @b not have free() called on it by the caller of the
 * function.
 *
 * @see evas_object_text_font_set()
 */
EAPI void                 evas_object_text_font_get(const Evas_Object *obj, const char **font, Evas_Font_Size *size) EINA_ARG_NONNULL(1);

/**
 * Sets the text string to be displayed by the given text object.
 *
 * @param obj The text object to set text string on.
 * @param text Text string to display on it.
 *
 * @see evas_object_text_text_get()
 */
EAPI void                 evas_object_text_text_set(Evas_Object *obj, const char *text) EINA_ARG_NONNULL(1);

/**
 * Retrieves the text string currently being displayed by the given
 * text object.
 *
 * @param  obj The given text object.
 * @return The text string currently being displayed on it.
 *
 * @note Do not free() the return value.
 *
 * @see evas_object_text_text_set()
 */
EAPI const char          *evas_object_text_text_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief Sets the BiDi delimiters used in the textblock.
 *
 * BiDi delimiters are use for in-paragraph separation of bidi segments. This
 * is useful for example in recipients fields of e-mail clients where bidi
 * oddities can occur when mixing RTL and LTR.
 *
 * @param obj The given text object.
 * @param delim A null terminated string of delimiters, e.g ",|".
 * @since 1.1
 */
EAPI void                 evas_object_text_bidi_delimiters_set(Evas_Object *obj, const char *delim);

/**
 * @brief Gets the BiDi delimiters used in the textblock.
 *
 * BiDi delimiters are use for in-paragraph separation of bidi segments. This
 * is useful for example in recipients fields of e-mail clients where bidi
 * oddities can occur when mixing RTL and LTR.
 *
 * @param obj The given text object.
 * @return A null terminated string of delimiters, e.g ",|". If empty, returns NULL.
 * @since 1.1
 */
EAPI const char          *evas_object_text_bidi_delimiters_get(const Evas_Object *obj);

EAPI Evas_Coord           evas_object_text_ascent_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);
EAPI Evas_Coord           evas_object_text_descent_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);
EAPI Evas_Coord           evas_object_text_max_ascent_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);
EAPI Evas_Coord           evas_object_text_max_descent_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);
EAPI Evas_Coord           evas_object_text_horiz_advance_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);
EAPI Evas_Coord           evas_object_text_vert_advance_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);
EAPI Evas_Coord           evas_object_text_inset_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Retrieve position and dimension information of a character within a text @c Evas_Object.
 *
 * This function is used to obtain the X, Y, width and height of a the character
 * located at @p pos within the @c Evas_Object @p obj. @p obj must be a text object
 * as created with evas_object_text_add(). Any of the @c Evas_Coord parameters (@p cx,
 * @p cy, @p cw, @p ch) may be @c NULL in which case no value will be assigned to that
 * parameter.
 *
 * @param obj	The text object to retrieve position information for.
 * @param pos	The character position to request co-ordinates for.
 * @param cx	A pointer to an @c Evas_Coord to store the X value in (can be NULL).
 * @param cy	A pointer to an @c Evas_Coord to store the Y value in (can be NULL).
 * @param cw	A pointer to an @c Evas_Coord to store the Width value in (can be NULL).
 * @param ch	A pointer to an @c Evas_Coord to store the Height value in (can be NULL).
 *
 * @return @c EINA_FALSE on success, @c EINA_TRUE on error.
 */
EAPI Eina_Bool            evas_object_text_char_pos_get(const Evas_Object *obj, int pos, Evas_Coord *cx, Evas_Coord *cy, Evas_Coord *cw, Evas_Coord *ch) EINA_ARG_NONNULL(1);
EAPI int                  evas_object_text_char_coords_get(const Evas_Object *obj, Evas_Coord x, Evas_Coord y, Evas_Coord *cx, Evas_Coord *cy, Evas_Coord *cw, Evas_Coord *ch) EINA_ARG_NONNULL(1);

/**
 * Returns the logical position of the last char in the text
 * up to the pos given. this is NOT the position of the last char
 * because of the possibility of RTL in the text.
 */
EAPI int                  evas_object_text_last_up_to_pos(const Evas_Object *obj, Evas_Coord x, Evas_Coord y) EINA_ARG_NONNULL(1);

/**
 * Retrieves the style on use on the given text object.
 *
 * @param obj the given text object to set style on.
 * @return the style type in use.
 *
 * @see evas_object_text_style_set() for more details.
 */
EAPI Evas_Text_Style_Type evas_object_text_style_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Sets the style to apply on the given text object.
 *
 * @param obj the given text object to set style on.
 * @param type a style type.
 *
 * Text object styles are one of the values in
 * #Evas_Text_Style_Type. Some of those values are combinations of
 * more than one style, and some account for the direction of the
 * rendering of shadow effects.
 *
 * @note One may use the helper macros #EVAS_TEXT_STYLE_BASIC_SET and
 * #EVAS_TEXT_STYLE_SHADOW_DIRECTION_SET to assemble a style value.
 *
 * The following figure illustrates the text styles:
 *
 * @image html text-styles.png
 * @image rtf text-styles.png
 * @image latex text-styles.eps
 *
 * @see evas_object_text_style_get()
 * @see evas_object_text_shadow_color_set()
 * @see evas_object_text_outline_color_set()
 * @see evas_object_text_glow_color_set()
 * @see evas_object_text_glow2_color_set()
 */
EAPI void                 evas_object_text_style_set(Evas_Object *obj, Evas_Text_Style_Type type) EINA_ARG_NONNULL(1);

/**
 * Sets the shadow color for the given text object.
 *
 * @param obj The given Evas text object.
 * @param r The red component of the given color.
 * @param g The green component of the given color.
 * @param b The blue component of the given color.
 * @param a The alpha component of the given color.
 *
 * Shadow effects, which are fading colors decorating the text
 * underneath it, will just be shown if the object is set to one of
 * the following styles:
 *
 * - #EVAS_TEXT_STYLE_SHADOW
 * - #EVAS_TEXT_STYLE_OUTLINE_SHADOW
 * - #EVAS_TEXT_STYLE_FAR_SHADOW
 * - #EVAS_TEXT_STYLE_OUTLINE_SOFT_SHADOW
 * - #EVAS_TEXT_STYLE_SOFT_SHADOW
 * - #EVAS_TEXT_STYLE_FAR_SOFT_SHADOW
 *
 * One can also change the direction where the shadow grows to, with
 * evas_object_text_style_set().
 *
 * @see evas_object_text_shadow_color_get()
 */
EAPI void                 evas_object_text_shadow_color_set(Evas_Object *obj, int r, int g, int b, int a) EINA_ARG_NONNULL(1);

/**
 * Retrieves the shadow color for the given text object.
 *
 * @param obj The given Evas text object.
 * @param r Pointer to variable to hold the red component of the given
 * color.
 * @param g Pointer to variable to hold the green component of the
 * given color.
 * @param b Pointer to variable to hold the blue component of the
 * given color.
 * @param a Pointer to variable to hold the alpha component of the
 * given color.
 *
 * @note Use @c NULL pointers on the color components you're not
 * interested in: they'll be ignored by the function.
 *
 * @see evas_object_text_shadow_color_set() for more details.
 */
EAPI void                 evas_object_text_shadow_color_get(const Evas_Object *obj, int *r, int *g, int *b, int *a) EINA_ARG_NONNULL(1);

/**
 * Sets the glow color for the given text object.
 *
 * @param obj The given Evas text object.
 * @param r The red component of the given color.
 * @param g The green component of the given color.
 * @param b The blue component of the given color.
 * @param a The alpha component of the given color.
 *
 * Glow effects, which are glowing colors decorating the text's
 * surroundings, will just be shown if the object is set to the
 * #EVAS_TEXT_STYLE_GLOW style.
 *
 * @note Glow effects are placed from a short distance of the text
 * itself, but no touching it. For glowing effects right on the
 * borders of the glyphs, see 'glow 2' effects
 * (evas_object_text_glow2_color_set()).
 *
 * @see evas_object_text_glow_color_get()
 */
EAPI void                 evas_object_text_glow_color_set(Evas_Object *obj, int r, int g, int b, int a) EINA_ARG_NONNULL(1);

/**
 * Retrieves the glow color for the given text object.
 *
 * @param obj The given Evas text object.
 * @param r Pointer to variable to hold the red component of the given
 * color.
 * @param g Pointer to variable to hold the green component of the
 * given color.
 * @param b Pointer to variable to hold the blue component of the
 * given color.
 * @param a Pointer to variable to hold the alpha component of the
 * given color.
 *
 * @note Use @c NULL pointers on the color components you're not
 * interested in: they'll be ignored by the function.
 *
 * @see evas_object_text_glow_color_set() for more details.
 */
EAPI void                 evas_object_text_glow_color_get(const Evas_Object *obj, int *r, int *g, int *b, int *a) EINA_ARG_NONNULL(1);

/**
 * Sets the 'glow 2' color for the given text object.
 *
 * @param obj The given Evas text object.
 * @param r The red component of the given color.
 * @param g The green component of the given color.
 * @param b The blue component of the given color.
 * @param a The alpha component of the given color.
 *
 * 'Glow 2' effects, which are glowing colors decorating the text's
 * (immediate) surroundings, will just be shown if the object is set
 * to the #EVAS_TEXT_STYLE_GLOW style. See also
 * evas_object_text_glow_color_set().
 *
 * @see evas_object_text_glow2_color_get()
 */
EAPI void                 evas_object_text_glow2_color_set(Evas_Object *obj, int r, int g, int b, int a) EINA_ARG_NONNULL(1);

/**
 * Retrieves the 'glow 2' color for the given text object.
 *
 * @param obj The given Evas text object.
 * @param r Pointer to variable to hold the red component of the given
 * color.
 * @param g Pointer to variable to hold the green component of the
 * given color.
 * @param b Pointer to variable to hold the blue component of the
 * given color.
 * @param a Pointer to variable to hold the alpha component of the
 * given color.
 *
 * @note Use @c NULL pointers on the color components you're not
 * interested in: they'll be ignored by the function.
 *
 * @see evas_object_text_glow2_color_set() for more details.
 */
EAPI void                 evas_object_text_glow2_color_get(const Evas_Object *obj, int *r, int *g, int *b, int *a) EINA_ARG_NONNULL(1);

/**
 * Sets the outline color for the given text object.
 *
 * @param obj The given Evas text object.
 * @param r The red component of the given color.
 * @param g The green component of the given color.
 * @param b The blue component of the given color.
 * @param a The alpha component of the given color.
 *
 * Outline effects (colored lines around text glyphs) will just be
 * shown if the object is set to one of the following styles:
 * - #EVAS_TEXT_STYLE_OUTLINE
 * - #EVAS_TEXT_STYLE_SOFT_OUTLINE
 * - #EVAS_TEXT_STYLE_OUTLINE_SHADOW
 * - #EVAS_TEXT_STYLE_OUTLINE_SOFT_SHADOW
 *
 * @see evas_object_text_outline_color_get()
 */
EAPI void                 evas_object_text_outline_color_set(Evas_Object *obj, int r, int g, int b, int a) EINA_ARG_NONNULL(1);

/**
 * Retrieves the outline color for the given text object.
 *
 * @param obj The given Evas text object.
 * @param r Pointer to variable to hold the red component of the given
 * color.
 * @param g Pointer to variable to hold the green component of the
 * given color.
 * @param b Pointer to variable to hold the blue component of the
 * given color.
 * @param a Pointer to variable to hold the alpha component of the
 * given color.
 *
 * @note Use @c NULL pointers on the color components you're not
 * interested in: they'll be ignored by the function.
 *
 * @see evas_object_text_outline_color_set() for more details.
 */
EAPI void                 evas_object_text_outline_color_get(const Evas_Object *obj, int *r, int *g, int *b, int *a) EINA_ARG_NONNULL(1);

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
EAPI void                 evas_object_text_style_pad_get(const Evas_Object *obj, int *l, int *r, int *t, int *b) EINA_ARG_NONNULL(1);

/**
 * Retrieves the direction of the text currently being displayed in the
 * text object.
 * @param  obj The given evas text object.
 * @return the direction of the text
 */
EAPI Evas_BiDi_Direction  evas_object_text_direction_get(const Evas_Object *obj) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

/**
 * @}
 */

/**
 * @defgroup Evas_Object_Textblock Textblock Object Functions
 *
 * Functions used to create and manipulate textblock objects. Unlike
 * @ref Evas_Object_Text, these handle complex text, doing multiple
 * styles and multiline text based on HTML-like tags. Of these extra
 * features will be heavier on memory and processing cost.
 *
 * @section Evas_Object_Textblock_Tutorial Textblock Object Tutorial
 *
 * This part explains about the textblock object's API and proper usage.
 * The main user of the textblock object is the edje entry object in Edje, so
 * that's a good place to learn from, but I think this document is more than
 * enough, if it's not, please contact me and I'll update it.
 *
 * @subsection textblock_intro Introduction
 * The textblock objects is, as implied, an object that can show big chunks of
 * text. Textblock supports many features including: Text formatting, automatic
 * and manual text alignment, embedding items (for example icons) and more.
 * Textblock has three important parts, the text paragraphs, the format nodes
 * and the cursors.
 *
 * You can use markup to format text, for example: "<font_size=50>Big!</font_size>".
 * You can also put more than one style directive in one tag:
 * "<font_size=50 color=#F00>Big and Red!</font_size>".
 * Please notice that we used "</font_size>" although the format also included
 * color, this is because the first format determines the matching closing tag's
 * name. You can also use anonymous tags, like: "<font_size=30>Big</>" which
 * just pop any type of format, but it's advised to use the named alternatives
 * instead.
 *
 * @subsection textblock_cursors Textblock Object Cursors
 * A textblock Cursor is data type that represents
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
 * A closing tag (i.e a \</tag\> tag) should NEVER be visible.
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
 * Textblock supports various format directives that can be used in markup. In
 * addition to the mentioned format directives, textblock allows creating
 * additional format directives using "tags" that can be set in the style see
 * @ref evas_textblock_style_set .
 *
 * Textblock supports the following formats:
 * @li font - Font description in fontconfig like format, e.g: "Sans:style=Italic:lang=hi". or "Serif:style=Bold".
 * @li font_weight - Overrides the weight defined in "font". E.g: "font_weight=Bold" is the same as "font=:style=Bold". Supported weights: "normal", "thin", "ultralight", "light", "book", "medium", "semibold", "bold", "ultrabold", "black", and "extrablack".
 * @li font_style - Overrides the style defined in "font". E.g: "font_style=Italic" is the same as "font=:style=Italic". Supported styles: "normal", "oblique", and "italic".
 * @li font_width - Overrides the width defined in "font". E.g: "font_width=Condensed" is the same as "font=:style=Condensed". Supported widths: "normal", "ultracondensed", "extracondensed", "condensed", "semicondensed", "semiexpanded", "expanded", "extraexpanded", and "ultraexpanded".
 * @li lang - Overrides the language defined in "font". E.g: "lang=he" is the same as "font=:lang=he".
 * @li font_fallbacks - A comma delimited list of fonts to try if finding the main font fails.
 * @li font_size - The font size in points.
 * @li font_source - The source of the font, e.g an eet file.
 * @li color - Text color in one of the following formats: "#RRGGBB", "#RRGGBBAA", "#RGB", and "#RGBA".
 * @li underline_color - color in one of the following formats: "#RRGGBB", "#RRGGBBAA", "#RGB", and "#RGBA".
 * @li underline2_color - color in one of the following formats: "#RRGGBB", "#RRGGBBAA", "#RGB", and "#RGBA".
 * @li outline_color - color in one of the following formats: "#RRGGBB", "#RRGGBBAA", "#RGB", and "#RGBA".
 * @li shadow_color - color in one of the following formats: "#RRGGBB", "#RRGGBBAA", "#RGB", and "#RGBA".
 * @li glow_color - color in one of the following formats: "#RRGGBB", "#RRGGBBAA", "#RGB", and "#RGBA".
 * @li glow2_color - color in one of the following formats: "#RRGGBB", "#RRGGBBAA", "#RGB", and "#RGBA".
 * @li strikethrough_color - color in one of the following formats: "#RRGGBB", "#RRGGBBAA", "#RGB", and "#RGBA".
 * @li align - Either "auto" (meaning according to text direction), "left", "right", "center", "middle", a value between 0.0 and 1.0, or a value between 0% to 100%.
 * @li valign - Either "top", "bottom", "middle", "center", "baseline", "base", a value between 0.0 and 1.0, or a value between 0% to 100%.
 * @li wrap - "word", "char", "mixed", or "none".
 * @li left_margin - Either "reset", or a pixel value indicating the margin.
 * @li right_margin - Either "reset", or a pixel value indicating the margin.
 * @li underline - "on", "off", "single", or "double".
 * @li strikethrough - "on" or "off"
 * @li backing_color - Background color in one of the following formats: "#RRGGBB", "#RRGGBBAA", "#RGB", and "#RGBA".
 * @li backing - Enable/disable background color. ex) "on" or "off"
 * @li style - Either "off", "none", "plain", "shadow", "outline", "soft_outline", "outline_shadow", "outline_soft_shadow", "glow", "far_shadow", "soft_shadow", or "far_soft_shadow". Direction can be selected by adding "bottom_right", "bottom", "bottom_left", "left", "top_left", "top", "top_right", or "right". E.g: "style=shadow,bottom_right".
 * @li tabstops - Pixel value for tab width.
 * @li linesize - Force a line size in pixels.
 * @li linerelsize - Either a floating point value or a percentage indicating the wanted size of the line relative to the calculated size.
 * @li linegap - Force a line gap in pixels.
 * @li linerelgap - Either a floating point value or a percentage indicating the wanted size of the line relative to the calculated size.
 * @li item - Creates an empty space that should be filled by an upper layer. Use "size", "abssize", or "relsize". To define the items size, and an optional: vsize=full/ascent to define the item's position in the line.
 * @li linefill - Either a float value or percentage indicating how much to fill the line.
 * @li ellipsis - Value between 0.0-1.0 to indicate the type of ellipsis, or -1.0 to indicate ellipsis isn't wanted.
 * @li password - "on" or "off". This is used to specifically turn replacing chars with the replacement char (i.e password mode) on and off.
 *
 * @warning We don't guarantee any proper results if you create a Textblock
 * object
 * without setting the evas engine.
 *
 * @todo put here some usage examples
 *
 * @ingroup Evas_Object_Specific
 *
 * @{
 */

typedef struct _Evas_Textblock_Style              Evas_Textblock_Style;
typedef struct _Evas_Textblock_Cursor             Evas_Textblock_Cursor;
/**
 * @typedef Evas_Object_Textblock_Node_Format
 * A format node.
 */
typedef struct _Evas_Object_Textblock_Node_Format Evas_Object_Textblock_Node_Format;
typedef struct _Evas_Textblock_Rectangle          Evas_Textblock_Rectangle;

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

#define EVAS_OBJ_TEXTBLOCK_CLASS evas_object_textblock_class_get()

const Eo_Class *evas_object_textblock_class_get(void) EINA_CONST;

extern EAPI Eo_Op EVAS_OBJ_TEXTBLOCK_BASE_ID;

enum
{
   EVAS_OBJ_TEXTBLOCK_SUB_ID_STYLE_SET,
   EVAS_OBJ_TEXTBLOCK_SUB_ID_STYLE_GET,
   EVAS_OBJ_TEXTBLOCK_SUB_ID_STYLE_USER_PUSH,
   EVAS_OBJ_TEXTBLOCK_SUB_ID_STYLE_USER_PEEK,
   EVAS_OBJ_TEXTBLOCK_SUB_ID_STYLE_USER_POP,
   EVAS_OBJ_TEXTBLOCK_SUB_ID_REPLACE_CHAR_SET,
   EVAS_OBJ_TEXTBLOCK_SUB_ID_LEGACY_NEWLINE_SET,
   EVAS_OBJ_TEXTBLOCK_SUB_ID_LEGACY_NEWLINE_GET,
   EVAS_OBJ_TEXTBLOCK_SUB_ID_VALIGN_SET,
   EVAS_OBJ_TEXTBLOCK_SUB_ID_VALIGN_GET,
   EVAS_OBJ_TEXTBLOCK_SUB_ID_BIDI_DELIMITERS_SET,
   EVAS_OBJ_TEXTBLOCK_SUB_ID_BIDI_DELIMITERS_GET,
   EVAS_OBJ_TEXTBLOCK_SUB_ID_REPLACE_CHAR_GET,
   EVAS_OBJ_TEXTBLOCK_SUB_ID_TEXT_MARKUP_SET,
   EVAS_OBJ_TEXTBLOCK_SUB_ID_TEXT_MARKUP_GET,
   EVAS_OBJ_TEXTBLOCK_SUB_ID_CURSOR_GET,
   EVAS_OBJ_TEXTBLOCK_SUB_ID_CURSOR_NEW,
   EVAS_OBJ_TEXTBLOCK_SUB_ID_NODE_FORMAT_LIST_GET,
   EVAS_OBJ_TEXTBLOCK_SUB_ID_NODE_FORMAT_FIRST_GET,
   EVAS_OBJ_TEXTBLOCK_SUB_ID_NODE_FORMAT_LAST_GET,
   EVAS_OBJ_TEXTBLOCK_SUB_ID_NODE_FORMAT_REMOVE_PAIR,
   EVAS_OBJ_TEXTBLOCK_SUB_ID_LINE_NUMBER_GEOMETRY_GET,
   EVAS_OBJ_TEXTBLOCK_SUB_ID_CLEAR,
   EVAS_OBJ_TEXTBLOCK_SUB_ID_SIZE_FORMATTED_GET,
   EVAS_OBJ_TEXTBLOCK_SUB_ID_SIZE_NATIVE_GET,
   EVAS_OBJ_TEXTBLOCK_SUB_ID_STYLE_INSETS_GET,
   EVAS_OBJ_TEXTBLOCK_SUB_ID_LAST
};

#define EVAS_OBJ_TEXTBLOCK_ID(sub_id) (EVAS_OBJ_TEXTBLOCK_BASE_ID + sub_id)

/**
 * @def evas_obj_textblock_style_set
 * @since 1.8
 *
 * Set the objects style to ts.
 *
 * @param[in] ts
 *
 * @see evas_object_textblock_style_set
 */
#define evas_obj_textblock_style_set(ts) EVAS_OBJ_TEXTBLOCK_ID(EVAS_OBJ_TEXTBLOCK_SUB_ID_STYLE_SET), EO_TYPECHECK(Evas_Textblock_Style *, ts)

/**
 * @def evas_obj_textblock_style_get
 * @since 1.8
 *
 * Return the style of an object.
 *
 * @param[out] ts
 *
 * @see evas_object_textblock_style_get
 */
#define evas_obj_textblock_style_get(ts) EVAS_OBJ_TEXTBLOCK_ID(EVAS_OBJ_TEXTBLOCK_SUB_ID_STYLE_GET), EO_TYPECHECK(const Evas_Textblock_Style **, ts)

/**
 * @def evas_obj_textblock_style_user_push
 * @since 1.8
 *
 * Push ts to the top of the user style stack.
 *
 * @param[in] ts
 *
 * @see evas_object_textblock_style_user_push
 */
#define evas_obj_textblock_style_user_push(ts) EVAS_OBJ_TEXTBLOCK_ID(EVAS_OBJ_TEXTBLOCK_SUB_ID_STYLE_USER_PUSH), EO_TYPECHECK(Evas_Textblock_Style *, ts)

/**
 * @def evas_obj_textblock_style_user_peek
 * @since 1.8
 *
 * Get (don't remove) the style at the top of the user style stack.
 *
 * @param[out] ts
 *
 * @see evas_object_textblock_style_user_peek
 */
#define evas_obj_textblock_style_user_peek(ts) EVAS_OBJ_TEXTBLOCK_ID(EVAS_OBJ_TEXTBLOCK_SUB_ID_STYLE_USER_PEEK), EO_TYPECHECK(const Evas_Textblock_Style **, ts)

/**
 * @def evas_obj_textblock_style_user_pop
 * @since 1.8
 *
 * Del the from the top of the user style stack.
 *
 *
 * @see evas_object_textblock_style_user_pop
 */
#define evas_obj_textblock_style_user_pop() EVAS_OBJ_TEXTBLOCK_ID(EVAS_OBJ_TEXTBLOCK_SUB_ID_STYLE_USER_POP)

/**
 * @def evas_obj_textblock_replace_char_set
 * @since 1.8
 *
 * Set the "replacement character" to use for the given textblock object.
 *
 * @param[in] ch
 *
 * @see evas_object_textblock_replace_char_set
 */
#define evas_obj_textblock_replace_char_set(ch) EVAS_OBJ_TEXTBLOCK_ID(EVAS_OBJ_TEXTBLOCK_SUB_ID_REPLACE_CHAR_SET), EO_TYPECHECK(const char *, ch)

/**
 * @def evas_obj_textblock_legacy_newline_set
 * @since 1.8
 *
 * Sets newline mode. When true, newline character will behave
 * as a paragraph separator.
 *
 * @param[in] mode
 *
 * @see evas_object_textblock_legacy_newline_set
 */
#define evas_obj_textblock_legacy_newline_set(mode) EVAS_OBJ_TEXTBLOCK_ID(EVAS_OBJ_TEXTBLOCK_SUB_ID_LEGACY_NEWLINE_SET), EO_TYPECHECK(Eina_Bool, mode)

/**
 * @def evas_obj_textblock_legacy_newline_get
 * @since 1.8
 *
 * Gets newline mode. When true, newline character behaves
 * as a paragraph separator.
 *
 * @param[out] newline
 *
 * @see evas_object_textblock_legacy_newline_get
 */
#define evas_obj_textblock_legacy_newline_get(newline) EVAS_OBJ_TEXTBLOCK_ID(EVAS_OBJ_TEXTBLOCK_SUB_ID_LEGACY_NEWLINE_GET), EO_TYPECHECK(Eina_Bool *, newline)

/**
 * @def evas_obj_textblock_valign_set
 * @since 1.8
 *
 * Sets the vertical alignment of text within the textblock object
 *
 * @param[in] align
 *
 * @see evas_object_textblock_valign_set
 */
#define evas_obj_textblock_valign_set(align) EVAS_OBJ_TEXTBLOCK_ID(EVAS_OBJ_TEXTBLOCK_SUB_ID_VALIGN_SET), EO_TYPECHECK(double, align)

/**
 * @def evas_obj_textblock_valign_get
 * @since 1.8
 *
 * Gets the vertical alignment of a textblock
 *
 * @param[out] valign
 *
 * @see evas_object_textblock_valign_get
 */
#define evas_obj_textblock_valign_get(valign) EVAS_OBJ_TEXTBLOCK_ID(EVAS_OBJ_TEXTBLOCK_SUB_ID_VALIGN_GET), EO_TYPECHECK(double *, valign)

/**
 * @def evas_obj_textblock_bidi_delimiters_set
 * @since 1.8
 *
 * Sets the BiDi delimiters used in the textblock.
 *
 * @param[in] delim
 *
 * @see evas_object_textblock_bidi_delimiters_set
 */
#define evas_obj_textblock_bidi_delimiters_set(delim) EVAS_OBJ_TEXTBLOCK_ID(EVAS_OBJ_TEXTBLOCK_SUB_ID_BIDI_DELIMITERS_SET), EO_TYPECHECK(const char *, delim)

/**
 * @def evas_obj_textblock_bidi_delimiters_get
 * @since 1.8
 *
 * Gets the BiDi delimiters used in the textblock.
 *
 * @param[out] delim
 *
 * @see evas_object_textblock_bidi_delimiters_get
 */
#define evas_obj_textblock_bidi_delimiters_get(delim) EVAS_OBJ_TEXTBLOCK_ID(EVAS_OBJ_TEXTBLOCK_SUB_ID_BIDI_DELIMITERS_GET), EO_TYPECHECK(const char **, delim)

/**
 * @def evas_obj_textblock_replace_char_get
 * @since 1.8
 *
 * Get the "replacement character" for given textblock object.
 *
 * @param[out] repch
 *
 * @see evas_object_textblock_replace_char_get
 */
#define evas_obj_textblock_replace_char_get(repch) EVAS_OBJ_TEXTBLOCK_ID(EVAS_OBJ_TEXTBLOCK_SUB_ID_REPLACE_CHAR_GET), EO_TYPECHECK(const char **, repch)

/**
 * @def evas_obj_textblock_text_markup_set
 * @since 1.8
 *
 * Sets the tetxblock's text to the markup text.
 *
 * @param[in] text
 *
 * @see evas_object_textblock_text_markup_set
 */
#define evas_obj_textblock_text_markup_set(text) EVAS_OBJ_TEXTBLOCK_ID(EVAS_OBJ_TEXTBLOCK_SUB_ID_TEXT_MARKUP_SET), EO_TYPECHECK(const char *, text)

/**
 * @def evas_obj_textblock_text_markup_get
 * @since 1.8
 *
 * Return the markup of the object.
 *
 * @param[out] markup
 *
 * @see evas_object_textblock_text_markup_get
 */
#define evas_obj_textblock_text_markup_get(markup) EVAS_OBJ_TEXTBLOCK_ID(EVAS_OBJ_TEXTBLOCK_SUB_ID_TEXT_MARKUP_GET), EO_TYPECHECK(const char **, markup)

/**
 * @def evas_obj_textblock_cursor_get
 * @since 1.8
 *
 * Return the object's main cursor.
 *
 * @param[out] cursor
 *
 * @see evas_object_textblock_cursor_get
 */
#define evas_obj_textblock_cursor_get(cursor) EVAS_OBJ_TEXTBLOCK_ID(EVAS_OBJ_TEXTBLOCK_SUB_ID_CURSOR_GET), EO_TYPECHECK(Evas_Textblock_Cursor **, cursor)

/**
 * @def evas_obj_textblock_cursor_new
 * @since 1.8
 *
 * Create a new cursor, associate it to the obj and init it to point
 * to the start of the textblock. Association to the object means the cursor
 * will be updated when the object will change.
 *
 * @param[in] cur
 *
 * @see evas_object_textblock_cursor_new
 */
#define evas_obj_textblock_cursor_new(cur) EVAS_OBJ_TEXTBLOCK_ID(EVAS_OBJ_TEXTBLOCK_SUB_ID_CURSOR_NEW), EO_TYPECHECK(Evas_Textblock_Cursor **, cur)
#define evas_obj_textblock_node_format_list_get(obj, anchor, list) EVAS_OBJ_TEXTBLOCK_ID(EVAS_OBJ_TEXTBLOCK_SUB_ID_NODE_FORMAT_LIST_GET), EO_TYPECHECK(const char *, anchor), EO_TYPECHECK(const Eina_List **, list)
#define evas_obj_textblock_node_format_first_get(format) EVAS_OBJ_TEXTBLOCK_ID(EVAS_OBJ_TEXTBLOCK_SUB_ID_NODE_FORMAT_FIRST_GET), EO_TYPECHECK(const Evas_Object_Textblock_Node_Format **, format)
#define evas_obj_textblock_node_format_last_get(format) EVAS_OBJ_TEXTBLOCK_ID(EVAS_OBJ_TEXTBLOCK_SUB_ID_NODE_FORMAT_LAST_GET), EO_TYPECHECK(const Evas_Object_Textblock_Node_Format **, format)
#define evas_obj_textblock_node_format_remove_pair(n) EVAS_OBJ_TEXTBLOCK_ID(EVAS_OBJ_TEXTBLOCK_SUB_ID_NODE_FORMAT_REMOVE_PAIR), EO_TYPECHECK(Evas_Object_Textblock_Node_Format *, n)

/**
 * @def evas_obj_textblock_line_number_geometry_get
 * @since 1.8
 *
 * Get the geometry of a line number.
 *
 * @param[in] line
 * @param[out] cx
 * @param[out] cy
 * @param[out] cw
 * @param[out] ch
 * @param[out] result
 *
 * @see evas_object_textblock_line_number_geometry_get
 */
#define evas_obj_textblock_line_number_geometry_get(line, cx, cy, cw, ch, result) EVAS_OBJ_TEXTBLOCK_ID(EVAS_OBJ_TEXTBLOCK_SUB_ID_LINE_NUMBER_GEOMETRY_GET), EO_TYPECHECK(int, line), EO_TYPECHECK(Evas_Coord *, cx), EO_TYPECHECK(Evas_Coord *, cy), EO_TYPECHECK(Evas_Coord *, cw), EO_TYPECHECK(Evas_Coord *, ch), EO_TYPECHECK(Eina_Bool *, result)

/**
 * @def evas_obj_textblock_clear
 * @since 1.8
 *
 * Clear the textblock object.
 *
 *
 * @see evas_object_textblock_clear
 */
#define evas_obj_textblock_clear() EVAS_OBJ_TEXTBLOCK_ID(EVAS_OBJ_TEXTBLOCK_SUB_ID_CLEAR)

/**
 * @def evas_obj_textblock_size_formatted_get
 * @since 1.8
 *
 * Get the formatted width and height. This calculates the actual size after restricting
 * the textblock to the current size of the object.
 *
 * @param[out] w
 * @param[out] h
 *
 * @see evas_object_textblock_size_formatted_get
 */
#define evas_obj_textblock_size_formatted_get(w, h) EVAS_OBJ_TEXTBLOCK_ID(EVAS_OBJ_TEXTBLOCK_SUB_ID_SIZE_FORMATTED_GET), EO_TYPECHECK(Evas_Coord *, w), EO_TYPECHECK(Evas_Coord *, h)

/**
 * @def evas_obj_textblock_size_native_get
 * @since 1.8
 *
 * Get the native width and height. This calculates the actual size without taking account
 * the current size of the object.
 *
 * @param[out] w
 * @param[out] h
 *
 * @see evas_object_textblock_size_native_get
 */
#define evas_obj_textblock_size_native_get(w, h) EVAS_OBJ_TEXTBLOCK_ID(EVAS_OBJ_TEXTBLOCK_SUB_ID_SIZE_NATIVE_GET), EO_TYPECHECK(Evas_Coord *, w), EO_TYPECHECK(Evas_Coord *, h)
#define evas_obj_textblock_style_insets_get(l, r, t, b) EVAS_OBJ_TEXTBLOCK_ID(EVAS_OBJ_TEXTBLOCK_SUB_ID_STYLE_INSETS_GET), EO_TYPECHECK(Evas_Coord *, l), EO_TYPECHECK(Evas_Coord *, r), EO_TYPECHECK(Evas_Coord *, t), EO_TYPECHECK(Evas_Coord *, b)

/**
 * Adds a textblock to the given evas.
 * @param   e The given evas.
 * @return  The new textblock object.
 */
EAPI Evas_Object                             *evas_object_textblock_add(Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * Returns the unescaped version of escape.
 * @param escape the string to be escaped
 * @return the unescaped version of escape
 */
EAPI const char                              *evas_textblock_escape_string_get(const char *escape) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Returns the escaped version of the string.
 * @param string to escape
 * @param len_ret the len of the part of the string that was used.
 * @return the escaped string.
 */
EAPI const char                              *evas_textblock_string_escape_get(const char *string, int *len_ret) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Return the unescaped version of the string between start and end.
 *
 * @param escape_start the start of the string.
 * @param escape_end the end of the string.
 * @return the unescaped version of the range
 */
EAPI const char                              *evas_textblock_escape_string_range_get(const char *escape_start, const char *escape_end) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1, 2);

/**
 * Return the plain version of the markup.
 *
 * Works as if you set the markup to a textblock and then retrieve the plain
 * version of the text. i.e: <br> and <\n> will be replaced with \n, &...; with
 * the actual char and etc.
 *
 * @param obj The textblock object to work with. (if @c NULL, tries the
 * default).
 * @param text The markup text (if @c NULL, return @c NULL).
 * @return An allocated plain text version of the markup.
 * @since 1.2
 */
EAPI char                                    *evas_textblock_text_markup_to_utf8(const Evas_Object *obj, const char *text) EINA_WARN_UNUSED_RESULT EINA_MALLOC;

/**
 * Return the markup version of the plain text.
 *
 * Replaces \\n -\> \<br/\> \\t -\> \<tab/\> and etc. Generally needed before you pass
 * plain text to be set in a textblock.
 *
 * @param obj the textblock object to work with (if @c NULL, it just does the
 * default behaviour, i.e with no extra object information).
 * @param text The markup text (if @c NULL, return @c NULL).
 * @return An allocated plain text version of the markup.
 * @since 1.2
 */
EAPI char                                    *evas_textblock_text_utf8_to_markup(const Evas_Object *obj, const char *text) EINA_WARN_UNUSED_RESULT EINA_MALLOC;

/**
 * Creates a new textblock style.
 * @return  The new textblock style.
 */
EAPI Evas_Textblock_Style                    *evas_textblock_style_new(void) EINA_WARN_UNUSED_RESULT EINA_MALLOC;

/**
 * Destroys a textblock style.
 * @param ts The textblock style to free.
 */
EAPI void                                     evas_textblock_style_free(Evas_Textblock_Style *ts) EINA_ARG_NONNULL(1);

/**
 * Sets the style ts to the style passed as text by text.
 * Expected a string consisting of many (or none) tag='format' pairs.
 *
 * @param ts  the style to set.
 * @param text the text to parse - NOT NULL.
 * @return Returns no value.
 */
EAPI void                                     evas_textblock_style_set(Evas_Textblock_Style *ts, const char *text) EINA_ARG_NONNULL(1);

/**
 * Return the text of the style ts.
 * @param ts  the style to get it's text.
 * @return the text of the style or null on error.
 */
EAPI const char                              *evas_textblock_style_get(const Evas_Textblock_Style *ts) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Set the objects style to ts.
 * @param obj the Evas object to set the style to.
 * @param ts  the style to set.
 * @return Returns no value.
 */
EAPI void                                     evas_object_textblock_style_set(Evas_Object *obj, Evas_Textblock_Style *ts) EINA_ARG_NONNULL(1);

/**
 * Return the style of an object.
 * @param obj  the object to get the style from.
 * @return the style of the object.
 */
EAPI const Evas_Textblock_Style              *evas_object_textblock_style_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Push ts to the top of the user style stack.
 *
 * FIXME: API is solid but currently only supports 1 style in the stack.
 *
 * The user style overrides the corresponding elements of the regular style.
 * This is the proper way to do theme overrides in code.
 * @param obj the Evas object to set the style to.
 * @param ts  the style to set.
 * @return Returns no value.
 * @see evas_object_textblock_style_set
 * @since 1.2
 */
EAPI void                                     evas_object_textblock_style_user_push(Evas_Object *obj, Evas_Textblock_Style *ts) EINA_ARG_NONNULL(1);

/**
 * Del the from the top of the user style stack.
 *
 * @param obj  the object to get the style from.
 * @see evas_object_textblock_style_get
 * @since 1.2
 */
EAPI void                                     evas_object_textblock_style_user_pop(Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * Get (don't remove) the style at the top of the user style stack.
 *
 * @param obj  the object to get the style from.
 * @return the style of the object.
 * @see evas_object_textblock_style_get
 * @since 1.2
 */
EAPI const Evas_Textblock_Style              *evas_object_textblock_style_user_peek(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief Set the "replacement character" to use for the given textblock object.
 *
 * @param obj The given textblock object.
 * @param ch The charset name.
 */
EAPI void                                     evas_object_textblock_replace_char_set(Evas_Object *obj, const char *ch) EINA_ARG_NONNULL(1);

/**
 * @brief Get the "replacement character" for given textblock object. Returns
 * @c NULL if no replacement character is in use.
 *
 * @param obj The given textblock object
 * @return Replacement character or @c NULL.
 */
EAPI const char                              *evas_object_textblock_replace_char_get(Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief Sets the vertical alignment of text within the textblock object
 * as a whole.
 *
 * Normally alignment is 0.0 (top of object). Values given should be
 * between 0.0 and 1.0 (1.0 bottom of object, 0.5 being vertically centered
 * etc.).
 *
 * @param obj The given textblock object.
 * @param align A value between @c 0.0 and @c 1.0.
 * @since 1.1
 */
EAPI void                                     evas_object_textblock_valign_set(Evas_Object *obj, double align);

/**
 * @brief Gets the vertical alignment of a textblock
 *
 * @param obj The given textblock object.
 * @return The alignment set for the object.
 * @since 1.1
 */
EAPI double                                   evas_object_textblock_valign_get(const Evas_Object *obj);

/**
 * @brief Sets the BiDi delimiters used in the textblock.
 *
 * BiDi delimiters are use for in-paragraph separation of bidi segments. This
 * is useful for example in recipients fields of e-mail clients where bidi
 * oddities can occur when mixing RTL and LTR.
 *
 * @param obj The given textblock object.
 * @param delim A null terminated string of delimiters, e.g ",|".
 * @since 1.1
 */
EAPI void                                     evas_object_textblock_bidi_delimiters_set(Evas_Object *obj, const char *delim);

/**
 * @brief Gets the BiDi delimiters used in the textblock.
 *
 * BiDi delimiters are use for in-paragraph separation of bidi segments. This
 * is useful for example in recipients fields of e-mail clients where bidi
 * oddities can occur when mixing RTL and LTR.
 *
 * @param obj The given textblock object.
 * @return A null terminated string of delimiters, e.g ",|". If empty, returns
 * @c NULL.
 * @since 1.1
 */
EAPI const char                              *evas_object_textblock_bidi_delimiters_get(const Evas_Object *obj);

/**
 * @brief Sets newline mode. When true, newline character will behave
 * as a paragraph separator.
 *
 * @param obj The given textblock object.
 * @param mode @c EINA_TRUE for legacy mode, @c EINA_FALSE otherwise.
 * @since 1.1
 */
EAPI void                                     evas_object_textblock_legacy_newline_set(Evas_Object *obj, Eina_Bool mode) EINA_ARG_NONNULL(1);

/**
 * @brief Gets newline mode. When true, newline character behaves
 * as a paragraph separator.
 *
 * @param obj The given textblock object.
 * @return @c EINA_TRUE if in legacy mode, @c EINA_FALSE otherwise.
 * @since 1.1
 */
EAPI Eina_Bool                                evas_object_textblock_legacy_newline_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Sets the tetxblock's text to the markup text.
 *
 * @note assumes text does not include the unicode object replacement char (0xFFFC)
 *
 * @param obj  the textblock object.
 * @param text the markup text to use.
 * @return Return no value.
 */
EAPI void                                     evas_object_textblock_text_markup_set(Evas_Object *obj, const char *text) EINA_ARG_NONNULL(1);

/**
 * Prepends markup to the cursor cur.
 *
 * @note assumes text does not include the unicode object replacement char (0xFFFC)
 *
 * @param cur  the cursor to prepend to.
 * @param text the markup text to prepend.
 * @return Return no value.
 */
EAPI void                                     evas_object_textblock_text_markup_prepend(Evas_Textblock_Cursor *cur, const char *text) EINA_ARG_NONNULL(1, 2);

/**
 * Return the markup of the object.
 *
 * @param obj the Evas object.
 * @return the markup text of the object.
 */
EAPI const char                              *evas_object_textblock_text_markup_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * Return the object's main cursor.
 *
 * @param obj the object.
 * @return The @p obj's main cursor.
 */
EAPI Evas_Textblock_Cursor                   *evas_object_textblock_cursor_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

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
EAPI Evas_Textblock_Cursor                   *evas_object_textblock_cursor_new(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * Free the cursor and unassociate it from the object.
 * @note do not use it to free unassociated cursors.
 *
 * @param cur the cursor to free.
 * @return Returns no value.
 */
EAPI void                                     evas_textblock_cursor_free(Evas_Textblock_Cursor *cur) EINA_ARG_NONNULL(1);

/**
 * Sets the cursor to the start of the first text node.
 *
 * @param cur the cursor to update.
 * @return Returns no value.
 */
EAPI void                                     evas_textblock_cursor_paragraph_first(Evas_Textblock_Cursor *cur) EINA_ARG_NONNULL(1);

/**
 * sets the cursor to the end of the last text node.
 *
 * @param cur the cursor to set.
 * @return Returns no value.
 */
EAPI void                                     evas_textblock_cursor_paragraph_last(Evas_Textblock_Cursor *cur) EINA_ARG_NONNULL(1);

/**
 * Advances to the start of the next text node
 *
 * @param cur the cursor to update
 * @return @c EINA_TRUE if it managed to advance a paragraph, @c EINA_FALSE
 * otherwise.
 */
EAPI Eina_Bool                                evas_textblock_cursor_paragraph_next(Evas_Textblock_Cursor *cur) EINA_ARG_NONNULL(1);

/**
 * Advances to the end of the previous text node
 *
 * @param cur the cursor to update
 * @return @c EINA_TRUE if it managed to advance a paragraph, @c EINA_FALSE
 * otherwise.
 */
EAPI Eina_Bool                                evas_textblock_cursor_paragraph_prev(Evas_Textblock_Cursor *cur) EINA_ARG_NONNULL(1);

/**
 * Returns the
 *
 * @param obj The evas, must not be @c NULL.
 * @param anchor the anchor name to get
 * @return Returns the list format node corresponding to the anchor, may be null if there are none.
 */
EAPI const Eina_List                         *evas_textblock_node_format_list_get(const Evas_Object *obj, const char *anchor) EINA_ARG_NONNULL(1, 2);

/**
 * Returns the first format node.
 *
 * @param obj The evas, must not be @c NULL.
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
 * Remove a format node and it's match. i.e, removes a \<tag\> \</tag\> pair.
 * Assumes the node is the first part of \<tag\> i.e, this won't work if
 * n is a closing tag.
 *
 * @param obj the Evas object of the textblock - not null.
 * @param n the current format node - not null.
 */
EAPI void                                     evas_textblock_node_format_remove_pair(Evas_Object *obj, Evas_Object_Textblock_Node_Format *n) EINA_ARG_NONNULL(1, 2);

/**
 * Sets the cursor to point to the place where format points to.
 *
 * @param cur the cursor to update.
 * @param n the format node to update according.
 * @deprecated duplicate of evas_textblock_cursor_at_format_set
 */
EAPI void                                     evas_textblock_cursor_set_at_format(Evas_Textblock_Cursor *cur, const Evas_Object_Textblock_Node_Format *n) EINA_ARG_NONNULL(1, 2);

/**
 * Return the format node at the position pointed by cur.
 *
 * @param cur the position to look at.
 * @return the format node if found, @c NULL otherwise.
 * @see evas_textblock_cursor_format_is_visible_get()
 */
EAPI const Evas_Object_Textblock_Node_Format *evas_textblock_cursor_format_get(const Evas_Textblock_Cursor *cur) EINA_ARG_NONNULL(1);

/**
 * Get the text format representation of the format node.
 *
 * @param fnode the format node.
 * @return the textual format of the format node.
 */
EAPI const char                              *evas_textblock_node_format_text_get(const Evas_Object_Textblock_Node_Format *fnode) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Set the cursor to point to the position of fmt.
 *
 * @param cur the cursor to update
 * @param fmt the format to update according to.
 */
EAPI void                                     evas_textblock_cursor_at_format_set(Evas_Textblock_Cursor *cur, const Evas_Object_Textblock_Node_Format *fmt) EINA_ARG_NONNULL(1, 2);

/**
 * Check if the current cursor position is a visible format. This way is more
 * efficient than evas_textblock_cursor_format_get() to check for the existence
 * of a visible format.
 *
 * @param cur the cursor to look at.
 * @return @c EINA_TRUE if the cursor points to a visible format, @c EINA_FALSE
 * otherwise.
 * @see evas_textblock_cursor_format_get()
 */
EAPI Eina_Bool                                evas_textblock_cursor_format_is_visible_get(const Evas_Textblock_Cursor *cur) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Advances to the next format node
 *
 * @param cur the cursor to be updated.
 * @return @c EINA_TRUE on success @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool                                evas_textblock_cursor_format_next(Evas_Textblock_Cursor *cur) EINA_ARG_NONNULL(1);

/**
 * Advances to the previous format node.
 *
 * @param cur the cursor to update.
 * @return @c EINA_TRUE on success @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool                                evas_textblock_cursor_format_prev(Evas_Textblock_Cursor *cur) EINA_ARG_NONNULL(1);

/**
 * Returns true if the cursor points to a format.
 *
 * @param cur the cursor to check.
 * @return  @c EINA_TRUE if a cursor points to a format @c EINA_FALSE
 * otherwise.
 */
EAPI Eina_Bool                                evas_textblock_cursor_is_format(const Evas_Textblock_Cursor *cur) EINA_ARG_NONNULL(1);

/**
 * Advances 1 char forward.
 *
 * @param cur the cursor to advance.
 * @return @c EINA_TRUE on success @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool                                evas_textblock_cursor_char_next(Evas_Textblock_Cursor *cur) EINA_ARG_NONNULL(1);

/**
 * Advances 1 char backward.
 *
 * @param cur the cursor to advance.
 * @return @c EINA_TRUE on success @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool                                evas_textblock_cursor_char_prev(Evas_Textblock_Cursor *cur) EINA_ARG_NONNULL(1);

/**
 * Moves the cursor to the start of the word under the cursor.
 *
 * @param cur the cursor to move.
 * @return @c EINA_TRUE on success @c EINA_FALSE otherwise.
 * @since 1.2
 */
EAPI Eina_Bool                                evas_textblock_cursor_word_start(Evas_Textblock_Cursor *cur) EINA_ARG_NONNULL(1);

/**
 * Moves the cursor to the end of the word under the cursor.
 *
 * @param cur the cursor to move.
 * @return @c EINA_TRUE on success @c EINA_FALSE otherwise.
 * @since 1.2
 */
EAPI Eina_Bool                                evas_textblock_cursor_word_end(Evas_Textblock_Cursor *cur) EINA_ARG_NONNULL(1);

/**
 * Go to the first char in the node the cursor is pointing on.
 *
 * @param cur the cursor to update.
 * @return Returns no value.
 */
EAPI void                                     evas_textblock_cursor_paragraph_char_first(Evas_Textblock_Cursor *cur) EINA_ARG_NONNULL(1);

/**
 * Go to the last char in a text node.
 *
 * @param cur the cursor to update.
 * @return Returns no value.
 */
EAPI void                                     evas_textblock_cursor_paragraph_char_last(Evas_Textblock_Cursor *cur) EINA_ARG_NONNULL(1);

/**
 * Go to the start of the current line
 *
 * @param cur the cursor to update.
 * @return Returns no value.
 */
EAPI void                                     evas_textblock_cursor_line_char_first(Evas_Textblock_Cursor *cur) EINA_ARG_NONNULL(1);

/**
 * Go to the end of the current line.
 *
 * @param cur the cursor to update.
 * @return Returns no value.
 */
EAPI void                                     evas_textblock_cursor_line_char_last(Evas_Textblock_Cursor *cur) EINA_ARG_NONNULL(1);

/**
 * Return the current cursor pos.
 *
 * @param cur the cursor to take the position from.
 * @return the position or -1 on error
 */
EAPI int                                      evas_textblock_cursor_pos_get(const Evas_Textblock_Cursor *cur) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Set the cursor pos.
 *
 * @param cur the cursor to be set.
 * @param pos the pos to set.
 */
EAPI void                                     evas_textblock_cursor_pos_set(Evas_Textblock_Cursor *cur, int pos) EINA_ARG_NONNULL(1);

/**
 * Go to the start of the line passed
 *
 * @param cur cursor to update.
 * @param line numer to set.
 * @return @c EINA_TRUE on success, @c EINA_FALSE on error.
 */
EAPI Eina_Bool                                evas_textblock_cursor_line_set(Evas_Textblock_Cursor *cur, int line) EINA_ARG_NONNULL(1);

/**
 * Compare two cursors.
 *
 * @param cur1 the first cursor.
 * @param cur2 the second cursor.
 * @return -1 if cur1 < cur2, 0 if cur1 == cur2 and 1 otherwise.
 */
EAPI int                                      evas_textblock_cursor_compare(const Evas_Textblock_Cursor *cur1, const Evas_Textblock_Cursor *cur2) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1, 2);

/**
 * Make cur_dest point to the same place as cur. Does not work if they don't
 * point to the same object.
 *
 * @param cur the source cursor.
 * @param cur_dest destination cursor.
 * @return Returns no value.
 */
EAPI void                                     evas_textblock_cursor_copy(const Evas_Textblock_Cursor *cur, Evas_Textblock_Cursor *cur_dest) EINA_ARG_NONNULL(1, 2);

/**
 * Adds text to the current cursor position and set the cursor to *before*
 * the start of the text just added.
 *
 * @param cur the cursor to where to add text at.
 * @param text the text to add.
 * @return Returns the len of the text added.
 * @see evas_textblock_cursor_text_prepend()
 */
EAPI int                                      evas_textblock_cursor_text_append(Evas_Textblock_Cursor *cur, const char *text) EINA_ARG_NONNULL(1, 2);

/**
 * Adds text to the current cursor position and set the cursor to *after*
 * the start of the text just added.
 *
 * @param cur the cursor to where to add text at.
 * @param text the text to add.
 * @return Returns the len of the text added.
 * @see evas_textblock_cursor_text_append()
 */
EAPI int                                      evas_textblock_cursor_text_prepend(Evas_Textblock_Cursor *cur, const char *text) EINA_ARG_NONNULL(1, 2);

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
 * @return @c EINA_TRUE if the cursor points to the terminating null, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool                                evas_textblock_cursor_format_append(Evas_Textblock_Cursor *cur, const char *format) EINA_ARG_NONNULL(1, 2);

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
EAPI Eina_Bool                                evas_textblock_cursor_format_prepend(Evas_Textblock_Cursor *cur, const char *format) EINA_ARG_NONNULL(1, 2);

/**
 * Delete the character at the location of the cursor. If there's a format
 * pointing to this position, delete it as well.
 *
 * @param cur the cursor pointing to the current location.
 * @return Returns no value.
 */
EAPI void                                     evas_textblock_cursor_char_delete(Evas_Textblock_Cursor *cur) EINA_ARG_NONNULL(1);

/**
 * Delete the range between cur1 and cur2.
 *
 * @param cur1 one side of the range.
 * @param cur2 the second side of the range
 * @return Returns no value.
 */
EAPI void                                     evas_textblock_cursor_range_delete(Evas_Textblock_Cursor *cur1, Evas_Textblock_Cursor *cur2) EINA_ARG_NONNULL(1, 2);

/**
 * Return the text of the paragraph cur points to - returns the text in markup.
 *
 * @param cur the cursor pointing to the paragraph.
 * @return the text on success, @c NULL otherwise.
 */
EAPI const char                              *evas_textblock_cursor_paragraph_text_get(const Evas_Textblock_Cursor *cur) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Return the length of the paragraph, cheaper the eina_unicode_strlen()
 *
 * @param cur the position of the paragraph.
 * @return the length of the paragraph on success, -1 otehrwise.
 */
EAPI int                                      evas_textblock_cursor_paragraph_text_length_get(const Evas_Textblock_Cursor *cur) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Return the currently visible range.
 *
 * @param start the start of the range.
 * @param end the end of the range.
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise.
 * @since 1.1
 */
EAPI Eina_Bool                                evas_textblock_cursor_visible_range_get(Evas_Textblock_Cursor *start, Evas_Textblock_Cursor *end) EINA_ARG_NONNULL(1, 2);

/**
 * Return the format nodes in the range between cur1 and cur2.
 *
 * @param cur1 one side of the range.
 * @param cur2 the other side of the range
 * @return the foramt nodes in the range. You have to free it.
 * @since 1.1
 */
EAPI Eina_List                               *evas_textblock_cursor_range_formats_get(const Evas_Textblock_Cursor *cur1, const Evas_Textblock_Cursor *cur2) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1, 2);

/**
 * Return the text in the range between cur1 and cur2
 *
 * @param cur1 one side of the range.
 * @param cur2 the other side of the range
 * @param format The form on which to return the text. Markup - in textblock markup. Plain - UTF8.
 * @return the text in the range
 * @see elm_entry_markup_to_utf8()
 */
EAPI char                                    *evas_textblock_cursor_range_text_get(const Evas_Textblock_Cursor *cur1, const Evas_Textblock_Cursor *cur2, Evas_Textblock_Text_Type format) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1, 2);

/**
 * Return the content of the cursor.
 *
 * Free the returned string pointer when done (if it is not NULL).
 *
 * @param cur the cursor
 * @return the text in the range, terminated by a nul byte (may be utf8).
 */
EAPI char                                    *evas_textblock_cursor_content_get(const Evas_Textblock_Cursor *cur) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * Returns the geometry of the cursor. Depends on the type of cursor requested.
 * This should be used instead of char_geometry_get because there are weird
 * special cases with BiDi text.
 * in '_' cursor mode (i.e a line below the char) it's the same as char_geometry
 * get, except for the case of the last char of a line which depends on the
 * paragraph direction.
 *
 * in '|' cursor mode (i.e a line between two chars) it is very variable.
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
EAPI int                                      evas_textblock_cursor_geometry_get(const Evas_Textblock_Cursor *cur, Evas_Coord *cx, Evas_Coord *cy, Evas_Coord *cw, Evas_Coord *ch, Evas_BiDi_Direction *dir, Evas_Textblock_Cursor_Type ctype) EINA_ARG_NONNULL(1);

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
EAPI int                                      evas_textblock_cursor_char_geometry_get(const Evas_Textblock_Cursor *cur, Evas_Coord *cx, Evas_Coord *cy, Evas_Coord *cw, Evas_Coord *ch) EINA_ARG_NONNULL(1);

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
EAPI int                                      evas_textblock_cursor_pen_geometry_get(const Evas_Textblock_Cursor *cur, Evas_Coord *cpen_x, Evas_Coord *cy, Evas_Coord *cadv, Evas_Coord *ch) EINA_ARG_NONNULL(1);

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
EAPI int                                      evas_textblock_cursor_line_geometry_get(const Evas_Textblock_Cursor *cur, Evas_Coord *cx, Evas_Coord *cy, Evas_Coord *cw, Evas_Coord *ch) EINA_ARG_NONNULL(1);

/**
 * Set the position of the cursor according to the X and Y coordinates.
 *
 * @param cur the cursor to set.
 * @param x coord to set by.
 * @param y coord to set by.
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool                                evas_textblock_cursor_char_coord_set(Evas_Textblock_Cursor *cur, Evas_Coord x, Evas_Coord y) EINA_ARG_NONNULL(1);

/**
 * Set the cursor position according to the y coord.
 *
 * @param cur the cur to be set.
 * @param y the coord to set by.
 * @return the line number found, -1 on error.
 */
EAPI int                                      evas_textblock_cursor_line_coord_set(Evas_Textblock_Cursor *cur, Evas_Coord y) EINA_ARG_NONNULL(1);

/**
 * Get the geometry of a range.
 *
 * @param cur1 one side of the range.
 * @param cur2 other side of the range.
 * @return a list of Rectangles representing the geometry of the range.
 */
EAPI Eina_List                               *evas_textblock_cursor_range_geometry_get(const Evas_Textblock_Cursor *cur1, const Evas_Textblock_Cursor *cur2) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1, 2);
EAPI Eina_Bool                                evas_textblock_cursor_format_item_geometry_get(const Evas_Textblock_Cursor *cur, Evas_Coord *cx, Evas_Coord *cy, Evas_Coord *cw, Evas_Coord *ch) EINA_ARG_NONNULL(1);

/**
 * Checks if the cursor points to the end of the line.
 *
 * @param cur the cursor to check.
 * @return @c EINA_TRUE if true, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool                                evas_textblock_cursor_eol_get(const Evas_Textblock_Cursor *cur) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Get the geometry of a line number.
 *
 * @param obj the object.
 * @param line the line number.
 * @param cx x coord of the line.
 * @param cy y coord of the line.
 * @param cw w coord of the line.
 * @param ch h coord of the line.
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool                                evas_object_textblock_line_number_geometry_get(const Evas_Object *obj, int line, Evas_Coord *cx, Evas_Coord *cy, Evas_Coord *cw, Evas_Coord *ch) EINA_ARG_NONNULL(1);

/**
 * Clear the textblock object.
 * @note Does *NOT* free the Evas object itself.
 *
 * @param obj the object to clear.
 * @return nothing.
 */
EAPI void                                     evas_object_textblock_clear(Evas_Object *obj) EINA_ARG_NONNULL(1);

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
 * @param w the width of the object.
 * @param h the height of the object
 * @return Returns no value.
 * @see evas_object_textblock_size_native_get
 */
EAPI void                                     evas_object_textblock_size_formatted_get(const Evas_Object *obj, Evas_Coord *w, Evas_Coord *h) EINA_ARG_NONNULL(1);

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
 * @param w the width returned
 * @param h the height returned
 * @return Returns no value.
 */
EAPI void                                     evas_object_textblock_size_native_get(const Evas_Object *obj, Evas_Coord *w, Evas_Coord *h) EINA_ARG_NONNULL(1);
EAPI void                                     evas_object_textblock_style_insets_get(const Evas_Object *obj, Evas_Coord *l, Evas_Coord *r, Evas_Coord *t, Evas_Coord *b) EINA_ARG_NONNULL(1);

/**
 * @}
 */


/**
 * @defgroup Evas_Object_Textgrid Textgrid Object Functions
 *
 * @todo put here some usage examples
 *
 * @since 1.7
 *
 * @ingroup Evas_Object_Specific
 *
 * @{
 */

#define EVAS_OBJ_TEXTGRID_CLASS evas_object_textgrid_class_get()

const Eo_Class *evas_object_textgrid_class_get(void) EINA_CONST;

extern EAPI Eo_Op EVAS_OBJ_TEXTGRID_BASE_ID;

enum
{
   EVAS_OBJ_TEXTGRID_SUB_ID_SIZE_SET,
   EVAS_OBJ_TEXTGRID_SUB_ID_SIZE_GET,
   EVAS_OBJ_TEXTGRID_SUB_ID_FONT_SOURCE_SET,
   EVAS_OBJ_TEXTGRID_SUB_ID_FONT_SOURCE_GET,
   EVAS_OBJ_TEXTGRID_SUB_ID_FONT_SET,
   EVAS_OBJ_TEXTGRID_SUB_ID_FONT_GET,
   EVAS_OBJ_TEXTGRID_SUB_ID_CELL_SIZE_GET,
   EVAS_OBJ_TEXTGRID_SUB_ID_PALETTE_SET,
   EVAS_OBJ_TEXTGRID_SUB_ID_PALETTE_GET,
   EVAS_OBJ_TEXTGRID_SUB_ID_SUPPORTED_FONT_STYLES_SET,
   EVAS_OBJ_TEXTGRID_SUB_ID_SUPPORTED_FONT_STYLES_GET,
   EVAS_OBJ_TEXTGRID_SUB_ID_CELLROW_SET,
   EVAS_OBJ_TEXTGRID_SUB_ID_CELLROW_GET,
   EVAS_OBJ_TEXTGRID_SUB_ID_UPDATE_ADD,
   EVAS_OBJ_TEXTGRID_SUB_ID_LAST
};

#define EVAS_OBJ_TEXTGRID_ID(sub_id) (EVAS_OBJ_TEXTGRID_BASE_ID + sub_id)


/**
 * @def evas_obj_textgrid_size_set
 * @since 1.8
 *
 * Set the size of the textgrid object.
 *
 * @param[in] w
 * @param[in] h
 *
 * @see evas_object_textgrid_size_set
 */
#define evas_obj_textgrid_size_set(w, h) EVAS_OBJ_TEXTGRID_ID(EVAS_OBJ_TEXTGRID_SUB_ID_SIZE_SET), EO_TYPECHECK(int, w), EO_TYPECHECK(int, h)

/**
 * @def evas_obj_textgrid_size_get
 * @since 1.8
 *
 * Get the size of the textgrid object.
 *
 * @param[out] w
 * @param[out] h
 *
 * @see evas_object_textgrid_size_get
 */
#define evas_obj_textgrid_size_get(w, h) EVAS_OBJ_TEXTGRID_ID(EVAS_OBJ_TEXTGRID_SUB_ID_SIZE_GET), EO_TYPECHECK(int *, w), EO_TYPECHECK(int *, h)

/**
 * @def evas_obj_textgrid_font_source_set
 * @since 1.8
 *
 * Set the font (source) file to be used on a given textgrid object.
 *
 * @param[in] font_source
 *
 * @see evas_object_textgrid_font_source_set
 */
#define evas_obj_textgrid_font_source_set(font_source) EVAS_OBJ_TEXTGRID_ID(EVAS_OBJ_TEXTGRID_SUB_ID_FONT_SOURCE_SET), EO_TYPECHECK(const char *, font_source)

/**
 * @def evas_obj_textgrid_font_source_get
 * @since 1.8
 *
 * Get the font file's path which is being used on a given textgrid object.
 *
 * @param[out] ret
 *
 * @see evas_object_textgrid_font_source_get
 */
#define evas_obj_textgrid_font_source_get(ret) EVAS_OBJ_TEXTGRID_ID(EVAS_OBJ_TEXTGRID_SUB_ID_FONT_SOURCE_GET), EO_TYPECHECK(const char **, ret)

/**
 * @def evas_obj_textgrid_font_set
 * @since 1.8
 *
 * Set the font family and size on a given textgrid object.
 *
 * @param[in] font_name
 * @param[in] font_size
 *
 * @see evas_object_textgrid_font_set
 */
#define evas_obj_textgrid_font_set(font_name, font_size) EVAS_OBJ_TEXTGRID_ID(EVAS_OBJ_TEXTGRID_SUB_ID_FONT_SET), EO_TYPECHECK(const char *, font_name), EO_TYPECHECK(Evas_Font_Size, font_size)

/**
 * @def evas_obj_textgrid_font_get
 * @since 1.8
 *
 * Retrieve the font family and size in use on a given textgrid object.
 *
 * @param[out] font_name
 * @param[out] font_size
 *
 * @see evas_object_textgrid_font_get
 */
#define evas_obj_textgrid_font_get(font_name, font_size) EVAS_OBJ_TEXTGRID_ID(EVAS_OBJ_TEXTGRID_SUB_ID_FONT_GET), EO_TYPECHECK(const char **, font_name), EO_TYPECHECK(Evas_Font_Size *, font_size)

/**
 * @def evas_obj_textgrid_cell_size_get
 * @since 1.8
 *
 * Retrieve the size of a cell of the given textgrid object in pixels.
 *
 * @param[out] width
 * @param[out] height
 *
 * @see evas_object_textgrid_cell_size_get
 */
#define evas_obj_textgrid_cell_size_get(width, height) EVAS_OBJ_TEXTGRID_ID(EVAS_OBJ_TEXTGRID_SUB_ID_CELL_SIZE_GET), EO_TYPECHECK(int *, width), EO_TYPECHECK(int *, height)

/**
 * @def evas_obj_textgrid_palette_set
 * @since 1.8
 *
 * The set color to the given palette at the given index of the given textgrid object.
 *
 * @param[in] pal
 * @param[in] idx
 * @param[in] r
 * @param[in] g
 * @param[in] b
 * @param[in] a
 *
 * @see evas_object_textgrid_palette_set
 */
#define evas_obj_textgrid_palette_set(pal, idx, r, g, b, a) EVAS_OBJ_TEXTGRID_ID(EVAS_OBJ_TEXTGRID_SUB_ID_PALETTE_SET), EO_TYPECHECK(Evas_Textgrid_Palette, pal), EO_TYPECHECK(int, idx), EO_TYPECHECK(int, r), EO_TYPECHECK(int, g), EO_TYPECHECK(int, b), EO_TYPECHECK(int, a)

/**
 * @def evas_obj_textgrid_palette_get
 * @since 1.8
 *
 * The retrieve color to the given palette at the given index of the given textgrid object.
 *
 * @param[out] pal
 * @param[out] idx
 * @param[out] r
 * @param[out] g
 * @param[out] b
 * @param[out] a
 *
 * @see evas_object_textgrid_palette_get
 */
#define evas_obj_textgrid_palette_get(pal, idx, r, g, b, a) EVAS_OBJ_TEXTGRID_ID(EVAS_OBJ_TEXTGRID_SUB_ID_PALETTE_GET), EO_TYPECHECK(Evas_Textgrid_Palette, pal), EO_TYPECHECK(int, idx), EO_TYPECHECK(int *, r), EO_TYPECHECK(int *, g), EO_TYPECHECK(int *, b), EO_TYPECHECK(int *, a)
#define evas_obj_textgrid_supported_font_styles_set(styles) EVAS_OBJ_TEXTGRID_ID(EVAS_OBJ_TEXTGRID_SUB_ID_SUPPORTED_FONT_STYLES_SET), EO_TYPECHECK(Evas_Textgrid_Font_Style, styles)
#define evas_obj_textgrid_supported_font_styles_get(ret) EVAS_OBJ_TEXTGRID_ID(EVAS_OBJ_TEXTGRID_SUB_ID_SUPPORTED_FONT_STYLES_GET), EO_TYPECHECK(Evas_Textgrid_Font_Style *, ret)

/**
 * @def evas_obj_textgrid_cellrow_set
 * @since 1.8
 *
 * Set the string at the given row of the given textgrid object.
 *
 * @param[in] y
 * @param[in] row
 *
 * @see evas_object_textgrid_cellrow_set
 */
#define evas_obj_textgrid_cellrow_set(y, row) EVAS_OBJ_TEXTGRID_ID(EVAS_OBJ_TEXTGRID_SUB_ID_CELLROW_SET), EO_TYPECHECK(int, y), EO_TYPECHECK(const Evas_Textgrid_Cell *, row)

/**
 * @def evas_obj_textgrid_cellrow_get
 * @since 1.8
 *
 * Get the string at the given row of the given textgrid object.
 *
 * @param[in] y
 * @param[out] ret
 *
 * @see evas_object_textgrid_cellrow_get
 */
#define evas_obj_textgrid_cellrow_get(y, ret) EVAS_OBJ_TEXTGRID_ID(EVAS_OBJ_TEXTGRID_SUB_ID_CELLROW_GET), EO_TYPECHECK(int, y), EO_TYPECHECK(Evas_Textgrid_Cell **, ret)

/**
 * @def evas_obj_textgrid_update_add
 * @since 1.8
 *
 * Indicate for evas that part of a textgrid region (cells) has been updated.
 *
 * @param[in] x
 * @param[in] y
 * @param[in] w
 * @param[in] h
 *
 * @see evas_object_textgrid_update_add
 */
#define evas_obj_textgrid_update_add(x, y, w, h) EVAS_OBJ_TEXTGRID_ID(EVAS_OBJ_TEXTGRID_SUB_ID_UPDATE_ADD), EO_TYPECHECK(int, x), EO_TYPECHECK(int, y), EO_TYPECHECK(int, w), EO_TYPECHECK(int, h)

/**
 * @typedef Evas_Textgrid_Palette
 *
 * The palette to use for the forgraound and background colors.
 *
 * @since 1.7
 */
typedef enum
{
   EVAS_TEXTGRID_PALETTE_NONE,     /**< No palette is used */
   EVAS_TEXTGRID_PALETTE_STANDARD, /**< standard palette (around 16 colors) */
   EVAS_TEXTGRID_PALETTE_EXTENDED, /**< extended palette (at max 256 colors) */
   EVAS_TEXTGRID_PALETTE_LAST      /**< ignore it */
} Evas_Textgrid_Palette;

/**
 * @typedef Evas_Textgrid_Font_Style
 *
 * The style to give to each character of the grid.
 *
 * @since 1.7
 */
typedef enum
{
   EVAS_TEXTGRID_FONT_STYLE_NORMAL = (1 << 0), /**< Normal style */
   EVAS_TEXTGRID_FONT_STYLE_BOLD   = (1 << 1), /**< Bold style */
   EVAS_TEXTGRID_FONT_STYLE_ITALIC = (1 << 2)  /**< Oblique style */
} Evas_Textgrid_Font_Style;

/**
 * @typedef Evas_Textgrid_Cell
 *
 * The values that describes each cell.
 *
 * @since 1.7
 */
typedef struct _Evas_Textgrid_Cell Evas_Textgrid_Cell;

/**
 * @struct _Evas_Textgrid_Cell
 *
 * The values that describes each cell.
 *
 * @since 1.7
 */
struct _Evas_Textgrid_Cell
{
   Eina_Unicode   codepoint;         /**< the UNICODE value of the character */
   unsigned char  fg;                /**< the index of the palette for the foreground color */
   unsigned char  bg;                /**< the index of the palette for the background color */
   unsigned short bold          : 1; /**< whether the character is bold */
   unsigned short italic        : 1; /**< whether the character is oblique */
   unsigned short underline     : 1; /**< whether the character is underlined */
   unsigned short strikethrough : 1; /**< whether the character is strikethrough'ed */
   unsigned short fg_extended   : 1; /**< whether the extended palette is used for the foreground color */
   unsigned short bg_extended   : 1; /**< whether the extended palette is used for the background color */
   unsigned short double_width  : 1; /**< if the codepoint is merged with the following cell to the right visually (cells must be in pairs with 2nd cell being a duplicate in all ways except codepoint is 0) */
};

/**
 * @brief Add a textgrid to the given Evas.
 *
 * @param e The given evas.
 * @return The new textgrid object.
 *
 * This function adds a new textgrid object to the Evas @p e and returns the object.
 *
 * @since 1.7
 */
EAPI Evas_Object *evas_object_textgrid_add(Evas *e);

/**
 * @brief Set the size of the textgrid object.
 *
 * @param obj The textgrid object.
 * @param w The number of columns (width in cells) of the grid.
 * @param h The number of rows (height in cells) of the grid.
 *
 * This function sets the number of lines @p h and the number
 * of columns @p w to the textgrid object @p obj. If
 * @p w or @p h are less or equal than 0, this
 * functiond does nothing.
 *
 * @since 1.7
 */
EAPI void evas_object_textgrid_size_set(Evas_Object *obj, int w, int h);

/**
 * @brief Get the size of the textgrid object.
 *
 * @param obj The textgrid object.
 * @param w The number of columns of the grid.
 * @param h The number of rows of the grid.
 *
 * This function retrieves the number of lines in the buffer @p
 * h and the number of columns in the buffer @p w of
 * the textgrid object @p obj. @p w or @p h can be
 * @c NULL. On error, their value is 0.
 *
 * @since 1.7
 */
EAPI void evas_object_textgrid_size_get(const Evas_Object *obj, int *w, int *h);

/**
 * @brief Set the font (source) file to be used on a given textgrid object.
 *
 * @param obj The textgrid object to set font for.
 * @param font_source The font file's path.
 *
 * This function allows the font file @p font_source to be explicitly
 * set for the textgrid object @p obj, overriding system lookup, which
 * will first occur in the given file's contents. If @font_source is
 * @c NULL or is an empty string, or the same font_source has already
 * been set, or on error, this function does nothing.
 *
 * @see evas_object_textgrid_font_get()
 * @see evas_object_textgrid_font_set()
 * @see evas_object_textgrid_font_source_get()
 *
 * @since 1.7
 */
EAPI void evas_object_textgrid_font_source_set(Evas_Object *obj, const char *font_source);

/**
 * @brief Get the font file's path which is being used on a given textgrid object.
 *
 * @param obj The textgrid object to set font for.
 * @return The font file's path.
 *
 * This function returns the font source path of the textgrid object
 * @p obj. If the font source path has not been set, or on error,
 * @c NULL is returned.
 *
 * @see evas_object_textgrid_font_get()
 * @see evas_object_textgrid_font_set()
 * @see evas_object_textgrid_font_source_set()
 *
 * @since 1.7
 */
EAPI const char *evas_object_textgrid_font_source_get(const Evas_Object *obj);

/**
 * @brief Set the font family and size on a given textgrid object.
 *
 * @param obj The textgrid object to set font for.
 * @param font_name The font (family) name.
 * @param font_size The font size, in points.
 *
 * This function allows the font name @p font_name and size
 * @p font_size of the textgrid object @p obj to be set. The @p font_name
 * string has to follow fontconfig's convention on naming fonts, as
 * it's the underlying library used to query system fonts by Evas (see
 * the @c fc-list command's output, on your system, to get an
 * idea). It also has to be a monospace font. If @p font_name is
 * @c NULL, or if it is an empty string, or if @p font_size is less or
 * equal than 0, or on error, this function does nothing.
 *
 * @see evas_object_textgrid_font_get()
 * @see evas_object_textgrid_font_source_set()
 * @see evas_object_textgrid_font_source_get()
 *
 * @since 1.7
 */
EAPI void evas_object_textgrid_font_set(Evas_Object *obj, const char *font_name, Evas_Font_Size font_size);

/**
 * @brief Retrieve the font family and size in use on a given textgrid object.
 *
 * @param obj The textgrid object to query for font information.
 * @param font_name A pointer to the location to store the font name in.
 * @param font_size A pointer to the location to store the font size in.
 *
 * This function allows the font name and size of a textgrid object
 * @p obj to be queried and stored respectively in the buffers
 * @p font_name and @p font_size. Be aware that the font name string is
 * still owned by Evas and should @b not have free() called on it by
 * the caller of the function. On error, the font name is the empty
 * string and the font size is 0. @p font_name and @p font_source can
 * be @c NULL.
 *
 * @see evas_object_textgrid_font_set()
 * @see evas_object_textgrid_font_source_set()
 * @see evas_object_textgrid_font_source_get()
 *
 * @since 1.7
 */
EAPI void evas_object_textgrid_font_get(const Evas_Object *obj, const char **font_name, Evas_Font_Size *font_size);

/**
 * @brief Retrieve the size of a cell of the given textgrid object in pixels.
 *
 * @param obj The textgrid object to query for font information.
 * @param width A pointer to the location to store the width in pixels of a cell.
 * @param height A pointer to the location to store the height in
 * pixels of a cell.
 *
 * This functions retrieves the width and height, in pixels, of a cell
 * of the textgrid object @p obj and store them respectively in the
 * buffers @p width and @p height. Their value depends on the
 * monospace font used for the textgrid object, as well as the
 * style. @p width and @p height can be @c NULL. On error, they are
 * set to 0.
 *
 * @see evas_object_textgrid_font_set()
 * @see evas_object_textgrid_supported_font_styles_set()
 *
 * @since 1.7
 */
EAPI void evas_object_textgrid_cell_size_get(const Evas_Object *obj, Evas_Coord *w, Evas_Coord *h);

/**
 * @brief The set color to the given palette at the given index of the given textgrid object.
 *
 * @param obj The textgrid object to query for font information.
 * @param pal The type of the palette to set the color.
 * @param idx The index of the paletter to wich the color is stored.
 * @param r The red component of the color.
 * @param g The green component of the color.
 * @param b The blue component of the color.
 * @param a The alpha component of the color.
 *
 * This function sets the color for the palette of type @p pal at the
 * index @p idx of the textgrid object @p obj. The ARGB components are
 * given by @p r, @p g, @p b and @p a. This color can be used when
 * setting the #Evas_Textgrid_Cell structure. The components must set
 * a pre-multiplied color. If pal is #EVAS_TEXTGRID_PALETTE_NONE or
 * #EVAS_TEXTGRID_PALETTE_LAST, or if @p idx is not between 0 and 255,
 * or on error, this function does nothing. The color components are
 * clamped between 0 and 255. If @p idx is greater than the latest set
 * color, the colors between this last index and @p idx - 1 are set to
 * black (0, 0, 0, 0).
 *
 * @see evas_object_textgrid_palette_get()
 *
 * @since 1.7
 */
EAPI void evas_object_textgrid_palette_set(Evas_Object *obj, Evas_Textgrid_Palette pal, int idx, int r, int g, int b, int a);

/**
 * @brief The retrieve color to the given palette at the given index of the given textgrid object.
 *
 * @param obj The textgrid object to query for font information.
 * @param pal The type of the palette to set the color.
 * @param idx The index of the paletter to wich the color is stored.
 * @param r A pointer to the red component of the color.
 * @param g A pointer to the green component of the color.
 * @param b A pointer to the blue component of the color.
 * @param a A pointer to the alpha component of the color.
 *
 * This function retrieves the color for the palette of type @p pal at the
 * index @p idx of the textgrid object @p obj. The ARGB components are
 * stored in the buffers @p r, @p g, @p b and @p a. If @p idx is not
 * between 0 and the index of the latest set color, or if @p pal is
 * #EVAS_TEXTGRID_PALETTE_NONE or #EVAS_TEXTGRID_PALETTE_LAST, the
 * values of the components are 0. @p r, @p g, @pb and @p a can be
 * @c NULL.
 *
 * @see evas_object_textgrid_palette_set()
 *
 * @since 1.7
 */
EAPI void evas_object_textgrid_palette_get(const Evas_Object *obj, Evas_Textgrid_Palette pal, int idx, int *r, int *g, int *b, int *a);

EAPI void evas_object_textgrid_supported_font_styles_set(Evas_Object *obj, Evas_Textgrid_Font_Style styles);
EAPI Evas_Textgrid_Font_Style evas_object_textgrid_supported_font_styles_get(const Evas_Object *obj);

/**
 * @brief Set the string at the given row of the given textgrid object.
 *
 * @param obj The textgrid object to query for font information.
 * @param y The row index of the grid.
 * @param The string as a sequence of #Evas_Textgrid_Cell.
 *
 * This function returns cells to the textgrid taken by
 * evas_object_textgrid_cellrow_get(). The row pointer @p row should be the
 * same row pointer returned by evas_object_textgrid_cellrow_get() for the
 * same row @p y.
 *
 * @see evas_object_textgrid_cellrow_get()
 * @see evas_object_textgrid_size_set()
 * @see evas_object_textgrid_update_add()
 *
 * @since 1.7
 */
EAPI void evas_object_textgrid_cellrow_set(Evas_Object *obj, int y, const Evas_Textgrid_Cell *row);

/**
 * @brief Get the string at the given row of the given textgrid object.
 *
 * @param obj The textgrid object to query for font information.
 * @param y The row index of the grid.
 * @return A pointer to the first cell of the given row.
 *
 * This function returns a pointer to the first cell of the line @p y
 * of the textgrid object @p obj. If @p y is not between 0 and the
 * number of lines of the grid - 1, or on error, this function return @c NULL.
 *
 * @see evas_object_textgrid_cellrow_set()
 * @see evas_object_textgrid_size_set()
 * @see evas_object_textgrid_update_add()
 *
 * @since 1.7
 */
EAPI Evas_Textgrid_Cell *evas_object_textgrid_cellrow_get(const Evas_Object *obj, int y);

/**
 * @brief Indicate for evas that part of a textgrid region (cells) has been updated.
 *
 * @param obj The textgrid object.
 * @param x The rect region of cells top-left x (column)
 * @param y The rect region of cells top-left y (row)
 * @param w The rect region size in number of cells (columns)
 * @param h The rect region size in number of cells (rows)
 *
 * This function declares to evas that a region of cells was updated by
 * code and needs refreshing. An application should modify cells like this
 * as an example:
 * 
 * @code
 * Evas_Textgrid_Cell *cells;
 * int i;
 * 
 * cells = evas_object_textgrid_cellrow_get(obj, row);
 * for (i = 0; i < width; i++) cells[i].codepoint = 'E';
 * evas_object_textgrid_cellrow_set(obj, row, cells);
 * evas_object_textgrid_update_add(obj, 0, row, width, 1);
 * @endcode
 *
 * @see evas_object_textgrid_cellrow_set()
 * @see evas_object_textgrid_cellrow_get()
 * @see evas_object_textgrid_size_set()
 *
 * @since 1.7
 */
EAPI void evas_object_textgrid_update_add(Evas_Object *obj, int x, int y, int w, int h);

/**
 * @}
 */

/**
 * @defgroup Evas_Line_Group Line Object Functions
 *
 * Functions used to deal with evas line objects.
 *
 * @warning We don't guarantee any proper results if you create a Line object
 * without setting the evas engine.
 *
 * @ingroup Evas_Object_Specific
 *
 * @{
 */

#define EVAS_OBJ_LINE_CLASS evas_object_line_class_get()
const Eo_Class *evas_object_line_class_get(void) EINA_CONST;

extern EAPI Eo_Op EVAS_OBJ_LINE_BASE_ID;

enum
{
   EVAS_OBJ_LINE_SUB_ID_XY_SET,
   EVAS_OBJ_LINE_SUB_ID_XY_GET,
   EVAS_OBJ_LINE_SUB_ID_LAST
};

#define EVAS_OBJ_LINE_ID(sub_id) (EVAS_OBJ_LINE_BASE_ID + sub_id)


/**
 * @def evas_obj_line_xy_set
 * @since 1.8
 *
 * Sets the coordinates of the end points of the given evas line object.
 *
 * @param[in] x1
 * @param[in] y1
 * @param[in] x2
 * @param[in] y2
 *
 * @see evas_object_line_xy_set
 */
#define evas_obj_line_xy_set(x1, y1, x2, y2) EVAS_OBJ_LINE_ID(EVAS_OBJ_LINE_SUB_ID_XY_SET), EO_TYPECHECK(Evas_Coord, x1), EO_TYPECHECK(Evas_Coord, y1), EO_TYPECHECK(Evas_Coord, x2), EO_TYPECHECK(Evas_Coord, y2)

/**
 * @def evas_obj_line_xy_get
 * @since 1.8
 *
 * Retrieves the coordinates of the end points of the given evas line object.
 *
 * @param[out] x1
 * @param[out] y1
 * @param[out] x2
 * @param[out] y2
 *
 * @see evas_object_line_xy_get
 */
#define evas_obj_line_xy_get(x1, y1, x2, y2) EVAS_OBJ_LINE_ID(EVAS_OBJ_LINE_SUB_ID_XY_GET), EO_TYPECHECK(Evas_Coord *, x1), EO_TYPECHECK(Evas_Coord *, y1), EO_TYPECHECK(Evas_Coord *, x2), EO_TYPECHECK(Evas_Coord *, y2)

/**
 * Adds a new evas line object to the given evas.
 * @param   e The given evas.
 * @return  The new evas line object.
 */
EAPI Evas_Object *evas_object_line_add(Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * Sets the coordinates of the end points of the given evas line object.
 * @param   obj The given evas line object.
 * @param   x1  The X coordinate of the first point.
 * @param   y1  The Y coordinate of the first point.
 * @param   x2  The X coordinate of the second point.
 * @param   y2  The Y coordinate of the second point.
 */
EAPI void         evas_object_line_xy_set(Evas_Object *obj, Evas_Coord x1, Evas_Coord y1, Evas_Coord x2, Evas_Coord y2);

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
EAPI void         evas_object_line_xy_get(const Evas_Object *obj, Evas_Coord *x1, Evas_Coord *y1, Evas_Coord *x2, Evas_Coord *y2);
/**
 * @}
 */

/**
 * @defgroup Evas_Object_Polygon Polygon Object Functions
 *
 * Functions that operate on evas polygon objects.
 *
 * Hint: as evas does not provide ellipse, smooth paths or circle, one
 * can calculate points and convert these to a polygon.
 *
 * @warning We don't guarantee any proper results if you create a Polygon
 * object without setting the evas engine.
 *
 * @ingroup Evas_Object_Specific
 *
 * @{
 */

#define EVAS_OBJ_POLYGON_CLASS evas_object_polygon_class_get()
const Eo_Class *evas_object_polygon_class_get(void) EINA_CONST;

extern EAPI Eo_Op EVAS_OBJ_POLYGON_BASE_ID;

enum
{
   EVAS_OBJ_POLYGON_SUB_ID_POINT_ADD,
   EVAS_OBJ_POLYGON_SUB_ID_POINTS_CLEAR,
   EVAS_OBJ_POLYGON_SUB_ID_LAST
};

#define EVAS_OBJ_POLYGON_ID(sub_id) (EVAS_OBJ_POLYGON_BASE_ID + sub_id)


/**
 * @def evas_obj_polygon_point_add
 * @since 1.8
 *
 * Adds the given point to the given evas polygon object.
 *
 * @param[in] x
 * @param[in] y
 *
 * @see evas_object_polygon_point_add
 */
#define evas_obj_polygon_point_add(x, y) EVAS_OBJ_POLYGON_ID(EVAS_OBJ_POLYGON_SUB_ID_POINT_ADD), EO_TYPECHECK(Evas_Coord, x), EO_TYPECHECK(Evas_Coord, y)

/**
 * @def evas_obj_polygon_points_clear
 * @since 1.8
 *
 * Removes all of the points from the given evas polygon object.
 *
 *
 * @see evas_object_polygon_points_clear
 */
#define evas_obj_polygon_points_clear() EVAS_OBJ_POLYGON_ID(EVAS_OBJ_POLYGON_SUB_ID_POINTS_CLEAR)

/**
 * Adds a new evas polygon object to the given evas.
 * @param   e The given evas.
 * @return  A new evas polygon object.
 */
EAPI Evas_Object *evas_object_polygon_add(Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * Adds the given point to the given evas polygon object.
 * @param obj The given evas polygon object.
 * @param x   The X coordinate of the given point.
 * @param y   The Y coordinate of the given point.
 * @ingroup Evas_Polygon_Group
 */
EAPI void         evas_object_polygon_point_add(Evas_Object *obj, Evas_Coord x, Evas_Coord y) EINA_ARG_NONNULL(1);

/**
 * Removes all of the points from the given evas polygon object.
 * @param   obj The given polygon object.
 */
EAPI void         evas_object_polygon_points_clear(Evas_Object *obj) EINA_ARG_NONNULL(1);
/**
 * @}
 */

/* @since 1.2 */
EAPI void         evas_object_is_frame_object_set(Evas_Object *obj, Eina_Bool is_frame);

/* @since 1.2 */
EAPI Eina_Bool    evas_object_is_frame_object_get(Evas_Object *obj);

/**
 * @defgroup Evas_Smart_Group Smart Functions
 *
 * Functions that deal with #Evas_Smart structs, creating definition
 * (classes) of objects that will have customized behavior for methods
 * like evas_object_move(), evas_object_resize(),
 * evas_object_clip_set() and others.
 *
 * These objects will accept the generic methods defined in @ref
 * Evas_Object_Group and the extensions defined in @ref
 * Evas_Smart_Object_Group. There are a couple of existent smart
 * objects in Evas itself (see @ref Evas_Object_Box, @ref
 * Evas_Object_Table and @ref Evas_Smart_Object_Clipped).
 *
 * See also some @ref Example_Evas_Smart_Objects "examples" of this
 * group of functions.
 */

/**
 * @addtogroup Evas_Smart_Group
 * @{
 */

/**
 * @def EVAS_SMART_CLASS_VERSION
 *
 * The version you have to put into the version field in the
 * #Evas_Smart_Class struct. Used to safeguard from binaries with old
 * smart object intefaces running with newer ones.
 *
 * @ingroup Evas_Smart_Group
 */
#define EVAS_SMART_CLASS_VERSION 4
/**
 * @struct _Evas_Smart_Class
 *
 * A smart object's @b base class definition
 *
 * @ingroup Evas_Smart_Group
 */
struct _Evas_Smart_Class
{
   const char                      *name; /**< the name string of the class */
   int                              version;
   void                             (*add)(Evas_Object *o); /**< code to be run when adding object to a canvas */
   void                             (*del)(Evas_Object *o); /**< code to be run when removing object from a canvas */
   void                             (*move)(Evas_Object *o, Evas_Coord x, Evas_Coord y); /**< code to be run when moving object on a canvas. @a x and @a y will be new coordinates one applied to the object. use evas_object_geometry_get() if you need the old values, during this call. after that, the old values will be lost. */
   void                             (*resize)(Evas_Object *o, Evas_Coord w, Evas_Coord h); /**< code to be run when resizing object on a canvas. @a w and @a h will be new dimensions one applied to the object. use evas_object_geometry_get() if you need the old values, during this call. after that, the old values will be lost. */
   void                             (*show)(Evas_Object *o); /**< code to be run when showing object on a canvas */
   void                             (*hide)(Evas_Object *o); /**< code to be run when hiding object on a canvas */
   void                             (*color_set)(Evas_Object *o, int r, int g, int b, int a); /**< code to be run when setting color of object on a canvas. @a r, @a g, @a b and @a y will be new color components one applied to the object. use evas_object_color_get() if you need the old values, during this call. after that, the old values will be lost. */
   void                             (*clip_set)(Evas_Object *o, Evas_Object *clip); /**< code to be run when setting clipper of object on a canvas. @a clip will be new clipper one applied to the object. use evas_object_clip_get() if you need the old one, during this call. after that, the old (object pointer) value will be lost. */
   void                             (*clip_unset)(Evas_Object *o); /**< code to be run when unsetting clipper of object on a canvas. if you need the pointer to a previous set clipper, during this call, use evas_object_clip_get(). after that, the old (object pointer) value will be lost. */
   void                             (*calculate)(Evas_Object *o); /**< code to be run when object has rendering updates on a canvas */
   void                             (*member_add)(Evas_Object *o, Evas_Object *child); /**< code to be run when a child member is added to object */
   void                             (*member_del)(Evas_Object *o, Evas_Object *child); /**< code to be run when a child member is removed from object */

   const Evas_Smart_Class          *parent; /**< this class inherits from this parent */
   const Evas_Smart_Cb_Description *callbacks; /**< callbacks at this level, @c NULL terminated */
   const Evas_Smart_Interface     **interfaces; /**< #Evas_Smart_Interface pointers array, @c NULL terminated. These will be the interfaces supported at this level for an object (parents may have others) @since 1.7 */
   const void                      *data;
};

/**
 * @struct _Evas_Smart_Interface
 *
 * A smart object's @b base interface definition
 *
 * Every Evas interface must have a name field, pointing to a global,
 * constant string variable. This string pointer will be the only way
 * of retrieving back a given interface from a smart object. Two
 * function pointers must be defined, too, which will be called at
 * object creation and deletion times.
 *
 * See also some @ref Example_Evas_Smart_Interfaces "examples" on
 * smart interfaces.
 *
 * @since 1.7
 *
 * @ingroup Evas_Smart_Group
 */
struct _Evas_Smart_Interface
{
   const char *name; /**< Name of the given interface */
   unsigned    private_size; /**< Size, in bytes, of the interface's private dada blob. This will be allocated and freed automatically for you. Get it with evas_object_smart_interface_data_get(). */
   Eina_Bool   (*add)(Evas_Object *obj); /**< Function to be called at object creation time. This will take place @b before the object's smart @c add() function. */
   void        (*del)(Evas_Object *obj); /**< Function to be called at object deletion time. This will take place @b after the object's smart @c del() function. */
};

/**
 * @struct _Evas_Smart_Cb_Description
 *
 * Describes a callback issued by a smart object
 * (evas_object_smart_callback_call()), as defined in its smart object
 * class. This is particularly useful to explain to end users and
 * their code (i.e., introspection) what the parameter @c event_info
 * will point to.
 *
 * @ingroup Evas_Smart_Group
 */
struct _Evas_Smart_Cb_Description
{
   const char *name; /**< callback name ("changed", for example) */

   /**
    * @brief Hint on the type of @c event_info parameter's contents on
    * a #Evas_Smart_Cb callback.
    *
    * The type string uses the pattern similar to
    * http://dbus.freedesktop.org/doc/dbus-specification.html#message-protocol-signatures,
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
    *       or enforced in any way. Implementors should make the best
    *       use of it to help bindings, documentation and other users
    *       of introspection features.
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
#define EVAS_SMART_CLASS_INIT_NULL    {NULL, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}

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
#define EVAS_SMART_CLASS_INIT_NAME_VERSION(name)                                     {name, EVAS_SMART_CLASS_VERSION, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}

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
#define EVAS_SMART_CLASS_INIT_NAME_VERSION_PARENT(name, parent)                      {name, EVAS_SMART_CLASS_VERSION, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, parent, NULL, NULL}

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
 * Convenience macro to subclass a given Evas smart class.
 *
 * @param smart_name The name used for the smart class. e.g:
 * @c "Evas_Object_Box".
 * @param prefix Prefix used for all variables and functions defined
 * and referenced by this macro.
 * @param api_type Type of the structure used as API for the smart
 * class. Either #Evas_Smart_Class or something derived from it.
 * @param parent_type Type of the parent class API.
 * @param parent_func Function that gets the parent class. e.g:
 * evas_object_box_smart_class_get().
 * @param cb_desc Array of callback descriptions for this smart class.
 *
 * This macro saves some typing when writing a smart class derived
 * from another one. In order to work, the user @b must provide some
 * functions adhering to the following guidelines:
 *  - @<prefix@>_smart_set_user(): the @b internal @c _smart_set
 *    function (defined by this macro) will call this one, provided by
 *    the user, after inheriting everything from the parent, which
 *    should <b>take care of setting the right member functions for
 *    the class</b>, both overrides and extensions, if any.
 *  - If this new class should be subclassable as well, a @b public
 *    @c _smart_set() function is desirable to fill in the class used as
 *    parent by the children. It's up to the user to provide this
 *    interface, which will most likely call @<prefix@>_smart_set() to
 *    get the job done.
 *
 * After the macro's usage, the following will be defined for use:
 *  - @<prefix@>_parent_sc: A pointer to the @b parent smart
 *    class. When calling parent functions from overloaded ones, use
 *    this global variable.
 *  - @<prefix@>_smart_class_new(): this function returns the
 *    #Evas_Smart needed to create smart objects with this class,
 *    which should be passed to evas_object_smart_add().
 *
 * @warning @p smart_name has to be a pointer to a globally available
 * string! The smart class created here will just have a pointer set
 * to that, and all object instances will depend on it for smart class
 * name lookup.
 *
 * @ingroup Evas_Smart_Group
 */
#define EVAS_SMART_SUBCLASS_NEW(smart_name, prefix, api_type, parent_type, parent_func, cb_desc) \
  static const parent_type * prefix##_parent_sc = NULL;                                          \
  static void prefix##_smart_set_user(api_type * api);                                           \
  static void prefix##_smart_set(api_type * api)                                                 \
  {                                                                                              \
     Evas_Smart_Class *sc;                                                                       \
     if (!(sc = (Evas_Smart_Class *)api))                                                        \
       return;                                                                                   \
     if (!prefix##_parent_sc)                                                                    \
       prefix##_parent_sc = parent_func();                                                       \
     evas_smart_class_inherit(sc, prefix##_parent_sc);                                           \
     prefix##_smart_set_user(api);                                                               \
  }                                                                                              \
  static Evas_Smart *prefix##_smart_class_new(void)                                              \
  {                                                                                              \
     static Evas_Smart *smart = NULL;                                                            \
     static api_type api;                                                                        \
     if (!smart)                                                                                 \
       {                                                                                         \
          Evas_Smart_Class *sc = (Evas_Smart_Class *)&api;                                       \
          memset(&api, 0, sizeof(api_type));                                                     \
          sc->version = EVAS_SMART_CLASS_VERSION;                                                \
          sc->name = smart_name;                                                                 \
          sc->callbacks = cb_desc;                                                               \
          prefix##_smart_set(&api);                                                              \
          smart = evas_smart_class_new(sc);                                                      \
       }                                                                                         \
     return smart;                                                                               \
  }

/**
 * @def EVAS_SMART_SUBCLASS_IFACE_NEW
 *
 * @since 1.7
 *
 * Convenience macro to subclass a given Evas smart class. This is the
 * same as #EVAS_SMART_SUBCLASS_NEW, but now <b>declaring smart
 * interfaces</b> besides the smart callbacks.
 *
 * @param smart_name The name used for the smart class. e.g:
 *                   @c "Evas_Object_Box".
 * @param prefix Prefix used for all variables and functions defined
 *               and referenced by this macro.
 * @param api_type Type of the structure used as API for the smart
 *                 class. Either #Evas_Smart_Class or something
 *                 derived from it.
 * @param parent_type Type of the parent class API.
 * @param parent_func Function that gets the parent class. e.g:
 *                    evas_object_box_smart_class_get().
 * @param cb_desc Array of smart callback descriptions for this smart
 *                class.
 * @param ifaces Array of Evas smart interafaces for this smart
 *               class.
 *
 * This macro saves some typing when writing a smart class derived
 * from another one. In order to work, the user @b must provide some
 * functions adhering to the following guidelines:
 *  - @<prefix@>_smart_set_user(): the @b internal @c _smart_set
 *    function (defined by this macro) will call this one, provided by
 *    the user, after inheriting everything from the parent, which
 *    should <b>take care of setting the right member functions for
 *    the class</b>, both overrides and extensions, if any.
 *  - If this new class should be subclassable as well, a @b public
 *    @c _smart_set() function is desirable to fill in the class used as
 *    parent by the children. It's up to the user to provide this
 *    interface, which will most likely call @<prefix@>_smart_set() to
 *    get the job done.
 *
 * After the macro's usage, the following will be defined for use:
 *  - @<prefix@>_parent_sc: A pointer to the @b parent smart
 *    class. When calling parent functions from overloaded ones, use
 *    this global variable.
 *  - @<prefix@>_smart_class_new(): this function returns the
 *    #Evas_Smart needed to create smart objects with this class,
 *    which should be passed to evas_object_smart_add().
 *
 * @warning @p smart_name has to be a pointer to a globally available
 * string! The smart class created here will just have a pointer set
 * to that, and all object instances will depend on it for smart class
 * name lookup.
 *
 * @ingroup Evas_Smart_Group
 */
#define EVAS_SMART_SUBCLASS_IFACE_NEW(smart_name,          \
                                      prefix,              \
                                      api_type,            \
                                      parent_type,         \
                                      parent_func,         \
                                      cb_desc,             \
                                      ifaces)              \
  static const parent_type * prefix##_parent_sc = NULL;    \
  static void prefix##_smart_set_user(api_type * api);     \
  static void prefix##_smart_set(api_type * api)           \
  {                                                        \
     Evas_Smart_Class *sc;                                 \
     if (!(sc = (Evas_Smart_Class *)api))                  \
       return;                                             \
     if (!prefix##_parent_sc)                              \
       prefix##_parent_sc = parent_func();                 \
     evas_smart_class_inherit(sc, prefix##_parent_sc);     \
     prefix##_smart_set_user(api);                         \
  }                                                        \
  static Evas_Smart *prefix##_smart_class_new(void)        \
  {                                                        \
     static Evas_Smart *smart = NULL;                      \
     static api_type api;                                  \
     if (!smart)                                           \
       {                                                   \
          Evas_Smart_Class *sc = (Evas_Smart_Class *)&api; \
          memset(&api, 0, sizeof(api_type));               \
          sc->version = EVAS_SMART_CLASS_VERSION;          \
          sc->name = smart_name;                           \
          sc->callbacks = cb_desc;                         \
          sc->interfaces = ifaces;                         \
          prefix##_smart_set(&api);                        \
          smart = evas_smart_class_new(sc);                \
       }                                                   \
     return smart;                                         \
  }

/**
 * @def EVAS_SMART_DATA_ALLOC
 *
 * Convenience macro to allocate smart data only if needed.
 *
 * When writing a subclassable smart object, the @c .add() function
 * will need to check if the smart private data was already allocated
 * by some child object or not. This macro makes it easier to do it.
 *
 * @note This is an idiom used when one calls the parent's @c .add()
 * after the specialized code. Naturally, the parent's base smart data
 * has to be contemplated as the specialized one's first member, for
 * things to work.
 *
 * @param o Evas object passed to the @c .add() function
 * @param priv_type The type of the data to allocate
 *
 * @ingroup Evas_Smart_Group
 */
#define EVAS_SMART_DATA_ALLOC(o, priv_type)              \
  priv_type * priv;                                      \
  priv = evas_object_smart_data_get(o);                  \
  if (!priv) {                                           \
       priv = (priv_type *)calloc(1, sizeof(priv_type)); \
       if (!priv) return;                                \
       evas_object_smart_data_set(o, priv);              \
    }

#define EVAS_OBJ_SMART_CLASS evas_object_smart_class_get()

const Eo_Class *evas_object_smart_class_get(void) EINA_CONST;

extern EAPI Eo_Op EVAS_OBJ_SMART_BASE_ID;

enum
{
   EVAS_OBJ_SMART_SUB_ID_DATA_SET,
   EVAS_OBJ_SMART_SUB_ID_SMART_GET,
   EVAS_OBJ_SMART_SUB_ID_MEMBER_ADD,
   EVAS_OBJ_SMART_SUB_ID_MEMBER_DEL,
   EVAS_OBJ_SMART_SUB_ID_MEMBERS_GET,
   EVAS_OBJ_SMART_SUB_ID_CALLBACKS_DESCRIPTIONS_SET,
   EVAS_OBJ_SMART_SUB_ID_CALLBACKS_DESCRIPTIONS_GET,
   EVAS_OBJ_SMART_SUB_ID_CALLBACK_DESCRIPTION_FIND,
   EVAS_OBJ_SMART_SUB_ID_NEED_RECALCULATE_SET,
   EVAS_OBJ_SMART_SUB_ID_NEED_RECALCULATE_GET,
   EVAS_OBJ_SMART_SUB_ID_CALCULATE,
   EVAS_OBJ_SMART_SUB_ID_CHANGED,
   EVAS_OBJ_SMART_SUB_ID_ATTACH,
   // Specific Smart functions that can be overriden by the inherit classes
   EVAS_OBJ_SMART_SUB_ID_ADD,
   EVAS_OBJ_SMART_SUB_ID_DEL,
   EVAS_OBJ_SMART_SUB_ID_RESIZE,
   EVAS_OBJ_SMART_SUB_ID_MOVE,
   EVAS_OBJ_SMART_SUB_ID_SHOW,
   EVAS_OBJ_SMART_SUB_ID_HIDE,
   EVAS_OBJ_SMART_SUB_ID_COLOR_SET,
   EVAS_OBJ_SMART_SUB_ID_CLIP_SET,
   EVAS_OBJ_SMART_SUB_ID_CLIP_UNSET,
   EVAS_OBJ_SMART_SUB_ID_LAST
};

#define EVAS_OBJ_SMART_ID(sub_id) (EVAS_OBJ_SMART_BASE_ID + sub_id)

/**
 * @def evas_obj_smart_data_set
 * @since 1.8
 *
 * Store a pointer to user data for a given smart object.
 *
 * @param[in] data in
 *
 * @see evas_object_smart_data_set
 */
#define evas_obj_smart_data_set(data) EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_DATA_SET), EO_TYPECHECK(void *, data)

/**
 * @def evas_obj_smart_smart_get
 * @since 1.8
 *
 * Get the #Evas_Smart from which smart object was created.
 *
 * @param[out] smart out
 *
 * @see evas_object_smart_smart_get
 */
#define evas_obj_smart_smart_get(smart) EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_SMART_GET), EO_TYPECHECK(Evas_Smart **, smart)

/**
 * @def evas_obj_smart_member_add
 * @since 1.8
 *
 * Set an Evas object as a member of a given smart object.
 *
 * @param[in] obj in
 *
 * @see evas_object_smart_member_add
 */
#define evas_obj_smart_member_add(obj) EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_MEMBER_ADD), EO_TYPECHECK(Evas_Object *, obj)

/**
 * @def evas_obj_smart_member_del
 * @since 1.8
 *
 * Removes a member object from a given smart object.
 *
 * @param[in] obj in
 *
 * @see evas_object_smart_member_del
 */
#define evas_obj_smart_member_del(obj) EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_MEMBER_DEL), EO_TYPECHECK(Evas_Object *, obj)

/**
 * @def evas_obj_smart_members_get
 * @since 1.8
 *
 * Retrieves the list of the member objects of a given Evas smart
 * object
 *
 * @param[out] list out
 *
 * @see evas_object_smart_members_get
 */
#define evas_obj_smart_members_get(list) EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_MEMBERS_GET), EO_TYPECHECK(Eina_List **, list)

/**
 * @def evas_obj_smart_callback_priority_add
 * @since 1.8
 *
 * Add (register) a callback function to the smart event specified by
 * @p event on the smart object. Except for the priority field,
 * it's exactly the same as @ref evas_object_smart_callback_add
 *
 * @param[in] event in
 * @param[in] priority in
 * @param[in] func in
 * @param[in] data in
 *
 * @see evas_object_smart_callback_priority_add
 */
#define evas_obj_smart_callback_priority_add(event, priority, func, data) EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_CALLBACK_PRIORITY_ADD), EO_TYPECHECK(const char *, event), EO_TYPECHECK(Evas_Callback_Priority, priority), EO_TYPECHECK(Evas_Smart_Cb, func), EO_TYPECHECK(const void *, data)

/**
 * @def evas_obj_smart_callback_del
 * @since 1.8
 *
 * Delete (unregister) a callback function from the smart event
 * specified by @p event on the smart object.
 *
 * @param[in] event in
 * @param[in] func in
 * @param[out] ret_data out
 *
 * @see evas_object_smart_callback_del
 */
#define evas_obj_smart_callback_del(event, func, ret_data) EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_CALLBACK_DEL), EO_TYPECHECK(const char *, event), EO_TYPECHECK(Evas_Smart_Cb, func), EO_TYPECHECK(void **, ret_data)

/**
 * @def evas_obj_smart_callback_del_full
 * @since 1.8
 *
 * Delete (unregister) a callback function from the smart event
 * specified by @p event on the smart object.
 *
 * @param[in] event in
 * @param[in] func in
 * @param[in] data in
 * @param[out] ret_data out
 *
 * @see evas_object_smart_callback_del_full
 */
#define evas_obj_smart_callback_del_full(event, func, data, ret_data) EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_CALLBACK_DEL_FULL), EO_TYPECHECK(const char *, event), EO_TYPECHECK(Evas_Smart_Cb, func), EO_TYPECHECK(const void *, data), EO_TYPECHECK(void **, ret_data)

/**
 * @def evas_obj_smart_callback_call
 * @since 1.8
 *
 * Call a given smart callback on the smart object.
 *
 * @param[in] event
 * @param[in] event_info
 *
 * @see evas_object_smart_callback_call
 */
#define evas_obj_smart_callback_call(event, event_info) EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_CALLBACK_CALL), EO_TYPECHECK(const char *, event), EO_TYPECHECK(void *, event_info)

/**
 * @def evas_obj_smart_callbacks_descriptions_set
 * @since 1.8
 *
 * Set an smart object @b instance's smart callbacks descriptions.
 *
 * @param[in] descriptions in
 * @param[out] result out
 *
 * @see evas_object_smart_callbacks_descriptions_set
 */
#define evas_obj_smart_callbacks_descriptions_set(descriptions, result) EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_CALLBACKS_DESCRIPTIONS_SET), EO_TYPECHECK(const Evas_Smart_Cb_Description *, descriptions), EO_TYPECHECK(Eina_Bool *, result)

/**
 * @def evas_obj_smart_callbacks_descriptions_get
 * @since 1.8
 *
 * Retrieve an smart object's know smart callback descriptions (both
 * instance and class ones).
 *
 * @param[out] class_descriptions out
 * @param[out] class_count out
 * @param[out] instance_descriptions out
 * @param[out] instance_count out
 *
 * @see evas_object_smart_callbacks_descriptions_get
 */
#define evas_obj_smart_callbacks_descriptions_get(class_descriptions, class_count, instance_descriptions, instance_count) EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_CALLBACKS_DESCRIPTIONS_GET), EO_TYPECHECK(const Evas_Smart_Cb_Description ***, class_descriptions), EO_TYPECHECK(unsigned int *, class_count), EO_TYPECHECK(const Evas_Smart_Cb_Description ***, instance_descriptions), EO_TYPECHECK(unsigned int *, instance_count)

/**
 * @def evas_obj_smart_callback_description_find
 * @since 1.8
 *
 * Find callback description for callback called @a name.
 *
 * @param[in] name in
 * @param[out] class_description out
 * @param[out] instance_description out
 *
 * @see evas_object_smart_callback_description_find
 */
#define evas_obj_smart_callback_description_find(name, class_description, instance_description) EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_CALLBACK_DESCRIPTION_FIND), EO_TYPECHECK(const char *, name), EO_TYPECHECK(const Evas_Smart_Cb_Description **, class_description), EO_TYPECHECK(const Evas_Smart_Cb_Description **, instance_description)

/**
 * @def evas_obj_smart_need_recalculate_set
 * @since 1.8
 *
 * Set or unset the flag signalling that a given smart object needs to
 * get recalculated.
 *
 * @param[in] value in
 *
 * @see evas_object_smart_need_recalculate_set
 */
#define evas_obj_smart_need_recalculate_set(value) EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_NEED_RECALCULATE_SET), EO_TYPECHECK(Eina_Bool, value)

/**
 * @def evas_obj_smart_need_recalculate_get
 * @since 1.8
 *
 * Get the value of the flag signalling that a given smart object needs to
 * get recalculated.
 *
 * @param[out] need_recalculate out
 *
 * @see evas_object_smart_need_recalculate_get
 */
#define evas_obj_smart_need_recalculate_get(need_recalculate) EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_NEED_RECALCULATE_GET), EO_TYPECHECK(Eina_Bool *, need_recalculate)

/**
 * @def evas_obj_smart_calculate
 * @since 1.8
 *
 * Call the @b calculate() smart function immediately on a given smart
 * object.
 *
 * @see evas_object_smart_calculate
 */
#define evas_obj_smart_calculate() EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_CALCULATE)

/**
 * @def evas_obj_smart_changed
 * @since 1.8
 *
 * Mark smart object as changed, dirty.
 *
 * @see evas_object_smart_changed
 */
#define evas_obj_smart_changed() EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_CHANGED)


/**
 * @def evas_obj_smart_add
 * @since 1.8
 *
 * Instantiates a new smart object described by s.
 *
 *
 * @see evas_object_smart_add
 */
#define evas_obj_smart_add() EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_ADD)
#define evas_obj_smart_del() EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_DEL)
#define evas_obj_smart_resize(w, h) EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_RESIZE), EO_TYPECHECK(Evas_Coord, w), EO_TYPECHECK(Evas_Coord, h)
#define evas_obj_smart_move(x, y) EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_MOVE), EO_TYPECHECK(Evas_Coord, x), EO_TYPECHECK(Evas_Coord, y)
#define evas_obj_smart_show() EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_SHOW)
#define evas_obj_smart_hide() EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_HIDE)
#define evas_obj_smart_color_set(r, g, b, a) EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_COLOR_SET), EO_TYPECHECK(int, r), EO_TYPECHECK(int, g), EO_TYPECHECK(int, b), EO_TYPECHECK(int, a)
#define evas_obj_smart_clip_set(clip) EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_CLIP_SET), EO_TYPECHECK(Evas_Object *, clip)
#define evas_obj_smart_clip_unset() EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_CLIP_UNSET)

/**
 * Free an #Evas_Smart struct
 *
 * @param s the #Evas_Smart struct to free
 *
 * @warning If this smart handle was created using
 * evas_smart_class_new(), the associated #Evas_Smart_Class will not
 * be freed.
 *
 * @note If you're using the #EVAS_SMART_SUBCLASS_NEW schema to create your
 * smart object, note that an #Evas_Smart handle will be shared amongst all
 * instances of the given smart class, through a static variable.
 * Evas will internally count references on #Evas_Smart handles and free them
 * when they are not referenced anymore. Thus, this function is of no use
 * for Evas users, most probably.
 */
EAPI void                              evas_smart_free(Evas_Smart *s) EINA_ARG_NONNULL(1);

/**
 * Creates a new #Evas_Smart from a given #Evas_Smart_Class struct
 *
 * @param sc the smart class definition
 * @return a new #Evas_Smart pointer
 *
 * #Evas_Smart handles are necessary to create new @b instances of
 * smart objects belonging to the class described by @p sc. That
 * handle will contain, besides the smart class interface definition,
 * all its smart callbacks infrastructure set, too.
 *
 * @note If you are willing to subclass a given smart class to
 * construct yours, consider using the #EVAS_SMART_SUBCLASS_NEW macro,
 * which will make use of this function automatically for you.
 */
EAPI Evas_Smart                       *evas_smart_class_new(const Evas_Smart_Class *sc) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * Get the #Evas_Smart_Class handle of an #Evas_Smart struct
 *
 * @param s a valid #Evas_Smart pointer
 * @return the #Evas_Smart_Class in it
 */
EAPI const Evas_Smart_Class           *evas_smart_class_get(const Evas_Smart *s) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief Get the data pointer set on an #Evas_Smart struct
 *
 * @param s a valid #Evas_Smart handle
 *
 * This data pointer is set as the data field in the #Evas_Smart_Class
 * passed in to evas_smart_class_new().
 */
EAPI void                             *evas_smart_data_get(const Evas_Smart *s) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Get the smart callbacks known by this #Evas_Smart handle's smart
 * class hierarchy.
 *
 * @param s A valid #Evas_Smart handle.
 * @param[out] count Returns the number of elements in the returned
 * array.
 * @return The array with callback descriptions known by this smart
 *         class, with its size returned in @a count parameter. It
 *         should not be modified in any way. If no callbacks are
 *         known, @c NULL is returned. The array is sorted by event
 *         names and elements refer to the original values given to
 *         evas_smart_class_new()'s Evas_Smart_Class::callbacks
 *         (pointer to them).
 *
 * This is likely different from
 * evas_object_smart_callbacks_descriptions_get() as it will contain
 * the callbacks of @b all this class hierarchy sorted, while the
 * direct smart class member refers only to that specific class and
 * should not include parent's.
 *
 * If no callbacks are known, this function returns @c NULL.
 *
 * The array elements and thus their contents will be @b references to
 * original values given to evas_smart_class_new() as
 * Evas_Smart_Class::callbacks.
 *
 * The array is sorted by Evas_Smart_Cb_Description::name. The last
 * array element is a @c NULL pointer and is not accounted for in @a
 * count. Loop iterations can check any of these size indicators.
 *
 * @note objects may provide per-instance callbacks, use
 *       evas_object_smart_callbacks_descriptions_get() to get those
 *       as well.
 * @see evas_object_smart_callbacks_descriptions_get()
 */
EAPI const Evas_Smart_Cb_Description **evas_smart_callbacks_descriptions_get(const Evas_Smart *s, unsigned int *count) EINA_ARG_NONNULL(1, 1);

/**
 * Find a callback description for the callback named @a name.
 *
 * @param s The #Evas_Smart where to search for class registered smart
 * event callbacks.
 * @param name Name of the desired callback, which must @b not be @c
 *        NULL. The search has a special case for @a name being the
 *        same pointer as registered with #Evas_Smart_Cb_Description.
 *        One can use it to avoid excessive use of strcmp().
 * @return A reference to the description if found, or @c NULL, otherwise
 *
 * @see evas_smart_callbacks_descriptions_get()
 */
EAPI const Evas_Smart_Cb_Description  *evas_smart_callback_description_find(const Evas_Smart *s, const char *name) EINA_ARG_NONNULL(1, 2);

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
EAPI Eina_Bool                         evas_smart_class_inherit_full(Evas_Smart_Class *sc, const Evas_Smart_Class *parent_sc, unsigned int parent_sc_size) EINA_ARG_NONNULL(1, 2);

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
EAPI int                               evas_smart_usage_get(const Evas_Smart *s);

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
#define evas_smart_class_inherit(sc, parent_sc) evas_smart_class_inherit_full(sc, (Evas_Smart_Class *)parent_sc, sizeof(*parent_sc))

/**
 * @}
 */

/**
 * @defgroup Evas_Smart_Object_Group Smart Object Functions
 *
 * Functions dealing with Evas smart objects (instances).
 *
 * Smart objects are groupings of primitive Evas objects that behave
 * as a cohesive group. For instance, a file manager icon may be a
 * smart object composed of an image object, a text label and two
 * rectangles that appear behind the image and text when the icon is
 * selected. As a smart object, the normal Evas object API could be
 * used on the icon object.
 *
 * Besides that, generally smart objects implement a <b>specific
 * API</b>, so that users interact with its own custom features. The
 * API takes form of explicit exported functions one may call and
 * <b>smart callbacks</b>.
 *
 * @section Evas_Smart_Object_Group_Callbacks Smart events and callbacks
 *
 * Smart objects can elect events (smart events, from now on) occurring
 * inside of them to be reported back to their users via callback
 * functions (smart callbacks). This way, you can extend Evas' own
 * object events. They are defined by an <b>event string</b>, which
 * identifies them uniquely. There's also a function prototype
 * definition for the callback functions: #Evas_Smart_Cb.
 *
 * When defining an #Evas_Smart_Class, smart object implementors are
 * strongly encouraged to properly set the Evas_Smart_Class::callbacks
 * callbacks description array, so that the users of the smart object
 * can have introspection on its events API <b>at run time</b>.
 *
 * See some @ref Example_Evas_Smart_Objects "examples" of this group
 * of functions.
 *
 * @see @ref Evas_Smart_Group for class definitions.
 */

/**
 * @addtogroup Evas_Smart_Object_Group
 * @{
 */

/**
 * Instantiates a new smart object described by @p s.
 *
 * @param e the canvas on which to add the object
 * @param s the #Evas_Smart describing the smart object
 * @return a new #Evas_Object handle
 *
 * This is the function one should use when defining the public
 * function @b adding an instance of the new smart object to a given
 * canvas. It will take care of setting all of its internals to work
 * as they should, if the user set things properly, as seem on the
 * #EVAS_SMART_SUBCLASS_NEW, for example.
 *
 * @ingroup Evas_Smart_Object_Group
 */
EAPI Evas_Object *evas_object_smart_add(Evas *e, Evas_Smart *s) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1, 2) EINA_MALLOC;

/**
 * Set an Evas object as a member of a given smart object.
 *
 * @param obj The member object
 * @param smart_obj The smart object
 *
 * Members will automatically be stacked and layered together with the
 * smart object. The various stacking functions will operate on
 * members relative to the other members instead of the entire canvas,
 * since they now live on an exclusive layer (see
 * evas_object_stack_above(), for more details).
 *
 * Any @p smart_obj object's specific implementation of the @c
 * member_add() smart function will take place too, naturally.
 *
 * @see evas_object_smart_member_del()
 * @see evas_object_smart_members_get()
 *
 * @ingroup Evas_Smart_Object_Group
 */
EAPI void         evas_object_smart_member_add(Evas_Object *obj, Evas_Object *smart_obj) EINA_ARG_NONNULL(1, 2);

/**
 * Removes a member object from a given smart object.
 *
 * @param obj the member object
 * @ingroup Evas_Smart_Object_Group
 *
 * This removes a member object from a smart object, if it was added
 * to any. The object will still be on the canvas, but no longer
 * associated with whichever smart object it was associated with.
 *
 * @see evas_object_smart_member_add() for more details
 * @see evas_object_smart_members_get()
 */
EAPI void         evas_object_smart_member_del(Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * Retrieves the list of the member objects of a given Evas smart
 * object
 *
 * @param obj the smart object to get members from
 * @return Returns the list of the member objects of @p obj.
 *
 * The returned list should be freed with @c eina_list_free() when you
 * no longer need it.
 *
 * @since 1.7 This function will return @c NULL when a non-smart object is passed.
 *
 * @see evas_object_smart_member_add()
 * @see evas_object_smart_member_del()
 */
EAPI Eina_List   *evas_object_smart_members_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Gets the parent smart object of a given Evas object, if it has one.
 *
 * @param obj the Evas object you want to get the parent smart object
 * from
 * @return Returns the parent smart object of @a obj or @c NULL, if @a
 * obj is not a smart member of any
 *
 * @ingroup Evas_Smart_Object_Group
 */
EAPI Evas_Object *evas_object_smart_parent_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Checks whether a given smart object or any of its smart object
 * parents is of a given smart class.
 *
 * @param obj An Evas smart object to check the type of
 * @param type The @b name (type) of the smart class to check for
 * @return @c EINA_TRUE, if @a obj or any of its parents is of type @a
 * type, @c EINA_FALSE otherwise
 *
 * If @p obj is not a smart object, this call will fail
 * immediately. Otherwise, make sure evas_smart_class_inherit() or its
 * sibling functions were used correctly when creating the smart
 * object's class, so it has a valid @b parent smart class pointer
 * set.
 *
 * The checks use smart classes names and <b>string
 * comparison</b>. There is a version of this same check using
 * <b>pointer comparison</b>, since a smart class' name is a single
 * string in Evas.
 *
 * @see evas_object_smart_type_check_ptr()
 * @see #EVAS_SMART_SUBCLASS_NEW
 *
 * @ingroup Evas_Smart_Object_Group
 */
EAPI Eina_Bool    evas_object_smart_type_check(const Evas_Object *obj, const char *type) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1, 2);

/**
 * Checks whether a given smart object or any of its smart object
 * parents is of a given smart class, <b>using pointer comparison</b>.
 *
 * @param obj An Evas smart object to check the type of
 * @param type The type (name string) to check for. Must be the name
 * @return @c EINA_TRUE, if @a obj or any of its parents is of type @a
 * type, @c EINA_FALSE otherwise
 *
 * @see evas_object_smart_type_check() for more details
 *
 * @ingroup Evas_Smart_Object_Group
 */
EAPI Eina_Bool    evas_object_smart_type_check_ptr(const Evas_Object *obj, const char *type) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1, 2);

/**
 * Get the #Evas_Smart from which @p obj smart object was created.
 *
 * @param obj a smart object
 * @return the #Evas_Smart handle or @c NULL, on errors
 *
 * @ingroup Evas_Smart_Object_Group
 */
EAPI Evas_Smart  *evas_object_smart_smart_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Retrieve user data stored on a given smart object.
 *
 * @param obj The smart object's handle
 * @return A pointer to data stored using
 *         evas_object_smart_data_set(), or @c NULL, if none has been
 *         set.
 *
 * @see evas_object_smart_data_set()
 *
 * @ingroup Evas_Smart_Object_Group
 */
EAPI void        *evas_object_smart_data_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Store a pointer to user data for a given smart object.
 *
 * @param obj The smart object's handle
 * @param data A pointer to user data
 *
 * This data is stored @b independently of the one set by
 * evas_object_data_set(), naturally.
 *
 * @see evas_object_smart_data_get()
 *
 * @ingroup Evas_Smart_Object_Group
 */
EAPI void         evas_object_smart_data_set(Evas_Object *obj, void *data) EINA_ARG_NONNULL(1);

/**
 * Add (register) a callback function to the smart event specified by
 * @p event on the smart object @p obj.
 *
 * @param obj a smart object
 * @param event the event's name string
 * @param func the callback function
 * @param data user data to be passed to the callback function
 *
 * Smart callbacks look very similar to Evas callbacks, but are
 * implemented as smart object's custom ones.
 *
 * This function adds a function callback to an smart object when the
 * event named @p event occurs in it. The function is @p func.
 *
 * In the event of a memory allocation error during addition of the
 * callback to the object, evas_alloc_error() should be used to
 * determine the nature of the error, if any, and the program should
 * sensibly try and recover.
 *
 * A smart callback function must have the ::Evas_Smart_Cb prototype
 * definition. The first parameter (@p data) in this definition will
 * have the same value passed to evas_object_smart_callback_add() as
 * the @p data parameter, at runtime. The second parameter @p obj is a
 * handle to the object on which the event occurred. The third
 * parameter, @p event_info, is a pointer to data which is totally
 * dependent on the smart object's implementation and semantic for the
 * given event.
 *
 * There is an infrastructure for introspection on smart objects'
 * events (see evas_smart_callbacks_descriptions_get()), but no
 * internal smart objects on Evas implement them yet.
 *
 * @see @ref Evas_Smart_Object_Group_Callbacks for more details.
 *
 * @see evas_object_smart_callback_del()
 * @ingroup Evas_Smart_Object_Group
 */
EAPI void         evas_object_smart_callback_add(Evas_Object *obj, const char *event, Evas_Smart_Cb func, const void *data) EINA_ARG_NONNULL(1, 2, 3);

/**
 * Add (register) a callback function to the smart event specified by
 * @p event on the smart object @p obj. Except for the priority field,
 * it's exactly the same as @ref evas_object_smart_callback_add
 *
 * @param obj a smart object
 * @param event the event's name string
 * @param priority The priority of the callback, lower values called first.
 * @param func the callback function
 * @param data user data to be passed to the callback function
 *
 * @see evas_object_smart_callback_add
 * @since 1.1
 * @ingroup Evas_Smart_Object_Group
 */
EAPI void         evas_object_smart_callback_priority_add(Evas_Object *obj, const char *event, Evas_Callback_Priority priority, Evas_Smart_Cb func, const void *data);

/**
 * Delete (unregister) a callback function from the smart event
 * specified by @p event on the smart object @p obj.
 *
 * @param obj a smart object
 * @param event the event's name string
 * @param func the callback function
 * @return the data pointer
 *
 * This function removes <b>the first</b> added smart callback on the
 * object @p obj matching the event name @p event and the registered
 * function pointer @p func. If the removal is successful it will also
 * return the data pointer that was passed to
 * evas_object_smart_callback_add() (that will be the same as the
 * parameter) when the callback(s) was(were) added to the canvas. If
 * not successful @c NULL will be returned.
 *
 * @see evas_object_smart_callback_add() for more details.
 *
 * @ingroup Evas_Smart_Object_Group
 */
EAPI void        *evas_object_smart_callback_del(Evas_Object *obj, const char *event, Evas_Smart_Cb func) EINA_ARG_NONNULL(1, 2, 3);

/**
 * Delete (unregister) a callback function from the smart event
 * specified by @p event on the smart object @p obj.
 *
 * @param obj a smart object
 * @param event the event's name string
 * @param func the callback function
 * @param data the data pointer that was passed to the callback
 * @return the data pointer
 *
 * This function removes <b>the first</b> added smart callback on the
 * object @p obj matching the event name @p event, the registered
 * function pointer @p func and the callback data pointer @p data. If
 * the removal is successful it will also return the data pointer that
 * was passed to evas_object_smart_callback_add() (that will be the same
 * as the parameter) when the callback(s) was(were) added to the canvas.
 * If not successful @c NULL will be returned. A common use would be to
 * remove an exact match of a callback
 *
 * @see evas_object_smart_callback_add() for more details.
 * @since 1.2
 * @ingroup Evas_Smart_Object_Group
 *
 * @note To delete all smart event callbacks which match @p type and @p func,
 * use evas_object_smart_callback_del().
 */
EAPI void        *evas_object_smart_callback_del_full(Evas_Object *obj, const char *event, Evas_Smart_Cb func, const void *data) EINA_ARG_NONNULL(1, 2, 3);

/**
 * Call a given smart callback on the smart object @p obj.
 *
 * @param obj the smart object
 * @param event the event's name string
 * @param event_info pointer to an event specific struct or information to
 * pass to the callback functions registered on this smart event
 *
 * This should be called @b internally, from the smart object's own
 * code, when some specific event has occurred and the implementor
 * wants is to pertain to the object's events API (see @ref
 * Evas_Smart_Object_Group_Callbacks). The documentation for the smart
 * object should include a list of possible events and what type of @p
 * event_info to expect for each of them. Also, when defining an
 * #Evas_Smart_Class, smart object implementors are strongly
 * encouraged to properly set the Evas_Smart_Class::callbacks
 * callbacks description array, so that the users of the smart object
 * can have introspection on its events API <b>at run time</b>.
 *
 * @ingroup Evas_Smart_Object_Group
 */
EAPI void         evas_object_smart_callback_call(Evas_Object *obj, const char *event, void *event_info) EINA_ARG_NONNULL(1, 2);

/**
 * Set an smart object @b instance's smart callbacks descriptions.
 *
 * @param obj A smart object
 * @param descriptions @c NULL terminated array with
 * #Evas_Smart_Cb_Description descriptions. Array elements won't be
 * modified at run time, but references to them and their contents
 * will be made, so this array should be kept alive during the whole
 * object's lifetime.
 * @return @c EINA_TRUE on success, @c EINA_FALSE on failure.
 *
 * These descriptions are hints to be used by introspection and are
 * not enforced in any way.
 *
 * It will not be checked if instance callbacks descriptions have the
 * same name as respective possibly registered in the smart object
 * @b class. Both are kept in different arrays and users of
 * evas_object_smart_callbacks_descriptions_get() should handle this
 * case as they wish.
 *
 * @note Becase @p descriptions must be @c NULL terminated, and
 *        because a @c NULL name makes little sense, too,
 *        Evas_Smart_Cb_Description::name must @b not be @c NULL.
 *
 * @note While instance callbacks descriptions are possible, they are
 *       @b not recommended. Use @b class callbacks descriptions
 *       instead as they make you smart object user's life simpler and
 *       will use less memory, as descriptions and arrays will be
 *       shared among all instances.
 *
 * @ingroup Evas_Smart_Object_Group
 */
EAPI Eina_Bool    evas_object_smart_callbacks_descriptions_set(Evas_Object *obj, const Evas_Smart_Cb_Description *descriptions) EINA_ARG_NONNULL(1);

/**
 * Retrieve an smart object's know smart callback descriptions (both
 * instance and class ones).
 *
 * @param obj The smart object to get callback descriptions from.
 * @param class_descriptions Where to store class callbacks
 *        descriptions array, if any is known. If no descriptions are
 *        known, @c NULL is returned
 * @param class_count Returns how many class callbacks descriptions
 *        are known.
 * @param instance_descriptions Where to store instance callbacks
 *        descriptions array, if any is known. If no descriptions are
 *        known, @c NULL is returned.
 * @param instance_count Returns how many instance callbacks
 *        descriptions are known.
 *
 * This call searches for registered callback descriptions for both
 * instance and class of the given smart object. These arrays will be
 * sorted by Evas_Smart_Cb_Description::name and also @c NULL
 * terminated, so both @a class_count and @a instance_count can be
 * ignored, if the caller wishes so. The terminator @c NULL is not
 * counted in these values.
 *
 * @note If just class descriptions are of interest, try
 *       evas_smart_callbacks_descriptions_get() instead.
 *
 * @note Use @c NULL pointers on the descriptions/counters you're not
 * interested in: they'll be ignored by the function.
 *
 * @see evas_smart_callbacks_descriptions_get()
 *
 * @ingroup Evas_Smart_Object_Group
 */
EAPI void         evas_object_smart_callbacks_descriptions_get(const Evas_Object *obj, const Evas_Smart_Cb_Description ***class_descriptions, unsigned int *class_count, const Evas_Smart_Cb_Description ***instance_descriptions, unsigned int *instance_count) EINA_ARG_NONNULL(1);

/**
 * Find callback description for callback called @a name.
 *
 * @param obj the smart object.
 * @param name name of desired callback, must @b not be @c NULL.  The
 *        search have a special case for @a name being the same
 *        pointer as registered with Evas_Smart_Cb_Description, one
 *        can use it to avoid excessive use of strcmp().
 * @param class_description pointer to return class description or
 *        @c NULL if not found. If parameter is @c NULL, no search will
 *        be done on class descriptions.
 * @param instance_description pointer to return instance description
 *        or @c NULL if not found. If parameter is @c NULL, no search
 *        will be done on instance descriptions.
 * @return reference to description if found, @c NULL if not found.
 */
EAPI void         evas_object_smart_callback_description_find(const Evas_Object *obj, const char *name, const Evas_Smart_Cb_Description **class_description, const Evas_Smart_Cb_Description **instance_description) EINA_ARG_NONNULL(1, 2);

/**
 * Retrieve an Evas smart object's interface, by name string pointer.
 *
 * @param obj An Evas smart object.
 * @param name Name string of the desired interface, which must be the
 *             same pointer used at the interface's declarion, when
 *             creating the smart object @a obj.
 *
 * @since 1.7
 *
 * @return The interface's handle pointer, if found, @c NULL
 * otherwise.
 */
const void       *evas_object_smart_interface_get(const Evas_Object *obj, const char *name);

/**
 * Retrieve an Evas smart object interface's <b>private data</b>.
 *
 * @param obj An Evas smart object.
 * @param iface The given object's interface handle.
 *
 * @since 1.7
 *
 * @return The object interface's private data blob pointer, if found,
 * @c NULL otherwise.
 */
void             *evas_object_smart_interface_data_get(const Evas_Object *obj, const Evas_Smart_Interface *iface);

/**
 * Mark smart object as changed, dirty.
 *
 * @param obj The given Evas smart object
 *
 * This will flag the given object as needing recalculation,
 * forcefully. As an effect, on the next rendering cycle it's @b
 * calculate() (see #Evas_Smart_Class) smart function will be called.
 *
 * @see evas_object_smart_need_recalculate_set().
 * @see evas_object_smart_calculate().
 *
 * @ingroup Evas_Smart_Object_Group
 */
EAPI void         evas_object_smart_changed(Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * Set or unset the flag signalling that a given smart object needs to
 * get recalculated.
 *
 * @param obj the smart object
 * @param value whether one wants to set (@c EINA_TRUE) or to unset
 * (@c EINA_FALSE) the flag.
 *
 * If this flag is set, then the @c calculate() smart function of @p
 * obj will be called, if one is provided, during rendering phase of
 * Evas (see evas_render()), after which this flag will be
 * automatically unset.
 *
 * If that smart function is not provided for the given object, this
 * flag will be left unchanged.
 *
 * @note just setting this flag will not make the canvas' whole scene
 *       dirty, by itself, and evas_render() will have no effect. To
 *       force that, use evas_object_smart_changed(), that will also
 *       automatically call this function automatically, with
 *       @c EINA_TRUE as parameter.
 *
 * @see evas_object_smart_need_recalculate_get()
 * @see evas_object_smart_calculate()
 * @see evas_smart_objects_calculate()
 *
 * @ingroup Evas_Smart_Object_Group
 */
EAPI void         evas_object_smart_need_recalculate_set(Evas_Object *obj, Eina_Bool value) EINA_ARG_NONNULL(1);

/**
 * Get the value of the flag signalling that a given smart object needs to
 * get recalculated.
 *
 * @param obj the smart object
 * @return if flag is set or not.
 *
 * @note this flag will be unset during the rendering phase, when the
 *       @c calculate() smart function is called, if one is provided.
 *       If it's not provided, then the flag will be left unchanged
 *       after the rendering phase.
 *
 * @see evas_object_smart_need_recalculate_set(), for more details
 *
 * @ingroup Evas_Smart_Object_Group
 */
EAPI Eina_Bool    evas_object_smart_need_recalculate_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Call the @b calculate() smart function immediately on a given smart
 * object.
 *
 * @param obj the smart object's handle
 *
 * This will force immediate calculations (see #Evas_Smart_Class)
 * needed for renderization of this object and, besides, unset the
 * flag on it telling it needs recalculation for the next rendering
 * phase.
 *
 * @see evas_object_smart_need_recalculate_set()
 *
 * @ingroup Evas_Smart_Object_Group
 */
EAPI void         evas_object_smart_calculate(Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * Call user-provided @c calculate() smart functions and unset the
 * flag signalling that the object needs to get recalculated to @b all
 * smart objects in the canvas.
 *
 * @param e The canvas to calculate all smart objects in
 *
 * @see evas_object_smart_need_recalculate_set()
 *
 * @ingroup Evas_Smart_Object_Group
 */
EAPI void         evas_smart_objects_calculate(Evas *e);

/**
 * This gets the internal counter that counts the number of smart calculations
 *
 * @param e The canvas to get the calculate counter from
 *
 * Whenever evas performs smart object calculations on the whole canvas
 * it increments a counter by 1. This is the smart object calculate counter
 * that this function returns the value of. It starts at the value of 0 and
 * will increase (and eventually wrap around to negative values and so on) by
 * 1 every time objects are calculated. You can use this counter to ensure
 * you don't re-do calculations withint the same calculation generation/run
 * if the calculations maybe cause self-feeding effects.
 *
 * @ingroup Evas_Smart_Object_Group
 * @since 1.1
 */
EAPI int          evas_smart_objects_calculate_count_get(const Evas *e);

/**
 * Moves all children objects of a given smart object relative to a
 * given offset.
 *
 * @param obj the smart object.
 * @param dx horizontal offset (delta).
 * @param dy vertical offset (delta).
 *
 * This will make each of @p obj object's children to move, from where
 * they before, with those delta values (offsets) on both directions.
 *
 * @note This is most useful on custom smart @c move() functions.
 *
 * @note Clipped smart objects already make use of this function on
 * their @c move() smart function definition.
 */
EAPI void         evas_object_smart_move_children_relative(Evas_Object *obj, Evas_Coord dx, Evas_Coord dy) EINA_ARG_NONNULL(1);

/**
 * @}
 */

/**
 * @defgroup Evas_Smart_Object_Clipped Clipped Smart Object
 *
 * Clipped smart object is a base to construct other smart objects
 * based on the concept of having an internal clipper that is applied
 * to all children objects. This clipper will control the visibility,
 * clipping and color of sibling objects (remember that the clipping
 * is recursive, and clipper color modulates the color of its
 * clippees). By default, this base will also move children relatively
 * to the parent, and delete them when parent is deleted. In other
 * words, it is the base for simple object grouping.
 *
 * See some @ref Example_Evas_Smart_Objects "examples" of this group
 * of functions.
 *
 * @see evas_object_smart_clipped_smart_set()
 *
 * @ingroup Evas_Smart_Object_Group
 */

/**
 * @addtogroup Evas_Smart_Object_Clipped
 * @{
 */

#define EVAS_OBJ_SMART_CLIPPED_CLASS evas_object_smart_clipped_eo_class_get()

const Eo_Class *evas_object_smart_clipped_eo_class_get(void) EINA_CONST;

extern EAPI Eo_Op EVAS_OBJ_SMART_CLIPPED_BASE_ID;

enum
{
   EVAS_OBJ_SMART_CLIPPED_SUB_ID_LAST
};

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
 * @param obj the clipped smart object to retrieve associated clipper
 * from.
 * @return the clipper object.
 *
 * Use this function if you want to change any of this clipper's
 * properties, like colors.
 *
 * @see evas_object_smart_clipped_smart_add()
 */
EAPI Evas_Object            *evas_object_smart_clipped_clipper_get(Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Set a given smart class' callbacks so it implements the <b>clipped smart
 * object"</b>'s interface.
 *
 * @param sc The smart class handle to operate on
 *
 * This call will assign all the required methods of the @p sc
 * #Evas_Smart_Class instance to the implementations set for clipped
 * smart objects. If one wants to "subclass" it, call this function
 * and then override desired values. If one wants to call any original
 * method, save it somewhere. Example:
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
 * Default behavior for each of #Evas_Smart_Class functions on a
 * clipped smart object are:
 * - @c add: creates a hidden clipper with "infinite" size, to clip
 *    any incoming members;
 * - @c del: delete all children objects;
 * - @c move: move all objects relative relatively;
 * - @c resize: <b>not defined</b>;
 * - @c show: if there are children objects, show clipper;
 * - @c hide: hides clipper;
 * - @c color_set: set the color of clipper;
 * - @c clip_set: set clipper of clipper;
 * - @c clip_unset: unset the clipper of clipper;
 *
 * @note There are other means of assigning parent smart classes to
 * child ones, like the #EVAS_SMART_SUBCLASS_NEW macro or the
 * evas_smart_class_inherit_full() function.
 */
EAPI void                    evas_object_smart_clipped_smart_set(Evas_Smart_Class *sc) EINA_ARG_NONNULL(1);

/**
 * Get a pointer to the <b>clipped smart object's</b> class, to use
 * for proper inheritance
 *
 * @see #Evas_Smart_Object_Clipped for more information on this smart
 * class
 */
EAPI const Evas_Smart_Class *evas_object_smart_clipped_class_get(void) EINA_CONST;

/**
 * @}
 */

/**
 * @defgroup Evas_Object_Box Box Smart Object
 *
 * A box is a convenience smart object that packs children inside it
 * in @b sequence, using a layouting function specified by the
 * user. There are a couple of pre-made layouting functions <b>built-in
 * in Evas</b>, all of them using children size hints to define their
 * size and alignment inside their cell space.
 *
 * Examples on this smart object's usage:
 * - @ref Example_Evas_Box
 * - @ref Example_Evas_Size_Hints
 *
 * @see @ref Evas_Object_Group_Size_Hints
 *
 * @ingroup Evas_Smart_Object_Group
 */

/**
 * @addtogroup Evas_Object_Box
 * @{
 */

/**
 * @typedef Evas_Object_Box_Api
 *
 * Smart class extension, providing extra box object requirements.
 *
 * @ingroup Evas_Object_Box
 */
typedef struct _Evas_Object_Box_Api Evas_Object_Box_Api;

/**
 * @typedef Evas_Object_Box_Data
 *
 * Smart object instance data, providing box object requirements.
 *
 * @ingroup Evas_Object_Box
 */
typedef struct _Evas_Object_Box_Data Evas_Object_Box_Data;

/**
 * @typedef Evas_Object_Box_Option
 *
 * The base structure for a box option. Box options are a way of
 * extending box items properties, which will be taken into account
 * for layouting decisions. The box layouting functions provided by
 * Evas will only rely on objects' canonical size hints to layout
 * them, so the basic box option has @b no (custom) property set.
 *
 * Users creating their own layouts, but not depending on extra child
 * items' properties, would be fine just using
 * evas_object_box_layout_set(). But if one desires a layout depending
 * on extra child properties, he/she has to @b subclass the box smart
 * object. Thus, by using evas_object_box_smart_class_get() and
 * evas_object_box_smart_set(), the @c option_new() and @c
 * option_free() smart class functions should be properly
 * redefined/extended.
 *
 * Object properties are bound to an integer identifier and must have
 * a name string. Their values are open to any data. See the API on
 * option properties for more details.
 *
 * @ingroup Evas_Object_Box
 */
typedef struct _Evas_Object_Box_Option Evas_Object_Box_Option;

/**
 * @typedef Evas_Object_Box_Layout
 *
 * Function signature for an Evas box object layouting routine. By
 * @a o it will be passed the box object in question, by @a priv it will
 * be passed the box's internal data and, by @a user_data, it will be
 * passed any custom data one could have set to a given box layouting
 * function, with evas_object_box_layout_set().
 *
 * @ingroup Evas_Object_Box
 */
typedef void (*Evas_Object_Box_Layout)(Evas_Object *o, Evas_Object_Box_Data *priv, void *user_data);

/**
 * @def EVAS_OBJECT_BOX_API_VERSION
 *
 * Current version for Evas box object smart class, a value which goes
 * to _Evas_Object_Box_Api::version.
 *
 * @ingroup Evas_Object_Box
 */
#define EVAS_OBJECT_BOX_API_VERSION 1

/**
 * @struct _Evas_Object_Box_Api
 *
 * This structure should be used by any smart class inheriting from
 * the box's one, to provide custom box behavior which could not be
 * achieved only by providing a layout function, with
 * evas_object_box_layout_set().
 *
 * @extends Evas_Smart_Class
 * @ingroup Evas_Object_Box
 */
struct _Evas_Object_Box_Api
{
   Evas_Smart_Class        base;      /**< Base smart class struct, need for all smart objects */
   int                     version;      /**< Version of this smart class definition */
   Evas_Object_Box_Option *(*append)(Evas_Object * o, Evas_Object_Box_Data * priv, Evas_Object * child);            /**< Smart function to append child elements in boxes */
   Evas_Object_Box_Option *(*prepend)(Evas_Object * o, Evas_Object_Box_Data * priv, Evas_Object * child);           /**< Smart function to prepend child elements in boxes */
   Evas_Object_Box_Option *(*insert_before)(Evas_Object * o, Evas_Object_Box_Data * priv, Evas_Object * child, const Evas_Object * reference);    /**< Smart function to insert a child element before another in boxes */
   Evas_Object_Box_Option *(*insert_after)(Evas_Object * o, Evas_Object_Box_Data * priv, Evas_Object * child, const Evas_Object * reference);     /**< Smart function to insert a child element after another in boxes */
   Evas_Object_Box_Option *(*insert_at)(Evas_Object * o, Evas_Object_Box_Data * priv, Evas_Object * child, unsigned int pos);         /**< Smart function to insert a child element at a given position on boxes */
   Evas_Object            *(*remove)(Evas_Object * o, Evas_Object_Box_Data * priv, Evas_Object * child);            /**< Smart function to remove a child element from boxes */
   Evas_Object            *(*remove_at)(Evas_Object * o, Evas_Object_Box_Data * priv, unsigned int pos);          /**< Smart function to remove a child element from boxes, by its position */
   Eina_Bool               (*property_set)(Evas_Object *o, Evas_Object_Box_Option *opt, int property, va_list args);         /**< Smart function to set a custom property on a box child */
   Eina_Bool               (*property_get)(const Evas_Object *o, Evas_Object_Box_Option *opt, int property, va_list args);         /**< Smart function to retrieve a custom property from a box child */
   const char             *(*property_name_get)(const Evas_Object * o, int property);   /**< Smart function to get the name of a custom property of box children */
   int                     (*property_id_get)(const Evas_Object *o, const char *name);      /**< Smart function to get the numerical ID of a custom property of box children */
   Evas_Object_Box_Option *(*option_new)(Evas_Object * o, Evas_Object_Box_Data * priv, Evas_Object * child);        /**< Smart function to create a new box option struct */
   void                    (*option_free)(Evas_Object *o, Evas_Object_Box_Data *priv, Evas_Object_Box_Option *opt);          /**< Smart function to delete a box option struct */
};

/**
 * @def EVAS_OBJECT_BOX_API_INIT
 *
 * Initializer for a whole #Evas_Object_Box_Api structure, with
 * @c NULL values on its specific fields.
 *
 * @param smart_class_init initializer to use for the "base" field
 * (#Evas_Smart_Class).
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
 *
 * Initializer to zero out a whole #Evas_Object_Box_Api structure.
 *
 * @see EVAS_OBJECT_BOX_API_INIT_VERSION
 * @see EVAS_OBJECT_BOX_API_INIT_NAME_VERSION
 * @see EVAS_OBJECT_BOX_API_INIT
 * @ingroup Evas_Object_Box
 */
#define EVAS_OBJECT_BOX_API_INIT_NULL    EVAS_OBJECT_BOX_API_INIT(EVAS_SMART_CLASS_INIT_NULL)

/**
 * @def EVAS_OBJECT_BOX_API_INIT_VERSION
 *
 * Initializer to zero out a whole #Evas_Object_Box_Api structure and
 * set a specific version on it.
 *
 * This is similar to #EVAS_OBJECT_BOX_API_INIT_NULL, but it will set
 * the version field of #Evas_Smart_Class (base field) to the latest
 * #EVAS_SMART_CLASS_VERSION.
 *
 * @see EVAS_OBJECT_BOX_API_INIT_NULL
 * @see EVAS_OBJECT_BOX_API_INIT_NAME_VERSION
 * @see EVAS_OBJECT_BOX_API_INIT
 * @ingroup Evas_Object_Box
 */
#define EVAS_OBJECT_BOX_API_INIT_VERSION EVAS_OBJECT_BOX_API_INIT(EVAS_SMART_CLASS_INIT_VERSION)

/**
 * @def EVAS_OBJECT_BOX_API_INIT_NAME_VERSION
 *
 * Initializer to zero out a whole #Evas_Object_Box_Api structure and
 * set its name and version.
 *
 * This is similar to #EVAS_OBJECT_BOX_API_INIT_NULL, but it will also
 * set the version field of #Evas_Smart_Class (base field) to the
 * latest #EVAS_SMART_CLASS_VERSION and name it to the specific value.
 *
 * It will keep a reference to the name field as a <c>"const char *"</c>,
 * i.e., the name must be available while the structure is
 * used (hint: static or global variable!) and must not be modified.
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
 * a subclass inherits from #Evas_Object_Box_Api, then it may augment
 * #Evas_Object_Box_Data to fit its own needs.
 *
 * @extends Evas_Object_Smart_Clipped_Data
 * @ingroup Evas_Object_Box
 */
struct _Evas_Object_Box_Data
{
   Evas_Object_Smart_Clipped_Data base;
   const Evas_Object_Box_Api     *api;
   struct
   {
      double h, v;
   } align;
   struct
   {
      Evas_Coord h, v;
   } pad;
   Eina_List                     *children;
   struct
   {
      Evas_Object_Box_Layout cb;
      void                  *data;
      void                   (*free_data)(void *data);
   } layout;
   Eina_Bool                      layouting : 1;
   Eina_Bool                      children_changed : 1;
};

struct _Evas_Object_Box_Option
{
   Evas_Object *obj;    /**< Pointer to the box child object, itself */
   Eina_Bool    max_reached : 1;
   Eina_Bool    min_reached : 1;
   Evas_Coord   alloc_size;
};    /**< #Evas_Object_Box_Option struct fields */

#define EVAS_OBJ_BOX_CLASS evas_object_box_class_get()

const Eo_Class *evas_object_box_class_get(void) EINA_CONST;

extern EAPI Eo_Op EVAS_OBJ_BOX_BASE_ID;

enum
{
   EVAS_OBJ_BOX_SUB_ID_INTERNAL_APPEND,
   EVAS_OBJ_BOX_SUB_ID_INTERNAL_PREPEND,
   EVAS_OBJ_BOX_SUB_ID_INTERNAL_INSERT_BEFORE,
   EVAS_OBJ_BOX_SUB_ID_INTERNAL_INSERT_AFTER,
   EVAS_OBJ_BOX_SUB_ID_INTERNAL_INSERT_AT,
   EVAS_OBJ_BOX_SUB_ID_INTERNAL_REMOVE,
   EVAS_OBJ_BOX_SUB_ID_INTERNAL_REMOVE_AT,
   EVAS_OBJ_BOX_SUB_ID_INTERNAL_OPTION_NEW,
   EVAS_OBJ_BOX_SUB_ID_INTERNAL_OPTION_FREE,

   EVAS_OBJ_BOX_SUB_ID_ADD_TO,
   EVAS_OBJ_BOX_SUB_ID_LAYOUT_SET,
   EVAS_OBJ_BOX_SUB_ID_LAYOUT_HORIZONTAL,
   EVAS_OBJ_BOX_SUB_ID_LAYOUT_VERTICAL,
   EVAS_OBJ_BOX_SUB_ID_LAYOUT_HOMOGENEOUS_HORIZONTAL,
   EVAS_OBJ_BOX_SUB_ID_LAYOUT_HOMOGENEOUS_VERTICAL,
   EVAS_OBJ_BOX_SUB_ID_LAYOUT_HOMOGENEOUS_MAX_SIZE_HORIZONTAL,
   EVAS_OBJ_BOX_SUB_ID_LAYOUT_HOMOGENEOUS_MAX_SIZE_VERTICAL,
   EVAS_OBJ_BOX_SUB_ID_LAYOUT_FLOW_HORIZONTAL,
   EVAS_OBJ_BOX_SUB_ID_LAYOUT_FLOW_VERTICAL,
   EVAS_OBJ_BOX_SUB_ID_LAYOUT_STACK,
   EVAS_OBJ_BOX_SUB_ID_ALIGN_SET,
   EVAS_OBJ_BOX_SUB_ID_ALIGN_GET,
   EVAS_OBJ_BOX_SUB_ID_PADDING_SET,
   EVAS_OBJ_BOX_SUB_ID_PADDING_GET,
   EVAS_OBJ_BOX_SUB_ID_APPEND,
   EVAS_OBJ_BOX_SUB_ID_PREPEND,
   EVAS_OBJ_BOX_SUB_ID_INSERT_BEFORE,
   EVAS_OBJ_BOX_SUB_ID_INSERT_AFTER,
   EVAS_OBJ_BOX_SUB_ID_INSERT_AT,
   EVAS_OBJ_BOX_SUB_ID_REMOVE,
   EVAS_OBJ_BOX_SUB_ID_REMOVE_AT,
   EVAS_OBJ_BOX_SUB_ID_REMOVE_ALL,
   EVAS_OBJ_BOX_SUB_ID_ITERATOR_NEW,
   EVAS_OBJ_BOX_SUB_ID_ACCESSOR_NEW,
   EVAS_OBJ_BOX_SUB_ID_OPTION_PROPERTY_NAME_GET,
   EVAS_OBJ_BOX_SUB_ID_OPTION_PROPERTY_ID_GET,
   EVAS_OBJ_BOX_SUB_ID_OPTION_PROPERTY_VSET,
   EVAS_OBJ_BOX_SUB_ID_OPTION_PROPERTY_VGET,
   EVAS_OBJ_BOX_SUB_ID_LAST
};

#define EVAS_OBJ_BOX_ID(sub_id) (EVAS_OBJ_BOX_BASE_ID + sub_id)

#define evas_obj_box_internal_append(child, option) EVAS_OBJ_BOX_ID(EVAS_OBJ_BOX_SUB_ID_INTERNAL_APPEND), EO_TYPECHECK(Evas_Object *, child), EO_TYPECHECK(Evas_Object_Box_Option **, option)
#define evas_obj_box_internal_prepend(child, option) EVAS_OBJ_BOX_ID(EVAS_OBJ_BOX_SUB_ID_INTERNAL_PREPEND), EO_TYPECHECK(Evas_Object *, child), EO_TYPECHECK(Evas_Object_Box_Option **, option)
#define evas_obj_box_internal_insert_before(child, reference, option) EVAS_OBJ_BOX_ID(EVAS_OBJ_BOX_SUB_ID_INTERNAL_INSERT_BEFORE), EO_TYPECHECK(Evas_Object *, child), EO_TYPECHECK(const Evas_Object *, reference), EO_TYPECHECK(Evas_Object_Box_Option **, option)
#define evas_obj_box_internal_insert_after(child, reference, option) EVAS_OBJ_BOX_ID(EVAS_OBJ_BOX_SUB_ID_INTERNAL_INSERT_AFTER), EO_TYPECHECK(Evas_Object *, child), EO_TYPECHECK(const Evas_Object *, reference), EO_TYPECHECK(Evas_Object_Box_Option **, option)
#define evas_obj_box_internal_insert_at(child, pos, option) EVAS_OBJ_BOX_ID(EVAS_OBJ_BOX_SUB_ID_INTERNAL_INSERT_AT), EO_TYPECHECK(Evas_Object *, child), EO_TYPECHECK(unsigned int, pos), EO_TYPECHECK(Evas_Object_Box_Option **, option)
#define evas_obj_box_internal_remove(child, result) EVAS_OBJ_BOX_ID(EVAS_OBJ_BOX_SUB_ID_INTERNAL_REMOVE), EO_TYPECHECK(Evas_Object *, child), EO_TYPECHECK(Evas_Object **, result)
#define evas_obj_box_internal_remove_at(pos, result) EVAS_OBJ_BOX_ID(EVAS_OBJ_BOX_SUB_ID_INTERNAL_REMOVE_AT), EO_TYPECHECK(unsigned int, pos), EO_TYPECHECK(Evas_Object **, result)
#define evas_obj_box_internal_option_new(child, ret) EVAS_OBJ_BOX_ID(EVAS_OBJ_BOX_SUB_ID_INTERNAL_OPTION_NEW), EO_TYPECHECK(Evas_Object *, child), EO_TYPECHECK(Evas_Object_Box_Option **, ret)
#define evas_obj_box_internal_option_free(opt) EVAS_OBJ_BOX_ID(EVAS_OBJ_BOX_SUB_ID_INTERNAL_OPTION_FREE), EO_TYPECHECK(Evas_Object_Box_Option *, opt)


/**
 * @def evas_obj_box_add_to
 * @since 1.8
 *
 * Add a new box as a child of a given smart object.
 *
 * @param[in] o
 *
 * @see evas_object_box_add_to
 */
#define evas_obj_box_add_to(o) EVAS_OBJ_BOX_ID(EVAS_OBJ_BOX_SUB_ID_ADD_TO), EO_TYPECHECK(Evas_Object **, o)

/**
 * @def evas_obj_box_layout_set
 * @since 1.8
 *
 * Set a new layouting function to a given box object
 *
 * @param[in] cb
 * @param[in] data
 * @param[in] free_data
 *
 * @see evas_object_box_layout_set
 */
#define evas_obj_box_layout_set(cb, data, free_data) EVAS_OBJ_BOX_ID(EVAS_OBJ_BOX_SUB_ID_LAYOUT_SET), EO_TYPECHECK(Evas_Object_Box_Layout, cb), EO_TYPECHECK(const void *, data), EO_TYPECHECK(Eina_Free_Cb, free_data)

/**
 * @def evas_obj_box_layout_horizontal
 * @since 1.8
 *
 * Layout function which sets the box o to a (basic) horizontal box
 *
 *
 * @see evas_object_box_layout_horizontal
 */
#define evas_obj_box_layout_horizontal() EVAS_OBJ_BOX_ID(EVAS_OBJ_BOX_SUB_ID_LAYOUT_HORIZONTAL)

/**
 * @def evas_obj_box_layout_vertical
 * @since 1.8
 *
 * Layout function which sets the box o to a (basic) vertical box
 *
 *
 * @see evas_object_box_layout_vertical
 */
#define evas_obj_box_layout_vertical() EVAS_OBJ_BOX_ID(EVAS_OBJ_BOX_SUB_ID_LAYOUT_VERTICAL)

/**
 * @def evas_obj_box_layout_homogeneous_horizontal
 * @since 1.8
 *
 * Layout function which sets the box o to a @b homogeneous
 *
 *
 * @see evas_object_box_layout_homogeneous_horizontal
 */
#define evas_obj_box_layout_homogeneous_horizontal() EVAS_OBJ_BOX_ID(EVAS_OBJ_BOX_SUB_ID_LAYOUT_HOMOGENEOUS_HORIZONTAL)

/**
 * @def evas_obj_box_layout_homogeneous_vertical
 * @since 1.8
 *
 * Layout function which sets the box o to a @b homogeneous
 *
 *
 * @see evas_object_box_layout_homogeneous_vertical
 */
#define evas_obj_box_layout_homogeneous_vertical() EVAS_OBJ_BOX_ID(EVAS_OBJ_BOX_SUB_ID_LAYOUT_HOMOGENEOUS_VERTICAL)

/**
 * @def evas_obj_box_layout_homogeneous_max_size_horizontal
 * @since 1.8
 *
 * Layout function which sets the box o to a maximum size.
 *
 *
 * @see evas_object_box_layout_homogeneous_max_size_horizontal
 */
#define evas_obj_box_layout_homogeneous_max_size_horizontal() EVAS_OBJ_BOX_ID(EVAS_OBJ_BOX_SUB_ID_LAYOUT_HOMOGENEOUS_MAX_SIZE_HORIZONTAL)

/**
 * @def evas_obj_box_layout_homogeneous_max_size_vertical
 * @since 1.8
 *
 * Layout function which sets the box o to a <b>maximum size,
 *
 *
 * @see evas_object_box_layout_homogeneous_max_size_vertical
 */
#define evas_obj_box_layout_homogeneous_max_size_vertical() EVAS_OBJ_BOX_ID(EVAS_OBJ_BOX_SUB_ID_LAYOUT_HOMOGENEOUS_MAX_SIZE_VERTICAL)

/**
 * @def evas_obj_box_layout_flow_horizontal
 * @since 1.8
 *
 * Layout function which sets the box o to a @b flow horizontal
 *
 *
 * @see evas_object_box_layout_flow_horizontal
 */
#define evas_obj_box_layout_flow_horizontal() EVAS_OBJ_BOX_ID(EVAS_OBJ_BOX_SUB_ID_LAYOUT_FLOW_HORIZONTAL)

/**
 * @def evas_obj_box_layout_flow_vertical
 * @since 1.8
 *
 * Layout function which sets the box o to a @b flow vertical box.
 *
 *
 * @see evas_object_box_layout_flow_vertical
 */
#define evas_obj_box_layout_flow_vertical() EVAS_OBJ_BOX_ID(EVAS_OBJ_BOX_SUB_ID_LAYOUT_FLOW_VERTICAL)

/**
 * @def evas_obj_box_layout_stack
 * @since 1.8
 *
 * Layout function which sets the box o to a @b stacking box
 *
 *
 * @see evas_object_box_layout_stack
 */
#define evas_obj_box_layout_stack() EVAS_OBJ_BOX_ID(EVAS_OBJ_BOX_SUB_ID_LAYOUT_STACK)

/**
 * @def evas_obj_box_align_set
 * @since 1.8
 *
 * Set the alignment of the whole bounding box of contents, for a
 *
 * @param[in] horizontal
 * @param[in] vertical
 *
 * @see evas_object_box_align_set
 */
#define evas_obj_box_align_set(horizontal, vertical) EVAS_OBJ_BOX_ID(EVAS_OBJ_BOX_SUB_ID_ALIGN_SET), EO_TYPECHECK(double, horizontal), EO_TYPECHECK(double, vertical)

/**
 * @def evas_obj_box_align_get
 * @since 1.8
 *
 * Get the alignment of the whole bounding box of contents, for a
 *
 * @param[out] horizontal
 * @param[out] vertical
 *
 * @see evas_object_box_align_get
 */
#define evas_obj_box_align_get(horizontal, vertical) EVAS_OBJ_BOX_ID(EVAS_OBJ_BOX_SUB_ID_ALIGN_GET), EO_TYPECHECK(double *, horizontal), EO_TYPECHECK(double *, vertical)

/**
 * @def evas_obj_box_padding_set
 * @since 1.8
 *
 * Set the (space) padding between cells set for a given box object.
 *
 * @param[in] horizontal
 * @param[in] vertical
 *
 * @see evas_object_box_padding_set
 */
#define evas_obj_box_padding_set(horizontal, vertical) EVAS_OBJ_BOX_ID(EVAS_OBJ_BOX_SUB_ID_PADDING_SET), EO_TYPECHECK(Evas_Coord, horizontal), EO_TYPECHECK(Evas_Coord, vertical)

/**
 * @def evas_obj_box_padding_get
 * @since 1.8
 *
 * Get the (space) padding between cells set for a given box object.
 *
 * @param[out] horizontal
 * @param[out] vertical
 *
 * @see evas_object_box_padding_get
 */
#define evas_obj_box_padding_get(horizontal, vertical) EVAS_OBJ_BOX_ID(EVAS_OBJ_BOX_SUB_ID_PADDING_GET), EO_TYPECHECK(Evas_Coord *, horizontal), EO_TYPECHECK(Evas_Coord *, vertical)

/**
 * @def evas_obj_box_append
 * @since 1.8
 *
 * Append a new child object to the given box object @a o.
 *
 * @param[in] child
 * @param[in] option
 *
 * @see evas_object_box_append
 */
#define evas_obj_box_append(child, option) EVAS_OBJ_BOX_ID(EVAS_OBJ_BOX_SUB_ID_APPEND), EO_TYPECHECK(Evas_Object *, child), EO_TYPECHECK(Evas_Object_Box_Option **, option)

/**
 * @def evas_obj_box_prepend
 * @since 1.8
 *
 * Prepend a new child object to the given box object @a o.
 *
 * @param[in] child
 * @param[in] option
 *
 * @see evas_object_box_prepend
 */
#define evas_obj_box_prepend(child, option) EVAS_OBJ_BOX_ID(EVAS_OBJ_BOX_SUB_ID_PREPEND), EO_TYPECHECK(Evas_Object *, child), EO_TYPECHECK(Evas_Object_Box_Option **, option)

/**
 * @def evas_obj_box_insert_before
 * @since 1.8
 *
 * Insert a new child object <b>before another existing one</b>, in
 *
 * @param[in] child
 * @param[in] reference
 * @param[in] option
 *
 * @see evas_object_box_insert_before
 */
#define evas_obj_box_insert_before(child, reference, option) EVAS_OBJ_BOX_ID(EVAS_OBJ_BOX_SUB_ID_INSERT_BEFORE), EO_TYPECHECK(Evas_Object *, child), EO_TYPECHECK(const Evas_Object *, reference), EO_TYPECHECK(Evas_Object_Box_Option **, option)

/**
 * @def evas_obj_box_insert_after
 * @since 1.8
 *
 * Insert a new child object <b>after another existing one</b>, in
 *
 * @param[in] child
 * @param[in] reference
 * @param[in] option
 *
 * @see evas_object_box_insert_after
 */
#define evas_obj_box_insert_after(child, reference, option) EVAS_OBJ_BOX_ID(EVAS_OBJ_BOX_SUB_ID_INSERT_AFTER), EO_TYPECHECK(Evas_Object *, child), EO_TYPECHECK(const Evas_Object *, reference), EO_TYPECHECK(Evas_Object_Box_Option **, option)

/**
 * @def evas_obj_box_insert_at
 * @since 1.8
 *
 * Insert a new child object <b>at a given position</b>, in a given
 *
 * @param[in] child
 * @param[in] pos
 * @param[in] option
 *
 * @see evas_object_box_insert_at
 */
#define evas_obj_box_insert_at(child, pos, option) EVAS_OBJ_BOX_ID(EVAS_OBJ_BOX_SUB_ID_INSERT_AT), EO_TYPECHECK(Evas_Object *, child), EO_TYPECHECK(unsigned int, pos), EO_TYPECHECK(Evas_Object_Box_Option **, option)

/**
 * @def evas_obj_box_remove
 * @since 1.8
 *
 * Remove a given object from a box object, unparenting it again.
 *
 * @param[in] child
 * @param[out] result
 *
 * @see evas_object_box_remove
 */
#define evas_obj_box_remove(child, result) EVAS_OBJ_BOX_ID(EVAS_OBJ_BOX_SUB_ID_REMOVE), EO_TYPECHECK(Evas_Object *, child), EO_TYPECHECK(Eina_Bool *, result)

/**
 * @def evas_obj_box_remove_at
 * @since 1.8
 *
 * Remove an object, <b>bound to a given position</b> in a box object,
 *
 * @param[in] pos
 * @param[out] result
 *
 * @see evas_object_box_remove_at
 */
#define evas_obj_box_remove_at(pos, result) EVAS_OBJ_BOX_ID(EVAS_OBJ_BOX_SUB_ID_REMOVE_AT), EO_TYPECHECK(unsigned int, pos), EO_TYPECHECK(Eina_Bool *, result)

/**
 * @def evas_obj_box_remove_all
 * @since 1.8
 *
 * Remove all child objects from a box object, unparenting them
 *
 * @param[in] clear
 * @param[out] result
 *
 * @see evas_object_box_remove_all
 */
#define evas_obj_box_remove_all(clear, result) EVAS_OBJ_BOX_ID(EVAS_OBJ_BOX_SUB_ID_REMOVE_ALL), EO_TYPECHECK(Eina_Bool, clear), EO_TYPECHECK(Eina_Bool *, result)

/**
 * @def evas_obj_box_iterator_new
 * @since 1.8
 *
 * Get an iterator to walk the list of children of a given box object.
 *
 * @param[in] itr
 *
 * @see evas_object_box_iterator_new
 */
#define evas_obj_box_iterator_new(itr) EVAS_OBJ_BOX_ID(EVAS_OBJ_BOX_SUB_ID_ITERATOR_NEW), EO_TYPECHECK(Eina_Iterator **, itr)

/**
 * @def evas_obj_box_accessor_new
 * @since 1.8
 *
 * Get an accessor (a structure providing random items access) to the
 *
 * @param[in] accessor
 *
 * @see evas_object_box_accessor_new
 */
#define evas_obj_box_accessor_new(accessor) EVAS_OBJ_BOX_ID(EVAS_OBJ_BOX_SUB_ID_ACCESSOR_NEW), EO_TYPECHECK(Eina_Accessor **, accessor)

/**
 * @def evas_obj_box_option_property_name_get
 * @since 1.8
 *
 * Get the name of the property of the child elements of the box o
 *
 * @param[in] property
 * @param[in] name
 *
 * @see evas_object_box_option_property_name_get
 */
#define evas_obj_box_option_property_name_get(property, name) EVAS_OBJ_BOX_ID(EVAS_OBJ_BOX_SUB_ID_OPTION_PROPERTY_NAME_GET), EO_TYPECHECK(int, property), EO_TYPECHECK(const char **, name)

/**
 * @def evas_obj_box_option_property_id_get
 * @since 1.8
 *
 * Get the numerical identifier of the property of the child elements
 *
 * @param[in] name
 * @param[in] id
 *
 * @see evas_object_box_option_property_id_get
 */
#define evas_obj_box_option_property_id_get(name, id) EVAS_OBJ_BOX_ID(EVAS_OBJ_BOX_SUB_ID_OPTION_PROPERTY_ID_GET), EO_TYPECHECK(const char *, name), EO_TYPECHECK(int *, id)

/**
 * @def evas_obj_box_option_property_vset
 * @since 1.8
 *
 * Set a property value (by its given numerical identifier), on a
 *
 * @param[in] opt
 * @param[in] property
 * @param[in] args
 * @param[out] ret
 *
 * @see evas_object_box_option_property_vset
 */
#define evas_obj_box_option_property_vset(opt, property, args, ret) EVAS_OBJ_BOX_ID(EVAS_OBJ_BOX_SUB_ID_OPTION_PROPERTY_VSET), EO_TYPECHECK(Evas_Object_Box_Option *, opt), EO_TYPECHECK(int, property), EO_TYPECHECK(va_list *, args), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def evas_obj_box_option_property_vget
 * @since 1.8
 *
 * Get a property's value (by its given numerical identifier), on a
 *
 * @param[in] opt
 * @param[in] property
 * @param[in] args
 * @param[out] ret
 *
 * @see evas_object_box_option_property_vget
 */
#define evas_obj_box_option_property_vget(opt, property, args, ret) EVAS_OBJ_BOX_ID(EVAS_OBJ_BOX_SUB_ID_OPTION_PROPERTY_VGET), EO_TYPECHECK(Evas_Object_Box_Option *, opt), EO_TYPECHECK(int, property), EO_TYPECHECK(va_list *, args), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * Set the default box @a api struct (Evas_Object_Box_Api)
 * with the default values. May be used to extend that API.
 *
 * @param api The box API struct to set back, most probably with
 * overridden fields (on class extensions scenarios)
 */
EAPI void                       evas_object_box_smart_set(Evas_Object_Box_Api *api) EINA_ARG_NONNULL(1);

/**
 * Get the Evas box smart class, for inheritance purposes.
 *
 * @return the (canonical) Evas box smart class.
 *
 * The returned value is @b not to be modified, just use it as your
 * parent class.
 */
EAPI const Evas_Object_Box_Api *evas_object_box_smart_class_get(void) EINA_CONST;

/**
 * Set a new layouting function to a given box object
 *
 * @param o The box object to operate on.
 * @param cb The new layout function to set on @p o.
 * @param data Data pointer to be passed to @p cb.
 * @param free_data Function to free @p data, if need be.
 *
 * A box layout function affects how a box object displays child
 * elements within its area. The list of pre-defined box layouts
 * available in Evas is:
 * - evas_object_box_layout_horizontal()
 * - evas_object_box_layout_vertical()
 * - evas_object_box_layout_homogeneous_horizontal()
 * - evas_object_box_layout_homogeneous_vertical()
 * - evas_object_box_layout_homogeneous_max_size_horizontal()
 * - evas_object_box_layout_homogeneous_max_size_vertical()
 * - evas_object_box_layout_flow_horizontal()
 * - evas_object_box_layout_flow_vertical()
 * - evas_object_box_layout_stack()
 *
 * Refer to each of their documentation texts for details on them.
 *
 * @note A box layouting function will be triggered by the @c
 * 'calculate' smart callback of the box's smart class.
 */
EAPI void                       evas_object_box_layout_set(Evas_Object *o, Evas_Object_Box_Layout cb, const void *data, void (*free_data)(void *data)) EINA_ARG_NONNULL(1, 2);

/**
 * Add a new box object on the provided canvas.
 *
 * @param evas The canvas to create the box object on.
 * @return @c NULL on error, a pointer to a new box object on
 * success.
 *
 * After instantiation, if a box object hasn't its layout function
 * set, via evas_object_box_layout_set(), it will have it by default
 * set to evas_object_box_layout_horizontal(). The remaining
 * properties of the box must be set/retrieved via
 * <c>evas_object_box_{h,v}_{align,padding}_{get,set)()</c>.
 */
EAPI Evas_Object               *evas_object_box_add(Evas *evas) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * Add a new box as a @b child of a given smart object.
 *
 * @param parent The parent smart object to put the new box in.
 * @return @c NULL on error, a pointer to a new box object on
 * success.
 *
 * This is a helper function that has the same effect of putting a new
 * box object into @p parent by use of evas_object_smart_member_add().
 *
 * @see evas_object_box_add()
 */
EAPI Evas_Object               *evas_object_box_add_to(Evas_Object *parent) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * Layout function which sets the box @a o to a (basic) horizontal box
 *
 * @param o The box object in question
 * @param priv The smart data of the @p o
 * @param data The data pointer passed on
 * evas_object_box_layout_set(), if any
 *
 * In this layout, the box object's overall behavior is controlled by
 * its padding/alignment properties, which are set by the
 * <c>evas_object_box_{h,v}_{align,padding}_set()</c> family of
 * functions. The size hints of the elements in the box -- set by the
 * <c>evas_object_size_hint_{align,padding,weight}_set()</c> functions
 * -- also control the way this function works.
 *
 * \par Box's properties:
 * @c align_h controls the horizontal alignment of the child objects
 * relative to the containing box. When set to @c 0.0, children are
 * aligned to the left. A value of @c 1.0 makes them aligned to the
 * right border. Values in between align them proportionally. Note
 * that if the size required by the children, which is given by their
 * widths and the @c padding_h property of the box, is bigger than the
 * their container's width, the children will be displayed out of the
 * box's bounds. A negative value of @c align_h makes the box to
 * @b justify its children. The padding between them, in this case, is
 * corrected so that the leftmost one touches the left border and the
 * rightmost one touches the right border (even if they must
 * overlap). The @c align_v and @c padding_v properties of the box
 * @b don't contribute to its behaviour when this layout is chosen.
 *
 * \par Child element's properties:
 * @c align_x does @b not influence the box's behavior. @c padding_l
 * and @c padding_r sum up to the container's horizontal padding
 * between elements. The child's @c padding_t, @c padding_b and
 * @c align_y properties apply for padding/alignment relative to the
 * overall height of the box. Finally, there is the @c weight_x
 * property, which, if set to a non-zero value, tells the container
 * that the child width is @b not pre-defined. If the container can't
 * accommodate all its children, it sets the widths of the ones
 * <b>with weights</b> to sizes as small as they can all fit into
 * it. If the size required by the children is less than the
 * available, the box increases its childrens' (which have weights)
 * widths as to fit the remaining space. The @c weight_x property,
 * besides telling the element is resizable, gives a @b weight for the
 * resizing process.  The parent box will try to distribute (or take
 * off) widths accordingly to the @b normalized list of weigths: most
 * weighted children remain/get larger in this process than the least
 * ones. @c weight_y does not influence the layout.
 *
 * If one desires that, besides having weights, child elements must be
 * resized bounded to a minimum or maximum size, those size hints must
 * be set, by the <c>evas_object_size_hint_{min,max}_set()</c>
 * functions.
 */
EAPI void                       evas_object_box_layout_horizontal(Evas_Object *o, Evas_Object_Box_Data *priv, void *data) EINA_ARG_NONNULL(1, 2);

/**
 * Layout function which sets the box @a o to a (basic) vertical box
 *
 * This function behaves analogously to
 * evas_object_box_layout_horizontal(). The description of its
 * behaviour can be derived from that function's documentation.
 */
EAPI void                       evas_object_box_layout_vertical(Evas_Object *o, Evas_Object_Box_Data *priv, void *data) EINA_ARG_NONNULL(1, 2);

/**
 * Layout function which sets the box @a o to a @b homogeneous
 * vertical box
 *
 * This function behaves analogously to
 * evas_object_box_layout_homogeneous_horizontal().  The description
 * of its behaviour can be derived from that function's documentation.
 */
EAPI void                       evas_object_box_layout_homogeneous_vertical(Evas_Object *o, Evas_Object_Box_Data *priv, void *data) EINA_ARG_NONNULL(1, 2);

/**
 * Layout function which sets the box @a o to a @b homogeneous
 * horizontal box
 *
 * @param o The box object in question
 * @param priv The smart data of the @p o
 * @param data The data pointer passed on
 * evas_object_box_layout_set(), if any
 *
 * In a homogeneous horizontal box, its width is divided @b equally
 * between the contained objects. The box's overall behavior is
 * controlled by its padding/alignment properties, which are set by
 * the <c>evas_object_box_{h,v}_{align,padding}_set()</c> family of
 * functions.  The size hints the elements in the box -- set by the
 * <c>evas_object_size_hint_{align,padding,weight}_set()</c> functions
 * -- also control the way this function works.
 *
 * \par Box's properties:
 * @c align_h has no influence on the box for this layout.
 * @c padding_h tells the box to draw empty spaces of that size, in
 * pixels, between the (equal) child objects's cells. The @c align_v
 * and @c padding_v properties of the box don't contribute to its
 * behaviour when this layout is chosen.
 *
 * \par Child element's properties:
 * @c padding_l and @c padding_r sum up to the required width of the
 * child element. The @c align_x property tells the relative position
 * of this overall child width in its allocated cell (@c 0.0 to
 * extreme left, @c 1.0 to extreme right). A value of @c -1.0 to
 * @c align_x makes the box try to resize this child element to the exact
 * width of its cell (respecting the minimum and maximum size hints on
 * the child's width and accounting for its horizontal padding
 * hints). The child's @c padding_t, @c padding_b and @c align_y
 * properties apply for padding/alignment relative to the overall
 * height of the box. A value of @c -1.0 to @c align_y makes the box
 * try to resize this child element to the exact height of its parent
 * (respecting the maximum size hint on the child's height).
 */
EAPI void                       evas_object_box_layout_homogeneous_horizontal(Evas_Object *o, Evas_Object_Box_Data *priv, void *data) EINA_ARG_NONNULL(1, 2);

/**
 * Layout function which sets the box @a o to a <b>maximum size,
 * homogeneous</b> horizontal box
 *
 * @param o The box object in question
 * @param priv The smart data of the @p o
 * @param data The data pointer passed on
 * evas_object_box_layout_set(), if any
 *
 * In a maximum size, homogeneous horizontal box, besides having cells
 * of <b>equal size</b> reserved for the child objects, this size will
 * be defined by the size of the @b largest child in the box (in
 * width). The box's overall behavior is controlled by its properties,
 * which are set by the
 * <c>evas_object_box_{h,v}_{align,padding}_set()</c> family of
 * functions.  The size hints of the elements in the box -- set by the
 * <c>evas_object_size_hint_{align,padding,weight}_set()</c> functions
 * -- also control the way this function works.
 *
 * \par Box's properties:
 * @c padding_h tells the box to draw empty spaces of that size, in
 * pixels, between the child objects's cells. @c align_h controls the
 * horizontal alignment of the child objects, relative to the
 * containing box. When set to @c 0.0, children are aligned to the
 * left. A value of @c 1.0 lets them aligned to the right
 * border. Values in between align them proportionally. A negative
 * value of @c align_h makes the box to @b justify its children
 * cells. The padding between them, in this case, is corrected so that
 * the leftmost one touches the left border and the rightmost one
 * touches the right border (even if they must overlap). The
 * @c align_v and @c padding_v properties of the box don't contribute to
 * its behaviour when this layout is chosen.
 *
 * \par Child element's properties:
 * @c padding_l and @c padding_r sum up to the required width of the
 * child element. The @c align_x property tells the relative position
 * of this overall child width in its allocated cell (@c 0.0 to
 * extreme left, @c 1.0 to extreme right). A value of @c -1.0 to
 * @c align_x makes the box try to resize this child element to the exact
 * width of its cell (respecting the minimum and maximum size hints on
 * the child's width and accounting for its horizontal padding
 * hints). The child's @c padding_t, @c padding_b and @c align_y
 * properties apply for padding/alignment relative to the overall
 * height of the box. A value of @c -1.0 to @c align_y makes the box
 * try to resize this child element to the exact height of its parent
 * (respecting the max hint on the child's height).
 */
EAPI void                       evas_object_box_layout_homogeneous_max_size_horizontal(Evas_Object *o, Evas_Object_Box_Data *priv, void *data) EINA_ARG_NONNULL(1, 2);

/**
 * Layout function which sets the box @a o to a <b>maximum size,
 * homogeneous</b> vertical box
 *
 * This function behaves analogously to
 * evas_object_box_layout_homogeneous_max_size_horizontal(). The
 * description of its behaviour can be derived from that function's
 * documentation.
 */
EAPI void                       evas_object_box_layout_homogeneous_max_size_vertical(Evas_Object *o, Evas_Object_Box_Data *priv, void *data) EINA_ARG_NONNULL(1, 2);

/**
 * Layout function which sets the box @a o to a @b flow horizontal
 * box.
 *
 * @param o The box object in question
 * @param priv The smart data of the @p o
 * @param data The data pointer passed on
 * evas_object_box_layout_set(), if any
 *
 * In a flow horizontal box, the box's child elements are placed in
 * @b rows (think of text as an analogy). A row has as much elements as
 * can fit into the box's width. The box's overall behavior is
 * controlled by its properties, which are set by the
 * <c>evas_object_box_{h,v}_{align,padding}_set()</c> family of
 * functions.  The size hints of the elements in the box -- set by the
 * <c>evas_object_size_hint_{align,padding,weight}_set()</c> functions
 * -- also control the way this function works.
 *
 * \par Box's properties:
 * @c padding_h tells the box to draw empty spaces of that size, in
 * pixels, between the child objects's cells. @c align_h dictates the
 * horizontal alignment of the rows (@c 0.0 to left align them, @c 1.0
 * to right align). A value of @c -1.0 to @c align_h lets the rows
 * @b justified horizontally. @c align_v controls the vertical alignment
 * of the entire set of rows (@c 0.0 to top, @c 1.0 to bottom). A
 * value of @c -1.0 to @c align_v makes the box to @b justify the rows
 * vertically. The padding between them, in this case, is corrected so
 * that the first row touches the top border and the last one touches
 * the bottom border (even if they must overlap). @c padding_v has no
 * influence on the layout.
 *
 * \par Child element's properties:
 * @c padding_l and @c padding_r sum up to the required width of the
 * child element. The @c align_x property has no influence on the
 * layout. The child's @c padding_t and @c padding_b sum up to the
 * required height of the child element and is the only means (besides
 * row justifying) of setting space between rows. Note, however, that
 * @c align_y dictates positioning relative to the <b>largest
 * height</b> required by a child object in the actual row.
 */
EAPI void                       evas_object_box_layout_flow_horizontal(Evas_Object *o, Evas_Object_Box_Data *priv, void *data) EINA_ARG_NONNULL(1, 2);

/**
 * Layout function which sets the box @a o to a @b flow vertical box.
 *
 * This function behaves analogously to
 * evas_object_box_layout_flow_horizontal(). The description of its
 * behaviour can be derived from that function's documentation.
 */
EAPI void                       evas_object_box_layout_flow_vertical(Evas_Object *o, Evas_Object_Box_Data *priv, void *data) EINA_ARG_NONNULL(1, 2);

/**
 * Layout function which sets the box @a o to a @b stacking box
 *
 * @param o The box object in question
 * @param priv The smart data of the @p o
 * @param data The data pointer passed on
 * evas_object_box_layout_set(), if any
 *
 * In a stacking box, all children will be given the same size -- the
 * box's own size -- and they will be stacked one above the other, so
 * that the first object in @p o's internal list of child elements
 * will be the bottommost in the stack.
 *
 * \par Box's properties:
 * No box properties are used.
 *
 * \par Child element's properties:
 * @c padding_l and @c padding_r sum up to the required width of the
 * child element. The @c align_x property tells the relative position
 * of this overall child width in its allocated cell (@c 0.0 to
 * extreme left, @c 1.0 to extreme right). A value of @c -1.0 to @c
 * align_x makes the box try to resize this child element to the exact
 * width of its cell (respecting the min and max hints on the child's
 * width and accounting for its horizontal padding properties). The
 * same applies to the vertical axis.
 */
EAPI void                       evas_object_box_layout_stack(Evas_Object *o, Evas_Object_Box_Data *priv, void *data) EINA_ARG_NONNULL(1, 2);

/**
 * Set the alignment of the whole bounding box of contents, for a
 * given box object.
 *
 * @param o The given box object to set alignment from
 * @param horizontal The horizontal alignment, in pixels
 * @param vertical the vertical alignment, in pixels
 *
 * This will influence how a box object is to align its bounding box
 * of contents within its own area. The values @b must be in the range
 * @c 0.0 - @c 1.0, or undefined behavior is expected. For horizontal
 * alignment, @c 0.0 means to the left, with @c 1.0 meaning to the
 * right. For vertical alignment, @c 0.0 means to the top, with @c 1.0
 * meaning to the bottom.
 *
 * @note The default values for both alignments is @c 0.5.
 *
 * @see evas_object_box_align_get()
 */
EAPI void                       evas_object_box_align_set(Evas_Object *o, double horizontal, double vertical) EINA_ARG_NONNULL(1);

/**
 * Get the alignment of the whole bounding box of contents, for a
 * given box object.
 *
 * @param o The given box object to get alignment from
 * @param horizontal Pointer to a variable where to store the
 * horizontal alignment
 * @param vertical Pointer to a variable where to store the vertical
 * alignment
 *
 * @see evas_object_box_align_set() for more information
 */
EAPI void                       evas_object_box_align_get(const Evas_Object *o, double *horizontal, double *vertical) EINA_ARG_NONNULL(1);

/**
 * Set the (space) padding between cells set for a given box object.
 *
 * @param o The given box object to set padding from
 * @param horizontal The horizontal padding, in pixels
 * @param vertical the vertical padding, in pixels
 *
 * @note The default values for both padding components is @c 0.
 *
 * @see evas_object_box_padding_get()
 */
EAPI void                       evas_object_box_padding_set(Evas_Object *o, Evas_Coord horizontal, Evas_Coord vertical) EINA_ARG_NONNULL(1);

/**
 * Get the (space) padding between cells set for a given box object.
 *
 * @param o The given box object to get padding from
 * @param horizontal Pointer to a variable where to store the
 * horizontal padding
 * @param vertical Pointer to a variable where to store the vertical
 * padding
 *
 * @see evas_object_box_padding_set()
 */
EAPI void                       evas_object_box_padding_get(const Evas_Object *o, Evas_Coord *horizontal, Evas_Coord *vertical) EINA_ARG_NONNULL(1);

/**
 * Append a new @a child object to the given box object @a o.
 *
 * @param o The given box object
 * @param child A child Evas object to be made a member of @p o
 * @return A box option bound to the recently added box item or @c
 * NULL, on errors
 *
 * On success, the @c "child,added" smart event will take place.
 *
 * @note The actual placing of the item relative to @p o's area will
 * depend on the layout set to it. For example, on horizontal layouts
 * an item in the end of the box's list of children will appear on its
 * right.
 *
 * @note This call will trigger the box's _Evas_Object_Box_Api::append
 * smart function.
 */
EAPI Evas_Object_Box_Option    *evas_object_box_append(Evas_Object *o, Evas_Object *child) EINA_ARG_NONNULL(1, 2);

/**
 * Prepend a new @a child object to the given box object @a o.
 *
 * @param o The given box object
 * @param child A child Evas object to be made a member of @p o
 * @return A box option bound to the recently added box item or @c
 * NULL, on errors
 *
 * On success, the @c "child,added" smart event will take place.
 *
 * @note The actual placing of the item relative to @p o's area will
 * depend on the layout set to it. For example, on horizontal layouts
 * an item in the beginning of the box's list of children will appear
 * on its left.
 *
 * @note This call will trigger the box's
 * _Evas_Object_Box_Api::prepend smart function.
 */
EAPI Evas_Object_Box_Option    *evas_object_box_prepend(Evas_Object *o, Evas_Object *child) EINA_ARG_NONNULL(1, 2);

/**
 * Insert a new @a child object <b>before another existing one</b>, in
 * a given box object @a o.
 *
 * @param o The given box object
 * @param child A child Evas object to be made a member of @p o
 * @param reference The child object to place this new one before
 * @return A box option bound to the recently added box item or @c
 * NULL, on errors
 *
 * On success, the @c "child,added" smart event will take place.
 *
 * @note This function will fail if @p reference is not a member of @p
 * o.
 *
 * @note The actual placing of the item relative to @p o's area will
 * depend on the layout set to it.
 *
 * @note This call will trigger the box's
 * _Evas_Object_Box_Api::insert_before smart function.
 */
EAPI Evas_Object_Box_Option    *evas_object_box_insert_before(Evas_Object *o, Evas_Object *child, const Evas_Object *reference) EINA_ARG_NONNULL(1, 2, 3);

/**
 * Insert a new @a child object <b>after another existing one</b>, in
 * a given box object @a o.
 *
 * @param o The given box object
 * @param child A child Evas object to be made a member of @p o
 * @param reference The child object to place this new one after
 * @return A box option bound to the recently added box item or @c
 * NULL, on errors
 *
 * On success, the @c "child,added" smart event will take place.
 *
 * @note This function will fail if @p reference is not a member of @p
 * o.
 *
 * @note The actual placing of the item relative to @p o's area will
 * depend on the layout set to it.
 *
 * @note This call will trigger the box's
 * _Evas_Object_Box_Api::insert_after smart function.
 */
EAPI Evas_Object_Box_Option    *evas_object_box_insert_after(Evas_Object *o, Evas_Object *child, const Evas_Object *reference) EINA_ARG_NONNULL(1, 2, 3);

/**
 * Insert a new @a child object <b>at a given position</b>, in a given
 * box object @a o.
 *
 * @param o The given box object
 * @param child A child Evas object to be made a member of @p o
 * @param pos The numeric position (starting from @c 0) to place the
 * new child object at
 * @return A box option bound to the recently added box item or @c
 * NULL, on errors
 *
 * On success, the @c "child,added" smart event will take place.
 *
 * @note This function will fail if the given position is invalid,
 * given @p o's internal list of elements.
 *
 * @note The actual placing of the item relative to @p o's area will
 * depend on the layout set to it.
 *
 * @note This call will trigger the box's
 * _Evas_Object_Box_Api::insert_at smart function.
 */
EAPI Evas_Object_Box_Option    *evas_object_box_insert_at(Evas_Object *o, Evas_Object *child, unsigned int pos) EINA_ARG_NONNULL(1, 2);

/**
 * Remove a given object from a box object, unparenting it again.
 *
 * @param o The box object to remove a child object from
 * @param child The handle to the child object to be removed
 * @return @c EINA_TRUE, on success, @c EINA_FALSE otherwise
 *
 * On removal, you'll get an unparented object again, just as it was
 * before you inserted it in the box. The
 * _Evas_Object_Box_Api::option_free box smart callback will be called
 * automatically for you and, also, the @c "child,removed" smart event
 * will take place.
 *
 * @note This call will trigger the box's _Evas_Object_Box_Api::remove
 * smart function.
 */
EAPI Eina_Bool                  evas_object_box_remove(Evas_Object *o, Evas_Object *child) EINA_ARG_NONNULL(1, 2);

/**
 * Remove an object, <b>bound to a given position</b> in a box object,
 * unparenting it again.
 *
 * @param o The box object to remove a child object from
 * @param pos The numeric position (starting from @c 0) of the child
 * object to be removed
 * @return @c EINA_TRUE, on success, @c EINA_FALSE otherwise
 *
 * On removal, you'll get an unparented object again, just as it was
 * before you inserted it in the box. The @c option_free() box smart
 * callback will be called automatically for you and, also, the
 * @c "child,removed" smart event will take place.
 *
 * @note This function will fail if the given position is invalid,
 * given @p o's internal list of elements.
 *
 * @note This call will trigger the box's
 * _Evas_Object_Box_Api::remove_at smart function.
 */
EAPI Eina_Bool                  evas_object_box_remove_at(Evas_Object *o, unsigned int pos) EINA_ARG_NONNULL(1);

/**
 * Remove @b all child objects from a box object, unparenting them
 * again.
 *
 * @param o The box object to remove a child object from
 * @param clear if true, it will delete just removed children.
 * @return @c EINA_TRUE, on success, @c EINA_FALSE otherwise
 *
 * This has the same effect of calling evas_object_box_remove() on
 * each of @p o's child objects, in sequence. If, and only if, all
 * those calls succeed, so does this one.
 */
EAPI Eina_Bool                  evas_object_box_remove_all(Evas_Object *o, Eina_Bool clear) EINA_ARG_NONNULL(1);

/**
 * Get an iterator to walk the list of children of a given box object.
 *
 * @param o The box to retrieve an items iterator from
 * @return An iterator on @p o's child objects, on success, or @c NULL,
 * on errors
 *
 * @note Do @b not remove or delete objects while walking the list.
 */
EAPI Eina_Iterator             *evas_object_box_iterator_new(const Evas_Object *o) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * Get an accessor (a structure providing random items access) to the
 * list of children of a given box object.
 *
 * @param o The box to retrieve an items iterator from
 * @return An accessor on @p o's child objects, on success, or @c NULL,
 * on errors
 *
 * @note Do not remove or delete objects while walking the list.
 */
EAPI Eina_Accessor             *evas_object_box_accessor_new(const Evas_Object *o) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * Get the list of children objects in a given box object.
 *
 * @param o The box to retrieve an items list from
 * @return A list of @p o's child objects, on success, or @c NULL,
 * on errors (or if it has no child objects)
 *
 * The returned list should be freed with @c eina_list_free() when you
 * no longer need it.
 *
 * @note This is a duplicate of the list kept by the box internally.
 *       It's up to the user to destroy it when it no longer needs it.
 *       It's possible to remove objects from the box when walking
 *       this list, but these removals won't be reflected on it.
 */
EAPI Eina_List                 *evas_object_box_children_get(const Evas_Object *o) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * Get the name of the property of the child elements of the box @a o
 * which have @a id as identifier
 *
 * @param o The box to search child options from
 * @param property The numerical identifier of the option being searched,
 * for its name
 * @return The name of the given property or @c NULL, on errors.
 *
 * @note This call won't do anything for a canonical Evas box. Only
 * users which have @b subclassed it, setting custom box items options
 * (see #Evas_Object_Box_Option) on it, would benefit from this
 * function. They'd have to implement it and set it to be the
 * _Evas_Object_Box_Api::property_name_get smart class function of the
 * box, which is originally set to @c NULL.
 */
EAPI const char                *evas_object_box_option_property_name_get(const Evas_Object *o, int property) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Get the numerical identifier of the property of the child elements
 * of the box @a o which have @a name as name string
 *
 * @param o The box to search child options from
 * @param name The name string of the option being searched, for
 * its ID
 * @return The numerical ID of the given property or @c -1, on
 * errors.
 *
 * @note This call won't do anything for a canonical Evas box. Only
 * users which have @b subclassed it, setting custom box items options
 * (see #Evas_Object_Box_Option) on it, would benefit from this
 * function. They'd have to implement it and set it to be the
 * _Evas_Object_Box_Api::property_id_get smart class function of the
 * box, which is originally set to @c NULL.
 */
EAPI int                        evas_object_box_option_property_id_get(const Evas_Object *o, const char *name) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1, 2);

/**
 * Set a property value (by its given numerical identifier), on a
 * given box child element
 *
 * @param o The box parenting the child element
 * @param opt The box option structure bound to the child box element
 * to set a property on
 * @param property The numerical ID of the given property
 * @param ... (List of) actual value(s) to be set for this
 * property. It (they) @b must be of the same type the user has
 * defined for it (them).
 * @return @c EINA_TRUE on success, @c EINA_FALSE on failure.
 *
 * @note This call won't do anything for a canonical Evas box. Only
 * users which have @b subclassed it, setting custom box items options
 * (see #Evas_Object_Box_Option) on it, would benefit from this
 * function. They'd have to implement it and set it to be the
 * _Evas_Object_Box_Api::property_set smart class function of the box,
 * which is originally set to @c NULL.
 *
 * @note This function will internally create a variable argument
 * list, with the values passed after @p property, and call
 * evas_object_box_option_property_vset() with this list and the same
 * previous arguments.
 */
EAPI Eina_Bool                  evas_object_box_option_property_set(Evas_Object *o, Evas_Object_Box_Option *opt, int property, ...) EINA_ARG_NONNULL(1, 2);

/**
 * Set a property value (by its given numerical identifier), on a
 * given box child element -- by a variable argument list
 *
 * @param o The box parenting the child element
 * @param opt The box option structure bound to the child box element
 * to set a property on
 * @param property The numerical ID of the given property
 * @param args The variable argument list implementing the value to
 * be set for this property. It @b must be of the same type the user has
 * defined for it.
 * @return @c EINA_TRUE on success, @c EINA_FALSE on failure.
 *
 * This is a variable argument list variant of the
 * evas_object_box_option_property_set(). See its documentation for
 * more details.
 */
EAPI Eina_Bool                  evas_object_box_option_property_vset(Evas_Object *o, Evas_Object_Box_Option *opt, int property, va_list args) EINA_ARG_NONNULL(1, 2);

/**
 * Get a property's value (by its given numerical identifier), on a
 * given box child element
 *
 * @param o The box parenting the child element
 * @param opt The box option structure bound to the child box element
 * to get a property from
 * @param property The numerical ID of the given property
 * @param ... (List of) pointer(s) where to store the value(s) set for
 * this property. It (they) @b must point to variable(s) of the same
 * type the user has defined for it (them).
 * @return @c EINA_TRUE on success, @c EINA_FALSE on failure.
 *
 * @note This call won't do anything for a canonical Evas box. Only
 * users which have @b subclassed it, getting custom box items options
 * (see #Evas_Object_Box_Option) on it, would benefit from this
 * function. They'd have to implement it and get it to be the
 * _Evas_Object_Box_Api::property_get smart class function of the
 * box, which is originally get to @c NULL.
 *
 * @note This function will internally create a variable argument
 * list, with the values passed after @p property, and call
 * evas_object_box_option_property_vget() with this list and the same
 * previous arguments.
 */
EAPI Eina_Bool                  evas_object_box_option_property_get(const Evas_Object *o, Evas_Object_Box_Option *opt, int property, ...) EINA_ARG_NONNULL(1, 2);

/**
 * Get a property's value (by its given numerical identifier), on a
 * given box child element -- by a variable argument list
 *
 * @param o The box parenting the child element
 * @param opt The box option structure bound to the child box element
 * to get a property from
 * @param property The numerical ID of the given property
 * @param args The variable argument list with pointers to where to
 * store the values of this property. They @b must point to variables
 * of the same type the user has defined for them.
 * @return @c EINA_TRUE on success, @c EINA_FALSE on failure.
 *
 * This is a variable argument list variant of the
 * evas_object_box_option_property_get(). See its documentation for
 * more details.
 */
EAPI Eina_Bool                  evas_object_box_option_property_vget(const Evas_Object *o, Evas_Object_Box_Option *opt, int property, va_list args) EINA_ARG_NONNULL(1, 2);

/**
 * @}
 */

/**
 * @defgroup Evas_Object_Table Table Smart Object.
 *
 * Convenience smart object that packs children using a tabular
 * layout using children size hints to define their size and
 * alignment inside their cell space.
 *
 * @ref tutorial_table shows how to use this Evas_Object.
 *
 * @see @ref Evas_Object_Group_Size_Hints
 *
 * @ingroup Evas_Smart_Object_Group
 *
 * @{
 */

#define EVAS_OBJ_TABLE_CLASS evas_object_table_class_get()

const Eo_Class *evas_object_table_class_get(void) EINA_CONST;

extern EAPI Eo_Op EVAS_OBJ_TABLE_BASE_ID;

enum
{
   EVAS_OBJ_TABLE_SUB_ID_ADD_TO,
   EVAS_OBJ_TABLE_SUB_ID_HOMOGENEOUS_SET,
   EVAS_OBJ_TABLE_SUB_ID_HOMOGENEOUS_GET,
   EVAS_OBJ_TABLE_SUB_ID_ALIGN_SET,
   EVAS_OBJ_TABLE_SUB_ID_ALIGN_GET,
   EVAS_OBJ_TABLE_SUB_ID_PADDING_SET,
   EVAS_OBJ_TABLE_SUB_ID_PADDING_GET,
   EVAS_OBJ_TABLE_SUB_ID_PACK_GET,
   EVAS_OBJ_TABLE_SUB_ID_PACK,
   EVAS_OBJ_TABLE_SUB_ID_UNPACK,
   EVAS_OBJ_TABLE_SUB_ID_CLEAR,
   EVAS_OBJ_TABLE_SUB_ID_COL_ROW_SIZE_GET,
   EVAS_OBJ_TABLE_SUB_ID_ITERATOR_NEW,
   EVAS_OBJ_TABLE_SUB_ID_ACCESSOR_NEW,
   EVAS_OBJ_TABLE_SUB_ID_CHILDREN_GET,
   EVAS_OBJ_TABLE_SUB_ID_MIRRORED_GET,
   EVAS_OBJ_TABLE_SUB_ID_MIRRORED_SET,
   EVAS_OBJ_TABLE_SUB_ID_LAST
};

#define EVAS_OBJ_TABLE_ID(sub_id) (EVAS_OBJ_TABLE_BASE_ID + sub_id)


/**
 * @def evas_obj_table_add_to
 * @since 1.8
 *
 * Create a table that is child of a given element parent.
 *
 * @param[out] ret
 *
 * @see evas_object_table_add_to
 */
#define evas_obj_table_add_to(ret) EVAS_OBJ_TABLE_ID(EVAS_OBJ_TABLE_SUB_ID_ADD_TO), EO_TYPECHECK(Evas_Object **, ret)

/**
 * @def evas_obj_table_homogeneous_set
 * @since 1.8
 *
 * Set how this table should layout children.
 *
 * @param[in] homogeneous
 *
 * @see evas_object_table_homogeneous_set
 */
#define evas_obj_table_homogeneous_set(homogeneous) EVAS_OBJ_TABLE_ID(EVAS_OBJ_TABLE_SUB_ID_HOMOGENEOUS_SET), EO_TYPECHECK(Evas_Object_Table_Homogeneous_Mode, homogeneous)

/**
 * @def evas_obj_table_homogeneous_get
 * @since 1.8
 *
 * Get the current layout homogeneous mode.
 *
 * @param[out] ret
 *
 * @see evas_object_table_homogeneous_get
 */
#define evas_obj_table_homogeneous_get(ret) EVAS_OBJ_TABLE_ID(EVAS_OBJ_TABLE_SUB_ID_HOMOGENEOUS_GET), EO_TYPECHECK(Evas_Object_Table_Homogeneous_Mode *, ret)

/**
 * @def evas_obj_table_align_set
 * @since 1.8
 *
 * Set the alignment of the whole bounding box of contents.
 *
 * @param[in] horizontal
 * @param[in] vertical
 *
 * @see evas_object_table_align_set
 */
#define evas_obj_table_align_set(horizontal, vertical) EVAS_OBJ_TABLE_ID(EVAS_OBJ_TABLE_SUB_ID_ALIGN_SET), EO_TYPECHECK(double, horizontal), EO_TYPECHECK(double, vertical)

/**
 * @def evas_obj_table_align_get
 * @since 1.8
 *
 * Get alignment of the whole bounding box of contents.
 *
 * @param[out] horizontal
 * @param[out] vertical
 *
 * @see evas_object_table_align_get
 */
#define evas_obj_table_align_get(horizontal, vertical) EVAS_OBJ_TABLE_ID(EVAS_OBJ_TABLE_SUB_ID_ALIGN_GET), EO_TYPECHECK(double *, horizontal), EO_TYPECHECK(double *, vertical)

/**
 * @def evas_obj_table_padding_set
 * @since 1.8
 *
 * Set padding between cells.
 *
 * @param[in] horizontal
 * @param[in] vertical
 *
 * @see evas_object_table_padding_set
 */
#define evas_obj_table_padding_set(horizontal, vertical) EVAS_OBJ_TABLE_ID(EVAS_OBJ_TABLE_SUB_ID_PADDING_SET), EO_TYPECHECK(Evas_Coord, horizontal), EO_TYPECHECK(Evas_Coord, vertical)

/**
 * @def evas_obj_table_padding_get
 * @since 1.8
 *
 * Get padding between cells.
 *
 * @param[out] horizontal
 * @param[out] vertical
 *
 * @see evas_object_table_padding_get
 */
#define evas_obj_table_padding_get(horizontal, vertical) EVAS_OBJ_TABLE_ID(EVAS_OBJ_TABLE_SUB_ID_PADDING_GET), EO_TYPECHECK(Evas_Coord *, horizontal), EO_TYPECHECK(Evas_Coord *, vertical)

/**
 * @def evas_obj_table_pack_get
 * @since 1.8
 *
 * Get packing location of a child of table
 *
 * @param[in] child
 * @param[out] col
 * @param[out] row
 * @param[out] colspan
 * @param[out] rowspan
 * @param[out] ret
 *
 * @see evas_object_table_pack_get
 */
#define evas_obj_table_pack_get(child, col, row, colspan, rowspan, ret) EVAS_OBJ_TABLE_ID(EVAS_OBJ_TABLE_SUB_ID_PACK_GET), EO_TYPECHECK(Evas_Object *, child), EO_TYPECHECK(unsigned short *, col), EO_TYPECHECK(unsigned short *, row), EO_TYPECHECK(unsigned short *, colspan), EO_TYPECHECK(unsigned short *, rowspan), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def evas_obj_table_pack
 * @since 1.8
 *
 * Add a new child to a table object or set its current packing.
 *
 * @param[in] child
 * @param[in] col
 * @param[in] row
 * @param[in] colspan
 * @param[in] rowspan
 * @param[out] ret
 *
 * @see evas_object_table_pack
 */
#define evas_obj_table_pack(child, col, row, colspan, rowspan, ret) EVAS_OBJ_TABLE_ID(EVAS_OBJ_TABLE_SUB_ID_PACK), EO_TYPECHECK(Evas_Object *, child), EO_TYPECHECK(unsigned short, col), EO_TYPECHECK(unsigned short, row), EO_TYPECHECK(unsigned short, colspan), EO_TYPECHECK(unsigned short, rowspan), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def evas_obj_table_unpack
 * @since 1.8
 *
 * Remove child from table.
 *
 * @param[in] child
 * @param[out] ret
 *
 * @see evas_object_table_unpack
 */
#define evas_obj_table_unpack(child, ret) EVAS_OBJ_TABLE_ID(EVAS_OBJ_TABLE_SUB_ID_UNPACK), EO_TYPECHECK(Evas_Object *, child), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def evas_obj_table_clear
 * @since 1.8
 *
 * Faster way to remove all child objects from a table object.
 *
 * @param[in] clear
 *
 * @see evas_object_table_clear
 */
#define evas_obj_table_clear(clear) EVAS_OBJ_TABLE_ID(EVAS_OBJ_TABLE_SUB_ID_CLEAR), EO_TYPECHECK(Eina_Bool, clear)

/**
 * @def evas_obj_table_col_row_size_get
 * @since 1.8
 *
 * Get the number of columns and rows this table takes.
 *
 * @param[out] cols
 * @param[out] rows
 *
 * @see evas_object_table_col_row_size_get
 */
#define evas_obj_table_col_row_size_get(cols, rows) EVAS_OBJ_TABLE_ID(EVAS_OBJ_TABLE_SUB_ID_COL_ROW_SIZE_GET), EO_TYPECHECK(int *, cols), EO_TYPECHECK(int *, rows)

/**
 * @def evas_obj_table_iterator_new
 * @since 1.8
 *
 * Get an iterator to walk the list of children for the table.
 *
 * @param[out] ret
 *
 * @see evas_object_table_iterator_new
 */
#define evas_obj_table_iterator_new(ret) EVAS_OBJ_TABLE_ID(EVAS_OBJ_TABLE_SUB_ID_ITERATOR_NEW), EO_TYPECHECK(Eina_Iterator **, ret)

/**
 * @def evas_obj_table_accessor_new
 * @since 1.8
 *
 * Get an accessor to get random access to the list of children for the table.
 *
 * @param[out] ret
 *
 * @see evas_object_table_accessor_new
 */
#define evas_obj_table_accessor_new(ret) EVAS_OBJ_TABLE_ID(EVAS_OBJ_TABLE_SUB_ID_ACCESSOR_NEW), EO_TYPECHECK(Eina_Accessor **, ret)

/**
 * @def evas_obj_table_children_get
 * @since 1.8
 *
 * Get the list of children for the table.
 *
 * @param[out] ret
 *
 * @see evas_object_table_children_get
 */
#define evas_obj_table_children_get(ret) EVAS_OBJ_TABLE_ID(EVAS_OBJ_TABLE_SUB_ID_CHILDREN_GET), EO_TYPECHECK(Eina_List **, ret)

/**
 * @def evas_obj_table_mirrored_get
 * @since 1.8
 *
 * Gets the mirrored mode of the table.
 *
 * @param[out] ret
 *
 * @see evas_object_table_mirrored_get
 */
#define evas_obj_table_mirrored_get(ret) EVAS_OBJ_TABLE_ID(EVAS_OBJ_TABLE_SUB_ID_MIRRORED_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def evas_obj_table_mirrored_set
 * @since 1.8
 *
 * Sets the mirrored mode of the table. In mirrored mode the table items go
 *
 * @param[in] mirrored
 *
 * @see evas_object_table_mirrored_set
 */
#define evas_obj_table_mirrored_set(mirrored) EVAS_OBJ_TABLE_ID(EVAS_OBJ_TABLE_SUB_ID_MIRRORED_SET), EO_TYPECHECK(Eina_Bool, mirrored)

/**
 * @brief Create a new table.
 *
 * @param evas Canvas in which table will be added.
 */
EAPI Evas_Object                       *evas_object_table_add(Evas *evas) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * @brief Create a table that is child of a given element @a parent.
 *
 * @see evas_object_table_add()
 */
EAPI Evas_Object                       *evas_object_table_add_to(Evas_Object *parent) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * @brief Set how this table should layout children.
 *
 * @todo consider aspect hint and respect it.
 *
 * @par EVAS_OBJECT_TABLE_HOMOGENEOUS_NONE
 * If table does not use homogeneous mode then columns and rows will
 * be calculated based on hints of individual cells. This operation
 * mode is more flexible, but more complex and heavy to calculate as
 * well. @b Weight properties are handled as a boolean expand. Negative
 * alignment will be considered as 0.5. This is the default.
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
EAPI void                               evas_object_table_homogeneous_set(Evas_Object *o, Evas_Object_Table_Homogeneous_Mode homogeneous) EINA_ARG_NONNULL(1);

/**
 * Get the current layout homogeneous mode.
 *
 * @see evas_object_table_homogeneous_set()
 */
EAPI Evas_Object_Table_Homogeneous_Mode evas_object_table_homogeneous_get(const Evas_Object *o) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Set padding between cells.
 */
EAPI void                               evas_object_table_padding_set(Evas_Object *o, Evas_Coord horizontal, Evas_Coord vertical) EINA_ARG_NONNULL(1);

/**
 * Get padding between cells.
 */
EAPI void                               evas_object_table_padding_get(const Evas_Object *o, Evas_Coord *horizontal, Evas_Coord *vertical) EINA_ARG_NONNULL(1);

/**
 * Set the alignment of the whole bounding box of contents.
 */
EAPI void                               evas_object_table_align_set(Evas_Object *o, double horizontal, double vertical) EINA_ARG_NONNULL(1);

/**
 * Get alignment of the whole bounding box of contents.
 */
EAPI void                               evas_object_table_align_get(const Evas_Object *o, double *horizontal, double *vertical) EINA_ARG_NONNULL(1);

/**
 * Sets the mirrored mode of the table. In mirrored mode the table items go
 * from right to left instead of left to right. That is, 1,1 is top right, not
 * top left.
 *
 * @param o The table object.
 * @param mirrored the mirrored mode to set
 * @since 1.1
 */
EAPI void                               evas_object_table_mirrored_set(Evas_Object *o, Eina_Bool mirrored) EINA_ARG_NONNULL(1);

/**
 * Gets the mirrored mode of the table.
 *
 * @param o The table object.
 * @return @c EINA_TRUE if it's a mirrored table, @c EINA_FALSE otherwise.
 * @since 1.1
 * @see evas_object_table_mirrored_set()
 */
EAPI Eina_Bool                          evas_object_table_mirrored_get(const Evas_Object *o) EINA_ARG_NONNULL(1);

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
 * @since 1.1
 */
EAPI Eina_Bool                          evas_object_table_pack_get(const Evas_Object *o, Evas_Object *child, unsigned short *col, unsigned short *row, unsigned short *colspan, unsigned short *rowspan);

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
EAPI Eina_Bool                          evas_object_table_pack(Evas_Object *o, Evas_Object *child, unsigned short col, unsigned short row, unsigned short colspan, unsigned short rowspan) EINA_ARG_NONNULL(1, 2);

/**
 * Remove child from table.
 *
 * @note removing a child will immediately call a walk over children in order
 *       to recalculate numbers of columns and rows. If you plan to remove
 *       all children, use evas_object_table_clear() instead.
 *
 * @return 1 on success, 0 on failure.
 */
EAPI Eina_Bool                          evas_object_table_unpack(Evas_Object *o, Evas_Object *child) EINA_ARG_NONNULL(1, 2);

/**
 * Faster way to remove all child objects from a table object.
 *
 * @param o The given table object.
 * @param clear if true, it will delete just removed children.
 */
EAPI void                               evas_object_table_clear(Evas_Object *o, Eina_Bool clear) EINA_ARG_NONNULL(1);

/**
 * Get the number of columns and rows this table takes.
 *
 * @note columns and rows are virtual entities, one can specify a table
 *       with a single object that takes 4 columns and 5 rows. The only
 *       difference for a single cell table is that paddings will be
 *       accounted proportionally.
 */
EAPI void                               evas_object_table_col_row_size_get(const Evas_Object *o, int *cols, int *rows) EINA_ARG_NONNULL(1);

/**
 * Get an iterator to walk the list of children for the table.
 *
 * @note Do not remove or delete objects while walking the list.
 */
EAPI Eina_Iterator                     *evas_object_table_iterator_new(const Evas_Object *o) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * Get an accessor to get random access to the list of children for the table.
 *
 * @note Do not remove or delete objects while walking the list.
 */
EAPI Eina_Accessor                     *evas_object_table_accessor_new(const Evas_Object *o) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * Get the list of children for the table.
 *
 * @note This is a duplicate of the list kept by the table internally.
 *       It's up to the user to destroy it when it no longer needs it.
 *       It's possible to remove objects from the table when walking this
 *       list, but these removals won't be reflected on it.
 */
EAPI Eina_List                         *evas_object_table_children_get(const Evas_Object *o) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * Get the child of the table at the given coordinates
 *
 * @note This does not take into account col/row spanning
 */
EAPI Evas_Object                       *evas_object_table_child_get(const Evas_Object *o, unsigned short col, unsigned short row) EINA_ARG_NONNULL(1);
/**
 * @}
 */

/**
 * @defgroup Evas_Object_Grid Grid Smart Object.
 *
 * Convenience smart object that packs children under a regular grid
 * layout, using their virtual grid location and size to determine
 * children's positions inside the grid object's area.
 *
 * @ingroup Evas_Smart_Object_Group
 * @since 1.1
 */

/**
 * @addtogroup Evas_Object_Grid
 * @{
 */

#define EVAS_OBJ_GRID_CLASS evas_object_grid_class_get()

const Eo_Class *evas_object_grid_class_get(void) EINA_CONST;

extern EAPI Eo_Op EVAS_OBJ_GRID_BASE_ID;

enum
{
   EVAS_OBJ_GRID_SUB_ID_ADD_TO,
   EVAS_OBJ_GRID_SUB_ID_SIZE_SET,
   EVAS_OBJ_GRID_SUB_ID_SIZE_GET,
   EVAS_OBJ_GRID_SUB_ID_PACK,
   EVAS_OBJ_GRID_SUB_ID_UNPACK,
   EVAS_OBJ_GRID_SUB_ID_CLEAR,
   EVAS_OBJ_GRID_SUB_ID_PACK_GET,
   EVAS_OBJ_GRID_SUB_ID_ITERATOR_NEW,
   EVAS_OBJ_GRID_SUB_ID_ACCESSOR_NEW,
   EVAS_OBJ_GRID_SUB_ID_CHILDREN_GET,
   EVAS_OBJ_GRID_SUB_ID_MIRRORED_GET,
   EVAS_OBJ_GRID_SUB_ID_MIRRORED_SET,
   EVAS_OBJ_GRID_SUB_ID_LAST
};

#define EVAS_OBJ_GRID_ID(sub_id) (EVAS_OBJ_GRID_BASE_ID + sub_id)


/**
 * @def evas_obj_grid_add
 * @since 1.8
 *
 * Create a new grid.
 *
 * @param[out] ret
 *
 * @see evas_object_grid_add
 */
#define evas_obj_grid_add(ret) EVAS_OBJ_GRID_ID(EVAS_OBJ_GRID_SUB_ID_ADD), EO_TYPECHECK(Evas_Object **, ret)

/**
 * @def evas_obj_grid_add_to
 * @since 1.8
 *
 * Create a grid that is child of a given element parent.
 *
 * @param[out] ret
 *
 * @see evas_object_grid_add_to
 */
#define evas_obj_grid_add_to(ret) EVAS_OBJ_GRID_ID(EVAS_OBJ_GRID_SUB_ID_ADD_TO), EO_TYPECHECK(Evas_Object **, ret)

/**
 * @def evas_obj_grid_size_set
 * @since 1.8
 *
 * Set the virtual resolution for the grid
 *
 * @param[in] w
 * @param[in] h
 *
 * @see evas_object_grid_size_set
 */
#define evas_obj_grid_size_set(w, h) EVAS_OBJ_GRID_ID(EVAS_OBJ_GRID_SUB_ID_SIZE_SET), EO_TYPECHECK(int, w), EO_TYPECHECK(int, h)

/**
 * @def evas_obj_grid_size_get
 * @since 1.8
 *
 * Get the current virtual resolution
 *
 * @param[out] w
 * @param[out] h
 *
 * @see evas_object_grid_size_get
 */
#define evas_obj_grid_size_get(w, h) EVAS_OBJ_GRID_ID(EVAS_OBJ_GRID_SUB_ID_SIZE_GET), EO_TYPECHECK(int *, w), EO_TYPECHECK(int *, h)

/**
 * @def evas_obj_grid_pack
 * @since 1.8
 *
 * Add a new child to a grid object.
 *
 * @param[in] child
 * @param[in] x
 * @param[in] y
 * @param[in] w
 * @param[in] h
 * @param[out] ret
 *
 * @see evas_object_grid_pack
 */
#define evas_obj_grid_pack(child, x, y, w, h, ret) EVAS_OBJ_GRID_ID(EVAS_OBJ_GRID_SUB_ID_PACK), EO_TYPECHECK(Evas_Object *, child), EO_TYPECHECK(int, x), EO_TYPECHECK(int, y), EO_TYPECHECK(int, w), EO_TYPECHECK(int, h), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def evas_obj_grid_unpack
 * @since 1.8
 *
 * Remove child from grid.
 *
 * @param[in] child
 * @param[out] ret
 *
 * @see evas_object_grid_unpack
 */
#define evas_obj_grid_unpack(child, ret) EVAS_OBJ_GRID_ID(EVAS_OBJ_GRID_SUB_ID_UNPACK), EO_TYPECHECK(Evas_Object *, child), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def evas_obj_grid_clear
 * @since 1.8
 *
 * Faster way to remove all child objects from a grid object.
 *
 * @param[in] clear
 *
 * @see evas_object_grid_clear
 */
#define evas_obj_grid_clear(clear) EVAS_OBJ_GRID_ID(EVAS_OBJ_GRID_SUB_ID_CLEAR), EO_TYPECHECK(Eina_Bool, clear)

/**
 * @def evas_obj_grid_pack_get
 * @since 1.8
 *
 * Get the pack options for a grid child
 *
 * @param[in] child
 * @param[out] x
 * @param[out] y
 * @param[out] w
 * @param[out] h
 * @param[out] ret
 *
 * @see evas_object_grid_pack_get
 */
#define evas_obj_grid_pack_get(child, x, y, w, h, ret) EVAS_OBJ_GRID_ID(EVAS_OBJ_GRID_SUB_ID_PACK_GET), EO_TYPECHECK(Evas_Object *, child), EO_TYPECHECK(int *, x), EO_TYPECHECK(int *, y), EO_TYPECHECK(int *, w), EO_TYPECHECK(int *, h), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def evas_obj_grid_iterator_new
 * @since 1.8
 *
 * Get an iterator to walk the list of children for the grid.
 *
 * @param[out] ret
 *
 * @see evas_object_grid_iterator_new
 */
#define evas_obj_grid_iterator_new(ret) EVAS_OBJ_GRID_ID(EVAS_OBJ_GRID_SUB_ID_ITERATOR_NEW), EO_TYPECHECK(Eina_Iterator **, ret)

/**
 * @def evas_obj_grid_accessor_new
 * @since 1.8
 *
 * Get an accessor to get random access to the list of children for the grid.
 *
 * @param[out] ret
 *
 * @see evas_object_grid_accessor_new
 */
#define evas_obj_grid_accessor_new(ret) EVAS_OBJ_GRID_ID(EVAS_OBJ_GRID_SUB_ID_ACCESSOR_NEW), EO_TYPECHECK(Eina_Accessor **, ret)

/**
 * @def evas_obj_grid_children_get
 * @since 1.8
 *
 * Get the list of children for the grid.
 *
 * @param[out] ret
 *
 * @see evas_object_grid_children_get
 */
#define evas_obj_grid_children_get(ret) EVAS_OBJ_GRID_ID(EVAS_OBJ_GRID_SUB_ID_CHILDREN_GET), EO_TYPECHECK(Eina_List **, ret)

/**
 * @def evas_obj_grid_mirrored_get
 * @since 1.8
 *
 * Gets the mirrored mode of the grid.
 *
 * @param[out] ret
 *
 * @see evas_object_grid_mirrored_get
 */
#define evas_obj_grid_mirrored_get(ret) EVAS_OBJ_GRID_ID(EVAS_OBJ_GRID_SUB_ID_MIRRORED_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def evas_obj_grid_mirrored_set
 * @since 1.8
 *
 * Sets the mirrored mode of the grid. In mirrored mode the grid items go
 *
 * @param[in] mirrored
 *
 * @see evas_object_grid_mirrored_set
 */
#define evas_obj_grid_mirrored_set(mirrored) EVAS_OBJ_GRID_ID(EVAS_OBJ_GRID_SUB_ID_MIRRORED_SET), EO_TYPECHECK(Eina_Bool, mirrored)

/**
 * Create a new grid.
 *
 * It's set to a virtual size of 1x1 by default and add children with
 * evas_object_grid_pack().
 * @since 1.1
 */
EAPI Evas_Object   *evas_object_grid_add(Evas *evas) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * Create a grid that is child of a given element @a parent.
 *
 * @see evas_object_grid_add()
 * @since 1.1
 */
EAPI Evas_Object   *evas_object_grid_add_to(Evas_Object *parent) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * Set the virtual resolution for the grid
 *
 * @param o The grid object to modify
 * @param w The virtual horizontal size (resolution) in integer units
 * @param h The virtual vertical size (resolution) in integer units
 * @since 1.1
 */
EAPI void           evas_object_grid_size_set(Evas_Object *o, int w, int h) EINA_ARG_NONNULL(1);

/**
 * Get the current virtual resolution
 *
 * @param o The grid object to query
 * @param w A pointer to an integer to store the virtual width
 * @param h A pointer to an integer to store the virtual height
 * @see evas_object_grid_size_set()
 * @since 1.1
 */
EAPI void           evas_object_grid_size_get(const Evas_Object *o, int *w, int *h) EINA_ARG_NONNULL(1);

/**
 * Sets the mirrored mode of the grid. In mirrored mode the grid items go
 * from right to left instead of left to right. That is, 0,0 is top right, not
 * to left.
 *
 * @param o The grid object.
 * @param mirrored the mirrored mode to set
 * @since 1.1
 */
EAPI void           evas_object_grid_mirrored_set(Evas_Object *o, Eina_Bool mirrored) EINA_ARG_NONNULL(1);

/**
 * Gets the mirrored mode of the grid.
 *
 * @param o The grid object.
 * @return @c EINA_TRUE if it's a mirrored grid, @c EINA_FALSE otherwise.
 * @see evas_object_grid_mirrored_set()
 * @since 1.1
 */
EAPI Eina_Bool      evas_object_grid_mirrored_get(const Evas_Object *o) EINA_ARG_NONNULL(1);

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
 * @since 1.1
 */
EAPI Eina_Bool      evas_object_grid_pack(Evas_Object *o, Evas_Object *child, int x, int y, int w, int h) EINA_ARG_NONNULL(1, 2);

/**
 * Remove child from grid.
 *
 * @note removing a child will immediately call a walk over children in order
 *       to recalculate numbers of columns and rows. If you plan to remove
 *       all children, use evas_object_grid_clear() instead.
 *
 * @return 1 on success, 0 on failure.
 * @since 1.1
 */
EAPI Eina_Bool      evas_object_grid_unpack(Evas_Object *o, Evas_Object *child) EINA_ARG_NONNULL(1, 2);

/**
 * Faster way to remove all child objects from a grid object.
 *
 * @param o The given grid object.
 * @param clear if true, it will delete just removed children.
 * @since 1.1
 */
EAPI void           evas_object_grid_clear(Evas_Object *o, Eina_Bool clear) EINA_ARG_NONNULL(1);

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
 * @since 1.1
 */
EAPI Eina_Bool      evas_object_grid_pack_get(const Evas_Object *o, Evas_Object *child, int *x, int *y, int *w, int *h);

/**
 * Get an iterator to walk the list of children for the grid.
 *
 * @note Do not remove or delete objects while walking the list.
 * @since 1.1
 */
EAPI Eina_Iterator *evas_object_grid_iterator_new(const Evas_Object *o) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * Get an accessor to get random access to the list of children for the grid.
 *
 * @note Do not remove or delete objects while walking the list.
 * @since 1.1
 */
EAPI Eina_Accessor *evas_object_grid_accessor_new(const Evas_Object *o) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * Get the list of children for the grid.
 *
 * @note This is a duplicate of the list kept by the grid internally.
 *       It's up to the user to destroy it when it no longer needs it.
 *       It's possible to remove objects from the grid when walking this
 *       list, but these removals won't be reflected on it.
 * @since 1.1
 */
EAPI Eina_List     *evas_object_grid_children_get(const Evas_Object *o) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * @}
 */

/**
 * @defgroup Evas_Cserve Shared Image Cache Server
 *
 * Evas has an (optional) module which provides client-server
 * infrastructure to <b>share bitmaps across multiple processes</b>,
 * saving data and processing power.
 *
 * Be warned that it @b doesn't work when <b>threaded image
 * preloading</b> is enabled for Evas, though.
 */
typedef struct _Evas_Cserve_Stats       Evas_Cserve_Stats;
typedef struct _Evas_Cserve_Image_Cache Evas_Cserve_Image_Cache;
typedef struct _Evas_Cserve_Image       Evas_Cserve_Image;
typedef struct _Evas_Cserve_Config      Evas_Cserve_Config;

/**
 * Statistics about the server that shares cached bitmaps.
 * @ingroup Evas_Cserve
 */
struct _Evas_Cserve_Stats
{
   int    saved_memory;      /**< current amount of saved memory, in bytes */
   int    wasted_memory;      /**< current amount of wasted memory, in bytes */
   int    saved_memory_peak;      /**< peak amount of saved memory, in bytes */
   int    wasted_memory_peak;      /**< peak amount of wasted memory, in bytes */
   double saved_time_image_header_load;      /**< time, in seconds, saved in header loads by sharing cached loads instead */
   double saved_time_image_data_load;      /**< time, in seconds, saved in data loads by sharing cached loads instead */
};

/**
 * A handle of a cache of images shared by a server.
 * @ingroup Evas_Cserve
 */
struct _Evas_Cserve_Image_Cache
{
   struct
   {
      int mem_total;
      int count;
   } active, cached;
   Eina_List *images;
};

/**
 * A handle to an image shared by a server.
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
 * @return @c EINA_TRUE if it wants, @c EINA_FALSE otherwise.
 * @ingroup Evas_Cserve
 */
EAPI Eina_Bool   evas_cserve_want_get(void) EINA_WARN_UNUSED_RESULT;

/**
 * Retrieves if the system is connected to the server used to share
 * bitmaps.
 *
 * @return @c EINA_TRUE if it's connected, @c EINA_FALSE otherwise.
 * @ingroup Evas_Cserve
 */
EAPI Eina_Bool   evas_cserve_connected_get(void) EINA_WARN_UNUSED_RESULT;

/**
 * Retrieves statistics from a running bitmap sharing server.
 * @param stats pointer to structure to fill with statistics about the
 *        bitmap cache server.
 *
 * @return @c EINA_TRUE if @p stats were filled with data,
 *         @c EINA_FALSE otherwise (when @p stats is untouched)
 * @ingroup Evas_Cserve
 */
EAPI Eina_Bool   evas_cserve_stats_get(Evas_Cserve_Stats *stats) EINA_WARN_UNUSED_RESULT;

/**
 * Completely discard/clean a given images cache, thus re-setting it.
 *
 * @param cache A handle to the given images cache.
 */
EAPI void        evas_cserve_image_cache_contents_clean(Evas_Cserve_Image_Cache *cache);

/**
 * Retrieves the current configuration of the Evas image caching
 * server.
 *
 * @param config where to store current image caching server's
 * configuration.
 *
 * @return @c EINA_TRUE if @p config was filled with data,
 *         @c EINA_FALSE otherwise (when @p config is untouched)
 *
 * The fields of @p config will be altered to reflect the current
 * configuration's values.
 *
 * @see evas_cserve_config_set()
 *
 * @ingroup Evas_Cserve
 */
EAPI Eina_Bool   evas_cserve_config_get(Evas_Cserve_Config *config) EINA_WARN_UNUSED_RESULT;

/**
 * Changes the configurations of the Evas image caching server.
 *
 * @param config A bitmap cache configuration handle with fields set
 * to desired configuration values.
 * @return @c EINA_TRUE if @p config was successfully applied,
 *         @c EINA_FALSE otherwise.
 *
 * @see evas_cserve_config_get()
 *
 * @ingroup Evas_Cserve
 */
EAPI Eina_Bool   evas_cserve_config_set(const Evas_Cserve_Config *config) EINA_WARN_UNUSED_RESULT;

/**
 * Force the system to disconnect from the bitmap caching server.
 *
 * @ingroup Evas_Cserve
 */
EAPI void        evas_cserve_disconnect(void);

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
 * @dontinclude evas-images.c
 * @skip img1 =
 * @until ecore_main_loop_begin(
 *
 * Here, being @c valid_path the path to a valid image and @c
 * bogus_path a path to a file which does not exist, the two outputs
 * of evas_load_error_str() would be (if no other errors occur):
 * <code>"No error on load"</code> and <code>"File (or file path) does
 * not exist"</code>, respectively. See the full @ref
 * Example_Evas_Images "example".
 *
 * @ingroup Evas_Utils
 */
EAPI const char *evas_load_error_str(Evas_Load_Error error);

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
EAPI void evas_color_hsv_to_rgb(float h, float s, float v, int *r, int *g, int *b);

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
EAPI void evas_color_rgb_to_hsv(int r, int g, int b, float *h, float *s, float *v);

/* argb color space has a,r,g,b in the range 0 to 255 */

/**
 * Pre-multiplies a rgb triplet by an alpha factor.
 *
 * @param a The alpha factor.
 * @param r The Red component of the color.
 * @param g The Green component of the color.
 * @param b The Blue component of the color.
 *
 * This function pre-multiplies a given rgb triplet by an alpha
 * factor. Alpha factor is used to define transparency.
 *
 * @ingroup Evas_Utils
 **/
EAPI void evas_color_argb_premul(int a, int *r, int *g, int *b);

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
EAPI void evas_color_argb_unpremul(int a, int *r, int *g, int *b);

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
EAPI void evas_data_argb_premul(unsigned int *data, unsigned int len);

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
EAPI void evas_data_argb_unpremul(unsigned int *data, unsigned int len);

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
EAPI int  evas_string_char_next_get(const char *str, int pos, int *decoded) EINA_ARG_NONNULL(1);

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
EAPI int  evas_string_char_prev_get(const char *str, int pos, int *decoded) EINA_ARG_NONNULL(1);

/**
 * Get the length in characters of the string.
 * @param  str The string to get the length of.
 * @return The length in characters (not bytes)
 * @ingroup Evas_Utils
 */
EAPI int  evas_string_char_len_get(const char *str) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

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
EAPI const Evas_Modifier *evas_key_modifier_get(const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

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
EAPI const Evas_Lock     *evas_key_lock_get(const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

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
EAPI Eina_Bool            evas_key_modifier_is_set(const Evas_Modifier *m, const char *keyname) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1, 2);

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
EAPI Eina_Bool            evas_key_lock_is_set(const Evas_Lock *l, const char *keyname) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1, 2);

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
EAPI void                 evas_key_modifier_add(Evas *e, const char *keyname) EINA_ARG_NONNULL(1, 2);

/**
 * Removes the @p keyname key from the current list of modifier keys
 * on canvas @p e.
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
EAPI void                 evas_key_modifier_del(Evas *e, const char *keyname) EINA_ARG_NONNULL(1, 2);

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
EAPI void                 evas_key_lock_add(Evas *e, const char *keyname) EINA_ARG_NONNULL(1, 2);

/**
 * Removes the @p keyname key from the current list of lock keys on
 * canvas @p e.
 *
 * @param e The pointer to the Evas canvas
 * @param keyname The name of the key to remove from the locks list.
 *
 * @see evas_key_lock_get
 * @see evas_key_lock_add
 * @see evas_key_lock_on
 * @see evas_key_lock_off
 */
EAPI void                 evas_key_lock_del(Evas *e, const char *keyname) EINA_ARG_NONNULL(1, 2);

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
EAPI void                 evas_key_modifier_on(Evas *e, const char *keyname) EINA_ARG_NONNULL(1, 2);

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
EAPI void                 evas_key_modifier_off(Evas *e, const char *keyname) EINA_ARG_NONNULL(1, 2);

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
EAPI void                 evas_key_lock_on(Evas *e, const char *keyname) EINA_ARG_NONNULL(1, 2);

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
EAPI void                 evas_key_lock_off(Evas *e, const char *keyname) EINA_ARG_NONNULL(1, 2);

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
EAPI Evas_Modifier_Mask   evas_key_modifier_mask_get(const Evas *e, const char *keyname) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1, 2);

/**
 * Requests @p keyname key events be directed to @p obj.
 *
 * @param obj the object to direct @p keyname events to.
 * @param keyname the key to request events for.
 * @param modifiers a mask of modifiers that must be present to
 * trigger the event.
 * @param not_modifiers a mask of modifiers that must @b not be present
 * to trigger the event.
 * @param exclusive request that the @p obj is the only object
 * receiving the @p keyname events.
 * @return @c EINA_TRUE, if the call succeeded, @c EINA_FALSE otherwise.
 *
 * Key grabs allow one or more objects to receive key events for
 * specific key strokes even if other objects have focus. Whenever a
 * key is grabbed, only the objects grabbing it will get the events
 * for the given keys.
 *
 * @p keyname is a platform dependent symbolic name for the key
 * pressed (see @ref Evas_Keys for more information).
 *
 * @p modifiers and @p not_modifiers are bit masks of all the
 * modifiers that must and mustn't, respectively, be pressed along
 * with @p keyname key in order to trigger this new key
 * grab. Modifiers can be things such as Shift and Ctrl as well as
 * user defined types via evas_key_modifier_add(). Retrieve them with
 * evas_key_modifier_mask_get() or use @c 0 for empty masks.
 *
 * @p exclusive will make the given object the only one permitted to
 * grab the given key. If given @c EINA_TRUE, subsequent calls on this
 * function with different @p obj arguments will fail, unless the key
 * is ungrabbed again.
 *
 * Example code follows.
 * @dontinclude evas-events.c
 * @skip if (d.focus)
 * @until else
 *
 * See the full example @ref Example_Evas_Events "here".
 *
 * @warning Providing impossible modifier sets creates undefined behavior
 *
 * @see evas_object_key_ungrab
 * @see evas_object_focus_set
 * @see evas_object_focus_get
 * @see evas_focus_get
 * @see evas_key_modifier_add
 */
EAPI Eina_Bool            evas_object_key_grab(Evas_Object *obj, const char *keyname, Evas_Modifier_Mask modifiers, Evas_Modifier_Mask not_modifiers, Eina_Bool exclusive) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1, 2);

/**
 * Removes the grab on @p keyname key events by @p obj.
 *
 * @param obj the object that has an existing key grab.
 * @param keyname the key the grab is set for.
 * @param modifiers a mask of modifiers that must be present to
 * trigger the event.
 * @param not_modifiers a mask of modifiers that must not not be
 * present to trigger the event.
 *
 * Removes a key grab on @p obj if @p keyname, @p modifiers, and @p
 * not_modifiers match.
 *
 * Example code follows.
 * @dontinclude evas-events.c
 * @skip got here by key grabs
 * @until }
 *
 * See the full example @ref Example_Evas_Events "here".
 *
 * @see evas_object_key_grab
 * @see evas_object_focus_set
 * @see evas_object_focus_get
 * @see evas_focus_get
 */
EAPI void                 evas_object_key_ungrab(Evas_Object *obj, const char *keyname, Evas_Modifier_Mask modifiers, Evas_Modifier_Mask not_modifiers) EINA_ARG_NONNULL(1, 2);

/**
 * @}
 */

/**
 * @defgroup Evas_Touch_Point_List Touch Point List Functions
 *
 * Functions to get information of touched points in the Evas.
 *
 * Evas maintains list of touched points on the canvas. Each point has
 * its co-ordinates, id and state. You can get the number of touched
 * points and information of each point using evas_touch_point_list
 * functions.
 *
 * @ingroup Evas_Canvas
 */

/**
 * @addtogroup Evas_Touch_Point_List
 * @{
 */

/**
 * Get the number of touched point in the evas.
 *
 * @param e The pointer to the Evas canvas.
 * @return The number of touched point on the evas.
 *
 * New touched point is added to the list whenever touching the evas
 * and point is removed whenever removing touched point from the evas.
 *
 * Example:
 * @code
 * extern Evas *evas;
 * int count;
 *
 * count = evas_touch_point_list_count(evas);
 * printf("The count of touch points: %i\n", count);
 * @endcode
 *
 * @see evas_touch_point_list_nth_xy_get()
 * @see evas_touch_point_list_nth_id_get()
 * @see evas_touch_point_list_nth_state_get()
 */
EAPI unsigned int           evas_touch_point_list_count(Evas *e) EINA_ARG_NONNULL(1);

/**
 * This function returns the nth touch point's co-ordinates.
 *
 * @param e The pointer to the Evas canvas.
 * @param n The number of the touched point (0 being the first).
 * @param x The pointer to a Evas_Coord to be filled in.
 * @param y The pointer to a Evas_Coord to be filled in.
 *
 * Touch point's co-ordinates is updated whenever moving that point
 * on the canvas.
 *
 * Example:
 * @code
 * extern Evas *evas;
 * Evas_Coord x, y;
 *
 * if (evas_touch_point_list_count(evas))
 *   {
 *      evas_touch_point_nth_xy_get(evas, 0, &x, &y);
 *      printf("The first touch point's co-ordinate: (%i, %i)\n", x, y);
 *   }
 * @endcode
 *
 * @see evas_touch_point_list_count()
 * @see evas_touch_point_list_nth_id_get()
 * @see evas_touch_point_list_nth_state_get()
 */
EAPI void                   evas_touch_point_list_nth_xy_get(Evas *e, unsigned int n, Evas_Coord *x, Evas_Coord *y) EINA_ARG_NONNULL(1);

/**
 * This function returns the @p id of nth touch point.
 *
 * @param e The pointer to the Evas canvas.
 * @param n The number of the touched point (0 being the first).
 * @return id of nth touch point, if the call succeeded, -1 otherwise.
 *
 * The point which comes from Mouse Event has @p id 0 and The point
 * which comes from Multi Event has @p id that is same as Multi
 * Event's device id.
 *
 * Example:
 * @code
 * extern Evas *evas;
 * int id;
 *
 * if (evas_touch_point_list_count(evas))
 *   {
 *      id = evas_touch_point_nth_id_get(evas, 0);
 *      printf("The first touch point's id: %i\n", id);
 *   }
 * @endcode
 *
 * @see evas_touch_point_list_count()
 * @see evas_touch_point_list_nth_xy_get()
 * @see evas_touch_point_list_nth_state_get()
 */
EAPI int                    evas_touch_point_list_nth_id_get(Evas *e, unsigned int n) EINA_ARG_NONNULL(1);

/**
 * This function returns the @p state of nth touch point.
 *
 * @param e The pointer to the Evas canvas.
 * @param n The number of the touched point (0 being the first).
 * @return @p state of nth touch point, if the call succeeded,
 *         EVAS_TOUCH_POINT_CANCEL otherwise.
 *
 * The point's @p state is EVAS_TOUCH_POINT_DOWN when pressed,
 * EVAS_TOUCH_POINT_STILL when the point is not moved after pressed,
 * EVAS_TOUCH_POINT_MOVE when moved at least once after pressed and
 * EVAS_TOUCH_POINT_UP when released.
 *
 * Example:
 * @code
 * extern Evas *evas;
 * Evas_Touch_Point_State state;
 *
 * if (evas_touch_point_list_count(evas))
 *   {
 *      state = evas_touch_point_nth_state_get(evas, 0);
 *      printf("The first touch point's state: %i\n", state);
 *   }
 * @endcode
 *
 * @see evas_touch_point_list_count()
 * @see evas_touch_point_list_nth_xy_get()
 * @see evas_touch_point_list_nth_id_get()
 */
EAPI Evas_Touch_Point_State evas_touch_point_list_nth_state_get(Evas *e, unsigned int n) EINA_ARG_NONNULL(1);

/**
 * @}
 */

#define EVAS_COMMON_CLASS evas_common_class_get()

const Eo_Class *evas_common_class_get(void) EINA_CONST;

extern EAPI Eo_Op EVAS_COMMON_BASE_ID;

enum
{
   EVAS_COMMON_SUB_ID_EVAS_GET,
   EVAS_COMMON_SUB_ID_LAST
};

#define EVAS_COMMON_ID(sub_id) (EVAS_COMMON_BASE_ID + sub_id)

#define evas_common_evas_get(ret) EVAS_COMMON_ID(EVAS_COMMON_SUB_ID_EVAS_GET), EO_TYPECHECK(Evas **, ret)

extern EAPI Eo_Op EVAS_OBJ_BASE_ID;

enum
{
   EVAS_OBJ_SUB_ID_POSITION_SET,
   EVAS_OBJ_SUB_ID_POSITION_GET,
   EVAS_OBJ_SUB_ID_SIZE_SET,
   EVAS_OBJ_SUB_ID_SIZE_GET,
   EVAS_OBJ_SUB_ID_SIZE_HINT_MIN_SET,
   EVAS_OBJ_SUB_ID_SIZE_HINT_MIN_GET,
   EVAS_OBJ_SUB_ID_SIZE_HINT_MAX_SET,
   EVAS_OBJ_SUB_ID_SIZE_HINT_MAX_GET,
   EVAS_OBJ_SUB_ID_SIZE_HINT_REQUEST_SET,
   EVAS_OBJ_SUB_ID_SIZE_HINT_REQUEST_GET,
   EVAS_OBJ_SUB_ID_SIZE_HINT_ASPECT_SET,
   EVAS_OBJ_SUB_ID_SIZE_HINT_ASPECT_GET,
   EVAS_OBJ_SUB_ID_SIZE_HINT_ALIGN_SET,
   EVAS_OBJ_SUB_ID_SIZE_HINT_ALIGN_GET,
   EVAS_OBJ_SUB_ID_SIZE_HINT_WEIGHT_SET,
   EVAS_OBJ_SUB_ID_SIZE_HINT_WEIGHT_GET,
   EVAS_OBJ_SUB_ID_SIZE_HINT_PADDING_SET,
   EVAS_OBJ_SUB_ID_SIZE_HINT_PADDING_GET,
   EVAS_OBJ_SUB_ID_VISIBILITY_SET,
   EVAS_OBJ_SUB_ID_VISIBILITY_GET,
   EVAS_OBJ_SUB_ID_COLOR_SET,
   EVAS_OBJ_SUB_ID_COLOR_GET,
   EVAS_OBJ_SUB_ID_ANTI_ALIAS_SET,
   EVAS_OBJ_SUB_ID_ANTI_ALIAS_GET,
   EVAS_OBJ_SUB_ID_SCALE_SET,
   EVAS_OBJ_SUB_ID_SCALE_GET,
   EVAS_OBJ_SUB_ID_RENDER_OP_SET,
   EVAS_OBJ_SUB_ID_RENDER_OP_GET,
   EVAS_OBJ_SUB_ID_TYPE_SET,
   EVAS_OBJ_SUB_ID_TYPE_GET,
   EVAS_OBJ_SUB_ID_PRECISE_IS_INSIDE_SET,
   EVAS_OBJ_SUB_ID_PRECISE_IS_INSIDE_GET,
   EVAS_OBJ_SUB_ID_STATIC_CLIP_SET,
   EVAS_OBJ_SUB_ID_STATIC_CLIP_GET,
   EVAS_OBJ_SUB_ID_IS_FRAME_OBJECT_SET,
   EVAS_OBJ_SUB_ID_IS_FRAME_OBJECT_GET,
   EVAS_OBJ_SUB_ID_FREEZE_EVENTS_SET,
   EVAS_OBJ_SUB_ID_FREEZE_EVENTS_GET,
   EVAS_OBJ_SUB_ID_PASS_EVENTS_SET,
   EVAS_OBJ_SUB_ID_PASS_EVENTS_GET,
   EVAS_OBJ_SUB_ID_REPEAT_EVENTS_SET,
   EVAS_OBJ_SUB_ID_REPEAT_EVENTS_GET,
   EVAS_OBJ_SUB_ID_PROPAGATE_EVENTS_SET,
   EVAS_OBJ_SUB_ID_PROPAGATE_EVENTS_GET,
   EVAS_OBJ_SUB_ID_POINTER_MODE_SET,
   EVAS_OBJ_SUB_ID_POINTER_MODE_GET,
   EVAS_OBJ_SUB_ID_KEY_GRAB,
   EVAS_OBJ_SUB_ID_KEY_UNGRAB,
   EVAS_OBJ_SUB_ID_FOCUS_SET,
   EVAS_OBJ_SUB_ID_FOCUS_GET,
   EVAS_OBJ_SUB_ID_NAME_SET,
   EVAS_OBJ_SUB_ID_NAME_GET,
   EVAS_OBJ_SUB_ID_NAME_CHILD_FIND,
   EVAS_OBJ_SUB_ID_LAYER_SET,
   EVAS_OBJ_SUB_ID_LAYER_GET,
   EVAS_OBJ_SUB_ID_CLIP_SET,
   EVAS_OBJ_SUB_ID_CLIP_GET,
   EVAS_OBJ_SUB_ID_CLIP_UNSET,
   EVAS_OBJ_SUB_ID_CLIPEES_GET,
   EVAS_OBJ_SUB_ID_MAP_ENABLE_SET,
   EVAS_OBJ_SUB_ID_MAP_ENABLE_GET,
   EVAS_OBJ_SUB_ID_MAP_SET,
   EVAS_OBJ_SUB_ID_MAP_GET,
   EVAS_OBJ_SUB_ID_SMART_PARENT_GET,
   EVAS_OBJ_SUB_ID_SMART_DATA_GET,
   EVAS_OBJ_SUB_ID_SMART_TYPE_CHECK,
   EVAS_OBJ_SUB_ID_SMART_TYPE_CHECK_PTR,
   EVAS_OBJ_SUB_ID_SMART_MOVE_CHILDREN_RELATIVE,
   EVAS_OBJ_SUB_ID_SMART_CLIPPED_CLIPPER_GET,
   EVAS_OBJ_SUB_ID_RAISE,
   EVAS_OBJ_SUB_ID_LOWER,
   EVAS_OBJ_SUB_ID_STACK_ABOVE,
   EVAS_OBJ_SUB_ID_STACK_BELOW,
   EVAS_OBJ_SUB_ID_ABOVE_GET,
   EVAS_OBJ_SUB_ID_BELOW_GET,
   EVAS_OBJ_SUB_ID_TYPE_CHECK,
   EVAS_OBJ_SUB_ID_LAST
};

#define EVAS_OBJ_ID(sub_id) (EVAS_OBJ_BASE_ID + sub_id)

/**
 * @def evas_obj_position_set
 * @since 1.8
 *
 * Move the given Evas object to the given location inside its
 * canvas' viewport.
 *
 * @param[in] x   in
 * @param[in] y   in
 *
 * @see evas_object_move
 */
#define evas_obj_position_set(x, y) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_POSITION_SET), EO_TYPECHECK(Evas_Coord, x), EO_TYPECHECK(Evas_Coord, y)

/**
 * @def evas_obj_position_get
 * @since 1.8
 *
 * Retrieves the position of the given Evas object.
 *
 * @param[out] x out
 * @param[out] y out
 *
 * @see evas_object_geometry_get
 */
#define evas_obj_position_get(x, y) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_POSITION_GET), EO_TYPECHECK(Evas_Coord *, x), EO_TYPECHECK(Evas_Coord *, y)

/**
 * @def evas_obj_size_set
 * @since 1.8
 * Changes the size of the given Evas object.
 *
 * @param [in] w in
 * @param [in] h in
 *
 * @see evas_object_resize
 */
#define evas_obj_size_set(w, h) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_SIZE_SET), EO_TYPECHECK(Evas_Coord, w), EO_TYPECHECK(Evas_Coord, h)

/**
 * @def evas_obj_size_get
 * @since 1.8
 *
 * Retrieves the (rectangular) size of the given Evas object.
 *
 * @param[out] w out
 * @param[out] h out
 *
 * @see evas_object_geometry_get
 */
#define evas_obj_size_get(w, h) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_SIZE_GET), EO_TYPECHECK(Evas_Coord *, w), EO_TYPECHECK(Evas_Coord *, h)

/**
 * @def evas_obj_size_hint_min_set
 * @since 1.8
 *
 * Sets the hints for an object's minimum size.
 *
 * @param[in] w in
 * @param[in] h in
 *
 * @see evas_object_size_hint_min_set
 */
#define evas_obj_size_hint_min_set(w, h) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_SIZE_HINT_MIN_SET), EO_TYPECHECK(Evas_Coord, w), EO_TYPECHECK(Evas_Coord, h)

/**
 * @def evas_obj_size_hint_min_get
 * @since 1.8
 *
 * Retrieves the hints for an object's minimum size.
 *
 * @param[out] w out
 * @param[out] h out
 *
 * @see evas_object_size_hint_min_get
 */
#define evas_obj_size_hint_min_get(w, h) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_SIZE_HINT_MIN_GET), EO_TYPECHECK(Evas_Coord *, w), EO_TYPECHECK(Evas_Coord *, h)

/**
 * @def evas_obj_size_hint_max_set
 * @since 1.8
 *
 * Sets the hints for an object's maximum size.
 *
 * @param[in] w in
 * @param[in] h in
 *
 * @see evas_object_size_hint_max_set
 */
#define evas_obj_size_hint_max_set(w, h) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_SIZE_HINT_MAX_SET), EO_TYPECHECK(Evas_Coord, w), EO_TYPECHECK(Evas_Coord, h)

/**
 * @def evas_obj_size_hint_max_get
 * @since 1.8
 *
 * Retrieves the hints for an object's maximum size.
 *
 * @param[out] w out
 * @param[out] h out
 *
 * @see evas_object_size_hint_max_get
 */
#define evas_obj_size_hint_max_get(w, h) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_SIZE_HINT_MAX_GET), EO_TYPECHECK(Evas_Coord *, w), EO_TYPECHECK(Evas_Coord *, h)

/**
 * @def evas_obj_size_hint_request_set
 * @since 1.8
 *
 * Sets the hints for an object's optimum size.
 *
 * @param[in] w in
 * @param[in] h in
 *
 * @see evas_object_size_hint_request_set
 */
#define evas_obj_size_hint_request_set(w, h) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_SIZE_HINT_REQUEST_SET), EO_TYPECHECK(Evas_Coord, w), EO_TYPECHECK(Evas_Coord, h)

/**
 * @def evas_obj_size_hint_request_get
 * @since 1.8
 *
 * Retrieves the hints for an object's optimum size.
 *
 * @param[out] w out
 * @param[out] h out
 *
 * @see evas_object_size_hint_request_get
 */
#define evas_obj_size_hint_request_get(w, h) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_SIZE_HINT_REQUEST_GET), EO_TYPECHECK(Evas_Coord *, w), EO_TYPECHECK(Evas_Coord *, h)

/**
 * @def evas_obj_size_hint_aspect_set
 * @since 1.8
 *
 * Sets the hints for an object's aspect ratio.
 *
 * @param[in] aspect in
 * @param[in] w in
 * @param[in] h in
 *
 * @see evas_object_size_hint_aspect_set
 */
#define evas_obj_size_hint_aspect_set(aspect, w, h) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_SIZE_HINT_ASPECT_SET), EO_TYPECHECK(Evas_Aspect_Control, aspect), EO_TYPECHECK(Evas_Coord, w), EO_TYPECHECK(Evas_Coord, h)

/**
 * @def evas_obj_size_hint_aspect_get
 * @since 1.8
 *
 * Retrieves the hints for an object's aspect ratio.
 *
 * @param[out] aspect out
 * @param[out] w out
 * @param[out] h out
 *
 * @see evas_object_size_hint_aspect_get
 */
#define evas_obj_size_hint_aspect_get(aspect, w, h) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_SIZE_HINT_ASPECT_GET), EO_TYPECHECK(Evas_Aspect_Control *, aspect), EO_TYPECHECK(Evas_Coord *, w), EO_TYPECHECK(Evas_Coord *, h)

/**
 * @def evas_obj_size_hint_align_set
 * @since 1.8
 *
 * Sets the hints for an object's alignment.
 *
 * @param[in] x in
 * @param[in] y in
 *
 * @see evas_object_size_hint_align_set
 */
#define evas_obj_size_hint_align_set(x, y) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_SIZE_HINT_ALIGN_SET), EO_TYPECHECK(double, x), EO_TYPECHECK(double, y)

/**
 * @def evas_obj_size_hint_align_get
 * @since 1.8
 *
 * Retrieves the hints for on object's alignment.
 *
 * @param[out] x out
 * @param[out] y out
 *
 * @see evas_object_size_hint_align_get
 */
#define evas_obj_size_hint_align_get(x, y) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_SIZE_HINT_ALIGN_GET), EO_TYPECHECK(double *, x), EO_TYPECHECK(double *, y)

/**
 * @def evas_obj_size_hint_weight_set
 * @since 1.8
 *
 * Sets the hints for an object's weight.
 *
 * @param[in] x in
 * @param[in] y in
 *
 * @see evas_object_size_hint_weight_set
 */
#define evas_obj_size_hint_weight_set(x, y) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_SIZE_HINT_WEIGHT_SET), EO_TYPECHECK(double, x), EO_TYPECHECK(double, y)

/**
 * @def evas_obj_size_hint_weight_get
 * @since 1.8
 *
 * Retrieves the hints for an object's weight.
 *
 * @param[out] x out
 * @param[out] y out
 *
 * @see evas_object_size_hint_weight_get
 */
#define evas_obj_size_hint_weight_get(x, y) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_SIZE_HINT_WEIGHT_GET), EO_TYPECHECK(double *, x), EO_TYPECHECK(double *, y)

/**
 * @def evas_obj_size_hint_padding_set
 * @since 1.8
 *
 * Sets the hints for an object's padding space.
 *
 * @param[in] l in
 * @param[in] r in
 * @param[in] t in
 * @param[in] b in
 *
 * @see evas_object_size_hint_padding_set
 */
#define evas_obj_size_hint_padding_set(l, r, t, b) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_SIZE_HINT_PADDING_SET), EO_TYPECHECK(Evas_Coord, l), EO_TYPECHECK(Evas_Coord, r), EO_TYPECHECK(Evas_Coord, t), EO_TYPECHECK(Evas_Coord, b)

/**
 * @def evas_obj_size_hint_padding_get
 * @since 1.8
 *
 * Retrieves the hints for an object's padding space.
 *
 * @param[out] l out
 * @param[out] r out
 * @param[out] t out
 * @param[out] b out
 *
 * @see evas_object_size_hint_padding_get
 */
#define evas_obj_size_hint_padding_get(l, r, t, b) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_SIZE_HINT_PADDING_GET), EO_TYPECHECK(Evas_Coord *, l), EO_TYPECHECK(Evas_Coord *, r), EO_TYPECHECK(Evas_Coord *, t), EO_TYPECHECK(Evas_Coord *, b)

/**
 * @def evas_obj_visibility_set
 * @since 1.8
 *
 * Makes the given Evas object visible or invisible.
 * @param[in] v @c EINA_TRUE if to make the object visible, @c EINA_FALSE
 * otherwise.
 *
 * @see evas_object_show
 * @see evas_object_hide
 */
#define evas_obj_visibility_set(v) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_VISIBILITY_SET), EO_TYPECHECK(Eina_Bool, v)

/**
 * @def evas_obj_visibility_get
 * @since 1.8
 *
 * Retrieves whether or not the given Evas object is visible.
 *
 * @param[out] v @c EINA_TRUE if the object is visible, @c EINA_FALSE
 * otherwise.
 *
 * @see evas_object_visible_get
 */
#define evas_obj_visibility_get(v) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_VISIBILITY_GET), EO_TYPECHECK(Eina_Bool *, v)

/**
 * @def evas_obj_color_set
 * @since 1.8
 *
 * Sets the general/main color of the given Evas object to the given
 * one.
 *
 * @param[in] r   in
 * @param[in] g   in
 * @param[in] b   in
 * @param[in] a   in
 *
 * @see evas_object_color_set
 */
#define evas_obj_color_set(r, g, b, a) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_COLOR_SET), EO_TYPECHECK(int, r), EO_TYPECHECK(int, g), EO_TYPECHECK(int, b), EO_TYPECHECK(int, a)

/**
 * @def evas_obj_color_get
 * @since 1.8
 * Retrieves the general/main color of the given Evas object.
 *
 * @param r out
 * @param g out
 * @param b out
 * @param a out
 *
 * @see evas_object_color_get
 */
#define evas_obj_color_get(r, g, b, a) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_COLOR_GET), EO_TYPECHECK(int *, r), EO_TYPECHECK(int *, g), EO_TYPECHECK(int *, b), EO_TYPECHECK(int *, a)

/**
 * @def evas_obj_anti_alias_set
 * @since 1.8
 *
 * Sets whether or not the given Evas object is to be drawn anti-aliased.
 *
 * @param[in] anti_alias in
 *
 * @see evas_object_anti_alias_set
 */
#define evas_obj_anti_alias_set(anti_alias) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_ANTI_ALIAS_SET), EO_TYPECHECK(Eina_Bool, anti_alias)

/**
 * @def evas_obj_anti_alias_get
 * @since 1.8
 *
 * Retrieves whether or not the given Evas object is to be drawn anti_aliased.
 *
 * @param[out] anti_alias out
 *
 * @see evas_object_anti_alias_get
 */
#define evas_obj_anti_alias_get(anti_alias) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_ANTI_ALIAS_GET), EO_TYPECHECK(Eina_Bool *, anti_alias)

/**
 * @def evas_obj_scale_set
 * @since 1.8
 *
 * Sets the scaling factor for an Evas object. Does not affect all
 * objects.
 *
 * @param[in] scale in
 *
 * @see evas_object_scale_set
 */
#define evas_obj_scale_set(scale) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_SCALE_SET), EO_TYPECHECK(double, scale)

/**
 * @def evas_obj_scale_get
 * @since 1.8
 *
 * Retrieves the scaling factor for the given Evas object.
 *
 * @param[out] scale out
 *
 * @see evas_object_scale_get
 */
#define evas_obj_scale_get(scale) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_SCALE_GET), EO_TYPECHECK(double *, scale)

/**
 * @def evas_obj_render_op_set
 * @since 1.8
 *
 * Sets the render_op to be used for rendering the Evas object.
 *
 * @param[in] render_op in
 *
 * @see evas_object_render_op_set
 */
#define evas_obj_render_op_set(render_op) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_RENDER_OP_SET), EO_TYPECHECK(Evas_Render_Op, render_op)

/**
 * @def evas_obj_render_op_get
 * @since 1.8
 *
 * Retrieves the current value of the operation used for rendering the Evas object.
 *
 * @param[out] render_op
 *
 * @see evas_object_render_op_get
 */
#define evas_obj_render_op_get(render_op) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_RENDER_OP_GET), EO_TYPECHECK(Evas_Render_Op *, render_op)

/**
 * @def evas_obj_evas_get
 * @since 1.8
 * Retrieves the Evas canvas that the given object lives on.
 *
 * @param[out] evas
 * @see evas_object_evas_get
 */
#define evas_obj_evas_get(evas) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_EVAS_GET), EO_TYPECHECK(Evas **, evas)

/**
 * @def evas_obj_type_get
 * @since 1.8
 * Retrieves the type of the given Evas object.
 *
 * @param[out] type out
 * @see evas_object_type_get
 */
#define evas_obj_type_get(type) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_TYPE_GET), EO_TYPECHECK(const char **, type)

/**
 * @def evas_obj_type_set
 * @since 1.8
 * Sets the type of the given Evas object.
 *
 * @param[in] type in
 */
#define evas_obj_type_set(type) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_TYPE_SET), EO_TYPECHECK(const char *, type)

/**
 * @def evas_obj_precise_is_inside_set
 * @since 1.8
 *
 * Set whether to use precise (usually expensive) point collision
 * detection for a given Evas object.
 *
 * @param[in] precise in
 *
 * @see evas_object_precise_is_inside_set
 */
#define evas_obj_precise_is_inside_set(precise) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_PRECISE_IS_INSIDE_SET), EO_TYPECHECK(Eina_Bool, precise)

/**
 * @def evas_obj_precise_is_inside_get
 * @since 1.8
 *
 * Determine whether an object is set to use precise point collision
 * detection.
 *
 * @param[out] precise out
 *
 * @see evas_object_precise_is_inside_get
 */
#define evas_obj_precise_is_inside_get(precise) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_PRECISE_IS_INSIDE_GET), EO_TYPECHECK(Eina_Bool *, precise)

/**
 * @def evas_obj_static_clip_set
 * @since 1.8
 *
 * Set a hint flag on the given Evas object that it's used as a "static
 * clipper".
 *
 * @param[in] is_static_clip in
 *
 * @see evas_object_static_clip_set
 */
#define evas_obj_static_clip_set(is_static_clip) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_STATIC_CLIP_SET), EO_TYPECHECK(Eina_Bool, is_static_clip)

/**
 * @def evas_obj_static_clip_get
 * @since 1.8
 *
 * Get the "static clipper" hint flag for a given Evas object.
 *
 * @param[out] is_static_clip out
 *
 * @see evas_object_static_clip_get
 */
#define evas_obj_static_clip_get(is_static_clip) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_STATIC_CLIP_GET), EO_TYPECHECK(Eina_Bool *, is_static_clip)

/**
 * @def evas_obj_is_frame_object_set
 * @since 1.8
 *
 * @param[in] is_frame in
 *
 * @see evas_object_is_frame_object_set
 */
#define evas_obj_is_frame_object_set(is_frame) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_IS_FRAME_OBJECT_SET), EO_TYPECHECK(Eina_Bool, is_frame)

/**
 * @def evas_obj_is_frame_object_get
 * @since 1.8
 *
 * @param[out] is_frame out
 *
 * @see evas_object_is_frame_object_get
 */
#define evas_obj_is_frame_object_get(is_frame) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_IS_FRAME_OBJECT_GET), EO_TYPECHECK(Eina_Bool *, is_frame)

/**
 * @def evas_obj_freeze_events_set
 * @since 1.8
 *
 * Set whether an Evas object is to freeze (discard) events.
 *
 * @param[in] freeze in
 *
 * @see evas_object_freeze_events_set
 */
#define evas_obj_freeze_events_set(freeze) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_FREEZE_EVENTS_SET), EO_TYPECHECK(Eina_Bool, freeze)

/**
 * @def evas_obj_freeze_events_get
 * @since 1.8
 *
 * Determine whether an object is set to freeze (discard) events.
 *
 * @param[out] freeze out
 *
 * @see evas_object_freeze_events_get
 */
#define evas_obj_freeze_events_get(freeze) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_FREEZE_EVENTS_GET), EO_TYPECHECK(Eina_Bool *, freeze)

/**
 * @def evas_obj_pass_events_set
 * @since 1.8
 *
 * Set whether an Evas object is to pass (ignore) events.
 *
 * @param[in] pass in
 *
 * @see evas_object_pass_events_set
 */
#define evas_obj_pass_events_set(pass) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_PASS_EVENTS_SET), EO_TYPECHECK(Eina_Bool, pass)

/**
 * @def evas_obj_pass_events_get
 * @since 1.8
 *
 * Determine whether an object is set to pass (ignore) events.
 *
 * @param[out] pass
 *
 * @see evas_object_pass_events_get
 */
#define evas_obj_pass_events_get(pass) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_PASS_EVENTS_GET), EO_TYPECHECK(Eina_Bool *, pass)

/**
 * @def evas_obj_repeat_events_set
 * @since 1.8
 *
 * Set whether an Evas object is to repeat events.
 *
 * @param[in] repeat in
 *
 * @see evas_object_repeat_events_set
 */
#define evas_obj_repeat_events_set(repeat) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_REPEAT_EVENTS_SET), EO_TYPECHECK(Eina_Bool, repeat)

/**
 * @def evas_obj_repeat_events_get
 * @since 1.8
 *
 * Determine whether an object is set to repeat events.
 *
 * @param[out] repeat out
 *
 * @see evas_object_repeat_events_get
 */
#define evas_obj_repeat_events_get(repeat) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_REPEAT_EVENTS_GET), EO_TYPECHECK(Eina_Bool *, repeat)

/**
 * @def evas_obj_propagate_events_set
 * @since 1.8
 *
 * Set whether events on a smart object's member should get propagated
 * up to its parent.
 *
 * @param[in] propagate in
 * @see evas_object_propagate_events_set
 */
#define evas_obj_propagate_events_set(propagate) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_PROPAGATE_EVENTS_SET), EO_TYPECHECK(Eina_Bool, propagate)

/**
 * @def evas_obj_propagate_events_get
 * @since 1.8
 *
 * Retrieve whether an Evas object is set to propagate events.
 *
 * @param[out] propagate out
 *
 * @see evas_object_propagate_events_get
 */
#define evas_obj_propagate_events_get(propagate) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_PROPAGATE_EVENTS_GET), EO_TYPECHECK(Eina_Bool *, propagate)

/**
 * @def evas_obj_pointer_mode_set
 * @since 1.8
 *
 * Set pointer behavior.
 *
 * @param[in] pointer_mode in
 *
 * @see evas_object_pointer_mode_set
 */
#define evas_obj_pointer_mode_set(pointer_mode) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_POINTER_MODE_SET), EO_TYPECHECK(Evas_Object_Pointer_Mode, pointer_mode)

/**
 * @def evas_obj_pointer_mode_get
 * @since 1.8
 *
 * Determine how pointer will behave.
 *
 * @param[out] pointer_mode out
 *
 * @see evas_object_pointer_mode_get
 */
#define evas_obj_pointer_mode_get(pointer_mode) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_POINTER_MODE_GET), EO_TYPECHECK(Evas_Object_Pointer_Mode *, pointer_mode)

/**
 * @def evas_obj_clip_set
 * @since 1.8
 * Clip one object to another.
 *
 * @param[in] clip in
 *
 * @see evas_object_clip_set
 */
#define evas_obj_clip_set(clip) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_CLIP_SET), EO_TYPECHECK(Evas_Object *, clip)

/**
 * @def evas_obj_clip_get
 * @since 1.8
 * Get the object clipping @p obj (if any).
 *
 * @param[out] clip out
 *
 * @see evas_object_clip_get
 */
#define evas_obj_clip_get(clip) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_CLIP_GET), EO_TYPECHECK(Evas_Object **, clip)

/**
 * @def evas_obj_clip_unset
 * @since 1.8
 * Disable/cease clipping on a clipped @p obj object.
 *
 * @see evas_object_clip_unset
 */
#define evas_obj_clip_unset() EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_CLIP_UNSET)

/**
 * @def evas_obj_clipees_get
 * @since 1.8
 * Return a list of objects currently clipped by @p obj.
 *
 * @param[out] clipees out
 *
 * @see evas_object_clipees_get
 */
#define evas_obj_clipees_get(clipees) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_CLIPEES_GET), EO_TYPECHECK(const Eina_List **, clipees)

/**
 * @def evas_obj_focus_set
 * @since 1.8
 * Sets or unsets a given object as the currently focused one on its
 * canvas.
 * @param[in] focus in
 *
 * @see evas_object_focus_set
 */
#define evas_obj_focus_set(focus) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_FOCUS_SET), EO_TYPECHECK(Eina_Bool, focus)

/**
 * @def evas_obj_focus_get
 * @since 1.8
 * Retrieve whether an object has the focus.
 *
 * @param[out] focus out
 *
 * @see evas_object_focus_get
 */
#define evas_obj_focus_get(focus) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_FOCUS_GET), EO_TYPECHECK(Eina_Bool *, focus)

/**
 * @def evas_obj_name_set
 * @since 1.8
 * Sets the name of the given Evas object to the given name.
 *
 * @param[in]   name in
 *
 * @see evas_object_name_set
 */
#define evas_obj_name_set(name) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_NAME_SET), EO_TYPECHECK(const char *, name)

/**
 * @def evas_obj_name_get
 * @since 1.8
 *
 * Retrieves the name of the given Evas object.
 *
 * @param[out] name out
 *
 * @see evas_object_name_get
 *
 */
#define evas_obj_name_get(name) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_NAME_GET), EO_TYPECHECK(const char **, name)

/**
 * @def evas_obj_name_child_find
 * @since 1.8
 *
 * Retrieves the object from children of the given object with the given name.
 * @param[in] name in
 * @param[in] recurse in
 * @param[out] child out
 *
 * @see evas_object_name_child_find
 */
#define evas_obj_name_child_find(name, recurse, child) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_NAME_CHILD_FIND), EO_TYPECHECK(const char *, name), EO_TYPECHECK(int, recurse), EO_TYPECHECK(Evas_Object **, child)

/**
 * @def evas_obj_key_grab
 * @since 1.8
 *
 * Requests @p keyname key events be directed to the obj.
 * @param[in] keyname in
 * @param[in] modifiers in
 * @param[in] not_modifiers in
 * @param[in] exclusive in
 * @param[out] ret out
 *
 * @see evas_object_key_grab
 */
#define evas_obj_key_grab(keyname, modifiers, not_modifiers, exclusive, ret) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_KEY_GRAB), EO_TYPECHECK(const char *, keyname), EO_TYPECHECK(Evas_Modifier_Mask, modifiers), EO_TYPECHECK(Evas_Modifier_Mask, not_modifiers), EO_TYPECHECK(Eina_Bool, exclusive), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def evas_obj_key_ungrab
 * @since 1.8
 *
 * Removes the grab on @p keyname key events by the obj.
 *
 * @param[in] keyname
 * @param[in] modifiers
 * @param[in] not_modifiers
 *
 * @see evas_object_key_ungrab
 */
#define evas_obj_key_ungrab(keyname, modifiers, not_modifiers) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_KEY_UNGRAB), EO_TYPECHECK(const char *, keyname), EO_TYPECHECK(Evas_Modifier_Mask, modifiers), EO_TYPECHECK(Evas_Modifier_Mask, not_modifiers)

/**
 * @def evas_obj_layer_set
 * @since 1.8
 * Sets the layer of the its canvas that the given object will be part
 * of.
 *
 * @param[in]   l   in
 *
 * @see evas_object_layer_set
 */
#define evas_obj_layer_set(l) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_LAYER_SET), EO_TYPECHECK(short, l)

/**
 * @def evas_obj_layer_get
 * @since 1.8
 * Retrieves the layer of its canvas that the given object is part of.
 *
 * @param[out] l out
 *
 * @see evas_object_layer_get
 */
#define evas_obj_layer_get(l) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_LAYER_GET), EO_TYPECHECK(short *, l)

/**
 * @def evas_obj_map_enable_set
 * @since 1.8
 *
 * Enable or disable the map that is set.
 *
 * @param[in] enabled in
 *
 * @see evas_object_map_enable_set
 */
#define evas_obj_map_enable_set(enabled) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_MAP_ENABLE_SET), EO_TYPECHECK(Eina_Bool, enabled)

/**
 * @def evas_obj_map_enable_get
 * @since 1.8
 *
 * Get the map enabled state
 *
 * @param[out] enabled out
 *
 * @see evas_object_map_enable_get
 */
#define evas_obj_map_enable_get(enabled) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_MAP_ENABLE_GET), EO_TYPECHECK(Eina_Bool *, enabled)

/**
 * @def evas_obj_map_source_set
 * @since 1.8
 *
 * Set the map source object
 *
 * @param[in] source in
 *
 * @see evas_object_map_source_set
 */
#define evas_obj_map_source_set(source) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_MAP_SOURCE_SET), EO_TYPECHECK(Evas_Object *, source)

/**
 * @def evas_obj_map_source_get
 * @since 1.8
 *
 * Get the map source object
 *
 * @param[out] source out
 *
 * @see evas_object_map_source_get
 */
#define evas_obj_map_source_get(source) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_MAP_SOURCE_GET), EO_TYPECHECK(Evas_Object **, source)

/**
 * @def evas_obj_map_set
 * @since 1.8
 *
 * Set current object transformation map.
 *
 * @param[in] map in
 *
 * @see evas_object_map_set
 */
#define evas_obj_map_set(map) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_MAP_SET), EO_TYPECHECK(const Evas_Map *, map)

/**
 * @def evas_obj_map_get
 * @since 1.8
 *
 * Get current object transformation map.
 *
 * @param[out] map out
 *
 * @see evas_object_map_get
 */
#define evas_obj_map_get(map) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_MAP_GET), EO_TYPECHECK(const Evas_Map **, map)

/**
 * @def evas_obj_smart_parent_get
 * @since 1.8
 *
 * Gets the parent smart object of a given Evas object, if it has one.
 *
 * @param[out] smart_parent out
 *
 * @see evas_object_smart_parent_get
 */
#define evas_obj_smart_parent_get(smart_parent) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_SMART_PARENT_GET), EO_TYPECHECK(Evas_Object **, smart_parent)
/**
 * @def evas_obj_smart_data_get
 * @since 1.8
 *
 * Retrieve user data stored on a given smart object.
 *
 * @param[out] data out
 *
 * @see evas_object_smart_data_get
 */
#define evas_obj_smart_data_get(data) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_SMART_DATA_GET), EO_TYPECHECK(void **, data)

/**
 * @def evas_obj_smart_type_check
 * @since 1.8
 *
 * Checks whether a given smart object or any of its smart object
 * parents is of a given smart class.
 *
 * @param[in] type in
 * @param[out] type_check out
 *
 * @see evas_object_smart_type_check
 */
#define evas_obj_smart_type_check(type, type_check) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_SMART_TYPE_CHECK), EO_TYPECHECK(const char *, type), EO_TYPECHECK(Eina_Bool *, type_check)

/**
 * @def evas_obj_smart_type_check_ptr
 * @since 1.8
 *
 * Checks whether a given smart object or any of its smart object
 * parents is of a given smart class, <b>using pointer comparison</b>.
 *
 * @param[in] type in
 * @param[out] type_check out
 *
 * @see evas_object_smart_type_check_ptr
 */
#define evas_obj_smart_type_check_ptr(type, type_check) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_SMART_TYPE_CHECK_PTR), EO_TYPECHECK(const char *, type), EO_TYPECHECK(Eina_Bool *, type_check)

/**
 * @def evas_obj_smart_move_children_relative
 * @since 1.8
 *
 * Moves all children objects of a given smart object relative to a
 * given offset.
 *
 * @param[in] dx in
 * @param[in] dy in
 *
 * @see evas_object_smart_move_children_relative
 */
#define evas_obj_smart_move_children_relative(dx, dy) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_SMART_MOVE_CHILDREN_RELATIVE), EO_TYPECHECK(Evas_Coord, dx), EO_TYPECHECK(Evas_Coord, dy)

/**
 * @def evas_obj_smart_clipped_clipper_get
 * @since 1.8
 *
 * Get the clipper object for the given clipped smart object.
 *
 * @param[out] ret out
 *
 * @see evas_object_smart_clipped_clipper_get
 */
#define evas_obj_smart_clipped_clipper_get(ret) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_SMART_CLIPPED_CLIPPER_GET), EO_TYPECHECK(Evas_Object **, ret)

/**
 * @def evas_obj_raise
 * @since 1.8
 *
 * Raise obj to the top of its layer.
 *
 * @see evas_object_raise
 */
#define evas_obj_raise() EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_RAISE)

/**
 * @def evas_obj_lower
 * @since 1.8
 *
 * Lower obj to the bottom of its layer.
 *
 * @see evas_object_lower
 */
#define evas_obj_lower() EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_LOWER)

/**
 * @def evas_obj_stack_above
 * @since 1.8
 *
 * Stack the object immediately above @p above
 *
 * @param[in] above in
 *
 * @see evas_object_stack_above
 */
#define evas_obj_stack_above(above) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_STACK_ABOVE), EO_TYPECHECK(Evas_Object *, above)

/**
 * @def evas_obj_stack_below
 * @since 1.8
 *
 * Stack the object immediately below @p below
 *
 * @param[in] below in
 *
 * @see evas_object_stack_below
 */
#define evas_obj_stack_below(below) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_STACK_BELOW), EO_TYPECHECK(Evas_Object *, below)

/**
 * @def evas_obj_above_get
 * @since 1.8
 *
 * Get the Evas object stacked right above the object
 *
 * @param[out] ret out
 *
 * @see evas_object_above_get
 */
#define evas_obj_above_get(ret) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_ABOVE_GET), EO_TYPECHECK(Evas_Object **, ret)

/**
 * @def evas_obj_below_get
 * @since 1.8
 *
 * Get the Evas object stacked right below the object
 *
 * @param[out] ret out
 *
 * @see evas_object_below_get
 */
#define evas_obj_below_get(ret) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_BELOW_GET), EO_TYPECHECK(Evas_Object **, ret)

/**
 * @def evas_obj_type_check
 * @since 1.8
 *
 * Checks whether a given object is of a given class.
 *
 * @param[in] type in
 * @param[out] type_check out
 *
 * This function has been implemented to support legacy smart inheritance
 * and needs to be removed when all the objects are Eo objects (inc. Edje and ELM)
 * @see evas_object_smart_type_check
 */
#define evas_obj_type_check(type, type_check) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_TYPE_CHECK), EO_TYPECHECK(const char *, type), EO_TYPECHECK(Eina_Bool *, type_check)

#define EVAS_OBJ_CLASS evas_object_class_get()

const Eo_Class *evas_object_class_get(void) EINA_CONST;

#define EVAS_OBJ_IMAGE_CLASS evas_object_image_class_get()
const Eo_Class *evas_object_image_class_get(void) EINA_CONST;

extern EAPI Eo_Op EVAS_OBJ_IMAGE_BASE_ID;

enum
{
   EVAS_OBJ_IMAGE_SUB_ID_MEMFILE_SET,
   EVAS_OBJ_IMAGE_SUB_ID_FILE_SET,
   EVAS_OBJ_IMAGE_SUB_ID_FILE_GET,
   EVAS_OBJ_IMAGE_SUB_ID_SOURCE_SET,
   EVAS_OBJ_IMAGE_SUB_ID_SOURCE_GET,
   EVAS_OBJ_IMAGE_SUB_ID_BORDER_SET,
   EVAS_OBJ_IMAGE_SUB_ID_BORDER_GET,
   EVAS_OBJ_IMAGE_SUB_ID_BORDER_CENTER_FILL_SET,
   EVAS_OBJ_IMAGE_SUB_ID_BORDER_CENTER_FILL_GET,
   EVAS_OBJ_IMAGE_SUB_ID_FILLED_SET,
   EVAS_OBJ_IMAGE_SUB_ID_FILLED_GET,
   EVAS_OBJ_IMAGE_SUB_ID_BORDER_SCALE_SET,
   EVAS_OBJ_IMAGE_SUB_ID_BORDER_SCALE_GET,
   EVAS_OBJ_IMAGE_SUB_ID_FILL_SET,
   EVAS_OBJ_IMAGE_SUB_ID_FILL_GET,
   EVAS_OBJ_IMAGE_SUB_ID_FILL_SPREAD_SET,
   EVAS_OBJ_IMAGE_SUB_ID_FILL_SPREAD_GET,
   EVAS_OBJ_IMAGE_SUB_ID_SIZE_SET,
   EVAS_OBJ_IMAGE_SUB_ID_SIZE_GET,
   EVAS_OBJ_IMAGE_SUB_ID_STRIDE_GET,
   EVAS_OBJ_IMAGE_SUB_ID_LOAD_ERROR_GET,
   EVAS_OBJ_IMAGE_SUB_ID_DATA_CONVERT,
   EVAS_OBJ_IMAGE_SUB_ID_DATA_SET,
   EVAS_OBJ_IMAGE_SUB_ID_DATA_GET,
   EVAS_OBJ_IMAGE_SUB_ID_PRELOAD,
   EVAS_OBJ_IMAGE_SUB_ID_DATA_COPY_SET,
   EVAS_OBJ_IMAGE_SUB_ID_DATA_UPDATE_ADD,
   EVAS_OBJ_IMAGE_SUB_ID_ALPHA_SET,
   EVAS_OBJ_IMAGE_SUB_ID_ALPHA_GET,
   EVAS_OBJ_IMAGE_SUB_ID_SMOOTH_SCALE_SET,
   EVAS_OBJ_IMAGE_SUB_ID_SMOOTH_SCALE_GET,
   EVAS_OBJ_IMAGE_SUB_ID_RELOAD,
   EVAS_OBJ_IMAGE_SUB_ID_SAVE,
   EVAS_OBJ_IMAGE_SUB_ID_PIXELS_IMPORT,
   EVAS_OBJ_IMAGE_SUB_ID_PIXELS_GET_CALLBACK_SET,
   EVAS_OBJ_IMAGE_SUB_ID_PIXELS_DIRTY_SET,
   EVAS_OBJ_IMAGE_SUB_ID_PIXELS_DIRTY_GET,
   EVAS_OBJ_IMAGE_SUB_ID_LOAD_DPI_SET,
   EVAS_OBJ_IMAGE_SUB_ID_LOAD_DPI_GET,
   EVAS_OBJ_IMAGE_SUB_ID_LOAD_SIZE_SET,
   EVAS_OBJ_IMAGE_SUB_ID_LOAD_SIZE_GET,
   EVAS_OBJ_IMAGE_SUB_ID_LOAD_SCALE_DOWN_SET,
   EVAS_OBJ_IMAGE_SUB_ID_LOAD_SCALE_DOWN_GET,
   EVAS_OBJ_IMAGE_SUB_ID_LOAD_REGION_SET,
   EVAS_OBJ_IMAGE_SUB_ID_LOAD_REGION_GET,
   EVAS_OBJ_IMAGE_SUB_ID_LOAD_ORIENTATION_SET,
   EVAS_OBJ_IMAGE_SUB_ID_LOAD_ORIENTATION_GET,
   EVAS_OBJ_IMAGE_SUB_ID_COLORSPACE_SET,
   EVAS_OBJ_IMAGE_SUB_ID_COLORSPACE_GET,
   EVAS_OBJ_IMAGE_SUB_ID_VIDEO_SURFACE_SET,
   EVAS_OBJ_IMAGE_SUB_ID_VIDEO_SURFACE_GET,
   EVAS_OBJ_IMAGE_SUB_ID_NATIVE_SURFACE_SET,
   EVAS_OBJ_IMAGE_SUB_ID_NATIVE_SURFACE_GET,
   EVAS_OBJ_IMAGE_SUB_ID_SCALE_HINT_SET,
   EVAS_OBJ_IMAGE_SUB_ID_SCALE_HINT_GET,
   EVAS_OBJ_IMAGE_SUB_ID_CONTENT_HINT_SET,
   EVAS_OBJ_IMAGE_SUB_ID_CONTENT_HINT_GET,
   EVAS_OBJ_IMAGE_SUB_ID_REGION_SUPPORT_GET,
   EVAS_OBJ_IMAGE_SUB_ID_ANIMATED_GET,
   EVAS_OBJ_IMAGE_SUB_ID_ANIMATED_FRAME_COUNT_GET,
   EVAS_OBJ_IMAGE_SUB_ID_ANIMATED_LOOP_TYPE_GET,
   EVAS_OBJ_IMAGE_SUB_ID_ANIMATED_LOOP_COUNT_GET,
   EVAS_OBJ_IMAGE_SUB_ID_ANIMATED_FRAME_DURATION_GET,
   EVAS_OBJ_IMAGE_SUB_ID_ANIMATED_FRAME_SET,
   EVAS_OBJ_IMAGE_SUB_ID_SOURCE_VISIBLE_SET,
   EVAS_OBJ_IMAGE_SUB_ID_SOURCE_VISIBLE_GET,
   EVAS_OBJ_IMAGE_SUB_ID_SOURCE_EVENTS_SET,
   EVAS_OBJ_IMAGE_SUB_ID_SOURCE_EVENTS_GET,
   EVAS_OBJ_IMAGE_SUB_ID_LAST
};

#define EVAS_OBJ_IMAGE_ID(sub_id) (EVAS_OBJ_IMAGE_BASE_ID + sub_id)

/**
 * @def evas_obj_image_memfile_set
 * @since 1.8
 *
 * Sets the data for an image from memory to be loaded
 *
 * @param[in] data in
 * @param[in] size in
 * @param[in] format in
 * @param[in] key in
 *
 * @see evas_object_image_memfile_set
 */
#define evas_obj_image_memfile_set(data, size, format, key) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_MEMFILE_SET), EO_TYPECHECK(void *, data), EO_TYPECHECK(int, size), EO_TYPECHECK(char *, format), EO_TYPECHECK(char *, key)

/**
 * @def evas_obj_image_file_set
 * @since 1.8
 *
 * Set the source file from where an image object must fetch the real
 * image data (it may be an Eet file, besides pure image ones).
 *
 * @param[in] file in
 * @param[in] key in
 *
 * @see evas_object_image_file_set
 */
#define evas_obj_image_file_set(file, key) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_FILE_SET), EO_TYPECHECK(const char *, file), EO_TYPECHECK(const char*, key)

/**
 * @def evas_obj_image_file_get
 * @since 1.8
 *
 * This is the same as evas_object_image_file_set() but the file to be loaded
 * may exist at an address in memory (the data for the file, not the filename
 * itself). The @p data at the address is copied and stored for future use, so
 * no @p data needs to be kept after this call is made. It will be managed and
 * freed for you when no longer needed. The @p size is limited to 2 gigabytes
 * in size, and must be greater than 0. A @c NULL @p data pointer is also
 * invalid. Set the filename to @c NULL to reset to empty state and have the
 * image file data freed from memory using evas_object_image_file_set().
 *
 * @param[in] file out
 * @param[in] key out
 *
 * @see evas_object_image_file_get
 */
#define evas_obj_image_file_get(file, key) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_FILE_GET), EO_TYPECHECK(const char **, file), EO_TYPECHECK(const char **, key)

/**
 * @def evas_obj_image_source_set
 * @since 1.8
 *
 * Set the source object on an image object to used as a @b proxy.
 *
 * @param[in] src in
 * @param[out] result out
 *
 * @see evas_object_image_source_set
 */
#define evas_obj_image_source_set(src, result) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_SOURCE_SET), EO_TYPECHECK(Evas_Object *, src), EO_TYPECHECK(Eina_Bool *, result)

/**
 * @def evas_obj_image_source_get
 * @since 1.8
 *
 * Get the current source object of an image object.
 *
 * @param[out] src out
 *
 * @see evas_object_image_source_get
 */
#define evas_obj_image_source_get(src) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_SOURCE_GET), EO_TYPECHECK(Evas_Object **, src)

/**
 * @def evas_obj_image_source_visible_set
 * @since 1.8
 *
 * Set the source object to be shown or hidden.
 *
 * @param[in] visible in
 *
 * @see evas_object_image_source_visible_get
 */
#define evas_obj_image_source_visible_set(visible) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_SOURCE_VISIBLE_SET), EO_TYPECHECK(Eina_Bool, visible)

/**
 * @def evas_obj_image_source_visible_get
 * @since 1.8
 *
 * Get the state of the source object visibility.
 *
 * @param[out] visible out
 *
 * @see evas_obj_image_source_visible_set
 */
#define evas_obj_image_source_visible_get(visible) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_SOURCE_VISIBLE_GET), EO_TYPECHECK(Eina_Bool *, visible)

/**
 * @def evas_obj_image_source_events_set
 *
 * Set events to be repeated to the source object.
 *
 * @param[in] source in
 *
 * @see evas_object_image_source_events_get
 */
#define evas_obj_image_source_events_set(source) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_SOURCE_EVENTS_SET), EO_TYPECHECK(Eina_Bool, source)

/**
 * @def evas_obj_image_source_events_get
 *
 * Get the state of the source event.
 *
 * @param[out] source out
 *
 * @see evas_obj_image_source_event_set
 */
#define evas_obj_image_source_events_get(source) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_SOURCE_EVENTS_GET), EO_TYPECHECK(Eina_Bool *, source)

/**
 * @def evas_obj_image_border_set
 * @since 1.8
 *
 * Set the dimensions for an image object's border, a region which @b
 * won't ever be scaled together with its center.
 *
 * @param[in] l in
 * @param[in] r in
 * @param[in] t in
 * @param[in] b in
 *
 * @see evas_object_image_border_set
 */
#define evas_obj_image_border_set(l, r, t, b) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_BORDER_SET), EO_TYPECHECK(int, l), EO_TYPECHECK(int, r), EO_TYPECHECK(int, t), EO_TYPECHECK(int, b)

/**
 * @def evas_obj_image_border_get
 * @since 1.8
 *
 * Retrieve the dimensions for an image object's border, a region
 * which @b won't ever be scaled together with its center.
 *
 * @param[out] l in
 * @param[out] r in
 * @param[out] t in
 * @param[out] b in
 *
 * @see evas_object_image_border_get
 */
#define evas_obj_image_border_get(l, r, t, b) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_BORDER_GET), EO_TYPECHECK(int *, l), EO_TYPECHECK(int *, r), EO_TYPECHECK(int *, t), EO_TYPECHECK(int *, b)

/**
 * @def evas_obj_image_border_center_fill_set
 * @since 1.8
 *
 * Sets @b how the center part of the given image object (not the
 * borders) should be drawn when Evas is rendering it.
 *
 * @param[in] fill in
 *
 * @see evas_object_image_border_center_fill_set
 */
#define evas_obj_image_border_center_fill_set(fill) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_BORDER_CENTER_FILL_SET), EO_TYPECHECK(Evas_Border_Fill_Mode, fill)

/**
 * @def evas_obj_image_border_center_fill_get
 * @since 1.8
 *
 * Retrieves @b how the center part of the given image object (not the
 * borders) is to be drawn when Evas is rendering it.
 *
 * @param[out] fill out
 *
 * @see evas_object_image_border_center_fill_get
 */
#define evas_obj_image_border_center_fill_get(fill) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_BORDER_CENTER_FILL_GET), EO_TYPECHECK(Evas_Border_Fill_Mode *, fill)

/**
 * @def evas_obj_image_filled_set
 * @since 1.8
 *
 * Set whether the image object's fill property should track the
 * object's size.
 *
 * @param[in] filled in
 *
 * @see evas_object_image_filled_set
 */
#define evas_obj_image_filled_set(filled) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_FILLED_SET), EO_TYPECHECK(Eina_Bool, filled)

/**
 * @def evas_obj_image_filled_get
 * @since 1.8
 *
 * Retrieve whether the image object's fill property should track the
 * object's size.
 *
 * @param[out] filled out
 *
 * @see evas_object_image_filled_get
 */
#define evas_obj_image_filled_get(filled) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_FILLED_GET), EO_TYPECHECK(Eina_Bool *, filled)

/**
 * @def evas_obj_image_border_scale_set
 * @since 1.8
 *
 * Sets the scaling factor (multiplier) for the borders of an image
 * object.
 *
 * @param[in] scale in
 *
 * @see evas_object_image_border_scale_set
 */
#define evas_obj_image_border_scale_set(scale) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_BORDER_SCALE_SET), EO_TYPECHECK(double, scale)

/**
 * @def evas_obj_image_border_scale_get
 * @since 1.8
 *
 * Retrieves the scaling factor (multiplier) for the borders of an
 * image object.
 *
 * @param[out] scale out
 *
 * @see evas_object_image_border_scale_get
 */
#define evas_obj_image_border_scale_get(scale) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_BORDER_SCALE_GET), EO_TYPECHECK(double *, scale)

/**
 * @def evas_obj_image_fill_set
 * @since 1.8
 *
 * Set how to fill an image object's drawing rectangle given the
 * (real) image bound to it.
 *
 * @param[in] x in
 * @param[in] y in
 * @param[in] w in
 * @param[in] h in
 *
 * @see evas_object_image_fill_set
 */
#define evas_obj_image_fill_set(x, y, w, h) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_FILL_SET), EO_TYPECHECK(Evas_Coord, x), EO_TYPECHECK(Evas_Coord, y), EO_TYPECHECK(Evas_Coord, w), EO_TYPECHECK(Evas_Coord, h)

/**
 * @def evas_obj_image_fill_get
 * @since 1.8
 *
 * Retrieve how an image object is to fill its drawing rectangle,
 * given the (real) image bound to it.
 *
 * @param[out] x out
 * @param[out] y out
 * @param[out] w out
 * @param[out] h out
 *
 * @see evas_object_image_fill_get
 */
#define evas_obj_image_fill_get(x, y, w, h) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_FILL_GET), EO_TYPECHECK(Evas_Coord *, x), EO_TYPECHECK(Evas_Coord *, y), EO_TYPECHECK(Evas_Coord *, w), EO_TYPECHECK(Evas_Coord *, h)

/**
 * @def evas_obj_image_fill_spread_set
 * @since 1.8
 *
 * Sets the tiling mode for the given evas image object's fill.
 *
 * @param[in] spread in
 *
 * @see evas_object_image_fill_spread_set
 */
#define evas_obj_image_fill_spread_set(spread) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_FILL_SPREAD_SET), EO_TYPECHECK(Evas_Fill_Spread, spread)

/**
 * @def evas_obj_image_fill_spread_get
 * @since 1.8
 *
 * Retrieves the spread (tiling mode) for the given image object's
 * fill.
 *
 * @param[out] spread out
 *
 * @see evas_object_image_fill_spread_get
 */
#define evas_obj_image_fill_spread_get(spread) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_FILL_SPREAD_GET), EO_TYPECHECK(Evas_Fill_Spread *, spread)

/**
 * @def evas_obj_image_size_set
 * @since 1.8
 *
 * Sets the size of the given image object.
 *
 * @param[in] w in
 * @param[in] h in
 *
 * @see evas_object_image_size_set
 */
#define evas_obj_image_size_set(w, h) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_SIZE_SET), EO_TYPECHECK(int, w), EO_TYPECHECK(int, h)

/**
 * @def evas_obj_image_size_get
 * @since 1.8
 *
 * Retrieves the size of the given image object.
 *
 * @param[out] w out
 * @param[out] h out
 *
 * @see evas_object_image_size_get
 */
#define evas_obj_image_size_get(w, h) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_SIZE_GET), EO_TYPECHECK(int *, w), EO_TYPECHECK(int *, h)

/**
 * @def evas_obj_image_stride_get
 * @since 1.8
 *
 * Retrieves the row stride of the given image object.
 *
 * @param[out] stride out
 *
 * @see evas_object_image_stride_get
 */
#define evas_obj_image_stride_get(stride) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_STRIDE_GET), EO_TYPECHECK(int *, stride)

/**
 * @def evas_obj_image_load_error_get
 * @since 1.8
 *
 * Retrieves a number representing any error that occurred during the
 * last loading of the given image object's source image.
 *
 * @param[out] load_error
 *
 * @see evas_object_image_load_error_get
 */
#define evas_obj_image_load_error_get(load_error) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_LOAD_ERROR_GET), EO_TYPECHECK(Evas_Load_Error *, load_error)

/**
 * @def evas_obj_image_data_convert
 * @since 1.8
 *
 * Converts the raw image data of the given image object to the
 * specified colorspace.
 *
 * @param[in] to_cspace in
 * @param[out] data out
 *
 * @see evas_object_image_data_convert
 */
#define evas_obj_image_data_convert(to_cspace, data) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_DATA_CONVERT), EO_TYPECHECK(Evas_Colorspace, to_cspace), EO_TYPECHECK(void **, data)

/**
 * @def evas_obj_image_data_set
 * @since 1.8
 *
 * Sets the raw image data of the given image object.
 *
 * @param[in] data in
 *
 * @see evas_object_image_data_set
 */
#define evas_obj_image_data_set(data) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_DATA_SET), EO_TYPECHECK(void *, data)

/**
 * @def evas_obj_image_data_get
 * @since 1.8
 *
 * Get a pointer to the raw image data of the given image object.
 *
 * @param[in] for_writing in
 * @param[out] data out
 *
 * @see evas_object_image_data_get
 */
#define evas_obj_image_data_get(for_writing, data) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_DATA_GET), EO_TYPECHECK(Eina_Bool, for_writing), EO_TYPECHECK(void **, data)

/**
 * @def evas_obj_image_data_copy_set
 * @since 1.8
 *
 * Replaces the raw image data of the given image object.
 *
 * @param[in] data in
 *
 * @see evas_object_image_data_copy_set
 */
#define evas_obj_image_data_copy_set(data) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_DATA_COPY_SET), EO_TYPECHECK(void *, data)

/**
 * @def evas_obj_image_data_update_add
 * @since 1.8
 *
 * Mark a sub-region of the given image object to be redrawn.
 *
 * @param[in] x in
 * @param[in] y in
 * @param[in] w in
 * @param[in] h in
 *
 * @see evas_object_image_data_update_add
 */
#define evas_obj_image_data_update_add(x, y, w, h) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_DATA_UPDATE_ADD), EO_TYPECHECK(int, x), EO_TYPECHECK(int, y), EO_TYPECHECK(int, w), EO_TYPECHECK(int, h)

/**
 * @def evas_obj_image_alpha_set
 * @since 1.8
 *
 * Enable or disable alpha channel usage on the given image object.
 *
 * @param[in] alpha in
 *
 * @see evas_object_image_alpha_set
 */
#define evas_obj_image_alpha_set(alpha) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_ALPHA_SET), EO_TYPECHECK(Eina_Bool, alpha)

/**
 * @def evas_obj_image_alpha_get
 * @since 1.8
 *
 * Retrieve whether alpha channel data is being used on the given
 * image object.
 *
 * @param[out] alpha out
 *
 * @see evas_object_image_alpha_get
 */
#define evas_obj_image_alpha_get(alpha) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_ALPHA_GET), EO_TYPECHECK(Eina_Bool *, alpha)

/**
 * @def evas_obj_image_smooth_scale_set
 * @since 1.8
 *
 * Sets whether to use high-quality image scaling algorithm on the
 * given image object.
 *
 * @param[in] smooth_scale in
 *
 * @see evas_object_image_smooth_scale_set
 */
#define evas_obj_image_smooth_scale_set(smooth_scale) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_SMOOTH_SCALE_SET), EO_TYPECHECK(Eina_Bool, smooth_scale)

/**
 * @def evas_obj_image_smooth_scale_get
 * @since 1.8
 *
 * Retrieves whether the given image object is using high-quality
 * image scaling algorithm.
 *
 * @param[out] smooth_scale out
 *
 * @see evas_object_image_smooth_scale_get
 */
#define evas_obj_image_smooth_scale_get(smooth_scale) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_SMOOTH_SCALE_GET), EO_TYPECHECK(Eina_Bool *, smooth_scale)

/**
 * @def evas_obj_image_preload
 * @since 1.8
 *
 * Preload an image object's image data in the background
 *
 * @param[in] cancel in
 *
 * @see evas_object_image_preload
 */
#define evas_obj_image_preload(cancel) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_PRELOAD), EO_TYPECHECK(Eina_Bool, cancel)

/**
 * @def evas_obj_image_reload
 * @since 1.8
 *
 * Reload an image object's image data.
 *
 * @see evas_object_image_reload
 */
#define evas_obj_image_reload() EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_RELOAD)

/**
 * @def evas_obj_image_save
 * @since 1.8
 *
 * Save the given image object's contents to an (image) file.
 *
 * @param[in] file in
 * @param[in] key in
 * @param[in] flags in
 * @param[out] result out
 *
 * @see evas_object_image_save
 */
#define evas_obj_image_save(file, key, flags, result) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_SAVE), EO_TYPECHECK(const char *, file), EO_TYPECHECK(const char *, key), EO_TYPECHECK(const char *, flags), EO_TYPECHECK(Eina_Bool *, result)

/**
 * @def evas_obj_image_pixels_import
 * @since 1.8
 *
 * Import pixels from given source to a given canvas image object.
 *
 * @param[in] pixels in
 * @param[out] result out
 *
 * @see evas_object_image_pixels_import
 */
#define evas_obj_image_pixels_import(pixels, result) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_PIXELS_IMPORT), EO_TYPECHECK(Evas_Pixel_Import_Source *, pixels), EO_TYPECHECK(Eina_Bool *, result)

/**
 * @def evas_obj_image_pixels_get_callback_set
 * @since 1.8
 *
 * Set the callback function to get pixels from a canvas' image.
 *
 * @param[in] func in
 * @param[in] data in
 *
 * @see evas_object_image_pixels_get_callback_set
 */
#define evas_obj_image_pixels_get_callback_set(func, data) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_PIXELS_GET_CALLBACK_SET), EO_TYPECHECK(Evas_Object_Image_Pixels_Get_Cb, func), EO_TYPECHECK(void *, data)

/**
 * @def evas_obj_image_pixels_dirty_set
 * @since 1.8
 *
 * Mark whether the given image object is dirty (needs to be redrawn).
 *
 * @param[in] dirty in
 *
 * @see evas_object_image_pixels_get_callback_set
 */
#define evas_obj_image_pixels_dirty_set(dirty) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_PIXELS_DIRTY_SET), EO_TYPECHECK(Eina_Bool, dirty)

/**
 * @def evas_obj_image_pixels_dirty_get
 * @since 1.8
 *
 * Retrieves whether the given image object is dirty (needs to be redrawn).
 *
 * @param[out] dirty out
 *
 * @see evas_object_image_pixels_dirty_get
 */
#define evas_obj_image_pixels_dirty_get(dirty) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_PIXELS_DIRTY_GET), EO_TYPECHECK(Eina_Bool *, dirty)

/**
 * @def evas_obj_image_load_dpi_set
 * @since 1.8
 *
 * Set the DPI resolution of an image object's source image.
 *
 * @param[in] dpi in
 *
 * @see evas_object_image_load_dpi_set
 */
#define evas_obj_image_load_dpi_set(dpi) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_LOAD_DPI_SET), EO_TYPECHECK(double, dpi)

/**
 * @def evas_obj_image_load_dpi_get
 * @since 1.8
 *
 * Get the DPI resolution of a loaded image object in the canvas.
 *
 * @param[out] dpi out
 *
 * @see evas_object_image_load_dpi_get
 */
#define evas_obj_image_load_dpi_get(dpi) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_LOAD_DPI_GET), EO_TYPECHECK(double *, dpi)

/**
 * @def evas_obj_image_load_size_set
 * @since 1.8
 *
 * Set the size of a given image object's source image, when loading
 * it.
 *
 * @param[in] w in
 * @param[in] h in
 *
 * @see evas_object_image_load_size_set
 */
#define evas_obj_image_load_size_set(w, h) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_LOAD_SIZE_SET), EO_TYPECHECK(int, w), EO_TYPECHECK(int, h)

/**
 * @def evas_obj_image_load_size_get
 * @since 1.8
 *
 * Get the size of a given image object's source image, when loading
 * it.
 *
 * @param[out] w out
 * @param[out] h out
 *
 * @see evas_object_image_load_size_get
 */
#define evas_obj_image_load_size_get(w, h) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_LOAD_SIZE_GET), EO_TYPECHECK(int *, w), EO_TYPECHECK(int *, h)

/**
 * @def evas_obj_image_load_scale_down_set
 * @since 1.8
 *
 * Set the scale down factor of a given image object's source image,
 * when loading it.
 *
 * @param[in] scale_down in
 *
 * @see evas_object_image_load_scale_down_set
 */
#define evas_obj_image_load_scale_down_set(scale_down) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_LOAD_SCALE_DOWN_SET), EO_TYPECHECK(int, scale_down)

/**
 * @def evas_obj_image_load_scale_down_get
 * @since 1.8
 *
 * Get the scale down factor of a given image object's source image,
 * when loading it.
 *
 * @param[out] scale_down out
 *
 * @see evas_object_image_load_scale_down_get
 */
#define evas_obj_image_load_scale_down_get(scale_down) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_LOAD_SCALE_DOWN_GET), EO_TYPECHECK(int *, scale_down)

/**
 * @def evas_obj_image_load_region_set
 * @since 1.8
 *
 * Inform a given image object to load a selective region of its
 * source image.
 *
 * @param[in] x in
 * @param[in] y in
 * @param[in] w in
 * @param[in] h in
 *
 * @see evas_object_image_load_region_set
 */
#define evas_obj_image_load_region_set(x, y, w, h) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_LOAD_REGION_SET), EO_TYPECHECK(int, x), EO_TYPECHECK(int, y), EO_TYPECHECK(int, w), EO_TYPECHECK(int, h)

/**
 * @def evas_obj_image_load_region_get
 * @since 1.8
 *
 * Retrieve the coordinates of a given image object's selective
 * (source image) load region.
 *
 * @param[out] x out
 * @param[out] y out
 * @param[out] w out
 * @param[out] h out
 *
 * @see evas_object_image_load_region_get
 */
#define evas_obj_image_load_region_get(x, y, w, h) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_LOAD_REGION_GET), EO_TYPECHECK(int *, x), EO_TYPECHECK(int *, y), EO_TYPECHECK(int *, w), EO_TYPECHECK(int *, h)

/**
 * @def evas_obj_image_load_orientation_set
 * @since 1.8
 *
 * Define if the orientation information in the image file should be honored.
 *
 * @param[in] enable in
 *
 * @see evas_object_image_load_orientation_set
 */
#define evas_obj_image_load_orientation_set(enable) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_LOAD_ORIENTATION_SET), EO_TYPECHECK(Eina_Bool, enable)

/**
 * @def evas_obj_image_load_orientation_get
 * @since 1.8
 *
 * Get if the orientation information in the image file should be honored.
 *
 * @param[out] enable out
 *
 * @see evas_object_image_load_orientation_get
 */
#define evas_obj_image_load_orientation_get(enable) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_LOAD_ORIENTATION_GET), EO_TYPECHECK(Eina_Bool *, enable)

/**
 * @def evas_obj_image_colorspace_set
 * @since 1.8
 *
 * Set the colorspace of a given image of the canvas.
 *
 * @param[in] cspace in
 *
 * @see evas_object_image_colorspace_set
 */
#define evas_obj_image_colorspace_set(cspace) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_COLORSPACE_SET), EO_TYPECHECK(Evas_Colorspace, cspace)

/**
 * @def evas_obj_image_colorspace_get
 * @since 1.8
 *
 * Get the colorspace of a given image of the canvas.
 *
 * @param[out] cspace out
 *
 * @see evas_object_image_colorspace_get
 */
#define evas_obj_image_colorspace_get(cspace) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_COLORSPACE_GET), EO_TYPECHECK(Evas_Colorspace *, cspace)

/**
 * @def evas_obj_image_video_surface_set
 * @since 1.8
 *
 * Set the video surface linked to a given image of the canvas
 *
 * @param[in] surf in
 *
 * @see evas_object_image_video_surface_set
 */
#define evas_obj_image_video_surface_set(surf) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_VIDEO_SURFACE_SET), EO_TYPECHECK(Evas_Video_Surface *, surf)

/**
 * @def evas_obj_image_video_surface_get
 * @since 1.8
 *
 * Get the video surface linked to a given image of the canvas
 *
 * @param[out] surf out
 *
 * @see evas_object_image_video_surface_get
 */
#define evas_obj_image_video_surface_get(surf) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_VIDEO_SURFACE_GET), EO_TYPECHECK(const Evas_Video_Surface **, surf)

/**
 * @def evas_obj_image_native_surface_set
 * @since 1.8
 *
 * Set the native surface of a given image of the canvas
 *
 * @param[in] surf in
 *
 * @see evas_object_image_native_surface_set
 */
#define evas_obj_image_native_surface_set(surf) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_NATIVE_SURFACE_SET), EO_TYPECHECK(Evas_Native_Surface *, surf)

/**
 * @def evas_obj_image_native_surface_get
 * @since 1.8
 *
 * Get the native surface of a given image of the canvas
 *
 * @param[out] surf out
 *
 * @see evas_object_image_native_surface_get
 */
#define evas_obj_image_native_surface_get(surf) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_NATIVE_SURFACE_GET), EO_TYPECHECK(Evas_Native_Surface **, surf)

/**
 * @def evas_obj_image_scale_hint_set
 * @since 1.8
 *
 * Set the scale hint of a given image of the canvas.
 *
 * @param[in] hint in
 *
 * @see evas_object_image_scale_hint_set
 */
#define evas_obj_image_scale_hint_set(hint) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_SCALE_HINT_SET), EO_TYPECHECK(Evas_Image_Scale_Hint, hint)

/**
 * @def evas_obj_image_scale_hint_get
 * @since 1.8
 *
 * Get the scale hint of a given image of the canvas.
 *
 * @param[out] hint out
 *
 * @see evas_object_image_scale_hint_get
 */
#define evas_obj_image_scale_hint_get(hint) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_SCALE_HINT_GET), EO_TYPECHECK(Evas_Image_Scale_Hint *, hint)

/**
 * @def evas_obj_image_content_hint_set
 * @since 1.8
 *
 * Set the content hint setting of a given image object of the canvas.
 *
 * @param[in] hint in
 *
 * @see evas_object_image_content_hint_set
 */
#define evas_obj_image_content_hint_set(hint) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_CONTENT_HINT_SET), EO_TYPECHECK(Evas_Image_Content_Hint, hint)

/**
 * @def evas_obj_image_content_hint_get
 * @since 1.8
 *
 * Get the content hint setting of a given image object of the canvas.
 *
 * @param[out] hint out
 *
 * @see evas_object_image_content_hint_get
 */
#define evas_obj_image_content_hint_get(hint) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_CONTENT_HINT_GET), EO_TYPECHECK(Evas_Image_Content_Hint *, hint)

/**
 * @def evas_obj_image_region_support_get
 * @since 1.8
 *
 * Get the support state of a given image
 *
 * @param[out] region out
 *
 * @see evas_object_image_region_support_get
 */
#define evas_obj_image_region_support_get(region) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_REGION_SUPPORT_GET), EO_TYPECHECK(Eina_Bool *, region)

/**
 * @def evas_obj_image_animated_get
 * @since 1.8
 *
 * Check if an image object can be animated (have multiple frames)
 *
 * @param[out] animated out
 *
 * @see evas_object_image_animated_get
 */
#define evas_obj_image_animated_get(animated) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_ANIMATED_GET), EO_TYPECHECK(Eina_Bool *, animated)

/**
 * @def evas_obj_image_animated_frame_count_get
 * @since 1.8
 *
 * Get the total number of frames of the image object.
 *
 * @param[out] frame_count out
 *
 * @see evas_object_image_animated_frame_count_get
 */
#define evas_obj_image_animated_frame_count_get(frame_count) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_ANIMATED_FRAME_COUNT_GET), EO_TYPECHECK(int *, frame_count)

/**
 * @def evas_obj_image_animated_loop_type_get
 * @since 1.8
 *
 * Get the kind of looping the image object does.
 *
 * @param[out] hint out
 *
 * @see evas_object_image_animated_loop_type_get
 */
#define evas_obj_image_animated_loop_type_get(hint) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_ANIMATED_LOOP_TYPE_GET), EO_TYPECHECK(Evas_Image_Animated_Loop_Hint *, hint)

/**
 * @def evas_obj_image_animated_loop_count_get
 * @since 1.8
 *
 * Get the number times the animation of the object loops.
 *
 * @param[out] loop_count out
 *
 * @see evas_object_image_animated_loop_count_get
 */
#define evas_obj_image_animated_loop_count_get(loop_count) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_ANIMATED_LOOP_COUNT_GET), EO_TYPECHECK(int *, loop_count)

/**
 * @def evas_obj_image_animated_frame_duration_get
 * @since 1.8
 *
 * Get the duration of a sequence of frames.
 *
 * @param[in] start_frame in
 * @param[in] frame_num in
 * @param[out] frame_duration out
 *
 * @see evas_object_image_animated_frame_duration_get
 */
#define evas_obj_image_animated_frame_duration_get(start_frame, frame_num, frame_duration) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_ANIMATED_FRAME_DURATION_GET), EO_TYPECHECK(int, start_frame), EO_TYPECHECK(int, frame_num), EO_TYPECHECK(double *, frame_duration)

/**
 * @def evas_obj_image_animated_frame_set
 * @since 1.8
 *
 * Set the frame to current frame of an image object
 *
 * @param[in] frame_index in
 *
 * @see evas_object_image_animated_frame_set
 */
#define evas_obj_image_animated_frame_set(frame_index) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_ANIMATED_FRAME_SET), EO_TYPECHECK(int, frame_index)

#ifdef __cplusplus
}
#endif

#endif
