#ifndef ELM_INTEFARCE_SCROLLER_H
#define ELM_INTEFARCE_SCROLLER_H

/**
 * @addtogroup Widget
 * @{
 *
 * @section elm-scrollable-interface The Elementary Scrollable Interface
 *
 * This is a common interface for widgets having @b scrollable views.
 * Widgets using/implementing this must use the
 * @c EVAS_SMART_SUBCLASS_IFACE_NEW macro (instead of the
 * @c EVAS_SMART_SUBCLASS_NEW one) when declaring its smart class,
 * so an interface is also declared.
 *
 * The scrollable interface comes built with Elementary and is exposed
 * as #ELM_SCROLLABLE_IFACE.
 *
 * The interface API is explained in details at
 * #Elm_Scrollable_Smart_Interface.
 *
 * An Elementary scrollable interface will handle an internal @b
 * panning object. It has the function of clipping and moving the
 * actual scrollable content around, by the command of the scrollable
 * interface calls. Though it's not the common case, one might
 * want/have to change some aspects of the internal panning object
 * behavior.  For that, we have it also exposed here --
 * #Elm_Pan_Smart_Class. Use elm_pan_smart_class_get() to build your
 * custom panning object, when creating a scrollable widget (again,
 * only if you need a custom panning object) and set it with
 * Elm_Scrollable_Smart_Interface::extern_pan_set.
 */

#define ELM_OBJ_PAN_CLASS elm_obj_pan_eo_class_get()
const Eo_Class *elm_obj_pan_eo_class_get(void) EINA_CONST;

extern EAPI Eo_Op ELM_OBJ_PAN_BASE_ID;

enum
{
   ELM_OBJ_PAN_SUB_ID_POS_SET,
   ELM_OBJ_PAN_SUB_ID_POS_GET,
   ELM_OBJ_PAN_SUB_ID_POS_MAX_GET,
   ELM_OBJ_PAN_SUB_ID_POS_MIN_GET,
   ELM_OBJ_PAN_SUB_ID_CONTENT_SIZE_GET,
   ELM_OBJ_PAN_SUB_ID_GRAVITY_SET,
   ELM_OBJ_PAN_SUB_ID_GRAVITY_GET,
   ELM_OBJ_PAN_SUB_ID_LAST
};

#define ELM_OBJ_PAN_ID(sub_id) (ELM_OBJ_PAN_BASE_ID + sub_id)

/**
 * @def elm_obj_pan_pos_set
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[in] x
 * @param[in] y
 *
 * @see elm_pan_pos_set
 */
#define elm_obj_pan_pos_set(x, y) ELM_OBJ_PAN_ID(ELM_OBJ_PAN_SUB_ID_POS_SET), EO_TYPECHECK(Evas_Coord, x), EO_TYPECHECK(Evas_Coord, y)

/**
 * @def elm_obj_pan_pos_get
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[out] x
 * @param[out] y
 *
 * @see elm_pan_pos_get
 */
#define elm_obj_pan_pos_get(x, y) ELM_OBJ_PAN_ID(ELM_OBJ_PAN_SUB_ID_POS_GET), EO_TYPECHECK(Evas_Coord *, x), EO_TYPECHECK(Evas_Coord *, y)

/**
 * @def elm_obj_pan_pos_max_get
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[out] x
 * @param[out] y
 *
 * @see elm_pan_pos_max_get
 */
#define elm_obj_pan_pos_max_get(x, y) ELM_OBJ_PAN_ID(ELM_OBJ_PAN_SUB_ID_POS_MAX_GET), EO_TYPECHECK(Evas_Coord *, x), EO_TYPECHECK(Evas_Coord *, y)

/**
 * @def elm_obj_pan_pos_min_get
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[out] x
 * @param[out] y
 *
 * @see elm_pan_pos_min_get
 */
#define elm_obj_pan_pos_min_get(x, y) ELM_OBJ_PAN_ID(ELM_OBJ_PAN_SUB_ID_POS_MIN_GET), EO_TYPECHECK(Evas_Coord *, x), EO_TYPECHECK(Evas_Coord *, y)

/**
 * @def elm_obj_pan_content_size_get
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[out] w
 * @param[out] h
 *
 * @see elm_pan_content_size_get
 */
#define elm_obj_pan_content_size_get(w, h) ELM_OBJ_PAN_ID(ELM_OBJ_PAN_SUB_ID_CONTENT_SIZE_GET), EO_TYPECHECK(Evas_Coord *, w), EO_TYPECHECK(Evas_Coord *, h)

/**
 * @def elm_obj_pan_gravity_set
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[in] x
 * @param[in] y
 *
 * @see elm_pan_gravity_set
 */
#define elm_obj_pan_gravity_set(x, y) ELM_OBJ_PAN_ID(ELM_OBJ_PAN_SUB_ID_GRAVITY_SET), EO_TYPECHECK(double, x), EO_TYPECHECK(double, y)

/**
 * @def elm_obj_pan_gravity_get
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[out] x
 * @param[out] y
 *
 * @see elm_pan_gravity_get
 */
#define elm_obj_pan_gravity_get(x, y) ELM_OBJ_PAN_ID(ELM_OBJ_PAN_SUB_ID_GRAVITY_GET), EO_TYPECHECK(double *, x), EO_TYPECHECK(double *, y)

#define ELM_SCROLLABLE_INTERFACE elm_scrollable_interface_get()

const Eo_Class *elm_scrollable_interface_get(void) EINA_CONST;

extern EAPI Eo_Op ELM_SCROLLABLE_INTERFACE_BASE_ID;

