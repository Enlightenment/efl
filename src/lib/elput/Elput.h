#ifndef _ELPUT_H
# define _ELPUT_H

# ifdef EFL_BETA_API_SUPPORT
# include <Eina.h>

# ifdef EAPI
#  undef EAPI
# endif

# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EAPI __attribute__ ((visibility("default")))
#  else
#   define EAPI
#  endif
# else
#  define EAPI
# endif


typedef enum
{
   ELPUT_DEVICE_CAPS_POINTER = (1 << 0),
   ELPUT_DEVICE_CAPS_KEYBOARD = (1 << 1),
   ELPUT_DEVICE_CAPS_TOUCH = (1 << 2),
   ELPUT_DEVICE_CAPS_TABLET_TOOL = (1 << 3),
   ELPUT_DEVICE_CAPS_TABLET_PAD = (1 << 4),
   ELPUT_DEVICE_CAPS_GESTURE = (1 << 5),
   ELPUT_DEVICE_CAPS_SWITCH = (1 << 6),
} Elput_Device_Caps;

/* opaque structure to represent an input manager */
typedef struct _Elput_Manager Elput_Manager;

/* opaque structure to represent an input seat */
typedef struct _Elput_Seat Elput_Seat;

/* opaque structure to represent an input device */
typedef struct _Elput_Device Elput_Device;

/* opaque structure to represent a keyboard */
typedef struct _Elput_Keyboard Elput_Keyboard;

/* opaque structure to represent a mouse */
typedef struct _Elput_Pointer Elput_Pointer;

/* opaque structure to represent a touch device */
typedef struct _Elput_Touch Elput_Touch;

/* structure to represent event for seat capability changes */
typedef struct _Elput_Event_Seat_Caps
{
   int pointer_count;
   int keyboard_count;
   int touch_count;
   Elput_Seat *seat;
} Elput_Event_Seat_Caps;

/* structure to represent event for seat frame */
typedef struct _Elput_Event_Seat_Frame
{
   Elput_Seat *seat;
} Elput_Event_Seat_Frame;

/* structure to represent event for seat modifiers changes */
typedef struct _Elput_Event_Modifiers_Send
{
   unsigned int depressed;
   unsigned int latched;
   unsigned int locked;
   unsigned int group;
} Elput_Event_Modifiers_Send;

typedef enum _Elput_Device_Change_Type
{
   ELPUT_DEVICE_ADDED,
   ELPUT_DEVICE_REMOVED,
} Elput_Device_Change_Type;

/* structure to represent event for device being added or removed */
typedef struct _Elput_Event_Device_Change
{
   Elput_Device *device;
   Elput_Device_Change_Type type;
} Elput_Event_Device_Change;

/* structure to represent session active changes */
typedef struct _Elput_Event_Session_Active
{
   const char *session;
   Eina_Bool active : 1;
} Elput_Event_Session_Active;

/** @since 1.19 */
typedef struct Elput_Event_Pointer_Motion
{
   uint64_t time_usec;
   double dx;
   double dy;
   double dx_unaccel;
   double dy_unaccel;
} Elput_Event_Pointer_Motion;

/** @since 1.21 */
typedef enum
{
   ELPUT_SWITCH_TYPE_LID = 1,
   ELPUT_SWITCH_TYPE_TABLET_MODE,
} Elput_Switch_Type;

/** @since 1.21 */
typedef enum
{
   ELPUT_SWITCH_STATE_OFF = 0,
   ELPUT_SWITCH_STATE_ON = 1,
} Elput_Switch_State;

/** @since 1.21 */
typedef struct _Elput_Event_Switch
{
   Elput_Device *device;
   uint64_t time_usec;
   Elput_Switch_Type type;
   Elput_Switch_State state;
} Elput_Event_Switch;


