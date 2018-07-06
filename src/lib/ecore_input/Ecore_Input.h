#ifndef _ECORE_INPUT_H
#define _ECORE_INPUT_H

#ifdef _WIN32
# include <stddef.h>
#else
# include <inttypes.h>
#endif

#include <Eina.h>

#include <Eo.h>

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

/**
 * @defgroup Ecore_Input_Group Ecore Input
 * @ingroup Ecore_Group
 *
 *@{
 */
   EAPI extern int ECORE_EVENT_KEY_DOWN;
   EAPI extern int ECORE_EVENT_KEY_UP;
   EAPI extern int ECORE_EVENT_MOUSE_BUTTON_DOWN;
   EAPI extern int ECORE_EVENT_MOUSE_BUTTON_UP;
   EAPI extern int ECORE_EVENT_MOUSE_MOVE;
   EAPI extern int ECORE_EVENT_MOUSE_WHEEL;
   EAPI extern int ECORE_EVENT_MOUSE_IN;
   EAPI extern int ECORE_EVENT_MOUSE_OUT;
   EAPI extern int ECORE_EVENT_AXIS_UPDATE; /**< @since 1.13 */
   EAPI extern int ECORE_EVENT_MOUSE_BUTTON_CANCEL; /**< @since 1.15 */
   EAPI extern int ECORE_EVENT_JOYSTICK; /**< @since 1.18 */

#define ECORE_EVENT_MODIFIER_SHIFT      0x0001
#define ECORE_EVENT_MODIFIER_CTRL       0x0002
#define ECORE_EVENT_MODIFIER_ALT        0x0004
#define ECORE_EVENT_MODIFIER_WIN        0x0008
#define ECORE_EVENT_MODIFIER_SCROLL     0x0010
#define ECORE_EVENT_MODIFIER_NUM        0x0020
#define ECORE_EVENT_MODIFIER_CAPS       0x0040
#define ECORE_EVENT_LOCK_SCROLL         0x0080
#define ECORE_EVENT_LOCK_NUM            0x0100
#define ECORE_EVENT_LOCK_CAPS           0x0200
#define ECORE_EVENT_LOCK_SHIFT          0x0300
#define ECORE_EVENT_MODIFIER_ALTGR      0x0400 /**< @since 1.7 */

#ifndef _ECORE_WINDOW_PREDEF
   typedef uintptr_t                        Ecore_Window;