enum
{
   ELM_SCROLLABLE_INTERFACE_SUB_ID_OBJECTS_SET,
   ELM_SCROLLABLE_INTERFACE_SUB_ID_CONTENT_SET,
   ELM_SCROLLABLE_INTERFACE_SUB_ID_EXTERN_PAN_SET,
   ELM_SCROLLABLE_INTERFACE_SUB_ID_DRAG_START_CB_SET,
   ELM_SCROLLABLE_INTERFACE_SUB_ID_DRAG_STOP_CB_SET,
   ELM_SCROLLABLE_INTERFACE_SUB_ID_ANIMATE_START_CB_SET,
   ELM_SCROLLABLE_INTERFACE_SUB_ID_ANIMATE_STOP_CB_SET,
   ELM_SCROLLABLE_INTERFACE_SUB_ID_SCROLL_CB_SET,
   ELM_SCROLLABLE_INTERFACE_SUB_ID_SCROLL_LEFT_CB_SET,
   ELM_SCROLLABLE_INTERFACE_SUB_ID_SCROLL_RIGHT_CB_SET,
   ELM_SCROLLABLE_INTERFACE_SUB_ID_SCROLL_UP_CB_SET,
   ELM_SCROLLABLE_INTERFACE_SUB_ID_SCROLL_DOWN_CB_SET,
   ELM_SCROLLABLE_INTERFACE_SUB_ID_EDGE_LEFT_CB_SET,
   ELM_SCROLLABLE_INTERFACE_SUB_ID_EDGE_RIGHT_CB_SET,
   ELM_SCROLLABLE_INTERFACE_SUB_ID_EDGE_TOP_CB_SET,
   ELM_SCROLLABLE_INTERFACE_SUB_ID_EDGE_BOTTOM_CB_SET,
   ELM_SCROLLABLE_INTERFACE_SUB_ID_VBAR_DRAG_CB_SET,
   ELM_SCROLLABLE_INTERFACE_SUB_ID_VBAR_PRESS_CB_SET,
   ELM_SCROLLABLE_INTERFACE_SUB_ID_VBAR_UNPRESS_CB_SET,
   ELM_SCROLLABLE_INTERFACE_SUB_ID_HBAR_DRAG_CB_SET,
   ELM_SCROLLABLE_INTERFACE_SUB_ID_HBAR_PRESS_CB_SET,
   ELM_SCROLLABLE_INTERFACE_SUB_ID_HBAR_UNPRESS_CB_SET,
   ELM_SCROLLABLE_INTERFACE_SUB_ID_CONTENT_MIN_LIMIT_CB_SET,
   ELM_SCROLLABLE_INTERFACE_SUB_ID_CONTENT_VIEWPORT_RESIZE_CB_SET,
   ELM_SCROLLABLE_INTERFACE_SUB_ID_CONTENT_POS_SET,
   ELM_SCROLLABLE_INTERFACE_SUB_ID_CONTENT_POS_GET,
   ELM_SCROLLABLE_INTERFACE_SUB_ID_CONTENT_REGION_SHOW,
   ELM_SCROLLABLE_INTERFACE_SUB_ID_CONTENT_REGION_SET,
   ELM_SCROLLABLE_INTERFACE_SUB_ID_CONTENT_REGION_GET,
   ELM_SCROLLABLE_INTERFACE_SUB_ID_CONTENT_SIZE_GET,
   ELM_SCROLLABLE_INTERFACE_SUB_ID_CONTENT_VIEWPORT_SIZE_GET,
   ELM_SCROLLABLE_INTERFACE_SUB_ID_CONTENT_MIN_LIMIT,
   ELM_SCROLLABLE_INTERFACE_SUB_ID_STEP_SIZE_SET,
   ELM_SCROLLABLE_INTERFACE_SUB_ID_STEP_SIZE_GET,
   ELM_SCROLLABLE_INTERFACE_SUB_ID_PAGE_SIZE_SET,
   ELM_SCROLLABLE_INTERFACE_SUB_ID_PAGE_SIZE_GET,
   ELM_SCROLLABLE_INTERFACE_SUB_ID_POLICY_SET,
   ELM_SCROLLABLE_INTERFACE_SUB_ID_POLICY_GET,
   ELM_SCROLLABLE_INTERFACE_SUB_ID_SINGLE_DIRECTION_SET,
   ELM_SCROLLABLE_INTERFACE_SUB_ID_SINGLE_DIRECTION_GET,
   ELM_SCROLLABLE_INTERFACE_SUB_ID_REPEAT_EVENTS_SET,
   ELM_SCROLLABLE_INTERFACE_SUB_ID_REPEAT_EVENTS_GET,
   ELM_SCROLLABLE_INTERFACE_SUB_ID_MIRRORED_SET,
   ELM_SCROLLABLE_INTERFACE_SUB_ID_HOLD_SET,
   ELM_SCROLLABLE_INTERFACE_SUB_ID_FREEZE_SET,
   ELM_SCROLLABLE_INTERFACE_SUB_ID_PAGE_BOUNCE_ALLOW_SET,
   ELM_SCROLLABLE_INTERFACE_SUB_ID_PAGE_BOUNCE_ALLOW_GET,
   ELM_SCROLLABLE_INTERFACE_SUB_ID_BOUNCE_ALLOW_SET,
   ELM_SCROLLABLE_INTERFACE_SUB_ID_BOUNCE_ALLOW_GET,
   ELM_SCROLLABLE_INTERFACE_SUB_ID_PAGING_SET,
   ELM_SCROLLABLE_INTERFACE_SUB_ID_PAGING_GET,
   ELM_SCROLLABLE_INTERFACE_SUB_ID_PAGE_RELATIVE_SET,
   ELM_SCROLLABLE_INTERFACE_SUB_ID_PAGE_SCROLL_LIMIT_SET,
   ELM_SCROLLABLE_INTERFACE_SUB_ID_PAGE_SCROLL_LIMIT_GET,
   ELM_SCROLLABLE_INTERFACE_SUB_ID_CURRENT_PAGE_GET,
   ELM_SCROLLABLE_INTERFACE_SUB_ID_LAST_PAGE_GET,
   ELM_SCROLLABLE_INTERFACE_SUB_ID_PAGE_SHOW,
   ELM_SCROLLABLE_INTERFACE_SUB_ID_PAGE_BRING_IN,
   ELM_SCROLLABLE_INTERFACE_SUB_ID_REGION_BRING_IN,
   ELM_SCROLLABLE_INTERFACE_SUB_ID_GRAVITY_SET,
   ELM_SCROLLABLE_INTERFACE_SUB_ID_GRAVITY_GET,
   ELM_SCROLLABLE_INTERFACE_SUB_ID_MOMENTUM_ANIMATOR_DISABLED_GET,
   ELM_SCROLLABLE_INTERFACE_SUB_ID_MOMENTUM_ANIMATOR_DISABLED_SET,
   ELM_SCROLLABLE_INTERFACE_SUB_ID_BOUNCE_ANIMATOR_DISABLED_SET,
   ELM_SCROLLABLE_INTERFACE_SUB_ID_BOUNCE_ANIMATOR_DISABLED_GET,
   ELM_SCROLLABLE_INTERFACE_SUB_ID_WHEEL_DISABLED_GET,
   ELM_SCROLLABLE_INTERFACE_SUB_ID_WHEEL_DISABLED_SET,
   ELM_SCROLLABLE_INTERFACE_SUB_ID_MOVEMENT_BLOCK_SET,
   ELM_SCROLLABLE_INTERFACE_SUB_ID_MOVEMENT_BLOCK_GET,
   ELM_SCROLLABLE_INTERFACE_SUB_ID_PAGE_CHANGE_CB_SET,
   ELM_SCROLLABLE_INTERFACE_SUB_ID_LAST
};

#define ELM_SCROLLABLE_INTERFACE_ID(sub_id) (ELM_SCROLLABLE_INTERFACE_BASE_ID + sub_id)


