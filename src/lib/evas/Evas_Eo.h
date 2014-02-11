#ifndef _EVAS_H
# error You shall not include this header directly
#endif

EAPI extern const Eo_Event_Description _EVAS_OBJECT_EVENT_MOUSE_IN;
EAPI extern const Eo_Event_Description _EVAS_OBJECT_EVENT_MOUSE_OUT;
EAPI extern const Eo_Event_Description _EVAS_OBJECT_EVENT_MOUSE_DOWN;
EAPI extern const Eo_Event_Description _EVAS_OBJECT_EVENT_MOUSE_UP;
EAPI extern const Eo_Event_Description _EVAS_OBJECT_EVENT_MOUSE_MOVE;
EAPI extern const Eo_Event_Description _EVAS_OBJECT_EVENT_MOUSE_WHEEL;
EAPI extern const Eo_Event_Description _EVAS_OBJECT_EVENT_MULTI_DOWN;
EAPI extern const Eo_Event_Description _EVAS_OBJECT_EVENT_MULTI_UP;
EAPI extern const Eo_Event_Description _EVAS_OBJECT_EVENT_MULTI_MOVE;
EAPI extern const Eo_Event_Description _EVAS_OBJECT_EVENT_FREE;
EAPI extern const Eo_Event_Description _EVAS_OBJECT_EVENT_KEY_DOWN;
EAPI extern const Eo_Event_Description _EVAS_OBJECT_EVENT_KEY_UP;
EAPI extern const Eo_Event_Description _EVAS_OBJECT_EVENT_FOCUS_IN;
EAPI extern const Eo_Event_Description _EVAS_OBJECT_EVENT_FOCUS_OUT;
EAPI extern const Eo_Event_Description _EVAS_OBJECT_EVENT_SHOW;
EAPI extern const Eo_Event_Description _EVAS_OBJECT_EVENT_HIDE;
EAPI extern const Eo_Event_Description _EVAS_OBJECT_EVENT_MOVE;
EAPI extern const Eo_Event_Description _EVAS_OBJECT_EVENT_RESIZE;
EAPI extern const Eo_Event_Description _EVAS_OBJECT_EVENT_RESTACK;
EAPI extern const Eo_Event_Description _EVAS_OBJECT_EVENT_DEL;
EAPI extern const Eo_Event_Description _EVAS_OBJECT_EVENT_HOLD;
EAPI extern const Eo_Event_Description _EVAS_OBJECT_EVENT_CHANGED_SIZE_HINTS;
EAPI extern const Eo_Event_Description _EVAS_OBJECT_EVENT_IMAGE_PRELOADED;
EAPI extern const Eo_Event_Description _EVAS_OBJECT_EVENT_IMAGE_RESIZE;
EAPI extern const Eo_Event_Description _EVAS_OBJECT_EVENT_IMAGE_UNLOADED;

EAPI extern const Eo_Event_Description _EVAS_CANVAS_EVENT_FOCUS_IN;
EAPI extern const Eo_Event_Description _EVAS_CANVAS_EVENT_FOCUS_OUT;
EAPI extern const Eo_Event_Description _EVAS_CANVAS_EVENT_RENDER_FLUSH_PRE;
EAPI extern const Eo_Event_Description _EVAS_CANVAS_EVENT_RENDER_FLUSH_POST;
EAPI extern const Eo_Event_Description _EVAS_CANVAS_EVENT_OBJECT_FOCUS_IN;
EAPI extern const Eo_Event_Description _EVAS_CANVAS_EVENT_OBJECT_FOCUS_OUT;
EAPI extern const Eo_Event_Description _EVAS_CANVAS_EVENT_RENDER_PRE;
EAPI extern const Eo_Event_Description _EVAS_CANVAS_EVENT_RENDER_POST;

// Callbacks events for use with Evas objects
#define EVAS_OBJECT_EVENT_MOUSE_IN (&(_EVAS_OBJECT_EVENT_MOUSE_IN))
#define EVAS_OBJECT_EVENT_MOUSE_OUT (&(_EVAS_OBJECT_EVENT_MOUSE_OUT))
#define EVAS_OBJECT_EVENT_MOUSE_DOWN (&(_EVAS_OBJECT_EVENT_MOUSE_DOWN))
#define EVAS_OBJECT_EVENT_MOUSE_UP (&(_EVAS_OBJECT_EVENT_MOUSE_UP))
#define EVAS_OBJECT_EVENT_MOUSE_MOVE (&(_EVAS_OBJECT_EVENT_MOUSE_MOVE))
#define EVAS_OBJECT_EVENT_MOUSE_WHEEL (&(_EVAS_OBJECT_EVENT_MOUSE_WHEEL))
#define EVAS_OBJECT_EVENT_MULTI_DOWN (&(_EVAS_OBJECT_EVENT_MULTI_DOWN))
#define EVAS_OBJECT_EVENT_MULTI_UP (&(_EVAS_OBJECT_EVENT_MULTI_UP))
#define EVAS_OBJECT_EVENT_MULTI_MOVE (&(_EVAS_OBJECT_EVENT_MULTI_MOVE))
#define EVAS_OBJECT_EVENT_FREE (&(_EVAS_OBJECT_EVENT_FREE))
#define EVAS_OBJECT_EVENT_KEY_DOWN (&(_EVAS_OBJECT_EVENT_KEY_DOWN))
#define EVAS_OBJECT_EVENT_KEY_UP (&(_EVAS_OBJECT_EVENT_KEY_UP))
#define EVAS_OBJECT_EVENT_FOCUS_IN (&(_EVAS_OBJECT_EVENT_FOCUS_IN))
#define EVAS_OBJECT_EVENT_FOCUS_OUT (&(_EVAS_OBJECT_EVENT_FOCUS_OUT))
#define EVAS_OBJECT_EVENT_SHOW (&(_EVAS_OBJECT_EVENT_SHOW))
#define EVAS_OBJECT_EVENT_HIDE (&(_EVAS_OBJECT_EVENT_HIDE))
#define EVAS_OBJECT_EVENT_MOVE (&(_EVAS_OBJECT_EVENT_MOVE))
#define EVAS_OBJECT_EVENT_RESIZE (&(_EVAS_OBJECT_EVENT_RESIZE))
#define EVAS_OBJECT_EVENT_RESTACK (&(_EVAS_OBJECT_EVENT_RESTACK))
#define EVAS_OBJECT_EVENT_DEL (&(_EVAS_OBJECT_EVENT_DEL))
#define EVAS_OBJECT_EVENT_HOLD (&(_EVAS_OBJECT_EVENT_HOLD))
#define EVAS_OBJECT_EVENT_CHANGED_SIZE_HINTS (&(_EVAS_OBJECT_EVENT_CHANGED_SIZE_HINTS))
#define EVAS_OBJECT_EVENT_IMAGE_PRELOADED (&(_EVAS_OBJECT_EVENT_IMAGE_PRELOADED))
#define EVAS_OBJECT_EVENT_IMAGE_RESIZE (&(_EVAS_OBJECT_EVENT_IMAGE_RESIZE))
#define EVAS_OBJECT_EVENT_IMAGE_UNLOADED (&(_EVAS_OBJECT_EVENT_IMAGE_UNLOADED))

// Callbacks events for use with Evas canvases
#define EVAS_CANVAS_EVENT_FOCUS_IN (&(_EVAS_CANVAS_EVENT_FOCUS_IN))
#define EVAS_CANVAS_EVENT_FOCUS_OUT (&(_EVAS_CANVAS_EVENT_FOCUS_OUT))
#define EVAS_CANVAS_EVENT_RENDER_FLUSH_PRE (&(_EVAS_CANVAS_EVENT_RENDER_FLUSH_PRE))
#define EVAS_CANVAS_EVENT_RENDER_FLUSH_POST (&(_EVAS_CANVAS_EVENT_RENDER_FLUSH_POST))
#define EVAS_CANVAS_EVENT_OBJECT_FOCUS_IN (&(_EVAS_CANVAS_EVENT_OBJECT_FOCUS_IN))
#define EVAS_CANVAS_EVENT_OBJECT_FOCUS_OUT (&(_EVAS_CANVAS_EVENT_OBJECT_FOCUS_OUT))
#define EVAS_CANVAS_EVENT_RENDER_PRE (&(_EVAS_CANVAS_EVENT_RENDER_PRE))
#define EVAS_CANVAS_EVENT_RENDER_POST (&(_EVAS_CANVAS_EVENT_RENDER_POST))

EAPI extern const Eo_Event_Description _CLICKED_EVENT;
EAPI extern const Eo_Event_Description _CLICKED_DOUBLE_EVENT;
EAPI extern const Eo_Event_Description _CLICKED_TRIPLE_EVENT;
EAPI extern const Eo_Event_Description _PRESSED_EVENT;
EAPI extern const Eo_Event_Description _UNPRESSED_EVENT;
EAPI extern const Eo_Event_Description _LONGPRESSED_EVENT;
EAPI extern const Eo_Event_Description _REPEATED_EVENT;
EAPI extern const Eo_Event_Description _SCROLL_EVENT;
EAPI extern const Eo_Event_Description _SCROLL_ANIM_START_EVENT;
EAPI extern const Eo_Event_Description _SCROLL_ANIM_STOP_EVENT;
EAPI extern const Eo_Event_Description _SCROLL_DRAG_START_EVENT;
EAPI extern const Eo_Event_Description _SCROLL_DRAG_STOP_EVENT;
EAPI extern const Eo_Event_Description _ZOOM_START_EVENT;
EAPI extern const Eo_Event_Description _ZOOM_STOP_EVENT;
EAPI extern const Eo_Event_Description _ZOOM_CHANGE_EVENT;
EAPI extern const Eo_Event_Description _SELECTED_EVENT;
EAPI extern const Eo_Event_Description _UNSELECTED_EVENT;
EAPI extern const Eo_Event_Description _SELECTION_PASTE_EVENT;
EAPI extern const Eo_Event_Description _SELECTION_COPY_EVENT;
EAPI extern const Eo_Event_Description _SELECTION_CUT_EVENT;
EAPI extern const Eo_Event_Description _SELECTION_START_EVENT;
EAPI extern const Eo_Event_Description _SELECTION_CHANGED_EVENT;
EAPI extern const Eo_Event_Description _SELECTION_CLEARED_EVENT;
EAPI extern const Eo_Event_Description _DRAG_EVENT;
EAPI extern const Eo_Event_Description _DRAG_START_EVENT;
EAPI extern const Eo_Event_Description _DRAG_STOP_EVENT;
EAPI extern const Eo_Event_Description _DRAG_END_EVENT;
EAPI extern const Eo_Event_Description _DRAG_START_UP_EVENT;
EAPI extern const Eo_Event_Description _DRAG_START_DOWN_EVENT;
EAPI extern const Eo_Event_Description _DRAG_START_RIGHT_EVENT;
EAPI extern const Eo_Event_Description _DRAG_START_LEFT_EVENT;

#define EVAS_SMART_CLICKED_EVENT (&(_CLICKED_EVENT))
#define EVAS_SMART_CLICKED_DOUBLE_EVENT (&(_CLICKED_DOUBLE_EVENT))
#define EVAS_SMART_CLICKED_TRIPLE_EVENT (&(_CLICKED_TRIPLE_EVENT))
#define EVAS_SMART_PRESSED_EVENT (&(_PRESSED_EVENT))
#define EVAS_SMART_UNPRESSED_EVENT (&(_UNPRESSED_EVENT))
#define EVAS_SMART_LONGPRESSED_EVENT (&(_LONGPRESSED_EVENT))
#define EVAS_SMART_REPEATED_EVENT (&(_REPEATED_EVENT))
#define EVAS_SMART_SCROLL_EVENT (&(_SCROLL_EVENT))
#define EVAS_SMART_SCROLL_ANIM_START_EVENT (&(_SCROLL_ANIM_START_EVENT))
#define EVAS_SMART_SCROLL_ANIM_STOP_EVENT (&(_SCROLL_ANIM_STOP_EVENT))
#define EVAS_SMART_SCROLL_DRAG_START_EVENT (&(_SCROLL_DRAG_START_EVENT))
#define EVAS_SMART_SCROLL_DRAG_STOP_EVENT (&(_SCROLL_DRAG_STOP_EVENT))
#define EVAS_SMART_ZOOM_START_EVENT (&(_ZOOM_START_EVENT))
#define EVAS_SMART_ZOOM_STOP_EVENT (&(_ZOOM_STOP_EVENT))
#define EVAS_SMART_ZOOM_CHANGE_EVENT (&(_ZOOM_CHANGE_EVENT))
#define EVAS_SMART_SELECTED_EVENT (&(_SELECTED_EVENT))
#define EVAS_SMART_UNSELECTED_EVENT (&(_UNSELECTED_EVENT))
#define EVAS_SMART_SELECTION_PASTE_EVENT (&(_SELECTION_PASTE_EVENT))
#define EVAS_SMART_SELECTION_COPY_EVENT (&(_SELECTION_COPY_EVENT))
#define EVAS_SMART_SELECTION_CUT_EVENT (&(_SELECTION_CUT_EVENT))
#define EVAS_SMART_SELECTION_START_EVENT (&(_SELECTION_START_EVENT))
#define EVAS_SMART_SELECTION_CHANGED_EVENT (&(_SELECTION_CHANGED_EVENT))
#define EVAS_SMART_SELECTION_CLEARED_EVENT (&(_SELECTION_CLEARED_EVENT))
#define EVAS_SMART_DRAG_EVENT (&(_DRAG_EVENT))
#define EVAS_SMART_DRAG_START_EVENT (&(_DRAG_START_EVENT))
#define EVAS_SMART_DRAG_STOP_EVENT (&(_DRAG_STOP_EVENT))
#define EVAS_SMART_DRAG_END_EVENT (&(_DRAG_END_EVENT))
#define EVAS_SMART_DRAG_START_UP_EVENT (&(_DRAG_START_UP_EVENT))
#define EVAS_SMART_DRAG_START_DOWN_EVENT (&(_DRAG_START_DOWN_EVENT))
#define EVAS_SMART_DRAG_START_RIGHT_EVENT (&(_DRAG_START_RIGHT_EVENT))
#define EVAS_SMART_DRAG_START_LEFT_EVENT (&(_DRAG_START_LEFT_EVENT))

const Eo_Class *evas_smart_signal_interface_get(void) EINA_CONST;
const Eo_Class *evas_smart_clickable_interface_get(void) EINA_CONST;
const Eo_Class *evas_smart_scrollable_interface_get(void) EINA_CONST;
const Eo_Class *evas_smart_zoomable_interface_get(void) EINA_CONST;
const Eo_Class *evas_smart_selectable_interface_get(void) EINA_CONST;
const Eo_Class *evas_smart_draggable_interface_get(void) EINA_CONST;

#define EVAS_SMART_SIGNAL_INTERFACE evas_smart_signal_interface_get()
#define EVAS_SMART_CLICKABLE_INTERFACE evas_smart_clickable_interface_get()
#define EVAS_SMART_SCROLLABLE_INTERFACE evas_smart_scrollable_interface_get()
#define EVAS_SMART_ZOOMABLE_INTERFACE evas_smart_zoomable_interface_get()
#define EVAS_SMART_SELECTABLE_INTERFACE evas_smart_selectable_interface_get()
#define EVAS_SMART_DRAGGABLE_INTERFACE evas_smart_draggable_interface_get()

#define EVAS_CLASS evas_class_get()

const Eo_Class *evas_class_get(void) EINA_CONST;

extern EAPI Eo_Op EVAS_CANVAS_BASE_ID;

enum
{
   EVAS_CANVAS_SUB_ID_OUTPUT_METHOD_SET,
   EVAS_CANVAS_SUB_ID_OUTPUT_METHOD_GET,
   EVAS_CANVAS_SUB_ID_ENGINE_INFO_GET,
   EVAS_CANVAS_SUB_ID_ENGINE_INFO_SET,
   EVAS_CANVAS_SUB_ID_OUTPUT_SIZE_SET,
   EVAS_CANVAS_SUB_ID_OUTPUT_SIZE_GET,
   EVAS_CANVAS_SUB_ID_OUTPUT_VIEWPORT_SET,
   EVAS_CANVAS_SUB_ID_OUTPUT_VIEWPORT_GET,
   EVAS_CANVAS_SUB_ID_OUTPUT_FRAMESPACE_SET,
   EVAS_CANVAS_SUB_ID_OUTPUT_FRAMESPACE_GET,
   EVAS_CANVAS_SUB_ID_COORD_SCREEN_X_TO_WORLD,
   EVAS_CANVAS_SUB_ID_COORD_SCREEN_Y_TO_WORLD,
   EVAS_CANVAS_SUB_ID_COORD_WORLD_X_TO_SCREEN,
   EVAS_CANVAS_SUB_ID_COORD_WORLD_Y_TO_SCREEN,
   EVAS_CANVAS_SUB_ID_POINTER_OUTPUT_XY_GET,
   EVAS_CANVAS_SUB_ID_POINTER_CANVAS_XY_GET,
   EVAS_CANVAS_SUB_ID_POINTER_BUTTON_DOWN_MASK_GET,
   EVAS_CANVAS_SUB_ID_POINTER_INSIDE_GET,
   EVAS_CANVAS_SUB_ID_DATA_ATTACH_SET,
   EVAS_CANVAS_SUB_ID_DATA_ATTACH_GET,
   EVAS_CANVAS_SUB_ID_FOCUS_IN,
   EVAS_CANVAS_SUB_ID_FOCUS_OUT,
   EVAS_CANVAS_SUB_ID_FOCUS_STATE_GET,
   EVAS_CANVAS_SUB_ID_NOCHANGE_PUSH,
   EVAS_CANVAS_SUB_ID_NOCHANGE_POP,
   EVAS_CANVAS_SUB_ID_EVENT_DEFAULT_FLAGS_SET,
   EVAS_CANVAS_SUB_ID_EVENT_DEFAULT_FLAGS_GET,
   EVAS_CANVAS_SUB_ID_EVENT_FEED_MOUSE_DOWN,
   EVAS_CANVAS_SUB_ID_EVENT_FEED_MOUSE_UP,
   EVAS_CANVAS_SUB_ID_EVENT_FEED_MOUSE_CANCEL,
   EVAS_CANVAS_SUB_ID_EVENT_FEED_MOUSE_WHEEL,
   EVAS_CANVAS_SUB_ID_EVENT_INPUT_MOUSE_MOVE,
   EVAS_CANVAS_SUB_ID_EVENT_FEED_MOUSE_MOVE,
   EVAS_CANVAS_SUB_ID_EVENT_FEED_MOUSE_IN,
   EVAS_CANVAS_SUB_ID_EVENT_FEED_MOUSE_OUT,
   EVAS_CANVAS_SUB_ID_EVENT_FEED_MULTI_DOWN,
   EVAS_CANVAS_SUB_ID_EVENT_FEED_MULTI_UP,
   EVAS_CANVAS_SUB_ID_EVENT_FEED_MULTI_MOVE,
   EVAS_CANVAS_SUB_ID_EVENT_INPUT_MULTI_DOWN,
   EVAS_CANVAS_SUB_ID_EVENT_INPUT_MULTI_UP,
   EVAS_CANVAS_SUB_ID_EVENT_INPUT_MULTI_MOVE,
   EVAS_CANVAS_SUB_ID_EVENT_FEED_KEY_DOWN,
   EVAS_CANVAS_SUB_ID_EVENT_FEED_KEY_UP,
   EVAS_CANVAS_SUB_ID_EVENT_FEED_HOLD,
   EVAS_CANVAS_SUB_ID_EVENT_REFEED_EVENT,
   EVAS_CANVAS_SUB_ID_EVENT_DOWN_COUNT_GET,
   EVAS_CANVAS_SUB_ID_FOCUS_GET,
   EVAS_CANVAS_SUB_ID_FONT_PATH_CLEAR,
   EVAS_CANVAS_SUB_ID_FONT_PATH_APPEND,
   EVAS_CANVAS_SUB_ID_FONT_PATH_PREPEND,
   EVAS_CANVAS_SUB_ID_FONT_PATH_LIST,
   EVAS_CANVAS_SUB_ID_FONT_HINTING_SET,
   EVAS_CANVAS_SUB_ID_FONT_HINTING_GET,
   EVAS_CANVAS_SUB_ID_FONT_HINTING_CAN_HINT,
   EVAS_CANVAS_SUB_ID_FONT_CACHE_FLUSH,
   EVAS_CANVAS_SUB_ID_FONT_CACHE_SET,
   EVAS_CANVAS_SUB_ID_FONT_CACHE_GET,
   EVAS_CANVAS_SUB_ID_FONT_AVAILABLE_LIST,
   EVAS_CANVAS_SUB_ID_KEY_MODIFIER_GET,
   EVAS_CANVAS_SUB_ID_KEY_LOCK_GET,
   EVAS_CANVAS_SUB_ID_KEY_MODIFIER_ADD,
   EVAS_CANVAS_SUB_ID_KEY_MODIFIER_DEL,
   EVAS_CANVAS_SUB_ID_KEY_LOCK_ADD,
   EVAS_CANVAS_SUB_ID_KEY_LOCK_DEL,
   EVAS_CANVAS_SUB_ID_KEY_MODIFIER_ON,
   EVAS_CANVAS_SUB_ID_KEY_MODIFIER_OFF,
   EVAS_CANVAS_SUB_ID_KEY_LOCK_ON,
   EVAS_CANVAS_SUB_ID_KEY_LOCK_OFF,
   EVAS_CANVAS_SUB_ID_KEY_MODIFIER_MASK_GET,
   EVAS_CANVAS_SUB_ID_DAMAGE_RECTANGLE_ADD,
   EVAS_CANVAS_SUB_ID_OBSCURED_RECTANGLE_ADD,
   EVAS_CANVAS_SUB_ID_OBSCURED_CLEAR,
   EVAS_CANVAS_SUB_ID_RENDER_UPDATES,
   EVAS_CANVAS_SUB_ID_RENDER,
   EVAS_CANVAS_SUB_ID_NORENDER,
   EVAS_CANVAS_SUB_ID_RENDER_IDLE_FLUSH,
   EVAS_CANVAS_SUB_ID_SYNC,
   EVAS_CANVAS_SUB_ID_RENDER_DUMP,
   EVAS_CANVAS_SUB_ID_OBJECT_BOTTOM_GET,
   EVAS_CANVAS_SUB_ID_OBJECT_TOP_GET,
   EVAS_CANVAS_SUB_ID_TOUCH_POINT_LIST_COUNT,
   EVAS_CANVAS_SUB_ID_TOUCH_POINT_LIST_NTH_XY_GET,
   EVAS_CANVAS_SUB_ID_TOUCH_POINT_LIST_NTH_ID_GET,
   EVAS_CANVAS_SUB_ID_TOUCH_POINT_LIST_NTH_STATE_GET,
   EVAS_CANVAS_SUB_ID_IMAGE_CACHE_FLUSH,
   EVAS_CANVAS_SUB_ID_IMAGE_CACHE_RELOAD,
   EVAS_CANVAS_SUB_ID_IMAGE_CACHE_SET,
   EVAS_CANVAS_SUB_ID_IMAGE_CACHE_GET,
   EVAS_CANVAS_SUB_ID_IMAGE_MAX_SIZE_GET,
   EVAS_CANVAS_SUB_ID_OBJECT_NAME_FIND,
   EVAS_CANVAS_SUB_ID_OBJECT_TOP_AT_XY_GET,
   EVAS_CANVAS_SUB_ID_OBJECT_TOP_IN_RECTANGLE_GET,
   EVAS_CANVAS_SUB_ID_OBJECTS_AT_XY_GET,
   EVAS_CANVAS_SUB_ID_OBJECTS_IN_RECTANGLE_GET,
   EVAS_CANVAS_SUB_ID_SMART_OBJECTS_CALCULATE,
   EVAS_CANVAS_SUB_ID_SMART_OBJECTS_CALCULATE_COUNT_GET,
   EVAS_CANVAS_SUB_ID_RENDER_ASYNC,
   EVAS_CANVAS_SUB_ID_TREE_OBJECTS_AT_XY_GET,
   EVAS_CANVAS_SUB_ID_LAST
};

#define EVAS_CANVAS_ID(sub_id) (EVAS_CANVAS_BASE_ID + sub_id)

/**
 * @ingroup Evas_Output_Size
 *
 * @{
 */

/**
 * @def evas_canvas_output_method_set
 * @since 1.8
 *
 * Sets the output engine for the given evas.
 *
 * @param[in] render_method
 *
 * @see evas_output_method_set
 */
#define evas_canvas_output_method_set(render_method) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_OUTPUT_METHOD_SET), EO_TYPECHECK(int, render_method)

/**
 * @def evas_canvas_output_method_get
 * @since 1.8
 *
 * Retrieves the number of the output engine used for the given evas.
 *
 * @param[out] ret
 *
 * @see evas_output_method_get
 */
#define evas_canvas_output_method_get(ret) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_OUTPUT_METHOD_GET), EO_TYPECHECK(int *, ret)

/**
 * @def evas_canvas_engine_info_get
 * @since 1.8
 *
 * Retrieves the current render engine info struct from the given evas.
 *
 * @param[out] ret
 *
 * @see evas_engine_info_get
 */
#define evas_canvas_engine_info_get(ret) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_ENGINE_INFO_GET), EO_TYPECHECK(Evas_Engine_Info **, ret)

/**
 * @def evas_canvas_engine_info_set
 * @since 1.8
 *
 * Applies the engine settings for the given evas from the given @c
 * Evas_Engine_Info structure.
 *
 * @param[in] info
 * @param[out] ret
 *
 * @see evas_engine_info_set
 */
#define evas_canvas_engine_info_set(info, ret) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_ENGINE_INFO_SET), EO_TYPECHECK(Evas_Engine_Info *, info), EO_TYPECHECK(Eina_Bool *, ret)
/**
 * @}
 */

/**
 * @ingroup Evas_Output_Size
 *
 * @{
 */

/**
 * @def evas_canvas_output_size_set
 * @since 1.8
 *
 * Sets the output size of the render engine of the given evas.
 *
 * @param[in] w
 * @param[in] h
 *
 * @see evas_output_size_set
 */
#define evas_canvas_output_size_set(w, h) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_OUTPUT_SIZE_SET), EO_TYPECHECK(int, w), EO_TYPECHECK(int, h)

/**
 * @def evas_canvas_output_size_get
 * @since 1.8
 *
 * Retrieve the output size of the render engine of the given evas.
 *
 * @param[out] w
 * @param[out] h
 *
 * @see evas_output_size_get
 */
#define evas_canvas_output_size_get(w, h) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_OUTPUT_SIZE_GET), EO_TYPECHECK(int *, w), EO_TYPECHECK(int *, h)

/**
 * @def evas_canvas_output_viewport_set
 * @since 1.8
 *
 * Sets the output viewport of the given evas in evas units.
 *
 * @param[in] x
 * @param[in] y
 * @param[in] w
 * @param[in] h
 *
 * @see evas_output_viewport_set
 */
#define evas_canvas_output_viewport_set(x, y, w, h) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_OUTPUT_VIEWPORT_SET), EO_TYPECHECK(Evas_Coord, x), EO_TYPECHECK(Evas_Coord, y), EO_TYPECHECK(Evas_Coord, w), EO_TYPECHECK(Evas_Coord, h)

/**
 * @def evas_canvas_output_viewport_get
 * @since 1.8
 *
 * Get the render engine's output viewport co-ordinates in canvas units.
 *
 * @param[out] x
 * @param[out] y
 * @param[out] w
 * @param[out] h
 *
 * @see evas_output_viewport_get
 */
#define evas_canvas_output_viewport_get(x, y, w, h) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_OUTPUT_VIEWPORT_GET), EO_TYPECHECK(Evas_Coord *, x), EO_TYPECHECK(Evas_Coord *, y), EO_TYPECHECK(Evas_Coord *, w), EO_TYPECHECK(Evas_Coord *, h)

/**
 * @def evas_canvas_output_framespace_set
 * @since 1.8
 *
 * Sets the output framespace size of the render engine of the given evas.
 *
 * @param[in] x
 * @param[in] y
 * @param[in] w
 * @param[in] h
 *
 * @see evas_output_framespace_set
 */
#define evas_canvas_output_framespace_set(x, y, w, h) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_OUTPUT_FRAMESPACE_SET), EO_TYPECHECK(Evas_Coord, x), EO_TYPECHECK(Evas_Coord, y), EO_TYPECHECK(Evas_Coord, w), EO_TYPECHECK(Evas_Coord, h)

/**
 * @def evas_canvas_output_framespace_get
 * @since 1.8
 *
 * Get the render engine's output framespace co-ordinates in canvas units.
 *
 * @param[out] x
 * @param[out] y
 * @param[out] w
 * @param[out] h
 *
 * @see evas_output_framespace_get
 */
#define evas_canvas_output_framespace_get(x, y, w, h) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_OUTPUT_FRAMESPACE_GET), EO_TYPECHECK(Evas_Coord *, x), EO_TYPECHECK(Evas_Coord *, y), EO_TYPECHECK(Evas_Coord *, w), EO_TYPECHECK(Evas_Coord *, h)
/**
 * @}
 */

/*
 * @ingroup Evas_Coord_Mapping_Group
 *
 * @{
 */

/**
 * @def evas_canvas_coord_screen_x_to_world
 * @since 1.8
 *
 * Convert/scale an output screen co-ordinate into canvas co-ordinates
 *
 * @param[in] x
 * @param[out] ret
 *
 * @see evas_coord_screen_x_to_world
 */
#define evas_canvas_coord_screen_x_to_world(x, ret) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_COORD_SCREEN_X_TO_WORLD), EO_TYPECHECK(int, x), EO_TYPECHECK(Evas_Coord *, ret)

/**
 * @def evas_canvas_coord_screen_y_to_world
 * @since 1.8
 *
 * Convert/scale an output screen co-ordinate into canvas co-ordinates
 *
 * @param[in] y
 * @param[out] ret
 *
 * @see evas_coord_screen_y_to_world
 */
#define evas_canvas_coord_screen_y_to_world(y, ret) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_COORD_SCREEN_Y_TO_WORLD), EO_TYPECHECK(int, y), EO_TYPECHECK(Evas_Coord *, ret)

/**
 * @def evas_canvas_coord_world_x_to_screen
 * @since 1.8
 *
 * Convert/scale a canvas co-ordinate into output screen co-ordinates
 *
 * @param[in] x
 * @param[out] ret
 *
 * @see evas_coord_world_x_to_screen
 */
#define evas_canvas_coord_world_x_to_screen(x, ret) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_COORD_WORLD_X_TO_SCREEN), EO_TYPECHECK(Evas_Coord, x), EO_TYPECHECK(int *, ret)

/**
 * @def evas_canvas_coord_world_y_to_screen
 * @since 1.8
 *
 * Convert/scale a canvas co-ordinate into output screen co-ordinates
 *
 * @param[in] y
 * @param[out] ret
 *
 * @see evas_coord_world_y_to_screen
 */
#define evas_canvas_coord_world_y_to_screen(y, ret) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_COORD_WORLD_Y_TO_SCREEN), EO_TYPECHECK(Evas_Coord, y), EO_TYPECHECK(int *, ret)
/**
 * @}
 */

/**
 * @ingroup Evas_Pointer_Group
 *
 * @{
 */

/**
 * @def evas_canvas_pointer_output_xy_get
 * @since 1.8
 *
 * This function returns the current known pointer co-ordinates
 *
 * @param[out] x
 * @param[out] y
 *
 * @see evas_pointer_output_xy_get
 */
#define evas_canvas_pointer_output_xy_get(x, y) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_POINTER_OUTPUT_XY_GET), EO_TYPECHECK(int *, x), EO_TYPECHECK(int *, y)

/**
 * @def evas_canvas_pointer_canvas_xy_get
 * @since 1.8
 *
 * This function returns the current known pointer co-ordinates
 *
 * @param[out] x
 * @param[out] y
 *
 * @see evas_pointer_canvas_xy_get
 */
#define evas_canvas_pointer_canvas_xy_get(x, y) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_POINTER_CANVAS_XY_GET), EO_TYPECHECK(Evas_Coord *, x), EO_TYPECHECK(Evas_Coord *, y)

/**
 * @def evas_canvas_pointer_button_down_mask_get
 * @since 1.8
 *
 * Returns a bitmask with the mouse buttons currently pressed, set to 1
 *
 * @param[out] ret
 *
 * @see evas_pointer_button_down_mask_get
 */
