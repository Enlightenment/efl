/**
 * @enum _Elm_Gesture_Type
 * Enum of supported gesture types.
 */
enum _Elm_Gesture_Type
{
   ELM_GESTURE_FIRST = 0,

   ELM_GESTURE_N_TAPS, /**< N fingers single taps */
   ELM_GESTURE_N_LONG_TAPS, /**< N fingers single long-taps */
   ELM_GESTURE_N_DOUBLE_TAPS, /**< N fingers double-single taps */
   ELM_GESTURE_N_TRIPLE_TAPS, /**< N fingers triple-single taps */

   ELM_GESTURE_MOMENTUM, /**< Reports momentum in the direction of move */

   ELM_GESTURE_N_LINES, /**< N fingers line gesture */
   ELM_GESTURE_N_FLICKS, /**< N fingers flick gesture */

   ELM_GESTURE_ZOOM, /**< Zoom */
   ELM_GESTURE_ROTATE, /**< Rotate */

   ELM_GESTURE_LAST
};

/**
 * @typedef Elm_Gesture_Type
 * Convenient macro around #_Elm_Gesture_Type
 */
typedef enum _Elm_Gesture_Type Elm_Gesture_Type;

/**
 * @enum _Elm_Gesture_State
 * Enum of gesture states.
 */
enum _Elm_Gesture_State
{
   ELM_GESTURE_STATE_UNDEFINED = -1, /**< Gesture not STARTed */
   ELM_GESTURE_STATE_START, /**< Gesture STARTed     */
   ELM_GESTURE_STATE_MOVE, /**< Gesture is ongoing  */
   ELM_GESTURE_STATE_END, /**< Gesture completed   */
   ELM_GESTURE_STATE_ABORT /**< Ongoing gesture was ABORTed */
};

/**
 * @typedef Elm_Gesture_State
 * Convenient macro around #_Elm_Gesture_State
 */
typedef enum _Elm_Gesture_State Elm_Gesture_State;

/**
 * @struct _Elm_Gesture_Taps_Info
 * Struct holds taps info for user
 */
struct _Elm_Gesture_Taps_Info
{
   Evas_Coord   x, y; /**< Holds center point between fingers */
   unsigned int n; /**< Number of fingers tapped           */
   unsigned int timestamp; /**< event timestamp       */
};

/**
 * @typedef Elm_Gesture_Taps_Info
 * holds taps info for user
 */
typedef struct _Elm_Gesture_Taps_Info Elm_Gesture_Taps_Info;

/**
 * @struct _Elm_Gesture_Momentum_Info
 * Struct holds momentum info for user
 * x1 and y1 are not necessarily in sync
 * x1 holds x value of x direction starting point
 * and same holds for y1.
 * This is noticeable when doing V-shape movement
 */
struct _Elm_Gesture_Momentum_Info /* Report line ends, timestamps, and momentum computed        */
{Evas_Coord   x1; /**< Final-swipe direction starting point on X */
 Evas_Coord   y1; /**< Final-swipe direction starting point on Y */
 Evas_Coord   x2; /**< Final-swipe direction ending point on X   */
 Evas_Coord   y2; /**< Final-swipe direction ending point on Y   */

 unsigned int tx; /**< Timestamp of start of final x-swipe */
 unsigned int ty; /**< Timestamp of start of final y-swipe */

 Evas_Coord   mx; /**< Momentum on X */
 Evas_Coord   my; /**< Momentum on Y */

 unsigned int n; /**< Number of fingers */
};

/**
 * @typedef Elm_Gesture_Momentum_Info
 * holds momentum info for user
 */
typedef struct _Elm_Gesture_Momentum_Info Elm_Gesture_Momentum_Info;

/**
 * @struct _Elm_Gesture_Line_Info
 * Struct holds line info for user
 */
struct _Elm_Gesture_Line_Info   /* Report line ends, timestamps, and momentum computed      */
{Elm_Gesture_Momentum_Info momentum; /**< Line momentum info */
 double                    angle; /**< Angle (direction) of lines  */
};

/**
 * @typedef Elm_Gesture_Line_Info
 * Holds line info for user
 */
typedef struct _Elm_Gesture_Line_Info Elm_Gesture_Line_Info;

/**
 * @struct _Elm_Gesture_Zoom_Info
 * Struct holds zoom info for user
 */