/**
 * @def elm_scrollable_interface_objects_set
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[in] edje_object
 * @param[in] hit_rectangle
 *
 */
#define elm_scrollable_interface_objects_set(edje_object, hit_rectangle) ELM_SCROLLABLE_INTERFACE_ID(ELM_SCROLLABLE_INTERFACE_SUB_ID_OBJECTS_SET), EO_TYPECHECK(Evas_Object *, edje_object), EO_TYPECHECK(Evas_Object *, hit_rectangle)

/**
 * @def elm_scrollable_interface_content_set
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[in] content
 *
 */
#define elm_scrollable_interface_content_set(content) ELM_SCROLLABLE_INTERFACE_ID(ELM_SCROLLABLE_INTERFACE_SUB_ID_CONTENT_SET), EO_TYPECHECK(Evas_Object *, content)

/**
 * @def elm_scrollable_interface_extern_pan_set
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[in] pan
 *
 */
#define elm_scrollable_interface_extern_pan_set(pan) ELM_SCROLLABLE_INTERFACE_ID(ELM_SCROLLABLE_INTERFACE_SUB_ID_EXTERN_PAN_SET), EO_TYPECHECK(Evas_Object *, pan)

/**
 * @def elm_scrollable_interface_drag_start_cb_set
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[in] drag_start_cb
 *
 */
#define elm_scrollable_interface_drag_start_cb_set(drag_start_cb) ELM_SCROLLABLE_INTERFACE_ID(ELM_SCROLLABLE_INTERFACE_SUB_ID_DRAG_START_CB_SET), EO_TYPECHECK(Elm_Interface_Scrollable_Cb, drag_start_cb)

/**
 * @def elm_scrollable_interface_drag_stop_cb_set
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[in] drag_stop_cb
 *
 */
#define elm_scrollable_interface_drag_stop_cb_set(drag_stop_cb) ELM_SCROLLABLE_INTERFACE_ID(ELM_SCROLLABLE_INTERFACE_SUB_ID_DRAG_STOP_CB_SET), EO_TYPECHECK(Elm_Interface_Scrollable_Cb, drag_stop_cb)

/**
 * @def elm_scrollable_interface_animate_start_cb_set
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[in] animate_start_cb
 *
 */
#define elm_scrollable_interface_animate_start_cb_set(animate_start_cb) ELM_SCROLLABLE_INTERFACE_ID(ELM_SCROLLABLE_INTERFACE_SUB_ID_ANIMATE_START_CB_SET), EO_TYPECHECK(Elm_Interface_Scrollable_Cb, animate_start_cb)

/**
 * @def elm_scrollable_interface_animate_stop_cb_set
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[in] animate_stop_cb
 *
 */
#define elm_scrollable_interface_animate_stop_cb_set(animate_stop_cb) ELM_SCROLLABLE_INTERFACE_ID(ELM_SCROLLABLE_INTERFACE_SUB_ID_ANIMATE_STOP_CB_SET), EO_TYPECHECK(Elm_Interface_Scrollable_Cb, animate_stop_cb)

/**
 * @def elm_scrollable_interface_scroll_cb_set
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[in] scroll_cb
 *
 */
#define elm_scrollable_interface_scroll_cb_set(scroll_cb) ELM_SCROLLABLE_INTERFACE_ID(ELM_SCROLLABLE_INTERFACE_SUB_ID_SCROLL_CB_SET), EO_TYPECHECK(Elm_Interface_Scrollable_Cb, scroll_cb)

/**
 * @def elm_scrollable_interface_scroll_left_cb_set
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[in] scroll_left_cb
 *
 */
#define elm_scrollable_interface_scroll_left_cb_set(scroll_left_cb) ELM_SCROLLABLE_INTERFACE_ID(ELM_SCROLLABLE_INTERFACE_SUB_ID_SCROLL_LEFT_CB_SET), EO_TYPECHECK(Elm_Interface_Scrollable_Cb, scroll_left_cb)

/**
 * @def elm_scrollable_interface_scroll_right_cb_set
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[in] scroll_right_cb
 *
 */
#define elm_scrollable_interface_scroll_right_cb_set(scroll_right_cb) ELM_SCROLLABLE_INTERFACE_ID(ELM_SCROLLABLE_INTERFACE_SUB_ID_SCROLL_RIGHT_CB_SET), EO_TYPECHECK(Elm_Interface_Scrollable_Cb, scroll_right_cb)

/**
 * @def elm_scrollable_interface_scroll_up_cb_set
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[in] scroll_up_cb
 *
 */
#define elm_scrollable_interface_scroll_up_cb_set(scroll_up_cb) ELM_SCROLLABLE_INTERFACE_ID(ELM_SCROLLABLE_INTERFACE_SUB_ID_SCROLL_UP_CB_SET), EO_TYPECHECK(Elm_Interface_Scrollable_Cb, scroll_up_cb)

/**
 * @def elm_scrollable_interface_scroll_down_cb_set
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[in] scroll_down_cb
 *
 */
#define elm_scrollable_interface_scroll_down_cb_set(scroll_down_cb) ELM_SCROLLABLE_INTERFACE_ID(ELM_SCROLLABLE_INTERFACE_SUB_ID_SCROLL_DOWN_CB_SET), EO_TYPECHECK(Elm_Interface_Scrollable_Cb, scroll_down_cb)

/**
 * @def elm_scrollable_interface_edge_left_cb_set
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[in] edge_left_cb
 *
 */
#define elm_scrollable_interface_edge_left_cb_set(edge_left_cb) ELM_SCROLLABLE_INTERFACE_ID(ELM_SCROLLABLE_INTERFACE_SUB_ID_EDGE_LEFT_CB_SET), EO_TYPECHECK(Elm_Interface_Scrollable_Cb, edge_left_cb)

/**
 * @def elm_scrollable_interface_edge_right_cb_set
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[in] edge_right_cb
 *
 */
#define elm_scrollable_interface_edge_right_cb_set(edge_right_cb) ELM_SCROLLABLE_INTERFACE_ID(ELM_SCROLLABLE_INTERFACE_SUB_ID_EDGE_RIGHT_CB_SET), EO_TYPECHECK(Elm_Interface_Scrollable_Cb, edge_right_cb)

/**
 * @def elm_scrollable_interface_edge_top_cb_set
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[in] edge_top_cb
 *
 */
#define elm_scrollable_interface_edge_top_cb_set(edge_top_cb) ELM_SCROLLABLE_INTERFACE_ID(ELM_SCROLLABLE_INTERFACE_SUB_ID_EDGE_TOP_CB_SET), EO_TYPECHECK(Elm_Interface_Scrollable_Cb, edge_top_cb)