#define evas_canvas_pointer_button_down_mask_get(ret) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_POINTER_BUTTON_DOWN_MASK_GET), EO_TYPECHECK(int *, ret)

/**
 * @def evas_canvas_pointer_inside_get
 * @since 1.8
 *
 * Returns whether the mouse pointer is logically inside the canvas
 *
 * @param[out] ret
 *
 * @see evas_pointer_inside_get
 */
#define evas_canvas_pointer_inside_get(ret) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_POINTER_INSIDE_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def evas_canvas_sync
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 */
#define evas_canvas_sync() EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_SYNC)

/**
 * @}
 */

/**
 * @ingroup Evas_Canvas
 *
 * @{
 */

/**
 * @def evas_canvas_data_attach_set
 * @since 1.8
 *
 * Attaches a specific pointer to the evas for fetching later
 *
 * @param[in] data
 *
 * @see evas_data_attach_set
 */
#define evas_canvas_data_attach_set(data) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_DATA_ATTACH_SET), EO_TYPECHECK(void *, data)

/**
 * @def evas_canvas_data_attach_get
 * @since 1.8
 *
 * Returns the pointer attached by evas_data_attach_set()
 *
 * @param[out] ret
 *
 * @see evas_data_attach_get
 */
#define evas_canvas_data_attach_get(ret) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_DATA_ATTACH_GET), EO_TYPECHECK(void **, ret)

/**
 * @def evas_canvas_focus_in
 * @since 1.8
 *
 * Inform to the evas that it got the focus.
 *
 *
 * @see evas_focus_in
 */
#define evas_canvas_focus_in() EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_FOCUS_IN)

/**
 * @def evas_canvas_focus_out
 * @since 1.8
 *
 * Inform to the evas that it lost the focus.
 *
 *
 * @see evas_focus_out
 */
#define evas_canvas_focus_out() EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_FOCUS_OUT)

/**
 * @def evas_canvas_focus_state_get
 * @since 1.8
 *
 * Get the focus state known by the given evas
 *
 * @param[out] ret
 *
 * @see evas_focus_state_get
 */
#define evas_canvas_focus_state_get(ret) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_FOCUS_STATE_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def evas_canvas_nochange_push
 * @since 1.8
 *
 * Push the nochange flag up 1
 *
 *
 * @see evas_nochange_push
 */
#define evas_canvas_nochange_push() EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_NOCHANGE_PUSH)

/**
 * @def evas_canvas_nochange_pop
 * @since 1.8
 *
 * Pop the nochange flag down 1
 *
 *
 * @see evas_nochange_pop
 */
#define evas_canvas_nochange_pop() EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_NOCHANGE_POP)

/**
 * @def evas_canvas_damage_rectangle_add
 * @since 1.8
 *
 * Add a damage rectangle.
 *
 * @param[in] x
 * @param[in] y
 * @param[in] w
 * @param[in] h
 *
 * @see evas_damage_rectangle_add
 */
#define evas_canvas_damage_rectangle_add(x, y, w, h) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_DAMAGE_RECTANGLE_ADD), EO_TYPECHECK(int, x), EO_TYPECHECK(int, y), EO_TYPECHECK(int, w), EO_TYPECHECK(int, h)

/**
 * @def evas_canvas_obscured_rectangle_add
 * @since 1.8
 *
 * Add an "obscured region" to an Evas canvas.
 *
 * @param[in] x
 * @param[in] y
 * @param[in] w
 * @param[in] h
 *
 * @see evas_obscured_rectangle_add
 */
#define evas_canvas_obscured_rectangle_add(x, y, w, h) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_OBSCURED_RECTANGLE_ADD), EO_TYPECHECK(int, x), EO_TYPECHECK(int, y), EO_TYPECHECK(int, w), EO_TYPECHECK(int, h)

/**
 * @def evas_canvas_obscured_clear
 * @since 1.8
 *
 * Remove all "obscured regions" from an Evas canvas.
 *
 *
 * @see evas_obscured_clear
 */
#define evas_canvas_obscured_clear() EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_OBSCURED_CLEAR)

/**
 * @def evas_canvas_render_async
 * @since 1.8
 *
 * Render canvas asynchronously
 *
 * @param[out] ret Whether or not a frame will get rendered after the call
 *
 * @see evas_render_async
 */
#define evas_canvas_render_async(ret) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_RENDER_ASYNC), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def evas_canvas_render_updates
 * @since 1.8
 *
 * Force immediate renderization of the given Evas canvas.
 *
 * @param[out] ret
 *
 * @see evas_render_updates
 */
#define evas_canvas_render_updates(ret) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_RENDER_UPDATES), EO_TYPECHECK(Eina_List **, ret)

/**
 * @def evas_canvas_render
 * @since 1.8
 *
 * Force renderization of the given canvas.
 *
 *
 * @see evas_render
 */
#define evas_canvas_render() EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_RENDER)

/**
 * @def evas_canvas_norender
 * @since 1.8
 *
 * Update the canvas internal objects but not triggering immediate
 * renderization.
 *
 *
 * @see evas_norender
 */
#define evas_canvas_norender() EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_NORENDER)

/**
 * @def evas_canvas_render_idle_flush
 * @since 1.8
 *
 * Make the canvas discard internally cached data used for rendering.
 *
 *
 * @see evas_render_idle_flush
 */
#define evas_canvas_render_idle_flush() EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_RENDER_IDLE_FLUSH)

/**
 * @def evas_canvas_render_dump
 * @since 1.8
 *
 * Make the canvas discard as much data as possible used by the engine at
 *
 *
 * @see evas_render_dump
 */
#define evas_canvas_render_dump() EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_RENDER_DUMP)

/**
 * @}
 */

/**
 * @ingroup Evas_Event_Feeding_Group
 *
 * @{
 */

/**
 * @def evas_canvas_event_default_flags_set
 * @since 1.8
 *
 * Set the default set of flags an event begins with
 *
 * @param[in] flags
 *
 * @see evas_event_default_flags_set
 */
#define evas_canvas_event_default_flags_set(flags) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_EVENT_DEFAULT_FLAGS_SET), EO_TYPECHECK(Evas_Event_Flags, flags)

/**
 * @def evas_canvas_event_default_flags_get
 * @since 1.8
 *
 * Get the defaulty set of flags an event begins with
 *
 * @param[out] ret
 *
 * @see evas_event_default_flags_get
 */
#define evas_canvas_event_default_flags_get(ret) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_EVENT_DEFAULT_FLAGS_GET), EO_TYPECHECK(Evas_Event_Flags *, ret)

/**
 * @def evas_canvas_event_freeze
 * @since 1.8
 *
 * Freeze all input events processing.
 *
 *
 * @see evas_event_freeze
 */
#define evas_canvas_event_freeze() EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_EVENT_FREEZE)

/**
 * @def evas_canvas_event_thaw
 * @since 1.8
 *
 * Thaw a canvas out after freezing (for input events).
 *
 *
 * @see evas_event_thaw
 */
#define evas_canvas_event_thaw() EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_EVENT_THAW)

/**
 * @def evas_canvas_event_freeze_get
 * @since 1.8
 *
 * Return the freeze count on input events of a given canvas.
 *
 * @param[out] ret
 *
 * @see evas_event_freeze_get
 */
#define evas_canvas_event_freeze_get(ret) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_EVENT_FREEZE_GET), EO_TYPECHECK(int *, ret)

/**
 * @def evas_canvas_event_thaw_eval
 * @since 1.8
 *
 * After thaw of a canvas, re-evaluate the state of objects and call callbacks
 *
 *
 * @see evas_event_thaw_eval
 */
#define evas_canvas_event_thaw_eval() EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_EVENT_THAW_EVAL)
/**
 * @}
 */

/**
 * @ingroup Evas_Event_Feeding_Group
 *
 * @{
 */

/**
 * @def evas_canvas_event_down_count_get
 * @since 1.8
 *
 * Get the number of mouse or multi presses currently active
 *
 * @param[out] ret
 *
 * @see evas_event_down_count_get
 */
#define evas_canvas_event_down_count_get(ret) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_EVENT_DOWN_COUNT_GET), EO_TYPECHECK(int *, ret)

/**
 * @def evas_canvas_event_feed_mouse_down
 * @since 1.8
 *
 * Mouse down event feed.
 *
 * @param[in] b
 * @param[in] flags
 * @param[in] timestamp
 * @param[in] data
 *
 * @see evas_event_feed_mouse_down
 */
#define evas_canvas_event_feed_mouse_down(b, flags, timestamp, data) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_EVENT_FEED_MOUSE_DOWN), EO_TYPECHECK(int, b), EO_TYPECHECK(Evas_Button_Flags, flags), EO_TYPECHECK(unsigned int, timestamp), EO_TYPECHECK(const void *, data)

/**
 * @def evas_canvas_event_feed_mouse_up
 * @since 1.8
 *
 * Mouse up event feed.
 *
 * @param[in] b
 * @param[in] flags
 * @param[in] timestamp
 * @param[in] data
 *
 * @see evas_event_feed_mouse_up
 */
#define evas_canvas_event_feed_mouse_up(b, flags, timestamp, data) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_EVENT_FEED_MOUSE_UP), EO_TYPECHECK(int, b), EO_TYPECHECK(Evas_Button_Flags, flags), EO_TYPECHECK(unsigned int, timestamp), EO_TYPECHECK(const void *, data)

/**
 * @def evas_canvas_event_feed_mouse_cancel
 * @since 1.8
 *
 * Mouse cancel event feed.
 *
 * @param[in] timestamp
 * @param[in] data
 *
 * @see evas_event_feed_mouse_cancel
 */
#define evas_canvas_event_feed_mouse_cancel(timestamp, data) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_EVENT_FEED_MOUSE_CANCEL), EO_TYPECHECK(unsigned int, timestamp), EO_TYPECHECK(const void *, data)

/**
 * @def evas_canvas_event_feed_mouse_wheel
 * @since 1.8
 *
 * Mouse wheel event feed.
 *
 * @param[in] direction
 * @param[in] z
 * @param[in] timestamp
 * @param[in] data
 *
 * @see evas_event_feed_mouse_wheel
 */
#define evas_canvas_event_feed_mouse_wheel(direction, z, timestamp, data) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_EVENT_FEED_MOUSE_WHEEL), EO_TYPECHECK(int, direction), EO_TYPECHECK(int, z), EO_TYPECHECK(unsigned int, timestamp), EO_TYPECHECK(const void *, data)

/**
 * @def evas_canvas_event_input_mouse_move
 * @since 1.8
 *
 * Mouse move event feed from input.
 *
 * @param[in] x
 * @param[in] y
 * @param[in] timestamp
 * @param[in] data
 *
 * @see evas_event_input_mouse_move
 */
#define evas_canvas_event_input_mouse_move(x, y, timestamp, data) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_EVENT_INPUT_MOUSE_MOVE), EO_TYPECHECK(int, x), EO_TYPECHECK(int, y), EO_TYPECHECK(unsigned int, timestamp), EO_TYPECHECK(const void *, data)

/**
 * @def evas_canvas_event_feed_mouse_move
 * @since 1.8
 *
 * Mouse move event feed.
 *
 * @param[in] x
 * @param[in] y
 * @param[in] timestamp
 * @param[in] data
 *
 * @see evas_event_feed_mouse_move
 */
#define evas_canvas_event_feed_mouse_move(x, y, timestamp, data) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_EVENT_FEED_MOUSE_MOVE), EO_TYPECHECK(int, x), EO_TYPECHECK(int, y), EO_TYPECHECK(unsigned int, timestamp), EO_TYPECHECK(const void *, data)

/**
 * @def evas_canvas_event_feed_mouse_in
 * @since 1.8
 *
 * Mouse in event feed.
 *
 * @param[in] timestamp
 * @param[in] data
 *
 * @see evas_event_feed_mouse_in
 */
#define evas_canvas_event_feed_mouse_in(timestamp, data) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_EVENT_FEED_MOUSE_IN), EO_TYPECHECK(unsigned int, timestamp), EO_TYPECHECK(const void *, data)

/**
 * @def evas_canvas_event_feed_mouse_out
 * @since 1.8
 *
 * Mouse out event feed.
 *
 * @param[in] timestamp
 * @param[in] data
 *
 * @see evas_event_feed_mouse_out
 */
#define evas_canvas_event_feed_mouse_out(timestamp, data) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_EVENT_FEED_MOUSE_OUT), EO_TYPECHECK(unsigned int, timestamp), EO_TYPECHECK(const void *, data)

/**
 * @def evas_canvas_event_input_multi_down
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[in] d
 * @param[in] x
 * @param[in] y
 * @param[in] rad
 * @param[in] radx
 * @param[in] rady
 * @param[in] pres
 * @param[in] ang
 * @param[in] fx
 * @param[in] fy
 * @param[in] flags
 * @param[in] timestamp
 * @param[in] data
 *
 * @see evas_event_input_multi_down
 */
#define evas_canvas_event_input_multi_down(d, x, y, rad, radx, rady, pres, ang, fx, fy, flags, timestamp, data) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_EVENT_INPUT_MULTI_DOWN), EO_TYPECHECK(int, d), EO_TYPECHECK(int, x), EO_TYPECHECK(int, y), EO_TYPECHECK(double, rad), EO_TYPECHECK(double, radx), EO_TYPECHECK(double, rady), EO_TYPECHECK(double, pres), EO_TYPECHECK(double, ang), EO_TYPECHECK(double, fx), EO_TYPECHECK(double, fy), EO_TYPECHECK(Evas_Button_Flags, flags), EO_TYPECHECK(unsigned int, timestamp), EO_TYPECHECK(const void *, data)

/**
 * @def evas_canvas_event_feed_multi_down
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[in] d
 * @param[in] x
 * @param[in] y
 * @param[in] rad
 * @param[in] radx
 * @param[in] rady
 * @param[in] pres
 * @param[in] ang
 * @param[in] fx
 * @param[in] fy
 * @param[in] flags
 * @param[in] timestamp
 * @param[in] data
 *
 * @see evas_event_feed_multi_down
 */
#define evas_canvas_event_feed_multi_down(d, x, y, rad, radx, rady, pres, ang, fx, fy, flags, timestamp, data) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_EVENT_FEED_MULTI_DOWN), EO_TYPECHECK(int, d), EO_TYPECHECK(int, x), EO_TYPECHECK(int, y), EO_TYPECHECK(double, rad), EO_TYPECHECK(double, radx), EO_TYPECHECK(double, rady), EO_TYPECHECK(double, pres), EO_TYPECHECK(double, ang), EO_TYPECHECK(double, fx), EO_TYPECHECK(double, fy), EO_TYPECHECK(Evas_Button_Flags, flags), EO_TYPECHECK(unsigned int, timestamp), EO_TYPECHECK(const void *, data)

/**
 * @def evas_canvas_event_input_multi_up
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[in] d
 * @param[in] x
 * @param[in] y
 * @param[in] rad
 * @param[in] radx
 * @param[in] rady
 * @param[in] pres
 * @param[in] ang
 * @param[in] fx
 * @param[in] fy
 * @param[in] flags
 * @param[in] timestamp
 * @param[in] data
 *
 * @see evas_event_input_multi_up
 */
#define evas_canvas_event_input_multi_up(d, x, y, rad, radx, rady, pres, ang, fx, fy, flags, timestamp, data) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_EVENT_INPUT_MULTI_UP), EO_TYPECHECK(int, d), EO_TYPECHECK(int, x), EO_TYPECHECK(int, y), EO_TYPECHECK(double, rad), EO_TYPECHECK(double, radx), EO_TYPECHECK(double, rady), EO_TYPECHECK(double, pres), EO_TYPECHECK(double, ang), EO_TYPECHECK(double, fx), EO_TYPECHECK(double, fy), EO_TYPECHECK(Evas_Button_Flags, flags), EO_TYPECHECK(unsigned int, timestamp), EO_TYPECHECK(const void *, data)

/**
 * @def evas_canvas_event_feed_multi_up
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[in] d
 * @param[in] x
 * @param[in] y
 * @param[in] rad
 * @param[in] radx
 * @param[in] rady
 * @param[in] pres
 * @param[in] ang
 * @param[in] fx
 * @param[in] fy
 * @param[in] flags
 * @param[in] timestamp
 * @param[in] data
 *
 * @see evas_event_feed_multi_up
 */
#define evas_canvas_event_feed_multi_up(d, x, y, rad, radx, rady, pres, ang, fx, fy, flags, timestamp, data) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_EVENT_FEED_MULTI_UP), EO_TYPECHECK(int, d), EO_TYPECHECK(int, x), EO_TYPECHECK(int, y), EO_TYPECHECK(double, rad), EO_TYPECHECK(double, radx), EO_TYPECHECK(double, rady), EO_TYPECHECK(double, pres), EO_TYPECHECK(double, ang), EO_TYPECHECK(double, fx), EO_TYPECHECK(double, fy), EO_TYPECHECK(Evas_Button_Flags, flags), EO_TYPECHECK(unsigned int, timestamp), EO_TYPECHECK(const void *, data)

/**
 * @def evas_canvas_event_input_multi_move
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[in] d
 * @param[in] x
 * @param[in] y
 * @param[in] rad
 * @param[in] radx
 * @param[in] rady
 * @param[in] pres
 * @param[in] ang
 * @param[in] fx
 * @param[in] fy
 * @param[in] timestamp
 * @param[in] data
 *
 * @see evas_event_feed_multi_move
 */
#define evas_canvas_event_input_multi_move(d, x, y, rad, radx, rady, pres, ang, fx, fy, timestamp, data) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_EVENT_INPUT_MULTI_MOVE), EO_TYPECHECK(int, d), EO_TYPECHECK(int, x), EO_TYPECHECK(int, y), EO_TYPECHECK(double, rad), EO_TYPECHECK(double, radx), EO_TYPECHECK(double, rady), EO_TYPECHECK(double, pres), EO_TYPECHECK(double, ang), EO_TYPECHECK(double, fx), EO_TYPECHECK(double, fy), EO_TYPECHECK(unsigned int, timestamp), EO_TYPECHECK(const void *, data)

/**
 * @def evas_canvas_event_feed_multi_move
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[in] d
 * @param[in] x
 * @param[in] y
 * @param[in] rad
 * @param[in] radx
 * @param[in] rady
 * @param[in] pres
 * @param[in] ang
 * @param[in] fx
 * @param[in] fy
 * @param[in] timestamp
 * @param[in] data
 *
 * @see evas_event_feed_multi_move
 */
#define evas_canvas_event_feed_multi_move(d, x, y, rad, radx, rady, pres, ang, fx, fy, timestamp, data) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_EVENT_FEED_MULTI_MOVE), EO_TYPECHECK(int, d), EO_TYPECHECK(int, x), EO_TYPECHECK(int, y), EO_TYPECHECK(double, rad), EO_TYPECHECK(double, radx), EO_TYPECHECK(double, rady), EO_TYPECHECK(double, pres), EO_TYPECHECK(double, ang), EO_TYPECHECK(double, fx), EO_TYPECHECK(double, fy), EO_TYPECHECK(unsigned int, timestamp), EO_TYPECHECK(const void *, data)

/**
 * @def evas_canvas_event_feed_key_down
 * @since 1.8
 *
 * Key down event feed
 *
 * @param[in] keyname
 * @param[in] key
 * @param[in] string
 * @param[in] compose
 * @param[in] timestamp
 * @param[in] data
 *
 * @see evas_event_feed_key_down
 */
#define evas_canvas_event_feed_key_down(keyname, key, string, compose, timestamp, data) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_EVENT_FEED_KEY_DOWN), EO_TYPECHECK(const char *, keyname), EO_TYPECHECK(const char *, key), EO_TYPECHECK(const char *, string), EO_TYPECHECK(const char *, compose), EO_TYPECHECK(unsigned int, timestamp), EO_TYPECHECK(const void *, data)

/**
 * @def evas_canvas_event_feed_key_up
 * @since 1.8
 *
 * Key up event feed
 *
 * @param[in] keyname
 * @param[in] key
 * @param[in] string
 * @param[in] compose
 * @param[in] timestamp
 * @param[in] data
 *
 * @see evas_event_feed_key_up
 */
#define evas_canvas_event_feed_key_up(keyname, key, string, compose, timestamp, data) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_EVENT_FEED_KEY_UP), EO_TYPECHECK(const char *, keyname), EO_TYPECHECK(const char *, key), EO_TYPECHECK(const char *, string), EO_TYPECHECK(const char *, compose), EO_TYPECHECK(unsigned int, timestamp), EO_TYPECHECK(const void *, data)

/**
 * @def evas_canvas_event_feed_hold
 * @since 1.8
 *
 * Hold event feed
 *
 * @param[in] hold
 * @param[in] timestamp
 * @param[in] data
 *
 * @see evas_event_feed_hold
 */
#define evas_canvas_event_feed_hold(hold, timestamp, data) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_EVENT_FEED_HOLD), EO_TYPECHECK(int, hold), EO_TYPECHECK(unsigned int, timestamp), EO_TYPECHECK(const void *, data)

/**
 * @def evas_canvas_event_refeed_event
 * @since 1.8
 *
 * Re feed event.
 *
 * @param[in] event_copy
 * @param[in] event_type
 *
 * @see evas_event_refeed_event
 */
#define evas_canvas_event_refeed_event(event_copy, event_type) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_EVENT_REFEED_EVENT), EO_TYPECHECK(void *, event_copy), EO_TYPECHECK(Evas_Callback_Type, event_type)

/**
 * @def evas_canvas_tree_objects_at_xy_get
 * @since 1.8
 *
 * Retrieve a list of Evas objects lying over a given position in
 * a canvas.
 *
 * @param[in] stop An Evas Object where to stop searching.
 * @param[in] x The horizontal coordinate of the position.
 * @param[in] y The vertical coordinate of the position.
 * @param[out] list of Evas Objects.
 *
 * @see evas_tree_objects_at_xy_get
 */
#define evas_canvas_tree_objects_at_xy_get(stop, x, y, ret) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_TREE_OBJECTS_AT_XY_GET), EO_TYPECHECK(Evas_Object *, stop), EO_TYPECHECK(int, x), EO_TYPECHECK(int, y), EO_TYPECHECK(Eina_List **, ret)

/**
 * @}
 */

/**
 * @ingroup Evas_Object_Group_Find
 *
 * @{
 */

/**
 * @def evas_canvas_focus_get
 * @since 1.8
 *
 * Retrieve the object that currently has focus.
 *
 * @param[out] ret
 *
 * @see evas_focus_get
 */
#define evas_canvas_focus_get(ret) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_FOCUS_GET), EO_TYPECHECK(Evas_Object **, ret)

/**
 * @def evas_canvas_object_name_find
 * @since 1.8
 *
 * Retrieves the object on the given evas with the given name.
 *
 * @param[in] name
 * @param[out] ret
 *
 * @see evas_object_name_find
 */
#define evas_canvas_object_name_find(name, ret) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_OBJECT_NAME_FIND),EO_TYPECHECK(const char *, name),  EO_TYPECHECK(Evas_Object **, ret)

/**
 * @def evas_obj_name_child_find
 * @since 1.8
 *
 * Retrieves the object from children of the given object with the given name.
 * @param[in] name in
 * @param[in] recurse in
 * @param[out] child out
 *
 * @see evas_object_name_child_find
 */
#define evas_obj_name_child_find(name, recurse, child) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_NAME_CHILD_FIND), EO_TYPECHECK(const char *, name), EO_TYPECHECK(int, recurse), EO_TYPECHECK(Evas_Object **, child)

/**
 * @def evas_canvas_object_top_at_xy_get
 * @since 1.8
 *
 * Retrieve the Evas object stacked at the top of a given position in
 * a canvas.
 *
 * @param[in] x
 * @param[in] y
 * @param[in] include_pass_events_objects
 * @param[in] include_hidden_objects
 * @param[out] ret
 *
 * @see evas_object_top_at_xy_get
 */
#define evas_canvas_object_top_at_xy_get(x, y, include_pass_events_objects, include_hidden_objects, ret) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_OBJECT_TOP_AT_XY_GET), EO_TYPECHECK(Evas_Coord, x), EO_TYPECHECK(Evas_Coord, y), EO_TYPECHECK(Eina_Bool, include_pass_events_objects), EO_TYPECHECK(Eina_Bool, include_hidden_objects), EO_TYPECHECK(Evas_Object **, ret)

/**
 * @def evas_canvas_object_top_in_rectangle_get
 * @since 1.8
 *
 * Retrieve the Evas object stacked at the top of a given rectangular
 * region in a canvas.
 *
 * @param[in] x
 * @param[in] y
 * @param[in] w
 * @param[in] h
 * @param[in] include_pass_events_objects
 * @param[in] include_hidden_objects
 * @param[out] ret
 *
 * @see evas_object_top_in_rectangle_get
 */
#define evas_canvas_object_top_in_rectangle_get(x, y, w, h, include_pass_events_objects, include_hidden_objects, ret) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_OBJECT_TOP_IN_RECTANGLE_GET), EO_TYPECHECK(Evas_Coord, x), EO_TYPECHECK(Evas_Coord, y), EO_TYPECHECK(Evas_Coord, w), EO_TYPECHECK(Evas_Coord, h), EO_TYPECHECK(Eina_Bool, include_pass_events_objects), EO_TYPECHECK(Eina_Bool, include_hidden_objects), EO_TYPECHECK(Evas_Object **, ret)

/**
 * @def evas_canvas_objects_at_xy_get
 * @since 1.8
 *
 * Retrieve a list of Evas objects lying over a given position in
 * a canvas.
 *
 * @param[in] x
 * @param[in] y
 * @param[in] include_pass_events_objects
 * @param[in] include_hidden_objects
 * @param[out] ret
 *
 * @see evas_objects_at_xy_get
 */
#define evas_canvas_objects_at_xy_get(x, y, include_pass_events_objects, include_hidden_objects, ret) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_OBJECTS_AT_XY_GET), EO_TYPECHECK(Evas_Coord, x), EO_TYPECHECK(Evas_Coord, y), EO_TYPECHECK(Eina_Bool, include_pass_events_objects), EO_TYPECHECK(Eina_Bool, include_hidden_objects), EO_TYPECHECK(Eina_List **, ret)

/**
 * @def evas_canvas_objects_in_rectangle_get
 * @since 1.8
 *
 * @param[in] x
 * @param[in] y
 * @param[in] w
 * @param[in] h
 * @param[in] include_pass_events_objects
 * @param[in] include_hidden_objects
 * @param[out] ret
 *
 * @see evas_objects_in_rectangle_get
 */
#define evas_canvas_objects_in_rectangle_get(x, y, w, h, include_pass_events_objects, include_hidden_objects, ret) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_OBJECTS_IN_RECTANGLE_GET), EO_TYPECHECK(Evas_Coord, x), EO_TYPECHECK(Evas_Coord, y), EO_TYPECHECK(Evas_Coord, w), EO_TYPECHECK(Evas_Coord, h), EO_TYPECHECK(Eina_Bool, include_pass_events_objects), EO_TYPECHECK(Eina_Bool, include_hidden_objects), EO_TYPECHECK(Eina_List **, ret)

/**
 * @def evas_canvas_object_bottom_get
 * @since 1.8
 *
 * Get the lowest (stacked) Evas object on the canvas e.
 *
 * @param[out] ret
 *
 * @see evas_object_bottom_get
 */
#define evas_canvas_object_bottom_get(ret) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_OBJECT_BOTTOM_GET), EO_TYPECHECK(Evas_Object **, ret)

/**
 * @def evas_canvas_object_top_get
 * @since 1.8
 *
 * Get the highest (stacked) Evas object on the canvas e.
 *
 * @param[out] ret
 *
 * @see evas_object_top_get
 */
#define evas_canvas_object_top_get(ret) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_OBJECT_TOP_GET), EO_TYPECHECK(Evas_Object **, ret)

/**
 * @}
 */

/**
 * @ingroup Evas_Font_Path_Group
 *
 * @{
 */

/**
 * @def evas_canvas_font_path_clear
 * @since 1.8
 *
 * Removes all font paths loaded into memory for the given evas.
 *
 *
 * @see evas_font_path_clear
 */
#define evas_canvas_font_path_clear() EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_FONT_PATH_CLEAR)

/**
 * @def evas_canvas_font_path_append
 * @since 1.8
 *
 * Appends a font path to the list of font paths used by the given evas.
 *
 * @param[in] path
 *
 * @see evas_font_path_append
 */
#define evas_canvas_font_path_append(path) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_FONT_PATH_APPEND), EO_TYPECHECK(const char *, path)

/**
 * @def evas_canvas_font_path_prepend
 * @since 1.8
 *
 * Prepends a font path to the list of font paths used by the given evas.
 *
 * @param[in] path
 *
 * @see evas_font_path_prepend
 */
#define evas_canvas_font_path_prepend(path) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_FONT_PATH_PREPEND), EO_TYPECHECK(const char *, path)

/**
 * @def evas_canvas_font_path_list
 * @since 1.8
 *
 * Retrieves the list of font paths used by the given evas.
 *
 * @param[out] ret
 *
 * @see evas_font_path_list
 */
#define evas_canvas_font_path_list(ret) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_FONT_PATH_LIST), EO_TYPECHECK(const Eina_List **, ret)
/**
 * @}
 */

/**
 * @ingroup Evas_Font_Group
 *
 * @{
 */

/**
 * @def evas_canvas_font_hinting_set
 * @since 1.8
 *
 * Changes the font hinting for the given evas.
 *
 * @param[in] hinting
 *
 * @see evas_font_hinting_set
 */
#define evas_canvas_font_hinting_set(hinting) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_FONT_HINTING_SET), EO_TYPECHECK(Evas_Font_Hinting_Flags, hinting)

/**
 * @def evas_canvas_font_hinting_get
 * @since 1.8
 *
 * Retrieves the font hinting used by the given evas.
 *
 * @param[out] ret
 *
 * @see evas_font_hinting_get
 */
#define evas_canvas_font_hinting_get(ret) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_FONT_HINTING_GET), EO_TYPECHECK(Evas_Font_Hinting_Flags *, ret)

/**
 * @def evas_canvas_font_hinting_can_hint
 * @since 1.8
 *
 * Checks if the font hinting is supported by the given evas.
 *
 * @param[in] hinting
 * @param[out] ret
 *
 * @see evas_font_hinting_can_hint
 */
#define evas_canvas_font_hinting_can_hint(hinting, ret) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_FONT_HINTING_CAN_HINT), EO_TYPECHECK(Evas_Font_Hinting_Flags, hinting), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def evas_canvas_font_cache_flush
 * @since 1.8
 *
 * Force the given evas and associated engine to flush its font cache.
 *
 *
 * @see evas_font_cache_flush
 */
#define evas_canvas_font_cache_flush() EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_FONT_CACHE_FLUSH)

/**
 * @def evas_canvas_font_cache_set
 * @since 1.8
 *
 * Changes the size of font cache of the given evas.
 *
 * @param[in] size
 *
 * @see evas_font_cache_set
 */
#define evas_canvas_font_cache_set(size) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_FONT_CACHE_SET), EO_TYPECHECK(int, size)

/**
 * @def evas_canvas_font_cache_get
 * @since 1.8
 *
 * Changes the size of font cache of the given evas.
 *
 * @param[out] ret
 *
 * @see evas_font_cache_get
 */
#define evas_canvas_font_cache_get(ret) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_FONT_CACHE_GET), EO_TYPECHECK(int *, ret)

/**
 * @def evas_canvas_font_available_list
 * @since 1.8
 *
 * List of available font descriptions known or found by this evas.
 *
 * @param[out] ret
 *
 * @see evas_font_available_list
 */
#define evas_canvas_font_available_list(ret) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_FONT_AVAILABLE_LIST), EO_TYPECHECK(Eina_List **, ret)

/**
 * @}
 */

/**
 * @ingroup Evas_Keys
 *
 * @{
 */

/**
 * @def evas_canvas_key_modifier_get
 * @since 1.8
 *
 * Returns a handle to the list of modifier keys registered in the
 *
 * @param[out] ret
 *
 * @see evas_key_modifier_get
 */
#define evas_canvas_key_modifier_get(ret) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_KEY_MODIFIER_GET), EO_TYPECHECK(const Evas_Modifier **, ret)

