/**
 * @defgroup Evas Evas
 * @ingroup EFL_Group
 *
 * @brief Evas provide a clean display canvas API.
 *
 * See @ref evas_main for more details
 *
 * @{
 */

/**
 * @page evas_main Evas
 *
 * @date 2000 (created)
 *
 * @section toc Table of Contents
 *
 * @li @ref evas_main_intro
 * @li @ref evas_main_work
 * @li @ref evas_main_next_steps
 *
 *
 * @section evas_main_intro Introduction
 *
 * Evas is a clean display canvas API for several target display systems
 * that can draw anti-aliased text, smooth super and sub-sampled scaled
 * images, alpha-blend objects, and much more.
 *
 * It abstracts any need to know much about what the characteristics of
 * your display system are or what graphics calls are used to draw them
 * and how. It deals on an object level where all you do is create and
 * manipulate objects in a canvas, set their properties, and the rest is
 * done for you.
 *
 * Evas optimises the rendering pipeline to minimise effort in redrawing
 * changes made to the canvas and so takes this work out of the
 * programmers hand, saving a lot of time and energy.
 *
 * It is small and lean, and is designed to work on embedded systems all the way
 * to large and powerful multi-CPU workstations. It can be compiled to
 * only have the features you need for your target platform if you so
 * wish, thus keeping it small and lean. It has several display
 * back-ends, letting it display on several display systems, making it
 * portable for cross-device and cross-platform development.
 *
 * @subsection evas_main_intro_not_evas What Evas is not?
 *
 * Evas is not a widget set or widget toolkit, however it is their
 * base. See @ref Elementary for a toolkit based on @ref Evas, @ref Edje_Group,
 * @ref Ecore_Group and other Enlightenment technologies.
 *
 * It is not dependent or aware of main loops, input or output
 * systems. Input should be polled from various sources and fed to
 * Evas. Similarly, it does not create windows or report windows updates
 * to your system, but just draws the pixels and report to the
 * user the areas that were changed.
 * @internal
 * Of course, these operations are quite
 * common and thus they are ready to use in Ecore, particularly in @ref
 * Ecore_Evas_Group.
 * @endinternal
 *
 *
 * @section evas_main_work How does Evas work?
 *
 * Evas is a canvas display library. This is markedly different from most
 * display and windowing systems as a canvas is structural and is also a
 * state engine, whereas most display and windowing systems are immediate
 * mode display targets. Evas handles the logic between a structural
 * display via its state engine, and controls the target windowing system
 * in order to produce rendered results of the current canvas' state on
 * the display.
 *
 * Immediate mode display systems retain very little, or no state. A
 * program executes a series of commands, as in the pseudo code:
 *
 * @verbatim
 * draw line from position (0, 0) to position (100, 200);
 *
 * draw rectangle from position (10, 30) to position (50, 500);
 *
 * bitmap_handle = create_bitmap();
 * scale bitmap_handle to size 100 x 100;
 * draw image bitmap_handle at position (10, 30);
 * @endverbatim
 *
 * The series of commands is executed by the windowing system and the
 * results are displayed on the screen (normally). Once the commands are
 * executed the display system has little or no idea of how to reproduce
 * this image again, and so has to be instructed by the application on how
 * to redraw sections of the screen whenever needed. Each successive
 * command is executed as instructed by the application and either
 * emulated by software or sent to the graphics hardware on the device to
 * be performed.
 *
 * The advantage of such a system is that it is simple, and gives a
 * program tight control over how something looks and is drawn. Given the
 * increasing complexity of displays and demands by users to have better
 * looking interfaces, more and more work is needing to be done at this
 * level by the internals of widget sets, custom display widgets and
 * other programs. This means that more and more logic and display rendering
 * code needs to be written each time the application needs to figure
 * out how to minimise redraws so that display is fast and interactive,
 * and keeps track of redraw logic. The power comes at a high-price,
 * with lots of extra code and work.  Programmers not very familiar with
 * graphics programming often make mistakes at this level and produce
 * code that is sub optimal. Those familiar with this kind of programming
 * simply get bored by writing the same code again and again.
 *
 * For example, if in the above scene, the windowing system requires the
 * application to redraw the area from 0, 0 to 50, 50 (also referred as
 * "expose event"), then the programmer must calculate manually the
 * updates and repaint it again:
 *
 * @verbatim
 * Redraw from position (0, 0) to position (50, 50):
 *
 * // what is in area (0, 0, 50, 50)?
 *
 * // 1. intersection part of line (0, 0) to (100, 200)?
 *    draw line from position (0, 0) to position (25, 50);
 *
 * // 2. intersection part of rectangle (10, 30) to (50, 500)?
 *    draw rectangle from position (10, 30) to position (50, 50)
 *
 * // 3. intersection part of image at (10, 30), size 100 x 100?
 *    bitmap_subimage = subregion from position (0, 0) to position (40, 20)
 *    draw image bitmap_subimage at position (10, 30);
 * @endverbatim
 *
 * You might have noticed that, if all elements in the above scene are
 * opaque, then the system is doing useless paints: part of the line is
 * behind the rectangle, and part of the rectangle is behind the image.
 * These useless paints tend to be very costly, as pixels tend to be 4 bytes
 * in size; thus an overlapping region of 100 x 100 pixels is around 40000
 * useless writes! You could write code to calculate the overlapping
 * areas and avoid painting then, but then it should be mixed with the
 * "expose event" handling mentioned above and you quickly realize that
 * the initially simpler method became really complex.
 *
 * Evas is a structural system in which the programmer creates and
 * manages display objects and their properties, and as a result of this
 * higher level state management, the canvas is able to redraw the set of
 * objects when needed to represent the current state of the canvas.
 *
 * For example, the pseudo code:
 *
 * @verbatim
 * line_handle = create_line();
 * set line_handle from position (0, 0) to position (100, 200);
 * show line_handle;
 *
 * rectangle_handle = create_rectangle();
 * move rectangle_handle to position (10, 30);
 * resize rectangle_handle to size 40 x 470;
 * show rectangle_handle;
 *
 * bitmap_handle = create_bitmap();
 * scale bitmap_handle to size 100 x 100;
 * move bitmap_handle to position (10, 30);
 * show bitmap_handle;
 *
 * render scene;
 * @endverbatim
 *
 * This may look longer, but when the display needs to be refreshed or
 * updated, you move, resize, show, or hide the objects that need to change.
 * You can simply think at the object logic level, and the canvas software
 * does the rest of the work for you, figuring out what actually changed in the
 * canvas since it had been last drawn, how to most efficiently redraw the canvas and 
 * its contents to reflect the current state, and then it can go off and do
 * the actual drawing of the canvas.
 *
 * This lets you think in a more natural way when dealing with
 * a display, and saves time and effort of working out how to load and
 * display images, render given the current display system, and so on. Since
 * Evas also is portable across different display systems, this also
 * gives you the ability to have their code ported and
 * displayed on different display systems with very little work.
 *
 * Evas can be seen as a display system that stands somewhere between a
 * widget set and an immediate mode display system. It retains basic
 * display logic, but does very little high-level logic such as
 * scrollbars, sliders, and push buttons.
 *
 * @section evas_main_next_steps Next Steps
 *
 * After you understood what Evas is and installed it in your system you
 * should proceed to understand the programming interface for all
 * objects, and then see the specifics for the most used elements.
 * You should take a while to learn @ref Ecore_Group and @ref Edje_Group as they
 * likely save you tons of work compared to using just Evas directly.
 *
 * Recommended reading:
 *
 * @li @ref Evas_Object_Group, where you get information on how to basically
 * manipulate generic objects lying on an Evas canvas, handle canvas
 * and object events, and so on.
 * @li @ref Evas_Object_Rectangle, to learn about the most basic object
 * type on Evas -- the rectangle.
 * @li @ref Evas_Object_Polygon, to learn how to create polygon elements
 * on the canvas.
 * @li @ref Evas_Line_Group, to learn how to create line elements on the
 * canvas.
 * @li @ref Evas_Object_Image, to learn about image objects, over which
 * Evas can do a plethora of operations.
 * @li @ref Evas_Object_Text, to learn how to create textual elements on
 * the canvas.
 * @li @ref Evas_Object_Textblock, to learn how to create multiline
 * textual elements on the canvas.
 * @internal
 * @li @ref Evas_Smart_Object_Group and @ref Evas_Smart_Group, to define
 * new objects that provide @b custom functions to handle clipping,
 * hiding, moving, resizing, color setting and more. These could
 * be as simple as a group of objects that move together (see @ref
 * Evas_Smart_Object_Clipped) up to implementations of what
 * ends to be a widget, providing some intelligence (thus the name)
 * to Evas objects -- like a button or check box, for example.
 * @endinternal
 *
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
#define EVAS_VERSION_MINOR 8

/**
 * @typedef Evas_Version
 * @brief The version of Evas.
 */
typedef struct _Evas_Version
{
   int major;     /**< Major component of the version */
   int minor;     /**< Minor component of the version */
   int micro;     /**< Micro component of the version */
   int revision;  /**< Revision component of the version */
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
 * @brief Identifier of callbacks to be set for Evas canvases or Evas objects.
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
   EVAS_CALLBACK_HOLD, /**< Hold Event, Informational purpose event to indicate something */
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
   EVAS_CALLBACK_LAST /**< Last element/sentinel -- not really an event */
} Evas_Callback_Type; /**< The types of events triggering a callback */

/**
 * @def EVAS_CALLBACK_PRIORITY_BEFORE
 * @brief Slightly more prioritized than default.
 * @since 1.1
 */
#define EVAS_CALLBACK_PRIORITY_BEFORE  -100
/**
 * @def EVAS_CALLBACK_PRIORITY_DEFAULT
 * @brief Default callback priority level
 * @since 1.1
 */
#define EVAS_CALLBACK_PRIORITY_DEFAULT 0
/**
 * @def EVAS_CALLBACK_PRIORITY_AFTER
 * @brief Slightly less prioritized than default.
 * @since 1.1
 */
#define EVAS_CALLBACK_PRIORITY_AFTER   100

/**
 * @typedef Evas_Callback_Priority
 *
 * @brief Callback priority value. Range is -32k to 32k. The lower the number, the
 * higher the priority.
 *
 * @since 1.1
 *
 * @see EVAS_CALLBACK_PRIORITY_AFTER
 * @see EVAS_CALLBACK_PRIORITY_BEFORE
 * @see EVAS_CALLBACK_PRIORITY_DEFAULT
 */
typedef short Evas_Callback_Priority;

/**
 * @brief Enumeration for Mouse Button events.
 */
typedef enum _Evas_Button_Flags
{
   EVAS_BUTTON_NONE = 0, /**< No extra mouse button data */
   EVAS_BUTTON_DOUBLE_CLICK = (1 << 0), /**< Second press of a double click */
   EVAS_BUTTON_TRIPLE_CLICK = (1 << 1) /**< Third press of a triple click */
} Evas_Button_Flags; /**< Flags for Mouse Button events */

/**
 * @brief Enumeration for events.
 */
typedef enum _Evas_Event_Flags
{
   EVAS_EVENT_FLAG_NONE = 0, /**< No fancy flags set */
   EVAS_EVENT_FLAG_ON_HOLD = (1 << 0), /**< The event is being delivered but should be put "on hold" until the on hold flag is unset \n 
   The event should be used for informational purposes and maybe for some indications visually, but should not actually perform anything. */
   EVAS_EVENT_FLAG_ON_SCROLL = (1 << 1) /**< The event occurs while scrolling \n 
   For example, DOWN event occurs during scrolling; the event should be used for informational purposes and maybe for some indications visually, but should not actually perform anything. */
} Evas_Event_Flags; /**< Flags for Events */

/**
 * @brief Enumeration for touch point states.
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
 * @brief Enumeration for font hinting.
 * @ingroup Evas_Font_Group
 */
typedef enum _Evas_Font_Hinting_Flags
{
   EVAS_FONT_HINTING_NONE, /**< No font hinting */
   EVAS_FONT_HINTING_AUTO, /**< Automatic font hinting */
   EVAS_FONT_HINTING_BYTECODE /**< Bytecode font hinting */
} Evas_Font_Hinting_Flags; /**< Flags for Font Hinting */

/**
 * @brief Enumeration for pixel data of colorspaces supported by Evas.
 * @ingroup Evas_Object_Image
 */
typedef enum _Evas_Colorspace
{
   EVAS_COLORSPACE_ARGB8888, /**< ARGB 32 bits per pixel, high-byte is Alpha, accessed 1 32bit word at a time */
   /* The following are not currently supported - but planned for the future */
   EVAS_COLORSPACE_YCBCR422P601_PL, /**< YCbCr 4:2:2 Planar, ITU.BT-601 specifications. The data pointed to is just an array of row pointer, pointing to the Y rows, then the Cb, then Cr rows */
   EVAS_COLORSPACE_YCBCR422P709_PL, /**< YCbCr 4:2:2 Planar, ITU.BT-709 specifications. The data pointed to is just an array of row pointer, pointing to the Y rows, then the Cb, then Cr rows */
   EVAS_COLORSPACE_RGB565_A5P, /**< 16bit rgb565 + Alpha plane at end - 5 bits of the 8 being used per alpha byte */
   EVAS_COLORSPACE_GRY8, /**< 8bit grayscale */
   EVAS_COLORSPACE_YCBCR422601_PL, /**<  YCbCr 4:2:2, ITU.BT-601 specifications. The data pointed to is just an array of row pointer, pointing to line of Y,Cb,Y,Cr bytes */
   EVAS_COLORSPACE_YCBCR420NV12601_PL, /**< YCbCr 4:2:0, ITU.BT-601 specification. The data pointed to is just an array of row pointer, pointing to the Y rows, then the Cb,Cr rows. */
   EVAS_COLORSPACE_YCBCR420TM12601_PL, /**< YCbCr 4:2:0, ITU.BT-601 specification. The data pointed to is just an array of tiled row pointer, pointing to the Y rows, then the Cb,Cr rows. */
   EVAS_COLORSPACE_AGRY88, /**< AY 8bits Alpha and 8bits Grey, accessed 1 16bits at a time */
   EVAS_COLORSPACE_ETC1, /**< OpenGL ETC1 encoding of RGB texture (4 bit per pixel) @since 1.10 */
   EVAS_COLORSPACE_RGB8_ETC2, /**< OpenGL GL_COMPRESSED_RGB8_ETC2 texture compression format (4 bit per pixel) @since 1.10 */
   EVAS_COLORSPACE_RGBA8_ETC2_EAC, /**< OpenGL GL_COMPRESSED_RGBA8_ETC2_EAC texture compression format, supports alpha (8 bit per pixel) @since 1.10 */
   EVAS_COLORSPACE_ETC1_ALPHA, /**< ETC1 with alpha support using two planes: ETC1 RGB and ETC1 grey for alpha @since 1.11 */
} Evas_Colorspace; /**< Colorspaces for pixel data supported by Evas */

/**
 * @brief   Enumeration for modes of packing items into cells in a table.
 *
 * @see     evas_object_table_homogeneous_set() for an explanation of the function of
 *          each one.
 * @ingroup Evas_Object_Table
 */
typedef enum _Evas_Object_Table_Homogeneous_Mode
{
   EVAS_OBJECT_TABLE_HOMOGENEOUS_NONE = 0,
   EVAS_OBJECT_TABLE_HOMOGENEOUS_TABLE = 1,
   EVAS_OBJECT_TABLE_HOMOGENEOUS_ITEM = 2
} Evas_Object_Table_Homogeneous_Mode; /**< Table cell pack mode */

typedef struct _Evas_Coord_Rectangle       Evas_Coord_Rectangle; /**< @brief A generic rectangle handle */
typedef struct _Evas_Point                 Evas_Point;   /**< @brief Integer point */

typedef struct _Evas_Coord_Point           Evas_Coord_Point;    /**< @brief Evas_Coord point */
typedef struct _Evas_Coord_Precision_Point Evas_Coord_Precision_Point;   /**< @brief Evas_Coord point with sub-pixel precision */

typedef struct _Evas_Coord_Size            Evas_Coord_Size;    /**< @brief Evas_Coord size @since 1.8 */
typedef struct _Evas_Position              Evas_Position;   /**< @brief Associates the given point in Canvas and Output */
typedef struct _Evas_Precision_Position    Evas_Precision_Position;   /**< @brief Associates the given point in Canvas and Output, with sub-pixel precision */

/**
 * @typedef Evas_Smart_Class
 *
 * @brief   A smart object @b base class definition.
 *
 * @ingroup Evas_Smart_Group
 */
typedef struct _Evas_Smart_Class Evas_Smart_Class;

/**
 * @typedef Evas_Smart_Interface
 *
 * @brief   A smart object @b base interface definition.
 *
 * @since   1.7
 *
 * @remarks An Evas interface is exactly like the OO-concept: It is a 'contract' or
 *          API that the object is declared to support. A smart object may have
 *          more than one interface, thus extending the behavior it gets from
 *          sub-classing.
 *
 * @ingroup Evas_Smart_Group
 */
typedef struct _Evas_Smart_Interface         Evas_Smart_Interface;

/**
 * @typedef Evas_Smart_Cb_Description
 *
 * @brief   A smart object callback description, used to provide introspection.
 *
 * @ingroup Evas_Smart_Group
 */
typedef struct _Evas_Smart_Cb_Description Evas_Smart_Cb_Description;

/**
 * @typedef Evas_Map
 *
 * @brief  An opaque handle to map points.
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
 * @brief  An opaque handle to an Evas canvas.
 *
 * @see evas_new()
 * @see evas_free()
 *
 * @ingroup Evas_Canvas
 */
typedef struct _Evas Evas;

/**
 * @typedef Evas_Object
 * @brief   An Evas Object handle.
 * @ingroup Evas_Object_Group
 */
typedef struct _Evas_Object         Evas_Object;

typedef void                        Evas_Performance; /**< @brief Evas Performance handle */
typedef struct _Evas_Modifier       Evas_Modifier; /**< @brief Opaque type containing information on which modifier keys are registered in an Evas canvas */
typedef struct _Evas_Lock           Evas_Lock; /**< @brief Opaque type containing information on which lock keys are registered in an Evas canvas */
typedef struct _Evas_Smart          Evas_Smart; /**< @brief Evas Smart Object handle */
typedef struct _Evas_Native_Surface Evas_Native_Surface; /**< @brief Generic datatype for engine specific native surface information */

/**
 * @typedef Evas_Video_Surface
 *
 * @brief  A generic data type for video specific surface information.
 *
 * @since  1.1
 *
 * @see evas_object_image_video_surface_set
 * @see evas_object_image_video_surface_get
 */
typedef struct _Evas_Video_Surface Evas_Video_Surface;

typedef unsigned long long         Evas_Modifier_Mask;  /**< @brief An Evas modifier mask type */

typedef int                        Evas_Coord;         /**< @brief Evas x y coordinates */
typedef int                        Evas_Font_Size;     /**< @brief Evas Font Sizes */
typedef int                        Evas_Angle;         /**< @brief Evas angle */

struct _Evas_Coord_Rectangle /**< @brief A rectangle in Evas_Coord */
{
   Evas_Coord x; /**< Top-left x co-ordinate of rectangle */
   Evas_Coord y; /**< Top-left y co-ordinate of rectangle */
   Evas_Coord w; /**< Width of rectangle */
   Evas_Coord h; /**< Height of rectangle */
};

struct _Evas_Point
{
   int x, y;
};

struct _Evas_Coord_Point
{
   Evas_Coord x; /**< X co-ordinate */
   Evas_Coord y; /**< Y co-ordinate */
};

struct _Evas_Coord_Size
{
   Evas_Coord w; /**< Width */
   Evas_Coord h; /**< Height */
};

struct _Evas_Coord_Precision_Point
{
   Evas_Coord x, y;
   double     xsub, ysub;
};

struct _Evas_Position
{
   Evas_Point       output;
   Evas_Coord_Point canvas; /**< Position on the canvas */
};

struct _Evas_Precision_Position
{
   Evas_Point                 output;
   Evas_Coord_Precision_Point canvas;
};

/**
 * @brief  Enumeration for aspect types or policies for scaling size hints, used for evas_object_size_hint_aspect_set().
 */
typedef enum _Evas_Aspect_Control
{
   EVAS_ASPECT_CONTROL_NONE = 0, /**< Unset scaling preference */
   EVAS_ASPECT_CONTROL_NEITHER = 1, /**< Same effect as unset preference on scaling */
   EVAS_ASPECT_CONTROL_HORIZONTAL = 2, /**< Use all horizontal container space to place an object, using the given aspect */
   EVAS_ASPECT_CONTROL_VERTICAL = 3, /**< Use all vertical container space to place an object, using the given aspect */
   EVAS_ASPECT_CONTROL_BOTH = 4 /**< Use all horizontal @b and vertical container spaces to place an object (never growing it out of those bounds), using the given aspect */
} Evas_Aspect_Control; /**< Aspect types or policies for scaling size hints, used for evas_object_size_hint_aspect_set() */

/**
 * @brief  Enumeration for object's display modes.
 */
typedef enum _Evas_Display_Mode
{
   EVAS_DISPLAY_MODE_NONE = 0, /**<Default display mode */
   EVAS_DISPLAY_MODE_INHERIT = 1, /**< Object display mode depends on its ancestor display mode */
   EVAS_DISPLAY_MODE_COMPRESS = 2, /**< Give compress display mode hint to object */
   EVAS_DISPLAY_MODE_EXPAND = 3, /**< Give expand display mode hint to object */
   EVAS_DISPLAY_MODE_DONT_CHANGE = 4 /**< Object does not change display mode */
} Evas_Display_Mode; /**< Object display mode type related with compress and expand or etc mode */

typedef struct _Evas_Pixel_Import_Source Evas_Pixel_Import_Source; /**< @brief A source description of pixels for importing pixels */
typedef struct _Evas_Engine_Info         Evas_Engine_Info; /**< @brief A generic Evas Engine information structure */
typedef struct _Evas_Device              Evas_Device; /**< @brief A source device handle - where the event came from */
typedef struct _Evas_Event_Mouse_Down    Evas_Event_Mouse_Down; /**< @brief Event structure for #EVAS_CALLBACK_MOUSE_DOWN event callbacks */
typedef struct _Evas_Event_Mouse_Up      Evas_Event_Mouse_Up; /**< @brief Event structure for #EVAS_CALLBACK_MOUSE_UP event callbacks */
typedef struct _Evas_Event_Mouse_In      Evas_Event_Mouse_In; /**< @brief Event structure for #EVAS_CALLBACK_MOUSE_IN event callbacks */
typedef struct _Evas_Event_Mouse_Out     Evas_Event_Mouse_Out; /**< @brief Event structure for #EVAS_CALLBACK_MOUSE_OUT event callbacks */
typedef struct _Evas_Event_Mouse_Move    Evas_Event_Mouse_Move; /**< @brief Event structure for #EVAS_CALLBACK_MOUSE_MOVE event callbacks */
typedef struct _Evas_Event_Mouse_Wheel   Evas_Event_Mouse_Wheel; /**< @brief Event structure for #EVAS_CALLBACK_MOUSE_WHEEL event callbacks */
typedef struct _Evas_Event_Multi_Down    Evas_Event_Multi_Down; /**< @brief Event structure for #EVAS_CALLBACK_MULTI_DOWN event callbacks */
typedef struct _Evas_Event_Multi_Up      Evas_Event_Multi_Up; /**< @brief Event structure for #EVAS_CALLBACK_MULTI_UP event callbacks */
typedef struct _Evas_Event_Multi_Move    Evas_Event_Multi_Move; /**< @brief Event structure for #EVAS_CALLBACK_MULTI_MOVE event callbacks */
typedef struct _Evas_Event_Key_Down      Evas_Event_Key_Down; /**< @brief Event structure for #EVAS_CALLBACK_KEY_DOWN event callbacks */
typedef struct _Evas_Event_Key_Up        Evas_Event_Key_Up; /**< @brief Event structure for #EVAS_CALLBACK_KEY_UP event callbacks */
typedef struct _Evas_Event_Hold          Evas_Event_Hold; /**< @brief Event structure for #EVAS_CALLBACK_HOLD event callbacks */

/**
 * @brief  Enumeration for load errors.
 */
typedef enum _Evas_Load_Error
{
   EVAS_LOAD_ERROR_NONE = 0, /**< No error on load */
   EVAS_LOAD_ERROR_GENERIC = 1, /**< A non-specific error occurred */
   EVAS_LOAD_ERROR_DOES_NOT_EXIST = 2, /**< File (or file path) does not exist */
   EVAS_LOAD_ERROR_PERMISSION_DENIED = 3, /**< Permission denied to an existing file (or path) */
   EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED = 4, /**< Allocation of resources failure prevented load */
   EVAS_LOAD_ERROR_CORRUPT_FILE = 5, /**< File corrupt (but is detected as a known format) */
   EVAS_LOAD_ERROR_UNKNOWN_FORMAT = 6 /**< File is not a known format */
} Evas_Load_Error; /**< Evas image load error codes one can get - see evas_load_error_str() */

/**
 * @brief  Enumeration for allocation errors.
 */
typedef enum _Evas_Alloc_Error
{
   EVAS_ALLOC_ERROR_NONE = 0, /**< No allocation error */
   EVAS_ALLOC_ERROR_FATAL = 1, /**< Allocation failed despite attempts to free up memory */
   EVAS_ALLOC_ERROR_RECOVERED = 2 /**< Allocation succeeded, but extra memory had to be found by freeing up speculative resources */
} Evas_Alloc_Error; /**< Possible allocation errors returned by evas_alloc_error() */

/**
 * @brief  Enumeration for spread of image fill.
 */
typedef enum _Evas_Fill_Spread
{
   EVAS_TEXTURE_REFLECT = 0, /**< Image fill tiling mode - tiling reflects */
   EVAS_TEXTURE_REPEAT = 1, /**< Tiling repeats */
   EVAS_TEXTURE_RESTRICT = 2, /**< Tiling clamps - range offset ignored */
   EVAS_TEXTURE_RESTRICT_REFLECT = 3, /**< Tiling clamps and any range offset reflects */
   EVAS_TEXTURE_RESTRICT_REPEAT = 4, /**< Tiling clamps and any range offset repeats */
   EVAS_TEXTURE_PAD = 5 /**< Tiling extends with end values */
} Evas_Fill_Spread; /**< Fill types used for evas_object_image_fill_spread_set() */

/**
 * @brief  Enumeration for aspect types or policies.
 */
typedef enum _Evas_Pixel_Import_Pixel_Format
{
   EVAS_PIXEL_FORMAT_NONE = 0, /**< No pixel format */
   EVAS_PIXEL_FORMAT_ARGB32 = 1, /**< ARGB 32bit pixel format with A in the high byte per 32bit pixel word */
   EVAS_PIXEL_FORMAT_YUV420P_601 = 2 /**< YUV 420 Planar format with CCIR 601 color encoding with contiguous planes in the order Y, U and V */
} Evas_Pixel_Import_Pixel_Format; /**< Pixel format for import call. See evas_object_image_pixels_import() */

struct _Evas_Pixel_Import_Source
{
   Evas_Pixel_Import_Pixel_Format format; /**< Pixel format type i.e. ARGB32, YUV420P_601, and so on */
   int                            w, h; /**< Width and height of source in pixels */
   void                         **rows; /**< Array of pointers (size depends on format) pointing to left edge of each scanline */
};

/* Magic version number to know what the native surf struct looks like */
#define EVAS_NATIVE_SURFACE_VERSION 3

/**
 * @brief Native surface types that image object supports
 *
 * @see Evas_Native_Surface
 * @see evas_object_image_native_surface_set()
 */
typedef enum _Evas_Native_Surface_Type
{
   EVAS_NATIVE_SURFACE_NONE, /**< No surface type */
   EVAS_NATIVE_SURFACE_X11,  /**< X Window system based type. pixmap id or visual of the pixmap */
   EVAS_NATIVE_SURFACE_OPENGL, /**< OpenGL system based type. texture or framebuffer id*/
   EVAS_NATIVE_SURFACE_TIZEN, /**< @internal don't use this type but @see EVAS_NATIVE_SURFACE_TBM */
   EVAS_NATIVE_SURFACE_TBM    /**< Tizen system based type. This is used for tizen buffer manager. */
} Evas_Native_Surface_Type;

/**
 * @brief A generic datatype for engine specific native surface information.
 *
 * Please fill up Evas_Native_Surface fields that regarded with current surface
 * type. If you want to set the native surface type to
 * EVAS_NATIVE_SURFACE_X11, you need to set union data with x11.visual or
 * x11.pixmap. If you need to set the native surface as
 * EVAS_NATIVE_SURFACE_OPENGL, on the other hand, you need to set union data
 * with opengl.texture_id or opengl.framebuffer_id and so on. The version field
 * should be set with EVAS_NATIVE_SURFACE_VERSION in order to check abi
 * break in your application on the different efl library versions.
 *
 * @warning Native surface types totally depend on the system. Please
 *          be aware that the types are supported on your system before using
 *          them.
 * @see evas_object_image_native_surface_set()
 */
struct _Evas_Native_Surface
{
   int                      version; /**< Current Native Surface Version. Use EVAS_NATIVE_SURFACE_VERSION */
   Evas_Native_Surface_Type type; /**< Surface type. @see Evas_Native_Surface_Type */
   union {
      struct
      {
         void         *visual; /**< Visual of the pixmap to use (Visual) */
         unsigned long pixmap; /**< Pixmap ID to use (Pixmap) */
      } x11; /**< Set this struct fields if your surface data is X11 based. */
      struct
      {
         unsigned int texture_id; /**< opengl texture ID to use from glGenTextures() */
         unsigned int framebuffer_id; /**< 0 if this is not an FBO, otherwise FBO ID from glGenFramebuffers() */
         unsigned int internal_format; /**< Same as 'internalFormat' for glTexImage2D() */
         unsigned int format; /**< Same as 'format' for glTexImage2D() */
         unsigned int x, y, w, h; /**< Region inside the texture to use (Image size is assumed as texture size, with 0, 0 being the top-left and co-ordinates working down to the right and bottom being positive) */
      } opengl; /**< Set this struct fields if your surface data is OpenGL based. */
      struct
      {
         void *buffer; /**< tbm surface */
         int   rot; /**< rotation (0, 90, 180, 270) */
         float ratio; /**< width/height ratio of the source image */
         int   flip; /**< flip (0:none, 1:horizontal, 2:vertical, 3:both) */
      } tizen; /**< Set this struct fields if your surface data is Tizen based. */
   } data; /**< Choose one union data according to your surface. */
};

/**
 * @def EVAS_VIDEO_SURFACE_VERSION
 * @brief  Definition of the magic version number to know what the video surf struct looks like.
 * @since  1.1
 */
#define EVAS_VIDEO_SURFACE_VERSION 1

typedef void (*Evas_Video_Cb)(void *data, Evas_Object *obj, const Evas_Video_Surface *surface);
typedef void (*Evas_Video_Coord_Cb)(void *data, Evas_Object *obj, const Evas_Video_Surface *surface, Evas_Coord a, Evas_Coord b);

/**
 * @brief Struct of Evas Video Surface.
 */
struct _Evas_Video_Surface
{
   int                 version;

   Evas_Video_Coord_Cb move; /**< Moves the video surface to this position */
   Evas_Video_Coord_Cb resize; /**< Resizes the video surface to that size */
   Evas_Video_Cb       show; /**< Shows the video overlay surface */
   Evas_Video_Cb       hide; /**< Hides the video overlay surface */
   Evas_Video_Cb       update_pixels; /**< Updates the Evas_Object_Image pixels when called */

   Evas_Object        *parent;
   void               *data;
};

#define EVAS_LAYER_MIN                   -32768 /**< @brief Bottom-most layer number */
#define EVAS_LAYER_MAX                   32767 /**< @brief Top-most layer number */

#define EVAS_COLOR_SPACE_ARGB            0 /**< @brief Not used for anything */
#define EVAS_COLOR_SPACE_AHSV            1 /**< @brief Not used for anything */
#define EVAS_TEXT_INVALID                -1 /**< @brief Not used for anything */
#define EVAS_TEXT_SPECIAL                -2 /**< @brief Not used for anything */

#define EVAS_HINT_EXPAND                 1.0 /**< @brief Use with evas_object_size_hint_weight_set(), evas_object_size_hint_weight_get(), evas_object_size_hint_expand_set(), evas_object_size_hint_expand_get() */
#define EVAS_HINT_FILL                   -1.0 /**< @brief Use with evas_object_size_hint_align_set(), evas_object_size_hint_align_get(), evas_object_size_hint_fill_set(), evas_object_size_hint_fill_get() */

/**
 * @brief Convenience macro to make it easier to understand that align is also used for fill properties (as fill is mutually exclusive to align)
 * @ingroup Evas_Object_Group_Size_Hints
 */
#define evas_object_size_hint_fill_set   evas_object_size_hint_align_set

/**
 * @brief Convenience macro to make it easier to understand that align is also used for fill properties (as fill is mutually exclusive to align)
 * @ingroup Evas_Object_Group_Size_Hints
 */
#define evas_object_size_hint_fill_get   evas_object_size_hint_align_get

/**
 * @brief Convenience macro to make it easier to understand that weight is also used for expand properties
 * @ingroup Evas_Object_Group_Size_Hints
 */
#define evas_object_size_hint_expand_set evas_object_size_hint_weight_set

/**
 * @brief Convenience macro to make it easier to understand that weight is also used for expand properties
 * @ingroup Evas_Object_Group_Size_Hints
 */
#define evas_object_size_hint_expand_get evas_object_size_hint_weight_get

/**
 * @brief   Enumeration for modes of object rendering to output.
 * @ingroup Evas_Object_Group_Extras
 */
typedef enum _Evas_Render_Op
{
   EVAS_RENDER_BLEND = 0, /**< Default op: d = d*(1-sa) + s */
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
} Evas_Render_Op; /**< How the object should be rendered to output */

/**
 * @brief  Enumeration for border fill mode.
 */
typedef enum _Evas_Border_Fill_Mode
{
   EVAS_BORDER_FILL_NONE = 0, /**< Image's center region is @b not to be rendered */
   EVAS_BORDER_FILL_DEFAULT = 1, /**< Image's center region is to be @b blended with objects underneath it, if it has transparency. This is the default behavior for image objects */
   EVAS_BORDER_FILL_SOLID = 2 /**< Image's center region is to be made solid, even if it has transparency on it */
} Evas_Border_Fill_Mode; /**< How an image's center region (the complement to the border region) should be rendered by Evas */

/**
 * @brief  Enumeration for image scale hints.
 */
typedef enum _Evas_Image_Scale_Hint
{
   EVAS_IMAGE_SCALE_HINT_NONE = 0, /**< No scale hint at all */
   EVAS_IMAGE_SCALE_HINT_DYNAMIC = 1, /**< Image is being re-scaled over time, thus turning scaling cache @b off for its data */
   EVAS_IMAGE_SCALE_HINT_STATIC = 2 /**< Image is not being re-scaled over time, thus turning scaling cache @b on for its data */
} Evas_Image_Scale_Hint; /**< How an image's data is to be treated by Evas, with regard to scaling cache */

/**
 * @brief  Enumeration for animated loop hints.
 */
typedef enum _Evas_Image_Animated_Loop_Hint
{
   EVAS_IMAGE_ANIMATED_HINT_NONE = 0,
   EVAS_IMAGE_ANIMATED_HINT_LOOP = 1, /**< Image's animation mode is loop like 1->2->3->1->2->3 */
   EVAS_IMAGE_ANIMATED_HINT_PINGPONG = 2 /**< Image's animation mode is pingpong like 1->2->3->2->1-> ... */
} Evas_Image_Animated_Loop_Hint;

/**
 * @brief  Enumeration for engine rendering modes.
 */
typedef enum _Evas_Engine_Render_Mode
{
   EVAS_RENDER_MODE_BLOCKING = 0,
   EVAS_RENDER_MODE_NONBLOCKING = 1,
} Evas_Engine_Render_Mode;

/**
 * @brief  Enumeration for image content hints.
 */
typedef enum _Evas_Image_Content_Hint
{
   EVAS_IMAGE_CONTENT_HINT_NONE = 0, /**< No hint at all */
   EVAS_IMAGE_CONTENT_HINT_DYNAMIC = 1, /**< The contents change over time */
   EVAS_IMAGE_CONTENT_HINT_STATIC = 2 /**< The contents do not change over time */
} Evas_Image_Content_Hint; /**< How an image's data is to be treated by Evas, for optimization */

/**
 * @brief  Enumeration for device class.
 */
typedef enum _Evas_Device_Class
{
   EVAS_DEVICE_CLASS_NONE, /**< Not a device @since 1.8 */
   EVAS_DEVICE_CLASS_SEAT, /**< The user/seat (the user themselves) @since 1.8 */
   EVAS_DEVICE_CLASS_KEYBOARD, /**< Regular keyboard, numberpad or attached buttons @since 1.8 */
   EVAS_DEVICE_CLASS_MOUSE, /**< Mouse, trackball or touchpad relative motion device @since 1.8 */
   EVAS_DEVICE_CLASS_TOUCH, /**< Touchscreen with fingers or stylus @since 1.8 */
   EVAS_DEVICE_CLASS_PEN, /**< Special pen device @since 1.8 */
   EVAS_DEVICE_CLASS_POINTER, /**< Laser pointer, wii-style or "minority report" pointing device @since 1.8 */
   EVAS_DEVICE_CLASS_GAMEPAD /**<  Gamepad controller or joystick @since 1.8 */
} Evas_Device_Class;
   
struct _Evas_Engine_Info /** @brief Generic engine information. Generic info is not of much use. */
{
   int magic; /**< Magic number */
};

struct _Evas_Event_Mouse_Down /** @brief Mouse button press event */
{
   int               button; /**< Mouse button number that went down (1 - 32) */

   Evas_Point        output; /**< The X/Y location of the cursor */
   Evas_Coord_Point  canvas; /**< The X/Y location of the cursor */

   void             *data;
   Evas_Modifier    *modifiers; /**< Modifier keys pressed during the event */
   Evas_Lock        *locks;

   Evas_Button_Flags flags; /**< Button flags set during the event */
   unsigned int      timestamp;
   Evas_Event_Flags  event_flags;
   Evas_Device      *dev;
};

struct _Evas_Event_Mouse_Up /** @brief Mouse button release event */
{
   int               button; /**< Mouse button number that is raised (1 - 32) */

   Evas_Point        output; /**< The X/Y location of the cursor */
   Evas_Coord_Point  canvas; /**< The X/Y location of the cursor */

   void             *data;
   Evas_Modifier    *modifiers; /**< Modifier keys pressed during the event */
   Evas_Lock        *locks;

   Evas_Button_Flags flags; /**< Button flags set during the event */
   unsigned int      timestamp;
   Evas_Event_Flags  event_flags;
   Evas_Device      *dev;
};

struct _Evas_Event_Mouse_In /** @brief Mouse enter event */
{
   int              buttons; /**< Button pressed mask, Bits set to @c 1 are buttons currently pressed (bit 0 = mouse button 1, bit 1 = mouse button 2 and so on) */

   Evas_Point        output; /**< The X/Y location of the cursor */
   Evas_Coord_Point  canvas; /**< The X/Y location of the cursor */

   void            *data;
   Evas_Modifier    *modifiers; /**< Modifier keys pressed during the event */
   Evas_Lock       *locks;
   unsigned int     timestamp;
   Evas_Event_Flags event_flags;
   Evas_Device     *dev;
};

struct _Evas_Event_Mouse_Out /** @brief Mouse leave event */
{
   int              buttons; /**< Button pressed mask, Bits set to @c 1 are buttons currently pressed (bit 0 = mouse button 1, bit 1 = mouse button 2 and so on) */

   Evas_Point        output; /**< The X/Y location of the cursor */
   Evas_Coord_Point  canvas; /**< The X/Y location of the cursor */

   void            *data;
   Evas_Modifier    *modifiers; /**< Modifier keys pressed during the event */
   Evas_Lock       *locks;
   unsigned int     timestamp;
   Evas_Event_Flags event_flags;
   Evas_Device     *dev;
};

struct _Evas_Event_Mouse_Move /** @brief Mouse move event */
{
   int              buttons; /**< Button pressed mask, Bits set to @c 1 are buttons currently pressed (bit 0 = mouse button 1, bit 1 = mouse button 2 and so on) */

   Evas_Position    cur; /**< Current mouse position */
   Evas_Position    prev; /**< Previous mouse position */

   void            *data;
   Evas_Modifier    *modifiers; /**< Modifier keys pressed during the event */
   Evas_Lock       *locks;
   unsigned int     timestamp;
   Evas_Event_Flags event_flags;
   Evas_Device     *dev;
};

struct _Evas_Event_Mouse_Wheel /** @brief Wheel event */
{
   int              direction; /* 0 = default up/down wheel FIXME: more wheel types */
   int              z; /* ...,-2,-1 = down, 1,2,... = up */

   Evas_Point        output; /**< The X/Y location of the cursor */
   Evas_Coord_Point  canvas; /**< The X/Y location of the cursor */

   void            *data;
   Evas_Modifier    *modifiers; /**< Modifier keys pressed during the event */
   Evas_Lock       *locks;
   unsigned int     timestamp;
   Evas_Event_Flags event_flags;
   Evas_Device     *dev;
};

struct _Evas_Event_Multi_Down /** @brief Multi button press event */
{
   int                        device; /**< Multi device number that went down (1 or more for extra touches) */
   double                     radius, radius_x, radius_y;
   double                     pressure, angle;

   Evas_Point                 output;
   Evas_Coord_Precision_Point canvas;

   void                      *data;
   Evas_Modifier    *modifiers; /**< Modifier keys pressed during the event */
   Evas_Lock                 *locks;

   Evas_Button_Flags flags; /**< Button flags set during the event */
   unsigned int               timestamp;
   Evas_Event_Flags           event_flags;
   Evas_Device               *dev;
};

struct _Evas_Event_Multi_Up /** @brief Multi button release event */
{
   int                        device; /**< Multi device number that went up (1 or more for extra touches) */
   double                     radius, radius_x, radius_y;
   double                     pressure, angle;

   Evas_Point                 output;
   Evas_Coord_Precision_Point canvas;

   void                      *data;
   Evas_Modifier    *modifiers; /**< Modifier keys pressed during the event */
   Evas_Lock                 *locks;

   Evas_Button_Flags flags; /**< Button flags set during the event */
   unsigned int               timestamp;
   Evas_Event_Flags           event_flags;
   Evas_Device               *dev;
};

struct _Evas_Event_Multi_Move /** @brief Multi button down event */
{
   int                     device; /**< Multi device number that moved (1 or more for extra touches) */
   double                  radius, radius_x, radius_y;
   double                  pressure, angle;

   Evas_Precision_Position cur;

   void                   *data;
   Evas_Modifier    *modifiers; /**< Modifier keys pressed during the event */
   Evas_Lock              *locks;
   unsigned int            timestamp;
   Evas_Event_Flags        event_flags;
   Evas_Device            *dev;
};

struct _Evas_Event_Key_Down /** @brief Key press event */
{
   char            *keyname; /**< Name string of the key pressed */
   void            *data;
   Evas_Modifier    *modifiers; /**< Modifier keys pressed during the event */
   Evas_Lock       *locks;

   const char      *key; /**< Logical key : (example, shift+1 == exclamation) */
   const char      *string; /**< UTF8 string if this keystroke has produced a visible string to be ADDED */
   const char      *compose; /**< UTF8 string if this keystroke has modified a string in the middle of being composed - this string replaces the previous one */
   unsigned int     timestamp;
   Evas_Event_Flags event_flags;
   Evas_Device     *dev;
};

struct _Evas_Event_Key_Up /** @brief Key release event */
{
   char            *keyname; /**< Name string of the key released */
   void            *data;
   Evas_Modifier    *modifiers; /**< Modifier keys pressed during the event */
   Evas_Lock       *locks;

   const char      *key; /**< Logical key : (example, shift+1 == exclamation) */
   const char      *string; /**< UTF8 string if this keystroke has produced a visible string to be ADDED */
   const char      *compose; /**< UTF8 string if this keystroke has modified a string in the middle of being composed - this string replaces the previous one */
   unsigned int     timestamp;
   Evas_Event_Flags event_flags;
   Evas_Device     *dev;
};

struct _Evas_Event_Hold /** @brief Hold change event */
{
   int              hold; /**< Hold flag */
   void            *data;

   unsigned int     timestamp;
   Evas_Event_Flags event_flags;
   Evas_Device     *dev;
};

/**
 * @brief   Enumeration for handling mouse pointer.
 *
 * @remarks In the mode #EVAS_OBJECT_POINTER_MODE_AUTOGRAB, when a mouse button
 *          is pressed down over an object and held, with the mouse pointer
 *          being moved outside of it, the pointer still behaves as being bound
 *          to that object, albeit out of its drawing region. When the button
 *          is released, the event is fed to the object, that may check if
 *          the final position is over it or not and do something about it.
 *
 * @remarks In the mode #EVAS_OBJECT_POINTER_MODE_NOGRAB, the pointer is
 *          always bound to the object right below it.
 *
 * @ingroup Evas_Object_Group_Extras
 */
typedef enum _Evas_Object_Pointer_Mode
{
   EVAS_OBJECT_POINTER_MODE_AUTOGRAB, /**< Default, X11-like */
   EVAS_OBJECT_POINTER_MODE_NOGRAB, /**< Pointer always bound to the object right below it */
   EVAS_OBJECT_POINTER_MODE_NOGRAB_NO_REPEAT_UPDOWN /**< Useful on object with "repeat events" enabled, where mouse and touch up and down events ARE NOT repeated to objects and these objects are not auto-grabbed @since 1.2 */
} Evas_Object_Pointer_Mode; /**< How the mouse pointer should be handled by Evas */

/**
 * @brief Evas smart objects' "smart callback" function signature
 * @since_tizen 2.3
 * @ingroup Evas_Smart_Object_Group
 */
typedef void      (*Evas_Smart_Cb)(void *data, Evas_Object *obj, void *event_info);

/**
 * @brief Evas event callback function signature
 * @since_tizen 2.3
 * @ingroup Evas_Canvas_Events
 */
typedef void      (*Evas_Event_Cb)(void *data, Evas *e, void *event_info);

/**
 * @brief Evas event callback Post function signature
 * @since_tizen 2.3
 * @ingroup Evas_Canvas_Events
 */
typedef Eina_Bool (*Evas_Object_Event_Post_Cb)(void *data, Evas *e);

/**
 * @brief Evas object event callback function signature
 * @since_tizen 2.3
 * @ingroup Evas_Object_Group_Events
 */
typedef void      (*Evas_Object_Event_Cb)(void *data, Evas *e, Evas_Object *obj, void *event_info);

/**
 * @brief Evas Async events put function signature
 * @since_tizen 2.3
 * @ingroup Evas_Top_Group
 */
typedef void      (*Evas_Async_Events_Put_Cb)(void *target, Evas_Callback_Type type, void *event_info);

/**
 * @}
 */

/**
 * @internal
 * @defgroup Evas_Top_Group Top Level Functions
   @ingroup Evas
 *
 * @brief  This group provides functions that affect Evas as a whole.
 * @{
 */

/**
 * @brief    Initializes Evas.
 *
 * @details  This function initializes Evas and increments a counter of the
 *           number of calls to it. It returns the new counter value. 
 *
 * @since_tizen 2.3
 *
 * @remarks  Most EFL users do not use this function directly, because
 *           they do not access Evas directly by themselves. Instead, they
 *           use higher level helpers, like @c ecore_evas_init().
 *           See http://docs.enlightenment.org/auto/ecore/
 *
 * @return  The init counter value
 *
 * @see     evas_shutdown()
 *
 * @ingroup Evas_Top_Group
 */
EAPI int               evas_init(void);

/**
 * @brief   Shuts down Evas.
 *
 * @details This function finalizes Evas, decrementing the counter of the
 *          number of calls to the function evas_init(). This new value for the
 *          counter is returned.
 *
 * @since_tizen 2.3
 *
 * @remarks If you are the sole user of Evas, you can use evas_init() to
 *          check if it is being properly shut down by expecting a return value
 *          of @c 0.
 *
 * @return  The Evas init counter value
 *
 * @see evas_init()
 *
 * @ingroup Evas_Top_Group
 */
EAPI int               evas_shutdown(void);

/**
 * @brief   Gets the allocation errors that have occurred during the execution of the prior function.
 *
 * @details This function returns any memory allocation errors that occurred during the execution
 *          and the kind of errors. Valid return values are @c EVAS_ALLOC_ERROR_NONE,
 *          @c EVAS_ALLOC_ERROR_FATAL, and @c EVAS_ALLOC_ERROR_RECOVERED.
 *
 * @since_tizen 2.3
 *
 * @remarks @c EVAS_ALLOC_ERROR_NONE means that no errors occurred at all and the function
 *          worked as expected.
 *
 * @remarks @c EVAS_ALLOC_ERROR_FATAL means the function is completely unable to perform
 *          its job and exited as cleanly as possible. You should consider this as a
 *          sign of very low memory and should try and safely recover from the prior function 
 *          failure. You can also try to free up memory elsewhere and try again after memory is freed.
 *
 * @remarks @c EVAS_ALLOC_ERROR_RECOVERED means that an allocation error occurred, but Evas
 *          recovered from it by finding memory of its own that it has allocated and
 *          freeing what it sees as not really usefully allocated memory. What is freed
 *          may vary. Evas may reduce the resolution of images, free cached images or
 *          fonts, throw out pre-rendered data, reduce the complexity of change lists,
 *          and so on. Evas and the program functions as per normal after this, but this
 *          is a sign of low memory, and it is suggested that your program try and
 *          identify memory it does not need, and free it.
 *
 * @remarks The following is an example:
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
 * @return  The allocation error flag
 *
 * @ingroup Evas_Top_Group
 */
EAPI Evas_Alloc_Error  evas_alloc_error(void);

/**
 * @brief   Gets the Evas internal asynchronous events read file descriptor.
 *
 * @details This function returns the read file descriptor of the
 *          asynchronous events of the canvas. Other mainloops,
 *          apart from ecore, may make use of it.
 *
 * @since_tizen 2.3
 *
 * @remarks The Evas asynchronous events are meant to be dealt with internally,
 *          i.e., when building stuff to be glued together into the EFL
 *          infrastructure - a module, for example. The context which demands
 *          its use is when calculations need to be done out of the main
 *          thread, asynchronously, and some action must be performed after
 *          that.
 *
 * @remarks An example of the actual use of this API is for image asynchronous
 *          preload inside Evas. If the canvas is instantiated through
 *          ecore-evas usage, ecore itself takes care of calling those
 *          event processing.
 *
 * @return  The asynchronous events read file descriptor of the canvas
 *
 * @ingroup Evas_Top_Group
 */
EAPI int               evas_async_events_fd_get(void) EINA_WARN_UNUSED_RESULT;

/**
 * @brief   Triggers the processing of all events waiting on the file
 *          descriptor returned by evas_async_events_fd_get().
 *
 * @details All asynchronous events queued up by evas_async_events_put() are
 *          processed here. More precisely, the callback functions, informed
 *          together with other event parameters, when queued, get called (with
 *          those parameters), in that order.
 *
 * @since_tizen 2.3
 *
 * @return  The number of events processed
 *
 * @ingroup Evas_Top_Group
 */
EAPI int               evas_async_events_process(void);

/**
 * @brief   Inserts asynchronous events on the canvas.
 *
 * @details This is the way, for a routine running outside evas' main thread,
 *          to report an asynchronous event. A callback function is informed,
 *          whose call is to happen after evas_async_events_process() is
 *          called. 
 *
 * @since_tizen 2.3
 *
 * @param[in]   target      The target to be affected by the events
 * @param[in]   type        The type of callback function
 * @param[in]   event_info  The information about the event
 * @param[in]   func        The callback function pointer
 * @return  #EINA_TRUE if the events are inserted successfully, \n
 *          otherwise #EINA_FALSE on failure
 *
 * @ingroup Evas_Top_Group
 */
EAPI Eina_Bool         evas_async_events_put(const void *target, Evas_Callback_Type type, void *event_info, Evas_Async_Events_Put_Cb func) EINA_ARG_NONNULL(1, 4);

/**
 * @}
 */

/**
 * @defgroup Evas_Canvas Canvas Functions
 * @ingroup Evas
 *
 * @brief    This group provides low level Evas canvas functions. Sub-groups 
 *           present more high level ones, though.
 *
 * @remarks  Most of these functions deal with low level Evas actions, like:
 *           @li creating or destroying raw canvases, not bound to any displaying engine
 *           @li telling a canvas that it got focused (in a windowing context, for example)
 *           @li telling a canvas that a region should not be calculated anymore in rendering
 *           @li telling a canvas to render its contents, immediately
 *
 * @remarks  You mostly use Evas with the @c Ecore_Evas wrapper, which
 *           deals with all the above mentioned issues automatically. Thus, you 
 *           need this section only if you are building low level stuff.
 *
 * @remarks  The groups present you functions that deal with the canvas
 *           directly, too, and not yet with its @b objects. They are the
 *           functions you need to use at a minimum to get a working canvas.
 *
 * @{
 */

/**
 * @brief    Creates a new empty evas.
 *
 * @since_tizen 2.3
 *
 * @remarks  This function should only fail if the memory allocation fails.
 *
 * @remarks  This function is a very low level function. Instead of using it
 *           directly, consider using the high level functions in
 *           Ecore_Evas such as @c ecore_evas_new(). See
 *           http://docs.enlightenment.org/auto/ecore/
 *
 * @remarks  It is recommended that you call evas_init() before creating new canvas.
 *
 * @return   A new uninitialised Evas canvas on success, \n 
 *           otherwise @c NULL on failure
 *
 * @pre      Note that before you can use evas, you have to:
 *           @li Set its render method with @ref evas_output_method_set.
 *           @li Set its viewport size with @ref evas_output_viewport_set.
 *           @li Set its size of the canvas with @ref evas_output_size_set.
 *           @li Ensure that the render engine is given the correct settings
 *               with @ref evas_engine_info_set.
 *
 * @ingroup Evas_Canvas
 */
EAPI Evas             *evas_new(void) EINA_WARN_UNUSED_RESULT EINA_MALLOC;

/**
 * @brief    Frees the given evas and any objects created on it.
 *
 * @since_tizen 2.3
 *
 * @remarks  Any objects with 'free' callbacks have those callbacks called
 *           in this function.
 *
 * @param[in]    e  The given evas
 *
 * @ingroup  Evas_Canvas
 */
EAPI void              evas_free(Evas *e)  EINA_ARG_NONNULL(1);

/**
 * @brief    Informs evas that it has got focus.
 *
 * @since_tizen 2.3
 *
 * @param[in]    e  The evas to inform
 * @ingroup  Evas_Canvas
 */
EAPI void              evas_focus_in(Evas *e);

/**
 * @brief   Informs the evas that it has lost focus.
 *
 * @since_tizen 2.3
 *
 * @param[in]   e  The evas to inform
 * @ingroup Evas_Canvas
 */
EAPI void              evas_focus_out(Evas *e);

/**
 * @brief   Gets the focus state of the given evas.
 *
 * @since_tizen 2.3
 *
 * @param[in]   e  The evas to query information
 * @return  #EINA_TRUE if it is focused, otherwise #EINA_FALSE.
 * @ingroup Evas_Canvas
 */
EAPI Eina_Bool         evas_focus_state_get(const Evas *e);

/**
 * @brief    Pushes the nochange flag up @c 1.
 *
 * @since_tizen 2.3
 *
 * @remarks  This tells evas that while the nochange flag is greater than 0, do not
 *           mark objects as "changed" when making changes.
 *
 * @param[in]    e  The evas to changes information
 * @ingroup  Evas_Canvas
 */
EAPI void              evas_nochange_push(Evas *e);

/**
 * @brief   Pops the nochange flag down @c 1.
 *
 * @since_tizen 2.3
 *
 * @remarks This tells evas that while the nochange flag is greater than 0, do not
 *          mark objects as "changed" when making changes.
 *
 * @param[in]   e  The evas to change information
 * @ingroup Evas_Canvas
 */
EAPI void              evas_nochange_pop(Evas *e);

/**
 * @brief   Attaches a specific pointer to evas for fetching later.
 *
 * @since_tizen 2.3
 *
 * @param[in]   e       The canvas to attach the pointer to
 * @param[in]   data    The pointer to attach
 * @ingroup Evas_Canvas
 */
EAPI void              evas_data_attach_set(Evas *e, void *data) EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the pointer attached by evas_data_attach_set().
 *
 * @since_tizen 2.3
 *
 * @param[in]   e  The canvas to attach the pointer to
 * @return  The pointer attached
 * @ingroup Evas_Canvas
 */
EAPI void             *evas_data_attach_get(const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief   Adds a damage rectangle.
 *
 * @details You can use this function to inform evas that a part of the
 *          canvas has to be repainted.
 *
 * @since_tizen 2.3
 *
 * @remarks All newly created Evas rectangles get the default color values 
 *          of 255 255 255 255 (opaque white). 
 *
 * @param[in]   e  The given canvas pointer
 * @param[in]   x  The rectangle's left position
 * @param[in]   y  The rectangle's top position
 * @param[in]   w  The rectangle's width
 * @param[in]   h  The rectangle's height
 *
 * @ingroup Evas_Canvas
 */
EAPI void              evas_damage_rectangle_add(Evas *e, int x, int y, int w, int h) EINA_ARG_NONNULL(1);

/**
 * @brief   Adds an "obscured region" to an Evas canvas.
 *
 * @since_tizen 2.3
 *
 * @remarks You can use this function to inform an Evas canvas that a part
 *          of it <b>must not</b> be repainted. The region must be
 *          rectangular and its coordinates inside the canvas viewport are
 *          passed in the call. After this call, the region specified do not
 *          participate in any form in Evas' calculations and actions during
 *          its rendering updates, having its displaying content frozen as it
 *          is just after this function is executed.
 *
 * @remarks This is called "obscured region" because the most common use case for
 *          this rendering (partial) freeze is something else (most probably
 *          other canvas) being on top of the specified rectangular region,
 *          thus shading it completely from the user's final scene in a
 *          display. To avoid unnecessary processing, one should indicate to the
 *          obscured canvas not to bother about the non-important area.
 *
 * @remarks The majority of users do not have to worry about this function, as
 *          they use just one canvas in their applications, with
 *          nothing inset or on top of it in any form.
 *
 * @remarks To make this region one that @b has to be repainted again, call the
 *          function evas_obscured_clear().
 *
 * @remarks This is a <b>very low level function</b>, which you may not use.
 *
 * @remarks This function does @b not flag the canvas as having its state
 *          changed. If you want to re-render it afterwards expecting new
 *          contents, you have to add "damage" regions yourself (see
 *          evas_damage_rectangle_add()).
 *
 * @param[in]   e  The given canvas pointer
 * @param[in]   x  The rectangle's top left corner's horizontal coordinate
 * @param[in]   y  The rectangle's top left corner's vertical coordinate
 * @param[in]   w  The rectangle's width
 * @param[in]   h  The rectangle's height
 *
 * @see evas_obscured_clear()
 * @see evas_render_updates()
 *
 * @ingroup Evas_Canvas
 */
EAPI void              evas_obscured_rectangle_add(Evas *e, int x, int y, int w, int h) EINA_ARG_NONNULL(1);

/**
 * @brief   Removes all "obscured regions" from an Evas canvas.
 *
 * @details This function removes all the rectangles from the obscured regions
 *          list of the canvas @a e. It takes obscured areas added with
 *          evas_obscured_rectangle_add() and make them again a regions that @b
 *          have to be repainted on rendering updates.
 *
 * @since_tizen 2.3
 *
 * @remarks This is a <b>very low level function</b>, which you may not use.
 *
 * @remarks This function does @b not flag the canvas as having its state
 *          changed. If you want to re-render it afterwards expecting new
 *          contents, you have to add "damage" regions yourself (see
 *          evas_damage_rectangle_add()).
 *
 * @param[in]   e  The given canvas pointer
 *
 * @see evas_obscured_rectangle_add() for an example
 * @see evas_render_updates()
 *
 * @ingroup Evas_Canvas
 */
EAPI void              evas_obscured_clear(Evas *e) EINA_ARG_NONNULL(1);

/**
 * @brief   Forces immediate renderization of the given Evas canvas.
 *
 * @details This function forces an immediate renderization update of the given
 *          canvas @a e.
 *
 * @since_tizen 2.3
 *
 * @remarks This is a <b>very low level function</b>, which you may not use. 
 *          You would use it, for example, to grab an Evas' canvas update regions 
 *          and paint them back, using the canvas' pixmap, on a displaying system 
 *          working below Evas.
 *
 * @remarks Evas is a stateful canvas. If no operations changing its
 *          state took place since the last rendering action, you do not see any
 *          changes and this call becomes a no-op.
 *
 * @param[in]   e  The given canvas pointer
 * @return  A newly allocated list of updated rectangles of the canvas (@c Eina_Rectangle structs) \n 
 *          Free this list with evas_render_updates_free().
 * 
 * @ingroup Evas_Canvas
 */
EAPI Eina_List        *evas_render_updates(Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief   Frees the rectangles returned by evas_render_updates().
 *
 * @details This function removes the region from the render updates list.
 *          The region does not get render updated anymore.
 *
 * @since_tizen 2.3
 *
 * @param[in]	updates	The list of updated rectangles of the canvas
 *
 * @see evas_render_updates() for an example
 *
 * @ingroup Evas_Canvas
 */
EAPI void              evas_render_updates_free(Eina_List *updates);

/**
 * @brief   Forces rendering of the given canvas.
 *
 * @since_tizen 2.3
 *
 * @param[in]   e  The given canvas pointer
 *
 * @ingroup Evas_Canvas
 */
EAPI void              evas_render(Evas *e) EINA_ARG_NONNULL(1);

/**
 * @brief   Updates the canvas internal objects but does not trigger immediate rendering.
 *
 * @details This function updates the canvas internal objects not triggering
 *          rendering. To force rendering, use evas_render().
 *
 * @since_tizen 2.3
 *
 * @param[in]   e  The given canvas pointer
 *
 * @see evas_render
 *
 * @ingroup Evas_Canvas
 */
EAPI void              evas_norender(Evas *e) EINA_ARG_NONNULL(1);

/**
 * @brief   Makes the canvas discard internally cached data used for rendering.
 *
 * @details This function flushes the arrays of delete, active and render objects.
 *          The other things it may discard include shared memory segments,
 *          temporary scratch buffers, and cached data to avoid re-compute of that data.
 *
 * @since_tizen 2.3
 *
 * @param[in]   e  The given canvas pointer
 *
 * @ingroup Evas_Canvas
 */
EAPI void              evas_render_idle_flush(Evas *e) EINA_ARG_NONNULL(1);

/**
 * @brief   Makes the canvas discard as much data as possible used by the engine at runtime.
 *
 * @details This function unloads images, deletes textures and much more, where
 *          possible. You may also want to call evas_render_idle_flush() immediately
 *          prior to this to perhaps discard a little more, though evas_render_dump()
 *          should implicitly delete most of what evas_render_idle_flush() might
 *          discard too.
 *
 * @since_tizen 2.3
 *
 * @param[in]   e  The given canvas pointer
 *
 * @ingroup Evas_Canvas
 */
EAPI void              evas_render_dump(Evas *e) EINA_ARG_NONNULL(1);

/**
 * @}
 */

/**
 * @defgroup Evas_Output_Method Render Engine Functions
 * @ingroup Evas_Canvas
 *
 * @brief   This goup provides functions that are used to set the render engine for a given
 *          function, and then get that engine working.
 *
 * @remarks The following code snippet shows how they can be used to
 *          initialise an evas that uses the X11 software engine:
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
 * @{
 */

/**
 * @brief   Looks up a numeric ID from a string name of a rendering engine.
 *
 * @details This function looks up a numeric return value for the named engine
 *          in the string @a name. This is a normal C string, NULL byte
 *          terminated. The name is case sensitive. If the rendering engine is
 *          available, a numeric ID for that engine is returned that is not
 *          @c 0. If the engine is not available, @c 0 is returned, indicating an
 *          invalid engine.
 *
 * @since_tizen 2.3
 *
 * @remarks You should NEVER rely on the numeric ID of an engine unless it is returned 
 *          by this function. You should NOT write programs written accessing render 
 *          method ID's directly, without first obtaining it from this function.
 *
 * @remarks It is mandatory that you call evas_init() before
 *          looking up the render method.
 *
 * @remarks The following is an example.
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
 *
 * @param[in]   name  The name string of an engine
 * @return  A numeric (opaque) ID for the rendering engine
 *
 * @ingroup Evas_Output_Method
 */
EAPI int               evas_render_method_lookup(const char *name) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief   Lists all the rendering engines compiled into the copy of the Evas library.
 *
 * @since_tizen 2.3
 *
 *
 * @remarks Calling this returns a handle (pointer) to an Evas linked
 *          list. Each node in the linked list has the data pointer be a
 *          (char *) pointer to the name string of the rendering engine
 *          available. The strings should never be modified, neither should the
 *          list be modified. This list should be cleaned up as soon as the
 *          program no longer needs it using evas_render_method_list_free(). If
 *          no engines are available from Evas, @c NULL is returned.
 *
 * @remarks The following is an example:
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
 *
 * @return  A linked list whose data members are C strings of engine names
 *
 * @ingroup Evas_Output_Method
 */
EAPI Eina_List        *evas_render_method_list(void) EINA_WARN_UNUSED_RESULT;

/**
 * @brief   Frees the list of engine names.
 *
 * @since_tizen 2.3
 *
 * @remarks When this function is called it frees the engine list passed in
 *          as @a list. The list should only be a list of engines generated by
 *          calling evas_render_method_list(). If @a list is NULL, nothing happens.
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
 *
 * @param[in]   list  The Eina_List base pointer for the engine list to be freed
 * @ingroup Evas_Output_Method
 */
EAPI void              evas_render_method_list_free(Eina_List *list);

/**
 * @brief   Sets the output engine for the given evas.
 *
 * @since_tizen 2.3
 *
 * @remarks Once the output engine for an evas is set, any attempt to change it
 *          is ignored. The value for @a render_method can be found using
 *          @ref evas_render_method_lookup.
 *
 * @remarks It is mandatory that you call evas_init() before setting the output method.
 *
 * @param[in]   e               The given evas
 * @param[in]   render_method   The numeric engine value to use
 *
 * @ingroup Evas_Output_Method
 */
EAPI void              evas_output_method_set(Evas *e, int render_method) EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the number of the output engines used for the given evas.
 *
 * @since_tizen 2.3
 *
 * @param[in]   e  The given evas
 * @return  The ID number of the output engine being used \n 
 *          @c 0 is returned if there is an error.
 *
 * @ingroup Evas_Output_Method
 */
EAPI int               evas_output_method_get(const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the current render engine info struct from the given evas.
 *
 * @since_tizen 2.3
 *
 * @remarks The returned structure is publicly modifiable. The contents are
 *          valid until either @ref evas_engine_info_set or @ref evas_render
 *          are called.
 *
 * @remarks You do not have to free this structure.
 *
 * @param[in]   e  The given evas
 * @return  A pointer to the Engine Info structure \n
 *          @c NULL is returned if an engine has not yet been assigned.
 *
 * @ingroup Evas_Output_Method
 */
EAPI Evas_Engine_Info *evas_engine_info_get(const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief   Applies the engine settings for the given evas from the given @c
 *          Evas_Engine_Info structure.
 *
 @ @since_tizen 2.3
 *
 * @remarks To get the Evas_Engine_Info structure to use, call @ref
 *          evas_engine_info_get. Do not try to obtain a pointer to an
 *          @c Evas_Engine_Info structure in any other way.
 *
 * @remarks You need to call this function at least once before you can
 *          create objects on an evas or render that evas. Some engines allow
 *          their settings to be changed more than once.
 *
 * @remarks Once called, the @a info pointer should be considered invalid.
 *
 * @param[in]   e       The pointer to the Evas canvas
 * @param[in]   info    The pointer to the Engine Info to use
 * @return  #EINA_TRUE if the engine setting is applied successfully, \n
 *          otherwise #EINA_FALSE if an error occurred
 * @ingroup Evas_Output_Method
 */
EAPI Eina_Bool         evas_engine_info_set(Evas *e, Evas_Engine_Info *info) EINA_ARG_NONNULL(1);

/**
 * @}
 */

/**
 * @defgroup Evas_Output_Size Output and Viewport Resizing Functions
 * @ingroup  Evas_Canvas
 *
 * @brief    This group provides functions that set and retrieve the output and viewport size of an evas.
 *
 * @{
 */

/**
 * @brief   Sets the output size of the render engine of the given evas.
 *
 * @since_tizen 2.3
 *
 * @remarks The evas renders to a rectangle of the given size once this
 *          function is called. The output size is independent of the viewport
 *          size. The viewport is stretched to fill the given rectangle.
 *
 * @remarks The units used for @a w and @a h depend on the engine used by the evas.
 *
 * @param[in]   e  The given evas
 * @param[in]   w  The width in output units, usually pixels
 * @param[in]   h  The height in output units, usually pixels
 *
 * @ingroup Evas_Output_Size
 */
EAPI void              evas_output_size_set(Evas *e, int w, int h) EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the output size of the render engine of the given evas.
 *
 * @since_tizen 2.3
 *
 * @remarks The output size is in the output units for the engine.
 *
 * @remarks If either @a w or @a h is @c NULL, then it is ignored. If @a e is
 *          invalid, the returned results are undefined.
 *
 * @param[in]   e  The given evas
 * @param[out]   w  The pointer to an integer to store the width in
 * @param[out]   h  The pointer to an integer to store the height in
 *
 * @ingroup Evas_Output_Size
 */
EAPI void              evas_output_size_get(const Evas *e, int *w, int *h) EINA_ARG_NONNULL(1);

/**
 * @brief   Sets the output viewport of the given evas in evas units.
 *
 * @since_tizen 2.3
 *
 * @remarks The output viewport is the area of the evas that is visible to	the viewer.  
 *          The viewport is stretched to fit the output target of the evas when 
 *          rendering is performed.
 *
 * @remarks The coordinate values do not have to map 1-to-1 with the output
 *          target. However, it is generally advised that it is done for ease of use.
 *
 * @param[in]   e  The given evas
 * @param[in]   x  The top-left corner x value of the viewport
 * @param[in]   y  The top-left corner y value of the viewport
 * @param[in]   w  The width of the viewport \n 
 *             Must be greater than @c 0.
 * @param[in]   h  The height of the viewport \n 
 *             Must be greater than @c 0.
 * @ingroup Evas_Output_Size
 */
EAPI void              evas_output_viewport_set(Evas *e, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h) EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the render engine's output viewport co-ordinates in canvas units.
 *
 * @since_tizen 2.3
 *
 * @remarks Calling this function writes the current canvas output viewport
 *          size and location values into the variables pointed to by @a x, @a y, 
 *          @a w and @a h. On success, the variables have the output
 *          location and size values written to them in canvas units. Any of @a x,
 *          @a y, @a w or @a h that are @c NULL is not written to. If @a e
 *          is invalid, the results are undefined.
 *
 * @remarks The following is an example.
 * @code
 * extern Evas *evas;
 * Evas_Coord x, y, width, height;
 *
 * evas_output_viewport_get(evas, &x, &y, &w, &h);
 * @endcode
 *
 * @param[in]   e  The pointer to the Evas Canvas
 * @param[out]   x  The pointer to the x variable to be filled in
 * @param[out]   y  The pointer to the y variable to be filled in
 * @param[out]   w  The pointer to the width variable to be filled in
 * @param[out]   h  The pointer to the height variable to be filled in
 * @ingroup Evas_Output_Size 
 */
EAPI void              evas_output_viewport_get(const Evas *e, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h) EINA_ARG_NONNULL(1);

/**
 * @brief   Sets the output framespace size of the render engine of the given evas.
 * @since   1.1
 *
 * @since_tizen 2.3
 *
 * @remarks The framespace size is used in the Wayland engines to denote space where
 *          the output is not drawn. This is mainly used in ecore_evas to draw borders.
 *
 * @remarks The units used for @a w and @a h depend on the engine used by the evas.
 *
 * @param[in]   e  The given evas
 * @param[in]   x  The left coordinate in output units, usually pixels
 * @param[in]   y  The top coordinate in output units, usually pixels
 * @param[in]   w  The width in output units, usually pixels
 * @param[in]   h  The height in output units, usually pixels
 * @ingroup Evas_Output_Size
 */
EAPI void              evas_output_framespace_set(Evas *e, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h);

/**
 * @brief   Gets the render engine's output framespace co-ordinates in canvas units.
 * @since   1.1
 *
 * @since_tizen 2.3
 *
 * @param[in]   e  The pointer to the Evas Canvas
 * @param[out]   x  The pointer to the x variable to be filled in
 * @param[out]   y  The pointer to the y variable to be filled in
 * @param[out]   w  The pointer to the width variable to be filled in
 * @param[out]   h  The pointer to the height variable to be filled in
 * @ingroup Evas_Output_Size
 */
EAPI void              evas_output_framespace_get(const Evas *e, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h);

/**
 * @}
 */

/**
 * @defgroup Evas_Coord_Mapping_Group Coordinate Mapping Functions
 * @ingroup Evas_Canvas
 *
 * @brief   This group provides functions that are used to map coordinates from the canvas to the
 *          screen or the screen to the canvas.
 *
 * @{
 */

/**
 * @brief   Converts or scales an ouput screen co-ordinate into canvas co-ordinates.
 *
 * @details This function takes in a horizontal co-ordinate as the @a x
 *          parameter and converts it into canvas units, accounting for output
 *          size, viewport size and location, returning it as the function
 *          return value. If @a e is invalid, the results are undefined.
 *
 * @since_tizen 2.3
 *
 * @remarks The following is an example:
 * @code
 * extern Evas *evas;
 * extern int screen_x;
 * Evas_Coord canvas_x;
 *
 * canvas_x = evas_coord_screen_x_to_world(evas, screen_x);
 * @endcode
 *
 * @param[in]   e  The pointer to the Evas Canvas
 * @param[in]   x  The screen or output x co-ordinate
 * @return  The screen co-ordinate translated to canvas unit co-ordinates
 * @ingroup Evas_Coord_Mapping_Group
 */
EAPI Evas_Coord        evas_coord_screen_x_to_world(const Evas *e, int x) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief   Converts or scales an ouput screen co-ordinate into canvas co-ordinates.
 *
 * @details This function takes in a vertical co-ordinate as the @a y parameter
 *          and converts it into canvas units, accounting for output size,
 *          viewport size and location, returning it as the function return
 *          value. If @a e is invalid, the results are undefined.
 *
 * @since_tizen 2.3
 *
 * @remarks The following is an example:
 * @code
 * extern Evas *evas;
 * extern int screen_y;
 * Evas_Coord canvas_y;
 *
 * canvas_y = evas_coord_screen_y_to_world(evas, screen_y);
 * @endcode
 *
 * @param[in]   e  The pointer to the Evas Canvas
 * @param[in]   y  The screen or output y co-ordinate
 * @return  The screen co-ordinate translated to canvas unit co-ordinates
 * @ingroup Evas_Coord_Mapping_Group
 */
EAPI Evas_Coord        evas_coord_screen_y_to_world(const Evas *e, int y) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief   Converts or scales a canvas co-ordinate into output screen co-ordinates.
 *
 * @details This function takes in a horizontal co-ordinate as the @a x
 *          parameter and converts it into output units, accounting for output
 *          size, viewport size and location, returning it as the function
 *          return value. If @a e is invalid, the results are undefined.
 *
 * @since_tizen 2.3
 *
 * @remarks The following is an example:
 * @code
 * extern Evas *evas;
 * int screen_x;
 * extern Evas_Coord canvas_x;
 *
 * screen_x = evas_coord_world_x_to_screen(evas, canvas_x);
 * @endcode
 *
 * @param[in]   e  The pointer to the Evas canvas
 * @param[in]   x  The canvas x co-ordinate
 * @return  The output or screen co-ordinate translated to output co-ordinates
 * @ingroup Evas_Coord_Mapping_Group
 */
EAPI int               evas_coord_world_x_to_screen(const Evas *e, Evas_Coord x) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief   Converts or scales a canvas co-ordinate into output screen co-ordinates.
 *
 * @details This function takes in a vertical co-ordinate as the @a x parameter
 *          and converts it into output units, accounting for output size,
 *          viewport size and location, returning it as the function return
 *          value. If @a e is invalid, the results are undefined.
 *
 * @since_tizen 2.3
 *
 * @remarks The following is an example:
 * @code
 * extern Evas *evas;
 * int screen_y;
 * extern Evas_Coord canvas_y;
 *
 * screen_y = evas_coord_world_y_to_screen(evas, canvas_y);
 * @endcode
 *
 * @param[in]   e  The pointer to the Evas Canvas
 * @param[in]   y  The canvas y co-ordinate
 * @return  The output or screen co-ordinate translated to output co-ordinates
 * @ingroup Evas_Coord_Mapping_Group
 */
EAPI int               evas_coord_world_y_to_screen(const Evas *e, Evas_Coord y) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @}
 */

/**
 * @defgroup Evas_Pointer_Group Pointer (Mouse) Functions
 * @ingroup Evas_Canvas
 *
 * @brief   This group provides functions that deal with the status of the pointer (mouse cursor).
 *
 * @{
 */

/**
 * @brief   Gets the current known pointer co-ordinates.
 *
 * @details This function returns the current known screen or output co-ordinates
 *          of the mouse pointer and sets the contents of the integers pointed
 *          to by @a x and @a y to contain these co-ordinates. If @a e is not a
 *          valid canvas the results of this function are undefined.
 *
 * @since_tizen 2.3
 *
 * @remarks The following is an example:
 * @code
 * extern Evas *evas;
 * int mouse_x, mouse_y;
 *
 * evas_pointer_output_xy_get(evas, &mouse_x, &mouse_y);
 * printf("Mouse is at screen position %i, %i\n", mouse_x, mouse_y);
 * @endcode
 *
 * @param[in]   e  The pointer to the Evas Canvas
 * @param[out]   x  The pointer to an integer to be filled in
 * @param[out]   y  The pointer to an integer to be filled in
 * @ingroup Evas_Pointer_Group
 */
EAPI void              evas_pointer_output_xy_get(const Evas *e, int *x, int *y) EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the current known pointer co-ordinates.
 *
 * @details This function returns the current known canvas unit co-ordinates of
 *          the mouse pointer and sets the contents of the Evas_Coords pointed
 *          to by @a x and @a y to contain these co-ordinates. If @a e is not a
 *          valid canvas the results of this function are undefined.
 *
 * @since_tizen 2.3
 *
 * @remarks The following is an example:
 * @code
 * extern Evas *evas;
 * Evas_Coord mouse_x, mouse_y;
 *
 * evas_pointer_output_xy_get(evas, &mouse_x, &mouse_y);
 * printf("Mouse is at canvas position %f, %f\n", mouse_x, mouse_y);
 * @endcode
 *
 * @param[in]   e  The pointer to the Evas Canvas
 * @param[out]   x  The pointer to a Evas_Coord to be filled in
 * @param[out]   y  The pointer to a Evas_Coord to be filled in
 * @ingroup Evas_Pointer_Group
 */
EAPI void              evas_pointer_canvas_xy_get(const Evas *e, Evas_Coord *x, Evas_Coord *y) EINA_ARG_NONNULL(1);

/**
 * @brief   Gets a bitmask with the mouse buttons currently pressed, set to @c 1.
 *
 * @since_tizen 2.3
 *
 * @remarks This function returns a 32-bit integer with the
 *          appropriate bits set to @c 1 that correspond to a mouse button being
 *          depressed. This limits Evas to mouse devices with a maximum of 32
 *          buttons, but that is generally in excess of any host system's
 *          pointing device abilities.
 *
 * @remarks The least significant bit corresponds to the first mouse button
 *          (button 1) and the most significant bit corresponds to the last
 *          mouse button (button 32).
 *
 * @remarks If @a e is not a valid canvas, the return value is undefined.
 *
 * @remarks The following is an example:
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
 *
 * @param[in]   e  The pointer to the Evas canvas
 * @return  A bitmask of the currently depressed buttons on the canvas
 * @ingroup Evas_Pointer_Group
 */
EAPI int               evas_pointer_button_down_mask_get(const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief   Checks whether the mouse pointer is logically inside the canvas.
 *
 * @since_tizen 2.3
 *
 * @remarks When this function is called it returns a value of either @c 0 or
 *          @c 1.
 *
 * @remarks A return value of @c 1 indicates the mouse is logically inside the
 *          canvas, and @c 0 implies it is logically outside the canvas.
 *
 * @remarks A canvas begins with the mouse being assumed outside (0).
 *
 * @remarks If @a e is not a valid canvas, the return value is undefined.
 *
 * @remarks The following is an example:
 * @code
 * extern Evas *evas;
 *
 * if (evas_pointer_inside_get(evas)) printf("Mouse is in!\n");
 * else printf("Mouse is out!\n");
 * @endcode
 *
 * @param[in]   e  The pointer to the Evas Canvas
 * @return  #EINA_TRUE if the mouse is inside the canvas, 
 *          otherwise #EINA_FALSE
 * @ingroup Evas_Pointer_Group
 */
EAPI Eina_Bool         evas_pointer_inside_get(const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

EAPI void              evas_sync(Evas *e) EINA_ARG_NONNULL(1);

/**
 * @}
 */

/**
 * @defgroup Evas_Canvas_Events Canvas Events
 * @ingroup Evas_Canvas
 *
 * @brief   This group provides functions relating to canvas events, which are mainly reports on
 *          its internal states changing such as an object getting focused, the rendering being updated,
 *          and so on.
 *
 * @{
 */

/**
 * @brief   Adds or registers a callback function to a given canvas event.
 *
 * @details This function adds a function callback to the canvas @a e when the
 *          event of type @a type occurs on it. The function pointer is @a func.
 *
 * @since_tizen 2.3
 *
 * @remarks In the event of a memory allocation error during the addition of
 *          the callback to the canvas, evas_alloc_error() should be used to
 *          determine the nature of the error, if any, and the program should
 *          sensibly try and recover.
 *
 * @remarks A callback function must have the ::Evas_Event_Cb prototype
 *          definition. The first parameter (@a data) in this definition has
 *          the same value passed to evas_event_callback_add() as the @a data
 *          parameter, at runtime. The second parameter @a e is the canvas
 *          pointer on which the event occurred. The third parameter @a
 *          event_info is a pointer to a data structure that may or may not be
 *          passed to the callback, depending on the event type that triggered
 *          the callback. This is so because some events do not carry extra
 *          context with them, but others do.
 *
 * @remarks The valid event types @a type to trigger the function are
 *          #EVAS_CALLBACK_RENDER_FLUSH_PRE, #EVAS_CALLBACK_RENDER_FLUSH_POST,
 *          #EVAS_CALLBACK_CANVAS_FOCUS_IN, #EVAS_CALLBACK_CANVAS_FOCUS_OUT,
 *          #EVAS_CALLBACK_CANVAS_OBJECT_FOCUS_IN and 
 *          #EVAS_CALLBACK_CANVAS_OBJECT_FOCUS_OUT. This determines the kind of
 *          event that triggers the callback to be called. Only the last
 *          two event types listed here provide useful event information
 *          data -- a pointer to the recently focused Evas object. For the
 *          others the @a event_info pointer is going to be @c NULL.
 *
 * @remarks The canvas flushes its rendering pipeline
 *          (#EVAS_CALLBACK_RENDER_FLUSH_PRE) whenever the @c _resize_cb
 *          routine takes place: it has to redraw that image at a different
 *          size. Also, the callback on an object being focused comes just
 *          after you focus it explicitly, on code.
 *
 * @remarks Be careful not to add the same callback multiple times, if
 *          that is not what you want, because Evas does not check if a callback
 *          existed before exactly as the one being registered (and thus, call
 *          it more than once on the event, in this case). This would make
 *          sense if you passed different functions and/or callback data, only.
 *
 * @param[in]   e     The canvas to attach a callback to
 * @param[in]   type  The type of event that triggers the callback
 * @param[in]   func  The (callback) function to be called when the event is triggered
 * @param[in]   data  The data pointer to be passed to @a func
 */
EAPI void  evas_event_callback_add(Evas *e, Evas_Callback_Type type, Evas_Event_Cb func, const void *data) EINA_ARG_NONNULL(1, 3);

/**
 * @brief  Adds or registers a callback function to a given canvas event with a
 *         non-default priority set. Except for the priority field, it is exactly the
 *         same as @ref evas_event_callback_add
 * @since  1.1
 *
 * @since_tizen 2.3
 *
 * @param[in]  e         The canvas to attach a callback to
 * @param[in]  type      The type of event that triggers the callback
 * @param[in]  priority  The priority of the callback, lower values called first
 * @param[in]  func      The (callback) function to be called when the event is triggered
 * @param[in]  data      The data pointer to be passed to @a func
 *
 * @see evas_event_callback_add
 */
EAPI void  evas_event_callback_priority_add(Evas *e, Evas_Callback_Type type, Evas_Callback_Priority priority, Evas_Event_Cb func, const void *data) EINA_ARG_NONNULL(1, 4);

/**
 * @brief   Deletes a callback function from the canvas.
 *
 * @details This function removes the most recently added callback from the
 *          canvas @a e which is triggered by the event type @a type and is
 *          calling the function @a func when triggered. If the removal is
 *          successful it also returns the data pointer that is passed to
 *          evas_event_callback_add() when the callback is added to the
 *          canvas. If not successful @c NULL is returned.
 *
 * @since_tizen 2.3
 *
 * @remarks The following is an example:
 * @code
 * extern Evas *e;
 * void *my_data;
 * void focus_in_callback(void *data, Evas *e, void *event_info);
 *
 * my_data = evas_event_callback_del(ebject, EVAS_CALLBACK_CANVAS_FOCUS_IN, focus_in_callback);
 * @endcode
 *
 * @param[in]   e     The canvas to remove a callback from
 * @param[in]   type  The type of event that is triggering the callback
 * @param[in]   func  The function that is to be called when the event is triggered
 * @return  The data pointer that is to be passed to the callback
 */
EAPI void *evas_event_callback_del(Evas *e, Evas_Callback_Type type, Evas_Event_Cb func) EINA_ARG_NONNULL(1, 3);

/**
 * @brief   Deletes (unregisters) a callback function registered to a given canvas event.
 *
 * @details This function removes <b>the first</b> added callback from the
 *          canvas @a e matching the event type @a type, the registered
 *          function pointer @a func and the callback data pointer @a data. If
 *          the removal is successful it also returns the data pointer that
 *          is passed to evas_event_callback_add() (that is the same as
 *          the parameter) when the callback(s) is (were) added to the
 *          canvas. If not successful @c NULL is returned. A common use
 *          would be to remove an exact match of a callback.
 *
 * @since_tizen 2.3
 *
 * @remarks For deleting canvas events callbacks filtering by just
 *          type and function pointer, use evas_event_callback_del().
 *
 * @param[in]   e     The Canvas to remove an event callback from
 * @param[in]   type  The type of event that triggers the callback
 * @param[in]   func  The function that is to be called when the event is triggered
 * @param[in]   data  The data pointer that is to be passed to the callback
 * @return  The data pointer that is to be passed to the callback
 */
EAPI void *evas_event_callback_del_full(Evas *e, Evas_Callback_Type type, Evas_Event_Cb func, const void *data) EINA_ARG_NONNULL(1, 3);

/**
 * @brief   Pushes a callback on the post-event callback stack.
 *
 * @since_tizen 2.3
 *
 * @remarks Evas has a stack of callbacks that get called after all the callbacks for
 *          an event have triggered (all the objects it triggers on and all the callbacks
 *          in each object triggered). When all these have been called, the stack is
 *          unwound from most recently to least recently pushed item and removed from the
 *          stack calling the callback set for it.
 *
 * @remarks This is intended for doing reverse logic-like processing, for example - when a
 *          child object that happens to get the event later is meant to be able to
 *          "steal" functions from a parent and when this stack is unwound, have its
 *          function called first, thus being able to set flags, or return @c 0 from the
 *          post-callback, that stops all other post-callbacks in the current stack from
 *          being called (thus basically allowing a child to take control, if the event
 *          callback prepares information ready for taking action, but the post callback
 *          actually does the action).
 *
 * @param[in]   e     The canvas to push the callback on
 * @param[in]   func  The function that to be called when the stack is unwound
 * @param[in]   data  The data pointer to be passed to the callback 
 */
EAPI void  evas_post_event_callback_push(Evas *e, Evas_Object_Event_Post_Cb func, const void *data);

/**
 * @brief   Removes a callback from the post-event callback stack.
 *
 * @details This removes a callback from the stack added with
 *          evas_post_event_callback_push(). The first instance of the function in
 *          the callback stack is removed from being executed when the stack is
 *          unwound. Further instances may still be run on unwind.
 *
 * @since_tizen 2.3
 *
 * @param[in]   e     The canvas to push the callback on
 * @param[in]   func  The function that to be called when the stack is unwound
 */
EAPI void  evas_post_event_callback_remove(Evas *e, Evas_Object_Event_Post_Cb func);

/**
 * @brief   Removes a callback from the post-event callback stack.
 *
 * @details This removes a callback from the stack added with
 *          evas_post_event_callback_push(). The first instance of the function and data
 *          in the callback stack is removed from being executed when the stack is
 *          unwound. Further instances may still be run on unwind.
 *
 * @since_tizen 2.3
 *
 * @param[in]   e     The canvas to push the callback on
 * @param[in]   func  The function that to be called when the stack is unwound
 * @param[in]   data  The data pointer to be passed to the callback
 */
EAPI void  evas_post_event_callback_remove_full(Evas *e, Evas_Object_Event_Post_Cb func, const void *data);

/**
 * @}
 */

/**
 * @defgroup Evas_Event_Freezing_Group Input Events Freezing Functions
 * @ingroup Evas_Canvas_Events
 *
 * @brief   This group provides functions that deal with the freezing of input event processing of
 *          an Evas canvas.
 *
 * @remarks There might be scenarios during a graphical user interface
 *          program's use when the developer wishes the users would not be able
 *          to deliver input events to this application. It may, for example,
 *          be the time for it to populate a view or to change some
 *          layout. Assuming proper behavior with user interaction during this
 *          exact time would be hard, as things are in a changing state. The
 *          programmer can then tell the canvas to ignore input events,
 *          bringing it back to normal behavior when he or she wants.
 *
 * @remarks Most of the time, the freezing events is used like this:
 * @code
 * evas_event_freeze(my_evas_canvas);
 * function_that_does_work_which_cant_be_interrupted_by_events();
 * evas_event_thaw(my_evas_canvas);
 * @endcode
 *
 * @{
 */

/**
 * @brief   Sets the default set of flags an event begins with.
 * @since   1.2
 *
 * @since_tizen 2.3
 *
 * @remarks Events in evas can have an event_flags member. This starts out with
 *          an initial value (no flags). This lets you set the default flags that
 *          an event begins with to be @a flags.
 *
 * @param[in]   e      The canvas to set the default event flags of
 * @param[in]   flags  The default flags to use
 */
EAPI void             evas_event_default_flags_set(Evas *e, Evas_Event_Flags flags) EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the default set of flags an event begins with.
 * @since   1.2
 *
 * @since_tizen 2.3
 *
 * @remarks This gets the default event flags with which the events are produced.
 *
 * @param[in]   e  The canvas to get the default event flags from
 * @return  The default event flags for that canvas
 *
 * @see evas_event_default_flags_set()
 */
EAPI Evas_Event_Flags evas_event_default_flags_get(const Evas *e) EINA_ARG_NONNULL(1);

/**
 * @brief   Freezes all input events processing.
 *
 * @since_tizen 2.3
 *
 * @remarks This function indicates to Evas that the canvas @a e is to have
 *          all input event processing frozen until a matching
 *          evas_event_thaw() function is called on the same canvas. All events
 *          of this kind during the freeze get @b discarded. Every freeze
 *          call must be matched by a thaw call in order to completely thaw out
 *          a canvas (i.e. these calls may be nested). The most common use is
 *          when you do not want the user to interact with your user interface
 *          when you are populating a view or changing the layout.
 *
 * @param[in]   e  The canvas to freeze input events processing on
 */
EAPI void             evas_event_freeze(Evas *e) EINA_ARG_NONNULL(1);

/**
 * @brief   Thaws a canvas out after freezing (for input events).
 *
 * @since_tizen 2.3
 *
 * @remarks This thaws out a canvas after a matching evas_event_freeze()
 *          call. If this call completely thaws out a canvas, i.e., there is no
 *          other unbalanced call to evas_event_freeze(), events start to
 *          be processed again, but any "missed" events are @b not be evaluated.
 *
 * @param[in]   e  The canvas to thaw out
 *
 * @see evas_event_freeze() for an example.
 */
EAPI void             evas_event_thaw(Evas *e) EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the freeze count on input events of a given canvas.
 *
 * @since_tizen 2.3
 *
 * @remarks This returns the number of times the canvas has been told to freeze
 *          input events. It is possible to call evas_event_freeze() multiple
 *          times, and these must be matched by evas_event_thaw() calls. This
 *          call allows the program to discover just how many times things have
 *          been frozen in case it may want to break out of a deep freeze state
 *          where the count is high.
 *
 * @remarks The following is an example:
 * @code
 * extern Evas *evas;
 *
 * while (evas_event_freeze_get(evas) > 0) evas_event_thaw(evas);
 * @endcode
 *
 * @param[in]   e  The canvas to fetch the freeze count from
 * @return The freeze count
 */
EAPI int              evas_event_freeze_get(const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief   Re-evaluates the state of objects and call callbacks after thaw of a canvas.
 *
 * @since_tizen 2.3
 *
 * @remarks This is normally called after evas_event_thaw() to re-evaluate mouse
 *          containment and other states and thus also call callbacks for mouse in and
 *          out on new objects if the state change demands it.
 *
 * @param[in]   e  The canvas to evaluate after a thaw
 */
EAPI void             evas_event_thaw_eval(Evas *e) EINA_ARG_NONNULL(1);

/**
 * @}
 */

/**
 * @internal
 * @defgroup Evas_Event_Feeding_Group Input Events Feeding Functions
 * @ingroup Evas_Canvas_Events
 *
 * @brief   This group provides functions to tell Evas that input events happened and should be processed.
 *
 * @remarks Most of the time these functions are @b not what you are looking for.
 *          These functions should only be used if you are not working with ecore evas(or
 *          another input handling system). If you are not using ecore evas, please
 *          consider using it, as in most situation it makes life a lot easier.
 *
 * @remarks As explained in @ref evas_main_intro_not_evas, Evas does not know how to poll
 *          for input events, so you should do it and then feed such
 *          events to the canvas to be processed. This is only required if
 *          operating Evas directly. Modules such as Ecore_Evas do that for you.
 *
 * @{
 */

/**
 * @internal
 * @since 1.8
 */
EAPI Evas_Device *evas_device_new(Evas *e);

/**
 * @internal
 * @since 1.8
 */
EAPI void evas_device_free(Evas_Device *dev);

/**
 * @internal
 * @since 1.8
 */
EAPI void evas_device_push(Evas *e, Evas_Device *dev);

/**
 * @internal
 * @since 1.8
 */
EAPI void evas_device_pop(Evas *e);

/**
 * @internal
 * @since 1.8
 */
EAPI const Eina_List *evas_device_list(Evas *e, const Evas_Device *dev);

/**
 * @internal
 * @since 1.8
 */
EAPI void evas_device_name_set(Evas_Device *dev, const char *name);

/**
 * @internal
 * @since 1.8
 */
EAPI const char *evas_device_name_get(const Evas_Device *dev);

/**
 * @internal
 * @since 1.8
 */
EAPI void evas_device_description_set(Evas_Device *dev, const char *desc);

/**
 * @internal
 * @since 1.8
 */
EAPI const char *evas_device_description_get(const Evas_Device *dev);

/**
 * @internal
 * @since 1.8
 */
EAPI void evas_device_parent_set(Evas_Device *dev, Evas_Device *parent);

/**
 * @internal
 * @since 1.8
 */
EAPI const Evas_Device *evas_device_parent_get(const Evas_Device *dev);

/**
 * @internal
 * @since 1.8
 */
EAPI void evas_device_class_set(Evas_Device *dev, Evas_Device_Class clas);

/**
 * @internal
 * @since 1.8
 */
EAPI Evas_Device_Class evas_device_class_get(const Evas_Device *dev);

/**
 * @internal
 * @since 1.8
 */
EAPI void evas_device_emulation_source_set(Evas_Device *dev, Evas_Device *src);

/**
 * @internal
 * @since 1.8
 */
EAPI const Evas_Device *evas_device_emulation_source_get(const Evas_Device *dev);

/**
 * @internal
 * @brief   Gets the number of mouse or multi presses currently active.
 * @since   1.2
 *
 * @since_tizen 2.3
 *
 * @param[in]   e  The given canvas pointer
 * @return  The number of presses, \n
 *          otherwise @c 0 if none active
 *
 */
EAPI int  evas_event_down_count_get(const Evas *e) EINA_ARG_NONNULL(1);

/**
 * @internal
 * @brief   Feeds the mouse down event for the given canvas @a e.
 *
 * @since_tizen 2.3
 *
 * @remarks This function sets some evas properties that is necessary when
 *          the mouse button is pressed. It prepares information to be treated
 *          by the callback function.
 *
 * @param[in]   e           The given canvas pointer
 * @param[in]   b           The button number
 * @param[in]   flags       The evas button flags
 * @param[in]   timestamp   The timestamp of the mouse down event
 * @param[in]   data        The data for canvas
 */
EAPI void evas_event_feed_mouse_down(Evas *e, int b, Evas_Button_Flags flags, unsigned int timestamp, const void *data) EINA_ARG_NONNULL(1);

/**
 * @internal
 * @brief   Feeds the mouse up event for the given canvas @a e.
 *
 * @since_tizen 2.3
 *
 * @remarks This function sets some evas properties that is necessary when
 *          the mouse button is released. It prepares information to be treated
 *          by the callback function.
 *
 * @param[in]   e          The given canvas pointer
 * @param[in]   b          The button number
 * @param[in]   flags      The evas button flags
 * @param[in]   timestamp  The timestamp of the mouse up event
 * @param[in]   data       The data for canvas
 */
EAPI void evas_event_feed_mouse_up(Evas *e, int b, Evas_Button_Flags flags, unsigned int timestamp, const void *data) EINA_ARG_NONNULL(1);

/**
 * @internal
 * @brief   Feeds the mouse move event for the given canvas @a e.
 *
 * @since_tizen 2.3
 *
 * @remarks This function sets some evas properties that is necessary when
 *          the mouse is moved from its last position. It prepares information
 *          to be treated by the callback function.
 *
 * @param[in]   e          The given canvas pointer
 * @param[in]   x          The horizontal position of the mouse pointer
 * @param[in]   y          The vertical position of the mouse pointer
 * @param[in]   timestamp  The timestamp of the mouse up event
 * @param[in]   data       The data for canvas
 */
EAPI void evas_event_feed_mouse_move(Evas *e, int x, int y, unsigned int timestamp, const void *data) EINA_ARG_NONNULL(1);

/**
 * @internal
 * @brief   Feeds the mouse in event for the given canvas @a e.
 *
 * @since_tizen 2.3
 *
 * @remarks This function sets some evas properties that is necessary when
 *          the mouse in event happens. It prepares information to be treated
 *          by the callback function.
 *
 * @param[in]   e          The given canvas pointer
 * @param[in]   timestamp  The timestamp of the mouse up event
 * @param[in]   data       The data for canvas
 */
EAPI void evas_event_feed_mouse_in(Evas *e, unsigned int timestamp, const void *data) EINA_ARG_NONNULL(1);

/**
 * @internal
 * @brief   Feeds the mouse out event for the given canvas @a e.
 *
 * @since_tizen 2.3
 *
 * @remarks This function sets some evas properties that is necessary when
 *          the mouse out event happens. It prepares information to be treated
 *          by the callback function.
 *
 * @param[in]   e          The given canvas pointer
 * @param[in]   timestamp  The timestamp of the mouse up event
 * @param[in]   data       The data for canvas
 */
EAPI void evas_event_feed_mouse_out(Evas *e, unsigned int timestamp, const void *data) EINA_ARG_NONNULL(1);
EAPI void evas_event_feed_multi_down(Evas *e, int d, int x, int y, double rad, double radx, double rady, double pres, double ang, double fx, double fy, Evas_Button_Flags flags, unsigned int timestamp, const void *data);
EAPI void evas_event_feed_multi_up(Evas *e, int d, int x, int y, double rad, double radx, double rady, double pres, double ang, double fx, double fy, Evas_Button_Flags flags, unsigned int timestamp, const void *data);
EAPI void evas_event_feed_multi_move(Evas *e, int d, int x, int y, double rad, double radx, double rady, double pres, double ang, double fx, double fy, unsigned int timestamp, const void *data);

/**
 * @internal
 * @brief   Feeds the mouse cancel event for the given canvas @a e.
 *
 * @remarks This function calls evas_event_feed_mouse_up() when a
 *          mouse cancel event happens.
 *
 * @param   e          The given canvas pointer
 * @param   timestamp  The timestamp of the mouse up event
 * @param   data       The data for canvas
 */
EAPI void evas_event_feed_mouse_cancel(Evas *e, unsigned int timestamp, const void *data) EINA_ARG_NONNULL(1);

/**
 * @internal
 * @brief   Feeds the mouse wheel event for the given canvas @a e.
 *
 * @remarks This function sets some evas properties that is necessary when
 *          the mouse wheel is scrolled up or down. It prepares information to
 *          be treated by the callback function.
 *
 * @param   e          The given canvas pointer
 * @param   direction  The wheel mouse direction
 * @param   z          The amount of up or down mouse wheel scrolling 
 * @param   timestamp  The timestamp of the mouse up event
 * @param   data       The data for canvas
 */
EAPI void evas_event_feed_mouse_wheel(Evas *e, int direction, int z, unsigned int timestamp, const void *data) EINA_ARG_NONNULL(1);

/**
 * @internal
 * @brief   Feeds the key down event for the given canvas @a e.
 *
 * @remarks This function sets some evas properties that is necessary when
 *          a key is pressed. It prepares information to be treated by the
 *          callback function.
 *
 * @param   e          The canvas to thaw out
 * @param   keyname    The name of the key
 * @param   key        The key pressed
 * @param   string     The UTF8 string
 * @param   compose    The compose string
 * @param   timestamp  The timestamp of the mouse up event
 * @param   data       The data for canvas
 */
EAPI void evas_event_feed_key_down(Evas *e, const char *keyname, const char *key, const char *string, const char *compose, unsigned int timestamp, const void *data) EINA_ARG_NONNULL(1);

/**
 * @internal
 * @brief   Feeds the key up event feed  for the given canvas @a e.
 *
 * @remarks This function sets some evas properties that is necessary when
 *          a key is released. It prepares information to be treated by the
 *          callback function.
 *
 * @param   e          The canvas to thaw out
 * @param   keyname    The name of the key
 * @param   key        The key released
 * @param   string     The UTF8 string
 * @param   compose    The compose string
 * @param   timestamp  The timestamp of the mouse up event
 * @param   data       The data for canvas
 */
EAPI void evas_event_feed_key_up(Evas *e, const char *keyname, const char *key, const char *string, const char *compose, unsigned int timestamp, const void *data) EINA_ARG_NONNULL(1);

/**
 * @internal
 * @brief   Feeds the hold event for the given canvas @a e.
 *
 * @remarks This function makes the object to stop sending events.
 *
 * @param   e          The given canvas pointer
 * @param   hold       The hold
 * @param   timestamp  The timestamp of the mouse up event
 * @param   data       The data for canvas
 *
 */
EAPI void evas_event_feed_hold(Evas *e, int hold, unsigned int timestamp, const void *data) EINA_ARG_NONNULL(1);

/**
 * @internal
 * @brief   Re-feeds the event for the given canvas @a e.
 *
 * @since_tizen 2.3
 *
 * @remarks This function re-feeds the event pointed by event_copy.
 *
 * @remarks This function call evas_event_feed_* functions, so it can
 *          cause havoc if not used wisely. Please use it responsibly.
 *
 * @param[in]   e           The given canvas pointer
 * @param[in]   event_copy  The event to re-feed
 * @param[in]   event_type  The event type 
 */
EAPI void evas_event_refeed_event(Evas *e, void *event_copy, Evas_Callback_Type event_type) EINA_ARG_NONNULL(1);

/**
 * @internal
 * @brief   Gets a list of Evas objects lying over a given position in a canvas.
 *
 * @since_tizen 2.3
 *
 * @remarks This function traverses all the layers of the given canvas,
 *          from top to bottom, querying for objects with areas covering the
 *          given position. It enters the smart objects.
 *
 * @param[in]   eo_e     A handle to the canvas
 * @param[in]   stop  The Evas object at which to stop searching
 * @param[in]   x     The horizontal coordinate of the position
 * @param[in]   y     The vertical coordinate of the position
 */
EAPI Eina_List *evas_tree_objects_at_xy_get(Evas *eo_e, Evas_Object *stop, int x, int y) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @}
 */

/**
 * @internal
 * @defgroup Evas_Image_Group Image Functions
 * @ingroup Evas_Canvas
 *
 * @brief This group provides functions that deals with images at canvas level.
 *
 * @{
 */

/**
 * @brief   Flushes the image cache of the canvas.
 *
 * @details This function flushes image cache of canvas.
 *
 * @since_tizen 2.3
 *
 * @param[in]   e  The given evas pointer
 *
 */
EAPI void      evas_image_cache_flush(Evas *e) EINA_ARG_NONNULL(1);

/**
 * @brief   Reloads the image cache.
 *
 * @details This function reloads the image cache of canvas. 
 *
 * @since_tizen 2.3
 *
 * @param[in]   e  The given evas pointer
 *
 */
EAPI void      evas_image_cache_reload(Evas *e) EINA_ARG_NONNULL(1);

/**
 * @brief   Sets the image cache.
 *
 * @details This function sets the image cache of canvas in bytes.
 *
 * @since_tizen 2.3
 *
 * @param[in]   e     The given evas pointer
 * @param[in]   size  The cache size
 */
EAPI void      evas_image_cache_set(Evas *e, int size) EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the image cache
 *
 * @details This function returns the image cache size of canvas in bytes. 
 *
 * @since_tizen 2.3
 *
 * @param[in]   e  The given evas pointer
 * @return  The image cache size of canvas in bytes
 */
EAPI int       evas_image_cache_get(const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the maximum image size evas can possibly handle.
 * @since   1.1
 *
 * @since_tizen 2.3
 *
 * @remarks This function returns the largest image or surface size that evas can handle
 *          in pixels, and if there is one, returns #EINA_TRUE. It returns
 *          #EINA_FALSE if no extra constraint on maximum image size exists. You still
 *          should check the return values of @a maxw and @a maxh as there may still be
 *          a limit, just a much higher one.
 *
 * @param[in]   e     The given evas pointer
 * @param[out]   maxw  The pointer to hold the return value in pixels of the maximum width
 * @param[out]   maxh  The pointer to hold the return value in pixels of the maximum height
 * @return  #EINA_TRUE if there is a maximum size that evas can handle, \n
 *          otherwise #EINA_FALSE if there is no maximum image size constraint
 */
EAPI Eina_Bool evas_image_max_size_get(const Evas *e, int *maxw, int *maxh) EINA_ARG_NONNULL(1);

/**
 * @}
 */

/**
 * @internal
 * @defgroup Evas_Font_Group Font Functions
 *
 * @brief This group provides functions that deals with fonts.
 *
 * @ingroup Evas_Canvas
 *
 * @{
 */

/**
 * @brief   Changes the font hinting for the given evas.
 *
 * @since_tizen 2.3
 *
 * @param[in]   e       The given evas
 * @param[in]   hinting The hinting to use \n
 *                  Valid values are #EVAS_FONT_HINTING_NONE,
 *                  #EVAS_FONT_HINTING_AUTO, and #EVAS_FONT_HINTING_BYTECODE.
 * @ingroup Evas_Font_Group
 */
EAPI void                    evas_font_hinting_set(Evas *e, Evas_Font_Hinting_Flags hinting) EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the font hinting used by the given evas.
 *
 * @since_tizen 2.3
 *
 * @param[in]   e  The given evas to query
 * @return  The hinting in use \n
 *          Valid return values are #EVAS_FONT_HINTING_NONE,
 *         #EVAS_FONT_HINTING_AUTO, and #EVAS_FONT_HINTING_BYTECODE.
 * @ingroup Evas_Font_Group
 */
EAPI Evas_Font_Hinting_Flags evas_font_hinting_get(const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief   Checks whether the font hinting is supported by the given evas.
 *
 * @since_tizen 2.3
 *
 * @param[in]   e        The given evas to query
 * @param[in]   hinting  The hinting to use \n
 *                   Valid values are #EVAS_FONT_HINTING_NONE,
 *                   #EVAS_FONT_HINTING_AUTO, and #EVAS_FONT_HINTING_BYTECODE.
 * @return  #EINA_TRUE if it is supported, \n
 *          otherwise #EINA_FALSE
 * @ingroup Evas_Font_Group
 */
EAPI Eina_Bool               evas_font_hinting_can_hint(const Evas *e, Evas_Font_Hinting_Flags hinting) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief   Forces the given evas and associated engine to flush its font cache.
 *
 * @since_tizen 2.3
 *
 * @param[in]   e  The given evas to flush font cache
 * @ingroup Evas_Font_Group
 */
EAPI void                    evas_font_cache_flush(Evas *e) EINA_ARG_NONNULL(1);

/**
 * @brief   Changes the size of font cache of the given evas.
 *
 * @since_tizen 2.3
 *
 * @param[in]   e     The given evas to flush font cache
 * @param[in]   size  The size in bytes
 *
 * @ingroup Evas_Font_Group
 */
EAPI void                    evas_font_cache_set(Evas *e, int size) EINA_ARG_NONNULL(1);

/**
 * @brief   Changes the size of font cache of the given evas.
 *
 * @since_tizen 2.3
 *
 * @param[in]   e  The given evas to flush font cache
 * @return  The size in bytes
 *
 * @ingroup Evas_Font_Group
 */
EAPI int                     evas_font_cache_get(const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the list of available font descriptions known or found by this evas.
 *
 * @since_tizen 2.3
 *
 * @remarks The list depends on Evas compile time configuration, such as
 *          fontconfig support, and the paths provided at runtime as explained
 *          in @ref Evas_Font_Path_Group.
 *
 * @param[in]   e  The evas instance to query
 * @return  The newly allocated list of strings \n 
 *          Do not change the strings. Be sure to call evas_font_available_list_free()
 *          after you are done.
 *
 * @ingroup Evas_Font_Group
 */
EAPI Eina_List              *evas_font_available_list(const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief   Frees the list of font descriptions returned by evas_font_dir_available_list().
 *
 * @since_tizen 2.3
 *
 * @param[in]   e          The evas instance that returned such list
 * @param[in]   available  The list returned by evas_font_dir_available_list()
 *
 * @ingroup Evas_Font_Group
 */
EAPI void                    evas_font_available_list_free(Evas *e, Eina_List *available) EINA_ARG_NONNULL(1);

/**
 * @brief  Reinitializes Evas by orphaning existing font data until it is unreferenced and
 *         starts with a fresh font configuration read from any appropriate
 *         system resources for all newly loaded and created fonts.
 *
 * @since_tizen 2.3
 *
 * @ingroup Evas_Font_Group
 */
EAPI void                    evas_font_reinit(void);

/**
 * @}
 */

/**
 * @internal
 * @defgroup Evas_Font_Path_Group Font Path Functions
 *
 * @brief   This group provides functions that edit the paths being used to load fonts.
 *
 * @ingroup Evas_Font_Group
 *
 * @{
 */

/**
 * @brief   Removes all font paths loaded into memory for the given evas.
 *
 * @since_tizen 2.3
 *
 * @param[in]   e  The given evas
 * @ingroup Evas_Font_Path_Group
 */
EAPI void                    evas_font_path_clear(Evas *e) EINA_ARG_NONNULL(1);

/**
 * @brief   Appends a font path to the list of font paths used by the given evas.
 *
 * @since_tizen 2.3
 *
 * @param[in]   e     The given evas
 * @param[in]   path  The new font path
 * @ingroup Evas_Font_Path_Group
 */
EAPI void                    evas_font_path_append(Evas *e, const char *path) EINA_ARG_NONNULL(1, 2);

/**
 * @brief   Prepends a font path to the list of font paths used by the given evas.
 *
 * @since_tizen 2.3
 *
 * @param[in]   e     The given evas
 * @param[in]   path  The new font path
 * @ingroup Evas_Font_Path_Group
 */
EAPI void                    evas_font_path_prepend(Evas *e, const char *path) EINA_ARG_NONNULL(1, 2);

/**
 * @brief   Gets the list of font paths used by the given evas.
 *
 * @since_tizen 2.3
 *
 * @param[in]   e  The given evas
 * @return  The list of font paths used
 * @ingroup Evas_Font_Path_Group
 */
EAPI const Eina_List        *evas_font_path_list(const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief   Removes all font paths loaded into memory by evas_font_path_app_* APIs for the application.
 * @since   1.9
 *
 * @since_tizen 2.3
 *
 * @ingroup Evas_Font_Path_Group
 */
EAPI void                    evas_font_path_global_clear(void);

/**
 * @brief   Appends a font path to the list of font paths used by the application.
 * @since   1.9
 *
 * @since_tizen 2.3
 *
 * @param[in]   path  The new font path
 * @ingroup Evas_Font_Path_Group
 */
EAPI void                    evas_font_path_global_append(const char *path) EINA_ARG_NONNULL(1);

/**
 * @brief   Prepends a font path to the list of font paths used by the application.
 * @since   1.9
 *
 * @since_tizen 2.3
 *
 * @param[in]   path  The new font path
 * @ingroup Evas_Font_Path_Group
 */
EAPI void                    evas_font_path_global_prepend(const char *path) EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the list of font paths used by the application.
 * @since   1.9
 *
 * @since_tizen 2.3
 *
 * @return  The list of font paths used
 * @ingroup Evas_Font_Path_Group
 */
EAPI const Eina_List        *evas_font_path_global_list(void) EINA_WARN_UNUSED_RESULT;

/**
 * @}
 */

/**
 * @defgroup Evas_Object_Group Generic Object Functions
   @ingroup Evas
 *
 * @brief   This group provides functions that manipulate generic Evas objects.
 *
 * @remarks All Evas displaying units are Evas objects. You can handle them all by
 *          means of the handle ::Evas_Object. Besides Evas treats their
 *          objects equally, they have @b types, which define their specific
 *          behavior (and individual API).
 *
 * @remarks Evas comes with a set of built-in object types:
 *          - rectangle,
 *          - line,
 *          - polygon,
 *          - text,
 *          - textblock, and
 *          - image.
 *
 * @remarks These functions apply to @b any Evas object, whichever type that
 *          may have.
 *
 * @remarks The built-in types which are most used are rectangles, text
 *          and images. In fact, with these ones one can create 2D interfaces
 *          of arbitrary complexity and EFL makes it easy.
 */

/**
 * @defgroup Evas_Object_Group_Basic Basic Object Manipulation
 * @ingroup Evas_Object_Group
 *
 * @brief   This group provides functions for basic object manipulation.
 *
 * @remarks Almost every evas object created has some generic function used to
 *          manipulate it. That is because there are a number of basic actions to be done
 *          to objects that are irrespective of the object's type like:
 *          @li Showing/Hiding
 *          @li Setting(and getting) geometry
 *          @li Bring up or down a layer
 *          @li Color management
 *          @li Handling focus
 *          @li Clipping
 *          @li Reference counting
 *
 * @{
 */

/**
 * @brief   Clips one object to another.
 *
 * @details This function clips the object @a obj to the area occupied by
 *          the object @a clip. This means the object @a obj is only
 *          visible within the area occupied by the clipping object (@a clip).
 *
 * @since_tizen 2.3
 *
 * @remarks The color of the object being clipped is multiplied by the
 *          color of the clipping one, so the resulting color for the former
 *          is <code>RESULT = (OBJ * CLIP) / (255 * 255)</code>, per color
 *          element (red, green, blue and alpha).
 *
 * @remarks Clipping is recursive, so clipping objects may be clipped by
 *          others, and their color is in term multiplied. You may @b not
 *          set up circular clipping lists (i.e. object 1 clips object 2, which
 *          clips object 1): the behavior of Evas is undefined in this case.
 *
 * @remarks Objects which do not clip others are visible in the canvas as
 *          normal; <b>those that clip one or more objects become invisible
 *          themselves</b>, only affecting what they clip. If an object ceases
 *          to have other objects being clipped by it, it becomes visible
 *          again.
 *
 * @remarks The visibility of an object affects the objects that are clipped by
 *          it, so if the object clipping others is not shown (as in
 *          evas_object_show()), the objects clipped by it is not shown
 *          either.
 *
 * @remarks If @a obj is being clipped by another object when this function is
 *          called, it gets implicitly removed from the old clipper's domain
 *          and is made now to be clipped by its new clipper.
 *
 * @remarks The following figure illustrates some clipping in Evas:
 *
 * @image html clipping.png
 * @image rtf clipping.png
 * @image latex clipping.eps
 *
 * @remarks At the moment the <b>only objects that can validly be used to
 *          clip other objects are rectangle objects</b>. All other object
 *          types are invalid and the result of using them is undefined. The
 *          clip object @a clip must be a valid object, but can also be @c
 *          NULL, in which case the effect of this function is the same as
 *          calling evas_object_clip_unset() on the @a obj object.
 *
 * @param[in]   obj   The object to be clipped
 * @param[in]   clip  The object to clip @a obj by
 */
EAPI void             evas_object_clip_set(Evas_Object *obj, Evas_Object *clip) EINA_ARG_NONNULL(1, 2);

/**
 * @brief   Gets the object clipping @a obj (if any).
 *
 * @details This function returns the object clipping @a obj. If @a obj is
 *          not being clipped at all, @c NULL is returned. The object @a obj
 *          must be a valid ::Evas_Object.
 *
 * @since_tizen 2.3
 *
 * @remarks See also evas_object_clip_set(), evas_object_clip_unset() and
 *          evas_object_clipees_get().
 *
 * @param[in]   obj  The object to get the clipper from
 * @return  The object clipping @a obj \n
 *          If @a obj object is not being clipped, @c NULL is returned.
 */
EAPI Evas_Object     *evas_object_clip_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief   Disables or ceases clipping on a clipped @a obj object.
 *
 * @details This function disables clipping for the object @a obj, if it is
 *          already clipped, i.e., its visibility and color get detached from
 *          the previous clipper. If it is not already clipped, this has no effect. The object
 *          @a obj must be a valid ::Evas_Object.
 *
 * @since_tizen 2.3
 *
 * @remarks See also evas_object_clip_set() (for an example),
 *          evas_object_clipees_get() and evas_object_clip_get().
 *
 * @param[in]   obj  The object to cease clipping on
 */
EAPI void             evas_object_clip_unset(Evas_Object *obj);

/**
 * @brief   Gets a list of objects currently clipped by @a obj.
 *
 * @details This returns the internal list handle that contains all objects
 *          clipped by the object @a obj. If none are clipped by it, the call
 *          returns @c NULL. This list is only valid until the clip list is
 *          changed and should be fetched again with another call to
 *          evas_object_clipees_get() if any objects being clipped by this
 *          object are unclipped, clipped by a new object, deleted or get the
 *          clipper deleted. These operations invalidate the list
 *          returned, so it should not be used anymore after that point. Any
 *          use of the list after this may have undefined results, possibly
 *          leading to crashes. The object @a obj must be a valid
 *          ::Evas_Object.
 *
 * @since_tizen 2.3
 *
 * @remarks See also evas_object_clip_set(), evas_object_clip_unset() and
 *          evas_object_clip_get().
 *
 * @remarks The following is an example:
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
 *
 * @param[in]   obj  The object to get a list of clippees from
 * @return  The list of objects being clipped by @a obj
 */
EAPI const Eina_List *evas_object_clipees_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief   Sets or unsets a given object as the currently focused one on its canvas.
 *
 * @since_tizen 2.3
 *
 * @remarks Changing focus only affects where (key) input events go. There can
 *          be only one object focused at any time. If @a focus is #EINA_TRUE,
 *          @a obj is set as the currently focused object and it
 *          receives all keyboard events that are not exclusive key grabs on
 *          other objects.
 *
 * @param[in]   obj    The object to be focused or unfocused
 * @param[in]   focus  #EINA_TRUE to set it as focused, \n
 *                 otherwise #EINA_FALSE to take away the focus from it
 *
 * @see evas_object_focus_get
 * @see evas_focus_get
 * @see evas_object_key_grab
 * @see evas_object_key_ungrab
 */
EAPI void             evas_object_focus_set(Evas_Object *obj, Eina_Bool focus) EINA_ARG_NONNULL(1);

/**
 * @brief   Checks whether an object has the focus.
 *
 * @since_tizen 2.3
 *
 * @remarks If the passed object is the currently focused one, #EINA_TRUE is
 *          returned. #EINA_FALSE is returned, otherwise.
 *
 * @param[in]   obj  The object to retrieve focus information from
 * @return  #EINA_TRUE if the object has the focus, \n
 *          otherwise #EINA_FALSE
 *
 * @see evas_object_focus_set
 * @see evas_focus_get
 * @see evas_object_key_grab
 * @see evas_object_key_ungrab
 */
EAPI Eina_Bool        evas_object_focus_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief   Sets the layer of its canvas that the given object is part of.
 *
 * @since_tizen 2.3
 *
 * @remarks If you do not use this function, you are dealing with a @b unique
 *          layer of objects, the default one. Additional layers are handy when
 *          you do not want a set of objects to interfere with another set with
 *          regard to @b stacking. Two layers are completely disjoint in that matter.
 *
 * @remarks This is a low-level function, which you would be using when something
 *          should be always on top, for example.
 *
 * @remarks Be careful, it does not make sense to change the layer of the children of
 *          smart objects. Smart objects have a layer of their own,
 *          which should contain all their children objects.
 *
 * @param[in]   obj  The given Evas object
 * @param[in]   l    The number of the layer to place the object on \n
 *               This must be between #EVAS_LAYER_MIN and #EVAS_LAYER_MAX.
 *
 * @see evas_object_layer_get()
 */
EAPI void             evas_object_layer_set(Evas_Object *obj, short l) EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the layer of its canvas that the given object is part of.
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj  The given Evas object to query the layer from
 * @return  The number of its layer
 *
 * @see evas_object_layer_set()
 */
EAPI short            evas_object_layer_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief   Sets the name of the given Evas object to the given name.
 *
 * @since_tizen 2.3
 *
 * @remarks There might be occasions where you would like to name your objects.
 *
 * @param[in]   obj   The given object
 * @param[in]   name  The given name
 */
EAPI void             evas_object_name_set(Evas_Object *obj, const char *name) EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the name of the given Evas object.
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj  The given object
 * @return  The name of the object, \n
 *          otherwise @c NULL if no name has been given to it
 */
EAPI const char      *evas_object_name_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief   Increments object reference count to defer its deletion.
 * @since   1.1
 *
 * @since_tizen 2.3
 *
 * @remarks This increments the reference count of an object, which if greater
 *          than @c 0 defers deletion by evas_object_del() until all
 *          references are released back (counter back to @c 0). References cannot
 *          go below @c 0 and unreferencing past that results in the reference
 *          count being limited to @c 0. References are limited to <c>2^32 - 1</c>
 *          for an object. Referencing it more than this results in it
 *          being limited to this value.
 *
 * @remarks This is a <b>very simple</b> reference counting mechanism. For
 *          instance, Evas is not ready to check for pending references on a
 *          canvas deletion, or things like that. This is useful on scenarios
 *          where, inside a code block, callbacks exist which would possibly
 *          delete an object that you are operating on afterwards. Then, you would
 *          evas_object_ref() it on the beginning of the block and
 *          evas_object_unref() it on the end. It would then be deleted at this
 *          point, if it should be.
 *
 * @remarks The following is an example:
 * @code
 *  evas_object_ref(obj);
 *
 *  // action here...
 *  evas_object_smart_callback_call(obj, SIG_SELECTED, NULL);
 *  // more action here...
 *  evas_object_unref(obj);
 * @endcode
 *
 * @param[in]   obj  The given Evas object to reference
 *
 * @see evas_object_unref()
 * @see evas_object_del()
 * @ingroup Evas_Object_Group_Basic
 */
EAPI void             evas_object_ref(Evas_Object *obj);

/**
 * @brief   Decrements object reference count.
 *
 * @details This decrements the reference count of an object. If the object has
 *          had evas_object_del() called on it while references were more than
 *          @c 0, it is deleted at the time this function is called and puts
 *          the counter back to @c 0. See evas_object_ref() for more information.
 * @since   1.1
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj  The given Evas object to unreference
 *
 * @see evas_object_ref() (for an example)
 * @see evas_object_del()
 *
 * @ingroup Evas_Object_Group_Basic
 */
EAPI void             evas_object_unref(Evas_Object *obj);

/**
 * @brief   Gets the object reference count.
 *
 * @details This gets the reference count for an object (normally @c 0 until it is
 *          referenced). Values of @c 1 or greater mean that someone is holding a
 *          reference to this object that needs to be unreffed before it can be
 *          deleted.
 * @since   1.2
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj  The given Evas object to query
 * @return The object reference count
 *
 * @see evas_object_ref()
 * @see evas_object_unref()
 * @see evas_object_del()
 *
 * @ingroup Evas_Object_Group_Basic
 */
EAPI int              evas_object_ref_get(const Evas_Object *obj);

/**
 * @brief   Marks the given Evas object for deletion (when Evas frees its memory).
 *
 * @details This call marks @a obj for deletion, which takes place
 *          whenever it has no more references to it (see evas_object_ref() and
 *          evas_object_unref()).
 *
 * @since_tizen 2.3
 *
 * @remarks At actual deletion time, which may or may not be just after this
 *          call, ::EVAS_CALLBACK_DEL and ::EVAS_CALLBACK_FREE callbacks are
 *          called. If the object currently had the focus, its
 *          ::EVAS_CALLBACK_FOCUS_OUT callback is also called.
 *
 * @param[in]   obj  The given Evas object
 *
 * @see evas_object_ref()
 * @see evas_object_unref()
 *
 * @ingroup Evas_Object_Group_Basic
 */
EAPI void             evas_object_del(Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * @brief   Moves the given Evas object to the given location inside its canvas' viewport.
 *
 * @since_tizen 2.3
 *
 * @remarks Besides being moved, the object's ::EVAS_CALLBACK_MOVE callback is called.
 *
 * @remarks Naturally, newly created objects are placed at the canvas' origin: <code>0, 0</code>.
 *
 * @param[in]   obj  The given Evas object
 * @param[in]   x    The X position to move the object to, in canvas units
 * @param[in]   y    The Y position to move the object to, in canvas units
 *
 * @ingroup Evas_Object_Group_Basic
 */
EAPI void             evas_object_move(Evas_Object *obj, Evas_Coord x, Evas_Coord y) EINA_ARG_NONNULL(1);

/**
 * @brief   Changes the size of the given Evas object.
 *
 * @since_tizen 2.3
 *
 * @remarks Besides being resized, the object's ::EVAS_CALLBACK_RESIZE callback
 *          is called.
 *
 * @remarks Newly created objects have zeroed dimensions. Then, you most
 *          probably want to use evas_object_resize() on them after they are
 *          created.
 *
 * @remarks Be aware that resizing an object changes its drawing area,
 *          but that does imply the object is rescaled. For instance, images
 *          are filled inside their drawing area using the specifications of
 *          evas_object_image_fill_set(). Thus to scale the image to match
 *          exactly your drawing area, you need to change the
 *          evas_object_image_fill_set() as well.
 *
 * @remarks This is more evident in images, but text, textblock, lines
 *          and polygons behave similarly. Check their specific APIs to
 *          know how to achieve your desired behavior. Consider the following example:
 *
 * @code
 * // rescale image to fill exactly its area without tiling:
 * evas_object_resize(img, w, h);
 * evas_object_image_fill_set(img, 0, 0, w, h);
 * @endcode
 *
 * @param[in]   obj  The given Evas object
 * @param[in]   w    The new width of the Evas object
 * @param[in]   h    The new height of the Evas object
 * @ingroup Evas_Object_Group_Basic
 */
EAPI void             evas_object_resize(Evas_Object *obj, Evas_Coord w, Evas_Coord h) EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the position and (rectangular) size of the given Evas object.
 *
 * @since_tizen 2.3
 *
 * @remarks The position is relative to the top left corner of
 *          the canvas' viewport.
 *
 * @remarks Use @c NULL pointers on the geometry components you are not
 *          interested in: they are ignored by the function.
 *
 * @param[in]   obj  The given Evas object
 * @param[out]   x    The pointer to an integer in which to store the X coordinate
 *               of the object
 * @param[out]   y    The pointer to an integer in which to store the Y coordinate
 *               of the object
 * @param[out]   w    The pointer to an integer in which to store the width of the object
 * @param[out]   h    The pointer to an integer in which to store the height of the object
 *
 * @ingroup Evas_Object_Group_Basic
 */
EAPI void             evas_object_geometry_get(const Evas_Object *obj, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h) EINA_ARG_NONNULL(1);

/**
 * @brief   Makes the given Evas object visible.
 *
 * @since_tizen 2.3
 *
 * @remarks Besides becoming visible, the object's ::EVAS_CALLBACK_SHOW
 *          callback are called.
 *
 * @param[in]   obj  The given Evas object
 *
 * @see evas_object_hide() for more on object visibility.
 * @see evas_object_visible_get()
 *
 * @ingroup Evas_Object_Group_Basic
 */
EAPI void             evas_object_show(Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * @brief   Makes the given Evas object invisible.
 *
 * @since_tizen 2.3
 *
 * @remarks Hidden objects, besides not being shown at all in your canvas,
 *          are not checked for changes on the canvas rendering
 *          process. Furthermore, they do not catch input events. Thus, they
 *          are much lighter (in processing needs) than an object that is
 *          invisible due to indirect causes, such as being clipped or out of
 *          the canvas' viewport.
 *
 * @remarks Besides becoming hidden, @a obj object's ::EVAS_CALLBACK_SHOW
 *          callback is called.
 *
 * @remarks All objects are created in the hidden state. If you want to
 *          show them, use evas_object_show() after their creation.
 *
 * @param[in]   obj  The given Evas object
 *
 * @see evas_object_show()
 * @see evas_object_visible_get()
 * @ingroup Evas_Object_Group_Basic
 */
EAPI void             evas_object_hide(Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * @brief   Checks whether the given Evas object is visible.
 *
 * @since_tizen 2.3
 *
 * @details This retrieves an object's visibility as the one enforced by
 *          evas_object_show() and evas_object_hide().
 *
 * @remarks The value returned is not, by any means, influenced by
 *          clippers covering @a obj, it being out of its canvas' viewport or
 *          stacked below other object.
 *
 * @param[in]   obj  The given Evas object
 * @return  #EINA_TRUE if the object is visible, \n
 *          otherwise #EINA_FALSE if the object is not visible
 *
 * @see evas_object_show()
 * @see evas_object_hide() (for an example)
 *
 * @ingroup Evas_Object_Group_Basic
 */
EAPI Eina_Bool        evas_object_visible_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief   Sets the general or main color of the given Evas object.
 *
 * @since_tizen 2.3
 *
 * @remarks These color values are expected to be premultiplied by @a a.
 *
 * @param[in]   obj  The given Evas object
 * @param[in]   r    The red component of the given color
 * @param[in]   g    The green component of the given color
 * @param[in]   b    The blue component of the given color
 * @param[in]   a    The alpha component of the given color
 *
 * @see evas_object_color_get() (for an example)
 *
 * @ingroup Evas_Object_Group_Basic
 */
EAPI void             evas_object_color_set(Evas_Object *obj, int r, int g, int b, int a) EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the general or main color of the given Evas object.
 *
 * @since_tizen 2.3
 *
 * @remarks Gets the "main" color's RGB component (and alpha channel)
 *          values, <b>which range from 0 to 255</b>. For the alpha channel,
 *          which defines the object's transparency level, @c 0 means totally
 *          transparent, while @c 255 means opaque. These color values are
 *          premultiplied by the alpha value.
 *
 * @remarks Usually you use this attribute for text and rectangle objects,
 *          where the "main" color is their unique one. If set for objects
 *          which themselves have colors, like the images one, those colors get
 *          modulated by this one.
 *
 * @remarks All newly created Evas rectangles get the default color
 *          values of <code>255 255 255 255</code> (opaque white).
 *
 * @remarks Use @c NULL pointers on the components you are not interested
 *          in: they are ignored by the function.
 *
 * @param[in]   obj  The given Evas object to retrieve color from
 * @param[out]   r    The pointer to an integer in which to store the red component 
 *               of the color
 * @param[out]   g    The pointer to an integer in which to store the green
 *               component of the color
 * @param[out]   b    The pointer to an integer in which to store the blue component
 *               of the color
 * @param[out]   a    The pointer to an integer in which to store the alpha
 *               component of the color
 *
 * @ingroup Evas_Object_Group_Basic
 */
EAPI void             evas_object_color_get(const Evas_Object *obj, int *r, int *g, int *b, int *a) EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the Evas canvas that the given object lives on.
 *
 * @details This function is most useful at code contexts where you need to
 *          operate on the canvas but have only the object pointer.
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj  The given Evas object
 * @return  A pointer to the canvas where the object is on
 *
 * @ingroup Evas_Object_Group_Basic
 */
EAPI Evas            *evas_object_evas_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the type of the given Evas object.
 *
 * @since_tizen 2.3
 *
 * @remarks For Evas' built-in types, the return strings are one of:
 *          - <c>"rectangle"</c>,
 *          - <c>"line"</c>,
 *          - <c>"polygon"</c>,
 *          - <c>"text"</c>,
 *          - <c>"textblock"</c> and
 *          - <c>"image"</c>.
 *
 * @internal
 * @remarks For Evas smart objects (see @ref Evas_Smart_Group), the name of the
 *          smart class itself is returned on this call. For the built-in smart
 *          objects, these names are:
 *          - <c>"EvasObjectSmartClipped"</c>, for the clipped smart object
 *          - <c>"Evas_Object_Box"</c>, for the box object and
 *          - <c>"Evas_Object_Table"</c>, for the table object.
 * @endinternal
 *
 * @param[in]   obj  The given object
 * @return  The type of the object
 */
EAPI const char      *evas_object_type_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief   Raises @a obj to the top of its layer.
 *
 * @since_tizen 2.3
 *
 * @remarks @a obj is, then, the highest one in the layer it belongs
 *          to. Objects on other layers do not get touched.
 *
 * @param[in]   obj  The object to raise
 *
 * @see evas_object_stack_above()
 * @see evas_object_stack_below()
 * @see evas_object_lower()
 */
EAPI void             evas_object_raise(Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * @brief   Lowers @a obj to the bottom of its layer.
 *
 * @since_tizen 2.3
 *
 * @remarks @a obj is, then, the lowest one in the layer it belongs
 *          to. Objects on other layers do not get touched.
 *
 * @param[in]   obj  The object to lower
 *
 * @see evas_object_stack_above()
 * @see evas_object_stack_below()
 * @see evas_object_raise()
 */
EAPI void             evas_object_lower(Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * @brief   Stacks @a obj immediately above @a above.
 *
 * @since_tizen 2.3
 *
 * @remarks Objects, in a given canvas, are stacked in the order they get added
 *          to it.  This means that, if they overlap, the highest ones
 *          cover the lowest ones, in that order. This function is a way to
 *          change the stacking order for the objects.
 *
 * @remarks This function is intended to be used with <b>objects belonging to
 *          the same layer</b> in a given canvas, otherwise it fails (and
 *          accomplish nothing).
 *
 * @remarks If you have smart objects on your canvas and @a obj is a member of
 *          one of them, then @a above must also be a member of the same
 *          smart object.
 *
 * @remarks Similarly, if @a obj is not a member of a smart object, @a above
 *          must not be either.
 *
 * @param[in]   obj    The object to stack
 * @param[in]   above  The object above which to stack
 *
 * @see evas_object_layer_get()
 * @see evas_object_layer_set()
 * @see evas_object_stack_below()
 */
EAPI void             evas_object_stack_above(Evas_Object *obj, Evas_Object *above) EINA_ARG_NONNULL(1, 2);

/**
 * @brief   Stacks @a obj immediately below @a below.
 *
 * @since_tizen 2.3
 *
 * @remarks Objects, in a given canvas, are stacked in the order they get added
 *          to it. This means that, if they overlap, the highest ones
 *          cover the lowest ones, in that order. This function is a way to
 *          change the stacking order for the objects.
 *
 * @remarks This function is intended to be used with <b>objects belonging to
 *          the same layer</b> in a given canvas, otherwise it fails (and
 *          accomplish nothing).
 *
 * @remarks If you have smart objects on your canvas and @a obj is a member of
 *          one of them, then @a below must also be a member of the same
 *          smart object.
 *
 * @remarks Similarly, if @a obj is not a member of a smart object, @a below
 *          must not be either.
 *
 * @param[in]   obj    The object to stack
 * @param[in]   below  The object below which to stack
 *
 * @see evas_object_layer_get()
 * @see evas_object_layer_set()
 * @see evas_object_stack_below()
 */
EAPI void             evas_object_stack_below(Evas_Object *obj, Evas_Object *below) EINA_ARG_NONNULL(1, 2);

/**
 * @brief   Gets the Evas object stacked right above @a obj.
 *
 * @since_tizen 2.3
 *
 * @remarks This function traverses layers in its search, if there are
 *          objects on layers above the one @a obj is placed at.
 *
 * @param[in]   obj  An #Evas_Object
 * @return  The #Evas_Object directly above @a obj, if any, \n
 *          otherwise @c NULL if none
 *
 * @see evas_object_layer_get()
 * @see evas_object_layer_set()
 * @see evas_object_below_get()
 *
 */
EAPI Evas_Object     *evas_object_above_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the Evas object stacked right below @a obj.
 *
 * @details This function traverses layers in its search, if there are
 *          objects on layers below the one @a obj is placed at.
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj  An #Evas_Object
 * @return  The #Evas_Object directly below @a obj, if any, \n
 *          otherwise @c NULL if none
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
 * @ingroup Evas_Object_Group
 *
 * @brief   This group provides functions for object events.
 *
 * @remarks Objects generate events when they are moved, resized, when their
 *          visibility change, when they are deleted and so on. These methods
 *          allow you to be notified about and to handle such events.
 *
 * @remarks Objects also generate events on input (keyboard and mouse), if they
 *          accept them (are visible, focused, and so on).
 *
 * @remarks For each of those events, Evas provides a way for one to register
 *          callback functions to be issued just after they happen.
 *
 * @remarks The following figure illustrates some Evas (event) callbacks:
 *
 * @image html evas-callbacks.png
 * @image rtf evas-callbacks.png
 * @image latex evas-callbacks.eps
 *
 * @remarks These events have their values in the #Evas_Callback_Type
 *          enumeration, which also has the happening on the canvas level (see
 *          @ref Evas_Canvas_Events ).
 *
 * @{
 */

/**
 * @brief   Adds or registers a callback function to a given Evas object event.
 *
 * @details This function adds a function callback to an object when the event
 *
 * @since_tizen 2.3          of type @a type occurs on object @a obj. The function is @a func.
 *
 *
 * @remarks In the event of a memory allocation error during addition of the
 *          callback to the object, evas_alloc_error() should be used to
 *          determine the nature of the error, if any, and the program should
 *          sensibly try and recover.
 *
 * @remarks A callback function must have the #Evas_Object_Event_Cb prototype
 *          definition. The first parameter (@a data) in this definition
 *          has the same value passed to evas_object_event_callback_add() as
 *          the @a data parameter, at runtime. The second parameter @a e is the
 *          canvas pointer on which the event occurred. The third parameter is
 *          a pointer to the object on which event occurred. Finally, the
 *          fourth parameter @a event_info is a pointer to a data structure
 *          that may or may not be passed to the callback, depending on the
 *          event type that triggered the callback. This is so because some
 *          events do not carry extra context with them, but others do.
 *
 * @remarks The valid event type @a type to trigger the function are
 *          #EVAS_CALLBACK_MOUSE_IN, #EVAS_CALLBACK_MOUSE_OUT,
 *          #EVAS_CALLBACK_MOUSE_DOWN, #EVAS_CALLBACK_MOUSE_UP,
 *          #EVAS_CALLBACK_MOUSE_MOVE, #EVAS_CALLBACK_MOUSE_WHEEL,
 *          #EVAS_CALLBACK_MULTI_DOWN, #EVAS_CALLBACK_MULTI_UP,
 *          #EVAS_CALLBACK_MULTI_MOVE, #EVAS_CALLBACK_FREE,
 *          #EVAS_CALLBACK_KEY_DOWN, #EVAS_CALLBACK_KEY_UP,
 *          #EVAS_CALLBACK_FOCUS_IN, #EVAS_CALLBACK_FOCUS_OUT,
 *          #EVAS_CALLBACK_SHOW, #EVAS_CALLBACK_HIDE, #EVAS_CALLBACK_MOVE,
 *          #EVAS_CALLBACK_RESIZE, #EVAS_CALLBACK_RESTACK, #EVAS_CALLBACK_DEL,
 *          #EVAS_CALLBACK_HOLD, #EVAS_CALLBACK_CHANGED_SIZE_HINTS,
 *          #EVAS_CALLBACK_IMAGE_PRELOADED or #EVAS_CALLBACK_IMAGE_UNLOADED.
 *
 * @remarks This determines the kind of event that triggers the callback.
 *          The event types are explained below along with their associated @a event_info pointers:
 *
 * @remarks - #EVAS_CALLBACK_MOUSE_IN: @a event_info is a pointer to an
 *          #Evas_Event_Mouse_In struct\n\n
 *          This event is triggered when the mouse pointer enters the area
 *          (not shaded by other objects) of the object @a obj. This may
 *          occur by the mouse pointer being moved or by the object being shown,
 *          raised, moved, resized, or other objects being moved out of the
 *          way, hidden or lowered, whatever may cause the mouse pointer to
 *          get on top of @a obj, having been on top of another object
 *          previously.
 *
 *          - #EVAS_CALLBACK_MOUSE_OUT: @a event_info is a pointer to an
 *          #Evas_Event_Mouse_Out struct\n\n
 *          This event is triggered exactly like #EVAS_CALLBACK_MOUSE_IN is,
 *          but it occurs when the mouse pointer exits an object's area. Note
 *          that no mouse out events are reported if the mouse pointer is
 *          implicitly grabbed to an object (mouse buttons are down, having
 *          been pressed while the pointer is over that object). In these
 *          cases, mouse out events are reported once all buttons are
 *          released, if the mouse pointer has left the object's area. The
 *          indirect ways of taking off the mouse pointer from an object,
 *          like cited above, for #EVAS_CALLBACK_MOUSE_IN, also apply here,
 *          naturally.
 *
 *          - #EVAS_CALLBACK_MOUSE_DOWN: @a event_info is a pointer to an
 *          #Evas_Event_Mouse_Down struct\n\n
 *          This event is triggered by a mouse button being pressed while the
 *          mouse pointer is over an object. If the pointer mode for Evas is
 *          #EVAS_OBJECT_POINTER_MODE_AUTOGRAB (default), this causes this
 *          object to <b>passively grab the mouse</b> until all mouse buttons
 *          have been released: all future mouse events are reported to
 *          only this object until no buttons are down. That includes mouse
 *          move events, mouse in and mouse out events, and further button
 *          presses. When all buttons are released, event propagation
 *          occurs as normal (see #Evas_Object_Pointer_Mode).
 *
 *          - #EVAS_CALLBACK_MOUSE_UP: @a event_info is a pointer to an
 *          #Evas_Event_Mouse_Up struct\n\n
 *          This event is triggered by a mouse button being released while
 *          the mouse pointer is over an object's area (or when passively
 *          grabbed to an object).
 *
 *          - #EVAS_CALLBACK_MOUSE_MOVE: @a event_info is a pointer to an
 *          #Evas_Event_Mouse_Move struct\n\n
 *          This event is triggered by the mouse pointer being moved while
 *          over an object's area (or while passively grabbed to an object).
 *
 *          - #EVAS_CALLBACK_MOUSE_WHEEL: @a event_info is a pointer to an
 *          #Evas_Event_Mouse_Wheel struct\n\n
 *          This event is triggered by the mouse wheel being rolled while the
 *          mouse pointer is over an object (or passively grabbed to an object).
 *
 *          - #EVAS_CALLBACK_MULTI_DOWN: @a event_info is a pointer to an
 *          #Evas_Event_Multi_Down struct
 *
 *          - #EVAS_CALLBACK_MULTI_UP: @a event_info is a pointer to an
 *          #Evas_Event_Multi_Up struct
 *
 *          - #EVAS_CALLBACK_MULTI_MOVE: @a event_info is a pointer to an
 *          #Evas_Event_Multi_Move struct
 *
 *          - #EVAS_CALLBACK_FREE: @a event_info is @c NULL \n\n
 *          This event is triggered just before Evas is about to free all
 *          memory used by an object and remove all references to it. This is
 *          useful for programs to use if they attached data to an object and
 *          want to free it when the object is deleted. The object is still
 *          valid when this callback is called, but after it returns, there
 *          is no guarantee on the object's validity.
 *
 *          - #EVAS_CALLBACK_KEY_DOWN: @a event_info is a pointer to an
 *          #Evas_Event_Key_Down struct\n\n
 *          This callback is called when a key is pressed and the focus is on
 *          the object, or a key has been grabbed to a particular object
 *          which wants to intercept the key press regardless of what object
 *          has the focus.
 *
 *          - #EVAS_CALLBACK_KEY_UP: @a event_info is a pointer to an
 *          #Evas_Event_Key_Up struct \n\n
 *          This callback is called when a key is released and the focus is
 *          on the object, or a key has been grabbed to a particular object
 *          which wants to intercept the key release regardless of what
 *          object has the focus.
 *
 *          - #EVAS_CALLBACK_FOCUS_IN: @a event_info is @c NULL \n\n
 *          This event is called when an object gains the focus. When it is
 *          called the object has already gained the focus.
 *
 *          - #EVAS_CALLBACK_FOCUS_OUT: @a event_info is @c NULL \n\n
 *          This event is triggered when an object loses the focus. When it
 *          is called the object has already lost the focus.
 *
 *          - #EVAS_CALLBACK_SHOW: @a event_info is @c NULL \n\n
 *          This event is triggered by the object being shown by
 *          evas_object_show().
 *
 *          - #EVAS_CALLBACK_HIDE: @a event_info is @c NULL \n\n
 *          This event is triggered by an object being hidden by
 *          evas_object_hide().
 *
 *          - #EVAS_CALLBACK_MOVE: @a event_info is @c NULL \n\n
 *          This event is triggered by an object being
 *          moved. evas_object_move() can trigger this, as can any
 *          object-specific manipulations that would mean the object's origin
 *          could move.
 *
 *          - #EVAS_CALLBACK_RESIZE: @a event_info is @c NULL \n\n
 *          This event is triggered by an object being resized. Resizes can
 *          be triggered by evas_object_resize() or by any object-specific
 *          calls that may cause the object to resize.
 *
 *          - #EVAS_CALLBACK_RESTACK: @a event_info is @c NULL \n\n
 *          This event is triggered by an object being re-stacked. Stacking
 *          changes can be triggered by
 *          evas_object_stack_below()/evas_object_stack_above() and others.
 *
 *          - #EVAS_CALLBACK_DEL: @a event_info is @c NULL.
 *
 *          - #EVAS_CALLBACK_HOLD: @a event_info is a pointer to an
 *          #Evas_Event_Hold struct
 *
 *          - #EVAS_CALLBACK_CHANGED_SIZE_HINTS: @a event_info is @c NULL.
 *
 *          - #EVAS_CALLBACK_IMAGE_PRELOADED: @a event_info is @c NULL.
 *
 *          - #EVAS_CALLBACK_IMAGE_UNLOADED: @a event_info is @c NULL.
 *
 * @remarks Be careful not to add the same callback multiple times, if
 *          that is not what you want, because Evas does not check if a callback
 *          existed before exactly as the one being registered (and thus, call
 *          it more than once on the event, in this case). This would make
 *          sense if you passed different functions and/or callback data, only.
 *
 * @param[in]   obj   The object to attach a callback to
 * @param[in]   type  The type of event that triggers the callback
 * @param[in]   func  The function to be called when the event is triggered
 * @param[in]   data  The data pointer to be passed to @a func
 */
EAPI void      evas_object_event_callback_add(Evas_Object *obj, Evas_Callback_Type type, Evas_Object_Event_Cb func, const void *data) EINA_ARG_NONNULL(1, 3);

/**
 * @brief   Adds or registers a callback function to a given Evas object event with a
 *          non-default priority set. Except for the priority field, it is exactly the
 *          same as @ref evas_object_event_callback_add.
 * @since   1.1
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj       The object to attach a callback to
 * @param[in]   type      The type of event that triggers the callback
 * @param[in]   priority  The priority of the callback \n 
 *                    Lower values are called first.
 * @param[in]   func      The function to be called when the event is triggered
 * @param[in]   data      The data pointer to be passed to @a func
 *
 * @see evas_object_event_callback_add
 */
EAPI void      evas_object_event_callback_priority_add(Evas_Object *obj, Evas_Callback_Type type, Evas_Callback_Priority priority, Evas_Object_Event_Cb func, const void *data) EINA_ARG_NONNULL(1, 4);

/**
 * @brief   Deletes a callback function from an object.
 *
 * @details This function removes the most recently added callback from the
 *          object @a obj which is triggered by the event type @a type and is
 *          calling the function @a func when triggered. If the removal is
 *          successful it also returns the data pointer that is passed to
 *          evas_object_event_callback_add() when the callback is added to the
 *          object. If not successful @c NULL is returned.
 *
 * @since_tizen 2.3
 *
 * @remarks The following is an example:
 * @code
 * extern Evas_Object *object;
 * void *my_data;
 * void up_callback(void *data, Evas *e, Evas_Object *obj, void *event_info);
 *
 * my_data = evas_object_event_callback_del(object, EVAS_CALLBACK_MOUSE_UP, up_callback);
 * @endcode
 *
 * @param[in]   obj   The object to remove a callback from
 * @param[in]   type  The type of event triggering the callback
 * @param[in]   func  The function to be called when the event is triggered
 * @return  The data pointer to be passed to the callback
 */
EAPI void     *evas_object_event_callback_del(Evas_Object *obj, Evas_Callback_Type type, Evas_Object_Event_Cb func) EINA_ARG_NONNULL(1, 3);

/**
 * @brief   Deletes or unregisters a callback function registered to a given Evas object event.
 *
 * @details This function removes the most recently added callback from the
 *          object @a obj, which is triggered by the event type @a type and is
 *          calling the function @a func with data @a data, when triggered. If
 *          the removal is successful it also returns the data pointer that
 *          is passed to evas_object_event_callback_add() (that is the
 *          same as the parameter) when the callback is added to the
 *          object. In case of errors, @c NULL is returned.
 *
 * @since_tizen 2.3
 *
 * @remarks For deletion of Evas object events callbacks filtering by
 *          just type and function pointer, use evas_object_event_callback_del().
 *
 * @remarks The following is an example:
 * @code
 * extern Evas_Object *object;
 * void *my_data;
 * void up_callback(void *data, Evas *e, Evas_Object *obj, void *event_info);
 *
 * my_data = evas_object_event_callback_del_full(object, EVAS_CALLBACK_MOUSE_UP, up_callback, data);
 * @endcode
 *
 * @param[in]   obj   The object to remove a callback from
 * @param[in]   type  The type of event triggering the callback
 * @param[in]   func  The function to be called when the event is triggered
 * @param[in]   data  The data pointer to be passed to the callback
 * @return  The data pointer to be passed to the callback
 */
EAPI void     *evas_object_event_callback_del_full(Evas_Object *obj, Evas_Callback_Type type, Evas_Object_Event_Cb func, const void *data) EINA_ARG_NONNULL(1, 3);

/**
 * @brief   Sets whether an Evas object is to pass (ignore) events.
 *
 * @since_tizen 2.3
 *
 * @remarks If @a pass is #EINA_TRUE, it makes events on @a obj to be @b
 *          ignored. They instead are triggered on the @b next lower object that
 *          is not set to pass events. See evas_object_below_get().
 *
 * @remarks If @a pass is #EINA_FALSE, events are processed on that
 *          object as normal.
 *
 *
 * @param[in]   obj   The Evas object to operate on
 * @param[in]   pass  Set #EINA_TRUE for @a obj to pass events, \n 
 *                otherwise #EINA_FALSE not to pass events
 *
 * @see evas_object_pass_events_get() for an example
 * @see evas_object_repeat_events_set()
 * @see evas_object_propagate_events_set()
 * @see evas_object_freeze_events_set()
 */
EAPI void      evas_object_pass_events_set(Evas_Object *obj, Eina_Bool pass) EINA_ARG_NONNULL(1);

/**
 * @brief   Checks whether an object is set to pass (ignore) events.
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj  The Evas object to get information from
 * @return  #EINA_TRUE if @a obj is set to pass events, \n
 *          otherwise #EINA_FALSE if it is not set to pass events
 *
 * @see evas_object_pass_events_set()
 * @see evas_object_repeat_events_get()
 * @see evas_object_propagate_events_get()
 * @see evas_object_freeze_events_get()
 */
EAPI Eina_Bool evas_object_pass_events_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief   Sets whether an Evas object is to repeat events.
 *
 * @since_tizen 2.3
 *
 * @remarks If @a repeat is #EINA_TRUE, it makes events on @a obj to also
 *          be repeated for the @b next lower object in the objects' stack. See
 *          evas_object_below_get().
 *
 * @remarks If @a repeat is #EINA_FALSE, events occurring on @a obj are
 *          processed only on it.
 *
 * @param[in]   obj     The Evas object to operate on
 * @param[in]   repeat  Set #EINA_TRUE for @a obj to repeat events, \n 
 *                  otherwise set #EINA_FALSE
 *
 * @see evas_object_repeat_events_get()
 * @see evas_object_pass_events_set()
 * @see evas_object_propagate_events_set()
 * @see evas_object_freeze_events_set()
 */
EAPI void      evas_object_repeat_events_set(Evas_Object *obj, Eina_Bool repeat) EINA_ARG_NONNULL(1);

/**
 * @brief   Checks whether an object is set to repeat events.
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj  The given Evas object pointer
 * @return  #EINA_TRUE if @a obj is set to repeat events,
 *          otherwise #EINA_FALSE if it is not set to repeat events
 *
 * @see evas_object_repeat_events_set() for an example
 * @see evas_object_pass_events_get()
 * @see evas_object_propagate_events_get()
 * @see evas_object_freeze_events_get()
 */
EAPI Eina_Bool evas_object_repeat_events_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief   Sets whether events on a smart object's member should get propagated
 *          up to its parent.
 *
 * @details This function has @b no effect if @a obj is not a member of a smart object.
 *
 * @since_tizen 2.3
 *
 * @remarks If @a prop is #EINA_TRUE, events occurring on this object is
 *          propagated on to the smart object of which @a obj is a member.  If
 *          @a prop is #EINA_FALSE, events occurring on this object is @b
 *          not propagated on to the smart object of which @a obj is a
 *          member. The default value is #EINA_TRUE.
 *
 * @param[in]   obj   The smart object's child to operate on
 * @param[in]   prop  Set #EINA_TRUE to propagate events, \n 
 *                otherwise #EINA_FALSE to not propagate events
 *
 * @see evas_object_propagate_events_get()
 * @see evas_object_repeat_events_set()
 * @see evas_object_pass_events_set()
 * @see evas_object_freeze_events_set()
 */
EAPI void      evas_object_propagate_events_set(Evas_Object *obj, Eina_Bool prop) EINA_ARG_NONNULL(1);

/**
 * @brief   Checks whether an Evas object is set to propagate events.
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj  The given Evas object pointer
 * @return  #EINA_TRUE if @a obj is set to propagate events, \n
 *          otherwise #EINA_FALSE it is not set to propagate events
 *
 * @see evas_object_propagate_events_set()
 * @see evas_object_repeat_events_get()
 * @see evas_object_pass_events_get()
 * @see evas_object_freeze_events_get()
 */
EAPI Eina_Bool evas_object_propagate_events_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief   Sets whether an Evas object is to freeze (discard) events.
 * @since   1.1
 *
 * @since_tizen 2.3
 *
 * @remarks If @a freeze is #EINA_TRUE, it makes events on @a obj to be @b
 *          discarded. Unlike evas_object_pass_events_set(), events are not
 *          passed to @b next lower object. This API can be used for blocking
 *          events while @a obj is on transiting.
 *
 * @remarks If @a freeze is #EINA_FALSE, events are processed on that
 *          object as normal.
 *
 * @param[in]   obj     The Evas object to operate on
 * @param[in]   freeze  Set #EINA_TRUE for @a obj to freeze events, \n 
 *                  otherwise #EINA_FALSE not to freeze events
 *
 * @see evas_object_freeze_events_get()
 * @see evas_object_pass_events_set()
 * @see evas_object_repeat_events_set()
 * @see evas_object_propagate_events_set()
 */
EAPI void      evas_object_freeze_events_set(Evas_Object *obj, Eina_Bool freeze) EINA_ARG_NONNULL(1);

/**
 * @brief   Checks whether an object is set to freeze (discard) events.
 * @since   1.1
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj  The Evas object to get information from
 * @return  #EINA_TRUE if @a obj is set to freeze events, \n 
 *          otherwise #EINA_FALSE if it is not set to freeze events
 *
 * @see evas_object_freeze_events_set()
 * @see evas_object_pass_events_get()
 * @see evas_object_repeat_events_get()
 * @see evas_object_propagate_events_get()
 */
EAPI Eina_Bool evas_object_freeze_events_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @}
 */

/**
 * @defgroup Evas_Object_Group_Map UV Mapping (Rotation, Perspective, 3D...)
 * @ingroup Evas_Object_Group
 *
 * @brief   This group provides functions for UV mapping.
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
 * As usual, Evas provides both raw and easy to use methods. The
 * raw methods allow developers to create their maps somewhere else,
 * possibly loading them from some file format. The easy to use methods
 * calculate the points given some high-level parameters such as
 * rotation angle, ambient light, and so on.
 *
 * @remarks Applying mapping reduces performance, so use with
 *          care. The impact on performance depends on the engine in
 *          use. The software is quite optimized, but not as fast as OpenGL.
 *
 * @section sec-map-points Map points
 * @subsection subsec-rotation Rotation
 *
 * A map consists of a set of points, currently only four are supported. Each
 * of these points contains a set of canvas coordinates @c x and @c y that
 * can be used to alter the geometry of the mapped object, and a @c z
 * coordinate that indicates the depth of that point. This last coordinate
 * does not normally affect the map, but it is used by several of the utility
 * functions to calculate the right position of the point given other
 * parameters.
 *
 * The coordinates for each point are set with evas_map_point_coord_set().
 * The following image shows a map set to match the geometry of an existing object.
 *
 * @image html map-set-map-points-1.png
 * @image rtf map-set-map-points-1.png
 * @image latex map-set-map-points-1.eps
 *
 * This is a common practice, so there are a few functions that help make it easier.
 *
 * evas_map_util_points_populate_from_geometry() sets the coordinates of each
 * point in the given map to match the rectangle defined by the function parameters.
 *
 * evas_map_util_points_populate_from_object() and
 * evas_map_util_points_populate_from_object_full() both take an object and
 * set the map points to match its geometry. The difference between the two
 * is that the first function sets the @c z value of all points to @c 0, while
 * the latter receives the value to set in said coordinate as a parameter.
 *
 * The following lines of code all produce the same result as in the image above.
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
 * In all three cases above, setting the map to be used by the object is the same.
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
 * which rotates the object around its center point in a 45 degree angle
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
 * parameters of the evas_map_util_rotate() function to the right values. A
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
 * clockwise fashion. The next image shows this, with each point showing each
 * corner of the object with which it is identified within the map.
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
 * This has stronger implications later when we talk about lighting.
 *
 * To know if a map is facing towards the user or not it is enough to use
 * the evas_map_util_clockwise_get() function, but this is normally done
 * after all the other operations are applied on the map.
 *
 * @subsection subsec-3d-rot 3D rotation and perspective
 *
 * Much like evas_map_util_rotate(), there is the function
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
 * which does not look very real. This can be helped by adding perspective
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
 * Each point in a map can be set to a color, which is multiplied with
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
 * If the map is defined counter-clockwise, the object faces away
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
 * of objects and do almost anything with them, but it is completely oblivious
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
 * Once Evas knows how to handle the source image within the map, it
 * transforms it as needed. This is done with evas_map_point_image_uv_set(),
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
 * @image html map-uv-mapping-3.png
 * @image rtf map-uv-mapping-3.png
 * @image latex map-uv-mapping-3.eps
 *
 * Maps can also be set to use part of an image only, or even map them inverted,
 * and when combined with evas_object_image_source_set() it can be used to achieve
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
 * @{
 */

/**
 * @brief   Enables or disables the map that is set.
 *
 * @details This function enables or disables the use of map for the object @a obj.
 *          When enabled, the object geometry is saved, and the new geometry
 *          changes (position and size) to reflect the map geometry set.
 *
 * @since_tizen 2.3
 *
 * @remarks If the object does not have a map set (with evas_object_map_set()), the
 *          initial geometry is undefined. It is advised to always set a map
 *          to the object first, and then call this function to enable its use.
 *
 * @param[in]   obj      The object to enable the map on
 * @param[in]   enabled  Set #EINA_TRUE to enable the map, \n
 *                   otherwise #EINA_FALSE to not enable the map
 */
EAPI void            evas_object_map_enable_set(Evas_Object *obj, Eina_Bool enabled);

/**
 * @brief   Checks whether the map is enabled.
 *
 * @details This function returns the currently enabled state of the map on the object indicated.
 *          The default map enable state is off. You can enable and disable it with
 *          evas_object_map_enable_set().
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj  The object to get the map enabled state from
 * @return  #EINA_TRUE of the map is enabled, \n
 *          otherwise #EINA_FALSE of the map is not enabled
 */
EAPI Eina_Bool       evas_object_map_enable_get(const Evas_Object *obj);

/**
 * @brief   Sets current object transformation map.
 *
 * @details This function sets the map on a given object. It is copied from the @a map pointer,
 *          so there is no need to keep the @a map object if you do not need it anymore.
 *
 * @since_tizen 2.3
 *
 * @remarks A map is a set of 4 points which have canvas @c x, @c y coordinates per point,
 *          with an optional @c z point value as a hint for perspective correction, if it
 *          is available. As well each point has @c u and @c v coordinates. These are like
 *          "texture coordinates" in OpenGL in that they define a point in the source
 *          image that is mapped to that map vertex or point. The @c u corresponds to the @c x
 *          coordinate of this mapped point and @c v, the @c y coordinate. Note that these
 *          coordinates describe a bounding region to sample. If you have a 200x100
 *          source image and want to display it at 200x100 with proper pixel
 *          precision, then do the following:
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
 * @remarks Note that the map points a uv coordinates that match the image geometry. If
 *          the @a map parameter is @c NULL, the stored map is freed and geometry
 *          prior to enabling/setting a map is restored.
 *
 * @param[in]   obj  The object to change transformation map
 * @param[in]   map  The new map to use
 *
 * @see evas_map_new()
 */
EAPI void            evas_object_map_set(Evas_Object *obj, const Evas_Map *map);

/**
 * @brief   Gets the current object transformation map.
 *
 * @details This function returns the current internal map set on the indicated object. It is
 *          intended for read-only access and is only valid as long as the object is
 *          not deleted or the map on the object is not changed. If you wish to modify
 *          the map and set it back do the following:
 *
 * @since_tizen 2.3
 *
 * @code
 * const Evas_Map *m = evas_object_map_get(obj);
 * Evas_Map *m2 = evas_map_dup(m);
 * evas_map_util_rotate(m2, 30.0, 0, 0);
 * evas_object_map_set(obj, m2);
 * evas_map_free(m2);
 * @endcode
 *
 * @param[in]   obj  The object to query transformation map
 * @return  The map reference to the map in use \n 
 *          This is an internal data structure, so do not modify it.
 *
 * @see evas_object_map_set()
 */
EAPI const Evas_Map *evas_object_map_get(const Evas_Object *obj);

/**
 * @brief   Populates source and destination map points to exactly match the object.
 *
 * @since_tizen 2.3
 *
 * @remarks You initialize the map of an object to match its original
 *          position and size, then transform these with evas_map_util_*
 *          functions, such as evas_map_util_rotate() or
 *          evas_map_util_3d_rotate(). The original set is done by this
 *          function, avoiding code duplication all around.
 *
 * @param[in]   m    The map to change all 4 points (must be of size 4)
 * @param[in]   obj  The object to use unmapped geometry to populate map coordinates
 * @param[in]   z    The point Z coordinate hint (pre-perspective transform) \n 
 *               This value is used for all four points.
 *
 * @see evas_map_util_points_populate_from_object()
 * @see evas_map_point_coord_set()
 * @see evas_map_point_image_uv_set()
 */
EAPI void            evas_map_util_points_populate_from_object_full(Evas_Map *m, const Evas_Object *obj, Evas_Coord z);

/**
 * @brief   Populates the source and destination map points to exactly match the object.
 *
 * @since_tizen 2.3
 *
 * @remarks You initialize map of an object to match its original
 *          position and size, then transform these with evas_map_util_*
 *          functions, such as evas_map_util_rotate() or
 *          evas_map_util_3d_rotate(). The original set is done by this
 *          function, avoiding code duplication all around.
 *
 * @remarks The Z point coordinate is assumed as @c 0 (zero).
 *
 * @param[in]   m    The map to change all 4 points (must be of size 4)
 * @param[in]   obj  The object to use unmapped geometry to populate map coordinates
 *
 * @see evas_map_util_points_populate_from_object_full()
 * @see evas_map_util_points_populate_from_geometry()
 * @see evas_map_point_coord_set()
 * @see evas_map_point_image_uv_set()
 */
EAPI void            evas_map_util_points_populate_from_object(Evas_Map *m, const Evas_Object *obj);

/**
 * @brief   Populates the source and destination map points to match the given geometry.
 *
 * @since_tizen 2.3
 *
 * @remarks Similar to evas_map_util_points_populate_from_object_full(), this
 *          call takes raw values instead of querying object's unmapped
 *          geometry. The given width is used to calculate destination
 *          points (evas_map_point_coord_set()) and set the image uv
 *          (evas_map_point_image_uv_set()).
 *
 * @param[in]   m  The map to change all 4 points (must be of size 4)
 * @param[in]   x  The X coordinate
 * @param[in]   y  The Y coordinate
 * @param[in]   w  The width to use to calculate second and third points
 * @param[in]   h  The height to use to calculate third and fourth points
 * @param[in]   z  The Z coordinate hint (pre-perspective transform) \n 
 *             This value is used for all four points.
 *
 * @see evas_map_util_points_populate_from_object()
 * @see evas_map_point_coord_set()
 * @see evas_map_point_image_uv_set()
 */
EAPI void            evas_map_util_points_populate_from_geometry(Evas_Map *m, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h, Evas_Coord z);

/**
 * @brief   Sets the given color for all the points.
 *
 * @details This function is useful to reuse maps after they had 3D lightning or
 *          any other colorization applied before.
 *
 * @since_tizen 2.3
 *
 * @param[in]   m  The map to change the color of
 * @param[in]   r  The red color (0 - 255)
 * @param[in]   g  The green color (0 - 255)
 * @param[in]   b  The blue color (0 - 255)
 * @param[in]   a  The alpha color (0 - 255)
 *
 * @see evas_map_point_color_set()
 */
EAPI void            evas_map_util_points_color_set(Evas_Map *m, int r, int g, int b, int a);

/**
 * @brief   Changes the map to apply the given rotation.
 *
 * @details This function rotates the indicated map's coordinates around the center coordinate
 *          given by @a cx and @a cy as the rotation center. The points have their
 *          X and Y coordinates rotated clockwise by @a degrees degrees (@c 360.0 is a
 *          full rotation). Negative values for degrees rotate counter-clockwise
 *          by that amount. All coordinates are canvas global coordinates.
 *
 * @since_tizen 2.3
 *
 * @param[in]   m        The map to change
 * @param[in]   degrees  The amount of degrees from @c 0.0 to @c 360.0 to rotate
 * @param[in]   cx       The rotation's center horizontal position
 * @param[in]   cy       The rotation's center vertical position
 *
 * @see evas_map_point_coord_set()
 * @see evas_map_util_zoom()
 */
EAPI void            evas_map_util_rotate(Evas_Map *m, double degrees, Evas_Coord cx, Evas_Coord cy);

/**
 * @brief   Changes the map to apply the given zooming.
 *
 * @since_tizen 2.3
 *
 * @remarks Like evas_map_util_rotate(), this zooms the points of the map from a center
 *          point. That center is defined by @a cx and @a cy. The @a zoomx and @a zoomy
 *          parameters specify how much to zoom in the X and Y direction respectively.
 *          A value of @c 1.0 means "don't zoom", @c 2.0 means "double the size", @c 0.5 is
 *          "half the size", and so on. All coordinates are canvas global coordinates.
 *
 * @param[in]   m      The map to change
 * @param[in]   zoomx  The horizontal zoom to use
 * @param[in]   zoomy  The vertical zoom to use
 * @param[in]   cx     The zooming center horizontal position
 * @param[in]   cy     The zooming center vertical position
 *
 * @see evas_map_point_coord_set()
 * @see evas_map_util_rotate()
 */
EAPI void            evas_map_util_zoom(Evas_Map *m, double zoomx, double zoomy, Evas_Coord cx, Evas_Coord cy);

/**
 * @brief   Rotates the map around 3 axes in 3D.
 *
 * @details This function rotates not just around the "Z" axis as in evas_map_util_rotate()
 *          (which is a convenience call for those only wanting 2D). This rotates
 *          around the X, Y and Z axes. The Z axis points "into" the screen with low
 *          values at the screen and higher values further away. The X axis runs from
 *          left to right on the screen and the Y axis from top to bottom. Like with
 *          evas_map_util_rotate() you provide a center point to rotate around (in 3D).
 *
 * @since_tizen 2.3
 *
 * @param[in]  m   The map to change
 * @param[in]  dx  The amount of degrees from @c 0.0 to @c 360.0 to rotate around X axis
 * @param[in]  dy  The amount of degrees from @c 0.0 to @c 360.0 to rotate around Y axis
 * @param[in]  dz  The amount of degrees from @c 0.0 to @c 360.0 to rotate around Z axis
 * @param[in]  cx  The rotation's center horizontal position
 * @param[in]  cy  The rotation's center vertical position
 * @param[in]  cz  The rotation's center vertical position
 */
EAPI void            evas_map_util_3d_rotate(Evas_Map *m, double dx, double dy, double dz, Evas_Coord cx, Evas_Coord cy, Evas_Coord cz);

/**
 * @brief   Rotates the map in 3D using a unit quaternion.
 *
 * @details This function rotates in 3D using a unit quaternion. Like with
 *          evas_map_util_3d_rotate() you provide a center point 
 *          to rotate around (in 3D).
 * @since   1.8
 *
 * @since_tizen 2.3
 *
 * @remarks Rotations can be done using a unit quaternion. Thus, this
 *          function expects a unit quaternion (i.e. qx² + qy² + qz² + qw² == 1).
 *          If this is not the case the behavior is undefined.
 *
 * @param[in]   m   The map to change
 * @param[in]   qx  The x component of the imaginary part of the quaternion
 * @param[in]   qy  The y component of the imaginary part of the quaternion
 * @param[in]   qz  The z component of the imaginary part of the quaternion
 * @param[in]   qw  The w component of the real part of the quaternion
 * @param[in]   cx  The rotation's center x
 * @param[in]   cy  The rotation's center y
 * @param[in]   cz  The rotation's center z
 *
 */
EAPI void            evas_map_util_quat_rotate(Evas_Map *m, double qx, double qy, double qz, double qw, double cx, double cy, double cz);

/**
 * @brief   Performs lighting calculations on the given map.
 *
 * @details This function is used to apply lighting calculations (from a single light source)
 *          to a given map. The R, G and B values of each vertex is modified to
 *          reflect the lighting based on the lixth point coordinates, the light
 *          color and the ambient color, and at what angle the map is facing the
 *          light source. A surface should have its points declared in a
 *          clockwise fashion if the face is "facing" towards you (as opposed to
 *          away from you) as faces have a "logical" side for lighting.
 *
 * @since_tizen 2.3
 *
 * @image html map-light3.png
 * @image rtf map-light3.png
 * @image latex map-light3.eps
 * @remarks Grey object, no lighting used
 *
 * @image html map-light4.png
 * @image rtf map-light4.png
 * @image latex map-light4.eps
 * @remarks Lights out! Every color set to @c 0
 *
 * @image html map-light5.png
 * @image rtf map-light5.png
 * @image latex map-light5.eps
 * @remarks Ambient light to full black, red light coming from close at the
 *          bottom-left vertex.
 *
 * @image html map-light6.png
 * @image rtf map-light6.png
 * @image latex map-light6.eps
 * @remarks Same light as before, but not the light is set to 0 and ambient light
 *          is cyan.
 *
 * @image html map-light7.png
 * @image rtf map-light7.png
 * @image latex map-light7.eps
 * @remarks Both lights are on.
 *
 * @image html map-light8.png
 * @image rtf map-light8.png
 * @image latex map-light8.eps
 * @remarks Both lights again, but this time both are the same color.
 *
 * @param[in]   m   The map to change
 * @param[in]   lx  The X coordinate in space of light point
 * @param[in]   ly  The Y coordinate in space of light point
 * @param[in]   lz  The Z coordinate in space of light point
 * @param[in]   lr  The light red value (0 - 255)
 * @param[in]   lg  The light green value (0 - 255)
 * @param[in]   lb  The light blue value (0 - 255)
 * @param[in]   ar  The ambient color red value (0 - 255)
 * @param[in]   ag  The ambient color green value (0 - 255)
 * @param[in]   ab  The ambient color blue value (0 - 255)
 */
EAPI void            evas_map_util_3d_lighting(Evas_Map *m, Evas_Coord lx, Evas_Coord ly, Evas_Coord lz, int lr, int lg, int lb, int ar, int ag, int ab);

/**
 * @brief   Applies a perspective transform to the map.
 *
 * @details This function applies a given perspective (3D) to the map coordinates. X, Y and Z
 *          values are used. The px and py points specify the "infinite distance" point
 *          in the 3D conversion (where all lines converge to, like when artists draw
 *          3D by hand). The @a z0 value specifies the z value at which there is a 1:1
 *          mapping between spatial coordinates and screen coordinates. Any points
 *          on this z value do not have their X and Y values modified in the transform.
 *          Those further away (Z value higher) shrink into the distance, and
 *          those less than this value expand and become bigger. The @a foc value
 *          determines the "focal length" of the camera. This is in reality the distance
 *          between the camera lens plane itself (at or closer than this rendering
 *          results are undefined) and the "z0" z value. This allows for some "depth"
 *          control and @a foc must be greater than @c 0.
 *
 * @since_tizen 2.3
 *
 * @param[in]   m    The map to change
 * @param[in]   px   The perspective distance X coordinate
 * @param[in]   py   The perspective distance Y coordinate
 * @param[in]   z0   The "0" z plane value
 * @param[in]   foc  The focal distance
 */
EAPI void            evas_map_util_3d_perspective(Evas_Map *m, Evas_Coord px, Evas_Coord py, Evas_Coord z0, Evas_Coord foc);

/**
 * @brief   Gets the clockwise state of a map.
 *
 * @details This function determines if the output points (X and Y. Z is not used) are
 *          clockwise or counter-clockwise. This can be used for "back-face culling". This
 *          is where you hide objects that "face away" from you, in this case, the objects
 *          that are not clockwise.
 *
 * @since_tizen 2.3
 *
 * @param[in]   m  The map to query
 * @return  #EINA_TRUE if the output points are clockwise, 
 *          otherwise #EINA_FALSE if the output points are not clockwise
 */
EAPI Eina_Bool       evas_map_util_clockwise_get(Evas_Map *m);

/**
 * @brief   Creates a map of transformation points to be later used with an Evas object.
 *
 * @details This function creates a set of points (currently only 4 is supported and no other
 *          number for @a count works). That is empty and ready to be modified
 *          with evas_map calls.
 *
 * @since_tizen 2.3
 *
 * @param[in]   count  The number of points in the map
 * @return  The newly allocated map, \n
 *          otherwise @c NULL on errors
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
 * @brief   Sets the smoothing for map rendering.
 *
 * @details This function sets smoothing for map rendering. If the object is a type that has
 *          its own smoothing settings, then both the smooth settings for this object
 *          and the map must be turned off. By default, smooth maps are enabled.
 *
 * @since_tizen 2.3
 *
 * @param[in]   m        The map to modify \n 
 *                   This must not be @c NULL.
 * @param[in]   enabled  Set #EINA_TRUE to enable smooth map rendering, \n
 *                   otherwise set #EINA_FALSE not to enable smooth map rendering
 */
EAPI void            evas_map_smooth_set(Evas_Map *m, Eina_Bool enabled);

/**
 * Checks whether the smoothing for map rendering is enabled.
 *
 * @details  This gets smoothing for map rendering.
 *
 * @since_tizen 2.3
 *
 * @param[in]    m  The map for which to get the smoothing status \n 
 *              This must not be @c NULL.
 * @return   #EINA_TRUE if smoothing for map rendering is enabled, \n
 *           otherwise #EINA_FALSE if smoothing for map rendering is not enabled
 */
EAPI Eina_Bool       evas_map_smooth_get(const Evas_Map *m);

/**
 * @brief   Sets the alpha flag for map rendering.
 *
 * @details This function sets alpha flag for map rendering. If the object is a type that has
 *          its own alpha settings, then this takes precedence. Only image objects
 *          have this currently. Setting this off stops alpha blending of the map area, and is
 *          useful if you know the object and/or all sub-objects is 100% solid.
 *
 * @since_tizen 2.3
 *
 * @param[in]   m        The map to modify \n 
 *                   This must not be @c NULL.
 * @param[in]   enabled  Set #EINA_TRUE to enable alpha map rendering, \n
 *                   otheriwse set #EINA_FALSE to disable alpha map rendering
 */
EAPI void            evas_map_alpha_set(Evas_Map *m, Eina_Bool enabled);

/**
 * @brief   Gets the alpha flag for map rendering.
 *
 * @details This gets the alpha flag for map rendering.
 *
 * @since_tizen 2.3
 *
 * @param[in]   m  The map to get the alpha from \n
 *             This must not be @c NULL.
 * @return The alpha flag
 */
EAPI Eina_Bool       evas_map_alpha_get(const Evas_Map *m);

/**
 * @brief   Copies a previously allocated map.
 *
 * @details This makes a duplicate of the @a m object and returns it.
 *
 * @since_tizen 2.3
 *
 * @param[in]   m  The map to copy \n 
 *             This must not be @c NULL.
 * @return  The newly allocated map with the same count and contents as @a m
 */
EAPI Evas_Map       *evas_map_dup(const Evas_Map *m);

/**
 * @brief   Frees a previously allocated map.
 *
 * @details This function frees a given map @a m and all memory associated with it. You must NOT
 *          free a map returned by evas_object_map_get() as this is internal.
 *
 * @since_tizen 2.3
 *
 * @param[in]   m  The map to free
 */
EAPI void            evas_map_free(Evas_Map *m);

/**
 * @brief   Gets a maps size.
 *
 * @since_tizen 2.3
 *
 * @remarks This function returns the number of points in a map. It should be at least be @c 4.
 *
 * @param[in]   m  The map to get the size
 * @return  The number of points in a map, \n
 *          otherwise @c -1 on error
 */
EAPI int             evas_map_count_get(const Evas_Map *m) EINA_CONST;

/**
 * @brief   Changes the map point's coordinate.
 *
 * @details This function sets the fixed point's coordinate in the map. Note that points
 *          describe the outline of a quadrangle and are ordered either clockwise
 *          or counter-clockwise. It is suggested to keep your quadrangles concave and
 *          non-complex, though these polygon modes may work, they may not render
 *          a desired set of output. The quadrangle uses points @c 0 and @c 1 , @c 1 and @c 2,
 *          @c 2 and @c 3, and @c 3 and @c 0 to describe the edges of the quadrangle.
 *
 * @since_tizen 2.3
 *
 * @remarks The X, Y, and Z coordinates are in canvas units. Z is optional and may
 *          or may not be honored in drawing. Z is a hint and does not affect the
 *          X and Y rendered coordinates. It may be used for calculating fills with
 *          perspective correct rendering.
 *
 * @remarks Remember all coordinates are canvas global ones like with move and resize in evas.
 *
 * @param[in]   m    The map to change point \n 
 *               This must not be @c NULL.
 * @param[in]   idx  The index of point to change \n 
 *               This must be smaller than map size.
 * @param[in]   x    The X coordinate
 * @param[in]   y    The Y coordinate
 * @param[in]   z    The Z coordinate hint (pre-perspective transform)
 *
 * @see evas_map_util_rotate()
 * @see evas_map_util_zoom()
 * @see evas_map_util_points_populate_from_object_full()
 * @see evas_map_util_points_populate_from_object()
 */
EAPI void            evas_map_point_coord_set(Evas_Map *m, int idx, Evas_Coord x, Evas_Coord y, Evas_Coord z);

/**
 * @brief   Gets the map point's coordinate.
 *
 * @details This function returns the coordinates of the given point in the map.
 *
 * @since_tizen 2.3
 *
 * @param[in]   m    The map to query point
 * @param[in]   idx  The index of point to query \n 
 *               This must be smaller than the map size.
 * @param[out]   x    The X coordinate
 * @param[out]   y    The Y coordinate
 * @param[out]   z    The Z coordinate
 */
EAPI void            evas_map_point_coord_get(const Evas_Map *m, int idx, Evas_Coord *x, Evas_Coord *y, Evas_Coord *z);

/**
 * @brief   Changes the map point's U and V texture source point.
 *
 * @details This sets the U and V coordinates for the point. This determines which
 *          coordinate in the source image is mapped to the given point, much like
 *          OpenGL and textures. Note that these points do select the pixel, but
 *          are double floating point values to allow for accuracy and sub-pixel selection.
 *
 * @since_tizen 2.3
 *
 * @param[in]   m    The map to change the point of
 * @param[in]   idx  The index of point to change \n 
 *               This must be smaller than map size.
 * @param[in]   u    The X coordinate within the image or texture source
 * @param[in]   v    The Y coordinate within the image or texture source
 *
 * @see evas_map_point_coord_set()
 * @see evas_object_map_set()
 * @see evas_map_util_points_populate_from_object_full()
 * @see evas_map_util_points_populate_from_object()
 */
EAPI void            evas_map_point_image_uv_set(Evas_Map *m, int idx, double u, double v);

/**
 * @brief   Gets the map point's U and V texture source points.
 *
 * @details This function returns the texture points set by evas_map_point_image_uv_set().
 *
 * @since_tizen 2.3
 *
 * @param[in]   m    The map to query point
 * @param[in]   idx  The index of point to query \n 
 *               This must be smaller than map size.
 * @param[out]   u    The X coordinate within the image or texture source
 * @param[out]   v    The Y coordinate within the image or texture source
 */
EAPI void            evas_map_point_image_uv_get(const Evas_Map *m, int idx, double *u, double *v);

/**
 * @brief   Sets the color of a vertex in the map.
 *
 * @details This sets the color of the vertex in the map. Colors are linearly
 *          interpolated between vertex points through the map. Color multiplies
 *          the "texture" pixels (like GL_MODULATE in OpenGL). The default color of
 *          a vertex in a map is white solid (255, 255, 255, 255) which means it
 *          has no affect on modifying the texture pixels.
 *
 * @since_tizen 2.3
 *
 * @param[in]   m    The map to change the color of
 * @param[in]   idx  The index of point to change \n 
 *               This must be smaller than the map size.
 * @param[in]   r    The red color (0 - 255)
 * @param[in]   g    The green color (0 - 255)
 * @param[in]   b    The blue color (0 - 255)
 * @param[in]   a    The alpha color (0 - 255)
 *
 * @see evas_map_util_points_color_set()
 * @see evas_map_point_coord_set()
 * @see evas_object_map_set()
 */
EAPI void            evas_map_point_color_set(Evas_Map *m, int idx, int r, int g, int b, int a);

/**
 * @brief   Gets the color set on a vertex in the map.
 *
 * @details This function gets the color set by evas_map_point_color_set() on the given vertex
 *          of the map.
 *
 * @since_tizen 2.3
 *
 * @param[in]   m    The map to get the color of the vertex from
 * @param[in]   idx  The index of point get \n 
 *               This must be smaller than the map size.
 * @param[out]   r    The pointer to red color
 * @param[out]   g    The pointer to green color
 * @param[out]   b    The pointer to blue color
 * @param[out]   a    The pointer to alpha color (0 - 255)
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
 * @ingroup Evas_Object_Group
 *
 * @brief  This group provides functions for size hints.
 *
 * Objects may carry hints, so that another object that acts as a
 * manager (see @ref Evas_Smart_Object_Group) may know how to properly
 * position and resize its subordinate objects. The Size Hints provide
 * a common interface that is recommended as the protocol for such
 * information.
 *
 * For example, box objects use alignment hints to align its
 * lines or columns inside its container, padding hints to set the
 * padding between each individual child, and so on.
 *
 * @{
 */

/**
 * @brief   Gets the hints for an object's minimum size.
 *
 * @since_tizen 2.3
 *
 * @remarks These are hints on the minimim sizes @a obj should have. This is
 *          not a size enforcement in any way. It is just a hint that should be
 *          used whenever appropriate.
 *
 * @remarks Use @c NULL pointers on the hint components that you are not
 *          interested in: they are ignored by the function.
 *
 * @param[in]   obj  The given Evas object to query hints from
 * @param[out]   w    The pointer to an integer in which to store the minimum width
 * @param[out]   h    The pointer to an integer in which to store the minimum height
 *
 * @see evas_object_size_hint_min_set() for an example
 */
EAPI void evas_object_size_hint_min_get(const Evas_Object *obj, Evas_Coord *w, Evas_Coord *h) EINA_ARG_NONNULL(1);

/**
 * @brief   Sets the hints for an object's minimum size.
 *
 * @since_tizen 2.3
 *
 * @remarks This is not a size enforcement in any way. It is just a hint that
 *          should be used whenever appropriate.
 *
 * @remarks Values like @c 0 are treated as unset hint components, when queried
 *          by managers.
 *
 * @param[in]   obj  The given Evas object to query hints from
 * @param[in]   w    The integer to use as the minimum width hint
 * @param[in]   h    The integer to use as the minimum height hint
 *
 * @see evas_object_size_hint_min_get()
 */
EAPI void evas_object_size_hint_min_set(Evas_Object *obj, Evas_Coord w, Evas_Coord h) EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the hints for an object's maximum size.
 *
 * @since_tizen 2.3
 *
 * @remarks These are hints on the maximum sizes @a obj should have. This is
 *          not a size enforcement in any way. It is just a hint that should be
 *          used whenever appropriate.
 *
 * @remarks Use @c NULL pointers on the hint components that you are not
 *          interested in: they are ignored by the function.
 *
 * @param[in]   obj  The given Evas object to query hints from
 * @param[out]   w    The pointer to an integer in which to store the maximum width
 * @param[out]   h    The pointer to an integer in which to store the maximum height
 * @see evas_object_size_hint_max_set()
 */
EAPI void evas_object_size_hint_max_get(const Evas_Object *obj, Evas_Coord *w, Evas_Coord *h) EINA_ARG_NONNULL(1);

/**
 * @brief   Sets the hints for an object's maximum size.
 *
 * @since_tizen 2.3
 *
 * @remarks This is not a size enforcement in any way. It is just a hint that
 *          should be used whenever appropriate.
 *
 * @remarks Values like @c -1 are treated as unset hint components, when queried
 *          by managers.
 *
 * @param[in]   obj  The given Evas object to query hints from
 * @param[in]   w    The integer to use as the maximum width hint
 * @param[in]   h    The integer to use as the maximum height hint
 * @see evas_object_size_hint_max_get()
 */
EAPI void evas_object_size_hint_max_set(Evas_Object *obj, Evas_Coord w, Evas_Coord h) EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the hints for an object's display mode.
 *
 * @since_tizen 2.3
 *
 * @remarks These are hints on the display mode @a obj. This is
 *          not a size enforcement in any way. It is just a hint that can be
 *          used whenever appropriate. This mode can be used with object's display 
 *          mode like compress or expand.
 *
 * @param[in]   obj  The given Evas object to query hints from
 * @return The display mode hints
 *
 * @see evas_object_size_hint_display_mode_set()
 */
EAPI Evas_Display_Mode evas_object_size_hint_display_mode_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * @brief   Sets the hints for an object's display mode.
 *
 * @since_tizen 2.3
 *
 * @remarks This is not a size enforcement in any way. It is just a hint that
 *          can be used whenever appropriate.
 *
 * @param[in]   obj       The given Evas object to query hints from
 * @param[in]   dispmode  The display mode hint
 *
 * @see evas_object_size_hint_display_mode_get()
 */
EAPI void evas_object_size_hint_display_mode_set(Evas_Object *obj, Evas_Display_Mode dispmode) EINA_ARG_NONNULL(1);

/**
 * @internal
 * @brief   Gets the hints for an object's optimum size.
 *
 * @since_tizen 2.3
 *
 * @remarks These are hints on the optimum sizes @a obj should have. This is
 *          not a size enforcement in any way. It is just a hint that should be
 *          used whenever appropriate.
 *
 * @remarks Use @c NULL pointers on the hint components that you are not
 *          interested in: they are ignored by the function.
 *
 * @param[in]   obj  The given Evas object to query hints from
 * @param[out]   w    The pointer to an integer in which to store the requested width
 * @param[out]   h    The pointer to an integer in which to store the requested height
 *
 * @see evas_object_size_hint_request_set()
 */
EAPI void evas_object_size_hint_request_get(const Evas_Object *obj, Evas_Coord *w, Evas_Coord *h) EINA_ARG_NONNULL(1);

/**
 * @internal
 * @brief   Sets the hints for an object's optimum size.
 *
 * @since_tizen 2.3
 *
 * @remarks This is not a size enforcement in any way. It is just a hint that
 *          should be used whenever appropriate.
 *
 * @remarks Values like @c 0 are treated as unset hint components, when queried
 *          by managers.
 *
 * @param[in]   obj  The given Evas object to query hints from
 * @param[in]   w    The integer to use as the preferred width hint
 * @param[in]   h    The integer to use as the preferred height hint
 *
 * @see evas_object_size_hint_request_get()
 */
EAPI void evas_object_size_hint_request_set(Evas_Object *obj, Evas_Coord w, Evas_Coord h) EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the hints for an object's aspect ratio.
 *
 * @since_tizen 2.3
 *
 * @remarks The different aspect ratio policies are documented in the
 *          #Evas_Aspect_Control type. A container respecting these size hints
 *          would @b resize its children accordingly to those policies.
 *
 * @remarks For any policy, if any of the given aspect ratio terms are @c 0,
 *          the object's container should ignore the aspect and scale @a obj to
 *          occupy the whole available area. If they are both positive
 *          integers, that proportion is respected, under each scaling policy.
 *
 * @remarks These images illustrate some of the #Evas_Aspect_Control policies:
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
 * @remarks This is not a size enforcement in any way. It is just a hint that
 *          should be used whenever appropriate.
 *
 * @remarks Use @c NULL pointers on the hint components that you are not
 *          interested in: they are ignored by the function.
 *
 * @param[in]   obj     The given Evas object to query hints from
 * @param[out]   aspect  The policy or type of aspect ratio applied to @a obj that is returned
 * @param[out]   w       The pointer to an integer in which to store the aspect's width
 *                  ratio term
 * @param[out]   h       The pointer to an integer in which to store the aspect's
 *                  height ratio term
 *
 * @see evas_object_size_hint_aspect_set()
 */
EAPI void evas_object_size_hint_aspect_get(const Evas_Object *obj, Evas_Aspect_Control *aspect, Evas_Coord *w, Evas_Coord *h) EINA_ARG_NONNULL(1);

/**
 * @brief   Sets the hints for an object's aspect ratio.
 *
 * @since_tizen 2.3
 *
 * @remarks This is not a size enforcement in any way. It is just a hint that should
 *          be used whenever appropriate.
 *
 * @remarks If any of the given aspect ratio terms are @c 0,
 *          the object's container ignores the aspect and scale @a obj to
 *          occupy the whole available area, for any given policy.
 *
 * @param[in]   obj     The given Evas object to query hints from
 * @param[in]   aspect  The policy or type of aspect ratio to apply to @a obj
 * @param[in]   w       The integer to use as aspect width ratio term
 * @param[in]   h       The integer to use as aspect height ratio term
 *
 * @see evas_object_size_hint_aspect_get() for more information.
 */
EAPI void evas_object_size_hint_aspect_set(Evas_Object *obj, Evas_Aspect_Control aspect, Evas_Coord w, Evas_Coord h) EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the hints for the object's alignment.
 *
 * @since_tizen 2.3
 *
 * @remarks This is not a size enforcement in any way. It is just a hint that
 *          should be used whenever appropriate.
 *
 * @remarks Use @c NULL pointers on the hint components that you are not
 *          interested in: they are ignored by the function.
 * @remarks If @c obj is invalid, then the hint components are set with @c 0.5
 *
 * @param[in]   obj  The given Evas object to query hints from
 * @param[out]   x    The pointer to a double in which to store the horizontal alignment hint
 * @param[out]   y    The pointer to a double in which to store the vertical alignment hint
 *
 * @see evas_object_size_hint_align_set() for more information
 */
EAPI void evas_object_size_hint_align_get(const Evas_Object *obj, double *x, double *y) EINA_ARG_NONNULL(1);

/**
 * @brief   Sets the hints for an object's alignment.
 *
 * @since_tizen 2.3
 *
 * @remarks These are hints on how to align an object <b>inside the boundaries
 *          of a container/manager</b>. Accepted values are in the @c 0.0 to @c
 *          1.0 range, with the special value #EVAS_HINT_FILL used to specify
 *          "justify" or "fill" by some users. In this case, maximum size hints
 *          should be enforced with higher priority, if they are set. Also, any
 *          padding hint set on objects should add up to the alignment space on
 *          the final scene composition.
 *
 * @remarks See documentation of possible users: in Evas, they are the @ref
 *          Evas_Object_Box "box" and @ref Evas_Object_Table "table" smart objects.
 *
 * @remarks For the horizontal component, @c 0.0 means to the left, @c 1.0
 *          means to the right. Analogously, for the vertical component, @c 0.0
 *          to the top, @c 1.0 means to the bottom.
 *
 * @remarks See the following figure:
 *
 * @image html alignment-hints.png
 * @image rtf alignment-hints.png
 * @image latex alignment-hints.eps
 *
 * @remarks This is not a size enforcement in any way. It is just a hint that
 *          should be used whenever appropriate.
 *
 * @remarks The default alignment hint values are @c 0.5, for both axis.
 *
 * @param[in]   obj  The given Evas object to query hints from
 * @param[in]   x    The horizontal alignment hint as double value ranging from @c 0.0 to @c 1.0 or with the
 *               special value #EVAS_HINT_FILL
 * @param[in]   y    The vertical alignment hint as double value ranging from @c 0.0 to @c 1.0 or with the
 *               special value #EVAS_HINT_FILL 
 *
 * @see evas_object_size_hint_align_get()
 * @see evas_object_size_hint_max_set()
 * @see evas_object_size_hint_padding_set()
 */
EAPI void evas_object_size_hint_align_set(Evas_Object *obj, double x, double y) EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the hints for an object's weight.
 *
 * @since_tizen 2.3
 *
 * @remarks Accepted values are @c 0 or positive values. Some users might use
 *          this hint as a boolean, but some might consider it as a @b
 *          proportion. See documentation of possible users, which in Evas are
 *          the @ref Evas_Object_Box "box" and @ref Evas_Object_Table "table"
 *          smart objects.
 *
 * @remarks This is not a size enforcement in any way. It is just a hint that
 *          should be used whenever appropriate.
 *
 * @remarks Use @c NULL pointers on the hint components that you are not
 *          interested in: they are ignored by the function.
 * @remarks If @c obj is invalid, then the hint components are set with @c 0.0
 *
 * @param[in]   obj  The given Evas object to query hints from
 * @param[out]   x    The pointer to a double in which to store the horizontal weight
 * @param[out]   y    The pointer to a double in which to store the vertical weight
 *
 * @see evas_object_size_hint_weight_set() for an example
 */
EAPI void evas_object_size_hint_weight_get(const Evas_Object *obj, double *x, double *y) EINA_ARG_NONNULL(1);

/**
 * @brief   Sets the hints for an object's weight.
 *
 * @since_tizen 2.3
 *
 * @remarks This is not a size enforcement in any way. It is just a hint that
 *          should be used whenever appropriate.
 *
 * @remarks This is a hint on how a container object should @b resize a given
 *          child within its area. Containers may adhere to the simpler logic
 *          of just expanding the child object's dimensions to fit its own (see
 *          the #EVAS_HINT_EXPAND helper weight macro) or the complete one of
 *          taking each child's weight hint as real @b weights to how much of
 *          its size to allocate for them in each axis. A container is supposed
 *          to, after @b normalizing the weights of its children (with weight
 *          hints), distribute the space it has to layout them by those factors
 *          -- most weighted children get larger in this process than the least ones.
 *
 * @remarks Default weight hint values are @c 0.0, for both axis.
 *
 * @param[in]   obj  The given Evas object to query hints from
 * @param[in]   x    The non-negative double value to use as horizontal weight hint
 * @param[in]   y    The non-negative double value to use as vertical weight hint
 *
 * @see evas_object_size_hint_weight_get() for more information
 */
EAPI void evas_object_size_hint_weight_set(Evas_Object *obj, double x, double y) EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the hints for an object's padding space.
 *
 * @since_tizen 2.3
 *
 * @remarks Padding is extra space that an object takes on each of its delimiting
 *          rectangle sides, in canvas units. This space is rendered
 *          transparent, naturally, as in the following figure:
 *
 * @image html padding-hints.png
 * @image rtf padding-hints.png
 * @image latex padding-hints.eps
 *
 * @remarks This is not a size enforcement in any way. It is just a hint that
 *          should be used whenever appropriate.
 *
 * @remarks Use @c NULL pointers on the hint components that you are not
 *          interested in: they are ignored by the function.
 *
 * @param[in]   obj  The given Evas object to query hints from
 * @param[out]   l    The pointer to an integer in which to store left padding
 * @param[out]   r    The pointer to an integer in which to store right padding
 * @param[out]   t    The pointer to an integer in which to store top padding
 * @param[out]   b    The pointer to an integer in which to store bottom padding
 *
 * @see evas_object_size_hint_padding_set()
 */
EAPI void evas_object_size_hint_padding_get(const Evas_Object *obj, Evas_Coord *l, Evas_Coord *r, Evas_Coord *t, Evas_Coord *b) EINA_ARG_NONNULL(1);

/**
 * @brief   Sets the hints for an object's padding space.
 *
 * @since_tizen 2.3
 *
 * @remarks This is not a size enforcement in any way. It is just a hint that
 *          should be used whenever appropriate.
 *
 * @param[in]   obj  The given Evas object to query hints from
 * @param[in]   l    The integer to specify left padding
 * @param[in]   r    The integer to specify right padding
 * @param[in]   t    The integer to specify top padding
 * @param[in]   b    The integer to specify bottom padding
 *
 * @see evas_object_size_hint_padding_get() for more information
 */
EAPI void evas_object_size_hint_padding_set(Evas_Object *obj, Evas_Coord l, Evas_Coord r, Evas_Coord t, Evas_Coord b) EINA_ARG_NONNULL(1);

/**
 * @}
 */

/**
 * @defgroup Evas_Object_Group_Extras Extra Object Manipulation
 * @ingroup Evas_Object_Group
 *
 * @brief  This group provides functions for extra object manipulation.
 *
 * Miscellaneous functions that also apply to any object, but are less
 * used or not implemented by all objects.
 *
 * @{
 */

/**
 * @brief   Sets an attached data pointer to an object with a given string key.
 *
 * @since_tizen 2.3
 *
 * @remarks This attaches the pointer @a data to the object @a obj, given the
 *          access string @a key. This pointer stays "hooked" to the object
 *          until a new pointer with the same string key is attached with
 *          evas_object_data_set() or it is deleted with
 *          evas_object_data_del(). On deletion of the object @a obj, the
 *          pointers is not accessible from the object anymore.
 *
 * @remarks You can find the pointer attached under a string key using
 *          evas_object_data_get(). It is the job of the calling application to
 *          free any data pointed to by @a data when it is no longer required.
 *
 * @remarks If @a data is @c NULL, the old value stored at @a key is
 *          removed but no new value is stored. This is synonymous with
 *          calling evas_object_data_del() with @a obj and @a key.
 *
 * @remarks This function is very handy when you have data associated
 *          specifically to an Evas object, being of use only when dealing with
 *          it. You do not have the burden to a pointer to it elsewhere,
 *          using this family of functions.
 *
 * @remarks The following is an example:
 *
 * @code
 * int *my_data;
 * extern Evas_Object *obj;
 *
 * my_data = malloc(500);
 * evas_object_data_set(obj, "name_of_data", my_data);
 * printf("The data that is attached is %p\n", evas_object_data_get(obj, "name_of_data"));
 * @endcode
 *
 * @param[in]   obj   The object to attach the data pointer to
 * @param[in]   key   The string key for the data to access it
 * @param[in]   data  The pointer to the data to be attached
 *
 */
EAPI void                     evas_object_data_set(Evas_Object *obj, const char *key, const void *data) EINA_ARG_NONNULL(1, 2);

/**
 * @brief   Gets an attached data pointer on an Evas object by its given
 *          string key.
 *
 * @details This function returns the data pointer attached to the object
 *          @a obj, stored using the string key @a key. If the object is valid
 *          and a data pointer is stored under the given key, that pointer
 *          is returned. If this is not the case, @c NULL is
 *          returned, signifying an invalid object or a non-existent key. It is
 *          possible that a @c NULL pointer is stored given that key, but this
 *          situation is not probable and thus can be considered an error as
 *          well. @c NULL pointers are never stored as this is the return value
 *          if an error occurs.
 *
 * @since_tizen 2.3
 *
 * @remarks The following is an example:
 *
 * @code
 * int *my_data;
 * extern Evas_Object *obj;
 *
 * my_data = evas_object_data_get(obj, "name_of_my_data");
 * if (my_data) printf("Data stored is %p\n", my_data);
 * else printf("No data is stored on the object\n");
 * @endcode
 *
 * @param[in]   obj  The object to which the data is attached
 * @param[in]   key  The string key the data is stored under
 * @return  The data pointer stored, 
 *          otherwise @c NULL if none are stored
 */
EAPI void                    *evas_object_data_get(const Evas_Object *obj, const char *key) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1, 2);

/**
 * @brief   Deletes an attached data pointer from an object.
 *
 * @since_tizen 2.3
 *
 * @remarks This removes the stored data pointer from @a obj stored under
 *          @a key and return this same pointer, if actually there is data
 *          there, or @c NULL, if nothing is stored under that key.
 *
 * @remarks The following is an example:
 *
 * @code
 * int *my_data;
 * extern Evas_Object *obj;
 *
 * my_data = evas_object_data_del(obj, "name_of_my_data");
 * @endcode
 *
 * @param[in]   obj  The object to delete the data pointer from
 * @param[in]   key  The string key the data is stored under
 * @return  The original data pointer stored at @a key on @a obj
 */
EAPI void                    *evas_object_data_del(Evas_Object *obj, const char *key) EINA_ARG_NONNULL(1, 2);

/**
 * @brief   Sets the pointer behavior.
 *
 * @since_tizen 2.3
 *
 * @remarks This function has direct effect on event callbacks related to mouse.
 *
 * @remarks If @a setting is @c EVAS_OBJECT_POINTER_MODE_AUTOGRAB, then when mouse
 *          is down at this object, events are restricted to it as source,
 *          mouse moves, for example, are emitted even if outside this
 *          object area.
 *
 * @remarks If @a setting is @c EVAS_OBJECT_POINTER_MODE_NOGRAB, then events are
 *          emitted just when inside this object area.
 *
 * @remarks The default value is @c EVAS_OBJECT_POINTER_MODE_AUTOGRAB.
 *
 * @param[in]   obj      The pointer for which behavior is set
 * @param[in]   setting  The desired behavior
 *
 * @ingroup Evas_Object_Group_Extras
 */
EAPI void                     evas_object_pointer_mode_set(Evas_Object *obj, Evas_Object_Pointer_Mode setting) EINA_ARG_NONNULL(1);

/**
 * @brief   Gets how the pointer behaves.
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj  The pointer
 * @return  The pointer behavior
 * @ingroup Evas_Object_Group_Extras
 */
EAPI Evas_Object_Pointer_Mode evas_object_pointer_mode_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief   Sets whether the given Evas object is to be drawn anti-aliased.
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj        The given Evas object
 * @param[in]   antialias  Set #EINA_TRUE for the object to be anti-aliased, \n
 *                     otherwise #EINA_FALSE for it not to be anti-aliased
 * @ingroup Evas_Object_Group_Extras
 */
EAPI void                     evas_object_anti_alias_set(Evas_Object *obj, Eina_Bool antialias) EINA_ARG_NONNULL(1);

/**
 * @brief   Checks whether the given Evas object is to be drawn anti-aliased.
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj  The given Evas object
 * @return  #EINA_TRUE if the object is to be anti-aliased,
 *          otherwise #EINA_FALSE if it is not to be anti-aliased
 * @ingroup Evas_Object_Group_Extras
 */
EAPI Eina_Bool                evas_object_anti_alias_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief   Sets the scaling factor for an Evas object. \n
 *          Does not affect all objects.
 *
 * @since_tizen 2.3
 *
 * @remarks This multiplies the object's dimension by the given factor, thus
 *          altering its geometry (width and height). This is useful when you want
 *          scalable UI elements, possibly at run time.
 *
 * @remarks Only text and textblock objects have scaling change
 *          handlers. Other objects do not change visually on this call.
 *
 * @param[in]   obj    The given Evas object
 * @param[in]   scale  The scaling factor \n 
 *                 <c>1.0</c> means no scaling, default size.
 *
 * @see evas_object_scale_get()
 *
 * @ingroup Evas_Object_Group_Extras
 */
EAPI void                     evas_object_scale_set(Evas_Object *obj, double scale) EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the scaling factor for the given Evas object.
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj The given Evas object
 * @return  The scaling factor
 *
 * @ingroup Evas_Object_Group_Extras
 *
 * @see evas_object_scale_set()
 */
EAPI double                   evas_object_scale_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief   Sets the render_op to be used for rendering the Evas object.
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj  The given Evas object
 * @param[in]   op   An Evas_Render_Op value
 * @ingroup Evas_Object_Group_Extras
 */
EAPI void                     evas_object_render_op_set(Evas_Object *obj, Evas_Render_Op op) EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the current value of the operation used for rendering the Evas object.
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj  The given Evas object
 * @return  An enumerated value in Evas_Render_Op
 * @ingroup Evas_Object_Group_Extras
 */
EAPI Evas_Render_Op           evas_object_render_op_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief   Sets whether to use precise (usually expensive) point collision
 *          detection for a given Evas object.
 *
 * @since_tizen 2.3
 *
 * @remarks Use this function to make Evas treat objects' transparent areas as
 *          @b not belonging to it with regard to mouse pointer events. By
 *          default, all of the object's boundary rectangle is taken into
 *          account for them.
 *
 * @remarks By using precise point collision detection you are
 *          making Evas more resource intensive.
 *
 * @param[in]   obj      The given object
 * @param[in]   precise  Set #EINA_TRUE to use precise point collision detection, \n 
 *                   otherwise set #EINA_FALSE to not use it \n
 *                   The default value is #EINA_FALSE.
 *
 * @see evas_object_precise_is_inside_get()
 * @ingroup Evas_Object_Group_Extras
 */
EAPI void                     evas_object_precise_is_inside_set(Evas_Object *obj, Eina_Bool precise) EINA_ARG_NONNULL(1);

/**
 * @brief   Checks whether an object is set to use precise point collision detection.
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj  The given object
 * @return  #EINA_TRUE if @a obj is set to use precise point collision detection, \n 
 *          otherwise #EINA_FALSE if it is not set to use \n 
 *          The default value is #EINA_FALSE.
 *
 * @see evas_object_precise_is_inside_set() for an example
 *
 * @ingroup Evas_Object_Group_Extras
 */
EAPI Eina_Bool                evas_object_precise_is_inside_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief   Sets a hint flag on the given Evas object that it is used as a "static clipper".
 *
 * @since_tizen 2.3
 *
 * @remarks This is a hint to Evas that this object is used as a big static
 *          clipper and should not be moved with children and otherwise
 *          considered specially. The default value for new objects is #EINA_FALSE.
 *
 * @param[in]   obj             The given object
 * @param[in]   is_static_clip  Set #EINA_TRUE if it is to be used as a static clipper, \n
 *                          otherwise set #EINA_FALSE
 *
 * @see evas_object_static_clip_get()
 *
 * @ingroup Evas_Object_Group_Extras
 */
EAPI void                     evas_object_static_clip_set(Evas_Object *obj, Eina_Bool is_static_clip) EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the "static clipper" hint flag for a given Evas object.
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj  The given object
 * @return  #EINA_TRUE if it is set as a static clipper, \n
 *          otheriwse #EINA_FALSE
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
 * @internal
 * @defgroup Evas_Object_Group_Find Finding Objects
 * @ingroup Evas_Object_Group
 *
 * @brief   This group provides functions for finding objects.
 *          Functions that allows finding objects by their position, name or
 *          other properties.
 *
 * @{
 */

/**
 * @brief   Gets the object that currently has focus.
 *
 * @since_tizen 2.3
 *
 * @remarks Evas can have (at most) one of its objects focused at a time.
 *          Focused objects are the ones having <b>key events</b> delivered
 *          to, which the programmer can act upon by means of
 *          evas_object_event_callback_add() usage.
 *
 * @remarks Mostly you would not be dealing directly with Evas focused
 *          objects. Instead, you would be using a higher level library for
 *          that (like a toolkit, as Elementary) to handle focus and who is
 *          receiving input for them.
 *
 * @remarks This call returns the object that currently has focus on the canvas
 *          @a e or @c NULL, if none.
 *
 * @param[in]   e  The Evas canvas to query for focused object
 * @return  The object that has focus, \n 
 *          otherwise @c NULL if there is no object that has focus
 *
 * @see evas_object_focus_set
 * @see evas_object_focus_get
 * @see evas_object_key_grab
 * @see evas_object_key_ungrab
 * @ingroup Evas_Object_Group_Find
 */
EAPI Evas_Object *evas_focus_get(const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the object on the given evas with the given name.
 *
 * @since_tizen 2.3
 *
 * @remarks This looks for the evas object given a name by evas_object_name_set(). If
 *          the name is not unique canvas-wide, then which one of the many objects
 *          with that name is returned is undefined. So only use this if you can ensure
 *          the object name is unique.
 *
 * @param[in]   e     The given evas
 * @param[in]   name  The given name
 * @return  The Evas object with the given name, \n  
 *          otherwise @c NULL on failure
 *
 * @ingroup Evas_Object_Group_Find
 */
EAPI Evas_Object *evas_object_name_find(const Evas *e, const char *name) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the object from children of the given object with the given name.
 *
 * @details This function looks for the evas object, which has its name set using evas_object_name_set(), but
 *          it ONLY looks at the children of the object @a obj, and only recurse
 *          into those children if @a recurse is greater than @c 0. If the name is not
 *          unique within immediate children (or the whole child tree) then it is not
 *          defined which child object is returned. If @a recurse is set to @c -1 then
 *          it recurses without limit.
 * @since   1.2
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj      The parent (smart) object whose children to search
 * @param[in]   name     The given name
 * @param[in]   recurse  Set to the number of child levels to recurse \n
 *                   (0 == do not recurse, \n
 *                   1 == only look at the children of @a obj or their immediate children and no further etc.).
 * @return  The Evas object with the given name, \n 
 *          otherwise @c NULL on failure
 *
 * @ingroup Evas_Object_Group_Find
 */
EAPI Evas_Object *evas_object_name_child_find(const Evas_Object *obj, const char *name, int recurse) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the Evas object stacked at the top of a given position in a canvas.
 *
 * @details This function traverses all the layers of the given canvas,
 *          from top to bottom, querying for objects with areas covering the
 *          given position. The user can remove from the query
 *          objects which are hidden and/or which are set to pass events.
 *
 * @since_tizen 2.3
 *
 * @remarks This function @b skips objects parented by smart
 *          objects, acting only on the ones at the "top level", with regard to
 *          object parenting.
 *
 * @param[in]   e                            A handle to the canvas
 * @param[in]   x                            The horizontal coordinate of the position
 * @param[in]   y                            The vertical coordinate of the position
 * @param[in]   include_pass_events_objects  Set #EINA_TRUE to include objects which pass events in this calculation, \n
 *                                       otherwise set #EINA_FALSE to not include the objects
 * @param[in]   include_hidden_objects       Set #EINA_TRUE to include hidden objects in this calculation, \n
 *                                       otherwise set #EINA_FALSE to not include hidden objects 
 * @return  The Evas object that is over all other objects at the given position
 */
EAPI Evas_Object *evas_object_top_at_xy_get(const Evas *e, Evas_Coord x, Evas_Coord y, Eina_Bool include_pass_events_objects, Eina_Bool include_hidden_objects) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the Evas object stacked at the top at the position of the
 *          mouse cursor, over a given canvas.
 *
 * @details This function traverses all the layers of the given canvas,
 *          from top to bottom, querying for objects with areas covering the
 *          mouse pointer's position, over @a e.
 *
 * @since_tizen 2.3
 *
 * @remarks This function @b skips objects parented by smart
 *          objects, acting only on the ones at the "top level", with regard to
 *          object parenting.
 *
 * @param[in]   e  A handle to the canvas
 * @return  The Evas object that is over all other objects at the mouse pointer's position
 */
EAPI Evas_Object *evas_object_top_at_pointer_get(const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the Evas object stacked at the top of a given rectangular
 *          region in a canvas.
 *
 * @details This function traverses all the layers of the given canvas,
 *          from top to bottom, querying for objects with areas overlapping
 *          with the given rectangular region inside @a e. The user can remove
 *          from the query objects which are hidden and/or which are set to
 *          pass events.
 *
 * @since_tizen 2.3
 *
 * @remarks This function @b skips objects parented by smart
 *          objects, acting only on the ones at the "top level", with regard to
 *          object parenting.
 *
 * @param[in]   e                            A handle to the canvas
 * @param[in]   x                            The top left corner's horizontal coordinate for the
 *                                       rectangular region
 * @param[in]   y                            The top left corner's vertical coordinate for the
 *                                       rectangular region
 * @param[in]   w                            The width of the rectangular region
 * @param[in]   h                            The height of the rectangular region
 * @param[in]   include_pass_events_objects  Set #EINA_TRUE to include objects which pass events in this calculation,  \n
 *                                       otherwise #EINA_FALSE to not include the objects
 * @param[in]   include_hidden_objects       Set #EINA_TRUE to include hidden objects in this calculation, \n
 *                                       otherwise #EINA_FALSE to not include the hidden objects
 * @return  The Evas object that is over all other objects at the given rectangular region
 */
EAPI Evas_Object *evas_object_top_in_rectangle_get(const Evas *e, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h, Eina_Bool include_pass_events_objects, Eina_Bool include_hidden_objects) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief   Gets a list of Evas objects lying over a given position in a canvas.
 *
 * @details This function traverses all the layers of the given canvas,
 *          from top to bottom, querying for objects with areas covering the
 *          given position. The user can remove from query
 *          objects which are hidden and/or which are set to pass events.
 *
 * @since_tizen 2.3
 *
 * @remarks This function @b skips objects parented by smart
 *          objects, acting only on the ones at the "top level", with regard to
 *          object parenting.
 *
 * @param[in]   e                            A handle to the canvas
 * @param[in]   x                            The horizontal coordinate of the position
 * @param[in]   y                            The vertical coordinate of the position
 * @param[in]   include_pass_events_objects  Set #EINA_TRUE to include objects which pass events in this calculation, \n
 *                                       otherwise set #EINA_FALSE to not include the objects
 * @param[in]   include_hidden_objects       Set #EINA_TRUE to include hidden objects in this calculation, \n
 *                                       otherwise set #EINA_FALSE to not include the hidden objects
 * @return  The list of Evas objects that are over the given position in @a e
 */
EAPI Eina_List   *evas_objects_at_xy_get(const Evas *e, Evas_Coord x, Evas_Coord y, Eina_Bool include_pass_events_objects, Eina_Bool include_hidden_objects) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);
EAPI Eina_List   *evas_objects_in_rectangle_get(const Evas *e, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h, Eina_Bool include_pass_events_objects, Eina_Bool include_hidden_objects) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the lowest (stacked) Evas object on the canvas @a e.
 *
 * @details This function takes all populated layers in the canvas into
 *          account, getting the lowest object for the lowest layer, naturally.
 *
 * @since_tizen 2.3
 *
 * @remarks This function @b skips objects parented by smart
 *          objects, acting only on the ones at the "top level", with regard to
 *          object parenting.
 *
 * @param[in]   e  A valid canvas pointer
 * @return  A pointer to the lowest object on it, if any, \n
 *          otherwise @c NULL if the pointer is not obtained
 *
 * @see evas_object_layer_get()
 * @see evas_object_layer_set()
 * @see evas_object_below_get()
 * @see evas_object_above_get()
 */
EAPI Evas_Object *evas_object_bottom_get(const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the highest (stacked) Evas object on the canvas @a e.
 *
 * @details This function takes all populated layers in the canvas into
 *          account, getting the highest object for the highest layer, naturally.
 *
 * @since_tizen 2.3
 *
 * @remarks This function @b skips objects parented by smart
 *          objects, acting only on the ones at the "top level", with regard to
 *          object parenting.
 *
 * @param[in]   e  A valid canvas pointer
 * @return  A pointer to the highest object on it, if any, \n
 *          otherwise @c NULL if the pointer is not obtained 
 *
 * @see evas_object_layer_get()
 * @see evas_object_layer_set()
 * @see evas_object_below_get()
 * @see evas_object_above_get() 
 */
EAPI Evas_Object *evas_object_top_get(const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @}
 */

/**
 * @internal
 * @defgroup Evas_Object_Group_Interceptors Object Method Interceptors
 * @ingroup Evas_Object_Group
 *
 * @brief  This group provides functions for method interceptors.
 *
 * Evas provides a way to intercept method calls. The interceptor
 * callback may opt to completely deny the call, or may check and
 * change the parameters before continuing. The continuation of an
 * intercepted call is done by calling the intercepted call again,
 * from inside the interceptor callback.
 *
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
 * @brief   Sets the callback function that intercepts a show event of an object.
 *
 * @details This function sets a callback function to intercepts a show event
 *          of a canvas object.
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj   The given canvas object pointer
 * @param[in]   func  The given function to be the callback function
 * @param[in]   data  The data passed to the callback function
 *
 * @see evas_object_intercept_show_callback_del().
 *
 */
EAPI void  evas_object_intercept_show_callback_add(Evas_Object *obj, Evas_Object_Intercept_Show_Cb func, const void *data) EINA_ARG_NONNULL(1, 2);

/**
 * @brief   Unsets the callback function that intercepts a show event of a object.
 *
 * @details This function sets a callback function to intercept a show event
 *          of a canvas object.
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj   The given canvas object pointer
 * @param[in]   func  The given callback function
 *
 * @see evas_object_intercept_show_callback_add().
 *
 */
EAPI void *evas_object_intercept_show_callback_del(Evas_Object *obj, Evas_Object_Intercept_Show_Cb func) EINA_ARG_NONNULL(1, 2);

/**
 * @brief   Sets the callback function that intercepts a hide event of a object.
 *
 * @details This function sets a callback function to intercepts a hide event
 *          of a canvas object.
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj   The given canvas object pointer
 * @param[in]   func  The given function to be the callback function
 * @param[in]   data  The data passed to the callback function
 *
 * @see evas_object_intercept_hide_callback_del().
 *
 */
EAPI void  evas_object_intercept_hide_callback_add(Evas_Object *obj, Evas_Object_Intercept_Hide_Cb func, const void *data) EINA_ARG_NONNULL(1, 2);

/**
 * @brief   Unsets the callback function that intercepts a hide event of a object.
 *
 * @details This function sets a callback function to intercepts a hide event
 *          of a canvas object.
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj   The given canvas object pointer
 * @param[in]   func  The given callback function
 *
 * @see evas_object_intercept_hide_callback_add().
 *
 */
EAPI void *evas_object_intercept_hide_callback_del(Evas_Object *obj, Evas_Object_Intercept_Hide_Cb func) EINA_ARG_NONNULL(1, 2);

/**
 * @brief   Sets the callback function that intercepts a move event of a object.
 *
 * @details This function sets a callback function to intercepts a move event
 *          of a canvas object.
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj   The given canvas object pointer
 * @param[in]   func  The given function to be the callback function
 * @param[in]   data  The data passed to the callback function
 *
 * @see evas_object_intercept_move_callback_del().
 *
 */
EAPI void  evas_object_intercept_move_callback_add(Evas_Object *obj, Evas_Object_Intercept_Move_Cb func, const void *data) EINA_ARG_NONNULL(1, 2);

/**
 * @brief   Unsets the callback function that intercepts a move event of a object.
 *
 * @details This function sets a callback function to intercept a move event
 *          of a canvas object.
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj   The given canvas object pointer
 * @param[in]   func  The given callback function
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
 * @ingroup Evas_Object_Group
 *
 * @brief This group provides functions that work on specific objects.
 *
 */

/**
 * @defgroup Evas_Object_Rectangle Rectangle Object Functions
 * @ingroup Evas_Object_Specific
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
 * background. This can be accomplished with the following very simple code:
 * @code
 * Evas_Object *bg = evas_object_rectangle_add(evas_canvas);
 * //Here we set the rectangles red, green, blue and opacity levels
 * evas_object_color_set(bg, 255, 255, 255, 255); // opaque white background
 * evas_object_resize(bg, WIDTH, HEIGHT); // covers full canvas
 * evas_object_show(bg);
 * @endcode
 *
 * This however has issues if the @c evas_canvas is resized. However most
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
 * So this gives us a white background to our window that is resized
 * together with it.
 *
 * @section Debugging
 *
 * Debugging is a major part of any programmers task and when debugging visual
 * issues with evas programs the rectangle is an extremely useful tool. The
 * rectangle's simplicity means that it is easier to pinpoint issues with it than
 * with more complex objects. Therefore a common technique to use when writing
 * an evas program and not getting the desired visual result is to replace the
 * misbehaving object for a solid color rectangle and seeing how it interacts
 * with the other elements, this often allows us to notice clipping, parenting
 * or positioning issues. Once the issues have been identified and corrected the
 * rectangle can be replaced for the original part and in all likelihood any
 * remaining issues are specific to that object's type.
 *
 * @section clipping Clipping
 *
 * Clipping serves two main functions:
 * @li Limiting visibility (i.e. hiding portions of an object).
 * @li Applying a layer of color to an object.
 *
 * @subsection hiding Limiting visibility
 *
 * It is often necessary to show only parts of an object, while it may be
 * possible to create an object that corresponds only to the part that must be
 * shown (and it is not always possible), it is usually easier to use a clipper. A
 * clipper is a rectangle that defines what is visible and what is not. The way
 * to do this is to create a solid white rectangle (which is the default, no need
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
 * change in the color of the clipped object, it just hid what is outside the
 * clippers area. It is however sometimes desirable to change the color of an
 * object, this can be accomplished using a clipper that has a non-white color.
 * Clippers with color work by multiplying the colors of clipped object. The
 * following code shows how to remove all the red from an object:
 * @code
 * Evas_Object *clipper = evas_object_rectangle_add(evas);
 * evas_object_move(clipper, my_evas_object_x, my_evas_object_y);
 * evas_object_resize(clipper, my_evas_object_width, my_evas_object_height);
 * evas_object_color_set(clipper, 0, 255, 255, 255);
 * evas_object_clip_set(obj, clipper);
 * evas_object_show(clipper);
 * @endcode
 *
 * @remarks We do not guarantee any proper results if you create a Rectangle
 *          object without setting the evas engine.
 *
 * @{
 */

/**
 * @brief   Adds a rectangle to the given evas.
 *
 * @since_tizen 2.3 
 *
 * @param[in]   e  The given evas
 * @return  The new rectangle object
 */
EAPI Evas_Object *evas_object_rectangle_add(Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * @}
 */

/**
 * @defgroup Evas_Object_Image Image Object Functions
 * @ingroup Evas_Object_Specific
 *
 * @brief  This group provides functions for image objects.
 *
 * The functions used to create and manipulate image objects
 * are grouped together. They are available to whichever occasion one needs
 * complex imagery on a GUI that could not be achieved by the other
 * Evas' primitive object types, or to make image manipulations.
 *
 * Evas supports whichever image file types it is compiled with
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
 * one must set a source file on it, so that it knows where to fetch
 * image data from. Next, one must set <b>how to fill the image
 * object's area</b> with the given pixel data. One could use just a
 * sub-region of the original image or even have it tiled repeatedly
 * on the image object. For the common case of having the whole source
 * image to be displayed on the image object, stretched to the
 * destination's size, there is also a function helper, to be used
 * instead of evas_object_image_fill_set():
 * @code
 * evas_object_image_filled_set(img, EINA_TRUE);
 * @endcode
 * See those functions' documentation for more details.
 *
 * @section Evas_Object_Image_Scale Scale and resizing
 *
 * Resizing of image objects scales their respective source images
 * to their areas, if they are set to "fill" the object's area
 * (evas_object_image_filled_set()). If you want any control on
 * the aspect ratio of an image for different sizes, you have to
 * take care of that yourself. There are functions to make images to
 * get loaded scaled (up or down) in memory, already, if you are
 * going to use them at pre-determined sizes and want to save
 * computations.
 *
 * Evas has even a scale cache, which takes care of caching scaled
 * versions of images with most usage/hits. Finally, you can
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
 * In image viewer applications, for example, the user is looking
 * at a given image, at full size, and desires that the navigation
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
 * If you are loading images which are too big, consider setting
 * its loading size to something smaller, in case you do not expose 
 * them in real size. It may speed up the loading considerably:
 * @code
 * //To load a scaled down version of the image in memory, if that is
 * //the size you are displaying anyway
 * evas_object_image_load_scale_down_set(img, zoom);
 *
 * //optional: if you know you are going to show a sub-set of the image's
 * //pixels, you can avoid loading the complementary data
 * evas_object_image_load_region_set(img, x, y, w, h);
 * @endcode
 * Refer to Elementary's Photocam widget for a high level (smart)
 * object which does lots of loading speed-ups for you.
 *
 * @subsection Evas_Object_Image_Animation Animation hints
 *
 * If you want to animate image objects on a UI (what you would get by
 * concomitant usage of other libraries, like Ecore and Edje), there
 * are also some tips on how to boost the performance of your
 * application. If the animation involves resizing of an image (thus,
 * re-scaling), you would better turn off smooth scaling on it @b during
 * the animation, turning it back on afterwards, for less
 * computations. Also, in this case you would better flag the image object
 * in question not to cache scaled versions of it:
 * @code
 * evas_object_image_scale_hint_set(wd->img, EVAS_IMAGE_SCALE_HINT_DYNAMIC);
 *
 * // Resizing takes place in between
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
 * treated specially -- as "borders". This makes those regions to be
 * treated specially on resizing scales, by keeping their aspect. This
 * makes setting frames around other objects on UIs easy.
 * See the following figures for a visual explanation:\n
 *
 * @image html image-borders.png
 * @htmlonly
 * <a href="image-borders.png">Full-size</a>
 * @endhtmlonly
 * @image rtf image-borders.png
 * @image latex image-borders.eps "image borders" width=\textwidth
 *
 * @image html border-effect.png
 * @htmlonly
 * <a href="border-effect.png">Full-size</a>
 * @endhtmlonly
 * @image rtf border-effect.png
 * @image latex border-effect.eps "border effect" width=\textwidth
 *
 * @section Evas_Object_Image_Manipulation Manipulating pixels
 *
 * Evas image objects can be used to manipulate raw pixels in many
 * ways.  The meaning of the data in the pixel arrays depends on
 * the image's color space, be warned (see next section). You can set
 * your own data as an image's pixel data, fetch an image's pixel data
 * for saving or altering, convert images between different color spaces
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
 *   region. All pixels are 32-bit unsigned int with the high-byte
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
 *   So 50% transparent blue is: 0x80000080. This is not
 *   "dark" - just 50% transparent. Values are 0 == black, 255 ==
 *   solid or full red, green or blue.
 * .
 * - #EVAS_COLORSPACE_YCBCR422P601_PL:
 *   This is a pointer-list indirected set of YUV (YCbCr) pixel
 *   data. This means that the data returned or set is not actual
 *   pixel data, but pointers TO lines of pixel data. The list of
 *   pointers are first be N rows of pointers to the Y plane -
 *   pointing to the first pixel at the start of each row in the Y
 *   plane. N is the height of the image data in pixels. Each pixel in
 *   the Y, U and V planes is 1 byte exactly, packed. The next N / 2
 *   pointers points to rows in the U plane, and the next N / 2
 *   pointers points to the V plane rows. U and V planes are half
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
 *   Values are @c 0 to @c 255, indicating full or no signal in that plane
 *   respectively.
 * .
 * - #EVAS_COLORSPACE_YCBCR422P709_PL:
 *   Not implemented yet.
 * .
 * - #EVAS_COLORSPACE_RGB565_A5P:
 *   In the process of being implemented in one engine only. This may
 *   change.
 *   \n\n
 *   This is a pointer to image data for 16-bit half-word pixel data
 *   in 16bpp RGB 565 format (5 bits red, 6 bits green, 5 bits blue),
 *   with the high-byte containing red and the low byte containing
 *   blue, per pixel. This data is packed row by row from the top-left
 *   to the bottom right.
 *   \n\n
 *   If the image has an alpha channel enabled there is an extra
 *   alpha plane after the color pixel plane. If not, then this data
 *   does not exist and should not be accessed in any way. This plane
 *   is a set of pixels with 1 byte per pixel defining the alpha
 *   values of all pixels in the image from the top-left to the bottom
 *   right of the image, row by row. Even though the values of the
 *   alpha pixels can be @c 0 to @c 255, only values @c 0 through to @c 32 are
 *   used, @c 32 being solid and 0 being transparent.
 *   \n\n
 *   RGB values can be @c 0 to @c 31 for red and blue and @c 0 to @c 63 for green,
 *   with @c 0 being black and @c 31 or @c 63 being full red, green or blue
 *   respectively. This colorspace is also pre-multiplied like
 *   EVAS_COLORSPACE_ARGB8888 so:
 *   \n\n
 *   R = (r * a) / 32; G = (g * a) / 32; B = (b * a) / 32;
 * .
 * - #EVAS_COLORSPACE_GRY8:
 *   The image is just an alpha mask (8 bits per pixel). This is used
 *   for alpha masking.
 *
 * @remarks We do not guarantee any proper results if you create an Image object
 *          without setting the evas engine.
 *
 * @{
 */

/**
 * @brief Callback of Evas Object Image Pixels
 * @see evas_object_image_pixels_get_callback_set
 */
typedef void (*Evas_Object_Image_Pixels_Get_Cb)(void *data, Evas_Object *o);

/**
 * @brief   Creates a new image object on the given Evas @a e canvas.
 *
 * @since_tizen 2.3
 *
 * @remarks If you intend to @b display an image somehow in a GUI,
 *          besides binding it to a real image file or source (with
 *          evas_object_image_file_set(), for example), you have to tell
 *          this image object how to fill its space with the pixels it can get
 *          from the source. See evas_object_image_filled_add(), for a helper
 *          on the common case of scaling up an image source to the whole area
 *          of the image object.
 *
 * @remarks The following is an example:
 * @code
 * img = evas_object_image_add(canvas);
 * evas_object_image_file_set(img, "/path/to/img", NULL);
 * @endcode
 *
 * @param[in]   e  The given canvas
 * @return  The created image object handle
 *
 * @see evas_object_image_fill_set()
 */
EAPI Evas_Object                  *evas_object_image_add(Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * @brief   Creates a new image object that @b automatically scales its bound
 *          image to the object's area, on both axis.
 *
 * @since_tizen 2.3
 *
 * @remarks This is a helper function around evas_object_image_add() and
 *          evas_object_image_filled_set(). It has the same effect of applying
 *          those functions in sequence, which is a very common use case.
 *
 * @remarks Whenever this object gets resized, the bound image is
 *          rescaled, too.
 *
 * @param[in]   e  The given canvas
 * @return  The created image object handle
 *
 * @see evas_object_image_add()
 * @see evas_object_image_filled_set()
 * @see evas_object_image_fill_set()
 */
EAPI Evas_Object                  *evas_object_image_filled_add(Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * @brief   Sets the data for an image from memory to be loaded.
 *
 * @details This function is the same as evas_object_image_file_set() but the file to be loaded
 *          may exist at an address in memory (the data for the file, not the filename
 *          itself). The @a data at the address is copied and stored for future use, so
 *          no @a data needs to be kept after this call is made. It is managed and
 *          freed for you when no longer needed. The @a size is limited to 2 gigabytes
 *          in size, and must be greater than @c 0. A @c NULL @a data pointer is also
 *          invalid. Set the filename to @c NULL to reset to empty state and have the
 *          image file data freed from memory using evas_object_image_file_set().
 *
 * @since_tizen 2.3
 *
 * @remarks The @a format is optional (pass @c NULL if you do not need/use it). It is
 *          used to help Evas guess better which loader to use for the data. It may
 *          simply be the "extension" of the file as it would normally be on disk
 *          such as "jpg", "png", and "gif".
 *
 * @param[in]   obj     The given image object
 * @param[in]   data    The image file data address
 * @param[in]   size    The size of the image file data in bytes
 * @param[in]   format  The format of the file (optional), \n
 *                  otherwise set @c NULL
 * @param[in]   key     The image key in file, \n 
 *                  otherwise set @c NULL
 */
EAPI void                          evas_object_image_memfile_set(Evas_Object *obj, void *data, int size, char *format, char *key) EINA_ARG_NONNULL(1, 2);

/**
 * @brief   Sets the source file from where an image object must fetch the real
 *          image data (it may be an Eet file, besides pure image ones).
 *
 * @since_tizen 2.3
 *
 * @remarks If the file supports multiple data stored in it (as Eet files do),
 *          you can specify the key to be used as the index of the image in
 *          this file.
 *
 * @remarks The following is an example:
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
 *
 * @param[in]   obj   The given image object
 * @param[in]   file  The image file path
 * @param[in]   key   The image key in @a file (if its an Eet one), \n
 *                otherwise set @c NULL
 */
EAPI void                          evas_object_image_file_set(Evas_Object *obj, const char *file, const char *key) EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the source file from where an image object is to fetch the
 *          real image data (it may be an Eet file, besides pure image ones).
 *
 * @since_tizen 2.3
 *
 * @remarks You must @b not modify the strings on the returned pointers.
 *
 * @remarks Use @c NULL pointers on the file components that you are not
 *          interested in: they are ignored by the function.
 *
 * @param[in]   obj   The given image object
 * @param[out]   file  The location to store the image file path
 * @param[out]   key   The location to store the image key (if @a file is an Eet one)
 */
EAPI void                          evas_object_image_file_get(const Evas_Object *obj, const char **file, const char **key) EINA_ARG_NONNULL(1, 2);

/**
 * @brief   Sets the dimensions for an image object's border, a region which @b
 *          is not scaled together with its center ever.
 *
 * @since_tizen 2.3
 *
 * @remarks When Evas is rendering, an image source may be scaled to fit the
 *          size of its image object. This function sets an area from the
 *          borders of the image inwards which is @b not to be scaled. This
 *          function is useful for making frames and for widget theming, where,
 *          for example, buttons may be of varying sizes, but their border size
 *          must remain constant.
 *
 * @remarks The units used for @a l, @a r, @a t and @a b are canvas units.
 *
 * @remarks The border region itself @b may be scaled by the
 *          evas_object_image_border_scale_set() function.
 *
 * @remarks By default, image objects have no borders set, i. e. @c l, @c
 *          r, @c t and @c b start as @c 0.
 *
 * @remarks See the following figures for visual explanation:\n
 *
 * @image html image-borders.png
 * @htmlonly
 * <a href="image-borders.png">Full-size</a>
 * @endhtmlonly
 * @image latex image-borders.eps "image borders" width=\textwidth
 *
 * @image html border-effect.png
 * @htmlonly
 * <a href="border-effect.png">Full-size</a>
 * @endhtmlonly
 * @image rtf border-effect.png
 * @image latex border-effect.eps "border effect" width=\textwidth
 *
 * @param[in]   obj  The given image object
 * @param[in]   l    The border's left width
 * @param[in]   r    The border's right width
 * @param[in]   t    The border's top width
 * @param[in]   b    The border's bottom width
 *
 * @see evas_object_image_border_get()
 * @see evas_object_image_border_center_fill_set()
 */
EAPI void                          evas_object_image_border_set(Evas_Object *obj, int l, int r, int t, int b) EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the dimensions for an image object's border, a region
 *          which @b is not scaled together with its center ever.
 *
 * @since_tizen 2.3
 *
 * @remarks Use @c NULL pointers on the border components that you are not
 *          interested in: they are ignored by the function.
 *
 * @param[in]   obj  The given image object
 * @param[out]   l    The location to store the border's left width in
 * @param[out]   r    The location to store the border's right width in
 * @param[out]   t    The location to store the border's top width in
 * @param[out]   b    The location to store the border's bottom width in
 *
 * @see evas_object_image_border_set() for more details.
 */
EAPI void                          evas_object_image_border_get(const Evas_Object *obj, int *l, int *r, int *t, int *b) EINA_ARG_NONNULL(1);

/**
 * @brief   Sets @b how the center part of the given image object (not the
 *          borders) should be drawn when Evas is rendering it.
 *
 * @details This function sets how the center part of the image object's source
 *          image is to be drawn, which must be one of the values in
 *          #Evas_Border_Fill_Mode. By center we mean the complementary part of
 *          that defined by evas_object_image_border_set(). This one is very
 *          useful for making frames and decorations. You would most probably
 *          also be using a filled image (as in evas_object_image_filled_set())
 *          to use as a frame.
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj   The given image object
 * @param[in]   fill  The fill mode of the center region of @a obj (a value in
 *                #Evas_Border_Fill_Mode)
 *
 * @see evas_object_image_border_center_fill_get()
 */
EAPI void                          evas_object_image_border_center_fill_set(Evas_Object *obj, Evas_Border_Fill_Mode fill) EINA_ARG_NONNULL(1);

/**
 * @brief   Gets @b how the center part of the given image object (not the
 *          borders) is to be drawn when Evas is rendering it.
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj   The given image object
 * @return  fill  The fill mode of the center region of @a obj (a value in
 *                #Evas_Border_Fill_Mode)
 *
 * @see evas_object_image_fill_set()
 */
EAPI Evas_Border_Fill_Mode         evas_object_image_border_center_fill_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief   Sets whether the image object's fill property should track the 
 *          object's size.
 *
 * @since_tizen 2.3
 *
 * @remarks If @a setting is #EINA_TRUE, then every evas_object_resize()
 *          @b automatically triggers a call to evas_object_image_fill_set()
 *          with the that new size (and @c 0, @c 0 as source image's origin),
 *          so the bound image fills the whole object's area.
 *
 * @param[in]   obj      The given image object
 * @param[in]   setting  Set #EINA_TRUE to make the fill property follow object size, \n 
 *                   otherwise set #EINA_FALSE
 *
 * @see evas_object_image_filled_add()
 * @see evas_object_image_fill_get()
 */
EAPI void                          evas_object_image_filled_set(Evas_Object *obj, Eina_Bool setting) EINA_ARG_NONNULL(1);

/**
 * @brief   Checks whether the image object's fill property should track the
 *          object's size.
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj  The given image object
 * @return  #EINA_TRUE if it is tracking, \n
 *          otherwise #EINA_FALSE if it is not tracking (and
 *          evas_object_fill_set() must be called manually)
 *
 * @see evas_object_image_filled_set() for more information
 */
EAPI Eina_Bool                     evas_object_image_filled_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief   Sets the scaling factor (multiplier) for the borders of an image object.
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj    The given image object
 * @param[in]   scale  The scale factor (default is @c 1.0 - i.e. no scaling)
 *
 * @see evas_object_image_border_set()
 * @see evas_object_image_border_scale_get()
 */
EAPI void                          evas_object_image_border_scale_set(Evas_Object *obj, double scale);

/**
 * @brief   Gets the scaling factor (multiplier) for the borders of an image object.
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj  The given image object
 * @return  The scale factor set for its borders
 *
 * @see evas_object_image_border_set()
 * @see evas_object_image_border_scale_set()
 */
EAPI double                        evas_object_image_border_scale_get(const Evas_Object *obj);

/**
 * @brief   Sets how to fill an image object's drawing rectangle given the
 *          (real) image bound to it.
 *
 * @since_tizen 2.3
 *
 * @remarks Note that if @a w or @a h are smaller than the dimensions of
 *          @a obj, the displayed image is @b tiled around the object's
 *          area. To have only one copy of the bound image drawn, @a x and @a y
 *          must be @c 0 and @a w and @a h need to be the exact width and height
 *          of the image object itself, respectively.
 *
 * @remarks See the following image to better understand the effects of this
 *          call. On this diagram, both image object and original image source
 *          have @c a x @c a dimensions and the image itself is a circle, with
 *          empty space around it:
 *
 * @image html image-fill.png
 * @image rtf image-fill.png
 * @image latex image-fill.eps
 *
 * @remarks The default values for the fill parameters are @a x = 0,
 *          @a y = 0, @a w = 0 and @a h = 0. Thus, if you are not using the
 *          evas_object_image_filled_add() helper and want your image
 *          displayed, you have to set valid values with this function on
 *          your object.
 *
 * @remarks evas_object_image_filled_set() is a helper function which
 *          @b overrides the values set here automatically, for you, in a
 *          given way.
 *
 * @param[in]   obj  The given image object to operate on
 * @param[in]   x    The x coordinate (from the top left corner of the bound
 *               image) to start drawing from
 * @param[in]   y    The y coordinate (from the top left corner of the bound
 *               image) to start drawing from
 * @param[in]   w    The width the bound image is displayed at
 * @param[in]   h    The height the bound image is displayed at
 */
EAPI void                          evas_object_image_fill_set(Evas_Object *obj, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h) EINA_ARG_NONNULL(1);

/**
 * @brief   Sets how an image object is to fill its drawing rectangle,
 *          given the (real) image bound to it.
 *
 * @since_tizen 2.3
 *
 * @remarks Use @c NULL pointers on the fill components that you are not
 *          interested in: they are ignored by the function.
 *
 * @param[in]   obj  The given image object
 * @param[out]   x    The location to store the x coordinate (from the top left
 *               corner of the bound image) to start drawing from
 * @param[out]   y    The location to store the y coordinate (from the top left
 *               corner of the bound image) to start drawing from
 * @param[out]   w    The location to store the width the bound image is to be
 *               displayed at
 * @param[out]   h    The location to store the height the bound image is to be
 *               displayed at
 *
 * See @ref evas_object_image_fill_set() for more details.
 */
EAPI void                          evas_object_image_fill_get(const Evas_Object *obj, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h) EINA_ARG_NONNULL(1);

/**
 * @brief   Sets the tiling mode for the given evas image object's fill.
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj     The given evas image object
 * @param[in]   spread  One of EVAS_TEXTURE_REFLECT, EVAS_TEXTURE_REPEAT,
 *                  EVAS_TEXTURE_RESTRICT, or EVAS_TEXTURE_PAD.
 */
EAPI void                          evas_object_image_fill_spread_set(Evas_Object *obj, Evas_Fill_Spread spread) EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the spread (tiling mode) for the given image object's fill.
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj  The given evas image object
 * @return  The current spread mode of the image object
 */
EAPI Evas_Fill_Spread              evas_object_image_fill_spread_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief   Sets the size of the given image object.
 *
 * @details This function scales down or crops the image so that it is
 *          treated as if it were at the given size. If the size given is
 *          smaller than the image, it is cropped. If the size given is
 *          larger, then the image is treated as if it were in the upper
 *          left hand corner of a larger image that is otherwise transparent.
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj  The given image object
 * @param[in]   w    The new width of the image
 * @param[in]   h    The new height of the image
 */
EAPI void                          evas_object_image_size_set(Evas_Object *obj, int w, int h) EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the size of the given image object.
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj  The given image object
 * @param[out]   w    The location to store the width of the image in, or @c NULL
 * @param[out]   h    The location to store the height of the image in, or @c NULL
 *
 * See @ref evas_object_image_size_set() for more details.
 */
EAPI void                          evas_object_image_size_get(const Evas_Object *obj, int *w, int *h) EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the row stride of the given image object.
 *
 * @since_tizen 2.3
 *
 * @remarks The row stride is the number of bytes between the start of a row
 *          and the start of the next row for image data.
 *
 * @param[in]   obj  The given image object
 * @return  The stride of the image (<b>in bytes</b>)
 */
EAPI int                           evas_object_image_stride_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief   Gets a number representing any error that occurred during the
 *          last loading of the given image object's source image.
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj  The given image object
 * @return  A value giving the last error that occurred \n 
 *          It should be a #Evas_Load_Error value. #EVAS_LOAD_ERROR_NONE
 *          is returned if there is no error.
 */
EAPI Evas_Load_Error               evas_object_image_load_error_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief   Sets the raw image data of the given image object.
 *
 * @since_tizen 2.3
 *
 * @remarks Note that the raw data must be of the same size (see
 *          evas_object_image_size_set(), which has to be called @b before this
 *          one) and colorspace (see evas_object_image_colorspace_set()) of the
 *          image. If data is @c NULL, the current image data is
 *          freed. Naturally, if one does not set an image object's data
 *          manually, it still has one, allocated by Evas.
 *
 * @param[in]   obj   The given image object
 * @param[in]   data  The raw data, or @c NULL
 *
 * @see evas_object_image_data_get()
 */
EAPI void                          evas_object_image_data_set(Evas_Object *obj, void *data) EINA_ARG_NONNULL(1);

/**
 * @brief   Gets a pointer to the raw image data of the given image object.
 *
 * @details This function returns a pointer to an image object's internal pixel
 *          buffer, for reading only or read/write. If you request it for
 *          writing, the image is marked as dirty so that it gets redrawn at
 *          the next update.
 *
 * @since_tizen 2.3
 *
 * @remarks Each time you call this function on an image object, its data
 *          buffer has an internal reference counter
 *          incremented. Decrement it back by using
 *          evas_object_image_data_set(). This is specially important for the
 *          directfb Evas engine.
 *
 * @remarks This is best suited for when you want to modify an existing image,
 *          without changing its dimensions.
 *
 * @remarks The contents' format returned by it depend on the color
 *          space of the given image object.
 *
 * @remarks You may want to use evas_object_image_data_update_add() to
 *          inform data changes, if you did any.
 *
 * @param[in]   obj          The given image object
 * @param[in]   for_writing  Set #EINA_TRUE to set retrieved data as modifiable, \n
 *                       otherwise #EINA_FALSE to not set it as modifiable
 * @return	The raw image data
 *
 * @see evas_object_image_data_set()
 */
EAPI void                         *evas_object_image_data_get(const Evas_Object *obj, Eina_Bool for_writing) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief   Converts the raw image data of the given image object to the
 *          specified colorspace.
 *
 * @since_tizen 2.3
 *
 * @remarks Note that this function does not modify the raw image data. If the
 *          requested colorspace is the same as the image colorspace nothing is
 *          done and @c NULL is returned. You should use
 *          evas_object_image_colorspace_get() to check the current image
 *          colorspace.
 *
 *
 * @param[in]   obj        The given image object
 * @param[in]   to_cspace  The colorspace to which the image raw data is converted
 *
 * @see @ref evas_object_image_colorspace_get
 */
EAPI void                         *evas_object_image_data_convert(Evas_Object *obj, Evas_Colorspace to_cspace) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief   Replaces the raw image data of the given image object.
 *
 * @details This function lets the application replace an image object's
 *          internal pixel buffer with an user-allocated one. For best results,
 *          you should generally first call evas_object_image_size_set() with
 *          the width and height for the new buffer.
 *
 * @since_tizen 2.3
 *
 * @remarks This call is best suited for when you are using image data with
 *          different dimensions than the existing image data, if any. If you
 *          only need to modify the existing image in some fashion, then using
 *          evas_object_image_data_get() is probably what you are after.
 *
 * @remarks Note that the caller is responsible for freeing the buffer when
 *          finished with it, as user-set image data is not automatically
 *          freed when the image object is deleted.
 *
 * @param[in]   obj   The given image object
 * @param[in]   data  The raw data to replace
 *
 * @see evas_object_image_data_get()
 */
EAPI void                          evas_object_image_data_copy_set(Evas_Object *obj, void *data) EINA_ARG_NONNULL(1);

/**
 * @brief   Marks a sub-region of the given image object to be redrawn.
 *
 * @details This function schedules a particular rectangular region of an image
 *          object to be updated (redrawn) at the next rendering cycle.
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj  The given image object
 * @param[in]   x    The X-offset of the region to be updated
 * @param[in]   y    The Y-offset of the region to be updated
 * @param[in]   w    The width of the region to be updated
 * @param[in]   h    The height of the region to be updated
 */
EAPI void                          evas_object_image_data_update_add(Evas_Object *obj, int x, int y, int w, int h) EINA_ARG_NONNULL(1);

/**
 * @brief   Enables or disables alpha channel usage on the given image object.
 *
 * @details This function sets a flag on an image object indicating whether or
 *          not to use alpha channel data. A value of #EINA_TRUE makes it use
 *          alpha channel data, and #EINA_FALSE makes it ignore that
 *          data. Note that this has nothing to do with an object's color as
 *          manipulated by evas_object_color_set().
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj        The given image object
 * @param[in]   has_alpha  Set #EINA_TRUE to use alpha channel () data, \n
 *                     otherwise set #EINA_FALSE to not use it
 *
 * @see evas_object_image_alpha_get()
 */
EAPI void                          evas_object_image_alpha_set(Evas_Object *obj, Eina_Bool has_alpha) EINA_ARG_NONNULL(1);

/**
 * @brief   Checks whether alpha channel data is being used on the given
 *          image object.
 *
 * @details This function returns #EINA_TRUE if the image object's alpha
 *          channel is being used, or #EINA_FALSE otherwise.
 *
 * @since_tizen 2.3
 *
 * @remarks See @ref evas_object_image_alpha_set() for more details.
 *
 * @param[in]   obj  The given image object
 * @return  #EINA_TRUE if the alpha channel data is being used, \n
 *          otherwise #EINA_FALSE if the alpha channel data is not being used
 *
 */
EAPI Eina_Bool                     evas_object_image_alpha_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief   Sets whether to use high-quality image scaling algorithm on the
 *          given image object.
 *
 * @since_tizen 2.3
 *
 * @remarks When enabled, a higher quality image scaling algorithm is used when
 *          scaling images to sizes other than the source image's original
 *          one. This gives better results but is more computationally expensive.
 *
 * @remarks Image objects get created originally with smooth scaling @b on.
 *
 * @param[in]   obj           The given image object
 * @param[in]   smooth_scale  Set #EINA_TRUE to use smooth scale, \n 
 *                        otherwise set #EINA_FALSE to not use it
 *
 * @see evas_object_image_smooth_scale_get()
 */
EAPI void                          evas_object_image_smooth_scale_set(Evas_Object *obj, Eina_Bool smooth_scale) EINA_ARG_NONNULL(1);

/**
 * @brief   Checks whether the given image object is using high-quality
 *          image scaling algorithm.
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj  The given image object
 * @return  #EINA_TRUE if smooth scale is being used, \n
 *          otherwise #EINA_FALSE if smooth scale is not being used
 *
 * @see evas_object_image_smooth_scale_set()
 */
EAPI Eina_Bool                     evas_object_image_smooth_scale_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief   Preloads an image object's image data in the background.
 *
 * @details This function requests the preload of the data image in the
 *          background. The work is queued before being processed (because
 *          there might be other pending requests of this type).
 *
 * @since_tizen 2.3
 *
 * @remarks Whenever the image data gets loaded, Evas calls
 *          #EVAS_CALLBACK_IMAGE_PRELOADED registered callbacks on @a obj (that
 *          may be immediately, if the data is already preloaded before).
 *
 * @remarks Use #EINA_TRUE for @a cancel on scenarios where you do not need
 *          the image data preloaded anymore.
 *
 * @remarks Any evas_object_show() call after evas_object_image_preload()
 *          makes the latter to be @b cancelled, with the loading process
 *          now taking place @b synchronously (and, thus, blocking the return
 *          of the former until the image is loaded). It is highly advisable,
 *          then, that the user preload an image with it being @b hidden, just
 *          to be shown on the #EVAS_CALLBACK_IMAGE_PRELOADED event's callback.
 *
 * @param[in]   obj     The given image object
 * @param[in]   cancel  Set #EINA_FALSE to add to the preloading work queue, \n
 *                  otherwise set #EINA_TRUE to remove it (if it is issued before)
 */
EAPI void                          evas_object_image_preload(Evas_Object *obj, Eina_Bool cancel) EINA_ARG_NONNULL(1);

/**
 * @brief   Reloads an image object's image data.
 *
 * @details This function reloads the image data bound to image object @a obj.
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj  The given image object pointer
 */
EAPI void                          evas_object_image_reload(Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * @brief   Saves the given image object's contents to an (image) file.
 *
 * @since_tizen 2.3
 *
 * @remarks The extension suffix on @a file determines which <b>saver
 *          module</b> Evas is to use when saving, thus the final file's
 *          format. If the file supports multiple data stored in it (Eet ones),
 *          you can specify the key to be used as the index of the image in it.
 *
 * @remarks You can specify some flags when saving the image. Currently
 *          acceptable flags are @c quality and @c compress. Eg.: @c
 *          "quality=100 compress=9"
 *
 * @param[in]   obj    The given image object
 * @param[in]   file   The filename to be used to save the image (extension obligatory)
 * @param[in]   key    The image key in the file (if an Eet one), \n
 *                 otherwise @c NULL
 * @param[in]   flags  String containing the flags to be used (@c NULL for none)
 * @return  #EINA_TRUE if the contents are saved successfully, \n
 *          otherwise #EINA_FALSE if it fails
 */
EAPI Eina_Bool                     evas_object_image_save(const Evas_Object *obj, const char *file, const char *key, const char *flags)  EINA_ARG_NONNULL(1, 2);

/**
 * @brief   Imports pixels from given source to a given canvas image object.
 *
 * @details This function imports pixels from a given source to a given canvas image.
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj     The given canvas object
 * @param[in]   pixels  The pixel's source to be imported
 * @return #EINA_TRUE if success, otherwise #EINA_FALSE.
 */
EAPI Eina_Bool                     evas_object_image_pixels_import(Evas_Object *obj, Evas_Pixel_Import_Source *pixels) EINA_ARG_NONNULL(1, 2);

/**
 * @brief   Sets the callback function to get pixels from a canvas' image.
 *
 * @details This sets a function to be the callback function that gets
 *          pixels from an image of the canvas.
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj   The given canvas pointer
 * @param[in]   func  The callback function
 * @param[in]   data  The data pointer to be passed to @a func
 *
 */
EAPI void                          evas_object_image_pixels_get_callback_set(Evas_Object *obj, Evas_Object_Image_Pixels_Get_Cb func, void *data) EINA_ARG_NONNULL(1, 2);

/**
 * @brief   Sets whether the given image object is dirty and needs to request its pixels.
 *
 * @details This function only works properly if a callback for getting pixels has been set.
 *
 * @since_tizen 2.3
 *
 * @remarks Use this function if you really know what you are doing.
 *
 * @param[in]   obj    The given image object
 * @param[in]   dirty  Set #EINA_TRUE to mark the image object as dirty, \n
 *                 otherwise set #EINA_FALSE to mark the image object as not dirty
 *
 * @see evas_object_image_pixels_get_callback_set()
 */
EAPI void                          evas_object_image_pixels_dirty_set(Evas_Object *obj, Eina_Bool dirty) EINA_ARG_NONNULL(1);

/**
 * @brief   Checks whether the given image object is dirty (needs to be redrawn).
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj  The given image object
 * @return  #EINA_TRUE if the image is dirty, \n
 *          otherwise #EINA_FALSE if the image is not dirty
 */
EAPI Eina_Bool                     evas_object_image_pixels_dirty_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief   Sets the DPI resolution of an image object's source image.
 *
 * @details This function sets the DPI resolution of a given loaded canvas
 *          image. This is useful for the SVG image loader.
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj  The given canvas pointer
 * @param[in]   dpi  The new DPI resolution
 *
 * @see evas_object_image_load_dpi_get()
 */
EAPI void                          evas_object_image_load_dpi_set(Evas_Object *obj, double dpi) EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the DPI resolution of a loaded image object in the canvas.
 *
 * @details This function returns the DPI resolution of the given canvas image.
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj  The given canvas pointer
 * @return  The DPI resolution of the given canvas image
 *
 * @see evas_object_image_load_dpi_set() for more details
 */
EAPI double                        evas_object_image_load_dpi_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief   Sets the load size of a given image object's source image.
 *
 * @details This function sets a new geometry size for the given canvas image.
 *          The image will be loaded into memory as if it was the set size instead of
 *          the original size.
 *
 * @since_tizen 2.3
 *
 * @remarks The size of a given image object's source image will be less than or
 *          equal to the size of @p w and @p h.
 *
 * @param[in]   obj  The given canvas object
 * @param[in]   w    The new width of the image's load size
 * @param[in]   h    The new height of the image's load size
 *
 * @see evas_object_image_load_size_get()
 */
EAPI void                          evas_object_image_load_size_set(Evas_Object *obj, int w, int h) EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the load size of a given image object's source image.
 *
 * @details This function gets the geometry size set manually for the given canvas image.
 *
 * @since_tizen 2.3
 *
 * @remarks Use @c NULL pointers on the size components that you are not
 *          interested in: they are ignored by the function.
 *          @p w and @p h will be set with the image's loading size only if
 *          the image's load size is set manually: if evas_object_image_load_size_set()
 *          has not been called, @p w and @p h will be set with 0.
 *
 * @param[in]   obj  The given image object
 * @param[out]   w    The new width of the image's load size that is returned
 * @param[out]   h    The new height of the image's load size that is returned
 *
 * @see evas_object_image_load_size_set() for more details
 */
EAPI void                          evas_object_image_load_size_get(const Evas_Object *obj, int *w, int *h) EINA_ARG_NONNULL(1);

/**
 * @brief   Sets the scale down factor of a given image object's source image,
 *          when loading it.
 *
 * @details This function sets the scale down factor of a given canvas
 *          image. This is useful for the SVG image loader.
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj         The given image object pointer
 * @param[in]   scale_down  The scale down factor
 *
 * @see evas_object_image_load_scale_down_get()
 */
EAPI void                          evas_object_image_load_scale_down_set(Evas_Object *obj, int scale_down) EINA_ARG_NONNULL(1);

/**
 * @brief  Gets the scale down factor of a given image object's source image,
 *         when loading it.
 *
 * @since_tizen 2.3
 *
 * @param[in]  obj  The given image object pointer
 * @return The scale down factor
 *
 * @see evas_object_image_load_scale_down_set() for more details
 */
EAPI int                           evas_object_image_load_scale_down_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief   Sets a selective region of the source image to load for the given image object.
 *
 * @details This function is useful when you are not showing all of an image's
 *          area on its image object.
 *
 * @since_tizen 2.3
 *
 * @remarks The image loader for the image format in question has to
 *          support selective region loading in order to this function to take effect.
 *
 * @param[in]   obj  The given image object pointer
 * @param[in]   x    The X-offset of the region to be loaded
 * @param[in]   y    The Y-offset of the region to be loaded
 * @param[in]   w    The width of the region to be loaded
 * @param[in]   h    The height of the region to be loaded
 *
 * @see evas_object_image_load_region_get()
 */
EAPI void                          evas_object_image_load_region_set(Evas_Object *obj, int x, int y, int w, int h) EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the coordinates of a given image object's selective
 *          (source image) load region.
 *
 * @since_tizen 2.3
 *
 * @remarks Use @c NULL pointers on the coordinates that you are not interested
 *          in: they are ignored by the function.
 *
 * @param[in]   obj  The given image object pointer
 * @param[out]   x    The X-offset of the region to be loaded
 * @param[out]   y    The Y-offset of the region to be loaded
 * @param[out]   w    The width of the region to be loaded
 * @param[out]   h    The height of the region to be loaded
 *
 * @see evas_object_image_load_region_get()
 */
EAPI void                          evas_object_image_load_region_get(const Evas_Object *obj, int *x, int *y, int *w, int *h) EINA_ARG_NONNULL(1);

/**
 * @brief   Sets whether the orientation information in the image file should be honored.
 * @since   1.1
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj     The given image object pointer
 * @param[in]   enable  Set #EINA_TRUE to honor the orientation information, \n
 *                  otherwise #EINA_FALSE to not honor the orientation information
 */
EAPI void                          evas_object_image_load_orientation_set(Evas_Object *obj, Eina_Bool enable) EINA_ARG_NONNULL(1);

/**
 * @brief  Checks whether the orientation information in the image file should be honored.
 * @since  1.1
 *
 * @since_tizen 2.3
 *
 * @param[in]  obj  The given image object pointer
 * @return #EINA_TRUE if the orientation information is honored, \n
 *         otherwise #EINA_FALSE if it is not honored
 */
EAPI Eina_Bool                     evas_object_image_load_orientation_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * @brief   Sets the colorspace of a given image of the canvas.
 *
 * @details This function sets the colorspace of given canvas image.
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj     The given image object pointer
 * @param[in]   cspace  The new color space
 *
 */
EAPI void                          evas_object_image_colorspace_set(Evas_Object *obj, Evas_Colorspace cspace) EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the colorspace of a given image of the canvas.
 *
 * @details This function returns the colorspace of given canvas image.
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj  The given image object pointer
 * @return  The colorspace of the image
 *
 */
EAPI Evas_Colorspace               evas_object_image_colorspace_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the support state of a given image.
 *
 * @details This function returns the state of the region support of given image
 * @since   1.2
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj  The given image object pointer
 * @return  The region support state
 */
EAPI Eina_Bool                     evas_object_image_region_support_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief   Sets the native surface of a given image of the canvas.
 *
 * @details This function sets a native surface of a given canvas image.
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj   The given canvas pointer
 * @param[in]   surf  The new native surface
 *
 */
EAPI void                          evas_object_image_native_surface_set(Evas_Object *obj, Evas_Native_Surface *surf) EINA_ARG_NONNULL(1, 2);

/**
 * @brief   Gets the native surface of a given image of the canvas.
 *
 * @details This function returns the native surface of a given canvas image.
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj  The given canvas pointer
 * @return  The native surface of the given canvas image
 *
 */
EAPI Evas_Native_Surface          *evas_object_image_native_surface_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief   Sets the video surface linked to a given image of the canvas.
 *
 * @details This function links a video surface to a given canvas image.
 * @since   1.1
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj   The given canvas pointer
 * @param[in]   surf  The new video surface
 *
 */
EAPI void                          evas_object_image_video_surface_set(Evas_Object *obj, Evas_Video_Surface *surf) EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the video surface linked to a given image of the canvas.
 *
 * @details This function returns the video surface linked to a given canvas image.
 * @since   1.1 
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj  The given canvas pointer
 * @return  The video surface of the given canvas image
 *
 */
EAPI const Evas_Video_Surface     *evas_object_image_video_surface_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief   Sets the scale hint of a given image of the canvas.
 *
 * @details This function sets the scale hint value of the given image object
 *          in the canvas, which affects how Evas is to cache scaled
 *          versions of its original source image.
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj   The given image object pointer
 * @param[in]   hint  The scale hint \n
 *                A #Evas_Image_Scale_Hint value.
 *
 * @see evas_object_image_scale_hint_get()
 */
EAPI void                          evas_object_image_scale_hint_set(Evas_Object *obj, Evas_Image_Scale_Hint hint) EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the scale hint of a given image of the canvas.
 *
 * @details This function returns the scale hint value of the given image
 *          object of the canvas.
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj  The given image object pointer
 * @return  The scale hint value set on @a obj \n 
 *          A #Evas_Image_Scale_Hint value.
 *
 * @see evas_object_image_scale_hint_set() for more details.
 */
EAPI Evas_Image_Scale_Hint         evas_object_image_scale_hint_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief   Sets the content hint setting of a given image object of the canvas.
 *
 * @details This function sets the content hint value of the given image of the
 *          canvas. For example, if you are on the GL engine and your driver
 *          implementation supports it, setting this hint to
 *          #EVAS_IMAGE_CONTENT_HINT_DYNAMIC makes it need @b zero copies
 *          at texture upload time, which is an "expensive" operation.
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj   The given canvas pointer
 * @param[in]   hint  The content hint value \n
 *                Valid values are defined in #Evas_Image_Content_Hint.
 *
 * @see evas_object_image_content_hint_get()
 */
EAPI void                          evas_object_image_content_hint_set(Evas_Object *obj, Evas_Image_Content_Hint hint) EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the content hint setting of a given image object of the canvas.
 *
 * @details This function returns the content hint value of the given image of
 *          the canvas.
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj  The given canvas pointer
 * @return  The content hint value set on it \n 
 *          Valid values are define in #Evas_Image_Content_Hint. \n
 *          #EVAS_IMAGE_CONTENT_HINT_NONE means an error.
 *
 * @see evas_object_image_content_hint_set()
 */
EAPI Evas_Image_Content_Hint       evas_object_image_content_hint_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief   Enables an image to be used as an alpha mask.
 *
 * @details This function sets flags, and discard any excess image data not used as an
 *          alpha mask.
 *
 * @since_tizen 2.3
 *
 * @remarks Note that there is little point in using a image as alpha mask unless it has an
 *          alpha channel.
 *
 * @param[in]   obj     The object to use as an alpha mask
 * @param[in]   ismask  Set #EINA_TRUE to use image as alphamask, 
 *                  otherwise #EINA_FALSE to not use image as alphamask
 */
EAPI void                          evas_object_image_alpha_mask_set(Evas_Object *obj, Eina_Bool ismask) EINA_ARG_NONNULL(1);

/**
 * @brief   Sets the source object on an image object to used as a @b proxy.
 *
 * @since_tizen 2.3
 *
 * @remarks If an image object is set to behave as a @b proxy, it mirrors
 *          the rendering contents of a given @b source object in its drawing
 *          region, without affecting that source in any way. The source must
 *          be another valid Evas object. Other effects may be applied to the
 *          proxy, such as a map (see evas_object_map_set()) to create a
 *          reflection of the original object (for example).
 *
 * @remarks Any existing source object on @a obj is removed after this
 *          call. Setting @a src to @c NULL clears the proxy object (not in
 *          "proxy state" anymore).
 *
 * @remarks You cannot set a proxy as another proxy's source.
 *
 * @param[in]   obj  The proxy (image) object
 * @param[in]   src  The source object to use for the proxy
 * @return  #EINA_TRUE if the source object is set successfully, \n
 *          otherwise #EINA_FALSE on error
 *
 * @see evas_object_image_source_get()
 * @see evas_object_image_source_unset()
 */
EAPI Eina_Bool                     evas_object_image_source_set(Evas_Object *obj, Evas_Object *src) EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the current source object of an image object.
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj  The Image object
 * @return  The source object (if any), \n
 *          otherwise @c NULL if not in "proxy mode" or on errors
 *
 * @see evas_object_image_source_set() for more details
 */
EAPI Evas_Object                  *evas_object_image_source_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * @brief   Clears the source object on a proxy image object.
 *
 * @details This function is equivalent to calling evas_object_image_source_set() with a
 *          @c NULL source.
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj  The Image object to clear source of
 * @return  #EINA_TRUE if the source object is cleared successfully, \n
 *          otherwise #EINA_FALSE on error
 */
EAPI Eina_Bool                     evas_object_image_source_unset(Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * @brief   Sets the source object to be visible.
 * @since   1.8
 *
 * @since_tizen 2.3
 *
 * @remarks If @a visible is set to #EINA_FALSE, the source object of the proxy (@a obj)
 *          is invisible.
 *
 * @remarks This function works differently to evas_object_show() and evas_object_hide().
 *          Once the source object is hidden by evas_object_hide() then the proxy object is 
 *          hidden as well. Actually in this case both objects are excluded from the
 *          Evas internal update circle.
 *
 * @remarks Using this method, you can toggle the visibility of a proxy's source
 *          object keeping the proxy visibility untouched.
 *
 * @param[in]   obj      The proxy (image) object
 * @param[in]   visible  Set #EINA_TRUE to show the source object, \n
 *                   otherwise set #EINA_FALSE to not show it
 *
 * @see evas_object_image_source_visible_get()
 * @see evas_object_image_source_set()
 * @see evas_object_show()
 * @see evas_object_hide()
 */
EAPI void                          evas_object_image_source_visible_set(Evas_Object *obj, Eina_Bool visible) EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the state of the source object visibility.
 * @since   1.8
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj  The proxy (image) object
 * @return  #EINA_TRUE if source object is visible, \n
 *          otherwise #EINA_FALSE
 *
 * @see evas_object_image_source_visible_set()
 * @see evas_object_image_source_set()
 * @see evas_object_show()
 * @see evas_object_hide()
 */
EAPI Eina_Bool                     evas_object_image_source_visible_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * @brief   Clips the proxy object with the source object's clipper.
 * @since   1.8
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj          The proxy (image) object
 * @param[in]   source_clip  Set #EINA_TRUE if @a obj must be clipped by the source clipper, \n
 *                       otherwise set #EINA_FALSE
 *
 * @see evas_object_clip_set()
 * @see evas_object_image_source_set()
 */
EAPI void evas_object_image_source_clip_set(Evas_Object *obj, Eina_Bool source_clip) EINA_ARG_NONNULL(1);

/**
 * @brief   Checks whether an object is clipped by source object's clipper.
 * @since   1.8
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj  The proxy (image) object
 * @return  #EINA_TRUE if source clip is enabled,
 *          otherwise #EINA_FALSE if source clip is not enabled
 *
 * @see evas_object_clip_set()
 * @see evas_object_image_source_set()
 * @see evas_object_image_source_clip_set()
 */
EAPI Eina_Bool evas_object_image_source_clip_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * @brief   Checks whether a file extension is supported by @ref Evas_Object_Image.
 * @since   1.1
 *
 * @since_tizen 2.3
 *
 * @remarks If file is a Eina_Stringshare, use @ref evas_object_image_extension_can_load_fast_get directly.
 *
 * @remarks This function is threadsafe.
 *
 * @param[in]   file  The file to check
 * @return  #EINA_TRUE if the file extension is supported, \n
 *          otherwise #EINA_FALSE if it is not supported
 */
EAPI Eina_Bool                     evas_object_image_extension_can_load_get(const char *file);

/**
 * @brief   Checks whether a file extension is supported by @ref Evas_Object_Image.
 * @since   1.1
 *
 * @since_tizen 2.3
 *
 * @remarks This function is threadsafe.
 * @param[in]   file  The file to check \n
 *                It should be Eina_Stringshare.
 * @return  #EINA_TRUE if the file extension is supported, \n
 *          otherwise #EINA_FALSE if it is not supported
 */
EAPI Eina_Bool                     evas_object_image_extension_can_load_fast_get(const char *file);

/**
 * @brief   Checks whether an image object can be animated (have multiple frames).
 *
 * @details This function returns if the image file of an image object is capable of animation
 *          such as an animated gif file might. This is only useful to be called once
 *          the image object file has been set.
 * @since   1.1
 *
 * @since_tizen 2.3
 *
 * @remarks The following is an example:
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
 * @param[in]   obj  The Image object
 * @return  #EINA_TRUE if @a obj supports animation, \n
 *          otherwise #EINA_FALSE if it does not support animation
 *
 * @see evas_object_image_animated_get()
 * @see evas_object_image_animated_frame_count_get()
 * @see evas_object_image_animated_loop_type_get()
 * @see evas_object_image_animated_loop_count_get()
 * @see evas_object_image_animated_frame_duration_get()
 * @see evas_object_image_animated_frame_set()
 */
EAPI Eina_Bool                     evas_object_image_animated_get(const Evas_Object *obj);

/**
 * @brief   Gets the total number of frames of the image object.
 *
 * @details This returns the total number of frames the image object supports (if animated).
 * @since   1.1
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj  The Image object
 * @return  The number of frames
 *
 * @see evas_object_image_animated_get()
 * @see evas_object_image_animated_frame_count_get()
 * @see evas_object_image_animated_loop_type_get()
 * @see evas_object_image_animated_loop_count_get()
 * @see evas_object_image_animated_frame_duration_get()
 * @see evas_object_image_animated_frame_set()
 */
EAPI int                           evas_object_image_animated_frame_count_get(const Evas_Object *obj);

/**
 * @brief   Gets the kind of looping the image object does.
 *
 * @details This function returns the kind of looping the image object wants to do.
 * @since   1.1
 *
 * @since_tizen 2.3
 *
 * @remarks If it returns @c EVAS_IMAGE_ANIMATED_HINT_LOOP, you should display frames in a sequence like:
 *          1->2->3->1->2->3->1...
 *          If it returns @c EVAS_IMAGE_ANIMATED_HINT_PINGPONG, it is better to
 *          display frames in a sequence like: 1->2->3->2->1->2->3->1... 
 *          The default type is EVAS_IMAGE_ANIMATED_HINT_LOOP.
 *
 * @param[in]   obj  The Image object
 * @return  The Loop type of the image object
 *
 * @see evas_object_image_animated_get()
 * @see evas_object_image_animated_frame_count_get()
 * @see evas_object_image_animated_loop_type_get()
 * @see evas_object_image_animated_loop_count_get()
 * @see evas_object_image_animated_frame_duration_get()
 * @see evas_object_image_animated_frame_set()
 */
EAPI Evas_Image_Animated_Loop_Hint evas_object_image_animated_loop_type_get(const Evas_Object *obj);

/**
 * @brief   Gets the number of times the animation of the object loops.
 *
 * @details This function returns loop count of image. The loop count is the number of times
 *          the animation plays fully from first to last frame until the animation
 *          should stop (at the final frame).
 * @since   1.1
 *
 * @since_tizen 2.3
 *
 * @remarks If @c 0 is returned, then looping should happen indefinitely (no limit to
 *          the number of times it loops).
 *
 * @param[in]   obj  The Image object
 * @return  The number of loop of an animated image object
 *
 * @see evas_object_image_animated_get()
 * @see evas_object_image_animated_frame_count_get()
 * @see evas_object_image_animated_loop_type_get()
 * @see evas_object_image_animated_loop_count_get()
 * @see evas_object_image_animated_frame_duration_get()
 * @see evas_object_image_animated_frame_set()
 */
EAPI int                           evas_object_image_animated_loop_count_get(const Evas_Object *obj);

/**
 * @brief   Gets the duration of a sequence of frames.
 *
 * @details This function returns the total duration that the specified sequence of frames should
 *          take in seconds.
 * @since   1.1
 *
 * @since_tizen 2.3
 *
 * @remarks If you set start_frame to 1 and frame_num 0, you get frame 1's duration.
 *          If you set start_frame to 1 and frame_num 1, you get frame 1's duration +
 *          frame2's duration.
 *
 * @param[in]   obj          The Image object
 * @param[in]   start_frame  The first frame
 * @param[in]   fram_num     The number of frames in the sequence
 * @return The duraction of a sequence of frames.
 *
 * @see evas_object_image_animated_get()
 * @see evas_object_image_animated_frame_count_get()
 * @see evas_object_image_animated_loop_type_get()
 * @see evas_object_image_animated_loop_count_get()
 * @see evas_object_image_animated_frame_duration_get()
 * @see evas_object_image_animated_frame_set()
 */
EAPI double                        evas_object_image_animated_frame_duration_get(const Evas_Object *obj, int start_frame, int fram_num);

/**
 * @brief   Sets the frame to the current frame of an image object.
 *
 * @details This function sets the image object's current frame to frame_num 
 *          with 1 being the first frame.
 * @since   1.1
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj        The given image object
 * @param[in]   frame_num  The index of current frame
 *
 * @see evas_object_image_animated_get()
 * @see evas_object_image_animated_frame_count_get()
 * @see evas_object_image_animated_loop_type_get()
 * @see evas_object_image_animated_loop_count_get()
 * @see evas_object_image_animated_frame_duration_get()
 * @see evas_object_image_animated_frame_set()
 */
EAPI void                          evas_object_image_animated_frame_set(Evas_Object *obj, int frame_num);

/**
 * @}
 */

/**
 * @defgroup Evas_Object_Text Text Object Functions
 * @ingroup Evas_Object_Specific
 *
 * @brief  This group provides functions that operate on single line, single style text objects.
 *
 * For multiline and multiple style text, see @ref Evas_Object_Textblock.
 *
 * @remarks We do not guarantee any proper results if you create a Text object
 *          without setting the evas engine.
 *
 * @{
 */

/* Definition for basic styles (4 bits allocated use 0->10 now, 5 left) */
#define EVAS_TEXT_STYLE_MASK_BASIC 0xf

/**
 * @brief Definition for text style type creation macro. Use style types on the 's'
 *        arguments, being 'x' your style variable.
 */
#define EVAS_TEXT_STYLE_BASIC_SET(x, s) \
  do { x = ((x) & ~EVAS_TEXT_STYLE_MASK_BASIC) | (s); } while (0)

#define EVAS_TEXT_STYLE_MASK_SHADOW_DIRECTION (0x7 << 4)

/**
 * @brief Definition for text style type creation macro. This one imposes shadow
 *        directions on the style type variable -- use the @c
 *        EVAS_TEXT_STYLE_SHADOW_DIRECTION_* values on 's', incrementally.
 */
#define EVAS_TEXT_STYLE_SHADOW_DIRECTION_SET(x, s) \
  do { x = ((x) & ~EVAS_TEXT_STYLE_MASK_SHADOW_DIRECTION) | (s); } while (0)

/**
 * Types of styles to be applied on text objects.
 * The @c EVAS_TEXT_STYLE_SHADOW_DIRECTION_* ones are to be ORed together with
 * others imposing shadow, to change shadow's direction
 */
typedef enum _Evas_Text_Style_Type
{
   EVAS_TEXT_STYLE_PLAIN,      /**< Plain, standard text */
   EVAS_TEXT_STYLE_SHADOW,      /**< Text with shadow underneath */
   EVAS_TEXT_STYLE_OUTLINE,      /**< Text with an outline */
   EVAS_TEXT_STYLE_SOFT_OUTLINE,      /**< Text with a soft outline */
   EVAS_TEXT_STYLE_GLOW,      /**< Text with a glow effect */
   EVAS_TEXT_STYLE_OUTLINE_SHADOW,      /**< Text with both outline and shadow effects */
   EVAS_TEXT_STYLE_FAR_SHADOW,      /**< Text with (far) shadow underneath */
   EVAS_TEXT_STYLE_OUTLINE_SOFT_SHADOW,      /**< Text with outline and soft shadow effects combined */
   EVAS_TEXT_STYLE_SOFT_SHADOW,      /**< Text with (soft) shadow underneath */
   EVAS_TEXT_STYLE_FAR_SOFT_SHADOW,      /**< Text with (far soft) shadow underneath */
   // TIZEN ONLY (20131106) : Font effect for tizen.
   EVAS_TEXT_STYLE_TIZEN_GLOW_SHADOW,      /** Tizen::Graphics::Effect::EFFECT_FOR_TEXT_ON_HOME_SCREEN_ICON */
   EVAS_TEXT_STYLE_TIZEN_SOFT_GLOW_SHADOW,      /** Tizen::Graphics::Effect::EFFECT_FOR_TEXT_ON_DYNAMIC_BOX_IMPORT_IMAGE */
   EVAS_TEXT_STYLE_TIZEN_SHADOW,      /** Tizen::Graphics::Effect::EFFECT_FOR_TEXT_ON_DYNAMIC_BOX_IMAGE */
   //////////////////////////////

   /* OR these to modify shadow direction (3 bits needed) */
   EVAS_TEXT_STYLE_SHADOW_DIRECTION_BOTTOM_RIGHT = (0x0 << 4),      /**< Shadow growing to bottom right */
   EVAS_TEXT_STYLE_SHADOW_DIRECTION_BOTTOM = (0x1 << 4),            /**< Shadow growing to the bottom */
   EVAS_TEXT_STYLE_SHADOW_DIRECTION_BOTTOM_LEFT = (0x2 << 4),       /**< Shadow growing to bottom left */
   EVAS_TEXT_STYLE_SHADOW_DIRECTION_LEFT = (0x3 << 4),              /**< Shadow growing to the left */
   EVAS_TEXT_STYLE_SHADOW_DIRECTION_TOP_LEFT = (0x4 << 4),          /**< Shadow growing to top left */
   EVAS_TEXT_STYLE_SHADOW_DIRECTION_TOP = (0x5 << 4),               /**< Shadow growing to the top */
   EVAS_TEXT_STYLE_SHADOW_DIRECTION_TOP_RIGHT = (0x6 << 4),         /**< Shadow growing to top right */
   EVAS_TEXT_STYLE_SHADOW_DIRECTION_RIGHT = (0x7 << 4)             /**< Shadow growing to the right */
} Evas_Text_Style_Type;

/**
 * @brief   Creates a new text object on the provided canvas.
 *
 * @since_tizen 2.3
 *
 * @remarks Text objects are for simple, single line text elements. If you want
 *          more elaborated text blocks, see @ref Evas_Object_Textblock.
 *
 * @param[in]   e  The canvas to create the text object on
 * @return  A pointer to a new text object on success, \n
 *          otherwise @c NULL on error
 *
 * @see evas_object_text_font_source_set()
 * @see evas_object_text_font_set()
 * @see evas_object_text_text_set()
 */
EAPI Evas_Object         *evas_object_text_add(Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * @brief   Sets the font (source) file to be used on a given text object.
 *
 * @details This function allows the font file to be explicitly set for a given
 *          text object, overriding system lookup, which first occurs in
 *          the given file's contents.
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj   The text object to set font for
 * @param[in]   font  The font file's path
 *
 * @see evas_object_text_font_get()
 */
EAPI void                 evas_object_text_font_source_set(Evas_Object *obj, const char *font) EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the font file's path which is being used on a given text object.
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj  The text object to set the font for
 * @return  The font file's path
 *
 * @see evas_object_text_font_get() for more details
 */
EAPI const char          *evas_object_text_font_source_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief   Sets the font family and size on a given text object.
 *
 * @details This function allows the font name and size of a text object to be
 *          set. The @a font string has to follow fontconfig's convention on
 *          naming fonts, as it is the underlying library used to query system
 *          fonts by Evas (see the @c fc-list command's output, on your system,
 *          to get an idea).
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj   The text object to set font for
 * @param[in]   font  The font (family) name
 * @param[in]   size  The font size, in points
 *
 * @see evas_object_text_font_get()
 * @see evas_object_text_font_source_set()
 */
EAPI void                 evas_object_text_font_set(Evas_Object *obj, const char *font, Evas_Font_Size size) EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the font family and size in use on a given text object.
 *
 * @details This function allows the font name and size of a text object to be
 *          queried. Be aware that the font name string is still owned by Evas
 *          and should @b not have free() called on it by the caller of the
 *          function.
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj   The evas text object to query for font information
 * @param[out]   font  A pointer to the location to store the font name in
 * @param[out]   size  A pointer to the location to store the font size in
 *
 * @see evas_object_text_font_set()
 */
EAPI void                 evas_object_text_font_get(const Evas_Object *obj, const char **font, Evas_Font_Size *size) EINA_ARG_NONNULL(1);

/**
 * @brief  Sets the text string to be displayed by the given text object.
 *
 * @since_tizen 2.3
 *
 * @param[in]  obj   The text object to set text string on
 * @param[in]  text  The text string to display on it
 *
 * @see evas_object_text_text_get()
 */
EAPI void                 evas_object_text_text_set(Evas_Object *obj, const char *text) EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the text string currently being displayed by the given text object.
 *
 * @since_tizen 2.3
 *
 * @remarks Do not free() the return value.
 *
 * @param[in]   obj  The given text object
 * @return  The text string currently being displayed on it
 *
 * @see evas_object_text_text_set()
 */
EAPI const char          *evas_object_text_text_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief   Sets the BiDi delimiters used in the textblock. 
 * @since   1.1
 *
 * @since_tizen 2.3
 *
 * @remarks BiDi delimiters are use for in-paragraph separation of bidi segments. This
 *          is useful for example in recipients fields of e-mail clients where bidi
 *          oddities can occur when mixing RTL and LTR.
 *
 * @param[in]   obj    The given text object
 * @param[in]   delim  A null terminated string of delimiters, for example ",|"
 */
EAPI void                 evas_object_text_bidi_delimiters_set(Evas_Object *obj, const char *delim);

/**
 * @brief   Gets the BiDi delimiters used in the textblock.
 * @since   1.1
 *
 * @since_tizen 2.3
 *
 * @remarks BiDi delimiters are use for in-paragraph separation of bidi segments. This
 *          is useful for example in recipients fields of e-mail clients where bidi
 *          oddities can occur when mixing RTL and LTR.
 *
 * @param[in]   obj  The given text object
 * @return  A null terminated string of delimiters, for example, ",|", \n 
 *          otherwise @c NULL if empty
 */
EAPI const char          *evas_object_text_bidi_delimiters_get(const Evas_Object *obj);

/**
 * @brief   Sets the ellipsis that should be used for the text object.
 *
 * @details This is a value between @c 0.0 and @c 1.0 indicating the position of the text
 *          to be shown. @c 0.0 means the start is shown and the end trimmed, @c 1.0
 *          means the beginning is trimmed and the end is shown, and any value
 *          in between causes ellipsis to be added in both the end of the text and the
 *          requested part to be shown.
 * @since   1.8
 *
 * @since_tizen 2.3
 *
 * @remarks @c -1.0 means ellipsis is turned off.
 *
 * @param[in]   obj       The given text object
 * @param[in]   ellipsis  The ellipsis
 */
EAPI void                 evas_object_text_ellipsis_set(Evas_Object *obj, double ellipsis);

/**
 * @brief   Gets the ellipsis currently set on the text object.
 * @since   1.8
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj  The given text object
 * @return  The ellipsis set on the text object
 * @see evas_object_text_ellipsis_set()
 */
EAPI double               evas_object_text_ellipsis_get(const Evas_Object *obj);

/**
 * @internal
 */
EAPI Evas_Coord           evas_object_text_ascent_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @internal
 */
EAPI Evas_Coord           evas_object_text_descent_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @internal
 */
EAPI Evas_Coord           evas_object_text_max_ascent_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @internal
 */
EAPI Evas_Coord           evas_object_text_max_descent_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @internal
 */
EAPI Evas_Coord           evas_object_text_horiz_advance_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @internal
 */
EAPI Evas_Coord           evas_object_text_vert_advance_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @internal
 */
EAPI Evas_Coord           evas_object_text_inset_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the position and dimension information of a character within a text @c Evas_Object.
 *
 * @details This function is used to obtain the X, Y, width and height of a the character
 *          located at @a pos within the @c Evas_Object @a obj. @a obj must be a text object
 *          as created with evas_object_text_add(). Any of the @c Evas_Coord parameters (@p cx,
 *          @a cy, @a cw, @a ch) may be @c NULL in which case no value is assigned to that
 *          parameter.
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj  The text object to retrieve position information for
 * @param[in]   pos  The character position to request co-ordinates for
 * @param[out]   cx   A pointer to an @c Evas_Coord to store the X value in (can be @c NULL)
 * @param[out]   cy   A pointer to an @c Evas_Coord to store the Y value in (can be @c NULL)
 * @param[out]   cw   A pointer to an @c Evas_Coord to store the Width value in (can be @c NULL)
 * @param[out]   ch   A pointer to an @c Evas_Coord to store the Height value in (can be @c NULL)
 * @return  #EINA_FALSE if the information is obtained successfully, 
 *          otherwise #EINA_TRUE on error
 */
EAPI Eina_Bool            evas_object_text_char_pos_get(const Evas_Object *obj, int pos, Evas_Coord *cx, Evas_Coord *cy, Evas_Coord *cw, Evas_Coord *ch) EINA_ARG_NONNULL(1);

/**
 * @internal
 */
EAPI int                  evas_object_text_char_coords_get(const Evas_Object *obj, Evas_Coord x, Evas_Coord y, Evas_Coord *cx, Evas_Coord *cy, Evas_Coord *cw, Evas_Coord *ch) EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the logical position of the last char in the text up to the given position. \n
 *          This is NOT the position of the last char because of the possibility of RTL in the text.
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj     The text object
 * @param[in]   x       The X co-ordinate
 * @param[in]   y       The Y co-ordinate
 * @return  The logical The positoin of the last char in the text up to the given position
 */
EAPI int                  evas_object_text_last_up_to_pos(const Evas_Object *obj, Evas_Coord x, Evas_Coord y) EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the style on use on the given text object.
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj  The given text object to set style on
 * @return  The style type in use
 *
 * @see evas_object_text_style_set() for more details.
 */
EAPI Evas_Text_Style_Type evas_object_text_style_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief   Sets the style to apply on the given text object.
 *
 * @since_tizen 2.3
 *
 * @remarks Valid text object styles are defined in #Evas_Text_Style_Type. 
 *          Some of those values are combinations of more than one style, 
 *          and some account for the direction of the rendering of shadow effects.
 *
 * @remarks You can use the helper macros #EVAS_TEXT_STYLE_BASIC_SET and
 *          #EVAS_TEXT_STYLE_SHADOW_DIRECTION_SET to assemble a style value.
 *
 * @remarks The following figure illustrates the text styles:
 *
 * @image html text-styles.png
 * @image rtf text-styles.png
 * @image latex text-styles.eps
 *
 * @param[in]   obj   The given text object to set style on
 * @param[in]   type  The style type
 *
 * @see evas_object_text_style_get()
 * @see evas_object_text_shadow_color_set()
 * @see evas_object_text_outline_color_set()
 * @see evas_object_text_glow_color_set()
 * @see evas_object_text_glow2_color_set()
 */
EAPI void                 evas_object_text_style_set(Evas_Object *obj, Evas_Text_Style_Type type) EINA_ARG_NONNULL(1);

/**
 * @brief   Sets the shadow color for the given text object.
 *
 * @since_tizen 2.3
 *
 * @remarks Shadow effects, which are fading colors decorating the text
 *          underneath it, are just shown if the object is set to one of
 *          the following styles:
 *          - #EVAS_TEXT_STYLE_SHADOW
 *          - #EVAS_TEXT_STYLE_OUTLINE_SHADOW
 *          - #EVAS_TEXT_STYLE_FAR_SHADOW
 *          - #EVAS_TEXT_STYLE_OUTLINE_SOFT_SHADOW
 *          - #EVAS_TEXT_STYLE_SOFT_SHADOW
 *          - #EVAS_TEXT_STYLE_FAR_SOFT_SHADOW
 *
 * @remarks You can also change the direction where the shadow grows to, with
 *          evas_object_text_style_set().
 *
 * @param[in]   obj  The given Evas text object
 * @param[in]   r    The red component of the given color
 * @param[in]   g    The green component of the given color
 * @param[in]   b    The blue component of the given color
 * @param[in]   a    The alpha component of the given color
 *
 * @see evas_object_text_shadow_color_get()
 */
EAPI void                 evas_object_text_shadow_color_set(Evas_Object *obj, int r, int g, int b, int a) EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the shadow color for the given text object.
 *
 * @since_tizen 2.3
 *
 * @remarks Use @c NULL pointers on the color components that you are not
 *          interested in: they are ignored by the function.
 *
 * @param[in]   obj  The given Evas text object
 * @param[out]   r    The pointer to variable to hold the red component of the given color
 * @param[out]   g    The pointer to variable to hold the green component of the given color
 * @param[out]   b    The pointer to variable to hold the blue component of the given color
 * @param[out]   a    The pointer to variable to hold the alpha component of the given color
 *
 * @see evas_object_text_shadow_color_set() for more details.
 */
EAPI void                 evas_object_text_shadow_color_get(const Evas_Object *obj, int *r, int *g, int *b, int *a) EINA_ARG_NONNULL(1);

/**
 * @brief   Sets the glow color for the given text object.
 *
 * @since_tizen 2.3
 *
 * @remarks Glow effects, which are glowing colors decorating the text's
 *          surroundings, are shown if the object is set to the
 *          #EVAS_TEXT_STYLE_GLOW style.
 *
 * @remarks Glow effects are placed from a short distance of the text
 *          itself, but no touching it. For glowing effects right on the
 *          borders of the glyphs, see 'glow 2' effects
 *          (evas_object_text_glow2_color_set()).
 *
 * @param[in]   obj  The given Evas text object
 * @param[in]   r    The red component of the given color
 * @param[in]   g    The green component of the given color
 * @param[in]   b    The blue component of the given color
 * @param[in]   a    The alpha component of the given color
 *
 * @see evas_object_text_glow_color_get()
 */
EAPI void                 evas_object_text_glow_color_set(Evas_Object *obj, int r, int g, int b, int a) EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the glow color for the given text object.
 *
 * @since_tizen 2.3
 *
 * @remarks Use @c NULL pointers on the color components that you are not
 *          interested in: they are ignored by the function.
 *
 * @param[in]   obj  The given Evas text object.
 * @param[out]   r    The pointer to variable to hold the red component of the given color
 * @param[out]   g    The pointer to variable to hold the green component of the given color
 * @param[out]   b    The pointer to variable to hold the blue component of the given color
 * @param[out]   a    The pointer to variable to hold the alpha component of the given color
 *
 * @see evas_object_text_glow_color_set() for more details.
 */
EAPI void                 evas_object_text_glow_color_get(const Evas_Object *obj, int *r, int *g, int *b, int *a) EINA_ARG_NONNULL(1);

/**
 * @brief   Sets the 'glow 2' color for the given text object.
 *
 * @since_tizen 2.3
 *
 * @remarks 'Glow 2' effects, which are glowing colors decorating the text's
 *          (immediate) surroundings, are shown if the object is set
 *          to the #EVAS_TEXT_STYLE_GLOW style. See also
 *          evas_object_text_glow_color_set().
 *
 * @param[in]   obj  The given Evas text object
 * @param[in]   r    The red component of the given color
 * @param[in]   g    The green component of the given color
 * @param[in]   b    The blue component of the given color
 * @param[in]   a    The alpha component of the given color
 *
 * @see evas_object_text_glow2_color_get()
 */
EAPI void                 evas_object_text_glow2_color_set(Evas_Object *obj, int r, int g, int b, int a) EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the 'glow 2' color for the given text object.
 *
 * @since_tizen 2.3
 *
 * @remarks Use @c NULL pointers on the color components that you are not
 *          interested in: they are ignored by the function.
 *
 * @param[in]   obj  The given Evas text object
 * @param[out]   r    The pointer to variable to hold the red component of the given color
 * @param[out]   g    The pointer to variable to hold the green component of the given color
 * @param[out]   b    The pointer to variable to hold the blue component of the given color
 * @param[out]   a    The pointer to variable to hold the alpha component of the given color
 *
 * @see evas_object_text_glow2_color_set() for more details.
 */
EAPI void                 evas_object_text_glow2_color_get(const Evas_Object *obj, int *r, int *g, int *b, int *a) EINA_ARG_NONNULL(1);

/**
 * @brief   Sets the outline color for the given text object.
 *
 * @since_tizen 2.3
 *
 * @remarks Outline effects (colored lines around text glyphs) are just
 *          shown if the object is set to one of the following styles:
 *          - #EVAS_TEXT_STYLE_OUTLINE
 *          - #EVAS_TEXT_STYLE_SOFT_OUTLINE
 *          - #EVAS_TEXT_STYLE_OUTLINE_SHADOW
 *          - #EVAS_TEXT_STYLE_OUTLINE_SOFT_SHADOW
 *
 * @param[in]   obj  The given Evas text object
 * @param[in]   r    The red component of the given color
 * @param[in]   g    The green component of the given color
 * @param[in]   b    The blue component of the given color
 * @param[in]   a    The alpha component of the given color
 *
 * @see evas_object_text_outline_color_get()
 */
EAPI void                 evas_object_text_outline_color_set(Evas_Object *obj, int r, int g, int b, int a) EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the outline color for the given text object.
 *
 * @since_tizen 2.3
 *
 * @remarks Use @c NULL pointers on the color components that you are not
 *          interested in: they are ignored by the function.
 *
 * @param[in]   obj  The given Evas text object
 * @param[out]   r    The pointer to the variable to hold the red component of the given color
 * @param[out]   g    The pointer to the variable to hold the green component of the given color
 * @param[out]   b    The pointer to the variable to hold the blue component of the given color
 * @param[out]   a    The pointer to the variable to hold the alpha component of the given color
 *
 * @see evas_object_text_outline_color_set() for more details.
 */
EAPI void                 evas_object_text_outline_color_get(const Evas_Object *obj, int *r, int *g, int *b, int *a) EINA_ARG_NONNULL(1);

/**
 * @brief  Gets the text style pad of a text object.
 *
 * @since_tizen 2.3
 *
 * @param[in]  obj  The given text object
 * @param[out]  l    The left pad (or @c NULL)
 * @param[out]  r    The right pad (or @c NULL)
 * @param[out]  t    The top pad (or @c NULL)
 * @param[out]  b    The bottom pad (or @c NULL)
 *
 */
EAPI void                 evas_object_text_style_pad_get(const Evas_Object *obj, int *l, int *r, int *t, int *b) EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the direction of the text currently being displayed in the text object.
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj  The given evas text object
 * @return  The direction of the text
 */
EAPI Evas_BiDi_Direction  evas_object_text_direction_get(const Evas_Object *obj) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

/**
 * @brief      Sets an Evas filter program on this Text Object.
 * @since      1.9. Backported for Tizen (1.7.99).
 *
 * @since_tizen 2.3
 *
 * @remarks    If the program fails to compile (syntax error, invalid buffer name, etc...),
 *             the standard text effects are applied instead (SHADOW, etc...).
 *
 * @remarks    EXPERIMENTAL FEATURE. This is an unstable API, please use this only for testing purposes.
 *
 * @param[in]  obj  The given evas text object
 * @param[in]  arg  The program code, as defined by the "Evas filters script language".
 *                  Pass @c NULL to remove the former program and
 *                  switch back to the standard text effects.
 *
 * @internal
 * @see @ref evasfiltersref "Evas filters reference"
 * @endinternal
 */
EAPI void                 evas_object_text_filter_program_set(Evas_Object *obj, const char *arg);

/**
 * @brief      Binds an object to use as a mask or texture with Evas Filters. This
 *             automatically creates a new RGBA buffer containing the source object's
 *             pixels (as it is rendered).
 * @since      1.9. Backported for Tizen (1.7.99).
 *
 * @since_tizen 2.3
 *
 * @remarks    EXPERIMENTAL FEATURE. This is an unstable API, please use this only for testing purposes.
 *
 * @param[in]  obj  The given evas object
 * @param[in]  name   The object name as used in the program code
 * @param[in]  source The evas object to use through proxy rendering
 *
 * @see evas_obj_text_filter_program_set
 * @internal
 * @see @ref evasfiltersref "Evas filters reference"
 * @endinternal
 */
EAPI void                 evas_object_text_filter_source_set(Evas_Object *obj, const char *name, Evas_Object *source);

/**
 * @}
 */

/**
 * @defgroup Evas_Object_Textblock Textblock Object Functions
 * @ingroup Evas_Object_Specific
 *
 * @brief  This group provides functions used to create and manipulate textblock objects. 
 *
 * Unlike @ref Evas_Object_Text, these handle complex text, doing multiple
 * styles and multiline text based on HTML-like tags. Of these extra
 * features are heavier on memory and processing cost.
 *
 * @section Evas_Object_Textblock_Tutorial Textblock Object Tutorial
 *
 * This part explains about the textblock object's API and proper usage.
 * The main user of the textblock object is the edje entry object in Edje. So
 * that is a good place to learn from, but this document is more than
 * enough. If it is not, please contact the developer to update it.
 *
 * @subsection textblock_intro Introduction
 * The textblock object is, as implied, an object that can show big chunks of
 * text. Textblock supports many features including: Text formatting, automatic
 * and manual text alignment, embedding items (for example icons) and more.
 * Textblock has three important parts: the text paragraphs, the format nodes,
 * and the cursors.
 *
 * You can use markup to format text, for example: "<font_size=50>Big!</font_size>".
 * You can also put more than one style directive in one tag:
 * "<font_size=50 color=#F00>Big and Red!</font_size>".
 * Please notice that we used "</font_size>" although the format also included
 * color. This is because the first format determines the matching closing tag's
 * name. You can also use anonymous tags, like: "<font_size=30>Big</>" which
 * just pop any type of format, but it is advised to use the named alternatives
 * instead.
 *
 * @subsection textblock_cursors Textblock Object Cursors
 * A textblock cursor is the data type that represents
 * a position in a textblock. Each cursor contains information about the
 * paragraph it points to, the position in that paragraph, and the object itself.
 * Cursors register to textblock objects upon creation. This means that once
 * you created a cursor, it belongs to a specific object and you cannot, for example,
 * copy a cursor "into" a cursor of a different object. Registered cursors
 * also have the added benefit of updating automatically upon textblock changes,
 * this means that if you have a cursor pointing to a specific character, it
 * still points to it even after you change the whole object completely (as long
 * as the char is not deleted). This is not possible without updating, because
 * as mentioned, each cursor holds a character position. There are many
 * functions that handle cursors. See the evas_textblock_cursor*
 * functions. For creation and deletion of cursors, see:
 * @see evas_object_textblock_cursor_new()
 * @see evas_textblock_cursor_free()
 * @remarks Cursors are generally the correct way to handle text in the textblock object, 
 *          and there are enough functions to do everything you need with them 
 *          (no need to get big chunks of text and processing them yourself).
 *
 * @subsection textblock_paragraphs Textblock Object Paragraphs
 * The textblock object is made out of text split to paragraphs (delimited
 * by the paragraph separation character). Each paragraph has many (or none)
 * format nodes associated with it which are responsible for the formatting
 * of that paragraph.
 *
 * @subsection textblock_format_nodes Textblock Object Format Nodes
 * As explained in @ref textblock_paragraphs each one of the format nodes
 * is associated with a paragraph.
 * There are two types of format nodes: visible and invisible.
 * Visible: Formats that a cursor can point to, that is, the formats that
 * occupy space. For example: newlines, tabs, items and so on. Some visible items
 * are made of two parts. In this case, only the opening tag is visible.
 * A closing tag (i.e a \</tag\> tag) should NEVER be visible.
 * Invisible: Formats that do not occupy space. For example: bold and underline.
 * Being able to access format nodes is very important for some uses. For
 * example, edje uses the "<a>" format to create links in the text (and pop
 * popups above them when clicked). For the textblock object a is just a
 * formatting instruction (how to color the text), but edje utilizes the access
 * to the format nodes to make it do more.
 * For more information, see all the evas_textblock_node_format_* functions.
 * The translation of "<tag>" tags to actual format is done according to the
 * tags defined in the style, see @ref evas_textblock_style_set
 *
 * @subsection textblock_special_formats Special Formats
 * Textblock supports various format directives that can be used in markup. In
 * addition to the mentioned format directives, textblock allows creating
 * additional format directives using "tags" that can be set in the style. See
 * @ref evas_textblock_style_set .
 *
 * Textblock supports the following formats:
 * @li font - Font description in fontconfig like format. Example: "Sans:style=Italic:lang=hi". or "Serif:style=Bold".
 * @li font_weight - Overrides the weight defined in "font". Example: "font_weight=Bold" is the same as "font=:style=Bold". Supported weights: "normal", "thin", "ultralight", "light", "book", "medium", "semibold", "bold", "ultrabold", "black", and "extrablack".
 * @li font_style - Overrides the style defined in "font". Example: "font_style=Italic" is the same as "font=:style=Italic". Supported styles: "normal", "oblique", and "italic".
 * @li font_width - Overrides the width defined in "font". E.g: "font_width=Condensed" is the same as "font=:style=Condensed". Supported widths: "normal", "ultracondensed", "extracondensed", "condensed", "semicondensed", "semiexpanded", "expanded", "extraexpanded", and "ultraexpanded".
 * @li lang - Overrides the language defined in "font". Example: "lang=he" is the same as "font=:lang=he".
 * @li font_fallbacks - A comma delimited list of fonts to try if finding the main font fails.
 * @li font_size - The font size in points.
 * @li font_source - The source of the font. Example: An eet file.
 * @li color - Text color in one of the following formats: "#RRGGBB", "#RRGGBBAA", "#RGB", and "#RGBA".
 * @li underline_color - Color in one of the following formats: "#RRGGBB", "#RRGGBBAA", "#RGB", and "#RGBA".
 * @li underline2_color - Color in one of the following formats: "#RRGGBB", "#RRGGBBAA", "#RGB", and "#RGBA".
 * @li outline_color - Color in one of the following formats: "#RRGGBB", "#RRGGBBAA", "#RGB", and "#RGBA".
 * @li shadow_color - Color in one of the following formats: "#RRGGBB", "#RRGGBBAA", "#RGB", and "#RGBA".
 * @li glow_color - Color in one of the following formats: "#RRGGBB", "#RRGGBBAA", "#RGB", and "#RGBA".
 * @li glow2_color - Color in one of the following formats: "#RRGGBB", "#RRGGBBAA", "#RGB", and "#RGBA".
 * @li strikethrough_color - Color in one of the following formats: "#RRGGBB", "#RRGGBBAA", "#RGB", and "#RGBA".
 * @li align - Either "auto" (meaning according to text direction), "left", "right", "center", "middle", a value between @c 0.0 and @c 1.0, or a value between 0% to 100%.
 * @li valign - Either "top", "bottom", "middle", "center", "baseline", "base", a value between @c 0.0 and @c 1.0, or a value between 0% to 100%.
 * @li wrap - "word", "char", "mixed", or "none".
 * @li left_margin - Either "reset", or a pixel value indicating the margin.
 * @li right_margin - Either "reset", or a pixel value indicating the margin.
 * @li underline - "on", "off", "single", or "double".
 * @li strikethrough - "on" or "off"
 * @li backing_color - Background color in one of the following formats: "#RRGGBB", "#RRGGBBAA", "#RGB", and "#RGBA".
 * @li backing - Enable or disable background color. Example: "on" or "off"
 * @li style - Either "off", "none", "plain", "shadow", "outline", "soft_outline", "outline_shadow", "outline_soft_shadow", "glow", "far_shadow", "soft_shadow", or "far_soft_shadow".
 * @li tabstops - Pixel value for tab width.
 * @li linesize - Force a line size in pixels.
 * @li linerelsize - Either a floating point value or a percentage indicating the wanted size of the line relative to the calculated size.
 * @li linegap - Force a line gap in pixels.
 * @li linerelgap - Either a floating point value or a percentage indicating the wanted size of the line relative to the calculated size.
 * @li item - Creates an empty space that should be filled by an upper layer. Use "size", "abssize", or "relsize". To define the items size, and an optional: vsize=full/ascent to define the item's position in the line.
 * @li linefill - Either a float value or percentage indicating how much to fill the line.
 * @li ellipsis - Value between @c 0.0-@c 1.0 to indicate the type of ellipsis, or @c -1.0 to indicate that ellipsis is not wanted.
 * @li password - "on" or "off". This is used to specifically turn replacing chars with the replacement char (i.e password mode) on and off.
 *
 * @remarks There is no guarantee of any proper results if you create a Textblock
 *          object without setting the evas engine.
 *
 * @todo put here some usage examples
 *
 * @{
 */

typedef struct _Evas_Textblock_Style              Evas_Textblock_Style;    /**< @brief handle for Evas Textblock Style */
typedef struct _Evas_Textblock_Cursor             Evas_Textblock_Cursor;   /**< @brief handle for Evas Textblock Cursor */
/**
 * @typedef Evas_Object_Textblock_Node_Format
 * @brief The structure type containing a format node.
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

/**
 * @brief  Adds a textblock to the given evas.
 *
 * @since_tizen 2.3
 *
 * @param[in]  e  The given evas
 * @return The new textblock object
 */
EAPI Evas_Object                             *evas_object_textblock_add(Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * @brief   Gets the unescaped version of escape.
 *
 * @since_tizen 2.3
 *
 * @param[in]   escape  The string to be escaped
 * @return  The unescaped version of escape
 */
EAPI const char                              *evas_textblock_escape_string_get(const char *escape) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the escaped version of the string.
 *
 * @since_tizen 2.3
 *
 * @param[in]   string   The string to escape
 * @param[out]   len_ret  The len of the part of the string that is used
 * @return  The escaped string
 */
EAPI const char                              *evas_textblock_string_escape_get(const char *string, int *len_ret) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the unescaped version of the string between start and end.
 *
 * @since_tizen 2.3
 *
 * @param[in]   escape_start  The start of the string
 * @param[in]   escape_end    The end of the string
 * @return  The unescaped version of the range
 */
EAPI const char                              *evas_textblock_escape_string_range_get(const char *escape_start, const char *escape_end) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1, 2);

/**
 * @brief   Gets the plain version of the markup.
 *
 * @details This function works as if you set the markup to a textblock and then retrieve the plain
 *          version of the text. That is: <br> and <\n> is replaced with \n, &...; with
 *          the actual char and so on.
 *
 * @since   1.2
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj   The textblock object to work with \n
 *                If @c NULL, it tries the default.
 * @param[in]   text  The markup text \n
 *                If @c NULL, it returns @c NULL.
 * @return  An allocated plain text version of the markup
 */
EAPI char                                    *evas_textblock_text_markup_to_utf8(const Evas_Object *obj, const char *text) EINA_WARN_UNUSED_RESULT EINA_MALLOC;

/**
 * @brief   Gets the markup version of the plain text.
 *
 * @details This function replaces \\n -\> \<br/\> \\t -\> \<tab/\> and so on. 
 *          This is generally needed before you pass plain text to be set in a textblock.
 * @since   1.2
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj   The textblock object to work with \n
 *                If @c NULL, it just does the default behaviour, i.e with no extra object information.
 * @param[in]   text  The markup text \n
 *                If @c NULL, it returns @c NULL.
 * @return  An allocated plain text version of the markup
 */
EAPI char                                    *evas_textblock_text_utf8_to_markup(const Evas_Object *obj, const char *text) EINA_WARN_UNUSED_RESULT EINA_MALLOC;

/**
 * @brief   Creates a new textblock style.
 *
 * @since_tizen 2.3
 *
 * @return  The new textblock style
 */
EAPI Evas_Textblock_Style                    *evas_textblock_style_new(void) EINA_WARN_UNUSED_RESULT EINA_MALLOC;

/**
 * @brief   Destroys a textblock style.
 *
 * @since_tizen 2.3
 *
 * @param[in]   ts  The textblock style to free
 */
EAPI void                                     evas_textblock_style_free(Evas_Textblock_Style *ts) EINA_ARG_NONNULL(1);

/**
 * @brief   Sets the style @a ts to the style passed as text by text.
 * @details This function expects a string consisting of many (or none) tag='format' pairs.
 *
 * @since_tizen 2.3
 *
 * @param[in]   ts    The style to set
 * @param[in]   text  The text to parse \n
 *                This should be NOT NULL.
 */
EAPI void                                     evas_textblock_style_set(Evas_Textblock_Style *ts, const char *text) EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the text of the style @a ts.
 *
 * @since_tizen 2.3
 *
 * @param[in]   ts  The style to get the text
 * @return  The text of the style, \n
 *          otherwise @c NULL on error
 */
EAPI const char                              *evas_textblock_style_get(const Evas_Textblock_Style *ts) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief  Sets the object style to @a ts.
 *
 * @since_tizen 2.3
 *
 * @param[in]  obj  The Evas object to set the style to
 * @param[in]  ts   The style to set
  */
EAPI void                                     evas_object_textblock_style_set(Evas_Object *obj, Evas_Textblock_Style *ts) EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the style of an object.
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj  The object to get the style from
 * @return  The style of the object
 */
EAPI const Evas_Textblock_Style              *evas_object_textblock_style_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief   Pushes @a ts to the top of the user style stack.
 *
 * FIXME: API is solid but currently only supports 1 style in the stack.
 *
 * @since   1.2
 *
 * @since_tizen 2.3
 *
 * @remarks The user style overrides the corresponding elements of the regular style.
 *          This is the proper way to do theme overrides in code.
 * @param[in]   obj  The Evas object to set the style to
 * @param[in]   ts   The style to set
 * @see evas_object_textblock_style_set
 */
EAPI void                                     evas_object_textblock_style_user_push(Evas_Object *obj, Evas_Textblock_Style *ts) EINA_ARG_NONNULL(1);

/**
 * @brief   Deletes the style from the top of the user style stack.
 * @since   1.2
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj  The object to get the style from
 * @see evas_object_textblock_style_get
 */
EAPI void                                     evas_object_textblock_style_user_pop(Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * @brief   Gets (does not remove) the style at the top of the user style stack.
 * @since   1.2
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj  The object to get the style from
 * @return  The style of the object
 * @see evas_object_textblock_style_get
 */
EAPI const Evas_Textblock_Style              *evas_object_textblock_style_user_peek(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief   Sets the "replacement character" to use for the given textblock object.
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj  The given textblock object
 * @param[in]   ch   The charset name
 */
EAPI void                                     evas_object_textblock_replace_char_set(Evas_Object *obj, const char *ch) EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the "replacement character" for given textblock object. 
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj  The given textblock object
 * @return  The replacement character, \n
 *          otherwise @c NULL if no replacement character is in use
 */
EAPI const char                              *evas_object_textblock_replace_char_get(Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief   Sets the vertical alignment of text within the textblock object as a whole.
 * @since   1.1
 *
 * @since_tizen 2.3
 *
 * @remarks Normally alignment is @c 0.0 (top of object). Values given should be
 *          between @c 0.0 and @c 1.0 , where @c 1.0 is bottom of object, @c 0.5 is 
 *          vertically centered, and so on.
 *
 * @param[in]   obj    The given textblock object
 * @param[in]   align  A value between @c 0.0 and @c 1.0
 */
EAPI void                                     evas_object_textblock_valign_set(Evas_Object *obj, double align);

/**
 * @brief   Gets the vertical alignment of a textblock.
 * @since   1.1
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj  The given textblock object
 * @return  The alignment set for the object
 */
EAPI double                                   evas_object_textblock_valign_get(const Evas_Object *obj);

/**
 * @brief   Sets the BiDi delimiters used in the textblock.
 * @since   1.1
 *
 * @since_tizen 2.3
 *
 * @remarks BiDi delimiters are uses for in-paragraph separation of bidi segments. This
 *          is useful, for example, in recipients fields of e-mail clients where bidi
 *          oddities can occur when mixing RTL and LTR.
 *
 * @param[in]   obj    The given textblock object
 * @param[in]   delim  A null terminated string of delimiters, for example ",|"
 */
EAPI void                                     evas_object_textblock_bidi_delimiters_set(Evas_Object *obj, const char *delim);

/**
 * @brief   Gets the BiDi delimiters used in the textblock.
 *
 * @since   1.1
 *
 * @since_tizen 2.3
 *
 * @remarks BiDi delimiters are used for in-paragraph separation of bidi segments. This
 *          is useful, for example, in recipients fields of e-mail clients where bidi
 *          oddities can occur when mixing RTL and LTR.
 * 
 * @param[in]   obj  The given textblock object
 * @return  A null terminated string of delimiters, for example ",|", \n 
 *          otherwise @c NULL if it is empty
 */
EAPI const char                              *evas_object_textblock_bidi_delimiters_get(const Evas_Object *obj);

/**
 * @brief   Sets the newline mode. 
 * @since   1.1
 *
 * @since_tizen 2.3
 *
 * @remarks When @c true, newline character behaves as a paragraph separator.
 *
 * @param[in]   obj   The given textblock object
 * @param[in]   mode  Set #EINA_TRUE for legacy mode, \n
 *                otherwise set #EINA_FALSE
 */
EAPI void                                     evas_object_textblock_legacy_newline_set(Evas_Object *obj, Eina_Bool mode) EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the newline mode. 
 * @since   1.1
 *
 * @since_tizen 2.3
 *
 * @remarks When true, newline character behaves as a paragraph separator.
 *
 * @param[in]   obj  The given textblock object
 * @return  #EINA_TRUE if it is in legacy mode, \n
 *          otherwise #EINA_FALSE

 */
EAPI Eina_Bool                                evas_object_textblock_legacy_newline_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief   Sets the textblock's text to the markup text.
 *
 * @since_tizen 2.3
 *
 * @remarks This assumes that the text does not include the unicode object replacement char (0xFFFC).
 *
 * @param[in]   obj   The textblock object
 * @param[in]   text  The markup text to use
 */
EAPI void                                     evas_object_textblock_text_markup_set(Evas_Object *obj, const char *text) EINA_ARG_NONNULL(1);

/**
 * @brief   Prepends markup to the cursor @a cur.
 *
 * @since_tizen 2.3
 *
 * @remarks This assumes that the text does not include the unicode object replacement char (0xFFFC).
 *
 * @param[in]   cur   The cursor to prepend to
 * @param[in]   text  The markup text to prepend
 */
EAPI void                                     evas_object_textblock_text_markup_prepend(Evas_Textblock_Cursor *cur, const char *text) EINA_ARG_NONNULL(1, 2);

/**
 * @brief   Gets the markup of the object.
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj  The Evas object
 * @return  The markup text of the object
 */
EAPI const char                              *evas_object_textblock_text_markup_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the object's main cursor.
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj  The object
 * @return  @a obj's main cursor
 */
EAPI Evas_Textblock_Cursor                   *evas_object_textblock_cursor_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief   Creates a new cursor, associates it to the obj and inits it to point
 *          to the start of the textblock. 
 *
 * @since_tizen 2.3
 *
 * @remarks Association to the object means the cursor is updated when the object changes.
 *
 * @remarks If you need speed and you know what you are doing, it is slightly faster to 
 *          just allocate the cursor yourself and not associate it. 
 *
 * @param[in]   obj  The object to associate to
 * @return  The new cursor
 */
EAPI Evas_Textblock_Cursor                   *evas_object_textblock_cursor_new(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * @brief   Frees the cursor and unassociates it from the object.
 *
 * @since_tizen 2.3
 *
 * @remarks Do not use it to free the unassociated cursors.
 *
 * @param[in]   cur  The cursor to free
 */
EAPI void                                     evas_textblock_cursor_free(Evas_Textblock_Cursor *cur) EINA_ARG_NONNULL(1);

/**
 * @brief  Sets the cursor to the start of the first text node.
 *
 * @since_tizen 2.3
 *
 * @param[in]  cur  The cursor to update
 */
EAPI void                                     evas_textblock_cursor_paragraph_first(Evas_Textblock_Cursor *cur) EINA_ARG_NONNULL(1);

/**
 * @brief  Sets the cursor to the end of the last text node.
 *
 * @since_tizen 2.3
 *
 * @param[in]  cur  The cursor to set
 */
EAPI void                                     evas_textblock_cursor_paragraph_last(Evas_Textblock_Cursor *cur) EINA_ARG_NONNULL(1);

/**
 * @brief   Advances to the start of the next text node.
 *
 * @since_tizen 2.3
 *
 * @param[in]   cur  The cursor to update
 * @return  #EINA_TRUE if the cursor advances a paragraph, \n
 *          otherwise #EINA_FALSE
 */
EAPI Eina_Bool                                evas_textblock_cursor_paragraph_next(Evas_Textblock_Cursor *cur) EINA_ARG_NONNULL(1);

/**
 * @brief   Goes to the end of the previous text node.
 *
 * @since_tizen 2.3
 *
 * @param[in]   cur  The cursor to update
 * @return  #EINA_TRUE if the cursor goes to the end of the previous paragraph, \n
 *          otherwise #EINA_FALSE
 */
EAPI Eina_Bool                                evas_textblock_cursor_paragraph_prev(Evas_Textblock_Cursor *cur) EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the the list format node corresponding to @a anchor.
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj     The evas object \n
 *                  This must not be @c NULL.
 * @param[in]   anchor  The anchor name to get
 * @return  The list format node corresponding to the anchor, \n
 *          otherwise @c NULL if there is no list format node
 */
EAPI const Eina_List                         *evas_textblock_node_format_list_get(const Evas_Object *obj, const char *anchor) EINA_ARG_NONNULL(1, 2);

/**
 * @brief   Gets the first format node.
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj  The evas object \n
 *               This must not be @c NULL.
 * @return  The first format node, \n
 *          otherwise @c NULL if there is no first format node
 */
EAPI const Evas_Object_Textblock_Node_Format *evas_textblock_node_format_first_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the last format node.
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj  The evas textblock \n 
 *               This must not be @c NULL.
 * @return  The last format node, \n
 *          otherwise @c NULL if there is no last format node
 */
EAPI const Evas_Object_Textblock_Node_Format *evas_textblock_node_format_last_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the next format node (after n).
 *
 * @since_tizen 2.3
 *
 * @param[in]   n  The current format node \n
 *             This must not be @c NULL.
 * @return  The next format node, \n
 *          otherwise @c NULL if there is no next format node
 */
EAPI const Evas_Object_Textblock_Node_Format *evas_textblock_node_format_next_get(const Evas_Object_Textblock_Node_Format *n) EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the previous format node (after n).
 *
 * @since_tizen 2.3
 *
 * @param[in]   n  The current format node \n 
 *             This is must not be @c NULL.
 * @return  The previous format node, \n
 *          otherwise @c NULL if there is no previous format node
 */
EAPI const Evas_Object_Textblock_Node_Format *evas_textblock_node_format_prev_get(const Evas_Object_Textblock_Node_Format *n) EINA_ARG_NONNULL(1);

/**
 * @brief   Removes a format node and its match. \n
 *          That is it removes a \<tag\> \</tag\> pair.
 *
 * @since_tizen 2.3
 *
 * @remarks Assumes that the node is the first part of \<tag\>. 
 *          This does not work if @a n is a closing tag.
 *
 * @param[in]   obj  The Evas object of the textblock \n 
 *               This must not be @c NULL.
 * @param[in]   n    The current format node \n
 *               This must not be @c NULL.
 */
EAPI void                                     evas_textblock_node_format_remove_pair(Evas_Object *obj, Evas_Object_Textblock_Node_Format *n) EINA_ARG_NONNULL(1, 2);

/**
 * @brief   Sets the cursor to point to the place where format points to.
 *
 * @since_tizen 2.3
 *
 * @param[in]   cur  The cursor to update
 * @param[in]   n    The format node to update according
 * @deprecated  Duplicate of evas_textblock_cursor_at_format_set
 */
EAPI void                                     evas_textblock_cursor_set_at_format(Evas_Textblock_Cursor *cur, const Evas_Object_Textblock_Node_Format *n) EINA_ARG_NONNULL(1, 2);

/**
 * @brief   Gets the format node at the position pointed by @a cur.
 *
 * @since_tizen 2.3
 *
 * @param[in]   cur  The position to look at
 * @return  The format node, \n
 *          otherwise @c NULL if it is not found
 * @see evas_textblock_cursor_format_is_visible_get()
 */
EAPI const Evas_Object_Textblock_Node_Format *evas_textblock_cursor_format_get(const Evas_Textblock_Cursor *cur) EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the text format representation of the format node.
 *
 * @since_tizen 2.3
 *
 * @param[in]   fnode  The format node
 * @return  The textual format of the format node
 */
EAPI const char                              *evas_textblock_node_format_text_get(const Evas_Object_Textblock_Node_Format *fnode) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief   Sets the cursor to point to the position of @a fmt.
 *
 * @since_tizen 2.3
 *
 * @param[in]   cur  The cursor to update
 * @param[in]   fmt  The format to update according to
 */
EAPI void                                     evas_textblock_cursor_at_format_set(Evas_Textblock_Cursor *cur, const Evas_Object_Textblock_Node_Format *fmt) EINA_ARG_NONNULL(1, 2);

/**
 * @brief   Checks whether the current cursor position is a visible format. 
 *
 * @since_tizen 2.3
 *
 * @remarks This way is more efficient than evas_textblock_cursor_format_get() to check for the existence
 *          of a visible format.
 *
 * @param[in]   cur  The cursor to look at
 * @return  #EINA_TRUE if the cursor points to a visible format, \n
 *          otherwise #EINA_FALSE if the cursor does not point to a visible format
 * @see evas_textblock_cursor_format_get()
 */
EAPI Eina_Bool                                evas_textblock_cursor_format_is_visible_get(const Evas_Textblock_Cursor *cur) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief   Advances the cursor to the next format node.
 *
 * @since_tizen 2.3
 *
 * @param[in]   cur  The cursor to be updated
 * @return  #EINA_TRUE if the cursor is advanced successfully, \n 
 *          otherwise #EINA_FALSE on failure
 */
EAPI Eina_Bool                                evas_textblock_cursor_format_next(Evas_Textblock_Cursor *cur) EINA_ARG_NONNULL(1);

/**
 * @brief   Advances the cursor to the previous format node.
 *
 * @since_tizen 2.3
 *
 * @param[in]   cur  The cursor to update
 * @return  #EINA_TRUE if the cursor is advanced successfully, \n 
 *          otherwise #EINA_FALSE on failure
 */
EAPI Eina_Bool                                evas_textblock_cursor_format_prev(Evas_Textblock_Cursor *cur) EINA_ARG_NONNULL(1);

/**
 * @brief   Checks whether the cursor points to a format.
 *
 * @since_tizen 2.3
 *
 * @param[in]   cur  The cursor to check
 * @return  #EINA_TRUE if the cursor points to a format, \n 
 *          otherwise #EINA_FALSE if the cursor does not point to a format
 */
EAPI Eina_Bool                                evas_textblock_cursor_is_format(const Evas_Textblock_Cursor *cur) EINA_ARG_NONNULL(1);

/**
 * @internal
 * @brief   Advances the cursor 1 cluster forward.
 *
 * @since_tizen 2.3
 *
 * @param[in]   cur  The cursor to advance
 * @return  #EINA_TRUE if the cursor is advanced successfully, \n
 *          otherwise #EINA_FALSE on failure
 */
EAPI Eina_Bool                                evas_textblock_cursor_cluster_next(Evas_Textblock_Cursor *cur) EINA_ARG_NONNULL(1);

/**
 * @internal
 * @brief   Advances the cursor 1 cluster backward.
 *
 * @since_tizen 2.3
 *
 * @param[in]   cur  The cursor to advance
 * @return  #EINA_TRUE if the cursor is advanced successfully, \n
 *          otherwise #EINA_FALSE on failure
 */
EAPI Eina_Bool                                evas_textblock_cursor_cluster_prev(Evas_Textblock_Cursor *cur) EINA_ARG_NONNULL(1);

/**
 * @brief   Advances the cursor 1 char forward.
 *
 * @since_tizen 2.3
 *
 * @param[in]   cur  The cursor to advance
 * @return  #EINA_TRUE if the cursor is advanced successfully, \n
 *          otherwise #EINA_FALSE on failure
 */
EAPI Eina_Bool                                evas_textblock_cursor_char_next(Evas_Textblock_Cursor *cur) EINA_ARG_NONNULL(1);

/**
 * @brief   Advances the cursor 1 char backward.
 *
 * @since_tizen 2.3
 *
 * @param[in]   cur  The cursor to advance
 * @return  #EINA_TRUE if the cursor is advanced successfully, \n
 *          otherwise #EINA_FALSE on failure
 */
EAPI Eina_Bool                                evas_textblock_cursor_char_prev(Evas_Textblock_Cursor *cur) EINA_ARG_NONNULL(1);

/**
 * @brief   Moves the cursor to the start of the word under the cursor.
 * @since   1.2
 *
 * @since_tizen 2.3
 *
 * @param[in]   cur  The cursor to move
 * @return  #EINA_TRUE if the cursor is moved successfully, \n
 *          otherwise #EINA_FALSE on failure
 */
EAPI Eina_Bool                                evas_textblock_cursor_word_start(Evas_Textblock_Cursor *cur) EINA_ARG_NONNULL(1);

/**
 * @brief   Moves the cursor to the end of the word under the cursor.
 * @since   1.2
 *
 * @since_tizen 2.3
 *
 * @param[in]   cur  The cursor to move
 * @return  #EINA_TRUE if the cursor is moved successfully, \n
 *          otherwise #EINA_FALSE on failure
 */
EAPI Eina_Bool                                evas_textblock_cursor_word_end(Evas_Textblock_Cursor *cur) EINA_ARG_NONNULL(1);

/**
 * @brief  Moves the cursor to the first char in the node the cursor is pointing on.
 *
 * @since_tizen 2.3
 *
 * @param[in]  cur  The cursor to move
 */
EAPI void                                     evas_textblock_cursor_paragraph_char_first(Evas_Textblock_Cursor *cur) EINA_ARG_NONNULL(1);

/**
 * @brief  Moves the cursor to the last char in a text node.
 *
 * @since_tizen 2.3
 *
 * @param[in]  cur  The cursor to move
 */
EAPI void                                     evas_textblock_cursor_paragraph_char_last(Evas_Textblock_Cursor *cur) EINA_ARG_NONNULL(1);

/**
 * @brief  Moves the cursor to the start of the current line.
 *
 * @since_tizen 2.3
 *
 * @param[in]  cur  The cursor to move
 */
EAPI void                                     evas_textblock_cursor_line_char_first(Evas_Textblock_Cursor *cur) EINA_ARG_NONNULL(1);

/**
 * @brief  Moves the cursor to the end of the current line.
 *
 * @since_tizen 2.3
 *
 * @param[in]  cur  The cursor to move
 */
EAPI void                                     evas_textblock_cursor_line_char_last(Evas_Textblock_Cursor *cur) EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the current cursor position.
 *
 * @since_tizen 2.3
 *
 * @param[in]   cur  The cursor
 * @return  The cursor position, \n 
 *          otherwise @c -1 on error
 */
EAPI int                                      evas_textblock_cursor_pos_get(const Evas_Textblock_Cursor *cur) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief  Sets the cursor position.
 *
 * @since_tizen 2.3
 *
 * @param[in]  cur  The cursor to be set
 * @param[in]  pos  The position to set
 */
EAPI void                                     evas_textblock_cursor_pos_set(Evas_Textblock_Cursor *cur, int pos) EINA_ARG_NONNULL(1);

/**
 * @brief   Moves the cursor to the start of the line passed.
 *
 * @since_tizen 2.3
 *
 * @param[in]   cur   The cursor
 * @param[in]   line  The line number
 * @return  #EINA_TRUE if the cursor is moved successfully, \n
 *          otherwise #EINA_FALSE on error
 */
EAPI Eina_Bool                                evas_textblock_cursor_line_set(Evas_Textblock_Cursor *cur, int line) EINA_ARG_NONNULL(1);

/**
 * @brief   Compares two cursors.
 *
 * @since_tizen 2.3
 *
 * @param[in]   cur1  The first cursor
 * @param[in]   cur2  The second cursor
 * @return  @c -1 if cur1 < cur2, @c 0 if cur1 == cur2, \n 
 *          otherwise @c 1
 */
EAPI int                                      evas_textblock_cursor_compare(const Evas_Textblock_Cursor *cur1, const Evas_Textblock_Cursor *cur2) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1, 2);

/**
 * @brief   Makes @a cur_dest point to the same point as @a cur. 
 *
 * @since_tizen 2.3
 *
 * @remarks This does not work if they do not point to the same object.
 *
 * @param[in]   cur       The source cursor
 * @param[in]   cur_dest  The destination cursor
 */
EAPI void                                     evas_textblock_cursor_copy(const Evas_Textblock_Cursor *cur, Evas_Textblock_Cursor *cur_dest) EINA_ARG_NONNULL(1, 2);

/**
 * @brief   Adds text to the current cursor position and sets the cursor to *before*
 *          the start of the text just added.
 *
 * @since_tizen 2.3
 *
 * @param[in]   cur   The cursor to the position to add text at
 * @param[in]   text  The text to add
 * @return  The length of the text added
 * @see evas_textblock_cursor_text_prepend()
 */
EAPI int                                      evas_textblock_cursor_text_append(Evas_Textblock_Cursor *cur, const char *text) EINA_ARG_NONNULL(1, 2);

/**
 * @brief   Adds text to the current cursor position and sets the cursor to *after*
 *          the start of the text just added.
 *
 * @since_tizen 2.3
 *
 * @param[in]   cur   The cursor to the position to add text at
 * @param[in]   text  The text to add
 * @return  The length of the text added
 * @see evas_textblock_cursor_text_append()
 */
EAPI int                                      evas_textblock_cursor_text_prepend(Evas_Textblock_Cursor *cur, const char *text) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Adds format to the current cursor position.
 * @details If the format being added is a visible format,
            add it *before* the cursor position, otherwise, add it after.
 *          This behavior is because visible formats are like characters and invisible
 *          should be stacked in a way that the last one is added last.
 *
 * @since_tizen 2.3
 *
 * @remarks This function works with native formats, that means that style defined
 *          tags like <br> won't work here. For those kind of things use markup prepend.
 *
 * @param[in] cur the cursor to where to add format at.
 * @param[in] format the format to add.
 * @return Returns true if a visible format was added, false otherwise.
 * @see evas_textblock_cursor_format_prepend()
 */
EAPI Eina_Bool                                evas_textblock_cursor_format_append(Evas_Textblock_Cursor *cur, const char *format) EINA_ARG_NONNULL(1, 2);

/**
 * @brief   Adds the format to the current cursor position. 
 * @details If the format being added is a visible format, 
 *          add it *before* the cursor position, otherwise, add it after.
 *          This behavior is because visible formats are like characters and invisible
 *          should be stacked in a way that the last one is added last.
 *          If the format is visible the cursor is advanced after it.
 *
 * @since_tizen 2.3
 *
 * @remarks This function works with native formats, that means that style defined
 *          tags like <br> does not work here. For those, use markup prepend.
 *
 * @param[in]   cur     The cursor to where to add format at
 * @param[in]   format  The format to add
 * @return  #EINA_TRUE if the visible format is added, \n
 *          otherwise #EINA_FALSE if the visible format is not added
 * @see evas_textblock_cursor_format_prepend()
 */
EAPI Eina_Bool                                evas_textblock_cursor_format_prepend(Evas_Textblock_Cursor *cur, const char *format) EINA_ARG_NONNULL(1, 2);

/**
 * @brief   Deletes the character at the location of the cursor. 
 * @details If there is a format pointing to this position, delete it as well.
 *
 * @since_tizen 2.3
 *
 * @param[in]   cur  The cursor pointing to the current location
 */
EAPI void                                     evas_textblock_cursor_char_delete(Evas_Textblock_Cursor *cur) EINA_ARG_NONNULL(1);

/**
 * @brief   Deletes the range between @a cur1 and @a cur2.
 *
 * @since_tizen 2.3
 *
 * @param[in]   cur1  Starting point of the range
 * @param[in]   cur2  Ending point of the range
 */
EAPI void                                     evas_textblock_cursor_range_delete(Evas_Textblock_Cursor *cur1, Evas_Textblock_Cursor *cur2) EINA_ARG_NONNULL(1, 2);

/**
 * @brief   Gets the text of the paragraph @a cur points to.
 *
 * @since_tizen 2.3
 *
 * @param[in]   cur  The cursor pointing to the paragraph
 * @return  The text in markup, \n
 *          otherwise @c NULL on error
 */
EAPI const char                              *evas_textblock_cursor_paragraph_text_get(const Evas_Textblock_Cursor *cur) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the length of the paragraph. 
 *
 * @since_tizen 2.3
 *
 * @remarks This is cheaper that the eina_unicode_strlen().
 *
 * @param[in]   cur  The position of the paragraph
 * @return  The length of the paragraph on success, \n
 *          otherwise @c -1 on error
 */
EAPI int                                      evas_textblock_cursor_paragraph_text_length_get(const Evas_Textblock_Cursor *cur) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the currently visible range.
 * @since   1.1
 *
 * @since_tizen 2.3
 *
 * @param[in]   start  The start of the range
 * @param[in]   end    The end of the range
 * @return  #EINA_TRUE if the range is obtained successfully, \n
 *          otherwise #EINA_FALSE on failure
 */
EAPI Eina_Bool                                evas_textblock_cursor_visible_range_get(Evas_Textblock_Cursor *start, Evas_Textblock_Cursor *end) EINA_ARG_NONNULL(1, 2);

/**
 * @brief   Gets the format nodes in the range between @a cur1 and @a cur2.
 * @since   1.1
 *
 * @since_tizen 2.3
 *
 * @param[in]   cur1  The start of the range
 * @param[in]   cur2  The end of the range
 * @return  The format nodes in the range \n 
 *          You have to free it.
 */
EAPI Eina_List                               *evas_textblock_cursor_range_formats_get(const Evas_Textblock_Cursor *cur1, const Evas_Textblock_Cursor *cur2) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1, 2);

/**
 * @brief   Gets the text in the range between @a cur1 and @a cur2.
 *
 * @since_tizen 2.3
 *
 * @param[in]   cur1    The start of the range
 * @param[in]   cur2    The end of the range
 * @param[in]   format  The format in which to return the text \n 
 *                  Markup - in textblock markup. Plain - UTF8.
 * @return  The text in the range
 * @see elm_entry_markup_to_utf8()
 */
EAPI char                                    *evas_textblock_cursor_range_text_get(const Evas_Textblock_Cursor *cur1, const Evas_Textblock_Cursor *cur2, Evas_Textblock_Text_Type format) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1, 2);

/**
 * @internal
 * // TIZEN_ONLY(20131218)
 * // Add evas_textblock_cursor_range_text_valid_markup_get API.
 * @brief   Gets the text and markup tags in the range between @a cur1 and @a cur2.
 *
 * @param   cur1  The start of the range
 * @param   cur2  The end of the range
 * @return  The text in the range and the markup tags that affect the text
 * @see elm_entry_markup_to_utf8()
 */
EAPI char                                    *evas_textblock_cursor_range_text_valid_markup_get(const Evas_Textblock_Cursor *cur1, const Evas_Textblock_Cursor *cur2) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1, 2);
/****************************************************/
/************* TIZEN ONLY END   *********************/
/****************************************************/

/**
 * @brief   Gets the content of the cursor.
 *
 * @since_tizen 2.3
 *
 * @remarks Frees the returned string pointer when done if it is not @c NULL.
 *
 * @param[in]   cur  The cursor
 * @return  The text in the range, terminated by a null byte (may be utf8)
 */
EAPI char                                    *evas_textblock_cursor_content_get(const Evas_Textblock_Cursor *cur) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * @brief   Gets the geometry of two cursors ("split cursor"), if logical cursor is
 *          between LTR/RTL text, also considering paragraph direction.
 *
 * @since_tizen 2.3
 *
 * @remarks The upper cursor is shown for the text of the same direction as paragraph,
 *          lower cursor - for the opposite.
 *
 * @remarks The split cursor geometry is valid only  in '|' cursor mode.
 *          In this case #EINA_TRUE is returned and @a cx2, @a cy2, @a cw2, @a ch2 are set,
 *          otherwise it behaves like cursor_geometry_get.
 *
 * @param[in]   cur    The cursor
 * @param[out]  cx     The x coordinate of the cursor (or upper cursor)
 * @param[out]  cy     The y coordinate of the cursor (or upper cursor)
 * @param[out]  cw     The width of the cursor (or upper cursor)
 * @param[out]  ch     The height of the cursor (or upper cursor)
 * @param[out]  cx2    The x coordinate of the lower cursor
 * @param[out]  cy2    The y coordinate of the lower cursor
 * @param[out]  cw2    The width of the lower cursor
 * @param[out]  ch2    The height of the lower cursor
 * @param[in]   ctype  The type of the cursor
 * @return      #EINA_TRUE if split cursor,
 *              otherwise #EINA_FALSE
 */
EAPI Eina_Bool evas_textblock_cursor_geometry_bidi_get(const Evas_Textblock_Cursor *cur, Evas_Coord *cx, Evas_Coord *cy, Evas_Coord *cw, Evas_Coord *ch, Evas_Coord *cx2, Evas_Coord *cy2, Evas_Coord *cw2, Evas_Coord *ch2, Evas_Textblock_Cursor_Type ctype);

/**
 * @brief   Gets the geometry of the cursor. 
 *
 * @since_tizen 2.3
 *
 * @remarks It depends on the type of cursor requested.
 *          This should be used instead of char_geometry_get because there are weird
 *          special cases with BiDi text.
 *
 * @remarks In '_' cursor mode (i.e a line below the char) it is the same as char_geometry_get,
 *          except for the case of the last char of a line which depends on the
 *          paragraph direction.
 *
 * @remarks In '|' cursor mode (i.e a line between two chars) it is very variable.
 *          For example, consider the following visual string:
 *          "abcCBA" (ABC are rtl chars), a cursor pointing on A should actually draw
 *          a '|' between the c and the C.
 *
 * @param[in]   cur    The cursor
 * @param[out]   cx     The x coordinate of the cursor
 * @param[out]   cy     The y coordinate of the cursor
 * @param[out]   cw     The width of the cursor
 * @param[out]   ch     The height of the cursor
 * @param[in]   dir    The direction of the cursor \n 
 *                 This can be @c NULL.
 * @param[in]   ctype  The type of the cursor
 * @return  The line number of the char on success, \n
 *          otherwise @c -1 on error
 */
EAPI int                                      evas_textblock_cursor_geometry_get(const Evas_Textblock_Cursor *cur, Evas_Coord *cx, Evas_Coord *cy, Evas_Coord *cw, Evas_Coord *ch, Evas_BiDi_Direction *dir, Evas_Textblock_Cursor_Type ctype) EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the geometry of the char at @a cur.
 *
 * @since_tizen 2.3
 *
 * @param[in]   cur  The position of the char
 * @param[out]   cx   The x coordinate of the char
 * @param[out]   cy   The y coordinate of the char
 * @param[out]   cw   The width of the char
 * @param[out]   ch   The height of the char
 * @return  The line number of the char, \n
 *          otherwise @c -1 on error
 */
EAPI int                                      evas_textblock_cursor_char_geometry_get(const Evas_Textblock_Cursor *cur, Evas_Coord *cx, Evas_Coord *cy, Evas_Coord *cw, Evas_Coord *ch) EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the geometry of the pen at @a cur.
 *
 * @since_tizen 2.3
 *
 * @param[in]   cur     The position of the char
 * @param[out]   cpen_x  The pen_x of the char
 * @param[out]   cy      The y of the char
 * @param[out]   cadv    The adv of the char
 * @param[out]   ch      The h of the char
 * @return  The line number of the char, \n
 *          otherwise @c -1 on error
 */
EAPI int                                      evas_textblock_cursor_pen_geometry_get(const Evas_Textblock_Cursor *cur, Evas_Coord *cpen_x, Evas_Coord *cy, Evas_Coord *cadv, Evas_Coord *ch) EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the geometry of the line at @a cur.
 *
 * @since_tizen 2.3
 *
 * @param[in]   cur  The position of the line
 * @param[out]   cx   The x coordinate of the line
 * @param[out]   cy   The y coordinate of the line
 * @param[out]   cw   The width of the line
 * @param[out]   ch   The height of the line
 * @return  The line number of the line, \n
 *          otherwise @c -1 on error
 */
EAPI int                                      evas_textblock_cursor_line_geometry_get(const Evas_Textblock_Cursor *cur, Evas_Coord *cx, Evas_Coord *cy, Evas_Coord *cw, Evas_Coord *ch) EINA_ARG_NONNULL(1);

/**
 * @internal
 * @brief   Sets the position of the cursor at a proper cluster according to the X and Y coordinates.
 *
 * @since_tizen 2.3
 *
 * @param[in]   cur  The cursor to set
 * @param[in]   x    The x coordinate to set
 * @param[in]   y    The y coordinate to set
 * @return  #EINA_TRUE if the position of the cursor is set successfully, \n
 *          otherwise #EINA_FALSE on failure
 */
EAPI Eina_Bool                                evas_textblock_cursor_cluster_coord_set(Evas_Textblock_Cursor *cur, Evas_Coord x, Evas_Coord y) EINA_ARG_NONNULL(1);

/**
 * @brief   Sets the position of the cursor according to the X and Y coordinates.
 *
 * @since_tizen 2.3
 *
 * @param[in]   cur  The cursor to set
 * @param[in]   x    The x coordinate to set
 * @param[in]   y    The y coordinate to set
 * @return  #EINA_TRUE if the position of the cursor is set successfully, \n 
 *          otherwise #EINA_FALSE on failure
 */
EAPI Eina_Bool                                evas_textblock_cursor_char_coord_set(Evas_Textblock_Cursor *cur, Evas_Coord x, Evas_Coord y) EINA_ARG_NONNULL(1);

/**
 * @brief   Sets the cursor position according to the y coordinate.
 *
 * @since_tizen 2.3
 *
 * @param[in]   cur  The cur to be set
 * @param[in]   y    The y coordinate to set
 * @return  The line number found, \n
 *          otherwise @c -1 on error
 */
EAPI int                                      evas_textblock_cursor_line_coord_set(Evas_Textblock_Cursor *cur, Evas_Coord y) EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the geometry of a range.
 *
 * @since_tizen 2.3
 *
 * @param[in]   cur1  The start of the range
 * @param[in]   cur2  The end of the range
 * @return  A list of rectangles representing the geometry of the range
 */
EAPI Eina_List                               *evas_textblock_cursor_range_geometry_get(const Evas_Textblock_Cursor *cur1, const Evas_Textblock_Cursor *cur2) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1, 2);

/**
 * @brief   Get the simple geometry of a range.
 * The simple geometry is the geomtry in which rectangles in middle
 * lines of range are merged into one big rectangle.
 *
 * @since_tizen 2.3
 *
 * @param cur1 one side of the range.
 * @param cur2 other side of the range.
 * @return an iterator of rectangles representing the geometry of the range.
 */
EAPI Eina_Iterator                               *evas_textblock_cursor_range_simple_geometry_get(const Evas_Textblock_Cursor *cur1, const Evas_Textblock_Cursor *cur2) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1, 2);


EAPI Eina_Bool                                evas_textblock_cursor_format_item_geometry_get(const Evas_Textblock_Cursor *cur, Evas_Coord *cx, Evas_Coord *cy, Evas_Coord *cw, Evas_Coord *ch) EINA_ARG_NONNULL(1);

/**
 * @brief   Checks whether the cursor points to the end of the line.
 *
 * @since_tizen 2.3
 *
 * @param[in]   cur  The cursor to check
 * @return  #EINA_TRUE if the cursor points to the end of the line, \n
 *          otherwise #EINA_FALSE
 */
EAPI Eina_Bool                                evas_textblock_cursor_eol_get(const Evas_Textblock_Cursor *cur) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the geometry of a line number.
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj   The object
 * @param[in]   line  The line number
 * @param[out]   cx    The x coordinate of the line
 * @param[out]   cy    The y coordinate of the line
 * @param[out]   cw    The w coordinate of the line
 * @param[out]   ch    The h coordinate of the line
 * @return  #EINA_TRUE if the geometry of the line is obtained successfully, 
 *          otherwise #EINA_FALSE on failure
 */
EAPI Eina_Bool                                evas_object_textblock_line_number_geometry_get(const Evas_Object *obj, int line, Evas_Coord *cx, Evas_Coord *cy, Evas_Coord *cw, Evas_Coord *ch) EINA_ARG_NONNULL(1);

/**
 * @brief   Clears the textblock object.
 *
 * @since_tizen 2.3
 *
 * @remarks Does *NOT* free the Evas object itself.
 *
 * @param[in]   obj  The object to clear
 */
EAPI void                                     evas_object_textblock_clear(Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the formatted width and height. 
 *
 * @since_tizen 2.3
 *
 * @remarks This calculates the actual size after restricting
 *          the textblock to the current size of the object.
 *          The main difference between this and @ref evas_object_textblock_size_native_get
 *          is that the "native" function does not take wrapping into account.
 *          It just calculates the real width of the object if it is placed on an
 *          infinite canvas, while this function gives the size after wrapping
 *          according to the size restrictions of the object.
 *
 * @remarks For example, a textblock containing the text: "You shall not pass!"
 *          with no margins or padding and assuming a monospace font and a size of
 *          7x10 char widths (for simplicity) has a native size of 19x1
 *          and a formatted size of 5x4.
 *
 *
 * @param[in]   obj  The Evas object
 * @param[out]   w    The width of the object
 * @param[out]   h    The height of the object
 * @see evas_object_textblock_size_native_get
 */
EAPI void                                     evas_object_textblock_size_formatted_get(const Evas_Object *obj, Evas_Coord *w, Evas_Coord *h) EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the native width and height. 
 *
 * @since_tizen 2.3
 *
 * @remarks This calculates the actual size without taking into account
 *          the current size of the object.
 *          The main difference between this and @ref evas_object_textblock_size_formatted_get
 *          is that the "native" function does not take wrapping into account.
 *          It just calculates the real width of the object if it is placed on an
 *          infinite canvas, while the "formatted" function gives the size after
 *          wrapping text according to the size restrictions of the object.
 *
 * @remarks For example, a textblock containing the text: "You shall not pass!"
 *          with no margins or padding and assuming a monospace font and a size of
 *          7x10 char widths (for simplicity) has a native size of 19x1
 *          and a formatted size of 5x4.
 *
 * @param[in]   obj  The Evas object of the textblock
 * @param[out]   w    The width returned
 * @param[out]   h    The height returned
 */
EAPI void                                     evas_object_textblock_size_native_get(const Evas_Object *obj, Evas_Coord *w, Evas_Coord *h) EINA_ARG_NONNULL(1);

/**
 * @internal
 */
EAPI void                                     evas_object_textblock_style_insets_get(const Evas_Object *obj, Evas_Coord *l, Evas_Coord *r, Evas_Coord *t, Evas_Coord *b) EINA_ARG_NONNULL(1);

/**
 * @}
 */


/**
 * @internal
 * @defgroup Evas_Object_Textgrid Textgrid Object Functions
 * @ingroup Evas_Object_Specific
 *
 * @todo put here some usage examples
 *
 * @since 1.7
 *
 * @{
 */

/**
 * @typedef Evas_Textgrid_Palette
 *
 * @brief Enumeration for the palette to use for the foreground and background colors.
 *
 * @since 1.7
 */
typedef enum
{
   EVAS_TEXTGRID_PALETTE_NONE,     /**< No palette is used */
   EVAS_TEXTGRID_PALETTE_STANDARD, /**< Standard palette (around 16 colors) */
   EVAS_TEXTGRID_PALETTE_EXTENDED, /**< Extended palette (at max 256 colors) */
   EVAS_TEXTGRID_PALETTE_LAST      /**< Ignore this */
} Evas_Textgrid_Palette;

/**
 * @typedef Evas_Textgrid_Font_Style
 *
 * @brief Enumeration for the style to give to each character of the grid.
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
 * @brief The structure type containing the values that describe each cell.
 *
 * @since 1.7
 */
typedef struct _Evas_Textgrid_Cell Evas_Textgrid_Cell;

/**
 * @internal
 * @struct _Evas_Textgrid_Cell
 *
 *  @brief The structure type containing the values that describe each cell.
 *
 * @since 1.7
 */
struct _Evas_Textgrid_Cell
{
   Eina_Unicode   codepoint;         /**< The UNICODE value of the character */
   unsigned char  fg;                /**< The index of the palette for the foreground color */
   unsigned char  bg;                /**< The index of the palette for the background color */
   unsigned short bold          : 1; /**< The character is bold */
   unsigned short italic        : 1; /**< The character is oblique */
   unsigned short underline     : 1; /**< The character is underlined */
   unsigned short strikethrough : 1; /**< The character is struck through */
   unsigned short fg_extended   : 1; /**< The extended palette is used for the foreground color */
   unsigned short bg_extended   : 1; /**< The extended palette is used for the background color */
   unsigned short double_width  : 1; /**< The codepoint is merged with the following cell to the right visually (cells must be in pairs with 2nd cell being a duplicate in all ways except codepoint is 0) */
};

/**
 * @brief   Adds a textgrid to the given Evas.
 *
 * @details This function adds a new textgrid object to the Evas @a e and returns the object.
 * @since   1.7
 *
 * @since_tizen 2.3
 *
 * @param[in]   e  The given evas
 * @return  The new textgrid object
 *
 */
EAPI Evas_Object *evas_object_textgrid_add(Evas *e);

/**
 * @brief   Sets the size of the textgrid object.
 *
 * @details This function sets the number of lines @a h and the number
 *          of columns @a w to the textgrid object @a obj. If
 *          @a w or @a h are less or equal than @c 0, this
 *          function does nothing.
 *
 * @since   1.7
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj  The textgrid object
 * @param[in]   w    The number of columns (width in cells) of the grid
 * @param[in]   h    The number of rows (height in cells) of the grid
 */
EAPI void evas_object_textgrid_size_set(Evas_Object *obj, int w, int h);

/**
 * @brief   Gets the size of the textgrid object.
 *
 * @details This function retrieves the number of lines in the buffer @a
 *          h and the number of columns in the buffer @a w of
 *          the textgrid object @a obj. @a w or @a h can be
 *          @c NULL. On error, their value is @c 0.
 *
 * @since   1.7
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj  The textgrid object
 * @param[out]   w    The number of columns of the grid
 * @param[out]   h    The number of rows of the grid
 */
EAPI void evas_object_textgrid_size_get(const Evas_Object *obj, int *w, int *h);

/**
 * @brief   Sets the font (source) file to be used on a given textgrid object.
 *
 * @details This function allows the font file @a font_source to be explicitly
 *          set for the textgrid object @a obj, overriding system lookup, which
 *          first occurs in the given file's contents. If @a font_source is
 *          @c NULL or is an empty string, or the same @a font_source has already
 *          been set, or on error, this function does nothing.
 *
 * @since   1.7
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj          The textgrid object to set font for
 * @param[in]   font_source  The font file's path
 *
 * @see evas_object_textgrid_font_get()
 * @see evas_object_textgrid_font_set()
 * @see evas_object_textgrid_font_source_get()
 */
EAPI void evas_object_textgrid_font_source_set(Evas_Object *obj, const char *font_source);

/**
 * @brief   Gets the font file's path which is being used on a given textgrid object.
 *
 * @details This function returns the font source path of the textgrid object
 *          @a obj. If the font source path has not been set, or on error,
 *          @c NULL is returned.
 *
 * @since   1.7
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj  The textgrid object to set font for
 * @return  The font file's path
 *
 * @see evas_object_textgrid_font_get()
 * @see evas_object_textgrid_font_set()
 * @see evas_object_textgrid_font_source_set()
 */
EAPI const char *evas_object_textgrid_font_source_get(const Evas_Object *obj);

/**
 * @brief   Sets the font family and size on a given textgrid object.
 *
 * @details This function allows the font name @a font_name and size
 *          @a font_size of the textgrid object @a obj to be set. The @a font_name
 *          string has to follow fontconfig's convention on naming fonts, as
 *          it is the underlying library used to query system fonts by Evas (see
 *          the @c fc-list command's output, on your system, to get an
 *          idea). It also has to be a monospace font. If @a font_name is
 *          @c NULL, or if it is an empty string, or if @a font_size is less or
 *          equal than @c 0, or on error, this function does nothing.
 *
 * @since   1.7
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj        The textgrid object to set font for
 * @param[in]   font_name  The font (family) name
 * @param[in]   font_size  The font size, in points
 *
 * @see evas_object_textgrid_font_get()
 * @see evas_object_textgrid_font_source_set()
 * @see evas_object_textgrid_font_source_get()
 */
EAPI void evas_object_textgrid_font_set(Evas_Object *obj, const char *font_name, Evas_Font_Size font_size);

/**
 * @brief   Gets the font family and size in use on a given textgrid object.
 *
 * @details This function allows the font name and size of a textgrid object
 *          @a obj to be queried and stored respectively in the buffers
 *          @a font_name and @a font_size. Be aware that the font name string is
 *          still owned by Evas and should @b not have free() called on it by
 *          the caller of the function. On error, the font name is the empty
 *          string and the font size is @c 0. @a font_name and @a font_source can
 *          be @c NULL.
 *
 * @since   1.7
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj        The textgrid object to query for font information
 * @param[out]   font_name  A pointer to the location to store the font name in
 * @param[out]   font_size  A pointer to the location to store the font size in
 *
 * @see evas_object_textgrid_font_set()
 * @see evas_object_textgrid_font_source_set()
 * @see evas_object_textgrid_font_source_get()
 */
EAPI void evas_object_textgrid_font_get(const Evas_Object *obj, const char **font_name, Evas_Font_Size *font_size);

/**
 * @brief   Gets the size of a cell of the given textgrid object in pixels.
 *
 * @details This functions retrieves the width and height, in pixels, of a cell
 *          of the textgrid object @a obj and store them respectively in the
 *          buffers @a width and @a height. Their value depends on the
 *          monospace font used for the textgrid object, as well as the
 *          style. @a width and @a height can be @c NULL. On error, they are
 *          set to @c 0.
 *
 * @since   1.7
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj    The textgrid object to query for font information
 * @param[out]  w      A pointer to the location to store the width in pixels of a cell
 * @param[out]  h      A pointer to the location to store the height in pixels of a cell
 *
 * @see evas_object_textgrid_font_set()
 * @see evas_object_textgrid_supported_font_styles_set()
 */
EAPI void evas_object_textgrid_cell_size_get(const Evas_Object *obj, Evas_Coord *w, Evas_Coord *h);

/**
 * @brief   Sets the color to the given palette at the given index of the given textgrid object.
 *
 * @details This function sets the color for the palette of type @a pal at the
 *          index @a idx of the textgrid object @a obj. The ARGB components are
 *          given by @a r, @a g, @a b and @a a. This color can be used when
 *          setting the #Evas_Textgrid_Cell structure. The components must set
 *          a pre-multiplied color. If pal is #EVAS_TEXTGRID_PALETTE_NONE or
 *          #EVAS_TEXTGRID_PALETTE_LAST, or if @a idx is not between @c 0 and @c 255,
 *          or on error, this function does nothing. The color components are
 *          clamped between @c 0 and @c 255. If @a idx is greater than the latest set
 *          color, the colors between this last index and @a idx - 1 are set to
 *          black (0, 0, 0, 0).
 *
 * @since   1.7
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj  The textgrid object to query for font information
 * @param[in]   pal  The type of the palette to set the color
 * @param[in]   idx  The index of the paletter to which the color is stored
 * @param[in]   r    The red component of the color
 * @param[in]   g    The green component of the color
 * @param[in]   b    The blue component of the color
 * @param[in]   a    The alpha component of the color
 *
 * @see evas_object_textgrid_palette_get()
 */
EAPI void evas_object_textgrid_palette_set(Evas_Object *obj, Evas_Textgrid_Palette pal, int idx, int r, int g, int b, int a);

/**
 * @brief   Gets the color to the given palette at the given index of the given textgrid object.
 *
 * @details This function retrieves the color for the palette of type @a pal at the
 *          index @a idx of the textgrid object @a obj. The ARGB components are
 *          stored in the buffers @a r, @a g, @a b and @a a. If @a idx is not
 *          between @c 0 and the index of the latest set color, or if @a pal is
 *          #EVAS_TEXTGRID_PALETTE_NONE or #EVAS_TEXTGRID_PALETTE_LAST, the
 *          values of the components are @c 0. @a r, @a g, @a b and @a a can be
 *          @c NULL.
 *
 * @since   1.7
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj  The textgrid object to query for font information
 * @param[in]   pal  The type of the palette to set the color
 * @param[in]   idx  The index of the paletter to which the color is stored
 * @param[out]   r    A pointer to the red component of the color
 * @param[out]   g    A pointer to the green component of the color
 * @param[out]   b    A pointer to the blue component of the color
 * @param[out]   a    A pointer to the alpha component of the color
 *
 * @see evas_object_textgrid_palette_set()
 */
EAPI void evas_object_textgrid_palette_get(const Evas_Object *obj, Evas_Textgrid_Palette pal, int idx, int *r, int *g, int *b, int *a);

EAPI void evas_object_textgrid_supported_font_styles_set(Evas_Object *obj, Evas_Textgrid_Font_Style styles);
EAPI Evas_Textgrid_Font_Style evas_object_textgrid_supported_font_styles_get(const Evas_Object *obj);

/**
 * @brief   Sets the string at the given row of the given textgrid object.
 *
 * @details This function returns cells to the textgrid taken by
 *          evas_object_textgrid_cellrow_get(). The row pointer @a row should be the
 *          same row pointer returned by evas_object_textgrid_cellrow_get() for the
 *          same row @a y.
 *
 * @since   1.7
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj  The textgrid object to query for font information
 * @param[in]   y    The row index of the grid
 * @param[in]   row  The string as a sequence of #Evas_Textgrid_Cell
 *
 * @see evas_object_textgrid_cellrow_get()
 * @see evas_object_textgrid_size_set()
 * @see evas_object_textgrid_update_add()
 */
EAPI void evas_object_textgrid_cellrow_set(Evas_Object *obj, int y, const Evas_Textgrid_Cell *row);

/**
 * @brief   Gets the string at the given row of the given textgrid object.
 *
 * @details This function returns a pointer to the first cell of the line @a y
 *          of the textgrid object @a obj. If @a y is not between @c 0 and the
 *          number of lines of the grid - 1, or on error, this function return @c NULL.
 *
 * @since   1.7
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj  The textgrid object to query for font information
 * @param[in]   y    The row index of the grid
 * @return  A pointer to the first cell of the given row
 *
 * @see evas_object_textgrid_cellrow_set()
 * @see evas_object_textgrid_size_set()
 * @see evas_object_textgrid_update_add()
 */
EAPI Evas_Textgrid_Cell *evas_object_textgrid_cellrow_get(const Evas_Object *obj, int y);

/**
 * @brief   Indicates to evas that part of a textgrid region (cells) has been updated.
 *
 * @since   1.7
 *
 * @since_tizen 2.3
 *
 * @remarks This function declares to evas that a region of cells is updated by
 *          code and needs refreshing. An application should modify cells like this
 *          as an example:
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
 * @param[in]   obj  The textgrid object
 * @param[in]   x    The rect region of cells top-left x (column)
 * @param[in]   y    The rect region of cells top-left y (row)
 * @param[in]   w    The rect region size in number of cells (columns)
 * @param[in]   h    The rect region size in number of cells (rows)
 *
 * @see evas_object_textgrid_cellrow_set()
 * @see evas_object_textgrid_cellrow_get()
 * @see evas_object_textgrid_size_set()
 */
EAPI void evas_object_textgrid_update_add(Evas_Object *obj, int x, int y, int w, int h);

/**
 * @}
 */

/**
 * @defgroup Evas_Line_Group Line Object Functions
 * @ingroup Evas_Object_Specific
 *
 * @brief This group provides functions to deal with evas line objects.
 *
 * @remarks We do not guarantee any proper results if you create a Line object
 *          without setting the evas engine.
 *
 * @{
 */

/**
 * @brief   Adds a new evas line object to the given evas.
 *
 * @since_tizen 2.3
 *
 * @param[in]   e  The given evas
 * @return  The new evas line object
 */
EAPI Evas_Object *evas_object_line_add(Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * @brief  Sets the coordinates of the end points of the given evas line object.
 *
 * @since_tizen 2.3
 *
 * @param[in]  obj  The given evas line object
 * @param[in]  x1   The X coordinate of the first point
 * @param[in]  y1   The Y coordinate of the first point
 * @param[in]  x2   The X coordinate of the second point
 * @param[in]  y2   The Y coordinate of the second point
 */
EAPI void         evas_object_line_xy_set(Evas_Object *obj, Evas_Coord x1, Evas_Coord y1, Evas_Coord x2, Evas_Coord y2);

/**
 * @brief  Gets the coordinates of the end points of the given evas line object.
 *
 * @since_tizen 2.3
 *
 * @param[in]  obj  The given line object
 * @param[out]  x1   The pointer to an integer in which to store the X coordinate of the
 *              first end point
 * @param[out]  y1   The pointer to an integer in which to store the Y coordinate of the
 *              first end point
 * @param[out]  x2   The pointer to an integer in which to store the X coordinate of the
 *              second end point
 * @param[out]  y2   The pointer to an integer in which to store the Y coordinate of the
 *              second end point
 */
EAPI void         evas_object_line_xy_get(const Evas_Object *obj, Evas_Coord *x1, Evas_Coord *y1, Evas_Coord *x2, Evas_Coord *y2);

/**
 * @}
 */

/**
 * @defgroup Evas_Object_Polygon Polygon Object Functions
 * @ingroup Evas_Object_Specific
 *
 * @brief This group provides functions that operate on evas polygon objects.
 *
 * Hint: as evas does not provide ellipse, smooth paths or circle, one
 * can calculate points and convert these to a polygon.
 *
 * @remarks We do not guarantee any proper results if you create a Polygon
 *          object without setting the evas engine.
 *
 * @{
 */

/**
 * @brief   Adds a new evas polygon object to the given evas.
 *
 * @since_tizen 2.3
 *
 * @param[in]   e  The given evas
 * @return  A new evas polygon object
 */
EAPI Evas_Object *evas_object_polygon_add(Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * @brief  Adds the given point to the given evas polygon object.
 *
 * @since_tizen 2.3
 *
 * @param[in]  obj  The given evas polygon object
 * @param[in]  x    The X coordinate of the given point
 * @param[in]  y    The Y coordinate of the given point
 * @ingroup Evas_Polygon_Group
 */
EAPI void         evas_object_polygon_point_add(Evas_Object *obj, Evas_Coord x, Evas_Coord y) EINA_ARG_NONNULL(1);

/**
 * @brief  Removes all of the points from the given evas polygon object.
 *
 * @since_tizen 2.3
 *
 * @param[in]  obj  The given polygon object
 */
EAPI void         evas_object_polygon_points_clear(Evas_Object *obj) EINA_ARG_NONNULL(1);
/**
 * @}
 */

/**
 * @internal
 * @since 1.2
 *
 * Need description
 * @ingroup Evas_Object_Group
 */
EAPI void         evas_object_is_frame_object_set(Evas_Object *obj, Eina_Bool is_frame);

/**
 * @internal
 * @since 1.2
 *
 * Need description
 * @ingroup Evas_Object_Group
 */
EAPI Eina_Bool    evas_object_is_frame_object_get(Evas_Object *obj);

/**
 * @internal
 * @defgroup Evas_Smart_Group Smart Functions
   @ingroup Evas
 *
 * This group provides functions that deal with #Evas_Smart structs, creating definition
 * (classes) of objects that have customized behavior for methods
 * like evas_object_move(), evas_object_resize(),
 * evas_object_clip_set() and others.
 *
 * These objects accept the generic methods defined in @ref
 * Evas_Object_Group and the extensions defined in @ref
 * Evas_Smart_Object_Group. There are a couple of existent smart
 * objects in Evas itself (see @ref Evas_Object_Box, @ref
 * Evas_Object_Table and @ref Evas_Smart_Object_Clipped).
 *
 * @{
 */

/**
 * @def EVAS_SMART_CLASS_VERSION
 *
 * The version you have to put into the version field in the
 * #Evas_Smart_Class struct. Used to safeguard from binaries with old
 * smart object intefaces running with newer ones.
 */
#define EVAS_SMART_CLASS_VERSION 4

/**
 * @internal
 * @struct _Evas_Smart_Class
 *
 * @brief The structure type containing a smart object's @b base class definition.
 */
struct _Evas_Smart_Class
{
   const char                      *name; /**< The name string of the class */
   int                              version;
   void                             (*add)(Evas_Object *o); /**< Code to be run when adding object to a canvas */
   void                             (*del)(Evas_Object *o); /**< Code to be run when removing object from a canvas */
   void                             (*move)(Evas_Object *o, Evas_Coord x, Evas_Coord y); /**< Code to be run when moving object on a canvas. @a x and @a y are new coordinates you apply to the object. Use evas_object_geometry_get() if you need the old values, during this call. After that, the old values are lost */
   void                             (*resize)(Evas_Object *o, Evas_Coord w, Evas_Coord h); /**< Code to be run when resizing object on a canvas. @a w and @a h are new dimensions you apply to the object. Use evas_object_geometry_get() if you need the old values, during this call. After that, the old values are lost */
   void                             (*show)(Evas_Object *o); /**< Code to be run when showing object on a canvas */
   void                             (*hide)(Evas_Object *o); /**< Code to be run when hiding object on a canvas */
   void                             (*color_set)(Evas_Object *o, int r, int g, int b, int a); /**< Code to be run when setting color of object on a canvas. @a r, @a g, @a b and @a y are new color components one applied to the object. Use evas_object_color_get() if you need the old values, during this call. After that, the old values are lost */
   void                             (*clip_set)(Evas_Object *o, Evas_Object *clip); /**< Code to be run when setting clipper of object on a canvas. @a clip is a new clipper you apply to the object. Use evas_object_clip_get() if you need the old one, during this call. After that, the old (object pointer) value is lost */
   void                             (*clip_unset)(Evas_Object *o); /**< Code to be run when unsetting clipper of object on a canvas. If you need the pointer to a previous set clipper, during this call, use evas_object_clip_get(). After that, the old (object pointer) value is lost */
   void                             (*calculate)(Evas_Object *o); /**< Code to be run when object has rendering updates on a canvas */
   void                             (*member_add)(Evas_Object *o, Evas_Object *child); /**< Code to be run when a child member is added to object */
   void                             (*member_del)(Evas_Object *o, Evas_Object *child); /**< Code to be run when a child member is removed from object */

   const Evas_Smart_Class          *parent; /**< This class inherits from this parent */
   const Evas_Smart_Cb_Description *callbacks; /**< Callbacks at this level, @c NULL terminated */
   const Evas_Smart_Interface     **interfaces; /**< #Evas_Smart_Interface pointers array, @c NULL terminated. These are the interfaces supported at this level for an object (parents may have others) @since 1.7 */
   const void                      *data;
};

/**
 * @internal
 * @struct _Evas_Smart_Interface
 *
 * @brief   The structure type containing a smart object's @b base interface definition
 * @since   1.7
 *
 * @remarks Every Evas interface must have a name field, pointing to a global,
 *          constant string variable. This string pointer is the only way
 *          of retrieving back a given interface from a smart object. Two
 *          function pointers must be defined, too, which is called at
 *          object creation and deletion times.
 *
 *
 * @ingroup Evas_Smart_Group
 */
struct _Evas_Smart_Interface
{
   const char *name; /**< Name of the given interface */
   unsigned    private_size; /**< Size, in bytes, of the interface's private data blob. This is allocated and freed automatically for you. Get it with evas_object_smart_interface_data_get() */
   Eina_Bool   (*add)(Evas_Object *obj); /**< Function to be called at object creation time. This takes place @b before the object's smart @c add() function */
   void        (*del)(Evas_Object *obj); /**< Function to be called at object deletion time. This takes place @b after the object's smart @c del() function */
};

/**
 * @internal
 * @struct _Evas_Smart_Cb_Description
 *
 * @brief The structure type that describes a callback issued by a smart object
 *        (evas_object_smart_callback_call()), as defined in its smart object
 *        class. This is particularly useful to explain to end users and
 *        their code (i.e., introspection) what the parameter @a event_info
 *        points to.
 */
struct _Evas_Smart_Cb_Description
{
   const char *name; /**< callback name ("changed", for example) */

   /**
    * @brief Hint on the type of @a event_info parameter's contents on
    *        a #Evas_Smart_Cb callback.
    *
    * @remarks The type string uses the pattern similar to
    *          http://dbus.freedesktop.org/doc/dbus-specification.html#message-protocol-signatures,
    *          but extended to optionally include variable names within
    *          brackets preceding types. Example:
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
    * @remarks This type string is used as a hint and is @b not validated
    *          or enforced in any way. Implementors should make the best
    *          use of it to help bindings, documentation and other users
    *          of introspection features.
    */
   const char *type;
};

/**
 * @def EVAS_SMART_CLASS_INIT_NULL
 * @brief Definition for initializing to zero a whole Evas_Smart_Class structure.
 *
 * @see EVAS_SMART_CLASS_INIT_VERSION
 * @see EVAS_SMART_CLASS_INIT_NAME_VERSION
 * @see EVAS_SMART_CLASS_INIT_NAME_VERSION_PARENT
 * @see EVAS_SMART_CLASS_INIT_NAME_VERSION_PARENT_CALLBACKS
 */
#define EVAS_SMART_CLASS_INIT_NULL    {NULL, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}

/**
 * @def EVAS_SMART_CLASS_INIT_VERSION
 * @brief Definition for initializing to zero a whole Evas_Smart_Class structure and set version.
 *
 * @remarks Similar to EVAS_SMART_CLASS_INIT_NULL, but sets version field to
 *          latest EVAS_SMART_CLASS_VERSION.
 *
 * @see EVAS_SMART_CLASS_INIT_NULL
 * @see EVAS_SMART_CLASS_INIT_NAME_VERSION
 * @see EVAS_SMART_CLASS_INIT_NAME_VERSION_PARENT
 * @see EVAS_SMART_CLASS_INIT_NAME_VERSION_PARENT_CALLBACKS
 */
#define EVAS_SMART_CLASS_INIT_VERSION {NULL, EVAS_SMART_CLASS_VERSION, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}

/**
 * @def EVAS_SMART_CLASS_INIT_NAME_VERSION
 * @brief   Definition for initializing to zero a whole Evas_Smart_Class structure and set name
 *          and version.
 *
 * @remarks Similar to EVAS_SMART_CLASS_INIT_NULL, but sets version field to
 *          latest EVAS_SMART_CLASS_VERSION and name to the specified value.
 *
 * @remarks It keeps a reference to name field as a "const char *", that is,
 *          name must be available while the structure is used (hint: static or global!)
 *          and is not modified.
 *
 * @see EVAS_SMART_CLASS_INIT_NULL
 * @see EVAS_SMART_CLASS_INIT_VERSION
 * @see EVAS_SMART_CLASS_INIT_NAME_VERSION_PARENT
 * @see EVAS_SMART_CLASS_INIT_NAME_VERSION_PARENT_CALLBACKS
 */
#define EVAS_SMART_CLASS_INIT_NAME_VERSION(name)                                     {name, EVAS_SMART_CLASS_VERSION, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}

/**
 * @def EVAS_SMART_CLASS_INIT_NAME_VERSION_PARENT
 * @brief   Definition for initializing to zero a whole Evas_Smart_Class structure and set name,
 *          version and parent class.
 *
 * @remarks Similar to EVAS_SMART_CLASS_INIT_NULL, but sets version field to
 *          latest EVAS_SMART_CLASS_VERSION, name to the specified value and
 *          parent class.
 *
 * @remarks It keeps a reference to name field as a "const char *", that is,
 *          name must be available while the structure is used (hint: static or global!)
 *          and is not modified. Similarly, parent reference is kept.
 *
 * @see EVAS_SMART_CLASS_INIT_NULL
 * @see EVAS_SMART_CLASS_INIT_VERSION
 * @see EVAS_SMART_CLASS_INIT_NAME_VERSION
 * @see EVAS_SMART_CLASS_INIT_NAME_VERSION_PARENT_CALLBACKS
 */
#define EVAS_SMART_CLASS_INIT_NAME_VERSION_PARENT(name, parent)                      {name, EVAS_SMART_CLASS_VERSION, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, parent, NULL, NULL}

/**
 * @def EVAS_SMART_CLASS_INIT_NAME_VERSION_PARENT_CALLBACKS
 * @brief   Definition for initializing to zero a whole Evas_Smart_Class structure and set name,
 *          version, parent class and callbacks definition.
 *
 * @remarks Similar to EVAS_SMART_CLASS_INIT_NULL, but sets version field to
 *          latest EVAS_SMART_CLASS_VERSION, name to the specified value, parent
 *          class and callbacks at this level.
 *
 * @remarks It keeps a reference to name field as a "const char *", that is,
 *          name must be available while the structure is used (hint: static or global!)
 *          and is not modified. Similarly, parent and callbacks reference
 *          is kept.
 *
 * @see EVAS_SMART_CLASS_INIT_NULL
 * @see EVAS_SMART_CLASS_INIT_VERSION
 * @see EVAS_SMART_CLASS_INIT_NAME_VERSION
 * @see EVAS_SMART_CLASS_INIT_NAME_VERSION_PARENT
 */
#define EVAS_SMART_CLASS_INIT_NAME_VERSION_PARENT_CALLBACKS(name, parent, callbacks) {name, EVAS_SMART_CLASS_VERSION, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, parent, callbacks, NULL}

/**
 * @def EVAS_SMART_SUBCLASS_NEW
 *
 * @brief Convenience macro to subclass a given Evas smart class.
 *
 * @details This macro saves some typing when writing a smart class derived
 *          from another one. In order to work, the user @b must provide some
 *          functions adhering to the following guidelines:
 *          - @<prefix@>_smart_set_user(): The @b internal @c _smart_set
 *            function (defined by this macro) calls this one, provided by
 *            the user, after inheriting everything from the parent, which
 *            should <b>take care of setting the right member functions for
 *            the class</b>, both overrides and extensions, if any.
 *          - If this new class should be subclassable as well, a @b public
 *            @c _smart_set() function is desirable to fill in the class used as
 *            parent by the children. It is up to the user to provide this
 *            interface, which most likely calls @<prefix@>_smart_set() to
 *            get the job done.
 *
 *          After the macro's usage, the following are defined for use:
 *          - @<prefix@>_parent_sc: A pointer to the @b parent smart
 *            class. When calling parent functions from overloaded ones, use
 *            this global variable.
 *          - @<prefix@>_smart_class_new(): This function returns the
 *            #Evas_Smart needed to create smart objects with this class,
 *            which should be passed to evas_object_smart_add().
 *
 * @remarks @a smart_name has to be a pointer to a globally available
 *          string. The smart class created here just has a pointer set
 *          to that, and all object instances depend on it for smart class
 *          name lookup.
 *
 * @param   smart_name   The name used for the smart class \n 
 *                       For example, @c "Evas_Object_Box".
 * @param   prefix       Prefix used for all variables and functions defined
 *                       and referenced by this macro
 * @param   api_type     Type of the structure used as API for the smart class \n
 *                       Either #Evas_Smart_Class or something derived from it.
 * @param   parent_type  Type of the parent class API
 * @param   parent_func  Function that gets the parent class \n 
 *                       For example, evas_object_box_smart_class_get().
 * @param   cb_desc      Array of callback descriptions for this smart class
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
 * @brief   Convenience macro to subclass a given Evas smart class. This is the
 *          same as #EVAS_SMART_SUBCLASS_NEW, but <b>declares smart
 *          interfaces</b> besides the smart callbacks.
 *
 * @details This macro saves some typing when writing a smart class derived
 *          from another one. In order to work, the user @b must provide some
 *          functions adhering to the following guidelines:
 *          - @<prefix@>_smart_set_user(): The @b internal @c _smart_set
 *            function (defined by this macro) calls this one, provided by
 *            the user, after inheriting everything from the parent, which
 *            should <b>take care of setting the right member functions for
 *            the class</b>, both overrides and extensions, if any.
 *          - If this new class should be subclassable as well, a @b public
 *            @c _smart_set() function is desirable to fill in the class used as
 *            parent by the children. It is up to the user to provide this
 *            interface, which most likely calls @<prefix@>_smart_set() to
 *            get the job done.
 *
 *          After the macro's usage, the following is defined for use:
 *          - @<prefix@>_parent_sc: A pointer to the @b parent smart
 *            class. When calling parent functions from overloaded ones, use
 *            this global variable.
 *          - @<prefix@>_smart_class_new(): This function returns the
 *            #Evas_Smart needed to create smart objects with this class,
 *            which should be passed to evas_object_smart_add().
 *
 * @since   1.7
 *
 * @remarks @a smart_name has to be a pointer to a globally available
 *          string. The smart class created here just has a pointer set
 *          to that, and all object instances depend on it for smart class
 *          name lookup.
 *
 * @param   smart_name   The name used for the smart class \n 
 *                       For example, @c "Evas_Object_Box".
 * @param   prefix       Prefix used for all variables and functions defined
 *                       and referenced by this macro
 * @param   api_type     Type of the structure used as API for the smart class
 *                       Either #Evas_Smart_Class or something
 *                       derived from it.
 * @param   parent_type  Type of the parent class API
 * @param   parent_func  Function that gets the parent class \n 
 *                       For example, evas_object_box_smart_class_get().
 * @param   cb_desc      Array of smart callback descriptions for this smart class
 * @param   ifaces       Array of Evas smart interfaces for this smart class
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
 * @brief Convenience macro to allocate smart data only if needed.
 *
 * @remarks When writing a subclassable smart object, the @c .add() function
 *          needs to check if the smart private data is already allocated
 *          by some child object or not. This macro makes it easier to do it.
 *
 * @remarks This is an idiom used when one calls the parent's @c .add()
 *          after the specialized code. Naturally, the parent's base smart data
 *          has to be contemplated as the specialized one's first member, for
 *          things to work.
 *
 * @param   o          The Evas object passed to the @c .add() function
 * @param   priv_type  The type of the data to allocate
 */
#define EVAS_SMART_DATA_ALLOC(o, priv_type)              \
  priv_type * priv;                                      \
  priv = evas_object_smart_data_get(o);                  \
  if (!priv) {                                           \
       priv = (priv_type *)calloc(1, sizeof(priv_type)); \
       if (!priv) return;                                \
       evas_object_smart_data_set(o, priv);              \
    }

/**
 * @brief   Frees an #Evas_Smart struct.
 *
 * @since_tizen 2.3
 *
 * @remarks If this smart handle is created using evas_smart_class_new(),
 *          the associated #Evas_Smart_Class is not freed.
 *
 * @remarks If you are using the #EVAS_SMART_SUBCLASS_NEW schema to create your
 *          smart object, note that an #Evas_Smart handle is shared amongst all
 *          instances of the given smart class, through a static variable.
 *          Evas internally counts references on #Evas_Smart handles and free them
 *          when they are not referenced any more. Thus, this function is mostly of no use
 *          for Evas users.
 *
 * @param[in]   s  The #Evas_Smart struct to free
 */
EAPI void                              evas_smart_free(Evas_Smart *s) EINA_ARG_NONNULL(1);

/**
 * @brief   Creates a new #Evas_Smart from a given #Evas_Smart_Class struct.
 *
 * @since_tizen 2.3
 *
 * @remarks #Evas_Smart handles are necessary to create new @b instances of
 *          smart objects belonging to the class described by @a sc. That
 *          handle contains, besides the smart class interface definition,
 *          all its smart callbacks infrastructure set, too.
 *
 * @remarks If you are willing to subclass a given smart class to
 *          construct yours, consider using the #EVAS_SMART_SUBCLASS_NEW macro,
 *          which makes use of this function automatically for you.
 *
 * @param[in]   sc  The smart class definition
 * @return  A new #Evas_Smart pointer
 */
EAPI Evas_Smart                       *evas_smart_class_new(const Evas_Smart_Class *sc) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * @brief   Gets the #Evas_Smart_Class handle of an #Evas_Smart struct.
 *
 * @since_tizen 2.3
 *
 * @param[in]   s  A valid #Evas_Smart pointer
 * @return  The #Evas_Smart_Class
 */
EAPI const Evas_Smart_Class           *evas_smart_class_get(const Evas_Smart *s) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the data pointer set on an #Evas_Smart struct.
 *
 * @since_tizen 2.3
 *
 * @remarks This data pointer is set as the data field in the #Evas_Smart_Class
 *          passed in to evas_smart_class_new().
 *
 * @param[in]   s  A valid #Evas_Smart handle
 */
EAPI void                             *evas_smart_data_get(const Evas_Smart *s) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the smart callbacks known by this #Evas_Smart handle's smart
 *          class hierarchy.
 *
 * @details This function is likely different from
 *          evas_object_smart_callbacks_descriptions_get() as it contains
 *          the callbacks of @b all this class hierarchy sorted, while the
 *          direct smart class member refers only to that specific class and
 *          should not include the parent's.
 *
 * @since_tizen 2.3
 *
 * @remarks If no callbacks are known, this function returns @c NULL.
 *
 * @remarks The array elements and thus their contents are @b references to
 *          original values given to evas_smart_class_new() as
 *          Evas_Smart_Class::callbacks.
 *
 * @remarks The array is sorted by Evas_Smart_Cb_Description::name. The last
 *          array element is a @c NULL pointer and is not accounted for in @a
 *          count. Loop iterations can check any of these size indicators.
 *
 * @remarks Objects may provide per-instance callbacks. Use
 *          evas_object_smart_callbacks_descriptions_get() to get those
 *          as well.
 *
 * @param[in]       s      A valid #Evas_Smart handle.
 * @param[out]  count  The number of elements in the returned array
 * @return      The array with callback descriptions known by this smart
 *              class, with its size returned in @a count parameter \n 
 *              It should not be modified in any way. If no callbacks are
 *              known, @c NULL is returned. The array is sorted by event
 *              names and elements refer to the original values given to
 *              evas_smart_class_new()'s Evas_Smart_Class::callbacks
 *              (pointer to them).
 *
 * @see evas_object_smart_callbacks_descriptions_get()
 */
EAPI const Evas_Smart_Cb_Description **evas_smart_callbacks_descriptions_get(const Evas_Smart *s, unsigned int *count) EINA_ARG_NONNULL(1, 1);

/**
 * @brief   Finds a callback description for the callback named @a name.
 *
 * @since_tizen 2.3
 *
 * @param[in]   s     The #Evas_Smart where to search for class registered smart
 *                event callbacks
 * @param[in]   name  The name of the desired callback, which must @b not be @c
 *                NULL. The search has a special case for @a name being the
 *                same pointer as registered with #Evas_Smart_Cb_Description.
 *                You can use it to avoid excessive use of strcmp().
 * @return  A reference to the description if found, \n
 *          otherwise @c NULL if no description is found
 *
 * @see evas_smart_callbacks_descriptions_get()
 */
EAPI const Evas_Smart_Cb_Description  *evas_smart_callback_description_find(const Evas_Smart *s, const char *name) EINA_ARG_NONNULL(1, 2);

/**
 * brief    Sets one class to inherit from the other.
 *
 * @since_tizen 2.3
 *          everything after sizeof(Evas_Smart_Class) present in @a parent_sc,
 *          using @a parent_sc_size as reference.
 *
 * @remarks This is recommended instead of a single memcpy() since it takes
 *          care to not modify @a sc name, version, callbacks and possible
 *          other members.
 *
 * @param[in]   sc              The child class
 * @param[in]   parent_sc       The parent class, provides attributes
 * @param[in]   parent_sc_size  The size of parent_sc structure \n
 *                          The child should be at least this size. \n
 *                          Everything after @c Evas_Smart_Class size is copied
 *                          using regular memcpy().
 */
EAPI Eina_Bool                         evas_smart_class_inherit_full(Evas_Smart_Class *sc, const Evas_Smart_Class *parent_sc, unsigned int parent_sc_size) EINA_ARG_NONNULL(1, 2);

/**
 * @brief   Gets the number of users of the smart instance.
 *
 * @details This function tells you how many more uses of the smart instance are in
 *          existence. This should be used before freeing or clearing any of the
 *          Evas_Smart_Class that is used to create the smart instance. The smart
 *          instance refers to data in the Evas_Smart_Class used to create it and
 *          thus you cannot remove the original data until all users of it are gone.
 *          When the usage count goes to @c 0, you can evas_smart_free() the smart
 *          instance @a s and remove from memory any of the Evas_Smart_Class that
 *          is used to create the smart instance, if you desire. Removing it from
 *          memory without doing this causes problems (crashes, undefined
 *          behavior, and so on). So either never remove the original
 *          Evas_Smart_Class data from memory (have it be a constant structure and
 *          data), or use this API call and be very careful.
 *
 * @since_tizen 2.3
 *
 * @param[in]   s  The Evas_Smart to get the usage count of
 * @return  The number of uses of the smart instance
 */
EAPI int                               evas_smart_usage_get(const Evas_Smart *s);

/**
 * @def evas_smart_class_inherit
 * @brief   Easy to use version of evas_smart_class_inherit_full().
 *
 * @remarks This version uses sizeof(parent_sc), copying everything.
 *
 * @param   sc         The child class \n 
 *                     It has methods copied from @a parent_sc.
 * @param   parent_sc  The parent class \n 
 *                     It provides contents to be copied.
 * @return  @c 1 if everything is copied successfully, \n
 *          otherwise @c 0 on failure
 * @ingroup Evas_Smart_Group
 */
#define evas_smart_class_inherit(sc, parent_sc) evas_smart_class_inherit_full(sc, (Evas_Smart_Class *)parent_sc, sizeof(*parent_sc))

/**
 * @}
 */

/**
 * @defgroup Evas_Smart_Object_Group Smart Object Functions
   @ingroup Evas_Object_Group
 *
 * @brief This group provides functions dealing with Evas smart objects (instances).
 *
 * Smart objects are groupings of primitive Evas objects that behave
 * as a cohesive group. For instance, a file manager icon may be a
 * smart object composed of an image object, a text label, and two
 * rectangles that appear behind the image and text when the icon is
 * selected. As a smart object, the normal Evas object API could be
 * used on the icon object.
 *
 * Besides that, generally smart objects implement a <b>specific
 * API</b>, so that users interact with its own custom features. The
 * API takes the form of explicit exported functions one may call and
 * <b>smart callbacks</b>.
 *
 * @section Evas_Smart_Object_Group_Callbacks Smart events and callbacks
 *
 * Smart objects can elect events (smart events, from now on) occurring
 * inside of them to be reported back to their users via callback
 * functions (smart callbacks). This way, you can extend Evas' own
 * object events. They are defined by an <b>event string</b>, which
 * identifies them uniquely. There is also a function prototype
 * definition for the callback functions: Evas_Smart_Cb.
 *
 * When defining an #Evas_Smart_Class, smart object implementors are
 * strongly encouraged to properly set the Evas_Smart_Class::callbacks
 * callbacks description array, so that the users of the smart object
 * can have introspection on its events API <b>at run time</b>.
 *
 * @internal
 * @see @ref Evas_Smart_Group for class definitions.
 * @endinternal
 *
 * @{
 */

/**
 * @brief   Instantiates a new smart object described by @a s.
 *
 * @details This is the function that you should use when defining the public
 *          function @b adding an instance of the new smart object to a given
 *          canvas.
 *          @internal
 *          It takes care of setting all of its internals to work
 *          as they should, if the user set things properly, like
 *          #EVAS_SMART_SUBCLASS_NEW, for example.
 *          @endinternal
 *
 * @since_tizen 2.3
 *
 * @param[in]   e  The canvas on which to add the object
 * @param[in]   s  The #Evas_Smart describing the smart object
 * @return  A new #Evas_Object handle
 */
EAPI Evas_Object *evas_object_smart_add(Evas *e, Evas_Smart *s) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1, 2) EINA_MALLOC;

/**
 * @brief   Sets an Evas object as a member of a given smart object.
 *
 * @since_tizen 2.3
 *
 * @remarks Members are automatically stacked and layered together with the
 *          smart object. The various stacking functions operate on
 *          members relative to the other members instead of the entire canvas,
 *          since they now live on an exclusive layer (see
 *          evas_object_stack_above(), for more details).
 *
 * @remarks Any @a smart_obj object's specific implementation of the @c
 *          member_add() smart function takes place too, naturally.
 *
 * @param[in]   obj        The member object
 * @param[in]   smart_obj  The smart object
 *
 * @see evas_object_smart_member_del()
 * @see evas_object_smart_members_get()
 */
EAPI void         evas_object_smart_member_add(Evas_Object *obj, Evas_Object *smart_obj) EINA_ARG_NONNULL(1, 2);

/**
 * @brief   Removes a member object from a given smart object.
 *
 * @details This function removes a member object from a smart object, if it is added
 *          to any. The object is still on the canvas, but no longer
 *          associated with whichever smart object it is associated with.
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj  The member object
 *
 * @see evas_object_smart_member_add() for more details
 * @see evas_object_smart_members_get()
 */
EAPI void         evas_object_smart_member_del(Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the list of the member objects of a given Evas smart object.
 * @since   1.7
 *
 * @since_tizen 2.3
 *
 * @remarks The returned list should be freed with @c eina_list_free() when you
 *          no longer need it.
 *
 * @param[in]   obj  The smart object to get members from
 * @return  The list of the member objects of @a obj, \n
 *          otherwise @c NULL when a non-smart object is passed
 *
 * @see evas_object_smart_member_add()
 * @see evas_object_smart_member_del()
 */
EAPI Eina_List   *evas_object_smart_members_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the parent smart object of a given Evas object, if it has one.
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj  The Evas object to get the parent smart object from
 * @return  The parent smart object of @a obj \n
 *          otherwise @c NULL if @a obj is not a smart member of any Evas object
 */
EAPI Evas_Object *evas_object_smart_parent_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief   Checks whether a given smart object or any of its smart object
 *          parents is of a given smart class.
 *
 * @since_tizen 2.3
 *
 * @remarks If @a obj is not a smart object, this call fails
 *          immediately. Otherwise, make sure evas_smart_class_inherit() or its
 *          sibling functions were used correctly when creating the smart
 *          object's class, so it has a valid @b parent smart class pointer set.
 *
 * @remarks The checks use smart classes names and <b>string
 *          comparison</b>. There is a version of this same check using
 *          <b>pointer comparison</b>, since a smart class' name is a single
 *          string in Evas.
 *
 * @param[in]   obj   An Evas smart object to check the type of
 * @param[in]   type  The @b name (type) of the smart class to check for
 * @return  #EINA_TRUE if @a obj or any of its parents is of type @a type, \n
 *          otherwise #EINA_FALSE
 *
 * @see evas_object_smart_type_check_ptr()
 * @internal
 * @see #EVAS_SMART_SUBCLASS_NEW
 * @endinternal
 */
EAPI Eina_Bool    evas_object_smart_type_check(const Evas_Object *obj, const char *type) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1, 2);

/**
 * @internal
 * @brief   Checks whether a given smart object or any of its smart object
 *          parents is of a given smart class, <b>using pointer comparison</b>.
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj   An Evas smart object to check the type of
 * @param[in]   type  The type (name string) to check for \n
 *                Must be the name.
 * @return  #EINA_TRUE if @a obj or any of its parents is of type @a type, \n
 *          otherwise #EINA_FALSE
 *
 * @see evas_object_smart_type_check() for more details
 *
 * @ingroup Evas_Smart_Object_Group
 */
EAPI Eina_Bool    evas_object_smart_type_check_ptr(const Evas_Object *obj, const char *type) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1, 2);

/**
 * @internal
 * @brief   Gets the #Evas_Smart from which @a obj smart object is created.
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj  A smart object
 * @return  The #Evas_Smart handle, \n 
 *          otherwise @c NULL on errors
 */
EAPI Evas_Smart  *evas_object_smart_smart_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the user data stored on a given smart object.
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj  The smart object's handle
 * @return  A pointer to data stored using evas_object_smart_data_set(), \n
 *          otherwise @c NULL if none has been set
 *
 * @see evas_object_smart_data_set()
 */
EAPI void        *evas_object_smart_data_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief   Sets a pointer to the user data for a given smart object.
 *
 * @since_tizen 2.3
 *
 * @remarks This data is stored @b independently of the one set by
 *          evas_object_data_set(), naturally.
 *
 * @param[in]   obj   The smart object's handle
 * @param[in]   data  A pointer to user data
 *
 * @see evas_object_smart_data_get()
 */
EAPI void         evas_object_smart_data_set(Evas_Object *obj, void *data) EINA_ARG_NONNULL(1);

/**
 * @brief   Adds or registers a callback function to the smart event specified by
 *          @a event on the smart object @a obj.
 *
 * @since_tizen 2.3
 *
 * @remarks Smart callbacks look very similar to Evas callbacks, but are
 *          implemented as smart object's custom ones.
 *
 * @remarks This function adds a function callback to an smart object when the
 *          event named @a event occurs in it. The function is @a func.
 *
 * @remarks In the event of a memory allocation error during addition of the
 *          callback to the object, evas_alloc_error() should be used to
 *          determine the nature of the error, if any, and the program should
 *          sensibly try and recover.
 *
 * @remarks A smart callback function must have the ::Evas_Smart_Cb prototype
 *          definition. The first parameter (@a data) in this definition
 *          has the same value passed to evas_object_smart_callback_add() as
 *          the @a data parameter, at runtime. The second parameter @a obj is a
 *          handle to the object on which the event occurred. The third
 *          parameter, @a event_info, is a pointer to data which is totally
 *          dependent on the smart object's implementation and semantic for the
 *          given event.
 *
 * @remarks There is an infrastructure for introspection on smart objects'
 *          events (see evas_smart_callbacks_descriptions_get()), but no
 *          internal smart objects on Evas implement them yet.
 *
 * @remarks The event's name strings are implemented by each smart object.
 *          Please refer the documentation of a smart object which you are
 *          insterested in.
 *
 * @param[in]   obj    A smart object
 * @param[in]   event  The event's name string
 * @param[in]   func   The callback function
 * @param[in]   data   The user data to be passed to the callback function
 *
 * @see @ref Evas_Smart_Object_Group_Callbacks for more details.
 *
 * @see evas_object_smart_callback_del()
 */
EAPI void         evas_object_smart_callback_add(Evas_Object *obj, const char *event, Evas_Smart_Cb func, const void *data) EINA_ARG_NONNULL(1, 2, 3);

/**
 * @internal
 * @brief   Adds or registers a callback function to the smart event specified by
 *          @a event on the smart object @a obj.
 *
 * @details Except for the priority field, it is exactly the same as @ref evas_object_smart_callback_add.
 * @since   1.1
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj       A smart object
 * @param[in]   event     The event's name string
 * @param[in]   priority  The priority of the callback \n
 *                    Lower values are called first.
 * @param[in]   func      The callback function
 * @param[in]   data      The user data to be passed to the callback function
 *
 * @see evas_object_smart_callback_add
 */
EAPI void         evas_object_smart_callback_priority_add(Evas_Object *obj, const char *event, Evas_Callback_Priority priority, Evas_Smart_Cb func, const void *data);

/**
 * @brief   Deletes or unregisters a callback function from the smart event
 *          specified by @a event on the smart object @a obj.
 *
 * @details This function removes <b>the first</b> added smart callback on the
 *          object @a obj matching the event name @a event and the registered
 *          function pointer @a func. If the removal is successful it also
 *          returns the data pointer that is passed to
 *          evas_object_smart_callback_add() (that is the same as the
 *          parameter) when the callback(s) is(are) added to the canvas.
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj    A smart object
 * @param[in]   event  The event's name string
 * @param[in]   func   The callback function
 * @return  The data pointer, \n
 *          otherwise @c NULL if it is not successful
 *
 * @see evas_object_smart_callback_add() for more details.
 */
EAPI void        *evas_object_smart_callback_del(Evas_Object *obj, const char *event, Evas_Smart_Cb func) EINA_ARG_NONNULL(1, 2, 3);

/**
 * @brief   Deletes or unregisters a callback function from the smart event
 *          specified by @a event on the smart object @a obj.
 *
 * @details This function removes <b>the first</b> added smart callback on the
 *          object @a obj matching the event name @a event, the registered
 *          function pointer @a func and the callback data pointer @a data. If
 *          the removal is successful, it also returns the data pointer that
 *          is passed to evas_object_smart_callback_add() (that is the same
 *          as the parameter) when the callback(s) is(are) added to the canvas.
 *          If not successful @c NULL is returned. A common use would be to
 *          remove an exact match of a callback
 * @since   1.2
 *
 * @since_tizen 2.3
 *
 * @remarks To delete all smart event callbacks which match @a type and @a func,
 *          use evas_object_smart_callback_del().
 *
 * @param[in]   obj    A smart object
 * @param[in]   event  The event's name string
 * @param[in]   func   The callback function
 * @param[in]   data   The data pointer that is passed to the callback
 * @return  The data pointer
 *
 * @see evas_object_smart_callback_add() for more details.
 */
EAPI void        *evas_object_smart_callback_del_full(Evas_Object *obj, const char *event, Evas_Smart_Cb func, const void *data) EINA_ARG_NONNULL(1, 2, 3);

/**
 * @brief   Calls a given smart callback on the smart object @a obj.
 *
 * @details This function should be called @b internally, from the smart object's own
 *          code, when some specific event has occurred and the implementor
 *          wants to listen to the object's events API (see @ref
 *          Evas_Smart_Object_Group_Callbacks). The documentation for the smart
 *          object should include a list of possible events and what type of @a
 *          event_info to expect for each of them. Also, when defining an
 *          #Evas_Smart_Class, smart object implementors are strongly
 *          encouraged to properly set the Evas_Smart_Class::callbacks
 *          callbacks description array, so that the users of the smart object
 *          can have introspection on its events API <b>at run time</b>.
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj         The smart object
 * @param[in]   event       The event's name string
 * @param[in]   event_info  The pointer to an event specific struct or information to
 *                      pass to the callback functions registered on this smart event
 */
EAPI void         evas_object_smart_callback_call(Evas_Object *obj, const char *event, void *event_info) EINA_ARG_NONNULL(1, 2);

/**
 * @brief   Sets an smart object @b instance's smart callbacks descriptions.
 *
 * @since_tizen 2.3
 *
 * @remarks These descriptions are hints to be used by introspection and are
 *          not enforced in any way.
 *
 * @remarks It is not checked if instance callbacks descriptions have the
 *          same name as respective possibly registered in the smart object
 *          @b class. Both are kept in different arrays and users of
 *          evas_object_smart_callbacks_descriptions_get() should handle this
 *          case as they wish.
 *
 * @remarks Because @a descriptions must be @c NULL terminated, and
 *          because a @c NULL name makes little sense, too,
 *          Evas_Smart_Cb_Description::name must @b not be @c NULL.
 *
 * @remarks While instance callbacks descriptions are possible, they are
 *          @b not recommended. Use @b class callbacks descriptions
 *          instead as they make it easier for you to use smart objects
 *          and use less memory, as descriptions and arrays are
 *          shared among all instances.
 *
 * @param[in]   obj           A smart object
 * @param[in]   descriptions  @c NULL terminated array with #Evas_Smart_Cb_Description descriptions \n 
 *                        Array elements are not modified at run time, but references to 
 *                        them and their contents are made, so this array should be kept 
 *                        alive during the whole object's lifetime.
 * @return  #EINA_TRUE if the descriptions are set successfully, 
 *          otherwise #EINA_FALSE on failure
 */
EAPI Eina_Bool    evas_object_smart_callbacks_descriptions_set(Evas_Object *obj, const Evas_Smart_Cb_Description *descriptions) EINA_ARG_NONNULL(1);

/**
 * @brief   Gets a smart object's smart callback descriptions (both
 *          instance and class ones).
 *
 * @details This function searches for registered callback descriptions for both
 *          instance and class of the given smart object. These arrays are
 *          sorted by Evas_Smart_Cb_Description::name and also @c NULL
 *          terminated, so both @a class_count and @a instance_count can be
 *          ignored, if the caller wishes so. The terminator @c NULL is not
 *          counted in these values.
 *
 * @since_tizen 2.3
 *
 * @remarks If just class descriptions are of interest, try
 *          evas_smart_callbacks_descriptions_get() instead.
 *
 * @remarks Use @c NULL pointers on the descriptions or counters that you are not
 *          interested in: they are ignored by the function.
 *
 * @param[in]   obj                    The smart object to get callback descriptions from
 * @param[in]   class_descriptions     The class callbacks descriptions array, if any, that is returned \n 
 *                                 If no descriptions are known, @c NULL is returned.
 * @param[in]   class_count            The number of class callbacks descriptions, that is returned
 * @param[in]   instance_descriptions  The instance callbacks descriptions array, if any, that is returned \n
 *                                 If no descriptions are known, @c NULL is returned.
 * @param[in]   instance_count         The number of instance callbacks descriptions, that is returned
 *
 * @see evas_smart_callbacks_descriptions_get()
 */
EAPI void         evas_object_smart_callbacks_descriptions_get(const Evas_Object *obj, const Evas_Smart_Cb_Description ***class_descriptions, unsigned int *class_count, const Evas_Smart_Cb_Description ***instance_descriptions, unsigned int *instance_count) EINA_ARG_NONNULL(1);

/**
 * @internal
 * @brief   Finds the callback description for callback called @a name.
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj                   The smart object
 * @param[in]   name                  The name of desired callback, \n
 *                                This must @b not be @c NULL. The search has a special case 
 *                                for @a name being the same pointer as registered with 
 *                                Evas_Smart_Cb_Description; you can use it to avoid excessive 
 *                                use of strcmp().
 * @param[in]   class_description     The pointer to return class description or @c NULL, if not found \n 
 *                                If parameter is @c NULL, no search is done on class descriptions.
 * @param[in]   instance_description  The pointer to return instance description or @c NULL if not found \n 
 *                                If parameter is @c NULL, no search is done on instance descriptions.
 * @return  The reference to description if found, \n
 *          otherwise @c NULL if not found
 */
EAPI void         evas_object_smart_callback_description_find(const Evas_Object *obj, const char *name, const Evas_Smart_Cb_Description **class_description, const Evas_Smart_Cb_Description **instance_description) EINA_ARG_NONNULL(1, 2);

/**
 * @internal
 * @brief   Gets an Evas smart object's interface, by name string pointer.
 * @since   1.7
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj   An Evas smart object.
 * @param[in]   name  The name string of the desired interface \n 
 *                This must be the same pointer used at the interface's declaration, 
 *                when creating the smart object @a obj.
 * @return  The interface's handle pointer, \n 
 *          otherwise @c NULL if not found
 */
EAPI const void       *evas_object_smart_interface_get(const Evas_Object *obj, const char *name);

/**
 * @internal
 * @brief   Gets an Evas smart object interface's <b>private data</b>.
 * @since   1.7
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj    An Evas smart object
 * @param[in]   iface  The given object's interface handle
 * @return  The object interface's private data blob pointer, \n
 *          otherwise @c NULL if not found
 */
EAPI void             *evas_object_smart_interface_data_get(const Evas_Object *obj, const Evas_Smart_Interface *iface);

/**
 * @brief   Marks smart object as changed, dirty.
 *
 * @details This function flags the given object as needing recalculation,
 *          forcefully. As an effect, on the next rendering cycle its @b
 *          calculate() (see #Evas_Smart_Class) smart function is called.
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj  The given Evas smart object
 *
 * @see evas_object_smart_need_recalculate_set().
 * @see evas_object_smart_calculate().
 */
EAPI void         evas_object_smart_changed(Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * @internal
 * @brief   Sets or unsets the flag signalling that a given smart object needs to
 *          get recalculated.
 *
 * @since_tizen 2.3
 *
 * @remarks If this flag is set, then the @c calculate() smart function of @a
 *          obj is called, if one is provided, during rendering phase of
 *          Evas (see evas_render()), after which this flag is
 *          automatically unset.
 *
 * @remarks If that smart function is not provided for the given object, this
 *          flag is left unchanged.
 *
 * @remarks Just setting this flag does not make the canvas' whole scene
 *          dirty, by itself, and evas_render() has no effect. To
 *          force that, use evas_object_smart_changed(), that also
 *          automatically calls this function, with
 *          #EINA_TRUE as parameter.
 *
 * @param[in]   obj    The smart object
 * @param[in]   value  Set #EINA_TRUE to recalculate the smart object, \n
 *                 otherwise set #EINA_FALSE to not recalculate the smart object
 *
 * @see evas_object_smart_need_recalculate_get()
 * @see evas_object_smart_calculate()
 * @see evas_smart_objects_calculate()
 */
EAPI void         evas_object_smart_need_recalculate_set(Evas_Object *obj, Eina_Bool value) EINA_ARG_NONNULL(1);

/**
 * @internal
 * @brief   Gets the value of the flag signalling that a given smart object needs to
 *          get recalculated.
 *
 * @since_tizen 2.3
 *
 * @remarks This flag is unset during the rendering phase, when the
 *          @c calculate() smart function is called, if one is provided.
 *          If it is not provided, then the flag is left unchanged
 *          after the rendering phase.
 *
 * @param[in]   obj  The smart object
 * @return  #EINA_TRUE if flag is set to recalculate, \n
 *          otherwise #EINA_FALSE if flag is not set to recalculate
 *
 * @see evas_object_smart_need_recalculate_set()
 */
EAPI Eina_Bool    evas_object_smart_need_recalculate_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @internal
 * @brief   Calls the @b calculate() smart function immediately on a given smart object.
 *
 * @details This function forces immediate calculations (see #Evas_Smart_Class)
 *          needed for rendering of this object and, besides, unset the
 *          flag on it telling it needs recalculation for the next rendering phase.
 *
 * @since_tizen 2.3
 *
 * @param[in]   obj  The smart object's handle
 *
 * @see evas_object_smart_need_recalculate_set()
 */
EAPI void         evas_object_smart_calculate(Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * @brief  Calls user-provided @c calculate() smart functions and unset the
 *         flag signalling that the object needs to get recalculated to @b all
 *         smart objects in the canvas.
 *
 * @since_tizen 2.3
 *
 * @param[in]  e  The canvas to calculate all smart objects in
 *
 * @see evas_object_smart_need_recalculate_set()
 */
EAPI void         evas_smart_objects_calculate(Evas *e);

/**
 * @brief   Gets the internal counter that counts the number of smart calculations.
 * @since   1.1
 *
 * @since_tizen 2.3
 *
 * @remarks Whenever evas performs smart object calculations on the whole canvas
 *          it increments a counter by @c 1. This is the smart object calculate counter
 *          that this function returns the value of. It starts at the value of @c 0 and
 *          increases (and eventually wrap around to negative values and so on) by
 *          @c 1 every time objects are calculated. You can use this counter to ensure
 *          that you do not re-do calculations within the same calculation generation or run
 *          if the calculations maybe cause self-feeding effects.
 *
 * @param[in]   e  The canvas to get the calculate counter from
 * @return The number of smart calculations
 */
EAPI int          evas_smart_objects_calculate_count_get(const Evas *e);

/**
 * @internal
 * @brief   Moves all children objects of a given smart object relative to a
 *          given offset.
 *
 * @details This function makes each of @a obj object's children to move, from where
 *          they before, with those delta values (offsets) on both directions.
 *
 * @since_tizen 2.3
 *
 * @remarks This is most useful on custom smart @c move() functions.
 *
 * @remarks Clipped smart objects already make use of this function on
 *          their @c move() smart function definition.
 *
 * @param[in]   obj  The smart object
 * @param[in]   dx   The horizontal offset (delta)
 * @param[in]   dy   The vertical offset (delta)
 */
EAPI void         evas_object_smart_move_children_relative(Evas_Object *obj, Evas_Coord dx, Evas_Coord dy) EINA_ARG_NONNULL(1);

/**
 * @}
 */

/**
 * @internal
 * @defgroup Evas_Smart_Object_Clipped Clipped Smart Object
 * @ingroup Evas_Smart_Object_Group
 *
 * @brief  This group provides functions for clipped smart objects.
 *
 * Clipped smart object is a base to construct other smart objects
 * based on the concept of having an internal clipper that is applied
 * to all children objects. This clipper controls the visibility,
 * clipping and color of sibling objects (remember that the clipping
 * is recursive, and clipper color modulates the color of its
 * clippees). By default, this base also moves children relatively
 * to the parent, and delete them when parent is deleted. In other
 * words, it is the base for simple object grouping.
 *
 * @see evas_object_smart_clipped_smart_set()
 *
 * @{
 */

/**
 * @brief The strcuture type that every subclass should provide this at the beginning of their own
 *        data set with evas_object_smart_data_set().
 */
typedef struct _Evas_Object_Smart_Clipped_Data Evas_Object_Smart_Clipped_Data;
struct _Evas_Object_Smart_Clipped_Data
{
   Evas_Object *clipper;
   Evas        *evas;
};

/**
 * @brief   Gets the clipper object for the given clipped smart object.
 *
 * @since_tizen 2.3
 *
 * @remarks Use this function if you want to change any of this clipper's
 *          properties, like colors.
 *
 * @param[in]   obj  The clipped smart object to retrieve associated clipper from
 * @return  The clipper object
 *
 * @see evas_object_smart_clipped_smart_add()
 */
EAPI Evas_Object            *evas_object_smart_clipped_clipper_get(Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @internal
 * @brief   Sets a given smart class' callbacks so it implements the <b>clipped smart
 *          object</b>'s interface.
 *
 * @since_tizen 2.3
 *
 * @remarks This call assigns all the required methods of the @a sc
 *          #Evas_Smart_Class instance to the implementations set for clipped
 *          smart objects. If one wants to "subclass" it, call this function
 *          and then override desired values. If one wants to call any original
 *          method, save it somewhere. 
 *
 * @remarks The following is an example:
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
 * @remarks The default behavior for each of #Evas_Smart_Class functions on a
 *          clipped smart object are:
 *          - @c add: creates a hidden clipper with "infinite" size, to clip any incoming members;
 *          - @c del: delete all children objects;
 *          - @c move: move all objects relative relatively;
 *          - @c resize: <b>not defined</b>;
 *          - @c show: if there are children objects, show clipper;
 *          - @c hide: hides clipper;
 *          - @c color_set: set the color of clipper;
 *          - @c clip_set: set clipper of clipper;
 *          - @c clip_unset: unset the clipper of clipper;
 *
 * @remarks There are other means of assigning parent smart classes to
 *          child ones, like the #EVAS_SMART_SUBCLASS_NEW macro or the
 *          evas_smart_class_inherit_full() function.
 *
 * @param[in]   sc  The smart class handle to operate on
 */
EAPI void                    evas_object_smart_clipped_smart_set(Evas_Smart_Class *sc) EINA_ARG_NONNULL(1);

/**
 * @internal
 * @brief  Gets a pointer to the <b>clipped smart object's</b> class, to use
 *         for proper inheritance.
 *
 * @since_tizen 2.3
 *
 * @see #Evas_Smart_Object_Clipped for more information on this smart class.
 */
EAPI const Evas_Smart_Class *evas_object_smart_clipped_class_get(void) EINA_CONST;

/**
 * @}
 */

/**
 * @defgroup Evas_Object_Box Box Smart Object
 * @ingroup Evas_Smart_Object_Group
 *
 * @brief  This group provides functions for bos smart objects.
 *
 * A box is a convenience smart object that packs children inside it
 * in @b sequence, using a layouting function specified by the
 * user. There are a couple of pre-made layouting functions <b>built-in
 * in Evas</b>, all of them using children size hints to define their
 * size and alignment inside their cell space.
 *
 * @see @ref Evas_Object_Group_Size_Hints
 *
 * @{
 */

/**
 * @typedef Evas_Object_Box_Api
 *
 * @brief  The structure type containing the smart class extension, providing extra box object requirements.
 */
typedef struct _Evas_Object_Box_Api Evas_Object_Box_Api;

/**
 * @typedef Evas_Object_Box_Data
 *
 * @brief  The structure type containing the smart object instance data, providing box object requirements.
 */
typedef struct _Evas_Object_Box_Data Evas_Object_Box_Data;

/**
 * @typedef Evas_Object_Box_Option
 *
 * @brief The structure type containing the base structure for a box option.
 *        Box options are a way of extending box items properties, which are taken into account
 *        for layouting decisions. The box layouting functions provided by
 *        Evas only relies on objects' canonical size hints to layout
 *        them, so the basic box option has @b no (custom) property set.
 *
 *        Users creating their own layouts, but not depending on extra child
 *        items' properties, would be fine just using
 *        evas_object_box_layout_set(). But if one desires a layout depending
 *        on extra child properties, he or she has to @b subclass the box smart
 *        object. Thus, by using evas_object_box_smart_class_get() and
 *        evas_object_box_smart_set(), the @c option_new() and @c
 *        option_free() smart class functions should be properly
 *        redefined or extended.
 *
 *        Object properties are bound to an integer identifier and must have
 *        a name string. Their values are open to any data. See the API on
 *        option properties for more details.
 */
typedef struct _Evas_Object_Box_Option Evas_Object_Box_Option;

/**
 * @typedef Evas_Object_Box_Layout
 *
 * @brief Called for the function signature for an Evas box object layouting routine. 
 *        @a o is the box object in question, @a priv is the box's internal data
 *        and, @a user_data is any custom data you could have set to
 *        a given box layouting function, with evas_object_box_layout_set().
 */
typedef void (*Evas_Object_Box_Layout)(Evas_Object *o, Evas_Object_Box_Data *priv, void *user_data);

/**
 * @def EVAS_OBJECT_BOX_API_VERSION
 *
 * @brief Definition of the current version for Evas box object smart class, a value which goes
 *        to _Evas_Object_Box_Api::version.
 */
#define EVAS_OBJECT_BOX_API_VERSION 1

/**
 * @struct _Evas_Object_Box_Api
 *
 * @brief The structure type that should be used by any smart class inheriting from
 *        the box's one, to provide custom box behavior which could not be
 *        achieved only by providing a layout function, with evas_object_box_layout_set().
 *
 * @extends Evas_Smart_Class
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
 * @brief Definition for initializing for a whole #Evas_Object_Box_Api structure, with
 *        @c NULL values on its specific fields.
 *
 * @param smart_class_init  The initializer to use for the "base" field 
 *                          (#Evas_Smart_Class)
 *
 * @see EVAS_SMART_CLASS_INIT_NULL
 * @see EVAS_SMART_CLASS_INIT_VERSION
 * @see EVAS_SMART_CLASS_INIT_NAME_VERSION
 * @see EVAS_OBJECT_BOX_API_INIT_NULL
 * @see EVAS_OBJECT_BOX_API_INIT_VERSION
 * @see EVAS_OBJECT_BOX_API_INIT_NAME_VERSION
 */
#define EVAS_OBJECT_BOX_API_INIT(smart_class_init) {smart_class_init, EVAS_OBJECT_BOX_API_VERSION, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}

/**
 * @def EVAS_OBJECT_BOX_API_INIT_NULL
 *
 * @brief  Definition for initializing to zero out a whole #Evas_Object_Box_Api structure.
 *
 * @see EVAS_OBJECT_BOX_API_INIT_VERSION
 * @see EVAS_OBJECT_BOX_API_INIT_NAME_VERSION
 * @see EVAS_OBJECT_BOX_API_INIT
 */
#define EVAS_OBJECT_BOX_API_INIT_NULL    EVAS_OBJECT_BOX_API_INIT(EVAS_SMART_CLASS_INIT_NULL)

/**
 * @def EVAS_OBJECT_BOX_API_INIT_VERSION
 *
 * @brief   Definition for initializing to zero out a whole #Evas_Object_Box_Api structure and
 *          set a specific version on it.
 *
 * @remarks This is similar to #EVAS_OBJECT_BOX_API_INIT_NULL, but it sets
 *          the version field of #Evas_Smart_Class (base field) to the latest
 *          EVAS_SMART_CLASS_VERSION.
 *
 * @see EVAS_OBJECT_BOX_API_INIT_NULL
 * @see EVAS_OBJECT_BOX_API_INIT_NAME_VERSION
 * @see EVAS_OBJECT_BOX_API_INIT
 */
#define EVAS_OBJECT_BOX_API_INIT_VERSION EVAS_OBJECT_BOX_API_INIT(EVAS_SMART_CLASS_INIT_VERSION)

/**
 * @def EVAS_OBJECT_BOX_API_INIT_NAME_VERSION
 *
 * @brief   Definition for initializing to zero out a whole #Evas_Object_Box_Api structure and
 *          set its name and version.
 *
 * @remarks This is similar to #EVAS_OBJECT_BOX_API_INIT_NULL, but it also
 *          sets the version field of #Evas_Smart_Class (base field) to the
 *          latest EVAS_SMART_CLASS_VERSION and name it to the specific value.
 *
 * @remarks It keeps a reference to the name field as a <c>"const char *"</c>,
 *          i.e., the name must be available while the structure is
 *          used (hint: static or global variable) and must not be modified.
 *
 * @see EVAS_OBJECT_BOX_API_INIT_NULL
 * @see EVAS_OBJECT_BOX_API_INIT_VERSION
 * @see EVAS_OBJECT_BOX_API_INIT
 */
#define EVAS_OBJECT_BOX_API_INIT_NAME_VERSION(name) EVAS_OBJECT_BOX_API_INIT(EVAS_SMART_CLASS_INIT_NAME_VERSION(name))

/**
 * @struct _Evas_Object_Box_Data
 *
 * @brief The structure type that augments clipped smart object's instance data,
 *        providing extra members required by generic box implementation. If
 *        a subclass inherits from #Evas_Object_Box_Api, then it may augment
 *        #Evas_Object_Box_Data to fit its own needs.
 *
 * @extends Evas_Object_Smart_Clipped_Data
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

/**
 * @brief Evas_Object_Box_Option struct fields
 */
struct _Evas_Object_Box_Option
{
   Evas_Object *obj;    /**< Pointer to the box child object, itself */
   Eina_Bool    max_reached : 1;
   Eina_Bool    min_reached : 1;
   Evas_Coord   alloc_size;
};

/**
 * @brief   Sets the default box @a api struct (Evas_Object_Box_Api)
 *          with the default values. This may be used to extend that API.
 *
 * @param   api  The box API struct to set back, most probably with
 *               overridden fields (on class extensions scenarios)
 */
EAPI void                       evas_object_box_smart_set(Evas_Object_Box_Api *api) EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the Evas box smart class, for inheritance purposes.
 *
 * @remarks The returned value is @b not to be modified, just use it as your
 *          parent class.
 * @return  The (canonical) Evas box smart class
 */
EAPI const Evas_Object_Box_Api *evas_object_box_smart_class_get(void) EINA_CONST;

/**
 * @brief   Sets a new layouting function to a given box object.
 *
 * @since_tizen 2.3
 *
 * @remarks A box layout function affects how a box object displays child
 *          elements within its area. The list of pre-defined box layouts
 *          available in Evas is:
 *          - evas_object_box_layout_horizontal()
 *          - evas_object_box_layout_vertical()
 *          - evas_object_box_layout_homogeneous_horizontal()
 *          - evas_object_box_layout_homogeneous_vertical()
 *          - evas_object_box_layout_homogeneous_max_size_horizontal()
 *          - evas_object_box_layout_homogeneous_max_size_vertical()
 *          - evas_object_box_layout_flow_horizontal()
 *          - evas_object_box_layout_flow_vertical()
 *          - evas_object_box_layout_stack()
 *          See each of their documentation texts for details on them.
 *
 * @remarks A box layouting function is triggered by the @c
 *          'calculate' smart callback of the box's smart class.
 *
 * @param[in]   o          The box object to operate on
 * @param[in]   cb         The new layout function to set on @a o
 * @param[in]   data       The data pointer to be passed to @a cb
 * @param[in]   free_data  The function to free @a data, if need be
 */
EAPI void                       evas_object_box_layout_set(Evas_Object *o, Evas_Object_Box_Layout cb, const void *data, void (*free_data)(void *data)) EINA_ARG_NONNULL(1, 2);

/**
 * @brief   Adds a new box object on the provided canvas.
 *
 * @since_tizen 2.3
 *
 * @remarks After instantiation, if a box object has not its layout function
 *          set, via evas_object_box_layout_set(), it has it by default
 *          set to evas_object_box_layout_horizontal(). The remaining
 *          properties of the box must be set/retrieved via
 *          <c>evas_object_box_{h,v}_{align,padding}_{get,set)()</c>.
 *
 * @param[in]   evas  The canvas to create the box object on
 * @return  A pointer to a new box object, \n
 *          otherwise @c NULL on error
 */
EAPI Evas_Object               *evas_object_box_add(Evas *evas) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * @brief   Adds a new box as a @b child of a given smart object.
 *
 * @since_tizen 2.3
 *
 * @remarks This function is a helper function that has the same effect of putting a new
 *          box object into @a parent by use of evas_object_smart_member_add().
 *
 * @param[in]   parent  The parent smart object to put the new box in
 * @return  A pointer to a new box object, \n
 *          otherwise @c NULL on error
 *
 * @see evas_object_box_add()
 */
EAPI Evas_Object               *evas_object_box_add_to(Evas_Object *parent) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * @brief  Layout function which sets the box @a o to a (basic) horizontal box.
 *
 * In this layout, the box object's overall behavior is controlled by
 * its padding/alignment properties, which are set by the
 * <c>evas_object_box_{h,v}_{align,padding}_set()</c> family of
 * functions. The size hints of the elements in the box -- set by the
 * <c>evas_object_size_hint_{align,padding,weight}_set()</c> functions
 * -- also control the way this function works.
 *
 * @par Box's properties:
 * @c align_h controls the horizontal alignment of the child objects
 * relative to the containing box. When set to @c 0.0, children are
 * aligned to the left. A value of @c 1.0 makes them aligned to the
 * right border. Values in between align them proportionally. Note
 * that if the size required by the children, which is given by their
 * widths and the @c padding_h property of the box, is bigger than the
 * their container's width, the children are displayed out of the
 * box's bounds. A negative value of @c align_h makes the box to
 * @b justify its children. The padding between them, in this case, is
 * corrected so that the leftmost one touches the left border and the
 * rightmost one touches the right border (even if they must
 * overlap). The @c align_v and @c padding_v properties of the box
 * @b do not contribute to its behaviour when this layout is chosen.
 *
 * @par Child element's properties:
 * @c align_x does @b not influence the box's behavior. @c padding_l
 * and @c padding_r sum up to the container's horizontal padding
 * between elements. The child's @c padding_t, @c padding_b and
 * @c align_y properties apply for padding or alignment relative to the
 * overall height of the box. Finally, there is the @c weight_x
 * property, which, if set to a non-zero value, tells the container
 * that the child width is @b not pre-defined. If the container can not
 * accommodate all its children, it sets the widths of the ones
 * <b>with weights</b> to sizes as small as they can all fit into
 * it. If the size required by the children is less than the
 * available, the box increases its childrens' (which have weights)
 * widths as to fit the remaining space. The @c weight_x property,
 * besides telling the element is resizable, gives a @b weight for the
 * resizing process.  The parent box tries to distribute (or take
 * off) widths accordingly to the @b normalized list of weights: most
 * weighted children remain or get larger in this process than the least
 * ones. @c weight_y does not influence the layout.
 *
 * If you desire that, besides having weights, child elements must be
 * resized bounded to a minimum or maximum size, those size hints must
 * be set, by the <c>evas_object_size_hint_{min,max}_set()</c>
 * functions.
 *
 * @since_tizen 2.3
 *
 * @param[in]  o     The box object in question
 * @param[in]  priv  The smart data of the @a o
 * @param[in]  data  The data pointer passed on evas_object_box_layout_set(), if any
 */
EAPI void                       evas_object_box_layout_horizontal(Evas_Object *o, Evas_Object_Box_Data *priv, void *data) EINA_ARG_NONNULL(1, 2);

/**
 * @brief   Sets the box @a o to a (basic) vertical box.
 *
 * @details This Layout function behaves analogously to
 *          evas_object_box_layout_horizontal(). The description of its
 *          behaviour can be derived from that function's documentation.
 *
 * @param[in]  o     The box object in question
 * @param[in]  priv  The smart data of the @a o
 * @param[in]  data  The data pointer passed on evas_object_box_layout_set(), if any
 *
 * @since_tizen 2.3
 *
 */
EAPI void                       evas_object_box_layout_vertical(Evas_Object *o, Evas_Object_Box_Data *priv, void *data) EINA_ARG_NONNULL(1, 2);

/**
 * @brief   Sets the box @a o to a @b homogeneous vertical box.
 *
 * @details This Layout function behaves analogously to
 *          evas_object_box_layout_homogeneous_horizontal(). The description
 *          of its behaviour can be derived from that function's documentation.
 *
 * @param[in]  o     The box object in question
 * @param[in]  priv  The smart data of the @a o
 * @param[in]  data  The data pointer passed on evas_object_box_layout_set(), if any
 *
 * @since_tizen 2.3
 *
 */
EAPI void                       evas_object_box_layout_homogeneous_vertical(Evas_Object *o, Evas_Object_Box_Data *priv, void *data) EINA_ARG_NONNULL(1, 2);

/**
 * @brief  Sets the box @a o to a @b homogeneous horizontal box.
 *
 * In a homogeneous horizontal box, its width is divided @b equally
 * between the contained objects. The box's overall behavior is
 * controlled by its padding or alignment properties, which are set by
 * the <c>evas_object_box_{h,v}_{align,padding}_set()</c> family of
 * functions.  The size hints the elements in the box -- set by the
 * <c>evas_object_size_hint_{align,padding,weight}_set()</c> functions
 * -- also control the way this function works.
 *
 * @par Box's properties:
 * @c align_h has no influence on the box for this layout.
 * @c padding_h tells the box to draw empty spaces of that size, in
 * pixels, between the (equal) child objects' cells. The @c align_v
 * and @c padding_v properties of the box do not contribute to its
 * behaviour when this layout is chosen.
 *
 * @par Child element's properties:
 * @c padding_l and @c padding_r sum up to the required width of the
 * child element. The @c align_x property tells the relative position
 * of this overall child width in its allocated cell (@c 0.0 to
 * extreme left, @c 1.0 to extreme right). A value of @c -1.0 to
 * @c align_x makes the box try to resize this child element to the exact
 * width of its cell (respecting the minimum and maximum size hints on
 * the child's width and accounting for its horizontal padding
 * hints). The child's @c padding_t, @c padding_b and @c align_y
 * properties apply for padding or alignment relative to the overall
 * height of the box. A value of @c -1.0 to @c align_y makes the box
 * try to resize this child element to the exact height of its parent
 * (respecting the maximum size hint on the child's height).
 *
 * @since_tizen 2.3
 *
 * @param[in]  o     The box object in question
 * @param[in]  priv  The smart data of the @a o
 * @param[in]  data  The data pointer passed on evas_object_box_layout_set(), if any
 */
EAPI void                       evas_object_box_layout_homogeneous_horizontal(Evas_Object *o, Evas_Object_Box_Data *priv, void *data) EINA_ARG_NONNULL(1, 2);

/**
 * @brief  Sets the box @a o to a <b>maximum size, homogeneous</b> horizontal box
 *
 * In a maximum size, homogeneous horizontal box, besides having cells
 * of <b>equal size</b> reserved for the child objects, this size is
 * defined by the size of the @b largest child in the box (in
 * width). The box's overall behavior is controlled by its properties,
 * which are set by the
 * <c>evas_object_box_{h,v}_{align,padding}_set()</c> family of
 * functions. The size hints of the elements in the box -- set by the
 * <c>evas_object_size_hint_{align,padding,weight}_set()</c> functions
 * -- also control the way this function works.
 *
 * @par Box's properties:
 * @c padding_h tells the box to draw empty spaces of that size, in
 * pixels, between the child objects' cells. @c align_h controls the
 * horizontal alignment of the child objects, relative to the
 * containing box. When set to @c 0.0, children are aligned to the
 * left. A value of @c 1.0 lets them aligned to the right
 * border. Values in between align them proportionally. A negative
 * value of @c align_h makes the box to @b justify its children
 * cells. The padding between them, in this case, is corrected so that
 * the leftmost one touches the left border and the rightmost one
 * touches the right border (even if they must overlap). The
 * @c align_v and @c padding_v properties of the box do not contribute to
 * its behaviour when this layout is chosen.
 *
 * @par Child element's properties:
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
 *
 * @since_tizen 2.3
 *
 * @param[in]  o     The box object in question
 * @param[in]  priv  The smart data of the @a o
 * @param[in]  data  The data pointer passed on evas_object_box_layout_set(), if any
 */
EAPI void                       evas_object_box_layout_homogeneous_max_size_horizontal(Evas_Object *o, Evas_Object_Box_Data *priv, void *data) EINA_ARG_NONNULL(1, 2);

/**
 * @brief   Sets the box @a o to a <b>maximum size, homogeneous</b> vertical box.
 *
 * @since_tizen 2.3
 *
 * @remarks This function behaves analogously to 
 *          evas_object_box_layout_homogeneous_max_size_horizontal(). The
 *          description of its behaviour can be derived from that function's
 *          documentation.
 *
 * @param[in]  o     The box object in question
 * @param[in]  priv  The smart data of the @a o
 * @param[in]  data  The data pointer passed on evas_object_box_layout_set(), if any
 */
EAPI void                       evas_object_box_layout_homogeneous_max_size_vertical(Evas_Object *o, Evas_Object_Box_Data *priv, void *data) EINA_ARG_NONNULL(1, 2);

/**
 * @brief  Sets the box @a o to a @b flow horizontal box.
 *
 * In a flow horizontal box, the box's child elements are placed in
 * @b rows (think of text as an analogy). A row has as much elements as
 * can fit into the box's width. The box's overall behavior is
 * controlled by its properties, which are set by the
 * <c>evas_object_box_{h,v}_{align,padding}_set()</c> family of
 * functions. The size hints of the elements in the box -- set by the
 * <c>evas_object_size_hint_{align,padding,weight}_set()</c> functions
 * -- also control the way this function works.
 *
 * @par Box's properties:
 * @c padding_h tells the box to draw empty spaces of that size, in
 * pixels, between the child objects' cells. @c align_h dictates the
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
 * @par Child element's properties:
 * @c padding_l and @c padding_r sum up to the required width of the
 * child element. The @c align_x property has no influence on the
 * layout. The child's @c padding_t and @c padding_b sum up to the
 * required height of the child element and is the only means (besides
 * row justifying) of setting space between rows. Note, however, that
 * @c align_y dictates positioning relative to the <b>largest
 * height</b> required by a child object in the actual row.
 *
 * @since_tizen 2.3
 *
 * @param[in]  o     The box object in question
 * @param[in]  priv  The smart data of the @a o
 * @param[in]  data  The data pointer passed on evas_object_box_layout_set(), if any
 */
EAPI void                       evas_object_box_layout_flow_horizontal(Evas_Object *o, Evas_Object_Box_Data *priv, void *data) EINA_ARG_NONNULL(1, 2);

/**
 * @brief   Sets the box @a o to a @b flow vertical box.
 *
 * @details This function behaves analogously to
 *          evas_object_box_layout_flow_horizontal(). The description of its
 *          behaviour can be derived from that function's documentation.
 *
 * @param[in]  o     The box object in question
 * @param[in]  priv  The smart data of the @a o
 * @param[in]  data  The data pointer passed on evas_object_box_layout_set(), if any
 *
 * @since_tizen 2.3
 *
 */
EAPI void                       evas_object_box_layout_flow_vertical(Evas_Object *o, Evas_Object_Box_Data *priv, void *data) EINA_ARG_NONNULL(1, 2);

/**
 * @brief  Sets the box @a o to a @b stacking box.
 *
 * In a stacking box, all children are given the same size -- the
 * box's own size -- and they are stacked one above the other, so
 * that the first object in @a o's internal list of child elements
 * are the bottommost in the stack.
 *
 * @par Box's properties:
 * No box properties are used.
 *
 * @par Child element's properties:
 * @c padding_l and @c padding_r sum up to the required width of the
 * child element. The @c align_x property tells the relative position
 * of this overall child width in its allocated cell (@c 0.0 to
 * extreme left, @c 1.0 to extreme right). A value of @c -1.0 to @c
 * align_x makes the box try to resize this child element to the exact
 * width of its cell (respecting the min and max hints on the child's
 * width and accounting for its horizontal padding properties). The
 * same applies to the vertical axis.
 *
 * @since_tizen 2.3
 *
 * @param[in]  o     The box object in question
 * @param[in]  priv  The smart data of the @a o
 * @param[in]  data  The data pointer passed on evas_object_box_layout_set(), if any
 */
EAPI void                       evas_object_box_layout_stack(Evas_Object *o, Evas_Object_Box_Data *priv, void *data) EINA_ARG_NONNULL(1, 2);

/**
 * @brief   Sets the alignment of the whole bounding box of contents, for a
 *          given box object.
 *
 * @details This influences how a box object is to align its bounding box
 *          of contents within its own area. The values @b must be in the range
 *          @c 0.0 - @c 1.0, or undefined behavior is expected. For horizontal
 *          alignment, @c 0.0 means to the left, with @c 1.0 meaning to the
 *          right. For vertical alignment, @c 0.0 means to the top, with @c 1.0
 *          meaning to the bottom.
 *
 * @since_tizen 2.3
 *
 * @remarks The default values for both alignments is @c 0.5.
 *
 * @param[in]   o           The given box object to set alignment from
 * @param[in]   horizontal  The horizontal alignment, in pixels
 * @param[in]   vertical    The vertical alignment, in pixels
 *
 * @see evas_object_box_align_get()
 */
EAPI void                       evas_object_box_align_set(Evas_Object *o, double horizontal, double vertical) EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the alignment of the whole bounding box of contents, for a
 *          given box object.
 *
 * @since_tizen 2.3
 *
 * @param[in]   o           The given box object to get alignment from
 * @param[out]   horizontal  The pointer to a variable where to store the
 *                      horizontal alignment
 * @param[out]   vertical    The pointer to a variable where to store the vertical alignment
 *
 * @see evas_object_box_align_set() for more information
 */
EAPI void                       evas_object_box_align_get(const Evas_Object *o, double *horizontal, double *vertical) EINA_ARG_NONNULL(1);

/**
 * @brief   Sets the (space) padding between cells set for a given box object.
 *
 * @since_tizen 2.3
 *
 * @remarks The default values for both padding components is @c 0.
 *
 * @param[in]   o           The given box object to set padding from
 * @param[in]   horizontal  The horizontal padding, in pixels
 * @param[in]   vertical    The vertical padding, in pixels
 *
 * @see evas_object_box_padding_get()
 */
EAPI void                       evas_object_box_padding_set(Evas_Object *o, Evas_Coord horizontal, Evas_Coord vertical) EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the (space) padding between cells set for a given box object.
 *
 * @since_tizen 2.3
 *
 * @param[in]   o           The given box object to get padding from
 * @param[out]   horizontal  The pointer to a variable where to store the
 *                      horizontal padding
 * @param[out]   vertical    The pointer to a variable where to store the vertical padding
 *
 * @see evas_object_box_padding_set()
 */
EAPI void                       evas_object_box_padding_get(const Evas_Object *o, Evas_Coord *horizontal, Evas_Coord *vertical) EINA_ARG_NONNULL(1);

/**
 * @brief   Appends a new @a child object to the given box object @a o.
 *
 * @since_tizen 2.3
 *
 * @remarks On success, the @c "child,added" smart event takes place.
 *
 * @remarks The actual placing of the item relative to the area of @a o
 *          depends on the layout set to it. For example, on horizontal layouts
 *          an item in the end of the box's list of children appear on its right.
 *
 * @remarks This call triggers the box's _Evas_Object_Box_Api::append
 *          smart function.
 *
 * @param[in]   o      The given box object
 * @param[in]   child  A child Evas object to be made a member of @a o
 * @return  A box option bound to the recently added box item, \n
 *          otherwise @c NULL on errors
 */
EAPI Evas_Object_Box_Option    *evas_object_box_append(Evas_Object *o, Evas_Object *child) EINA_ARG_NONNULL(1, 2);

/**
 * @brief   Prepends a new @a child object to the given box object @a o.
 *
 * @since_tizen 2.3
 *
 * @remarks On success, the @c "child,added" smart event takes place.
 *
 * @remarks The actual placing of the item relative to the area of @a o
 *          depends on the layout set to it. For example, on horizontal layouts
 *          an item in the beginning of the box's list of children appear
 *          on its left.
 *
 * @remarks This call triggers the box's
 *          _Evas_Object_Box_Api::prepend smart function.
 *
 * @param[in]   o      The given box object
 * @param[in]   child  A child Evas object to be made a member of @a o
 * @return  A box option bound to the recently added box item, \n 
 *          otherwise @c NULL on errors
 */
EAPI Evas_Object_Box_Option    *evas_object_box_prepend(Evas_Object *o, Evas_Object *child) EINA_ARG_NONNULL(1, 2);

/**
 * @brief  Inserts a new @a child object <b>before another existing one</b>, in
 *         a given box object @a o.
 *
 * @since_tizen 2.3
 *
 * @remarks On success, the @c "child,added" smart event takes place.
 *
 * @remarks This function fails if @a reference is not a member of @a o.
 *
 * @remarks The actual placing of the item relative to the area of @a o
 *          depends on the layout set to it.
 *
 * @remarks This call triggers the box's
 *          _Evas_Object_Box_Api::insert_before smart function.
 *
 * @param[in]   o          The given box object
 * @param[in]   child      A child Evas object to be made a member of @a o
 * @param[in]   reference  The child object to place this new one before
 * @return  A box option bound to the recently added box item, \n 
 *          otherwise @c NULL on errors
 */
EAPI Evas_Object_Box_Option    *evas_object_box_insert_before(Evas_Object *o, Evas_Object *child, const Evas_Object *reference) EINA_ARG_NONNULL(1, 2, 3);

/**
 * @brief   Inserts a new @a child object <b>after another existing one</b>, in
 *          a given box object @a o.
 *
 * @since_tizen 2.3
 *
 * @remarks On success, the @c "child,added" smart event takes place.
 *
 * @remarks This function fails if @a reference is not a member of @a o.
 *
 * @remarks The actual placing of the item relative to the area of @a o
 *          depends on the layout set to it.
 *
 * @remarks This call triggers the box's
 *          _Evas_Object_Box_Api::insert_after smart function.
 *
 * @param[in]   o          The given box object
 * @param[in]   child      A child Evas object to be made a member of @a o
 * @param[in]   reference  The child object to place this new one after
 * @return  A box option bound to the recently added box item, \n 
 *          otherwise @c NULL on errors
 */
EAPI Evas_Object_Box_Option    *evas_object_box_insert_after(Evas_Object *o, Evas_Object *child, const Evas_Object *reference) EINA_ARG_NONNULL(1, 2, 3);

/**
 * @brief   Inserts a new @a child object <b>at a given position</b>, in a given
 *          box object @a o.
 *
 * @since_tizen 2.3
 *
 * @remarks On success, the @c "child,added" smart event takes place.
 *
 * @remarks This function fails if the given position is invalid,
 *          given the internal list of elements of @a o.
 *
 * @remarks The actual placing of the item relative to the area of @a o
 *          depends on the layout set to it.
 *
 * @remarks This call triggers the box's
 *          _Evas_Object_Box_Api::insert_at smart function.
 *
 * @param[in]   o      The given box object
 * @param[in]   child  A child Evas object to be made a member of @a o
 * @param[in]   pos    The numeric position (starting from @c 0) to place the
 *                 new child object at
 * @return  A box option bound to the recently added box item, \n 
 *          otherwise @c NULL on errors
 */
EAPI Evas_Object_Box_Option    *evas_object_box_insert_at(Evas_Object *o, Evas_Object *child, unsigned int pos) EINA_ARG_NONNULL(1, 2);

/**
 * @brief   Removes a given object from a box object, unparenting it again.
 *
 * @since_tizen 2.3
 *
 * @remarks On removal, you get an unparented object again, just as it is
 *          before you inserted it in the box. The
 *          _Evas_Object_Box_Api::option_free box smart callback is called
 *          automatically for you and, also, the @c "child,removed" smart event
 *          takes place.
 *
 * @remarks This call triggers the box's _Evas_Object_Box_Api::remove
 *          smart function.
 *
 * @param[in]   o      The box object to remove a child object from
 * @param[in]   child  The handle to the child object to be removed
 * @return  #EINA_TRUE if the object is removed from the box object successfully, \n 
 *          otherwise #EINA_FALSE on failure
 */
EAPI Eina_Bool                  evas_object_box_remove(Evas_Object *o, Evas_Object *child) EINA_ARG_NONNULL(1, 2);

/**
 * @brief   Removes an object, <b>bound to a given position</b> in a box object,
 *          unparenting it again.
 *
 * @since_tizen 2.3
 *
 * @remarks On removal, you get an unparented object again, just as it is
 *          before you inserted it in the box. The @c option_free() box smart
 *          callback is called automatically for you and, also, the
 *          @c "child,removed" smart event takes place.
 *
 * @remarks This function fails if the given position is invalid,
 *          given the internal list of elements of @a o.
 *
 * @remarks This call triggers the box's
 *          _Evas_Object_Box_Api::remove_at smart function.
 *
 * @param[in]   o    The box object to remove a child object from
 * @param[in]   pos  The numeric position (starting from @c 0) of the child
 *               object to be removed
 * @return  #EINA_TRUE if the object is removed successfully, 
 *          otherwise #EINA_FALSE on failure
 */
EAPI Eina_Bool                  evas_object_box_remove_at(Evas_Object *o, unsigned int pos) EINA_ARG_NONNULL(1);

/**
 * @brief   Removes @b all child objects from a box object, unparenting them again.
 *
 * @since_tizen 2.3
 *
 * @remarks This has the same effect of calling evas_object_box_remove() on
 *          each of @a o's child objects, in sequence. If, and only if, all
 *          those calls succeed, so does this one.
 *
 * @param[in]   o      The box object to remove a child object from
 * @param[in]   clear  Set #EINA_TRUE to delete the just removed children, \n
 *                 otherwise set #EINA_FALSE to not delete the children
 * @return  #EINA_TRUE if the child objects are removed successfully, \n
 *          otherwise #EINA_FALSE on failure
 */
EAPI Eina_Bool                  evas_object_box_remove_all(Evas_Object *o, Eina_Bool clear) EINA_ARG_NONNULL(1);

/**
 * @brief   Gets an iterator to walk the list of children of a given box object.
 *
 * @since_tizen 2.3
 *
 * @remarks Do @b not remove or delete objects while walking the list.
 *
 * @param[in]   o  The box to retrieve an items iterator from
 * @return  An iterator on @a o's child objects, \n
 *          otherwise @c NULL on errors
 */
EAPI Eina_Iterator             *evas_object_box_iterator_new(const Evas_Object *o) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * @brief   Gets an accessor (a structure providing random items access) to the
 *          list of children of a given box object.
 *
 * @since_tizen 2.3
 *
 * @remarks Do not remove or delete objects while walking the list.
 *
 * @param[in]   o  The box to retrieve an items iterator from
 * @return  An accessor on @a o's child objects, \n 
 *          otherwise @c NULL on errors
 */
EAPI Eina_Accessor             *evas_object_box_accessor_new(const Evas_Object *o) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * @brief   Gets the list of children objects in a given box object.
 *
 * @since_tizen 2.3
 *
 * @remarks The returned list should be freed with @c eina_list_free() when you
 *          no longer need it.
 *
 * @remarks This is a duplicate of the list kept by the box internally.
 *          It is up to the user to destroy it when it no longer needs it.
 *          It is possible to remove objects from the box when walking
 *          this list, but these removals are not reflected on it.
 *
 * @param[in]   o  The box to retrieve an items list from
 * @return  A list of @a o's child objects, \n 
 *          otherwise @c NULL on errors or if it has no child objects
 */
EAPI Eina_List                 *evas_object_box_children_get(const Evas_Object *o) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * @brief   Gets the name of the property of the child elements of the box @a o
 *          which have @a id as identifier.
 *
 * @since_tizen 2.3
 *
 * @remarks This call does not do anything for a canonical Evas box. Only
 *          users which have @b subclassed it, setting custom box items options
 *          (see #Evas_Object_Box_Option) on it, would benefit from this
 *          function. They would have to implement it and set it to be the
 *          _Evas_Object_Box_Api::property_name_get smart class function of the
 *          box, which is originally set to @c NULL.
 *
 * @param[in]   o         The box to search child options from
 * @param[in]   property  The numerical identifier of the option being searched,
 *                    for its name
 * @return  The name of the given property, \n
 *          otherwise @c NULL on errors
 */
EAPI const char                *evas_object_box_option_property_name_get(const Evas_Object *o, int property) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the numerical identifier of the property of the child elements
 *          of the box @a o which have @a name as name string.
 *
 * @since_tizen 2.3
 *
 * @remarks This call does not do anything for a canonical Evas box. Only
 *          users which have @b subclassed it, setting custom box items options
 *          (see #Evas_Object_Box_Option) on it, would benefit from this
 *          function. They would have to implement it and set it to be the
 *          _Evas_Object_Box_Api::property_id_get smart class function of the
 *          box, which is originally set to @c NULL.
 *
 * @param[in]   o     The box to search child options from
 * @param[in]   name  The name string of the option being searched, for its ID
 * @return  The numerical ID of the given property, \n 
 *          otherwise @c -1 on errors
 */
EAPI int                        evas_object_box_option_property_id_get(const Evas_Object *o, const char *name) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1, 2);

/**
 * @brief   Sets a property value (by its given numerical identifier), on a
 *          given box child element
 *
 * @since_tizen 2.3
 *
 * @remarks This call does not do anything for a canonical Evas box. Only
 *          users which have @b subclassed it, setting custom box items options
 *          (see #Evas_Object_Box_Option) on it, would benefit from this
 *          function. They would have to implement it and set it to be the
 *          _Evas_Object_Box_Api::property_set smart class function of the box,
 *          which is originally set to @c NULL.
 *
 * @remarks This function internally creates a variable argument
 *          list, with the values passed after @a property, and call
 *          evas_object_box_option_property_vset() with this list and the same
 *          previous arguments.
 *
 * @param[in]   o         The box parenting the child element
 * @param[in]   opt       The box option structure bound to the child box element
 *                    to set a property on
 * @param[in]   property  The numerical ID of the given property
 * @param[in]   ...       (List of) actual value(s) to be set for this property \n
 *                    It (they) @b must be of the same type the user has
 *                    defined for it (them).
 * @return  #EINA_TRUE if the property value is set successfully, 
 *          otherwise #EINA_FALSE on failure
 */
EAPI Eina_Bool                  evas_object_box_option_property_set(Evas_Object *o, Evas_Object_Box_Option *opt, int property, ...) EINA_ARG_NONNULL(1, 2);

/**
 * @brief   Sets a property value (by its given numerical identifier), on a
 *          given box child element -- by a variable argument list
 *
 * @since_tizen 2.3
 *
 * @remarks This is a variable argument list variant of the
 *          evas_object_box_option_property_set(). See its documentation for
 *          more details.
 *
 * @param[in]   o         The box parenting the child element
 * @param[in]   opt       The box option structure bound to the child box element
 *                    to set a property on
 * @param[in]   property  The numerical ID of the given property
 * @param[in]   args      The variable argument list implementing the value to
 *                    be set for this property. It @b must be of the same type the user has
 *                    defined for it.
 * @return  #EINA_TRUE if teh property value is set successfully, \n
 *          otherwise #EINA_FALSE on failure
 */
EAPI Eina_Bool                  evas_object_box_option_property_vset(Evas_Object *o, Evas_Object_Box_Option *opt, int property, va_list args) EINA_ARG_NONNULL(1, 2);

/**
 * @brief   Gets a property's value (by its given numerical identifier), on a
 *          given box child element.
 *
 * @since_tizen 2.3
 *
 * @remarks This call does not do anything for a canonical Evas box. Only
 *          users which have @b subclassed it, getting custom box items options
 *          (see #Evas_Object_Box_Option) on it, would benefit from this
 *          function. They would have to implement it and get it to be the
 *          _Evas_Object_Box_Api::property_get smart class function of the
 *          box, which is originally get to @c NULL.
 *
 * @remarks This function internally creates a variable argument
 *          list, with the values passed after @a property, and call
 *          evas_object_box_option_property_vget() with this list and the same
 *          previous arguments.
 *
 * @param[in]   o         The box parenting the child element
 * @param[in]   opt       The box option structure bound to the child box element
 *                    to get a property from
 * @param[in]   property  The numerical ID of the given property
 * @param[in]   ...       (List of) pointer(s) where to store the value(s) set for this property, \n
 *                    It (they) @b must point to variable(s) of the same type the user 
 *                    has defined for it (them).
 * @return  #EINA_TRUE if the property values are obtained successfully, \n
 *          otherwise #EINA_FALSE on failure
 */
EAPI Eina_Bool                  evas_object_box_option_property_get(const Evas_Object *o, Evas_Object_Box_Option *opt, int property, ...) EINA_ARG_NONNULL(1, 2);

/**
 * @brief   Gets a property's value (by its given numerical identifier), on a
 *          given box child element -- by a variable argument list
 *
 * @since_tizen 2.3
 *
 * @remarks This is a variable argument list variant of the
 *          evas_object_box_option_property_get(). See its documentation for
 *          more details.
 *
 * @param[in]   o         The box parenting the child element
 * @param[in]   opt       The box option structure bound to the child box element
 *                    to get a property from
 * @param[in]   property  The numerical ID of the given property
 * @param[in]   args      The variable argument list with pointers to where to
 *                    store the values of this property \n 
 *                    They @b must point to variables 
 *                    of the same type the user has defined for them.
 * @return  #EINA_TRUE if the property values are obtained successfully, \n
 *          otherwise #EINA_FALSE on failure
 */
EAPI Eina_Bool                  evas_object_box_option_property_vget(const Evas_Object *o, Evas_Object_Box_Option *opt, int property, va_list args) EINA_ARG_NONNULL(1, 2);

/**
 * @}
 */

/**
 * @defgroup Evas_Object_Table Table Smart Object.
 * @ingroup Evas_Smart_Object_Group
 *
 * @brief   This group provides functions for table smart objects.
 *
 * @remarks Convenience smart object that packs children using a tabular
 *          layout using children size hints to define their size and
 *          alignment inside their cell space.
 *
 * @see @ref Evas_Object_Group_Size_Hints
 *
 * @{
 */

/**
 * @brief   Creates a new table.
 *
 * @since_tizen 2.3
 *
 * @param[in]   evas  The canvas in which table are added
 * @return  The new table object
 */
EAPI Evas_Object                       *evas_object_table_add(Evas *evas) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * @brief   Creates a table that is child of a given element @a parent.
 *
 * @since_tizen 2.3
 *
 * @param[in]   parent  The parent element
 * @return  The new table object
 *
 * @see evas_object_table_add()
 */
EAPI Evas_Object                       *evas_object_table_add_to(Evas_Object *parent) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * @brief Sets how this table should layout children.
 *
 * @todo consider aspect hint and respect it.
 *
 * @par EVAS_OBJECT_TABLE_HOMOGENEOUS_NONE
 * If table does not use homogeneous mode then columns and rows
 * are calculated based on hints of individual cells. This operation
 * mode is more flexible, but more complex and heavy to calculate as
 * well. @b Weight properties are handled as a boolean expand. Negative
 * alignment is considered as @c 0.5. This is the default.
 *
 * @todo @c EVAS_OBJECT_TABLE_HOMOGENEOUS_NONE should balance weight.
 *
 * @par EVAS_OBJECT_TABLE_HOMOGENEOUS_TABLE
 * When homogeneous is relative to table, the own table size is divided
 * equally among children, filling the whole table area. That is, if
 * table has @c WIDTH and @c COLUMNS, each cell gets <tt>WIDTH /
 * COLUMNS</tt> pixels. If children have minimum size that is larger
 * than this amount (including padding), then it overflows and is
 * aligned respecting the alignment hint, possible overlapping sibling
 * cells. @b Weight hint is used as a boolean, if greater than zero it
 * makes the child expand in that axis, taking as much space as
 * possible (bounded to maximum size hint). Negative alignment is
 * considered as @c 0.5.
 *
 * @par EVAS_OBJECT_TABLE_HOMOGENEOUS_ITEM
 * When homogeneous is relative to item, it means the greatest minimum
 * cell size is used. That is, if no element is set to expand,
 * the table has its contents to a minimum size, the bounding
 * box of all these children is aligned relatively to the table
 * object using evas_object_table_align_get(). If the table area is
 * too small to hold this minimum bounding box, then the objects
 * keep their size and the bounding box overflows the box area,
 * still respecting the alignment. @b Weight hint is used as a
 * boolean, if greater than zero it makes that cell expand in that
 * axis, toggling the <b>expand mode</b>, which makes the table behave
 * much like @b EVAS_OBJECT_TABLE_HOMOGENEOUS_TABLE, except that the
 * bounding box overflows and items do not overlap siblings. If
 * no minimum size is provided at all then the table fallsback to
 * expand mode as well.
 *
 * @since_tizen 2.3
 *
 * @param[in]  o            The table object 
 * @param[in]  homogeneous  The homogeneous mode
 */
EAPI void                               evas_object_table_homogeneous_set(Evas_Object *o, Evas_Object_Table_Homogeneous_Mode homogeneous) EINA_ARG_NONNULL(1);

/**
 * @brief  Gets the current layout homogeneous mode.
 *
 * @since_tizen 2.3
 *
 * @param[in]  o  The table object 
 * @return The homogeneous mode

 * @see evas_object_table_homogeneous_set()
 */
EAPI Evas_Object_Table_Homogeneous_Mode evas_object_table_homogeneous_get(const Evas_Object *o) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief  Sets padding between cells.
 *
 *  @since_tizen 2.3
 *
 * @param[in]  o           The table object 
 * @param[in]  horizontal  The horizontal padding to set
 * @param[in]  vertical    The vertical padding to set
 *
 */
EAPI void                               evas_object_table_padding_set(Evas_Object *o, Evas_Coord horizontal, Evas_Coord vertical) EINA_ARG_NONNULL(1);

/**
 * @brief  Gets padding between cells.
 *
 * @since_tizen 2.3
 *
 * @param[in]  o           The table object 
 * @param[out]  horizontal  The horizontal padding that is obtained
 * @param[out]  vertical    The vertical padding that is obtained
 */
EAPI void                               evas_object_table_padding_get(const Evas_Object *o, Evas_Coord *horizontal, Evas_Coord *vertical) EINA_ARG_NONNULL(1);

/**
 * @brief  Sets the alignment of the whole bounding box of contents.
 *
 * @since_tizen 2.3
 *
 * @param[in]  o           The table object 
 * @param[in]  horizontal  The horizontal alignment to set
 * @param[in]  vertical    The vertical alignment to set
 */
EAPI void                               evas_object_table_align_set(Evas_Object *o, double horizontal, double vertical) EINA_ARG_NONNULL(1);

/**
 * @brief  Gets the alignment of the whole bounding box of contents.
 *
 * @since_tizen 2.3
 *
 * @param[in]  o           The table object 
 * @param[out]  horizontal  The horizontal alignment that is obtained
 * @param[out]  vertical    The vertical alignment that is obtained
 */
EAPI void                               evas_object_table_align_get(const Evas_Object *o, double *horizontal, double *vertical) EINA_ARG_NONNULL(1);

/**
 * @brief   Sets the mirrored mode of the table. 
 * @since   1.1
 *
 * @since_tizen 2.3
 *
 * remarks  In mirrored mode the table items go from right to left instead of left to right. 
 *          That is, 1,1 is top right, not top left.
 *
 * @param[in]   o         The table object
 * @param[in]   mirrored  The mirrored mode to set
 */
EAPI void                               evas_object_table_mirrored_set(Evas_Object *o, Eina_Bool mirrored) EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the mirrored mode of the table.
 * @since   1.1
 *
 * @since_tizen 2.3
 *
 * @param[in]   o  The table object
 * @return  #EINA_TRUE if it is a mirrored table, \n
 *          otherwise #EINA_FALSE if it is not a mirrored table
 * @see evas_object_table_mirrored_set()
 */
EAPI Eina_Bool                          evas_object_table_mirrored_get(const Evas_Object *o) EINA_ARG_NONNULL(1);

/**
 * @brief   Gets packing location of a child of table.
 * @since   1.1
 *
 * @since_tizen 2.3
 *
 * @param[in]   o        The given table object
 * @param[in]   child    The child object to add
 * @param[out]   col      The pointer to store relative-horizontal position to place child
 * @param[out]   row      The pointer to store relative-vertical position to place child
 * @param[out]   colspan  The pointer to store how many relative-horizontal position to use for this child
 * @param[out]   rowspan  The pointer to store how many relative-vertical position to use for this child
 *
 * @return  #EINA_TRUE if the packing location is obtained successfully, \n
 *          otherwise #EINA_FALSE on failure
 */
EAPI Eina_Bool                          evas_object_table_pack_get(const Evas_Object *o, Evas_Object *child, unsigned short *col, unsigned short *row, unsigned short *colspan, unsigned short *rowspan);

/**
 * @brief   Adds a new child to a table object or set its current packing.
 *
 * @since_tizen 2.3
 *
 * @param[in]   o        The given table object
 * @param[in]   child    The child object to add
 * @param[in]   col      The relative-horizontal position to place the child
 * @param[in]   row      The relative-vertical position to place the child
 * @param[in]   colspan  The number of relative-horizontal position to use for this child
 * @param[in]   rowspan  The number of relative-vertical position to use for this child
 * @return  #EINA_TRUE if the new child is added successfully, \n
 *          otherwise #EINA_FALSE on failure
 */
EAPI Eina_Bool                          evas_object_table_pack(Evas_Object *o, Evas_Object *child, unsigned short col, unsigned short row, unsigned short colspan, unsigned short rowspan) EINA_ARG_NONNULL(1, 2);

/**
 * @brief   Removes child from table.
 *
 * @since_tizen 2.3
 *
 * @remarks Removing a child immediately calls a walk over children in order
 *          to recalculate the numbers of columns and rows. If you plan to remove
 *          all children, use evas_object_table_clear() instead.
 *
 * @param[in]   o      The given table object
 * @param[in]   child    The child object to remove
 * @return  #EINA_TRUE if the child is removed successfully, \n
 *          otherwise #EINA_FALSE on failure
 */
EAPI Eina_Bool                          evas_object_table_unpack(Evas_Object *o, Evas_Object *child) EINA_ARG_NONNULL(1, 2);

/**
 * @brief   Removes all child objects from a table object.
 *
 * @since_tizen 2.3
 *
 * @remarks This is a faster way to remove the child objects.
 *
 * @param[in]   o      The given table object
 * @param[in]   clear  Set #EINA_TRUE to delete the just removed children, \n
 *                 otherwise set #EINA_FALSE to not delete the removed children
 */
EAPI void                               evas_object_table_clear(Evas_Object *o, Eina_Bool clear) EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the number of columns and rows this table takes.
 *
 * @since_tizen 2.3
 *
 * @remarks The columns and rows are virtual entities, one can specify a table
 *          with a single object that takes 4 columns and 5 rows. The only
 *          difference for a single cell table is that paddings are
 *          accounted proportionally.
 *
 * @param[in]   o     The table object 
 * @param[out]   cols  The number of columns
 * @param[out]   rows  The number of rows
 */
EAPI void                               evas_object_table_col_row_size_get(const Evas_Object *o, int *cols, int *rows) EINA_ARG_NONNULL(1);

/**
 * @brief    Gets an iterator to walk through the list of children for the table.
 *
 * @since_tizen 2.3
 *
 * @remarks  Do not remove or delete objects while walking the list.
 *
 * @param[in]    o  The table object 
 * @return   An iterator to walk through the children of the object, \n
 *           otherwise @c NULL in case of errors
 */
EAPI Eina_Iterator                     *evas_object_table_iterator_new(const Evas_Object *o) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * @brief    Gets an accessor to get random access to the list of children for the table.
 *
 * @since_tizen 2.3
 *
 * @remarks  Do not remove or delete objects while walking the list.
 *
 * @param[in]    o  The table object 
 * @return   An accessor on @a o's child objects, \n
 *           otherwise @c NULL in case of errors
 */
EAPI Eina_Accessor                     *evas_object_table_accessor_new(const Evas_Object *o) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * @brief   Gets the list of children for the table.
 *
 * @since_tizen 2.3
 *
 * @remarks This is a duplicate of the list kept by the table internally.
 *          It is up to the user to destroy it when it no longer needs it.
 *          It is possible to remove objects from the table when walking this
 *          list, but these removals are not reflected on it.
 *
 * @param[in]   o  The table object 
 * @return  The list of the children, \n
 *          otherwise @c NULL in case of errors
 */
EAPI Eina_List                         *evas_object_table_children_get(const Evas_Object *o) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * @brief    Gets the child of the table at the given coordinates.
 *
 * @since_tizen 2.3
 *
 * @remarks  This does not take into account col or row spanning.
 *
 * @param[in]    o    The table object 
 * @param[in]    col  The number of columns
 * @param[in]    row  The number of rows
 * @return   A child of the table object 
 */
EAPI Evas_Object                       *evas_object_table_child_get(const Evas_Object *o, unsigned short col, unsigned short row) EINA_ARG_NONNULL(1);

/**
 * @}
 */

/**
 * @defgroup Evas_Object_Grid Grid Smart Object.
 * @ingroup Evas_Smart_Object_Group
 *
 * @brief   This group provides functions for grid smart objects.
 * @since   1.1
 *
 * @remarks Convenience smart object that packs children under a regular grid
 *          layout, using their virtual grid location and size to determine
 *          children's positions inside the grid object's area.
 *
 * @{
 */

/**
 * @brief    Creates a new grid.
 *
 * @since    1.1
 *
 * @since_tizen 2.3
 *
 * @remarks  It is set to a virtual size of 1x1 by default and add children with
 *           evas_object_grid_pack().
 *
 * @param[in]    evas  The given evas
 * @return   The new grid object
 */
EAPI Evas_Object   *evas_object_grid_add(Evas *evas) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * @brief   Creates a grid that is child of a given element @a parent.
 * @since   1.1
 *
 * @since_tizen 2.3
 *
 * @param[in] parent The parent element
 * @return  The new grid object
 *
 * @see evas_object_grid_add()
 */
EAPI Evas_Object   *evas_object_grid_add_to(Evas_Object *parent) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * @brief   Sets the virtual resolution for the grid.
 * @since   1.1
 *
 * @since_tizen 2.3
 *
 * @param[in]   o  The grid object to modify
 * @param[in]   w  The virtual horizontal size (resolution) in integer units
 * @param[in]   h  The virtual vertical size (resolution) in integer units
 */
EAPI void           evas_object_grid_size_set(Evas_Object *o, int w, int h) EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the current virtual resolution.
 * @since   1.1
 *
 * @since_tizen 2.3
 *
 * @param[in]   o  The grid object to query
 * @param[out]   w  A pointer to an integer to store the virtual width
 * @param[out]   h  A pointer to an integer to store the virtual height
 * @see evas_object_grid_size_set()
 */
EAPI void           evas_object_grid_size_get(const Evas_Object *o, int *w, int *h) EINA_ARG_NONNULL(1);

/**
 * @brief   Sets the mirrored mode of the grid. 
 * @since   1.1
 *
 * @since_tizen 2.3
 *
 * @remarks In mirrored mode the grid items go from right to left 
 *          instead of left to right. That is, 0,0 is top right, not to left.
 *
 * @param[in]   o         The grid object
 * @param[in]   mirrored  The mirrored mode to set
 */
EAPI void           evas_object_grid_mirrored_set(Evas_Object *o, Eina_Bool mirrored) EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the mirrored mode of the grid.
 * @since   1.1
 *
 * @since_tizen 2.3
 *
 * @param[in]   o  The grid object
 * @return  #EINA_TRUE if it is a mirrored grid, \n
 *          otherwise #EINA_FALSE if it is not a mirrored grid 
 *
 * @see evas_object_grid_mirrored_set()
 */
EAPI Eina_Bool      evas_object_grid_mirrored_get(const Evas_Object *o) EINA_ARG_NONNULL(1);

/**
 * @brief   Adds a new child to a grid object.
 * @since   1.1
 *
 * @since_tizen 2.3
 *
 * @param[in]   o      The given grid object
 * @param[in]   child  The child object to add
 * @param[in]   x      The virtual x coordinate of the child
 * @param[in]   y      The virtual y coordinate of the child
 * @param[in]   w      The virtual width of the child
 * @param[in]   h      The virtual height of the child
 * @return  #EINA_TRUE if the child is added successfully, \n
 *          otherwise #EINA_FALSE on failure
 */
EAPI Eina_Bool      evas_object_grid_pack(Evas_Object *o, Evas_Object *child, int x, int y, int w, int h) EINA_ARG_NONNULL(1, 2);

/**
 * @brief   Removes child from grid.
 * @since   1.1
 *
 * @since_tizen 2.3
 *
 * @remarks Removing a child immediately calls a walk over children in order
 *          to recalculate numbers of columns and rows. If you plan to remove
 *          all children, use evas_object_grid_clear() instead.
 *
 * @param[in]   o      The given grid object
 * @param[in]   child  The child object to remove
 * @return  #EINA_TRUE if the child is removed successfully, \n
 *          otherwise #EINA_FALSE on failure
 */
EAPI Eina_Bool      evas_object_grid_unpack(Evas_Object *o, Evas_Object *child) EINA_ARG_NONNULL(1, 2);

/**
 * @brief   Removes all child objects from a grid object.
 * @since   1.1
 *
 * @since_tizen 2.3
 *
 * @remarks This is a faster way to remove all child objects.
 *
 * @param[in]   o       The given grid object
 * @param[in]   clear   Set #EINA_TRUE to delete the just removed children, \n
 *                  otherwise set #EINA_FALSE to not delete them
 */
EAPI void           evas_object_grid_clear(Evas_Object *o, Eina_Bool clear) EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the pack options for a grid child.
 * @since   1.1
 *
 * @since_tizen 2.3
 *
 * @remarks Gets the pack x, y, width and height in virtual coordinates set by
 *          evas_object_grid_pack().
 *
 * @param[in]   o      The grid object
 * @param[in]   child  The grid child to query for coordinates
 * @param[out]   x      The pointer to where the x coordinate to be returned
 * @param[out]   y      The pointer to where the y coordinate to be returned
 * @param[out]   w      The pointer to where the width to be returned
 * @param[out]   h      The pointer to where the height to be returned
 * @return  #EINA_TRUE if the pack options are obtained successfully,
 *          otherwise #EINA_FALSE on failure
 */
EAPI Eina_Bool      evas_object_grid_pack_get(const Evas_Object *o, Evas_Object *child, int *x, int *y, int *w, int *h);

/**
 * @brief    Gets an iterator to walk the list of children for the grid.
 * @since    1.1
 *
 * @since_tizen 2.3
 *
 * @remarks  Do not remove or delete objects while walking the list.
 * 
 * @param[in]    o  The grid object 
 * @return   An iterator to walk through the children of the object, \n
 *           otherwise @c NULL in case of errors
 */
EAPI Eina_Iterator *evas_object_grid_iterator_new(const Evas_Object *o) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * @brief   Gets an accessor to get random access to the list of children for the grid.
 * @since   1.1
 *
 * @since_tizen 2.3
 *
 * @remarks Do not remove or delete objects while walking the list.
 *
 * @param[in]   o  The grid object 
 * @return  An accessor to get random access to the list of children for the grid, \n
 *           otherwise @c NULL in case of errors 
 */
EAPI Eina_Accessor *evas_object_grid_accessor_new(const Evas_Object *o) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * @brief   Gets the list of children for the grid.
 * @since   1.1
 *
 * @since_tizen 2.3
 *
 * @remarks This is a duplicate of the list kept by the grid internally.
 *          It is up to the user to destroy it when it no longer needs it.
 *          It is possible to remove objects from the grid when walking this
 *          list, but these removals are not reflected on it.
 *
 * @param[in]    o  The grid object 
 * @return   The list of children for the grid
 */
EAPI Eina_List     *evas_object_grid_children_get(const Evas_Object *o) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * @}
 */

/**
 * @internal
 * @defgroup Evas_Cserve Shared Image Cache Server
 * @ingroup Evas
 *
 * @brief  This group provides functions for shared image cache server
 *
 * Evas has an (optional) module which provides client-server
 * infrastructure to <b>share bitmaps across multiple processes</b>,
 * saving data and processing power.
 *
 * Be warned that it @b does not work when <b>threaded image
 * preloading</b> is enabled for Evas, though.
 *
 * @{
 */
typedef struct _Evas_Cserve_Stats       Evas_Cserve_Stats;
typedef struct _Evas_Cserve_Image_Cache Evas_Cserve_Image_Cache;
typedef struct _Evas_Cserve_Image       Evas_Cserve_Image;
typedef struct _Evas_Cserve_Config      Evas_Cserve_Config;

/**
 * @brief   The structure type containing the statistics about the server that shares cached bitmaps.
 */
struct _Evas_Cserve_Stats
{
   int    saved_memory;      /**< The current amount of saved memory, in bytes */
   int    wasted_memory;      /**< The current amount of wasted memory, in bytes */
   int    saved_memory_peak;      /**< The peak amount of saved memory, in bytes */
   int    wasted_memory_peak;      /**< The peak amount of wasted memory, in bytes */
   double saved_time_image_header_load;      /**< The time, in seconds, saved in header loads by sharing cached loads instead */
   double saved_time_image_data_load;      /**< The time, in seconds, saved in data loads by sharing cached loads instead */
};

/**
 * @brief   The structure type containing a handle of a cache of images shared by a server.
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
 * @brief   The structure type containing a handle to an image shared by a server.
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
 * @brief   The structure type containing the configuration that controls the server that shares cached bitmaps.
 */
struct _Evas_Cserve_Config
{
   int cache_max_usage;
   int cache_item_timeout;
   int cache_item_timeout_check;
};

/**
 * @brief   Checks whether the system shares bitmaps using the server.
 *
 * @since_tizen 2.3
 *
 * @return  #EINA_TRUE if the system shares bitmaps using the server, \n
 *          otherwise #EINA_FALSE if the system does not share bitmaps
 */
EAPI Eina_Bool   evas_cserve_want_get(void) EINA_WARN_UNUSED_RESULT;

/**
 * @brief   Checks whether the system is connected to the server used to share bitmaps.
 *
 * @since_tizen 2.3
 *
 * @return  #EINA_TRUE if the system is connected to the server, \n
 *          otherwise #EINA_FALSE if it is not connected
 */
EAPI Eina_Bool   evas_cserve_connected_get(void) EINA_WARN_UNUSED_RESULT;

/**
 * @brief   Gets the statistics from a running bitmap sharing server.
 *
 * @since_tizen 2.3
 *
 * @param[in]   stats  The pointer to the structure to fill the statistics about the
 *                 bitmap cache server
 * @return  #EINA_TRUE if @a stats is filled with data, \n
 *          otherwise #EINA_FALSE when @a stats is untouched
 */
EAPI Eina_Bool   evas_cserve_stats_get(Evas_Cserve_Stats *stats) EINA_WARN_UNUSED_RESULT;

/**
 * @brief  Completely discards or cleans a given images cache, thus re-setting it.
 *
 * @since_tizen 2.3
 *
 * @param[in]  cache  A handle to the given images cache
 */
EAPI void        evas_cserve_image_cache_contents_clean(Evas_Cserve_Image_Cache *cache);

/**
 * @brief   Gets the current configuration of the Evas image caching server.
 *
 * @since_tizen 2.3
 *
 * @remarks  The fields of @a config are altered to reflect the current
 *           configuration's values.
 *
 * @param[in]    config  The current image caching server's configuration
 *
 * @return   #EINA_TRUE if @a config is filled with data, \n
 *           otherwise #EINA_FALSE when @a config is untouched
 *
 * @see evas_cserve_config_set()
 */
EAPI Eina_Bool   evas_cserve_config_get(Evas_Cserve_Config *config) EINA_WARN_UNUSED_RESULT;

/**
 * @brief   Sets the configurations of the Evas image caching server.
 *
 * @since_tizen 2.3
 *
 * @param[in]   config  A bitmap cache configuration handle with fields set
 *                  to desired configuration values
 * @return  #EINA_TRUE if @a config is applied successfully,
 *          otherwise #EINA_FALSE
 *
 * @see evas_cserve_config_get()
 */
EAPI Eina_Bool   evas_cserve_config_set(const Evas_Cserve_Config *config) EINA_WARN_UNUSED_RESULT;

/**
 * @brief  Forces the system to disconnect from the bitmap caching server.
 *
 * @since_tizen 2.3
 */
EAPI void        evas_cserve_disconnect(void);

/**
 * @}
 */

/**
 * @defgroup Evas_Utils General Utilities
 * @ingroup Evas
 * @brief   This group provides general utilities functions.
 * @remarks Some functions that are handy but are not specific to canvas or objects.
 *
 * @{
 */

/**
 * @brief   Converts the given Evas image load error code into a string
 *          describing it in English.
 *
 * @since_tizen 2.3
 *
 * @remarks Mostly evas_object_image_file_set() would be the function setting
 *          that error value afterwards, but also evas_object_image_load(),
 *          evas_object_image_save(), evas_object_image_data_get(),
 *          evas_object_image_data_convert(), evas_object_image_pixels_import()
 *          and evas_object_image_is_inside(). This function is meant to be
 *          used in conjunction with evas_object_image_load_error_get(), as in:
 *
 * @remarks The following is an example code:
 * @skip img1 =
 * @until ecore_main_loop_begin(
 *
 * @remarks Here, @c valid_path is the path to a valid image and @c
 *          bogus_path is a path to a file which does not exist. The two outputs
 *          of evas_load_error_str() would be (if no other errors occur):
 *          <code>"No error on load"</code> and <code>"File (or file path) does
 *          not exist"</code>, respectively.
 *
 *
 * @param[in]   error  The error code \n
 *                 A value in #Evas_Load_Error.
 * @return  A valid string \n
 *          If the given @a error is not supported, <code>"Unknown error"</code> is returned.
 */
EAPI const char *evas_load_error_str(Evas_Load_Error error);

/* Evas utility routines for color space conversions */
/* hsv color space has h in the range 0.0 to 360.0, and s,v in the range 0.0 to 1.0 */
/* rgb color space has r,g,b in the range 0 to 255 */

/**
 * @brief   Converts a given color from HSV to RGB format.
 *
 * @details This function converts a given color in HSV color format to RGB
 *          color format.
 *
 * @since_tizen 2.3
 *
 * @param[in]   h  The Hue component of the color
 * @param[in]   s  The Saturation component of the color
 * @param[in]   v  The Value component of the color
 * @param[out]   r  The Red component of the color
 * @param[out]   g  The Green component of the color
 * @param[out]   b  The Blue component of the color
 **/
EAPI void evas_color_hsv_to_rgb(float h, float s, float v, int *r, int *g, int *b);

/**
 * @brief   Converts a given color from RGB to HSV format.
 *
 * @details This function converts a given color in RGB color format to HSV
 *          color format.
 *
 * @since_tizen 2.3
 *
 * @param[in]   r  The Red component of the color
 * @param[in]   g  The Green component of the color
 * @param[in]   b  The Blue component of the color
 * @param[out]   h  The Hue component of the color
 * @param[out]   s  The Saturation component of the color
 * @param[out]   v  The Value component of the color
 **/
EAPI void evas_color_rgb_to_hsv(int r, int g, int b, float *h, float *s, float *v);

/* argb color space has a,r,g,b in the range 0 to 255 */

/**
 * @brief   Pre-multiplies a rgb triplet by an alpha factor.
 * @details This function pre-multiplies a given rgb triplet by an alpha
 *          factor. Alpha factor is used to define transparency.
 *
 * @since_tizen 2.3
 *
 * @param[in]   a  The alpha factor
 * @param[out]   r  The Red component of the color
 * @param[out]   g  The Green component of the color
 * @param[out]   b  The Blue component of the color
 **/
EAPI void evas_color_argb_premul(int a, int *r, int *g, int *b);

/**
 * @brief   Undoes pre-multiplication of a rgb triplet by an alpha factor.
 *
 * @details This function undoes pre-multiplication a given rbg triplet by an
 *          alpha factor. Alpha factor is used to define transparency.
 *
 * @since_tizen 2.3
 *
 * @param[in]   a  The alpha factor
 * @param[out]   r  The Red component of the color
 * @param[out]   g  The Green component of the color
 * @param[out]   b  The Blue component of the color
 *
 * @see evas_color_argb_premul().
 **/
EAPI void evas_color_argb_unpremul(int a, int *r, int *g, int *b);

/**
 * @brief   Pre-multiplies data by an alpha factor.
 * @details This function pre-multiplies a given data by an alpha
 *          factor. Alpha factor is used to define transparency.
 *
 * @since_tizen 2.3
 *
 * @param[in]   data  The data value
 * @param[in]   len   The length value
 **/
EAPI void evas_data_argb_premul(unsigned int *data, unsigned int len);

/**
 * @brief   Undoes pre-multiplication data by an alpha factor.
 *
 * @details This function undoes the pre-multiplication of a given data by an alpha
 *          factor. Alpha factor is used to define transparency.
 *
 * @since_tizen 2.3
 *
 * @param[in]   data  The data value
 * @param[in]   len   The length value
 **/
EAPI void evas_data_argb_unpremul(unsigned int *data, unsigned int len);

/**
 * @internal
 * @remarks Tizen only feature
 * @remarks TIZEN_ONLY(20140822): Added evas_bidi_direction_hint_set, get APIs and applied to textblock, text.
 * @brief   Set BiDi direction hint to the given evas canvas.
 *
 * @details Evas has a BiDi direction hint value which affect to textblock, text object.
 *          If a text that has only neutral BiDi direction is set to textblock or text,
 *          the BiDi direction hint will be used to decide alignment of paragraphs.
 *
 * @since_tizen 2.3
 *
 * @param[in]  e     The pointer to the Evas canvas
 * @param[in]  dir   The BiDi direction hint
 **/
EAPI void evas_bidi_direction_hint_set(Evas *e, Evas_BiDi_Direction dir);

/**
 * @internal
 * @remarks Tizen only feature
 * @remarks TIZEN_ONLY(20140822): Added evas_bidi_direction_hint_set, get APIs and applied to textblock, text.
 * @brief   Get BiDi direction hint of the given evas canvas.
 *
 * @details Evas has a BiDi direction hint value which affect to textblock, text object.
 *          If a text that has only neutral BiDi direction is set to textblock or text,
 *          the BiDi direction hint will be used to decide alignment of paragraphs.
 *
 * @since_tizen 2.3
 *
 * @param[in]  e     The pointer to the Evas canvas
 * @return     BiDi direction hint
 *
 * @see evas_bidi_direction_hint_set().
 **/
EAPI Evas_BiDi_Direction evas_bidi_direction_hint_get(Evas *e);


/* string and font handling */

/**
 * @brief   Gets the next character in the string.
 *
 * @remarks Given the UTF-8 string in @a str, and starting byte position in @a pos,
 *          this function places in @a decoded the decoded code point at @a pos
 *          and return the byte index for the next character in the string.
 *          The only boundary check done is that @a pos must be >= 0. Other than that,
 *          no checks are performed, so passing an index value that is not within the
 *          length of the string results in undefined behavior.
 *
 * @since_tizen 2.3
 *
 * @param[in]   str      The UTF-8 string
 * @param[in]   pos      The byte index where to start
 * @param[out]   decoded  The address to store the decoded code point \n
 *                   This is optional.
 * @return  The byte index of the next character
 */
EAPI int  evas_string_char_next_get(const char *str, int pos, int *decoded) EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the previous character in the string
 *
 * @since_tizen 2.3
 *
 * @remarks Given the UTF-8 string in @a str, and starting byte position in @a pos,
 *          this function places in @a decoded the decoded code point at @a pos
 *          and return the byte index for the previous character in the string.
 *
 * @remarks The only boundary check done is that @a pos must be >= 1. Other than that,
 *          no checks are performed, so passing an index value that is not within the
 *          length of the string results in undefined behavior.
 *
 * @param[in]   str      The UTF-8 string
 * @param[in]   pos      The byte index where to start
 * @param[out]   decoded  The address where to store the decoded code point \n 
 *                   This is optional.
 * @return  The byte index of the previous character
 */
EAPI int  evas_string_char_prev_get(const char *str, int pos, int *decoded) EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the length in characters of the string.
 *
 * @since_tizen 2.3
 *
 * @param[in]   str  The string to get the length of
 * @return  The length in characters (not bytes)
 */
EAPI int  evas_string_char_len_get(const char *str) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @}
 */

/**
 * @defgroup Evas_Keys Key Input Functions
 * @ingroup Evas_Canvas
 *
 * @brief  This group provides functions which feed key events to the canvas.
 *
 * Evas is @b not aware of input
 * systems at all. Then, the user, if using it crudely (evas_new()),
 * has to feed it with input events, so that it can react
 * somehow. If, however, the user creates a canvas by means of the
 * Ecore_Evas wrapper, it automatically binds the chosen display
 * engine's input events to the canvas, for you.
 *
 * This group presents the functions dealing with the feeding of key
 * events to the canvas. On most of them, one has to reference a given
 * key by a name (<code>keyname</code> argument). Those are
 * <b>platform dependent</b> symbolic names for the keys. Sometimes
 * you get the right <code>keyname</code> by simply using an ASCII
 * value of the key name, but it is not like that always.
 *
 * Typical platforms are Linux frame buffer (Ecore_FB) and X server
 * (Ecore_X) when using Evas with Ecore and Ecore_Evas. Please refer
 * to your display engine's documentation when using evas through an
 * Ecore helper wrapper when you need the <code>keyname</code>s.
 *
 * @{
 */

/**
 * @brief   Gets a handle to the list of modifier keys registered in the canvas @a e. 
 *
 * @since_tizen 2.3
 *
 * @remarks This is required to check for which modifiers are set
 *          at a given time with the evas_key_modifier_is_set() function.
 *
 * @param[in]   e  The pointer to the Evas canvas
 * @return  An Evas_Modifier handle to query Evas' keys subsystem with evas_key_modifier_is_set(), \n
 *          otherwise @c NULL on error
 *
 * @see evas_key_modifier_add
 * @see evas_key_modifier_del
 * @see evas_key_modifier_on
 * @see evas_key_modifier_off
 * @see evas_key_modifier_is_set
 */
EAPI const Evas_Modifier *evas_key_modifier_get(const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief   Gets a handle to the list of lock keys registered in the canvas @a e. 
 *
 * @since_tizen 2.3
 *
 * @remarks This is required to check for which locks are set at a given
 *          time with the evas_key_lock_is_set() function.
 *
 * @param[in]   e  The pointer to the Evas canvas
 * @return  An Evas_Lock handle to query Evas' keys subsystem with evas_key_lock_is_set(), \n
 *          otherwise @c NULL on error
 *
 * @see evas_key_lock_add
 * @see evas_key_lock_del
 * @see evas_key_lock_on
 * @see evas_key_lock_off
 * @see evas_key_lock_is_set
 *
 */
EAPI const Evas_Lock     *evas_key_lock_get(const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief   Checks whether a given modifier key is set at the time of the call.
 *
 * @since_tizen 2.3
 *
 * @remarks If the modifier is set, such as shift being pressed, this
 *          function returns @c Eina_True.
 *
 * @param[in]   m        The current modifiers set, as returned by evas_key_modifier_get()
 * @param[in]   keyname  The name of the modifier key to check status for
 * @return  #EINA_TRUE if the modifier key named @a keyname is on, \n
 *          otherwise #EINA_FALSE if the modifier key is not on 
 *
 * @see evas_key_modifier_add
 * @see evas_key_modifier_del
 * @see evas_key_modifier_get
 * @see evas_key_modifier_on
 * @see evas_key_modifier_off
 */
EAPI Eina_Bool            evas_key_modifier_is_set(const Evas_Modifier *m, const char *keyname) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1, 2);

/**
 * @brief   Checks whether the given lock key is set at the time of the call. 
 *
 * @since_tizen 2.3
 *
 * @remarks If the lock is set, such as caps lock, this function returns @c Eina_True.
 *
 * @param[in]   l        The current locks set, as returned by evas_key_lock_get()
 * @param[in]   keyname  The name of the lock key to check status for
 * @return  #EINA_TRUE if the @a keyname lock key is set, \n
 *          otherwise @c Eina_False if the lock key is not set
 *
 * @see evas_key_lock_get
 * @see evas_key_lock_add
 * @see evas_key_lock_del
 * @see evas_key_lock_on
 * @see evas_key_lock_off
 */
EAPI Eina_Bool            evas_key_lock_is_set(const Evas_Lock *l, const char *keyname) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1, 2);

/**
 * @brief   Adds the @a keyname key to the current list of modifier keys.
 *
 * @since_tizen 2.3
 *
 * @remarks Modifiers are keys like shift, alt and ctrl, that is, keys which are
 *          meant to be pressed together with others, altering the behavior of
 *          the secondly pressed keys somehow. Evas allows these keys to be
 *          user defined.
 *
 * @remarks This call allows custom modifiers to be added to the Evas system at
 *          run time. It is then possible to set and unset modifier keys
 *          programmatically for other parts of the program to check and act
 *          on. Programmers using Evas would check for modifier keys on key
 *          event callbacks using evas_key_modifier_is_set().
 *
 * @remarks If you instantiate the canvas by means of the
 *          ecore_evas_new() family of helper functions, Ecore takes
 *          care of registering on it all standard modifiers: "Shift",
 *          "Control", "Alt", "Meta", "Hyper", "Super".
 *
 * @param[in]   e        The pointer to the Evas canvas
 * @param[in]   keyname  The name of the modifier key to add to the list of
 *                   Evas modifiers
 *
 * @see evas_key_modifier_del
 * @see evas_key_modifier_get
 * @see evas_key_modifier_on
 * @see evas_key_modifier_off
 * @see evas_key_modifier_is_set
 *
 */
EAPI void                 evas_key_modifier_add(Evas *e, const char *keyname) EINA_ARG_NONNULL(1, 2);

/**
 * @brief   Removes the @a keyname key from the current list of modifier keys
 *          on canvas @a e.
 *
 * @since_tizen 2.3
 *
 * @param[in]   e        The pointer to the Evas canvas
 * @param[in]   keyname  The name of the key to remove from the modifiers list
 *
 * @see evas_key_modifier_add
 * @see evas_key_modifier_get
 * @see evas_key_modifier_on
 * @see evas_key_modifier_off
 * @see evas_key_modifier_is_set
 */
EAPI void                 evas_key_modifier_del(Evas *e, const char *keyname) EINA_ARG_NONNULL(1, 2);

/**
 * @brief   Adds the @a keyname key to the current list of lock keys.
 *
 * @since_tizen 2.3
 *
 * @remarks Locks are keys like caps lock, num lock or scroll lock, that is, keys
 *          which are meant to be pressed once -- toggling a binary state which
 *          is bound to it -- and thus altering the behavior of all
 *          subsequently pressed keys somehow, depending on its state. Evas is
 *          so that these keys can be defined by the user.
 *
 * @remarks This allows custom locks to be added to the evas system at run
 *          time. It is then possible to set and unset lock keys
 *          programmatically for other parts of the program to check and act
 *          on. Programmers using Evas would check for lock keys on key event
 *          callbacks using evas_key_lock_is_set().
 *
 * @remarks If you instantiate the canvas by means of the
 *          ecore_evas_new() family of helper functions, Ecore takes
 *          care of registering on it all standard lock keys: "Caps_Lock",
 *          "Num_Lock", "Scroll_Lock".
 *
 * @param[in]   e        The pointer to the Evas canvas
 * @param[in]   keyname  The name of the key to add to the locks list
 *
 * @see evas_key_lock_get
 * @see evas_key_lock_del
 * @see evas_key_lock_on
 * @see evas_key_lock_off
 * @see evas_key_lock_is_set
 */
EAPI void                 evas_key_lock_add(Evas *e, const char *keyname) EINA_ARG_NONNULL(1, 2);

/**
 * @brief   Removes the @a keyname key from the current list of lock keys on
 *          canvas @a e.
 *
 * @since_tizen 2.3
 *
 * @param[in]   e        The pointer to the Evas canvas
 * @param[in]   keyname  The name of the key to remove from the locks list
 *
 * @see evas_key_lock_get
 * @see evas_key_lock_add
 * @see evas_key_lock_on
 * @see evas_key_lock_off
 */
EAPI void                 evas_key_lock_del(Evas *e, const char *keyname) EINA_ARG_NONNULL(1, 2);

/**
 * @brief   Enables or turns on programmatically the modifier key with name @a keyname.
 *
 * @since_tizen 2.3
 *
 * @remarks The effect is as if the key is pressed for the whole time
 *          between this call and a matching evas_key_modifier_off().
 *
 * @param[in]   e        The pointer to the Evas canvas
 * @param[in]   keyname  The name of the modifier to enable
 *
 * @see evas_key_modifier_add
 * @see evas_key_modifier_get
 * @see evas_key_modifier_off
 * @see evas_key_modifier_is_set
 */
EAPI void                 evas_key_modifier_on(Evas *e, const char *keyname) EINA_ARG_NONNULL(1, 2);

/**
 * @brief   Disables or turns off programmatically the modifier key with name @a keyname.
 *
 * @since_tizen 2.3
 *
 * @param[in]   e        The pointer to the Evas canvas
 * @param[in]   keyname  The name of the modifier to disable
 *
 * @see evas_key_modifier_add
 * @see evas_key_modifier_get
 * @see evas_key_modifier_on
 * @see evas_key_modifier_is_set
 */
EAPI void                 evas_key_modifier_off(Evas *e, const char *keyname) EINA_ARG_NONNULL(1, 2);

/**
 * @brief   Enables or turns on programmatically the lock key with name @a keyname.
 *
 * @since_tizen 2.3
 *
 * @remarks The effect is as if the key is put on its active state after
 *          this call.
 *
 * @param[in]   e        The pointer to the Evas canvas
 * @param[in]   keyname  The name of the lock to enable
 *
 * @see evas_key_lock_get
 * @see evas_key_lock_add
 * @see evas_key_lock_del
 * @see evas_key_lock_off
 */
EAPI void                 evas_key_lock_on(Evas *e, const char *keyname) EINA_ARG_NONNULL(1, 2);

/**
 * @brief   Disables or turns off programmatically the lock key with name @a keyname.
 *
 * @since_tizen 2.3
 *
 * @remarks The effect is as if the key is put on its inactive state
 *          after this call.
 *
 * @param[in]   e        The pointer to the Evas canvas
 * @param[in]   keyname  The name of the lock to disable
 *
 * @see evas_key_lock_get
 * @see evas_key_lock_add
 * @see evas_key_lock_del
 * @see evas_key_lock_on
 */
EAPI void                 evas_key_lock_off(Evas *e, const char *keyname) EINA_ARG_NONNULL(1, 2);

/**
 * @brief   Creates a bit mask from the @a keyname @b modifier key. 
 *
 * @since_tizen 2.3
 *
 * @remarks Values returned from different calls to it may be ORed together,
 *          naturally.
 *
 * @remarks This function is meant to be used in conjunction with
 *          evas_object_key_grab()/evas_object_key_ungrab(). See their
 *          documentation for more information.
 *
 * @param[in]   e        The canvas to query the bit mask from
 * @param[in]   keyname  The name of the modifier key to create the mask for
 * @returns The bit mask or @c 0 if the @a keyname key is not registered as a
 *          modifier for canvas @a e
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
 * @brief   Requests @a keyname key events be directed to @a obj.
 *
 * @since_tizen 2.3
 *
 * @remarks Key grabs allow one or more objects to receive key events for
 *          specific key strokes even if other objects have focus. Whenever a
 *          key is grabbed, only the objects grabbing it gets the events
 *          for the given keys.
 *
 * @remarks @a keyname is a platform dependent symbolic name for the key
 *          pressed (see @ref Evas_Keys for more information).
 *
 * @remarks @a modifiers and @a not_modifiers are bit masks of all the
 *          modifiers that must and must not, respectively, be pressed along
 *          with @a keyname key in order to trigger this new key
 *          grab. Modifiers can be things such as Shift and Ctrl as well as
 *          user defined types via evas_key_modifier_add(). Retrieve them with
 *          evas_key_modifier_mask_get() or use @c 0 for empty masks.
 *
 * @remarks @a exclusive makes the given object the only one permitted to
 *          grab the given key. If given #EINA_TRUE, subsequent calls on this
 *          function with different @a obj arguments it fails, unless the key
 *          is ungrabbed again.
 *
 * @remarks Providing impossible modifier sets creates undefined behavior.
 *
 * @param[in]   obj            The object to direct @a keyname events to
 * @param[in]   keyname        The key to request events for
 * @param[in]   modifiers      A mask of modifiers that must be present to
 *                         trigger the event
 * @param[in]   not_modifiers  A mask of modifiers that must @b not be present
 *                         to trigger the event
 * @param[in]   exclusive      Set #EINA_TRUE to request that the @a obj is the only object
 *                         receiving the @a keyname events, \n
 *                         otherwise set #EINA_FALSE
 * @return  #EINA_TRUE if the call succeeded, \n
 *          otherwise #EINA_FALSE on failure
 *
 * @see evas_object_key_ungrab
 * @see evas_object_focus_set
 * @see evas_object_focus_get
 * @see evas_focus_get
 * @see evas_key_modifier_add
 */
EAPI Eina_Bool            evas_object_key_grab(Evas_Object *obj, const char *keyname, Evas_Modifier_Mask modifiers, Evas_Modifier_Mask not_modifiers, Eina_Bool exclusive) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1, 2);

/**
 * @brief   Removes the grab on @a keyname key events by @a obj.
 *
 * @since_tizen 2.3
 *
 * @remarks Removes a key grab on @a obj if @a keyname, @a modifiers, 
 *          and @a not_modifiers match.
 *
 * @param[in]   obj            The object that has an existing key grab
 * @param[in]   keyname        The key the grab is set for
 * @param[in]   modifiers      A mask of modifiers that must be present to
 *                         trigger the event
 * @param[in]   not_modifiers  A mask of modifiers that must not be
 *                         present to trigger the event
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
 * @internal
 * @defgroup Evas_Touch_Point_List Touch Point List Functions
 * @ingroup Evas_Canvas
 *
 * @brief   This group provides functions to get information of touched points in the Evas.
 *
 * @remarks Evas maintains list of touched points on the canvas. Each point has
 *          its co-ordinates, ID and state. You can get the number of touched
 *          points and information of each point using evas_touch_point_list functions.
 *
 * @{
 */

/**
 * @brief   Gets the number of touched point in the evas.
 *
 * @since_tizen 2.3
 *
 * @remarks New touched point is added to the list whenever touching the evas
 *          and point is removed whenever removing touched point from the evas.
 *
 * @remarks The following is an example:
 * @code
 * extern Evas *evas;
 * int count;
 *
 * count = evas_touch_point_list_count(evas);
 * printf("The count of touch points: %i\n", count);
 * @endcode
 *
 * @param[in]   e  The pointer to the Evas canvas
 * @return  The number of touched point on the evas
 *
 * @see evas_touch_point_list_nth_xy_get()
 * @see evas_touch_point_list_nth_id_get()
 * @see evas_touch_point_list_nth_state_get()
 */
EAPI unsigned int           evas_touch_point_list_count(Evas *e) EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the nth touch point's co-ordinates.
 *
 * @since_tizen 2.3
 *
 * @remarks Touch point's co-ordinates is updated whenever moving that point
 *          on the canvas.
 *
 * @remarks The following is an example:
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
 * @param[in]   e  The pointer to the Evas canvas
 * @param[in]   n  The number of the touched point (0 being the first)
 * @param[out]   x  The pointer to a Evas_Coord to be filled in
 * @param[out]   y  The pointer to a Evas_Coord to be filled in
 *
 * @see evas_touch_point_list_count()
 * @see evas_touch_point_list_nth_id_get()
 * @see evas_touch_point_list_nth_state_get()
 */
EAPI void                   evas_touch_point_list_nth_xy_get(Evas *e, unsigned int n, Evas_Coord *x, Evas_Coord *y) EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the @a id of nth touch point.
 *
 * @since_tizen 2.3
 *
 * @remarks The point which comes from Mouse Event has @a id 0 and The point
 *          which comes from Multi Event has @a id that is same as Multi
 *          Event's device ID.
 *
 * @remarks The following is an example:
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
 * @param[in]   e  The pointer to the Evas canvas
 * @param[in]   n  The number of the touched point (@c 0 being the first)
 * @return  The ID of nth touch point, \n 
 *          otherwise @c -1
 *
 * @see evas_touch_point_list_count()
 * @see evas_touch_point_list_nth_xy_get()
 * @see evas_touch_point_list_nth_state_get()
 */
EAPI int                    evas_touch_point_list_nth_id_get(Evas *e, unsigned int n) EINA_ARG_NONNULL(1);

/**
 * @brief   Gets the @a state of nth touch point.
 *
 * @since_tizen 2.3
 *
 * @remarks The point's @a state is EVAS_TOUCH_POINT_DOWN when pressed,
 *          EVAS_TOUCH_POINT_STILL when the point is not moved after pressed,
 *          EVAS_TOUCH_POINT_MOVE when moved at least once after pressed and
 *          EVAS_TOUCH_POINT_UP when released.
 *
 * @remarks The following is an example:
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
 * @param[in]   e  The pointer to the Evas canvas
 * @param[in]   n  The number of the touched point (@c 0 being the first)
 * @return  @a state of nth touch point, \n
 *          otherwise EVAS_TOUCH_POINT_CANCEL
 *
 * @see evas_touch_point_list_count()
 * @see evas_touch_point_list_nth_xy_get()
 * @see evas_touch_point_list_nth_id_get()
 */
EAPI Evas_Touch_Point_State evas_touch_point_list_nth_state_get(Evas *e, unsigned int n) EINA_ARG_NONNULL(1);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif
