#ifndef _EVAS_H
#define _EVAS_H

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


/**
 * @file
 * @brief These routines are used for Evas library interaction.
 *
 * @todo check boolean return values and convert to Eina_Bool
 * @todo change all api to use EINA_SAFETY_*
 * @todo finish api documentation
 */

#include <Evas_Data.h>

typedef enum _Evas_Callback_Type
{
   EVAS_CALLBACK_MOUSE_IN, /**< Mouse In Event */
   EVAS_CALLBACK_MOUSE_OUT, /**< Mouse Out Event */
   EVAS_CALLBACK_MOUSE_DOWN, /**< Mouse Button Down Event */
   EVAS_CALLBACK_MOUSE_UP, /**< Mouse Button Up Event */
   EVAS_CALLBACK_MOUSE_MOVE, /**< Mouse Move Event */
   EVAS_CALLBACK_MOUSE_WHEEL, /**< Mouse Wheel Event */
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
   EVAS_CALLBACK_IMAGE_PRELOADED /**< Image as been preloaded */
} Evas_Callback_Type; /**< The type of event to trigger the callback */

typedef enum _Evas_Button_Flags
{
   EVAS_BUTTON_NONE = 0, /**< No extra mouse button data */
   EVAS_BUTTON_DOUBLE_CLICK = (1 << 0), /**< This mouse button press was the 2nd press of a double click */
   EVAS_BUTTON_TRIPLE_CLICK = (1 << 1) /**< This mouse button press was the 3rd press of a triple click */
} Evas_Button_Flags; /**< Flags for Mouse Button events */

typedef enum _Evas_Event_Flags
{
   EVAS_EVENT_FLAG_NONE = 0, /**< No fancy flags set */
   EVAS_EVENT_FLAG_ON_HOLD = (1 << 0) /**< This event is being delivered but should be put "on hold" until the on hold flag is unset. the event should be used for informational purposes and maybe some indications visually, but not actually perform anything */
} Evas_Event_Flags; /**< Flags for Events */

typedef enum _Evas_Font_Hinting_Flags
{
   EVAS_FONT_HINTING_NONE, /**< No font hinting */
   EVAS_FONT_HINTING_AUTO, /**< Automatic font hinting */
   EVAS_FONT_HINTING_BYTECODE /**< Bytecode font hinting */
} Evas_Font_Hinting_Flags; /**< Flags for Font Hinting */

typedef enum _Evas_Colorspace
{
   EVAS_COLORSPACE_ARGB8888, /**< ARGB 32 bits per pixel, high-byte is Alpha, accessed 1 32bit word at a time */
     /* these are not currently supported - but planned for the future */
   EVAS_COLORSPACE_YCBCR422P601_PL, /**< YCbCr 4:2:2 Planar, ITU.BT-601 specifications. The data poitned to is just an array of row pointer, pointing to the Y rows, then the Cb, then Cr rows */
   EVAS_COLORSPACE_YCBCR422P709_PL,/**< YCbCr 4:2:2 Planar, ITU.BT-709 specifications. The data poitned to is just an array of row pointer, pointing to the Y rows, then the Cb, then Cr rows */
   EVAS_COLORSPACE_RGB565_A5P /**< 16bit rgb565 + Alpha plane at end - 5 bits of the 8 being used per alpha byte */
} Evas_Colorspace; /**< Colorspaces for pixel data supported by Evas */

typedef enum _Evas_Object_Table_Homogeneous_Mode
{
  EVAS_OBJECT_TABLE_HOMOGENEOUS_NONE = 0,
  EVAS_OBJECT_TABLE_HOMOGENEOUS_TABLE = 1,
  EVAS_OBJECT_TABLE_HOMOGENEOUS_ITEM = 2
} Evas_Object_Table_Homogeneous_Mode;

typedef struct _Evas_Transform Evas_Transform; /**< An Evas projective or affine transform */
typedef struct _Evas_Rectangle        Evas_Rectangle; /**< A generic rectangle handle */
typedef struct _Evas_Coord_Rectangle  Evas_Coord_Rectangle; /**< A generic rectangle handle */
typedef struct _Evas_Smart_Class      Evas_Smart_Class; /**< A smart object base class */

typedef struct _Evas Evas; /**< An Evas canvas handle */
typedef struct _Evas_Object Evas_Object; /**< An Evas Object handle */
typedef void Evas_Performance; /**< An Evas Performance handle */
typedef struct _Evas_Modifier Evas_Modifier; /**< An Evas Modifier */
typedef struct _Evas_Lock Evas_Lock; /**< An Evas Lock */
typedef struct _Evas_Smart Evas_Smart; /**< An Evas Smart Object handle */
typedef struct _Evas_Native_Surface Evas_Native_Surface; /**< A generic datatype for engine specific native surface information */
typedef unsigned long long Evas_Modifier_Mask; /**< An Evas modifier mask type */

typedef int           Evas_Coord;
typedef int           Evas_Font_Size;
typedef int           Evas_Angle;

struct _Evas_Transform /** An affine or projective coordinate transformation matrix */
{
   float mxx, mxy, mxz;
   float myx, myy, myz;
   float mzx, mzy, mzz;
};

struct _Evas_Rectangle /** A rectangle */
{
   int x; /**< top-left x co-ordinate of rectangle */
   int y; /**< top-left y co-ordinate of rectangle */
   int w; /**< width of rectangle */
   int h; /**< height of rectangle */
};

struct _Evas_Coord_Rectangle /** A rectangle in Evas_Coord */
{
   Evas_Coord x; /**< top-left x co-ordinate of rectangle */
   Evas_Coord y; /**< top-left y co-ordinate of rectangle */
   Evas_Coord w; /**< width of rectangle */
   Evas_Coord h; /**< height of rectangle */
};

typedef enum _Evas_Aspect_Control
{
   EVAS_ASPECT_CONTROL_NONE = 0,
   EVAS_ASPECT_CONTROL_NEITHER = 1,
   EVAS_ASPECT_CONTROL_HORIZONTAL = 2,
   EVAS_ASPECT_CONTROL_VERTICAL = 3,
   EVAS_ASPECT_CONTROL_BOTH = 4
} Evas_Aspect_Control;


#define EVAS_SMART_CLASS_VERSION 3 /** the version you have to put into the version field in the smart class struct */
struct _Evas_Smart_Class /** a smart object class */
{
   const char *name; /** the string name of the class */
   
   int version;

   void  (*add)         (Evas_Object *o);
   void  (*del)         (Evas_Object *o);
   void  (*move)        (Evas_Object *o, Evas_Coord x, Evas_Coord y);
   void  (*resize)      (Evas_Object *o, Evas_Coord w, Evas_Coord h); 
   void  (*show)        (Evas_Object *o); // FIXME: DELETE ME
   void  (*hide)        (Evas_Object *o); // FIXME: DELETE ME
   void  (*color_set)   (Evas_Object *o, int r, int g, int b, int a); // FIXME: DELETE ME
   void  (*clip_set)    (Evas_Object *o, Evas_Object *clip); // FIXME: DELETE ME
   void  (*clip_unset)  (Evas_Object *o); // FIXME: DELETE ME
   void  (*calculate)   (Evas_Object *o);
   void  (*member_add)  (Evas_Object *o, Evas_Object *child);
   void  (*member_del)  (Evas_Object *o, Evas_Object *child);

   const void *data;
};

/**
 * Initializer to zero a whole Evas_Smart_Class structure.
 *
 * @see EVAS_SMART_CLASS_INIT_VERSION
 * @see EVAS_SMART_CLASS_INIT_NAME_VERSION
 */
#define EVAS_SMART_CLASS_INIT_NULL {NULL, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}

/**
 * Initializer to zero a whole Evas_Smart_Class structure and set version.
 *
 * Similar to EVAS_SMART_CLASS_INIT_NULL, but will set version field to
 * latest EVAS_SMART_CLASS_VERSION.
 *
 * @see EVAS_SMART_CLASS_INIT_NULL
 * @see EVAS_SMART_CLASS_INIT_NAME_VERSION
 */
#define EVAS_SMART_CLASS_INIT_VERSION {NULL, EVAS_SMART_CLASS_VERSION, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}

/**
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
 */
#define EVAS_SMART_CLASS_INIT_NAME_VERSION(name) {name, EVAS_SMART_CLASS_VERSION, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}



typedef struct _Evas_Pixel_Import_Source Evas_Pixel_Import_Source; /**< A source description of pixels for importing pixels */
typedef struct _Evas_Engine_Info      Evas_Engine_Info; /**< A generic Evas Engine information structure */
typedef struct _Evas_Event_Mouse_Down Evas_Event_Mouse_Down; /**< Event structure for #EVAS_CALLBACK_MOUSE_DOWN event callbacks */
typedef struct _Evas_Event_Mouse_Up   Evas_Event_Mouse_Up; /**< Event structure for #EVAS_CALLBACK_MOUSE_UP event callbacks */
typedef struct _Evas_Event_Mouse_In   Evas_Event_Mouse_In; /**< Event structure for #EVAS_CALLBACK_MOUSE_IN event callbacks */
typedef struct _Evas_Event_Mouse_Out  Evas_Event_Mouse_Out; /**< Event structure for #EVAS_CALLBACK_MOUSE_OUT event callbacks */
typedef struct _Evas_Event_Mouse_Move Evas_Event_Mouse_Move; /**< Event structure for #EVAS_CALLBACK_MOUSE_MOVE event callbacks */
typedef struct _Evas_Event_Mouse_Wheel Evas_Event_Mouse_Wheel; /**< Event structure for #EVAS_CALLBACK_MOUSE_WHEEL event callbacks */
typedef struct _Evas_Event_Key_Down   Evas_Event_Key_Down; /**< Event structure for #EVAS_CALLBACK_KEY_DOWN event callbacks */
typedef struct _Evas_Event_Key_Up     Evas_Event_Key_Up; /**< Event structure for #EVAS_CALLBACK_KEY_UP event callbacks */
typedef struct _Evas_Event_Hold       Evas_Event_Hold; /**< Event structure for #EVAS_CALLBACK_HOLD event callbacks */

