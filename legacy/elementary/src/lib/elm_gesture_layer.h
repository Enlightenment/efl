/**
 * @defgroup Elm_Gesture_Layer Gesture Layer
 * @ingroup Elementary
 *
 * Gesture Layer Usage:
 *
 * Use Gesture Layer to detect gestures.
 * The advantage is that you don't have to implement
 * gesture detection, just set callbacks of gesture state.
 * By using gesture layer we make standard interface.
 *
 * In order to use Gesture Layer you start with @ref elm_gesture_layer_add
 * with a parent object parameter.
 * Next 'activate' gesture layer with a @ref elm_gesture_layer_attach
 * call. Usually with same object as target (2nd parameter).
 *
 * Now you need to tell gesture layer what gestures you follow.
 * This is done with @ref elm_gesture_layer_cb_set call.
 * By setting the callback you actually saying to gesture layer:
 * I would like to know when the gesture @ref Elm_Gesture_Type
 * switches to state @ref Elm_Gesture_State.
 *
 * Next, you need to implement the actual action that follows the input
 * in your callback.
 *
 * Note that if you like to stop being reported about a gesture, just set
 * all callbacks referring this gesture to NULL.
 * (again with @ref elm_gesture_layer_cb_set)
 *
 * The information reported by gesture layer to your callback is depending
 * on @ref Elm_Gesture_Type:
 * @ref Elm_Gesture_Taps_Info is the info reported for tap gestures:
 * @ref ELM_GESTURE_N_TAPS, @ref ELM_GESTURE_N_LONG_TAPS,
 * @ref ELM_GESTURE_N_DOUBLE_TAPS, @ref ELM_GESTURE_N_TRIPLE_TAPS.
 *
 * @ref Elm_Gesture_Momentum_Info is info reported for momentum gestures:
 * @ref ELM_GESTURE_MOMENTUM.
 *
 * @ref Elm_Gesture_Line_Info is the info reported for line gestures:
 * (this also contains @ref Elm_Gesture_Momentum_Info internal structure)
 * @ref ELM_GESTURE_N_LINES, @ref ELM_GESTURE_N_FLICKS.
 * Note that we consider a flick as a line-gesture that should be completed
 * in flick-time-limit as defined in @ref Config.
 *
 * @ref Elm_Gesture_Zoom_Info is the info reported for @ref ELM_GESTURE_ZOOM gesture.
 *
 * @ref Elm_Gesture_Rotate_Info is the info reported for @ref ELM_GESTURE_ROTATE gesture.
 *
 *
 * Gesture Layer Tweaks:
 *
 * Note that line, flick, gestures can start without the need to remove fingers from surface.
 * When user fingers rests on same-spot gesture is ended and starts again when fingers moved.
 *
 * Setting glayer_continues_enable to false in @ref Config will change this behavior
 * so gesture starts when user touches (a *DOWN event) touch-surface
 * and ends when no fingers touches surface (a *UP event).
 *
 * Supported elm_object common APIs.
 * @li @ref elm_object_disabled_set
 * @li @ref elm_object_disabled_get
 *
 */

/**
 * Enum of supported gesture types.
 * @ingroup Elm_Gesture_Layer
 */
typedef enum
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
} Elm_Gesture_Type;

/**
 * @enum Elm_Gesture_State
 * Enum of gesture states.
 * @typedef Elm_Gesture_State
 * Typedef of gesture states.
 * @ingroup Elm_Gesture_Layer
 */
typedef enum
{
   ELM_GESTURE_STATE_UNDEFINED = -1, /**< Gesture not STARTed */
   ELM_GESTURE_STATE_START, /**< Gesture STARTed     */
   ELM_GESTURE_STATE_MOVE, /**< Gesture is ongoing  */
   ELM_GESTURE_STATE_END, /**< Gesture completed   */
   ELM_GESTURE_STATE_ABORT /**< Ongoing gesture was ABORTed */
} Elm_Gesture_State;

/**
 * @struct _Elm_Gesture_Taps_Info
 * Struct holds taps info for user
 * @ingroup Elm_Gesture_Layer
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
 * @ingroup Elm_Gesture_Layer
 */
typedef struct _Elm_Gesture_Taps_Info Elm_Gesture_Taps_Info;

/**
 * @struct _Elm_Gesture_Momentum_Info
 * Struct holds momentum info for user
 * x1 and y1 are not necessarily in sync
 * x1 holds x value of x direction starting point
 * and same holds for y1.
 * This is noticeable when doing V-shape movement
 * @ingroup Elm_Gesture_Layer
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
 * @ingroup Elm_Gesture_Layer
 */
typedef struct _Elm_Gesture_Momentum_Info Elm_Gesture_Momentum_Info;

/**
 * @struct _Elm_Gesture_Line_Info
 * Struct holds line info for user
 * @ingroup Elm_Gesture_Layer
 */
