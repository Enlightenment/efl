#ifndef _EVAS_H
# error You shall not include this header directly
#endif

/**
 * @ingroup Evas_Canvas
 *
 * @{
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
 *       @ref Ecore_Evas_Group such as @c ecore_evas_new(). See
 *       @ref Ecore.
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

typedef struct _Evas_Engine_Info /** Generic engine information. Generic info is useless */
{
   int magic; /**< Magic number */
} Evas_Engine_Info;

/**
 * @brief Applies the engine settings for the given evas from the given
 * @c Evas_Engine_Info structure.
 *
 * To get the Evas_Engine_Info structure to use, call
 * @ref evas_engine_info_get. Do not try to obtain a pointer to an
 * @c Evas_Engine_Info structure in any other way.
 *
 * You will need to call this function at least once before you can create
 * objects on an evas or render that evas. Some engines allow their settings to
 * be changed more than once.
 *
 * Once called, the @c info pointer should be considered invalid.
 *
 * @param[in] info The pointer to the engine info to use.
 *
 * @return @c true if no error occurred, @c false otherwise.
 *
 * @ingroup Evas_Canvas
 */
EAPI Eina_Bool evas_engine_info_set(Evas *obj, Evas_Engine_Info *info);

/**
 * @brief Retrieves the current render engine info struct from the given evas.
 *
 * The returned structure is publicly modifiable.  The contents are valid until
 * either @ref evas_engine_info_set or @ref evas_render are called.
 *
 * This structure does not need to be freed by the caller.
 *
 * @return The pointer to the engine info to use.
 *
 * @ingroup Evas_Canvas
 */
EAPI Evas_Engine_Info *evas_engine_info_get(const Evas *obj);

/**
 * @brief Get the maximum image size evas can possibly handle.
 *
 * This function returns the largest image or surface size that evas can handle
 * in pixels, and if there is one, returns @c true. It returns @c false if no
 * extra constraint on maximum image size exists. You still should check the
 * return values of @c maxw and @c maxh as there may still be a limit, just a
 * much higher one.
 *
 * @param[in] obj The object.
 * @param[out] max The maximum image size (in pixels).
 *
 * @return @c true on success, @c false otherwise
 */
EAPI Eina_Bool evas_image_max_size_get(Eo *eo_e, int *w, int *h);


#include "canvas/evas_canvas_eo.legacy.h"

/**
 * @}
 */

/** @addtogroup Evas_Keys
 * @{
 */

/**
 * An opaque type containing information on which lock keys are registered in
 * an Evas canvas.
 */
typedef struct _Evas_Lock Evas_Lock;

/**
 * An opaque type containing information on which modifier keys are registered
 * in an Evas canvas.
 */
typedef struct _Evas_Modifier Evas_Modifier;

/**
 * A bitmask of modifier keys.
 *
 * See evas_key_modifier_mask_get() for the keyname to bit conversion.
 */
typedef unsigned long long Evas_Modifier_Mask;

/**
 * @}
 */


/**
 * @ingroup Evas_Canvas_Events
 *
 * @{
 */

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
typedef struct _Evas_Event_Axis_Update   Evas_Event_Axis_Update; /**< Event structure for #EVAS_CALLBACK_AXIS_UPDATE event callbacks @since 1.13 */

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
   Evas_Object      *event_src; /**< The Evas Object that actually triggered the event, used in cases of proxy event propagation */
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
   Evas_Object     *event_src; /**< The Evas Object that actually triggered the event, used in cases of proxy event propagation */
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
   Evas_Object     *event_src; /**< The Evas Object that actually triggered the event, used in cases of proxy event propagation */
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
   Evas_Object     *event_src; /**< The Evas Object that actually triggered the event, used in cases of proxy event propagation */
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
   Evas_Object     *event_src; /**< The Evas Object that actually triggered the event, used in cases of proxy event propagation */
};

struct _Evas_Event_Mouse_Wheel /** Wheel event */
{
   int              direction; /**< Axis of screen wheel - 0 = default up/down wheel, 1 = horizontal left/right wheel */
   int              z; /**< The step for the wheel movement relative to the direction above...,-2,-1 = down, 1,2,... = up */

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

   unsigned int     keycode; /**< Key scan code numeric value @since 1.10 */
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

   unsigned int     keycode; /**< Key scan code numeric value @since 1.10 */
};

struct _Evas_Event_Hold /** Hold change event */
{
   int              hold; /**< The hold flag */
   void            *data;

   unsigned int     timestamp;
   Evas_Event_Flags event_flags;
   Evas_Device     *dev;
};

typedef enum _Evas_Axis_Label
{
   EVAS_AXIS_LABEL_UNKNOWN,       /**< Axis containing unknown (or not yet representable) data. Range: Unbounded. Unit: Undefined. @since 1.13 */
   EVAS_AXIS_LABEL_X,             /**< Position along physical X axis; not window relative. Range: Unbounded. Unit: Undefined. @since 1.13 */
   EVAS_AXIS_LABEL_Y,             /**< Position along physical Y axis; not window relative. Range: Unbounded. Unit: Undefined. @since 1.13 */
   EVAS_AXIS_LABEL_PRESSURE,      /**< Force applied to tool tip. Range: [0.0, 1.0]. Unit: Unitless. @since 1.13 */
   EVAS_AXIS_LABEL_DISTANCE,      /**< Relative distance along physical Z axis. Range: [0.0, 1.0]. Unit: Unitless @since 1.13 */
   EVAS_AXIS_LABEL_AZIMUTH,       /**< Angle of tool about the Z axis from positive X axis. Range: [-PI, PI]. Unit: Radians. @since 1.13 */
   EVAS_AXIS_LABEL_TILT,          /**< Angle of tool about plane of sensor from positive Z axis. Range: [0.0, PI]. Unit: Radians. @since 1.13 */
   EVAS_AXIS_LABEL_TWIST,         /**< Rotation of tool about its major axis from its "natural" position. Range: [-PI, PI] Unit: Radians. @since 1.13 */
   EVAS_AXIS_LABEL_TOUCH_WIDTH_MAJOR,   /**< Length of contact ellipse along AZIMUTH. Range: Unbounded: Unit: Same as EVAS_AXIS_LABEL_{X,Y}. @since 1.13 */
   EVAS_AXIS_LABEL_TOUCH_WIDTH_MINOR,   /**< Length of contact ellipse perpendicular to AZIMUTH. Range: Unbounded. Unit: Same as EVAS_AXIS_LABEL_{X,Y}. @since 1.13 */
   EVAS_AXIS_LABEL_TOOL_WIDTH_MAJOR,    /**< Length of tool ellipse along AZIMUTH. Range: Unbounded. Unit: Same as EVAS_AXIS_LABEL_{X,Y}. @since 1.13 */
   EVAS_AXIS_LABEL_TOOL_WIDTH_MINOR,    /**< Length of tool ellipse perpendicular to AZIMUTH. Range: Unbounded. Unit: Same as EVAS_AXIS_LABEL_{X,Y}. @since 1.13 */
   EVAS_AXIS_LABEL_WINDOW_X,      /**< X coordinate mapped to the window. @since 1.19 */
   EVAS_AXIS_LABEL_WINDOW_Y,      /**< Y coordinate mapped to the window. @since 1.19 */
   EVAS_AXIS_LABEL_NORMAL_X,      /**< X normalized to the [0, 1] range. @since 1.19 */
   EVAS_AXIS_LABEL_NORMAL_Y,      /**< Y normalized to the [0, 1] range. @since 1.19 */
} Evas_Axis_Label; /**< Types of recognized device axes @since 1.13 */

typedef struct _Evas_Axis
{
   Evas_Axis_Label label;
   double value;
} Evas_Axis;

struct _Evas_Event_Axis_Update
{
   void             *data;

   unsigned int timestamp;
   int device;
   int toolid;

   int naxis;
   Evas_Axis *axis;
   Evas_Device *dev;
};

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
 * unwound from most recently to least recently pushed item and removed from the
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
 * This function should only be called from inside an evas input event
 * callback. The event_info data may be kept up until @p func is called, in
 * order to check the state of the "on-hold" flag for instance. Do not modify
 * the canvas or otherwise trigger or feed a events to the canvas from inside
 * @p func. Use jobs to safely modify the canvas.
 *
 * @warning Only use this function if you know exactly what you are doing!
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
 * @}
 */

/**
 * @ingroup Evas_Event_Feeding_Group
 *
 * @{
 */

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
 * @brief Mouse move event feed.
 *
 * This function will set some evas properties that are necessary when the mouse
 * is moved from its last position. It prepares information to be treated by
 * the callback function.
 *
 * @param[in] y The vertical position of the mouse pointer.
 * @param[in] timestamp The timestamp of the mouse up event.
 * @param[in] data The data for canvas.
 */
EAPI void             evas_event_feed_mouse_move(Evas *obj, int x, int y, unsigned int timestamp, const void *data);

/**
 * @brief Mouse move event feed from input.
 *
 * Similar to the @ref evas_event_feed_mouse_move, this function will inform
 * Evas about mouse move events which were received by the input system,
 * relative to the 0,0 of the window, not to the canvas 0,0. It will take care
 * of doing any special transformation like adding the framespace offset to the
 * mouse event.
 *
 * @param[in] y The vertical position of the mouse pointer relative to the 0,0
 * of the window/surface.
 * @param[in] timestamp The timestamp of the mouse move event.
 * @param[in] data The data for canvas.
 *
 * @since 1.8
 */
EAPI void             evas_event_input_mouse_move(Evas *obj, int x, int y, unsigned int timestamp, const void *data);

/**
 * @brief Mouse up event feed.
 *
 * This function will set some evas properties that are necessary when the mouse
 * button is released. It prepares information to be treated by the callback
 * function.
 *
 * @param[in] flags Evas button flags.
 * @param[in] timestamp The timestamp of the mouse up event.
 * @param[in] data The data for canvas.
 */
EAPI void             evas_event_feed_mouse_up(Evas *obj, int b, Evas_Button_Flags flags, unsigned int timestamp, const void *data);

/**
 * @brief Mouse down event feed.
 *
 * This function will set some evas properties that are necessary when the mouse
 * button is pressed. It prepares information to be treated by the callback
 * function.
 *
 * @param[in] flags Evas button flags.
 * @param[in] timestamp The timestamp of the mouse up event.
 * @param[in] data The data for canvas.
 */
EAPI void             evas_event_feed_mouse_down(Evas *obj, int b, Evas_Button_Flags flags, unsigned int timestamp, const void *data);

/**
 * @brief Mouse wheel event feed.
 *
 * This function will set some evas properties that are necessary when the mouse
 * wheel is scrolled up or down. It prepares information to  be treated by the
 * callback function.
 *
 * @param[in] z How much mouse wheel was scrolled up or down.
 * @param[in] timestamp The timestamp of the mouse up event.
 * @param[in] data The data for canvas.
 */
EAPI void             evas_event_feed_mouse_wheel(Evas *obj, int direction, int z, unsigned int timestamp, const void *data);

/**
 * @brief Mouse in event feed.
 *
 * This function will set some evas properties that are necessary when the mouse
 * in event happens. It prepares information to be treated by the callback
 * function.
 *
 * @param[in] data The data for canvas.
 */
EAPI void             evas_event_feed_mouse_in(Evas *obj, unsigned int timestamp, const void *data);

/**
 * @brief Mouse out event feed.
 *
 * This function will set some evas properties that are necessary when the mouse
 * out event happens. It prepares information to be treated by the callback
 * function.
 *
 * @param[in] data The data for canvas.
 */
EAPI void             evas_event_feed_mouse_out(Evas *obj, unsigned int timestamp, const void *data);

/**
 * @brief Mouse cancel event feed.
 *
 * This function will call generate a mouse up event.
 *
 * @param[in] data The data for canvas.
 */
EAPI void             evas_event_feed_mouse_cancel(Evas *obj, unsigned int timestamp, const void *data);

/* multi touch events - no doc */
EAPI void             evas_event_input_multi_down(Evas *obj, int d, int x, int y, double rad, double radx, double rady, double pres, double ang, double fx, double fy, Evas_Button_Flags flags, unsigned int timestamp, const void *data);
EAPI void             evas_event_input_multi_move(Evas *obj, int d, int x, int y, double rad, double radx, double rady, double pres, double ang, double fx, double fy, unsigned int timestamp, const void *data);
EAPI void             evas_event_input_multi_up(Evas *obj, int d, int x, int y, double rad, double radx, double rady, double pres, double ang, double fx, double fy, Evas_Button_Flags flags, unsigned int timestamp, const void *data);
EAPI void             evas_event_feed_multi_down(Evas *obj, int d, int x, int y, double rad, double radx, double rady, double pres, double ang, double fx, double fy, Evas_Button_Flags flags, unsigned int timestamp, const void *data);
EAPI void             evas_event_feed_multi_move(Evas *obj, int d, int x, int y, double rad, double radx, double rady, double pres, double ang, double fx, double fy, unsigned int timestamp, const void *data);
EAPI void             evas_event_feed_multi_up(Evas *obj, int d, int x, int y, double rad, double radx, double rady, double pres, double ang, double fx, double fy, Evas_Button_Flags flags, unsigned int timestamp, const void *data);

/**
 * @brief Key down event feed.
 *
 * This function will set some evas properties that are necessary when a key is
 * pressed. It prepares information to be treated by the callback function.
 *
 * @param[in] key The key pressed.
 * @param[in] string A string.
 * @param[in] compose The compose string.
 * @param[in] timestamp Timestamp of the mouse up event.
 * @param[in] data Data for canvas.
 */
EAPI void             evas_event_feed_key_down(Evas *obj, const char *keyname, const char *key, const char *string, const char *compose, unsigned int timestamp, const void *data);

/**
 * @brief Key up event feed.
 *
 * This function will set some evas properties that are necessary when a key is
 * released. It prepares information to be treated by the callback function.
 *
 * @param[in] key The key released.
 * @param[in] string A string.
 * @param[in] compose Compose.
 * @param[in] timestamp Timestamp of the mouse up event.
 * @param[in] data Data for canvas.
 */
EAPI void             evas_event_feed_key_up(Evas *obj, const char *keyname, const char *key, const char *string, const char *compose, unsigned int timestamp, const void *data);

/**
 * @brief Key down event feed with keycode.
 *
 * This function will set some evas properties that are necessary when a key is
 * pressed. It prepares information to be treated by the callback function.
 *
 * @param[in] key The key released.
 * @param[in] string A string.
 * @param[in] compose Compose.
 * @param[in] timestamp Timestamp of the mouse up event.
 * @param[in] data Data for canvas.
 * @param[in] keycode Key scan code numeric value for canvas.
 *
 * @since 1.10
 */
EAPI void             evas_event_feed_key_down_with_keycode(Evas *obj, const char *keyname, const char *key, const char *string, const char *compose, unsigned int timestamp, const void *data, unsigned int keycode);

/**
 * @brief Key up event feed with keycode.
 *
 * This function will set some evas properties that are necessary when a key is
 * released. It prepares information to be treated by the callback function.
 *
 * @param[in] key The key released.
 * @param[in] string A string.
 * @param[in] compose Compose.
 * @param[in] timestamp Timestamp of the mouse up event.
 * @param[in] data Data for canvas.
 * @param[in] keycode Key scan code numeric value for canvas.
 *
 * @since 1.10
 */
EAPI void             evas_event_feed_key_up_with_keycode(Evas *obj, const char *keyname, const char *key, const char *string, const char *compose, unsigned int timestamp, const void *data, unsigned int keycode);

/**
 * @brief Input device axis update event feed.
 *
 * This function will set some evas properties that are necessary when an e.g.
 * stylus axis is updated. It prepares information to be treated by the
 * callback function.
 *
 * @param[in] device System-provided device identifier.
 * @param[in] toolid Type of tool currently being used.
 * @param[in] naxes Number of elements in the \p axis array.
 * @param[in] axis Array containing the current value of all updated axes.
 * @param[in] data Data for canvas.
 *
 * @since 1.13
 */
EAPI void             evas_event_feed_axis_update(Evas *obj, unsigned int timestamp, int device, int toolid, int naxes, const Evas_Axis *axis, const void *data);

/**
 * @brief Hold event feed.
 *
 * This function makes the object to stop sending events.
 *
 * @param[in] timestamp The timestamp of the mouse up event.
 * @param[in] data The data for canvas.
 */
EAPI void             evas_event_feed_hold(Evas *obj, int hold, unsigned int timestamp, const void *data);

/**
 * @brief Re feed event.
 *
 * This function re-feeds the event pointed by event_copy.
 *
 * This function call evas_event_feed_* functions, so it can cause havoc if not
 * used wisely. Please use it responsibly.
 *
 * @param[in] event_type Event type.
 */
EAPI void             evas_event_refeed_event(Evas *obj, void *event_copy, Evas_Callback_Type event_type);

/**
 * @}
 */

/**
 * @addtogroup Evas_Keys
 * @{
 */

/**
 * @brief Returns a handle to the list of modifier keys registered in the
 * canvas @c e.
 *
 * This is required to check for which modifiers are set at a given time with
 * the @ref evas_key_modifier_is_set function.
 *
 * See also @ref evas_key_modifier_add, @ref evas_key_modifier_del,
 * @ref evas_key_modifier_on, @ref evas_key_modifier_off,
 * @ref evas_seat_key_modifier_on, @ref evas_seat_key_modifier_off.
 *
 * @return An Evas_Modifier handle to query Evas' keys subsystem with @ref
 * evas_key_modifier_is_set or @ref evas_seat_key_modifier_is_set, or @c null
 * on error.
 *
 * @ingroup Evas_Canvas
 */
EAPI const Evas_Modifier *evas_key_modifier_get(const Evas *obj) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Creates a bit mask from the @c keyname modifier key. Values returned
 * from different calls to it may be ORed together, naturally.
 *
 * This function is meant to be using in conjunction with @ref
 * evas_object_key_grab/\@ref evas_object_key_ungrab. Go check their
 * documentation for more information.
 *
 * See also @ref evas_key_modifier_add, @ref evas_key_modifier_get,
 * @ref evas_key_modifier_on, @ref evas_key_modifier_off,
 * @ref evas_seat_key_modifier_on, @ref evas_seat_key_modifier_off, @ref
 * evas_key_modifier_is_set, @ref evas_seat_key_modifier_is_set..
 *
 * @param[in] keyname The name of the modifier key to create the mask for.
 *
 * @return The bit mask or 0 if the @c keyname key wasn't registered as a
 * modifier for canvas @c e.
 *
 * @ingroup Evas_Canvas
 */
EAPI Evas_Modifier_Mask evas_key_modifier_mask_get(const Evas *evas, const char *keyname) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(2);

/**
 * Checks the state of a given modifier of the default seat, at the time of the
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
 * @see evas_seat_key_modifier_is_set
 */
EAPI Eina_Bool            evas_key_modifier_is_set(const Evas_Modifier *m, const char *keyname) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1, 2);

/**
 * Checks the state of a given modifier key of a given seat, at the time of the
 * call. If the modifier is set, such as shift being pressed, this
 * function returns @c Eina_True.
 *
 * @param m The current modifiers set, as returned by
 *        evas_key_modifier_get().
 * @param keyname The name of the modifier key to check status for.
 * @param seat The seat to check if the lock is set. Use @c NULL for the default seat.
 *
 * @return @c Eina_True if the modifier key named @p keyname is on, @c
 *         Eina_False otherwise.
 *
 * @see evas_key_modifier_add
 * @see evas_key_modifier_del
 * @see evas_key_modifier_get
 * @see evas_key_modifier_on
 * @see evas_key_modifier_off
 * @see evas_seat_key_modifier_on
 * @see evas_seat_key_modifier_off
 * @see evas_key_modifier_is_set
 * @since 1.19
 */
EAPI Eina_Bool            evas_seat_key_modifier_is_set(const Evas_Modifier *m, const char *keyname, const Evas_Device *seat) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1, 2);

/**
 * Checks the state of a given lock key of the default seat, at the time of the call. If
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
 * @see evas_seat_key_lock_on
 * @see evas_seat_key_lock_off
 * @see evas_seat_key_lock_is_set
 */
EAPI Eina_Bool            evas_key_lock_is_set(const Evas_Lock *l, const char *keyname) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1, 2);

/**
 * Checks the state of a given lock key of a given seat, at the time of the call. If
 * the lock is set, such as caps lock, this function returns @c
 * Eina_True.
 *
 * @param l The current locks set, as returned by evas_key_lock_get().
 * @param keyname The name of the lock key to check status for.
 * @param seat The seat to check if the lock is set. Use @c NULL for the default seat.
 *
 * @return @c Eina_True if the @p keyname lock key is set, @c
 *        Eina_False otherwise.
 *
 * @see evas_key_lock_get
 * @see evas_key_lock_add
 * @see evas_key_lock_del
 * @see evas_key_lock_on
 * @see evas_key_lock_off
 * @see evas_key_lock_is_set
 * @since 1.19
 */
EAPI Eina_Bool            evas_seat_key_lock_is_set(const Evas_Lock *l, const char *keyname, const Evas_Device *seat) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1, 2);

/**
 * @brief Returns a handle to the list of lock keys registered in the canvas
 * @c e.
 *
 * This is required to check for which locks are set at a given time with the
 * @ref evas_key_lock_is_set function.
 *
 * @return An Evas_Lock handle to query Evas' keys subsystem with @ref
 * evas_key_lock_is_set or @ref evas_seat_key_lock_is_set, or @c null on error.
 *
 * @ingroup Evas_Canvas
 */
EAPI const Evas_Lock *evas_key_lock_get(const Evas *obj) EINA_WARN_UNUSED_RESULT;

/**
 * @}
 */

/**
 * @brief Returns whether the mouse pointer is logically inside the canvas.
 *
 * @param[in] obj The object.
 * @param[in] dev The pointer device.
 *
 * @return @c true if the pointer is inside, @c false otherwise.
 *
 * @since 1.19
 *
 * @ingroup Evas_Canvas
 */
EAPI Eina_Bool evas_pointer_inside_by_device_get(const Evas *obj, Efl_Input_Device *dev);

/**
 * @brief Returns whether the default mouse pointer is logically inside the
 * canvas.
 *
 * When this function is called it will return a value of either @c false or
 * @c true, depending on if event_feed_mouse_in or event_feed_mouse_out have
 * been called to feed in a  mouse enter event into the canvas.
 *
 * A return value of @c true indicates the mouse is logically inside the
 * canvas, and @c false implies it is logically outside the canvas.
 *
 * A canvas begins with the mouse being assumed outside ($false).
 *
 * If @c e is not a valid canvas, the return value is undefined.
 *
 * @param[in] obj The object.
 *
 * @return @c true if the mouse pointer is inside the canvas, @c false
 * otherwise
 *
 * @ingroup Evas_Canvas
 */