EAPI extern int ELPUT_EVENT_SEAT_CAPS;
EAPI extern int ELPUT_EVENT_SEAT_FRAME;
EAPI extern int ELPUT_EVENT_MODIFIERS_SEND;
EAPI extern int ELPUT_EVENT_DEVICE_CHANGE;
EAPI extern int ELPUT_EVENT_SESSION_ACTIVE;

/** @since 1.19 */
EAPI extern int ELPUT_EVENT_POINTER_MOTION;

/** @since 1.21 */
EAPI extern int ELPUT_EVENT_SWITCH;

/**
 * @file
 * @brief Ecore functions for dealing with libinput
 *
 * @defgroup Elput_Group Elput - libinput integration
 * @ingroup Ecore
 *
 * Elput provides a wrapper and functions for using libinput
 *
 * @li @ref Elput_Init_Group
 * @li @ref Elput_Manager_Group
 * @li @ref Elput_Input_Group
 * @li @ref Elput_Touch_Group
 *
 */

/**
 * @defgroup Elput_Init_Group Library Init and Shutdown functions
 *
 * Functions that start and shutdown the Elput library
 */

/**
 * Initialize the Elput library
 *
 * @return The number of times the library has been initialized without being
 *         shutdown. 0 is returned if an error occurs.
 *
 * @ingroup Elput_Init_Group
 * @since 1.18
 */
EAPI int elput_init(void);

/**
 * Shutdown the Elput library
 *
 * @return The number of times the library has been initialized without being
 *         shutdown. 0 is returned if an error occurs.
 *
 * @ingroup Elput_Init_Group
 * @since 1.18
 */
EAPI int elput_shutdown(void);

/**
 * @defgroup Elput_Manager_Group
 *
 * Functions that deal with connecting, disconnecting, opening, closing
 * of input devices.
 */

/**
 * Create an input manager on the specified seat
 *
 * @param seat
 * @param tty
 * @param sync
 *
 * @return A Elput_Manager on success, NULL on failure
 *
 * @ingroup Elput_Manager_Group
 * @since 1.18
 */
EAPI Elput_Manager *elput_manager_connect(const char *seat, unsigned int tty);

/**
 * Disconnect an input manager
 *
 * @param manager
 *
 * @ingroup Elput_Manager_Group
 * @since 1.18
 */
EAPI void elput_manager_disconnect(Elput_Manager *manager);

/**
 * Request input manager to open a file
 *
 * @param manager
 * @param path
 * @param flags
 *
 * @return Filedescriptor of opened file or -1 on failure
 *
 * @ingroup Elput_Manager_Group
 * @since 1.18
 */
EAPI int elput_manager_open(Elput_Manager *manager, const char *path, int flags);

/**
 * Request input manager to close a file
 *
 * @param manager
 * @param fd
 *
 * @ingroup Elput_Manager_Group
 * @since 1.18
 */
EAPI void elput_manager_close(Elput_Manager *manager, int fd);

/**
 * Request to switch to a given vt
 *
 * @param manager
 * @param vt
 *
 * @return EINA_TRUE on success, EINA_FALSE otherwise
 *
 * @ingroup Elput_Manager_Group
 * @since 1.18
 */
EAPI Eina_Bool elput_manager_vt_set(Elput_Manager *manager, int vt);

/**
 * Get the list of seats from a manager
 *
 * @param manager
 *
 * @return An Eina_List of existing Elput_Seats or NULL on failure
 *
 * @ingroup Elput_Manager_Group
 * @since 1.18
 */
EAPI const Eina_List *elput_manager_seats_get(Elput_Manager *manager);


/**
 * Set which window to use for this input manager
 *
 * This function should be used to specify which window to set on the
 * input manager. Setting a window on the input manager is done so that
 * when we raise events (mouse movement, keyboard key, etc) then this
 * window is passed to the event structure as the window which the event
 * occured on.
 *
 * @param manager
 * @param window
 *
 * @ingroup Elput_Manager_Group
 * @since 1.18
 */
EAPI void elput_manager_window_set(Elput_Manager *manager, unsigned int window);