#define _ECORE_WINDOW_PREDEF 1
#endif

   typedef struct _Ecore_Event_Key          Ecore_Event_Key;
   typedef struct _Ecore_Event_Mouse_Button Ecore_Event_Mouse_Button;
   typedef struct _Ecore_Event_Mouse_Wheel  Ecore_Event_Mouse_Wheel;
   typedef struct _Ecore_Event_Mouse_Move   Ecore_Event_Mouse_Move;
   typedef struct _Ecore_Event_Mouse_IO     Ecore_Event_Mouse_IO;
   typedef struct _Ecore_Event_Modifiers    Ecore_Event_Modifiers;
   typedef struct _Ecore_Event_Axis_Update  Ecore_Event_Axis_Update; /**< @since 1.13 */
   typedef struct _Ecore_Axis               Ecore_Axis; /**< @since 1.13 */
   typedef struct _Ecore_Event_Joystick     Ecore_Event_Joystick; /**< @since 1.18 */

   /**
    * @typedef Ecore_Event_Modifier
    * An enum of modifier events.
    */
   typedef enum _Ecore_Event_Modifier
     {
        ECORE_NONE,
        ECORE_SHIFT,
        ECORE_CTRL,
        ECORE_ALT,
        ECORE_WIN,
        ECORE_SCROLL,
        ECORE_CAPS,
        ECORE_MODE, /**< @since 1.7 */
        ECORE_LAST
     } Ecore_Event_Modifier;

   /**
    * @typedef Ecore_Event_Press
    * An enum of press events.
    */
   typedef enum _Ecore_Event_Press
     {
        ECORE_DOWN,
        ECORE_UP,
        ECORE_CANCEL
     } Ecore_Event_Press;

   /**
    * @typedef Ecore_Event_IO
    * An enum of Input/Output events.
    */
   typedef enum _Ecore_Event_IO
     {
        ECORE_IN,
        ECORE_OUT
     } Ecore_Event_IO;

   /**
    * @typedef Ecore_Compose_State
    * An enum of Compose states.
    */
   typedef enum _Ecore_Compose_State
     {   
        ECORE_COMPOSE_NONE,
        ECORE_COMPOSE_MIDDLE,
        ECORE_COMPOSE_DONE
     } Ecore_Compose_State;

   /**
    * @struct _Ecore_Event_Joystic_Button
    * Contains information about a joystick button event.
    */
   typedef enum _Ecore_Event_Joystick_Button
     {
        ECORE_EVENT_JOYSTICK_BUTTON_NONE,
        ECORE_EVENT_JOYSTICK_BUTTON_FACE_0,
        ECORE_EVENT_JOYSTICK_BUTTON_FACE_1,
        ECORE_EVENT_JOYSTICK_BUTTON_FACE_2,
        ECORE_EVENT_JOYSTICK_BUTTON_FACE_3,
        ECORE_EVENT_JOYSTICK_BUTTON_LEFT_SHOULDER,
        ECORE_EVENT_JOYSTICK_BUTTON_RIGHT_SHOULDER,
        ECORE_EVENT_JOYSTICK_BUTTON_SELECT,
        ECORE_EVENT_JOYSTICK_BUTTON_START,
        ECORE_EVENT_JOYSTICK_BUTTON_LEFT_ANALOG_STICK,
        ECORE_EVENT_JOYSTICK_BUTTON_RIGHT_ANALOG_STICK,
        ECORE_EVENT_JOYSTICK_BUTTON_META,
        ECORE_EVENT_JOYSTICK_BUTTON_LAST
     } Ecore_Event_Joystick_Button; /**< @since 1.18 */

   /**
    * @struct _Ecore_Event_Joystic_Axis
    * Contains information about a joystick axis event.
    */
   typedef enum _Ecore_Event_Joystick_Axis
     {
        ECORE_EVENT_JOYSTICK_AXIS_NONE,
        ECORE_EVENT_JOYSTICK_AXIS_HAT_X,
        ECORE_EVENT_JOYSTICK_AXIS_HAT_Y,
        ECORE_EVENT_JOYSTICK_AXIS_LEFT_SHOULDER,
        ECORE_EVENT_JOYSTICK_AXIS_RIGHT_SHOULDER,
        ECORE_EVENT_JOYSTICK_AXIS_LEFT_ANALOG_HOR,
        ECORE_EVENT_JOYSTICK_AXIS_LEFT_ANALOG_VER,
        ECORE_EVENT_JOYSTICK_AXIS_RIGHT_ANALOG_HOR,
        ECORE_EVENT_JOYSTICK_AXIS_RIGHT_ANALOG_VER,
        ECORE_EVENT_JOYSTICK_AXIS_LAST
     } Ecore_Event_Joystick_Axis; /**< @since 1.18 */

   /**
    * @struct _Ecore_Event_Joystic_Event_Type
    * Contains information about a joystick event type.
    */
   typedef enum _Ecore_Event_Joystick_Event
     {
        ECORE_EVENT_JOYSTICK_EVENT_TYPE_NONE,
        ECORE_EVENT_JOYSTICK_EVENT_TYPE_CONNECTED,
        ECORE_EVENT_JOYSTICK_EVENT_TYPE_DISCONNECTED,
        ECORE_EVENT_JOYSTICK_EVENT_TYPE_BUTTON,
        ECORE_EVENT_JOYSTICK_EVENT_TYPE_AXIS,
        ECORE_EVENT_JOYSTICK_EVENT_TYPE_LAST
     } Ecore_Event_Joystick_Event_Type; /**< @since 1.18 */

   /**
    * @struct _Ecore_Event_Key
    * Contains information about an Ecore keyboard event.
    */
   struct _Ecore_Event_Key
     {
        const char      *keyname; /**< The key name */
        const char      *key; /**< The key symbol */
        const char      *string;
        const char      *compose; /**< final string corresponding to the key symbol composed */
        Ecore_Window     window; /**< The main window where event happened */
        Ecore_Window     root_window; /**< The root window where event happened */
        Ecore_Window     event_window; /**< The child window where event happened */
        
        unsigned int     timestamp; /**< Time when the event occurred */
        unsigned int     modifiers; /**< The combination of modifiers key (SHIFT,CTRL,ALT,..)*/
        
        int              same_screen; /**< same screen flag */

        unsigned int     keycode; /**< Key scan code numeric value @since 1.10 */

        void            *data; /**< User data associated with an Ecore_Event_Key @since 1.10 */

        Eo *dev; /**< The Efl_Input_Device that originated the event @since 1.19 */
     };

   /**
    * @struct _Ecore_Event_Mouse_Button
    * Contains information about an Ecore mouse button event.
    */
   struct _Ecore_Event_Mouse_Button
     {
        Ecore_Window     window; /**< The main window where event happened */
        Ecore_Window     root_window; /**< The root window where event happened */
        Ecore_Window     event_window; /**< The child window where event happened */

        unsigned int     timestamp; /**< Time when the event occurred */
        unsigned int     modifiers; /**< The combination of modifiers key (SHIFT,CTRL,ALT,..)*/
        unsigned int     buttons; /**< The button that was used */
        unsigned int     double_click; /**< Double click event */
        unsigned int     triple_click; /**< Triple click event */
        int              same_screen; /**< Same screen flag */
        
        int              x; /**< x coordinate relative to window where event happened */
        int              y; /**< y coordinate relative to window where event happened */
        struct {
           int           x;
           int           y;
        } root; /**< Coordinates relative to root window */
        
        struct {
           int           device; /**< 0 if normal mouse, 1+ for other mouse-devices (eg multi-touch - other fingers) */
           double        radius, radius_x, radius_y; /**< radius of press point - radius_x and y if its an ellipse (radius is the average of the 2) */
           double        pressure; /**< pressure - 1.0 == normal, > 1.0 == more, 0.0 == none */
           double        angle; /**< angle relative to perpendicular (0.0 == perpendicular), in degrees */
           double        x, y; /**< same as x, y, but with sub-pixel precision, if available */
           struct {
              double     x, y;
           } root; /**< same as root.x, root.y, but with sub-pixel precision, if available */
        } multi;

        Eo *dev; /**< The Efl_Input_Device that originated the event @since 1.19 */
     };

   /**
    * @struct _Ecore_Event_Mouse_Wheel
    * Contains information about an Ecore mouse wheel event.
    */
   struct _Ecore_Event_Mouse_Wheel
     {
        Ecore_Window     window; /**< The main window where event happened */
        Ecore_Window     root_window; /**< The root window where event happened */
        Ecore_Window     event_window; /**< The child window where event happened */
        
        unsigned int     timestamp; /**< Time when the event occurred */
        unsigned int     modifiers; /**< The combination of modifiers key (SHIFT,CTRL,ALT,..)*/
        
        int              same_screen; /**< Same screen flag */
        int              direction; /**< Orientation of the wheel (horizontal/vertical) */
        int              z; /**< Value of the wheel event (+1/-1) */
        
        int              x; /**< x coordinate relative to window where event happened */
        int              y; /**< y coordinate relative to window where event happened */
        struct {
           int           x;
           int           y;
        } root; /**< Coordinates relative to root window */

        Eo *dev; /**< The Efl_Input_Device that originated the event @since 1.19 */
     };

   /**
    * @struct _Ecore_Event_Mouse_Move
    * Contains information about an Ecore mouse move event.
    */
   struct _Ecore_Event_Mouse_Move
     {
        Ecore_Window     window; /**< The main window where event happened */
        Ecore_Window     root_window; /**< The root window where event happened */
        Ecore_Window     event_window; /**< The child window where event happened */
        
        unsigned int     timestamp; /**< Time when the event occurred */
        unsigned int     modifiers; /**< The combination of modifiers key (SHIFT,CTRL,ALT,..)*/
        
        int              same_screen; /**< Same screen flag */
        
        int              x; /**< x coordinate relative to window where event happened */
        int              y; /**< y coordinate relative to window where event happened */
        struct {
           int           x;
           int           y;
        } root; /**< Coordinates relative to root window */
        
        struct {
           int           device; /**< 0 if normal mouse, 1+ for other mouse-devices (eg multi-touch - other fingers) */
           double        radius, radius_x, radius_y; /**< radius of press point - radius_x and y if its an ellipse (radius is the average of the 2) */
           double        pressure; /**< pressure - 1.0 == normal, > 1.0 == more, 0.0 == none */
           double        angle; /**< angle relative to perpendicular (0.0 == perpendicular), in degrees */
           double        x, y; /**< same as x, y root.x, root.y, but with sub-pixel precision, if available */
           struct {
              double     x, y;
           } root;
        } multi;

        Eo *dev; /**< The Efl_Input_Device that originated the event @since 1.19 */
     };

   typedef enum _Ecore_Axis_Label
     {
        ECORE_AXIS_LABEL_UNKNOWN,       /**< Axis type is not known. Range: Unbounded. Unit: Undefined. @since 1.13 */
        ECORE_AXIS_LABEL_X,             /**< Position along physical X axis; not window relative. Range: Unbounded. Unit: Undefined. @since 1.13 */
        ECORE_AXIS_LABEL_Y,             /**< Position along physical Y axis; not window relative. Range: Unbounded. Unit: Undefined. @since 1.13 */
        ECORE_AXIS_LABEL_PRESSURE,      /**< Force applied to tool tip. Range: [0.0, 1.0]. Unit: Unitless. @since 1.13 */
        ECORE_AXIS_LABEL_DISTANCE,      /**< Relative distance along physical Z axis. Range: [0.0, 1.0]. Unit: Unitless. @since 1.13 */
        ECORE_AXIS_LABEL_AZIMUTH,       /**< Angle of tool about the Z axis from positive X axis. Range: [-PI, PI]. Unit: Radians. @since 1.13*/
        ECORE_AXIS_LABEL_TILT,          /**< Angle of tool about plane of sensor from positive Z axis. Range: [0.0, PI]. Unit: Radians. @since 1.13 */
        ECORE_AXIS_LABEL_TWIST,         /**< Rotation of tool about its major axis from its "natural" position. Range: [-PI, PI] Unit: Radians. @since 1.13 */
        ECORE_AXIS_LABEL_TOUCH_WIDTH_MAJOR,   /**< Length of contact ellipse along AZIMUTH. Range: Unbounded: Unit: Same as ECORE_AXIS_LABEL_{X,Y}. @since 1.13 */
        ECORE_AXIS_LABEL_TOUCH_WIDTH_MINOR,   /**< Length of contact ellipse perpendicular to AZIMUTH. Range: Unbounded. Unit: Same as ECORE_AXIS_LABEL_{X,Y}. @since 1.13 */
        ECORE_AXIS_LABEL_TOOL_WIDTH_MAJOR,    /**< Length of tool ellipse along AZIMUTH. Range: Unbounded. Unit: Same as ECORE_AXIS_LABEL_{X,Y}. @since 1.13 */
        ECORE_AXIS_LABEL_TOOL_WIDTH_MINOR,    /**< Length of tool ellipse perpendicular to AZIMUTH. Range: Unbounded. Unit: Same as ECORE_AXIS_LABEL_{X,Y}. @since 1.13 */
        ECORE_AXIS_LABEL_WINDOW_X,      /**< X coordinate mapped to the window. @since 1.19 */
        ECORE_AXIS_LABEL_WINDOW_Y,      /**< Y coordinate mapped to the window. @since 1.19 */
        ECORE_AXIS_LABEL_NORMAL_X,      /**< X normalized to the [0, 1] range. @since 1.19 */
        ECORE_AXIS_LABEL_NORMAL_Y,      /**< Y normalized to the [0, 1] range. @since 1.19 */
        ECORE_AXIS_LABEL_TOUCH_PALM,    /**< Size of palm contact. Range: Unbounded. Unit: undefined. May be a boolean or a distance value, depending on the underlying H/W. @since 1.20 */
   } Ecore_Axis_Label; /**< @since 1.13 */

   struct _Ecore_Axis
     {
        Ecore_Axis_Label label;
        double value;
     };

   struct _Ecore_Event_Axis_Update
     {
        Ecore_Window     window;
        Ecore_Window     root_window;
        Ecore_Window     event_window;

        unsigned int timestamp;
        int device;
        int toolid;

        int naxis;
        Ecore_Axis *axis;

        Eo *dev; /**< The Efl_Input_Device that originated the event @since 1.19 */
     };

   /**
    * @struct _Ecore_Event_Mouse_IO
    * Contains information about an Ecore mouse input/output event.
    */
   struct _Ecore_Event_Mouse_IO
     {
        Ecore_Window     window; /**< The main window where event happened */
        Ecore_Window     event_window; /**< The child window where event happened */
        
        unsigned int     timestamp; /**< Time when the event occurred */
        unsigned int     modifiers; /**< The combination of modifiers key (SHIFT,CTRL,ALT,..)*/
        
        int              x; /**< x coordinate relative to window where event happened */
        int              y; /**< y coordinate relative to window where event happened */

        Eo *dev; /**< The Efl_Input_Device that originated the event @since 1.19 */
     };

   /**
    * @struct _Ecore_Event_Modifiers
    * Contains information about an Ecore event modifier.
    */
   struct _Ecore_Event_Modifiers
     {
        unsigned int size;
        unsigned int array[ECORE_LAST];
     };

   /**
    * @struct _Ecore_Event_Joystick
    * Contains information about a joystick event.
    */
   struct _Ecore_Event_Joystick
     {
        Ecore_Event_Joystick_Event_Type type;
        unsigned int                   index;
        unsigned int               timestamp;

        union
          {
             struct
               {
                  Ecore_Event_Joystick_Axis index;
                  double       value;  /* [-1.0 .. 1.0] -1.0 == up or left, 1.0 == down or right */
               } axis;

             struct
               {
                  Ecore_Event_Joystick_Button index;
                  double       value; /* [0.0 .. 1.0] 0.0 == fully unpressed, 1.0 == fully pressed */
               } button;
          };

        Eo *dev; /**< The Efl_Input_Device that originated the event @since 1.19 */
     };

   /**
    * Initializes the Ecore Event system.
    */
   EAPI int                  ecore_event_init(void);
   /**
    * Shutdowns the Ecore Event system.
    */
   EAPI int                  ecore_event_shutdown(void);

   /**
    * Returns the Ecore modifier event integer associated to a
    * Ecore_Event_Modifier modifier event.
    *
    * @param modifier A Ecore_Event_Modifier event.
    * @return A event_modifier integer that matches with the provided modifier
    * event.
    */
   EAPI unsigned int         ecore_event_modifier_mask(Ecore_Event_Modifier modifier);

   /**
    * Update a Ecore_Event_Modifiers array with "key" modifier.
    *
    * @param key A string describing a modifier key.
    * @param modifiers A Ecore_Event_Modifiers structure.
    * @param inc The value to increment in the modifiers array.
    *
    * @return ECORE_NONE if the key does not match with an existing one, else
    * the corresponding Ecore_Event_Modifier.
    */
   EAPI Ecore_Event_Modifier ecore_event_update_modifier(const char *key, Ecore_Event_Modifiers *modifiers, int inc);

   /**
    * Handles a sequence of key symbols to make a final compose string.
    *
    * The final compose string seqstr_ret is allocated in this function and
    * thus shall be freed when not needed anymore.
    *
    * @param seq The sequence of key symbols in a Eina_List.
    * @param seqstr_ret The final compose string.
    * @return The status of the composition.
    */
   EAPI Ecore_Compose_State  ecore_compose_get(const Eina_List *seq, char **seqstr_ret);

   /**
    * Set deadzone of joystick event for an axis.
    *
    * The axis type joystick event occurs without user's control if joystick is
    * too sensitive. The deadzone prevents unnecessary events.
    * The default value is 200. The event value for an axis is a signed integer
    * between -32767 and +32767.
    *
    * @param event_axis_deadzone The joystick event axis deadzone.
    * @since 1.19
    */
   EAPI void ecore_input_joystick_event_axis_deadzone_set(int event_axis_deadzone);

   /**
    * Get deadzone of joystick event for an axis.
    *
    * @return deadzone of joystick event for an axis.
    * @since 1.19
    */
   EAPI int ecore_input_joystick_event_axis_deadzone_get(void);

   /**
    * Get name of joystick
    *
    * This function returns the name string of the joysitck. If @p index
    * does not exist, or on error, this function returns NULL.
    *
    * @param index The index of joystick.
    * @return name of joystick.
    * @since 1.20
    */
   EAPI const char *ecore_input_joystick_name_get(int index);
#ifdef __cplusplus
}
#endif

#undef EAPI
#define EAPI

/** @} */
#endif
