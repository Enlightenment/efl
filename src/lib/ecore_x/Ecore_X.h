#ifndef _ECORE_X_H
#define _ECORE_X_H

#include <Eina.h>
#include <Efl_Config.h>

#ifdef EAPI
# undef EAPI
#endif // ifdef EAPI

#ifdef _MSC_VER
# ifdef BUILDING_DLL
#  define EAPI __declspec(dllexport)
# else // ifdef BUILDING_DLL
#  define EAPI __declspec(dllimport)
# endif // ifdef BUILDING_DLL
#else // ifdef _MSC_VER
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EAPI __attribute__ ((visibility("default")))
#  else // if __GNUC__ >= 4
#   define EAPI
#  endif // if __GNUC__ >= 4
# else // ifdef __GNUC__
#  define EAPI
# endif // ifdef __GNUC__
#endif // ifdef _MSC_VER

#define ECORE_X_VERSION_MAJOR EFL_VERSION_MAJOR
#define ECORE_X_VERSION_MINOR EFL_VERSION_MINOR

typedef struct _Ecore_X_Version
{
      int major;
      int minor;
      int micro;
      int revision;
} Ecore_X_Version;

EAPI extern Ecore_X_Version *ecore_x_version;

#include "ecore_x_version.h"

#include <sys/types.h>

/**
 * @file
 * @brief Ecore functions for dealing with the X Windows System
 *
 * @defgroup Ecore_X_Group Ecore_X - X11 Integration
 * @ingroup Ecore
 *
 * Ecore_X provides a wrapper and convenience functions for using the
 * X Windows System.  Function groups for this part of the library
 * include the following:
 * @li @ref Ecore_X_Init_Group
 * @li @ref Ecore_X_Display_Attr_Group
 * @li @ref Ecore_X_Flush_Group
 * @li @ref Ecore_X_DPMS_Group
 * @li @ref Ecore_X_Drawable_Group
 * @li @ref Ecore_X_Pixmap_Group
 * @li @ref Ecore_X_Window_Create_Group
 * @li @ref Ecore_X_Window_Properties_Group
 * @li @ref Ecore_X_Window_Destroy_Group
 * @li @ref Ecore_X_Window_Visibility_Group
 * @li @ref Ecore_X_Window_Geometry_Group
 * @li @ref Ecore_X_Window_Focus_Functions
 * @li @ref Ecore_X_Window_Z_Order_Group
 * @li @ref Ecore_X_Window_Parent_Group
 * @li @ref Ecore_X_Window_Shape
 *
 * When using the XLib backend, setting the ECORE_X_SYNC environment variable
 * will cause X calls to be run synchronously for easier debugging.
 */

typedef unsigned int   Ecore_X_ID;
#ifndef _ECORE_X_WINDOW_PREDEF
typedef Ecore_X_ID     Ecore_X_Window;
typedef Ecore_X_ID     Ecore_X_Pixmap;
#endif // ifndef _ECORE_X_WINDOW_PREDEF
typedef void          *Ecore_X_Visual;
typedef Ecore_X_ID     Ecore_X_Drawable;
#ifdef HAVE_ECORE_X_XCB
typedef Ecore_X_ID     Ecore_X_GC;
#else // ifdef HAVE_ECORE_X_XCB
typedef void          *Ecore_X_GC;
#endif /* HAVE_ECORE_X_XCB */
typedef Ecore_X_ID     Ecore_X_Atom;
typedef Ecore_X_ID     Ecore_X_Colormap;
typedef Ecore_X_ID     Ecore_X_Time;
typedef Ecore_X_ID     Ecore_X_Cursor;
typedef void           Ecore_X_Display;
typedef void           Ecore_X_Connection;
typedef void           Ecore_X_Screen;
typedef Ecore_X_ID     Ecore_X_Sync_Counter;
typedef Ecore_X_ID     Ecore_X_Sync_Alarm;
typedef Ecore_X_ID     Ecore_X_Sync_Fence; /**< @since 1.9 */
typedef void           Ecore_X_XRegion;

typedef Ecore_X_ID     Ecore_X_Randr_Output;
typedef Ecore_X_ID     Ecore_X_Randr_Crtc;
typedef Ecore_X_ID     Ecore_X_Randr_Mode;
typedef unsigned short Ecore_X_Randr_Size_ID;
typedef int            Ecore_X_Randr_Screen;

typedef Ecore_X_ID     Ecore_X_Device;

