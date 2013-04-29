/**
 * @defgroup Mapbuf Mapbuf
 * @ingroup Elementary
 *
 * @image html mapbuf_inheritance_tree.png
 * @image latex mapbuf_inheritance_tree.eps
 *
 * @image html img/widget/mapbuf/preview-00.png
 * @image latex img/widget/mapbuf/preview-00.eps width=\textwidth
 *
 * This holds one content object and uses an Evas Map of transformation
 * points to be later used with this content. So the content will be
 * moved, resized, etc as a single image. So it will improve performance
 * when you have a complex interface, with a lot of elements, and will
 * need to resize or move it frequently (the content object and its
 * children).
 *
 * This widget inherits from @ref elm-container-class, so that the
 * functions meant to act on it will work for mapbuf objects:
 *
 * @li @ref elm_object_part_content_set
 * @li @ref elm_object_part_content_get
 * @li @ref elm_object_part_content_unset
 *
 * Default content parts of the mapbuf widget that you can use are:
 * @li "default" - The main content of the mapbuf
 *
 * To enable map, elm_mapbuf_enabled_set() should be used.
 *
 * See how to use this widget in this example:
 * @ref mapbuf_example
 */

/**
 * @addtogroup Mapbuf
 * @{
 */

#ifdef EFL_EO_API_SUPPORT
#include <elm_mapbuf_eo.h>
#endif
#ifndef EFL_NOLEGACY_API_SUPPORT
#include <elm_mapbuf_legacy.h>
#endif

/**
 * @}
 */