/**
 * @defgroup Elput_Input_Group Elput input functions
 *
 * Functions that deal with setup of inputs
 */

/**
 * Initialize input
 *
 * @param manager
 *
 * @return EINA_TRUE on success, EINA_FALSE on failure
 *
 * @ingroup Elput_Input_Group
 * @since 1.18
 */
EAPI Eina_Bool elput_input_init(Elput_Manager *manager);

/**
 * Shutdown input
 *
 * @param manager
 *
 * @ingroup Elput_Input_Group
 * @since 1.18
 */
EAPI void elput_input_shutdown(Elput_Manager *manager);

/**
 * Get the pointer position on a given seat
 *
 * @param manager
 * @param seat
 * @param x
 * @param y
 *
 * @ingroup Elput_Input_Group
 * @since 1.18
 */
EAPI void elput_input_pointer_xy_get(Elput_Manager *manager, const char *seat, int *x, int *y);

/**
 * Set the pointer position on a given seat
 *
 * @param manager
 * @param seat
 * @param x
 * @param y
 *
 * @ingroup Elput_Input_Group
 * @since 1.18
 */
EAPI void elput_input_pointer_xy_set(Elput_Manager *manager, const char *seat, int x, int y);

/**
 * Set the pointer left-handed mode
 *
 * @param manager
 * @param seat
 * @param left
 *
 * @return EINA_TRUE on success, EINA_FALSE otherwise
 *
 * @ingroup Elput_Input_Group
 * @since 1.18
 */
EAPI Eina_Bool elput_input_pointer_left_handed_set(Elput_Manager *manager, const char *seat, Eina_Bool left);

/**
 * Set the maximum position of any existing mouse pointers
 *
 * @param manager
 * @param maxw
 * @param maxh
 *
 * @ingroup Elput_Input_Group
 * @since 1.18
 */
EAPI void elput_input_pointer_max_set(Elput_Manager *manager, int maxw, int maxh);

/**
 * Set pointer value rotation
 *
 * @param manager
 * @param rotation
 *
 * @return EINA_TRUE on success, EINA_FALSE otherwise
 *
 * @ingroup Elput_Input_Group
 * @since 1.20
 */
EAPI Eina_Bool elput_input_pointer_rotation_set(Elput_Manager *manager, int rotation);

/**
 * Set tap-to-click status
 *
 * @param manager
 * @param seat
 * @param enabled
 *
 * @return EINA_TRUE on success, EINA_FALSE otherwise
 *
 * @ingroup Elput_Input_Group
 * @since 1.22
 */
EAPI void elput_input_touch_tap_to_click_enabled_set(Elput_Manager *manager, const char *seat, Eina_Bool enabled);

/**
 * Calibrate input devices for given screen size
 *
 * @param manager
 * @param w
 * @param h
 *
 * @ingroup Elput_Input_Group
 * @since 1.18
 */
EAPI void elput_input_devices_calibrate(Elput_Manager *manager, int w, int h);

/**
 * Enable key remap functionality
 *
 * @param manager
 * @param enable
 *
 * @return EINA_TRUE on success, EINA_FALSE otherwise
 *
 * @ingroup Elput_Input_Group
 * @since 1.18
 */
EAPI Eina_Bool elput_input_key_remap_enable(Elput_Manager *manager, Eina_Bool enable);

/**
 * Set a given set of keys as remapped keys
 *
 * @param manager
 * @param from_keys
 * @param to_keys
 * @param num
 *
 * @return EINA_TRUE on success, EINA_FALSE otherwise
 *
 * @ingroup Elput_Input_Group
 * @since 1.18
 */
EAPI Eina_Bool elput_input_key_remap_set(Elput_Manager *manager, int *from_keys, int *to_keys, int num);

/**
 * Set info to be used for keyboards
 *
 * @param manager
 * @param context
 * @param keymap
 * @param group
 *
 * @ingroup Elput_Input_Group
 * @since 1.20
 */