/**
 * @def elm_scrollable_interface_edge_bottom_cb_set
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[in] edge_bottom_cb
 *
 */
#define elm_scrollable_interface_edge_bottom_cb_set(edge_bottom_cb) ELM_SCROLLABLE_INTERFACE_ID(ELM_SCROLLABLE_INTERFACE_SUB_ID_EDGE_BOTTOM_CB_SET), EO_TYPECHECK(Elm_Interface_Scrollable_Cb, edge_bottom_cb)

/**
 * @def elm_scrollable_interface_vbar_drag_cb_set
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[in] vbra_drag_cb
 *
 */
#define elm_scrollable_interface_vbar_drag_cb_set(vbar_drag_cb) ELM_SCROLLABLE_INTERFACE_ID(ELM_SCROLLABLE_INTERFACE_SUB_ID_VBAR_DRAG_CB_SET), EO_TYPECHECK(Elm_Interface_Scrollable_Cb, vbar_drag_cb)

/**
 * @def elm_scrollable_interface_vbar_press_cb_set
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[in] vbra_press_cb
 *
 */
#define elm_scrollable_interface_vbar_press_cb_set(vbar_press_cb) ELM_SCROLLABLE_INTERFACE_ID(ELM_SCROLLABLE_INTERFACE_SUB_ID_VBAR_PRESS_CB_SET), EO_TYPECHECK(Elm_Interface_Scrollable_Cb, vbar_press_cb)

/**
 * @def elm_scrollable_interface_vbar_unpress_cb_set
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[in] vbra_unpress_cb
 *
 */
#define elm_scrollable_interface_vbar_unpress_cb_set(vbar_unpress_cb) ELM_SCROLLABLE_INTERFACE_ID(ELM_SCROLLABLE_INTERFACE_SUB_ID_VBAR_UNPRESS_CB_SET), EO_TYPECHECK(Elm_Interface_Scrollable_Cb, vbar_unpress_cb)

/**
 * @def elm_scrollable_interface_hbar_drag_cb_set
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[in] vbra_drag_cb
 *
 */
#define elm_scrollable_interface_hbar_drag_cb_set(hbar_drag_cb) ELM_SCROLLABLE_INTERFACE_ID(ELM_SCROLLABLE_INTERFACE_SUB_ID_HBAR_DRAG_CB_SET), EO_TYPECHECK(Elm_Interface_Scrollable_Cb, hbar_drag_cb)

/**
 * @def elm_scrollable_interface_hbar_press_cb_set
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[in] vbra_press_cb
 *
 */
#define elm_scrollable_interface_hbar_press_cb_set(hbar_press_cb) ELM_SCROLLABLE_INTERFACE_ID(ELM_SCROLLABLE_INTERFACE_SUB_ID_HBAR_PRESS_CB_SET), EO_TYPECHECK(Elm_Interface_Scrollable_Cb, hbar_press_cb)

/**
 * @def elm_scrollable_interface_hbar_unpress_cb_set
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[in] vbra_unpress_cb
 *
 */
#define elm_scrollable_interface_hbar_unpress_cb_set(hbar_unpress_cb) ELM_SCROLLABLE_INTERFACE_ID(ELM_SCROLLABLE_INTERFACE_SUB_ID_HBAR_UNPRESS_CB_SET), EO_TYPECHECK(Elm_Interface_Scrollable_Cb, hbar_unpress_cb)

/**
 * @def elm_scrollable_interface_content_min_limit_cb_set
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[in] min_limit_cb
 *
 */
#define elm_scrollable_interface_content_min_limit_cb_set(min_limit_cb) ELM_SCROLLABLE_INTERFACE_ID(ELM_SCROLLABLE_INTERFACE_SUB_ID_CONTENT_MIN_LIMIT_CB_SET), EO_TYPECHECK(Elm_Interface_Scrollable_Min_Limit_Cb, min_limit_cb)

/**
 * @def elm_scrollable_interface_content_viewport_resize_cb_set
 * @since 1.8
 *
 * When the viewport is resized, the callback is called.
 *
 * @param[in] viewport_resize_cb
 *
 */
#define elm_scrollable_interface_content_viewport_resize_cb_set(viewport_resize_cb) ELM_SCROLLABLE_INTERFACE_ID(ELM_SCROLLABLE_INTERFACE_SUB_ID_CONTENT_VIEWPORT_RESIZE_CB_SET), EO_TYPECHECK(Elm_Interface_Scrollable_Resize_Cb, viewport_resize_cb)

/**
 * @def elm_scrollable_interface_content_pos_set
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[in] x
 * @param[in] y
 * @param[in] sig
 *
 */
#define elm_scrollable_interface_content_pos_set(x, y, sig) ELM_SCROLLABLE_INTERFACE_ID(ELM_SCROLLABLE_INTERFACE_SUB_ID_CONTENT_POS_SET), EO_TYPECHECK(Evas_Coord, x), EO_TYPECHECK(Evas_Coord, y), EO_TYPECHECK(Eina_Bool, sig)

/**
 * @def elm_scrollable_interface_content_pos_get
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[out] x
 * @param[out] y
 *
 */
#define elm_scrollable_interface_content_pos_get(x, y) ELM_SCROLLABLE_INTERFACE_ID(ELM_SCROLLABLE_INTERFACE_SUB_ID_CONTENT_POS_GET), EO_TYPECHECK(Evas_Coord *, x), EO_TYPECHECK(Evas_Coord *, y)

/**
 * @def elm_scrollable_interface_content_region_show
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[in] x
 * @param[in] y
 * @param[in] w
 * @param[in] h
 *
 */
#define elm_scrollable_interface_content_region_show(x, y, w, h) ELM_SCROLLABLE_INTERFACE_ID(ELM_SCROLLABLE_INTERFACE_SUB_ID_CONTENT_REGION_SHOW), EO_TYPECHECK(Evas_Coord, x), EO_TYPECHECK(Evas_Coord, y), EO_TYPECHECK(Evas_Coord, w), EO_TYPECHECK(Evas_Coord, h)

/**
 * @def elm_scrollable_interface_content_region_set
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[in] x
 * @param[in] y
 * @param[in] w
 * @param[in] h
 *
 */
#define elm_scrollable_interface_content_region_set(x, y, w, h) ELM_SCROLLABLE_INTERFACE_ID(ELM_SCROLLABLE_INTERFACE_SUB_ID_CONTENT_REGION_SET), EO_TYPECHECK(Evas_Coord, x), EO_TYPECHECK(Evas_Coord, y), EO_TYPECHECK(Evas_Coord, w), EO_TYPECHECK(Evas_Coord, h)