/**
 * @def evas_canvas_key_lock_get
 * @since 1.8
 *
 * Returns a handle to the list of lock keys registered in the canvas
 *
 * @param[out] ret
 *
 * @see evas_key_lock_get
 */
#define evas_canvas_key_lock_get(ret) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_KEY_LOCK_GET), EO_TYPECHECK(const Evas_Lock **, ret)

/**
 * @def evas_canvas_key_modifier_add
 * @since 1.8
 *
 * Adds the keyname key to the current list of modifier keys.
 *
 * @param[in] keyname
 *
 * @see evas_key_modifier_add
 */
#define evas_canvas_key_modifier_add(keyname) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_KEY_MODIFIER_ADD), EO_TYPECHECK(const char *, keyname)

/**
 * @def evas_canvas_key_modifier_del
 * @since 1.8
 *
 * Removes the keyname key from the current list of modifier keys
 *
 * @param[in] keyname
 *
 * @see evas_key_modifier_del
 */
#define evas_canvas_key_modifier_del(keyname) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_KEY_MODIFIER_DEL), EO_TYPECHECK(const char *, keyname)

/**
 * @def evas_canvas_key_lock_add
 * @since 1.8
 *
 * Adds the keyname key to the current list of lock keys.
 *
 * @param[in] keyname
 *
 * @see evas_key_lock_add
 */
#define evas_canvas_key_lock_add(keyname) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_KEY_LOCK_ADD), EO_TYPECHECK(const char *, keyname)

/**
 * @def evas_canvas_key_lock_del
 * @since 1.8
 *
 * Removes the keyname key from the current list of lock keys on
 *
 * @param[in] keyname
 *
 * @see evas_key_lock_del
 */
#define evas_canvas_key_lock_del(keyname) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_KEY_LOCK_DEL), EO_TYPECHECK(const char *, keyname)

/**
 * @def evas_canvas_key_modifier_on
 * @since 1.8
 *
 * Enables or turns on programmatically the modifier key with name @p keyname.
 *
 * @param[in] keyname
 *
 * @see evas_key_modifier_on
 */
#define evas_canvas_key_modifier_on(keyname) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_KEY_MODIFIER_ON), EO_TYPECHECK(const char *, keyname)

/**
 * @def evas_canvas_key_modifier_off
 * @since 1.8
 *
 * Disables or turns off programmatically the modifier key with name @p keyname
 *
 * @param[in] keyname
 *
 * @see evas_key_modifier_off
 */
#define evas_canvas_key_modifier_off(keyname) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_KEY_MODIFIER_OFF), EO_TYPECHECK(const char *, keyname)

/**
 * @def evas_canvas_key_lock_on
 * @since 1.8
 *
 * Enables or turns on programmatically the lock key with name @p keyname
 *
 * @param[in] keyname
 *
 * @see evas_key_lock_on
 */
#define evas_canvas_key_lock_on(keyname) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_KEY_LOCK_ON), EO_TYPECHECK(const char *, keyname)

/**
 * @def evas_canvas_key_lock_off
 * @since 1.8
 *
 * Disables or turns off programmatically the lock key with name @p keyname
 *
 * @param[in] keyname
 *
 * @see evas_key_lock_off
 */
#define evas_canvas_key_lock_off(keyname) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_KEY_LOCK_OFF), EO_TYPECHECK(const char *, keyname)

/**
 * @def evas_canvas_key_modifier_mask_get
 * @since 1.8
 *
 * Creates a bit mask from the keyname @b modifier key.
 *
 * @param[in] keyname
 * @param[out] ret
 *
 * @see evas_key_modifier_mask_get
 */
#define evas_canvas_key_modifier_mask_get(keyname, ret) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_KEY_MODIFIER_MASK_GET), EO_TYPECHECK(const char *, keyname), EO_TYPECHECK(Evas_Modifier_Mask *, ret)
/**
 * @}
 */

/**
 * @ingroup Evas_Touch_Point_List
 *
 * @{
 */
/**
 * @def evas_canvas_touch_point_list_count
 * @since 1.8
 *
 * Get the number of touched point in the evas.
 *
 * @param[out] ret
 *
 * @see evas_touch_point_list_count
 */
#define evas_canvas_touch_point_list_count(ret) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_TOUCH_POINT_LIST_COUNT), EO_TYPECHECK(unsigned int *, ret)

/**
 * @def evas_canvas_touch_point_list_nth_xy_get
 * @since 1.8
 *
 * This function returns the nth touch point's co-ordinates.
 *
 * @param[in] n
 * @param[out] x
 * @param[out] y
 *
 * @see evas_touch_point_list_nth_xy_get
 */
#define evas_canvas_touch_point_list_nth_xy_get(n, x, y) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_TOUCH_POINT_LIST_NTH_XY_GET), EO_TYPECHECK(unsigned int, n), EO_TYPECHECK(Evas_Coord *, x), EO_TYPECHECK(Evas_Coord *, y)

/**
 * @def evas_canvas_touch_point_list_nth_id_get
 * @since 1.8
 *
 * This function returns the id of nth touch point.
 *
 * @param[in] n
 * @param[out] ret
 *
 * @see evas_touch_point_list_nth_id_get
 */
#define evas_canvas_touch_point_list_nth_id_get(n, ret) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_TOUCH_POINT_LIST_NTH_ID_GET), EO_TYPECHECK(unsigned int, n), EO_TYPECHECK(int *, ret)

/**
 * @def evas_canvas_touch_point_list_nth_state_get
 * @since 1.8
 *
 * This function returns the state of nth touch point.
 *
 * @param[in] n
 * @param[out] ret
 *
 * @see evas_touch_point_list_nth_state_get
 */
#define evas_canvas_touch_point_list_nth_state_get(n, ret) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_TOUCH_POINT_LIST_NTH_STATE_GET), EO_TYPECHECK(unsigned int, n), EO_TYPECHECK(Evas_Touch_Point_State *, ret)
/**
 * @}
 */

/**
 * @ingroup Evas_Image_Group
 *
 * @{
 */

/**
 * @def evas_canvas_image_cache_flush
 * @since 1.8
 *
 * Flush the image cache of the canvas.
 *
 *
 * @see evas_image_cache_flush
 */
#define evas_canvas_image_cache_flush() EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_IMAGE_CACHE_FLUSH)

/**
 * @def evas_canvas_image_cache_reload
 * @since 1.8
 *
 * Reload the image cache
 *
 *
 * @see evas_image_cache_reload
 */
#define evas_canvas_image_cache_reload() EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_IMAGE_CACHE_RELOAD)

/**
 * @def evas_canvas_image_cache_set
 * @since 1.8
 *
 * Set the image cache.
 *
 * @param[in] size
 *
 * @see evas_image_cache_set
 */
#define evas_canvas_image_cache_set(size) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_IMAGE_CACHE_SET), EO_TYPECHECK(int, size)

/**
 * @def evas_canvas_image_cache_get
 * @since 1.8
 *
 * Get the image cache
 *
 * @param[out] ret
 *
 * @see evas_image_cache_get
 */
#define evas_canvas_image_cache_get(ret) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_IMAGE_CACHE_GET), EO_TYPECHECK(int *, ret)

/**
 * @def evas_canvas_image_max_size_get
 * @since 1.8
 *
 * Get the maximum image size evas can possibly handle
 *
 * @param[out] maxw
 * @param[out] maxh
 * @param[out] ret
 *
 * @see evas_image_max_size_get
 */
#define evas_canvas_image_max_size_get(maxw, maxh, ret) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_IMAGE_MAX_SIZE_GET), EO_TYPECHECK(int *, maxw), EO_TYPECHECK(int *, maxh), EO_TYPECHECK(Eina_Bool *, ret)
/**
 * @}
 */

/**
 * @ingroup Evas_Smart_Object_Group
 *
 * @{
 */

/**
 * @def evas_canvas_smart_objects_calculate
 * @since 1.8
 *
 * Call user-provided calculate() smart functions and unset the
 * flag signalling that the object needs to get recalculated to @b all
 * smart objects in the canvas.
 *
 *
 * @see evas_smart_objects_calculate
 */
#define evas_canvas_smart_objects_calculate() EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_SMART_OBJECTS_CALCULATE)

/**
 * @def evas_canvas_smart_objects_calculate_count_get
 * @since 1.8
 *
 * This gets the internal counter that counts the number of smart calculations
 *
 * @param[out] ret
 *
 * @see evas_smart_objects_calculate_count_get
 */
#define evas_canvas_smart_objects_calculate_count_get(ret) EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_SMART_OBJECTS_CALCULATE_COUNT_GET), EO_TYPECHECK(int *, ret)

/**
 * @}
 */

/**
 * @ingroup Evas_Object_Rectangle
 *
 * @{
 */

#define EVAS_OBJ_RECTANGLE_CLASS evas_object_rectangle_class_get()

const Eo_Class *evas_object_rectangle_class_get(void) EINA_CONST;

extern EAPI Eo_Op EVAS_OBJ_RECTANGLE_BASE_ID;

enum
{
   EVAS_OBJ_RECTANGLE_SUB_ID_LAST
};

#define EVAS_OBJ_RECTANGLE_ID(sub_id) (EVAS_OBJ_RECTANGLE_BASE_ID + sub_id)

/**
 * @}
 */

/**
 * @ingroup Evas_Object_Text
 *
 * @{
 */

#define EVAS_OBJ_TEXT_CLASS evas_object_text_class_get()

const Eo_Class *evas_object_text_class_get(void) EINA_CONST;

extern EAPI Eo_Op EVAS_OBJ_TEXT_BASE_ID;

enum
{
   EVAS_OBJ_TEXT_SUB_ID_FONT_SOURCE_SET,
   EVAS_OBJ_TEXT_SUB_ID_FONT_SOURCE_GET,
   EVAS_OBJ_TEXT_SUB_ID_FONT_SET,
   EVAS_OBJ_TEXT_SUB_ID_FONT_GET,
   EVAS_OBJ_TEXT_SUB_ID_TEXT_SET,
   EVAS_OBJ_TEXT_SUB_ID_BIDI_DELIMITERS_SET,
   EVAS_OBJ_TEXT_SUB_ID_BIDI_DELIMITERS_GET,
   EVAS_OBJ_TEXT_SUB_ID_TEXT_GET,
   EVAS_OBJ_TEXT_SUB_ID_DIRECTION_GET,
   EVAS_OBJ_TEXT_SUB_ID_ASCENT_GET,
   EVAS_OBJ_TEXT_SUB_ID_DESCENT_GET,
   EVAS_OBJ_TEXT_SUB_ID_MAX_ASCENT_GET,
   EVAS_OBJ_TEXT_SUB_ID_MAX_DESCENT_GET,
   EVAS_OBJ_TEXT_SUB_ID_INSET_GET,
   EVAS_OBJ_TEXT_SUB_ID_HORIZ_ADVANCE_GET,
   EVAS_OBJ_TEXT_SUB_ID_VERT_ADVANCE_GET,
   EVAS_OBJ_TEXT_SUB_ID_CHAR_POS_GET,
   EVAS_OBJ_TEXT_SUB_ID_LAST_UP_TO_POS,
   EVAS_OBJ_TEXT_SUB_ID_CHAR_COORDS_GET,
   EVAS_OBJ_TEXT_SUB_ID_STYLE_SET,
   EVAS_OBJ_TEXT_SUB_ID_STYLE_GET,
   EVAS_OBJ_TEXT_SUB_ID_SHADOW_COLOR_SET,
   EVAS_OBJ_TEXT_SUB_ID_SHADOW_COLOR_GET,
   EVAS_OBJ_TEXT_SUB_ID_GLOW_COLOR_SET,
   EVAS_OBJ_TEXT_SUB_ID_GLOW_COLOR_GET,
   EVAS_OBJ_TEXT_SUB_ID_GLOW2_COLOR_SET,
   EVAS_OBJ_TEXT_SUB_ID_GLOW2_COLOR_GET,
   EVAS_OBJ_TEXT_SUB_ID_OUTLINE_COLOR_SET,
   EVAS_OBJ_TEXT_SUB_ID_OUTLINE_COLOR_GET,
   EVAS_OBJ_TEXT_SUB_ID_STYLE_PAD_GET,
   EVAS_OBJ_TEXT_SUB_ID_ELLIPSIS_SET,
   EVAS_OBJ_TEXT_SUB_ID_ELLIPSIS_GET,
   EVAS_OBJ_TEXT_SUB_ID_FILTER_PROGRAM_SET,
   EVAS_OBJ_TEXT_SUB_ID_FILTER_SOURCE_SET,
   EVAS_OBJ_TEXT_SUB_ID_LAST
};

#define EVAS_OBJ_TEXT_ID(sub_id) (EVAS_OBJ_TEXT_BASE_ID + sub_id)

/**
 * @def evas_obj_text_font_source_set
 * @since 1.8
 *
 * Set the font (source) file to be used on a given text object.
 *
 * @param[in] font_source in
 *
 * @see evas_object_text_font_source_set
 */
#define evas_obj_text_font_source_set(font_source) EVAS_OBJ_TEXT_ID(EVAS_OBJ_TEXT_SUB_ID_FONT_SOURCE_SET), EO_TYPECHECK(const char *, font_source)

/**
 * @def evas_obj_text_font_source_get
 * @since 1.8
 *
 * Get the font file's path which is being used on a given text
 * object.
 *
 * @param[out] font_source out
 *
 * @see evas_object_text_font_source_get
 */
#define evas_obj_text_font_source_get(font_source) EVAS_OBJ_TEXT_ID(EVAS_OBJ_TEXT_SUB_ID_FONT_SOURCE_GET), EO_TYPECHECK(const char **, font_source)

/**
 * @def evas_obj_text_font_set
 * @since 1.8
 *
 * Set the font family and size on a given text object.
 *
 * @param[in] font in
 * @param[in] size in
 *
 * @see evas_object_text_font_set
 */
#define evas_obj_text_font_set(font, size) EVAS_OBJ_TEXT_ID(EVAS_OBJ_TEXT_SUB_ID_FONT_SET), EO_TYPECHECK(const char *, font), EO_TYPECHECK(Evas_Font_Size, size)

/**
 * @def evas_obj_text_font_get
 * @since 1.8
 *
 * Retrieve the font family and size in use on a given text object.
 *
 * @param[out] font out
 * @param[out] size out
 *
 * @see evas_object_text_font_get
 */
#define evas_obj_text_font_get(font, size) EVAS_OBJ_TEXT_ID(EVAS_OBJ_TEXT_SUB_ID_FONT_GET), EO_TYPECHECK(const char **, font), EO_TYPECHECK(Evas_Font_Size *, size)

/**
 * @def evas_obj_text_text_set
 * @since 1.8
 *
 * Sets the text string to be displayed by the given text object.
 *
 * @param[in] text
 *
 * @see evas_object_text_text_set
 */
#define evas_obj_text_text_set(text) EVAS_OBJ_TEXT_ID(EVAS_OBJ_TEXT_SUB_ID_TEXT_SET), EO_TYPECHECK(const char *, text)

/**
 * @def evas_obj_text_text_get
 * @since 1.8
 *
 * Retrieves the text string currently being displayed by the given
 * text object.
 *
 * @param[out] text out
 *
 * @see evas_object_text_text_get
 */
#define evas_obj_text_text_get(text) EVAS_OBJ_TEXT_ID(EVAS_OBJ_TEXT_SUB_ID_TEXT_GET), EO_TYPECHECK(const char **, text)

/**
 * @def evas_obj_text_bidi_delimiters_set
 * @since 1.8
 *
 * Sets the BiDi delimiters used in the textblock.
 *
 * @param[in] delim in
 *
 * @see evas_object_text_bidi_delimiters_set
 */
#define evas_obj_text_bidi_delimiters_set(delim) EVAS_OBJ_TEXT_ID(EVAS_OBJ_TEXT_SUB_ID_BIDI_DELIMITERS_SET), EO_TYPECHECK(const char *, delim)

/**
 * @def evas_obj_text_bidi_delimiters_get
 * @since 1.8
 *
 * Gets the BiDi delimiters used in the textblock.
 *
 * @param[out] delim out
 *
 * @see evas_object_text_bidi_delimiters_get
 */
#define evas_obj_text_bidi_delimiters_get(delim) EVAS_OBJ_TEXT_ID(EVAS_OBJ_TEXT_SUB_ID_BIDI_DELIMITERS_GET), EO_TYPECHECK(const char **, delim)

/**
 * @def evas_obj_text_direction_get
 * @since 1.8
 *
 * Retrieves the direction of the text currently being displayed in the
 * text object.
 *
 * @param[out] bidi_dir out
 *
 * @see evas_object_text_direction_get
 */
#define evas_obj_text_direction_get(bidi_dir) EVAS_OBJ_TEXT_ID(EVAS_OBJ_TEXT_SUB_ID_DIRECTION_GET), EO_TYPECHECK(Evas_BiDi_Direction *, bidi_dir)

/**
 * @def evas_obj_text_ascent_get
 * @since 1.8
 *
 * @param[out] ascent out
 *
 * @see evas_object_text_ascent_get
 */
#define evas_obj_text_ascent_get(ascent) EVAS_OBJ_TEXT_ID(EVAS_OBJ_TEXT_SUB_ID_ASCENT_GET), EO_TYPECHECK(Evas_Coord *, ascent)

/**
 * @def evas_obj_text_descent_get
 * @since 1.8
 *
 * @param[out] descent out
 *
 * @see evas_object_text_descent_get
 */
#define evas_obj_text_descent_get(descent) EVAS_OBJ_TEXT_ID(EVAS_OBJ_TEXT_SUB_ID_DESCENT_GET), EO_TYPECHECK(Evas_Coord *, descent)

/**
 * @def evas_obj_text_max_ascent_get
 * @since 1.8
 *
 * @param[out] max_ascent out
 *
 * @see evas_object_text_max_ascent_get
 */
#define evas_obj_text_max_ascent_get(max_ascent) EVAS_OBJ_TEXT_ID(EVAS_OBJ_TEXT_SUB_ID_MAX_ASCENT_GET), EO_TYPECHECK(Evas_Coord *, max_ascent)

/**
 * @def evas_obj_text_max_descent_get
 * @since 1.8
 *
 * @param[out] max_descent out
 *
 * @see evas_object_text_max_descent_get
 */
#define evas_obj_text_max_descent_get(max_descent) EVAS_OBJ_TEXT_ID(EVAS_OBJ_TEXT_SUB_ID_MAX_DESCENT_GET), EO_TYPECHECK(Evas_Coord *, max_descent)

/**
 * @def evas_obj_text_inset_get
 * @since 1.8
 *
 * @param[out] inset out
 *
 * @see evas_object_text_inset_get
 */
#define evas_obj_text_inset_get(inset) EVAS_OBJ_TEXT_ID(EVAS_OBJ_TEXT_SUB_ID_INSET_GET), EO_TYPECHECK(Evas_Coord *, inset)

/**
 * @def evas_obj_text_horiz_advance_get
 * @since 1.8
 *
 * @param[out] horiz out
 *
 * @see evas_object_text_horiz_advance_get
 */
#define evas_obj_text_horiz_advance_get(horiz) EVAS_OBJ_TEXT_ID(EVAS_OBJ_TEXT_SUB_ID_HORIZ_ADVANCE_GET), EO_TYPECHECK(Evas_Coord *, horiz)

/**
 * @def evas_obj_text_vert_advance_get
 * @since 1.8
 *
 * @param[out] vert out
 *
 * @see evas_object_text_vert_advance_get
 */
#define evas_obj_text_vert_advance_get(vert) EVAS_OBJ_TEXT_ID(EVAS_OBJ_TEXT_SUB_ID_VERT_ADVANCE_GET), EO_TYPECHECK(Evas_Coord *, vert)

/**
 * @def evas_obj_text_char_pos_get
 * @since 1.8
 *
 * Retrieve position and dimension information of a character within a text @c Evas_Object.
 *
 * @param[in] pos in
 * @param[out] cx out
 * @param[out] cy out
 * @param[out] cw out
 * @param[out] ch out
 * @param[out] ret out
 *
 * @see evas_object_text_char_pos_get
 */
#define evas_obj_text_char_pos_get(pos, cx, cy, cw, ch, ret) EVAS_OBJ_TEXT_ID(EVAS_OBJ_TEXT_SUB_ID_CHAR_POS_GET), EO_TYPECHECK(int, pos), EO_TYPECHECK(Evas_Coord *, cx), EO_TYPECHECK(Evas_Coord *, cy), EO_TYPECHECK(Evas_Coord *, cw), EO_TYPECHECK(Evas_Coord *, ch), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def evas_obj_text_last_up_to_pos
 * @since 1.8
 *
 * Returns the logical position of the last char in the text
 * up to the pos given. this is NOT the position of the last char
 * because of the possibility of RTL in the text.
 *
 * @param[in] x in
 * @param[in] y in
 * @param[out] res out
 *
 * @see evas_object_text_last_up_to_pos
 */
#define evas_obj_text_last_up_to_pos(x, y, res) EVAS_OBJ_TEXT_ID(EVAS_OBJ_TEXT_SUB_ID_LAST_UP_TO_POS), EO_TYPECHECK(Evas_Coord, x), EO_TYPECHECK(Evas_Coord, y), EO_TYPECHECK(int *, res)
/**
 * @def evas_obj_text_char_coords_get
 * @since 1.8
 *
 * @param[in] x in
 * @param[in] y in
 * @param[out] cx out
 * @param[out] cy out
 * @param[out] cw out
 * @param[out] ch out
 * @param[out] res out
 *
 * @see evas_object_text_char_coords_get
 */
#define evas_obj_text_char_coords_get(x, y, cx, cy, cw, ch, res) EVAS_OBJ_TEXT_ID(EVAS_OBJ_TEXT_SUB_ID_CHAR_COORDS_GET), EO_TYPECHECK(Evas_Coord, x), EO_TYPECHECK(Evas_Coord, y), EO_TYPECHECK(Evas_Coord *, cx), EO_TYPECHECK(Evas_Coord *, cy), EO_TYPECHECK(Evas_Coord *, cw), EO_TYPECHECK(Evas_Coord *, ch), EO_TYPECHECK(int *, res)

/**
 * @def evas_obj_text_style_set
 * @since 1.8
 *
 * Sets the style to apply on the given text object.
 *
 * @param[in] style in
 *
 * @see evas_object_text_style_set
 */
#define evas_obj_text_style_set(style) EVAS_OBJ_TEXT_ID(EVAS_OBJ_TEXT_SUB_ID_STYLE_SET), EO_TYPECHECK(Evas_Text_Style_Type, style)

/**
 * @def evas_obj_text_style_get
 * @since 1.8
 *
 * Retrieves the style on use on the given text object.
 *
 * @param[out] style out
 *
 * @see evas_object_text_style_get
 */
#define evas_obj_text_style_get(style) EVAS_OBJ_TEXT_ID(EVAS_OBJ_TEXT_SUB_ID_STYLE_GET), EO_TYPECHECK(Evas_Text_Style_Type *, style)

/**
 * @def evas_obj_text_shadow_color_set
 * @since 1.8
 *
 * Sets the shadow color for the given text object.
 *
 * @param[in] r in
 * @param[in] g in
 * @param[in] b in
 * @param[in] a in
 *
 * @see evas_object_text_shadow_color_set
 */
#define evas_obj_text_shadow_color_set(r, g, b, a) EVAS_OBJ_TEXT_ID(EVAS_OBJ_TEXT_SUB_ID_SHADOW_COLOR_SET), EO_TYPECHECK(int, r), EO_TYPECHECK(int, g), EO_TYPECHECK(int, b), EO_TYPECHECK(int, a)

/**
 * @def evas_obj_text_shadow_color_get
 * @since 1.8
 *
 * Retrieves the shadow color for the given text object.
 *
 * @param[out] r out
 * @param[out] g out
 * @param[out] b out
 * @param[out] a out
 *
 * @see evas_object_text_shadow_color_get
 */
#define evas_obj_text_shadow_color_get(r, g, b, a) EVAS_OBJ_TEXT_ID(EVAS_OBJ_TEXT_SUB_ID_SHADOW_COLOR_GET), EO_TYPECHECK(int *, r), EO_TYPECHECK(int *, g), EO_TYPECHECK(int *, b), EO_TYPECHECK(int *, a)

/**
 * @def evas_obj_text_glow_color_set
 * @since 1.8
 *
 * Sets the glow color for the given text object.
 *
 * @param[in] r in
 * @param[in] g in
 * @param[in] b in
 * @param[in] a in
 *
 * @see evas_object_text_glow_color_set
 */
#define evas_obj_text_glow_color_set(r, g, b, a) EVAS_OBJ_TEXT_ID(EVAS_OBJ_TEXT_SUB_ID_GLOW_COLOR_SET), EO_TYPECHECK(int, r), EO_TYPECHECK(int, g), EO_TYPECHECK(int, b), EO_TYPECHECK(int, a)

/**
 * @def evas_obj_text_glow_color_get
 * @since 1.8
 *
 * Retrieves the glow color for the given text object.
 *
 * @param[out] r out
 * @param[out] g out
 * @param[out] b out
 * @param[out] a out
 *
 * @see evas_object_text_glow_color_get
 */
#define evas_obj_text_glow_color_get(r, g, b, a) EVAS_OBJ_TEXT_ID(EVAS_OBJ_TEXT_SUB_ID_GLOW_COLOR_GET), EO_TYPECHECK(int *, r), EO_TYPECHECK(int *, g), EO_TYPECHECK(int *, b), EO_TYPECHECK(int *, a)

/**
 * @def evas_obj_text_glow2_color_set
 * @since 1.8
 *
 * Sets the 'glow 2' color for the given text object.
 *
 * @param[in] r in
 * @param[in] g in
 * @param[in] b in
 * @param[in] a in
 *
 * @see evas_object_text_glow2_color_set
 */
#define evas_obj_text_glow2_color_set(r, g, b, a) EVAS_OBJ_TEXT_ID(EVAS_OBJ_TEXT_SUB_ID_GLOW2_COLOR_SET), EO_TYPECHECK(int, r), EO_TYPECHECK(int, g), EO_TYPECHECK(int, b), EO_TYPECHECK(int, a)

/**
 * @def evas_obj_text_glow2_color_get
 * @since 1.8
 *
 * Retrieves the 'glow 2' color for the given text object.
 *
 * @param[out] r out
 * @param[out] g out
 * @param[out] b out
 * @param[out] a out
 *
 * @see evas_object_text_glow2_color_get
 */
#define evas_obj_text_glow2_color_get(r, g, b, a) EVAS_OBJ_TEXT_ID(EVAS_OBJ_TEXT_SUB_ID_GLOW2_COLOR_GET), EO_TYPECHECK(int *, r), EO_TYPECHECK(int *, g), EO_TYPECHECK(int *, b), EO_TYPECHECK(int *, a)

/**
 * @def evas_obj_text_outline_color_set
 * @since 1.8
 *
 * Sets the outline color for the given text object.
 *
 * @param[in] r in
 * @param[in] g in
 * @param[in] b in
 * @param[in] a in
 *
 * @see evas_object_text_outline_color_set
 */
#define evas_obj_text_outline_color_set(r, g, b, a) EVAS_OBJ_TEXT_ID(EVAS_OBJ_TEXT_SUB_ID_OUTLINE_COLOR_SET), EO_TYPECHECK(int, r), EO_TYPECHECK(int, g), EO_TYPECHECK(int, b), EO_TYPECHECK(int, a)

/**
 * @def evas_obj_text_outline_color_get
 * @since 1.8
 *
 * Retrieves the outline color for the given text object.
 *
 * @param[out] r out
 * @param[out] g out
 * @param[out] b out
 * @param[out] a out
 *
 * @see evas_object_text_outline_color_get
 */
#define evas_obj_text_outline_color_get(r, g, b, a) EVAS_OBJ_TEXT_ID(EVAS_OBJ_TEXT_SUB_ID_OUTLINE_COLOR_GET), EO_TYPECHECK(int *, r), EO_TYPECHECK(int *, g), EO_TYPECHECK(int *, b), EO_TYPECHECK(int *, a)

/**
 * @def evas_obj_text_style_pad_get
 * @since 1.8
 *
 * Gets the text style pad of a text object.
 *
 * @param[out] l out
 * @param[out] r out
 * @param[out] t out
 * @param[out] b out
 *
 * @see evas_object_text_style_pad_get
 */
#define evas_obj_text_style_pad_get(l, r, t, b) EVAS_OBJ_TEXT_ID(EVAS_OBJ_TEXT_SUB_ID_STYLE_PAD_GET), EO_TYPECHECK(int *, l), EO_TYPECHECK(int *, r), EO_TYPECHECK(int *, t), EO_TYPECHECK(int *, b)


/**
 * @def evas_obj_text_ellipsis_set
 * @since 1.8
 *
 * Sets the ellipsis to apply on the given text object.
 * 0.0 -> means apply ellipsis on the right end of the text,
 * 1.0 -> means apply ellipsis on the start left of the text.
 *
 * @param[in] ellipsis in
 *
 * @see evas_object_text_ellipsis_get
 */
#define evas_obj_text_ellipsis_set(ellipsis) EVAS_OBJ_TEXT_ID(EVAS_OBJ_TEXT_SUB_ID_ELLIPSIS_SET), EO_TYPECHECK(double, ellipsis)

/**
 * @def evas_obj_text_ellipsis_get
 * @since 1.8
 *
 * Sets the ellipsis to apply on the given text object.
 * 0.0 -> means apply ellipsis on the right end of the text,
 * 1.0 -> means apply ellipsis on the start left of the text.
 * value below 0, means no ellipsis.
 *
 * @param[out] ellipsis out
 *
 * @see evas_object_text_ellipsis_set
 */
#define evas_obj_text_ellipsis_get(ellipsis) EVAS_OBJ_TEXT_ID(EVAS_OBJ_TEXT_SUB_ID_ELLIPSIS_GET), EO_TYPECHECK(double *, ellipsis)

/**
 * @def evas_obj_text_filter_program_set
 *
 * Set an Evas filter program on this Text Object.
 * If the program fails to compile (syntax error, invalid buffer name, etc...),
 * the standard text effects will be applied instead (SHADOW, etc...).
 *
 * @param[in] program  The program code, as defined by the @ref evasfiltersref
 *  "Evas filters script language". Pass NULL to remove the former program and
 *  switch back to the standard text effects.
 *
 * @since 1.9
 * @note EXPERIMENTAL FEATURE. This is an unstable API, please use only for testing purposes.
 * @see @ref evasfiltersref "Evas filters reference"
 */
#define evas_obj_text_filter_program_set(program) EVAS_OBJ_TEXT_ID(EVAS_OBJ_TEXT_SUB_ID_FILTER_PROGRAM_SET), EO_TYPECHECK(const char *, program)

/**
 * @def evas_obj_text_filter_source_set
 *
 * Bind an object to use as a mask or texture with Evas Filters. This will
 * create automatically a new RGBA buffer containing the source object's
 * pixels (as it is rendered).
 *
 * @param[in]  name   Object name as used in the program code
 * @param[in]  obj    Eo object to use through proxy rendering
 *
 * @see evas_obj_text_filter_program_set
 * @since 1.9
 * @note EXPERIMENTAL FEATURE. This is an unstable API, please use only for testing purposes.
 * @see @ref evasfiltersref "Evas filters reference"
 */
#define evas_obj_text_filter_source_set(name, obj) EVAS_OBJ_TEXT_ID(EVAS_OBJ_TEXT_SUB_ID_FILTER_SOURCE_SET), EO_TYPECHECK(const char *, name), EO_TYPECHECK(Eo *, obj)

/**
 * @}
 */

/**
 * @ingroup Evas_Object_Textblock
 *
 * @{
 */

#define EVAS_OBJ_TEXTBLOCK_CLASS evas_object_textblock_class_get()

const Eo_Class *evas_object_textblock_class_get(void) EINA_CONST;

extern EAPI Eo_Op EVAS_OBJ_TEXTBLOCK_BASE_ID;