#ifdef __cplusplus
extern "C" {
#endif // ifdef __cplusplus

typedef struct _Ecore_X_Rectangle
{
   int          x, y;
   unsigned int width, height;
} Ecore_X_Rectangle;

typedef struct _Ecore_X_Icon
{
   unsigned int  width, height;
   unsigned int *data;
} Ecore_X_Icon;

typedef enum _Ecore_X_GC_Value_Mask
{
   ECORE_X_GC_VALUE_MASK_FUNCTION = (1L << 0),
   ECORE_X_GC_VALUE_MASK_PLANE_MASK = (1L << 1),
   ECORE_X_GC_VALUE_MASK_FOREGROUND = (1L << 2),
   ECORE_X_GC_VALUE_MASK_BACKGROUND = (1L << 3),
   ECORE_X_GC_VALUE_MASK_LINE_WIDTH = (1L << 4),
   ECORE_X_GC_VALUE_MASK_LINE_STYLE = (1L << 5),
   ECORE_X_GC_VALUE_MASK_CAP_STYLE = (1L << 6),
   ECORE_X_GC_VALUE_MASK_JOIN_STYLE = (1L << 7),
   ECORE_X_GC_VALUE_MASK_FILL_STYLE = (1L << 8),
   ECORE_X_GC_VALUE_MASK_FILL_RULE = (1L << 9),
   ECORE_X_GC_VALUE_MASK_TILE = (1L << 10),
   ECORE_X_GC_VALUE_MASK_STIPPLE = (1L << 11),
   ECORE_X_GC_VALUE_MASK_TILE_STIPPLE_ORIGIN_X = (1L << 12),
   ECORE_X_GC_VALUE_MASK_TILE_STIPPLE_ORIGIN_Y = (1L << 13),
   ECORE_X_GC_VALUE_MASK_FONT = (1L << 14),
   ECORE_X_GC_VALUE_MASK_SUBWINDOW_MODE = (1L << 15),
   ECORE_X_GC_VALUE_MASK_GRAPHICS_EXPOSURES = (1L << 16),
   ECORE_X_GC_VALUE_MASK_CLIP_ORIGIN_X = (1L << 17),
   ECORE_X_GC_VALUE_MASK_CLIP_ORIGIN_Y = (1L << 18),
   ECORE_X_GC_VALUE_MASK_CLIP_MASK = (1L << 19),
   ECORE_X_GC_VALUE_MASK_DASH_OFFSET = (1L << 20),
   ECORE_X_GC_VALUE_MASK_DASH_LIST = (1L << 21),
   ECORE_X_GC_VALUE_MASK_ARC_MODE = (1L << 22)
} Ecore_X_GC_Value_Mask;

typedef enum _Ecore_X_Composite_Update_Type
{
   ECORE_X_COMPOSITE_UPDATE_AUTOMATIC,
   ECORE_X_COMPOSITE_UPDATE_MANUAL
} Ecore_X_Composite_Update_Type;

/**
 * @typedef _Ecore_X_Window_State
 * Defines the different states of the window of Ecore_X.
 */
typedef enum _Ecore_X_Window_State
{
   ECORE_X_WINDOW_STATE_UNKNOWN = 0,
   ECORE_X_WINDOW_STATE_ICONIFIED,  /** The window is iconified. */
   ECORE_X_WINDOW_STATE_MODAL,  /** The window is a modal dialog box. */
   ECORE_X_WINDOW_STATE_STICKY, /** The window manager should keep the window's position fixed
                                 * even if the virtual desktop scrolls. */
   ECORE_X_WINDOW_STATE_MAXIMIZED_VERT,  /** The window has the maximum vertical size. */
   ECORE_X_WINDOW_STATE_MAXIMIZED_HORZ,  /** The window has the maximum horizontal size. */
   ECORE_X_WINDOW_STATE_SHADED,  /** The window is shaded. */
   ECORE_X_WINDOW_STATE_SKIP_TASKBAR,  /** The window should not be included in the taskbar. */
   ECORE_X_WINDOW_STATE_SKIP_PAGER,  /** The window should not be included in the pager. */
   ECORE_X_WINDOW_STATE_HIDDEN,  /** The window is invisible (i.e. minimized/iconified) */
   ECORE_X_WINDOW_STATE_FULLSCREEN,  /** The window should fill the entire screen and have no
                                      * window border/decorations */
   ECORE_X_WINDOW_STATE_ABOVE,
   ECORE_X_WINDOW_STATE_BELOW,
   ECORE_X_WINDOW_STATE_DEMANDS_ATTENTION
} Ecore_X_Window_State;

typedef enum _Ecore_X_Window_State_Action
{
   ECORE_X_WINDOW_STATE_ACTION_REMOVE,
   ECORE_X_WINDOW_STATE_ACTION_ADD,
   ECORE_X_WINDOW_STATE_ACTION_TOGGLE
} Ecore_X_Window_State_Action;

typedef enum _Ecore_X_Window_Stack_Mode
{
   ECORE_X_WINDOW_STACK_ABOVE = 0,
   ECORE_X_WINDOW_STACK_BELOW = 1,
   ECORE_X_WINDOW_STACK_TOP_IF = 2,
   ECORE_X_WINDOW_STACK_BOTTOM_IF = 3,
   ECORE_X_WINDOW_STACK_OPPOSITE = 4
} Ecore_X_Window_Stack_Mode;

typedef enum _Ecore_X_Randr_Orientation
{
   ECORE_X_RANDR_ORIENTATION_ROT_0 = (1 << 0),
   ECORE_X_RANDR_ORIENTATION_ROT_90 = (1 << 1),
   ECORE_X_RANDR_ORIENTATION_ROT_180 = (1 << 2),
   ECORE_X_RANDR_ORIENTATION_ROT_270 = (1 << 3),
   ECORE_X_RANDR_ORIENTATION_FLIP_X = (1 << 4),
   ECORE_X_RANDR_ORIENTATION_FLIP_Y = (1 << 5)
} Ecore_X_Randr_Orientation;

typedef enum _Ecore_X_Randr_Connection_Status
{
   ECORE_X_RANDR_CONNECTION_STATUS_CONNECTED = 0,
   ECORE_X_RANDR_CONNECTION_STATUS_DISCONNECTED = 1,
   ECORE_X_RANDR_CONNECTION_STATUS_UNKNOWN = 2
} Ecore_X_Randr_Connection_Status;

typedef enum _Ecore_X_Randr_Output_Policy
{
   ECORE_X_RANDR_OUTPUT_POLICY_ABOVE = 1,
   ECORE_X_RANDR_OUTPUT_POLICY_RIGHT = 2,
   ECORE_X_RANDR_OUTPUT_POLICY_BELOW = 3,
   ECORE_X_RANDR_OUTPUT_POLICY_LEFT = 4,
   ECORE_X_RANDR_OUTPUT_POLICY_CLONE = 5,
   ECORE_X_RANDR_OUTPUT_POLICY_NONE = 6,
   ECORE_X_RANDR_OUTPUT_POLICY_ASK = 7
} Ecore_X_Randr_Output_Policy;

typedef enum _Ecore_X_Randr_Relative_Alignment
{
   ECORE_X_RANDR_RELATIVE_ALIGNMENT_NONE = 0,
   ECORE_X_RANDR_RELATIVE_ALIGNMENT_CENTER_REL = 1,
   ECORE_X_RANDR_RELATIVE_ALIGNMENT_CENTER_SCR = 2
} Ecore_X_Randr_Relative_Alignment;

typedef enum _Ecore_X_Render_Subpixel_Order
{
   ECORE_X_RENDER_SUBPIXEL_ORDER_UNKNOWN = 0,
   ECORE_X_RENDER_SUBPIXEL_ORDER_HORIZONTAL_RGB = 1,
   ECORE_X_RENDER_SUBPIXEL_ORDER_HORIZONTAL_BGR = 2,
   ECORE_X_RENDER_SUBPIXEL_ORDER_VERTICAL_RGB = 3,
   ECORE_X_RENDER_SUBPIXEL_ORDER_VERTICAL_BGR = 4,
   ECORE_X_RENDER_SUBPIXEL_ORDER_NONE = 5
} Ecore_X_Render_Subpixel_Order;

typedef enum _Ecore_X_Randr_Edid_Display_Interface_Type
{
   ECORE_X_RANDR_EDID_DISPLAY_INTERFACE_UNDEFINED,
   ECORE_X_RANDR_EDID_DISPLAY_INTERFACE_DVI,
   ECORE_X_RANDR_EDID_DISPLAY_INTERFACE_HDMI_A,
   ECORE_X_RANDR_EDID_DISPLAY_INTERFACE_HDMI_B,
   ECORE_X_RANDR_EDID_DISPLAY_INTERFACE_MDDI,
   ECORE_X_RANDR_EDID_DISPLAY_INTERFACE_DISPLAY_PORT
} Ecore_X_Randr_Edid_Display_Interface_Type;

typedef enum _Ecore_X_Randr_Edid_Display_Colorscheme
{
   ECORE_X_RANDR_EDID_DISPLAY_COLORSCHEME_MONOCHROME_GRAYSCALE = 0x00,
   ECORE_X_RANDR_EDID_DISPLAY_COLORSCHEME_COLOR_RGB = 0x08,
   ECORE_X_RANDR_EDID_DISPLAY_COLORSCHEME_COLOR_NON_RGB = 0x10,
   ECORE_X_RANDR_EDID_DISPLAY_COLORSCHEME_COLOR_UNDEFINED = 0x18,
   ECORE_X_RANDR_EDID_DISPLAY_COLORSCHEME_COLOR_RGB_4_4_4 = 0x444000,
   ECORE_X_RANDR_EDID_DISPLAY_COLORSCHEME_COLOR_RGB_YCRCB_4_4_4 = 0x444,
   ECORE_X_RANDR_EDID_DISPLAY_COLORSCHEME_COLOR_RGB_YCRCB_4_2_2 = 0x422
} Ecore_X_Randr_Edid_Display_Colorscheme;

typedef enum _Ecore_X_Randr_Edid_Aspect_Ratio
{
   ECORE_X_RANDR_EDID_ASPECT_RATIO_4_3 = 0x0,
   ECORE_X_RANDR_EDID_ASPECT_RATIO_16_9 = 0x1,
   ECORE_X_RANDR_EDID_ASPECT_RATIO_16_10 = 0x2,
   ECORE_X_RANDR_EDID_ASPECT_RATIO_5_4 = 0x4,
   ECORE_X_RANDR_EDID_ASPECT_RATIO_15_9 = 0x8
} Ecore_X_Randr_Edid_Aspect_Ratio;

#define ECORE_X_RANDR_EDID_UNKNOWN_VALUE       -1

#define ECORE_X_SELECTION_TARGET_TARGETS       "TARGETS"
#define ECORE_X_SELECTION_TARGET_TEXT          "TEXT"
#define ECORE_X_SELECTION_TARGET_COMPOUND_TEXT "COMPOUND_TEXT"
#define ECORE_X_SELECTION_TARGET_STRING        "STRING"
#define ECORE_X_SELECTION_TARGET_UTF8_STRING   "UTF8_STRING"
#define ECORE_X_SELECTION_TARGET_FILENAME      "FILENAME"
#define ECORE_X_SELECTION_TARGET_X_MOZ_URL     "X_MOZ_URL"

#define ECORE_X_DND_VERSION                    5

typedef enum _Ecore_X_Selection
{
   ECORE_X_SELECTION_PRIMARY,
   ECORE_X_SELECTION_SECONDARY,
   ECORE_X_SELECTION_XDND,
   ECORE_X_SELECTION_CLIPBOARD,
   ECORE_X_SELECTION_OTHER
} Ecore_X_Selection;

typedef enum _Ecore_X_Event_Mode
{
   ECORE_X_EVENT_MODE_NORMAL,
   ECORE_X_EVENT_MODE_WHILE_GRABBED,
   ECORE_X_EVENT_MODE_GRAB,
   ECORE_X_EVENT_MODE_UNGRAB
} Ecore_X_Event_Mode;

typedef enum _Ecore_X_Event_Detail
{
   ECORE_X_EVENT_DETAIL_ANCESTOR,
   ECORE_X_EVENT_DETAIL_VIRTUAL,
   ECORE_X_EVENT_DETAIL_INFERIOR,
   ECORE_X_EVENT_DETAIL_NON_LINEAR,
   ECORE_X_EVENT_DETAIL_NON_LINEAR_VIRTUAL,
   ECORE_X_EVENT_DETAIL_POINTER,
   ECORE_X_EVENT_DETAIL_POINTER_ROOT,
   ECORE_X_EVENT_DETAIL_DETAIL_NONE
} Ecore_X_Event_Detail;

typedef enum _Ecore_X_Event_Mask
{
   ECORE_X_EVENT_MASK_NONE = 0L,
   ECORE_X_EVENT_MASK_KEY_DOWN = (1L << 0),
   ECORE_X_EVENT_MASK_KEY_UP = (1L << 1),
   ECORE_X_EVENT_MASK_MOUSE_DOWN = (1L << 2),
   ECORE_X_EVENT_MASK_MOUSE_UP = (1L << 3),
   ECORE_X_EVENT_MASK_MOUSE_IN = (1L << 4),
   ECORE_X_EVENT_MASK_MOUSE_OUT = (1L << 5),
   ECORE_X_EVENT_MASK_MOUSE_MOVE = (1L << 6),
   ECORE_X_EVENT_MASK_WINDOW_DAMAGE = (1L << 15),
   ECORE_X_EVENT_MASK_WINDOW_VISIBILITY = (1L << 16),
   ECORE_X_EVENT_MASK_WINDOW_CONFIGURE = (1L << 17),
   ECORE_X_EVENT_MASK_WINDOW_RESIZE_MANAGE = (1L << 18),
   ECORE_X_EVENT_MASK_WINDOW_MANAGE = (1L << 19),
   ECORE_X_EVENT_MASK_WINDOW_CHILD_CONFIGURE = (1L << 20),
   ECORE_X_EVENT_MASK_WINDOW_FOCUS_CHANGE = (1L << 21),
   ECORE_X_EVENT_MASK_WINDOW_PROPERTY = (1L << 22),
   ECORE_X_EVENT_MASK_WINDOW_COLORMAP = (1L << 23),
   ECORE_X_EVENT_MASK_WINDOW_GRAB = (1L << 24),
   ECORE_X_EVENT_MASK_MOUSE_WHEEL = (1L << 29),
   ECORE_X_EVENT_MASK_WINDOW_FOCUS_IN = (1L << 30),
   ECORE_X_EVENT_MASK_WINDOW_FOCUS_OUT = (1L << 31)
} Ecore_X_Event_Mask;

typedef enum _Ecore_X_Gravity
{
   ECORE_X_GRAVITY_FORGET = 0,
   ECORE_X_GRAVITY_UNMAP = 0,
   ECORE_X_GRAVITY_NW = 1,
   ECORE_X_GRAVITY_N = 2,
   ECORE_X_GRAVITY_NE = 3,
   ECORE_X_GRAVITY_W = 4,
   ECORE_X_GRAVITY_CENTER = 5,
   ECORE_X_GRAVITY_E = 6,
   ECORE_X_GRAVITY_SW = 7,
   ECORE_X_GRAVITY_S = 8,
   ECORE_X_GRAVITY_SE = 9,
   ECORE_X_GRAVITY_STATIC = 10
} Ecore_X_Gravity;

/* Needed for ecore_x_region_window_shape_set */
typedef enum _Ecore_X_Shape_Type
{
   ECORE_X_SHAPE_BOUNDING,
   ECORE_X_SHAPE_CLIP,
   ECORE_X_SHAPE_INPUT
} Ecore_X_Shape_Type;

typedef enum _Ecore_X_Mapping_Type
{
   ECORE_X_MAPPING_MODIFIER,
   ECORE_X_MAPPING_KEYBOARD,
   ECORE_X_MAPPING_MOUSE
} Ecore_X_Mapping_Type;

typedef enum _Ecore_X_Randr_Property_Change
{
   ECORE_X_RANDR_PROPERTY_CHANGE_ADD,
   ECORE_X_RANDR_PROPERTY_CHANGE_DEL
} Ecore_X_Randr_Property_Change;

typedef enum _Ecore_X_Netwm_Direction
{
   ECORE_X_NETWM_DIRECTION_SIZE_TL = 0,
   ECORE_X_NETWM_DIRECTION_SIZE_T = 1,
   ECORE_X_NETWM_DIRECTION_SIZE_TR = 2,
   ECORE_X_NETWM_DIRECTION_SIZE_R = 3,
   ECORE_X_NETWM_DIRECTION_SIZE_BR = 4,
   ECORE_X_NETWM_DIRECTION_SIZE_B = 5,
   ECORE_X_NETWM_DIRECTION_SIZE_BL = 6,
   ECORE_X_NETWM_DIRECTION_SIZE_L = 7,
   ECORE_X_NETWM_DIRECTION_MOVE = 8,
   ECORE_X_NETWM_DIRECTION_CANCEL = 11,
} Ecore_X_Netwm_Direction;

/**
 * @typedef _Ecore_X_Error_Code
 * Defines the error codes of Ecore_X which wraps the X Window Systems
 * protocol's errors.
 *
 * @since 1.7.0
 */
typedef enum _Ecore_X_Error_Code
{
   /** Everything is okay. */
   ECORE_X_ERROR_CODE_SUCCESS = 0, /** Bad request code */
   ECORE_X_ERROR_CODE_BAD_REQUEST = 1, /** Int parameter out of range */
   ECORE_X_ERROR_CODE_BAD_VALUE = 2, /** Parameter not a Window */
   ECORE_X_ERROR_CODE_BAD_WINDOW = 3, /** Parameter not a Pixmap */
   ECORE_X_ERROR_CODE_BAD_PIXMAP = 4, /** Parameter not an Atom */
   ECORE_X_ERROR_CODE_BAD_ATOM = 5, /** Parameter not a Cursor */
   ECORE_X_ERROR_CODE_BAD_CURSOR = 6, /** Parameter not a Font */
   ECORE_X_ERROR_CODE_BAD_FONT = 7, /** Parameter mismatch */
   ECORE_X_ERROR_CODE_BAD_MATCH = 8, /** Parameter not a Pixmap or Window */
   ECORE_X_ERROR_CODE_BAD_DRAWABLE = 9, /** Bad access */
   ECORE_X_ERROR_CODE_BAD_ACCESS = 10, /** Insufficient resources */
   ECORE_X_ERROR_CODE_BAD_ALLOC = 11, /** No such colormap */
   ECORE_X_ERROR_CODE_BAD_COLOR = 12, /** Parameter not a GC */
   ECORE_X_ERROR_CODE_BAD_GC = 13, /** Choice not in range or already used */
   ECORE_X_ERROR_CODE_BAD_ID_CHOICE = 14, /** Font or color name doesn't exist */
   ECORE_X_ERROR_CODE_BAD_NAME = 15, /** Request length incorrect */
   ECORE_X_ERROR_CODE_BAD_LENGTH = 16, /** Server is defective */
   ECORE_X_ERROR_CODE_BAD_IMPLEMENTATION = 17,
} Ecore_X_Error_Code;

typedef enum _Ecore_X_Dpms_Mode
{
   ECORE_X_DPMS_MODE_ON = 0,
   ECORE_X_DPMS_MODE_STANDBY = 1,
   ECORE_X_DPMS_MODE_SUSPEND = 2,
   ECORE_X_DPMS_MODE_OFF = 3
} Ecore_X_Dpms_Mode;

typedef struct _Ecore_X_Event_Mouse_In                     Ecore_X_Event_Mouse_In;
typedef struct _Ecore_X_Event_Mouse_Out                    Ecore_X_Event_Mouse_Out;
typedef struct _Ecore_X_Event_Window_Focus_In              Ecore_X_Event_Window_Focus_In;
typedef struct _Ecore_X_Event_Window_Focus_Out             Ecore_X_Event_Window_Focus_Out;
typedef struct _Ecore_X_Event_Window_Keymap                Ecore_X_Event_Window_Keymap;
typedef struct _Ecore_X_Event_Window_Damage                Ecore_X_Event_Window_Damage;
typedef struct _Ecore_X_Event_Window_Visibility_Change     Ecore_X_Event_Window_Visibility_Change;
typedef struct _Ecore_X_Event_Window_Create                Ecore_X_Event_Window_Create;
typedef struct _Ecore_X_Event_Window_Destroy               Ecore_X_Event_Window_Destroy;
typedef struct _Ecore_X_Event_Window_Hide                  Ecore_X_Event_Window_Hide;
typedef struct _Ecore_X_Event_Window_Show                  Ecore_X_Event_Window_Show;
typedef struct _Ecore_X_Event_Window_Show_Request          Ecore_X_Event_Window_Show_Request;
typedef struct _Ecore_X_Event_Window_Reparent              Ecore_X_Event_Window_Reparent;
typedef struct _Ecore_X_Event_Window_Configure             Ecore_X_Event_Window_Configure;
typedef struct _Ecore_X_Event_Window_Configure_Request     Ecore_X_Event_Window_Configure_Request;
typedef struct _Ecore_X_Event_Window_Gravity               Ecore_X_Event_Window_Gravity;
typedef struct _Ecore_X_Event_Window_Resize_Request        Ecore_X_Event_Window_Resize_Request;
typedef struct _Ecore_X_Event_Window_Stack                 Ecore_X_Event_Window_Stack;
typedef struct _Ecore_X_Event_Window_Stack_Request         Ecore_X_Event_Window_Stack_Request;
typedef struct _Ecore_X_Event_Window_Property              Ecore_X_Event_Window_Property;
typedef struct _Ecore_X_Event_Window_Colormap              Ecore_X_Event_Window_Colormap;
typedef struct _Ecore_X_Event_Mapping_Change               Ecore_X_Event_Mapping_Change;
typedef struct _Ecore_X_Event_Window_Mapping               Ecore_X_Event_Window_Mapping;
typedef struct _Ecore_X_Event_Selection_Clear              Ecore_X_Event_Selection_Clear;
typedef struct _Ecore_X_Event_Selection_Request            Ecore_X_Event_Selection_Request;
typedef struct _Ecore_X_Event_Selection_Notify             Ecore_X_Event_Selection_Notify;
typedef struct _Ecore_X_Event_Fixes_Selection_Notify       Ecore_X_Event_Fixes_Selection_Notify;
typedef struct _Ecore_X_Selection_Data                     Ecore_X_Selection_Data;
typedef struct _Ecore_X_Selection_Data_Files               Ecore_X_Selection_Data_Files;
typedef struct _Ecore_X_Selection_Data_Text                Ecore_X_Selection_Data_Text;
typedef struct _Ecore_X_Selection_Data_X_Moz_Url           Ecore_X_Selection_Data_X_Moz_Url;
typedef struct _Ecore_X_Selection_Data_Targets             Ecore_X_Selection_Data_Targets;
typedef struct _Ecore_X_Event_Xdnd_Enter                   Ecore_X_Event_Xdnd_Enter;
typedef struct _Ecore_X_Event_Xdnd_Position                Ecore_X_Event_Xdnd_Position;
typedef struct _Ecore_X_Event_Xdnd_Status                  Ecore_X_Event_Xdnd_Status;
typedef struct _Ecore_X_Event_Xdnd_Leave                   Ecore_X_Event_Xdnd_Leave;
typedef struct _Ecore_X_Event_Xdnd_Drop                    Ecore_X_Event_Xdnd_Drop;
typedef struct _Ecore_X_Event_Xdnd_Finished                Ecore_X_Event_Xdnd_Finished;
typedef struct _Ecore_X_Event_Client_Message               Ecore_X_Event_Client_Message;
typedef struct _Ecore_X_Event_Window_Shape                 Ecore_X_Event_Window_Shape;
typedef struct _Ecore_X_Event_Screensaver_Notify           Ecore_X_Event_Screensaver_Notify;
typedef struct _Ecore_X_Event_Gesture_Notify_Flick         Ecore_X_Event_Gesture_Notify_Flick;
typedef struct _Ecore_X_Event_Gesture_Notify_Pan           Ecore_X_Event_Gesture_Notify_Pan;
typedef struct _Ecore_X_Event_Gesture_Notify_PinchRotation Ecore_X_Event_Gesture_Notify_PinchRotation;
typedef struct _Ecore_X_Event_Gesture_Notify_Tap           Ecore_X_Event_Gesture_Notify_Tap;
typedef struct _Ecore_X_Event_Gesture_Notify_TapNHold      Ecore_X_Event_Gesture_Notify_TapNHold;
typedef struct _Ecore_X_Event_Gesture_Notify_Hold          Ecore_X_Event_Gesture_Notify_Hold;
typedef struct _Ecore_X_Event_Gesture_Notify_Group         Ecore_X_Event_Gesture_Notify_Group;
typedef struct _Ecore_X_Event_Sync_Counter                 Ecore_X_Event_Sync_Counter;
typedef struct _Ecore_X_Event_Sync_Alarm                   Ecore_X_Event_Sync_Alarm;
typedef struct _Ecore_X_Event_Screen_Change                Ecore_X_Event_Screen_Change;
typedef struct _Ecore_X_Event_Randr_Crtc_Change            Ecore_X_Event_Randr_Crtc_Change;
typedef struct _Ecore_X_Event_Randr_Output_Change          Ecore_X_Event_Randr_Output_Change;
typedef struct _Ecore_X_Event_Randr_Output_Property_Notify Ecore_X_Event_Randr_Output_Property_Notify;

typedef struct _Ecore_X_Event_Window_Delete_Request        Ecore_X_Event_Window_Delete_Request;
typedef struct _Ecore_X_Event_Window_Move_Resize_Request   Ecore_X_Event_Window_Move_Resize_Request;
typedef struct _Ecore_X_Event_Window_State_Request         Ecore_X_Event_Window_State_Request;
typedef struct _Ecore_X_Event_Frame_Extents_Request        Ecore_X_Event_Frame_Extents_Request;
typedef struct _Ecore_X_Event_Ping                         Ecore_X_Event_Ping;
typedef struct _Ecore_X_Event_Desktop_Change               Ecore_X_Event_Desktop_Change;

typedef struct _Ecore_X_Event_Startup_Sequence             Ecore_X_Event_Startup_Sequence;

typedef struct _Ecore_X_Event_Generic                      Ecore_X_Event_Generic;


typedef struct Ecore_X_Event_Present_Configure             Ecore_X_Event_Present_Configure; /**< @since 1.9 */
typedef struct Ecore_X_Event_Present_Complete              Ecore_X_Event_Present_Complete; /**< @since 1.9 */
typedef struct Ecore_X_Event_Present_Idle                  Ecore_X_Event_Present_Idle; /**< @since 1.9 */

typedef struct _Ecore_X_Randr_Screen_Size                  Ecore_X_Randr_Screen_Size;
typedef struct _Ecore_X_Randr_Screen_Size_MM               Ecore_X_Randr_Screen_Size_MM;
typedef struct _Ecore_X_Randr_Crtc_Info                    Ecore_X_Randr_Crtc_Info; /**< @since 1.8 */

typedef struct _Ecore_X_Xdnd_Position                      Ecore_X_Xdnd_Position;


struct _Ecore_X_Event_Mouse_In
{
   int                  modifiers;
   int                  x, y;
   Eina_Bool            same_screen : 1;
   struct
   {
      int x, y;
   } root;
   Ecore_X_Window       win;
   Ecore_X_Window       event_win;
   Ecore_X_Window       root_win;
   Ecore_X_Event_Mode   mode;
   Ecore_X_Event_Detail detail;
   Ecore_X_Time         time;
};

struct _Ecore_X_Event_Mouse_Out
{
   int                  modifiers;
   int                  x, y;
   int                  same_screen;
   struct
   {
      int x, y;
   } root;
   Ecore_X_Window       win;
   Ecore_X_Window       event_win;
   Ecore_X_Window       root_win;
   Ecore_X_Event_Mode   mode;
   Ecore_X_Event_Detail detail;
   Ecore_X_Time         time;
};

struct _Ecore_X_Event_Window_Focus_In
{
   Ecore_X_Window       win;
   Ecore_X_Event_Mode   mode;
   Ecore_X_Event_Detail detail;
   Ecore_X_Time         time;
};

struct _Ecore_X_Event_Window_Focus_Out
{
   Ecore_X_Window       win;
   Ecore_X_Event_Mode   mode;
   Ecore_X_Event_Detail detail;
   Ecore_X_Time         time;
};

struct _Ecore_X_Event_Window_Keymap
{
   Ecore_X_Window win;
};

struct _Ecore_X_Event_Window_Damage
{
   Ecore_X_Window win;
   int            x, y, w, h;
   int            count;
   Ecore_X_Time   time;
};

struct _Ecore_X_Event_Window_Visibility_Change
{
   Ecore_X_Window win;
   int            fully_obscured;
   Ecore_X_Time   time;
};

struct _Ecore_X_Event_Window_Create
{
   Ecore_X_Window win;
   Ecore_X_Window parent;
   int            override;
   int            x, y, w, h;
   int            border;
   Ecore_X_Time   time;
};

struct _Ecore_X_Event_Window_Destroy
{
   Ecore_X_Window win;
   Ecore_X_Window event_win;
   Ecore_X_Time   time;
};

struct _Ecore_X_Event_Window_Hide
{
   Ecore_X_Window win;
   Ecore_X_Window event_win;
   Ecore_X_Time   time;
   Eina_Bool      send_event : 1; /**< @since 1.8 */
};

struct _Ecore_X_Event_Window_Show
{
   Ecore_X_Window win;
   Ecore_X_Window event_win;
   Ecore_X_Time   time;
};

struct _Ecore_X_Event_Window_Show_Request
{
   Ecore_X_Window win;
   Ecore_X_Window parent;
   Ecore_X_Time   time;
};

struct _Ecore_X_Event_Window_Reparent
{
   Ecore_X_Window win;
   Ecore_X_Window event_win;
   Ecore_X_Window parent;
   Ecore_X_Time   time;
};

struct _Ecore_X_Event_Window_Configure
{
   Ecore_X_Window win;
   Ecore_X_Window event_win;
   Ecore_X_Window abovewin;
   int            x, y, w, h;
   int            border;
   Eina_Bool      override : 1;
   Eina_Bool      from_wm : 1;
   Ecore_X_Time   time;
};

struct _Ecore_X_Event_Window_Configure_Request
{
   Ecore_X_Window            win;
   Ecore_X_Window            parent_win;
   Ecore_X_Window            abovewin;
   int                       x, y, w, h;
   int                       border;
   Ecore_X_Window_Stack_Mode detail;
   unsigned long             value_mask;
   Ecore_X_Time              time;
};

struct _Ecore_X_Event_Window_Gravity
{
   Ecore_X_Window win;
   Ecore_X_Window event_win;
   Ecore_X_Time   time;
};

struct _Ecore_X_Event_Window_Resize_Request
{
   Ecore_X_Window win;
   int            w, h;
   Ecore_X_Time   time;
};

struct _Ecore_X_Event_Window_Stack
{
   Ecore_X_Window            win;
   Ecore_X_Window            event_win;
   Ecore_X_Window_Stack_Mode detail;
   Ecore_X_Time              time;
};

struct _Ecore_X_Event_Window_Stack_Request
{
   Ecore_X_Window            win;
   Ecore_X_Window            parent;
   Ecore_X_Window_Stack_Mode detail;
   Ecore_X_Time              time;
};

struct _Ecore_X_Event_Window_Property
{
   Ecore_X_Window win;
   Ecore_X_Atom   atom;
   Ecore_X_Time   time;
};

struct _Ecore_X_Event_Window_Colormap
{
   Ecore_X_Window   win;
   Ecore_X_Colormap cmap;
   Eina_Bool        installed : 1;
   Ecore_X_Time     time;
};

struct _Ecore_X_Event_Mapping_Change
{
   Ecore_X_Mapping_Type type;
   int                  keycode;
   int                  num;
};

struct _Ecore_X_Event_Selection_Clear
{
   Ecore_X_Window    win;
   Ecore_X_Selection selection;
   Ecore_X_Atom      atom;
   Ecore_X_Time      time;
};

struct _Ecore_X_Event_Selection_Request
{
   Ecore_X_Window owner;
   Ecore_X_Window requestor;
   Ecore_X_Time   time;
   Ecore_X_Atom   selection;
   Ecore_X_Atom   target;
   Ecore_X_Atom   property;
};

typedef enum
{
   ECORE_X_OWNER_CHANGE_REASON_NEW_OWNER,
   ECORE_X_OWNER_CHANGE_REASON_DESTROY,
   ECORE_X_OWNER_CHANGE_REASON_CLOSE
} Ecore_X_Owner_Change_Reason;

struct _Ecore_X_Event_Fixes_Selection_Notify
{
   Ecore_X_Window              win;
   Ecore_X_Window              owner;
   Ecore_X_Time                time;
   Ecore_X_Time                selection_time;
   Ecore_X_Selection           selection;
   Ecore_X_Atom                atom;
   Ecore_X_Owner_Change_Reason reason;
};

struct _Ecore_X_Event_Selection_Notify
{
   Ecore_X_Window    win;
   Ecore_X_Time      time;
   Ecore_X_Selection selection;
   Ecore_X_Atom      atom;
   char             *target;
   void             *data;
};

struct _Ecore_X_Selection_Data
{
   enum
   {
      ECORE_X_SELECTION_CONTENT_NONE,
      ECORE_X_SELECTION_CONTENT_TEXT,
      ECORE_X_SELECTION_CONTENT_FILES,
      ECORE_X_SELECTION_CONTENT_X_MOZ_URL,
      ECORE_X_SELECTION_CONTENT_TARGETS,
      ECORE_X_SELECTION_CONTENT_CUSTOM
   } content;
   unsigned char *data;
   int            length;
   int            format;
   int            (*free)(void *data);
};

struct _Ecore_X_Selection_Data_Files
{
   Ecore_X_Selection_Data data;
   char                 **files;
   int                    num_files;
};

struct _Ecore_X_Selection_Data_Text
{
   Ecore_X_Selection_Data data;
   char                  *text;
};

struct _Ecore_X_Selection_Data_X_Moz_Url
{
   Ecore_X_Selection_Data data;
   Eina_Inarray         *links;
   Eina_Inarray         *link_names;
};

struct _Ecore_X_Selection_Data_Targets
{
   Ecore_X_Selection_Data data;
   char                 **targets;
   int                    num_targets;
};

struct _Ecore_X_Event_Xdnd_Enter
{
   Ecore_X_Window win, source;

   char         **types;
   int            num_types;
};

struct _Ecore_X_Event_Xdnd_Position
{
   Ecore_X_Window win, source;
   struct
   {
      int x, y;
   } position;
   Ecore_X_Atom   action;
};

struct _Ecore_X_Xdnd_Position
{
   Ecore_X_Window win, prev;
   struct
   {
      int x, y;
   } position;
};

struct _Ecore_X_Event_Xdnd_Status
{
   Ecore_X_Window    win, target;
   Eina_Bool         will_accept : 1;
   Ecore_X_Rectangle rectangle;
   Ecore_X_Atom      action;
};

struct _Ecore_X_Event_Xdnd_Leave
{
   Ecore_X_Window win, source;
};

struct _Ecore_X_Event_Xdnd_Drop
{
   Ecore_X_Window win, source;
   Ecore_X_Atom   action;
   struct
   {
      int x, y;
   } position;
};

struct _Ecore_X_Event_Xdnd_Finished
{
   Ecore_X_Window win, target;
   Eina_Bool      completed : 1;
   Ecore_X_Atom   action;
};

struct _Ecore_X_Event_Client_Message
{
   Ecore_X_Window win;
   Ecore_X_Atom   message_type;
   int            format;
   union
   {
      char  b[20];
      short s[10];
      long  l[5];
   } data;
   Ecore_X_Time   time;
};

struct _Ecore_X_Event_Window_Shape
{
   Ecore_X_Window     win;
   Ecore_X_Time       time;
   Ecore_X_Shape_Type type;
   int                x, y, w, h;
   Eina_Bool          shaped : 1;
};

struct _Ecore_X_Event_Screensaver_Notify
{
   Ecore_X_Window win;
   Eina_Bool      on : 1;
   Ecore_X_Time   time;
};

struct _Ecore_X_Event_Sync_Counter
{
   Ecore_X_Time time;
};

struct _Ecore_X_Event_Sync_Alarm
{
   Ecore_X_Time       time;
   Ecore_X_Sync_Alarm alarm;
};

struct _Ecore_X_Randr_Screen_Size
{
   int width, height;
};

struct _Ecore_X_Randr_Screen_Size_MM
{
   int width, height, width_mm, height_mm;
};

struct _Ecore_X_Randr_Crtc_Info
{
   Ecore_X_Time timestamp;
   int x, y;
   unsigned int width, height;
   Ecore_X_Randr_Mode mode;
   Ecore_X_Randr_Orientation rotation;
   int noutput;
   Ecore_X_Randr_Output *outputs;
   Ecore_X_Randr_Orientation rotations;
   int npossible;
   Ecore_X_Randr_Output *possible;
}; /**< @since 1.8 */

struct _Ecore_X_Event_Screen_Change
{
   Ecore_X_Window                win;
   Ecore_X_Window                root;
   Ecore_X_Randr_Screen_Size_MM  size;  /* in pixel and millimeters */
   Ecore_X_Time                  time;
   Ecore_X_Time                  config_time;
   Ecore_X_Randr_Orientation     orientation;
   Ecore_X_Render_Subpixel_Order subpixel_order;
   Ecore_X_Randr_Size_ID         size_id;
};

struct _Ecore_X_Event_Randr_Crtc_Change
{
   Ecore_X_Window            win;
   Ecore_X_Randr_Crtc        crtc;
   Ecore_X_Randr_Mode        mode;
   Ecore_X_Randr_Orientation orientation;
   Eina_Rectangle            geo;
};

struct _Ecore_X_Event_Randr_Output_Change
{
   Ecore_X_Window                  win;
   Ecore_X_Randr_Output            output;
   Ecore_X_Randr_Crtc              crtc;
   Ecore_X_Randr_Mode              mode;
   Ecore_X_Randr_Orientation       orientation;
   Ecore_X_Randr_Connection_Status connection;
   Ecore_X_Render_Subpixel_Order   subpixel_order;
};

struct _Ecore_X_Event_Randr_Output_Property_Notify
{
   Ecore_X_Window                win;
   Ecore_X_Randr_Output          output;
   Ecore_X_Atom                  property;
   Ecore_X_Time                  time;
   Ecore_X_Randr_Property_Change state;
};

struct _Ecore_X_Event_Window_Delete_Request
{
   Ecore_X_Window win;
   Ecore_X_Time   time;
};

struct _Ecore_X_Event_Window_Prop_Title_Change
{
   Ecore_X_Window win;
   char          *title;
   Ecore_X_Time   time;
};

struct _Ecore_X_Event_Window_Prop_Visible_Title_Change
{
   Ecore_X_Window win;
   char          *title;
   Ecore_X_Time   time;
};

struct _Ecore_X_Event_Window_Prop_Icon_Name_Change
{
   Ecore_X_Window win;
   char          *name;
   Ecore_X_Time   time;
};

struct _Ecore_X_Event_Window_Prop_Visible_Icon_Name_Change
{
   Ecore_X_Window win;
   char          *name;
   Ecore_X_Time   time;
};

struct _Ecore_X_Event_Window_Prop_Client_Machine_Change
{
   Ecore_X_Window win;
   char          *name;
   Ecore_X_Time   time;
};

struct _Ecore_X_Event_Window_Prop_Name_Class_Change
{
   Ecore_X_Window win;
   char          *name;
   char          *clas;
   Ecore_X_Time   time;
};

struct _Ecore_X_Event_Window_Prop_Pid_Change
{
   Ecore_X_Window win;
   pid_t          pid;
   Ecore_X_Time   time;
};

struct _Ecore_X_Event_Window_Prop_Desktop_Change
{
   Ecore_X_Window win;
   long           desktop;
   Ecore_X_Time   time;
};

struct _Ecore_X_Event_Startup_Sequence
{
   Ecore_X_Window win;
};

struct _Ecore_X_Event_Window_Move_Resize_Request
{
   Ecore_X_Window win;
   int            x, y;
   int            direction;
   int            button;
   int            source;
};

struct _Ecore_X_Event_Window_State_Request
{
   Ecore_X_Window              win;
   Ecore_X_Window_State_Action action;
   Ecore_X_Window_State        state[2];
   int                         source;
};

struct _Ecore_X_Event_Frame_Extents_Request
{
   Ecore_X_Window win;
};

struct _Ecore_X_Event_Ping
{
   Ecore_X_Window win;
   Ecore_X_Window event_win;
   Ecore_X_Time   time;
};

struct _Ecore_X_Event_Desktop_Change
{
   Ecore_X_Window win;
   unsigned int   desk;
   int            source;
};

struct _Ecore_X_Event_Generic
{
   int          extension;
   int          evtype;
   unsigned int cookie;
   void        *data;
};

typedef enum Ecore_X_Present_Event_Mask
{
    ECORE_X_PRESENT_EVENT_MASK_NO_EVENT = 0,
    ECORE_X_PRESENT_EVENT_MASK_CONFIGURE_NOTIFY = 1,
    ECORE_X_PRESENT_EVENT_MASK_COMPLETE_NOTIFY = 2,
    ECORE_X_PRESENT_EVENT_MASK_IDLE_NOTIFY = 4,
} Ecore_X_Present_Event_Mask; /**< @since 1.9 */

typedef struct Ecore_X_Present
{
   Ecore_X_Window win;
   unsigned int serial;
} Ecore_X_Present; /**< @since 1.9 */

struct Ecore_X_Event_Present_Configure
{
   Ecore_X_Window win;

   int x, y;
   unsigned int width, height;
   int off_x, off_y;
   int pixmap_width, pixmap_height;
   long pixmap_flags;
}; /**< @since 1.9 */

typedef enum
{
   ECORE_X_PRESENT_COMPLETE_MODE_COPY,
   ECORE_X_PRESENT_COMPLETE_MODE_FLIP,
   ECORE_X_PRESENT_COMPLETE_MODE_SKIP,
} Ecore_X_Present_Complete_Mode;

struct Ecore_X_Event_Present_Complete
{
   Ecore_X_Window win;

   unsigned int serial; // value provided when generating request
   unsigned long long ust; // system time of presentation
   unsigned long long msc; // frame count at time of presentation
   Eina_Bool kind : 1; /* 0 for PresentCompleteKindPixmap (PresentPixmap completion),
                          1 for PresentCompleteKindNotifyMsc (PresentNotifyMSC completion) */
   Ecore_X_Present_Complete_Mode mode;
}; /**< @since 1.9 */

struct Ecore_X_Event_Present_Idle
{
   Ecore_X_Window win;

   unsigned int serial;
   Ecore_X_Pixmap pixmap;
   Ecore_X_Sync_Fence idle_fence;
}; /**< @since 1.9 */

EAPI extern int ECORE_X_EVENT_ANY; /**< low level event dependent on
                                        backend in use, if Xlib will be XEvent, if XCB will be xcb_generic_event_t.
                                        @warning avoid using it.
                                    */
EAPI extern int ECORE_X_EVENT_MOUSE_IN;
EAPI extern int ECORE_X_EVENT_MOUSE_OUT;
EAPI extern int ECORE_X_EVENT_WINDOW_FOCUS_IN;
EAPI extern int ECORE_X_EVENT_WINDOW_FOCUS_OUT;
EAPI extern int ECORE_X_EVENT_WINDOW_KEYMAP;
EAPI extern int ECORE_X_EVENT_WINDOW_DAMAGE;
EAPI extern int ECORE_X_EVENT_WINDOW_VISIBILITY_CHANGE;
EAPI extern int ECORE_X_EVENT_WINDOW_CREATE;
EAPI extern int ECORE_X_EVENT_WINDOW_DESTROY;
EAPI extern int ECORE_X_EVENT_WINDOW_HIDE;
EAPI extern int ECORE_X_EVENT_WINDOW_SHOW;
EAPI extern int ECORE_X_EVENT_WINDOW_SHOW_REQUEST;
EAPI extern int ECORE_X_EVENT_WINDOW_REPARENT;
EAPI extern int ECORE_X_EVENT_WINDOW_CONFIGURE;
EAPI extern int ECORE_X_EVENT_WINDOW_CONFIGURE_REQUEST;
EAPI extern int ECORE_X_EVENT_WINDOW_GRAVITY;
EAPI extern int ECORE_X_EVENT_WINDOW_RESIZE_REQUEST;
EAPI extern int ECORE_X_EVENT_WINDOW_STACK;
EAPI extern int ECORE_X_EVENT_WINDOW_STACK_REQUEST;
EAPI extern int ECORE_X_EVENT_WINDOW_PROPERTY;
EAPI extern int ECORE_X_EVENT_WINDOW_COLORMAP;
EAPI extern int ECORE_X_EVENT_WINDOW_MAPPING;
EAPI extern int ECORE_X_EVENT_MAPPING_CHANGE;
EAPI extern int ECORE_X_EVENT_SELECTION_CLEAR;
EAPI extern int ECORE_X_EVENT_SELECTION_REQUEST;
EAPI extern int ECORE_X_EVENT_SELECTION_NOTIFY;
EAPI extern int ECORE_X_EVENT_FIXES_SELECTION_NOTIFY;
EAPI extern int ECORE_X_EVENT_CLIENT_MESSAGE;
EAPI extern int ECORE_X_EVENT_WINDOW_SHAPE;
EAPI extern int ECORE_X_EVENT_SCREENSAVER_NOTIFY;
EAPI extern int ECORE_X_EVENT_GESTURE_NOTIFY_FLICK;
EAPI extern int ECORE_X_EVENT_GESTURE_NOTIFY_PAN;
EAPI extern int ECORE_X_EVENT_GESTURE_NOTIFY_PINCHROTATION;
EAPI extern int ECORE_X_EVENT_GESTURE_NOTIFY_TAP;
EAPI extern int ECORE_X_EVENT_GESTURE_NOTIFY_TAPNHOLD;
EAPI extern int ECORE_X_EVENT_GESTURE_NOTIFY_HOLD;
EAPI extern int ECORE_X_EVENT_GESTURE_NOTIFY_GROUP;
EAPI extern int ECORE_X_EVENT_SYNC_COUNTER;
EAPI extern int ECORE_X_EVENT_SYNC_ALARM;
EAPI extern int ECORE_X_EVENT_SCREEN_CHANGE;
EAPI extern int ECORE_X_EVENT_RANDR_CRTC_CHANGE;
EAPI extern int ECORE_X_EVENT_RANDR_OUTPUT_CHANGE;
EAPI extern int ECORE_X_EVENT_RANDR_OUTPUT_PROPERTY_NOTIFY;
EAPI extern int ECORE_X_EVENT_DAMAGE_NOTIFY;

EAPI extern int ECORE_X_EVENT_WINDOW_DELETE_REQUEST;

EAPI extern int ECORE_X_EVENT_WINDOW_MOVE_RESIZE_REQUEST;
EAPI extern int ECORE_X_EVENT_WINDOW_STATE_REQUEST;
EAPI extern int ECORE_X_EVENT_FRAME_EXTENTS_REQUEST;
EAPI extern int ECORE_X_EVENT_PING;
EAPI extern int ECORE_X_EVENT_DESKTOP_CHANGE;

EAPI extern int ECORE_X_EVENT_STARTUP_SEQUENCE_NEW;
EAPI extern int ECORE_X_EVENT_STARTUP_SEQUENCE_CHANGE;
EAPI extern int ECORE_X_EVENT_STARTUP_SEQUENCE_REMOVE;
EAPI extern int ECORE_X_EVENT_XKB_STATE_NOTIFY; /** @since 1.7 */
EAPI extern int ECORE_X_EVENT_XKB_NEWKBD_NOTIFY; /** @since 1.7 */

EAPI extern int ECORE_X_EVENT_GENERIC;

EAPI extern int ECORE_X_EVENT_PRESENT_CONFIGURE; /**< @since 1.9 */
EAPI extern int ECORE_X_EVENT_PRESENT_COMPLETE; /**< @since 1.9 */
EAPI extern int ECORE_X_EVENT_PRESENT_IDLE; /**< @since 1.9 */

EAPI extern int ECORE_X_EVENT_XDND_ENTER;
EAPI extern int ECORE_X_EVENT_XDND_POSITION;
EAPI extern int ECORE_X_EVENT_XDND_STATUS;
EAPI extern int ECORE_X_EVENT_XDND_LEAVE;
EAPI extern int ECORE_X_EVENT_XDND_DROP;
EAPI extern int ECORE_X_EVENT_XDND_FINISHED;

EAPI extern int ECORE_X_MODIFIER_SHIFT; /**< @since 1.7 */
EAPI extern int ECORE_X_MODIFIER_CTRL;  /**< @since 1.7 */
EAPI extern int ECORE_X_MODIFIER_ALT;   /**< @since 1.7 */
EAPI extern int ECORE_X_MODIFIER_WIN;   /**< @since 1.7 */
EAPI extern int ECORE_X_MODIFIER_ALTGR; /**< @since 1.7 */

EAPI extern int ECORE_X_LOCK_SCROLL;
EAPI extern int ECORE_X_LOCK_NUM;
EAPI extern int ECORE_X_LOCK_CAPS;
EAPI extern int ECORE_X_LOCK_SHIFT;

EAPI extern int ECORE_X_RAW_BUTTON_PRESS;   /**< @since 1.8 */
EAPI extern int ECORE_X_RAW_BUTTON_RELEASE; /**< @since 1.8 */
EAPI extern int ECORE_X_RAW_MOTION;         /**< @since 1.8 */

typedef enum _Ecore_X_WM_Protocol
{
   /** If enabled the window manager will be asked to send a
    * delete message instead of just closing (destroying) the window. */
   ECORE_X_WM_PROTOCOL_DELETE_REQUEST,

   /** If enabled the window manager will be told that the window
    * explicitly sets input focus. */
   ECORE_X_WM_PROTOCOL_TAKE_FOCUS,

   /** If enabled the window manager can ping the window to check
    * if it is alive. */
   ECORE_X_NET_WM_PROTOCOL_PING,

   /** If enabled the window manager can sync updating with the
    * window (?) */
   ECORE_X_NET_WM_PROTOCOL_SYNC_REQUEST,

   /** Number of defined items */
   ECORE_X_WM_PROTOCOL_NUM
} Ecore_X_WM_Protocol;

typedef enum _Ecore_X_Window_Input_Mode
{
   /** The window can never be focused */
   ECORE_X_WINDOW_INPUT_MODE_NONE,

   /** The window can be focused by the WM but doesn't focus itself */
   ECORE_X_WINDOW_INPUT_MODE_PASSIVE,

   /** The window sets the focus itself if one of its sub-windows
    * already is focused */
   ECORE_X_WINDOW_INPUT_MODE_ACTIVE_LOCAL,

   /** The window sets the focus itself even if another window
    * is currently focused */
   ECORE_X_WINDOW_INPUT_MODE_ACTIVE_GLOBAL
} Ecore_X_Window_Input_Mode;

/**
 * @typedef _Ecore_X_Window_State_Hint
 * Defines the different state hint of the window of Ecore_X.
 */
typedef enum _Ecore_X_Window_State_Hint
{
   /** Do not provide any state hint to the window manager */
   ECORE_X_WINDOW_STATE_HINT_NONE = -1,

   /** The window wants to remain hidden and NOT iconified */
   ECORE_X_WINDOW_STATE_HINT_WITHDRAWN,

   /** The window wants to be mapped normally */
   ECORE_X_WINDOW_STATE_HINT_NORMAL,

   /** The window wants to start in an iconified state */
   ECORE_X_WINDOW_STATE_HINT_ICONIC
} Ecore_X_Window_State_Hint;

typedef enum _Ecore_X_Window_Type
{
   ECORE_X_WINDOW_TYPE_UNKNOWN = 0,
   ECORE_X_WINDOW_TYPE_DESKTOP,
   ECORE_X_WINDOW_TYPE_DOCK,
   ECORE_X_WINDOW_TYPE_TOOLBAR,
   ECORE_X_WINDOW_TYPE_MENU,
   ECORE_X_WINDOW_TYPE_UTILITY,
   ECORE_X_WINDOW_TYPE_SPLASH,
   ECORE_X_WINDOW_TYPE_DIALOG,
   ECORE_X_WINDOW_TYPE_NORMAL,
   ECORE_X_WINDOW_TYPE_DROPDOWN_MENU,
   ECORE_X_WINDOW_TYPE_POPUP_MENU,
   ECORE_X_WINDOW_TYPE_TOOLTIP,
   ECORE_X_WINDOW_TYPE_NOTIFICATION,
   ECORE_X_WINDOW_TYPE_COMBO,
   ECORE_X_WINDOW_TYPE_DND
} Ecore_X_Window_Type;

typedef enum _Ecore_X_Action
{
   ECORE_X_ACTION_MOVE,
   ECORE_X_ACTION_RESIZE,
   ECORE_X_ACTION_MINIMIZE,
   ECORE_X_ACTION_SHADE,
   ECORE_X_ACTION_STICK,
   ECORE_X_ACTION_MAXIMIZE_HORZ,
   ECORE_X_ACTION_MAXIMIZE_VERT,
   ECORE_X_ACTION_FULLSCREEN,
   ECORE_X_ACTION_CHANGE_DESKTOP,
   ECORE_X_ACTION_CLOSE,
   ECORE_X_ACTION_ABOVE,
   ECORE_X_ACTION_BELOW
} Ecore_X_Action;

typedef enum _Ecore_X_Window_Configure_Mask
{
   ECORE_X_WINDOW_CONFIGURE_MASK_X = (1 << 0),
   ECORE_X_WINDOW_CONFIGURE_MASK_Y = (1 << 1),
   ECORE_X_WINDOW_CONFIGURE_MASK_W = (1 << 2),
   ECORE_X_WINDOW_CONFIGURE_MASK_H = (1 << 3),
   ECORE_X_WINDOW_CONFIGURE_MASK_BORDER_WIDTH = (1 << 4),
   ECORE_X_WINDOW_CONFIGURE_MASK_SIBLING = (1 << 5),
   ECORE_X_WINDOW_CONFIGURE_MASK_STACK_MODE = (1 << 6)
} Ecore_X_Window_Configure_Mask;

typedef enum _Ecore_X_Virtual_Keyboard_State
{
   ECORE_X_VIRTUAL_KEYBOARD_STATE_UNKNOWN = 0,
   ECORE_X_VIRTUAL_KEYBOARD_STATE_OFF,
   ECORE_X_VIRTUAL_KEYBOARD_STATE_ON,
   ECORE_X_VIRTUAL_KEYBOARD_STATE_ALPHA,
   ECORE_X_VIRTUAL_KEYBOARD_STATE_NUMERIC,
   ECORE_X_VIRTUAL_KEYBOARD_STATE_PIN,
   ECORE_X_VIRTUAL_KEYBOARD_STATE_PHONE_NUMBER,
   ECORE_X_VIRTUAL_KEYBOARD_STATE_HEX,
   ECORE_X_VIRTUAL_KEYBOARD_STATE_TERMINAL,
   ECORE_X_VIRTUAL_KEYBOARD_STATE_PASSWORD,
   ECORE_X_VIRTUAL_KEYBOARD_STATE_IP,
   ECORE_X_VIRTUAL_KEYBOARD_STATE_HOST,
   ECORE_X_VIRTUAL_KEYBOARD_STATE_FILE,
   ECORE_X_VIRTUAL_KEYBOARD_STATE_URL,
   ECORE_X_VIRTUAL_KEYBOARD_STATE_KEYPAD,
   ECORE_X_VIRTUAL_KEYBOARD_STATE_J2ME
} Ecore_X_Virtual_Keyboard_State;

typedef enum _Ecore_X_Illume_Mode
{
   ECORE_X_ILLUME_MODE_UNKNOWN = 0,
   ECORE_X_ILLUME_MODE_SINGLE,
   ECORE_X_ILLUME_MODE_DUAL_TOP,
   ECORE_X_ILLUME_MODE_DUAL_LEFT
} Ecore_X_Illume_Mode;

typedef enum _Ecore_X_Illume_Quickpanel_State
{
   ECORE_X_ILLUME_QUICKPANEL_STATE_UNKNOWN = 0,
   ECORE_X_ILLUME_QUICKPANEL_STATE_OFF,
   ECORE_X_ILLUME_QUICKPANEL_STATE_ON
} Ecore_X_Illume_Quickpanel_State;

typedef enum _Ecore_X_Illume_Indicator_State
{
   ECORE_X_ILLUME_INDICATOR_STATE_UNKNOWN = 0,
   ECORE_X_ILLUME_INDICATOR_STATE_OFF,
   ECORE_X_ILLUME_INDICATOR_STATE_ON
} Ecore_X_Illume_Indicator_State;

typedef enum _Ecore_X_Illume_Clipboard_State
{
   ECORE_X_ILLUME_CLIPBOARD_STATE_UNKNOWN = 0,
   ECORE_X_ILLUME_CLIPBOARD_STATE_OFF,
   ECORE_X_ILLUME_CLIPBOARD_STATE_ON
} Ecore_X_Illume_Clipboard_State;

typedef enum _Ecore_X_Illume_Indicator_Opacity_Mode
{
   ECORE_X_ILLUME_INDICATOR_OPACITY_UNKNOWN = 0,
   ECORE_X_ILLUME_INDICATOR_OPAQUE,
   ECORE_X_ILLUME_INDICATOR_TRANSLUCENT,
   ECORE_X_ILLUME_INDICATOR_TRANSPARENT
} Ecore_X_Illume_Indicator_Opacity_Mode;

typedef enum _Ecore_X_Illume_Indicator_Type_Mode
{
   ECORE_X_ILLUME_INDICATOR_TYPE_UNKNOWN = 0,
   ECORE_X_ILLUME_INDICATOR_TYPE_1,
   ECORE_X_ILLUME_INDICATOR_TYPE_2,
   ECORE_X_ILLUME_INDICATOR_TYPE_3
} Ecore_X_Illume_Indicator_Type_Mode; /**< @since 1.8 */

typedef enum _Ecore_X_Illume_Window_State
{
   ECORE_X_ILLUME_WINDOW_STATE_NORMAL = 0,
   ECORE_X_ILLUME_WINDOW_STATE_FLOATING
} Ecore_X_Illume_Window_State;

/* Window layer constants */
#define ECORE_X_WINDOW_LAYER_BELOW  2
#define ECORE_X_WINDOW_LAYER_NORMAL 4
#define ECORE_X_WINDOW_LAYER_ABOVE  6

/* Property list operations */
#define ECORE_X_PROP_LIST_REMOVE    0
#define ECORE_X_PROP_LIST_ADD       1
#define ECORE_X_PROP_LIST_TOGGLE    2

EAPI int                       ecore_x_init(const char *name);
EAPI int                       ecore_x_shutdown(void);
EAPI int                       ecore_x_disconnect(void);
EAPI Ecore_X_Display          *ecore_x_display_get(void);
EAPI Ecore_X_Connection       *ecore_x_connection_get(void);
EAPI int                       ecore_x_fd_get(void);
EAPI Ecore_X_Screen           *ecore_x_default_screen_get(void);
EAPI void                      ecore_x_screen_size_get(const Ecore_X_Screen *screen, int *w, int *h);
EAPI int                       ecore_x_screen_count_get(void);
EAPI int                       ecore_x_screen_index_get(const Ecore_X_Screen *screen);
EAPI Ecore_X_Screen           *ecore_x_screen_get(int index);

EAPI void                      ecore_x_double_click_time_set(double t);
EAPI double                    ecore_x_double_click_time_get(void);
EAPI void                      ecore_x_flush(void);
EAPI void                      ecore_x_sync(void);
EAPI void                      ecore_x_killall(Ecore_X_Window root);
EAPI void                      ecore_x_kill(Ecore_X_Window win);
EAPI int                       ecore_x_dpi_get(void);
EAPI Eina_Bool                 ecore_x_bell(int percent);
EAPI unsigned int              ecore_x_visual_id_get(Ecore_X_Visual visual);

EAPI Ecore_X_Visual            ecore_x_default_visual_get(Ecore_X_Display *disp, Ecore_X_Screen *screen);
EAPI Ecore_X_Colormap          ecore_x_default_colormap_get(Ecore_X_Display *disp, Ecore_X_Screen *screen);
EAPI int                       ecore_x_default_depth_get(Ecore_X_Display *disp, Ecore_X_Screen *screen);

EAPI Ecore_X_Time              ecore_x_current_time_get(void);

EAPI void                      ecore_x_error_handler_set(void (*func)(void *data), const void *data);
EAPI void                      ecore_x_io_error_handler_set(void (*func)(void *data), const void *data);
EAPI int                       ecore_x_error_request_get(void);
EAPI int                       ecore_x_error_code_get(void);
EAPI Ecore_X_ID                ecore_x_error_resource_id_get(void);

EAPI void                      ecore_x_event_mask_set(Ecore_X_Window w, Ecore_X_Event_Mask mask);
EAPI void                      ecore_x_event_mask_unset(Ecore_X_Window w, Ecore_X_Event_Mask mask);

EAPI Eina_Bool                 ecore_x_selection_notify_send(Ecore_X_Window requestor, Ecore_X_Atom selection, Ecore_X_Atom target, Ecore_X_Atom property, Ecore_X_Time time);
EAPI Eina_Bool                 ecore_x_selection_primary_set(Ecore_X_Window w, const void *data, int size);
EAPI Eina_Bool                 ecore_x_selection_primary_clear(void);
EAPI Eina_Bool                 ecore_x_selection_secondary_set(Ecore_X_Window w, const void *data, int size);
EAPI Eina_Bool                 ecore_x_selection_secondary_clear(void);
EAPI Eina_Bool                 ecore_x_selection_xdnd_set(Ecore_X_Window w, const void *data, int size);
EAPI Eina_Bool                 ecore_x_selection_xdnd_clear(void);
EAPI Eina_Bool                 ecore_x_selection_clipboard_set(Ecore_X_Window w, const void *data, int size);
EAPI Eina_Bool                 ecore_x_selection_clipboard_clear(void);
EAPI void                      ecore_x_selection_primary_request(Ecore_X_Window w, const char *target);
EAPI void                      ecore_x_selection_secondary_request(Ecore_X_Window w, const char *target);
EAPI void                      ecore_x_selection_xdnd_request(Ecore_X_Window w, const char *target);
EAPI void                      ecore_x_selection_clipboard_request(Ecore_X_Window w, const char *target);
EAPI Eina_Bool                 ecore_x_selection_convert(Ecore_X_Atom selection, Ecore_X_Atom target, void **data_ret, int *len, Ecore_X_Atom *targprop, int *targsize);
EAPI void                      ecore_x_selection_converter_add(char *target, Eina_Bool (*func)(char *target, void *data, int size, void **data_ret, int *size_ret, Ecore_X_Atom *, int *));
EAPI void                      ecore_x_selection_converter_atom_add(Ecore_X_Atom target, Eina_Bool (*func)(char *target, void *data, int size, void **data_ret, int *size_ret, Ecore_X_Atom *tprop, int *tsize));
EAPI void                      ecore_x_selection_converter_del(char *target);
EAPI void                      ecore_x_selection_converter_atom_del(Ecore_X_Atom target);
EAPI void                      ecore_x_selection_parser_add(const char *target, void *(*func)(const char *target, void *data, int size, int format));
EAPI void                      ecore_x_selection_parser_del(const char *target);
EAPI void                      ecore_x_selection_owner_set(Ecore_X_Window win, Ecore_X_Atom atom, Ecore_X_Time tm);
EAPI Ecore_X_Window            ecore_x_selection_owner_get(Ecore_X_Atom atom);
EAPI Eina_Bool                 ecore_x_selection_converter_text(char *target, void *data, int size, void **data_ret, int *size_ret, Ecore_X_Atom *targprop, int *s); /** @since 1.8 */

EAPI void                      ecore_x_dnd_aware_set(Ecore_X_Window win, Eina_Bool on);
EAPI int                       ecore_x_dnd_version_get(Ecore_X_Window win);
EAPI Eina_Bool                 ecore_x_dnd_type_isset(Ecore_X_Window win, const char *type);
EAPI void                      ecore_x_dnd_type_set(Ecore_X_Window win, const char *type, Eina_Bool on);
EAPI void                      ecore_x_dnd_types_set(Ecore_X_Window win, const char **types, unsigned int num_types);
EAPI void                      ecore_x_dnd_actions_set(Ecore_X_Window win, Ecore_X_Atom *actions, unsigned int num_actions);
EAPI Eina_Bool                 ecore_x_dnd_begin(Ecore_X_Window source, unsigned char *data, int size);
EAPI Eina_Bool                 ecore_x_dnd_drop(void);
EAPI Eina_Bool                 ecore_x_dnd_self_begin(Ecore_X_Window source, unsigned char *data, int size); /**< @since 1.8 */
EAPI Eina_Bool                 ecore_x_dnd_self_drop(void); /**< @since 1.8 */
EAPI void                      ecore_x_dnd_send_status(Eina_Bool will_accept, Eina_Bool suppress, Ecore_X_Rectangle rectangle, Ecore_X_Atom action);
EAPI void                      ecore_x_dnd_send_finished(void);
EAPI void                      ecore_x_dnd_source_action_set(Ecore_X_Atom action);
EAPI Ecore_X_Atom              ecore_x_dnd_source_action_get(void);
EAPI void                      ecore_x_dnd_callback_pos_update_set(void (*cb)(void *, Ecore_X_Xdnd_Position *data), const void *data);
EAPI Eina_Bool                 ecore_x_dnd_abort(Ecore_X_Window xwin_source); /**< @since 1.9 */

EAPI Ecore_X_Window            ecore_x_window_new(Ecore_X_Window parent, int x, int y, int w, int h);
EAPI Ecore_X_Window            ecore_x_window_override_new(Ecore_X_Window parent, int x, int y, int w, int h);
EAPI int                       ecore_x_window_argb_get(Ecore_X_Window win);
EAPI Ecore_X_Window            ecore_x_window_manager_argb_new(Ecore_X_Window parent, int x, int y, int w, int h);
EAPI Ecore_X_Window            ecore_x_window_argb_new(Ecore_X_Window parent, int x, int y, int w, int h);
EAPI Ecore_X_Window            ecore_x_window_override_argb_new(Ecore_X_Window parent, int x, int y, int w, int h);
EAPI Ecore_X_Window            ecore_x_window_permanent_new(Ecore_X_Window parent, Ecore_X_Atom unique_atom); /* @since 1.9 */
EAPI Ecore_X_Window            ecore_x_window_input_new(Ecore_X_Window parent, int x, int y, int w, int h);
EAPI void                      ecore_x_window_configure(Ecore_X_Window win, Ecore_X_Window_Configure_Mask mask, int x, int y, int w, int h, int border_width, Ecore_X_Window sibling, int stack_mode);
EAPI void                      ecore_x_window_cursor_set(Ecore_X_Window win, Ecore_X_Cursor c);
EAPI void                      ecore_x_window_free(Ecore_X_Window win);
EAPI void                      ecore_x_window_ignore_set(Ecore_X_Window win, int ignore);
EAPI Ecore_X_Window           *ecore_x_window_ignore_list(int *num);

EAPI void                      ecore_x_window_delete_request_send(Ecore_X_Window win);
EAPI void                      ecore_x_window_show(Ecore_X_Window win);
EAPI void                      ecore_x_window_hide(Ecore_X_Window win);
EAPI void                      ecore_x_window_move(Ecore_X_Window win, int x, int y);
EAPI void                      ecore_x_window_resize(Ecore_X_Window win, int w, int h);
EAPI void                      ecore_x_window_move_resize(Ecore_X_Window win, int x, int y, int w, int h);
EAPI void                      ecore_x_window_focus(Ecore_X_Window win);
EAPI void                      ecore_x_window_focus_at_time(Ecore_X_Window win, Ecore_X_Time t);
EAPI Ecore_X_Window            ecore_x_window_focus_get(void);
EAPI void                      ecore_x_window_raise(Ecore_X_Window win);
EAPI void                      ecore_x_window_lower(Ecore_X_Window win);
EAPI void                      ecore_x_window_reparent(Ecore_X_Window win, Ecore_X_Window new_parent, int x, int y);
EAPI void                      ecore_x_window_size_get(Ecore_X_Window win, int *w, int *h);
EAPI void                      ecore_x_window_geometry_get(Ecore_X_Window win, int *x, int *y, int *w, int *h);
EAPI int                       ecore_x_window_border_width_get(Ecore_X_Window win);
EAPI void                      ecore_x_window_border_width_set(Ecore_X_Window win, int width);
EAPI int                       ecore_x_window_depth_get(Ecore_X_Window win);
EAPI void                      ecore_x_window_cursor_show(Ecore_X_Window win, Eina_Bool show);
EAPI void                      ecore_x_window_defaults_set(Ecore_X_Window win);
EAPI int                       ecore_x_window_visible_get(Ecore_X_Window win);
EAPI Ecore_X_Window            ecore_x_window_shadow_tree_at_xy_with_skip_get(Ecore_X_Window base, int x, int y, Ecore_X_Window *skip, int skip_num);
EAPI Ecore_X_Window            ecore_x_window_shadow_parent_get(Ecore_X_Window root, Ecore_X_Window win);
EAPI void                      ecore_x_window_shadow_tree_flush(void);
EAPI Ecore_X_Window            ecore_x_window_root_get(Ecore_X_Window win);
EAPI Ecore_X_Window            ecore_x_window_at_xy_get(int x, int y);
EAPI Ecore_X_Window            ecore_x_window_at_xy_with_skip_get(int x, int y, Ecore_X_Window *skip, int skip_num);
EAPI Ecore_X_Window            ecore_x_window_at_xy_begin_get(Ecore_X_Window begin, int x, int y);
EAPI Ecore_X_Window            ecore_x_window_parent_get(Ecore_X_Window win);

EAPI void                      ecore_x_window_background_color_set(Ecore_X_Window win, unsigned short r, unsigned short g, unsigned short b);
EAPI void                      ecore_x_window_gravity_set(Ecore_X_Window win, Ecore_X_Gravity grav);
EAPI void                      ecore_x_window_pixel_gravity_set(Ecore_X_Window win, Ecore_X_Gravity grav);
EAPI void                      ecore_x_window_pixmap_set(Ecore_X_Window win, Ecore_X_Pixmap pmap);
EAPI void                      ecore_x_window_area_clear(Ecore_X_Window win, int x, int y, int w, int h);
EAPI void                      ecore_x_window_area_expose(Ecore_X_Window win, int x, int y, int w, int h);
EAPI void                      ecore_x_window_override_set(Ecore_X_Window win, Eina_Bool override);

EAPI void                      ecore_x_window_prop_card32_set(Ecore_X_Window win, Ecore_X_Atom atom, unsigned int *val, unsigned int num);
EAPI int                       ecore_x_window_prop_card32_get(Ecore_X_Window win, Ecore_X_Atom atom, unsigned int *val, unsigned int len);
EAPI int                       ecore_x_window_prop_card32_list_get(Ecore_X_Window win, Ecore_X_Atom atom, unsigned int **plst);

EAPI void                      ecore_x_window_prop_xid_set(Ecore_X_Window win, Ecore_X_Atom atom, Ecore_X_Atom type, Ecore_X_ID *lst, unsigned int num);
EAPI int                       ecore_x_window_prop_xid_get(Ecore_X_Window win, Ecore_X_Atom atom, Ecore_X_Atom type, Ecore_X_ID *lst, unsigned int len);
EAPI int                       ecore_x_window_prop_xid_list_get(Ecore_X_Window win, Ecore_X_Atom atom, Ecore_X_Atom type, Ecore_X_ID **plst);
EAPI void                      ecore_x_window_prop_xid_list_change(Ecore_X_Window win, Ecore_X_Atom atom, Ecore_X_Atom type, Ecore_X_ID item, int op);
EAPI void                      ecore_x_window_prop_atom_set(Ecore_X_Window win, Ecore_X_Atom atom, Ecore_X_Atom *val, unsigned int num);
EAPI int                       ecore_x_window_prop_atom_get(Ecore_X_Window win, Ecore_X_Atom atom, Ecore_X_Atom *val, unsigned int len);
EAPI int                       ecore_x_window_prop_atom_list_get(Ecore_X_Window win, Ecore_X_Atom atom, Ecore_X_Atom **plst);
EAPI void                      ecore_x_window_prop_atom_list_change(Ecore_X_Window win, Ecore_X_Atom atom, Ecore_X_Atom item, int op);
EAPI void                      ecore_x_window_prop_window_set(Ecore_X_Window win, Ecore_X_Atom atom, Ecore_X_Window *val, unsigned int num);
EAPI int                       ecore_x_window_prop_window_get(Ecore_X_Window win, Ecore_X_Atom atom, Ecore_X_Window *val, unsigned int len);
EAPI int                       ecore_x_window_prop_window_list_get(Ecore_X_Window win, Ecore_X_Atom atom, Ecore_X_Window **plst);

EAPI Ecore_X_Atom              ecore_x_window_prop_any_type(void);
EAPI void                      ecore_x_window_prop_property_set(Ecore_X_Window win, Ecore_X_Atom type, Ecore_X_Atom format, int size, void *data, int number);
EAPI int                       ecore_x_window_prop_property_get(Ecore_X_Window win, Ecore_X_Atom property, Ecore_X_Atom type, int size, unsigned char **data, int *num);
EAPI void                      ecore_x_window_prop_property_del(Ecore_X_Window win, Ecore_X_Atom property);
EAPI Ecore_X_Atom             *ecore_x_window_prop_list(Ecore_X_Window win, int *num_ret);
EAPI void                      ecore_x_window_prop_string_set(Ecore_X_Window win, Ecore_X_Atom type, const char *str);
EAPI char                     *ecore_x_window_prop_string_get(Ecore_X_Window win, Ecore_X_Atom type);
EAPI Eina_Bool                 ecore_x_window_prop_protocol_isset(Ecore_X_Window win, Ecore_X_WM_Protocol protocol);
EAPI Ecore_X_WM_Protocol      *ecore_x_window_prop_protocol_list_get(Ecore_X_Window win, int *num_ret);

EAPI void                      ecore_x_window_shape_mask_set(Ecore_X_Window win, Ecore_X_Pixmap mask);
EAPI void                      ecore_x_window_shape_window_set(Ecore_X_Window win, Ecore_X_Window shape_win);
EAPI void                      ecore_x_window_shape_window_set_xy(Ecore_X_Window win, Ecore_X_Window shape_win, int x, int y);
EAPI void                      ecore_x_window_shape_rectangle_set(Ecore_X_Window win, int x, int y, int w, int h);
EAPI void                      ecore_x_window_shape_rectangles_set(Ecore_X_Window win, Ecore_X_Rectangle *rects, int num);
EAPI void                      ecore_x_window_shape_input_rectangle_set(Ecore_X_Window win, int x, int y, int w, int h);
EAPI void                      ecore_x_window_shape_input_rectangles_set(Ecore_X_Window win, Ecore_X_Rectangle *rects, int num);
EAPI void                      ecore_x_window_shape_input_rectangle_add(Ecore_X_Window win, int x, int y, int w, int h);
EAPI void                      ecore_x_window_shape_rectangle_subtract(Ecore_X_Window win, int x, int y, int w, int h);
EAPI void                      ecore_x_window_shape_input_rectangle_subtract(Ecore_X_Window win, int x, int y, int w, int h);
EAPI void                      ecore_x_window_shape_input_window_set_xy(Ecore_X_Window win, Ecore_X_Window shape_win, int x, int y);
EAPI void                      ecore_x_window_shape_input_window_set(Ecore_X_Window win, Ecore_X_Window shape_win);
EAPI void                      ecore_x_window_shape_window_add(Ecore_X_Window win, Ecore_X_Window shape_win);
EAPI void                      ecore_x_window_shape_window_add_xy(Ecore_X_Window win, Ecore_X_Window shape_win, int x, int y);
EAPI void                      ecore_x_window_shape_input_window_add_xy(Ecore_X_Window win, Ecore_X_Window shape_win, int x, int y);
EAPI void                      ecore_x_window_shape_rectangle_add(Ecore_X_Window win, int x, int y, int w, int h);
EAPI void                      ecore_x_window_shape_rectangle_clip(Ecore_X_Window win, int x, int y, int w, int h);
EAPI void                      ecore_x_window_shape_input_rectangle_clip(Ecore_X_Window win, int x, int y, int w, int h);
EAPI void                      ecore_x_window_shape_rectangles_add(Ecore_X_Window win, Ecore_X_Rectangle *rects, int num);
EAPI void                      ecore_x_window_shape_input_rectangles_add(Ecore_X_Window win, Ecore_X_Rectangle *rects, int num);
EAPI Ecore_X_Rectangle        *ecore_x_window_shape_rectangles_get(Ecore_X_Window win, int *num_ret);
EAPI Ecore_X_Rectangle        *ecore_x_window_shape_input_rectangles_get(Ecore_X_Window win, int *num_ret);
EAPI void                      ecore_x_window_shape_events_select(Ecore_X_Window win, Eina_Bool on);
EAPI void                      ecore_x_window_shape_input_mask_set(Ecore_X_Window win, Ecore_X_Pixmap mask);

EAPI Ecore_X_Pixmap            ecore_x_pixmap_new(Ecore_X_Window win, int w, int h, int dep);
EAPI void                      ecore_x_pixmap_free(Ecore_X_Pixmap pmap);
EAPI void                      ecore_x_pixmap_paste(Ecore_X_Pixmap pmap, Ecore_X_Drawable dest, Ecore_X_GC gc, int sx, int sy, int w, int h, int dx, int dy);
EAPI void                      ecore_x_pixmap_geometry_get(Ecore_X_Pixmap pmap, int *x, int *y, int *w, int *h);
EAPI int                       ecore_x_pixmap_depth_get(Ecore_X_Pixmap pmap);

EAPI Ecore_X_GC                ecore_x_gc_new(Ecore_X_Drawable draw, Ecore_X_GC_Value_Mask value_mask, const unsigned int *value_list);
EAPI void                      ecore_x_gc_free(Ecore_X_GC gc);
EAPI void                      ecore_x_gc_foreground_set(Ecore_X_GC gc, unsigned long foreground);
EAPI void                      ecore_x_gc_background_set(Ecore_X_GC gc, unsigned long background);

EAPI Eina_Bool                 ecore_x_client_message32_send(Ecore_X_Window win, Ecore_X_Atom type, Ecore_X_Event_Mask mask, long d0, long d1, long d2, long d3, long d4);
EAPI Eina_Bool                 ecore_x_client_message8_send(Ecore_X_Window win, Ecore_X_Atom type, const void *data, int len);
EAPI Eina_Bool                 ecore_x_mouse_move_send(Ecore_X_Window win, int x, int y);
EAPI Eina_Bool                 ecore_x_mouse_down_send(Ecore_X_Window win, int x, int y, int b);
EAPI Eina_Bool                 ecore_x_mouse_up_send(Ecore_X_Window win, int x, int y, int b);
EAPI Eina_Bool                 ecore_x_mouse_in_send(Ecore_X_Window win, int x, int y);
EAPI Eina_Bool                 ecore_x_mouse_out_send(Ecore_X_Window win, int x, int y);

EAPI void                      ecore_x_drawable_geometry_get(Ecore_X_Drawable d, int *x, int *y, int *w, int *h);
EAPI int                       ecore_x_drawable_border_width_get(Ecore_X_Drawable d);
EAPI int                       ecore_x_drawable_depth_get(Ecore_X_Drawable d);
EAPI void                      ecore_x_drawable_rectangle_fill(Ecore_X_Drawable d, Ecore_X_GC gc, int x, int y, int width, int height);

EAPI Eina_Bool                 ecore_x_cursor_color_supported_get(void);
EAPI Ecore_X_Cursor            ecore_x_cursor_new(Ecore_X_Window win, int *pixels, int w, int h, int hot_x, int hot_y);
EAPI void                      ecore_x_cursor_free(Ecore_X_Cursor c);
EAPI Ecore_X_Cursor            ecore_x_cursor_shape_get(int shape);
EAPI void                      ecore_x_cursor_size_set(int size);
EAPI int                       ecore_x_cursor_size_get(void);

/* FIXME: these funcs need categorising */
EAPI Ecore_X_Window           *ecore_x_window_root_list(int *num_ret);
EAPI Ecore_X_Window            ecore_x_window_root_first_get(void);
EAPI Eina_Bool                 ecore_x_window_manage(Ecore_X_Window win);
EAPI void                      ecore_x_window_container_manage(Ecore_X_Window win);
EAPI void                      ecore_x_window_client_manage(Ecore_X_Window win);
EAPI void                      ecore_x_window_sniff(Ecore_X_Window win);
EAPI void                      ecore_x_window_client_sniff(Ecore_X_Window win);

EAPI Ecore_X_Atom              ecore_x_atom_get(const char *name);
EAPI void                      ecore_x_atoms_get(const char **names, int num, Ecore_X_Atom *atoms);
EAPI char                     *ecore_x_atom_name_get(Ecore_X_Atom atom);

EAPI void                      ecore_x_icccm_init(void);
EAPI void                      ecore_x_icccm_state_set(Ecore_X_Window win, Ecore_X_Window_State_Hint state);
EAPI Ecore_X_Window_State_Hint ecore_x_icccm_state_get(Ecore_X_Window win);
EAPI void                      ecore_x_icccm_delete_window_send(Ecore_X_Window win, Ecore_X_Time t);
EAPI void                      ecore_x_icccm_take_focus_send(Ecore_X_Window win, Ecore_X_Time t);
EAPI void                      ecore_x_icccm_save_yourself_send(Ecore_X_Window win, Ecore_X_Time t);
EAPI void                      ecore_x_icccm_move_resize_send(Ecore_X_Window win, int x, int y, int w, int h);
EAPI void                      ecore_x_icccm_hints_set(Ecore_X_Window win, Eina_Bool accepts_focus, Ecore_X_Window_State_Hint initial_state, Ecore_X_Pixmap icon_pixmap, Ecore_X_Pixmap icon_mask, Ecore_X_Window icon_window, Ecore_X_Window window_group, Eina_Bool is_urgent);
EAPI Eina_Bool                 ecore_x_icccm_hints_get(Ecore_X_Window win, Eina_Bool *accepts_focus, Ecore_X_Window_State_Hint *initial_state, Ecore_X_Pixmap *icon_pixmap, Ecore_X_Pixmap *icon_mask, Ecore_X_Window *icon_window, Ecore_X_Window *window_group, Eina_Bool *is_urgent);
EAPI void                      ecore_x_icccm_size_pos_hints_set(Ecore_X_Window win, Eina_Bool request_pos, Ecore_X_Gravity gravity, int min_w, int min_h, int max_w, int max_h, int base_w, int base_h, int step_x, int step_y, double min_aspect, double max_aspect);
EAPI Eina_Bool                 ecore_x_icccm_size_pos_hints_get(Ecore_X_Window win, Eina_Bool *request_pos, Ecore_X_Gravity *gravity, int *min_w, int *min_h, int *max_w, int *max_h, int *base_w, int *base_h, int *step_x, int *step_y, double *min_aspect, double *max_aspect);
EAPI void                      ecore_x_icccm_title_set(Ecore_X_Window win, const char *t);
EAPI char                     *ecore_x_icccm_title_get(Ecore_X_Window win);
EAPI void                      ecore_x_icccm_protocol_atoms_set(Ecore_X_Window win, Ecore_X_Atom *protos, int num);
EAPI void                      ecore_x_icccm_protocol_set(Ecore_X_Window win, Ecore_X_WM_Protocol protocol, Eina_Bool on);
EAPI Eina_Bool                 ecore_x_icccm_protocol_isset(Ecore_X_Window win, Ecore_X_WM_Protocol protocol);
EAPI void                      ecore_x_icccm_name_class_set(Ecore_X_Window win, const char *n, const char *c);
EAPI void                      ecore_x_icccm_name_class_get(Ecore_X_Window win, char **n, char **c);
EAPI char                     *ecore_x_icccm_client_machine_get(Ecore_X_Window win);
EAPI void                      ecore_x_icccm_command_set(Ecore_X_Window win, int argc, char **argv);
EAPI void                      ecore_x_icccm_command_get(Ecore_X_Window win, int *argc, char ***argv);
EAPI char                     *ecore_x_icccm_icon_name_get(Ecore_X_Window win);
EAPI void                      ecore_x_icccm_icon_name_set(Ecore_X_Window win, const char *t);
EAPI void                      ecore_x_icccm_colormap_window_set(Ecore_X_Window win, Ecore_X_Window subwin);
EAPI void                      ecore_x_icccm_colormap_window_unset(Ecore_X_Window win, Ecore_X_Window subwin);
EAPI void                      ecore_x_icccm_transient_for_set(Ecore_X_Window win, Ecore_X_Window forwin);
EAPI void                      ecore_x_icccm_transient_for_unset(Ecore_X_Window win);
EAPI Ecore_X_Window            ecore_x_icccm_transient_for_get(Ecore_X_Window win);
EAPI void                      ecore_x_icccm_window_role_set(Ecore_X_Window win, const char *role);
EAPI char                     *ecore_x_icccm_window_role_get(Ecore_X_Window win);
EAPI void                      ecore_x_icccm_client_leader_set(Ecore_X_Window win, Ecore_X_Window l);
EAPI Ecore_X_Window            ecore_x_icccm_client_leader_get(Ecore_X_Window win);
EAPI void                      ecore_x_icccm_iconic_request_send(Ecore_X_Window win, Ecore_X_Window root);

typedef enum _Ecore_X_MWM_Hint_Func
{
   ECORE_X_MWM_HINT_FUNC_ALL = (1 << 0),
   ECORE_X_MWM_HINT_FUNC_RESIZE = (1 << 1),
   ECORE_X_MWM_HINT_FUNC_MOVE = (1 << 2),
   ECORE_X_MWM_HINT_FUNC_MINIMIZE = (1 << 3),
   ECORE_X_MWM_HINT_FUNC_MAXIMIZE = (1 << 4),
   ECORE_X_MWM_HINT_FUNC_CLOSE = (1 << 5)
} Ecore_X_MWM_Hint_Func;

typedef enum _Ecore_X_MWM_Hint_Decor
{
   ECORE_X_MWM_HINT_DECOR_ALL = (1 << 0),
   ECORE_X_MWM_HINT_DECOR_BORDER = (1 << 1),
   ECORE_X_MWM_HINT_DECOR_RESIZEH = (1 << 2),
   ECORE_X_MWM_HINT_DECOR_TITLE = (1 << 3),
   ECORE_X_MWM_HINT_DECOR_MENU = (1 << 4),
   ECORE_X_MWM_HINT_DECOR_MINIMIZE = (1 << 5),
   ECORE_X_MWM_HINT_DECOR_MAXIMIZE = (1 << 6)
} Ecore_X_MWM_Hint_Decor;

typedef enum _Ecore_X_MWM_Hint_Input
{
   ECORE_X_MWM_HINT_INPUT_MODELESS = 0,
   ECORE_X_MWM_HINT_INPUT_PRIMARY_APPLICATION_MODAL = 1,
   ECORE_X_MWM_HINT_INPUT_SYSTEM_MODAL = 2,
   ECORE_X_MWM_HINT_INPUT_FULL_APPLICATION_MODAL = 3
} Ecore_X_MWM_Hint_Input;

EAPI Eina_Bool                       ecore_x_mwm_hints_get(Ecore_X_Window win, Ecore_X_MWM_Hint_Func *fhint, Ecore_X_MWM_Hint_Decor *dhint, Ecore_X_MWM_Hint_Input *ihint);
EAPI void                            ecore_x_mwm_borderless_set(Ecore_X_Window win, Eina_Bool borderless);

/* netwm */
EAPI void                            ecore_x_netwm_init(void);
EAPI void                            ecore_x_netwm_shutdown(void);
EAPI void                            ecore_x_netwm_wm_identify(Ecore_X_Window root, Ecore_X_Window check, const char *wm_name);
EAPI void                            ecore_x_netwm_supported_set(Ecore_X_Window root, Ecore_X_Atom *supported, int num);
EAPI Eina_Bool                       ecore_x_netwm_supported_get(Ecore_X_Window root, Ecore_X_Atom **supported, int *num);
EAPI void                            ecore_x_netwm_desk_count_set(Ecore_X_Window root, unsigned int n_desks);
EAPI void                            ecore_x_netwm_desk_roots_set(Ecore_X_Window root, Ecore_X_Window *vroots, unsigned int n_desks);
EAPI void                            ecore_x_netwm_desk_names_set(Ecore_X_Window root, const char **names, unsigned int n_desks);
EAPI void                            ecore_x_netwm_desk_size_set(Ecore_X_Window root, unsigned int width, unsigned int height);
EAPI void                            ecore_x_netwm_desk_workareas_set(Ecore_X_Window root, unsigned int *areas, unsigned int n_desks);
EAPI unsigned int                   *ecore_x_netwm_desk_workareas_get(Ecore_X_Window root, unsigned int *n_desks);
EAPI void                            ecore_x_netwm_desk_current_set(Ecore_X_Window root, unsigned int desk);
EAPI void                            ecore_x_netwm_desk_viewports_set(Ecore_X_Window root, unsigned int *origins, unsigned int n_desks);
EAPI void                            ecore_x_netwm_desk_layout_set(Ecore_X_Window root, int orientation, int columns, int rows, int starting_corner);
EAPI void                            ecore_x_netwm_showing_desktop_set(Ecore_X_Window root, Eina_Bool on);
EAPI void                            ecore_x_netwm_client_list_set(Ecore_X_Window root, Ecore_X_Window *p_clients, unsigned int n_clients);
EAPI void                            ecore_x_netwm_client_list_stacking_set(Ecore_X_Window root, Ecore_X_Window *p_clients, unsigned int n_clients);
EAPI void                            ecore_x_netwm_client_active_set(Ecore_X_Window root, Ecore_X_Window win);
EAPI void                            ecore_x_netwm_client_active_request(Ecore_X_Window root, Ecore_X_Window win, int type, Ecore_X_Window current_win);
EAPI void                            ecore_x_netwm_name_set(Ecore_X_Window win, const char *name);
EAPI int                             ecore_x_netwm_name_get(Ecore_X_Window win, char **name);
EAPI void                            ecore_x_netwm_startup_id_set(Ecore_X_Window win, const char *id);
EAPI int                             ecore_x_netwm_startup_id_get(Ecore_X_Window win, char **id);
EAPI void                            ecore_x_netwm_visible_name_set(Ecore_X_Window win, const char *name);
EAPI int                             ecore_x_netwm_visible_name_get(Ecore_X_Window win, char **name);
EAPI void                            ecore_x_netwm_icon_name_set(Ecore_X_Window win, const char *name);
EAPI int                             ecore_x_netwm_icon_name_get(Ecore_X_Window win, char **name);
EAPI void                            ecore_x_netwm_visible_icon_name_set(Ecore_X_Window win, const char *name);
EAPI int                             ecore_x_netwm_visible_icon_name_get(Ecore_X_Window win, char **name);
EAPI void                            ecore_x_netwm_desktop_set(Ecore_X_Window win, unsigned int desk);
EAPI Eina_Bool                       ecore_x_netwm_desktop_get(Ecore_X_Window win, unsigned int *desk);
EAPI void                            ecore_x_netwm_strut_set(Ecore_X_Window win, int left, int right, int top, int bottom);
EAPI Eina_Bool                       ecore_x_netwm_strut_get(Ecore_X_Window win, int *left, int *right, int *top, int *bottom);
EAPI void                            ecore_x_netwm_strut_partial_set(Ecore_X_Window win, int left, int right, int top, int bottom, int left_start_y, int left_end_y, int right_start_y, int right_end_y, int top_start_x, int top_end_x, int bottom_start_x, int bottom_end_x);
EAPI Eina_Bool                       ecore_x_netwm_strut_partial_get(Ecore_X_Window win, int *left, int *right, int *top, int *bottom, int *left_start_y, int *left_end_y, int *right_start_y, int *right_end_y, int *top_start_x, int *top_end_x, int *bottom_start_x, int *bottom_end_x);

EAPI void                            ecore_x_netwm_icons_set(Ecore_X_Window win, Ecore_X_Icon *icon, int num);

EAPI Eina_Bool                       ecore_x_netwm_icons_get(Ecore_X_Window win, Ecore_X_Icon **icon, int *num);
EAPI void                            ecore_x_netwm_icon_geometry_set(Ecore_X_Window win, int x, int y, int width, int height);
EAPI Eina_Bool                       ecore_x_netwm_icon_geometry_get(Ecore_X_Window win, int *x, int *y, int *width, int *height);
EAPI void                            ecore_x_netwm_pid_set(Ecore_X_Window win, int pid);
EAPI Eina_Bool                       ecore_x_netwm_pid_get(Ecore_X_Window win, int *pid);
EAPI void                            ecore_x_netwm_handled_icons_set(Ecore_X_Window win);
EAPI Eina_Bool                       ecore_x_netwm_handled_icons_get(Ecore_X_Window win);
EAPI void                            ecore_x_netwm_user_time_set(Ecore_X_Window win, unsigned int time);
EAPI Eina_Bool                       ecore_x_netwm_user_time_get(Ecore_X_Window win, unsigned int *time);
EAPI void                            ecore_x_netwm_window_state_set(Ecore_X_Window win, Ecore_X_Window_State *state, unsigned int num);
EAPI Eina_Bool                       ecore_x_netwm_window_state_get(Ecore_X_Window win, Ecore_X_Window_State **state, unsigned int *num);
EAPI void                            ecore_x_netwm_window_type_set(Ecore_X_Window win, Ecore_X_Window_Type type);
EAPI Eina_Bool                       ecore_x_netwm_window_type_get(Ecore_X_Window win, Ecore_X_Window_Type *type);
EAPI int                             ecore_x_netwm_window_types_get(Ecore_X_Window win, Ecore_X_Window_Type **types);
EAPI Eina_Bool                       ecore_x_netwm_allowed_action_isset(Ecore_X_Window win, Ecore_X_Action action);
EAPI void                            ecore_x_netwm_allowed_action_set(Ecore_X_Window win, Ecore_X_Action *action, unsigned int num);
EAPI Eina_Bool                       ecore_x_netwm_allowed_action_get(Ecore_X_Window win, Ecore_X_Action **action, unsigned int *num);
EAPI void                            ecore_x_netwm_opacity_set(Ecore_X_Window win, unsigned int opacity);
EAPI Eina_Bool                       ecore_x_netwm_opacity_get(Ecore_X_Window win, unsigned int *opacity);
EAPI void                            ecore_x_netwm_frame_size_set(Ecore_X_Window win, int fl, int fr, int ft, int fb);
EAPI Eina_Bool                       ecore_x_netwm_frame_size_get(Ecore_X_Window win, int *fl, int *fr, int *ft, int *fb);
EAPI Eina_Bool                       ecore_x_netwm_sync_counter_get(Ecore_X_Window win, Ecore_X_Sync_Counter *counter);
EAPI void                            ecore_x_netwm_ping_send(Ecore_X_Window win);
EAPI void                            ecore_x_netwm_sync_request_send(Ecore_X_Window win, unsigned int serial);
EAPI void                            ecore_x_netwm_state_request_send(Ecore_X_Window win, Ecore_X_Window root, Ecore_X_Window_State s1, Ecore_X_Window_State s2, Eina_Bool set);
EAPI void                            ecore_x_netwm_desktop_request_send(Ecore_X_Window win, Ecore_X_Window root, unsigned int desktop);
EAPI void                            ecore_x_netwm_moveresize_request_send(Ecore_X_Window win, int x, int y, Ecore_X_Netwm_Direction direction, unsigned int button);

EAPI void                            ecore_x_e_init(void);
EAPI void                            ecore_x_e_frame_size_set(Ecore_X_Window win, int fl, int fr, int ft, int fb);
EAPI void                            ecore_x_e_virtual_keyboard_set(Ecore_X_Window win, unsigned int is_keyboard);
EAPI Eina_Bool                       ecore_x_e_virtual_keyboard_get(Ecore_X_Window win);
EAPI void                            ecore_x_e_virtual_keyboard_state_set(Ecore_X_Window win, Ecore_X_Virtual_Keyboard_State state);
EAPI Ecore_X_Virtual_Keyboard_State  ecore_x_e_virtual_keyboard_state_get(Ecore_X_Window win);
EAPI void                            ecore_x_e_virtual_keyboard_state_send(Ecore_X_Window win, Ecore_X_Virtual_Keyboard_State state);

/* Illume functions */
EAPI void                            ecore_x_e_illume_zone_set(Ecore_X_Window win, Ecore_X_Window zone);
EAPI Ecore_X_Window                  ecore_x_e_illume_zone_get(Ecore_X_Window win);
EAPI void                            ecore_x_e_illume_zone_list_set(Ecore_X_Window win, Ecore_X_Window *zones, unsigned int n_zones);
EAPI void                            ecore_x_e_illume_conformant_set(Ecore_X_Window win, unsigned int is_conformant);
EAPI Eina_Bool                       ecore_x_e_illume_conformant_get(Ecore_X_Window win);
EAPI void                            ecore_x_e_illume_mode_set(Ecore_X_Window win, Ecore_X_Illume_Mode mode);
EAPI Ecore_X_Illume_Mode             ecore_x_e_illume_mode_get(Ecore_X_Window win);
EAPI void                            ecore_x_e_illume_mode_send(Ecore_X_Window win, Ecore_X_Illume_Mode mode);
EAPI void                            ecore_x_e_illume_focus_back_send(Ecore_X_Window win);
EAPI void                            ecore_x_e_illume_focus_forward_send(Ecore_X_Window win);
EAPI void                            ecore_x_e_illume_focus_home_send(Ecore_X_Window win);
EAPI void                            ecore_x_e_illume_close_send(Ecore_X_Window win);
EAPI void                            ecore_x_e_illume_home_new_send(Ecore_X_Window win);
EAPI void                            ecore_x_e_illume_home_del_send(Ecore_X_Window win);
EAPI void                            ecore_x_e_illume_access_action_next_send(Ecore_X_Window win);
EAPI void                            ecore_x_e_illume_access_action_prev_send(Ecore_X_Window win);
EAPI void                            ecore_x_e_illume_access_action_activate_send(Ecore_X_Window win);
EAPI void                            ecore_x_e_illume_access_action_over_send(Ecore_X_Window win);
EAPI void                            ecore_x_e_illume_access_action_read_send(Ecore_X_Window win);
EAPI void                            ecore_x_e_illume_access_action_read_next_send(Ecore_X_Window win);
EAPI void                            ecore_x_e_illume_access_action_read_prev_send(Ecore_X_Window win);
EAPI void                            ecore_x_e_illume_access_action_up_send(Ecore_X_Window win);
EAPI void                            ecore_x_e_illume_access_action_down_send(Ecore_X_Window win);

EAPI void                            ecore_x_e_illume_drag_set(Ecore_X_Window win, unsigned int drag);
EAPI Eina_Bool                       ecore_x_e_illume_drag_get(Ecore_X_Window win);
EAPI void                            ecore_x_e_illume_drag_locked_set(Ecore_X_Window win, unsigned int is_locked);
EAPI Eina_Bool                       ecore_x_e_illume_drag_locked_get(Ecore_X_Window win);
EAPI void                            ecore_x_e_illume_drag_start_send(Ecore_X_Window win);
EAPI void                            ecore_x_e_illume_drag_end_send(Ecore_X_Window win);
EAPI void                            ecore_x_e_illume_indicator_geometry_set(Ecore_X_Window win, int x, int y, int w, int h);
EAPI Eina_Bool                       ecore_x_e_illume_indicator_geometry_get(Ecore_X_Window win, int *x, int *y, int *w, int *h);
EAPI void                            ecore_x_e_illume_softkey_geometry_set(Ecore_X_Window win, int x, int y, int w, int h);
EAPI Eina_Bool                       ecore_x_e_illume_softkey_geometry_get(Ecore_X_Window win, int *x, int *y, int *w, int *h);
EAPI void                            ecore_x_e_illume_keyboard_geometry_set(Ecore_X_Window win, int x, int y, int w, int h);
EAPI Eina_Bool                       ecore_x_e_illume_keyboard_geometry_get(Ecore_X_Window win, int *x, int *y, int *w, int *h);
EAPI void                            ecore_x_e_illume_quickpanel_set(Ecore_X_Window win, unsigned int is_quickpanel);
EAPI Eina_Bool                       ecore_x_e_illume_quickpanel_get(Ecore_X_Window win);
EAPI void                            ecore_x_e_illume_quickpanel_state_set(Ecore_X_Window win, Ecore_X_Illume_Quickpanel_State state);
EAPI Ecore_X_Illume_Quickpanel_State ecore_x_e_illume_quickpanel_state_get(Ecore_X_Window win);
EAPI void                            ecore_x_e_illume_quickpanel_state_send(Ecore_X_Window win, Ecore_X_Illume_Quickpanel_State state);
EAPI void                            ecore_x_e_illume_quickpanel_state_toggle(Ecore_X_Window win);
EAPI void                            ecore_x_e_illume_quickpanel_priority_major_set(Ecore_X_Window win, unsigned int priority);
EAPI int                             ecore_x_e_illume_quickpanel_priority_major_get(Ecore_X_Window win);
EAPI void                            ecore_x_e_illume_quickpanel_priority_minor_set(Ecore_X_Window win, unsigned int priority);
EAPI int                             ecore_x_e_illume_quickpanel_priority_minor_get(Ecore_X_Window win);
EAPI void                            ecore_x_e_illume_quickpanel_zone_set(Ecore_X_Window win, unsigned int zone);
EAPI int                             ecore_x_e_illume_quickpanel_zone_get(Ecore_X_Window win);
EAPI void                            ecore_x_e_illume_quickpanel_zone_request_send(Ecore_X_Window win);
EAPI void                            ecore_x_e_illume_quickpanel_position_update_send(Ecore_X_Window win);

EAPI void                            ecore_x_e_illume_clipboard_state_set(Ecore_X_Window win, Ecore_X_Illume_Clipboard_State state);

EAPI Ecore_X_Illume_Clipboard_State  ecore_x_e_illume_clipboard_state_get(Ecore_X_Window win);

EAPI void                            ecore_x_e_illume_clipboard_geometry_set(Ecore_X_Window win, int x, int y, int w, int h);
EAPI Eina_Bool                       ecore_x_e_illume_clipboard_geometry_get(Ecore_X_Window win, int *x, int *y, int *w, int *h);
EAPI void                            ecore_x_e_comp_sync_counter_set(Ecore_X_Window win, Ecore_X_Sync_Counter counter);
EAPI Ecore_X_Sync_Counter            ecore_x_e_comp_sync_counter_get(Ecore_X_Window win);
EAPI void                            ecore_x_e_comp_sync_draw_done_send(Ecore_X_Window root, Ecore_X_Window win);
EAPI void                            ecore_x_e_comp_sync_draw_size_done_send(Ecore_X_Window root, Ecore_X_Window win, int w, int h);
EAPI void                            ecore_x_e_comp_sync_supported_set(Ecore_X_Window root, Eina_Bool enabled);
EAPI Eina_Bool                       ecore_x_e_comp_sync_supported_get(Ecore_X_Window root);
EAPI void                            ecore_x_e_comp_sync_begin_send(Ecore_X_Window win);
EAPI void                            ecore_x_e_comp_sync_end_send(Ecore_X_Window win);
EAPI void                            ecore_x_e_comp_sync_cancel_send(Ecore_X_Window win);

EAPI void                            ecore_x_e_comp_flush_send(Ecore_X_Window win);
EAPI void                            ecore_x_e_comp_dump_send(Ecore_X_Window win);
EAPI void                            ecore_x_e_comp_pixmap_set(Ecore_X_Window win, Ecore_X_Pixmap pixmap);
EAPI Ecore_X_Pixmap                  ecore_x_e_comp_pixmap_get(Ecore_X_Window win);

/**
 * @brief Get the window profile
 *
 * @param win The client x window
 * @return The string value of the window profile, or NULL if none exists
 */
EAPI char                            *ecore_x_e_window_profile_get(Ecore_X_Window win);
/**
 * @brief Set the window profile
 *
 * @param win The client x window
 * @param profile The string value of the window profile
 */
EAPI void                             ecore_x_e_window_profile_set(Ecore_X_Window win, const char *profile);
/**
 * @brief Set the array of window profiles
 *
 * @param win The client x window
 * @param profiles The string array of window profiles
 * @param num_profiles The number of window profiles
 *
 * @deprecated use ecore_x_e_window_available_profiles_set
 */
EAPI void                             ecore_x_e_window_profile_list_set(Ecore_X_Window  win, const char **profiles, unsigned int num_profiles);
/**
 * @brief Get the array of window profiles
 *
 * @param win The client x window
 * @param profiles Where to return the string array of window profiles
 * @param ret_num Where to return the number of window profiles
 * @return EINA_TRUE if window profiles exist, EINA_FALSE otherwise
 *
 * @deprecated use ecore_x_e_window_available_profiles_get
 */
EAPI Eina_Bool                        ecore_x_e_window_profile_list_get(Ecore_X_Window win, const char ***profiles, int *ret_num);
/**
 * @brief Set the status for the window profile support
 *
 * @param root The root window
 * @param enabled The enabled value for the window profile support
 *
 * @since 1.8
 */
EAPI void                             ecore_x_e_window_profile_supported_set(Ecore_X_Window root, Eina_Bool enabled);
/**
 * @brief Query if the window profile is supported
 *
 * @param root The root window
 * @return EINA_TRUE if it is supported, EINA_FALSE otherwise
 *
 * @since 1.8
 */
EAPI Eina_Bool                        ecore_x_e_window_profile_supported_get(Ecore_X_Window root);
/**
 * @brief Set the array of available window profiles
 *
 * @param win The client x window
 * @param profiles The string array of available window profiles
 * @param count The number of available window profiles
 *
 * @since 1.8
 */
EAPI void                             ecore_x_e_window_available_profiles_set(Ecore_X_Window win, const char **profiles, unsigned int count);
/**
 * @brief Get the array of avaialbe window profiles
 *
 * @param win The client x window
 * @param profiles Where to return the string array of available window profiles
 * @param count Where to return the number of members in profiles
 * @return EINA_TRUE if available window profiles exist, EINA_FALSE otherwise
 *
 * @since 1.8
 */
EAPI Eina_Bool                        ecore_x_e_window_available_profiles_get(Ecore_X_Window win, const char ***profiles, int *count);
/**
 * @brief Send a profile change event to the window manager
 *
 * This function sends a request to the window manager to change the profile.
 * If honored by the window manager, the client will receive a profile change
 * request event back. If the client has replied, the window manager will move
 * the client window on the virtual desktop associated with changed profile.
 *
 * @param root The root x window
 * @param win The client x window
 * @param profile The string value of the window profile
 *
 * @since 1.8
 */
EAPI void                             ecore_x_e_window_profile_change_send(Ecore_X_Window root, Ecore_X_Window win, const char *profile);
/**
 * @brief Send a profile change request event to the client
 *
 * This function sends a request to the client to change the profile.
 * If the client has replied, the window manager will move the client window
 * on the virtual desktop associated with changed profile.
 *
 * @param win The client x window
 * @param profile The string value of the window profile
 *
 * @since 1.8
 */
EAPI void                             ecore_x_e_window_profile_change_request_send(Ecore_X_Window win, const char *profile);
/**
 * @brief Send a profile change done event to the window manager
 *
 * This function sends a profile change done event to the window manager.
 * Upon receiving, the window manager will move the client window
 * on the virtual desktop associated with changed profile.
 *
 * @param root The root x window
 * @param win The client x window
 * @param profile The string value of the window profile
 *
 * @since 1.8
 */
EAPI void                             ecore_x_e_window_profile_change_done_send(Ecore_X_Window root, Ecore_X_Window win, const char *profile);

EAPI Ecore_X_Sync_Alarm              ecore_x_sync_alarm_new(Ecore_X_Sync_Counter counter);
EAPI Eina_Bool                       ecore_x_sync_alarm_free(Ecore_X_Sync_Alarm alarm);
EAPI Eina_Bool                       ecore_x_sync_counter_query(Ecore_X_Sync_Counter counter, unsigned int *val);
EAPI Ecore_X_Sync_Counter            ecore_x_sync_counter_new(int val);
EAPI void                            ecore_x_sync_counter_free(Ecore_X_Sync_Counter counter);
EAPI void                            ecore_x_sync_counter_inc(Ecore_X_Sync_Counter counter, int by);
EAPI void                            ecore_x_sync_counter_val_wait(Ecore_X_Sync_Counter counter, int val);

EAPI void                            ecore_x_sync_counter_set(Ecore_X_Sync_Counter counter, int val);
EAPI void                            ecore_x_sync_counter_2_set(Ecore_X_Sync_Counter counter, int val_hi, unsigned int val_lo);
EAPI Eina_Bool                       ecore_x_sync_counter_2_query(Ecore_X_Sync_Counter counter, int *val_hi, unsigned int *val_lo);

EAPI int                             ecore_x_xinerama_screen_count_get(void);
EAPI Eina_Bool                       ecore_x_xinerama_screen_geometry_get(int screen, int *x, int *y, int *w, int *h);

EAPI Eina_Bool                       ecore_x_screensaver_event_available_get(void);
EAPI int                             ecore_x_screensaver_idle_time_get(void);
EAPI void                            ecore_x_screensaver_set(int timeout, int interval, int prefer_blanking, int allow_exposures);
EAPI void                            ecore_x_screensaver_timeout_set(int timeout);
EAPI int                             ecore_x_screensaver_timeout_get(void);
EAPI void                            ecore_x_screensaver_blank_set(int timeout);
EAPI int                             ecore_x_screensaver_blank_get(void);
EAPI void                            ecore_x_screensaver_expose_set(int timeout);
EAPI int                             ecore_x_screensaver_expose_get(void);
EAPI void                            ecore_x_screensaver_interval_set(int timeout);
EAPI int                             ecore_x_screensaver_interval_get(void);
EAPI void                            ecore_x_screensaver_event_listen_set(Eina_Bool on);
EAPI Eina_Bool                       ecore_x_screensaver_custom_blanking_enable(void); /** @since 1.7 */
EAPI Eina_Bool                       ecore_x_screensaver_custom_blanking_disable(void); /** @since 1.7 */
       
/* FIXME: these funcs need categorising */

typedef struct _Ecore_X_Window_Attributes
{
   Ecore_X_Window   root;
   int              x, y, w, h;
   int              border;
   int              depth;
   Eina_Bool        visible : 1;
   Eina_Bool        viewable : 1;
   Eina_Bool        override : 1;
   Eina_Bool        input_only : 1;
   Eina_Bool        save_under : 1;
   struct
   {
      Ecore_X_Event_Mask mine;
      Ecore_X_Event_Mask all;
      Ecore_X_Event_Mask no_propagate;
   } event_mask;
   Ecore_X_Gravity  window_gravity;
   Ecore_X_Gravity  pixel_gravity;
   Ecore_X_Colormap colormap;
   Ecore_X_Visual   visual;
   /* FIXME: missing
    * int map_installed;
    * Screen *screen;
    */
} Ecore_X_Window_Attributes;

EAPI Eina_Bool        ecore_x_window_attributes_get(Ecore_X_Window win, Ecore_X_Window_Attributes *att_ret);
EAPI void             ecore_x_window_save_set_add(Ecore_X_Window win);
EAPI void             ecore_x_window_save_set_del(Ecore_X_Window win);
EAPI Ecore_X_Window  *ecore_x_window_children_get(Ecore_X_Window win, int *num);

EAPI Eina_Bool        ecore_x_pointer_control_set(int accel_num, int accel_denom, int threshold);
EAPI Eina_Bool        ecore_x_pointer_control_get(int *accel_num, int *accel_denom, int *threshold);
EAPI Eina_Bool        ecore_x_pointer_mapping_set(unsigned char *map, int nmap);
EAPI Eina_Bool        ecore_x_pointer_mapping_get(unsigned char *map, int nmap);
EAPI Eina_Bool        ecore_x_pointer_grab(Ecore_X_Window win);
EAPI Eina_Bool        ecore_x_pointer_confine_grab(Ecore_X_Window win);
EAPI void             ecore_x_pointer_ungrab(void);
EAPI Eina_Bool        ecore_x_pointer_warp(Ecore_X_Window win, int x, int y);
EAPI Eina_Bool        ecore_x_keyboard_grab(Ecore_X_Window win);
EAPI void             ecore_x_keyboard_ungrab(void);
EAPI void             ecore_x_grab(void);
EAPI void             ecore_x_ungrab(void);
EAPI void             ecore_x_passive_grab_replay_func_set(Eina_Bool (*func)(void *data, int event_type, void *event), void *data);
EAPI void             ecore_x_window_button_grab(Ecore_X_Window win, int button, Ecore_X_Event_Mask event_mask, int mod, int any_mod);
EAPI void             ecore_x_window_button_ungrab(Ecore_X_Window win, int button, int mod, int any_mod);
EAPI void             ecore_x_window_key_grab(Ecore_X_Window win, const char *key, int mod, int any_mod);
EAPI void             ecore_x_window_key_ungrab(Ecore_X_Window win, const char *key, int mod, int any_mod);

EAPI void             ecore_x_focus_reset(void);
EAPI void             ecore_x_events_allow_all(void);
EAPI void             ecore_x_pointer_last_xy_get(int *x, int *y);
EAPI void             ecore_x_pointer_xy_get(Ecore_X_Window win, int *x, int *y);
EAPI void             ecore_x_pointer_root_xy_get(int *x, int *y);

/* ecore_x_region.c */
EAPI Ecore_X_XRegion *ecore_x_xregion_new(void);
EAPI void             ecore_x_xregion_free(Ecore_X_XRegion *region);
EAPI Eina_Bool        ecore_x_xregion_set(Ecore_X_XRegion *region, Ecore_X_GC gc);
EAPI void             ecore_x_xregion_translate(Ecore_X_XRegion *region, int x, int y);
EAPI Eina_Bool        ecore_x_xregion_intersect(Ecore_X_XRegion *dst, Ecore_X_XRegion *r1, Ecore_X_XRegion *r2);
EAPI Eina_Bool        ecore_x_xregion_union(Ecore_X_XRegion *dst, Ecore_X_XRegion *r1, Ecore_X_XRegion *r2);
EAPI Eina_Bool        ecore_x_xregion_union_rect(Ecore_X_XRegion *dst, Ecore_X_XRegion *src, Ecore_X_Rectangle *rect);
EAPI Eina_Bool        ecore_x_xregion_subtract(Ecore_X_XRegion *dst, Ecore_X_XRegion *r1, Ecore_X_XRegion *r2);
EAPI Eina_Bool        ecore_x_xregion_is_empty(Ecore_X_XRegion *region);
EAPI Eina_Bool        ecore_x_xregion_is_equal(Ecore_X_XRegion *r1, Ecore_X_XRegion *r2);
EAPI Eina_Bool        ecore_x_xregion_point_contain(Ecore_X_XRegion *region, int x, int y);
EAPI Eina_Bool        ecore_x_xregion_rect_contain(Ecore_X_XRegion *region, Ecore_X_Rectangle *rect);

/* ecore_x_randr.c */

/* The usage of 'Ecore_X_Randr_None' or 'Ecore_X_Randr_Unset'
 * depends on the context. In most cases 'Ecore_X_Randr_Unset'
 * can be used, but in some cases -1 is a special value to
 * functions, thus 'Ecore_X_Randr_None' (=0) must be used.
 */

typedef short Ecore_X_Randr_Refresh_Rate;
typedef int   Ecore_X_Randr_Crtc_Gamma;
typedef int   Ecore_X_Randr_Signal_Format;
typedef int   Ecore_X_Randr_Signal_Property;
typedef int   Ecore_X_Randr_Connector_Type;

typedef struct _Ecore_X_Randr_Mode_Info
{
   Ecore_X_ID    xid;
   unsigned int  width;
   unsigned int  height;
   unsigned long dotClock;
   unsigned int  hSyncStart;
   unsigned int  hSyncEnd;
   unsigned int  hTotal;
   unsigned int  hSkew;
   unsigned int  vSyncStart;
   unsigned int  vSyncEnd;
   unsigned int  vTotal;
   char         *name;
   unsigned int  nameLength;
   unsigned long modeFlags;
} Ecore_X_Randr_Mode_Info;

typedef struct _Ecore_X_Randr_Crtc_Gamma_Info
{
   int size;
   unsigned short *red;
   unsigned short *green;
   unsigned short *blue;
} Ecore_X_Randr_Crtc_Gamma_Info;

EAPI int                                       ecore_x_randr_version_get(void);
EAPI Eina_Bool                                 ecore_x_randr_query(void);
EAPI Ecore_X_Time                              ecore_x_randr_config_timestamp_get(Ecore_X_Window root); /** @since 1.8 */
EAPI Ecore_X_Randr_Orientation                 ecore_x_randr_screen_primary_output_orientations_get(Ecore_X_Window root);
EAPI Ecore_X_Randr_Orientation                 ecore_x_randr_screen_primary_output_orientation_get(Ecore_X_Window root);
EAPI Eina_Bool                                 ecore_x_randr_screen_primary_output_orientation_set(Ecore_X_Window root, Ecore_X_Randr_Orientation orientation);
EAPI Ecore_X_Randr_Screen_Size_MM             *ecore_x_randr_screen_primary_output_sizes_get(Ecore_X_Window root, int *num);

/**
 * @brief get the current set size of a given screen's primary output
 * @param root window which's primary output will be queried
 * @param w the current size's width
 * @param h the current size's height
 * @param w_mm the current size's width in mm
 * @param h_mm the current size's height in mm
 * @param size_index of current set size to be used with ecore_x_randr_primary_output_size_set()
 */
EAPI void                                      ecore_x_randr_screen_primary_output_current_size_get(Ecore_X_Window root, int *w, int *h, int *w_mm, int *h_mm, int *size_index);
EAPI Eina_Bool                                 ecore_x_randr_screen_primary_output_size_set(Ecore_X_Window root, int size_index);
EAPI Ecore_X_Randr_Refresh_Rate                ecore_x_randr_screen_primary_output_current_refresh_rate_get(Ecore_X_Window root);
EAPI Ecore_X_Randr_Refresh_Rate               *ecore_x_randr_screen_primary_output_refresh_rates_get(Ecore_X_Window root, int size_index, int *num);
EAPI Eina_Bool                                 ecore_x_randr_screen_primary_output_refresh_rate_set(Ecore_X_Window root, int size_index, Ecore_X_Randr_Refresh_Rate rate);

EAPI void                                      ecore_x_randr_events_select(Ecore_X_Window win, Eina_Bool on);

EAPI void                                      ecore_x_randr_screen_current_size_get(Ecore_X_Window root, int *w, int *h, int *w_mm, int *h_mm);
EAPI void                                      ecore_x_randr_screen_size_range_get(Ecore_X_Window root, int *wmin, int *hmin, int *wmax, int *hmax);
EAPI void                                      ecore_x_randr_screen_reset(Ecore_X_Window root);
EAPI Eina_Bool                                 ecore_x_randr_screen_current_size_set(Ecore_X_Window root, int w, int h, int w_mm, int h_mm);
EAPI Ecore_X_Randr_Mode_Info                 **ecore_x_randr_modes_info_get(Ecore_X_Window root, int *num);
EAPI Ecore_X_Randr_Mode                        ecore_x_randr_mode_info_add(Ecore_X_Window root, Ecore_X_Randr_Mode_Info *mode_info);
EAPI void                                      ecore_x_randr_mode_del(Ecore_X_Randr_Mode mode);
EAPI Ecore_X_Randr_Mode_Info                  *ecore_x_randr_mode_info_get(Ecore_X_Window root, Ecore_X_Randr_Mode mode);
EAPI void                                      ecore_x_randr_mode_info_free(Ecore_X_Randr_Mode_Info *mode_info);
EAPI Ecore_X_Randr_Crtc                       *ecore_x_randr_crtcs_get(Ecore_X_Window root, int *num);
EAPI Ecore_X_Randr_Output                     *ecore_x_randr_outputs_get(Ecore_X_Window root, int *num);
EAPI Ecore_X_Randr_Output                     *ecore_x_randr_window_outputs_get(Ecore_X_Window window, int *num);
EAPI Ecore_X_Randr_Output                     *ecore_x_randr_current_output_get(Ecore_X_Window window, int *num);
EAPI Ecore_X_Randr_Crtc                       *ecore_x_randr_window_crtcs_get(Ecore_X_Window window, int *num);
EAPI Ecore_X_Randr_Crtc                       *ecore_x_randr_current_crtc_get(Ecore_X_Window window, int *num);
EAPI Ecore_X_Randr_Output                     *ecore_x_randr_crtc_outputs_get(Ecore_X_Window root, Ecore_X_Randr_Crtc crtc, int *num);
EAPI Ecore_X_Randr_Output                     *ecore_x_randr_crtc_possible_outputs_get(Ecore_X_Window root, Ecore_X_Randr_Crtc crtc, int *num);
EAPI void                                      ecore_x_randr_crtc_geometry_get(Ecore_X_Window root, Ecore_X_Randr_Crtc crtc, int *x, int *y, int *w, int *h);
EAPI void                                      ecore_x_randr_crtc_pos_get(Ecore_X_Window root, Ecore_X_Randr_Crtc crtc, int *x, int *y);
EAPI Eina_Bool                                 ecore_x_randr_crtc_pos_set(Ecore_X_Window root, Ecore_X_Randr_Crtc crtc, int x, int y);
EAPI Ecore_X_Randr_Mode                        ecore_x_randr_crtc_mode_get(Ecore_X_Window root, Ecore_X_Randr_Crtc crtc);
EAPI Eina_Bool                                 ecore_x_randr_crtc_mode_set(Ecore_X_Window root, Ecore_X_Randr_Crtc crtc, Ecore_X_Randr_Output *outputs, int noutputs, Ecore_X_Randr_Mode mode);
EAPI void                                      ecore_x_randr_crtc_size_get(Ecore_X_Window root, Ecore_X_Randr_Crtc crtc, int *w, int *h);
EAPI Ecore_X_Randr_Refresh_Rate                ecore_x_randr_crtc_refresh_rate_get(Ecore_X_Window root, Ecore_X_Randr_Crtc crtc, Ecore_X_Randr_Mode mode);
EAPI Ecore_X_Randr_Orientation                 ecore_x_randr_crtc_orientations_get(Ecore_X_Window root, Ecore_X_Randr_Crtc crtc);
EAPI Ecore_X_Randr_Orientation                 ecore_x_randr_crtc_orientation_get(Ecore_X_Window root, Ecore_X_Randr_Crtc crtc);
EAPI Eina_Bool                                 ecore_x_randr_crtc_orientation_set(Ecore_X_Window root, Ecore_X_Randr_Crtc crtc, const Ecore_X_Randr_Orientation orientation);
EAPI Eina_Bool                                 ecore_x_randr_crtc_clone_set(Ecore_X_Window root, Ecore_X_Randr_Crtc original, Ecore_X_Randr_Crtc clone);
EAPI Eina_Bool                                 ecore_x_randr_crtc_settings_set(Ecore_X_Window root, Ecore_X_Randr_Crtc crtc, Ecore_X_Randr_Output *outputs, int noutputs, int x, int y, Ecore_X_Randr_Mode mode, Ecore_X_Randr_Orientation orientation);
EAPI Eina_Bool                                 ecore_x_randr_crtc_pos_relative_set(Ecore_X_Window root, Ecore_X_Randr_Crtc crtc_r1, Ecore_X_Randr_Crtc crtc_r2, Ecore_X_Randr_Output_Policy policy, Ecore_X_Randr_Relative_Alignment alignment);
EAPI Ecore_X_Randr_Crtc_Info                  *ecore_x_randr_crtc_info_get(Ecore_X_Window root, const Ecore_X_Randr_Crtc crtc); /**< @since 1.8 */
EAPI void                                      ecore_x_randr_crtc_info_free(Ecore_X_Randr_Crtc_Info *info); /**< @since 1.8 */
EAPI Eina_Bool                                 ecore_x_randr_output_mode_add(Ecore_X_Randr_Output output, Ecore_X_Randr_Mode mode);
EAPI void                                      ecore_x_randr_output_mode_del(Ecore_X_Randr_Output output, Ecore_X_Randr_Mode mode);
EAPI Ecore_X_Randr_Mode                       *ecore_x_randr_output_modes_get(Ecore_X_Window root, Ecore_X_Randr_Output output, int *num, int *npreferred);
EAPI Ecore_X_Randr_Output                     *ecore_x_randr_output_clones_get(Ecore_X_Window root, Ecore_X_Randr_Output output, int *num);
EAPI Ecore_X_Randr_Crtc                       *ecore_x_randr_output_possible_crtcs_get(Ecore_X_Window root, Ecore_X_Randr_Output output, int *num);
EAPI Ecore_X_Randr_Crtc                        ecore_x_randr_output_crtc_get(Ecore_X_Window root, Ecore_X_Randr_Output output);
EAPI char                                     *ecore_x_randr_output_name_get(Ecore_X_Window root, Ecore_X_Randr_Output output, int *len);
EINA_DEPRECATED EAPI int                       ecore_x_randr_crtc_gamma_ramp_size_get(Ecore_X_Randr_Crtc crtc);
EINA_DEPRECATED EAPI Ecore_X_Randr_Crtc_Gamma **ecore_x_randr_crtc_gamma_ramps_get(Ecore_X_Randr_Crtc crtc);
EINA_DEPRECATED EAPI Eina_Bool                 ecore_x_randr_crtc_gamma_ramps_set(Ecore_X_Randr_Crtc crtc, const Ecore_X_Randr_Crtc_Gamma *red, const Ecore_X_Randr_Crtc_Gamma *green, const Ecore_X_Randr_Crtc_Gamma *blue);
EAPI Eina_Bool                                 ecore_x_randr_move_all_crtcs_but(Ecore_X_Window root, const Ecore_X_Randr_Crtc *not_moved, int nnot_moved, int dx, int dy);
EAPI Eina_Bool                                 ecore_x_randr_move_crtcs(Ecore_X_Window root, const Ecore_X_Randr_Crtc *crtcs, int ncrtc, int dx, int dy);
EAPI void                                      ecore_x_randr_mode_size_get(Ecore_X_Window root, Ecore_X_Randr_Mode mode, int *w, int *h);
EAPI Ecore_X_Randr_Connection_Status           ecore_x_randr_output_connection_status_get(Ecore_X_Window root, Ecore_X_Randr_Output output);
EAPI void                                      ecore_x_randr_output_size_mm_get(Ecore_X_Window root, Ecore_X_Randr_Output output, int *w, int *h);
EAPI Eina_Bool                                 ecore_x_randr_output_crtc_set(Ecore_X_Window root, Ecore_X_Randr_Output output, const Ecore_X_Randr_Crtc crtc);

EAPI int                                       ecore_x_randr_crtc_gamma_size_get(Ecore_X_Randr_Crtc crtc); /**< @since 1.8 */
EAPI Ecore_X_Randr_Crtc_Gamma_Info            *ecore_x_randr_crtc_gamma_get(Ecore_X_Randr_Crtc crtc); /**< @since 1.8 */
EAPI Eina_Bool                                 ecore_x_randr_crtc_gamma_set(Ecore_X_Randr_Crtc crtc, const Ecore_X_Randr_Crtc_Gamma_Info *gamma); /**< @since 1.8 */

/**
 * @brief Validates the header from raw EDID data.
 *
 * @param edid The edid structure.
 * @param edid_length Length of the edid structure.
 * @return @c EINA_TRUE, if the header is valid, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool                                 ecore_x_randr_edid_has_valid_header(unsigned char *edid, unsigned long edid_length);

/**
 * @brief Checks whether a display's EDID has a valid checksum.
 *
 * @param edid The edid structure.
 * @param edid_length Length of the edid structure.
 * @return @c EINA_TRUE, if the checksum is valid, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool                                 ecore_x_randr_edid_info_has_valid_checksum(unsigned char *edid, unsigned long edid_length);

/**
 * @brief Get the encoded version from raw EDID data.
 *
 * The return value has the minor version in the lowest 8 bits, and the major
 * version in all the rest of the bits. i.e.
 *
 * minor = (version & 0x000000ff);
 * major = (version & 0xffffff00) >> 8;
 *
 * @param edid the edid structure
 * @param edid_length length of the edid structure
 * @return The encoded major and minor version encasuplated an int.
 */
EAPI int                                       ecore_x_randr_edid_version_get(unsigned char *edid, unsigned long edid_length);

/**
 * @brief Get the encoded manufacturer from raw EDID data.
 *
 * @param edid the edid structure
 * @param edid_length length of the edid structure
 * @return The encoded manufacturer identifier.
 */
EAPI char                                     *ecore_x_randr_edid_manufacturer_name_get(unsigned char *edid, unsigned long edid_length);

/**
 * @brief Get the encoded name from raw EDID data.
 *
 * @param edid the edid structure
 * @param edid_length length of the edid structure
 * @return The encoded manufacturer identifier.
 */
EAPI char                                     *ecore_x_randr_edid_display_name_get(unsigned char *edid, unsigned long edid_length);

/**
 * @brief Get the encoded ASCII from raw EDID data.
 *
 * @param edid the edid structure
 * @param edid_length length of the edid structure
 * @return The encoded ASCII display identifier.
 */
EAPI char                                     *ecore_x_randr_edid_display_ascii_get(unsigned char *edid, unsigned long edid_length);

/**
 * @brief Get the encoded serial identifier from raw EDID data.
 *
 * @param edid the edid structure
 * @param edid_length length of the edid structure
 * @return The encoded serial identifier.
 */
EAPI char                                     *ecore_x_randr_edid_display_serial_get(unsigned char *edid, unsigned long edid_length);

/**
 * @brief Get the encoded model number from raw EDID data.
 *
 * The manufacturer ID table is necessary for a useful description.
 *
 * @param edid the edid structure
 * @param edid_length length of the edid structure
 * @return The encoded model number.
 */
EAPI int                                       ecore_x_randr_edid_model_get(unsigned char *edid, unsigned long edid_length);

/**
 * @brief Get the manufacturer serial number from raw EDID data.
 *
 * @param edid the edid structure
 * @param edid_length length of the edid structure
 * @return The encoded serial manufacturer serial number.
 */
EAPI int                                       ecore_x_randr_edid_manufacturer_serial_number_get(unsigned char *edid, unsigned long edid_length);

/**
 * @brief Get the manufacturer model number from raw EDID data.
 *
 * @param edid the edid structure
 * @param edid_length length of the edid structure
 * @return The manufacturer's model number.
 */
EAPI int                                       ecore_x_randr_edid_manufacturer_model_get(unsigned char *edid, unsigned long edid_length);

/**
 * @brief Looks up the DPMS support from raw EDID data.
 *
 * @param edid The edid structure.
 * @param edid_length Length of the edid structure.
 * @return @c EINA_TRUE, if DPMS is supported in some way, @c EINA_FALSE
 * otherwise.
 */
EAPI Eina_Bool                                 ecore_x_randr_edid_dpms_available_get(unsigned char *edid, unsigned long edid_length);

/**
 * @brief Looks up the DPMS Standby support from raw EDID data.
 *
 * @param edid The edid structure.
 * @param edid_length Length of the edid structure.
 * @return @c EINA_TRUE, if DPMS Standby is supported, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool                                 ecore_x_randr_edid_dpms_standby_available_get(unsigned char *edid, unsigned long edid_length);

/**
 * @brief Looks up the DPMS Suspend support from raw EDID data.
 *
 * @param edid The edid structure.
 * @param edid_length Length of the edid structure.
 * @return @c EINA_TRUE, if DPMS Suspend is supported, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool                                 ecore_x_randr_edid_dpms_suspend_available_get(unsigned char *edid, unsigned long edid_length);

/**
 * @brief Looks up the DPMS Off support from raw EDID data.
 *
 * @param edid The edid structure.
 * @param edid_length Length of the edid structure.
 * @return @c EINA_TRUE, if DPMS Off is supported, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool                                 ecore_x_randr_edid_dpms_off_available_get(unsigned char *edid, unsigned long edid_length);

/**
 * @brief Get the preferred aspect ratio from raw EDID data.
 *
 * @param edid the edid structure
 * @param edid_length length of the edid structure
 * @return The preferred aspect ratio.
 */
EAPI Ecore_X_Randr_Edid_Aspect_Ratio           ecore_x_randr_edid_display_aspect_ratio_preferred_get(unsigned char *edid, unsigned long edid_length);

/**
 * @brief Get the supported aspect ratios from raw EDID data.
 *
 * @param edid the edid structure
 * @param edid_length length of the edid structure
 * @return The supported aspect ratios.
 */
EAPI Ecore_X_Randr_Edid_Aspect_Ratio           ecore_x_randr_edid_display_aspect_ratios_get(unsigned char *edid, unsigned long edid_length);

/**
 * @brief Get the supported colorschemes from raw EDID data.
 *
 * @param edid the edid structure
 * @param edid_length length of the edid structure
 * @return The supported colorschemes.
 */
EAPI Ecore_X_Randr_Edid_Display_Colorscheme    ecore_x_randr_edid_display_colorscheme_get(unsigned char *edid, unsigned long edid_length);

/**
 * @brief Get the display type from raw EDID data.
 *
 * @param edid The edid structure.
 * @param edid_length Length of the edid structure.
 * @return @c EINA_TRUE, if the display is a digital one, @c EINA_FALSE
 * otherwise.
 */
EAPI Eina_Bool                                 ecore_x_randr_edid_display_type_digital_get(unsigned char *edid, unsigned long edid_length);

/**
 * @brief Get the display interface type from raw EDID data.
 *
 * @param edid the edid structure
 * @param edid_length length of the edid structure
 * @return The interface type.
 */
EAPI Ecore_X_Randr_Edid_Display_Interface_Type ecore_x_randr_edid_display_interface_type_get(unsigned char *edid, unsigned long edid_length);

EAPI Eina_Bool                                 ecore_x_randr_output_backlight_available(void);
EAPI void                                      ecore_x_randr_screen_backlight_level_set(Ecore_X_Window root, double level);
EAPI double                                    ecore_x_randr_output_backlight_level_get(Ecore_X_Window root, Ecore_X_Randr_Output output);
EAPI Eina_Bool                                 ecore_x_randr_output_backlight_level_set(Ecore_X_Window root, Ecore_X_Randr_Output output, double level);
EAPI Ecore_X_Randr_Output                      ecore_x_randr_primary_output_get(Ecore_X_Window root);
EAPI void                                      ecore_x_randr_primary_output_set(Ecore_X_Window root, Ecore_X_Randr_Output output);
EAPI Ecore_X_Render_Subpixel_Order             ecore_x_randr_output_subpixel_order_get(Ecore_X_Window root, Ecore_X_Randr_Output output);
EAPI unsigned char                            *ecore_x_randr_output_edid_get(Ecore_X_Window root, Ecore_X_Randr_Output output, unsigned long *length);
EAPI Ecore_X_Randr_Output                     *ecore_x_randr_output_wired_clones_get(Ecore_X_Window root, Ecore_X_Randr_Output output, int *num);
EAPI Ecore_X_Randr_Output                    **ecore_x_randr_output_compatibility_list_get(Ecore_X_Window root, Ecore_X_Randr_Output output, int *num);
EAPI Ecore_X_Randr_Signal_Format              *ecore_x_randr_output_signal_formats_get(Ecore_X_Window root, Ecore_X_Randr_Output output, int *num);
EAPI Eina_Bool                                 ecore_x_randr_output_signal_format_set(Ecore_X_Window root, Ecore_X_Randr_Output output, Ecore_X_Randr_Signal_Format *signal);
EAPI Ecore_X_Randr_Signal_Property            *ecore_x_randr_output_signal_properties_get(Ecore_X_Window root, Ecore_X_Randr_Output output, int *num);
EAPI int                                       ecore_x_randr_output_connector_number_get(Ecore_X_Window root, Ecore_X_Randr_Output output);
EAPI Ecore_X_Randr_Connector_Type              ecore_x_randr_output_connector_type_get(Ecore_X_Window root, Ecore_X_Randr_Output output);
EAPI void                                      ecore_x_randr_crtc_panning_area_get(Ecore_X_Window root, Ecore_X_Randr_Crtc crtc, int *x, int *y, int *w, int *h); /**< @since 1.8 */
EAPI Eina_Bool                                 ecore_x_randr_crtc_panning_area_set(Ecore_X_Window root, Ecore_X_Randr_Crtc crtc, const int x, const int y, const int w, const int h); /**< @since 1.8 */
EAPI void                                      ecore_x_randr_crtc_tracking_area_get(Ecore_X_Window root, Ecore_X_Randr_Crtc crtc, int *x, int *y, int *w, int *h); /**< @since 1.8 */
EAPI Eina_Bool                                 ecore_x_randr_crtc_tracking_area_set(Ecore_X_Window root, Ecore_X_Randr_Crtc crtc, const int x, const int y, const int w, const int h); /**< @since 1.8 */
EAPI void                                      ecore_x_randr_crtc_border_area_get(Ecore_X_Window root, Ecore_X_Randr_Crtc crtc, int *x, int *y, int *w, int *h); /**< @since 1.8 */
EAPI Eina_Bool                                 ecore_x_randr_crtc_border_area_set(Ecore_X_Window root, Ecore_X_Randr_Crtc crtc, const int left, const int top, const int right, const int bottom); /**< @since 1.8 */

/* XRender Support (horrendously incomplete) */
typedef Ecore_X_ID Ecore_X_Picture;

/* XFixes Extension Support */
typedef Ecore_X_ID Ecore_X_Region;

typedef enum _Ecore_X_Region_Type
{
   ECORE_X_REGION_BOUNDING,
   ECORE_X_REGION_CLIP
} Ecore_X_Region_Type;

EAPI Ecore_X_Region     ecore_x_region_new(Ecore_X_Rectangle *rects, int num);
EAPI Ecore_X_Region     ecore_x_region_new_from_bitmap(Ecore_X_Pixmap bitmap);
EAPI Ecore_X_Region     ecore_x_region_new_from_window(Ecore_X_Window win, Ecore_X_Region_Type type);
EAPI Ecore_X_Region     ecore_x_region_new_from_gc(Ecore_X_GC gc);
EAPI Ecore_X_Region     ecore_x_region_new_from_picture(Ecore_X_Picture picture);
EAPI void               ecore_x_region_free(Ecore_X_Region region);
EAPI void               ecore_x_region_set(Ecore_X_Region region, Ecore_X_Rectangle *rects, int num);
EAPI void               ecore_x_region_copy(Ecore_X_Region dest, Ecore_X_Region source);
EAPI void               ecore_x_region_combine(Ecore_X_Region dest, Ecore_X_Region source1, Ecore_X_Region source2);
EAPI void               ecore_x_region_intersect(Ecore_X_Region dest, Ecore_X_Region source1, Ecore_X_Region source2);
EAPI void               ecore_x_region_subtract(Ecore_X_Region dest, Ecore_X_Region source1, Ecore_X_Region source2);
EAPI void               ecore_x_region_invert(Ecore_X_Region dest, Ecore_X_Rectangle *bounds, Ecore_X_Region source);
EAPI void               ecore_x_region_translate(Ecore_X_Region region, int dx, int dy);
EAPI void               ecore_x_region_extents(Ecore_X_Region dest, Ecore_X_Region source);
EAPI Ecore_X_Rectangle *ecore_x_region_fetch(Ecore_X_Region region, int *num, Ecore_X_Rectangle *bounds);
EAPI void               ecore_x_region_expand(Ecore_X_Region dest, Ecore_X_Region source, unsigned int left, unsigned int right, unsigned int top, unsigned int bottom);
EAPI void               ecore_x_region_gc_clip_set(Ecore_X_Region region, Ecore_X_GC gc, int x_origin, int y_origin);
EAPI void               ecore_x_region_window_shape_set(Ecore_X_Region region, Ecore_X_Window win, Ecore_X_Shape_Type type, int x_offset, int y_offset);
EAPI void               ecore_x_region_picture_clip_set(Ecore_X_Region region, Ecore_X_Picture picture, int x_origin, int y_origin);

/**
 * xfixes selection notification request.
 *
 * This lets you choose which selections you want to get notifications for.
 * @param selection The selection atom.
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise.
 * @since 1.1.0
 */
EAPI Eina_Bool          ecore_x_fixes_selection_notification_request(Ecore_X_Atom selection);

/* XComposite Extension Support */
EAPI Eina_Bool          ecore_x_composite_query(void);
EAPI void               ecore_x_composite_redirect_window(Ecore_X_Window win, Ecore_X_Composite_Update_Type type);
EAPI void               ecore_x_composite_redirect_subwindows(Ecore_X_Window win, Ecore_X_Composite_Update_Type type);
EAPI void               ecore_x_composite_unredirect_window(Ecore_X_Window win, Ecore_X_Composite_Update_Type type);
EAPI void               ecore_x_composite_unredirect_subwindows(Ecore_X_Window win, Ecore_X_Composite_Update_Type type);
EAPI Ecore_X_Pixmap     ecore_x_composite_name_window_pixmap_get(Ecore_X_Window win);
EAPI void               ecore_x_composite_window_events_disable(Ecore_X_Window win);
EAPI void               ecore_x_composite_window_events_enable(Ecore_X_Window win);
EAPI Ecore_X_Window     ecore_x_composite_render_window_enable(Ecore_X_Window root);
EAPI void               ecore_x_composite_render_window_disable(Ecore_X_Window root);

/* XPresent Extension Support */
/** @since 1.9 */
EAPI void ecore_x_present_select_events(Ecore_X_Window win, unsigned int events);
/** @since 1.9 */
EAPI void ecore_x_present_notify_msc(Ecore_X_Window win, unsigned int serial, unsigned long long target_msc, unsigned long long divisor, unsigned long long remainder);
/** @since 1.9 */
EAPI void ecore_x_present_pixmap(Ecore_X_Window win, Ecore_X_Pixmap pixmap, unsigned int serial, Ecore_X_Region valid,
                                 Ecore_X_Region update, int x_off, int y_off, Ecore_X_Randr_Crtc target_crtc,
                                 Ecore_X_Sync_Fence wait_fence, Ecore_X_Sync_Fence idle_fence, unsigned int options,
                                 unsigned long long target_msc, unsigned long long divisor, unsigned long long remainder,
                                 Ecore_X_Present *notifies, int num_notifies);
/** @since 1.9 */
EAPI Eina_Bool ecore_x_present_exists(void);

/* XDamage Extension Support */
typedef Ecore_X_ID Ecore_X_Damage;

typedef enum _Ecore_X_Damage_Report_Level
{
   ECORE_X_DAMAGE_REPORT_RAW_RECTANGLES,
   ECORE_X_DAMAGE_REPORT_DELTA_RECTANGLES,
   ECORE_X_DAMAGE_REPORT_BOUNDING_BOX,
   ECORE_X_DAMAGE_REPORT_NON_EMPTY
} Ecore_X_Damage_Report_Level;

struct _Ecore_X_Event_Damage
{
   Ecore_X_Damage_Report_Level level;
   Ecore_X_Drawable            drawable;
   Ecore_X_Damage              damage;
   int                         more;
   Ecore_X_Time                time;
   Ecore_X_Rectangle           area;
   Ecore_X_Rectangle           geometry;
};

typedef struct _Ecore_X_Event_Damage Ecore_X_Event_Damage;

struct _Ecore_X_Event_Xkb
{
   int group;
};
typedef struct _Ecore_X_Event_Xkb Ecore_X_Event_Xkb; /** @since 1.7 */

EAPI Eina_Bool      ecore_x_damage_query(void);
EAPI Ecore_X_Damage ecore_x_damage_new(Ecore_X_Drawable d, Ecore_X_Damage_Report_Level level);
EAPI void           ecore_x_damage_free(Ecore_X_Damage damage);
EAPI void           ecore_x_damage_subtract(Ecore_X_Damage damage, Ecore_X_Region repair, Ecore_X_Region parts);

EAPI Eina_Bool      ecore_x_screen_is_composited(int screen);
EAPI void           ecore_x_screen_is_composited_set(int screen, Ecore_X_Window win);

EAPI Eina_Bool      ecore_x_dpms_query(void);
EAPI Eina_Bool      ecore_x_dpms_capable_get(void);
EAPI Eina_Bool      ecore_x_dpms_enabled_get(void);
EAPI void           ecore_x_dpms_enabled_set(int enabled);
EAPI Ecore_X_Dpms_Mode  ecore_x_dpms_power_level_get(void);
EAPI void           ecore_x_dpms_timeouts_get(unsigned int *standby, unsigned int *suspend, unsigned int *off);
EAPI Eina_Bool      ecore_x_dpms_timeouts_set(unsigned int standby, unsigned int suspend, unsigned int off);
EAPI unsigned int   ecore_x_dpms_timeout_standby_get(void);
EAPI unsigned int   ecore_x_dpms_timeout_suspend_get(void);
EAPI unsigned int   ecore_x_dpms_timeout_off_get(void);
EAPI void           ecore_x_dpms_timeout_standby_set(unsigned int new_timeout);
EAPI void           ecore_x_dpms_timeout_suspend_set(unsigned int new_timeout);
EAPI void           ecore_x_dpms_timeout_off_set(unsigned int new_timeout);

EAPI Eina_Bool      ecore_x_test_fake_key_down(const char *key);
EAPI Eina_Bool      ecore_x_test_fake_key_up(const char *key);
EAPI Eina_Bool      ecore_x_test_fake_key_press(const char *key);
EAPI const char    *ecore_x_keysym_string_get(int keysym);

/**
 * Given a keyname, return the keycode representing that key
 * @param keyname The key from which to get the keycode.
 * @return The keycode of the key.
 *
 * @since 1.2.0
 */
EAPI int            ecore_x_keysym_keycode_get(const char *keyname);

typedef struct _Ecore_X_Image Ecore_X_Image;

EAPI Ecore_X_Image *ecore_x_image_new(int w, int h, Ecore_X_Visual vis, int depth);
EAPI void           ecore_x_image_free(Ecore_X_Image *im);
EAPI Eina_Bool      ecore_x_image_get(Ecore_X_Image *im, Ecore_X_Drawable draw, int x, int y, int sx, int sy, int w, int h);
EAPI void           ecore_x_image_put(Ecore_X_Image *im, Ecore_X_Drawable draw, Ecore_X_GC gc, int x, int y, int sx, int sy, int w, int h);
EAPI void          *ecore_x_image_data_get(Ecore_X_Image *im, int *bpl, int *rows, int *bpp);
EAPI Eina_Bool      ecore_x_image_is_argb32_get(Ecore_X_Image *im);

EAPI Eina_Bool      ecore_x_image_to_argb_convert(void *src, int sbpp, int sbpl, Ecore_X_Colormap c, Ecore_X_Visual v, int x, int y, int w, int h, unsigned int *dst, int dbpl, int dx, int dy);

EAPI Eina_Bool      ecore_x_input_multi_select(Ecore_X_Window win);
EAPI Eina_Bool	    ecore_x_input_raw_select(Ecore_X_Window win); /**< @since 1.8 */

EAPI Eina_Bool      ecore_x_vsync_animator_tick_source_set(Ecore_X_Window win);

typedef enum _Ecore_X_Gesture_Event_Mask
{
   ECORE_X_GESTURE_EVENT_MASK_NONE = 0L,
   ECORE_X_GESTURE_EVENT_MASK_FLICK = (1L << 0),
   ECORE_X_GESTURE_EVENT_MASK_PAN = (1L << 1),
   ECORE_X_GESTURE_EVENT_MASK_PINCHROTATION = (1L << 2),
   ECORE_X_GESTURE_EVENT_MASK_TAP = (1L << 3),
   ECORE_X_GESTURE_EVENT_MASK_TAPNHOLD = (1L << 4),
   ECORE_X_GESTURE_EVENT_MASK_HOLD = (1L << 5),
   ECORE_X_GESTURE_EVENT_MASK_GROUP = (1L << 6)
} Ecore_X_Gesture_Event_Mask;

typedef enum _Ecore_X_Gesture_Event_Type
{
   ECORE_X_GESTURE_EVENT_FLICK,
   ECORE_X_GESTURE_EVENT_PAN,
   ECORE_X_GESTURE_EVENT_PINCHROTATION,
   ECORE_X_GESTURE_EVENT_TAP,
   ECORE_X_GESTURE_EVENT_TAPNHOLD,
   ECORE_X_GESTURE_EVENT_HOLD,
   ECORE_X_GESTURE_EVENT_GROUP
} Ecore_X_Gesture_Event_Type;

typedef enum _Ecore_X_Gesture_Event_Subtype
{
   ECORE_X_GESTURE_END,
   ECORE_X_GESTURE_BEGIN,
   ECORE_X_GESTURE_UPDATE,
   ECORE_X_GESTURE_DONE
} Ecore_X_Gesture_Event_Subtype;

typedef enum _Ecore_X_Gesture_Group_Subtype
{
   ECORE_X_GESTURE_GROUP_REMOVED,
   ECORE_X_GESTURE_GROUP_ADDED,
   ECORE_X_GESTURE_GROUP_CURRENT
} Ecore_X_Gesture_Group_Subtype;

typedef enum _Ecore_X_Gesture_Direction
{
   ECORE_X_GESTURE_NORTHWARD,
   ECORE_X_GESTURE_NORTHEASTWARD,
   ECORE_X_GESTURE_EASTWARD,
   ECORE_X_GESTURE_SOUTHEASTWARD,
   ECORE_X_GESTURE_SOUTHWARD,
   ECORE_X_GESTURE_SOUTHWESTWARD,
   ECORE_X_GESTURE_WESTWARD,
   ECORE_X_GESTURE_NORTHWESTWARD
} Ecore_X_Gesture_Direction;

struct _Ecore_X_Event_Gesture_Notify_Flick
{
   Ecore_X_Window                win;
   Ecore_X_Time                  time;
   Ecore_X_Gesture_Event_Subtype subtype;
   int                           num_fingers;
   int                           distance;
   Ecore_X_Time                  duration;
   Ecore_X_Gesture_Direction     direction;
   double                        angle;
};

struct _Ecore_X_Event_Gesture_Notify_Pan
{
   Ecore_X_Window                win;
   Ecore_X_Time                  time;
   Ecore_X_Gesture_Event_Subtype subtype;
   int                           num_fingers;
   int                           dx;
   int                           dy;
   int                           distance;
   Ecore_X_Time                  duration;
   Ecore_X_Gesture_Direction     direction;
};

struct _Ecore_X_Event_Gesture_Notify_PinchRotation
{
   Ecore_X_Window                win;
   Ecore_X_Time                  time;
   Ecore_X_Gesture_Event_Subtype subtype;
   int                           num_fingers;
   int                           distance;
   int                           cx;
   int                           cy;
   double                        zoom;
   double                        angle;
};

struct _Ecore_X_Event_Gesture_Notify_Tap
{
   Ecore_X_Window                win;
   Ecore_X_Time                  time;
   Ecore_X_Gesture_Event_Subtype subtype;
   int                           num_fingers;
   int                           cx;
   int                           cy;
   int                           tap_repeat;
   Ecore_X_Time                  interval;
};

struct _Ecore_X_Event_Gesture_Notify_TapNHold
{
   Ecore_X_Window                win;
   Ecore_X_Time                  time;
   Ecore_X_Gesture_Event_Subtype subtype;
   int                           num_fingers;
   int                           cx;
   int                           cy;
   Ecore_X_Time                  interval;
   Ecore_X_Time                  hold_time;
};

struct _Ecore_X_Event_Gesture_Notify_Hold
{
   Ecore_X_Window                win;
   Ecore_X_Time                  time;
   Ecore_X_Gesture_Event_Subtype subtype;
   int                           num_fingers;
   int                           cx;
   int                           cy;
   Ecore_X_Time                  hold_time;
};

struct _Ecore_X_Event_Gesture_Notify_Group
{
   Ecore_X_Window                win;
   Ecore_X_Time                  time;
   Ecore_X_Gesture_Group_Subtype subtype;
   int                           num_groups;
   int                           group_id;
};

EAPI Eina_Bool                             ecore_x_gesture_supported(void);

EAPI Eina_Bool                             ecore_x_gesture_events_select(Ecore_X_Window win, Ecore_X_Gesture_Event_Mask mask);

EAPI Ecore_X_Gesture_Event_Mask            ecore_x_gesture_events_selected_get(Ecore_X_Window win);

EAPI Eina_Bool                             ecore_x_gesture_event_grab(Ecore_X_Window win, Ecore_X_Gesture_Event_Type type, int num_fingers);

EAPI Eina_Bool                             ecore_x_gesture_event_ungrab(Ecore_X_Window win, Ecore_X_Gesture_Event_Type type, int num_fingers);

EAPI void                                  ecore_x_e_illume_indicator_state_set(Ecore_X_Window win, Ecore_X_Illume_Indicator_State state);
EAPI Ecore_X_Illume_Indicator_State        ecore_x_e_illume_indicator_state_get(Ecore_X_Window win);
EAPI void                                  ecore_x_e_illume_indicator_state_send(Ecore_X_Window win, Ecore_X_Illume_Indicator_State state);

EAPI void                                  ecore_x_e_illume_indicator_opacity_set(Ecore_X_Window win, Ecore_X_Illume_Indicator_Opacity_Mode mode);
EAPI Ecore_X_Illume_Indicator_Opacity_Mode ecore_x_e_illume_indicator_opacity_get(Ecore_X_Window win);
EAPI void                                  ecore_x_e_illume_indicator_opacity_send(Ecore_X_Window win, Ecore_X_Illume_Indicator_Opacity_Mode mode);

EAPI void                                  ecore_x_e_illume_indicator_type_set(Ecore_X_Window win, Ecore_X_Illume_Indicator_Type_Mode mode);  /**< @since 1.8 */
EAPI Ecore_X_Illume_Indicator_Type_Mode    ecore_x_e_illume_indicator_type_get(Ecore_X_Window win);  /**< @since 1.8 */
EAPI void                                  ecore_x_e_illume_indicator_type_send(Ecore_X_Window win, Ecore_X_Illume_Indicator_Type_Mode mode);  /**< @since 1.8 */

EAPI void                                  ecore_x_e_illume_window_state_set(Ecore_X_Window win, Ecore_X_Illume_Window_State state);
EAPI Ecore_X_Illume_Window_State           ecore_x_e_illume_window_state_get(Ecore_X_Window win);
EAPI void                                  ecore_x_e_illume_window_state_send(Ecore_X_Window win, Ecore_X_Illume_Window_State state); /**< @since 1.9 */

EAPI void                                  ecore_x_xkb_select_group(int group); /* @since 1.7 */

EAPI void                                  ecore_x_e_window_rotation_supported_set(Ecore_X_Window root, Eina_Bool enabled); /**< @since 1.9 */
EAPI Eina_Bool                             ecore_x_e_window_rotation_supported_get(Ecore_X_Window root); /**< @since 1.9 */
EAPI void                                  ecore_x_e_window_rotation_app_set(Ecore_X_Window win, Eina_Bool set); /**< @since 1.9 */
EAPI Eina_Bool                             ecore_x_e_window_rotation_app_get(Ecore_X_Window win); /**< @since 1.9 */
EAPI void                                  ecore_x_e_window_rotation_preferred_rotation_set(Ecore_X_Window win, int rot); /**< @since 1.9 */
EAPI Eina_Bool                             ecore_x_e_window_rotation_preferred_rotation_get(Ecore_X_Window win, int *rot); /**< @since 1.9 */
EAPI void                                  ecore_x_e_window_rotation_available_rotations_set(Ecore_X_Window win, const int *rots, unsigned int count); /**< @since 1.9 */
EAPI Eina_Bool                             ecore_x_e_window_rotation_available_rotations_get(Ecore_X_Window win, int **rots, unsigned int *count); /**< @since 1.9 */
EAPI void                                  ecore_x_e_window_rotation_change_prepare_send(Ecore_X_Window win, int rot, Eina_Bool resize, int w, int h); /**< @since 1.9 */
EAPI void                                  ecore_x_e_window_rotation_change_prepare_done_send(Ecore_X_Window root, Ecore_X_Window win, int rot); /**< @since 1.9 */
EAPI void                                  ecore_x_e_window_rotation_change_request_send(Ecore_X_Window win, int rot); /**< @since 1.9 */
EAPI void                                  ecore_x_e_window_rotation_change_done_send(Ecore_X_Window root, Ecore_X_Window win, int rot, int w, int h); /**< @since 1.9 */

#ifdef __cplusplus
}
#endif // ifdef __cplusplus

#include <Ecore_X_Atoms.h>
#include <Ecore_X_Cursor.h>

#endif // ifndef _ECORE_X_H