struct _Elm_Gesture_Zoom_Info
{
   Evas_Coord x, y; /**< Holds zoom center point reported to user  */
   Evas_Coord radius; /**< Holds radius between fingers reported to user */
   double     zoom; /**< Zoom value: 1.0 means no zoom             */
   double     momentum; /**< Zoom momentum: zoom growth per second (NOT YET SUPPORTED) */
};

/**
 * @typedef Elm_Gesture_Zoom_Info
 * Holds zoom info for user
 */
typedef struct _Elm_Gesture_Zoom_Info Elm_Gesture_Zoom_Info;

/**
 * @struct _Elm_Gesture_Rotate_Info
 * Struct holds rotation info for user
 */
struct _Elm_Gesture_Rotate_Info
{
   Evas_Coord x, y; /**< Holds zoom center point reported to user      */
   Evas_Coord radius; /**< Holds radius between fingers reported to user */
   double     base_angle; /**< Holds start-angle */
   double     angle; /**< Rotation value: 0.0 means no rotation         */
   double     momentum; /**< Rotation momentum: rotation done per second (NOT YET SUPPORTED) */
};

/**
 * @typedef Elm_Gesture_Rotate_Info
 * Holds rotation info for user
 */
typedef struct _Elm_Gesture_Rotate_Info Elm_Gesture_Rotate_Info;

/**
 * @typedef Elm_Gesture_Event_Cb
 * User callback used to stream gesture info from gesture layer
 * @param data user data
 * @param event_info gesture report info
 * Returns a flag field to be applied on the causing event.
 * You should probably return EVAS_EVENT_FLAG_ON_HOLD if your widget acted
 * upon the event, in an irreversible way.
 *
 */
typedef Evas_Event_Flags (*Elm_Gesture_Event_Cb)(void *data, void *event_info);

/**
 * @since 1.8
 * This function sets the gesture layer line min length of an object
 *
 * @param obj gesture-layer.
 * @param line_min_length the length.
 *
 */
EAPI void elm_gesture_layer_line_min_length_set(Evas_Object *obj, int line_min_length);

/**
 * @since 1.8
 * This function returns the gesture layer line min length of an object
 *
 * @param obj gesture-layer.
 * @return the length.
 *
 */
EAPI int elm_gesture_layer_line_min_length_get(const Evas_Object *obj);

/**
 * @since 1.8
 * This function sets the gesture layer zoom distance tolerance of an object
 *
 * @param obj gesture-layer.
 * @param zoom_distance_tolerance zoom distance tolerance
 *
 */
EAPI void elm_gesture_layer_zoom_distance_tolerance_set(Evas_Object *obj, Evas_Coord zoom_distance_tolerance);

/**
 * @since 1.8
 * This function returns the gesture layer zoom distance tolerance of an object
 *
 * @param obj gesture-layer.
 * @return zoom distance tolerance
 *
 */
EAPI Evas_Coord elm_gesture_layer_zoom_distance_tolerance_get(const Evas_Object *obj);

/**
 * @since 1.8
 * This function sets the gesture layer line distance tolerance of an object
 *
 * @param obj gesture-layer.
 * @param line_distance_tolerance line distance tolerance
 *
 */
EAPI void elm_gesture_layer_line_distance_tolerance_set(Evas_Object *obj, Evas_Coord line_distance_tolerance);

/**
 * @since 1.8
 * This function returns the gesture layer line distance tolerance of an object
 *
 * @param obj gesture-layer.
 * @return line distance tolerance
 *
 */
EAPI Evas_Coord elm_gesture_layer_line_distance_tolerance_get(const Evas_Object *obj);

/**
 * @since 1.8
 * This function sets the gesture layer line angular tolerance of an object
 *
 * @param obj gesture-layer.
 * @param line_angular_tolerance line angular tolerance
 *
 */
EAPI void elm_gesture_layer_line_angular_tolerance_set(Evas_Object *obj, double line_angular_tolerance);

/**
 * @since 1.8
 * This function returns the gesture layer line angular tolerance of an object
 *
 * @param obj gesture-layer.
 * @return line angular tolerance
 *
 */
EAPI double elm_gesture_layer_line_angular_tolerance_get(const Evas_Object *obj);

/**
 * @since 1.8
 * This function sets the gesture layer zoom wheel factor of an object
 *
 * @param obj gesture-layer.
 * @param zoom_wheel_factor zoom wheel factor
 *
 */
EAPI void elm_gesture_layer_zoom_wheel_factor_set(Evas_Object *obj, double zoom_wheel_factor);

/**
 * @since 1.8
 * This function returns the gesture layer zoom wheel factor of an object
 *
 * @param obj gesture-layer.
 * @return zoom wheel factor
 *
 */
