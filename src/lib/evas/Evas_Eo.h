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

#include "canvas/evas_signal_interface.eo.h"
#include "canvas/evas_draggable_interface.eo.h"
#include "canvas/evas_clickable_interface.eo.h"
#include "canvas/evas_scrollable_interface.eo.h"
#include "canvas/evas_selectable_interface.eo.h"
#include "canvas/evas_zoomable_interface.eo.h"

// Interface classes links
#define EVAS_SMART_SIGNAL_INTERFACE     EVAS_SIGNAL_INTERFACE_CLASS
#define EVAS_SMART_CLICKABLE_INTERFACE  EVAS_CLICKABLE_INTERFACE_CLASS
#define EVAS_SMART_SCROLLABLE_INTERFACE EVAS_SCROLLABLE_INTERFACE_CLASS
#define EVAS_SMART_DRAGGABLE_INTERFACE  EVAS_DRAGGABLE_INTERFACE_CLASS
#define EVAS_SMART_SELECTABLE_INTERFACE EVAS_SELECTABLE_INTERFACE_CLASS
#define EVAS_SMART_ZOOMABLE_INTERFACE   EVAS_ZOOMABLE_INTERFACE_CLASS

#include "canvas/evas.eo.h"
#define EVAS_CLASS EVAS_CANVAS_CLASS

/**
 * @ingroup Evas_Object_Rectangle
 *
 * @{
 */
#include "canvas/evas_rectangle.eo.h"
/**
 * @}
 */

/**
 * @ingroup Evas_Object_Text
 *
 * @{
 */
#include "canvas/evas_text.eo.h"
/**
 * @}
 */

/**
 * @ingroup Evas_Object_Textblock
 *
 * @{
 */
#include "canvas/evas_textblock.eo.h"
/**
 * @}
 */

/**
 * @ingroup Evas_Object_Textgrid
 *
 * @{
 */
#include "canvas/evas_textgrid.eo.h"
/**
 * @}
 */

/**
 * @ingroup Evas_Object_Line
 *
 * @{
 */
#include "canvas/evas_line.eo.h"
/**
 * @}
 */

/**
 * @ingroup Evas_Object_Polygon
 *
 * @{
 */
#include "canvas/evas_polygon.eo.h"
/**
 * @}
 */

/**
 * @ingroup Evas_Smart_Object_Group
 *
 * @{
 */
#include "canvas/evas_smart.eo.h"
/**
 * @}
 */

/**
 * @ingroup Evas_Smart_Object_Clipped
 *
 * @{
 */
#include "canvas/evas_smart_clipped.eo.h"
/**
 * @}
 */

/**
 * @ingroup Evas_Object_Box
 *
 * @{
 */
#include "canvas/evas_box.eo.h"
/**
 * @}
 */

/**
 * @ingroup Evas_Object_Table
 *
 * @{
 */
#include "canvas/evas_table.eo.h"
/**
 * @}
 */

/**
 * @ingroup Evas_Object_Grid
 *
 * @{
 */
#include "canvas/evas_grid.eo.h"
/**
 * @}
 */

#include "canvas/evas_common_interface.eo.h"

#include "canvas/evas_object.eo.h"

#include "canvas/evas_image.eo.h"

#include "canvas/evas_out.eo.h"

#include "canvas/evas_3d_camera.eo.h"

#include "canvas/evas_3d_texture.eo.h"

#include "canvas/evas_3d_material.eo.h"

#include "canvas/evas_3d_light.eo.h"

#include "canvas/evas_3d_mesh.eo.h"

#include "canvas/evas_3d_node.eo.h"

#include "canvas/evas_3d_scene.eo.h"