/**
 * @def elm_scrollable_interface_content_region_get
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[out] x
 * @param[out] y
 * @param[out] w
 * @param[out] h
 *
 */
#define elm_scrollable_interface_content_region_get(x, y, w, h) ELM_SCROLLABLE_INTERFACE_ID(ELM_SCROLLABLE_INTERFACE_SUB_ID_CONTENT_REGION_GET), EO_TYPECHECK(Evas_Coord *, x), EO_TYPECHECK(Evas_Coord *, y), EO_TYPECHECK(Evas_Coord *, w), EO_TYPECHECK(Evas_Coord *, h)

/**
 * @def elm_scrollable_interface_content_size_get
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[out] w
 * @param[out] h
 *
 */
#define elm_scrollable_interface_content_size_get(w, h) ELM_SCROLLABLE_INTERFACE_ID(ELM_SCROLLABLE_INTERFACE_SUB_ID_CONTENT_SIZE_GET), EO_TYPECHECK(Evas_Coord *, w), EO_TYPECHECK(Evas_Coord *, h)

/**
 * @def elm_scrollable_interface_content_viewport_size_get
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[out] w
 * @param[out] h
 *
 */
#define elm_scrollable_interface_content_viewport_size_get(w, h) ELM_SCROLLABLE_INTERFACE_ID(ELM_SCROLLABLE_INTERFACE_SUB_ID_CONTENT_VIEWPORT_SIZE_GET), EO_TYPECHECK(Evas_Coord *, w), EO_TYPECHECK(Evas_Coord *, h)

/**
 * @def elm_scrollable_interface_content_min_limit
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[in] w
 * @param[in] h
 *
 */
#define elm_scrollable_interface_content_min_limit(w, h) ELM_SCROLLABLE_INTERFACE_ID(ELM_SCROLLABLE_INTERFACE_SUB_ID_CONTENT_MIN_LIMIT), EO_TYPECHECK(Eina_Bool, w), EO_TYPECHECK(Eina_Bool, h)

/**
 * @def elm_scrollable_interface_step_size_set
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[in] x
 * @param[in] y
 *
 */
#define elm_scrollable_interface_step_size_set(x, y) ELM_SCROLLABLE_INTERFACE_ID(ELM_SCROLLABLE_INTERFACE_SUB_ID_STEP_SIZE_SET), EO_TYPECHECK(Evas_Coord, x), EO_TYPECHECK(Evas_Coord, y)

/**
 * @def elm_scrollable_interface_step_size_get
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[out] x
 * @param[out] y
 *
 */
#define elm_scrollable_interface_step_size_get(x, y) ELM_SCROLLABLE_INTERFACE_ID(ELM_SCROLLABLE_INTERFACE_SUB_ID_STEP_SIZE_GET), EO_TYPECHECK(Evas_Coord *, x), EO_TYPECHECK(Evas_Coord *, y)

/**
 * @def elm_scrollable_interface_page_size_set
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[in] x
 * @param[in] y
 *
 */
#define elm_scrollable_interface_page_size_set(x, y) ELM_SCROLLABLE_INTERFACE_ID(ELM_SCROLLABLE_INTERFACE_SUB_ID_PAGE_SIZE_SET), EO_TYPECHECK(Evas_Coord, x), EO_TYPECHECK(Evas_Coord, y)

/**
 * @def elm_scrollable_interface_page_size_get
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[out] x
 * @param[out] y
 *
 */
#define elm_scrollable_interface_page_size_get(x, y) ELM_SCROLLABLE_INTERFACE_ID(ELM_SCROLLABLE_INTERFACE_SUB_ID_PAGE_SIZE_GET), EO_TYPECHECK(Evas_Coord *, x), EO_TYPECHECK(Evas_Coord *, y)

/**
 * @def elm_scrollable_interface_page_relative_set
 * @since 1.8
 *
 * @brief Set scroll page size relative to viewport size.
 *
 * @param[in] h_pagerel
 * @param[in] v_pagerel
 *
 */
#define elm_scrollable_interface_page_relative_set(h_pagerel, v_pagerel) ELM_SCROLLABLE_INTERFACE_ID(ELM_SCROLLABLE_INTERFACE_SUB_ID_PAGE_RELATIVE_SET), EO_TYPECHECK(double, h_pagerel), EO_TYPECHECK(double, v_pagerel)

/**
 * @def elm_scrollable_interface_policy_set
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[in] hbar
 * @param[in] vbar
 *
 */
#define elm_scrollable_interface_policy_set(hbar, vbar) ELM_SCROLLABLE_INTERFACE_ID(ELM_SCROLLABLE_INTERFACE_SUB_ID_POLICY_SET), EO_TYPECHECK(Elm_Scroller_Policy, hbar), EO_TYPECHECK(Elm_Scroller_Policy, vbar)

/**
 * @def elm_scrollable_interface_policy_get
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[out] hbar
 * @param[out] vbar
 *
 */
#define elm_scrollable_interface_policy_get(hbar, vbar) ELM_SCROLLABLE_INTERFACE_ID(ELM_SCROLLABLE_INTERFACE_SUB_ID_POLICY_GET), EO_TYPECHECK(Elm_Scroller_Policy *, hbar), EO_TYPECHECK(Elm_Scroller_Policy *, vbar)

/**
 * @def elm_scrollable_interface_single_direction_set
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[in] single_dir
 *
 */
#define elm_scrollable_interface_single_direction_set(single_dir) ELM_SCROLLABLE_INTERFACE_ID(ELM_SCROLLABLE_INTERFACE_SUB_ID_SINGLE_DIRECTION_SET), EO_TYPECHECK(Elm_Scroller_Single_Direction, single_dir)

/**
 * @def elm_scrollable_interface_single_direction_get
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[out] single_dir
 *
 */
#define elm_scrollable_interface_single_direction_get(single_dir) ELM_SCROLLABLE_INTERFACE_ID(ELM_SCROLLABLE_INTERFACE_SUB_ID_SINGLE_DIRECTION_GET), EO_TYPECHECK(Elm_Scroller_Single_Direction *, single_dir)

/**
 * @def elm_scrollable_interface_repeat_events_set
 * @since 1.8
 *
 * It decides whether the scrollable object propagates the events to content
 * object or not.
 *
 * @param[in] repeat_events
 *
 */
#define elm_scrollable_interface_repeat_events_set(repeat_events) ELM_SCROLLABLE_INTERFACE_ID(ELM_SCROLLABLE_INTERFACE_SUB_ID_REPEAT_EVENTS_SET), EO_TYPECHECK(Eina_Bool, repeat_events)

/**
 * @def elm_scrollable_interface_repeat_events_get
 * @since 1.8
 *
 * It gets whether the scrollable object propagates the events to content object
 * or not.
 *
 * @param[out] repeat_events
 *
 */