EAPI double elm_gesture_layer_zoom_wheel_factor_get(const Evas_Object *obj);

/**
 * @since 1.8
 * This function sets the gesture layer zoom finger factor of an object
 *
 * @param obj gesture-layer.
 * @param zoom_finger_factor zoom finger factor
 *
 */
EAPI void elm_gesture_layer_zoom_finger_factor_set(Evas_Object *obj, double zoom_finger_factor);

/**
 * @since 1.8
 * This function returns the gesture layer zoom finger factor of an object
 *
 * @param obj gesture-layer.
 * @return zoom finger factor
 *
 */
EAPI double elm_gesture_layer_zoom_finger_factor_get(const Evas_Object *obj);

/**
 * @since 1.8
 * This function sets the gesture layer rotate angular tolerance of an object
 *
 * @param obj gesture-layer.
 * @param rotate_angular_tolerance rotate angular tolerance
 *
 */
EAPI void elm_gesture_layer_rotate_angular_tolerance_set(Evas_Object *obj, double rotate_angular_tolerance);

/**
 * @since 1.8
 * This function returns the gesture layer rotate angular tolerance of an object
 *
 * @param obj gesture-layer.
 * @return rotate angular tolerance
 *
 */
EAPI double elm_gesture_layer_rotate_angular_tolerance_get(const Evas_Object *obj);

/**
 * @since 1.8
 * This function sets the gesture layer flick time limit (in ms) of an object
 *
 * @param obj gesture-layer.
 * @param flick_time_limit_ms flick time limit (in ms)
 *
 */
EAPI void elm_gesture_layer_flick_time_limit_ms_set(Evas_Object *obj, unsigned int flick_time_limit_ms);

/**
 * @since 1.8
 * This function returns the gesture layer flick time limit (in ms) of an object
 *
 * @param obj gesture-layer.
 * @return flick time limit (in ms)
 *
 */
EAPI unsigned int elm_gesture_layer_flick_time_limit_ms_get(const Evas_Object *obj);

/**
 * @since 1.8
 * This function sets the gesture layer long tap start timeout of an object
 *
 * @param obj gesture-layer.
 * @param long_tap_start_timeout long tap start timeout
 *
 */
EAPI void elm_gesture_layer_long_tap_start_timeout_set(Evas_Object *obj, double long_tap_start_timeout);

/**
 * @since 1.8
 * this function returns the gesture layer long tap start timeout of an object
 *
 * @param obj gesture-layer.
 * @return long tap start timeout
 *
 */
EAPI double elm_gesture_layer_long_tap_start_timeout_get(const Evas_Object *obj);

/**
 * @since 1.8
 * This function sets the gesture layer continues enable of an object
 *
 * @param obj gesture-layer.
 * @param continues_enable continues enable
 *
 */
EAPI void elm_gesture_layer_continues_enable_set(Evas_Object *obj, Eina_Bool continues_enable);

/**
 * @since 1.8
 * This function returns the gesture layer continues enable of an object
 *
 * @param obj gesture-layer.
 * @return continues enable
 *
 */
EAPI Eina_Bool elm_gesture_layer_continues_enable_get(const Evas_Object *obj);

/**
 * @since 1.8
 * This function sets the gesture layer double tap timeout of an object
 *
 * @param obj gesture-layer.
 * @param double_tap_timeout double tap timeout
 *
 */
EAPI void elm_gesture_layer_double_tap_timeout_set(Evas_Object *obj, double double_tap_timeout);

/**
 * @since 1.8
 * this function returns the gesture layer double tap timeout of an object
 *
 * @param obj gesture-layer.
 * @return double tap timeout
 *
 */
EAPI double elm_gesture_layer_double_tap_timeout_get(const Evas_Object *obj);

/**
 * @since 1.8
 * This function sets the gesture layer finger-size for taps
 * If not set, this size taken from elm_config.
 * Set to ZERO if you want GLayer to use system finger size value (default)
 *
 * @param obj gesture-layer.
 * @param fsize Finger size
 *
 */
EAPI void elm_gesture_layer_tap_finger_size_set(Evas_Object *obj, Evas_Coord sz);

/**
 * @since 1.8
 * This function returns the gesture layer finger-size for taps
 *
 * @param obj gesture-layer.
 * @return Finger size that is currently used by Gesture Layer for taps.
 *
 */
EAPI Evas_Coord elm_gesture_layer_tap_finger_size_get(const Evas_Object *obj);