#define EVAS_LOAD_ERROR_NONE                       0 /**< No error on load */
#define EVAS_LOAD_ERROR_GENERIC                    1 /**< A non-specific error occured */
#define EVAS_LOAD_ERROR_DOES_NOT_EXIST             2 /**< File (or file path) does not exist */
#define EVAS_LOAD_ERROR_PERMISSION_DENIED          3 /**< Permission deinied to an existing file (or path) */
#define EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED 4 /**< Allocation of resources failure prevented load */
#define EVAS_LOAD_ERROR_CORRUPT_FILE               5 /**< File corrupt (but was detected as a known format) */
#define EVAS_LOAD_ERROR_UNKNOWN_FORMAT             6 /**< File is not a known format */

#define EVAS_ALLOC_ERROR_NONE                      0 /**< No allocation error */
#define EVAS_ALLOC_ERROR_FATAL                     1 /**< Allocation failed despite attempts to free up memory */
#define EVAS_ALLOC_ERROR_RECOVERED                 2 /**< Allocation succeeded, but extra memory had to be found by freeing up speculative resources */

#define EVAS_TEXT_INVALID -1
#define EVAS_TEXT_SPECIAL -2

struct _Evas_Pixel_Import_Source
{
   int format; /**< pixel format type ie ARGB32, YUV420P_601 etc. */
   int w, h; /**< width and height of source in pixels */
   void **rows; /**< an array of pointers (size depends on format) pointing to left edge of each scanline */
};

struct _Evas_Native_Surface
{
   union {
      void           *p;
      unsigned short  s;
      unsigned int    i;
      unsigned long   l;
   } data;
};

#define EVAS_LAYER_MIN -32768 /**< bottom-most layer number */
#define EVAS_LAYER_MAX 32767  /**< top-most layer number */

#define EVAS_PIXEL_FORMAT_NONE                     0 /**< No pixel format */
#define EVAS_PIXEL_FORMAT_ARGB32                   1 /**< ARGB 32bit pixel format with A in the high byte per 32bit pixel word */
#define EVAS_PIXEL_FORMAT_YUV420P_601              2 /**< YUV 420 Planar format with CCIR 601 color encoding wuth contiguous planes in the order Y, U and V */

#define EVAS_COLOR_SPACE_ARGB                      0 /**< ARGB color space */
#define EVAS_COLOR_SPACE_AHSV                      1 /**< AHSV color space */

#define EVAS_TEXTURE_REFLECT            0 /**< Gradient and image fill tiling mode - tiling reflects */
#define EVAS_TEXTURE_REPEAT             1 /**< tiling repeats */
#define EVAS_TEXTURE_RESTRICT           2 /**< tiling clamps - range offset ignored */
#define EVAS_TEXTURE_RESTRICT_REFLECT   3 /**< tiling clamps and any range offset reflects */
#define EVAS_TEXTURE_RESTRICT_REPEAT    4 /**< tiling clamps and any range offset repeats */
#define EVAS_TEXTURE_PAD                5 /**< tiling extends with end values */

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
} Evas_Render_Op; /**<  */

typedef enum _Evas_Border_Fill_Mode
{
   EVAS_BORDER_FILL_NONE = 0,
   EVAS_BORDER_FILL_DEFAULT = 1,
   EVAS_BORDER_FILL_SOLID = 2
} Evas_Border_Fill_Mode;

struct _Evas_Engine_Info /** Generic engine information. Generic info is useless */
{
   int magic; /**< Magic number */
};

struct _Evas_Event_Mouse_Down /** Mouse button press event */
{
   int button; /**< Mouse button number that went down (1 - 32) */
   struct {
      int x, y;
   } output;
   struct {
      Evas_Coord x, y;
   } canvas;
   void          *data;
   Evas_Modifier *modifiers;
   Evas_Lock     *locks;

   Evas_Button_Flags flags;
   unsigned int      timestamp;
   Evas_Event_Flags  event_flags;
};

struct _Evas_Event_Mouse_Up /** Mouse button release event */
{
   int button; /**< Mouse button number that was raised (1 - 32) */
   struct {
      int x, y;
   } output;
   struct {
      Evas_Coord x, y;
   } canvas;
   void          *data;
   Evas_Modifier *modifiers;
   Evas_Lock     *locks;

   Evas_Button_Flags flags;
   unsigned int      timestamp;
   Evas_Event_Flags  event_flags;
};

struct _Evas_Event_Mouse_In /** Mouse enter event */
{
   int buttons; /**< Button pressed mask, Bits set to 1 are buttons currently pressed (bit 0 = mouse button 1, bit 1 = mouse button 2 etc.) */
   struct {
      int x, y;
   } output;
   struct {
      Evas_Coord x, y;
   } canvas;
   void          *data;
   Evas_Modifier *modifiers;
   Evas_Lock     *locks;
   unsigned int   timestamp;
   Evas_Event_Flags  event_flags;
};

struct _Evas_Event_Mouse_Out /** Mouse leave event */
{
   int buttons; /**< Button pressed mask, Bits set to 1 are buttons currently pressed (bit 0 = mouse button 1, bit 1 = mouse button 2 etc.) */
   struct {
      int x, y;
   } output;
   struct {
      Evas_Coord x, y;
   } canvas;
   void          *data;
   Evas_Modifier *modifiers;
   Evas_Lock     *locks;
   unsigned int   timestamp;
   Evas_Event_Flags  event_flags;
};

struct _Evas_Event_Mouse_Move /** Mouse button down event */
{
   int buttons; /**< Button pressed mask, Bits set to 1 are buttons currently pressed (bit 0 = mouse button 1, bit 1 = mouse button 2 etc.) */
   struct {
      struct {
	 int x, y;
      } output;
      struct {
	 Evas_Coord x, y;
      } canvas;
   } cur, prev;
   void          *data;
   Evas_Modifier *modifiers;
   Evas_Lock     *locks;
   unsigned int   timestamp;
   Evas_Event_Flags  event_flags;
};

struct _Evas_Event_Mouse_Wheel /** Wheel event */
{
   int direction; /* 0 = default up/down wheel FIXME: more wheel types */
   int z; /* ...,-2,-1 = down, 1,2,... = up */

   struct {
      int x, y;
   } output;

   struct {
      Evas_Coord x, y;
   } canvas;

   void		 *data;
   Evas_Modifier *modifiers;
   Evas_Lock	 *locks;
   unsigned int   timestamp;
   Evas_Event_Flags  event_flags;
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
};

struct _Evas_Event_Hold /** Hold change event */
{
   int            hold; /**< The hold flag */
   void          *data;
   
   unsigned int   timestamp;
   Evas_Event_Flags  event_flags;
};

typedef enum _Evas_Object_Pointer_Mode
{
   EVAS_OBJECT_POINTER_MODE_AUTOGRAB, /**< default, X11-like */
   EVAS_OBJECT_POINTER_MODE_NOGRAB
} Evas_Object_Pointer_Mode;

#ifdef __cplusplus
extern "C" {
#endif

   EAPI int               evas_alloc_error                  (void);

   EAPI int               evas_init                         (void);
   EAPI int               evas_shutdown                     (void);

   EAPI Evas             *evas_new                          (void) EINA_WARN_UNUSED_RESULT EINA_MALLOC;
   EAPI void              evas_free                         (Evas *e)  EINA_ARG_NONNULL(1);

   EAPI int               evas_render_method_lookup         (const char *name) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);
   EAPI Eina_List        *evas_render_method_list           (void) EINA_WARN_UNUSED_RESULT;
   EAPI void              evas_render_method_list_free      (Eina_List *list);

   EAPI void              evas_output_method_set            (Evas *e, int render_method) EINA_ARG_NONNULL(1);
   EAPI int               evas_output_method_get            (const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

   EAPI Evas_Engine_Info *evas_engine_info_get              (const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI void              evas_engine_info_set              (Evas *e, Evas_Engine_Info *info) EINA_ARG_NONNULL(1);

   EAPI void              evas_output_size_set              (Evas *e, int w, int h) EINA_ARG_NONNULL(1);
   EAPI void              evas_output_size_get              (const Evas *e, int *w, int *h) EINA_ARG_NONNULL(1);
   EAPI void              evas_output_viewport_set          (Evas *e, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h) EINA_ARG_NONNULL(1);
   EAPI void              evas_output_viewport_get          (const Evas *e, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h) EINA_ARG_NONNULL(1);

   EAPI Evas_Coord        evas_coord_screen_x_to_world      (const Evas *e, int x) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);
   EAPI Evas_Coord        evas_coord_screen_y_to_world      (const Evas *e, int y) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);
   EAPI int               evas_coord_world_x_to_screen      (const Evas *e, Evas_Coord x) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);
   EAPI int               evas_coord_world_y_to_screen      (const Evas *e, Evas_Coord y) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

   EAPI void              evas_pointer_output_xy_get        (const Evas *e, int *x, int *y) EINA_ARG_NONNULL(1);
   EAPI void              evas_pointer_canvas_xy_get        (const Evas *e, Evas_Coord *x, Evas_Coord *y) EINA_ARG_NONNULL(1);
   EAPI int               evas_pointer_button_down_mask_get (const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);
   EAPI Eina_Bool         evas_pointer_inside_get           (const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);
   EAPI void              evas_data_attach_set              (Evas *e, void *data) EINA_ARG_NONNULL(1);
   EAPI void             *evas_data_attach_get              (const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);
       
/* DOC UP TO HERE */
   EAPI void              evas_damage_rectangle_add         (Evas *e, int x, int y, int w, int h) EINA_ARG_NONNULL(1);
   EAPI void              evas_obscured_rectangle_add       (Evas *e, int x, int y, int w, int h) EINA_ARG_NONNULL(1);
   EAPI void              evas_obscured_clear               (Evas *e) EINA_ARG_NONNULL(1);
   EAPI Eina_List        *evas_render_updates               (Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);
   EAPI void              evas_render_updates_free          (Eina_List *updates);
   EAPI void              evas_render                       (Evas *e) EINA_ARG_NONNULL(1);
   EAPI void              evas_norender                     (Evas *e) EINA_ARG_NONNULL(1);
   EAPI void              evas_render_idle_flush            (Evas *e) EINA_ARG_NONNULL(1);
       