#define elm_scrollable_interface_repeat_events_get(repeat_events) ELM_SCROLLABLE_INTERFACE_ID(ELM_SCROLLABLE_INTERFACE_SUB_ID_REPEAT_EVENTS_GET), EO_TYPECHECK(Eina_Bool *, repeat_events)

/**
 * @def elm_scrollable_interface_mirrored_set
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[in] mirrored
 *
 */
#define elm_scrollable_interface_mirrored_set(mirrored) ELM_SCROLLABLE_INTERFACE_ID(ELM_SCROLLABLE_INTERFACE_SUB_ID_MIRRORED_SET), EO_TYPECHECK(Eina_Bool, mirrored)

/**
 * @def elm_scrollable_interface_hold_set
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[in] hold
 *
 */
#define elm_scrollable_interface_hold_set(hold) ELM_SCROLLABLE_INTERFACE_ID(ELM_SCROLLABLE_INTERFACE_SUB_ID_HOLD_SET), EO_TYPECHECK(Eina_Bool, hold)

/**
 * @def elm_scrollable_interface_freeze_set
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[in] freeze
 *
 */
#define elm_scrollable_interface_freeze_set(freeze) ELM_SCROLLABLE_INTERFACE_ID(ELM_SCROLLABLE_INTERFACE_SUB_ID_FREEZE_SET), EO_TYPECHECK(Eina_Bool, freeze)

/**
 * @def elm_scrollable_interface_page_snap_allow_set
 * @since 1.8
 *
 * Enable/disable page bouncing, for paged scrollers, on each axis.
 *
 * @param[in] horiz
 * @param[in] vert
 *
 */
#define elm_scrollable_interface_page_snap_allow_set(horiz, vert) ELM_SCROLLABLE_INTERFACE_ID(ELM_SCROLLABLE_INTERFACE_SUB_ID_PAGE_BOUNCE_ALLOW_SET), EO_TYPECHECK(Eina_Bool, horiz), EO_TYPECHECK(Eina_Bool, vert)

/**
 * @def elm_scrollable_interface_page_snap_allow_get
 * @since 1.8
 *
 * Get wether page bouncing is enabled, for paged scrollers, on each
 * axis.
 *
 * @param[out] horiz
 * @param[out] vert
 *
 */
#define elm_scrollable_interface_page_snap_allow_get(horiz, vert) ELM_SCROLLABLE_INTERFACE_ID(ELM_SCROLLABLE_INTERFACE_SUB_ID_PAGE_BOUNCE_ALLOW_GET), EO_TYPECHECK(Eina_Bool *, horiz), EO_TYPECHECK(Eina_Bool *, vert)

/**
 * @def elm_scrollable_interface_bounce_allow_set
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[in] horiz
 * @param[in] vert
 *
 */
#define elm_scrollable_interface_bounce_allow_set(horiz, vert) ELM_SCROLLABLE_INTERFACE_ID(ELM_SCROLLABLE_INTERFACE_SUB_ID_BOUNCE_ALLOW_SET), EO_TYPECHECK(Eina_Bool, horiz), EO_TYPECHECK(Eina_Bool, vert)

/**
 * @def elm_scrollable_interface_bounce_allow_get
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[out] horiz
 * @param[out] vert
 *
 */
#define elm_scrollable_interface_bounce_allow_get(horiz, vert) ELM_SCROLLABLE_INTERFACE_ID(ELM_SCROLLABLE_INTERFACE_SUB_ID_BOUNCE_ALLOW_GET), EO_TYPECHECK(Eina_Bool *, horiz), EO_TYPECHECK(Eina_Bool *, vert)

/**
 * @def elm_scrollable_interface_paging_set
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[in] pagerel_h
 * @param[in] pagerel_v
 * @param[in] pagesize_h
 * @param[in] pagesize_v
 *
 */
#define elm_scrollable_interface_paging_set(pagerel_h, pagerel_v, pagesize_h, pagesize_v) ELM_SCROLLABLE_INTERFACE_ID(ELM_SCROLLABLE_INTERFACE_SUB_ID_PAGING_SET), EO_TYPECHECK(double, pagerel_h), EO_TYPECHECK(double, pagerel_v), EO_TYPECHECK(Evas_Coord, pagesize_h), EO_TYPECHECK(Evas_Coord, pagesize_v)

/**
 * @def elm_scrollable_interface_paging_get
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[out] pagerel_h
 * @param[out] pagerel_v
 * @param[out] pagesize_h
 * @param[out] pagesize_v
 *
 */
#define elm_scrollable_interface_paging_get(pagerel_h, pagerel_v, pagesize_h, pagesize_v) ELM_SCROLLABLE_INTERFACE_ID(ELM_SCROLLABLE_INTERFACE_SUB_ID_PAGING_GET), EO_TYPECHECK(double *, pagerel_h), EO_TYPECHECK(double *, pagerel_v), EO_TYPECHECK(Evas_Coord *, pagesize_h), EO_TYPECHECK(Evas_Coord *, pagesize_v)

/**
 * @def elm_scrollable_interface_page_scroll_limit_set
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[in] page_limit_h
 * @param[in] page_limit_v
 *
 */
#define elm_scrollable_interface_page_scroll_limit_set(page_limit_h, page_limit_v) ELM_SCROLLABLE_INTERFACE_ID(ELM_SCROLLABLE_INTERFACE_SUB_ID_PAGE_SCROLL_LIMIT_SET), EO_TYPECHECK(int, page_limit_h), EO_TYPECHECK(int, page_limit_v)

/**
 * @def elm_scrollable_interface_page_scroll_limit_get
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[out] page_limit_h
 * @param[out] page_limit_v
 *
 */
#define elm_scrollable_interface_page_scroll_limit_get(page_limit_h, page_limit_v) ELM_SCROLLABLE_INTERFACE_ID(ELM_SCROLLABLE_INTERFACE_SUB_ID_PAGE_SCROLL_LIMIT_GET), EO_TYPECHECK(int *, page_limit_h), EO_TYPECHECK(int *, page_limit_v)

/**
 * @def elm_scrollable_interface_current_page_get
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[out] pagenumber_h
 * @param[out] pagenumber_v
 *
 */
#define elm_scrollable_interface_current_page_get(pagenumber_h, pagenumber_v) ELM_SCROLLABLE_INTERFACE_ID(ELM_SCROLLABLE_INTERFACE_SUB_ID_CURRENT_PAGE_GET), EO_TYPECHECK(int *, pagenumber_h), EO_TYPECHECK(int *, pagenumber_v)

/**
 * @def elm_scrollable_interface_last_page_get
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[out] pagenumber_h
 * @param[out] pagenumber_v
 *
 */
