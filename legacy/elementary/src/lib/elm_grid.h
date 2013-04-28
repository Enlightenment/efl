/**
 * @defgroup Grid Grid
 * @ingroup Elementary
 *
 * @image html grid_inheritance_tree.png
 * @image latex grid_inheritance_tree.eps
 *
 * The grid is a grid layout widget that lays out a series of children as a
 * fixed "grid" of widgets using a given percentage of the grid width and
 * height each using the child object.
 *
 * The Grid uses a "Virtual resolution" that is stretched to fill the grid
 * widgets size itself. The default is 100 x 100, so that means the
 * position and sizes of children will effectively be percentages (0 to 100)
 * of the width or height of the grid widget
 *
 * @{
 */

#ifdef EFL_EO_API_SUPPORT
#include <elm_grid_eo.h>
#endif
#ifndef EFL_NOLEGACY_API_SUPPORT
#include <elm_grid_legacy.h>
#endif

/**
 * @}
 */