enum
{
   EVAS_OBJ_TEXTBLOCK_SUB_ID_STYLE_SET,
   EVAS_OBJ_TEXTBLOCK_SUB_ID_STYLE_GET,
   EVAS_OBJ_TEXTBLOCK_SUB_ID_STYLE_USER_PUSH,
   EVAS_OBJ_TEXTBLOCK_SUB_ID_STYLE_USER_PEEK,
   EVAS_OBJ_TEXTBLOCK_SUB_ID_STYLE_USER_POP,
   EVAS_OBJ_TEXTBLOCK_SUB_ID_REPLACE_CHAR_SET,
   EVAS_OBJ_TEXTBLOCK_SUB_ID_LEGACY_NEWLINE_SET,
   EVAS_OBJ_TEXTBLOCK_SUB_ID_LEGACY_NEWLINE_GET,
   EVAS_OBJ_TEXTBLOCK_SUB_ID_VALIGN_SET,
   EVAS_OBJ_TEXTBLOCK_SUB_ID_VALIGN_GET,
   EVAS_OBJ_TEXTBLOCK_SUB_ID_BIDI_DELIMITERS_SET,
   EVAS_OBJ_TEXTBLOCK_SUB_ID_BIDI_DELIMITERS_GET,
   EVAS_OBJ_TEXTBLOCK_SUB_ID_REPLACE_CHAR_GET,
   EVAS_OBJ_TEXTBLOCK_SUB_ID_TEXT_MARKUP_SET,
   EVAS_OBJ_TEXTBLOCK_SUB_ID_TEXT_MARKUP_GET,
   EVAS_OBJ_TEXTBLOCK_SUB_ID_CURSOR_GET,
   EVAS_OBJ_TEXTBLOCK_SUB_ID_CURSOR_NEW,
   EVAS_OBJ_TEXTBLOCK_SUB_ID_NODE_FORMAT_LIST_GET,
   EVAS_OBJ_TEXTBLOCK_SUB_ID_NODE_FORMAT_FIRST_GET,
   EVAS_OBJ_TEXTBLOCK_SUB_ID_NODE_FORMAT_LAST_GET,
   EVAS_OBJ_TEXTBLOCK_SUB_ID_NODE_FORMAT_REMOVE_PAIR,
   EVAS_OBJ_TEXTBLOCK_SUB_ID_LINE_NUMBER_GEOMETRY_GET,
   EVAS_OBJ_TEXTBLOCK_SUB_ID_CLEAR,
   EVAS_OBJ_TEXTBLOCK_SUB_ID_SIZE_FORMATTED_GET,
   EVAS_OBJ_TEXTBLOCK_SUB_ID_SIZE_NATIVE_GET,
   EVAS_OBJ_TEXTBLOCK_SUB_ID_STYLE_INSETS_GET,
   EVAS_OBJ_TEXTBLOCK_SUB_ID_LAST
};

#define EVAS_OBJ_TEXTBLOCK_ID(sub_id) (EVAS_OBJ_TEXTBLOCK_BASE_ID + sub_id)

/**
 * @def evas_obj_textblock_style_set
 * @since 1.8
 *
 * Set the objects style to ts.
 *
 * @param[in] ts
 *
 * @see evas_object_textblock_style_set
 */
#define evas_obj_textblock_style_set(ts) EVAS_OBJ_TEXTBLOCK_ID(EVAS_OBJ_TEXTBLOCK_SUB_ID_STYLE_SET), EO_TYPECHECK(const Evas_Textblock_Style *, ts)

/**
 * @def evas_obj_textblock_style_get
 * @since 1.8
 *
 * Return the style of an object.
 *
 * @param[out] ts
 *
 * @see evas_object_textblock_style_get
 */
#define evas_obj_textblock_style_get(ts) EVAS_OBJ_TEXTBLOCK_ID(EVAS_OBJ_TEXTBLOCK_SUB_ID_STYLE_GET), EO_TYPECHECK(const Evas_Textblock_Style **, ts)

/**
 * @def evas_obj_textblock_style_user_push
 * @since 1.8
 *
 * Push ts to the top of the user style stack.
 *
 * @param[in] ts
 *
 * @see evas_object_textblock_style_user_push
 */
#define evas_obj_textblock_style_user_push(ts) EVAS_OBJ_TEXTBLOCK_ID(EVAS_OBJ_TEXTBLOCK_SUB_ID_STYLE_USER_PUSH), EO_TYPECHECK(Evas_Textblock_Style *, ts)

/**
 * @def evas_obj_textblock_style_user_peek
 * @since 1.8
 *
 * Get (don't remove) the style at the top of the user style stack.
 *
 * @param[out] ts
 *
 * @see evas_object_textblock_style_user_peek
 */
#define evas_obj_textblock_style_user_peek(ts) EVAS_OBJ_TEXTBLOCK_ID(EVAS_OBJ_TEXTBLOCK_SUB_ID_STYLE_USER_PEEK), EO_TYPECHECK(const Evas_Textblock_Style **, ts)

/**
 * @def evas_obj_textblock_style_user_pop
 * @since 1.8
 *
 * Del the from the top of the user style stack.
 *
 *
 * @see evas_object_textblock_style_user_pop
 */
#define evas_obj_textblock_style_user_pop() EVAS_OBJ_TEXTBLOCK_ID(EVAS_OBJ_TEXTBLOCK_SUB_ID_STYLE_USER_POP)

/**
 * @def evas_obj_textblock_replace_char_set
 * @since 1.8
 *
 * Set the "replacement character" to use for the given textblock object.
 *
 * @param[in] ch
 *
 * @see evas_object_textblock_replace_char_set
 */
#define evas_obj_textblock_replace_char_set(ch) EVAS_OBJ_TEXTBLOCK_ID(EVAS_OBJ_TEXTBLOCK_SUB_ID_REPLACE_CHAR_SET), EO_TYPECHECK(const char *, ch)

/**
 * @def evas_obj_textblock_legacy_newline_set
 * @since 1.8
 *
 * Sets newline mode. When true, newline character will behave
 * as a paragraph separator.
 *
 * @param[in] mode
 *
 * @see evas_object_textblock_legacy_newline_set
 */
#define evas_obj_textblock_legacy_newline_set(mode) EVAS_OBJ_TEXTBLOCK_ID(EVAS_OBJ_TEXTBLOCK_SUB_ID_LEGACY_NEWLINE_SET), EO_TYPECHECK(Eina_Bool, mode)

/**
 * @def evas_obj_textblock_legacy_newline_get
 * @since 1.8
 *
 * Gets newline mode. When true, newline character behaves
 * as a paragraph separator.
 *
 * @param[out] newline
 *
 * @see evas_object_textblock_legacy_newline_get
 */
#define evas_obj_textblock_legacy_newline_get(newline) EVAS_OBJ_TEXTBLOCK_ID(EVAS_OBJ_TEXTBLOCK_SUB_ID_LEGACY_NEWLINE_GET), EO_TYPECHECK(Eina_Bool *, newline)

/**
 * @def evas_obj_textblock_valign_set
 * @since 1.8
 *
 * Sets the vertical alignment of text within the textblock object
 *
 * @param[in] align
 *
 * @see evas_object_textblock_valign_set
 */
#define evas_obj_textblock_valign_set(align) EVAS_OBJ_TEXTBLOCK_ID(EVAS_OBJ_TEXTBLOCK_SUB_ID_VALIGN_SET), EO_TYPECHECK(double, align)

/**
 * @def evas_obj_textblock_valign_get
 * @since 1.8
 *
 * Gets the vertical alignment of a textblock
 *
 * @param[out] valign
 *
 * @see evas_object_textblock_valign_get
 */
#define evas_obj_textblock_valign_get(valign) EVAS_OBJ_TEXTBLOCK_ID(EVAS_OBJ_TEXTBLOCK_SUB_ID_VALIGN_GET), EO_TYPECHECK(double *, valign)

/**
 * @def evas_obj_textblock_bidi_delimiters_set
 * @since 1.8
 *
 * Sets the BiDi delimiters used in the textblock.
 *
 * @param[in] delim
 *
 * @see evas_object_textblock_bidi_delimiters_set
 */
#define evas_obj_textblock_bidi_delimiters_set(delim) EVAS_OBJ_TEXTBLOCK_ID(EVAS_OBJ_TEXTBLOCK_SUB_ID_BIDI_DELIMITERS_SET), EO_TYPECHECK(const char *, delim)

/**
 * @def evas_obj_textblock_bidi_delimiters_get
 * @since 1.8
 *
 * Gets the BiDi delimiters used in the textblock.
 *
 * @param[out] delim
 *
 * @see evas_object_textblock_bidi_delimiters_get
 */
#define evas_obj_textblock_bidi_delimiters_get(delim) EVAS_OBJ_TEXTBLOCK_ID(EVAS_OBJ_TEXTBLOCK_SUB_ID_BIDI_DELIMITERS_GET), EO_TYPECHECK(const char **, delim)

/**
 * @def evas_obj_textblock_replace_char_get
 * @since 1.8
 *
 * Get the "replacement character" for given textblock object.
 *
 * @param[out] repch
 *
 * @see evas_object_textblock_replace_char_get
 */
#define evas_obj_textblock_replace_char_get(repch) EVAS_OBJ_TEXTBLOCK_ID(EVAS_OBJ_TEXTBLOCK_SUB_ID_REPLACE_CHAR_GET), EO_TYPECHECK(const char **, repch)

/**
 * @def evas_obj_textblock_text_markup_set
 * @since 1.8
 *
 * Sets the tetxblock's text to the markup text.
 *
 * @param[in] text
 *
 * @see evas_object_textblock_text_markup_set
 */
#define evas_obj_textblock_text_markup_set(text) EVAS_OBJ_TEXTBLOCK_ID(EVAS_OBJ_TEXTBLOCK_SUB_ID_TEXT_MARKUP_SET), EO_TYPECHECK(const char *, text)

/**
 * @def evas_obj_textblock_text_markup_get
 * @since 1.8
 *
 * Return the markup of the object.
 *
 * @param[out] markup
 *
 * @see evas_object_textblock_text_markup_get
 */
#define evas_obj_textblock_text_markup_get(markup) EVAS_OBJ_TEXTBLOCK_ID(EVAS_OBJ_TEXTBLOCK_SUB_ID_TEXT_MARKUP_GET), EO_TYPECHECK(const char **, markup)

/**
 * @def evas_obj_textblock_cursor_get
 * @since 1.8
 *
 * Return the object's main cursor.
 *
 * @param[out] cursor
 *
 * @see evas_object_textblock_cursor_get
 */
#define evas_obj_textblock_cursor_get(cursor) EVAS_OBJ_TEXTBLOCK_ID(EVAS_OBJ_TEXTBLOCK_SUB_ID_CURSOR_GET), EO_TYPECHECK(Evas_Textblock_Cursor **, cursor)

/**
 * @def evas_obj_textblock_cursor_new
 * @since 1.8
 *
 * Create a new cursor, associate it to the obj and init it to point
 * to the start of the textblock. Association to the object means the cursor
 * will be updated when the object will change.
 *
 * @param[in] cur
 *
 * @see evas_object_textblock_cursor_new
 */
#define evas_obj_textblock_cursor_new(cur) EVAS_OBJ_TEXTBLOCK_ID(EVAS_OBJ_TEXTBLOCK_SUB_ID_CURSOR_NEW), EO_TYPECHECK(Evas_Textblock_Cursor **, cur)

/**
 * @def evas_obj_textblock_node_format_list_get
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[in] anchor
 * @param[out] list
 *
 * @see evas_textblock_node_format_list_get
 */
#define evas_obj_textblock_node_format_list_get(anchor, list) EVAS_OBJ_TEXTBLOCK_ID(EVAS_OBJ_TEXTBLOCK_SUB_ID_NODE_FORMAT_LIST_GET), EO_TYPECHECK(const char *, anchor), EO_TYPECHECK(const Eina_List **, list)

/**
 * @def evas_obj_textblock_node_format_first_get
 * @since 1.8
 *
 * Returns the first format node.
 *
 * @param[out] format
 *
 * @see evas_textblock_node_format_first_get
 */
#define evas_obj_textblock_node_format_first_get(format) EVAS_OBJ_TEXTBLOCK_ID(EVAS_OBJ_TEXTBLOCK_SUB_ID_NODE_FORMAT_FIRST_GET), EO_TYPECHECK(const Evas_Object_Textblock_Node_Format **, format)

/**
 * @def evas_obj_textblock_node_format_last_get
 * @since 1.8
 *
 * Returns the last format node.
 *
 * @param[out] format
 *
 * @see evas_textblock_node_format_last_get
 */
#define evas_obj_textblock_node_format_last_get(format) EVAS_OBJ_TEXTBLOCK_ID(EVAS_OBJ_TEXTBLOCK_SUB_ID_NODE_FORMAT_LAST_GET), EO_TYPECHECK(const Evas_Object_Textblock_Node_Format **, format)

/**
 * @def evas_obj_textblock_node_format_remove_pair
 * @since 1.8
 *
 * Remove a format node and its match.
 *
 * @param[in] n
 *
 * @see evas_textblock_node_format_remove_pair
 */
#define evas_obj_textblock_node_format_remove_pair(n) EVAS_OBJ_TEXTBLOCK_ID(EVAS_OBJ_TEXTBLOCK_SUB_ID_NODE_FORMAT_REMOVE_PAIR), EO_TYPECHECK(Evas_Object_Textblock_Node_Format *, n)

/**
 * @def evas_obj_textblock_line_number_geometry_get
 * @since 1.8
 *
 * Get the geometry of a line number.
 *
 * @param[in] line
 * @param[out] cx
 * @param[out] cy
 * @param[out] cw
 * @param[out] ch
 * @param[out] result
 *
 * @see evas_object_textblock_line_number_geometry_get
 */
#define evas_obj_textblock_line_number_geometry_get(line, cx, cy, cw, ch, result) EVAS_OBJ_TEXTBLOCK_ID(EVAS_OBJ_TEXTBLOCK_SUB_ID_LINE_NUMBER_GEOMETRY_GET), EO_TYPECHECK(int, line), EO_TYPECHECK(Evas_Coord *, cx), EO_TYPECHECK(Evas_Coord *, cy), EO_TYPECHECK(Evas_Coord *, cw), EO_TYPECHECK(Evas_Coord *, ch), EO_TYPECHECK(Eina_Bool *, result)

/**
 * @def evas_obj_textblock_clear
 * @since 1.8
 *
 * Clear the textblock object.
 *
 *
 * @see evas_object_textblock_clear
 */
#define evas_obj_textblock_clear() EVAS_OBJ_TEXTBLOCK_ID(EVAS_OBJ_TEXTBLOCK_SUB_ID_CLEAR)

/**
 * @def evas_obj_textblock_size_formatted_get
 * @since 1.8
 *
 * Get the formatted width and height. This calculates the actual size after restricting
 * the textblock to the current size of the object.
 *
 * @param[out] w
 * @param[out] h
 *
 * @see evas_object_textblock_size_formatted_get
 */
#define evas_obj_textblock_size_formatted_get(w, h) EVAS_OBJ_TEXTBLOCK_ID(EVAS_OBJ_TEXTBLOCK_SUB_ID_SIZE_FORMATTED_GET), EO_TYPECHECK(Evas_Coord *, w), EO_TYPECHECK(Evas_Coord *, h)

/**
 * @def evas_obj_textblock_size_native_get
 * @since 1.8
 *
 * Get the native width and height. This calculates the actual size without taking account
 * the current size of the object.
 *
 * @param[out] w
 * @param[out] h
 *
 * @see evas_object_textblock_size_native_get
 */
#define evas_obj_textblock_size_native_get(w, h) EVAS_OBJ_TEXTBLOCK_ID(EVAS_OBJ_TEXTBLOCK_SUB_ID_SIZE_NATIVE_GET), EO_TYPECHECK(Evas_Coord *, w), EO_TYPECHECK(Evas_Coord *, h)

/**
 * @def evas_obj_textblock_style_insets_get
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[out] l
 * @param[out] r
 * @param[out] t
 * @param[out] b
 *
 * @see evas_object_textblock_style_insets_get
 */
#define evas_obj_textblock_style_insets_get(l, r, t, b) EVAS_OBJ_TEXTBLOCK_ID(EVAS_OBJ_TEXTBLOCK_SUB_ID_STYLE_INSETS_GET), EO_TYPECHECK(Evas_Coord *, l), EO_TYPECHECK(Evas_Coord *, r), EO_TYPECHECK(Evas_Coord *, t), EO_TYPECHECK(Evas_Coord *, b)
/**
 * @}
 */

/**
 * @ingroup Evas_Object_Textgrid
 *
 * @{
 */

#define EVAS_OBJ_TEXTGRID_CLASS evas_object_textgrid_class_get()

const Eo_Class *evas_object_textgrid_class_get(void) EINA_CONST;

extern EAPI Eo_Op EVAS_OBJ_TEXTGRID_BASE_ID;

enum
{
   EVAS_OBJ_TEXTGRID_SUB_ID_SIZE_SET,
   EVAS_OBJ_TEXTGRID_SUB_ID_SIZE_GET,
   EVAS_OBJ_TEXTGRID_SUB_ID_FONT_SOURCE_SET,
   EVAS_OBJ_TEXTGRID_SUB_ID_FONT_SOURCE_GET,
   EVAS_OBJ_TEXTGRID_SUB_ID_FONT_SET,
   EVAS_OBJ_TEXTGRID_SUB_ID_FONT_GET,
   EVAS_OBJ_TEXTGRID_SUB_ID_CELL_SIZE_GET,
   EVAS_OBJ_TEXTGRID_SUB_ID_PALETTE_SET,
   EVAS_OBJ_TEXTGRID_SUB_ID_PALETTE_GET,
   EVAS_OBJ_TEXTGRID_SUB_ID_SUPPORTED_FONT_STYLES_SET,
   EVAS_OBJ_TEXTGRID_SUB_ID_SUPPORTED_FONT_STYLES_GET,
   EVAS_OBJ_TEXTGRID_SUB_ID_CELLROW_SET,
   EVAS_OBJ_TEXTGRID_SUB_ID_CELLROW_GET,
   EVAS_OBJ_TEXTGRID_SUB_ID_UPDATE_ADD,
   EVAS_OBJ_TEXTGRID_SUB_ID_LAST
};

#define EVAS_OBJ_TEXTGRID_ID(sub_id) (EVAS_OBJ_TEXTGRID_BASE_ID + sub_id)


/**
 * @def evas_obj_textgrid_size_set
 * @since 1.8
 *
 * Set the size of the textgrid object.
 *
 * @param[in] w
 * @param[in] h
 *
 * @see evas_object_textgrid_size_set
 */
#define evas_obj_textgrid_size_set(w, h) EVAS_OBJ_TEXTGRID_ID(EVAS_OBJ_TEXTGRID_SUB_ID_SIZE_SET), EO_TYPECHECK(int, w), EO_TYPECHECK(int, h)

/**
 * @def evas_obj_textgrid_size_get
 * @since 1.8
 *
 * Get the size of the textgrid object.
 *
 * @param[out] w
 * @param[out] h
 *
 * @see evas_object_textgrid_size_get
 */
#define evas_obj_textgrid_size_get(w, h) EVAS_OBJ_TEXTGRID_ID(EVAS_OBJ_TEXTGRID_SUB_ID_SIZE_GET), EO_TYPECHECK(int *, w), EO_TYPECHECK(int *, h)

/**
 * @def evas_obj_textgrid_font_source_set
 * @since 1.8
 *
 * Set the font (source) file to be used on a given textgrid object.
 *
 * @param[in] font_source
 *
 * @see evas_object_textgrid_font_source_set
 */
#define evas_obj_textgrid_font_source_set(font_source) EVAS_OBJ_TEXTGRID_ID(EVAS_OBJ_TEXTGRID_SUB_ID_FONT_SOURCE_SET), EO_TYPECHECK(const char *, font_source)

/**
 * @def evas_obj_textgrid_font_source_get
 * @since 1.8
 *
 * Get the font file's path which is being used on a given textgrid object.
 *
 * @param[out] ret
 *
 * @see evas_object_textgrid_font_source_get
 */
#define evas_obj_textgrid_font_source_get(ret) EVAS_OBJ_TEXTGRID_ID(EVAS_OBJ_TEXTGRID_SUB_ID_FONT_SOURCE_GET), EO_TYPECHECK(const char **, ret)

/**
 * @def evas_obj_textgrid_font_set
 * @since 1.8
 *
 * Set the font family and size on a given textgrid object.
 *
 * @param[in] font_name
 * @param[in] font_size
 *
 * @see evas_object_textgrid_font_set
 */
#define evas_obj_textgrid_font_set(font_name, font_size) EVAS_OBJ_TEXTGRID_ID(EVAS_OBJ_TEXTGRID_SUB_ID_FONT_SET), EO_TYPECHECK(const char *, font_name), EO_TYPECHECK(Evas_Font_Size, font_size)

/**
 * @def evas_obj_textgrid_font_get
 * @since 1.8
 *
 * Retrieve the font family and size in use on a given textgrid object.
 *
 * @param[out] font_name
 * @param[out] font_size
 *
 * @see evas_object_textgrid_font_get
 */
#define evas_obj_textgrid_font_get(font_name, font_size) EVAS_OBJ_TEXTGRID_ID(EVAS_OBJ_TEXTGRID_SUB_ID_FONT_GET), EO_TYPECHECK(const char **, font_name), EO_TYPECHECK(Evas_Font_Size *, font_size)

/**
 * @def evas_obj_textgrid_cell_size_get
 * @since 1.8
 *
 * Retrieve the size of a cell of the given textgrid object in pixels.
 *
 * @param[out] width
 * @param[out] height
 *
 * @see evas_object_textgrid_cell_size_get
 */
#define evas_obj_textgrid_cell_size_get(width, height) EVAS_OBJ_TEXTGRID_ID(EVAS_OBJ_TEXTGRID_SUB_ID_CELL_SIZE_GET), EO_TYPECHECK(int *, width), EO_TYPECHECK(int *, height)

/**
 * @def evas_obj_textgrid_palette_set
 * @since 1.8
 *
 * The set color to the given palette at the given index of the given textgrid object.
 *
 * @param[in] pal
 * @param[in] idx
 * @param[in] r
 * @param[in] g
 * @param[in] b
 * @param[in] a
 *
 * @see evas_object_textgrid_palette_set
 */
#define evas_obj_textgrid_palette_set(pal, idx, r, g, b, a) EVAS_OBJ_TEXTGRID_ID(EVAS_OBJ_TEXTGRID_SUB_ID_PALETTE_SET), EO_TYPECHECK(Evas_Textgrid_Palette, pal), EO_TYPECHECK(int, idx), EO_TYPECHECK(int, r), EO_TYPECHECK(int, g), EO_TYPECHECK(int, b), EO_TYPECHECK(int, a)

/**
 * @def evas_obj_textgrid_palette_get
 * @since 1.8
 *
 * The retrieve color to the given palette at the given index of the given textgrid object.
 *
 * @param[in] pal
 * @param[in] idx
 * @param[out] r
 * @param[out] g
 * @param[out] b
 * @param[out] a
 *
 * @see evas_object_textgrid_palette_get
 */
#define evas_obj_textgrid_palette_get(pal, idx, r, g, b, a) EVAS_OBJ_TEXTGRID_ID(EVAS_OBJ_TEXTGRID_SUB_ID_PALETTE_GET), EO_TYPECHECK(Evas_Textgrid_Palette, pal), EO_TYPECHECK(int, idx), EO_TYPECHECK(int *, r), EO_TYPECHECK(int *, g), EO_TYPECHECK(int *, b), EO_TYPECHECK(int *, a)

/**
 * @def evas_obj_textgrid_supported_font_styles_set
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[in] styles
 *
 * @see evas_object_textgrid_supported_font_styles_set
 */
#define evas_obj_textgrid_supported_font_styles_set(styles) EVAS_OBJ_TEXTGRID_ID(EVAS_OBJ_TEXTGRID_SUB_ID_SUPPORTED_FONT_STYLES_SET), EO_TYPECHECK(Evas_Textgrid_Font_Style, styles)

/**
 * @def evas_obj_textgrid_supported_font_styles_get
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[out] ret
 *
 * @see evas_object_textgrid_supported_font_styles_get
 */
#define evas_obj_textgrid_supported_font_styles_get(ret) EVAS_OBJ_TEXTGRID_ID(EVAS_OBJ_TEXTGRID_SUB_ID_SUPPORTED_FONT_STYLES_GET), EO_TYPECHECK(Evas_Textgrid_Font_Style *, ret)

/**
 * @def evas_obj_textgrid_cellrow_set
 * @since 1.8
 *
 * Set the string at the given row of the given textgrid object.
 *
 * @param[in] y
 * @param[in] row
 *
 * @see evas_object_textgrid_cellrow_set
 */
#define evas_obj_textgrid_cellrow_set(y, row) EVAS_OBJ_TEXTGRID_ID(EVAS_OBJ_TEXTGRID_SUB_ID_CELLROW_SET), EO_TYPECHECK(int, y), EO_TYPECHECK(const Evas_Textgrid_Cell *, row)

/**
 * @def evas_obj_textgrid_cellrow_get
 * @since 1.8
 *
 * Get the string at the given row of the given textgrid object.
 *
 * @param[in] y
 * @param[out] ret
 *
 * @see evas_object_textgrid_cellrow_get
 */
#define evas_obj_textgrid_cellrow_get(y, ret) EVAS_OBJ_TEXTGRID_ID(EVAS_OBJ_TEXTGRID_SUB_ID_CELLROW_GET), EO_TYPECHECK(int, y), EO_TYPECHECK(Evas_Textgrid_Cell **, ret)

/**
 * @def evas_obj_textgrid_update_add
 * @since 1.8
 *
 * Indicate for evas that part of a textgrid region (cells) has been updated.
 *
 * @param[in] x
 * @param[in] y
 * @param[in] w
 * @param[in] h
 *
 * @see evas_object_textgrid_update_add
 */
#define evas_obj_textgrid_update_add(x, y, w, h) EVAS_OBJ_TEXTGRID_ID(EVAS_OBJ_TEXTGRID_SUB_ID_UPDATE_ADD), EO_TYPECHECK(int, x), EO_TYPECHECK(int, y), EO_TYPECHECK(int, w), EO_TYPECHECK(int, h)

/**
 * @}
 */

/**
 * @ingroup Evas_Object_Line
 *
 * @{
 */
#define EVAS_OBJ_LINE_CLASS evas_object_line_class_get()
const Eo_Class *evas_object_line_class_get(void) EINA_CONST;

extern EAPI Eo_Op EVAS_OBJ_LINE_BASE_ID;

enum
{
   EVAS_OBJ_LINE_SUB_ID_XY_SET,
   EVAS_OBJ_LINE_SUB_ID_XY_GET,
   EVAS_OBJ_LINE_SUB_ID_LAST
};

#define EVAS_OBJ_LINE_ID(sub_id) (EVAS_OBJ_LINE_BASE_ID + sub_id)


/**
 * @def evas_obj_line_xy_set
 * @since 1.8
 *
 * Sets the coordinates of the end points of the given evas line object.
 *
 * @param[in] x1
 * @param[in] y1
 * @param[in] x2
 * @param[in] y2
 *
 * @see evas_object_line_xy_set
 */
#define evas_obj_line_xy_set(x1, y1, x2, y2) EVAS_OBJ_LINE_ID(EVAS_OBJ_LINE_SUB_ID_XY_SET), EO_TYPECHECK(Evas_Coord, x1), EO_TYPECHECK(Evas_Coord, y1), EO_TYPECHECK(Evas_Coord, x2), EO_TYPECHECK(Evas_Coord, y2)

/**
 * @def evas_obj_line_xy_get
 * @since 1.8
 *
 * Retrieves the coordinates of the end points of the given evas line object.
 *
 * @param[out] x1
 * @param[out] y1
 * @param[out] x2
 * @param[out] y2
 *
 * @see evas_object_line_xy_get
 */
#define evas_obj_line_xy_get(x1, y1, x2, y2) EVAS_OBJ_LINE_ID(EVAS_OBJ_LINE_SUB_ID_XY_GET), EO_TYPECHECK(Evas_Coord *, x1), EO_TYPECHECK(Evas_Coord *, y1), EO_TYPECHECK(Evas_Coord *, x2), EO_TYPECHECK(Evas_Coord *, y2)

/**
 * @}
 */

/**
 * @ingroup Evas_Object_Polygon
 *
 * @{
 */
#define EVAS_OBJ_POLYGON_CLASS evas_object_polygon_class_get()
const Eo_Class *evas_object_polygon_class_get(void) EINA_CONST;

extern EAPI Eo_Op EVAS_OBJ_POLYGON_BASE_ID;

enum
{
   EVAS_OBJ_POLYGON_SUB_ID_POINT_ADD,
   EVAS_OBJ_POLYGON_SUB_ID_POINTS_CLEAR,
   EVAS_OBJ_POLYGON_SUB_ID_LAST
};

#define EVAS_OBJ_POLYGON_ID(sub_id) (EVAS_OBJ_POLYGON_BASE_ID + sub_id)


/**
 * @def evas_obj_polygon_point_add
 * @since 1.8
 *
 * Adds the given point to the given evas polygon object.
 *
 * @param[in] x
 * @param[in] y
 *
 * @see evas_object_polygon_point_add
 */
#define evas_obj_polygon_point_add(x, y) EVAS_OBJ_POLYGON_ID(EVAS_OBJ_POLYGON_SUB_ID_POINT_ADD), EO_TYPECHECK(Evas_Coord, x), EO_TYPECHECK(Evas_Coord, y)

/**
 * @def evas_obj_polygon_points_clear
 * @since 1.8
 *
 * Removes all of the points from the given evas polygon object.
 *
 *
 * @see evas_object_polygon_points_clear
 */
#define evas_obj_polygon_points_clear() EVAS_OBJ_POLYGON_ID(EVAS_OBJ_POLYGON_SUB_ID_POINTS_CLEAR)

/**
 * @}
 */

/**
 * @ingroup Evas_Smart_Object_Group
 *
 * @{
 */
#define EVAS_OBJ_SMART_CLASS evas_object_smart_class_get()

const Eo_Class *evas_object_smart_class_get(void) EINA_CONST;

extern EAPI Eo_Op EVAS_OBJ_SMART_BASE_ID;

enum
{
   EVAS_OBJ_SMART_SUB_ID_DATA_SET,
   EVAS_OBJ_SMART_SUB_ID_SMART_GET,
   EVAS_OBJ_SMART_SUB_ID_MEMBER_ADD,
   EVAS_OBJ_SMART_SUB_ID_MEMBER_DEL,
   EVAS_OBJ_SMART_SUB_ID_MEMBERS_GET,
   EVAS_OBJ_SMART_SUB_ID_ITERATOR_NEW,
   EVAS_OBJ_SMART_SUB_ID_CALLBACKS_DESCRIPTIONS_SET,
   EVAS_OBJ_SMART_SUB_ID_CALLBACKS_DESCRIPTIONS_GET,
   EVAS_OBJ_SMART_SUB_ID_CALLBACK_DESCRIPTION_FIND,
   EVAS_OBJ_SMART_SUB_ID_NEED_RECALCULATE_SET,
   EVAS_OBJ_SMART_SUB_ID_NEED_RECALCULATE_GET,
   EVAS_OBJ_SMART_SUB_ID_CALCULATE,
   EVAS_OBJ_SMART_SUB_ID_CHANGED,
   EVAS_OBJ_SMART_SUB_ID_ATTACH,
   // Specific Smart functions that can be overriden by the inherit classes
   EVAS_OBJ_SMART_SUB_ID_ADD,
   EVAS_OBJ_SMART_SUB_ID_DEL,
   EVAS_OBJ_SMART_SUB_ID_RESIZE,
   EVAS_OBJ_SMART_SUB_ID_MOVE,
   EVAS_OBJ_SMART_SUB_ID_SHOW,
   EVAS_OBJ_SMART_SUB_ID_HIDE,
   EVAS_OBJ_SMART_SUB_ID_COLOR_SET,
   EVAS_OBJ_SMART_SUB_ID_CLIP_SET,
   EVAS_OBJ_SMART_SUB_ID_CLIP_UNSET,
   EVAS_OBJ_SMART_SUB_ID_LAST
};

#define EVAS_OBJ_SMART_ID(sub_id) (EVAS_OBJ_SMART_BASE_ID + sub_id)

/**
 * @def evas_obj_smart_data_set
 * @since 1.8
 *
 * Store a pointer to user data for a given smart object.
 *
 * @param[in] data in
 *
 * @see evas_object_smart_data_set
 */
#define evas_obj_smart_data_set(data) EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_DATA_SET), EO_TYPECHECK(void *, data)

