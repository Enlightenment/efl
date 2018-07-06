#ifndef _EVAS_H
# error You shall not include this header directly
#endif

/**
 * @def EVAS_VERSION_MAJOR
 * The major number of evas version
 */
#define EVAS_VERSION_MAJOR EFL_VERSION_MAJOR

/**
 * @def EVAS_VERSION_MINOR
 * The minor number of evas version
 */
#define EVAS_VERSION_MINOR EFL_VERSION_MINOR

/**
 * @typedef Evas_Version
 *
 * This is the Evas version information structure that can be used at
 * runtime to detect which version of evas is being used and adapt
 * appropriately as follows for example:
 *
 * @code
 * #if defined(EVAS_VERSION_MAJOR) && (EVAS_VERSION_MAJOR >= 1) && defined(EVAS_VERSION_MINOR) && (EVAS_VERSION_MINOR > 0)
 * printf("Evas version: %i.%i.%i\n",
 *        evas_version->major,
 *        evas_version->minor,
 *        evas_version->micro);
 * if (evas_version->revision > 0)
 *   {
 *     printf("  Built from Git revision # %i\n", evas_version->revision);
 *   }
 * #endif
 * @endcode
 *
 */

typedef struct _Evas_Version
{
   int major; /**< major (binary or source incompatible changes) */
   int minor; /**< minor (new features, bugfixes, major improvements version) */
   int micro; /**< micro (bugfix, internal improvements, no new features version) */
   int revision;  /**< git revision (0 if a proper release or the git revision number Evas is built from) */
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
// Support not having eo available
#if defined (EFL_BETA_API_SUPPORT) && defined(EFL_EO_API_SUPPORT)
typedef Efl_Callback_Priority Evas_Callback_Priority;
#else
typedef short Evas_Callback_Priority;
#endif

// TODO: Mark as EINA_DEPRECATED
typedef struct _Evas_Coord_Rectangle       Evas_Coord_Rectangle; /**< A generic rectangle handle. @deprecated Use Eina_Rectangle instead */
typedef struct _Evas_Point                 Evas_Point;   /**< integer point */

typedef struct _Evas_Coord_Point           Evas_Coord_Point;    /**< Evas_Coord point */
typedef struct _Evas_Coord_Precision_Point Evas_Coord_Precision_Point;   /**< Evas_Coord point with sub-pixel precision */

typedef struct _Evas_Coord_Size            Evas_Coord_Size;    /**< Evas_Coord size @since 1.8 */
typedef struct _Evas_Coord_Precision_Size  Evas_Coord_Precision_Size;    /**< Evas_Coord size with sub-pixel precision @since 1.8 */

typedef struct _Evas_Position              Evas_Position;   /**< associates given point in Canvas and Output */
typedef struct _Evas_Precision_Position    Evas_Precision_Position;   /**< associates given point in Canvas and Output, with sub-pixel precision */

typedef int                                Evas_Coord; /**< Type used for coordinates (in pixels, int). */
typedef int                                Evas_Font_Size; /**< Type used for font sizes (int). */

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
 * An Evas interface is exactly like the OO-concept: a 'contract' or
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
typedef Eo                 Efl_Canvas_Object;
typedef Efl_Canvas_Object  Evas_Object;

/* These defines are used in H files generated by Eolian to avoid
 * redefinition of types */
#define _EFL_CANVAS_OBJECT_EO_CLASS_TYPE
#define _EFL_VG_EO_CLASS_TYPE

/**
 * Type of abstract VG node
 */
typedef Eo      Efl_VG;

typedef void                        Evas_Performance; /**< An Evas Performance handle */
typedef struct _Evas_Smart          Evas_Smart; /**< An Evas Smart Object handle */
typedef int                        Evas_Angle; /**< A type for angle */

// FIXME: This can be a simple alias to Eina_Rectangle instead.
struct _Evas_Coord_Rectangle /** A rectangle in Evas_Coord */
{
   Evas_Coord x; /**< top-left x co-ordinate of rectangle */
   Evas_Coord y; /**< top-left y co-ordinate of rectangle */
   Evas_Coord w; /**< width of rectangle */
   Evas_Coord h; /**< height of rectangle */
};

struct _Evas_Coord_Point /** A Point in Evas_Coord */
{
   Evas_Coord x; /**< x co-ordinate */
   Evas_Coord y; /**< y co-ordinate */
};

struct _Evas_Coord_Size /** A size in Evas_Coord */
{
   Evas_Coord w; /**< width */
   Evas_Coord h; /**< height */
};


struct _Evas_Coord_Precision_Size /** A size in Evas_Coord with subpixel precision*/
{
   Evas_Coord w; /**< width */
   Evas_Coord h; /**< height */
   double wsub;  /**< subpixel precision for width */
   double ysub;  /**< subpixel precision for height */
};

struct _Evas_Coord_Precision_Point /** A point in Evas_Coord with subpixel precision*/
{
   Evas_Coord x; /**< x co-ordinate */
   Evas_Coord y; /**< y co-ordinate */
   double     xsub; /**< subpixel precision for x */
   double     ysub; /**< subpixel precision for y */
};

struct _Evas_Point /** A point */
{
   int x; /**< x co-ordinate */
   int y; /**< y co-ordinate */
};

struct _Evas_Position /** A position */
{
   Evas_Point       output; /**< position on the output */
   Evas_Coord_Point canvas; /**< position on the canvas */
};

struct _Evas_Precision_Position /** A position with precision*/
{
   Evas_Point                 output; /**< position on the output */
   Evas_Coord_Precision_Point canvas; /**< position on the canvas */
};

typedef struct _Evas_Pixel_Import_Source Evas_Pixel_Import_Source; /**< A source description of pixels for importing pixels */

/* Opaque types */
typedef Eo                               Evas_Device; /**< A source device handle - where the event came from */

typedef Efl_Image_Content_Hint           Evas_Image_Content_Hint;
#define EVAS_IMAGE_CONTENT_HINT_NONE     EFL_IMAGE_CONTENT_HINT_NONE
#define EVAS_IMAGE_CONTENT_HINT_DYNAMIC  EFL_IMAGE_CONTENT_HINT_DYNAMIC
#define EVAS_IMAGE_CONTENT_HINT_STATIC   EFL_IMAGE_CONTENT_HINT_STATIC

typedef enum _Evas_Alloc_Error
{
   EVAS_ALLOC_ERROR_NONE = 0, /**< No allocation error */
   EVAS_ALLOC_ERROR_FATAL = 1, /**< Allocation failed despite attempts to free up memory */
   EVAS_ALLOC_ERROR_RECOVERED = 2 /**< Allocation succeeded after freeing up speculative resource memory */
} Evas_Alloc_Error; /**< Possible allocation errors returned by evas_alloc_error() */

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

typedef enum _Evas_Engine_Render_Mode
{
   EVAS_RENDER_MODE_BLOCKING = 0, /**< The rendering is blocking mode*/
   EVAS_RENDER_MODE_NONBLOCKING = 1, /**< The rendering is non blocking mode*/
} Evas_Engine_Render_Mode; /**< behaviour of the renderer*/

typedef Efl_Gfx_Event_Render_Post          Evas_Event_Render_Post; /**< Event info sent after a frame was rendered. @since 1.18 */

typedef Efl_Input_Device_Type             Evas_Device_Class;

#define EVAS_DEVICE_CLASS_NONE             EFL_INPUT_DEVICE_TYPE_NONE /**< Not a device @since 1.8 */
#define EVAS_DEVICE_CLASS_SEAT             EFL_INPUT_DEVICE_TYPE_SEAT /**< The user/seat (the user themselves) @since 1.8 */
#define EVAS_DEVICE_CLASS_KEYBOARD         EFL_INPUT_DEVICE_TYPE_KEYBOARD /**< A regular keyboard, numberpad or attached buttons @since 1.8 */
#define EVAS_DEVICE_CLASS_MOUSE            EFL_INPUT_DEVICE_TYPE_MOUSE /**< A mouse, trackball or touchpad relative motion device @since 1.8 */
#define EVAS_DEVICE_CLASS_TOUCH            EFL_INPUT_DEVICE_TYPE_TOUCH /**< A touchscreen with fingers or stylus @since 1.8 */
#define EVAS_DEVICE_CLASS_PEN              EFL_INPUT_DEVICE_TYPE_PEN /**< A special pen device @since 1.8 */
#define EVAS_DEVICE_CLASS_POINTER          EFL_INPUT_DEVICE_TYPE_WAND /**< A laser pointer, wii-style or "minority report" pointing device @since 1.8 */
#define EVAS_DEVICE_CLASS_WAND             EFL_INPUT_DEVICE_TYPE_WAND /**< A synonym for EVAS_DEVICE_CLASS_POINTER @since 1.18 */
#define EVAS_DEVICE_CLASS_GAMEPAD          EFL_INPUT_DEVICE_TYPE_GAMEPAD /**<  A gamepad controller or joystick @since 1.8 */

/**
 * @brief Specific type of input device.
 *
 * Note: Currently not used inside EFL.
 *
 * @since 1.8
 */
typedef enum
{
  EVAS_DEVICE_SUBCLASS_NONE = 0, /**< Not a device. */
  EVAS_DEVICE_SUBCLASS_FINGER, /**< The normal flat of your finger. */
  EVAS_DEVICE_SUBCLASS_FINGERNAIL, /**< A fingernail. */
  EVAS_DEVICE_SUBCLASS_KNUCKLE, /**< A Knuckle. */
  EVAS_DEVICE_SUBCLASS_PALM, /**< The palm of a users hand. */
  EVAS_DEVICE_SUBCLASS_HAND_SIZE, /**< The side of your hand. */
  EVAS_DEVICE_SUBCLASS_HAND_FLAT, /**< The flat of your hand. */
  EVAS_DEVICE_SUBCLASS_PEN_TIP, /**< The tip of a pen. */
  EVAS_DEVICE_SUBCLASS_TRACKPAD, /**< A trackpad style mouse. */
  EVAS_DEVICE_SUBCLASS_TRACKPOINT, /**< A trackpoint style mouse. */
  EVAS_DEVICE_SUBCLASS_TRACKBALL /**< A trackball style mouse. */
} Evas_Device_Subclass;

typedef Efl_Pointer_Flags                  Evas_Button_Flags;

#define EVAS_BUTTON_NONE                   EFL_POINTER_FLAGS_NONE
#define EVAS_BUTTON_DOUBLE_CLICK           EFL_POINTER_FLAGS_DOUBLE_CLICK
#define EVAS_BUTTON_TRIPLE_CLICK           EFL_POINTER_FLAGS_TRIPLE_CLICK

typedef Efl_Input_Flags                    Evas_Event_Flags;

#define EVAS_EVENT_FLAG_NONE               EFL_INPUT_FLAGS_NONE
#define EVAS_EVENT_FLAG_ON_HOLD            EFL_INPUT_FLAGS_PROCESSED
#define EVAS_EVENT_FLAG_ON_SCROLL          EFL_INPUT_FLAGS_SCROLLING

typedef Efl_Gfx_Size_Hint_Aspect           Evas_Aspect_Control; /**< Aspect types/policies for scaling size hints, used for evas_object_size_hint_aspect_set */

#define EVAS_ASPECT_CONTROL_NONE           EFL_GFX_SIZE_HINT_ASPECT_NONE
#define EVAS_ASPECT_CONTROL_NEITHER        EFL_GFX_SIZE_HINT_ASPECT_NEITHER
#define EVAS_ASPECT_CONTROL_HORIZONTAL     EFL_GFX_SIZE_HINT_ASPECT_HORIZONTAL
#define EVAS_ASPECT_CONTROL_VERTICAL       EFL_GFX_SIZE_HINT_ASPECT_VERTICAL
#define EVAS_ASPECT_CONTROL_BOTH           EFL_GFX_SIZE_HINT_ASPECT_BOTH

typedef Efl_Text_Bidirectional_Type        Evas_BiDi_Direction;

#define EVAS_BIDI_DIRECTION_NATURAL        EFL_TEXT_BIDIRECTIONAL_TYPE_NATURAL
#define EVAS_BIDI_DIRECTION_NEUTRAL        EFL_TEXT_BIDIRECTIONAL_TYPE_NEUTRAL
#define EVAS_BIDI_DIRECTION_LTR            EFL_TEXT_BIDIRECTIONAL_TYPE_LTR
#define EVAS_BIDI_DIRECTION_RTL            EFL_TEXT_BIDIRECTIONAL_TYPE_RTL
#define EVAS_BIDI_DIRECTION_INHERIT        EFL_TEXT_BIDIRECTIONAL_TYPE_INHERIT

typedef Efl_Input_Object_Pointer_Mode      Evas_Object_Pointer_Mode;

#define EVAS_OBJECT_POINTER_MODE_AUTOGRAB  EFL_INPUT_OBJECT_POINTER_MODE_AUTO_GRAB
#define EVAS_OBJECT_POINTER_MODE_NOGRAB    EFL_INPUT_OBJECT_POINTER_MODE_NO_GRAB
#define EVAS_OBJECT_POINTER_MODE_NOGRAB_NO_REPEAT_UPDOWN EFL_INPUT_OBJECT_POINTER_MODE_NO_GRAB_NO_REPEAT_UPDOWN

// FIXME: Move to Evas_Legacy.h
/** Identifier of callbacks to be set for Evas canvases or Evas objects. */
typedef enum
{
  EVAS_CALLBACK_MOUSE_IN = 0, /**< Mouse In Event */
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
  EVAS_CALLBACK_CANVAS_FOCUS_IN, /**< Canvas got focus as a whole */
  EVAS_CALLBACK_CANVAS_FOCUS_OUT, /**< Canvas lost focus as a whole */
  EVAS_CALLBACK_RENDER_FLUSH_PRE, /**< Called after render update regions have
                                   * been calculated, but only if update regions exist */
  EVAS_CALLBACK_RENDER_FLUSH_POST, /**< Called after render update regions have
                                    * been sent to the display server, but only
                                    * if update regions existed for the most recent frame */
  EVAS_CALLBACK_CANVAS_OBJECT_FOCUS_IN, /**< Canvas object got focus */
  EVAS_CALLBACK_CANVAS_OBJECT_FOCUS_OUT, /**< Canvas object lost focus */
  EVAS_CALLBACK_IMAGE_UNLOADED, /**< Image data has been unloaded (by some mechanism in Evas that throw out original image data) */
  EVAS_CALLBACK_RENDER_PRE, /**< Called just before rendering starts on the canvas target. @since 1.2 */
  EVAS_CALLBACK_RENDER_POST, /**< Called just after rendering stops on the canvas target. @since 1.2 */
  EVAS_CALLBACK_IMAGE_RESIZE, /**< Image size is changed. @since 1.8 */
  EVAS_CALLBACK_DEVICE_CHANGED, /**< Devices added, removed or changed on canvas. @since 1.8 */
  EVAS_CALLBACK_AXIS_UPDATE, /**< Input device changed value on some axis. @since 1.13 */
  EVAS_CALLBACK_CANVAS_VIEWPORT_RESIZE, /**< Canvas viewport resized. @since 1.15 */
  EVAS_CALLBACK_LAST /**< Sentinel value to indicate last enum field during
                      * iteration */
} Evas_Callback_Type;

typedef void      (*Evas_Smart_Cb)(void *data, Evas_Object *obj, void *event_info);  /**< Evas smart objects' "smart callback" function signature */
typedef void      (*Evas_Event_Cb)(void *data, Evas *e, void *event_info);  /**< Evas event callback function signature */
typedef Eina_Bool (*Evas_Object_Event_Post_Cb)(void *data, Evas *e);  /**< Evas object event (post) callback function signature */
typedef void      (*Evas_Object_Event_Cb)(void *data, Evas *e, Evas_Object *obj, void *event_info);  /**< Evas object event callback function signature */
typedef void      (*Evas_Async_Events_Put_Cb)(void *target, Evas_Callback_Type type, void *event_info); /**< Evas async callback function signature */

/**
 * @defgroup Evas_Main_Group Top Level Functions
 * @ingroup Evas
 *
 * Functions that affect Evas as a whole.
 */

/**
 * Get the path for the cserve binary to execute
 *
 * There is little need for anyone except a desktop environment to call this.
 * This can be called before evas_init() has been called. It will try and find
 * the full path to the to the cserve binary to run to provide cserve image
 * and font caching services for evas.
 *
 * @return NULL if error, or a string with the full path to the cserve binary.
 *
 * @since 1.8
 */
EAPI const char *evas_cserve_path_get(void);

/**
 * @brief Directly initialize Evas and its required dependencies.
 *
 * @return The number of times evas_init() has been called.
 *
 * Permits use of Evas independently from @ref Ecore.  This can be
 * useful in certain types of examples and test programs, as well as by
 * Ecore-Evas' @c ecore_evas_init() itself (which is what most EFL
 * applications will be using instead).
 *
 * The @ref Example_Evas_Buffer_Simple "evas-buffer-simple.c" example
 * demonstrates use of evas_init(), and then manually setting up the
 * canvas:
 *
 * @dontinclude evas-buffer-simple.c
 * @skip int main
 * @until return -1;
 *
 * The canvas is set up using the example's create_canvas() routine,
 * which forces selection of Evas' "buffer" rendering engine.  The
 * buffer engine simply renders to a memory buffer with no hardware
 * acceleration.
 *
 * @skip static Evas *create_canvas
 * @until    evas_output_viewport_set(canvas,
 *
 * @see evas_shutdown().
 *
 * @ingroup Evas_Main_Group
 */
EAPI int               evas_init(void);

/**
 * @brief Directly shutdown Evas.
 *
 * @return The (decremented) init reference counter.
 *
 * Low level routine to finalize Evas.  Decrements a counter of the
 * number of times evas_init() has been called, and, if appropriate,
 * shuts down associated dependency modules and libraries.  A return
 * value of 0 indicates that everything has been properly shut down.
 *
 * Ecore-Evas applications will typically use ecore_evas_shutdown()
 * instead, as described in evas_init().
 *
 * The @ref Example_Evas_Buffer_Simple "evas-buffer-simple.c" example
 * shows use of evas_shutdown() in its destroy_canvas() routine:
 *
 * @dontinclude evas-buffer-simple.c
 * @skip static void destroy_canvas
 * @until   evas_free(canvas)
 *
 * @see evas_init().
 *
 * @ingroup Evas_Main_Group
 */
EAPI int               evas_shutdown(void);

/**
 * @brief Get the error status of the most recent memory allocation call
 *
 * @return Allocation error codes EVAS_ALLOC_ERROR_NONE,
 * EVAS_ALLOC_ERROR_FATAL or EVAS_ALLOC_ERROR_RECOVERED.
 *
 * Accesses the current error status for memory allocation, or
 * EVAS_ALLOC_ERROR_NONE if allocation succeeded with no errors.
 *
 * EVAS_ALLOC_ERROR_FATAL means that no memory allocation was possible, but
 * the function call exited as cleanly as possible.  This is a sign of very low
 * memory, and indicates the caller should attempt a safe recovery and possibly
 * re-try after freeing up additional memory.
 *
 * EVAS_ALLOC_ERROR_RECOVERED indicates that Evas was able to free up
 * sufficient memory internally to perform the requested memory
 * allocation and the program will continue to function normally, but
 * memory is in a low state and the program should strive to free memory
 * itself.  Evas' approach to free memory internally may reduce the
 * resolution of images, free cached fonts or images, throw out
 * pre-rendered data, or reduce the complexity of change lists.
 *
 * Example:
 * @code
 * extern Evas_Object *object;
 * void callback (void *data, Evas *e, Evas_Object *obj, void *event_info);
 *
 * evas_object_event_callback_add(object, EVAS_CALLBACK_MOUSE_DOWN, callback, NULL);
 * if (evas_alloc_error() == EVAS_ALLOC_ERROR_FATAL)
 *   {
 *     fprintf(stderr, "ERROR: Failed to attach callback.  Out of memory.\n");
 *     fprintf(stderr, "       Must destroy object now as it cannot be used.\n");
 *     evas_object_del(object);
 *     object = NULL;
 *     fprintf(stderr, "WARNING: Cleaning out RAM.\n");
 *     my_memory_cleanup();
 *   }
 * if (evas_alloc_error() == EVAS_ALLOC_ERROR_RECOVERED)
 *   {
 *     fprintf(stderr, "WARNING: Memory is really low. Cleaning out RAM.\n");
 *     my_memory_cleanup();
 *   }
 * @endcode
 *
 * @ingroup Evas_Main_Group
 */
EAPI Evas_Alloc_Error  evas_alloc_error(void);

/**
 * @brief Access the canvas' asynchronous event queue.
 *
 * @return A file descriptor to the asynchronous events.
 *
 * Normally, Evas handles asynchronous events internally, particularly
 * in Evas-using modules that are part of the EFL infrastructure.
 * Notably, ecore-evas takes care of processing these events for
 * canvases instantiated through it.
 *
 * However, when asynchronous calculations need to be done outside the
 * main thread (in some other mainloop) with some followup action, this
 * function permits accessing the events.  An example would be
 * asynchronous image preloading.
 *
 * @ingroup Evas_Main_Group
 */
EAPI int               evas_async_events_fd_get(void) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Process the asynchronous event queue.
 *
 * @return The number of events processed.
 *
 * Triggers the callback functions for asynchronous events that were
 * queued up by evas_async_events_put().  The callbacks are called in
 * the same order that they were queued.
 *
 * @ingroup Evas_Main_Group
 */
EAPI int               evas_async_events_process(void);

/**
 * @brief Insert asynchronous events on the canvas.
 *
 * @param target The target to be affected by the events.
 * @param type The type of callback function.
 * @param event_info Information about the event.
 * @param func The callback function pointer.
 *
 * @return EINA_FALSE if an error occurred, EINA_TRUE otherwise.
 *
 * Allows routines running outside Evas' main thread to report an
 * asynchronous event.  The target, type, and event info will be passed
 * to the callback function when evas_async_events_process() is called.
 *
 * @ingroup Evas_Main_Group
 */
EAPI Eina_Bool         evas_async_events_put(const void *target, Evas_Callback_Type type, void *event_info, Evas_Async_Events_Put_Cb func) EINA_ARG_NONNULL(1, 4);

/**
 * @defgroup Evas_Canvas Canvas Functions
 * @ingroup Evas
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
 * @}
 */

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
 *   printf("%s\n", engine_name);
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
 *   printf("%s\n", engine_name);
 * evas_render_method_list_free(engine_list);
 * @endcode
 */
EAPI void              evas_render_method_list_free(Eina_List *list);

/**
 * @}
 */

/**
 * @defgroup Evas_Coord_Mapping_Group Coordinate Mapping Functions
 *
 * Functions that are used to map coordinates from the canvas to the
 * screen or the screen to the canvas.
 *
 * @ingroup Evas_Canvas
 */

/**
 * @defgroup Evas_Output_Size Output and Viewport Resizing Functions
 *
 * Functions that set and retrieve the output and viewport size of an
 * evas.
 *
 * @ingroup Evas_Canvas
 */

/**
 * @defgroup Evas_Canvas_Events Canvas Events
 *
 * Functions relating to canvas events that report on changes of
 * its internal states (an object got focused, the rendering
 * is updated, etc).
 *
 * Some of the functions in this group are exemplified @ref
 * Example_Evas_Events "here".
 *
 * @ingroup Evas_Canvas
 */

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
 * function_that_does_work_that_cant_be_interrupted_by_events();
 * evas_event_thaw(my_evas_canvas);
 * @endcode
 *
 * Some of the functions in this group are exemplified @ref
 * Example_Evas_Events "here".
 *
 * @ingroup Evas_Canvas_Events
 */

/**
 * @defgroup Evas_Event_Feeding_Group Input Events Feeding Functions
 *
 * Functions to tell Evas that input events happened and should be
 * processed.
 *
 * @warning Most of the time these functions are @b not what you're looking for.
 * These functions should only be used if you're not working with ecore evas (or
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
 * Add a new device type
 *
 * @param e The canvas to create the device node for.
 *
 * Adds a new device node to the given canvas @p e. All devices created as
 * part of the canvas @p e will automatically be deleted when the canvas
 * is freed.
 *
 * @return the device node created or NULL if an error occurred.
 *
 * @see evas_device_del
 * @see evas_device_add_full
 * @since 1.8
 */
EAPI Evas_Device *evas_device_add(Evas *e);

/**
 * Add a new device type
 *
 * @param e The canvas to create the device node for.
 * @param name The name of the device.
 * @param desc The description of the device.
 * @param parent_dev The parent device.
 * @param emulation_dev The source device.
 * @param clas The device class.
 * @param sub_class  The device subclass.
 *
 * Adds a new device node to the given canvas @p e. All devices created as
 * part of the canvas @p e will automatically be deleted when the canvas
 * is freed.
 *
 * @return the device node created or NULL if an error occurred.
 *
 * @see evas_device_del
 * @since 1.19
 */
EAPI Evas_Device *evas_device_add_full(Evas *e, const char *name,
                                       const char *desc,
                                       Evas_Device *parent_dev,
                                       Evas_Device *emulation_dev,
                                       Evas_Device_Class clas,
                                       Evas_Device_Subclass sub_clas);

/**
 * Delete a new device type
 *
 * @param dev The device node you want to delete.
 *
 * @see evas_device_add
 * @see evas_device_push
 * @see evas_device_pop
 * @since 1.8
 */
EAPI void evas_device_del(Evas_Device *dev);

/**
 * Push the current context device onto the device stack
 *
 * @param e The canvas to push the device on to
 * @param dev The device to push.
 *
 * This pushes the given device @p dev onto the stack for the canvas @p e
 * resulting in the dev pointer in all events that get fed to the canvas
 * being the device at the top of the device stack for that canvas.
 *
 * If a device is pushed onto the device stack, it will not be deleted
 * until a canvas free OR until it has been popped from the stack even if
 * evas_device_del() is called.
 *
 * The device @p dev must have been created as a device for the canvas it
 * is pushed onto (and not another canvas).
 *
 * Example:
 * @code
 * evas_device_push(canvas, dev);
 * evas_event_feed_mouse_move(canvas, 20, 30, 0, NULL);
 * evas_device_pop(canvas);
 * @endcode
 *
 * @see evas_device_pop
 * @since 1.8
 */
EAPI void evas_device_push(Evas *e, Evas_Device *dev);

/**
 * This pops the top of the device stack for the canvas
 *
 * @param e The canvas to pop the device stack from
 *
 * This pops the top of the device stack making the current device context
 * used for device events being what is now at the top of the stack after
 * popping.
 *
 * @see evas_device_push
 * @since 1.8
 */
EAPI void evas_device_pop(Evas *e);

/**
 * List all current devices attached to the given canvas and/or device
 *
 * @param e The canvas to query for a device list
 * @param dev A specific device inside the canvas to query for child devices or NULL if just querying the base canvas devices
 * @return An internal list of Evas_Device pointers, or NULL if no devices are found
 *
 * This will list all devices belonging to a specific evas canvas @p e, at the
 * top-level in the device tree if @p dev passed in is NULL. If @p dev is
 * a valid device for the given canvas @p e, then a list of child devices
 * of @p dev will be returned, allowing you to walk the device tree.
 *
 * The list returned is only valid so long as no changes are made to the
 * device tree in the given canvas @p e. If there are no devices or children
 * then NULL is returned.
 *
 * @see evas_device_parent_get
 * @see evas_device_name_get
 * @see evas_device_description_get
 * @see evas_device_class_get
 * @see evas_device_subclass_get
 * @see evas_device_emulation_source_get
 * @since 1.8
 */
EAPI const Eina_List *evas_device_list(Evas *e, const Evas_Device *dev);

/**
 * Get a device by its name
 *
 * @param e The canvas to find the device on
 * @param name The name of the device.
 *
 * Gets the first ocurrence of a device named as @p name
 * on Evas @p e list of devices.
 *
 * @return the device or NULL if an error occurred, no name was provided,
 * or no device with a matching name was found.
 *
 * @since 1.19
 */
EAPI Evas_Device *evas_device_get(Evas *e, const char *name);

/**
 * Get a device by its seat id
 *
 * @param e The canvas to find the device on
 * @param id The seat id of the device.
 *
 * Gets the device with id @p id on Evas @p e list of devices.
 *
 * @return the device or NULL if no device with a matching id was found.
 *
 * @since 1.20
 */
EAPI Evas_Device *evas_device_get_by_seat_id(Evas *eo_e, unsigned int id);

/**
 * Set the name of a device as a string
 *
 * @p dev The device to set the name of
 * @p name The name string as a readable C UTF8 string
 *
 * @since 1.8
 */
EAPI void evas_device_name_set(Evas_Device *dev, const char *name);

/**
 * Get the name of a device
 *
 * @p dev The device to query
 * @return The device name string or NULL if none is set
 *
 * This gets the name set by evas_device_name_set(). This is a readable UTF8
 * C string, or NULL if no name is set.
 *
 * The name should be a short name like "Wireless Mouse", "Joystick",
 * "Finger", "Keyboard" or "Numberpad" etc.
 *
 * @since 1.8
 */
EAPI const char *evas_device_name_get(const Evas_Device *dev);

/**
 * Set the seat id of a device
 *
 * @p dev The device to set the seat id of
 * @p name The seat id
 *
 * @since 1.20
 */
EAPI void evas_device_seat_id_set(Evas_Device *dev, unsigned int id);

/**
 * Get the seat id of a device
 *
 * @p dev The device to query
 * @return The device seat id or 0 if none is set
 *
 * This gets the seat id set by evas_device_seat id_set().
 *
 * A seat id is the hardware id of the seat.
 *
 * @since 1.20
 */
EAPI unsigned int evas_device_seat_id_get(const Evas_Device *dev);

/**
 * Set the description of a device as a string
 *
 * @p dev The device to set the description of
 * @p name The description string as a readable C UTF8 string
 *
 * @since 1.8
 */
EAPI void evas_device_description_set(Evas_Device *dev, const char *desc);

/**
 * Get the description of a device
 *
 * @p dev The device to query
 * @return The device description string or NULL if none is set
 *
 * This gets the description set by evas_device_description_set(). This is
 * a readable UTF8 C string, or NULL if no description is set.
 *
 * A description is meant to be a longer string describing the device so a
 * human may make sense of it. For example "Wireless 6 button mouse in Black
 * with red buttons" would be a good description, so a user may identify
 * precisely which device is being talked about.
 *
 * @since 1.8
 */
EAPI const char *evas_device_description_get(const Evas_Device *dev);

/**
 * Set the parent of a device
 *
 * @p dev The device to set the parent of
 * @p parent The new parent device
 *
 * This sets the parent of a device @p dev to the parent given by @p parent.
 * If the device already has a parent, it is removed from that parent's list.
 * If @p parent is NULL then the device is unparented and placed back as a
 * root device in the canvas.
 *
 * When a device is deleted with evas_device_del(), all children are also
 * deleted along with it.
 *
 * @see evas_device_del
 * @see evas_device_parent_get
 * @see evas_device_list
 *
 * @since 1.8
 */
EAPI void evas_device_parent_set(Evas_Device *dev, Evas_Device *parent);

/**
 * Get the parent of a device
 *
 * @param dev The device to query
 * @return The parent device or NULL if it is a toplevel
 *
 * This returns the parent device of any given device entry, or NULL if no
 * parent device exists (is a toplevel device).
 *
 * @since 1.8
 */
EAPI const Evas_Device *evas_device_parent_get(const Evas_Device *dev);

/**
 * Set the major class of device
 *
 * @param dev The device whose class to set
 * @param clas The class to set it to
 *
 * This sets the "primary" class of device (a broad thing like mouse, keyboard,
 * touch, pen etc.).
 *
 * @deprecated The class of a device can not be changed after creation.
 *
 * @since 1.8
 */
EAPI void evas_device_class_set(Evas_Device *dev, Evas_Device_Class clas) EINA_DEPRECATED;

/**
 * Get the major class of a device
 *
 * @param dev The devise to query
 * @return The device class to set
 *
 * This gets the device class set by evas_device_class_set().
 *
 * @since 1.8
 */
EAPI Evas_Device_Class evas_device_class_get(const Evas_Device *dev);

/**
 * Set the sub-class of a device
 *
 * @param dev The device to modify
 * @param clas The sub-class to set
 *
 * This sets the sub-class of a device, giving much more detailed usage
 * within a broader category.
 *
 * @since 1.8
 */
EAPI void evas_device_subclass_set(Evas_Device *dev, Evas_Device_Subclass clas);

/**
 * Get the device sub-class
 *
 * @param dev The device to query
 * @return The device sub-class set by evas_device_subclass_set().
 *
 * @since 1.8
 */
EAPI Evas_Device_Subclass evas_device_subclass_get(const Evas_Device *dev);

/**
 * Set the emulation source device
 *
 * @param dev The device being emulated
 * @param src The primary source device producing events in the emulated device
 *
 * Devices may not be real, but may be emulated by listening to input on other
 * devices and modifying or interpeting it to generate output on an emulated
 * device (example a finger on a touchscreen will often emulate a mouse when
 * it presses). This allows you to set which device primarily emulates @p dev
 * so the user can choose to ignore events from emulated devices if they also
 * pay attention to source device events for example.
 *
 * @since 1.8
 */
EAPI void evas_device_emulation_source_set(Evas_Device *dev, Evas_Device *src);

/**
 * Get the emulation source device
 *
 * @param dev The device to query
 * @return The source emulation device set by evas_device_emulation_source_set().
 *
 * @since 1.8
 */
EAPI const Evas_Device *evas_device_emulation_source_get(const Evas_Device *dev);

/**
 * @}
 */

/**
 * @defgroup Evas_Image_Group Image Functions
 *
 * Functions that deal with images at canvas level.
 *
 * @ingroup Evas_Canvas
 */

/**
 * @defgroup Evas_Font_Group Font Functions
 *
 * Functions that deal with fonts.
 *
 * @ingroup Evas_Canvas
 */

/**
 * @defgroup Evas_Object_Group Generic Object Functions
 * @ingroup Evas
 *
 * Functions that manipulate generic Evas objects.
 *
 * All Evas displaying units are Evas objects. One handles them all by
 * means of the handle ::Evas_Object. Besides Evas treats their
 * objects equally, they have @b types that define their specific
 * behavior (and individual API).
 *
 * Evas comes with a set of built-in object types:
 *   - rectangle,
 *   - line,
 *   - polygon,
 *   - text,
 *   - textblock,
 *   - textgrid and
 *   - image.
 *
 * These functions apply to @b any Evas object, whatever type they
 * may have.
 *
 * @note The built-in types that are most used are rectangles, text
 * and images. In fact, with these one can create 2D interfaces
 * of arbitrary complexity and EFL makes it easy.
 */

/**
 * @defgroup Evas_Object_Group_Basic Basic Object Manipulation
 *
 * Almost every evas object created will have some generic function used to
 * manipulate it. That's because there are a number of basic actions to be done
 * to objects that are irrespective of the object's type, things like:
 * @li Showing/Hiding
 * @li Setting (and getting) geometry
 * @li Bring up or down a layer
 * @li Color management
 * @li Handling focus
 * @li Clipping
 * @li Reference counting
 *
 * All of these issues are handled through the functions grouped here. Examples
 * of these function can be seen in @ref Example_Evas_Object_Manipulation(which
 * deals with the most common ones) and in @ref Example_Evas_Stacking (which
 * deals with stacking functions).
 *
 * @ingroup Evas_Object_Group
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
 * @defgroup Evas_Object_Group_Extras Extra Object Manipulation
 *
 * Miscellaneous functions that also apply to any object, but are less
 * used or not implemented by all objects.
 *
 * Examples of this group of functions can be found @ref
 * Example_Evas_Stacking "here" and @ref Example_Evas_Events "here".
 *
 * @ingroup Evas_Object_Group
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
 * @defgroup Evas_Object_Specific Specific Object Functions
 * @ingroup Evas
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
 * This will have issues if the @c evas_canvas is resized, however most
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
 * In the @ref clipping section we used a solid white clipper that produced no
 * change in the color of the clipped object, it just hid what was outside the
 * clippers area. It is however sometimes desirable to change the color of an
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

/**
 * @defgroup Evas_Object_Image Image Object Functions
 *
 * Here are grouped together functions used to create and manipulate
 * image objects. They are available to whichever occasion one needs
 * complex imagery on a GUI that could not be achieved by the other
 * Evas' primitive object types, or to make image manipulations.
 *
 * Evas will support whatever image file types it was compiled with
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
 * Evas has even a scale cache that will take care of caching scaled
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
 * while displaying a given image, the program can be in the
 * background loading the next and previous images already, so that
 * displaying them in sequence is just a matter of repainting the
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
 * If you're loading images that are too big, consider setting
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
 * object that does lots of loading speed-ups for you.
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
 *   The image is just an alpha mask (8 bit's per pixel). This is used
 *   for alpha masking.
 *
 * @warning We don't guarantee any proper results if you create an Image object
 * without setting the evas engine.
 *
 * Some examples of this group of functions can be found @ref
 * Example_Evas_Images "here".
 *
 * @ingroup Evas_Object_Specific
 * @{
 */

/**
 * Function signature for the evas object pixels get callback function
 * @see evas_object_image_pixels_get()
 *
 * By @a data it will be passed the private data. By @a o it will be passed the
 * Evas_Object image on which the pixels are requested.
 *
 */
typedef void (*Evas_Object_Image_Pixels_Get_Cb)(void *data, Evas_Object *o);

/**
 * Check if a file extension may be supported by @ref Evas_Object_Image.
 *
 * @param file The file to check
 * @return @c EINA_TRUE if we may be able to open it, @c EINA_FALSE if it's
 * unlikely.
 * @since 1.1
 *
 * If file is an Eina_Stringshare, use directly @ref evas_object_image_extension_can_load_fast_get.
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

/**
 * @}
 */

/**
 * @defgroup Evas_Object_Textgrid Textgrid Object Functions
 *
 * Textgrid objects manage chunks of text as a 2D grid of cells, each of
 * which contains a single unicode character with color and style
 * formatting information.  The object's text can be interacted with
 * character-by-character or by row.
 *
 * @todo put here some usage examples
 *
 * @since 1.7
 *
 * @ingroup Evas_Object_Specific
 *
 * @{
 */

/**
 * @struct _Evas_Textgrid_Cell
 *
 * A cell contains a single unicode character, with associated formatting
 * data including style, color, and color palette.  Double-wide characters
 * are flagged to permit visually merging the cell to the right.
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
 */

/**
 * @defgroup Evas_Smart_Group Smart Functions
 * @ingroup Evas
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
 *
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
 * Initialize to zero a whole Evas_Smart_Class structure.
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
 * Initialize to zero a whole Evas_Smart_Class structure and set version.
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
 * Initialize to zero a whole Evas_Smart_Class structure and set name
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
 * Initialize to zero a whole Evas_Smart_Class structure and set name,
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
 * Initialize to zero a whole Evas_Smart_Class structure and set name,
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
 * from another one. In order for this to work, the user @b must provide some
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
 * from another one. In order for this to work, the user @b must provide some
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
 *
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
 * Get the number of uses of the smart instance
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
 * behavior, etc.), so either never remove the original
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
 * @ingroup Evas
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
 * object events. They are defined by an <b>event string</b> that
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
 * Registers an object type and its associated class. LEGACY MECHANISM SUPPORT.
 *
 * This function is invoked in the class constructor of smart classes. It will
 * add the type and the class into a hash table that will then be used to check
 * the type of an object.
 * This function has been implemented to support legacy mechanism that checks
 * objects types by name.
 * USE IT ONLY FOR LEGACY SUPPORT.
 * Otherwise, it is HIGHLY recommended to use efl_isa.
 *
 * @param type The type (name string) to add.
 * @param klass The class to associate to the type.
 *
 * @see efl_isa
 *
 * @ingroup Evas_Smart_Object_Group
 */
EAPI void evas_smart_legacy_type_register(const char *type, const Efl_Class *klass) EINA_ARG_NONNULL(1, 2);

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
 * clippees). By default, this base will also move children relative
 * to the parent, and delete them when parent is deleted. In other
 * words, it is the base for simple object grouping.
 *
 * See some @ref Example_Evas_Smart_Objects "examples" of this group
 * of functions.
 *
 * @see evas_object_smart_clipped_smart_set()
 *
 * @ingroup Evas_Smart_Object_Group
 *
 * @{
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
 *
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
 * Current version for Evas box object smart class, a value that goes
 * to _Evas_Object_Box_Api::version.
 *
 * @ingroup Evas_Object_Box
 */
#define EVAS_OBJECT_BOX_API_VERSION 1

/**
 * @struct _Evas_Object_Box_Api
 *
 * This structure should be used by any smart class inheriting from
 * the box's one, to provide custom box behavior that could not be
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
 * Initialize to zero out a whole #Evas_Object_Box_Api structure.
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
 * Initialize to zero out a whole #Evas_Object_Box_Api structure and
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
 * Initialize to zero out a whole #Evas_Object_Box_Api structure and
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
 * @defgroup Evas_Cserve Shared Image Cache Server
 * @ingroup Evas
 *
 * Evas has an (optional) module that provides client-server
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
 * @ingroup Evas
 *
 * Some functions that are handy but are not specific of canvas or
 * objects.
 */

/**
 * Converts the given Evas image load error code into a string
 * describing it in human-readable text.
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
 * bogus_path a path to a file that does not exist, the two outputs
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
 *
 * @param  str The string to get the length of.
 * @return The length in characters (not bytes)
 *
 * @ingroup Evas_Utils
 */
EAPI int  evas_string_char_len_get(const char *str) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Get language direction.
 *
 * @ingroup Evas_Utils
 * @since 1.20
 */
EAPI Evas_BiDi_Direction     evas_language_direction_get(void);

/**
 * Reinitialize language from the environment.
 *
 * The locale can change while a process is running. This call tells evas to
 * reload the locale from the environment like it does on start.
 *
 * @ingroup Evas_Utils
 * @since 1.18
 */
EAPI void                    evas_language_reinit(void);

/**
 * @defgroup Evas_Keys Key Input Functions
 *
 * Functions that feed key events to the canvas.
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
 * @ingroup Evas_Font_Group
 *
 * @{
 */

/**
 * @defgroup Evas_Font_Path_Group Font Path Functions
 *
 * Functions that edit the paths being used to load fonts.
 *
 * @ingroup Evas_Font_Group
 */

/**
 * Removes all font paths loaded into memory by evas_font_path_app_* APIs
 * for the application.
 * @ingroup Evas_Font_Path_Group
 * @since 1.9
 */
EAPI void                    evas_font_path_global_clear(void);

/**
 * Appends a font path to the list of font paths used by the application.
 * @param   path The new font path.
 * @ingroup Evas_Font_Path_Group
 * @since 1.9
 */
EAPI void                    evas_font_path_global_append(const char *path) EINA_ARG_NONNULL(1);

/**
 * Prepends a font path to the list of font paths used by the application.
 * @param   path The new font path.
 * @ingroup Evas_Font_Path_Group
 * @since 1.9
 */
EAPI void                    evas_font_path_global_prepend(const char *path) EINA_ARG_NONNULL(1);

/**
 * Retrieves the list of font paths used by the application.
 * @return  The list of font paths used.
 * @ingroup Evas_Font_Path_Group
 * @since 1.9
 */
EAPI const Eina_List        *evas_font_path_global_list(void) EINA_WARN_UNUSED_RESULT;

/**
 * Reinitialize FontConfig. If FontConfig has to be reinitialized
 * according to changes of system environments (e.g. Changing font config files), it will be useful.
 *
 * @ingroup Evas_Font_Path_Group
 * @since 1.14
 */
EAPI void                    evas_font_reinit(void);

#ifndef _EFL_ANIMATION_EO_CLASS_TYPE
#define _EFL_ANIMATION_EO_CLASS_TYPE

typedef Eo Efl_Animation;

#endif

#ifndef _EFL_ANIMATION_ALPHA_EO_CLASS_TYPE
#define _EFL_ANIMATION_ALPHA_EO_CLASS_TYPE

typedef Eo Efl_Animation_Alpha;

#endif

#ifndef _EFL_ANIMATION_ROTATE_EO_CLASS_TYPE
#define _EFL_ANIMATION_ROTATE_EO_CLASS_TYPE

typedef Eo Efl_Animation_Rotate;

#endif

#ifndef _EFL_ANIMATION_SCALE_EO_CLASS_TYPE
#define _EFL_ANIMATION_SCALE_EO_CLASS_TYPE

typedef Eo Efl_Animation_Scale;

#endif

#ifndef _EFL_ANIMATION_TRANSLATE_EO_CLASS_TYPE
#define _EFL_ANIMATION_TRANSLATE_EO_CLASS_TYPE

typedef Eo Efl_Animation_Translate;

#endif

#ifndef _EFL_ANIMATION_GROUP_EO_CLASS_TYPE
#define _EFL_ANIMATION_GROUP_EO_CLASS_TYPE

typedef Eo Efl_Animation_Group;

#endif

#ifndef _EFL_ANIMATION_GROUP_PARALLEL_EO_CLASS_TYPE
#define _EFL_ANIMATION_GROUP_PARALLEL_EO_CLASS_TYPE

typedef Eo Efl_Animation_Group_Parallel;

#endif

#ifndef _EFL_ANIMATION_GROUP_SEQUENTIAL_EO_CLASS_TYPE
#define _EFL_ANIMATION_GROUP_SEQUENTIAL_EO_CLASS_TYPE

typedef Eo Efl_Animation_Group_Sequential;

#endif

#ifndef _EFL_ANIMATION_PLAYER_EO_CLASS_TYPE
#define _EFL_ANIMATION_PLAYER_EO_CLASS_TYPE

typedef Eo Efl_Animation_Player;

#endif

#ifndef _EFL_ANIMATION_PLAYER_ALPHA_EO_CLASS_TYPE
#define _EFL_ANIMATION_PLAYER_ALPHA_EO_CLASS_TYPE

typedef Eo Efl_Animation_Player_Alpha;

#endif

#ifndef _EFL_ANIMATION_PLAYER_ROTATE_EO_CLASS_TYPE
#define _EFL_ANIMATION_PLAYER_ROTATE_EO_CLASS_TYPE

typedef Eo Efl_Animation_Player_Rotate;

#endif

#ifndef _EFL_ANIMATION_PLAYER_SCALE_EO_CLASS_TYPE
#define _EFL_ANIMATION_PLAYER_SCALE_EO_CLASS_TYPE

typedef Eo Efl_Animation_Player_Scale;

#endif

#ifndef _EFL_ANIMATION_PLAYER_TRANSLATE_EO_CLASS_TYPE
#define _EFL_ANIMATION_PLAYER_TRANSLATE_EO_CLASS_TYPE

typedef Eo Efl_Animation_Player_Translate;

#endif

#ifndef _EFL_ANIMATION_PLAYER_GROUP_EO_CLASS_TYPE
#define _EFL_ANIMATION_PLAYER_GROUP_EO_CLASS_TYPE

typedef Eo Efl_Animation_Player_Group;

#endif

#ifndef _EFL_ANIMATION_GROUP_PARALLEL_EO_CLASS_TYPE
#define _EFL_ANIMATION_GROUP_PARALLEL_EO_CLASS_TYPE

typedef Eo Efl_Animation_Group_Parallel;

#endif

#ifndef _EFL_ANIMATION_GROUP_SEQUENTIAL_EO_CLASS_TYPE
#define _EFL_ANIMATION_GROUP_SEQUENTIAL_EO_CLASS_TYPE

typedef Eo Efl_Animation_Group_Sequential;

#endif

struct _Efl_Animation_Player_Running_Event_Info
{
   double progress;
};

#define EFL_ANIMATION_GROUP_DURATION_NONE -1
#define EFL_ANIMATION_PLAYER_GROUP_DURATION_NONE -1

#define EFL_ANIMATION_REPEAT_INFINITE -1
#define EFL_ANIMATION_PLAYER_REPEAT_INFINITE -1

/**
 * @}
 */

