/**
 * @defgroup Map Map
 * @ingroup Elementary
 *
 * @image html map_inheritance_tree.png
 * @image latex map_inheritance_tree.eps
 *
 * @image html img/widget/map/preview-00.png
 * @image latex img/widget/map/preview-00.eps
 *
 * This is a widget specifically for displaying a map. It uses basically
 * OpenStreetMap provider http://www.openstreetmap.org/,
 * but custom providers can be added.
 *
 * It supports some basic but yet nice features:
 * @li zooming and scrolling,
 * @li markers with content to be displayed when user clicks over them,
 * @li group of markers and
 * @li routes.
 *
 * This widget implements the @b @ref elm-scrollable-interface
 * interface, so that all (non-deprecated) functions for the base @ref
 * Scroller widget also work for map objects.
 *
 * Smart callbacks one can listen to:
 * - @c "clicked" - This is called when a user has clicked the map without
 *                  dragging around.
 * - @c "clicked,double" - This is called when a user has double-clicked
 *                         the map.
 * - @c "press" - This is called when a user has pressed down on the map.
 * - @c "longpressed" - This is called when a user has pressed down on the map
 *   @c for a long time without dragging around.
 * - @c "scroll" - the content has been scrolled (moved).
 * - @c "scroll,drag,start" - dragging the contents around has started.
 * - @c "scroll,drag,stop" - dragging the contents around has stopped.
 * - @c "scroll,anim,start" - scrolling animation has started.
 * - @c "scroll,anim,stop" - scrolling animation has stopped.
 * - @c "zoom,start" - Zoom animation started.
 * - @c "zoom,stop" - Zoom animation stopped.
 * - @c "zoom,change" - Zoom changed when using an auto zoom mode.
 * - @c "tile,load" - A map tile image load begins.
 * - @c "tile,loaded" -  A map tile image load ends.
 * - @c "tile,loaded,fail" -  A map tile image load fails.
 * - @c "route,load" - Route request begins.
 * - @c "route,loaded" - Route request ends.
 * - @c "route,loaded,fail" - Route request fails.
 * - @c "name,load" - Name request begins.
 * - @c "name,loaded" - Name request ends.
 * - @c "name,loaded,fail" - Name request fails.
 * - @c "overlay,clicked" - A overlay is clicked.
 * - @c "loaded" - when a map is finally loaded. (since 1.7)
 * - @c "language,changed" - the program's language changed
 * - @c "focused" - When the map has received focus. (since 1.8)
 * - @c "unfocused" - When the map has lost focus. (since 1.8)
 *
 * Available style for map widget:
 * - @c "default"
 *
 * Available style for markers:
 * - @c "radio"
 * - @c "radio2"
 * - @c "empty"
 *
 * Available style for marker bubble:
 * - @c "default"
 *
 * List of examples:
 * @li @ref map_example_01
 * @li @ref map_example_02
 * @li @ref map_example_03
 */

#include "elm_map_common.h"
#ifdef EFL_EO_API_SUPPORT
#include "elm_map_eo.h"
#endif
#ifndef EFL_NOLEGACY_API_SUPPORT
#include "elm_map_legacy.h"
#endif

/**
 * @}
 */