/**
 * @def evas_obj_smart_smart_get
 * @since 1.8
 *
 * Get the #Evas_Smart from which smart object was created.
 *
 * @param[out] smart out
 *
 * @see evas_object_smart_smart_get
 */
#define evas_obj_smart_smart_get(smart) EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_SMART_GET), EO_TYPECHECK(Evas_Smart **, smart)

/**
 * @def evas_obj_smart_member_add
 * @since 1.8
 *
 * Set an Evas object as a member of a given smart object.
 *
 * @param[in] obj in
 *
 * @see evas_object_smart_member_add
 */
#define evas_obj_smart_member_add(obj) EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_MEMBER_ADD), EO_TYPECHECK(Evas_Object *, obj)

/**
 * @def evas_obj_smart_member_del
 * @since 1.8
 *
 * Removes a member object from a given smart object.
 *
 * @param[in] obj in
 *
 * @see evas_object_smart_member_del
 */
#define evas_obj_smart_member_del(obj) EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_MEMBER_DEL), EO_TYPECHECK(Evas_Object *, obj)

/**
 * @def evas_obj_smart_members_get
 * @since 1.8
 *
 * Retrieves the list of the member objects of a given Evas smart
 * object
 *
 * @param[out] list out
 *
 * @see evas_object_smart_members_get
 * @see evas_object_smart_iterator_new
 */
#define evas_obj_smart_members_get(list) EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_MEMBERS_GET), EO_TYPECHECK(Eina_List **, list)

/**
 * @def evas_obj_smart_iterator_new
 * @since 1.8
 *
 * Retrieves an iterator of the member objects of a given Evas smart
 * object
 *
 * @param[out] it out
 *
 * @see evas_object_smart_iterator_new
 * @see evas_object_smart_members_get
 */
#define evas_obj_smart_iterator_new(it) EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_ITERATOR_NEW), EO_TYPECHECK(Eina_Iterator **, it)

/**
 * @def evas_obj_smart_callback_priority_add
 * @since 1.8
 *
 * Add (register) a callback function to the smart event specified by
 * @p event on the smart object. Except for the priority field,
 * it's exactly the same as @ref evas_object_smart_callback_add
 *
 * @param[in] event in
 * @param[in] priority in
 * @param[in] func in
 * @param[in] data in
 *
 * @see evas_object_smart_callback_priority_add
 */
#define evas_obj_smart_callback_priority_add(event, priority, func, data) EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_CALLBACK_PRIORITY_ADD), EO_TYPECHECK(const char *, event), EO_TYPECHECK(Evas_Callback_Priority, priority), EO_TYPECHECK(Evas_Smart_Cb, func), EO_TYPECHECK(const void *, data)

/**
 * @def evas_obj_smart_callback_del
 * @since 1.8
 *
 * Delete (unregister) a callback function from the smart event
 * specified by @p event on the smart object.
 *
 * @param[in] event in
 * @param[in] func in
 * @param[out] ret_data out
 *
 * @see evas_object_smart_callback_del
 */
#define evas_obj_smart_callback_del(event, func, ret_data) EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_CALLBACK_DEL), EO_TYPECHECK(const char *, event), EO_TYPECHECK(Evas_Smart_Cb, func), EO_TYPECHECK(void **, ret_data)

/**
 * @def evas_obj_smart_callback_del_full
 * @since 1.8
 *
 * Delete (unregister) a callback function from the smart event
 * specified by @p event on the smart object.
 *
 * @param[in] event in
 * @param[in] func in
 * @param[in] data in
 * @param[out] ret_data out
 *
 * @see evas_object_smart_callback_del_full
 */
#define evas_obj_smart_callback_del_full(event, func, data, ret_data) EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_CALLBACK_DEL_FULL), EO_TYPECHECK(const char *, event), EO_TYPECHECK(Evas_Smart_Cb, func), EO_TYPECHECK(const void *, data), EO_TYPECHECK(void **, ret_data)

/**
 * @def evas_obj_smart_callback_call
 * @since 1.8
 *
 * Call a given smart callback on the smart object.
 *
 * @param[in] event
 * @param[in] event_info
 *
 * @see evas_object_smart_callback_call
 */
#define evas_obj_smart_callback_call(event, event_info) EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_CALLBACK_CALL), EO_TYPECHECK(const char *, event), EO_TYPECHECK(void *, event_info)

/**
 * @def evas_obj_smart_callbacks_descriptions_set
 * @since 1.8
 *
 * Set an smart object @b instance's smart callbacks descriptions.
 *
 * @param[in] descriptions in
 * @param[out] result out
 *
 * @see evas_object_smart_callbacks_descriptions_set
 */
#define evas_obj_smart_callbacks_descriptions_set(descriptions, result) EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_CALLBACKS_DESCRIPTIONS_SET), EO_TYPECHECK(const Evas_Smart_Cb_Description *, descriptions), EO_TYPECHECK(Eina_Bool *, result)

/**
 * @def evas_obj_smart_callbacks_descriptions_get
 * @since 1.8
 *
 * Retrieve an smart object's know smart callback descriptions (both
 * instance and class ones).
 *
 * @param[out] class_descriptions out
 * @param[out] class_count out
 * @param[out] instance_descriptions out
 * @param[out] instance_count out
 *
 * @see evas_object_smart_callbacks_descriptions_get
 */
#define evas_obj_smart_callbacks_descriptions_get(class_descriptions, class_count, instance_descriptions, instance_count) EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_CALLBACKS_DESCRIPTIONS_GET), EO_TYPECHECK(const Evas_Smart_Cb_Description ***, class_descriptions), EO_TYPECHECK(unsigned int *, class_count), EO_TYPECHECK(const Evas_Smart_Cb_Description ***, instance_descriptions), EO_TYPECHECK(unsigned int *, instance_count)

/**
 * @def evas_obj_smart_callback_description_find
 * @since 1.8
 *
 * Find callback description for callback called @a name.
 *
 * @param[in] name in
 * @param[out] class_description out
 * @param[out] instance_description out
 *
 * @see evas_object_smart_callback_description_find
 */
#define evas_obj_smart_callback_description_find(name, class_description, instance_description) EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_CALLBACK_DESCRIPTION_FIND), EO_TYPECHECK(const char *, name), EO_TYPECHECK(const Evas_Smart_Cb_Description **, class_description), EO_TYPECHECK(const Evas_Smart_Cb_Description **, instance_description)

/**
 * @def evas_obj_smart_need_recalculate_set
 * @since 1.8
 *
 * Set or unset the flag signalling that a given smart object needs to
 * get recalculated.
 *
 * @param[in] value in
 *
 * @see evas_object_smart_need_recalculate_set
 */
#define evas_obj_smart_need_recalculate_set(value) EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_NEED_RECALCULATE_SET), EO_TYPECHECK(Eina_Bool, value)

/**
 * @def evas_obj_smart_need_recalculate_get
 * @since 1.8
 *
 * Get the value of the flag signalling that a given smart object needs to
 * get recalculated.
 *
 * @param[out] need_recalculate out
 *
 * @see evas_object_smart_need_recalculate_get
 */
#define evas_obj_smart_need_recalculate_get(need_recalculate) EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_NEED_RECALCULATE_GET), EO_TYPECHECK(Eina_Bool *, need_recalculate)

/**
 * @def evas_obj_smart_calculate
 * @since 1.8
 *
 * Call the @b calculate() smart function immediately on a given smart
 * object.
 *
 * @see evas_object_smart_calculate
 */
#define evas_obj_smart_calculate() EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_CALCULATE)

/**
 * @def evas_obj_smart_changed
 * @since 1.8
 *
 * Mark smart object as changed, dirty.
 *
 * @see evas_object_smart_changed
 */
#define evas_obj_smart_changed() EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_CHANGED)


/**
 * @def evas_obj_smart_add
 * @since 1.8
 *
 * Instantiates a new smart object described by s.
 *
 *
 * @see evas_object_smart_add
 */
#define evas_obj_smart_add() EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_ADD)

/**
 * @def evas_obj_smart_del
 * @since 1.8
 *
 * Deletes a smart object.
 *
 */
#define evas_obj_smart_del() EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_DEL)

/**
 * @def evas_obj_smart_resize
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[in] w
 * @param[in] h
 */
#define evas_obj_smart_resize(w, h) EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_RESIZE), EO_TYPECHECK(Evas_Coord, w), EO_TYPECHECK(Evas_Coord, h)

/**
 * @def evas_obj_smart_move
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[in] x
 * @param[in] y
 */
#define evas_obj_smart_move(x, y) EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_MOVE), EO_TYPECHECK(Evas_Coord, x), EO_TYPECHECK(Evas_Coord, y)

/**
 * @def evas_obj_smart_show
 * @since 1.8
 *
 * No description supplied by the EAPI.
 */
#define evas_obj_smart_show() EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_SHOW)

/**
 * @def evas_obj_smart_hide
 * @since 1.8
 *
 * No description supplied by the EAPI.
 */
#define evas_obj_smart_hide() EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_HIDE)

/**
 * @def evas_obj_smart_color_set
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[in] r
 * @param[in] g
 * @param[in] b
 * @param[in] a
 *
 * @see evas_object_smart_color_set
 */
#define evas_obj_smart_color_set(r, g, b, a) EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_COLOR_SET), EO_TYPECHECK(int, r), EO_TYPECHECK(int, g), EO_TYPECHECK(int, b), EO_TYPECHECK(int, a)

/**
 * @def evas_obj_smart_clip_set
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[in] clip
 */
#define evas_obj_smart_clip_set(clip) EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_CLIP_SET), EO_TYPECHECK(Evas_Object *, clip)

/**
 * @def evas_obj_smart_clip_unset
 * @since 1.8
 *
 * No description supplied by the EAPI.
 */
#define evas_obj_smart_clip_unset() EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_CLIP_UNSET)

/**
 * @}
 */

/**
 * @ingroup Evas_Smart_Object_Clipped
 *
 * @{
 */

#define EVAS_OBJ_SMART_CLIPPED_CLASS evas_object_smart_clipped_eo_class_get()

const Eo_Class *evas_object_smart_clipped_eo_class_get(void) EINA_CONST;

extern EAPI Eo_Op EVAS_OBJ_SMART_CLIPPED_BASE_ID;

enum
{
   EVAS_OBJ_SMART_CLIPPED_SUB_ID_LAST
};

/**
 * @}
 */

/**
 * @ingroup Evas_Object_Box
 *
 * @{
 */
#define EVAS_OBJ_BOX_CLASS evas_object_box_class_get()

const Eo_Class *evas_object_box_class_get(void) EINA_CONST;

extern EAPI Eo_Op EVAS_OBJ_BOX_BASE_ID;

enum
{
   EVAS_OBJ_BOX_SUB_ID_INTERNAL_APPEND,
   EVAS_OBJ_BOX_SUB_ID_INTERNAL_PREPEND,
   EVAS_OBJ_BOX_SUB_ID_INTERNAL_INSERT_BEFORE,
   EVAS_OBJ_BOX_SUB_ID_INTERNAL_INSERT_AFTER,
   EVAS_OBJ_BOX_SUB_ID_INTERNAL_INSERT_AT,
   EVAS_OBJ_BOX_SUB_ID_INTERNAL_REMOVE,
   EVAS_OBJ_BOX_SUB_ID_INTERNAL_REMOVE_AT,
   EVAS_OBJ_BOX_SUB_ID_INTERNAL_OPTION_NEW,
   EVAS_OBJ_BOX_SUB_ID_INTERNAL_OPTION_FREE,

   EVAS_OBJ_BOX_SUB_ID_ADD_TO,
   EVAS_OBJ_BOX_SUB_ID_LAYOUT_SET,
   EVAS_OBJ_BOX_SUB_ID_LAYOUT_HORIZONTAL,
   EVAS_OBJ_BOX_SUB_ID_LAYOUT_VERTICAL,
   EVAS_OBJ_BOX_SUB_ID_LAYOUT_HOMOGENEOUS_HORIZONTAL,
   EVAS_OBJ_BOX_SUB_ID_LAYOUT_HOMOGENEOUS_VERTICAL,
   EVAS_OBJ_BOX_SUB_ID_LAYOUT_HOMOGENEOUS_MAX_SIZE_HORIZONTAL,
   EVAS_OBJ_BOX_SUB_ID_LAYOUT_HOMOGENEOUS_MAX_SIZE_VERTICAL,
   EVAS_OBJ_BOX_SUB_ID_LAYOUT_FLOW_HORIZONTAL,
   EVAS_OBJ_BOX_SUB_ID_LAYOUT_FLOW_VERTICAL,
   EVAS_OBJ_BOX_SUB_ID_LAYOUT_STACK,
   EVAS_OBJ_BOX_SUB_ID_ALIGN_SET,
   EVAS_OBJ_BOX_SUB_ID_ALIGN_GET,
   EVAS_OBJ_BOX_SUB_ID_PADDING_SET,
   EVAS_OBJ_BOX_SUB_ID_PADDING_GET,
   EVAS_OBJ_BOX_SUB_ID_APPEND,
   EVAS_OBJ_BOX_SUB_ID_PREPEND,
   EVAS_OBJ_BOX_SUB_ID_INSERT_BEFORE,
   EVAS_OBJ_BOX_SUB_ID_INSERT_AFTER,
   EVAS_OBJ_BOX_SUB_ID_INSERT_AT,
   EVAS_OBJ_BOX_SUB_ID_REMOVE,
   EVAS_OBJ_BOX_SUB_ID_REMOVE_AT,
   EVAS_OBJ_BOX_SUB_ID_REMOVE_ALL,
   EVAS_OBJ_BOX_SUB_ID_ITERATOR_NEW,
   EVAS_OBJ_BOX_SUB_ID_ACCESSOR_NEW,
   EVAS_OBJ_BOX_SUB_ID_OPTION_PROPERTY_NAME_GET,
   EVAS_OBJ_BOX_SUB_ID_OPTION_PROPERTY_ID_GET,
   EVAS_OBJ_BOX_SUB_ID_OPTION_PROPERTY_VSET,
   EVAS_OBJ_BOX_SUB_ID_OPTION_PROPERTY_VGET,
   EVAS_OBJ_BOX_SUB_ID_LAST
};

#define EVAS_OBJ_BOX_ID(sub_id) (EVAS_OBJ_BOX_BASE_ID + sub_id)


/**
 * @def evas_obj_box_internal_append
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[in] child
 * @param[out] option
 *
 * @see evas_object_box_internal_append
 */
#define evas_obj_box_internal_append(child, option) EVAS_OBJ_BOX_ID(EVAS_OBJ_BOX_SUB_ID_INTERNAL_APPEND), EO_TYPECHECK(Evas_Object *, child), EO_TYPECHECK(Evas_Object_Box_Option **, option)

/**
 * @def evas_obj_box_internal_prepend
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[in] child
 * @param[out] option
 *
 * @see evas_object_box_internal_prepend
 */
#define evas_obj_box_internal_prepend(child, option) EVAS_OBJ_BOX_ID(EVAS_OBJ_BOX_SUB_ID_INTERNAL_PREPEND), EO_TYPECHECK(Evas_Object *, child), EO_TYPECHECK(Evas_Object_Box_Option **, option)

/**
 * @def evas_obj_box_internal_insert_before
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[in] child
 * @param[in] reference
 * @param[out] option
 *
 * @see evas_object_box_internal_insert_before
 */
#define evas_obj_box_internal_insert_before(child, reference, option) EVAS_OBJ_BOX_ID(EVAS_OBJ_BOX_SUB_ID_INTERNAL_INSERT_BEFORE), EO_TYPECHECK(Evas_Object *, child), EO_TYPECHECK(const Evas_Object *, reference), EO_TYPECHECK(Evas_Object_Box_Option **, option)

/**
 * @def evas_obj_box_internal_insert_after
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[in] child
 * @param[in] reference
 * @param[out] option
 *
 * @see evas_object_box_internal_insert_after
 */
#define evas_obj_box_internal_insert_after(child, reference, option) EVAS_OBJ_BOX_ID(EVAS_OBJ_BOX_SUB_ID_INTERNAL_INSERT_AFTER), EO_TYPECHECK(Evas_Object *, child), EO_TYPECHECK(const Evas_Object *, reference), EO_TYPECHECK(Evas_Object_Box_Option **, option)

/**
 * @def evas_obj_box_internal_insert_at
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[in] child
 * @param[in] pos
 * @param[out] option
 *
 * @see evas_object_box_internal_insert_at
 */
#define evas_obj_box_internal_insert_at(child, pos, option) EVAS_OBJ_BOX_ID(EVAS_OBJ_BOX_SUB_ID_INTERNAL_INSERT_AT), EO_TYPECHECK(Evas_Object *, child), EO_TYPECHECK(unsigned int, pos), EO_TYPECHECK(Evas_Object_Box_Option **, option)

/**
 * @def evas_obj_box_internal_remove
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[in] child
 * @param[out] result
 *
 * @see evas_object_box_internal_remove
 */
#define evas_obj_box_internal_remove(child, result) EVAS_OBJ_BOX_ID(EVAS_OBJ_BOX_SUB_ID_INTERNAL_REMOVE), EO_TYPECHECK(Evas_Object *, child), EO_TYPECHECK(Evas_Object **, result)

/**
 * @def evas_obj_box_internal_remove_at
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[in] pos
 * @param[out] result
 *
 * @see evas_object_box_internal_remove_at
 */
#define evas_obj_box_internal_remove_at(pos, result) EVAS_OBJ_BOX_ID(EVAS_OBJ_BOX_SUB_ID_INTERNAL_REMOVE_AT), EO_TYPECHECK(unsigned int, pos), EO_TYPECHECK(Evas_Object **, result)

/**
 * @def evas_obj_box_internal_option_new
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[in] child
 * @param[out] ret
 *
 * @see evas_object_box_internal_option_new
 */
#define evas_obj_box_internal_option_new(child, ret) EVAS_OBJ_BOX_ID(EVAS_OBJ_BOX_SUB_ID_INTERNAL_OPTION_NEW), EO_TYPECHECK(Evas_Object *, child), EO_TYPECHECK(Evas_Object_Box_Option **, ret)

/**
 * @def evas_obj_box_internal_option_free
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[in] opt
 *
 * @see evas_object_box_internal_option_free
 */
#define evas_obj_box_internal_option_free(opt) EVAS_OBJ_BOX_ID(EVAS_OBJ_BOX_SUB_ID_INTERNAL_OPTION_FREE), EO_TYPECHECK(Evas_Object_Box_Option *, opt)


/**
 * @def evas_obj_box_add_to
 * @since 1.8
 *
 * Add a new box as a child of a given smart object.
 *
 * @param[in] o
 *
 * @see evas_object_box_add_to
 */
#define evas_obj_box_add_to(o) EVAS_OBJ_BOX_ID(EVAS_OBJ_BOX_SUB_ID_ADD_TO), EO_TYPECHECK(Evas_Object **, o)

/**
 * @def evas_obj_box_layout_set
 * @since 1.8
 *
 * Set a new layouting function to a given box object
 *
 * @param[in] cb
 * @param[in] data
 * @param[in] free_data
 *
 * @see evas_object_box_layout_set
 */
#define evas_obj_box_layout_set(cb, data, free_data) EVAS_OBJ_BOX_ID(EVAS_OBJ_BOX_SUB_ID_LAYOUT_SET), EO_TYPECHECK(Evas_Object_Box_Layout, cb), EO_TYPECHECK(const void *, data), EO_TYPECHECK(Eina_Free_Cb, free_data)

/**
 * @def evas_obj_box_layout_horizontal
 * @since 1.8
 *
 * Layout function which sets the box o to a (basic) horizontal box
 *
 *
 * @see evas_object_box_layout_horizontal
 */
#define evas_obj_box_layout_horizontal() EVAS_OBJ_BOX_ID(EVAS_OBJ_BOX_SUB_ID_LAYOUT_HORIZONTAL)

/**
 * @def evas_obj_box_layout_vertical
 * @since 1.8
 *
 * Layout function which sets the box o to a (basic) vertical box
 *
 *
 * @see evas_object_box_layout_vertical
 */
#define evas_obj_box_layout_vertical() EVAS_OBJ_BOX_ID(EVAS_OBJ_BOX_SUB_ID_LAYOUT_VERTICAL)

/**
 * @def evas_obj_box_layout_homogeneous_horizontal
 * @since 1.8
 *
 * Layout function which sets the box o to a @b homogeneous
 *
 *
 * @see evas_object_box_layout_homogeneous_horizontal
 */
#define evas_obj_box_layout_homogeneous_horizontal() EVAS_OBJ_BOX_ID(EVAS_OBJ_BOX_SUB_ID_LAYOUT_HOMOGENEOUS_HORIZONTAL)

/**
 * @def evas_obj_box_layout_homogeneous_vertical
 * @since 1.8
 *
 * Layout function which sets the box o to a @b homogeneous
 *
 *
 * @see evas_object_box_layout_homogeneous_vertical
 */
#define evas_obj_box_layout_homogeneous_vertical() EVAS_OBJ_BOX_ID(EVAS_OBJ_BOX_SUB_ID_LAYOUT_HOMOGENEOUS_VERTICAL)

/**
 * @def evas_obj_box_layout_homogeneous_max_size_horizontal
 * @since 1.8
 *
 * Layout function which sets the box o to a maximum size.
 *
 *
 * @see evas_object_box_layout_homogeneous_max_size_horizontal
 */
#define evas_obj_box_layout_homogeneous_max_size_horizontal() EVAS_OBJ_BOX_ID(EVAS_OBJ_BOX_SUB_ID_LAYOUT_HOMOGENEOUS_MAX_SIZE_HORIZONTAL)

/**
 * @def evas_obj_box_layout_homogeneous_max_size_vertical
 * @since 1.8
 *
 * Layout function which sets the box o to a <b>maximum size,
 *
 *
 * @see evas_object_box_layout_homogeneous_max_size_vertical
 */
#define evas_obj_box_layout_homogeneous_max_size_vertical() EVAS_OBJ_BOX_ID(EVAS_OBJ_BOX_SUB_ID_LAYOUT_HOMOGENEOUS_MAX_SIZE_VERTICAL)

/**
 * @def evas_obj_box_layout_flow_horizontal
 * @since 1.8
 *
 * Layout function which sets the box o to a @b flow horizontal
 *
 *
 * @see evas_object_box_layout_flow_horizontal
 */
#define evas_obj_box_layout_flow_horizontal() EVAS_OBJ_BOX_ID(EVAS_OBJ_BOX_SUB_ID_LAYOUT_FLOW_HORIZONTAL)

/**
 * @def evas_obj_box_layout_flow_vertical
 * @since 1.8
 *
 * Layout function which sets the box o to a @b flow vertical box.
 *
 *
 * @see evas_object_box_layout_flow_vertical
 */
#define evas_obj_box_layout_flow_vertical() EVAS_OBJ_BOX_ID(EVAS_OBJ_BOX_SUB_ID_LAYOUT_FLOW_VERTICAL)

/**
 * @def evas_obj_box_layout_stack
 * @since 1.8
 *
 * Layout function which sets the box o to a @b stacking box
 *
 *
 * @see evas_object_box_layout_stack
 */
#define evas_obj_box_layout_stack() EVAS_OBJ_BOX_ID(EVAS_OBJ_BOX_SUB_ID_LAYOUT_STACK)

/**
 * @def evas_obj_box_align_set
 * @since 1.8
 *
 * Set the alignment of the whole bounding box of contents, for a
 *
 * @param[in] horizontal
 * @param[in] vertical
 *
 * @see evas_object_box_align_set
 */
#define evas_obj_box_align_set(horizontal, vertical) EVAS_OBJ_BOX_ID(EVAS_OBJ_BOX_SUB_ID_ALIGN_SET), EO_TYPECHECK(double, horizontal), EO_TYPECHECK(double, vertical)

/**
 * @def evas_obj_box_align_get
 * @since 1.8
 *
 * Get the alignment of the whole bounding box of contents, for a
 *
 * @param[out] horizontal
 * @param[out] vertical
 *
 * @see evas_object_box_align_get
 */
#define evas_obj_box_align_get(horizontal, vertical) EVAS_OBJ_BOX_ID(EVAS_OBJ_BOX_SUB_ID_ALIGN_GET), EO_TYPECHECK(double *, horizontal), EO_TYPECHECK(double *, vertical)

/**
 * @def evas_obj_box_padding_set
 * @since 1.8
 *
 * Set the (space) padding between cells set for a given box object.
 *
 * @param[in] horizontal
 * @param[in] vertical
 *
 * @see evas_object_box_padding_set
 */
#define evas_obj_box_padding_set(horizontal, vertical) EVAS_OBJ_BOX_ID(EVAS_OBJ_BOX_SUB_ID_PADDING_SET), EO_TYPECHECK(Evas_Coord, horizontal), EO_TYPECHECK(Evas_Coord, vertical)

/**
 * @def evas_obj_box_padding_get
 * @since 1.8
 *
 * Get the (space) padding between cells set for a given box object.
 *
 * @param[out] horizontal
 * @param[out] vertical
 *
 * @see evas_object_box_padding_get
 */
#define evas_obj_box_padding_get(horizontal, vertical) EVAS_OBJ_BOX_ID(EVAS_OBJ_BOX_SUB_ID_PADDING_GET), EO_TYPECHECK(Evas_Coord *, horizontal), EO_TYPECHECK(Evas_Coord *, vertical)

/**
 * @def evas_obj_box_append
 * @since 1.8
 *
 * Append a new child object to the given box object @a o.
 *
 * @param[in] child
 * @param[in] option
 *
 * @see evas_object_box_append
 */
#define evas_obj_box_append(child, option) EVAS_OBJ_BOX_ID(EVAS_OBJ_BOX_SUB_ID_APPEND), EO_TYPECHECK(Evas_Object *, child), EO_TYPECHECK(Evas_Object_Box_Option **, option)

/**
 * @def evas_obj_box_prepend
 * @since 1.8
 *
 * Prepend a new child object to the given box object @a o.
 *
 * @param[in] child
 * @param[in] option
 *
 * @see evas_object_box_prepend
 */
#define evas_obj_box_prepend(child, option) EVAS_OBJ_BOX_ID(EVAS_OBJ_BOX_SUB_ID_PREPEND), EO_TYPECHECK(Evas_Object *, child), EO_TYPECHECK(Evas_Object_Box_Option **, option)

/**
 * @def evas_obj_box_insert_before
 * @since 1.8
 *
 * Insert a new child object <b>before another existing one</b>, in
 *
 * @param[in] child
 * @param[in] reference
 * @param[in] option
 *
 * @see evas_object_box_insert_before
 */
#define evas_obj_box_insert_before(child, reference, option) EVAS_OBJ_BOX_ID(EVAS_OBJ_BOX_SUB_ID_INSERT_BEFORE), EO_TYPECHECK(Evas_Object *, child), EO_TYPECHECK(const Evas_Object *, reference), EO_TYPECHECK(Evas_Object_Box_Option **, option)

/**
 * @def evas_obj_box_insert_after
 * @since 1.8
 *
 * Insert a new child object <b>after another existing one</b>, in
 *
 * @param[in] child
 * @param[in] reference
 * @param[in] option
 *
 * @see evas_object_box_insert_after
 */
#define evas_obj_box_insert_after(child, reference, option) EVAS_OBJ_BOX_ID(EVAS_OBJ_BOX_SUB_ID_INSERT_AFTER), EO_TYPECHECK(Evas_Object *, child), EO_TYPECHECK(const Evas_Object *, reference), EO_TYPECHECK(Evas_Object_Box_Option **, option)

/**
 * @def evas_obj_box_insert_at
 * @since 1.8
 *
 * Insert a new child object <b>at a given position</b>, in a given
 *
 * @param[in] child
 * @param[in] pos
 * @param[in] option
 *
 * @see evas_object_box_insert_at
 */
#define evas_obj_box_insert_at(child, pos, option) EVAS_OBJ_BOX_ID(EVAS_OBJ_BOX_SUB_ID_INSERT_AT), EO_TYPECHECK(Evas_Object *, child), EO_TYPECHECK(unsigned int, pos), EO_TYPECHECK(Evas_Object_Box_Option **, option)

/**
 * @def evas_obj_box_remove
 * @since 1.8
 *
 * Remove a given object from a box object, unparenting it again.
 *
 * @param[in] child
 * @param[out] result
 *
 * @see evas_object_box_remove
 */
#define evas_obj_box_remove(child, result) EVAS_OBJ_BOX_ID(EVAS_OBJ_BOX_SUB_ID_REMOVE), EO_TYPECHECK(Evas_Object *, child), EO_TYPECHECK(Eina_Bool *, result)

/**
 * @def evas_obj_box_remove_at
 * @since 1.8
 *
 * Remove an object, <b>bound to a given position</b> in a box object,
 *
 * @param[in] pos
 * @param[out] result
 *
 * @see evas_object_box_remove_at
 */
#define evas_obj_box_remove_at(pos, result) EVAS_OBJ_BOX_ID(EVAS_OBJ_BOX_SUB_ID_REMOVE_AT), EO_TYPECHECK(unsigned int, pos), EO_TYPECHECK(Eina_Bool *, result)

/**
 * @def evas_obj_box_remove_all
 * @since 1.8
 *
 * Remove all child objects from a box object, unparenting them
 *
 * @param[in] clear
 * @param[out] result
 *
 * @see evas_object_box_remove_all
 */
#define evas_obj_box_remove_all(clear, result) EVAS_OBJ_BOX_ID(EVAS_OBJ_BOX_SUB_ID_REMOVE_ALL), EO_TYPECHECK(Eina_Bool, clear), EO_TYPECHECK(Eina_Bool *, result)

/**
 * @def evas_obj_box_iterator_new
 * @since 1.8
 *
 * Get an iterator to walk the list of children of a given box object.
 *
 * @param[in] itr
 *
 * @see evas_object_box_iterator_new
 */
#define evas_obj_box_iterator_new(itr) EVAS_OBJ_BOX_ID(EVAS_OBJ_BOX_SUB_ID_ITERATOR_NEW), EO_TYPECHECK(Eina_Iterator **, itr)

/**
 * @def evas_obj_box_accessor_new
 * @since 1.8
 *
 * Get an accessor (a structure providing random items access) to the
 *
 * @param[in] accessor
 *
 * @see evas_object_box_accessor_new
 */
#define evas_obj_box_accessor_new(accessor) EVAS_OBJ_BOX_ID(EVAS_OBJ_BOX_SUB_ID_ACCESSOR_NEW), EO_TYPECHECK(Eina_Accessor **, accessor)

/**
 * @def evas_obj_box_option_property_name_get
 * @since 1.8
 *
 * Get the name of the property of the child elements of the box o
 *
 * @param[in] property
 * @param[in] name
 *
 * @see evas_object_box_option_property_name_get
 */
#define evas_obj_box_option_property_name_get(property, name) EVAS_OBJ_BOX_ID(EVAS_OBJ_BOX_SUB_ID_OPTION_PROPERTY_NAME_GET), EO_TYPECHECK(int, property), EO_TYPECHECK(const char **, name)

/**
 * @def evas_obj_box_option_property_id_get
 * @since 1.8
 *
 * Get the numerical identifier of the property of the child elements
 *
 * @param[in] name
 * @param[in] id
 *
 * @see evas_object_box_option_property_id_get
 */
#define evas_obj_box_option_property_id_get(name, id) EVAS_OBJ_BOX_ID(EVAS_OBJ_BOX_SUB_ID_OPTION_PROPERTY_ID_GET), EO_TYPECHECK(const char *, name), EO_TYPECHECK(int *, id)

/**
 * @def evas_obj_box_option_property_vset
 * @since 1.8
 *
 * Set a property value (by its given numerical identifier), on a
 *
 * @param[in] opt
 * @param[in] property
 * @param[in] args
 * @param[out] ret
 *
 * @see evas_object_box_option_property_vset
 */
#define evas_obj_box_option_property_vset(opt, property, args, ret) EVAS_OBJ_BOX_ID(EVAS_OBJ_BOX_SUB_ID_OPTION_PROPERTY_VSET), EO_TYPECHECK(Evas_Object_Box_Option *, opt), EO_TYPECHECK(int, property), EO_TYPECHECK(va_list *, args), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def evas_obj_box_option_property_vget
 * @since 1.8
 *
 * Get a property's value (by its given numerical identifier), on a
 *
 * @param[in] opt
 * @param[in] property
 * @param[in] args
 * @param[out] ret
 *
 * @see evas_object_box_option_property_vget
 */
