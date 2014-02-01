#ifndef _EVAS_H
# error You shall not include this header directly
#endif

#define EVAS_VERSION_MAJOR EFL_VERSION_MAJOR
#define EVAS_VERSION_MINOR EFL_VERSION_MINOR

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
   EVAS_EVENT_FLAG_ON_HOLD = (1 << 0), /**< This event is being delivered but should be put "on hold" until the on hold flag is unset. The event should be used for informational purposes and maybe some indications visually, but not actually perform anything */
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

typedef struct _Evas_Coord_Size            Evas_Coord_Size;    /** <Evas_Coord size @since 1.8 */
typedef struct _Evas_Coord_Precision_Size  Evas_Coord_Precision_Size;    /** <Evas_Coord size with sub-pixel precision @since 1.8 */

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

struct _Evas_Coord_Point
{
   Evas_Coord x; /**< x co-ordinate */
   Evas_Coord y; /**< y co-ordinate */
};

struct _Evas_Coord_Size
{
   Evas_Coord w; /**< width */
   Evas_Coord h; /**< height */
};

struct _Evas_Coord_Precision_Size
{
   Evas_Coord w; /** < width */
   Evas_Coord h; /** < height */
   double wsub;  /** < subpixel precision for width */
   double ysub;  /** < subpixel precision for height */
};

struct _Evas_Coord_Precision_Point
{
   Evas_Coord x, y;
   double     xsub, ysub;
};

struct _Evas_Point
{
   int x, y;
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
   EVAS_DISPLAY_MODE_COMPRESS = 1, /**< Use this mode when you want to give compress display mode hint to an object */
   EVAS_DISPLAY_MODE_EXPAND = 2, /**< Use this mode when you want to give expand display mode hint to an object */
   EVAS_DISPLAY_MODE_DONT_CHANGE = 3 /**< Use this mode when an object should not change its display mode */
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
typedef struct _Evas_Event_Render_Post   Evas_Event_Render_Post; /**< Event structure that may come with #EVAS_CALLBACK_RENDER_POST event callbacks @since 1.8 */

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

typedef enum _Evas_Video_Surface_Caps
{
   EVAS_VIDEO_SURFACE_MOVE = 1,
   EVAS_VIDEO_SURFACE_RESIZE = 2,
   EVAS_VIDEO_SURFACE_CLIP = 4,
   EVAS_VIDEO_SURFACE_BELOW = 8,
   EVAS_VIDEO_SURFACE_STACKING_CHECK = 16,
   EVAS_VIDEO_SURFACE_IGNORE_WINDOW = 32,
} Evas_Video_Surface_Caps;

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
} Evas_Device_Class; /**< A general class of device @since 1.8 */

typedef enum _Evas_Device_Subclass
{
   EVAS_DEVICE_SUBCLASS_NONE, /**< Not a device @since 1.8 */
   EVAS_DEVICE_SUBCLASS_FINGER, /**< The normal flat of your finger @since 1.8 */
   EVAS_DEVICE_SUBCLASS_FINGERNAIL, /**< A fingernail @since 1.8 */
   EVAS_DEVICE_SUBCLASS_KNUCKLE, /**< A Knuckle @since 1.8 */
   EVAS_DEVICE_SUBCLASS_PALM, /**< The palm of a users hand @since 1.8 */
   EVAS_DEVICE_SUBCLASS_HAND_SIZE, /**< The side of your hand @since 1.8 */
   EVAS_DEVICE_SUBCLASS_HAND_FLAT, /**< The flat of your hand @since 1.8 */
   EVAS_DEVICE_SUBCLASS_PEN_TIP, /**< The tip of a pen @since 1.8 */
   EVAS_DEVICE_SUBCLASS_TRACKPAD, /**< A trackpad style mouse @since 1.8 */
   EVAS_DEVICE_SUBCLASS_TRACKPOINT, /**< A trackpoint style mouse @since 1.8 */
   EVAS_DEVICE_SUBCLASS_TRACKBALL, /**< A trackball style mouse @since 1.8 */
} Evas_Device_Subclass; /**< A general class of device @since 1.8 */

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
   Evas_Object      *event_src; /**< The Evas Object which actually triggered the event, used in cases of proxy event propagation */
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
   Evas_Object     *event_src; /**< The Evas Object which actually triggered the event, used in cases of proxy event propagation */
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
   Evas_Object     *event_src; /**< The Evas Object which actually triggered the event, used in cases of proxy event propagation */
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
   Evas_Object     *event_src; /**< The Evas Object which actually triggered the event, used in cases of proxy event propagation */
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
   Evas_Object     *event_src; /**< The Evas Object which actually triggered the event, used in cases of proxy event propagation */
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