EAPI void elput_input_keyboard_info_set(Elput_Manager *manager, void *context, void *keymap, int group);

/**
 * Set group layout to be used for keyboards
 *
 * @param manager
 * @param group
 *
 * @ingroup Elput_Input_Group
 * @since 1.20
 */
EAPI void elput_input_keyboard_group_set(Elput_Manager *manager, int group);

/**
 * Set the pointer acceleration profile
 *
 * @param manager
 * @param seat
 * @param profile
 *
 * @ingroup Elput_Input_Group
 * @since 1.19
 */
EAPI void elput_input_pointer_accel_profile_set(Elput_Manager *manager, const char *seat, uint32_t profile);

/**
 * Set the pointer acceleration speed
 *
 * @param manager
 * @param seat
 * @param speed
 *
 * @ingroup Elput_Input_Group
 * @since 1.21
 */
EAPI void elput_input_pointer_accel_speed_set(Elput_Manager *manager, const char *seat, double speed);

/**
 * @defgroup Elput_Touch_Group Configuration of touch devices
 *
 * Functions related to configuration of touch devices
 */

/**
 * Enable or disable tap-and-drag on this device.
 *
 * When enabled, a single-finger tap immediately followed by a finger
 * down results in a button down event, subsequent finger motion thus
 * triggers a drag. The button is released on finger up.
 *
 * @param device
 * @param enabled
 *
 * @return EINA_TRUE on success, EINA_FALSE otherwise
 *
 * @ingroup Elput_Touch_Group
 * @since 1.19
 */
EAPI Eina_Bool elput_touch_drag_enabled_set(Elput_Device *device, Eina_Bool enabled);

/**
 * Get if tap-and-drag is enabled on this device.
 *
 * @param device
 *
 * @return EINA_TRUE if enabled, EINA_FALSE otherwise
 *
 * @ingroup Elput_Touch_Group
 * @since 1.19
 */
EAPI Eina_Bool elput_touch_drag_enabled_get(Elput_Device *device);

/**
 * Enable or disable drag-lock during tapping on a device.
 *
 * When enabled, a finger may be lifted and put back on the touchpad
 * within a timeout and the drag process continues. When disabled,
 * lifting the finger during a tap-and-drag will immediately stop the
 * drag.
 *
 * @param device
 * @param enabled
 *
 * @return EINA_TRUE on sucess, EINA_FALSE otherwise
 *
 * @ingroup Elput_Touch_Group
 * @since 1.19
 */
EAPI Eina_Bool elput_touch_drag_lock_enabled_set(Elput_Device *device, Eina_Bool enabled);

/**
 * Get if drag-lock is enabled on this device.
 *
 * @param device
 *
 * @return EINA_TRUE if enabled, EINA_FALSE otherwise
 *
 * @ingroup Elput_Touch_Group
 * @since 1.19
 */
EAPI Eina_Bool elput_touch_drag_lock_enabled_get(Elput_Device *device);

/**
 * Enable or disable touchpad dwt (disable-while-typing) feature.
 *
 * When enabled, the device will be disabled while typing and for a
 * short period after.
 *
 * @param device
 * @param enabled
 *
 * @return EINA_TRUE on success, EINA_FALSE otherwise
 *
 * @ingroup Elput_Touch_Group
 * @since 1.19
 */
EAPI Eina_Bool elput_touch_dwt_enabled_set(Elput_Device *device, Eina_Bool enabled);

/**
 * Get if touchpad dwt (disable-while-typing) is enabled.
 *
 * @param device
 *
 * @return EINA_TRUE if enabled, EINA_FALSE otherwise
 *
 * @ingroup Elput_Touch_Group
 * @since 1.19
 */
EAPI Eina_Bool elput_touch_dwt_enabled_get(Elput_Device *device);