#define evas_obj_box_option_property_vget(opt, property, args, ret) EVAS_OBJ_BOX_ID(EVAS_OBJ_BOX_SUB_ID_OPTION_PROPERTY_VGET), EO_TYPECHECK(Evas_Object_Box_Option *, opt), EO_TYPECHECK(int, property), EO_TYPECHECK(va_list *, args), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @}
 */

/**
 * @ingroup Evas_Object_Table
 *
 * @{
 */
#define EVAS_OBJ_TABLE_CLASS evas_object_table_class_get()

const Eo_Class *evas_object_table_class_get(void) EINA_CONST;

extern EAPI Eo_Op EVAS_OBJ_TABLE_BASE_ID;

enum
{
   EVAS_OBJ_TABLE_SUB_ID_ADD_TO,
   EVAS_OBJ_TABLE_SUB_ID_HOMOGENEOUS_SET,
   EVAS_OBJ_TABLE_SUB_ID_HOMOGENEOUS_GET,
   EVAS_OBJ_TABLE_SUB_ID_ALIGN_SET,
   EVAS_OBJ_TABLE_SUB_ID_ALIGN_GET,
   EVAS_OBJ_TABLE_SUB_ID_PADDING_SET,
   EVAS_OBJ_TABLE_SUB_ID_PADDING_GET,
   EVAS_OBJ_TABLE_SUB_ID_PACK_GET,
   EVAS_OBJ_TABLE_SUB_ID_PACK,
   EVAS_OBJ_TABLE_SUB_ID_UNPACK,
   EVAS_OBJ_TABLE_SUB_ID_CLEAR,
   EVAS_OBJ_TABLE_SUB_ID_COL_ROW_SIZE_GET,
   EVAS_OBJ_TABLE_SUB_ID_ITERATOR_NEW,
   EVAS_OBJ_TABLE_SUB_ID_ACCESSOR_NEW,
   EVAS_OBJ_TABLE_SUB_ID_CHILDREN_GET,
   EVAS_OBJ_TABLE_SUB_ID_MIRRORED_GET,
   EVAS_OBJ_TABLE_SUB_ID_MIRRORED_SET,
   EVAS_OBJ_TABLE_SUB_ID_LAST
};

#define EVAS_OBJ_TABLE_ID(sub_id) (EVAS_OBJ_TABLE_BASE_ID + sub_id)


/**
 * @def evas_obj_table_add_to
 * @since 1.8
 *
 * Create a table that is child of a given element parent.
 *
 * @param[out] ret
 *
 * @see evas_object_table_add_to
 */
#define evas_obj_table_add_to(ret) EVAS_OBJ_TABLE_ID(EVAS_OBJ_TABLE_SUB_ID_ADD_TO), EO_TYPECHECK(Evas_Object **, ret)

/**
 * @def evas_obj_table_homogeneous_set
 * @since 1.8
 *
 * Set how this table should layout children.
 *
 * @param[in] homogeneous
 *
 * @see evas_object_table_homogeneous_set
 */
#define evas_obj_table_homogeneous_set(homogeneous) EVAS_OBJ_TABLE_ID(EVAS_OBJ_TABLE_SUB_ID_HOMOGENEOUS_SET), EO_TYPECHECK(Evas_Object_Table_Homogeneous_Mode, homogeneous)

/**
 * @def evas_obj_table_homogeneous_get
 * @since 1.8
 *
 * Get the current layout homogeneous mode.
 *
 * @param[out] ret
 *
 * @see evas_object_table_homogeneous_get
 */
#define evas_obj_table_homogeneous_get(ret) EVAS_OBJ_TABLE_ID(EVAS_OBJ_TABLE_SUB_ID_HOMOGENEOUS_GET), EO_TYPECHECK(Evas_Object_Table_Homogeneous_Mode *, ret)

/**
 * @def evas_obj_table_align_set
 * @since 1.8
 *
 * Set the alignment of the whole bounding box of contents.
 *
 * @param[in] horizontal
 * @param[in] vertical
 *
 * @see evas_object_table_align_set
 */
#define evas_obj_table_align_set(horizontal, vertical) EVAS_OBJ_TABLE_ID(EVAS_OBJ_TABLE_SUB_ID_ALIGN_SET), EO_TYPECHECK(double, horizontal), EO_TYPECHECK(double, vertical)

/**
 * @def evas_obj_table_align_get
 * @since 1.8
 *
 * Get alignment of the whole bounding box of contents.
 *
 * @param[out] horizontal
 * @param[out] vertical
 *
 * @see evas_object_table_align_get
 */
#define evas_obj_table_align_get(horizontal, vertical) EVAS_OBJ_TABLE_ID(EVAS_OBJ_TABLE_SUB_ID_ALIGN_GET), EO_TYPECHECK(double *, horizontal), EO_TYPECHECK(double *, vertical)

/**
 * @def evas_obj_table_padding_set
 * @since 1.8
 *
 * Set padding between cells.
 *
 * @param[in] horizontal
 * @param[in] vertical
 *
 * @see evas_object_table_padding_set
 */
#define evas_obj_table_padding_set(horizontal, vertical) EVAS_OBJ_TABLE_ID(EVAS_OBJ_TABLE_SUB_ID_PADDING_SET), EO_TYPECHECK(Evas_Coord, horizontal), EO_TYPECHECK(Evas_Coord, vertical)

/**
 * @def evas_obj_table_padding_get
 * @since 1.8
 *
 * Get padding between cells.
 *
 * @param[out] horizontal
 * @param[out] vertical
 *
 * @see evas_object_table_padding_get
 */
#define evas_obj_table_padding_get(horizontal, vertical) EVAS_OBJ_TABLE_ID(EVAS_OBJ_TABLE_SUB_ID_PADDING_GET), EO_TYPECHECK(Evas_Coord *, horizontal), EO_TYPECHECK(Evas_Coord *, vertical)

/**
 * @def evas_obj_table_pack_get
 * @since 1.8
 *
 * Get packing location of a child of table
 *
 * @param[in] child
 * @param[out] col
 * @param[out] row
 * @param[out] colspan
 * @param[out] rowspan
 * @param[out] ret
 *
 * @see evas_object_table_pack_get
 */
#define evas_obj_table_pack_get(child, col, row, colspan, rowspan, ret) EVAS_OBJ_TABLE_ID(EVAS_OBJ_TABLE_SUB_ID_PACK_GET), EO_TYPECHECK(Evas_Object *, child), EO_TYPECHECK(unsigned short *, col), EO_TYPECHECK(unsigned short *, row), EO_TYPECHECK(unsigned short *, colspan), EO_TYPECHECK(unsigned short *, rowspan), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def evas_obj_table_pack
 * @since 1.8
 *
 * Add a new child to a table object or set its current packing.
 *
 * @param[in] child
 * @param[in] col
 * @param[in] row
 * @param[in] colspan
 * @param[in] rowspan
 * @param[out] ret
 *
 * @see evas_object_table_pack
 */
#define evas_obj_table_pack(child, col, row, colspan, rowspan, ret) EVAS_OBJ_TABLE_ID(EVAS_OBJ_TABLE_SUB_ID_PACK), EO_TYPECHECK(Evas_Object *, child), EO_TYPECHECK(unsigned short, col), EO_TYPECHECK(unsigned short, row), EO_TYPECHECK(unsigned short, colspan), EO_TYPECHECK(unsigned short, rowspan), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def evas_obj_table_unpack
 * @since 1.8
 *
 * Remove child from table.
 *
 * @param[in] child
 * @param[out] ret
 *
 * @see evas_object_table_unpack
 */
#define evas_obj_table_unpack(child, ret) EVAS_OBJ_TABLE_ID(EVAS_OBJ_TABLE_SUB_ID_UNPACK), EO_TYPECHECK(Evas_Object *, child), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def evas_obj_table_clear
 * @since 1.8
 *
 * Faster way to remove all child objects from a table object.
 *
 * @param[in] clear
 *
 * @see evas_object_table_clear
 */
#define evas_obj_table_clear(clear) EVAS_OBJ_TABLE_ID(EVAS_OBJ_TABLE_SUB_ID_CLEAR), EO_TYPECHECK(Eina_Bool, clear)

/**
 * @def evas_obj_table_col_row_size_get
 * @since 1.8
 *
 * Get the number of columns and rows this table takes.
 *
 * @param[out] cols
 * @param[out] rows
 *
 * @see evas_object_table_col_row_size_get
 */
#define evas_obj_table_col_row_size_get(cols, rows) EVAS_OBJ_TABLE_ID(EVAS_OBJ_TABLE_SUB_ID_COL_ROW_SIZE_GET), EO_TYPECHECK(int *, cols), EO_TYPECHECK(int *, rows)

/**
 * @def evas_obj_table_iterator_new
 * @since 1.8
 *
 * Get an iterator to walk the list of children for the table.
 *
 * @param[out] ret
 *
 * @see evas_object_table_iterator_new
 */
#define evas_obj_table_iterator_new(ret) EVAS_OBJ_TABLE_ID(EVAS_OBJ_TABLE_SUB_ID_ITERATOR_NEW), EO_TYPECHECK(Eina_Iterator **, ret)

/**
 * @def evas_obj_table_accessor_new
 * @since 1.8
 *
 * Get an accessor to get random access to the list of children for the table.
 *
 * @param[out] ret
 *
 * @see evas_object_table_accessor_new
 */
#define evas_obj_table_accessor_new(ret) EVAS_OBJ_TABLE_ID(EVAS_OBJ_TABLE_SUB_ID_ACCESSOR_NEW), EO_TYPECHECK(Eina_Accessor **, ret)

/**
 * @def evas_obj_table_children_get
 * @since 1.8
 *
 * Get the list of children for the table.
 *
 * @param[out] ret
 *
 * @see evas_object_table_children_get
 */
#define evas_obj_table_children_get(ret) EVAS_OBJ_TABLE_ID(EVAS_OBJ_TABLE_SUB_ID_CHILDREN_GET), EO_TYPECHECK(Eina_List **, ret)

/**
 * @def evas_obj_table_mirrored_get
 * @since 1.8
 *
 * Gets the mirrored mode of the table.
 *
 * @param[out] ret
 *
 * @see evas_object_table_mirrored_get
 */
#define evas_obj_table_mirrored_get(ret) EVAS_OBJ_TABLE_ID(EVAS_OBJ_TABLE_SUB_ID_MIRRORED_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def evas_obj_table_mirrored_set
 * @since 1.8
 *
 * Sets the mirrored mode of the table. In mirrored mode the table items go
 *
 * @param[in] mirrored
 *
 * @see evas_object_table_mirrored_set
 */
#define evas_obj_table_mirrored_set(mirrored) EVAS_OBJ_TABLE_ID(EVAS_OBJ_TABLE_SUB_ID_MIRRORED_SET), EO_TYPECHECK(Eina_Bool, mirrored)


/**
 * @}
 */

/**
 * @ingroup Evas_Object_Grid
 *
 * @{
 */
#define EVAS_OBJ_GRID_CLASS evas_object_grid_class_get()

const Eo_Class *evas_object_grid_class_get(void) EINA_CONST;

extern EAPI Eo_Op EVAS_OBJ_GRID_BASE_ID;

enum
{
   EVAS_OBJ_GRID_SUB_ID_ADD_TO,
   EVAS_OBJ_GRID_SUB_ID_SIZE_SET,
   EVAS_OBJ_GRID_SUB_ID_SIZE_GET,
   EVAS_OBJ_GRID_SUB_ID_PACK,
   EVAS_OBJ_GRID_SUB_ID_UNPACK,
   EVAS_OBJ_GRID_SUB_ID_CLEAR,
   EVAS_OBJ_GRID_SUB_ID_PACK_GET,
   EVAS_OBJ_GRID_SUB_ID_ITERATOR_NEW,
   EVAS_OBJ_GRID_SUB_ID_ACCESSOR_NEW,
   EVAS_OBJ_GRID_SUB_ID_CHILDREN_GET,
   EVAS_OBJ_GRID_SUB_ID_MIRRORED_GET,
   EVAS_OBJ_GRID_SUB_ID_MIRRORED_SET,
   EVAS_OBJ_GRID_SUB_ID_LAST
};

#define EVAS_OBJ_GRID_ID(sub_id) (EVAS_OBJ_GRID_BASE_ID + sub_id)


/**
 * @def evas_obj_grid_add
 * @since 1.8
 *
 * Create a new grid.
 *
 * @param[out] ret
 *
 * @see evas_object_grid_add
 */
#define evas_obj_grid_add(ret) EVAS_OBJ_GRID_ID(EVAS_OBJ_GRID_SUB_ID_ADD), EO_TYPECHECK(Evas_Object **, ret)

/**
 * @def evas_obj_grid_add_to
 * @since 1.8
 *
 * Create a grid that is child of a given element parent.
 *
 * @param[out] ret
 *
 * @see evas_object_grid_add_to
 */
#define evas_obj_grid_add_to(ret) EVAS_OBJ_GRID_ID(EVAS_OBJ_GRID_SUB_ID_ADD_TO), EO_TYPECHECK(Evas_Object **, ret)

/**
 * @def evas_obj_grid_size_set
 * @since 1.8
 *
 * Set the virtual resolution for the grid
 *
 * @param[in] w
 * @param[in] h
 *
 * @see evas_object_grid_size_set
 */
#define evas_obj_grid_size_set(w, h) EVAS_OBJ_GRID_ID(EVAS_OBJ_GRID_SUB_ID_SIZE_SET), EO_TYPECHECK(int, w), EO_TYPECHECK(int, h)

/**
 * @def evas_obj_grid_size_get
 * @since 1.8
 *
 * Get the current virtual resolution
 *
 * @param[out] w
 * @param[out] h
 *
 * @see evas_object_grid_size_get
 */
#define evas_obj_grid_size_get(w, h) EVAS_OBJ_GRID_ID(EVAS_OBJ_GRID_SUB_ID_SIZE_GET), EO_TYPECHECK(int *, w), EO_TYPECHECK(int *, h)

/**
 * @def evas_obj_grid_pack
 * @since 1.8
 *
 * Add a new child to a grid object.
 *
 * @param[in] child
 * @param[in] x
 * @param[in] y
 * @param[in] w
 * @param[in] h
 * @param[out] ret
 *
 * @see evas_object_grid_pack
 */
#define evas_obj_grid_pack(child, x, y, w, h, ret) EVAS_OBJ_GRID_ID(EVAS_OBJ_GRID_SUB_ID_PACK), EO_TYPECHECK(Evas_Object *, child), EO_TYPECHECK(int, x), EO_TYPECHECK(int, y), EO_TYPECHECK(int, w), EO_TYPECHECK(int, h), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def evas_obj_grid_unpack
 * @since 1.8
 *
 * Remove child from grid.
 *
 * @param[in] child
 * @param[out] ret
 *
 * @see evas_object_grid_unpack
 */
#define evas_obj_grid_unpack(child, ret) EVAS_OBJ_GRID_ID(EVAS_OBJ_GRID_SUB_ID_UNPACK), EO_TYPECHECK(Evas_Object *, child), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def evas_obj_grid_clear
 * @since 1.8
 *
 * Faster way to remove all child objects from a grid object.
 *
 * @param[in] clear
 *
 * @see evas_object_grid_clear
 */
#define evas_obj_grid_clear(clear) EVAS_OBJ_GRID_ID(EVAS_OBJ_GRID_SUB_ID_CLEAR), EO_TYPECHECK(Eina_Bool, clear)

/**
 * @def evas_obj_grid_pack_get
 * @since 1.8
 *
 * Get the pack options for a grid child
 *
 * @param[in] child
 * @param[out] x
 * @param[out] y
 * @param[out] w
 * @param[out] h
 * @param[out] ret
 *
 * @see evas_object_grid_pack_get
 */
#define evas_obj_grid_pack_get(child, x, y, w, h, ret) EVAS_OBJ_GRID_ID(EVAS_OBJ_GRID_SUB_ID_PACK_GET), EO_TYPECHECK(Evas_Object *, child), EO_TYPECHECK(int *, x), EO_TYPECHECK(int *, y), EO_TYPECHECK(int *, w), EO_TYPECHECK(int *, h), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def evas_obj_grid_iterator_new
 * @since 1.8
 *
 * Get an iterator to walk the list of children for the grid.
 *
 * @param[out] ret
 *
 * @see evas_object_grid_iterator_new
 */
#define evas_obj_grid_iterator_new(ret) EVAS_OBJ_GRID_ID(EVAS_OBJ_GRID_SUB_ID_ITERATOR_NEW), EO_TYPECHECK(Eina_Iterator **, ret)

/**
 * @def evas_obj_grid_accessor_new
 * @since 1.8
 *
 * Get an accessor to get random access to the list of children for the grid.
 *
 * @param[out] ret
 *
 * @see evas_object_grid_accessor_new
 */
#define evas_obj_grid_accessor_new(ret) EVAS_OBJ_GRID_ID(EVAS_OBJ_GRID_SUB_ID_ACCESSOR_NEW), EO_TYPECHECK(Eina_Accessor **, ret)

/**
 * @def evas_obj_grid_children_get
 * @since 1.8
 *
 * Get the list of children for the grid.
 *
 * @param[out] ret
 *
 * @see evas_object_grid_children_get
 */
#define evas_obj_grid_children_get(ret) EVAS_OBJ_GRID_ID(EVAS_OBJ_GRID_SUB_ID_CHILDREN_GET), EO_TYPECHECK(Eina_List **, ret)

/**
 * @def evas_obj_grid_mirrored_get
 * @since 1.8
 *
 * Gets the mirrored mode of the grid.
 *
 * @param[out] ret
 *
 * @see evas_object_grid_mirrored_get
 */
#define evas_obj_grid_mirrored_get(ret) EVAS_OBJ_GRID_ID(EVAS_OBJ_GRID_SUB_ID_MIRRORED_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def evas_obj_grid_mirrored_set
 * @since 1.8
 *
 * Sets the mirrored mode of the grid. In mirrored mode the grid items go
 *
 * @param[in] mirrored
 *
 * @see evas_object_grid_mirrored_set
 */
#define evas_obj_grid_mirrored_set(mirrored) EVAS_OBJ_GRID_ID(EVAS_OBJ_GRID_SUB_ID_MIRRORED_SET), EO_TYPECHECK(Eina_Bool, mirrored)

/**
 * @}
 */

#define EVAS_COMMON_CLASS evas_common_class_get()

const Eo_Class *evas_common_class_get(void) EINA_CONST;

extern EAPI Eo_Op EVAS_COMMON_BASE_ID;

enum
{
   EVAS_COMMON_SUB_ID_EVAS_GET,
   EVAS_COMMON_SUB_ID_LAST
};

#define EVAS_COMMON_ID(sub_id) (EVAS_COMMON_BASE_ID + sub_id)


/**
 * @def evas_common_evas_get
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[out] ret
 *
 * @see evas_object_evas_common_evas_get
 */
#define evas_common_evas_get(ret) EVAS_COMMON_ID(EVAS_COMMON_SUB_ID_EVAS_GET), EO_TYPECHECK(Evas **, ret)

extern EAPI Eo_Op EVAS_OBJ_BASE_ID;

enum
{
   EVAS_OBJ_SUB_ID_POSITION_SET,
   EVAS_OBJ_SUB_ID_POSITION_GET,
   EVAS_OBJ_SUB_ID_SIZE_SET,
   EVAS_OBJ_SUB_ID_SIZE_GET,
   EVAS_OBJ_SUB_ID_SIZE_HINT_MIN_SET,
   EVAS_OBJ_SUB_ID_SIZE_HINT_MIN_GET,
   EVAS_OBJ_SUB_ID_SIZE_HINT_MAX_SET,
   EVAS_OBJ_SUB_ID_SIZE_HINT_MAX_GET,
   EVAS_OBJ_SUB_ID_SIZE_HINT_REQUEST_SET,
   EVAS_OBJ_SUB_ID_SIZE_HINT_REQUEST_GET,
   EVAS_OBJ_SUB_ID_SIZE_HINT_ASPECT_SET,
   EVAS_OBJ_SUB_ID_SIZE_HINT_ASPECT_GET,
   EVAS_OBJ_SUB_ID_SIZE_HINT_ALIGN_SET,
   EVAS_OBJ_SUB_ID_SIZE_HINT_ALIGN_GET,
   EVAS_OBJ_SUB_ID_SIZE_HINT_WEIGHT_SET,
   EVAS_OBJ_SUB_ID_SIZE_HINT_WEIGHT_GET,
   EVAS_OBJ_SUB_ID_SIZE_HINT_PADDING_SET,
   EVAS_OBJ_SUB_ID_SIZE_HINT_PADDING_GET,
   EVAS_OBJ_SUB_ID_VISIBILITY_SET,
   EVAS_OBJ_SUB_ID_VISIBILITY_GET,
   EVAS_OBJ_SUB_ID_COLOR_SET,
   EVAS_OBJ_SUB_ID_COLOR_GET,
   EVAS_OBJ_SUB_ID_ANTI_ALIAS_SET,
   EVAS_OBJ_SUB_ID_ANTI_ALIAS_GET,
   EVAS_OBJ_SUB_ID_SCALE_SET,
   EVAS_OBJ_SUB_ID_SCALE_GET,
   EVAS_OBJ_SUB_ID_RENDER_OP_SET,
   EVAS_OBJ_SUB_ID_RENDER_OP_GET,
   EVAS_OBJ_SUB_ID_TYPE_SET,
   EVAS_OBJ_SUB_ID_TYPE_GET,
   EVAS_OBJ_SUB_ID_PRECISE_IS_INSIDE_SET,
   EVAS_OBJ_SUB_ID_PRECISE_IS_INSIDE_GET,
   EVAS_OBJ_SUB_ID_STATIC_CLIP_SET,
   EVAS_OBJ_SUB_ID_STATIC_CLIP_GET,
   EVAS_OBJ_SUB_ID_IS_FRAME_OBJECT_SET,
   EVAS_OBJ_SUB_ID_IS_FRAME_OBJECT_GET,
   EVAS_OBJ_SUB_ID_FREEZE_EVENTS_SET,
   EVAS_OBJ_SUB_ID_FREEZE_EVENTS_GET,
   EVAS_OBJ_SUB_ID_PASS_EVENTS_SET,
   EVAS_OBJ_SUB_ID_PASS_EVENTS_GET,
   EVAS_OBJ_SUB_ID_REPEAT_EVENTS_SET,
   EVAS_OBJ_SUB_ID_REPEAT_EVENTS_GET,
   EVAS_OBJ_SUB_ID_PROPAGATE_EVENTS_SET,
   EVAS_OBJ_SUB_ID_PROPAGATE_EVENTS_GET,
   EVAS_OBJ_SUB_ID_POINTER_MODE_SET,
   EVAS_OBJ_SUB_ID_POINTER_MODE_GET,
   EVAS_OBJ_SUB_ID_KEY_GRAB,
   EVAS_OBJ_SUB_ID_KEY_UNGRAB,
   EVAS_OBJ_SUB_ID_FOCUS_SET,
   EVAS_OBJ_SUB_ID_FOCUS_GET,
   EVAS_OBJ_SUB_ID_NAME_SET,
   EVAS_OBJ_SUB_ID_NAME_GET,
   EVAS_OBJ_SUB_ID_NAME_CHILD_FIND,
   EVAS_OBJ_SUB_ID_LAYER_SET,
   EVAS_OBJ_SUB_ID_LAYER_GET,
   EVAS_OBJ_SUB_ID_CLIP_SET,
   EVAS_OBJ_SUB_ID_CLIP_GET,
   EVAS_OBJ_SUB_ID_CLIP_UNSET,
   EVAS_OBJ_SUB_ID_CLIPEES_GET,
   EVAS_OBJ_SUB_ID_CLIPEES_HAS,
   EVAS_OBJ_SUB_ID_MAP_ENABLE_SET,
   EVAS_OBJ_SUB_ID_MAP_ENABLE_GET,
   EVAS_OBJ_SUB_ID_MAP_SET,
   EVAS_OBJ_SUB_ID_MAP_GET,
   EVAS_OBJ_SUB_ID_SMART_PARENT_GET,
   EVAS_OBJ_SUB_ID_SMART_DATA_GET,
   EVAS_OBJ_SUB_ID_SMART_TYPE_CHECK,
   EVAS_OBJ_SUB_ID_SMART_TYPE_CHECK_PTR,
   EVAS_OBJ_SUB_ID_SMART_MOVE_CHILDREN_RELATIVE,
   EVAS_OBJ_SUB_ID_SMART_CLIPPED_CLIPPER_GET,
   EVAS_OBJ_SUB_ID_RAISE,
   EVAS_OBJ_SUB_ID_LOWER,
   EVAS_OBJ_SUB_ID_STACK_ABOVE,
   EVAS_OBJ_SUB_ID_STACK_BELOW,
   EVAS_OBJ_SUB_ID_ABOVE_GET,
   EVAS_OBJ_SUB_ID_BELOW_GET,
   EVAS_OBJ_SUB_ID_LAST
};

#define EVAS_OBJ_ID(sub_id) (EVAS_OBJ_BASE_ID + sub_id)

/**
 * @ingroup Evas_Object_Group_Basic
 *
 * @{
 */

/**
 * @def evas_obj_position_set
 * @since 1.8
 *
 * Move the given Evas object to the given location inside its
 * canvas' viewport.
 *
 * @param[in] x   in
 * @param[in] y   in
 *
 * @see evas_object_move
 */
#define evas_obj_position_set(x, y) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_POSITION_SET), EO_TYPECHECK(Evas_Coord, x), EO_TYPECHECK(Evas_Coord, y)

/**
 * @def evas_obj_position_get
 * @since 1.8
 *
 * Retrieves the position of the given Evas object.
 *
 * @param[out] x out
 * @param[out] y out
 *
 * @see evas_object_geometry_get
 */
#define evas_obj_position_get(x, y) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_POSITION_GET), EO_TYPECHECK(Evas_Coord *, x), EO_TYPECHECK(Evas_Coord *, y)

/**
 * @def evas_obj_size_set
 * @since 1.8
 * Changes the size of the given Evas object.
 *
 * @param [in] w in
 * @param [in] h in
 *
 * @see evas_object_resize
 */
#define evas_obj_size_set(w, h) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_SIZE_SET), EO_TYPECHECK(Evas_Coord, w), EO_TYPECHECK(Evas_Coord, h)

/**
 * @def evas_obj_size_get
 * @since 1.8
 *
 * Retrieves the (rectangular) size of the given Evas object.
 *
 * @param[out] w out
 * @param[out] h out
 *
 * @see evas_object_geometry_get
 */
#define evas_obj_size_get(w, h) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_SIZE_GET), EO_TYPECHECK(Evas_Coord *, w), EO_TYPECHECK(Evas_Coord *, h)

/**
 * @def evas_obj_visibility_set
 * @since 1.8
 *
 * Makes the given Evas object visible or invisible.
 * @param[in] v @c EINA_TRUE if to make the object visible, @c EINA_FALSE
 * otherwise.
 *
 * @see evas_object_show
 * @see evas_object_hide
 */
#define evas_obj_visibility_set(v) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_VISIBILITY_SET), EO_TYPECHECK(Eina_Bool, v)

/**
 * @def evas_obj_visibility_get
 * @since 1.8
 *
 * Retrieves whether or not the given Evas object is visible.
 *
 * @param[out] v @c EINA_TRUE if the object is visible, @c EINA_FALSE
 * otherwise.
 *
 * @see evas_object_visible_get
 */
#define evas_obj_visibility_get(v) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_VISIBILITY_GET), EO_TYPECHECK(Eina_Bool *, v)

/**
 * @def evas_obj_color_set
 * @since 1.8
 *
 * Sets the general/main color of the given Evas object to the given
 * one.
 *
 * @param[in] r   in
 * @param[in] g   in
 * @param[in] b   in
 * @param[in] a   in
 *
 * @see evas_object_color_set
 */
#define evas_obj_color_set(r, g, b, a) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_COLOR_SET), EO_TYPECHECK(int, r), EO_TYPECHECK(int, g), EO_TYPECHECK(int, b), EO_TYPECHECK(int, a)

/**
 * @def evas_obj_color_get
 * @since 1.8
 * Retrieves the general/main color of the given Evas object.
 *
 * @param[out] r out
 * @param[out] g out
 * @param[out] b out
 * @param[out] a out
 *
 * @see evas_object_color_get
 */
#define evas_obj_color_get(r, g, b, a) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_COLOR_GET), EO_TYPECHECK(int *, r), EO_TYPECHECK(int *, g), EO_TYPECHECK(int *, b), EO_TYPECHECK(int *, a)
/**
 * @}
 */

/**
 * @ingroup Evas_Object_Group_Size_Hints
 *
 * @{
 */
/**
 * @def evas_obj_size_hint_min_set
 * @since 1.8
 *
 * Sets the hints for an object's minimum size.
 *
 * @param[in] w in
 * @param[in] h in
 *
 * @see evas_object_size_hint_min_set
 */
#define evas_obj_size_hint_min_set(w, h) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_SIZE_HINT_MIN_SET), EO_TYPECHECK(Evas_Coord, w), EO_TYPECHECK(Evas_Coord, h)

/**
 * @def evas_obj_size_hint_min_get
 * @since 1.8
 *
 * Retrieves the hints for an object's minimum size.
 *
 * @param[out] w out
 * @param[out] h out
 *
 * @see evas_object_size_hint_min_get
 */
#define evas_obj_size_hint_min_get(w, h) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_SIZE_HINT_MIN_GET), EO_TYPECHECK(Evas_Coord *, w), EO_TYPECHECK(Evas_Coord *, h)

/**
 * @def evas_obj_size_hint_max_set
 * @since 1.8
 *
 * Sets the hints for an object's maximum size.
 *
 * @param[in] w in
 * @param[in] h in
 *
 * @see evas_object_size_hint_max_set
 */
#define evas_obj_size_hint_max_set(w, h) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_SIZE_HINT_MAX_SET), EO_TYPECHECK(Evas_Coord, w), EO_TYPECHECK(Evas_Coord, h)

/**
 * @def evas_obj_size_hint_max_get
 * @since 1.8
 *
 * Retrieves the hints for an object's maximum size.
 *
 * @param[out] w out
 * @param[out] h out
 *
 * @see evas_object_size_hint_max_get
 */
#define evas_obj_size_hint_max_get(w, h) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_SIZE_HINT_MAX_GET), EO_TYPECHECK(Evas_Coord *, w), EO_TYPECHECK(Evas_Coord *, h)

/**
 * @def evas_obj_size_hint_request_set
 * @since 1.8
 *
 * Sets the hints for an object's optimum size.
 *
 * @param[in] w in
 * @param[in] h in
 *
 * @see evas_object_size_hint_request_set
 */
#define evas_obj_size_hint_request_set(w, h) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_SIZE_HINT_REQUEST_SET), EO_TYPECHECK(Evas_Coord, w), EO_TYPECHECK(Evas_Coord, h)

/**
 * @def evas_obj_size_hint_request_get
 * @since 1.8
 *
 * Retrieves the hints for an object's optimum size.
 *
 * @param[out] w out
 * @param[out] h out
 *
 * @see evas_object_size_hint_request_get
 */
#define evas_obj_size_hint_request_get(w, h) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_SIZE_HINT_REQUEST_GET), EO_TYPECHECK(Evas_Coord *, w), EO_TYPECHECK(Evas_Coord *, h)

/**
 * @def evas_obj_size_hint_aspect_set
 * @since 1.8
 *
 * Sets the hints for an object's aspect ratio.
 *
 * @param[in] aspect in
 * @param[in] w in
 * @param[in] h in
 *
 * @see evas_object_size_hint_aspect_set
 */
#define evas_obj_size_hint_aspect_set(aspect, w, h) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_SIZE_HINT_ASPECT_SET), EO_TYPECHECK(Evas_Aspect_Control, aspect), EO_TYPECHECK(Evas_Coord, w), EO_TYPECHECK(Evas_Coord, h)

/**
 * @def evas_obj_size_hint_aspect_get
 * @since 1.8
 *
 * Retrieves the hints for an object's aspect ratio.
 *
 * @param[out] aspect out
 * @param[out] w out
 * @param[out] h out
 *
 * @see evas_object_size_hint_aspect_get
 */