#define elm_scrollable_interface_last_page_get(pagenumber_h, pagenumber_v) ELM_SCROLLABLE_INTERFACE_ID(ELM_SCROLLABLE_INTERFACE_SUB_ID_LAST_PAGE_GET), EO_TYPECHECK(int *, pagenumber_h), EO_TYPECHECK(int *, pagenumber_v)

/**
 * @def elm_scrollable_interface_page_show
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[in] pagenumber_h
 * @param[in] pagenumber_v
 *
 */
#define elm_scrollable_interface_page_show(pagenumber_h, pagenumber_v) ELM_SCROLLABLE_INTERFACE_ID(ELM_SCROLLABLE_INTERFACE_SUB_ID_PAGE_SHOW), EO_TYPECHECK(int, pagenumber_h), EO_TYPECHECK(int, pagenumber_v)

/**
 * @def elm_scrollable_interface_page_bring_in
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[in] pagenumber_h
 * @param[in] pagenumber_v
 *
 */
#define elm_scrollable_interface_page_bring_in(pagenumber_h, pagenumber_v) ELM_SCROLLABLE_INTERFACE_ID(ELM_SCROLLABLE_INTERFACE_SUB_ID_PAGE_BRING_IN), EO_TYPECHECK(int, pagenumber_h), EO_TYPECHECK(int, pagenumber_v)

/**
 * @def elm_scrollable_interface_region_bring_in
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[in] x
 * @param[in] y
 * @param[in] w
 * @param[in] h
 *
 */
#define elm_scrollable_interface_region_bring_in(x, y, w, h) ELM_SCROLLABLE_INTERFACE_ID(ELM_SCROLLABLE_INTERFACE_SUB_ID_REGION_BRING_IN), EO_TYPECHECK(Evas_Coord, x), EO_TYPECHECK(Evas_Coord, y), EO_TYPECHECK(Evas_Coord, w), EO_TYPECHECK(Evas_Coord, h)

/**
 * @def elm_scrollable_interface_gravity_set
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[in] x
 * @param[in] y
 *
 */
#define elm_scrollable_interface_gravity_set(x, y) ELM_SCROLLABLE_INTERFACE_ID(ELM_SCROLLABLE_INTERFACE_SUB_ID_GRAVITY_SET), EO_TYPECHECK(double, x), EO_TYPECHECK(double, y)

/**
 * @def elm_scrollable_interface_gravity_get
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[out] x
 * @param[out] y
 *
 */
#define elm_scrollable_interface_gravity_get(x, y) ELM_SCROLLABLE_INTERFACE_ID(ELM_SCROLLABLE_INTERFACE_SUB_ID_GRAVITY_GET), EO_TYPECHECK(double *, x), EO_TYPECHECK(double *, y)

/**
 * @def elm_scrollable_interface_momentum_animator_disabled_get
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[out] ret
 *
 */
#define elm_scrollable_interface_momentum_animator_disabled_get(ret) ELM_SCROLLABLE_INTERFACE_ID(ELM_SCROLLABLE_INTERFACE_SUB_ID_MOMENTUM_ANIMATOR_DISABLED_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_scrollable_interface_momentum_animator_disabled_set
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[in] disabled
 *
 */
#define elm_scrollable_interface_momentum_animator_disabled_set(disabled) ELM_SCROLLABLE_INTERFACE_ID(ELM_SCROLLABLE_INTERFACE_SUB_ID_MOMENTUM_ANIMATOR_DISABLED_SET), EO_TYPECHECK(Eina_Bool, disabled)

/**
 * @def elm_scrollable_interface_bounce_animator_disabled_set
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[in] disabled
 *
 */
#define elm_scrollable_interface_bounce_animator_disabled_set(ret) ELM_SCROLLABLE_INTERFACE_ID(ELM_SCROLLABLE_INTERFACE_SUB_ID_BOUNCE_ANIMATOR_DISABLED_SET), EO_TYPECHECK(Eina_Bool, disabled)

/**
 * @def elm_scrollable_interface_bounce_animator_disabled_get
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[out] disabled
 *
 */
#define elm_scrollable_interface_bounce_animator_disabled_get(disabled) ELM_SCROLLABLE_INTERFACE_ID(ELM_SCROLLABLE_INTERFACE_SUB_ID_BOUNCE_ANIMATOR_DISABLED_GET), EO_TYPECHECK(Eina_Bool *, disabled)

/**
 * @def elm_scrollable_interface_wheel_disabled_get
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[out] ret
 *
 */
#define elm_scrollable_interface_wheel_disabled_get(ret) ELM_SCROLLABLE_INTERFACE_ID(ELM_SCROLLABLE_INTERFACE_SUB_ID_WHEEL_DISABLED_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_scrollable_interface_wheel_disabled_set
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[in] disabled
 *
 */
#define elm_scrollable_interface_wheel_disabled_set(disabled) ELM_SCROLLABLE_INTERFACE_ID(ELM_SCROLLABLE_INTERFACE_SUB_ID_WHEEL_DISABLED_SET), EO_TYPECHECK(Eina_Bool, disabled)

/**
 * @def elm_scrollable_interface_movement_block_set
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[in] block
 *
 */
#define elm_scrollable_interface_movement_block_set(block) ELM_SCROLLABLE_INTERFACE_ID(ELM_SCROLLABLE_INTERFACE_SUB_ID_MOVEMENT_BLOCK_SET), EO_TYPECHECK(Elm_Scroller_Movement_Block, block)

/**
 * @def elm_scrollable_interface_movement_block_get
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[in] block
 *
 */
#define elm_scrollable_interface_movement_block_get(block) ELM_SCROLLABLE_INTERFACE_ID(ELM_SCROLLABLE_INTERFACE_SUB_ID_MOVEMENT_BLOCK_GET), EO_TYPECHECK(Elm_Scroller_Movement_Block *, block)

/**
 * @def elm_scrollable_interface_page_change_cb_set
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[in] page_change_cb
 *
 */
#define elm_scrollable_interface_page_change_cb_set(page_change_cb) ELM_SCROLLABLE_INTERFACE_ID(ELM_SCROLLABLE_INTERFACE_SUB_ID_PAGE_CHANGE_CB_SET), EO_TYPECHECK(Elm_Interface_Scrollable_Cb, page_change_cb)


/**
 * Elementary scroller panning base smart data.
 */
typedef struct _Elm_Pan_Smart_Data Elm_Pan_Smart_Data;
struct _Elm_Pan_Smart_Data
{
   Evas_Object                   *self;
   Evas_Object                   *content;
   Evas_Coord                     x, y, w, h;
   Evas_Coord                     content_w, content_h, px, py;
   double                         gravity_x, gravity_y;
   Evas_Coord                     prev_cw, prev_ch, delta_posx, delta_posy;
};