EAPI Eina_Bool evas_pointer_inside_get(const Evas *obj) EINA_WARN_UNUSED_RESULT;

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
 *
 * @{
 */

/**
 * State of Evas_Coord_Touch_Point
 */
typedef enum
{
  EVAS_TOUCH_POINT_DOWN = 0, /**< Touch point is pressed down */
  EVAS_TOUCH_POINT_UP, /**< Touch point is released */
  EVAS_TOUCH_POINT_MOVE, /**< Touch point is moved */
  EVAS_TOUCH_POINT_STILL, /**< Touch point is not moved after pressed */
  EVAS_TOUCH_POINT_CANCEL /**< Touch point is cancelled */
} Evas_Touch_Point_State;

/**
 * @brief Get the number of touched point in the evas.
 *
 * New touched point is added to the list whenever touching the evas and point
 * is removed whenever removing touched point from the evas.
 *
 * @return The number of touched point on the evas.
 */
EAPI unsigned int evas_touch_point_list_count(Evas *obj);

/**
 * @brief This function returns the @c id of nth touch point.
 *
 * The point which comes from Mouse Event has @c id 0 and The point which comes
 * from Multi Event has @c id that is same as Multi Event's device id.
 *
 * @param[in] n The number of the touched point (0 being the first).
 *
 * @return id of nth touch point, if the call succeeded, -1 otherwise.
 */
EAPI int evas_touch_point_list_nth_id_get(Evas *obj, unsigned int n);

/**
 * @brief This function returns the @c state of nth touch point.
 *
 * The point's @c state is EVAS_TOUCH_POINT_DOWN when pressed,
 * EVAS_TOUCH_POINT_STILL when the point is not moved after pressed,
 * EVAS_TOUCH_POINT_MOVE when moved at least once after pressed and
 * EVAS_TOUCH_POINT_UP when released.
 *
 * @param[in] n The number of the touched point (0 being the first).
 *
 * @return @c state of nth touch point, if the call succeeded,
 * EVAS_TOUCH_POINT_CANCEL otherwise.
 */
EAPI Evas_Touch_Point_State evas_touch_point_list_nth_state_get(Evas *obj, unsigned int n);

/**
 * @brief This function returns the nth touch point's coordinates.
 *
 * Touch point's coordinates is updated whenever moving that point on the
 * canvas.
 *
 * @param[in] n The number of the touched point (0 being the first).
 * @param[out] x The pointer to a Evas_Coord to be filled in.
 * @param[out] y The pointer to a Evas_Coord to be filled in.
 */
EAPI void evas_touch_point_list_nth_xy_get(Evas *eo_e, unsigned int n, Evas_Coord *x, Evas_Coord *y);

/**
 * @}
 */

/**
 * @ingroup Evas_Font_Group
 *
 * @{
 */

/**
 * Free list of font descriptions returned by evas_font_dir_available_list().
 *
 * @param e The evas instance that returned such list.
 * @param available the list returned by evas_font_dir_available_list().
 *
 * @ingroup Evas_Font_Group
 */
EAPI void                    evas_font_available_list_free(Evas *e, Eina_List *available) EINA_ARG_NONNULL(1);

/** Flags for Font Hinting
 *
 * @ingroup Evas_Font
 */
typedef enum
{
  EVAS_FONT_HINTING_NONE = 0, /**< No font hinting */
  EVAS_FONT_HINTING_AUTO, /**< Automatic font hinting */
  EVAS_FONT_HINTING_BYTECODE /**< Bytecode font hinting */
} Evas_Font_Hinting_Flags;

/**
 * @brief Changes the font hinting for the given evas.
 *
 * #EVAS_FONT_HINTING_AUTO, #EVAS_FONT_HINTING_BYTECODE.
 *
 * @param[in] hinting The used hinting, one of #EVAS_FONT_HINTING_NONE,
 * #EVAS_FONT_HINTING_AUTO, #EVAS_FONT_HINTING_BYTECODE.
 *
 * @ingroup Evas_Font_Group
 */
EAPI void evas_font_hinting_set(Evas *e, Evas_Font_Hinting_Flags hinting);

/**
 * @brief Retrieves the font hinting used by the given evas.
 *
 * @return The used hinting, one of #EVAS_FONT_HINTING_NONE,
 * #EVAS_FONT_HINTING_AUTO, #EVAS_FONT_HINTING_BYTECODE.
 *
 * @ingroup Evas_Font_Group
 */
EAPI Evas_Font_Hinting_Flags evas_font_hinting_get(const Evas *e);

/**
 * @brief Checks if the font hinting is supported by the given evas.
 *
 * One of #EVAS_FONT_HINTING_NONE, #EVAS_FONT_HINTING_AUTO,
 * #EVAS_FONT_HINTING_BYTECODE.
 *
 * @param[in] hinting The hinting to use.
 *
 * @return @c true if it is supported, @c false otherwise.
 *
 * @ingroup Evas_Canvas
 */
EAPI Eina_Bool evas_font_hinting_can_hint(const Evas *e, Evas_Font_Hinting_Flags hinting) EINA_WARN_UNUSED_RESULT;

/**
 * @}
 */

/**
 * @ingroup Evas_Object_Group_Basic
 *
 * @{
 */

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
 * @brief Retrieves the type of the given Evas object.
 *
 * For Evas' builtin types, the return strings will be one of "rectangle",
 * "line", "polygon", "text", "textblock" or "image".
 *
 * For Evas smart objects (see @ref Evas_Smart_Group), the name of the smart
 * class itself is returned on this call. For the built-in smart objects, these
 * names are "EvasObjectSmartClipped" for the clipped smart object,
 * "Evas_Object_Box" for the box object and "Evas_Object_Table for the table
 * object.
 *
 * @return The type of the object.
 *
 * @ingroup Evas_Object_Group_Basic
 * @since 1.18
 */
EAPI const char      *evas_object_type_get(const Evas_Object *obj);

/**
 * @brief Sets the name of the given Evas object to the given name.
 *
 * There might be occasions where one would like to name his/her objects.
 *
 * @param[in] name The given name.
 *
 * @ingroup Evas_Object_Group_Basic
 */
EAPI void evas_object_name_set(Evas_Object *obj, const char *name);

/**
 * @brief Retrieves the name of the given Evas object.
 *
 * Return: The name of the object or @c null, if no name has been given to it.
 *
 * @return The given name.
 *
 * @ingroup Evas_Object_Group_Basic
 */
EAPI const char *evas_object_name_get(const Evas_Object *obj);

/**
 * @brief Retrieves the object from children of the given object with the given
 * name.
 *
 * This looks for the evas object given a name by @ref evas_object_name_set,
 * but it ONLY looks at the children of the object *p obj, and will only
 * recurse into those children if @c recurse is greater than 0. If the name is
 * not unique within immediate children (or the whole child tree) then it is
 * not defined which child object will be returned. If @c recurse is set to -1
 * then it will recurse without limit.
 *
 * @param[in] name The given name.
 * @param[in] recurse Set to the number of child levels to recurse (0 == don't
 * recurse, 1 == only look at the children of @c obj or their immediate
 * children, but no further etc.).
 *
 * @return The Evas object with the given name on success, Otherwise @c null.
 *
 * @since 1.2
 *
 * @ingroup Evas_Object_Group_Basic
 */
EAPI Evas_Object *evas_object_name_child_find(const Evas_Object *obj, const char *name, int recurse) EINA_WARN_UNUSED_RESULT;

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
 * Set the position and (rectangular) size of the given Evas object.
 *
 * @param obj The given Evas object.
 * @param x   X position to move the object to, in canvas units.
 * @param y   Y position to move the object to, in canvas units.
 * @param w   The new width of the Evas object.
 * @param h   The new height of the Evas object.
 *
 * The position, naturally, will be relative to the top left corner of
 * the canvas' viewport.
 *
 * If the object get moved, the object's ::EVAS_CALLBACK_MOVE callback
 * will be called.
 *
 * If the object get resized, the object's ::EVAS_CALLBACK_RESIZE callback
 * will be called.
 *
 * @see evas_object_move()
 * @see evas_object_resize()
 * @see evas_object_geometry_get
 *
 * @since 1.8
 * @ingroup Evas_Object_Group_Basic
 */
EAPI void             evas_object_geometry_set(Evas_Object *obj, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h) EINA_ARG_NONNULL(1);


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
 *
 * Sets the general/main color of the given Evas object to the given
 * one.
 *
 * @see evas_object_color_get() (for an example)
 * @note These color values are expected to be premultiplied by @p a.
 *
 * @ingroup Evas_Object_Group_Basic
 *
 * @param[in] r The red component of the given color.
 * @param[in] g The green component of the given color.
 * @param[in] b The blue component of the given color.
 * @param[in] a The alpha component of the given color.
 */
EAPI void evas_object_color_set(Evas_Object *obj, int r, int g, int b, int a);

/**
 *
 * Retrieves the general/main color of the given Evas object.
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
 *
 * @param[out] r The red component of the given color.
 * @param[out] g The green component of the given color.
 * @param[out] b The blue component of the given color.
 * @param[out] a The alpha component of the given color.
 */
EAPI void evas_object_color_get(const Evas_Object *obj, int *r, int *g, int *b, int *a);

/**
 *
 * Move the given Evas object to the given location inside its canvas' viewport.
 *
 * @param[in] x in
 * @param[in] y in
 */
EAPI void evas_object_move(Evas_Object *obj, Evas_Coord x, Evas_Coord y);

/**
 *
 * Changes the size of the given Evas object.
 *
 * @param[in] w in
 * @param[in] h in
 */
EAPI void evas_object_resize(Evas_Object *obj, Evas_Coord w, Evas_Coord h);

/**
 *
 * Retrieves whether or not the given Evas object is visible.
 *
 */
EAPI Eina_Bool evas_object_visible_get(const Evas_Object *obj);

/**
 * @brief Sets the hints for an object's maximum size.
 *
 * This is not a size enforcement in any way, it's just a hint that should be
 * used whenever appropriate.
 *
 * Values -1 will be treated as unset hint components, when queried by
 * managers.
 *
 * @note Smart objects (such as elementary) can have their own size hint
 * policy. So calling this API may or may not affect the size of smart objects.
 *
 * @param[in] w Integer to use as the maximum width hint.
 * @param[in] h Integer to use as the maximum height hint.
 *
 * @ingroup Evas_Object
 */
EAPI void evas_object_size_hint_max_set(Evas_Object *obj, Evas_Coord w, Evas_Coord h);

/**
 * @brief Retrieves the hints for an object's maximum size.
 *
 * These are hints on the maximum sizes @c obj should have. This is not a size
 * enforcement in any way, it's just a hint that should be used whenever
 * appropriate.
 *
 * @note Use @c null pointers on the hint components you're not interested in:
 * they'll be ignored by the function.
 *
 * @param[out] w Integer to use as the maximum width hint.
 * @param[out] h Integer to use as the maximum height hint.
 *
 * @ingroup Evas_Object
 */
EAPI void evas_object_size_hint_max_get(const Evas_Object *obj, Evas_Coord *w, Evas_Coord *h);

/**
 * @brief Sets the hints for an object's optimum size.
 *
 * This is not a size enforcement in any way, it's just a hint that should be
 * used whenever appropriate.
 *
 * Values 0 will be treated as unset hint components, when queried by managers.
 *
 * @note Smart objects(such as elementary) can have their own size hint policy.
 * So calling this API may or may not affect the size of smart objects.
 *
 * @param[in] w Integer to use as the preferred width hint.
 * @param[in] h Integer to use as the preferred height hint.
 *
 * @ingroup Evas_Object
 */
EAPI void evas_object_size_hint_request_set(Evas_Object *obj, Evas_Coord w, Evas_Coord h);

/**
 * @brief Retrieves the hints for an object's optimum size.
 *
 * These are hints on the optimum sizes @c obj should have. This is not a size
 * enforcement in any way, it's just a hint that should be used whenever
 * appropriate.
 *
 * @note Use @c null pointers on the hint components you're not interested in:
 * they'll be ignored by the function.
 *
 * @param[out] w Integer to use as the preferred width hint.
 * @param[out] h Integer to use as the preferred height hint.
 *
 * @ingroup Evas_Object
 */
EAPI void evas_object_size_hint_request_get(const Evas_Object *obj, Evas_Coord *w, Evas_Coord *h);

/**
 * @brief Sets the hints for an object's minimum size.
 *
 * This is not a size enforcement in any way, it's just a hint that should be
 * used whenever appropriate.
 *
 * Values 0 will be treated as unset hint components, when queried by managers.
 *
 * @note Smart objects(such as elementary) can have their own size hint policy.
 * So calling this API may or may not affect the size of smart objects.
 *
 * @param[in] w Integer to use as the minimum width hint.
 * @param[in] h Integer to use as the minimum height hint.
 *
 * @ingroup Evas_Object
 */
EAPI void evas_object_size_hint_min_set(Evas_Object *obj, Evas_Coord w, Evas_Coord h);

/**
 * @brief Disable/cease clipping on a clipped @c obj object.
 *
 * This function disables clipping for the object @c obj, if it was already
 * clipped, i.e., its visibility and color get detached from the previous
 * clipper. If it wasn't, this has no effect. The object @c obj must be a valid
 * Evas_Object.
 *
 * See also @ref evas_object_clip_set, @ref evas_object_clipees_get and
 * @ref evas_object_clip_get.
 *
 * @ingroup Evas_Object
 */
EAPI void evas_object_clip_unset(Evas_Object *obj);

/**
 * @brief Retrieves the hints for an object's minimum size.
 *
 * These are hints on the minimum sizes @c obj should have. This is not a size
 * enforcement in any way, it's just a hint that should be used whenever
 * appropriate.
 *
 * @note Use @c null pointers on the hint components you're not interested in:
 * they'll be ignored by the function.
 *
 * @param[out] w Integer to use as the minimum width hint.
 * @param[out] h Integer to use as the minimum height hint.
 *
 * @ingroup Evas_Object
 */
EAPI void evas_object_size_hint_min_get(const Evas_Object *obj, Evas_Coord *w, Evas_Coord *h);

/**
 * @brief Sets the hints for an object's padding space.
 *
 * This is not a size enforcement in any way, it's just a hint that should be
 * used whenever appropriate.
 *
 * @note Smart objects(such as elementary) can have their own size hint policy.
 * So calling this API may or may not affect the size of smart objects.
 *
 * @param[in] l Integer to specify left padding.
 * @param[in] r Integer to specify right padding.
 * @param[in] t Integer to specify top padding.
 * @param[in] b Integer to specify bottom padding.
 *
 * @ingroup Evas_Object
 */
EAPI void evas_object_size_hint_padding_set(Evas_Object *obj, Evas_Coord l, Evas_Coord r, Evas_Coord t, Evas_Coord b);

/**
 * @brief Retrieves the hints for an object's padding space.
 *
 * Padding is extra space an object takes on each of its delimiting rectangle
 * sides, in canvas units.
 *
 * This is not a size enforcement in any way, it's just a hint that should be
 * used whenever appropriate.
 *
 * @note Use @c null pointers on the hint components you're not interested in:
 * they'll be ignored by the function.
 *
 * @param[out] l Integer to specify left padding.
 * @param[out] r Integer to specify right padding.
 * @param[out] t Integer to specify top padding.
 * @param[out] b Integer to specify bottom padding.
 *
 * @ingroup Evas_Object
 */
EAPI void evas_object_size_hint_padding_get(const Evas_Object *obj, Evas_Coord *l, Evas_Coord *r, Evas_Coord *t, Evas_Coord *b);

/**
 * @brief Sets the hints for an object's weight.
 *
 * This is not a size enforcement in any way, it's just a hint that should be
 * used whenever appropriate.
 *
 * This is a hint on how a container object should resize a given child within
 * its area. Containers may adhere to the simpler logic of just expanding the
 * child object's dimensions to fit its own (see the #EVAS_HINT_EXPAND helper
 * weight macro) or the complete one of taking each child's weight hint as real
 * weights to how much of its size to allocate for them in each axis. A
 * container is supposed to, after normalizing the weights of its children
 * (with weight  hints), distribut the space it has to layout them by those
 * factors -- most weighted children get larger in this process than the least
 * ones.
 *
 * @note Default weight hint values are 0.0, for both axis.
 *
 * @param[in] x Non-negative double value to use as horizontal weight hint.
 * @param[in] y Non-negative double value to use as vertical weight hint.
 *
 * @ingroup Evas_Object
 */
EAPI void evas_object_size_hint_weight_set(Evas_Object *obj, double x, double y);

/**
 * @brief Retrieves the hints for an object's weight.
 *
 * Accepted values are zero or positive values. Some users might use this hint
 * as a boolean, but some might consider it as a proportion, see documentation
 * of possible users, which in Evas are the @ref Evas_Object_Box "box" and @ref
 * Evas_Object_Table "table" smart objects.
 *
 * This is not a size enforcement in any way, it's just a hint that should be
 * used whenever appropriate.
 *
 * @note Use @c null pointers on the hint components you're not interested in:
 * they'll be ignored by the function.
 *
 * @note If @c obj is invalid, then the hint components will be set with 0.0.
 *
 * @param[out] x Non-negative double value to use as horizontal weight hint.
 * @param[out] y Non-negative double value to use as vertical weight hint.
 *
 * @ingroup Evas_Object
 */
EAPI void evas_object_size_hint_weight_get(const Evas_Object *obj, double *x, double *y);

/**
 * @brief Sets the hints for an object's alignment.
 *
 * These are hints on how to align an object inside the boundaries of a
 * container/manager. Accepted values are in the 0.0 to 1.0 range, with the
 * special value #EVAS_HINT_FILL used to specify "justify" or "fill" by some
 * users. In this case, maximum size hints should be enforced with higher
 * priority, if they are set. Also, any padding hint set on objects should add
 * up to the alignment space on the final scene composition.
 *
 * See documentation of possible users: in Evas, they are the @ref
 * Evas_Object_Box "box" and @ref Evas_Object_Table "table" smart objects.
 *
 * For the horizontal component, 0.0 means to the left, 1.0 means to the right.
 * Analogously, for the vertical component, 0.0 to the top, 1.0 means to the
 * bottom.
 *
 * This is not a size enforcement in any way, it's just a hint that should be
 * used whenever appropriate.
 *
 * @note Default alignment hint values are 0.5, for both axis.
 *
 * @param[in] x Double, ranging from 0.0 to 1.0 or with the special value
 * #EVAS_HINT_FILL, to use as horizontal alignment hint.
 * @param[in] y Double, ranging from 0.0 to 1.0 or with the special value
 * #EVAS_HINT_FILL, to use as vertical alignment hint.
 *
 * @ingroup Evas_Object
 */
EAPI void evas_object_size_hint_align_set(Evas_Object *obj, double x, double y);

/**
 * @brief Retrieves the hints for on object's alignment.
 *
 * This is not a size enforcement in any way, it's just a hint that should be
 * used whenever appropriate.
 *
 * @note Use @c null pointers on the hint components you're not interested in:
 * they'll be ignored by the function.
 *
 * @note If @c obj is invalid, then the hint components will be set with 0.5
 *
 * @param[out] x Double, ranging from 0.0 to 1.0 or with the special value
 * #EVAS_HINT_FILL, to use as horizontal alignment hint.
 * @param[out] y Double, ranging from 0.0 to 1.0 or with the special value
 * #EVAS_HINT_FILL, to use as vertical alignment hint.
 *
 * @ingroup Evas_Object
 */
EAPI void evas_object_size_hint_align_get(const Evas_Object *obj, double *x, double *y);

/**
 * @brief Sets the hints for an object's aspect ratio.
 *
 * This is not a size enforcement in any way, it's just a hint that should be
 * used whenever appropriate.
 *
 * If any of the given aspect ratio terms are 0, the object's container will
 * ignore the aspect and scale @c obj to occupy the whole available area, for
 * any given policy.
 *
 * @note Smart objects(such as elementary) can have their own size hint policy.
 * So calling this API may or may not affect the size of smart objects.
 *
 * @param[in] aspect The policy/type of aspect ratio to apply to @c obj.
 * @param[in] w Integer to use as aspect width ratio term.
 * @param[in] h Integer to use as aspect height ratio term.
 *
 * @ingroup Evas_Object
 */
EAPI void evas_object_size_hint_aspect_set(Evas_Object *obj, Evas_Aspect_Control aspect, Evas_Coord w, Evas_Coord h);

/**
 * @brief Retrieves the hints for an object's aspect ratio.
 *
 * The different aspect ratio policies are documented in the
 * #Evas_Aspect_Control type. A container respecting these size hints would
 * resize its children accordingly to those policies.
 *
 * For any policy, if any of the given aspect ratio terms are 0, the object's
 * container should ignore the aspect and scale @c obj to occupy the whole
 * available area. If they are both positive integers, that proportion will be
 * respected, under each scaling policy.
 *
 * @note Use @c null pointers on the hint components you're not interested in:
 * they'll be ignored by the function.
 *
 * @param[out] aspect The policy/type of aspect ratio to apply to @c obj.
 * @param[out] w Integer to use as aspect width ratio term.
 * @param[out] h Integer to use as aspect height ratio term.
 *
 * @ingroup Evas_Object
 */
EAPI void evas_object_size_hint_aspect_get(const Evas_Object *obj, Evas_Aspect_Control *aspect, Evas_Coord *w, Evas_Coord *h);

/** Display mode size hint. */
typedef enum
{
  EFL_GFX_HINT_MODE_NONE = 0, /**< Default mode */
  EFL_GFX_HINT_MODE_COMPRESS = 1, /**< Use this mode when you want to give
                                        * compress display mode hint to an object */
  EFL_GFX_HINT_MODE_EXPAND = 2, /**< Use this mode when you want to give
                                      * expand display mode hint to an object */
  EFL_GFX_HINT_MODE_DONT_CHANGE = 3 /**< Use this mode when an object
                                          * should not change its display mode */
} Efl_Gfx_Hint_Mode;

typedef Efl_Gfx_Hint_Mode             Evas_Display_Mode;

#define EVAS_DISPLAY_MODE_NONE             EFL_GFX_HINT_MODE_NONE
#define EVAS_DISPLAY_MODE_COMPRESS         EFL_GFX_HINT_MODE_COMPRESS
#define EVAS_DISPLAY_MODE_EXPAND           EFL_GFX_HINT_MODE_EXPAND
#define EVAS_DISPLAY_MODE_DONT_CHANGE      EFL_GFX_HINT_MODE_DONT_CHANGE

/**
 * @brief Sets the hints for an object's disply mode,
 *
 * This is not a size enforcement in any way, it's just a hint that can be used
 * whenever appropriate.
 *
 * @param[in] dispmode Display mode hint.
 *
 * @ingroup Evas_Object
 */
EAPI void evas_object_size_hint_display_mode_set(Evas_Object *obj, Evas_Display_Mode dispmode);

/**
 * @brief Retrieves the hints for an object's display mode
 *
 * These are hints on the display mode @c obj. This is not a size enforcement
 * in any way, it's just a hint that can be used whenever appropriate. This
 * mode can be used object's display mode like commpress or expand.
 *
 * @return Display mode hint.
 *
 * @ingroup Evas_Objects
 */
EAPI Evas_Display_Mode evas_object_size_hint_display_mode_get(const Evas_Object *obj);

/**
 *
 * Sets the layer of its canvas that the given object will be part of.
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
 *
 * @param[in] l The number of the layer to place the object on.
Must be between #EVAS_LAYER_MIN and #EVAS_LAYER_MAX.
 */
EAPI void evas_object_layer_set(Evas_Object *obj, short l);

/**
 *
 * Retrieves the layer of its canvas that the given object is part of.
 *
 * @return  Number of its layer
 *
 * @see evas_object_layer_set()
 *
 */
EAPI short evas_object_layer_get(const Evas_Object *obj);

/**
 *
 * Get the Evas object stacked right below @p obj
 *
 * @return the #Evas_Object directly below @p obj, if any, or @c NULL,
 * if none
 *
 * This function will traverse layers in its search, if there are
 * objects on layers below the one @p obj is placed at.
 *
 * @see evas_object_layer_get()
 * @see evas_object_layer_set()
 * @see evas_object_below_get()
 *
 */
EAPI Evas_Object *evas_object_below_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT;

/**
 *
 * Get the Evas object stacked right above @p obj
 *
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
EAPI Evas_Object *evas_object_above_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT;

/**
 *
 * Stack @p obj immediately below @p below
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
 *
 *
 * @param[in] below the object below which to stack
 */
EAPI void evas_object_stack_below(Evas_Object *obj, Evas_Object *below) EINA_ARG_NONNULL(2);

/**
 *
 * Raise @p obj to the top of its layer.
 *
 * @p obj will, then, be the highest one in the layer it belongs
 * to. Object on other layers won't get touched.
 *
 * @see evas_object_stack_above()
 * @see evas_object_stack_below()
 * @see evas_object_lower()
 *
 *
 */
EAPI void evas_object_raise(Evas_Object *obj);

/**
 *
 * Stack @p obj immediately above @p above
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
 *
 *
 * @param[in] above the object above which to stack
 */
EAPI void evas_object_stack_above(Evas_Object *obj, Evas_Object *above) EINA_ARG_NONNULL(2);

/**
 *
 * Lower @p obj to the bottom of its layer.
 *
 * @p obj will, then, be the lowest one in the layer it belongs
 * to. Objects on other layers won't get touched.
 *
 * @see evas_object_stack_above()
 * @see evas_object_stack_below()
 * @see evas_object_raise()
 *
 *
 */
EAPI void evas_object_lower(Evas_Object *obj);

/**
 * @brief Set a hint flag on the given Evas object that it's used as a "static
 * clipper".
 *
 * This is a hint to Evas that this object is used as a big static clipper and
 * shouldn't be moved with children and otherwise considered specially. The
 * default value for new objects is @c false.
 *
 * @param[in] is_static_clip @c true if it's to be used as a static clipper,
 * @c false otherwise.
 *
 * @ingroup Evas_Object
 */
EAPI void evas_object_static_clip_set(Evas_Object *obj, Eina_Bool is_static_clip);

/**
 * @brief Return a list of objects currently clipped by @c obj.
 *
 * This returns the internal list handle that contains all objects clipped by
 * the object @c obj. If none are clipped by it, the call returns @c null. This
 * list is only valid until the clip list is changed and should be fetched
 * again with another call to this function if any objects being clipped by
 * this object are unclipped, clipped by a new object, deleted or get the
 * clipper deleted. These operations will invalidate the list returned, so it
 * should not be used anymore after that point. Any use of the list after this
 * may have undefined results, possibly leading to crashes. The object @c obj
 * must be a valid Evas_Object.
 *
 * See also @ref evas_object_clip_set, @ref evas_object_clip_unset and
 * @ref evas_object_clip_get.
 *
 * @return A list of objects being clipped by @c obj.
 *
 * @ingroup Evas_Object
 */
EAPI const Eina_List *evas_object_clipees_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Test if any object is clipped by @c obj.
 *
 * @param[in] obj The object.
 *
 * @return @c true if any object is clipped by @c obj, @c false otherwise
 *
 * @since 1.8
 *
 * @ingroup Evas_Object
 */
EAPI Eina_Bool evas_object_clipees_has(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT;

/** How the object should be rendered to output.
 *
 * @ingroup Evas
 */
typedef enum
{
  EVAS_RENDER_BLEND = 0, /** Default render operation: d = d*(1-sa) + s. The
                          * object will be merged onto the bottom objects using
                          * simple alpha compositing (a over b). */
  EVAS_RENDER_BLEND_REL = 1, /** DEPRECATED. d = d*(1 - sa) + s*da */
  EVAS_RENDER_COPY = 2, /** Copy mode, d = s. The object's pixels will replace
                         * everything that was below, effectively hiding them.
                         */
  EVAS_RENDER_COPY_REL = 3, /** DEPRECATED. d = s*da */
  EVAS_RENDER_ADD = 4, /** DEPRECATED. d = d + s */
  EVAS_RENDER_ADD_REL = 5, /** DEPRECATED. d = d + s*da */
  EVAS_RENDER_SUB = 6, /** DEPRECATED. d = d - s */
  EVAS_RENDER_SUB_REL = 7, /** DEPRECATED. d = d - s*da */
  EVAS_RENDER_TINT = 8, /** DEPRECATED. d = d*s + d*(1 - sa) + s*(1 - da) */
  EVAS_RENDER_TINT_REL = 9, /** DEPRECATED. d = d*(1 - sa + s) */
  EVAS_RENDER_MASK = 10, /** DEPRECATED. d = d*sa. For masking support, please
                          * use Efl.Canvas.Object.clip_set or EDC "clip_to" instead.
                          */
  EVAS_RENDER_MUL = 11 /** DEPRECATED. d = d*s */
} Evas_Render_Op;

/**
 * @brief Sets the render mode to be used for compositing the Evas object.
 *
 * Note that only copy and blend modes are actually supported: -
 * @ref Evas_Render_Op.EVAS_RENDER_BLEND means the object will be merged on top
 * of objects below it using simple alpha compositing. -
 * @ref Evas_Render_Op.EVAS_RENDER_COPY means this object's pixels will replace
 * everything that is below, making this object opaque.
 *
 * Please do not assume that @ref Evas_Render_Op.EVAS_RENDER_COPY mode can be
 * used to "poke" holes in a window (to see through it), as only the compositor
 * can ensure that. Copy mode should only be used with otherwise opaque
 * widgets, or inside non-window surfaces (eg. a transparent background inside
 * an Ecore.Evas.Buffer).
 *
 * @param[in] render_op One of the Evas_Render_Op values. Only blend (default)
 * and copy modes are supported.
 *
 * @ingroup Evas_Object
 */
EAPI void evas_object_render_op_set(Evas_Object *obj, Evas_Render_Op render_op);

/**
 * @brief Retrieves the current value of the operation used for rendering the
 * Evas object.
 *
 * @return One of the Evas_Render_Op values. Only blend (default) and copy
 * modes are supported.
 *
 * @ingroup Evas_Object
 */
EAPI Evas_Render_Op evas_object_render_op_get(const Evas_Object *obj);

/**
 * @brief Get the "static clipper" hint flag for a given Evas object.
 *
 * @return @c true if it's to be used as a static clipper, @c false otherwise.
 *
 * @ingroup Evas_Object
 */
EAPI Eina_Bool evas_object_static_clip_get(const Evas_Object *obj);

/**
 * @brief Sets the scaling factor for an Evas object. Does not affect all
 * objects.
 *
 * This will multiply the object's dimension by the given factor, thus altering
 * its geometry (width and height). Useful when you want scalable UI elements,
 * possibly at run time.
 *
 * @note Only text and textblock objects have scaling change handlers. Other
 * objects won't change visually on this call.
 *
 * @param[in] obj The object.
 * @param[in] scale The scaling factor. 1.0 means no scaling, default size.
 *
 * @ingroup Evas_Object
 */
EAPI void evas_object_scale_set(Evas_Object *obj, double scale);

/**
 * @brief Retrieves the scaling factor for the given Evas object.
 *
 * @param[in] obj The object.
 *
 * @return The scaling factor. 1.0 means no scaling, default size.
 *
 * @ingroup Evas_Object
 */
EAPI double evas_object_scale_get(const Evas_Object *obj);

/**
 * @brief Returns whether the mouse pointer is logically inside the object.
 *
 * @param[in] dev The pointer device.
 *
 * @return @c true if the pointer is inside, @c false otherwise.
 *
 * @since 1.20
 *
 * @ingroup Evas_Object
 */
EAPI Eina_Bool evas_object_pointer_inside_by_device_get(const Evas_Object *obj, Efl_Input_Device * dev);

/**
 * @brief Returns whether the default mouse pointer is logically inside the
 * object.
 *
 * When this function is called it will return a value of either @c false or
 * @c true, depending on if event_feed_mouse_in or event_feed_mouse_out have
 * been called to feed in a mouse enter event into the object.
 *
 * A return value of @c true indicates the mouse is logically inside the
 * object, and @c false implies it is logically outside the object.
 *
 * If @c e is not a valid object, the return value is undefined.
 *
 * @return @c true if the mouse pointer is inside the object, @c false
 * otherwise
 *
 * @ingroup Evas_Object
 */
EAPI Eina_Bool evas_object_pointer_inside_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Returns whether the coords are logically inside the object.
 *
 * When this function is called it will return a value of either @c false or
 * @c true, depending on if the coords are inside the object's current
 * geometry.
 *
 * A return value of @c true indicates the position is logically inside the
 * object, and @c false implies it is logically outside the object.
 *
 * If @c e is not a valid object, the return value is undefined.
 *
 * @param[in] x The canvas-relative x coordinate.
 * @param[in] y The canvas-relative y coordinate.
 *
 * @return @c true if the coords are inside the object, @c false otherwise
 *
 * @ingroup Evas_Object
 */

EAPI Eina_Bool evas_object_pointer_coords_inside_get(const Evas_Object *eo_obj, int x, int y) EINA_WARN_UNUSED_RESULT;

#include "canvas/efl_canvas_object_eo.legacy.h"

/**
 * @brief Get the Evas to which this object belongs to
 *
 * The object may be an evas object, an elementary object or window, or an
 * evas 3D / VG object.
 */
EAPI Evas *evas_object_evas_get(const Eo *obj);

/**
 * @brief Retrieve a list of objects lying over a given position in a canvas.
 *
 * This function will traverse all the layers of the given canvas, from top to
 * bottom, querying for objects with areas covering the given position. The
 * user can remove from query objects which are hidden and/or which are set to
 * pass events.
 *
 * @warning This function will skip objects parented by smart objects, acting
 * only on the ones at the "top level", with regard to object parenting.
 *
 * @param[in] obj The object.
 * @param[in] x The pixel position.
 * @param[in] y The pixel position.
 * @param[in] include_pass_events_objects Boolean flag to include or not
 * objects which pass events in this calculation.
 * @param[in] include_hidden_objects Boolean flag to include or not hidden
 * objects in this calculation.
 *
 * @return The list of objects that are over the given position in @c e.
 *
 * @ingroup Efl_Canvas
 */
 EAPI Eina_List *evas_objects_at_xy_get(Eo *eo_e, int x, int y, Eina_Bool include_pass_events_objects, Eina_Bool include_hidden_objects);


/**
 * @brief Retrieve the object stacked at the top of a given position in a
 * canvas.
 *
 * This function will traverse all the layers of the given canvas, from top to
 * bottom, querying for objects with areas covering the given position. The
 * user can remove from the query objects which are hidden and/or which are set
 * to pass events.
 *
 * @warning This function will skip objects parented by smart objects, acting
 * only on the ones at the "top level", with regard to object parenting.
 *
 * @param[in] obj The object.
 * @param[in] x The pixel position.
 * @param[in] y The pixel position.
 * @param[in] include_pass_events_objects Boolean flag to include or not
 * objects which pass events in this calculation.
 * @param[in] include_hidden_objects Boolean flag to include or not hidden
 * objects in this calculation.
 *
 * @return The Evas object that is over all other objects at the given
 * position.
 */
 EAPI Evas_Object* evas_object_top_at_xy_get(Eo *eo_e, Evas_Coord x, Evas_Coord y, Eina_Bool include_pass_events_objects, Eina_Bool include_hidden_objects);


/**
 * @brief Get all objects in the given rectangle
 *
 * @param[in] obj The object.
 * @param[in] x X coordinate
 * @param[in] y Y coordinate
 * @param[in] w Width
 * @param[in] h Height
 * @param[in] include_pass_events_objects @c true if the list should include
 * objects which pass events
 * @param[in] include_hidden_objects @c true if the list should include hidden
 * objects
 *
 * @return List of objects
 */
EAPI Eina_List *evas_objects_in_rectangle_get(const Eo *obj, int x, int y, int w, int h, Eina_Bool include_pass_events_objects, Eina_Bool include_hidden_objects) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Retrieve the Evas object stacked at the top of a given rectangular
 * region in a canvas
 *
 * This function will traverse all the layers of the given canvas, from top to
 * bottom, querying for objects with areas overlapping with the given
 * rectangular region inside @c e. The user can remove from the query objects
 * which are hidden and/or which are set to pass events.
 *
 * @warning This function will skip objects parented by smart objects, acting
 * only on the ones at the "top level", with regard to object parenting.
 *
 * @param[in] obj The object.
 * @param[in] x The top left corner's horizontal coordinate for the rectangular
 * region.
 * @param[in] y The top left corner's vertical coordinate for the rectangular
 * region.
 * @param[in] w The width of the rectangular region.
 * @param[in] h The height of the rectangular region.
 * @param[in] include_pass_events_objects Boolean flag to include or not
 * objects which pass events in this calculation.
 * @param[in] include_hidden_objects Boolean flag to include or not hidden
 * objects in this calculation.
 *
 * @return The Evas object that is over all other objects at the given
 * rectangular region.
 *
 * @ingroup Evas_Canvas
 */
EAPI Evas_Object *evas_object_top_in_rectangle_get(const Eo *obj, int x, int y, int w, int h, Eina_Bool include_pass_events_objects, Eina_Bool include_hidden_objects) EINA_WARN_UNUSED_RESULT;
/**
 * @}
 */

/**
 * @ingroup Evas_Object_Group_Events
 *
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
 * #EVAS_CALLBACK_AXIS_UPDATE,
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
 * - #EVAS_CALLBACK_AXIS_UPDATE: @p event_info is a pointer to an
 *   #Evas_Event_Axis_Update struct
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
 * just type and function pointer, use
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
 * @brief Requests @c keyname key events be directed to @c obj.
 *
 * Key grabs allow one or more objects to receive key events for specific key
 * strokes even if other objects have focus. Whenever a key is grabbed, only
 * the objects grabbing it will get the events for the given keys.
 *
 * @c keyname is a platform dependent symbolic name for the key pressed (see
 * @ref Evas_Keys for more information).
 *
 * @c modifiers and @c not_modifiers are bit masks of all the modifiers that
 * must and mustn't, respectively, be pressed along with @c keyname key in
 * order to trigger this new key grab. Modifiers can be things such as Shift
 * and Ctrl as well as user defined types via @ref evas_key_modifier_add.
 * Retrieve them with @ref evas_key_modifier_mask_get or use 0 for empty masks.
 *
 * @c exclusive will make the given object the only one permitted to grab the
 * given key. If given @c true, subsequent calls on this function with
 * different @c obj arguments will fail, unless the key is ungrabbed again.
 *
 * @warning Providing impossible modifier sets creates undefined behavior.
 *
 * See also @ref evas_object_key_ungrab, @ref evas_object_focus_get,
 * @ref evas_object_focus_set, @ref evas_focus_get, @ref evas_key_modifier_add.
 *
 * @param[in] keyname The key to request events for.
 * @param[in] modifiers A mask of modifiers that must be present to trigger the
 * event.
 * @param[in] not_modifiers A mask of modifiers that must not be present to
 * trigger the event.
 * @param[in] exclusive Request that the @c obj is the only object receiving
 * the @c keyname events.
 *
 * @return @c true if the call succeeded, @c false otherwise.
 */
EAPI Eina_Bool evas_object_key_grab(Evas_Object *obj, const char *keyname, Evas_Modifier_Mask modifiers, Evas_Modifier_Mask not_modifiers, Eina_Bool exclusive) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(2);

/**
 * @brief Removes the grab on @c keyname key events by @c obj.
 *
 * Removes a key grab on @c obj if @c keyname, @c modifiers, and
 * @c not_modifiers match.
 *
 * See also @ref evas_object_key_grab, @ref evas_object_focus_get,
 * @ref evas_object_focus_set, @ref evas_focus_get.
 *
 * @param[in] keyname The key the grab is set for.
 * @param[in] modifiers A mask of modifiers that must be present to trigger the
 * event.
 * @param[in] not_modifiers A mask of modifiers that mus not not be present to
 * trigger the event.
 */
EAPI void evas_object_key_ungrab(Evas_Object *obj, const char *keyname, Evas_Modifier_Mask modifiers, Evas_Modifier_Mask not_modifiers) EINA_ARG_NONNULL(2);

/**
 * @}
 */

/**
 * @ingroup Evas_Object_Group_Extras
 *
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
 * situation is nonsensical and thus can be considered an error as
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
 * @}
 */

/**
 * @ingroup Evas_Object_Group_Find
 *
 * @{
 */

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
 * @ingroup Evas_Object_Group_Interceptors
 *
 * @{
 */

/**
 * Function signature for the resize event of an evas object
 *
 * @param data is the pointer passed through the callback.
 * @param obj the object being shown.
 *
 * @see evas_object_intercept_show_callback_add()
 * @see evas_object_intercept_show_callback_del()
 *
 */
typedef void (*Evas_Object_Intercept_Show_Cb)(void *data, Evas_Object *obj);

/**
 * Function signature for the hide event of an evas object
 *
 * @param data is the pointer passed through the callback.
 * @param obj the object being hidden.
 *
 * @see  evas_object_intercept_hide_callback_add()
 * @see  evas_object_intercept_hide_callback_del()
 *
 */
typedef void (*Evas_Object_Intercept_Hide_Cb)(void *data, Evas_Object *obj);

/**
 * Function signature for the move event of an evas object
 *
 * @param data the pointer passed through the callback.
 * @param obj the object being moved.
 * @param x move x position
 * @param y move y position
 *
 * @see  evas_object_intercept_move_callback_add()
 * @see  evas_object_intercept_move_callback_del()
 *
 */
typedef void (*Evas_Object_Intercept_Move_Cb)(void *data, Evas_Object *obj, Evas_Coord x, Evas_Coord y);

/**
 * Function signature for the resize event of an evas object
 *
 * @param data the pointer passed through the callback.
 * @param obj the object being resized.
 * @param width of the object
 * @param height of the object
 *
 * @see  evas_object_intercept_resize_callback_add()
 * @see  evas_object_intercept_resize_callback_del()
 *
 */
typedef void (*Evas_Object_Intercept_Resize_Cb)(void *data, Evas_Object *obj, Evas_Coord w, Evas_Coord h);

/**
 * Function signature for the raise event of an evas object
 *
 * @param data the pointer passed through the callback.
 * @param obj the object being raised.
 *
 * @see  evas_object_intercept_raise_callback_add()
 * @see  evas_object_intercept_raise_callback_del()
 *
 */
typedef void (*Evas_Object_Intercept_Raise_Cb)(void *data, Evas_Object *obj);

/**
 * Function signature for the lower event of an evas object
 *
 * @param data the pointer passed through the callback.
 * @param obj the object being lowered.
 *
 * @see  evas_object_intercept_lower_callback_add()
 * @see  evas_object_intercept_lower_callback_del()
 *
 */
typedef void (*Evas_Object_Intercept_Lower_Cb)(void *data, Evas_Object *obj);

/**
 * Function signature for the stack above event of an evas object
 *
 * @param data the pointer passed through the callback.
 * @param obj the object being stacked above.
 * @param above the object above which the object is stacked
 *
 * @see  evas_object_intercept_stack_above_callback_add()
 * @see  evas_object_intercept_stack_above_callback_del()
 *
 */
typedef void (*Evas_Object_Intercept_Stack_Above_Cb)(void *data, Evas_Object *obj, Evas_Object *above);

/**
 * Function signature for the stack below event of an evas object
 *
 * @param data the pointer passed through the callback.
 * @param obj the object being stacked below.
 * @param above the object below which the object is stacked
 *
 * @see  evas_object_intercept_stack_below_callback_add()
 * @see  evas_object_intercept_stack_below_callback_del()
 *
 */
typedef void (*Evas_Object_Intercept_Stack_Below_Cb)(void *data, Evas_Object *obj, Evas_Object *above);

/**
 * Function signature for the layer event of an evas object
 *
 * @param data the pointer passed through the callback.
 * @param obj the object being layered
 * @param l the layer value
 *
 * @see  evas_object_intercept_layer_callback_add()
 * @see  evas_object_intercept_layer_callback_del()
 *
 */
typedef void (*Evas_Object_Intercept_Layer_Set_Cb)(void *data, Evas_Object *obj, int l);

/**
 * Function signature for the focus set event of an evas object
 *
 * @param data the pointer passed through the callback.
 * @param obj the object being focused
 * @param focus the focus value, EINA_TRUE if the object is focused, EINA_FALSE otherwise
 *
 * @see  evas_object_intercept_focus_set_callback_add()
 * @see  evas_object_intercept_focus_set_callback_del()
 *
 */
typedef void (*Evas_Object_Intercept_Focus_Set_Cb)(void *data, Evas_Object *obj, Eina_Bool focus);

/**
 * Function signature for the focus set event of an evas object with seat info
 *
 * @param data the pointer passed through the callback.
 * @param obj the object being focused
 * @param focus the focus value, EINA_TRUE if the object is focused, EINA_FALSE otherwise
 * @param seat the seat for which focus is being set
 *
 * @see  evas_object_intercept_device_focus_set_callback_add()
 * @see  evas_object_intercept_device_focus_set_callback_del()
 *
 * @since 1.20
 */
typedef void (*Evas_Object_Intercept_Device_Focus_Set_Cb)(void *data, Evas_Object *obj, Eina_Bool focus, Eo *seat);

/**
 * Function signature for the color set event of an evas object
 *
 * @param data the pointer passed through the callback.
 * @param obj the object changing color
 * @param r the red component of the color
 * @param g the green component of the color
 * @param b the blue component of the color
 * @param a the alpha component of the color
 *
 * @see  evas_object_intercept_color_set_callback_add()
 * @see  evas_object_intercept_color_set_callback_del()
 *
 */
typedef void (*Evas_Object_Intercept_Color_Set_Cb)(void *data, Evas_Object *obj, int r, int g, int b, int a);

/**
 * Function signature for the clip set event of an evas object
 *
 * @param data the pointer passed through the callback.
 * @param obj the object being clipped
 * @param clip the evas object on which the object is clipped
 *
 * @see  evas_object_intercept_clip_set_callback_add()
 * @see  evas_object_intercept_clip_set_callback_del()
 *
 */
typedef void (*Evas_Object_Intercept_Clip_Set_Cb)(void *data, Evas_Object *obj, Evas_Object *clip);

/**
 * Function signature for the clip unset event of an evas object
 *
 * @param data the pointer passed through the callback.
 * @param obj the object being unclipped
 *
 * @see  evas_object_intercept_clip_unset_callback_add()
 * @see  evas_object_intercept_clip_unset_callback_del()
 *
 */
typedef void (*Evas_Object_Intercept_Clip_Unset_Cb)(void *data, Evas_Object *obj);

/**
 * Set the callback function that intercepts a show event of an object.
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
 * Unset the callback function that intercepts a show event of a object.
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
 * Unset the callback function that intercepts a hide event of a object.
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
 * Set the callback function that intercepts a move event of an object.
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
 * Unset the callback function that intercepts a move event of an object.
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

/**
 * Set the callback function that intercepts a resize event of an object.
 *
 * @param obj The given canvas object pointer.
 * @param func The given function to be the callback function.
 * @param data The data passed to the callback function.
 *
 * This function sets a callback function to intercepts a resize event
 * of a canvas object.
 *
 * @see evas_object_intercept_resize_callback_del().
 *
 */
EAPI void  evas_object_intercept_resize_callback_add(Evas_Object *obj, Evas_Object_Intercept_Resize_Cb func, const void *data) EINA_ARG_NONNULL(1, 2);

/**
 * Unset the callback function that intercepts a resize event of an object.
 *
 * @param obj The given canvas object pointer.
 * @param func The given callback function.
 *
 * This function sets a callback function to intercepts a resize event
 * of a canvas object.
 *
 * @see evas_object_intercept_resize_callback_add().
 *
 */
EAPI void *evas_object_intercept_resize_callback_del(Evas_Object *obj, Evas_Object_Intercept_Resize_Cb func) EINA_ARG_NONNULL(1, 2);

/**
 * Set the callback function that intercepts a raise event of an object.
 *
 * @param obj The given canvas object pointer.
 * @param func The given function to be the callback function.
 * @param data The data passed to the callback function.
 *
 * This function sets a callback function to intercepts a raise event
 * of a canvas object.
 *
 * @see evas_object_intercept_raise_callback_del().
 *
 */
EAPI void  evas_object_intercept_raise_callback_add(Evas_Object *obj, Evas_Object_Intercept_Raise_Cb func, const void *data) EINA_ARG_NONNULL(1, 2);

/**
 * Unset the callback function that intercepts a raise event of an object.
 *
 * @param obj The given canvas object pointer.
 * @param func The given callback function.
 *
 * This function sets a callback function to intercepts a raise event
 * of a canvas object.
 *
 * @see evas_object_intercept_raise_callback_add().
 *
 */
EAPI void *evas_object_intercept_raise_callback_del(Evas_Object *obj, Evas_Object_Intercept_Raise_Cb func) EINA_ARG_NONNULL(1, 2);

/**
 * Set the callback function that intercepts a lower event of an object.
 *
 * @param obj The given canvas object pointer.
 * @param func The given function to be the callback function.
 * @param data The data passed to the callback function.
 *
 * This function sets a callback function to intercepts a lower event
 * of a canvas object.
 *
 * @see evas_object_intercept_lower_callback_del().
 *
 */
EAPI void  evas_object_intercept_lower_callback_add(Evas_Object *obj, Evas_Object_Intercept_Lower_Cb func, const void *data) EINA_ARG_NONNULL(1, 2);

/**
 * Unset the callback function that intercepts a lower event of an object.
 *
 * @param obj The given canvas object pointer.
 * @param func The given callback function.
 *
 * This function sets a callback function to intercepts a lower event
 * of a canvas object.
 *
 * @see evas_object_intercept_lower_callback_add().
 *
 */
EAPI void *evas_object_intercept_lower_callback_del(Evas_Object *obj, Evas_Object_Intercept_Lower_Cb func) EINA_ARG_NONNULL(1, 2);

/**
 * Set the callback function that intercepts a stack above event of an object.
 *
 * @param obj The given canvas object pointer.
 * @param func The given function to be the callback function.
 * @param data The data passed to the callback function.
 *
 * This function sets a callback function to intercepts a stack above event
 * of a canvas object.
 *
 * @see evas_object_intercept_stack_above_callback_del().
 *
 */
EAPI void  evas_object_intercept_stack_above_callback_add(Evas_Object *obj, Evas_Object_Intercept_Stack_Above_Cb func, const void *data) EINA_ARG_NONNULL(1, 2);

/**
 * Unset the callback function that intercepts a stack above event of an object.
 *
 * @param obj The given canvas object pointer.
 * @param func The given callback function.
 *
 * This function sets a callback function to intercepts a stack above event
 * of a canvas object.
 *
 * @see evas_object_intercept_stack_above_callback_add().
 *
 */
EAPI void *evas_object_intercept_stack_above_callback_del(Evas_Object *obj, Evas_Object_Intercept_Stack_Above_Cb func) EINA_ARG_NONNULL(1, 2);

/**
 * Set the callback function that intercepts a stack below event of an object.
 *
 * @param obj The given canvas object pointer.
 * @param func The given function to be the callback function.
 * @param data The data passed to the callback function.
 *
 * This function sets a callback function to intercepts a stack below event
 * of a canvas object.
 *
 * @see evas_object_intercept_stack_below_callback_del().
 *
 */
EAPI void  evas_object_intercept_stack_below_callback_add(Evas_Object *obj, Evas_Object_Intercept_Stack_Below_Cb func, const void *data) EINA_ARG_NONNULL(1, 2);

/**
 * Unset the callback function that intercepts a stack below event of an object.
 *
 * @param obj The given canvas object pointer.
 * @param func The given callback function.
 *
 * This function sets a callback function to intercepts a stack below event
 * of a canvas object.
 *
 * @see evas_object_intercept_stack_below_callback_add().
 *
 */
EAPI void *evas_object_intercept_stack_below_callback_del(Evas_Object *obj, Evas_Object_Intercept_Stack_Below_Cb func) EINA_ARG_NONNULL(1, 2);

/**
 * Set the callback function that intercepts a layer set event of an object.
 *
 * @param obj The given canvas object pointer.
 * @param func The given function to be the callback function.
 * @param data The data passed to the callback function.
 *
 * This function sets a callback function to intercepts a layer set event
 * of a canvas object.
 *
 * @see evas_object_intercept_layer_set_callback_del().
 *
 */
EAPI void  evas_object_intercept_layer_set_callback_add(Evas_Object *obj, Evas_Object_Intercept_Layer_Set_Cb func, const void *data) EINA_ARG_NONNULL(1, 2);

/**
 * Unset the callback function that intercepts a layer set event of an object.
 *
 * @param obj The given canvas object pointer.
 * @param func The given callback function.
 *
 * This function sets a callback function to intercepts a layer set event
 * of a canvas object.
 *
 * @see evas_object_intercept_layer_set_callback_add().
 *
 */
EAPI void *evas_object_intercept_layer_set_callback_del(Evas_Object *obj, Evas_Object_Intercept_Layer_Set_Cb func) EINA_ARG_NONNULL(1, 2);

/**
 * Set the callback function that intercepts a color set event of an object.
 *
 * @param obj The given canvas object pointer.
 * @param func The given function to be the callback function.
 * @param data The data passed to the callback function.
 *
 * This function sets a callback function to intercepts a color set event
 * of a canvas object.
 *
 * @see evas_object_intercept_color_set_callback_del().
 *
 */
EAPI void  evas_object_intercept_color_set_callback_add(Evas_Object *obj, Evas_Object_Intercept_Color_Set_Cb func, const void *data) EINA_ARG_NONNULL(1, 2);

/**
 * Unset the callback function that intercepts a color set event of an object.
 *
 * @param obj The given canvas object pointer.
 * @param func The given callback function.
 *
 * This function sets a callback function to intercepts a color set event
 * of a canvas object.
 *
 * @see evas_object_intercept_color_set_callback_add().
 *
 */
EAPI void *evas_object_intercept_color_set_callback_del(Evas_Object *obj, Evas_Object_Intercept_Color_Set_Cb func) EINA_ARG_NONNULL(1, 2);

/**
 * Set the callback function that intercepts a clip set event of an object.
 *
 * @param obj The given canvas object pointer.
 * @param func The given function to be the callback function.
 * @param data The data passed to the callback function.
 *
 * This function sets a callback function to intercepts a clip set event
 * of a canvas object.
 *
 * @see evas_object_intercept_clip_set_callback_del().
 *
 */
EAPI void  evas_object_intercept_clip_set_callback_add(Evas_Object *obj, Evas_Object_Intercept_Clip_Set_Cb func, const void *data) EINA_ARG_NONNULL(1, 2);

/**
 * Unset the callback function that intercepts a clip set event of an object.
 *
 * @param obj The given canvas object pointer.
 * @param func The given callback function.
 *
 * This function sets a callback function to intercepts a clip set event
 * of a canvas object.
 *
 * @see evas_object_intercept_clip_set_callback_add().
 *
 */
EAPI void *evas_object_intercept_clip_set_callback_del(Evas_Object *obj, Evas_Object_Intercept_Clip_Set_Cb func) EINA_ARG_NONNULL(1, 2);

/**
 * Set the callback function that intercepts a clip unset event of an object.
 *
 * @param obj The given canvas object pointer.
 * @param func The given function to be the callback function.
 * @param data The data passed to the callback function.
 *
 * This function sets a callback function to intercepts a clip unset event
 * of a canvas object.
 *
 * @see evas_object_intercept_clip_unset_callback_del().
 *
 */
EAPI void  evas_object_intercept_clip_unset_callback_add(Evas_Object *obj, Evas_Object_Intercept_Clip_Unset_Cb func, const void *data) EINA_ARG_NONNULL(1, 2);

/**
 * Unset the callback function that intercepts a clip unset event of an object.
 *
 * @param obj The given canvas object pointer.
 * @param func The given callback function.
 *
 * This function sets a callback function to intercepts a clip unset event
 * of a canvas object.
 *
 * @see evas_object_intercept_clip_unset_callback_add().
 *
 */
EAPI void *evas_object_intercept_clip_unset_callback_del(Evas_Object *obj, Evas_Object_Intercept_Clip_Unset_Cb func) EINA_ARG_NONNULL(1, 2);

/**
 * Set the callback function that intercepts a focus set event of an object.
 *
 * @param obj The given canvas object pointer.
 * @param func The given function to be the callback function.
 * @param data The data passed to the callback function.
 *
 * This function sets a callback function to intercepts a focus set event
 * of a canvas object.
 *
 * @see evas_object_intercept_focus_set_callback_del().
 *
 */
EAPI void  evas_object_intercept_focus_set_callback_add(Evas_Object *obj, Evas_Object_Intercept_Focus_Set_Cb func, const void *data) EINA_ARG_NONNULL(1, 2);

/**
 * Unset the callback function that intercepts a focus set event of an object.
 *
 * @param obj The given canvas object pointer.
 * @param func The given callback function.
 *
 * This function sets a callback function to intercepts a focus set event
 * of a canvas object.
 *
 * @see evas_object_intercept_focus_set_callback_add().
 *
 */
EAPI void *evas_object_intercept_focus_set_callback_del(Evas_Object *obj, Evas_Object_Intercept_Focus_Set_Cb func) EINA_ARG_NONNULL(1, 2);

/**
 * Set the callback function that intercepts a focus set event of an object.
 *
 * @param obj The given canvas object pointer.
 * @param func The given function to be the callback function.
 * @param data The data passed to the callback function.
 *
 * This function sets a callback function to intercepts a focus set event
 * of a canvas object and provides seat info.
 *
 * @see evas_object_intercept_device_focus_set_callback_del().
 *
 * @since 1.20
 *
 */
EAPI void  evas_object_intercept_device_focus_set_callback_add(Evas_Object *obj, Evas_Object_Intercept_Device_Focus_Set_Cb func, const void *data) EINA_ARG_NONNULL(1, 2);

/**
 * Unset the callback function that intercepts a focus set event of an object.
 *
 * @param obj The given canvas object pointer.
 * @param func The given callback function.
 *
 * This function sets a callback function to intercepts a focus set event
 * of a canvas object and provides seat info.
 *
 * @see evas_object_intercept_device_focus_set_callback_add().
 *
 * @since 1.20
 *
 */
EAPI void *evas_object_intercept_device_focus_set_callback_del(Evas_Object *obj, Evas_Object_Intercept_Device_Focus_Set_Cb func) EINA_ARG_NONNULL(1, 2);

/* Internal APIs for legacy compatibility */
#ifdef EFL_CANVAS_OBJECT_PROTECTED

enum _Evas_Object_Intercept_Cb_Type
{
   EVAS_OBJECT_INTERCEPT_CB_VISIBLE,
   EVAS_OBJECT_INTERCEPT_CB_MOVE,
   EVAS_OBJECT_INTERCEPT_CB_RESIZE,
   EVAS_OBJECT_INTERCEPT_CB_RAISE,
   EVAS_OBJECT_INTERCEPT_CB_LOWER,
   EVAS_OBJECT_INTERCEPT_CB_STACK_ABOVE,
   EVAS_OBJECT_INTERCEPT_CB_STACK_BELOW,
   EVAS_OBJECT_INTERCEPT_CB_LAYER_SET,
   EVAS_OBJECT_INTERCEPT_CB_FOCUS_SET,
   EVAS_OBJECT_INTERCEPT_CB_COLOR_SET,
   EVAS_OBJECT_INTERCEPT_CB_CLIP_SET,
   EVAS_OBJECT_INTERCEPT_CB_DEVICE_FOCUS_SET,
};
typedef enum _Evas_Object_Intercept_Cb_Type Evas_Object_Intercept_Cb_Type;

EWAPI Eina_Bool _evas_object_intercept_call(Evas_Object *obj, Evas_Object_Intercept_Cb_Type type, int internal, ...);

#endif


/**
 * @}
 */

/**
 * @ingroup Evas_Object_Rectangle
 *
 * @{
 */

/**
 * Adds a rectangle to the given evas.
 * @param   e The given evas.
 * @return  The new rectangle object.
 *
 * @ingroup Evas_Object_Rectangle
 */
EAPI Evas_Object *evas_object_rectangle_add(Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

#include "canvas/efl_canvas_rectangle.eo.legacy.h"

/**
 * @}
 */

/**
 * @defgroup Evas_Object_Vg
 * @ingroup Evas
 *
 * Evas_Object_Vg is the scene graph for managing vector graphics  objects.
 * User can create shape objects as well as fill objects and give it to the
 * Evas_Object_Vg for drawing on the screen as well as managing the lifecycle
 * of the objects. enabling reuse of shape objects.
 *
 * As Evas_Object_Vg is a Evas_Object all the operation that applicable to
 * a Evas_Object can be performed on it(clipping , map, etc).
 *
 * To create any complex vector graphics you can create a hirarchy of shape
 * and fill objects and give the hirarchy to Evas_Object which  will be
 * responsible for drawing and showing on the screen.
 *
 * As the shape object and fill object (linear and radial gradient) have
 * retain mode API, you only have to create it once and set the properties
 * and give it to evas_object_vg.
 *
 * Any change in the property of shape/fill object will automaticaly notified
 * to the evas_object_vg which will trigger a redrawing to reflect the change.
 *
 * To create a vector path, you can give list of path commands to the shape
 * object using efl_gfx_shape_path_set() API.
 *
 * Enabling graphical shapes to be constructed and reused.
 *
 * Below are the list of feature currently supported by Vector object.
 *
 * @li Drawing SVG Path.
 *     You can construct a path by using api in efl_gfx_utils.h
 *
 * @li Gradient filling and stroking.
 *     You can fill or stroke the path using linear or radial gradient.
 *     @see Evas_Vg_Gradient_Linear and Evas_Vg_Gradient_Radial
 *
 * @li Transformation support for path and gradient fill. You can apply
       affin transformation on path object.
 *     @see Eina_Matrix.
 *
 * @note Below are the list of interface, classes can be used to draw vector
 *       graphics using vector object.
 *
 * @li Efl.Gfx.Shape
 * @li Evas.VG_Shape
 * @li Evas.VG_Node
 * @li Efl.Gfx.Gradient
 * @li Efl.Gfx.Gradient_Radial
 * @li Efl.Gfx.Gradient_Linear
 *
 * Example:
 * @code
 * vector = evas_object_vg_add(canvas);
 * root = evas_obj_vg_root_node_get(vector);
 * shape = efl_add(EVAS_VG_SHAPE_CLASS, root);
 * Efl_Gfx_Path_Command *path_cmd = NULL;
 * double *points = NULL;
 * efl_gfx_path_append_circle(&path_cmd, &points);
 * *       evas_vg_node_origin_set(shape, 10, 10);
 * *       efl_gfx_shape_stroke_width_set(shape, 1.0);
 * *       evas_vg_node_color_set(shape, 128, 128, 128, 80);
 * *       efl_gfx_shape_path_set(shape, path_cmd, points); 
 * @endcode
 *
 * @since 1.14
 */

/**
 * Creates a new vector object on the given Evas @p e canvas.
 *
 * @param e The given canvas.
 * @return The created vector object handle.
 *
 * The shape object hierarchy can be added to the evas_object_vg by accessing
 * the root node of the vg canvas and adding the hierarchy as child to the root
 * node.
 *
 * @see evas_obj_vg_root_node_get()
 * @since 1.14
 */
EAPI Evas_Object *evas_object_vg_add(Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * Get the total number of frames of the vector, if it's animated.
 *
 * @return The number of frames. 0, if it's not animated.
 *
 * @since 1.22
 */
EAPI int evas_object_vg_animated_frame_count_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * Get the duration of a sequence of frames.
 *
 * This returns total duration in seconds that the specified
 * sequence of frames should take.
 *
 * If @p start_frame is 1 and @p frame_num is 0, this returns the
 * duration of frame 1. If @p start_frame is 1 and @p frame_num is 1,
 * this returns the total duration of frame 1 + frame 2.
 *
 * @param[in] start_frame The first frame, ranges from 1 to maximum frame count.
 * @param[in] frame_num Number of frames in the sequence, starts from 0.
 *
 * @return Duration in seconds.
 *
 * @see evas_object_vg_animated_frame_count_get()
 * @since 1.22
 */
EAPI double evas_object_vg_animated_frame_duration_get(const Evas_Object *obj, int start_frame EINA_UNUSED, int frame_num EINA_UNUSED) EINA_ARG_NONNULL(1);

/**
 *
 * Set the source file from where an vector object must fetch the real
 * vector data (it may be one of json, svg, eet files).
 *
 * If the file supports multiple data stored in it (as Eet files do),
 * you can specify the key to be used as the index of the vector in
 * this file.
 *
 * @param[in] file The vector file path.
 * @param[in] key The vector key in @p file (if its an Eet one), or @c
NULL, otherwise.
 *
 * @return @c EINA_TRUE if it's succeed to read file, @c EINA_FALSE otherwise.
 *
 * @since 1.22
 */
EAPI Eina_Bool evas_object_vg_file_set(Evas_Object *obj, const char *file, const char *key);

/**
 *
 * Set the source mmaped file from where an vector object must fetch the real
 * vector data (it may be one of json, svg, eet files).
 *
 * If the file supports multiple data stored in it (as Eet files do),
 * you can specify the key to be used as the index of the vector in
 * this file.
 *
 * @since 1.22
 *
 * @param[in] f The mmaped file
 * @param[in] key The vector key in @p file (if its an Eet one), or @c
NULL, otherwise.
 *
 * @return @c EINA_TRUE if it's succeed to read file, @c EINA_FALSE otherwise.
 *
 */
EAPI Eina_Bool evas_object_vg_mmap_set(Evas_Object *obj, const Eina_File *f, const char *key);

/**
 * Set current frame of animated vector object.
 *
 * @param[in] frame_index The index of current frame.
 *
 * @note the @p frame_index must be in range of animation frames. (0 ~ max frame count)
 *
 * @return @c EINA_TRUE, if the frame index is valid. @c EINA_FALSE, otherwise.
 *
 * @see evas_object_vg_animated_frame_count_get()
 *
 * @since 1.22
 */
EAPI Eina_Bool evas_object_vg_animated_frame_set(Evas_Object *obj, int frame_index) EINA_ARG_NONNULL(1, 2);

/**
 * Get the current frame number of animated vector object.
 *
 * @return The frame index.
 *
 * @see evas_object_vg_animated_frame_set()
 * @see evas_object_vg_animated_frame_count_get()
 *
 * @since 1.22
 */
EAPI int evas_object_vg_animated_frame_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);


#include "canvas/efl_canvas_vg_node.eo.legacy.h"
#include "canvas/efl_canvas_vg_object_eo.legacy.h"
#include "canvas/efl_canvas_vg_object.eo.legacy.h"
#include "canvas/efl_canvas_vg_container.eo.legacy.h"

/**
 * Creates a new vector shape object.
 *
 * @param parent The given vector container object.
 * @return The created vector shape object handle.
 *
 * @since 1.14
 */
EAPI Efl_VG* evas_vg_shape_add(Efl_VG *parent);

/**
 * Creates a new vector container object.
 *
 * @param parent The given vector container object.
 * @return The created vector container object handle.
 *
 * @since 1.14
 */

EAPI Efl_VG* evas_vg_container_add(Efl_VG *parent);

/**
 *
 * Retrieves whether or not the given Efl_Vg object is visible.
 *
 *
 */
EAPI Eina_Bool evas_vg_node_visible_get(Eo *obj);

/**
 *
 * Makes the given Efl_Vg object visible or invisible.
 *
 * @param[in] v @c EINA_TRUE if to make the object visible, @c EINA_FALSE otherwise
 *
 */
EAPI void evas_vg_node_visible_set(Eo *obj, Eina_Bool v);

/**
 *
 * Retrieves the general/main color of the given Efl_Vg object.
 *
 * Retrieves the “main” color's RGB component (and alpha channel)
 * values, <b>which range from 0 to 255</b>. For the alpha channel,
 * which defines the object's transparency level, 0 means totally
 * transparent, while 255 means opaque. These color values are
 * premultiplied by the alpha value.
 *
 *
 * @note Use @c NULL pointers on the components you're not interested
 * in: they'll be ignored by the function.
 *
 * @param[out] r The red component of the given color.
 * @param[out] g The green component of the given color.
 * @param[out] b The blue component of the given color.
 * @param[out] a The alpha component of the given color.
 *
 */
EAPI void evas_vg_node_color_get(Eo *obj, int *r, int *g, int *b, int *a);

/**
 *
 * Sets the general/main color of the given Efl_Vg object to the given
 * one.
 *
 * @see evas_vg_node_color_get() (for an example)
 * @note These color values are expected to be premultiplied by @p a.
 *
 * @ingroup Evas_Object_Group_Basic
 *
 * @param[in] r The red component of the given color.
 * @param[in] g The green component of the given color.
 * @param[in] b The blue component of the given color.
 * @param[in] a The alpha component of the given color.
 *
 */
EAPI void evas_vg_node_color_set(Eo *obj, int r, int g, int b, int a);

/**
 *
 * Retrieves the geometry of the given Efl_Vg object.
 *
 * @param[out] x in
 * @param[out] y in
 * @param[out] w in
 * @param[out] h in
 *
 */
EAPI void evas_vg_node_geometry_get(Eo *obj, int *x, int *y, int *w, int *h);

/**
 *
 * Changes the geometry of the given Efl_Vg object.
 *
 * @param[in] x in
 * @param[in] y in
 * @param[in] w in
 * @param[in] h in
 *
 */
EAPI void evas_vg_node_geometry_set(Eo *obj, int x, int y, int w, int h);

/**
 *
 * Stack @p obj immediately below @p below
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
 * 
 *
 * @param[in] below the object below which to stack
 *
 */
EAPI void evas_vg_node_stack_below(Eo *obj, Eo *below);

/**
 *
 * Stack @p obj immediately above @p above
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
 * 
 *
 * @param[in] above the object above which to stack
 *
 */
EAPI void evas_vg_node_stack_above(Eo *obj, Eo *above);

/**
 *
 * Raise @p obj to the top of its layer.
 *
 * @p obj will, then, be the highest one in the layer it belongs
 * to. Object on other layers won't get touched.
 *
 * @see evas_object_stack_above()
 * @see evas_object_stack_below()
 * @see evas_object_lower()
 * 
 */
EAPI void evas_vg_node_raise(Eo *obj);

/**
 *
 * Lower @p obj to the bottom of its layer.
 *
 * @p obj will, then, be the lowest one in the layer it belongs
 * to. Objects on other layers won't get touched.
 *
 * @see evas_object_stack_above()
 * @see evas_object_stack_below()
 * @see evas_object_raise()
 * 
 *
 *
 */
EAPI void evas_vg_node_lower(Eo *obj);

#include "canvas/efl_canvas_vg_node_eo.legacy.h"

/**
 *
 * Get the stroke scaling factor used for stroking this path.
 * @since 1.14
 *
 *
 */
EAPI double evas_vg_shape_stroke_scale_get(Eo *obj);

/**
 *
 * Sets the stroke scale to be used for stroking the path.
 * the scale property will be used along with stroke width property.
 * @since 1.14
 *
 * @param[in] s stroke scale value
 *
 */
EAPI void evas_vg_shape_stroke_scale_set(Eo *obj, double s);

/**
 *
 * Gets the color used for stroking the path.
 * @since 1.14
 *
 * @param[out] r The red component of the given color.
 * @param[out] g The green component of the given color.
 * @param[out] b The blue component of the given color.
 * @param[out] a The alpha component of the given color.
 *
 */
EAPI void evas_vg_shape_stroke_color_get(Eo *obj, int *r, int *g, int *b, int *a);

/**
 *
 * Sets the color to be used for stroking the path.
 * @since 1.14
 *
 * @param[in] r The red component of the given color.
 * @param[in] g The green component of the given color.
 * @param[in] b The blue component of the given color.
 * @param[in] a The alpha component of the given color.
 *
 */
EAPI void evas_vg_shape_stroke_color_set(Eo *obj, int r, int g, int b, int a);

/**
 *
 * Gets the stroke width to be used for stroking the path.
 * @since 1.14
 *
 *
 */
EAPI double evas_vg_shape_stroke_width_get(Eo *obj);

/**
 *
 * Sets the stroke width to be used for stroking the path.
 * @since 1.14
 *
 * @param[in] w stroke width to be used
 *
 */
EAPI void evas_vg_shape_stroke_width_set(Eo *obj, double w);

/**
 *
 * Not Implemented
 *
 *
 */
EAPI double evas_vg_shape_stroke_location_get(Eo *obj);

/**
 *
 * Not Implemented
 *
 * @param[in] centered
 *
 */
EAPI void evas_vg_shape_stroke_location_set(Eo *obj, double centered);

/**
 *
 * Not Implemented
 *
 * @param[out] dash
 * @param[out] length
 *
 */
EAPI void evas_vg_shape_stroke_dash_get(Eo *obj, const Efl_Gfx_Dash **dash, unsigned int *length);

/**
 *
 * Not Implemented
 *
 * @param[in] dash
 * @param[in] length
 *
 */
EAPI void evas_vg_shape_stroke_dash_set(Eo *obj, const Efl_Gfx_Dash *dash, unsigned int length);

/**
 *
 * Gets the cap style used for stroking path.
 * @since 1.14
 *
 *
 */
EAPI Efl_Gfx_Cap evas_vg_shape_stroke_cap_get(Eo *obj);

/**
 *
 * Sets the cap style to be used for stroking the path.
 * The cap will be used for capping the end point of a 
 * open subpath.
 *
 * @see Efl_Gfx_Cap
 * @since 1.14
 *
 * @param[in] c cap style to use , default is EFL_GFX_CAP_BUTT
 *
 */
EAPI void evas_vg_shape_stroke_cap_set(Eo *obj, Efl_Gfx_Cap c);

/**
 *
 * Gets the join style used for stroking path.
 * @since 1.14
 *
 *
 */
EAPI Efl_Gfx_Join evas_vg_shape_stroke_join_get(Eo *obj);

/**
 *
 * Sets the join style to be used for stroking the path.
 * The join style will be used for joining the two line segment
 * while stroking the path.
 *
 * @see Efl_Gfx_Join
 * @since 1.14
 *
 * @param[in] j join style to use , default is
EFL_GFX_JOIN_MITER
 *
 */
EAPI void evas_vg_shape_stroke_join_set(Eo *obj, Efl_Gfx_Join j);

/**
 *
 * Set the list of commands and points to be used to create the
 * content of shape.
 *
 * @note see efl_gfx_path interface for how to create a command list.
 * @see Efl_Gfx_Path_Command
 * @since 1.14
 *
 * @param[in] op command list
 * @param[in] points point list
 *
 */
EAPI void evas_vg_shape_path_set(Eo *obj, const Efl_Gfx_Path_Command *op, const double *points);

/**
 *
 * Gets the command and points list
 * @since 1.14
 *
 * @param[out] op command list
 * @param[out] points point list
 *
 */

EAPI void evas_vg_shape_path_get(Eo *obj, const Efl_Gfx_Path_Command **op, const double **points);
EAPI void evas_vg_shape_path_length_get(Eo *obj, unsigned int *commands, unsigned int *points);
EAPI void evas_vg_shape_current_get(Eo *obj, double *x, double *y);
EAPI void evas_vg_shape_current_ctrl_get(Eo *obj, double *x, double *y);

/**
 *
 * Copy the shape data from the object specified .
 *
 * @since 1.14
 *
 *
 * @param[in] dup_from Shape object from where data will be copied.
 *
 */
EAPI void evas_vg_shape_dup(Eo *obj, Eo *dup_from);

/**
 *
 * Reset the shape data of the shape object.
 *
 * @since 1.14
 * 
 *
 *
 */
EAPI void evas_vg_shape_reset(Eo *obj);

/**
 *
 * Moves the current point to the given point, 
 * implicitly starting a new subpath and closing the previous one.
 *
 * @see efl_gfx_path_append_close()
 * @since 1.14
 *
 *
 * @param[in] x X co-ordinate of the current point.
 * @param[in] y Y co-ordinate of the current point.
 *
 */
EAPI void evas_vg_shape_append_move_to(Eo *obj, double x, double y);

/**
 *
 * Adds a straight line from the current position to the given endPoint.
 * After the line is drawn, the current position is updated to be at the end
 * point of the line.
 *
 * @note if no current position present, it draws a line to itself, basically
 * a point.
 *
 * @see efl_gfx_path_append_move_to()
 * @since 1.14
 *
 *
 * @param[in] x X co-ordinate of end point of the line.
 * @param[in] y Y co-ordinate of end point of the line.
 *
 */
EAPI void evas_vg_shape_append_line_to(Eo *obj, double x, double y);

/**
 *
 * Adds a quadratic Bezier curve between the current position and the
 * given end point (x,y) using the control points specified by (ctrl_x, ctrl_y).
 * After the path is drawn, the current position is updated to be at the end
 * point of the path.
 *
 * @since 1.14
 *
 *
 * @param[in] x X co-ordinate of end point of the line.
 * @param[in] y Y co-ordinate of end point of the line.
 * @param[in] ctrl_x X co-ordinate of control point.
 * @param[in] ctrl_y Y co-ordinate of control point.
 *
 */
EAPI void evas_vg_shape_append_quadratic_to(Eo *obj, double x, double y, double ctrl_x, double ctrl_y);

/**
 *
 * Same as efl_gfx_path_append_quadratic_to() api only difference is that it
 * uses the current control point to draw the bezier.
 *
 * @see efl_gfx_path_append_quadratic_to()
 * @since 1.14
 *
 *
 * @param[in] x X co-ordinate of end point of the line.
 * @param[in] y Y co-ordinate of end point of the line.
 *
 */
EAPI void evas_vg_shape_append_squadratic_to(Eo *obj, double x, double y);

/**
 *
 * Adds a cubic Bezier curve between the current position and the
 * given end point (x,y) using the control points specified by
 * (ctrl_x0, ctrl_y0), and (ctrl_x1, ctrl_y1). After the path is drawn,
 * the current position is updated to be at the end point of the path.
 *
 * @since 1.14
 *
 *
 * @param[in] x X co-ordinate of end point of the line.
 * @param[in] y Y co-ordinate of end point of the line.
 * @param[in] ctrl_x0 X co-ordinate of 1st control point.
 * @param[in] ctrl_y0 Y co-ordinate of 1st control point.
 * @param[in] ctrl_x1 X co-ordinate of 2nd control point.
 * @param[in] ctrl_y1 Y co-ordinate of 2nd control point.
 *
 */
EAPI void evas_vg_shape_append_cubic_to(Eo *obj, double x, double y, double ctrl_x0, double ctrl_y0, double ctrl_x1, double ctrl_y1);

/**
 *
 * Same as efl_gfx_path_append_cubic_to() api only difference is that it uses
 * the current control point to draw the bezier.
 *
 * @see efl_gfx_path_append_cubic_to()
 *
 * @since 1.14
 *
 *
 * @param[in] x X co-ordinate of end point of the line.
 * @param[in] y Y co-ordinate of end point of the line.
 * @param[in] ctrl_x X co-ordinate of 2nd control point.
 * @param[in] ctrl_y Y co-ordinate of 2nd control point.
 *
 */
EAPI void evas_vg_shape_append_scubic_to(Eo *obj, double x, double y, double ctrl_x, double ctrl_y);

/**
 *
 * Append an arc that connects from the current point int the point list
 * to the given point (x,y). The arc is defined by the given radius in 
 * x-direction (rx) and radius in y direction (ry) . 
 *
 * @note Use this api if you know the end point's of the arc otherwise
 * use more convenient function efl_gfx_path_append_arc_to()
 *
 * @see efl_gfx_path_append_arc_to()
 * @since 1.14
 *
 *
 * @param[in] x X co-ordinate of end point of the arc.
 * @param[in] y Y co-ordinate of end point of the arc.
 * @param[in] rx radius of arc in x direction.
 * @param[in] ry radius of arc in y direction.
 * @param[in] angle x-axis rotation , normally 0.
 * @param[in] large_arc Defines whether to draw the larger arc or smaller arc joining two point.
 * @param[in] sweep Defines whether the arc will be drawn counter-clockwise or clockwise from current point to the end point taking into account the large_arc property.
 *
 */
EAPI void evas_vg_shape_append_arc_to(Eo *obj, double x, double y, double rx, double ry, double angle, Eina_Bool large_arc, Eina_Bool sweep);

/**
 * @brief Append an arc that enclosed in the given rectangle (x, y, w, h). The
 * angle is defined in counter clock wise , use -ve angle for clockwise arc.
 *
 * @param[in] y Y co-ordinate of the rect.
 * @param[in] w width of the rect.
 * @param[in] h height of the rect.
 * @param[in] start_angle Angle at which the arc will start
 * @param[in] sweep_length @ Length of the arc.
 *
 * @since 1.18
 *
 * @ingroup Efl_Gfx_Shape
 */
EAPI void evas_vg_shape_append_arc(Eo *obj, double x, double y, double w, double h, double start_angle, double sweep_length);

/**
 *
 * Closes the current subpath by drawing a line to the beginning of the subpath,
 * automatically starting a new path. The current point of the new path is
 * (0, 0).
 *
 * @note If the subpath does not contain any points, this function does nothing.
 *
 * @since 1.14
 *
 *
 *
 */
EAPI void evas_vg_shape_append_close(Eo *obj);

/**
 *
 * Append a circle with given center and radius.
 *
 * @see efl_gfx_path_append_arc()
 * @since 1.14
 *
 *
 * @param[in] x X co-ordinate of the center of the circle.
 * @param[in] y Y co-ordinate of the center of the circle.
 * @param[in] radius radius of the circle.
 *
 */
EAPI void evas_vg_shape_append_circle(Eo *obj, double x, double y, double radius);

/**
 *
 * Append the given rectangle with rounded corner to the path.
 *
 * The xr and yr arguments specify the radii of the ellipses defining the
 * corners of the rounded rectangle.
 *
 * @note xr and yr are specified in terms of width and height respectively.
 *
 * @note if xr and yr are 0, then it will draw a rectangle without rounded corner.
 *
 * @since 1.14
 *
 *
 * @param[in] x X co-ordinate of the rectangle.
 * @param[in] y Y co-ordinate of the rectangle.
 * @param[in] w Width of the rectangle.
 * @param[in] h Height of the rectangle.
 * @param[in] rx The x radius of the rounded corner and should be in range [ 0 to w/2 ]
 * @param[in] ry The y radius of the rounded corner and should be in range [ 0 to h/2 ]
 *
 */
EAPI void evas_vg_shape_append_rect(Eo *obj, double x, double y, double w, double h, double rx, double ry);

EAPI void evas_vg_shape_append_svg_path(Eo *obj, const char *svg_path_data);
EAPI Eina_Bool evas_vg_shape_interpolate(Eo *obj, const Eo *from, const Eo *to, double pos_map);
EAPI Eina_Bool evas_vg_shape_equal_commands(Eo *obj, const Eo *with);

/**
 * set a vg object as the fill property
 *
 * @param obj The object whose fill property gets modified.
 * @param f The object content will be used for filling.
 *
 */
EAPI void evas_vg_shape_fill_set(Eo *obj, Efl_VG *f);

/**
 * returns the object that is set for the fill property
 *
 * @param obj The object whose fill property is inspected.
 * @return The object that is set as fill property.
 *
 */
EAPI Efl_VG* evas_vg_shape_fill_get(const Eo *obj);

/**
 * set a vg object as the stroke fill property
 *
 * @param obj The object whose stroke fill property gets modified.
 * @param f The object content will be used for stroke filling.
 *
 */
EAPI void evas_vg_shape_stroke_fill_set(Eo *obj, Efl_VG *f);

/**
 * returns the object that is set for the stroke fill property
 *
 * @param obj The object whose stroke fill property is inspected.
 * @return The object that is set as stroke fill property.
 *
 */
EAPI Efl_VG* evas_vg_shape_stroke_fill_get(const Eo *obj);

#include "canvas/efl_canvas_vg_shape_eo.legacy.h"

/**
 *
 * Set the list of color stops for the gradient
 * @since 1.14
 *
 * @param[in] colors color stops list
 * @param[in] length length of the list
 *
 */
EAPI void evas_vg_gradient_stop_set(Eo *obj, const Efl_Gfx_Gradient_Stop *colors, unsigned int length);

/**
 *
 * get the list of color stops.
 * @since 1.14
 *
 * @param[out] colors color stops list
 * @param[out] length length of the list
 *
 */
EAPI void evas_vg_gradient_stop_get(Eo *obj, const Efl_Gfx_Gradient_Stop **colors, unsigned int *length);

/**
 *
 * Specifies the spread method that should be used for this gradient.
 * @since 1.14
 *
 * @param[in] s spread type to be used
 *
 */
EAPI void evas_vg_gradient_spread_set(Eo *obj, Efl_Gfx_Gradient_Spread s);

/**
 *
 * Returns the spread method use by this gradient. The default is
 * EFL_GFX_GRADIENT_SPREAD_PAD.
 * @since 1.14
 *
 *
 */
EAPI Efl_Gfx_Gradient_Spread evas_vg_gradient_spread_get(Eo *obj);

#include "canvas/efl_canvas_vg_gradient.eo.legacy.h"

/**
 * Creates a new linear gradient object \.
 *
 * @param parent The given vector container object.
 * @return The created linear gradient object handle.
 *
 */
EAPI Efl_VG* evas_vg_gradient_linear_add(Efl_VG *parent);

/**
 *
 * Sets the start point of this linear gradient.
 *
 * @param[in] x x co-ordinate of start point
 * @param[in] y y co-ordinate of start point
 *
 */
EAPI void evas_vg_gradient_linear_start_set(Eo *obj, double x, double y);

/**
 *
 * Gets the start point of this linear gradient.
 *
 * @param[out] x x co-ordinate of start point
 * @param[out] y y co-ordinate of start point
 *
 */
EAPI void evas_vg_gradient_linear_start_get(Eo *obj, double *x, double *y);

/**
 *
 * Sets the end point of this linear gradient.
 *
 * @param[in] x x co-ordinate of end point
 * @param[in] y y co-ordinate of end point
 *
 */
EAPI void evas_vg_gradient_linear_end_set(Eo *obj, double x, double y);

/**
 *
 * Gets the end point of this linear gradient.
 *
 * @param[out] x x co-ordinate of end point
 * @param[out] y y co-ordinate of end point
 *
 */
EAPI void evas_vg_gradient_linear_end_get(Eo *obj, double *x, double *y);

#include "canvas/efl_canvas_vg_gradient_linear.eo.legacy.h"

/**
 * Creates a new radial gradient object \.
 *
 * @param parent The given vector container object.
 * @return The created radial gradient object handle.
 *
 */
EAPI Efl_VG* evas_vg_gradient_radial_add(Efl_VG *parent);

/**
 *
 * Sets the center of this radial gradient.
 *
 * @param[in] x x co-ordinate of center point
 * @param[in] y y co-ordinate of center point
 *
 */
EAPI void evas_vg_gradient_radial_center_set(Eo *obj, double x, double y);

/**
 *
 * Gets the center of this radial gradient.
 *
 * @param[out] x x co-ordinate of center point
 * @param[out] y y co-ordinate of center point
 *
 */
EAPI void evas_vg_gradient_radial_center_get(Eo *obj, double *x, double *y);

/**
 *
 * Sets the center radius of this radial gradient.
 *
 * @param[in] r center radius
 *
 */
EAPI void evas_vg_gradient_radial_radius_set(Eo *obj, double r);

/**
 *
 * Gets the center radius of this radial gradient.
 *
 *
 */
EAPI double evas_vg_gradient_radial_radius_get(Eo *obj);

/**
 *
 * Sets the focal point of this radial gradient.
 *
 * @param[in] x x co-ordinate of focal point
 * @param[in] y y co-ordinate of focal point
 *
 */
EAPI void evas_vg_gradient_radial_focal_set(Eo *obj, double x, double y);

/**
 *
 * Gets the focal point of this radial gradient.
 *
 * @param[out] x x co-ordinate of focal point
 * @param[out] y y co-ordinate of focal point
 *
 */
EAPI void evas_vg_gradient_radial_focal_get(Eo *obj, double *x, double *y);

#include "canvas/efl_canvas_vg_gradient_radial.eo.legacy.h"

/**
 * @}
 */

/**
 * @ingroup Evas_Object_Image
 *
 * @{
 */

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
 * @def EVAS_NATIVE_SURFACE_VERSION
 * Magic version number to know what the native surface struct looks like
 */

#define EVAS_NATIVE_SURFACE_VERSION 5

/**
 * Native surface types that image object supports
 *
 * @see Evas_Native_Surface
 * @see evas_object_image_native_surface_set()
 */
typedef enum _Evas_Native_Surface_Type
{
   EVAS_NATIVE_SURFACE_NONE, /**< No surface type */
   EVAS_NATIVE_SURFACE_X11,  /**< X Window system based type. pixmap id or visual of the pixmap */
   EVAS_NATIVE_SURFACE_OPENGL, /**< OpenGL system based type. texture or framebuffer id*/
   EVAS_NATIVE_SURFACE_WL, /**< Wayland system based type. buffer of surface */
   EVAS_NATIVE_SURFACE_TBM, /**< Tizen system based type. tbm surface @since 1.14  */
   EVAS_NATIVE_SURFACE_EVASGL, /**< Evas GL based type. evas gl surface @since 1.14 */
   EVAS_NATIVE_SURFACE_WL_DMABUF, /**< Wayland system based type. using dmabuf @since 1.18 */
} Evas_Native_Surface_Type;

/**
 * Native surface types that image object supports
 *
 * @see Evas_Native_Surface
 * @see evas_object_image_native_surface_set()
 */
typedef enum _Evas_Native_Surface_Status
{
   EVAS_NATIVE_SURFACE_STATUS_SCANOUT_ON,
   EVAS_NATIVE_SURFACE_STATUS_SCANOUT_OFF,
   EVAS_NATIVE_SURFACE_STATUS_PLANE_ASSIGN,
   EVAS_NATIVE_SURFACE_STATUS_PLANE_RELEASE,
} Evas_Native_Surface_Status;

typedef void (*Evas_Native_Scanout_Handler)(void *scanout_data, Evas_Native_Surface_Status status);

typedef struct _Evas_Native_Scanout
{
   Evas_Native_Scanout_Handler handler;
   void *data;
} Evas_Native_Scanout;

/**
 * @brief A generic datatype for engine specific native surface information.
 *
 * Please fill up Evas_Native_Surface fields that regarded with current surface
 * type. If you want to set the native surface type to
 * EVAS_NATIVE_SURFACE_X11, you need to set union data with x11.visual or
 * x11.pixmap. If you need to set the native surface as
 * EVAS_NATIVE_SURFACE_OPENGL, on the other hand, you need to set union data
 * with opengl.texture_id or opengl.framebuffer_id and so on.
 * If you need to set the native surface as EVAS_NATIVE_SURFACE_WL,
 * you need to set union data with wl.legacy_buffer.
 * If you need to set the native surface as EVAS_NATIVE_SURFACE_TBM,
 * you need to set union data with tbm surface. The version field
 * should be set with EVAS_NATIVE_SURFACE_VERSION in order to check abi
 * break in your application on the different efl library versions.
 *
 * @warning Native surface types totally depend on the system. Please
 *          be aware that the types are supported on your system before using
 *          them.
 *
 * @note The information stored in an @c Evas_Native_Surface returned by
 *       @ref evas_gl_native_surface_get() is not meant to be used by
 *       applications except for passing it to
 *       @ref evas_object_image_native_surface_set().
 *
 * @see evas_object_image_native_surface_set()
 */
typedef struct _Evas_Native_Surface
{
   int                      version; /**< Current Native Surface Version. Use EVAS_NATIVE_SURFACE_VERSION */
   Evas_Native_Surface_Type type; /**< Surface type. @see Evas_Native_Surface_Type */
   union {
      struct
      {
         void         *visual; /**< visual of the pixmap to use (Visual) */
         unsigned long pixmap; /**< pixmap id to use (Pixmap) */
         unsigned int  multiple_buffer; /**< From version 4. 1 if pixmap is multiple buffer pixmap such as named pixmap created by enlightenment. driver dependent. @since 1.19 */
      } x11; /**< Set this struct fields if surface data is X11 based. */

      struct
      {
         unsigned int texture_id; /**< opengl texture id to use from glGenTextures() */
         unsigned int framebuffer_id; /**< 0 if not a FBO, FBO id otherwise from glGenFramebuffers() */
         unsigned int internal_format; /**< same as 'internalFormat' for glTexImage2D() */
         unsigned int format; /**< same as 'format' for glTexImage2D() */
         unsigned int x, y, w, h; /**< region inside the texture to use (image size is assumed as texture size, with 0, 0 being the top-left and co-ordinates working down to the right and bottom being positive) */
      } opengl; /**< Set this struct fields if surface data is OpenGL based. */
      struct
      {
         void *legacy_buffer; /**< wayland client buffer to use */
      } wl; /**< Set this struct fields if surface data is Wayland based. */
      struct
      {
         void *attr; /**< Pointer to dmabuf attributes - contents copied */
         void *resource; /**< Wayland resource pointer, kept as is */
         Evas_Native_Scanout scanout;
      } wl_dmabuf; /**< Set this struct fields if surface data is Wayland dmabuf based. @since 1.18 */
      struct
      {
         void *buffer; /**< tbm surface buffer to use @since 1.14 */
      } tbm; /**< Set this struct fields if surface data is Tizen based. @since 1.14 */
      struct
      {
         void *surface; /**< evas gl surface to use @since 1.14 */
      } evasgl; /**< Set this struct fields if surface data is Evas GL based. @since 1.14 */
   } data; /**< Choose one union data according to your surface. */
} Evas_Native_Surface;

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
 * @brief Get the native surface of a given image of the canvas
 *
 * This function returns the native surface of a given canvas image.
 *
 * @return The native surface.
 */
EAPI Evas_Native_Surface          *evas_object_image_native_surface_get(const Evas_Object *obj);

/**
 * Possible orientation options for evas_object_image_orient_set().
 *
 * @since 1.14
 */
typedef enum
{
  EVAS_IMAGE_ORIENT_NONE = 0, /**< no orientation change */
  EVAS_IMAGE_ORIENT_0 = 0, /**< no orientation change */
  EVAS_IMAGE_ORIENT_90 = 1, /**< rotate 90 degrees clockwise */
  EVAS_IMAGE_ORIENT_180 = 2, /**< rotate 180 degrees clockwise */
  EVAS_IMAGE_ORIENT_270 = 3, /**< rotate 90 degrees counter-clockwise (i.e. 270 degrees clockwise) */
  EVAS_IMAGE_FLIP_HORIZONTAL = 4, /**< flip image horizontally (along the x = width / 2 line) */
  EVAS_IMAGE_FLIP_VERTICAL = 5, /**< flip image vertically (along the y = height / 2 line) */
  EVAS_IMAGE_FLIP_TRANSPOSE = 6, /**< flip image along the y = (width - x) line (bottom-left to top-right) */
  EVAS_IMAGE_FLIP_TRANSVERSE = 7 /**< flip image along the y = x line (top-left to bottom-right) */
} Evas_Image_Orient;

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
 *
 * Set the source file from where an image object must fetch the real
 * image data (it may be an Eet file, besides pure image ones).
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
 * {
 * fprintf(stderr, "could not load image '%s'. error string is \"%s\"\n",
 * valid_path, evas_load_error_str(err));
 * }
 * else
 * {
 * evas_object_image_fill_set(img, 0, 0, w, h);
 * evas_object_resize(img, w, h);
 * evas_object_show(img);
 * }
 * @endcode
 *
 * @param[in] file The image file path.
 * @param[in] key The image key in @p file (if its an Eet one), or @c
NULL, otherwise.
 */
EAPI void evas_object_image_file_set(Eo *obj, const char *file, const char *key);

/**
 *
 * Retrieve the source file from where an image object is to fetch the
 * real image data (it may be an Eet file, besides pure image ones).
 *
 * You must @b not modify the strings on the returned pointers.
 *
 * @note Use @c NULL pointers on the file components you're not
 * interested in: they'll be ignored by the function.
 *
 * @param[out] file The image file path.
 * @param[out] key The image key in @p file (if its an Eet one), or @c
NULL, otherwise.
 */
EAPI void evas_object_image_file_get(const Eo *obj, const char **file, const char **key);

/**
 *
 * Set the source mmaped file from where an image object must fetch the real
 * image data (it must be an Eina_File).
 *
 * If the file supports multiple data stored in it (as Eet files do),
 * you can specify the key to be used as the index of the image in
 * this file.
 *
 * @since 1.8
 *
 * @param[in] f The mmaped file
 * @param[in] key The image key in @p file (if its an Eet one), or @c
NULL, otherwise.
 */
EAPI void evas_object_image_mmap_set(Eo *obj, const Eina_File *f, const char *key);

/**
 *
 * Get the source mmaped file from where an image object must fetch the real
 * image data (it must be an Eina_File).
 *
 * If the file supports multiple data stored in it (as Eet files do),
 * you can get the key to be used as the index of the image in
 * this file.
 *
 * @since 1.10
 *
 * @param[out] f The mmaped file
 * @param[out] key The image key in @p file (if its an Eet one), or @c
NULL, otherwise.
 */
EAPI void evas_object_image_mmap_get(const Eo *obj, const Eina_File **f, const char **key);

/**
 *
 * Save the given image object's contents to an (image) file.
 * Proxy object is image object, but it doesn't have contents. So you can't use this function for proxy objects.
 *
 * The extension suffix on @p file will determine which <b>saver
 * module</b> Evas is to use when saving, thus the final file's
 * format. If the file supports multiple data stored in it (Eet ones),
 * you can specify the key to be used as the index of the image in it.
 *
 * You can specify some flags when saving the image.  Currently
 * acceptable flags are @c quality and @c compress. Eg.: @c
 * "quality=100 compress=9"
 *
 * quality is hint for the quality of image,0-100. 0 means low quality
 * and saved image size is small. 100 means high quality and saved image size is big.
 *
 * compress is hint for the compression modes (Eet ones) or for the compression flags (Png one)
 * (1 == compress, 0 = don't compress).
 *
 * @param[in] file The filename to be used to save the image (extension
obligatory).
 * @param[in] key The image key in the file (if an Eet one), or @c NULL,
otherwise.
 * @param[in] flags String containing the flags to be used (@c NULL for
none).
 */
EAPI Eina_Bool evas_object_image_save(const Eo *obj, const char *file, const char *key, const char *flags) EINA_ARG_NONNULL(2);

/**
 *
 * Check if an image object can be animated (have multiple frames)
 *
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
 * {
 * int frame_count;
 * int loop_count;
 * Evas_Image_Animated_Loop_Hint loop_type;
 * double duration;
 *
 * frame_count = evas_object_image_animated_frame_count_get(obj);
 * printf("This image has %d frames\n",frame_count);
 *
 * duration = evas_object_image_animated_frame_duration_get(obj,1,0);
 * printf("Frame 1's duration is %f. You had better set object's frame to 2 after this duration using timer\n");
 *
 * loop_count = evas_object_image_animated_loop_count_get(obj);
 * printf("loop count is %d. You had better run loop %d times\n",loop_count,loop_count);
 *
 * loop_type = evas_object_image_animated_loop_type_get(obj);
 * if (loop_type == EVAS_IMAGE_ANIMATED_HINT_LOOP)
 * printf("You had better set frame like 1->2->3->1->2->3...\n");
 * else if (loop_type == EVAS_IMAGE_ANIMATED_HINT_PINGPONG)
 * printf("You had better set frame like 1->2->3->2->1->2...\n");
 * else
 * printf("Unknown loop type\n");
 *
 * evas_object_image_animated_frame_set(obj,1);
 * printf("You set image object's frame to 1. You can see frame 1\n");
 * }
 * @endcode
 *
 * @see evas_object_image_animated_get()
 * @see evas_object_image_animated_frame_count_get()
 * @see evas_object_image_animated_loop_type_get()
 * @see evas_object_image_animated_loop_count_get()
 * @see evas_object_image_animated_frame_duration_get()
 * @see evas_object_image_animated_frame_set()
 * @since 1.1
 *
 */
EAPI Eina_Bool evas_object_image_animated_get(const Eo *obj);

/**
 * @brief Set the frame to current frame of an image object.
 *
 * This set image object's current frame to frame_num with 1 being the first
 * frame.
 *
 * See also @ref evas_object_image_animated_get,
 * @ref evas_object_image_animated_frame_count_get,
 * @ref evas_object_image_animated_loop_type_get,
 * @ref evas_object_image_animated_loop_count_get,
 * @ref evas_object_image_animated_frame_duration_get.
 *
 * @param[in] frame_index The index of current frame.
 *
 * @since 1.1
 */
EAPI void evas_object_image_animated_frame_set(Evas_Object *obj, int frame_index);

/**
 * @brief Get the total number of frames of the image object.
 *
 * This returns total number of frames the image object supports (if animated).
 *
 * See also @ref evas_object_image_animated_get,
 * @ref evas_object_image_animated_loop_type_get,
 * @ref evas_object_image_animated_loop_count_get,
 * @ref evas_object_image_animated_frame_duration_get.
 *
 * @return The number of frames.
 *
 * @since 1.1
 */
EAPI int evas_object_image_animated_frame_count_get(const Evas_Object *obj);

/**
 * @brief Get the kind of looping the image object does.
 *
 * This returns the kind of looping the image object wants to do.
 *
 * If it returns EVAS_IMAGE_ANIMATED_HINT_LOOP, you should display frames in a
 * sequence like: 1->2->3->1->2->3->1...
 *
 * If it returns EVAS_IMAGE_ANIMATED_HINT_PINGPONG, it is better to display
 * frames in a sequence like: 1->2->3->2->1->2->3->1...
 *
 * The default type is EVAS_IMAGE_ANIMATED_HINT_LOOP.
 *
 * See also @ref evas_object_image_animated_get,
 * @ref evas_object_image_animated_frame_count_get,
 * @ref evas_object_image_animated_loop_count_get,
 * @ref evas_object_image_animated_frame_duration_get.
 *
 * @return Loop type of the image object.
 *
 * @since 1.1
 */
EAPI Evas_Image_Animated_Loop_Hint evas_object_image_animated_loop_type_get(const Evas_Object *obj);

/**
 * @brief Get the number times the animation of the object loops.
 *
 * This returns loop count of image. The loop count is the number of times the
 * animation will play fully from first to last frame until the animation
 * should stop (at the final frame).
 *
 * If 0 is returned, then looping should happen indefinitely (no limit to the
 * number of times it loops).
 *
 * See also @ref evas_object_image_animated_get,
 * @ref evas_object_image_animated_frame_count_get,
 * @ref evas_object_image_animated_loop_type_get,
 * @ref evas_object_image_animated_frame_duration_get.
 *
 * @return The number of loop of an animated image object.
 *
 * @since 1.1
 */
EAPI int evas_object_image_animated_loop_count_get(const Evas_Object *obj);

/**
 * @brief Get the duration of a sequence of frames.
 *
 * This returns total duration that the specified sequence of frames should
 * take in seconds.
 *
 * If you set start_frame to 1 and frame_num 0, you get frame 1's duration. If
 * you set start_frame to 1 and frame_num 1, you get frame 1's duration +
 * frame2's duration.
 *
 * See also @ref evas_object_image_animated_get,
 * @ref evas_object_image_animated_frame_count_get,
 * @ref evas_object_image_animated_loop_type_get,
 * @ref evas_object_image_animated_loop_count_get,
 *
 * @param[in] frame_num Number of frames in the sequence.
 *
 * @since 1.1
 */
EAPI double evas_object_image_animated_frame_duration_get(const Evas_Object *obj, int start_frame, int frame_num);

/**
 * @brief Set the DPI resolution of an image object's source image.
 *
 * This function sets the DPI resolution of a given loaded canvas image. Most
 * useful for the SVG image loader.
 *
 * @param[in] dpi The DPI resolution.
 */
EAPI void evas_object_image_load_dpi_set(Evas_Object *obj, double dpi);

/**
 * @brief Get the DPI resolution of a loaded image object in the canvas.
 *
 * This function returns the DPI resolution of the given canvas image.
 *
 * @return The DPI resolution.
 */
EAPI double evas_object_image_load_dpi_get(const Evas_Object *obj);

/**
 *
 * Set the load size of a given image object's source image.
 *
 * This function sets a new geometry size for the given canvas image.
 * The image will be loaded into memory as if it was the set size instead of
 * the original size.
 *
 * @note The size of a given image object's source image will be less than or
 * equal to the size of @p w and @p h.
 *
 * @see evas_object_image_load_size_get()
 *
 * @param[in] w The new width of the image's load size.
 * @param[in] h The new height of the image's load size.
 */
EAPI void evas_object_image_load_size_set(Eo *obj, int w, int h);

/**
 *
 * Get the load size of a given image object's source image.
 *
 * This function gets the geometry size set manually for the given canvas image.
 *
 * @note Use @c NULL pointers on the size components you're not
 * interested in: they'll be ignored by the function.
 * @note @p w and @p h will be set with the image's loading size only if
 * the image's load size is set manually: if evas_object_image_load_size_set()
 * has not been called, @p w and @p h will be set with 0.
 *
 * @see evas_object_image_load_size_set() for more details
 *
 * @param[out] w The new width of the image's load size.
 * @param[out] h The new height of the image's load size.
 */
EAPI void evas_object_image_load_size_get(const Eo *obj, int *w, int *h);

/**
 * @brief Inform a given image object to load a selective region of its source
 * image.
 *
 * This function is useful when one is not showing all of an image's area on
 * its image object.
 *
 * @note The image loader for the image format in question has to support
 * selective region loading in order to this function to take effect.
 *
 * @param[in] x X-offset of the region to be loaded.
 * @param[in] y Y-offset of the region to be loaded.
 * @param[in] w Width of the region to be loaded.
 * @param[in] h Height of the region to be loaded.
 */
EAPI void evas_object_image_load_region_set(Evas_Object *obj, int x, int y, int w, int h);

/**
 * @brief Retrieve the coordinates of a given image object's selective (source
 * image) load region.
 *
 * @note Use @c null pointers on the coordinates you're not interested in:
 * they'll be ignored by the function.
 *
 * @param[out] x X-offset of the region to be loaded.
 * @param[out] y Y-offset of the region to be loaded.
 * @param[out] w Width of the region to be loaded.
 * @param[out] h Height of the region to be loaded.
 */
EAPI void evas_object_image_load_region_get(const Evas_Object *obj, int *x, int *y, int *w, int *h);

/**
 * @brief Get the support state of a given image.
 *
 * @return The region support state.
 *
 * @since 1.2
 *
 * @ingroup Evas_Image
 */
EAPI Eina_Bool evas_object_image_region_support_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Define if the orientation information in the image file should be
 * honored.
 *
 * @param[in] enable @c true means that it should honor the orientation
 * information.
 *
 * @since 1.1
 */
EAPI void evas_object_image_load_orientation_set(Evas_Object *obj, Eina_Bool enable);

/**
 * @brief Get if the orientation information in the image file should be
 * honored.
 *
 * @return @c true means that it should honor the orientation information.
 *
 * @since 1.1
 */
EAPI Eina_Bool evas_object_image_load_orientation_get(const Evas_Object *obj);

/**
 * @brief Set the scale down factor of a given image object's source image,
 * when loading it.
 *
 * This function sets the scale down factor of a given canvas image. Most
 * useful for the SVG image loader.
 *
 * @param[in] scale_down The scale down factor.
 */
EAPI void evas_object_image_load_scale_down_set(Evas_Object *obj, int scale_down);

/**
 * @brief Get the scale down factor of a given image object's source image,
 * when loading it.
 *
 * @return The scale down factor.
 */
EAPI int evas_object_image_load_scale_down_get(const Evas_Object *obj);

/**
 * @brief Set a load option to skip initial header load and defer to preload
 * 
 * This is meant to be used in conjunction with evas_object_image_file_set()
 * and evas_object_image_preload() by deferring any header loading until
 * a evas_object_image_preload() is issued making the file file set simply
 * set up the file to refer to without any validation of its type or
 * file existence or even inspecting the image header to get size or alpha
 * channel flags etc. All of this will then be done as part of the preload
 * stage.
 * 
 * @since 1.19
 */
EAPI void evas_object_image_load_head_skip_set(Evas_Object *obj, Eina_Bool skip);

/**
 * @breif Get the load option to skip header loads before preload
 * 
 * This gets the heade skip value set by evas_object_image_load_head_skip_set()
 * 
 * @see evas_object_image_load_head_skip_set
 * @since 1.19
 */
EAPI Eina_Bool evas_object_image_load_head_skip_get(const Evas_Object *obj);

/**
 * @brief Retrieves a number representing any error that occurred during the
 * last loading of the given image object's source image.
 *
 * @return A value giving the last error that occurred. It should be one of the
 * @ref Evas_Load_Error values. #EVAS_LOAD_ERROR_NONE is returned if there was
 * no error.
 */
EAPI Evas_Load_Error evas_object_image_load_error_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT;

/**
 *
 * Sets whether to use high-quality image scaling algorithm on the
 * given image object.
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
 *
 * @param[in] smooth_scale Whether to use smooth scale or not.
 */
EAPI void evas_object_image_smooth_scale_set(Eo *obj, Eina_Bool smooth_scale);

/**
 *
 * Retrieves whether the given image object is using high-quality
 * image scaling algorithm.
 *
 * @return Whether smooth scale is being used.
 *
 * See @ref evas_object_image_smooth_scale_set() for more details.
 *
 */
EAPI Eina_Bool evas_object_image_smooth_scale_get(const Eo *obj);

/* not implemented! removed from the interface, kept as legacy only */
typedef enum _Efl_Gfx_Fill_Spread {
   /** Fill spread mode. Warning: support is not implemented!
    * @since 1.14 */
   EFL_GFX_FILL_REFLECT = 0,          /**< Tiling reflects and repeats */
   EFL_GFX_FILL_REPEAT = 1,           /**< Tiling repeats like a mosaic */
   EFL_GFX_FILL_RESTRICT = 2,         /**< Tiling clamps, range offset ignored */
   EFL_GFX_FILL_RESTRICT_REFLECT = 3, /**< Tiling clamps and any range offset reflects */
   EFL_GFX_FILL_RESTRICT_REPEAT = 4,  /**< Tiling clamps and any range offset repeats */
   EFL_GFX_FILL_PAD = 5,              /**< Tiling extends with end values */
} Efl_Gfx_Fill_Spread;

typedef Efl_Gfx_Fill_Spread Evas_Fill_Spread;
#define EVAS_TEXTURE_REFLECT EFL_GFX_FILL_REFLECT
#define EVAS_TEXTURE_REPEAT EFL_GFX_FILL_REPEAT
#define EVAS_TEXTURE_RESTRICT EFL_GFX_FILL_RESTRICT
#define EVAS_TEXTURE_RESTRICT_REFLECT EFL_GFX_FILL_RESTRICT_REFLECT
#define EVAS_TEXTURE_RESTRICT_REPEAT EFL_GFX_FILL_RESTRICT_REPEAT
#define EVAS_TEXTURE_PAD EFL_GFX_FILL_PAD

/**
 *
 * Sets the tiling mode for the given evas image object's fill.
 * EVAS_TEXTURE_RESTRICT, or EVAS_TEXTURE_PAD.
 *
 * @param[in] spread One of EVAS_TEXTURE_REFLECT, EVAS_TEXTURE_REPEAT,
 */
EAPI void evas_object_image_fill_spread_set(Evas_Object *obj, Evas_Fill_Spread spread) EINA_DEPRECATED;

/**
 *
 * Retrieves the spread (tiling mode) for the given image object's
 * fill.
 *
 * @return  The current spread mode of the image object.
 *
 */
EAPI Evas_Fill_Spread evas_object_image_fill_spread_get(const Evas_Object *obj) EINA_DEPRECATED;

/**
 *
 * Set how to fill an image object's drawing rectangle given the
 * (real) image bound to it.
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
 *
 * @param[in] x The x coordinate (from the top left corner of the bound
image) to start drawing from.
 * @param[in] y The y coordinate (from the top left corner of the bound
image) to start drawing from.
 * @param[in] w The width the bound image will be displayed at.
 * @param[in] h The height the bound image will be displayed at.
 */
EAPI void evas_object_image_fill_set(Evas_Object *obj, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h);

/**
 *
 * Retrieve how an image object is to fill its drawing rectangle,
 * given the (real) image bound to it.
 *
 * @note Use @c NULL pointers on the fill components you're not
 * interested in: they'll be ignored by the function.
 *
 * See @ref evas_object_image_fill_set() for more details.
 *
 * @param[out] x The x coordinate (from the top left corner of the bound
image) to start drawing from.
 * @param[out] y The y coordinate (from the top left corner of the bound
image) to start drawing from.
 * @param[out] w The width the bound image will be displayed at.
 * @param[out] h The height the bound image will be displayed at.
 */
EAPI void evas_object_image_fill_get(const Evas_Object *obj, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h);

/**
 * @brief Set whether the image object's fill property should track the
 * object's size.
 *
 * If @c setting is @c true, then every @ref evas_object_resize will
 * automatically trigger a call to @ref evas_object_image_fill_set with the
 * that new size (and 0, 0 as source image's origin), so the bound image will
 * fill the whole object's area.
 *
 * @param[in] filled @c true to make the fill property follow object size or
 * @c false otherwise.
 */
EAPI void evas_object_image_filled_set(Evas_Object *obj, Eina_Bool filled);

/**
 * @brief Retrieve whether the image object's fill property should track the
 * object's size.
 *
 * Returns @c true if it is tracking, @c false if not (and @ref
 * evas_object_fill_set must be called manually).
 *
 * @return @c true to make the fill property follow object size or @c false
 * otherwise.
 */
EAPI Eina_Bool evas_object_image_filled_get(const Evas_Object *obj);

/**
 * @brief Retrieve whether alpha channel data is being used on the given image
 * object.
 *
 * This function returns @c true if the image object's alpha channel is being
 * used, or @c false otherwise.
 *
 * @return Whether to use alpha channel ($true) data or not ($false).
 *
 * @ingroup Evas_Image
 */
EAPI Eina_Bool evas_object_image_alpha_get(const Evas_Object *obj);

/**
 * @brief Enable or disable alpha channel usage on the given image object.
 *
 * This function sets a flag on an image object indicating whether or not to
 * use alpha channel data. A value of @c true makes it use alpha channel data,
 * and @c false makes it ignore that data. Note that this has nothing to do
 * with an object's color as  manipulated by @ref evas_object_color_set.
 *
 * @param[in] alpha Whether to use alpha channel ($true) data or not ($false).
 */
EAPI void evas_object_image_alpha_set(Evas_Object *obj, Eina_Bool alpha);

/**
 * @brief Dimensions of this image's border, a region that does not scale with
 * the center area.
 *
 * When EFL renders an image, its source may be scaled to fit the size of the
 * object. This function sets an area from the borders of the image inwards
 * which is not to be scaled. This function is useful for making frames and for
 * widget theming, where, for example, buttons may be of varying sizes, but
 * their border size must remain constant.
 *
 * The units used for @c l, @c r, @c t and @c b are canvas units (pixels).
 *
 * @note The border region itself may be scaled by the
 * @ref evas_object_image_border_scale_set function.
 *
 * @note By default, image objects have no borders set, i.e. @c l, @c r, @c t
 * and @c b start as 0.
 *
 * @note Similar to the concepts of 9-patch images or cap insets.
 *
 * @param[in] l The border's left width.
 * @param[in] r The border's right width.
 * @param[in] t The border's top height.
 * @param[in] b The border's bottom height.
 */
EAPI void evas_object_image_border_set(Evas_Object *obj, int l, int r, int t, int b);

/**
 * @brief Dimensions of this image's border, a region that does not scale with
 * the center area.
 *
 * When EFL renders an image, its source may be scaled to fit the size of the
 * object. This function sets an area from the borders of the image inwards
 * which is not to be scaled. This function is useful for making frames and for
 * widget theming, where, for example, buttons may be of varying sizes, but
 * their border size must remain constant.
 *
 * The units used for @c l, @c r, @c t and @c b are canvas units (pixels).
 *
 * @note The border region itself may be scaled by the
 * @ref evas_object_image_border_scale_set function.
 *
 * @note By default, image objects have no borders set, i.e. @c l, @c r, @c t
 * and @c b start as 0.
 *
 * @note Similar to the concepts of 9-patch images or cap insets.
 *
 * @param[out] l The border's left width.
 * @param[out] r The border's right width.
 * @param[out] t The border's top height.
 * @param[out] b The border's bottom height.
 */
EAPI void evas_object_image_border_get(const Evas_Object *obj, int *l, int *r, int *t, int *b);

/**
 * @brief Scaling factor applied to the image borders.
 *
 * This value multiplies the size of the @ref evas_object_image_border_get when
 * scaling an object.
 *
 * Default value is 1.0 (no scaling).
 *
 * @param[in] scale The scale factor.
 */
EAPI void evas_object_image_border_scale_set(Evas_Object *obj, double scale);

/**
 * @brief Scaling factor applied to the image borders.
 *
 * This value multiplies the size of the @ref evas_object_image_border_get when
 * scaling an object.
 *
 * Default value is 1.0 (no scaling).
 *
 * @return The scale factor.
 */
EAPI double evas_object_image_border_scale_get(const Evas_Object *obj);

/**
 * How an image's center region (the complement to the border region) should
 * be rendered by Evas
 */
typedef enum
{
  EVAS_BORDER_FILL_NONE = 0, /**< Image's center region is @b not to be rendered */
  EVAS_BORDER_FILL_DEFAULT = 1, /**< Image's center region is to be @b blended
                                 * with objects underneath it, if it has
                                 * transparency. This is the default behavior
                                 * for image objects */
  EVAS_BORDER_FILL_SOLID = 2 /**< Image's center region is to be made solid,
                              * even if it has transparency on it */
} Evas_Border_Fill_Mode;

/**
 * @brief Specifies how the center part of the object (not the borders) should
 * be drawn when EFL is rendering it.
 *
 * This function sets how the center part of the image object's source image is
 * to be drawn, which must be one of the values in @ref Evas_Border_Fill_Mode.
 * By center we mean the complementary part of that defined by
 * @ref evas_object_image_border_set. This one is very useful for making frames
 * and decorations. You would most probably also be using a filled image (as in
 * @ref Efl.Gfx.Fill.fill_auto) to use as a frame.
 *
 * The default value is
 *
 * @param[in] fill Fill mode of the center region of @c obj (a value in
 *  #Evas_Border_Fill_Mode).
 */
EAPI void evas_object_image_border_center_fill_set(Evas_Object *obj, Evas_Border_Fill_Mode fill);

/**
 * @brief Specifies how the center part of the object (not the borders) should
 * be drawn when EFL is rendering it.
 *
 * This function sets how the center part of the image object's source image is
 * to be drawn, which must be one of the values in @ref Evas_Border_Fill_Mode.
 * By center we mean the complementary part of that defined by
 * @ref evas_object_image_border_set. This one is very useful for making frames
 * and decorations. You would most probably also be using a filled image (as in
 * @ref Efl.Gfx.Fill.fill_auto) to use as a frame.
 *
 * The default value is
 *
 * @return Fill mode of the center region of @c obj (a value in
 *  #Evas_Border_Fill_Mode).
 */
EAPI Evas_Border_Fill_Mode evas_object_image_border_center_fill_get(const Evas_Object *obj);

/**
 * @brief Set the image orientation.
 *
 * This function allows to rotate or flip the image.
 *
 * @param[in] orient The image orientation @ref Evas_Image_Orient. Default is
 * #EVAS_IMAGE_ORIENT_NONE.
 *
 * @since 1.14
 *
 * @ingroup Evas_Image
 */
EAPI void evas_object_image_orient_set(Evas_Object *obj, Evas_Image_Orient orient);

/**
 * @brief Get the image orientation.
 *
 * @return The image orientation @ref Evas_Image_Orient. Default is
 * #EVAS_IMAGE_ORIENT_NONE.
 *
 * @since 1.14
 *
 * @ingroup Evas_Image
 */
EAPI Evas_Image_Orient evas_object_image_orient_get(const Evas_Object *obj);

/**
 * @brief Set the content hint setting of a given image object of the canvas.
 *
 * This function sets the content hint value of the given image of the canvas.
 * For example, if you're on the GL engine and your driver implementation
 * supports it, setting this hint to #EVAS_IMAGE_CONTENT_HINT_DYNAMIC will make
 * it need zero copies at texture upload time, which is an "expensive"
 * operation.
 *
 * @param[in] hint The content hint value, one of the
 * @ref Evas_Image_Content_Hint ones.
 */
EAPI void evas_object_image_content_hint_set(Evas_Object *obj, Evas_Image_Content_Hint hint);

/**
 * @brief Get the content hint setting of a given image object of the canvas.
 *
 * This returns #EVAS_IMAGE_CONTENT_HINT_NONE on error.
 *
 * @return The content hint value, one of the @ref Evas_Image_Content_Hint
 * ones.
 */
EAPI Evas_Image_Content_Hint evas_object_image_content_hint_get(const Evas_Object *obj);

/**
 * @brief Set the scale hint of a given image of the canvas.
 *
 * This function sets the scale hint value of the given image object in the
 * canvas, which will affect how Evas is to cache scaled versions of its
 * original source image.
 *
 * @param[in] hint The scale hint, a value in @ref Evas_Image_Scale_Hint.
 *
 * @ingroup Evas_Image
 */
EAPI void evas_object_image_scale_hint_set(Evas_Object *obj, Evas_Image_Scale_Hint hint);

/**
 * @brief Get the scale hint of a given image of the canvas.
 *
 * This function returns the scale hint value of the given image object of the
 * canvas.
 *
 * @return The scale hint, a value in @ref Evas_Image_Scale_Hint.
 *
 * @ingroup Evas_Image
 */
EAPI Evas_Image_Scale_Hint evas_object_image_scale_hint_get(const Evas_Object *obj);

/**
 *
 * Sets the size of the given image object.
 *
 * This function will scale down or crop the image so that it is
 * treated as if it were at the given size. If the size given is
 * smaller than the image, it will be cropped. If the size given is
 * larger, then the image will be treated as if it were in the upper
 * left hand corner of a larger image that is otherwise transparent.
 *
 * @param[in] w The new width of the image.
 * @param[in] h The new height of the image.
 */
EAPI void evas_object_image_size_set(Evas_Object *obj, int w, int h);

/**
 *
 * Retrieves the size of the given image object.
 *
 * See @ref evas_object_image_size_set() for more details.
 *
 * @param[out] w The new width of the image.
 * @param[out] h The new height of the image.
 */
EAPI void evas_object_image_size_get(const Evas_Object *obj, int *w, int *h);

/**
 * @brief Set the colorspace of a given image of the canvas.
 *
 * This function sets the colorspace of given canvas image.
 *
 * @param[in] cspace The new color space.
 *
 * @ingroup Evas_Image
 */
EAPI void evas_object_image_colorspace_set(Evas_Object *obj, Evas_Colorspace cspace);

/**
 * @brief Get the colorspace of a given image of the canvas.
 *
 * This function returns the colorspace of given canvas image.
 *
 * @return The new color space.
 *
 * @ingroup Evas_Image
 */
EAPI Evas_Colorspace evas_object_image_colorspace_get(const Evas_Object *obj);

/**
 * @brief Retrieves the row stride of the given image object.
 *
 * The row stride is the number of bytes between the start of a row and the
 * start of the next row for image data.
 *
 * @return The stride of the image (in bytes).
 *
 * @ingroup Evas_Image
 */
EAPI int evas_object_image_stride_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Replaces the raw image data of the given image object.
 *
 * This function lets the application replace an image object's internal pixel
 * buffer with an user-allocated one. For best results, you should generally
 * first call @ref evas_object_image_size_set with the width and height for the
 * new buffer.
 *
 * This call is best suited for when you will be using image data with
 * different dimensions than the existing image data, if any. If you only need
 * to modify the existing image in some fashion, then using
 * @ref evas_object_image_data_get is probably what you are after.
 *
 * Note that the caller is responsible for freeing the buffer when finished
 * with it, as user-set image data will not be automatically freed when the
 * image object is deleted.
 *
 * @param[in] data The raw data to replace.
 *
 * @ingroup Evas_Image
 */
EAPI void evas_object_image_data_copy_set(Evas_Object *obj, void *data);

/**
 * @brief Sets the raw image data of the given image object.
 *
 * Note that the raw data must be of the same size (see @ref
 * evas_object_image_size_set, which has to be called before this one) and
 * colorspace (see @Evas.Image.colorspace.set) of the image. If data is
 * @c null, the current image data will be freed. Naturally, if one does not
 * set an image object's data manually, it will still have one, allocated by
 * Evas.
 *
 * @param[in] data The raw data, or @c null.
 *
 * @ingroup Evas_Image
 */
EAPI void evas_object_image_data_set(Evas_Object *obj, void *data);

/**
 * @brief Get a pointer to the raw image data of the given image object.
 *
 * This function returns a pointer to an image object's internal pixel buffer,
 * for reading only or read/write. If you request it for writing, the image
 * will be marked dirty so that it gets redrawn at the next update.
 *
 * Each time you call this function on an image object, its data buffer will
 * have an internal reference counter incremented. Decrement it back by using
 * @ref evas_object_image_data_set.
 *
 * This is best suited for when you want to modify an existing image, without
 * changing its dimensions.
 *
 * @note The contents' format returned by it depend on the color space of the
 * given image object.
 *
 * @note You may want to use @Evas.Image.data_update_add to inform data
 * changes, if you did any.
 *
 * @param[in] for_writing Whether the data being retrieved will be modified
 * ($true) or not ($false).
 *
 * @return The raw image data.
 *
 * @ingroup Evas_Image
 */
EAPI void *evas_object_image_data_get(const Evas_Object *obj, Eina_Bool for_writing) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Mark a sub-region of the given image object to be redrawn.
 *
 * This function schedules a particular rectangular region of an image object
 * to be updated (redrawn) at the next rendering cycle.
 *
 * @param[in] y Y-offset of the region to be updated.
 * @param[in] w Width of the region to be updated.
 * @param[in] h Height of the region to be updated.
 */
EAPI void evas_object_image_data_update_add(Evas_Object *obj, int x, int y, int w, int h);

/**
 * @brief The content below the Evas_Object_Image will be rendered inside it
 * and you can reuse it as a source for any kind of effect.
 *
 * @param[in] s Wether to put the content of the canvas below inside the
 * Evas_Object_Image.
 *
 * @since 1.15
 */
EAPI void evas_object_image_snapshot_set(Evas_Object *obj, Eina_Bool s);

/**
 * @brief Determine wether the Evas_Object_Image replicate the content of the
 * canvas below.
 *
 * @return Wether to put the content of the canvas below inside the
 * Evas_Object_Image.
 *
 * @since 1.15
 */
EAPI Eina_Bool evas_object_image_snapshot_get(const Evas_Object *obj);

/**
 * @brief Set the source object on an image object to used as a proxy.
 *
 * If an image object is set to behave as a proxy, it will mirror the rendering
 * contents of a given source object in its drawing region, without affecting
 * that source in any way. The source must be another valid Evas object. Other
 * effects may be applied to the proxy, such as a map (see @ref
 * evas_object_map_set) to create a reflection of the original object (for
 * example).
 *
 * Any existing source object on @c obj will be removed after this call.
 * Setting @c src to @c null clears the proxy object (not in "proxy state"
 * anymore).
 *
 * @warning You cannot set a proxy as another proxy's source.
 *
 * @param[in] src Source object to use for the proxy.
 */
EAPI Eina_Bool evas_object_image_source_set(Evas_Object *obj, Evas_Object *src);

/**
 * @brief Get the current source object of an image object.
 *
 * @return Source object to use for the proxy.
 */
EAPI Evas_Object *evas_object_image_source_get(const Evas_Object *obj);

/**
 * @brief Clip the proxy object with the source object's clipper.
 *
 * @param[in] source_clip Whether @c obj is clipped by the source clipper
 * ($true) or not ($false).
 *
 * @since 1.8
 */
EAPI void evas_object_image_source_clip_set(Evas_Object *obj, Eina_Bool source_clip);

/**
 * @brief Determine whether an object is clipped by source object's clipper.
 *
 * @return Whether @c obj is clipped by the source clipper ($true) or not
 * ($false).
 *
 * @since 1.8
 */
EAPI Eina_Bool evas_object_image_source_clip_get(const Evas_Object *obj);

/**
 * @brief Set whether an Evas object is to source events.
 *
 * Set whether an Evas object is to repeat events to source.
 *
 * If @c repeat is @c true, it will make events on @c obj to also be repeated
 * for the source object (see @ref evas_object_image_source_set). Even the
 * @c obj and source geometries are different, the event position will be
 * transformed to the source object's space.
 *
 * If @c repeat is @c false, events occurring on @c obj will be processed only
 * on it.
 *
 * @param[in] source Whether @c obj is to pass events ($true) or not ($false).
 *
 * @since 1.8
 */
EAPI void evas_object_image_source_events_set(Evas_Object *obj, Eina_Bool repeat);

/**
 * @brief Determine whether an object is set to source events.
 *
 * @return Whether @c obj is to pass events ($true) or not ($false).
 *
 * @since 1.8
 */
EAPI Eina_Bool evas_object_image_source_events_get(const Evas_Object *obj);

/**
 * @brief Set the source object to be visible or not.
 *
 * If @c visible is set to @c false, the source object of the proxy ($obj) will
 * be invisible.
 *
 * This API works differently to @ref evas_object_show and @ref
 * evas_object_hide. Once source object is hidden, the proxy object will be
 * hidden as well. Actually in this case both objects are excluded from the
 * Evas internal update circle.
 *
 * By this API, instead, one can toggle the visibility of a proxy's source
 *  object remaining the proxy visibility untouched.
 *
 * @warning If the all of proxies are deleted, then the source visibility of
 * the source object will be cancelled.
 *
 * @param[in] visible @c true is source object to be shown, @c false otherwise.
 *
 * @deprecated Please use evas_object_norender_get() on the source instead.
 *
 * @since 1.8
 */
EAPI void evas_object_image_source_visible_set(Evas_Object *obj, Eina_Bool visible);

/**
 * @brief Get the state of the source object visibility.
 *
 * @return @c true is source object to be shown, @c false otherwise.
 *
 * @deprecated Please use evas_object_norender_get() on the source instead.
 *
 * @since 1.8
 */
EAPI Eina_Bool evas_object_image_source_visible_get(const Evas_Object *obj);

/**
 * @brief Mark whether the given image object is dirty and needs to request its
 * pixels.
 *
 * This function will only properly work if a pixels get callback has been set.
 *
 * @warning Use this function if you really know what you are doing.
 *
 * @param[in] dirty Whether the image is dirty.
 */
EAPI void evas_object_image_pixels_dirty_set(Evas_Object *obj, Eina_Bool dirty);

/**
 * @brief Retrieves whether the given image object is dirty (needs to be
 * redrawn).
 *
 * @return Whether the image is dirty.
 */
EAPI Eina_Bool evas_object_image_pixels_dirty_get(const Evas_Object *obj);

/**
 * @brief Set the callback function to get pixels from a canvas' image.
 *
 * This functions sets a function to be the callback function that get pixels
 * from a image of the canvas.
 *
 * @param[in] func The callback function.
 * @param[in] data The data pointer to be passed to @c func.
 */
EAPI void evas_object_image_pixels_get_callback_set(Evas_Object *obj, Evas_Object_Image_Pixels_Get_Cb func, void *data) EINA_ARG_NONNULL(2);


/**
 * @typedef Evas_Video_Surface
 *
 * A generic datatype for video specific surface information
 * @see evas_object_image_video_surface_set
 * @see evas_object_image_video_surface_get
 * @since 1.1
 */
typedef struct _Evas_Video_Surface Evas_Video_Surface;

/**
 * @def EVAS_VIDEO_SURFACE_VERSION
 * Magic version number to know what the video surf struct looks like
 * @since 1.1
 */
#define EVAS_VIDEO_SURFACE_VERSION 1

typedef void (*Evas_Video_Cb)(void *data, Evas_Object *obj, const Evas_Video_Surface *surface);  /**< Evas video callback function signature */
typedef void (*Evas_Video_Coord_Cb)(void *data, Evas_Object *obj, const Evas_Video_Surface *surface, Evas_Coord a, Evas_Coord b);  /**< Evas video coordinates callback function signature */

struct _Evas_Video_Surface
{
   int                 version; /**< The Evas Video surface version in use @see EVAS_VIDEO_SURFACE_VERSION*/

   Evas_Video_Coord_Cb move; /**< Move the video surface to this position */
   Evas_Video_Coord_Cb resize; /**< Resize the video surface to that size */
   Evas_Video_Cb       show; /**< Show the video overlay surface */
   Evas_Video_Cb       hide; /**< Hide the video overlay surface */
   Evas_Video_Cb       update_pixels; /**< Please update the Evas_Object_Image pixels when called */

   Evas_Object        *parent; /**< The parent object */
   void               *data;
};

/**
 * Enum values for the Video surface capabilities
 * @see evas_object_image_video_surface_caps_get()
 * @see evas_object_image_video_surface_caps_set()
 */

typedef enum _Evas_Video_Surface_Caps
{
   EVAS_VIDEO_SURFACE_MOVE = 1,   /**< Move capability */
   EVAS_VIDEO_SURFACE_RESIZE = 2,   /**< Resize capability */
   EVAS_VIDEO_SURFACE_CLIP = 4,   /**< Clip capability */
   EVAS_VIDEO_SURFACE_BELOW = 8,   /**< Below capability */
   EVAS_VIDEO_SURFACE_STACKING_CHECK = 16,   /**< Stacking capability */
   EVAS_VIDEO_SURFACE_IGNORE_WINDOW = 32,   /**< Ignore window capability */
} Evas_Video_Surface_Caps;

/**
 * @brief Set the video surface linked to a given image of the canvas.
 *
 * @param[in] surf The new video surface.
 *
 * @since 1.1
 */
EAPI void evas_object_image_video_surface_set(Evas_Object *obj, Evas_Video_Surface *surf);

/**
 * @brief Get the video surface linekd to a given image of the canvas.
 *
 * @return The new video surface.
 *
 * @since 1.1
 */
EAPI const Evas_Video_Surface *evas_object_image_video_surface_get(const Evas_Object *obj);

/**
 * @brief Set the video surface capabilities to a given image of the canvas.
 *
 * @param[in] caps
 */
EAPI void evas_object_image_video_surface_caps_set(Evas_Object *obj, unsigned int caps);

/** Get the video surface capabilities to a given image of the canvas.
 */
EAPI unsigned int evas_object_image_video_surface_caps_get(const Evas_Object *obj);


/*
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
 * @return data A newly allocated data in the format specified by to_cspace.
 *
 * @param[in] to_cspace The colorspace to which the image raw data will be converted.
 */
/** @deprecated evas_object_image_data_convert */
EAPI void *evas_object_image_data_convert(Evas_Object *obj, Evas_Colorspace to_cspace) EINA_WARN_UNUSED_RESULT EINA_DEPRECATED;

/*
 * Import pixels from given source to a given canvas image object.
 *
 * This function imports pixels from a given source to a given canvas image.
 *
 * @param[in] pixels The pixel's source to be imported.
 */
/** @deprecated evas_object_image_pixels_import */
EAPI Eina_Bool evas_object_image_pixels_import(Evas_Object *obj, Evas_Pixel_Import_Source *pixels) EINA_ARG_NONNULL(2) EINA_DEPRECATED;

/*
 * Reload an image object's image data.
 *
 * This function reloads the image data bound to image object @p obj.
 */
/** @deprecated evas_object_image_reload */
EAPI void evas_object_image_reload(Evas_Object *obj) EINA_DEPRECATED;

/**
 * @deprecated This function has never been implemented. Please use
 *             evas_object_clip_set() with an alpha or RGBA image instead
 *             of setting this flag.
 */
EAPI void evas_object_image_alpha_mask_set(Evas_Object *obj, Eina_Bool ismask) EINA_ARG_NONNULL(1) EINA_DEPRECATED;

#include "canvas/evas_image_eo.legacy.h"

/**
 * @}
 */

/**
 * @ingroup Evas_Object_Text
 *
 * @{
 */

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
 *
 * Sets the text string to be displayed by the given text object.
 *
 * @see evas_object_text_text_get()
 *
 * @param[in] text Text string to display on it.
 */
EAPI void evas_object_text_text_set(Eo *obj, const char *text);

/**
 *
 * Retrieves the text string currently being displayed by the given
 * text object.
 *
 * @return The text string currently being displayed on it.
 *
 * @note Do not free() the return value.
 *
 * @see evas_object_text_text_set()
 *
 */
EAPI const char *evas_object_text_text_get(const Eo *obj);

/**
 * Types of styles to be applied on text objects. The
 * EVAS_TEXT_STYLE_SHADOW_DIRECTION_* ones are to be OR'ed together with others
 * imposing shadow, to change the shadow's direction.
 *
 * @see evas_object_text_style_set()
 * @see evas_object_text_style_get()
 */
typedef enum
{
   EVAS_TEXT_STYLE_PLAIN = 0, /**< plain, standard text */
   EVAS_TEXT_STYLE_SHADOW, /**< text with shadow underneath */
   EVAS_TEXT_STYLE_OUTLINE, /**< text with an outline */
   EVAS_TEXT_STYLE_SOFT_OUTLINE, /**< text with a soft outline */
   EVAS_TEXT_STYLE_GLOW, /**< text with a glow effect */
   EVAS_TEXT_STYLE_OUTLINE_SHADOW, /**< text with both outline and shadow effects */
   EVAS_TEXT_STYLE_FAR_SHADOW, /**< text with (far) shadow underneath */
   EVAS_TEXT_STYLE_OUTLINE_SOFT_SHADOW, /**< text with outline and soft shadow effects combined */
   EVAS_TEXT_STYLE_SOFT_SHADOW, /**< text with (soft) shadow underneath */
   EVAS_TEXT_STYLE_FAR_SOFT_SHADOW, /**< text with (far soft) shadow underneath */

   // Shadow direction modifiers
   EVAS_TEXT_STYLE_SHADOW_DIRECTION_BOTTOM_RIGHT = 0 /* 0 >> 4 */, /**< shadow growing to bottom right */
   EVAS_TEXT_STYLE_SHADOW_DIRECTION_BOTTOM = 16 /* 1 >> 4 */, /**< shadow growing to the bottom */
   EVAS_TEXT_STYLE_SHADOW_DIRECTION_BOTTOM_LEFT = 32 /* 2 >> 4 */, /**< shadow growing to bottom left */
   EVAS_TEXT_STYLE_SHADOW_DIRECTION_LEFT = 48 /* 3 >> 4 */, /**< shadow growing to the left */
   EVAS_TEXT_STYLE_SHADOW_DIRECTION_TOP_LEFT = 64 /* 4 >> 4 */, /**< shadow growing to top left */
   EVAS_TEXT_STYLE_SHADOW_DIRECTION_TOP = 80 /* 5 >> 4 */, /**< shadow growing to the top */
   EVAS_TEXT_STYLE_SHADOW_DIRECTION_TOP_RIGHT = 96 /* 6 >> 4 */, /**< shadow growing to top right */
   EVAS_TEXT_STYLE_SHADOW_DIRECTION_RIGHT = 112 /* 7 >> 4 */ /**< shadow growing to the right */
} Evas_Text_Style_Type;

#include "canvas/evas_text_eo.legacy.h"

/**
 *
 * Set the font (source) file to be used on a given text object.
 *
 * This function allows the font file to be explicitly set for a given
 * text object, overriding system lookup, which will first occur in
 * the given file's contents.
 *
 * @see evas_object_text_font_get()
 *
 * @param[in] font_source The font file's path.
 */
EAPI void evas_object_text_font_source_set(Eo *obj, const char *font_source);

/**
 *
 * Get the font file's path which is being used on a given text
 * object.
 *
 * @return The font file's path.
 *
 * @see evas_object_text_font_get() for more details
 *
 */
EAPI const char *evas_object_text_font_source_get(const Eo *obj);

/**
 *
 * Set the font family or filename, and size on a given text object.
 *
 * This function allows the font name and size of a text object to be
 * set. The @p font string has to follow fontconfig's convention on
 * naming fonts, as it's the underlying library used to query system
 * fonts by Evas (see the @c fc-list command's output, on your system,
 * to get an idea). Alternatively, one can use a full path to a font file.
 *
 * @see evas_object_text_font_get()
 * @see evas_object_text_font_source_set()
 *
 * @param[in] font The font family name or filename.
 * @param[in] size The font size, in points.
 */
EAPI void evas_object_text_font_set(Eo *obj, const char *font, Evas_Font_Size size);

/**
 *
 * Retrieve the font family and size in use on a given text object.
 *
 * This function allows the font name and size of a text object to be
 * queried. Be aware that the font name string is still owned by Evas
 * and should @b not have free() called on it by the caller of the
 * function.
 *
 * @see evas_object_text_font_set()
 *
 * @param[out] font The font family name or filename.
 * @param[out] size The font size, in points.
 */
EAPI void evas_object_text_font_get(const Eo *obj, const char **font, Evas_Font_Size *size);


/**
 * @}
 */

#include "canvas/evas_textblock_legacy.h"

/**
 * @ingroup Evas_Object_Grid
 *
 * @{
 */
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

#include "canvas/evas_textgrid_eo.legacy.h"

/**
 *
 * @brief Set the font (source) file to be used on a given textgrid object.
 *
 * This function allows the font file @p font_source to be explicitly
 * set for the textgrid object @p obj, overriding system lookup, which
 * will first occur in the given file's contents. If @p font_source is
 * @c NULL or is an empty string, or the same font_source has already
 * been set, or on error, this function does nothing.
 *
 * @see evas_object_textgrid_font_get()
 * @see evas_object_textgrid_font_set()
 * @see evas_object_textgrid_font_source_get()
 *
 * @since 1.7
 *
 * @param[in] font_source The font file's path.
 */
EAPI void evas_object_textgrid_font_source_set(Eo *obj, const char *font_source);

/**
 *
 * @brief Get the font file's path which is being used on a given textgrid object.
 *
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
 *
 */
EAPI const char *evas_object_textgrid_font_source_get(const Eo *obj);

/**
 *
 * @brief Set the font family and size on a given textgrid object.
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
 *
 * @param[in] font_name The font (family) name.
 * @param[in] font_size The font size, in points.
 */
EAPI void evas_object_textgrid_font_set(Eo *obj, const char *font_name, Evas_Font_Size font_size);

/**
 *
 * @brief Retrieve the font family and size in use on a given textgrid object.
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
 *
 * @param[out] font_name The font (family) name.
 * @param[out] font_size The font size, in points.
 */
EAPI void evas_object_textgrid_font_get(const Eo *obj, const char **font_name, Evas_Font_Size *font_size);

/**
 * @}
 */

/**
 * @ingroup Evas_Object_Line
 *
 * @{
 */
/**
 * Adds a new evas line object to the given evas.
 * @param   e The given evas.
 * @return  The new evas line object.
 */
EAPI Evas_Object *evas_object_line_add(Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

#include "canvas/evas_line_eo.legacy.h"

/**
 * @}
 */

/**
 * @ingroup Evas_Object_Polygon
 *
 * @{
 */
/**
 * Adds a new evas polygon object to the given evas.
 * @param   e The given evas.
 * @return  A new evas polygon object.
 */
EAPI Evas_Object *evas_object_polygon_add(Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * @brief Adds the given point to the given evas polygon object.
 *
 * @param[in] y The Y coordinate of the given point.
 *
 * @ingroup Evas_Polygon
 */
EAPI void evas_object_polygon_point_add(Evas_Object *obj, Evas_Coord x, Evas_Coord y);

/** Removes all of the points from the given evas polygon object.
 *
 * @ingroup Evas_Polygon
 */
EAPI void evas_object_polygon_points_clear(Evas_Object *obj);

/**
 * @}
 */


/**
 * @}
 */


/**
 * @ingroup Evas_Smart_Object_Group
 *
 * @{
 */


/** Call user-provided @c calculate smart functions and unset the flag
 * signalling that the object needs to get recalculated to all smart objects in
 * the canvas.
 *
 * @ingroup Evas_Canvas
 */
EAPI void evas_smart_objects_calculate(Eo *obj);

/**
 * @brief Get if the canvas is currently calculating smart objects.
 *
 * @param[in] obj The object.
 *
 * @return @c true if currently calculating smart objects.
 *
 * @ingroup Evas_Canvas
 */
EAPI Eina_Bool evas_smart_objects_calculating_get(const Eo *obj);

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
EAPI const void       *evas_object_smart_interface_get(const Evas_Object *obj, const char *name);

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
EAPI void             *evas_object_smart_interface_data_get(const Evas_Object *obj, const Evas_Smart_Interface *iface);

/**
 * @brief Checks whether a given smart object or any of its smart object
 * parents is of a given smart class.
 *
 * If @c obj is not a smart object, this call will fail immediately.
 *
 * This function supports Eo and legacy inheritance mechanisms. However, it is
 * recommended to use @ref efl_isa instead if your object is using Eo from top
 * to bottom.
 *
 * The checks use smart classes names and string comparison. There is a version
 * of this same check using pointer comparison, since a smart class' name is a
 * single string in Evas.
 *
 * See also @ref evas_object_smart_type_check_ptr.
 *
 * @param[in] type The name (type) of the smart class to check for.
 *
 * @ingroup Evas_Object
 */
EAPI Eina_Bool evas_object_smart_type_check(const Evas_Object *obj, const char *type) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(2);

/**
 * @brief Checks whether a given smart object or any of its smart object
 * parents is of a given smart class, using pointer comparison.
 *
 * @param[in] type The type (name string) to check for. Must be the name.
 *
 * @return @c true if @c obj or any of its parents is of type @c type, @c false
 * otherwise.
 *
 * @ingroup Evas_Object
 */
EAPI Eina_Bool evas_object_smart_type_check_ptr(const Evas_Object *obj, const char *type) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(2);

/**
 * @brief Set an smart object instance's smart callbacks descriptions.
 *
 * These descriptions are hints to be used by introspection and are not
 * enforced in any way.
 *
 * It will not be checked if instance callbacks descriptions have the same name
 * as respective possibly registered in the smart object class. Both are kept
 * in different arrays and users of
 * evas_object_smart_callbacks_descriptions_get() should handle this case as
 * they wish.
 *
 * @note Because @c descriptions must be @c null terminated, and because a
 * @c null name makes little sense, too, Evas_Smart_Cb_Description.name must
 * not be @c null.
 *
 * @note While instance callbacks descriptions are possible, they are not
 * recommended. Use class callbacks descriptions instead as they make you smart
 * object user's life simpler and will use less memory, as descriptions and
 * arrays will be shared among all instances.
 *
 * @param[in] descriptions @c null terminated array with
 * @ref Evas_Smart_Cb_Description descriptions. Array elements won't be
 * modified at run time, but references to them and their contents will be
 * made, so this array should be kept alive during the whole object's lifetime.
 *
 * @return @c true on success, @c false on failure.
 *
 * @ingroup Evas_Object_Smart
 */
EAPI Eina_Bool evas_object_smart_callbacks_descriptions_set(Evas_Object *obj, const Evas_Smart_Cb_Description *descriptions);

/**
 * @brief Retrieve an smart object's know smart callback descriptions (both
 * instance and class ones).
 *
 * This call searches for registered callback descriptions for both instance
 * and class of the given smart object. These arrays will be sorted by
 * Evas_Smart_Cb_Description.name and also @c null terminated, so both
 * class_count and instance_count can be ignored, if the caller wishes so. The
 * terminator @c null is not counted in these values.
 *
 * @note If just class descriptions are of interest, try
 * evas_smart_callbacks_descriptions_get() instead.
 *
 * @note Use @c null pointers on the descriptions/counters you're not
 * interested in: they'll be ignored by the function.
 *
 * @ref evas_smart_callbacks_descriptions_get().
 *
 * @param[out] class_descriptions Where to store class callbacks descriptions
 * array, if any is known. If no descriptions are known, @c null is returned.
 * @param[out] class_count Returns how many class callbacks descriptions are
 * known.
 * @param[out] instance_descriptions Where to store instance callbacks
 * descriptions array, if any is known. If no descriptions are known, @c null
 * is returned.
 * @param[out] instance_count Returns how many instance callbacks descriptions
 * are known.
 *
 * @ingroup Evas_Object_Smart
 */
EAPI void evas_object_smart_callbacks_descriptions_get(const Evas_Object *obj, const Evas_Smart_Cb_Description ***class_descriptions, unsigned int *class_count, const Evas_Smart_Cb_Description ***instance_descriptions, unsigned int *instance_count);

/**
 * @brief Find callback description for callback called name or @c null if not
 * found.
 *
 * If parameter is @c null, no search will be done on instance descriptions.
 *
 * @param[in] name name of desired callback, must not be @c null.  The search
 * have a special case for name being the same pointer as registered with
 * Evas_Smart_Cb_Description, one can use it to avoid excessive use of
 * strcmp().
 * @param[out] class_description pointer to return class description or @c null
 * if not found. If parameter is @c null, no search will be done on class
 * descriptions.
 * @param[out] instance_description pointer to return instance description.
 *
 * @ingroup Evas_Object_Smart
 */
EAPI void evas_object_smart_callback_description_find(const Evas_Object *obj, const char *name, const Evas_Smart_Cb_Description **class_description, const Evas_Smart_Cb_Description **instance_description) EINA_ARG_NONNULL(2);

/**
 * @brief Get the @ref Evas_Smart from which @c obj smart object was created.
 *
 * @return the @ref Evas_Smart handle or @c null, on errors.
 */
EAPI Evas_Smart *evas_object_smart_smart_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Store a pointer to user data for a given smart object.
 *
 * This data is stored independently of the one set by evas_object_data_set(),
 * naturally.
 *
 * See also @ref evas_object_smart_data_get.
 *
 * @param[in] data A pointer to user data.
 *
 * @ingroup Evas_Object_Smart
 */
EAPI void evas_object_smart_data_set(Evas_Object *obj, void *data);
EAPI void *evas_object_smart_data_get(const Evas_Object *obj);

/**
 * @brief Get the clipper object for the given clipped smart object.
 *
 * Use this function if you want to change any of this clipper's properties,
 * like colors.
 *
 * @ingroup Evas_Object_Smart
 */
EAPI Evas_Object *evas_object_smart_clipped_clipper_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Retrieves the list of the member objects of a given Evas smart
 * object.
 *
 * The returned list should be freed with @c eina_list_free() when you no
 * longer need it.
 *
 * This function will return @c null when a non-smart object is passed.
 *
 * See also @ref Efl.Canvas.Group.group_member_add,
 * @ref Efl.Canvas.Group.group_member_remove and @ref evas_object_smart_iterator_new.
 *
 * @return Returns the list of the member objects of @c obj.
 *
 * @since 1.7
 *
 * @ingroup Evas_Object_Smart
 */
EAPI Eina_List *evas_object_smart_members_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Set or unset the flag signalling that a given smart object needs to
 * get recalculated.
 *
 * If this flag is set, then the @c calculate() smart function of @c obj will
 * be called, if one is provided, during rendering phase of Evas (see
 * evas_render()), after which this flag will be automatically unset.
 *
 * If that smart function is not provided for the given object, this flag will
 * be left unchanged.
 *
 * @note just setting this flag will not make the canvas' whole scene dirty, by
 * itself, and evas_render() will have no effect. To force that, use
 * evas_object_smart_changed(), that will also call this function
 * automatically, with @c true as parameter.
 *
 * See also @ref evas_object_smart_need_recalculate_get,
 * @ref evas_object_smart_calculate and @ref evas_smart_objects_calculate().
 *
 * @param[in] value whether one wants to set ($true) or to unset ($false) the
 * flag.
 *
 * @ingroup Evas_Object_Smart
 */
EAPI void evas_object_smart_need_recalculate_set(Evas_Object *obj, Eina_Bool value);

/**
 * @brief Get the value of the flag signalling that a given smart object needs
 * to get recalculated.
 *
 * @note this flag will be unset during the rendering phase, when the
 * @c calculate() smart function is called, if one is provided. If it's not
 * provided, then the flag will be left unchanged after the rendering phase.
 *
 * See also @ref evas_object_smart_need_recalculate_set, for more details.
 *
 * @return whether one wants to set ($true) or to unset ($false) the flag.
 *
 * @ingroup Evas_Object_Smart
 */
EAPI Eina_Bool evas_object_smart_need_recalculate_get(const Evas_Object *obj);

/**
 * @brief Retrieves an iterator of the member objects of a given Evas smart
 * object.
 *
 * See also @ref Efl.Canvas.Group.group_member_add and
 * @ref Efl.Canvas.Group.group_member_remove
 *
 * @return Returns the iterator of the member objects of @c obj.
 *
 * @since 1.8
 *
 * @ingroup Evas_Object_Smart
 */
EAPI Eina_Iterator *evas_object_smart_iterator_new(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Call the calculate() smart function immediately on a given smart
 * object.
 *
 * This will force immediate calculations (see #Evas_Smart_Class) needed for
 * renderization of this object and, besides, unset the flag on it telling it
 * needs recalculation for the next rendering phase.
 *
 * See also @ref evas_object_smart_need_recalculate_set
 *
 * @ingroup Evas_Object_Smart
 */
EAPI void evas_object_smart_calculate(Evas_Object *obj);

/**
 * @brief Mark smart object as changed, dirty.
 *
 * This will flag the given object as needing recalculation, forcefully. As an
 * effect, on the next rendering cycle its calculate() (see #Evas_Smart_Class)
 * smart function will be called.
 *
 * See also @ref evas_object_smart_need_recalculate_set and
 * @ref evas_object_smart_calculate.
 *
 * @ingroup Evas_Object_Smart
 */
EAPI void evas_object_smart_changed(Evas_Object *obj);

/**
 * @brief Moves all children objects of a given smart object relative to a
 * given offset.
 *
 * This will make each of @c obj object's children to move, from where they
 * before, with those delta values (offsets) on both directions.
 *
 * @note This is most useful on custom smart @c move functions.
 *
 * @note Clipped smart objects already make use of this function on their
 * @c move smart function definition.
 *
 * @param[in] dx Horizontal offset (delta).
 * @param[in] dy Vertical offset (delta).
 *
 * @ingroup Evas_Object_Smart
 */
EAPI void evas_object_smart_move_children_relative(Evas_Object *obj, Evas_Coord dx, Evas_Coord dy);

#include "canvas/efl_canvas_group_eo.legacy.h"

/**
 * @}
 */

/**
 * @ingroup Evas_Object_Box
 *
 * @{
 */
 
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

#include "canvas/evas_box_eo.legacy.h"

/**
 * @}
 */

/**
 * @ingroup Evas_Object_Table
 *
 * @{
 */
/**
 * @brief Create a new table.
 *
 * @param evas Canvas in which table will be added.
 */
EAPI Evas_Object                       *evas_object_table_add(Evas *evas) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * @brief Control the mirrored mode of the table. In mirrored mode, the table
 * items go from right to left instead of left to right. That is, 1,1 is top
 * right, not top left.
 *
 * @param[in] obj The object.
 * @param[in] mirrored @c true if mirrored, @c false otherwise
 *
 * @since 1.1
 *
 * @ingroup Evas_Table
 */
EAPI void evas_object_table_mirrored_set(Eo *obj, Eina_Bool mirrored);

/**
 * @brief Control the mirrored mode of the table. In mirrored mode, the table
 * items go from right to left instead of left to right. That is, 1,1 is top
 * right, not top left.
 *
 * @param[in] obj The object.
 *
 * @return @c true if mirrored, @c false otherwise
 *
 * @since 1.1
 *
 * @ingroup Evas_Table
 */
EAPI Eina_Bool evas_object_table_mirrored_get(const Eo *obj);

#include "canvas/evas_table_eo.legacy.h"

/**
 * @}
 */

/**
 * @ingroup Evas_Object_Grid
 *
 * @{
 */
/**
 * Create a new grid.
 *
 * It's set to a virtual size of 1x1 by default and add children with
 * evas_object_grid_pack().
 * @since 1.1
 */
EAPI Evas_Object   *evas_object_grid_add(Evas *evas) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;


/**
 * @brief Sets the mirrored mode of the grid. In mirrored mode the grid items
 * go from right to left instead of left to right. That is, 0,0 is top right,
 * not to left.
 *
 * @param[in] obj The object.
 * @param[in] mirrored @c true if mirrored mode is set, @c false otherwise
 *
 * @since 1.1
 *
 * @ingroup Evas_Grid
 */
EAPI void evas_object_grid_mirrored_set(Eo *obj, Eina_Bool mirrored);

/**
 * @brief Gets the mirrored mode of the grid.
 *
 * See also @ref evas_object_grid_mirrored_set
 *
 * @param[in] obj The object.
 *
 * @return @c true if mirrored mode is set, @c false otherwise
 *
 * @since 1.1
 *
 * @ingroup Evas_Grid
 */
EAPI Eina_Bool evas_object_grid_mirrored_get(const Eo *obj);

#include "canvas/evas_grid_eo.legacy.h"

/**
 * @}
 */

/**
 * @brief Sets the output framespace size of the render engine of the given
 * evas.
 *
 * The framespace size is used in the Wayland engines to denote space in the
 * viewport which is occupied by the window frame. This is mainly used in
 * ecore_evas to draw borders.
 *
 * The units used for @c w and @c h depend on the engine used by the evas.
 *
 * @param[in] x The left coordinate in output units, usually pixels.
 * @param[in] y The top coordinate in output units, usually pixels.
 * @param[in] w The width in output units, usually pixels.
 * @param[in] h The height in output units, usually pixels.
 *
 * @since 1.1
 *
 * @ingroup Evas_Canvas
 */
EAPI void evas_output_framespace_set(Evas *e, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h);

/**
 * @brief Get the render engine's output framespace coordinates in canvas
 * units.
 *
 * @param[out] x The left coordinate in output units, usually pixels.
 * @param[out] y The top coordinate in output units, usually pixels.
 * @param[out] w The width in output units, usually pixels.
 * @param[out] h The height in output units, usually pixels.
 *
 * @since 1.1
 *
 * @ingroup Evas_Canvas
 */
EAPI void evas_output_framespace_get(const Evas *e, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h);

/**
 * @brief Sets the output viewport of the given evas in evas units.
 *
 * The output viewport is the area of the evas that will be visible to the
 * viewer.	The viewport will be stretched to fit the output target of the evas
 * when rendering is performed.
 *
 * @note The coordinate values do not have to map 1-to-1 with the output
 * target. However, it is generally advised that it is done for ease of use.
 *
 * @param[in] x The top-left corner x value of the viewport.
 * @param[in] y The top-left corner y value of the viewport.
 * @param[in] w The width of the viewport.	Must be greater than 0.
 * @param[in] h The height of the viewport.  Must be greater than 0.
 *
 * @ingroup Evas_Canvas
 */
EAPI void evas_output_viewport_set(Evas *e, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h);

/**
 * @brief Get the render engine's output viewport coordinates in canvas units.
 *
 * Calling this function writes the current canvas output viewport size and
 * location values into the variables pointed to by @c x, @c y, @c w and @c h.
 *	On success the variables have the output location and size values written
 * to them in canvas units. Any of @c x, @c y, @c w or @c h that are @c null
 * will not be written to. If @c e is invalid, the results are undefined.
 *
 * @param[out] x The top-left corner x value of the viewport.
 * @param[out] y The top-left corner y value of the viewport.
 * @param[out] w The width of the viewport.  Must be greater than 0.
 * @param[out] h The height of the viewport.  Must be greater than 0.
 *
 * @ingroup Evas_Canvas
 */
EAPI void evas_output_viewport_get(const Evas *e, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h);

/**
 * @brief Sets the output engine for the given evas.
 *
 * Once the output engine for an evas is set, any attempt to change it	will be
 * ignored. The value for @c render_method can be found using @ref
 * evas_render_method_lookup.
 *
 * @note it is mandatory that one calls @ref evas_init before setting the
 * output method.
 *
 * @param[in] render_method The numeric engine value to use.
 *
 * @ingroup Evas_Canvas
 */
EAPI void evas_output_method_set(Evas *e, int render_method);

/**
 * @brief Retrieves the number of the output engine used for the given evas.
 *
 * @return The numeric engine value to use.
 *
 * @ingroup Evas_Canvas
 */
EAPI int evas_output_method_get(const Evas *e);

/**
 * @brief Sets the output size of the render engine of the given evas.
 *
 * The evas will render to a rectangle of the given size once this function is
 * called.	The output size is independent of the viewport size. The viewport
 * will be stretched to fill the given rectangle.
 *
 * The units used for @c w and @c h depend on the engine used by the evas.
 *
 * @param[in] w The width in output units, usually pixels.
 * @param[in] h The height in output units, usually pixels.
 *
 * @ingroup Evas_Canvas
 */
EAPI void evas_output_size_set(Evas *e, int w, int h);

/**
 * @brief Retrieve the output size of the render engine of the given evas.
 *
 * The output size is given in whatever the output units are for the engine.
 *
 * If either @c w or @c h is @c null, then it is ignored.  If @c e is invalid,
 * the returned results are undefined.
 *
 * @param[out] w The width in output units, usually pixels.
 * @param[out] h The height in output units, usually pixels.
 *
 * @ingroup Evas_Canvas
 */
EAPI void evas_output_size_get(const Evas *e, int *w, int *h);

typedef struct _Evas_Map Evas_Map;

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
 * This call is useful to reuse maps after they had 3d lighting or
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
 * reflect the lighting based on the light point coordinates, the light
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
 * @return EINA_FALSE if map is NULL EINA_TRUE otherwise.
 */
EAPI Eina_Bool       evas_map_alpha_get(const Evas_Map *m);

/**
 * Set the flag of the object move synchronization for map rendering
 *
 * This sets the flag of the object move synchronization for map rendering.
 * If the flag is set as enabled, the map will be moved as the object of the map
 * is moved. By default, the flag of the object move synchronization is not
 * enabled.
 *
 * @param m map to modify. Must not be NULL.
 * @param enabled enable or disable the object move synchronization for map
 *        rendering.
 * @since 1.13
 */
EAPI void            evas_map_util_object_move_sync_set(Evas_Map *m, Eina_Bool enabled);

/**
 * Get the flag of the object move synchronization for map rendering
 *
 * This gets the flag of the object move synchronization for map rendering.
 *
 * @param m map to get the flag of the object move synchronization from. Must
 * not be NULL.
 * @return EINA_FALSE if map is NULL EINA_TRUE otherwise.
 * @since 1.13
 */
EAPI Eina_Bool       evas_map_util_object_move_sync_get(const Evas_Map *m);

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
 * Apply a map transformation on given coordinate
 *
 * @param m map to use to transform x and y
 * @param x point x source coordinate
 * @param y point y source coordinate
 * @param mx point x after transformation by m
 * @param my point y after transformation by m
 * @param grab
 * @return #EINA_TRUE on success interpolation, @EINA_FALSE otherwise
 * @since 1.20
 */
EAPI Eina_Bool       evas_map_coords_get(const Evas_Map *m, double x, double y,
                                         double *mx, double *my, int grab);

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
 * @brief Set current object transformation map.
 *
 * This sets the map on a given object. It is copied from the @c map pointer,
 * so there is no need to keep the @c map object if you don't need it anymore.
 *
 * A map is a set of 4 points which have canvas x, y coordinates per point,
 * with an optional z point value as a hint for perspective correction, if it
 * is available. As well each point has u and v coordinates. These are like
 * "texture coordinates" in OpenGL in that they define a point in the source
 * image that is mapped to that map vertex/point. The u corresponds to the x
 * coordinate of this mapped point and v, the y coordinate. Note that these
 * coordinates describe a bounding region to sample.
 *
 * @note The map points a uv coordinates match the image geometry. If the
 * @c map parameter is @c null, the stored map will be freed and geometry prior
 * to enabling/setting a map will be restored.
 *
 * @param[in] map The map.
 *
 * @ingroup Efl_Canvas_Object
 */
EAPI void evas_object_map_set(Evas_Object *obj, const Evas_Map *map);

/**
 * @brief Get current object transformation map.
 *
 * This returns the current internal map set on the indicated object. It is
 * intended for read-only access and is only valid as long as the object is not
 * deleted or the map on the object is not changed.
 *
 * @return The map.
 *
 * @ingroup Efl_Canvas_Object
 */
EAPI const Evas_Map *evas_object_map_get(const Evas_Object *obj);

/**
 * @brief Enable or disable the map that is set.
 *
 * Enable or disable the use of map for the object @c obj. On enable, the
 * object geometry will be saved, and the new geometry will change (position
 * and size) to reflect the map geometry set.
 *
 * If the object doesn't have a map set (with @ref evas_object_map_set), the
 * initial geometry will be undefined. It is advised to always set a map to the
 * object first, and then call this function to enable its use.
 *
 * @param[in] enabled Enabled state.
 *
 * @ingroup Evas_Object
 */
EAPI void evas_object_map_enable_set(Evas_Object *obj, Eina_Bool enabled);

/**
 * @brief Whether the map is enabled or not
 *
 * @param[in] obj The evas object
 *
 * @return The value of @c enabled as passed to evas_object_map_enable_set().
 *
 * @see evas_object_map_enable_set
 */
EAPI Eina_Bool evas_object_map_enable_get(const Evas_Object *obj);

#include "canvas/efl_gfx_mapping.eo.legacy.h"

/**
 * @brief Apply an evas filter program on this text object.
 *
 * Note: the preferred method for filters is to edit EDC styles, rather than
 * calling this API directly.
 * 
 * @since 1.18
 */
EAPI void evas_object_text_filter_program_set(Evas_Object *obj, const char *code) EINA_DEPRECATED;

/**
 * @brief Set a named source object for an evas filter program.
 *
 * Note: the preferred method for filters is to edit EDC styles, rather than
 * calling this API directly.
 * @since 1.18
 */
EAPI void evas_object_text_filter_source_set(Evas_Object *obj, const char *name, Evas_Object *source) EINA_DEPRECATED;

/**
 * Creates a new smart rectangle object on the given Evas @p e canvas.
 *
 * @param e The given canvas.
 * @return The created object handle.
 *
 * This provides a smart version of the typical "event rectangle",
 * which allows objects to set this as their parent and route events
 * to a group of objects. Events will not propagate to non-member objects
 * below this object.
 *
 * Adding members is done just like a normal smart object, using
 * efl_canvas_group_member_add (Eo API) or evas_object_smart_member_add (legacy).
 *
 * Child objects are not modified in any way, unlike other types of smart objects.
 *
 * It is a user error for any child objects to be stacked above the event
 * grabber parent while the event grabber is visible.
 * A critical error will be raised if this is detected at any point.
 *
 * @since 1.20
 */
EAPI Evas_Object *evas_object_event_grabber_add(Evas *e);

/**
 * @brief If @c true the object belongs to the window border decorations.
 *
 * This will be @c false by default, and should be @c false for all objects
 * created by the application, unless swallowed in some very specific parts of
 * the window.
 *
 * It is very unlikely that an application needs to call this manually, as the
 * window will handle this feature automatically.
 *
 * @param[in] obj The object.
 * @param[in] is_frame @c true if the object is a frame, @c false otherwise
 *
 * @since 1.2
 *
 * @ingroup Evas_Object_Group
 */
EAPI void evas_object_is_frame_object_set(Efl_Canvas_Object *obj, Eina_Bool is_frame);

/**
 * @brief If @c true the object belongs to the window border decorations.
 *
 * This will be @c false by default, and should be @c false for all objects
 * created by the application, unless swallowed in some very specific parts of
 * the window.
 *
 * It is very unlikely that an application needs to call this manually, as the
 * window will handle this feature automatically.
 *
 * @param[in] obj The object.
 *
 * @return @c true if the object is a frame, @c false otherwise
 *
 * @since 1.2
 *
 * @ingroup Evas_Object_Group
 */
EAPI Eina_Bool evas_object_is_frame_object_get(const Efl_Canvas_Object *obj);

/**
 * @brief Set whether an Evas object is to freeze (discard) events.
 *
 * If @c freeze is @c true, it will force events on @c obj to be discarded.
 * Unlike @ref evas_object_pass_events_set, events will not be passed to next
 * lower object. This API can be used for blocking events while @c obj is in
 * transition.
 *
 * If @c freeze is @c false, events will be processed on that object as normal.
 *
 * @warning If you block only key/mouse up events with this API, you can't be
 * sure of the state of any objects that have only key/mouse down events.
 *
 * @param[in] obj The object.
 * @param[in] freeze Pass when @c obj is to freeze events ($true) or not
 * ($false).
 *
 * @since 1.1
 *
 * @ingroup Evas_Object_Group
 */
EAPI void evas_object_freeze_events_set(Efl_Canvas_Object *obj, Eina_Bool freeze);

/**
 * @brief Determine whether an object is set to freeze (discard) events.
 *
 * @param[in] obj The object.
 *
 * @return Pass when @c obj is to freeze events ($true) or not ($false).
 *
 * @since 1.1
 *
 * @ingroup Evas_Object_Group
 */
EAPI Eina_Bool evas_object_freeze_events_get(const Efl_Canvas_Object *obj);

#include "canvas/efl_canvas_event_grabber_eo.legacy.h"

#include "canvas/efl_canvas_animation_alpha.eo.legacy.h"
#include "canvas/efl_canvas_animation.eo.legacy.h"
#include "canvas/efl_canvas_animation_group.eo.legacy.h"
#include "canvas/efl_canvas_animation_group_parallel.eo.legacy.h"
#include "canvas/efl_canvas_animation_group_sequential.eo.legacy.h"
#include "canvas/efl_canvas_animation_player.eo.legacy.h"
#include "canvas/efl_canvas_animation_rotate.eo.legacy.h"
#include "canvas/efl_canvas_animation_scale.eo.legacy.h"
#include "canvas/efl_canvas_animation_translate.eo.legacy.h"