#define evas_obj_size_hint_aspect_get(aspect, w, h) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_SIZE_HINT_ASPECT_GET), EO_TYPECHECK(Evas_Aspect_Control *, aspect), EO_TYPECHECK(Evas_Coord *, w), EO_TYPECHECK(Evas_Coord *, h)

/**
 * @def evas_obj_size_hint_align_set
 * @since 1.8
 *
 * Sets the hints for an object's alignment.
 *
 * @param[in] x in
 * @param[in] y in
 *
 * @see evas_object_size_hint_align_set
 */
#define evas_obj_size_hint_align_set(x, y) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_SIZE_HINT_ALIGN_SET), EO_TYPECHECK(double, x), EO_TYPECHECK(double, y)

/**
 * @def evas_obj_size_hint_align_get
 * @since 1.8
 *
 * Retrieves the hints for on object's alignment.
 *
 * @param[out] x out
 * @param[out] y out
 *
 * @see evas_object_size_hint_align_get
 */
#define evas_obj_size_hint_align_get(x, y) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_SIZE_HINT_ALIGN_GET), EO_TYPECHECK(double *, x), EO_TYPECHECK(double *, y)

/**
 * @def evas_obj_size_hint_weight_set
 * @since 1.8
 *
 * Sets the hints for an object's weight.
 *
 * @param[in] x in
 * @param[in] y in
 *
 * @see evas_object_size_hint_weight_set
 */
#define evas_obj_size_hint_weight_set(x, y) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_SIZE_HINT_WEIGHT_SET), EO_TYPECHECK(double, x), EO_TYPECHECK(double, y)

/**
 * @def evas_obj_size_hint_weight_get
 * @since 1.8
 *
 * Retrieves the hints for an object's weight.
 *
 * @param[out] x out
 * @param[out] y out
 *
 * @see evas_object_size_hint_weight_get
 */
#define evas_obj_size_hint_weight_get(x, y) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_SIZE_HINT_WEIGHT_GET), EO_TYPECHECK(double *, x), EO_TYPECHECK(double *, y)

/**
 * @def evas_obj_size_hint_padding_set
 * @since 1.8
 *
 * Sets the hints for an object's padding space.
 *
 * @param[in] l in
 * @param[in] r in
 * @param[in] t in
 * @param[in] b in
 *
 * @see evas_object_size_hint_padding_set
 */
#define evas_obj_size_hint_padding_set(l, r, t, b) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_SIZE_HINT_PADDING_SET), EO_TYPECHECK(Evas_Coord, l), EO_TYPECHECK(Evas_Coord, r), EO_TYPECHECK(Evas_Coord, t), EO_TYPECHECK(Evas_Coord, b)

/**
 * @def evas_obj_size_hint_padding_get
 * @since 1.8
 *
 * Retrieves the hints for an object's padding space.
 *
 * @param[out] l out
 * @param[out] r out
 * @param[out] t out
 * @param[out] b out
 *
 * @see evas_object_size_hint_padding_get
 */
#define evas_obj_size_hint_padding_get(l, r, t, b) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_SIZE_HINT_PADDING_GET), EO_TYPECHECK(Evas_Coord *, l), EO_TYPECHECK(Evas_Coord *, r), EO_TYPECHECK(Evas_Coord *, t), EO_TYPECHECK(Evas_Coord *, b)

/**
 * @}
 */

/**
 * @ingroup Evas_Object_Group_Extras
 *
 * @{
 */

/**
 * @def evas_obj_anti_alias_set
 * @since 1.8
 *
 * Sets whether or not the given Evas object is to be drawn anti-aliased.
 *
 * @param[in] anti_alias in
 *
 * @see evas_object_anti_alias_set
 */
#define evas_obj_anti_alias_set(anti_alias) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_ANTI_ALIAS_SET), EO_TYPECHECK(Eina_Bool, anti_alias)

/**
 * @def evas_obj_anti_alias_get
 * @since 1.8
 *
 * Retrieves whether or not the given Evas object is to be drawn anti_aliased.
 *
 * @param[out] anti_alias out
 *
 * @see evas_object_anti_alias_get
 */
#define evas_obj_anti_alias_get(anti_alias) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_ANTI_ALIAS_GET), EO_TYPECHECK(Eina_Bool *, anti_alias)

/**
 * @def evas_obj_scale_set
 * @since 1.8
 *
 * Sets the scaling factor for an Evas object. Does not affect all
 * objects.
 *
 * @param[in] scale in
 *
 * @see evas_object_scale_set
 */
#define evas_obj_scale_set(scale) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_SCALE_SET), EO_TYPECHECK(double, scale)

/**
 * @def evas_obj_scale_get
 * @since 1.8
 *
 * Retrieves the scaling factor for the given Evas object.
 *
 * @param[out] scale out
 *
 * @see evas_object_scale_get
 */
#define evas_obj_scale_get(scale) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_SCALE_GET), EO_TYPECHECK(double *, scale)

/**
 * @def evas_obj_render_op_set
 * @since 1.8
 *
 * Sets the render_op to be used for rendering the Evas object.
 *
 * @param[in] render_op in
 *
 * @see evas_object_render_op_set
 */
#define evas_obj_render_op_set(render_op) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_RENDER_OP_SET), EO_TYPECHECK(Evas_Render_Op, render_op)

/**
 * @def evas_obj_render_op_get
 * @since 1.8
 *
 *
 * @param[out] render_op
 *
 * @see evas_object_render_op_get
 */
#define evas_obj_render_op_get(render_op) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_RENDER_OP_GET), EO_TYPECHECK(Evas_Render_Op *, render_op)

/**
 * @def evas_obj_precise_is_inside_set
 * @since 1.8
 *
 * Set whether to use precise (usually expensive) point collision
 * detection for a given Evas object.
 *
 * @param[in] precise in
 *
 * @see evas_object_precise_is_inside_set
 */
#define evas_obj_precise_is_inside_set(precise) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_PRECISE_IS_INSIDE_SET), EO_TYPECHECK(Eina_Bool, precise)

/**
 * @def evas_obj_precise_is_inside_get
 * @since 1.8
 *
 * Determine whether an object is set to use precise point collision
 * detection.
 *
 * @param[out] precise out
 *
 * @see evas_object_precise_is_inside_get
 */
#define evas_obj_precise_is_inside_get(precise) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_PRECISE_IS_INSIDE_GET), EO_TYPECHECK(Eina_Bool *, precise)

/**
 * @def evas_obj_static_clip_set
 * @since 1.8
 *
 * Set a hint flag on the given Evas object that it's used as a "static
 * clipper".
 *
 * @param[in] is_static_clip in
 *
 * @see evas_object_static_clip_set
 */
#define evas_obj_static_clip_set(is_static_clip) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_STATIC_CLIP_SET), EO_TYPECHECK(Eina_Bool, is_static_clip)

/**
 * @def evas_obj_static_clip_get
 * @since 1.8
 *
 * Get the "static clipper" hint flag for a given Evas object.
 *
 * @param[out] is_static_clip out
 *
 * @see evas_object_static_clip_get
 */
#define evas_obj_static_clip_get(is_static_clip) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_STATIC_CLIP_GET), EO_TYPECHECK(Eina_Bool *, is_static_clip)
/**
 * @}
 */

/**
 * @ingroup Evas_Object_Group_Basic
 *
 * @{
 */

/**
 * @def evas_obj_evas_get
 * @since 1.8
 * Retrieves the Evas canvas that the given object lives on.
 *
 * @param[out] evas
 * @see evas_object_evas_get
 */
#define evas_obj_evas_get(evas) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_EVAS_GET), EO_TYPECHECK(Evas **, evas)

/**
 * @def evas_obj_type_get
 * @since 1.8
 * Retrieves the type of the given Evas object.
 *
 * @param[out] type out
 * @see evas_object_type_get
 */
#define evas_obj_type_get(type) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_TYPE_GET), EO_TYPECHECK(const char **, type)

/**
 * @def evas_obj_type_set
 * @since 1.8
 * Sets the type of the given Evas object.
 *
 * @param[in] type in
 */
#define evas_obj_type_set(type) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_TYPE_SET), EO_TYPECHECK(const char *, type)

/**
 * @def evas_obj_raise
 * @since 1.8
 *
 * Raise obj to the top of its layer.
 *
 * @see evas_object_raise
 */
#define evas_obj_raise() EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_RAISE)

/**
 * @def evas_obj_lower
 * @since 1.8
 *
 * Lower obj to the bottom of its layer.
 *
 * @see evas_object_lower
 */
#define evas_obj_lower() EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_LOWER)

/**
 * @def evas_obj_stack_above
 * @since 1.8
 *
 * Stack the object immediately above @p above
 *
 * @param[in] above in
 *
 * @see evas_object_stack_above
 */
#define evas_obj_stack_above(above) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_STACK_ABOVE), EO_TYPECHECK(Evas_Object *, above)

/**
 * @def evas_obj_stack_below
 * @since 1.8
 *
 * Stack the object immediately below @p below
 *
 * @param[in] below in
 *
 * @see evas_object_stack_below
 */
#define evas_obj_stack_below(below) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_STACK_BELOW), EO_TYPECHECK(Evas_Object *, below)

/**
 * @def evas_obj_above_get
 * @since 1.8
 *
 * Get the Evas object stacked right above the object
 *
 * @param[out] ret out
 *
 * @see evas_object_above_get
 */
#define evas_obj_above_get(ret) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_ABOVE_GET), EO_TYPECHECK(Evas_Object **, ret)

/**
 * @def evas_obj_below_get
 * @since 1.8
 *
 * Get the Evas object stacked right below the object
 *
 * @param[out] ret out
 *
 * @see evas_object_below_get
 */
#define evas_obj_below_get(ret) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_BELOW_GET), EO_TYPECHECK(Evas_Object **, ret)
/**
 * @}
 */

/**
 * @def evas_obj_is_frame_object_set
 * @since 1.8
 *
 * @param[in] is_frame in
 *
 * @see evas_object_is_frame_object_set
 */
#define evas_obj_is_frame_object_set(is_frame) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_IS_FRAME_OBJECT_SET), EO_TYPECHECK(Eina_Bool, is_frame)

/**
 * @def evas_obj_is_frame_object_get
 * @since 1.8
 *
 * @param[out] is_frame out
 *
 * @see evas_object_is_frame_object_get
 */
#define evas_obj_is_frame_object_get(is_frame) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_IS_FRAME_OBJECT_GET), EO_TYPECHECK(Eina_Bool *, is_frame)

/**
 * @ingroup Evas_Object_Group_Events
 *
 * @{
 */

/**
 * @def evas_obj_freeze_events_set
 * @since 1.8
 *
 * Set whether an Evas object is to freeze (discard) events.
 *
 * @param[in] freeze in
 *
 * @see evas_object_freeze_events_set
 */
#define evas_obj_freeze_events_set(freeze) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_FREEZE_EVENTS_SET), EO_TYPECHECK(Eina_Bool, freeze)

/**
 * @def evas_obj_freeze_events_get
 * @since 1.8
 *
 * Determine whether an object is set to freeze (discard) events.
 *
 * @param[out] freeze out
 *
 * @see evas_object_freeze_events_get
 */
#define evas_obj_freeze_events_get(freeze) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_FREEZE_EVENTS_GET), EO_TYPECHECK(Eina_Bool *, freeze)

/**
 * @def evas_obj_pass_events_set
 * @since 1.8
 *
 * Set whether an Evas object is to pass (ignore) events.
 *
 * @param[in] pass in
 *
 * @see evas_object_pass_events_set
 */
#define evas_obj_pass_events_set(pass) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_PASS_EVENTS_SET), EO_TYPECHECK(Eina_Bool, pass)

/**
 * @def evas_obj_pass_events_get
 * @since 1.8
 *
 * Determine whether an object is set to pass (ignore) events.
 *
 * @param[out] pass
 *
 * @see evas_object_pass_events_get
 */
#define evas_obj_pass_events_get(pass) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_PASS_EVENTS_GET), EO_TYPECHECK(Eina_Bool *, pass)

/**
 * @def evas_obj_repeat_events_set
 * @since 1.8
 *
 * Set whether an Evas object is to repeat events.
 *
 * @param[in] repeat in
 *
 * @see evas_object_repeat_events_set
 */
#define evas_obj_repeat_events_set(repeat) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_REPEAT_EVENTS_SET), EO_TYPECHECK(Eina_Bool, repeat)

/**
 * @def evas_obj_repeat_events_get
 * @since 1.8
 *
 * Determine whether an object is set to repeat events.
 *
 * @param[out] repeat out
 *
 * @see evas_object_repeat_events_get
 */
#define evas_obj_repeat_events_get(repeat) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_REPEAT_EVENTS_GET), EO_TYPECHECK(Eina_Bool *, repeat)

/**
 * @def evas_obj_propagate_events_set
 * @since 1.8
 *
 * Set whether events on a smart object's member should get propagated
 * up to its parent.
 *
 * @param[in] propagate in
 * @see evas_object_propagate_events_set
 */
#define evas_obj_propagate_events_set(propagate) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_PROPAGATE_EVENTS_SET), EO_TYPECHECK(Eina_Bool, propagate)

/**
 * @def evas_obj_propagate_events_get
 * @since 1.8
 *
 * Retrieve whether an Evas object is set to propagate events.
 *
 * @param[out] propagate out
 *
 * @see evas_object_propagate_events_get
 */
#define evas_obj_propagate_events_get(propagate) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_PROPAGATE_EVENTS_GET), EO_TYPECHECK(Eina_Bool *, propagate)

/**
 * @}
 */

/**
 * @ingroup Evas_Object_Group_Extras
 *
 * @{
 */

/**
 * @def evas_obj_pointer_mode_set
 * @since 1.8
 *
 * Set pointer behavior.
 *
 * @param[in] pointer_mode in
 *
 * @see evas_object_pointer_mode_set
 */
#define evas_obj_pointer_mode_set(pointer_mode) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_POINTER_MODE_SET), EO_TYPECHECK(Evas_Object_Pointer_Mode, pointer_mode)

/**
 * @def evas_obj_pointer_mode_get
 * @since 1.8
 *
 * Determine how pointer will behave.
 *
 * @param[out] pointer_mode out
 *
 * @see evas_object_pointer_mode_get
 */
#define evas_obj_pointer_mode_get(pointer_mode) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_POINTER_MODE_GET), EO_TYPECHECK(Evas_Object_Pointer_Mode *, pointer_mode)
/**
 * @}
 */

/**
 * @ingroup Evas_Object_Group_Basic
 *
 * @{
 */

/**
 * @def evas_obj_clip_set
 * @since 1.8
 * Clip one object to another.
 *
 * @param[in] clip in
 *
 * @see evas_object_clip_set
 */
#define evas_obj_clip_set(clip) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_CLIP_SET), EO_TYPECHECK(Evas_Object *, clip)

/**
 * @def evas_obj_clip_get
 * @since 1.8
 * Get the object clipping @p obj (if any).
 *
 * @param[out] clip out
 *
 * @see evas_object_clip_get
 */
#define evas_obj_clip_get(clip) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_CLIP_GET), EO_TYPECHECK(Evas_Object **, clip)

/**
 * @def evas_obj_clip_unset
 * @since 1.8
 * Disable/cease clipping on a clipped @p obj object.
 *
 * @see evas_object_clip_unset
 */
#define evas_obj_clip_unset() EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_CLIP_UNSET)

/**
 * @def evas_obj_clipees_get
 * @since 1.8
 * Return a list of objects currently clipped by @p obj.
 *
 * @param[out] clipees out
 *
 * @see evas_object_clipees_get
 */
#define evas_obj_clipees_get(clipees) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_CLIPEES_GET), EO_TYPECHECK(const Eina_List **, clipees)

/**
 * @def evas_obj_clipees_has
 * @since 1.8
 * Return a list of objects currently clipped by @p obj.
 *
 * @param[out] clipees_has out
 *
 * @see evas_object_clipees_has
 */
#define evas_obj_clipees_has(clipees_has) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_CLIPEES_HAS), EO_TYPECHECK(const Eina_Bool *, clipees_has)

/**
 * @def evas_obj_focus_set
 * @since 1.8
 * Sets or unsets a given object as the currently focused one on its
 * canvas.
 * @param[in] focus in
 *
 * @see evas_object_focus_set
 */
#define evas_obj_focus_set(focus) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_FOCUS_SET), EO_TYPECHECK(Eina_Bool, focus)

/**
 * @def evas_obj_focus_get
 * @since 1.8
 * Retrieve whether an object has the focus.
 *
 * @param[out] focus out
 *
 * @see evas_object_focus_get
 */
#define evas_obj_focus_get(focus) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_FOCUS_GET), EO_TYPECHECK(Eina_Bool *, focus)

/**
 * @def evas_obj_name_set
 * @since 1.8
 * Sets the name of the given Evas object to the given name.
 *
 * @param[in]   name in
 *
 * @see evas_object_name_set
 */
#define evas_obj_name_set(name) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_NAME_SET), EO_TYPECHECK(const char *, name)

/**
 * @def evas_obj_name_get
 * @since 1.8
 *
 * Retrieves the name of the given Evas object.
 *
 * @param[out] name out
 *
 * @see evas_object_name_get
 *
 */
#define evas_obj_name_get(name) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_NAME_GET), EO_TYPECHECK(const char **, name)

/**
 * @ingroup Evas_Keys
 *
 * @{
 */
/**
 * @def evas_obj_key_grab
 * @since 1.8
 *
 * Requests @p keyname key events be directed to the obj.
 * @param[in] keyname in
 * @param[in] modifiers in
 * @param[in] not_modifiers in
 * @param[in] exclusive in
 * @param[out] ret out
 *
 * @see evas_object_key_grab
 */
#define evas_obj_key_grab(keyname, modifiers, not_modifiers, exclusive, ret) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_KEY_GRAB), EO_TYPECHECK(const char *, keyname), EO_TYPECHECK(Evas_Modifier_Mask, modifiers), EO_TYPECHECK(Evas_Modifier_Mask, not_modifiers), EO_TYPECHECK(Eina_Bool, exclusive), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def evas_obj_key_ungrab
 * @since 1.8
 *
 * Removes the grab on @p keyname key events by the obj.
 *
 * @param[in] keyname
 * @param[in] modifiers
 * @param[in] not_modifiers
 *
 * @see evas_object_key_ungrab
 */
#define evas_obj_key_ungrab(keyname, modifiers, not_modifiers) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_KEY_UNGRAB), EO_TYPECHECK(const char *, keyname), EO_TYPECHECK(Evas_Modifier_Mask, modifiers), EO_TYPECHECK(Evas_Modifier_Mask, not_modifiers)
/**
 * @}
 */

/**
 * @ingroup Evas_Object_Group_Basic
 *
 * @{
 */

/**
 * @def evas_obj_layer_set
 * @since 1.8
 * Sets the layer of the its canvas that the given object will be part
 * of.
 *
 * @param[in]   l   in
 *
 * @see evas_object_layer_set
 */
#define evas_obj_layer_set(l) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_LAYER_SET), EO_TYPECHECK(short, l)

/**
 * @def evas_obj_layer_get
 * @since 1.8
 * Retrieves the layer of its canvas that the given object is part of.
 *
 * @param[out] l out
 *
 * @see evas_object_layer_get
 */
#define evas_obj_layer_get(l) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_LAYER_GET), EO_TYPECHECK(short *, l)
/**
 * @}
 */

/**
 * @ingroup Evas_Object_Group_Map
 *
 * @{
 */

/**
 * @def evas_obj_map_enable_set
 * @since 1.8
 *
 * Enable or disable the map that is set.
 *
 * @param[in] enabled in
 *
 * @see evas_object_map_enable_set
 */
#define evas_obj_map_enable_set(enabled) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_MAP_ENABLE_SET), EO_TYPECHECK(Eina_Bool, enabled)

/**
 * @def evas_obj_map_enable_get
 * @since 1.8
 *
 * Get the map enabled state
 *
 * @param[out] enabled out
 *
 * @see evas_object_map_enable_get
 */
#define evas_obj_map_enable_get(enabled) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_MAP_ENABLE_GET), EO_TYPECHECK(Eina_Bool *, enabled)

/**
 * @def evas_obj_map_source_set
 * @since 1.8
 *
 * Set the map source object
 *
 * @param[in] source in
 *
 * @see evas_object_map_source_set
 */
#define evas_obj_map_source_set(source) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_MAP_SOURCE_SET), EO_TYPECHECK(Evas_Object *, source)

/**
 * @def evas_obj_map_source_get
 * @since 1.8
 *
 * Get the map source object
 *
 * @param[out] source out
 *
 * @see evas_object_map_source_get
 */
#define evas_obj_map_source_get(source) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_MAP_SOURCE_GET), EO_TYPECHECK(Evas_Object **, source)

/**
 * @def evas_obj_map_set
 * @since 1.8
 *
 * Set current object transformation map.
 *
 * @param[in] map in
 *
 * @see evas_object_map_set
 */
#define evas_obj_map_set(map) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_MAP_SET), EO_TYPECHECK(const Evas_Map *, map)

/**
 * @def evas_obj_map_get
 * @since 1.8
 *
 * Get current object transformation map.
 *
 * @param[out] map out
 *
 * @see evas_object_map_get
 */
#define evas_obj_map_get(map) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_MAP_GET), EO_TYPECHECK(const Evas_Map **, map)
/**
 * @}
 */

/**
 * @ingroup Evas_Smart_Object_Group
 *
 * @{
 */

/**
 * @def evas_obj_smart_parent_get
 * @since 1.8
 *
 * Gets the parent smart object of a given Evas object, if it has one.
 *
 * @param[out] smart_parent out
 *
 * @see evas_object_smart_parent_get
 */
#define evas_obj_smart_parent_get(smart_parent) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_SMART_PARENT_GET), EO_TYPECHECK(Evas_Object **, smart_parent)
/**
 * @def evas_obj_smart_data_get
 * @since 1.8
 *
 * Retrieve user data stored on a given smart object.
 *
 * @param[out] data out
 *
 * @see evas_object_smart_data_get
 */
#define evas_obj_smart_data_get(data) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_SMART_DATA_GET), EO_TYPECHECK(void **, data)

/**
 * @def evas_obj_smart_type_check
 * @since 1.8
 *
 * Checks whether a given smart object or any of its smart object
 * parents is of a given smart class.
 *
 * @param[in] type in
 * @param[out] type_check out
 *
 * @see evas_object_smart_type_check
 */
#define evas_obj_smart_type_check(type, type_check) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_SMART_TYPE_CHECK), EO_TYPECHECK(const char *, type), EO_TYPECHECK(Eina_Bool *, type_check)

/**
 * @def evas_obj_smart_type_check_ptr
 * @since 1.8
 *
 * Checks whether a given smart object or any of its smart object
 * parents is of a given smart class, <b>using pointer comparison</b>.
 *
 * @param[in] type in
 * @param[out] type_check out
 *
 * @see evas_object_smart_type_check_ptr
 */
#define evas_obj_smart_type_check_ptr(type, type_check) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_SMART_TYPE_CHECK_PTR), EO_TYPECHECK(const char *, type), EO_TYPECHECK(Eina_Bool *, type_check)

/**
 * @def evas_obj_smart_move_children_relative
 * @since 1.8
 *
 * Moves all children objects of a given smart object relative to a
 * given offset.
 *
 * @param[in] dx in
 * @param[in] dy in
 *
 * @see evas_object_smart_move_children_relative
 */
#define evas_obj_smart_move_children_relative(dx, dy) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_SMART_MOVE_CHILDREN_RELATIVE), EO_TYPECHECK(Evas_Coord, dx), EO_TYPECHECK(Evas_Coord, dy)
/**
 * @}
 */

/**
 * @ingroup Evas_Smart_Object_Clipped
 *
 * @{
 */
/**
 * @def evas_obj_smart_clipped_clipper_get
 * @since 1.8
 *
 * Get the clipper object for the given clipped smart object.
 *
 * @param[out] ret out
 *
 * @see evas_object_smart_clipped_clipper_get
 */
#define evas_obj_smart_clipped_clipper_get(ret) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_SMART_CLIPPED_CLIPPER_GET), EO_TYPECHECK(Evas_Object **, ret)
/**
 * @}
 */

#define EVAS_OBJ_CLASS evas_object_class_get()

const Eo_Class *evas_object_class_get(void) EINA_CONST;

#define EVAS_OBJ_IMAGE_CLASS evas_object_image_class_get()
const Eo_Class *evas_object_image_class_get(void) EINA_CONST;

extern EAPI Eo_Op EVAS_OBJ_IMAGE_BASE_ID;

enum
{
   EVAS_OBJ_IMAGE_SUB_ID_FILE_SET,
   EVAS_OBJ_IMAGE_SUB_ID_MMAP_SET,
   EVAS_OBJ_IMAGE_SUB_ID_FILE_GET,
   EVAS_OBJ_IMAGE_SUB_ID_SOURCE_SET,
   EVAS_OBJ_IMAGE_SUB_ID_SOURCE_GET,
   EVAS_OBJ_IMAGE_SUB_ID_BORDER_SET,
   EVAS_OBJ_IMAGE_SUB_ID_BORDER_GET,
   EVAS_OBJ_IMAGE_SUB_ID_BORDER_CENTER_FILL_SET,
   EVAS_OBJ_IMAGE_SUB_ID_BORDER_CENTER_FILL_GET,
   EVAS_OBJ_IMAGE_SUB_ID_FILLED_SET,
   EVAS_OBJ_IMAGE_SUB_ID_FILLED_GET,
   EVAS_OBJ_IMAGE_SUB_ID_BORDER_SCALE_SET,
   EVAS_OBJ_IMAGE_SUB_ID_BORDER_SCALE_GET,
   EVAS_OBJ_IMAGE_SUB_ID_FILL_SET,
   EVAS_OBJ_IMAGE_SUB_ID_FILL_GET,
   EVAS_OBJ_IMAGE_SUB_ID_FILL_SPREAD_SET,
   EVAS_OBJ_IMAGE_SUB_ID_FILL_SPREAD_GET,
   EVAS_OBJ_IMAGE_SUB_ID_SIZE_SET,
   EVAS_OBJ_IMAGE_SUB_ID_SIZE_GET,
   EVAS_OBJ_IMAGE_SUB_ID_STRIDE_GET,
   EVAS_OBJ_IMAGE_SUB_ID_LOAD_ERROR_GET,
   EVAS_OBJ_IMAGE_SUB_ID_DATA_CONVERT,
   EVAS_OBJ_IMAGE_SUB_ID_DATA_SET,
   EVAS_OBJ_IMAGE_SUB_ID_DATA_GET,
   EVAS_OBJ_IMAGE_SUB_ID_PRELOAD_BEGIN,
   EVAS_OBJ_IMAGE_SUB_ID_PRELOAD_CANCEL,
   EVAS_OBJ_IMAGE_SUB_ID_DATA_COPY_SET,
   EVAS_OBJ_IMAGE_SUB_ID_DATA_UPDATE_ADD,
   EVAS_OBJ_IMAGE_SUB_ID_ALPHA_SET,
   EVAS_OBJ_IMAGE_SUB_ID_ALPHA_GET,
   EVAS_OBJ_IMAGE_SUB_ID_SMOOTH_SCALE_SET,
   EVAS_OBJ_IMAGE_SUB_ID_SMOOTH_SCALE_GET,
   EVAS_OBJ_IMAGE_SUB_ID_RELOAD,
   EVAS_OBJ_IMAGE_SUB_ID_SAVE,
   EVAS_OBJ_IMAGE_SUB_ID_PIXELS_IMPORT,
   EVAS_OBJ_IMAGE_SUB_ID_PIXELS_GET_CALLBACK_SET,
   EVAS_OBJ_IMAGE_SUB_ID_PIXELS_DIRTY_SET,
   EVAS_OBJ_IMAGE_SUB_ID_PIXELS_DIRTY_GET,
   EVAS_OBJ_IMAGE_SUB_ID_LOAD_DPI_SET,
   EVAS_OBJ_IMAGE_SUB_ID_LOAD_DPI_GET,
   EVAS_OBJ_IMAGE_SUB_ID_LOAD_SIZE_SET,
   EVAS_OBJ_IMAGE_SUB_ID_LOAD_SIZE_GET,
   EVAS_OBJ_IMAGE_SUB_ID_LOAD_SCALE_DOWN_SET,
   EVAS_OBJ_IMAGE_SUB_ID_LOAD_SCALE_DOWN_GET,
   EVAS_OBJ_IMAGE_SUB_ID_LOAD_REGION_SET,
   EVAS_OBJ_IMAGE_SUB_ID_LOAD_REGION_GET,
   EVAS_OBJ_IMAGE_SUB_ID_LOAD_ORIENTATION_SET,
   EVAS_OBJ_IMAGE_SUB_ID_LOAD_ORIENTATION_GET,
   EVAS_OBJ_IMAGE_SUB_ID_COLORSPACE_SET,
   EVAS_OBJ_IMAGE_SUB_ID_COLORSPACE_GET,
   EVAS_OBJ_IMAGE_SUB_ID_VIDEO_SURFACE_SET,
   EVAS_OBJ_IMAGE_SUB_ID_VIDEO_SURFACE_GET,
   EVAS_OBJ_IMAGE_SUB_ID_VIDEO_SURFACE_CAPS_SET,
   EVAS_OBJ_IMAGE_SUB_ID_VIDEO_SURFACE_CAPS_GET,
   EVAS_OBJ_IMAGE_SUB_ID_NATIVE_SURFACE_SET,
   EVAS_OBJ_IMAGE_SUB_ID_NATIVE_SURFACE_GET,
   EVAS_OBJ_IMAGE_SUB_ID_SCALE_HINT_SET,
   EVAS_OBJ_IMAGE_SUB_ID_SCALE_HINT_GET,
   EVAS_OBJ_IMAGE_SUB_ID_CONTENT_HINT_SET,
   EVAS_OBJ_IMAGE_SUB_ID_CONTENT_HINT_GET,
   EVAS_OBJ_IMAGE_SUB_ID_REGION_SUPPORT_GET,
   EVAS_OBJ_IMAGE_SUB_ID_ANIMATED_GET,
   EVAS_OBJ_IMAGE_SUB_ID_ANIMATED_FRAME_COUNT_GET,
   EVAS_OBJ_IMAGE_SUB_ID_ANIMATED_LOOP_TYPE_GET,
   EVAS_OBJ_IMAGE_SUB_ID_ANIMATED_LOOP_COUNT_GET,
   EVAS_OBJ_IMAGE_SUB_ID_ANIMATED_FRAME_DURATION_GET,
   EVAS_OBJ_IMAGE_SUB_ID_ANIMATED_FRAME_SET,
   EVAS_OBJ_IMAGE_SUB_ID_SOURCE_VISIBLE_SET,
   EVAS_OBJ_IMAGE_SUB_ID_SOURCE_VISIBLE_GET,
   EVAS_OBJ_IMAGE_SUB_ID_SOURCE_EVENTS_SET,
   EVAS_OBJ_IMAGE_SUB_ID_SOURCE_EVENTS_GET,
   EVAS_OBJ_IMAGE_SUB_ID_SOURCE_CLIP_SET,
   EVAS_OBJ_IMAGE_SUB_ID_SOURCE_CLIP_GET,
   EVAS_OBJ_IMAGE_SUB_ID_LAST
};

#define EVAS_OBJ_IMAGE_ID(sub_id) (EVAS_OBJ_IMAGE_BASE_ID + sub_id)

/**
 * @ingroup Evas_Object_Image
 *
 * @{
 */

/**
 * @def evas_obj_image_file_set
 * @since 1.8
 *
 * Set the source file from where an image object must fetch the real
 * image data (it may be an Eet file, besides pure image ones).
 *
 * @param[in] file The image file path.
 * @param[in] key The image key in @p file (if its an Eet one), or @c
 *
 * @see evas_object_image_file_set
 */
#define evas_obj_image_file_set(file, key) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_FILE_SET), EO_TYPECHECK(const char *, file), EO_TYPECHECK(const char*, key)

/**
 * @def evas_obj_image_mmap_set
 * @since 1.8
 *
 * Set the source mmaped file from where an image object must fetch the real
 * image data (it may be any Eina_File).
 *
 * @param[in] f in
 * @param[in] key in
 *
 * @see evas_obj_image_file_set
 */
#define evas_obj_image_mmap_set(f, key) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_MMAP_SET), EO_TYPECHECK(const Eina_File *, f), EO_TYPECHECK(const char*, key)

