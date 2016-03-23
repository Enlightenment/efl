/**
 * @defgroup Plug Plug
 * @ingroup Elementary
 *
 * @image html plug_inheritance_tree.png
 * @image latex plug_inheritance_tree.eps
 *
 * An object that allows one to show an image which other process created.
 * It can be used anywhere like any other elementary widget.
 *
 * This widget emits the following signals:
 * @li "clicked": the user clicked the image (press/release). The @c
 *     event parameter of the callback will be @c NULL.
 * @li "image,deleted": the server side was deleted. The @c event
 *     parameter of the callback will be @c NULL.
 * @li "image,resized": the server side was resized. The @c event parameter of
 *     the callback will be @c Evas_Coord_Size (two integers).
 *
 * @note the event "image,resized" will be sent whenever the server
 *       resized its image and this @b always happen on the first
 *       time. Then it can be used to track when the server-side image
 *       is fully known (client connected to server, retrieved its
 *       image buffer through shared memory and resized the evas
 *       object).
 *
 */


/**
 * @addtogroup Plug
 * @{
 */

#ifdef EFL_EO_API_SUPPORT
#include "elm_plug_eo.h"
#endif
#ifndef EFL_NOLEGACY_API_SUPPORT
#include "elm_plug_legacy.h"
#endif
/**
 * @}
 */