struct _Evas_Event_Render_Post /** Send when the frame rendering is done @since 1.8 */
{
   Eina_List *updated_area; /**< A list of rectangle that were updated in the canvas */
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
 * See @ref Ecore.
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
 * @ingroup Evas_Main_Group
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
 * @ingroup Evas_Main_Group
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
 * fonts, throw out pre-rendered data, reduce the complexity of change lists
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
 * @ingroup Evas_Main_Group
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
 * @ingroup Evas_Main_Group
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
 * @ingroup Evas_Main_Group
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
 * Add a new device type
 * 
 * @param e The canvas to create the device node for.
 * 
 * Adds a new device nod to the given canvas @p e. All devices created as
 * part of the canvas @p e will automatically be deleted when the canvas
 * is freed.
 * 
 * @see evas_device_del
 * @since 1.8
 */
EAPI Evas_Device *evas_device_add(Evas *e);

/**
 * Delete a new device type
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
 * @since 1.8
 */
EAPI void evas_device_class_set(Evas_Device *dev, Evas_Device_Class clas);

/**
 * Get the major class of a device
 * 
 * @param dev The devise to query
 * @return The device class to set
 * 
 * This sets the device class set by evas_device_class_set().
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
 * This sets the sub-class of a device whihc gives much more detailed usage
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
 * device (example a fingeron a touchscreen will often emulate a mouse when
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
 * Functions that deals with images at canvas level.
 *
 * @ingroup Evas_Canvas
 */

/**
 * @defgroup Evas_Font_Group Font Functions
 *
 * Functions that deals with fonts.
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
 * objects equally, they have @b types, which define their specific
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
 * Get the smoothing for map rendering
 *
 * This gets smoothing for map rendering.
 *
 * @param m map to get the smooth from. Must not be NULL.
 * @return @c EINA_TRUE if the smooth is enabled, @c EINA_FALSE otherwise.
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
 * Get the alpha flag for map rendering
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
 * This frees a given map @p m and all memory associated with it. You must NOT
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
 * @param a pointer to alpha return
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
 * @defgroup Evas_Object_Group_Find Finding Objects
 *
 * Functions that allows finding objects by their position, name or
 * other properties.
 *
 * @ingroup Evas_Object_Group
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
typedef void (*Evas_Object_Intercept_Focus_Set_Cb)(void *data, Evas_Object *obj, Eina_Bool focus);
typedef void (*Evas_Object_Intercept_Color_Set_Cb)(void *data, Evas_Object *obj, int r, int g, int b, int a);
typedef void (*Evas_Object_Intercept_Clip_Set_Cb)(void *data, Evas_Object *obj, Evas_Object *clip);
typedef void (*Evas_Object_Intercept_Clip_Unset_Cb)(void *data, Evas_Object *obj);

/**
 * @}
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
 * @{
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
 * For more details see @ref evas_textblock_style_page
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
 * Returns the geometry of two cursors ("split cursor"), if logical cursor is
 * between LTR/RTL text, also considering paragraph direction.
 * Upper cursor is shown for the text of the same direction as paragraph,
 * lower cursor - for opposite.
 *
 * Split cursor geometry is valid only  in '|' cursor mode.
 * In this case @c EINA_TRUE is returned and cx2, cy2, cw2, ch2 are set,
 * otherwise it behaves like cursor_geometry_get.
 *
 * @param[in] cur the cursor.
 * @param[out] cx the x of the cursor (or upper cursor)
 * @param[out] cy the y of the cursor (or upper cursor)
 * @param[out] cw the width of the cursor (or upper cursor)
 * @param[out] ch the height of the cursor (or upper cursor)
 * @param[out] cx2 the x of the lower cursor
 * @param[out] cy2 the y of the lower cursor
 * @param[out] cw2 the width of the lower cursor
 * @param[out] ch2 the height of the lower cursor
 * @param[in] ctype the type of the cursor.
 * @return @c EINA_TRUE for split cursor, @c EINA_FALSE otherwise
 * @since 1.8
 */
EAPI Eina_Bool
evas_textblock_cursor_geometry_bidi_get(const Evas_Textblock_Cursor *cur, Evas_Coord *cx, Evas_Coord *cy, Evas_Coord *cw, Evas_Coord *ch, Evas_Coord *cx2, Evas_Coord *cy2, Evas_Coord *cw2, Evas_Coord *ch2, Evas_Textblock_Cursor_Type ctype);

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

/**
 * @typedef Evas_Textgrid_Palette
 *
 * The palette to use for the foreground and background colors.
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
 * Registers an object type and its associated class. LEGACY MECHANISM SUPPORT.
 *
 * This function is invoked in the class constructor of smart classes. It will
 * add the type and the class into a hash table that will then be used to check
 * the type of an object.
 * This function has been implemented to support legacy mechanism that checks
 * objects types by name.
 * USE IT ONLY FOR LEGACY SUPPORT.
 * Otherwise, it is HIGHLY recommended to use eo_isa.
 *
 * @param type The type (name string) to add.
 * @param klass The class to associate to the type.
 *
 * @see eo_isa
 *
 * @ingroup Evas_Smart_Object_Group
 */
EAPI void evas_smart_legacy_type_register(const char *type, const Eo_Class *klass) EINA_ARG_NONNULL(1, 2);

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
 * @ingroup Evas
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

typedef Eo Evas_Out;