/**
 * Elementary scrollable interface base data.
 */
typedef void      (*Elm_Interface_Scrollable_Cb)(Evas_Object *, void *data);
typedef void      (*Elm_Interface_Scrollable_Min_Limit_Cb)(Evas_Object *obj, Eina_Bool w, Eina_Bool h);
typedef void      (*Elm_Interface_Scrollable_Resize_Cb)(Evas_Object *obj, Evas_Coord w, Evas_Coord h);

typedef struct _Elm_Scrollable_Smart_Interface_Data
  Elm_Scrollable_Smart_Interface_Data;
struct _Elm_Scrollable_Smart_Interface_Data
{
   Evas_Coord                    x, y, w, h;
   Evas_Coord                    wx, wy, ww, wh; /**< Last "wanted" geometry */

   Evas_Object                  *obj;
   Evas_Object                  *content;
   Evas_Object                  *pan_obj;
   Evas_Object                  *edje_obj;
   Evas_Object                  *event_rect;

   Evas_Object                  *parent_widget;

   Elm_Scroller_Policy           hbar_flags, vbar_flags;
   Elm_Scroller_Single_Direction one_direction_at_a_time;
   Elm_Scroller_Movement_Block block;

   struct
   {
      Evas_Coord x, y;
      Evas_Coord sx, sy;
      Evas_Coord dx, dy;
      Evas_Coord pdx, pdy;
      Evas_Coord bx, by;
      Evas_Coord ax, ay;
      Evas_Coord bx0, by0;
      Evas_Coord b0x, b0y;
      Evas_Coord b2x, b2y;

      struct
      {
         Evas_Coord x, y;
         double     timestamp, localtimestamp;
      } history[60];

      struct
      {
         double tadd, dxsum, dysum;
         double est_timestamp_diff;
      } hist;

      double          dragged_began_timestamp;
      double          anim_start;
      double          anim_start2;
      double          anim_start3;
      double          onhold_vx, onhold_vy, onhold_tlast,
                      onhold_vxe, onhold_vye;
      double          extra_time;

      Evas_Coord      hold_x, hold_y;
      Evas_Coord      locked_x, locked_y;
      int             hdir, vdir;

      Ecore_Idle_Enterer *hold_enterer;
      Ecore_Animator *hold_animator;
      Ecore_Animator *onhold_animator;
      Ecore_Animator *momentum_animator;
      Ecore_Animator *bounce_x_animator;
      Ecore_Animator *bounce_y_animator;

      Eina_Bool       bounce_x_hold : 1;
      Eina_Bool       bounce_y_hold : 1;
      Eina_Bool       dragged_began : 1;
      Eina_Bool       want_dragged : 1;
      Eina_Bool       hold_parent : 1;
      Eina_Bool       want_reset : 1;
      Eina_Bool       cancelled : 1;
      Eina_Bool       dragged : 1;
      Eina_Bool       locked : 1;
      Eina_Bool       scroll : 1;
      Eina_Bool       dir_x : 1;
      Eina_Bool       dir_y : 1;
      Eina_Bool       hold : 1;
      Eina_Bool       now : 1;
   } down;

   struct
   {
      Evas_Coord w, h;
      Eina_Bool  resized : 1;
   } content_info;

   struct
   {
      Evas_Coord x, y;
   } step, page, current_page;

   struct
   {
      Elm_Interface_Scrollable_Cb drag_start;
      Elm_Interface_Scrollable_Cb drag_stop;
      Elm_Interface_Scrollable_Cb animate_start;
      Elm_Interface_Scrollable_Cb animate_stop;
      Elm_Interface_Scrollable_Cb scroll;
      Elm_Interface_Scrollable_Cb scroll_left;
      Elm_Interface_Scrollable_Cb scroll_right;
      Elm_Interface_Scrollable_Cb scroll_up;
      Elm_Interface_Scrollable_Cb scroll_down;
      Elm_Interface_Scrollable_Cb edge_left;
      Elm_Interface_Scrollable_Cb edge_right;
      Elm_Interface_Scrollable_Cb edge_top;
      Elm_Interface_Scrollable_Cb edge_bottom;
      Elm_Interface_Scrollable_Cb vbar_drag;
      Elm_Interface_Scrollable_Cb vbar_press;
      Elm_Interface_Scrollable_Cb vbar_unpress;
      Elm_Interface_Scrollable_Cb hbar_drag;
      Elm_Interface_Scrollable_Cb hbar_press;
      Elm_Interface_Scrollable_Cb hbar_unpress;
      Elm_Interface_Scrollable_Cb page_change;

      Elm_Interface_Scrollable_Min_Limit_Cb content_min_limit;
      Elm_Interface_Scrollable_Resize_Cb content_viewport_resize;
   } cb_func;

   struct
   {
      struct
      {
         Evas_Coord      start, end;
         double          t_start, t_end;
         Ecore_Animator *animator;
      } x, y;
   } scrollto;

   double     pagerel_h, pagerel_v;
   Evas_Coord pagesize_h, pagesize_v;
   int        page_limit_h, page_limit_v;
   int        current_calc;

   unsigned char size_adjust_recurse;
   unsigned char size_count;
   Eina_Bool  size_adjust_recurse_abort : 1;

   Eina_Bool  momentum_animator_disabled : 1;
   Eina_Bool  bounce_animator_disabled : 1;
   Eina_Bool  page_snap_horiz : 1;
   Eina_Bool  page_snap_vert : 1;
   Eina_Bool  wheel_disabled : 1;
   Eina_Bool  hbar_visible : 1;
   Eina_Bool  vbar_visible : 1;
   Eina_Bool  bounce_horiz : 1;
   Eina_Bool  bounce_vert : 1;
   Eina_Bool  is_mirrored : 1;
   Eina_Bool  extern_pan : 1;
   Eina_Bool  bouncemey : 1;
   Eina_Bool  bouncemex : 1;
   Eina_Bool  freeze : 1;
   Eina_Bool  hold : 1;
   Eina_Bool  min_w : 1;
   Eina_Bool  min_h : 1;
   Eina_Bool  go_left : 1;
   Eina_Bool  go_right : 1;
   Eina_Bool  go_up : 1;
   Eina_Bool  go_down : 1;
};

#define ELM_SCROLLABLE_CHECK(obj, ...)                                       \
                                                                             \
  if (!eo_isa(obj, ELM_SCROLLABLE_INTERFACE))                    \
    {                                                                        \
       ERR("The object (%p) doesn't implement the Elementary scrollable"     \
            " interface", obj);                                              \
       if (getenv("ELM_ERROR_ABORT")) abort();                               \
       return __VA_ARGS__;                                                   \
    }

/**
 * @}
 */

#endif