/**
 * @def evas_obj_image_file_get
 * @since 1.8
 *
 * Retrieve the source file from where an image object is to fetch the
 * real image data (it may be an Eet file, besides pure image ones).
 *
 * @param[out] file Location to store the image file path.
 * @param[out] key Location to store the image key (if @p file is an Eet
 * one).
 *
 * You must @b not modify the strings on the returned pointers.
 *
 * @note Use @c NULL pointers on the file components you're not
 * interested in: they'll be ignored by the function.
 *
 * @see evas_object_image_file_get
 */
#define evas_obj_image_file_get(file, key) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_FILE_GET), EO_TYPECHECK(const char **, file), EO_TYPECHECK(const char **, key)

/**
 * @def evas_obj_image_source_set
 * @since 1.8
 *
 * Set the source object on an image object to used as a @b proxy.
 *
 * @param[in] src in
 * @param[out] result out
 *
 * @see evas_object_image_source_set
 */
#define evas_obj_image_source_set(src, result) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_SOURCE_SET), EO_TYPECHECK(Evas_Object *, src), EO_TYPECHECK(Eina_Bool *, result)

/**
 * @def evas_obj_image_source_get
 * @since 1.8
 *
 * Get the current source object of an image object.
 *
 * @param[out] src out
 *
 * @see evas_object_image_source_get
 */
#define evas_obj_image_source_get(src) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_SOURCE_GET), EO_TYPECHECK(Evas_Object **, src)

/**
 * @def evas_obj_image_source_visible_set
 * @since 1.8
 *
 * Set the source object to be shown or hidden.
 *
 * @param[in] visible in
 *
 * @see evas_object_image_source_visible_set
 */
#define evas_obj_image_source_visible_set(visible) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_SOURCE_VISIBLE_SET), EO_TYPECHECK(Eina_Bool, visible)

/**
 * @def evas_obj_image_source_visible_get
 * @since 1.8
 *
 * Get the state of the source object visibility.
 *
 * @param[out] visible out
 *
 * @see evas_object_image_source_visible_get
 */
#define evas_obj_image_source_visible_get(visible) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_SOURCE_VISIBLE_GET), EO_TYPECHECK(Eina_Bool *, visible)

/**
 * @def evas_obj_image_source_events_set
 * @since 1.8
 *
 * Set events to be repeated to the source object.
 *
 * @param[in] source in
 *
 * @see evas_object_image_source_events_set
 */
#define evas_obj_image_source_events_set(source) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_SOURCE_EVENTS_SET), EO_TYPECHECK(Eina_Bool, source)

/**
 * @def evas_obj_image_source_events_get
 * @since 1.8
 *
 * Get the state of the source event.
 *
 * @param[out] source out
 *
 * @see evas_object_image_source_event_get
 */
#define evas_obj_image_source_events_get(source) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_SOURCE_EVENTS_GET), EO_TYPECHECK(Eina_Bool *, source)

/**
 * @def evas_obj_image_source_clip_set
 * @since 1.8
 *
 * Apply the source object's clip to the proxy
 *
 * @param[in] source_clip in
 *
 * @see evas_object_image_source_clip_set
 */
#define evas_obj_image_source_clip_set(source_clip) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_SOURCE_CLIP_SET), EO_TYPECHECK(Eina_Bool, source_clip)

/**
 * @def evas_obj_image_source_clip_get
 * @since 1.8
 *
 * Get the state of the source clip
 *
 * @param[out] source_clip out
 *
 * @see evas_object_image_source_clip_get
 */
#define evas_obj_image_source_clip_get(source_clip) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_SOURCE_CLIP_GET), EO_TYPECHECK(Eina_Bool *, source_clip)

/**
 * @def evas_obj_image_border_set
 * @since 1.8
 *
 * Set the dimensions for an image object's border, a region which @b
 * won't ever be scaled together with its center.
 *
 * @param[in] l in
 * @param[in] r in
 * @param[in] t in
 * @param[in] b in
 *
 * @see evas_object_image_border_set
 */
#define evas_obj_image_border_set(l, r, t, b) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_BORDER_SET), EO_TYPECHECK(int, l), EO_TYPECHECK(int, r), EO_TYPECHECK(int, t), EO_TYPECHECK(int, b)

/**
 * @def evas_obj_image_border_get
 * @since 1.8
 *
 * Retrieve the dimensions for an image object's border, a region
 * which @b won't ever be scaled together with its center.
 *
 * @param[out] l in
 * @param[out] r in
 * @param[out] t in
 * @param[out] b in
 *
 * @see evas_object_image_border_get
 */
#define evas_obj_image_border_get(l, r, t, b) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_BORDER_GET), EO_TYPECHECK(int *, l), EO_TYPECHECK(int *, r), EO_TYPECHECK(int *, t), EO_TYPECHECK(int *, b)

/**
 * @def evas_obj_image_border_center_fill_set
 * @since 1.8
 *
 * Sets @b how the center part of the given image object (not the
 * borders) should be drawn when Evas is rendering it.
 *
 * @param[in] fill in
 *
 * @see evas_object_image_border_center_fill_set
 */
#define evas_obj_image_border_center_fill_set(fill) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_BORDER_CENTER_FILL_SET), EO_TYPECHECK(Evas_Border_Fill_Mode, fill)

/**
 * @def evas_obj_image_border_center_fill_get
 * @since 1.8
 *
 * Retrieves @b how the center part of the given image object (not the
 * borders) is to be drawn when Evas is rendering it.
 *
 * @param[out] fill out
 *
 * @see evas_object_image_border_center_fill_get
 */
#define evas_obj_image_border_center_fill_get(fill) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_BORDER_CENTER_FILL_GET), EO_TYPECHECK(Evas_Border_Fill_Mode *, fill)

/**
 * @def evas_obj_image_filled_set
 * @since 1.8
 *
 * Set whether the image object's fill property should track the
 * object's size.
 *
 * @param[in] filled in
 *
 * @see evas_object_image_filled_set
 */
#define evas_obj_image_filled_set(filled) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_FILLED_SET), EO_TYPECHECK(Eina_Bool, filled)

/**
 * @def evas_obj_image_filled_get
 * @since 1.8
 *
 * Retrieve whether the image object's fill property should track the
 * object's size.
 *
 * @param[out] filled out
 *
 * @see evas_object_image_filled_get
 */
#define evas_obj_image_filled_get(filled) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_FILLED_GET), EO_TYPECHECK(Eina_Bool *, filled)

/**
 * @def evas_obj_image_border_scale_set
 * @since 1.8
 *
 * Sets the scaling factor (multiplier) for the borders of an image
 * object.
 *
 * @param[in] scale in
 *
 * @see evas_object_image_border_scale_set
 */
#define evas_obj_image_border_scale_set(scale) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_BORDER_SCALE_SET), EO_TYPECHECK(double, scale)

/**
 * @def evas_obj_image_border_scale_get
 * @since 1.8
 *
 * Retrieves the scaling factor (multiplier) for the borders of an
 * image object.
 *
 * @param[out] scale out
 *
 * @see evas_object_image_border_scale_get
 */
#define evas_obj_image_border_scale_get(scale) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_BORDER_SCALE_GET), EO_TYPECHECK(double *, scale)

/**
 * @def evas_obj_image_fill_set
 * @since 1.8
 *
 * Set how to fill an image object's drawing rectangle given the
 * (real) image bound to it.
 *
 * @param[in] x in
 * @param[in] y in
 * @param[in] w in
 * @param[in] h in
 *
 * @see evas_object_image_fill_set
 */
#define evas_obj_image_fill_set(x, y, w, h) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_FILL_SET), EO_TYPECHECK(Evas_Coord, x), EO_TYPECHECK(Evas_Coord, y), EO_TYPECHECK(Evas_Coord, w), EO_TYPECHECK(Evas_Coord, h)

/**
 * @def evas_obj_image_fill_get
 * @since 1.8
 *
 * Retrieve how an image object is to fill its drawing rectangle,
 * given the (real) image bound to it.
 *
 * @param[out] x out
 * @param[out] y out
 * @param[out] w out
 * @param[out] h out
 *
 * @see evas_object_image_fill_get
 */
#define evas_obj_image_fill_get(x, y, w, h) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_FILL_GET), EO_TYPECHECK(Evas_Coord *, x), EO_TYPECHECK(Evas_Coord *, y), EO_TYPECHECK(Evas_Coord *, w), EO_TYPECHECK(Evas_Coord *, h)

/**
 * @def evas_obj_image_fill_spread_set
 * @since 1.8
 *
 * Sets the tiling mode for the given evas image object's fill.
 *
 * @param[in] spread in
 *
 * @see evas_object_image_fill_spread_set
 */
#define evas_obj_image_fill_spread_set(spread) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_FILL_SPREAD_SET), EO_TYPECHECK(Evas_Fill_Spread, spread)

/**
 * @def evas_obj_image_fill_spread_get
 * @since 1.8
 *
 * Retrieves the spread (tiling mode) for the given image object's
 * fill.
 *
 * @param[out] spread out
 *
 * @see evas_object_image_fill_spread_get
 */
#define evas_obj_image_fill_spread_get(spread) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_FILL_SPREAD_GET), EO_TYPECHECK(Evas_Fill_Spread *, spread)

/**
 * @def evas_obj_image_size_set
 * @since 1.8
 *
 * Sets the size of the given image object.
 *
 * @param[in] w in
 * @param[in] h in
 *
 * @see evas_object_image_size_set
 */
#define evas_obj_image_size_set(w, h) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_SIZE_SET), EO_TYPECHECK(int, w), EO_TYPECHECK(int, h)

/**
 * @def evas_obj_image_size_get
 * @since 1.8
 *
 * Retrieves the size of the given image object.
 *
 * @param[out] w out
 * @param[out] h out
 *
 * @see evas_object_image_size_get
 */
#define evas_obj_image_size_get(w, h) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_SIZE_GET), EO_TYPECHECK(int *, w), EO_TYPECHECK(int *, h)

/**
 * @def evas_obj_image_stride_get
 * @since 1.8
 *
 * Retrieves the row stride of the given image object.
 *
 * @param[out] stride out
 *
 * @see evas_object_image_stride_get
 */
#define evas_obj_image_stride_get(stride) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_STRIDE_GET), EO_TYPECHECK(int *, stride)

/**
 * @def evas_obj_image_load_error_get
 * @since 1.8
 *
 * Retrieves a number representing any error that occurred during the
 * last loading of the given image object's source image.
 *
 * @param[out] load_error
 *
 * @see evas_object_image_load_error_get
 */
#define evas_obj_image_load_error_get(load_error) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_LOAD_ERROR_GET), EO_TYPECHECK(Evas_Load_Error *, load_error)

/**
 * @def evas_obj_image_data_convert
 * @since 1.8
 *
 * Converts the raw image data of the given image object to the
 * specified colorspace.
 *
 * @param[in] to_cspace in
 * @param[out] data out
 *
 * @see evas_object_image_data_convert
 */
#define evas_obj_image_data_convert(to_cspace, data) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_DATA_CONVERT), EO_TYPECHECK(Evas_Colorspace, to_cspace), EO_TYPECHECK(void **, data)

/**
 * @def evas_obj_image_data_set
 * @since 1.8
 *
 * Sets the raw image data of the given image object.
 *
 * @param[in] data in
 *
 * @see evas_object_image_data_set
 */
#define evas_obj_image_data_set(data) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_DATA_SET), EO_TYPECHECK(void *, data)

/**
 * @def evas_obj_image_data_get
 * @since 1.8
 *
 * Get a pointer to the raw image data of the given image object.
 *
 * @param[in] for_writing in
 * @param[out] data out
 *
 * @see evas_object_image_data_get
 */
#define evas_obj_image_data_get(for_writing, data) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_DATA_GET), EO_TYPECHECK(Eina_Bool, for_writing), EO_TYPECHECK(void **, data)

/**
 * @def evas_obj_image_data_copy_set
 * @since 1.8
 *
 * Replaces the raw image data of the given image object.
 *
 * @param[in] data in
 *
 * @see evas_object_image_data_copy_set
 */
#define evas_obj_image_data_copy_set(data) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_DATA_COPY_SET), EO_TYPECHECK(void *, data)

/**
 * @def evas_obj_image_data_update_add
 * @since 1.8
 *
 * Mark a sub-region of the given image object to be redrawn.
 *
 * @param[in] x in
 * @param[in] y in
 * @param[in] w in
 * @param[in] h in
 *
 * @see evas_object_image_data_update_add
 */
#define evas_obj_image_data_update_add(x, y, w, h) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_DATA_UPDATE_ADD), EO_TYPECHECK(int, x), EO_TYPECHECK(int, y), EO_TYPECHECK(int, w), EO_TYPECHECK(int, h)

/**
 * @def evas_obj_image_alpha_set
 * @since 1.8
 *
 * Enable or disable alpha channel usage on the given image object.
 *
 * @param[in] alpha in
 *
 * @see evas_object_image_alpha_set
 */
#define evas_obj_image_alpha_set(alpha) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_ALPHA_SET), EO_TYPECHECK(Eina_Bool, alpha)

/**
 * @def evas_obj_image_alpha_get
 * @since 1.8
 *
 * Retrieve whether alpha channel data is being used on the given
 * image object.
 *
 * @param[out] alpha out
 *
 * @see evas_object_image_alpha_get
 */
#define evas_obj_image_alpha_get(alpha) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_ALPHA_GET), EO_TYPECHECK(Eina_Bool *, alpha)

/**
 * @def evas_obj_image_smooth_scale_set
 * @since 1.8
 *
 * Sets whether to use high-quality image scaling algorithm on the
 * given image object.
 *
 * @param[in] smooth_scale in
 *
 * @see evas_object_image_smooth_scale_set
 */
#define evas_obj_image_smooth_scale_set(smooth_scale) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_SMOOTH_SCALE_SET), EO_TYPECHECK(Eina_Bool, smooth_scale)

/**
 * @def evas_obj_image_smooth_scale_get
 * @since 1.8
 *
 * Retrieves whether the given image object is using high-quality
 * image scaling algorithm.
 *
 * @param[out] smooth_scale out
 *
 * @see evas_object_image_smooth_scale_get
 */
#define evas_obj_image_smooth_scale_get(smooth_scale) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_SMOOTH_SCALE_GET), EO_TYPECHECK(Eina_Bool *, smooth_scale)

/**
 * @def evas_obj_image_preload_begin
 * @since 1.8
 *
 * Begin preloading an image object's image data in the background
 *
 * @see evas_object_image_preload
 */
#define evas_obj_image_preload_begin() EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_PRELOAD_BEGIN)

/**
 * @def evas_obj_image_preload_cancel
 * @since 1.8
 *
 * Cancel preloading an image object's image data in the background
 *
 * @see evas_object_image_preload
 */
#define evas_obj_image_preload_cancel() EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_PRELOAD_CANCEL)

/**
 * @def evas_obj_image_reload
 * @since 1.8
 *
 * Reload an image object's image data.
 *
 * @see evas_object_image_reload
 */
#define evas_obj_image_reload() EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_RELOAD)

/**
 * @def evas_obj_image_save
 * @since 1.8
 *
 * Save the given image object's contents to an (image) file.
 *
 * @param[in] file in
 * @param[in] key in
 * @param[in] flags in
 * @param[out] result out
 *
 * @see evas_object_image_save
 */
#define evas_obj_image_save(file, key, flags, result) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_SAVE), EO_TYPECHECK(const char *, file), EO_TYPECHECK(const char *, key), EO_TYPECHECK(const char *, flags), EO_TYPECHECK(Eina_Bool *, result)

/**
 * @def evas_obj_image_pixels_import
 * @since 1.8
 *
 * Import pixels from given source to a given canvas image object.
 *
 * @param[in] pixels in
 * @param[out] result out
 *
 * @see evas_object_image_pixels_import
 */
#define evas_obj_image_pixels_import(pixels, result) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_PIXELS_IMPORT), EO_TYPECHECK(Evas_Pixel_Import_Source *, pixels), EO_TYPECHECK(Eina_Bool *, result)

/**
 * @def evas_obj_image_pixels_get_callback_set
 * @since 1.8
 *
 * Set the callback function to get pixels from a canvas' image.
 *
 * @param[in] func in
 * @param[in] data in
 *
 * @see evas_object_image_pixels_get_callback_set
 */
#define evas_obj_image_pixels_get_callback_set(func, data) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_PIXELS_GET_CALLBACK_SET), EO_TYPECHECK(Evas_Object_Image_Pixels_Get_Cb, func), EO_TYPECHECK(void *, data)

/**
 * @def evas_obj_image_pixels_dirty_set
 * @since 1.8
 *
 * Mark whether the given image object is dirty (needs to be redrawn).
 *
 * @param[in] dirty in
 *
 * @see evas_object_image_pixels_get_callback_set
 */
#define evas_obj_image_pixels_dirty_set(dirty) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_PIXELS_DIRTY_SET), EO_TYPECHECK(Eina_Bool, dirty)

/**
 * @def evas_obj_image_pixels_dirty_get
 * @since 1.8
 *
 * Retrieves whether the given image object is dirty (needs to be redrawn).
 *
 * @param[out] dirty out
 *
 * @see evas_object_image_pixels_dirty_get
 */
#define evas_obj_image_pixels_dirty_get(dirty) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_PIXELS_DIRTY_GET), EO_TYPECHECK(Eina_Bool *, dirty)

/**
 * @def evas_obj_image_load_dpi_set
 * @since 1.8
 *
 * Set the DPI resolution of an image object's source image.
 *
 * @param[in] dpi in
 *
 * @see evas_object_image_load_dpi_set
 */
#define evas_obj_image_load_dpi_set(dpi) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_LOAD_DPI_SET), EO_TYPECHECK(double, dpi)

/**
 * @def evas_obj_image_load_dpi_get
 * @since 1.8
 *
 * Get the DPI resolution of a loaded image object in the canvas.
 *
 * @param[out] dpi out
 *
 * @see evas_object_image_load_dpi_get
 */
#define evas_obj_image_load_dpi_get(dpi) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_LOAD_DPI_GET), EO_TYPECHECK(double *, dpi)

/**
 * @def evas_obj_image_load_size_set
 * @since 1.8
 *
 * Set the size of a given image object's source image, when loading
 * it.
 *
 * @param[in] w in
 * @param[in] h in
 *
 * @see evas_object_image_load_size_set
 */
#define evas_obj_image_load_size_set(w, h) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_LOAD_SIZE_SET), EO_TYPECHECK(int, w), EO_TYPECHECK(int, h)

/**
 * @def evas_obj_image_load_size_get
 * @since 1.8
 *
 * Get the size of a given image object's source image, when loading
 * it.
 *
 * @param[out] w out
 * @param[out] h out
 *
 * @see evas_object_image_load_size_get
 */
#define evas_obj_image_load_size_get(w, h) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_LOAD_SIZE_GET), EO_TYPECHECK(int *, w), EO_TYPECHECK(int *, h)

/**
 * @def evas_obj_image_load_scale_down_set
 * @since 1.8
 *
 * Set the scale down factor of a given image object's source image,
 * when loading it.
 *
 * @param[in] scale_down in
 *
 * @see evas_object_image_load_scale_down_set
 */
#define evas_obj_image_load_scale_down_set(scale_down) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_LOAD_SCALE_DOWN_SET), EO_TYPECHECK(int, scale_down)

/**
 * @def evas_obj_image_load_scale_down_get
 * @since 1.8
 *
 * Get the scale down factor of a given image object's source image,
 * when loading it.
 *
 * @param[out] scale_down out
 *
 * @see evas_object_image_load_scale_down_get
 */
#define evas_obj_image_load_scale_down_get(scale_down) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_LOAD_SCALE_DOWN_GET), EO_TYPECHECK(int *, scale_down)

/**
 * @def evas_obj_image_load_region_set
 * @since 1.8
 *
 * Inform a given image object to load a selective region of its
 * source image.
 *
 * @param[in] x in
 * @param[in] y in
 * @param[in] w in
 * @param[in] h in
 *
 * @see evas_object_image_load_region_set
 */
#define evas_obj_image_load_region_set(x, y, w, h) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_LOAD_REGION_SET), EO_TYPECHECK(int, x), EO_TYPECHECK(int, y), EO_TYPECHECK(int, w), EO_TYPECHECK(int, h)

/**
 * @def evas_obj_image_load_region_get
 * @since 1.8
 *
 * Retrieve the coordinates of a given image object's selective
 * (source image) load region.
 *
 * @param[out] x out
 * @param[out] y out
 * @param[out] w out
 * @param[out] h out
 *
 * @see evas_object_image_load_region_get
 */
#define evas_obj_image_load_region_get(x, y, w, h) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_LOAD_REGION_GET), EO_TYPECHECK(int *, x), EO_TYPECHECK(int *, y), EO_TYPECHECK(int *, w), EO_TYPECHECK(int *, h)

/**
 * @def evas_obj_image_load_orientation_set
 * @since 1.8
 *
 * Define if the orientation information in the image file should be honored.
 *
 * @param[in] enable in
 *
 * @see evas_object_image_load_orientation_set
 */
#define evas_obj_image_load_orientation_set(enable) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_LOAD_ORIENTATION_SET), EO_TYPECHECK(Eina_Bool, enable)

/**
 * @def evas_obj_image_load_orientation_get
 * @since 1.8
 *
 * Get if the orientation information in the image file should be honored.
 *
 * @param[out] enable out
 *
 * @see evas_object_image_load_orientation_get
 */
#define evas_obj_image_load_orientation_get(enable) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_LOAD_ORIENTATION_GET), EO_TYPECHECK(Eina_Bool *, enable)

/**
 * @def evas_obj_image_colorspace_set
 * @since 1.8
 *
 * Set the colorspace of a given image of the canvas.
 *
 * @param[in] cspace in
 *
 * @see evas_object_image_colorspace_set
 */
#define evas_obj_image_colorspace_set(cspace) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_COLORSPACE_SET), EO_TYPECHECK(Evas_Colorspace, cspace)

/**
 * @def evas_obj_image_colorspace_get
 * @since 1.8
 *
 * Get the colorspace of a given image of the canvas.
 *
 * @param[out] cspace out
 *
 * @see evas_object_image_colorspace_get
 */
#define evas_obj_image_colorspace_get(cspace) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_COLORSPACE_GET), EO_TYPECHECK(Evas_Colorspace *, cspace)

/**
 * @def evas_obj_image_video_surface_set
 * @since 1.8
 *
 * Set the video surface linked to a given image of the canvas
 *
 * @param[in] surf in
 *
 * @see evas_object_image_video_surface_set
 */
#define evas_obj_image_video_surface_set(surf) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_VIDEO_SURFACE_SET), EO_TYPECHECK(Evas_Video_Surface *, surf)

/**
 * @def evas_obj_image_video_surface_get
 * @since 1.8
 *
 * Get the video surface linked to a given image of the canvas
 *
 * @param[out] surf out
 *
 * @see evas_object_image_video_surface_get
 */
#define evas_obj_image_video_surface_get(surf) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_VIDEO_SURFACE_GET), EO_TYPECHECK(const Evas_Video_Surface **, surf)

/**
 * @def evas_obj_image_video_surface_caps_set
 * @since 1.8
 *
 * Set the video surface capabilities to a given image of the canvas
 *
 * @param[in] caps in
 *
 * @see evas_object_image_video_surface_caps_set
 */
#define evas_obj_image_video_surface_caps_set(caps) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_VIDEO_SURFACE_CAPS_SET), EO_TYPECHECK(unsigned int, caps)

/**
 * @def evas_obj_image_video_surface_caps_get
 * @since 1.8
 *
 * Get the video surface capabilities to a given image of the canvas
 *
 * @param[out] caps out
 *
 * @see evas_object_image_video_surface_caps_get
 */
#define evas_obj_image_video_surface_caps_get(caps) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_VIDEO_SURFACE_CAPS_GET), EO_TYPECHECK(unsigned int *, caps)

/**
 * @def evas_obj_image_native_surface_set
 * @since 1.8
 *
 * Set the native surface of a given image of the canvas
 *
 * @param[in] surf in
 *
 * @see evas_object_image_native_surface_set
 */
#define evas_obj_image_native_surface_set(surf) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_NATIVE_SURFACE_SET), EO_TYPECHECK(Evas_Native_Surface *, surf)

/**
 * @def evas_obj_image_native_surface_get
 * @since 1.8
 *
 * Get the native surface of a given image of the canvas
 *
 * @param[out] surf out
 *
 * @see evas_object_image_native_surface_get
 */
#define evas_obj_image_native_surface_get(surf) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_NATIVE_SURFACE_GET), EO_TYPECHECK(Evas_Native_Surface **, surf)

/**
 * @def evas_obj_image_scale_hint_set
 * @since 1.8
 *
 * Set the scale hint of a given image of the canvas.
 *
 * @param[in] hint in
 *
 * @see evas_object_image_scale_hint_set
 */
#define evas_obj_image_scale_hint_set(hint) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_SCALE_HINT_SET), EO_TYPECHECK(Evas_Image_Scale_Hint, hint)

/**
 * @def evas_obj_image_scale_hint_get
 * @since 1.8
 *
 * Get the scale hint of a given image of the canvas.
 *
 * @param[out] hint out
 *
 * @see evas_object_image_scale_hint_get
 */
#define evas_obj_image_scale_hint_get(hint) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_SCALE_HINT_GET), EO_TYPECHECK(Evas_Image_Scale_Hint *, hint)

/**
 * @def evas_obj_image_content_hint_set
 * @since 1.8
 *
 * Set the content hint setting of a given image object of the canvas.
 *
 * @param[in] hint in
 *
 * @see evas_object_image_content_hint_set
 */
#define evas_obj_image_content_hint_set(hint) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_CONTENT_HINT_SET), EO_TYPECHECK(Evas_Image_Content_Hint, hint)

/**
 * @def evas_obj_image_content_hint_get
 * @since 1.8
 *
 * Get the content hint setting of a given image object of the canvas.
 *
 * @param[out] hint out
 *
 * @see evas_object_image_content_hint_get
 */
#define evas_obj_image_content_hint_get(hint) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_CONTENT_HINT_GET), EO_TYPECHECK(Evas_Image_Content_Hint *, hint)

/**
 * @def evas_obj_image_region_support_get
 * @since 1.8
 *
 * Get the support state of a given image
 *
 * @param[out] region out
 *
 * @see evas_object_image_region_support_get
 */
#define evas_obj_image_region_support_get(region) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_REGION_SUPPORT_GET), EO_TYPECHECK(Eina_Bool *, region)

/**
 * @def evas_obj_image_animated_get
 * @since 1.8
 *
 * Check if an image object can be animated (have multiple frames)
 *
 * @param[out] animated out
 *
 * @see evas_object_image_animated_get
 */
#define evas_obj_image_animated_get(animated) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_ANIMATED_GET), EO_TYPECHECK(Eina_Bool *, animated)

/**
 * @def evas_obj_image_animated_frame_count_get
 * @since 1.8
 *
 * Get the total number of frames of the image object.
 *
 * @param[out] frame_count out
 *
 * @see evas_object_image_animated_frame_count_get
 */
#define evas_obj_image_animated_frame_count_get(frame_count) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_ANIMATED_FRAME_COUNT_GET), EO_TYPECHECK(int *, frame_count)

/**
 * @def evas_obj_image_animated_loop_type_get
 * @since 1.8
 *
 * Get the kind of looping the image object does.
 *
 * @param[out] hint out
 *
 * @see evas_object_image_animated_loop_type_get
 */
#define evas_obj_image_animated_loop_type_get(hint) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_ANIMATED_LOOP_TYPE_GET), EO_TYPECHECK(Evas_Image_Animated_Loop_Hint *, hint)

/**
 * @def evas_obj_image_animated_loop_count_get
 * @since 1.8
 *
 * Get the number times the animation of the object loops.
 *
 * @param[out] loop_count out
 *
 * @see evas_object_image_animated_loop_count_get
 */
#define evas_obj_image_animated_loop_count_get(loop_count) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_ANIMATED_LOOP_COUNT_GET), EO_TYPECHECK(int *, loop_count)

/**
 * @def evas_obj_image_animated_frame_duration_get
 * @since 1.8
 *
 * Get the duration of a sequence of frames.
 *
 * @param[in] start_frame in
 * @param[in] frame_num in
 * @param[out] frame_duration out
 *
 * @see evas_object_image_animated_frame_duration_get
 */
#define evas_obj_image_animated_frame_duration_get(start_frame, frame_num, frame_duration) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_ANIMATED_FRAME_DURATION_GET), EO_TYPECHECK(int, start_frame), EO_TYPECHECK(int, frame_num), EO_TYPECHECK(double *, frame_duration)

/**
 * @def evas_obj_image_animated_frame_set
 * @since 1.8
 *
 * Set the frame to current frame of an image object
 *
 * @param[in] frame_index in
 *
 * @see evas_object_image_animated_frame_set
 */
#define evas_obj_image_animated_frame_set(frame_index) EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_ANIMATED_FRAME_SET), EO_TYPECHECK(int, frame_index)

/**
 * @}
 */

#define EVAS_OUT_CLASS evas_out_class_get()
const Eo_Class *evas_out_class_get(void) EINA_CONST;
extern EAPI Eo_Op EVAS_OUT_BASE_ID;

enum
  {
     EVAS_OUT_SUB_ID_VIEW_SET,
     EVAS_OUT_SUB_ID_VIEW_GET,
     EVAS_OUT_SUB_ID_ENGINE_INFO_SET,
     EVAS_OUT_SUB_ID_ENGINE_INFO_GET,
     EVAS_OUT_SUB_ID_LAST
  };

#define EVAS_OUT_ID(sub_id) (EVAS_OUT_BASE_ID + sub_id)

/**
 * @def evas_out_view_set
 * @since 1.8
 *
 * Sets the viewport region of the canvas that the output displays
 * 
 * This sets both the viewport region in the canvas that displays on the
 * given output, but also the viewport size will match the output
 * resolution 1:1.
 *
 * @param[in] x
 * @param[in] y
 * @param[in] w
 * @param[in] h
 *
 * @see evas_out_engine_info_set
 * @see evas_output_viewport_set
 * @see evas_output_size_set
 */
#define evas_out_view_set(x, y, w, h) \
   EVAS_OUT_ID(EVAS_OUT_SUB_ID_VIEW_SET), \
EO_TYPECHECK(Evas_Coord, x), \
EO_TYPECHECK(Evas_Coord, y), \
EO_TYPECHECK(Evas_Coord, w), \
EO_TYPECHECK(Evas_Coord, h)

/**
 * @def evas_out_view_get
 * @since 1.8
 *
 * Gets the viewport region of the canvas that the output displays
 *
 * @param[out] x
 * @param[out] y
 * @param[out] w
 * @param[out] h
 *
 * @see evas_out_engine_info_get
 * @see evas_out_view_set
 * @see evas_output_viewport_get
 * @see evas_output_size_get
 */
#define evas_out_view_get(x, y, w, h) \
   EVAS_OUT_ID(EVAS_OUT_SUB_ID_VIEW_GET), \
EO_TYPECHECK(Evas_Coord *, x), \
EO_TYPECHECK(Evas_Coord *, y), \
EO_TYPECHECK(Evas_Coord *, w), \
EO_TYPECHECK(Evas_Coord *, h)

/**
 * @def evas_out_engine_info_set
 * @since 1.8
 *
 * Sets the engine specific output parameters for a given output.
 *
 * @param[in] info The engine parameters
 * @param[out] ret Return value for success (EINA_TRUE for success)
 *
 * @see evas_out_engine_info_get
 * @see evas_output_viewport_set
 * @see evas_output_size_set
 */
#define evas_out_engine_info_set(info, ret) \
   EVAS_OUT_ID(EVAS_OUT_SUB_ID_ENGINE_INFO_SET), \
EO_TYPECHECK(Evas_Engine_Info *, info), \
EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def evas_out_engine_info_get
 * @since 1.8
 *
 * Gets the engine specific output parameters for a given output.
 *
 * @param[out] info The engine parameters return (NULL on failure)
 *
 * @see evas_out_engine_info_set
 * @see evas_output_viewport_get
 * @see evas_output_size_get
 */
#define evas_out_engine_info_get(ret) \
   EVAS_OUT_ID(EVAS_OUT_SUB_ID_ENGINE_INFO_GET), \
EO_TYPECHECK(Evas_Engine_Info **, ret)