struct _Elm_Gesture_Line_Info   /* Report line ends, timestamps, and momentum computed      */
{Elm_Gesture_Momentum_Info momentum; /**< Line momentum info */
 double                    angle; /**< Angle (direction) of lines  */
};

/**
 * @typedef Elm_Gesture_Line_Info
 * Holds line info for user
 * @ingroup Elm_Gesture_Layer
 */
typedef struct _Elm_Gesture_Line_Info Elm_Gesture_Line_Info;

/**
 * @struct _Elm_Gesture_Zoom_Info
 * Struct holds zoom info for user
 * @ingroup Elm_Gesture_Layer
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
 * @ingroup Elm_Gesture_Layer
 */
typedef struct _Elm_Gesture_Zoom_Info Elm_Gesture_Zoom_Info;

/**
 * @struct _Elm_Gesture_Rotate_Info
 * Struct holds rotation info for user
 * @ingroup Elm_Gesture_Layer
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
 * @ingroup Elm_Gesture_Layer
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
 * @ingroup Elm_Gesture_Layer
 */
typedef Evas_Event_Flags (*Elm_Gesture_Event_Cb)(void *data, void *event_info);

/**
 * Use function to set callbacks to be notified about
 * change of state of gesture.
 * When a user registers a callback with this function
 * this means this gesture has to be tested.
 *
 * When ALL callbacks for a gesture are set to NULL
 * it means user isn't interested in gesture-state
 * and it will not be tested.
 *
 * @param obj gesture-layer.
 * @param idx The gesture you would like to track its state.
 * @param cb callback function pointer.
 * @param cb_type what event this callback tracks: START, MOVE, END, ABORT.
 * @param data user info to be sent to callback (usually, Smart Data)
 *
 * @ingroup Elm_Gesture_Layer
 */
EAPI void         elm_gesture_layer_cb_set(Evas_Object *obj, Elm_Gesture_Type idx, Elm_Gesture_State cb_type, Elm_Gesture_Event_Cb cb, void *data);

/**
 * Call this function to get repeat-events settings.
 *
 * @param obj gesture-layer.
 *
 * @return repeat events settings.
 * @see elm_gesture_layer_hold_events_set()
 * @ingroup Elm_Gesture_Layer
 */
EAPI Eina_Bool    elm_gesture_layer_hold_events_get(const Evas_Object *obj);

/**
 * This function is to make gesture-layer repeat events.
 * Set this if you like to get the raw events only if gestures were not
 * detected.
 * Clear this if you like gesture layer to forward events as testing gestures.
 *
 * @param obj gesture layer.
 * @param hold_events hold events or not.
 *
 * @ingroup Elm_Gesture_Layer
 */
EAPI void         elm_gesture_layer_hold_events_set(Evas_Object *obj, Eina_Bool hold_events);

/**
 * This function sets step-value for zoom action.
 * Set step to any positive value.
 * Cancel step setting by setting to 0
 *
 * @param obj gesture-layer.
 * @param step new zoom step value.
 *
 * @see elm_gesture_layer_zoom_step_get()
 * @ingroup Elm_Gesture_Layer
 */
EAPI void         elm_gesture_layer_zoom_step_set(Evas_Object *obj, double step);

/**
 * This function returns step-value for zoom action.
 *
 * @param obj gesture-layer.
 * @return zoom step value.
 *
 * @see elm_gesture_layer_zoom_step_set()
 * @ingroup Elm_Gesture_Layer
 */
EAPI double       elm_gesture_layer_zoom_step_get(const Evas_Object *obj);

/**
 * This function sets step-value for rotate action.
 * Set step to any positive value.
 * Cancel step setting by setting to 0
 *
 * @param obj gesture-layer.
 * @param step new rotate step value.
 *
 * @ingroup Elm_Gesture_Layer
 */
EAPI void         elm_gesture_layer_rotate_step_set(Evas_Object *obj, double step);

/**
 * This function returns step-value for rotate action.
 *
 * @param obj gesture-layer.
 * @return rotate step value.
 *
 * @ingroup Elm_Gesture_Layer
 */
EAPI double       elm_gesture_layer_rotate_step_get(const Evas_Object *obj);

/**
 * This function called to attach gesture-layer to an Evas_Object.
 * @param obj gesture-layer.
 * @param target Pointer to underlying object (AKA Target)
 *
 * @return TRUE, FALSE on success, failure.
 *
 * @ingroup Elm_Gesture_Layer
 */
EAPI Eina_Bool    elm_gesture_layer_attach(Evas_Object *obj, Evas_Object *target);

/**
 * Call this function to construct a new gesture-layer object.
 * This does not activate the gesture layer. You have to
 * call elm_gesture_layer_attach in order to 'activate' gesture-layer.
 *
 * @param parent the parent object.
 *
 * @return new gesture-layer object.
 *
 * @ingroup Elm_Gesture_Layer
 */
EAPI Evas_Object *elm_gesture_layer_add(Evas_Object *parent);