/**
 * Set the scroll method used for this device.
 *
 * The scroll method defines when to generate scroll axis events instead
 * of pointer motion events.
 *
 * @param device
 * @param method
 *
 * @return EINA_TRUE on success, EINA_FALSE otherwise
 *
 * @ingroup Elput_Touch_Group
 * @since 1.19
 */
EAPI Eina_Bool elput_touch_scroll_method_set(Elput_Device *device, int method);

/**
 * Get the current scroll method set on a device
 *
 * @param device
 *
 * @return The current scroll method
 *
 * @ingroup Elput_Touch_Group
 * @since 1.19
 */
EAPI int elput_touch_scroll_method_get(Elput_Device *device);

/**
 * Set the button click method for a device.
 *
 * The button click method defines when to generate software emulated
 * buttons
 *
 * @param device
 * @param method
 *
 * @return EINA_TRUE on success, EINA_FALSE otherwise
 *
 * @ingroup Elput_Touch_Group
 * @since 1.19
 */
EAPI Eina_Bool elput_touch_click_method_set(Elput_Device *device, int method);

/**
 * Get the current button click method for a device
 *
 * @param device
 *
 * @return The current button click method
 *
 * @ingroup Elput_Touch_Group
 * @since 1.19
 */
EAPI int elput_touch_click_method_get(Elput_Device *device);

/**
 * Enable or disable tap-to-click on a given device
 *
 * @param device
 * @param enabled
 *
 * @return EINA_TRUE on success, EINA_FALSE otherwise
 *
 * @ingroup Elput_Touch_Group
 * @since 1.19
 */
EAPI Eina_Bool elput_touch_tap_enabled_set(Elput_Device *device, Eina_Bool enabled);

/**
 * Get if tap-to-click is enabled on a given device
 *
 * @param device
 *
 * @return EINA_TRUE if enabled, EINA_FALSE otherwise
 *
 * @ingroup Elput_Touch_Group
 * @since 1.19
 */
EAPI Eina_Bool elput_touch_tap_enabled_get(Elput_Device *device);


/**
 * @defgroup Elput_Device_Group Elput device functions
 *
 * Functions for getting attributes of devices
 */

/**
 * Get the seat object for a device
 * @param dev
 * @return The seat
 * @ingroup Elput_Device_Group
 * @since 1.20
 */
EAPI Elput_Seat *elput_device_seat_get(const Elput_Device *dev);

/**
 * Get the caps for a device
 * @param dev
 * @return The caps, 0 on failure
 * @ingroup Elput_Device_Group
 * @since 1.20
 */
EAPI Elput_Device_Caps elput_device_caps_get(const Elput_Device *dev);

/**
 * Return the output name associated with a given device
 *
 * @param device
 *
 * @return An Eina_Stringshare of the output name for this device, or NULL on error
 *
 * @ingroup Elput_Device_Group
 * @since 1.20
 */
EAPI Eina_Stringshare *elput_device_output_name_get(Elput_Device *device);

/**
 * @defgroup Elput_Seat_Group Elput seat functions
 *
 * Functions for getting attributes of seats
 */

/**
 * Get the list of devices on a given seat
 *
 * @param seat
 *
 * @return An immutable list of existing Elput_Devices on a given seat or NULL on failure
 *
 * @ingroup Elput_Seat_Group
 * @since 1.20
 */
EAPI const Eina_List *elput_seat_devices_get(const Elput_Seat *seat);

/**
 * Get the name of a given seat
 *
 * @param seat
 *
 * @return The name
 *
 * @ingroup Elput_Seat_Group
 * @since 1.20
 */
EAPI Eina_Stringshare *elput_seat_name_get(const Elput_Seat *seat);

/**
 * Get the manager of a given seat
 *
 * @param seat
 *
 * @return The Elput_Manager
 *
 * @ingroup Elput_Seat_Group
 * @since 1.20
 */
EAPI Elput_Manager *elput_seat_manager_get(const Elput_Seat *seat);
# endif

# undef EAPI
# define EAPI

#endif