/* rectangle objects */
   EAPI Evas_Object      *evas_object_rectangle_add         (Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/* line objects */
   EAPI Evas_Object      *evas_object_line_add              (Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;
   EAPI void              evas_object_line_xy_set           (Evas_Object *obj, Evas_Coord x1, Evas_Coord y1, Evas_Coord x2, Evas_Coord y2);
   EAPI void              evas_object_line_xy_get           (const Evas_Object *obj, Evas_Coord *x1, Evas_Coord *y1, Evas_Coord *x2, Evas_Coord *y2);

/* gradient objects */
   EAPI Evas_Object      *evas_object_gradient_add            (Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;
   EAPI void              evas_object_gradient_color_stop_add (Evas_Object *obj, int r, int g, int b, int a, int delta) EINA_ARG_NONNULL(1);
   EAPI void              evas_object_gradient_alpha_stop_add (Evas_Object *obj, int a, int delta) EINA_ARG_NONNULL(1);
   EAPI void              evas_object_gradient_color_data_set (Evas_Object *obj, void *color_data, int len, Eina_Bool has_alpha) EINA_ARG_NONNULL(1, 2);
   EAPI void              evas_object_gradient_alpha_data_set (Evas_Object *obj, void *alpha_data, int len) EINA_ARG_NONNULL(1, 2);
   EAPI void              evas_object_gradient_clear          (Evas_Object *obj) EINA_ARG_NONNULL(1);
   EAPI void              evas_object_gradient_type_set       (Evas_Object *obj, const char *type, const char *instance_params) EINA_ARG_NONNULL(1, 2);
   EAPI void              evas_object_gradient_type_get       (const Evas_Object *obj, char **type, char **instance_params) EINA_ARG_NONNULL(1, 2);
   EAPI void              evas_object_gradient_fill_set       (Evas_Object *obj, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h) EINA_ARG_NONNULL(1);
   EAPI void              evas_object_gradient_fill_get       (const Evas_Object *obj, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h) EINA_ARG_NONNULL(1);
   EAPI void              evas_object_gradient_fill_angle_set (Evas_Object *obj, Evas_Angle angle) EINA_ARG_NONNULL(1);
   EAPI Evas_Angle        evas_object_gradient_fill_angle_get (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI void              evas_object_gradient_fill_spread_set(Evas_Object *obj, int tile_mode) EINA_ARG_NONNULL(1);
   EAPI int               evas_object_gradient_fill_spread_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI void              evas_object_gradient_angle_set      (Evas_Object *obj, Evas_Angle angle) EINA_ARG_NONNULL(1);
   EAPI Evas_Angle        evas_object_gradient_angle_get      (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI void              evas_object_gradient_direction_set  (Evas_Object *obj, int direction) EINA_ARG_NONNULL(1);
   EAPI int               evas_object_gradient_direction_get  (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI void              evas_object_gradient_offset_set     (Evas_Object *obj, float offset) EINA_ARG_NONNULL(1);
   EAPI float             evas_object_gradient_offset_get     (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

/* new gradient2 objects - generic properties */
   EAPI void              evas_object_gradient2_color_np_stop_insert (Evas_Object *obj, int r, int g, int b, int a, float pos) EINA_ARG_NONNULL(1);
   EAPI void              evas_object_gradient2_fill_spread_set      (Evas_Object *obj, int tile_mode) EINA_ARG_NONNULL(1);
   EAPI int               evas_object_gradient2_fill_spread_get      (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI void              evas_object_gradient2_fill_transform_set   (Evas_Object *obj, Evas_Transform *t) EINA_ARG_NONNULL(1);
   EAPI void              evas_object_gradient2_fill_transform_get   (const Evas_Object *obj, Evas_Transform *t) EINA_ARG_NONNULL(1);

/* linear gradient2 objects */
   EAPI Evas_Object      *evas_object_gradient2_linear_add      (Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;
   EAPI void              evas_object_gradient2_linear_fill_set (Evas_Object *obj, float x0, float y0, float x1, float y1) EINA_ARG_NONNULL(1);
   EAPI void              evas_object_gradient2_linear_fill_get (const Evas_Object *obj, float *x0, float *y0, float *x1, float *y1) EINA_ARG_NONNULL(1);

/* radial gradient2 objects */
   EAPI Evas_Object      *evas_object_gradient2_radial_add      (Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;
   EAPI void              evas_object_gradient2_radial_fill_set (Evas_Object *obj, float cx, float cy, float rx, float ry) EINA_ARG_NONNULL(1);
   EAPI void              evas_object_gradient2_radial_fill_get (const Evas_Object *obj, float *cx, float *cy, float *rx, float *ry) EINA_ARG_NONNULL(1);

/* polygon objects */
   EAPI Evas_Object      *evas_object_polygon_add           (Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;
   EAPI void              evas_object_polygon_point_add     (Evas_Object *obj, Evas_Coord x, Evas_Coord y) EINA_ARG_NONNULL(1);
   EAPI void              evas_object_polygon_points_clear  (Evas_Object *obj) EINA_ARG_NONNULL(1);

/* image objects */
   EAPI Evas_Object      *evas_object_image_add             (Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;
   EAPI Evas_Object      *evas_object_image_filled_add      (Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

   EAPI void              evas_object_image_file_set        (Evas_Object *obj, const char *file, const char *key) EINA_ARG_NONNULL(1, 2);
   EAPI void              evas_object_image_file_get        (const Evas_Object *obj, const char **file, const char **key) EINA_ARG_NONNULL(1, 2);
   EAPI void              evas_object_image_border_set      (Evas_Object *obj, int l, int r, int t, int b) EINA_ARG_NONNULL(1);
   EAPI void              evas_object_image_border_get      (const Evas_Object *obj, int *l, int *r, int *t, int *b) EINA_ARG_NONNULL(1);
   EAPI void              evas_object_image_border_center_fill_set(Evas_Object *obj, Evas_Border_Fill_Mode fill) EINA_ARG_NONNULL(1);
   EAPI Evas_Border_Fill_Mode evas_object_image_border_center_fill_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI void              evas_object_image_filled_set      (Evas_Object *obj, Eina_Bool setting) EINA_ARG_NONNULL(1);
   EAPI Eina_Bool         evas_object_image_filled_get      (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI void              evas_object_image_fill_set        (Evas_Object *obj, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h) EINA_ARG_NONNULL(1);
   EAPI void              evas_object_image_fill_get        (const Evas_Object *obj, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h) EINA_ARG_NONNULL(1);
   EAPI void              evas_object_image_fill_spread_set   (Evas_Object *obj, int tile_mode) EINA_ARG_NONNULL(1);
   EAPI int               evas_object_image_fill_spread_get   (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI void              evas_object_image_fill_transform_set (Evas_Object *obj, Evas_Transform *t) EINA_ARG_NONNULL(1);
   EAPI void              evas_object_image_fill_transform_get (const Evas_Object *obj, Evas_Transform *t) EINA_ARG_NONNULL(1, 2);
   EAPI void              evas_object_image_size_set        (Evas_Object *obj, int w, int h) EINA_ARG_NONNULL(1);
   EAPI void              evas_object_image_size_get        (const Evas_Object *obj, int *w, int *h) EINA_ARG_NONNULL(1);
   EAPI int               evas_object_image_stride_get      (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI int               evas_object_image_load_error_get  (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI void              evas_object_image_data_set        (Evas_Object *obj, void *data) EINA_ARG_NONNULL(1);
   EAPI void             *evas_object_image_data_convert    (Evas_Object *obj, Evas_Colorspace to_cspace) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI void             *evas_object_image_data_get        (const Evas_Object *obj, Eina_Bool for_writing) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI void              evas_object_image_data_copy_set   (Evas_Object *obj, void *data) EINA_ARG_NONNULL(1);
   EAPI void              evas_object_image_data_update_add (Evas_Object *obj, int x, int y, int w, int h) EINA_ARG_NONNULL(1);
   EAPI void              evas_object_image_alpha_set       (Evas_Object *obj, Eina_Bool has_alpha) EINA_ARG_NONNULL(1);
   EAPI Eina_Bool         evas_object_image_alpha_get       (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI void              evas_object_image_smooth_scale_set(Evas_Object *obj, Eina_Bool smooth_scale) EINA_ARG_NONNULL(1);
   EAPI Eina_Bool         evas_object_image_smooth_scale_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI void              evas_object_image_preload         (Evas_Object *obj, Eina_Bool cancel) EINA_ARG_NONNULL(1);
   EAPI void              evas_object_image_reload          (Evas_Object *obj) EINA_ARG_NONNULL(1);
   EAPI Eina_Bool         evas_object_image_save            (const Evas_Object *obj, const char *file, const char *key, const char *flags)  EINA_ARG_NONNULL(1, 2);
   EAPI Eina_Bool         evas_object_image_pixels_import          (Evas_Object *obj, Evas_Pixel_Import_Source *pixels) EINA_ARG_NONNULL(1, 2);
   EAPI void              evas_object_image_pixels_get_callback_set(Evas_Object *obj, void (*func) (void *data, Evas_Object *o), void *data) EINA_ARG_NONNULL(1, 2);
   EAPI void              evas_object_image_pixels_dirty_set       (Evas_Object *obj, Eina_Bool dirty) EINA_ARG_NONNULL(1);
   EAPI Eina_Bool         evas_object_image_pixels_dirty_get       (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI void              evas_object_image_load_dpi_set           (Evas_Object *obj, double dpi) EINA_ARG_NONNULL(1);
   EAPI double            evas_object_image_load_dpi_get           (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI void              evas_object_image_load_size_set          (Evas_Object *obj, int w, int h) EINA_ARG_NONNULL(1);
   EAPI void              evas_object_image_load_size_get          (const Evas_Object *obj, int *w, int *h) EINA_ARG_NONNULL(1);
   EAPI void              evas_object_image_load_scale_down_set    (Evas_Object *obj, int scale_down) EINA_ARG_NONNULL(1);
   EAPI int               evas_object_image_load_scale_down_get    (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI void              evas_object_image_colorspace_set         (Evas_Object *obj, Evas_Colorspace cspace) EINA_ARG_NONNULL(1);
   EAPI Evas_Colorspace   evas_object_image_colorspace_get         (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI void                 evas_object_image_native_surface_set  (Evas_Object *obj, Evas_Native_Surface *surf) EINA_ARG_NONNULL(1, 2);
   EAPI Evas_Native_Surface *evas_object_image_native_surface_get  (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

/* image cache */
   EAPI void              evas_image_cache_flush            (Evas *e) EINA_ARG_NONNULL(1);
   EAPI void              evas_image_cache_reload           (Evas *e) EINA_ARG_NONNULL(1);
   EAPI void              evas_image_cache_set              (Evas *e, int size) EINA_ARG_NONNULL(1);
   EAPI int               evas_image_cache_get              (const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

/* text objects */
   typedef enum _Evas_Text_Style_Type
     {
	EVAS_TEXT_STYLE_PLAIN,
	EVAS_TEXT_STYLE_SHADOW,
	EVAS_TEXT_STYLE_OUTLINE,
	EVAS_TEXT_STYLE_SOFT_OUTLINE,
	EVAS_TEXT_STYLE_GLOW,
	EVAS_TEXT_STYLE_OUTLINE_SHADOW,
	EVAS_TEXT_STYLE_FAR_SHADOW,
	EVAS_TEXT_STYLE_OUTLINE_SOFT_SHADOW,
	EVAS_TEXT_STYLE_SOFT_SHADOW,
	EVAS_TEXT_STYLE_FAR_SOFT_SHADOW
     } Evas_Text_Style_Type;

   EAPI Evas_Object      *evas_object_text_add              (Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;
   EAPI void              evas_object_text_font_source_set  (Evas_Object *obj, const char *font) EINA_ARG_NONNULL(1);
   EAPI const char       *evas_object_text_font_source_get  (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI void              evas_object_text_font_set         (Evas_Object *obj, const char *font, Evas_Font_Size size) EINA_ARG_NONNULL(1);
   EAPI void              evas_object_text_font_get         (const Evas_Object *obj, const char **font, Evas_Font_Size *size) EINA_ARG_NONNULL(1, 2);
   EAPI void              evas_object_text_text_set         (Evas_Object *obj, const char *text) EINA_ARG_NONNULL(1);
   EAPI const char       *evas_object_text_text_get         (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI Evas_Coord        evas_object_text_ascent_get       (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI Evas_Coord        evas_object_text_descent_get      (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI Evas_Coord        evas_object_text_max_ascent_get   (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI Evas_Coord        evas_object_text_max_descent_get  (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI Evas_Coord        evas_object_text_horiz_advance_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI Evas_Coord        evas_object_text_vert_advance_get (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI Evas_Coord        evas_object_text_inset_get        (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI int               evas_object_text_char_pos_get     (const Evas_Object *obj, int pos, Evas_Coord *cx, Evas_Coord *cy, Evas_Coord *cw, Evas_Coord *ch) EINA_ARG_NONNULL(1);
   EAPI int               evas_object_text_char_coords_get  (const Evas_Object *obj, Evas_Coord x, Evas_Coord y, Evas_Coord *cx, Evas_Coord *cy, Evas_Coord *cw, Evas_Coord *ch) EINA_ARG_NONNULL(1);
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
   EAPI void              evas_object_text_style_pad_get    (const Evas_Object *obj, int *l, int *r, int *t, int *b) EINA_ARG_NONNULL(1);

/* string and font handling */
   EAPI int               evas_string_char_next_get         (const char *str, int pos, int *decoded) EINA_ARG_NONNULL(1);
   EAPI int               evas_string_char_prev_get         (const char *str, int pos, int *decoded) EINA_ARG_NONNULL(1);
   EAPI int               evas_string_char_len_get          (const char *str) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

   EAPI void              evas_font_path_clear              (Evas *e) EINA_ARG_NONNULL(1);
   EAPI void              evas_font_path_append             (Evas *e, const char *path) EINA_ARG_NONNULL(1, 2);
   EAPI void              evas_font_path_prepend            (Evas *e, const char *path) EINA_ARG_NONNULL(1, 2);
   EAPI const Eina_List  *evas_font_path_list               (const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   
   EAPI void              evas_font_hinting_set             (Evas *e, Evas_Font_Hinting_Flags hinting) EINA_ARG_NONNULL(1);
   EAPI Evas_Font_Hinting_Flags evas_font_hinting_get       (const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI Eina_Bool         evas_font_hinting_can_hint        (const Evas *e, Evas_Font_Hinting_Flags hinting) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

   EAPI void              evas_font_cache_flush             (Evas *e) EINA_ARG_NONNULL(1);
   EAPI void              evas_font_cache_set               (Evas *e, int size) EINA_ARG_NONNULL(1);
   EAPI int               evas_font_cache_get               (const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

   EAPI Eina_List	 *evas_font_available_list	    (const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI void		  evas_font_available_list_free	    (Evas *e, Eina_List *available) EINA_ARG_NONNULL(1);
   
/* textblock objects */
   typedef struct _Evas_Textblock_Style     Evas_Textblock_Style;
   typedef struct _Evas_Textblock_Cursor    Evas_Textblock_Cursor;
   typedef struct _Evas_Textblock_Rectangle Evas_Textblock_Rectangle;
   
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
   
   EAPI Evas_Object                 *evas_object_textblock_add(Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

   EAPI const char                  *evas_textblock_escape_string_get(const char *escape) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI const char                  *evas_textblock_string_escape_get(const char *string, int *len_ret) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   
   EAPI Evas_Textblock_Style        *evas_textblock_style_new(void) EINA_WARN_UNUSED_RESULT EINA_MALLOC;
   EAPI void                         evas_textblock_style_free(Evas_Textblock_Style *ts) EINA_ARG_NONNULL(1);
   EAPI void                         evas_textblock_style_set(Evas_Textblock_Style *ts, const char *text) EINA_ARG_NONNULL(1, 2);
   EAPI const char                  *evas_textblock_style_get(const Evas_Textblock_Style *ts) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   
   EAPI void                         evas_object_textblock_style_set(Evas_Object *obj, Evas_Textblock_Style *ts) EINA_ARG_NONNULL(1);
   EAPI const Evas_Textblock_Style  *evas_object_textblock_style_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI void                         evas_object_textblock_replace_char_set(Evas_Object *obj, const char *ch) EINA_ARG_NONNULL(1);
   EAPI const char                  *evas_object_textblock_replace_char_get(Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   
   EAPI void                         evas_object_textblock_text_markup_set(Evas_Object *obj, const char *text) EINA_ARG_NONNULL(1);
   EAPI void                         evas_object_textblock_text_markup_prepend(Evas_Textblock_Cursor *cur, const char *text) EINA_ARG_NONNULL(1, 2);
   EAPI const char                  *evas_object_textblock_text_markup_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   
   EAPI const Evas_Textblock_Cursor *evas_object_textblock_cursor_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI Evas_Textblock_Cursor       *evas_object_textblock_cursor_new(Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;
   
   EAPI void                         evas_textblock_cursor_free(Evas_Textblock_Cursor *cur) EINA_ARG_NONNULL(1);
   
   EAPI void                         evas_textblock_cursor_node_first(Evas_Textblock_Cursor *cur) EINA_ARG_NONNULL(1);
   EAPI void                         evas_textblock_cursor_node_last(Evas_Textblock_Cursor *cur) EINA_ARG_NONNULL(1);
   EAPI Eina_Bool                    evas_textblock_cursor_node_next(Evas_Textblock_Cursor *cur) EINA_ARG_NONNULL(1);
   EAPI Eina_Bool                    evas_textblock_cursor_node_prev(Evas_Textblock_Cursor *cur) EINA_ARG_NONNULL(1);
   EAPI Eina_Bool                    evas_textblock_cursor_char_next(Evas_Textblock_Cursor *cur) EINA_ARG_NONNULL(1);
   EAPI Eina_Bool                    evas_textblock_cursor_char_prev(Evas_Textblock_Cursor *cur) EINA_ARG_NONNULL(1);
   EAPI void                         evas_textblock_cursor_char_first(Evas_Textblock_Cursor *cur) EINA_ARG_NONNULL(1);
   EAPI void                         evas_textblock_cursor_char_last(Evas_Textblock_Cursor *cur) EINA_ARG_NONNULL(1);
   EAPI void                         evas_textblock_cursor_line_first(Evas_Textblock_Cursor *cur) EINA_ARG_NONNULL(1);
   EAPI void                         evas_textblock_cursor_line_last(Evas_Textblock_Cursor *cur) EINA_ARG_NONNULL(1);
   EAPI int                          evas_textblock_cursor_pos_get(const Evas_Textblock_Cursor *cur) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI void                         evas_textblock_cursor_pos_set(Evas_Textblock_Cursor *cur, int pos) EINA_ARG_NONNULL(1);
   EAPI Eina_Bool                    evas_textblock_cursor_line_set(Evas_Textblock_Cursor *cur, int line) EINA_ARG_NONNULL(1);
   EAPI int                          evas_textblock_cursor_compare(const Evas_Textblock_Cursor *cur1, const Evas_Textblock_Cursor *cur2) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1, 2) EINA_PURE;
   EAPI void                         evas_textblock_cursor_copy(const Evas_Textblock_Cursor *cur, Evas_Textblock_Cursor *cur_dest) EINA_ARG_NONNULL(1, 2);
   
   EAPI void                         evas_textblock_cursor_text_append(Evas_Textblock_Cursor *cur, const char *text) EINA_ARG_NONNULL(1, 2);
   EAPI void                         evas_textblock_cursor_text_prepend(Evas_Textblock_Cursor *cur, const char *text) EINA_ARG_NONNULL(1, 2);

   EAPI void                         evas_textblock_cursor_format_append(Evas_Textblock_Cursor *cur, const char *format) EINA_ARG_NONNULL(1, 2);
   EAPI void                         evas_textblock_cursor_format_prepend(Evas_Textblock_Cursor *cur, const char *format) EINA_ARG_NONNULL(1, 2);
   EAPI void                         evas_textblock_cursor_node_delete(Evas_Textblock_Cursor *cur) EINA_ARG_NONNULL(1);
   EAPI void                         evas_textblock_cursor_char_delete(Evas_Textblock_Cursor *cur) EINA_ARG_NONNULL(1);
   EAPI void                         evas_textblock_cursor_range_delete(Evas_Textblock_Cursor *cur1, Evas_Textblock_Cursor *cur2) EINA_ARG_NONNULL(1, 2);
   
   EAPI const char                  *evas_textblock_cursor_node_text_get(const Evas_Textblock_Cursor *cur) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI int                          evas_textblock_cursor_node_text_length_get(const Evas_Textblock_Cursor *cur) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI const char                  *evas_textblock_cursor_node_format_get(const Evas_Textblock_Cursor *cur) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI Eina_Bool                    evas_textblock_cursor_node_format_is_visible_get(const Evas_Textblock_Cursor *cur) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI char                        *evas_textblock_cursor_range_text_get(const Evas_Textblock_Cursor *cur1, const Evas_Textblock_Cursor *cur2, Evas_Textblock_Text_Type format) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1, 2) EINA_PURE;
       
   EAPI int                          evas_textblock_cursor_char_geometry_get(const Evas_Textblock_Cursor *cur, Evas_Coord *cx, Evas_Coord *cy, Evas_Coord *cw, Evas_Coord *ch) EINA_ARG_NONNULL(1);
   EAPI int                          evas_textblock_cursor_line_geometry_get(const Evas_Textblock_Cursor *cur, Evas_Coord *cx, Evas_Coord *cy, Evas_Coord *cw, Evas_Coord *ch) EINA_ARG_NONNULL(1);
   EAPI Eina_Bool                    evas_textblock_cursor_char_coord_set(Evas_Textblock_Cursor *cur, Evas_Coord x, Evas_Coord y) EINA_ARG_NONNULL(1);
   EAPI int                          evas_textblock_cursor_line_coord_set(Evas_Textblock_Cursor *cur, Evas_Coord y) EINA_ARG_NONNULL(1);
   EAPI Eina_List                   *evas_textblock_cursor_range_geometry_get(const Evas_Textblock_Cursor *cur1, const Evas_Textblock_Cursor *cur2) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1, 2) EINA_PURE;
   
   EAPI Eina_Bool                    evas_object_textblock_line_number_geometry_get(const Evas_Object *obj, int line, Evas_Coord *cx, Evas_Coord *cy, Evas_Coord *cw, Evas_Coord *ch) EINA_ARG_NONNULL(1);
   EAPI void                         evas_object_textblock_clear(Evas_Object *obj) EINA_ARG_NONNULL(1);
   EAPI void                         evas_object_textblock_size_formatted_get(const Evas_Object *obj, Evas_Coord *w, Evas_Coord *h) EINA_ARG_NONNULL(1);
   EAPI void                         evas_object_textblock_size_native_get(const Evas_Object *obj, Evas_Coord *w, Evas_Coord *h) EINA_ARG_NONNULL(1);
   EAPI void                         evas_object_textblock_style_insets_get(const Evas_Object *obj, Evas_Coord *l, Evas_Coord *r, Evas_Coord *t, Evas_Coord *b) EINA_ARG_NONNULL(1);
   
   
/* general objects */
   EAPI void              evas_object_del                   (Evas_Object *obj) EINA_ARG_NONNULL(1);

   EAPI const char       *evas_object_type_get              (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

   EAPI void              evas_object_layer_set             (Evas_Object *obj, short l) EINA_ARG_NONNULL(1);
   EAPI short             evas_object_layer_get             (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

   EAPI void              evas_object_raise                 (Evas_Object *obj) EINA_ARG_NONNULL(1);
   EAPI void              evas_object_lower                 (Evas_Object *obj) EINA_ARG_NONNULL(1);
   EAPI void              evas_object_stack_above           (Evas_Object *obj, Evas_Object *above) EINA_ARG_NONNULL(1, 2);
   EAPI void              evas_object_stack_below           (Evas_Object *obj, Evas_Object *below) EINA_ARG_NONNULL(1, 2);
   EAPI Evas_Object      *evas_object_above_get             (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI Evas_Object      *evas_object_below_get             (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI Evas_Object      *evas_object_bottom_get            (const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI Evas_Object      *evas_object_top_get               (const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

   EAPI void              evas_object_move                  (Evas_Object *obj, Evas_Coord x, Evas_Coord y) EINA_ARG_NONNULL(1);
   EAPI void              evas_object_resize                (Evas_Object *obj, Evas_Coord w, Evas_Coord h) EINA_ARG_NONNULL(1);
   EAPI void              evas_object_geometry_get          (const Evas_Object *obj, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h) EINA_ARG_NONNULL(1);

   EAPI void              evas_object_size_hint_min_get     (const Evas_Object *obj, Evas_Coord *w, Evas_Coord *h) EINA_ARG_NONNULL(1);
   EAPI void              evas_object_size_hint_min_set     (Evas_Object *obj, Evas_Coord w, Evas_Coord h) EINA_ARG_NONNULL(1);
   EAPI void              evas_object_size_hint_max_get     (const Evas_Object *obj, Evas_Coord *w, Evas_Coord *h) EINA_ARG_NONNULL(1);
   EAPI void              evas_object_size_hint_max_set     (Evas_Object *obj, Evas_Coord w, Evas_Coord h) EINA_ARG_NONNULL(1);
   EAPI void              evas_object_size_hint_request_get (const Evas_Object *obj, Evas_Coord *w, Evas_Coord *h) EINA_ARG_NONNULL(1);
   EAPI void              evas_object_size_hint_request_set (Evas_Object *obj, Evas_Coord w, Evas_Coord h) EINA_ARG_NONNULL(1);
   EAPI void              evas_object_size_hint_aspect_get  (const Evas_Object *obj, Evas_Aspect_Control *aspect, Evas_Coord *w, Evas_Coord *h) EINA_ARG_NONNULL(1);
   EAPI void              evas_object_size_hint_aspect_set  (Evas_Object *obj, Evas_Aspect_Control aspect, Evas_Coord w, Evas_Coord h) EINA_ARG_NONNULL(1);
   EAPI void              evas_object_size_hint_align_get   (const Evas_Object *obj, double *x, double *y) EINA_ARG_NONNULL(1);
   EAPI void              evas_object_size_hint_align_set   (Evas_Object *obj, double x, double y) EINA_ARG_NONNULL(1);
   EAPI void              evas_object_size_hint_weight_get  (const Evas_Object *obj, double *x, double *y) EINA_ARG_NONNULL(1);
   EAPI void              evas_object_size_hint_weight_set  (Evas_Object *obj, double x, double y) EINA_ARG_NONNULL(1);
   EAPI void              evas_object_size_hint_padding_get (const Evas_Object *obj, Evas_Coord *l, Evas_Coord *r, Evas_Coord *t, Evas_Coord *b) EINA_ARG_NONNULL(1);
   EAPI void              evas_object_size_hint_padding_set (Evas_Object *obj, Evas_Coord l, Evas_Coord r, Evas_Coord t, Evas_Coord b) EINA_ARG_NONNULL(1);
       
   EAPI void              evas_object_show                  (Evas_Object *obj) EINA_ARG_NONNULL(1);
   EAPI void              evas_object_hide                  (Evas_Object *obj) EINA_ARG_NONNULL(1);
   EAPI Eina_Bool         evas_object_visible_get           (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

   EAPI void              evas_object_render_op_set         (Evas_Object *obj, Evas_Render_Op op) EINA_ARG_NONNULL(1);
   EAPI Evas_Render_Op    evas_object_render_op_get         (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

   EAPI void              evas_object_anti_alias_set        (Evas_Object *obj, Eina_Bool antialias) EINA_ARG_NONNULL(1);
   EAPI Eina_Bool         evas_object_anti_alias_get        (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

   EAPI void              evas_object_scale_set             (Evas_Object *obj, double scale) EINA_ARG_NONNULL(1);
   EAPI double            evas_object_scale_get             (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

   EAPI void              evas_object_color_set             (Evas_Object *obj, int r, int g, int b, int a) EINA_ARG_NONNULL(1);
   EAPI void              evas_object_color_get             (const Evas_Object *obj, int *r, int *g, int *b, int *a) EINA_ARG_NONNULL(1);

   EAPI void              evas_object_color_interpolation_set  (Evas_Object *obj, int color_space) EINA_ARG_NONNULL(1);
   EAPI int               evas_object_color_interpolation_get  (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

   EAPI void              evas_object_clip_set              (Evas_Object *obj, Evas_Object *clip) EINA_ARG_NONNULL(1, 2);
   EAPI Evas_Object      *evas_object_clip_get              (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI void              evas_object_clip_unset            (Evas_Object *obj);
   EAPI const Eina_List  *evas_object_clipees_get           (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

   EAPI void              evas_object_data_set              (Evas_Object *obj, const char *key, const void *data) EINA_ARG_NONNULL(1, 2);
   EAPI void             *evas_object_data_get              (const Evas_Object *obj, const char *key) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1, 2) EINA_PURE;
   EAPI void             *evas_object_data_del              (Evas_Object *obj, const char *key) EINA_ARG_NONNULL(1, 2);

   EAPI void              evas_object_name_set              (Evas_Object *obj, const char *name) EINA_ARG_NONNULL(1);
   EAPI const char       *evas_object_name_get              (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI Evas_Object      *evas_object_name_find             (const Evas *e, const char *name) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

   EAPI Evas             *evas_object_evas_get              (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

   EAPI Evas_Object      *evas_object_top_at_xy_get         (const Evas *e, Evas_Coord x, Evas_Coord y, Eina_Bool include_pass_events_objects, Eina_Bool include_hidden_objects) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI Evas_Object      *evas_object_top_at_pointer_get    (const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI Evas_Object      *evas_object_top_in_rectangle_get  (const Evas *e, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h, Eina_Bool include_pass_events_objects, Eina_Bool include_hidden_objects) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

   EAPI Eina_List        *evas_objects_at_xy_get            (const Evas *e, Evas_Coord x, Evas_Coord y, Eina_Bool include_pass_events_objects, Eina_Bool include_hidden_objects) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI Eina_List        *evas_objects_in_rectangle_get     (const Evas *e, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h, Eina_Bool include_pass_events_objects, Eina_Bool include_hidden_objects) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

/* smart objects */
   EAPI Evas_Smart       *evas_smart_new                    (const char *name, void (*func_add) (Evas_Object *obj), void (*func_del) (Evas_Object *obj), void (*func_layer_set) (Evas_Object *obj, int l), void (*func_raise) (Evas_Object *obj), void (*func_lower) (Evas_Object *obj), void (*func_stack_above) (Evas_Object *obj, Evas_Object *above), void (*func_stack_below) (Evas_Object *obj, Evas_Object *below), void (*func_move) (Evas_Object *obj, Evas_Coord x, Evas_Coord y), void (*func_resize) (Evas_Object *obj, Evas_Coord w, Evas_Coord h), void (*func_show) (Evas_Object *obj), void (*func_hide) (Evas_Object *obj), void (*func_color_set) (Evas_Object *obj, int r, int g, int b, int a), void (*func_clip_set) (Evas_Object *obj, Evas_Object *clip), void (*func_clip_unset) (Evas_Object *obj), const void *data) EINA_DEPRECATED  EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;
   EAPI void              evas_smart_free                   (Evas_Smart *s) EINA_ARG_NONNULL(1);
   EAPI Evas_Smart       *evas_smart_class_new              (const Evas_Smart_Class *sc) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;
   EAPI const Evas_Smart_Class *evas_smart_class_get        (const Evas_Smart *s) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

   EAPI void             *evas_smart_data_get               (const Evas_Smart *s) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

   EAPI Evas_Object      *evas_object_smart_add             (Evas *e, Evas_Smart *s) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1, 2) EINA_MALLOC;
   EAPI void              evas_object_smart_member_add      (Evas_Object *obj, Evas_Object *smart_obj) EINA_ARG_NONNULL(1, 2);
   EAPI void              evas_object_smart_member_del      (Evas_Object *obj) EINA_ARG_NONNULL(1);
   EAPI Evas_Object      *evas_object_smart_parent_get      (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI Eina_List        *evas_object_smart_members_get     (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI Evas_Smart       *evas_object_smart_smart_get       (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI void             *evas_object_smart_data_get        (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI void              evas_object_smart_data_set        (Evas_Object *obj, void *data) EINA_ARG_NONNULL(1);
   EAPI void              evas_object_smart_callback_add    (Evas_Object *obj, const char *event, void (*func) (void *data, Evas_Object *obj, void *event_info), const void *data) EINA_ARG_NONNULL(1, 2, 3);
   EAPI void             *evas_object_smart_callback_del    (Evas_Object *obj, const char *event, void (*func) (void *data, Evas_Object *obj, void *event_info)) EINA_ARG_NONNULL(1, 2, 3);
   EAPI void              evas_object_smart_callback_call   (Evas_Object *obj, const char *event, void *event_info) EINA_ARG_NONNULL(1, 2);
   EAPI void              evas_object_smart_changed         (Evas_Object *obj) EINA_ARG_NONNULL(1);
   EAPI void              evas_object_smart_need_recalculate_set(Evas_Object *obj, Eina_Bool value) EINA_ARG_NONNULL(1);
   EAPI Eina_Bool         evas_object_smart_need_recalculate_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI void              evas_object_smart_calculate       (Evas_Object *obj) EINA_ARG_NONNULL(1);


/* events */
   EAPI void              evas_event_freeze                 (Evas *e) EINA_ARG_NONNULL(1);
   EAPI void              evas_event_thaw                   (Evas *e) EINA_ARG_NONNULL(1);
   EAPI int               evas_event_freeze_get             (const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI void              evas_event_feed_mouse_down        (Evas *e, int b, Evas_Button_Flags flags, unsigned int timestamp, const void *data) EINA_ARG_NONNULL(1);
   EAPI void              evas_event_feed_mouse_up          (Evas *e, int b, Evas_Button_Flags flags, unsigned int timestamp, const void *data) EINA_ARG_NONNULL(1);
   EAPI void              evas_event_feed_mouse_move        (Evas *e, int x, int y, unsigned int timestamp, const void *data) EINA_ARG_NONNULL(1);
   EAPI void              evas_event_feed_mouse_in          (Evas *e, unsigned int timestamp, const void *data) EINA_ARG_NONNULL(1);
   EAPI void              evas_event_feed_mouse_out         (Evas *e, unsigned int timestamp, const void *data) EINA_ARG_NONNULL(1);
   EAPI void              evas_event_feed_mouse_cancel      (Evas *e, unsigned int timestamp, const void *data) EINA_ARG_NONNULL(1);
   EAPI void              evas_event_feed_mouse_wheel       (Evas *e, int direction, int z, unsigned int timestamp, const void *data) EINA_ARG_NONNULL(1);
   EAPI void              evas_event_feed_key_down          (Evas *e, const char *keyname, const char *key, const char *string, const char *compose, unsigned int timestamp, const void *data) EINA_ARG_NONNULL(1);
   EAPI void              evas_event_feed_key_up            (Evas *e, const char *keyname, const char *key, const char *string, const char *compose, unsigned int timestamp, const void *data) EINA_ARG_NONNULL(1);
   EAPI void              evas_event_feed_hold              (Evas *e, int hold, unsigned int timestamp, const void *data) EINA_ARG_NONNULL(1);

   EAPI void              evas_object_focus_set             (Evas_Object *obj, Eina_Bool focus) EINA_ARG_NONNULL(1);
   EAPI Eina_Bool         evas_object_focus_get             (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

   EAPI Evas_Object      *evas_focus_get                    (const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

   EAPI const Evas_Modifier *evas_key_modifier_get             (const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI const Evas_Lock     *evas_key_lock_get                 (const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

   EAPI Eina_Bool         evas_key_modifier_is_set          (const Evas_Modifier *m, const char *keyname) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1, 2) EINA_PURE;

   EAPI Eina_Bool         evas_key_lock_is_set              (const Evas_Lock *l, const char *keyname) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1, 2) EINA_PURE;

   EAPI void              evas_key_modifier_add             (Evas *e, const char *keyname) EINA_ARG_NONNULL(1, 2);
   EAPI void              evas_key_modifier_del             (Evas *e, const char *keyname) EINA_ARG_NONNULL(1, 2);
   EAPI void              evas_key_lock_add                 (Evas *e, const char *keyname) EINA_ARG_NONNULL(1, 2);
   EAPI void              evas_key_lock_del                 (Evas *e, const char *keyname) EINA_ARG_NONNULL(1, 2);

   EAPI void              evas_key_modifier_on              (Evas *e, const char *keyname) EINA_ARG_NONNULL(1, 2);
   EAPI void              evas_key_modifier_off             (Evas *e, const char *keyname) EINA_ARG_NONNULL(1, 2);
   EAPI void              evas_key_lock_on                  (Evas *e, const char *keyname) EINA_ARG_NONNULL(1, 2);
   EAPI void              evas_key_lock_off                 (Evas *e, const char *keyname) EINA_ARG_NONNULL(1, 2);

   EAPI Evas_Modifier_Mask evas_key_modifier_mask_get       (const Evas *e, const char *keyname) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1, 2) EINA_PURE;

   EAPI Eina_Bool         evas_object_key_grab              (Evas_Object *obj, const char *keyname, Evas_Modifier_Mask modifiers, Evas_Modifier_Mask not_modifiers, Eina_Bool exclusive) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1, 2);
   EAPI void              evas_object_key_ungrab            (Evas_Object *obj, const char *keyname, Evas_Modifier_Mask modifiers, Evas_Modifier_Mask not_modifiers) EINA_ARG_NONNULL(1, 2);

   EAPI void              evas_object_pass_events_set       (Evas_Object *obj, Eina_Bool pass) EINA_ARG_NONNULL(1);
   EAPI Eina_Bool         evas_object_pass_events_get       (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI void              evas_object_repeat_events_set     (Evas_Object *obj, Eina_Bool repeat) EINA_ARG_NONNULL(1);
   EAPI Eina_Bool         evas_object_repeat_events_get     (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI void              evas_object_propagate_events_set  (Evas_Object *obj, Eina_Bool prop) EINA_ARG_NONNULL(1);
   EAPI Eina_Bool         evas_object_propagate_events_get  (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI void              evas_object_pointer_mode_set      (Evas_Object *obj, Evas_Object_Pointer_Mode setting) EINA_ARG_NONNULL(1);
   EAPI Evas_Object_Pointer_Mode evas_object_pointer_mode_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

       
   EAPI void              evas_object_precise_is_inside_set (Evas_Object *obj, Eina_Bool precise) EINA_ARG_NONNULL(1);
   EAPI Eina_Bool         evas_object_precise_is_inside_get (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

   EAPI void              evas_object_event_callback_add    (Evas_Object *obj, Evas_Callback_Type type, void (*func) (void *data, Evas *e, Evas_Object *obj, void *event_info), const void *data) EINA_ARG_NONNULL(1, 3);
   EAPI void             *evas_object_event_callback_del    (Evas_Object *obj, Evas_Callback_Type type, void (*func) (void *data, Evas *e, Evas_Object *obj, void *event_info)) EINA_ARG_NONNULL(1, 3);
   EAPI void             *evas_object_event_callback_del_full(Evas_Object *obj, Evas_Callback_Type type, void (*func) (void *data, Evas *e, Evas_Object *obj, void *event_info), const void *data) EINA_ARG_NONNULL(1, 3);

   EAPI int		  evas_async_events_fd_get          (void) EINA_WARN_UNUSED_RESULT EINA_PURE;
   EAPI int		  evas_async_events_process	    (void);
   EAPI Eina_Bool	  evas_async_events_put             (const void *target, Evas_Callback_Type type, void *event_info, void (*func)(void *target, Evas_Callback_Type type, void *event_info)) EINA_ARG_NONNULL(1, 4);

   EAPI void              evas_object_intercept_show_callback_add        (Evas_Object *obj, void (*func) (void *data, Evas_Object *obj), const void *data) EINA_ARG_NONNULL(1, 2);
   EAPI void             *evas_object_intercept_show_callback_del        (Evas_Object *obj, void (*func) (void *data, Evas_Object *obj)) EINA_ARG_NONNULL(1, 2);
   EAPI void              evas_object_intercept_hide_callback_add        (Evas_Object *obj, void (*func) (void *data, Evas_Object *obj), const void *data) EINA_ARG_NONNULL(1, 2);
   EAPI void             *evas_object_intercept_hide_callback_del        (Evas_Object *obj, void (*func) (void *data, Evas_Object *obj)) EINA_ARG_NONNULL(1, 2);
   EAPI void              evas_object_intercept_move_callback_add        (Evas_Object *obj, void (*func) (void *data, Evas_Object *obj, Evas_Coord x, Evas_Coord y), const void *data) EINA_ARG_NONNULL(1, 2);
   EAPI void             *evas_object_intercept_move_callback_del        (Evas_Object *obj, void (*func) (void *data, Evas_Object *obj, Evas_Coord x, Evas_Coord y)) EINA_ARG_NONNULL(1, 2);
   EAPI void              evas_object_intercept_resize_callback_add      (Evas_Object *obj, void (*func) (void *data, Evas_Object *obj, Evas_Coord w, Evas_Coord h), const void *data) EINA_ARG_NONNULL(1, 2);
   EAPI void             *evas_object_intercept_resize_callback_del      (Evas_Object *obj, void (*func) (void *data, Evas_Object *obj, Evas_Coord w, Evas_Coord h)) EINA_ARG_NONNULL(1, 2);
   EAPI void              evas_object_intercept_raise_callback_add       (Evas_Object *obj, void (*func) (void *data, Evas_Object *obj), const void *data) EINA_ARG_NONNULL(1, 2);
   EAPI void             *evas_object_intercept_raise_callback_del       (Evas_Object *obj, void (*func) (void *data, Evas_Object *obj)) EINA_ARG_NONNULL(1, 2);
   EAPI void              evas_object_intercept_lower_callback_add       (Evas_Object *obj, void (*func) (void *data, Evas_Object *obj), const void *data) EINA_ARG_NONNULL(1, 2);
   EAPI void             *evas_object_intercept_lower_callback_del       (Evas_Object *obj, void (*func) (void *data, Evas_Object *obj)) EINA_ARG_NONNULL(1, 2);
   EAPI void              evas_object_intercept_stack_above_callback_add (Evas_Object *obj, void (*func) (void *data, Evas_Object *obj, Evas_Object *above), const void *data) EINA_ARG_NONNULL(1, 2);
   EAPI void             *evas_object_intercept_stack_above_callback_del (Evas_Object *obj, void (*func) (void *data, Evas_Object *obj, Evas_Object *above)) EINA_ARG_NONNULL(1, 2);
   EAPI void              evas_object_intercept_stack_below_callback_add (Evas_Object *obj, void (*func) (void *data, Evas_Object *obj, Evas_Object *below), const void *data) EINA_ARG_NONNULL(1, 2);
   EAPI void             *evas_object_intercept_stack_below_callback_del (Evas_Object *obj, void (*func) (void *data, Evas_Object *obj, Evas_Object *below)) EINA_ARG_NONNULL(1, 2);
   EAPI void              evas_object_intercept_layer_set_callback_add   (Evas_Object *obj, void (*func) (void *data, Evas_Object *obj, int l), const void *data) EINA_ARG_NONNULL(1, 2);
   EAPI void             *evas_object_intercept_layer_set_callback_del   (Evas_Object *obj, void (*func) (void *data, Evas_Object *obj, int l)) EINA_ARG_NONNULL(1, 2);
   EAPI void              evas_object_intercept_color_set_callback_add   (Evas_Object *obj, void (*func) (void *data, Evas_Object *obj, int r, int g, int b, int a), const void *data) EINA_ARG_NONNULL(1, 2);
   EAPI void             *evas_object_intercept_color_set_callback_del   (Evas_Object *obj, void (*func) (void *data, Evas_Object *obj, int r, int g, int b, int a)) EINA_ARG_NONNULL(1, 2);
   EAPI void              evas_object_intercept_clip_set_callback_add    (Evas_Object *obj, void (*func) (void *data, Evas_Object *obj, Evas_Object *clip), const void *data) EINA_ARG_NONNULL(1, 2);
   EAPI void             *evas_object_intercept_clip_set_callback_del    (Evas_Object *obj, void (*func) (void *data, Evas_Object *obj, Evas_Object *clip)) EINA_ARG_NONNULL(1, 2);
   EAPI void              evas_object_intercept_clip_unset_callback_add  (Evas_Object *obj, void (*func) (void *data, Evas_Object *obj), const void *data) EINA_ARG_NONNULL(1, 2);
   EAPI void             *evas_object_intercept_clip_unset_callback_del  (Evas_Object *obj, void (*func) (void *data, Evas_Object *obj)) EINA_ARG_NONNULL(1, 2);

/* Evas utility routines for color space conversions */
/* hsv color space has h in the range 0.0 to 360.0, and s,v in the range 0.0 to 1.0 */
/* rgb color space has r,g,b in the range 0 to 255 */
   EAPI void              evas_color_hsv_to_rgb             (float h, float s, float v, int *r, int *g, int *b);
   EAPI void              evas_color_rgb_to_hsv             (int r, int g, int b, float *h, float *s, float *v);

/* argb color space has a,r,g,b in the range 0 to 255 */
   EAPI void              evas_color_argb_premul            (int a, int *r, int *g, int *b);
   EAPI void              evas_color_argb_unpremul          (int a, int *r, int *g, int *b);

   EAPI void              evas_data_argb_premul             (unsigned int *data, unsigned int len);
   EAPI void              evas_data_argb_unpremul           (unsigned int *data, unsigned int len);

/* Evas utility routines for working with transforms */
   /* Set t to the identity */
   EAPI void              evas_transform_identity_set       (Evas_Transform *t);
   /* Left-multiply t by an xy rotation defined by the given angle (in degrees) */
   EAPI void              evas_transform_rotate             (double angle, Evas_Transform *t);
   /* Left-multiply t by an xy translation defined by the given dx, dy values */
   EAPI void              evas_transform_translate          (float dx, float dy, Evas_Transform *t);
   /* Left-multiply t by an xy scaling defined by the given sx, sy factors */
   EAPI void              evas_transform_scale              (float sx, float sy, Evas_Transform *t);
   /* Left-multiply t by an xy shearing defined by the given sh, sv values */
   EAPI void              evas_transform_shear              (float sh, float sv, Evas_Transform *t);
   /* Left-multiply t by the given transform l */
   EAPI void              evas_transform_compose            (Evas_Transform *l, Evas_Transform *t);

/* Evas imaging api - exports some of the comon gfx engine routines */
/* this is not complete and should be considered experimental. use at your */
/* own risk */

   typedef struct _Evas_Imaging_Image Evas_Imaging_Image;
   typedef struct _Evas_Imaging_Font Evas_Imaging_Font;

   EAPI Evas_Imaging_Image *evas_imaging_image_load      (const char *file, const char *key) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1, 2) EINA_MALLOC;
   EAPI void                evas_imaging_image_free      (Evas_Imaging_Image *im) EINA_ARG_NONNULL(1);
   EAPI void                evas_imaging_image_size_get  (const Evas_Imaging_Image *im, int *w, int *h) EINA_ARG_NONNULL(1);
   EAPI Eina_Bool           evas_imaging_image_alpha_get (const Evas_Imaging_Image *im) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI void                evas_imaging_image_cache_set (int bytes);
   EAPI int                 evas_imaging_image_cache_get (void) EINA_WARN_UNUSED_RESULT;

   EAPI void                    evas_imaging_font_hinting_set      (Evas_Font_Hinting_Flags hinting);
   EAPI Evas_Font_Hinting_Flags evas_imaging_font_hinting_get      (void) EINA_WARN_UNUSED_RESULT;
   EAPI Eina_Bool               evas_imaging_font_hinting_can_hint (Evas_Font_Hinting_Flags hinting) EINA_WARN_UNUSED_RESULT;
   
   EAPI Evas_Imaging_Font  *evas_imaging_font_load                      (const char *file, const char *key, int size) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;
   EAPI void                evas_imaging_font_free                      (Evas_Imaging_Font *fn) EINA_ARG_NONNULL(1);
   EAPI int                 evas_imaging_font_ascent_get                (const Evas_Imaging_Font *fn) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI int                 evas_imaging_font_descent_get               (const Evas_Imaging_Font *fn) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI int                 evas_imaging_font_max_ascent_get            (const Evas_Imaging_Font *fn) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI int                 evas_imaging_font_max_descent_get           (const Evas_Imaging_Font *fn) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI int                 evas_imaging_font_line_advance_get          (const Evas_Imaging_Font *fn) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI void                evas_imaging_font_string_advance_get        (const Evas_Imaging_Font *fn, const char *str, int *x, int *y) EINA_ARG_NONNULL(1, 2);
   EAPI void                evas_imaging_font_string_size_query         (const Evas_Imaging_Font *fn, const char *str, int *w, int *h) EINA_ARG_NONNULL(1, 2);
   EAPI int                 evas_imaging_font_string_inset_get          (const Evas_Imaging_Font *fn, const char *str) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1, 2) EINA_PURE;
   EAPI int                 evas_imaging_font_string_char_coords_get    (const Evas_Imaging_Font *fn, const char *str, int pos, int *cx, int *cy, int *cw, int *ch) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1, 2) EINA_PURE;
   EAPI int                 evas_imaging_font_string_char_at_coords_get (const Evas_Imaging_Font *fn, const char *str, int x, int y, int *cx, int *cy, int *cw, int *ch) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1, 2) EINA_PURE;

   EAPI void                evas_imaging_font_cache_set  (int bytes);
   EAPI int                 evas_imaging_font_cache_get  (void) EINA_WARN_UNUSED_RESULT;


   /**
    * Utilities:
    */

   /**
    * Every subclass should provide this at the beginning of their own
    * data set with evas_object_smart_data_set().
    */
   typedef struct _Evas_Object_Smart_Clipped_Data Evas_Object_Smart_Clipped_Data;
   struct _Evas_Object_Smart_Clipped_Data
   {
      Evas_Object *clipper;
      Evas *evas;
   };

   EAPI Evas_Object *evas_object_smart_clipped_clipper_get(Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI void evas_object_smart_clipped_smart_set(Evas_Smart_Class *sc) EINA_ARG_NONNULL(1);

   /* convenience */
   EAPI void evas_object_smart_move_children_relative(Evas_Object *obj, Evas_Coord dx, Evas_Coord dy) EINA_ARG_NONNULL(1);


   typedef struct _Evas_Object_Box_Api        Evas_Object_Box_Api;
   typedef struct _Evas_Object_Box_Data       Evas_Object_Box_Data;
   typedef struct _Evas_Object_Box_Option     Evas_Object_Box_Option;
   typedef void (*Evas_Object_Box_Layout)(Evas_Object *o, Evas_Object_Box_Data *priv, void *user_data);

   struct _Evas_Object_Box_Api
   {
      Evas_Smart_Class base;
      Evas_Object_Box_Option *(*append)(Evas_Object *o, Evas_Object_Box_Data *priv, Evas_Object *child);
      Evas_Object_Box_Option *(*prepend)(Evas_Object *o, Evas_Object_Box_Data *priv, Evas_Object *child);
      Evas_Object_Box_Option *(*insert_before)(Evas_Object *o, Evas_Object_Box_Data *priv, Evas_Object *child, const Evas_Object *reference);
      Evas_Object_Box_Option *(*insert_at)(Evas_Object *o, Evas_Object_Box_Data *priv, Evas_Object *child, unsigned int pos);
      Evas_Object            *(*remove)(Evas_Object *o, Evas_Object_Box_Data *priv, Evas_Object *child);
      Evas_Object            *(*remove_at)(Evas_Object *o, Evas_Object_Box_Data *priv, unsigned int pos);
      Eina_Bool               (*property_set)(Evas_Object *o, Evas_Object_Box_Option *opt, int property, va_list args);
      Eina_Bool               (*property_get)(Evas_Object *o, Evas_Object_Box_Option *opt, int property, va_list args);
      const char             *(*property_name_get)(Evas_Object *o, int property);
      int                     (*property_id_get)(Evas_Object *o, const char *name);
      Evas_Object_Box_Option *(*option_new)(Evas_Object *o, Evas_Object_Box_Data *priv, Evas_Object *child);
      void                    (*option_free)(Evas_Object *o, Evas_Object_Box_Data *priv, Evas_Object_Box_Option *opt);
   };

/**
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
 */
#define EVAS_OBJECT_BOX_API_INIT(smart_class_init) {smart_class_init, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}

/**
 * Initializer to zero a whole Evas_Object_Box_Api structure.
 *
 * @see EVAS_OBJECT_BOX_API_INIT_VERSION
 * @see EVAS_OBJECT_BOX_API_INIT_NAME_VERSION
 * @see EVAS_OBJECT_BOX_API_INIT
 */
#define EVAS_OBJECT_BOX_API_INIT_NULL EVAS_OBJECT_BOX_API_INIT(EVAS_SMART_CLASS_INIT_NULL)

/**
 * Initializer to zero a whole Evas_Object_Box_Api structure and set version.
 *
 * Similar to EVAS_OBJECT_BOX_API_INIT_NULL, but will set version field of
 * Evas_Smart_Class (base field) to latest EVAS_SMART_CLASS_VERSION
 *
 * @see EVAS_OBJECT_BOX_API_INIT_NULL
 * @see EVAS_OBJECT_BOX_API_INIT_NAME_VERSION
 * @see EVAS_OBJECT_BOX_API_INIT
 */
#define EVAS_OBJECT_BOX_API_INIT_VERSION EVAS_OBJECT_BOX_API_INIT(EVAS_SMART_CLASS_INIT_VERSION)

/**
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
 */
#define EVAS_OBJECT_BOX_API_INIT_NAME_VERSION(name) EVAS_OBJECT_BOX_API_INIT(EVAS_SMART_CLASS_INIT_NAME_VERSION(name))

   struct _Evas_Object_Box_Data
   {
      Evas_Object_Smart_Clipped_Data base;
      const Evas_Object_Box_Api *api;
      struct {
	 double h, v;
      } align;
      struct {
	 Evas_Coord h, v;
      } pad;
      Eina_List *children;
      struct {
	 Evas_Object_Box_Layout cb;
	 void *data;
	 void (*free_data)(void *data);
      } layout;
   };

   struct _Evas_Object_Box_Option
   {
      Evas_Object *obj;
   };

   EAPI void evas_object_box_smart_set(Evas_Object_Box_Api *api) EINA_ARG_NONNULL(1);
   EAPI void evas_object_box_layout_set(Evas_Object *o, Evas_Object_Box_Layout cb, const void *data, void (*free_data)(void *data)) EINA_ARG_NONNULL(1, 2);

   EAPI Evas_Object *evas_object_box_add(Evas *evas) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;
   EAPI Evas_Object *evas_object_box_add_to(Evas_Object *parent) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

  EAPI void evas_object_box_layout_horizontal(Evas_Object *o, Evas_Object_Box_Data *priv, void *data) EINA_ARG_NONNULL(1, 2);
   EAPI void evas_object_box_layout_vertical(Evas_Object *o, Evas_Object_Box_Data *priv, void *data) EINA_ARG_NONNULL(1, 2);
   EAPI void evas_object_box_layout_homogeneous_vertical(Evas_Object *o, Evas_Object_Box_Data *priv, void *data) EINA_ARG_NONNULL(1, 2);
   EAPI void evas_object_box_layout_homogeneous_horizontal(Evas_Object *o, Evas_Object_Box_Data *priv, void *data) EINA_ARG_NONNULL(1, 2);
   EAPI void evas_object_box_layout_homogeneous_max_size_horizontal(Evas_Object *o, Evas_Object_Box_Data *priv, void *data) EINA_ARG_NONNULL(1, 2);
   EAPI void evas_object_box_layout_homogeneous_max_size_vertical(Evas_Object *o, Evas_Object_Box_Data *priv, void *data) EINA_ARG_NONNULL(1, 2);
   EAPI void evas_object_box_layout_flow_horizontal(Evas_Object *o, Evas_Object_Box_Data *priv, void *data) EINA_ARG_NONNULL(1, 2);
   EAPI void evas_object_box_layout_flow_vertical(Evas_Object *o, Evas_Object_Box_Data *priv, void *data) EINA_ARG_NONNULL(1, 2);
   EAPI void evas_object_box_layout_stack(Evas_Object *o, Evas_Object_Box_Data *priv, void *data) EINA_ARG_NONNULL(1, 2);

   EAPI void   evas_object_box_align_set(Evas_Object *o, double horizontal, double vertical) EINA_ARG_NONNULL(1);
   EAPI void   evas_object_box_align_get(const Evas_Object *o, double *horizontal, double *vertical) EINA_ARG_NONNULL(1);
   EAPI void   evas_object_box_padding_set(Evas_Object *o, Evas_Coord horizontal, Evas_Coord vertical) EINA_ARG_NONNULL(1);
   EAPI void   evas_object_box_padding_get(const Evas_Object *o, Evas_Coord *horizontal, Evas_Coord *vertical) EINA_ARG_NONNULL(1);

   EAPI Evas_Object_Box_Option *evas_object_box_append(Evas_Object *o, Evas_Object *child) EINA_ARG_NONNULL(1, 2);
   EAPI Evas_Object_Box_Option *evas_object_box_prepend(Evas_Object *o, Evas_Object *child) EINA_ARG_NONNULL(1, 2);
   EAPI Evas_Object_Box_Option *evas_object_box_insert_before(Evas_Object *o, Evas_Object *child, const Evas_Object *reference) EINA_ARG_NONNULL(1, 2, 3);
   EAPI Evas_Object_Box_Option *evas_object_box_insert_at(Evas_Object *o, Evas_Object *child, unsigned int pos) EINA_ARG_NONNULL(1, 2);
   EAPI Eina_Bool               evas_object_box_remove(Evas_Object *o, Evas_Object *child) EINA_ARG_NONNULL(1, 2);
   EAPI Eina_Bool               evas_object_box_remove_at(Evas_Object *o, unsigned int pos) EINA_ARG_NONNULL(1);
   EAPI Eina_Bool               evas_object_box_remove_all(Evas_Object *o, Eina_Bool clear) EINA_ARG_NONNULL(1);
   EAPI Eina_Iterator          *evas_object_box_iterator_new(const Evas_Object *o) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;
   EAPI Eina_Accessor          *evas_object_box_accessor_new(const Evas_Object *o) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;
   EAPI Eina_List              *evas_object_box_children_get(const Evas_Object *o) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

   EAPI const char *evas_object_box_option_property_name_get(Evas_Object *o, int property) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI int         evas_object_box_option_property_id_get(Evas_Object *o, const char *name) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1, 2) EINA_PURE;
   EAPI Eina_Bool   evas_object_box_option_property_set(Evas_Object *o, Evas_Object_Box_Option *opt, int property, ...) EINA_ARG_NONNULL(1, 2);
   EAPI Eina_Bool   evas_object_box_option_property_vset(Evas_Object *o, Evas_Object_Box_Option *opt, int property, va_list args) EINA_ARG_NONNULL(1, 2);
   EAPI Eina_Bool   evas_object_box_option_property_get(Evas_Object *o, Evas_Object_Box_Option *opt, int property, ...) EINA_ARG_NONNULL(1, 2);
   EAPI Eina_Bool   evas_object_box_option_property_vget(Evas_Object *o, Evas_Object_Box_Option *opt, int property, va_list args) EINA_ARG_NONNULL(1, 2);


   EAPI Evas_Object *evas_object_table_add(Evas *evas) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;
   EAPI Evas_Object *evas_object_table_add_to(Evas_Object *parent) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;
   EAPI void         evas_object_table_homogeneous_set(Evas_Object *o, Evas_Object_Table_Homogeneous_Mode homogeneous) EINA_ARG_NONNULL(1);
   EAPI Evas_Object_Table_Homogeneous_Mode evas_object_table_homogeneous_get(const Evas_Object *o) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI void         evas_object_table_padding_set(Evas_Object *o, Evas_Coord horizontal, Evas_Coord vertical) EINA_ARG_NONNULL(1);
   EAPI void         evas_object_table_padding_get(const Evas_Object *o, Evas_Coord *horizontal, Evas_Coord *vertical) EINA_ARG_NONNULL(1);
   EAPI void         evas_object_table_align_set(Evas_Object *o, double horizontal, double vertical) EINA_ARG_NONNULL(1);
   EAPI void         evas_object_table_align_get(const Evas_Object *o, double *horizontal, double *vertical) EINA_ARG_NONNULL(1);

   EAPI Eina_Bool    evas_object_table_pack(Evas_Object *o, Evas_Object *child, unsigned short col, unsigned short row, unsigned short colspan, unsigned short rowspan) EINA_ARG_NONNULL(1, 2);
   EAPI Eina_Bool    evas_object_table_unpack(Evas_Object *o, Evas_Object *child) EINA_ARG_NONNULL(1, 2);
   EAPI void         evas_object_table_clear(Evas_Object *o, Eina_Bool clear) EINA_ARG_NONNULL(1);

   EAPI void         evas_object_table_col_row_size_get(const Evas_Object *o, int *cols, int *rows) EINA_ARG_NONNULL(1);
   EAPI Eina_Iterator *evas_object_table_iterator_new(const Evas_Object *o) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;
   EAPI Eina_Accessor *evas_object_table_accessor_new(const Evas_Object *o) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;
   EAPI Eina_List     *evas_object_table_children_get(const Evas_Object *o) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

#ifdef __cplusplus
}
#endif

#endif
