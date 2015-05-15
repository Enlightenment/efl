/**
 * @defgroup Clipper Clipper
 * @ingroup Elementary
 *
 * This widget displays a clipped(masked) object.
 * For this widget, you need one clipper and one content objects.
 * The content would be clipped out by the clipper.
 * If clipper is not set, you would see the content without cuts
 *
 * The Evas_Object_Rectangle and the Evas_Object_Image object can be a clipper.
 * A content can have only a clipper even if user set some objects as the clipper.
 * 
 * This widget can display a clipped object into many shapes according to the clipper.
 * The shape of the clipped object be decided by pixel's transparency of the clipper object.
 * If you want to cut out the content into a circle,
 * the clipper's pixel should have non-zero as alpha value into the circle.
 * In case of outside of the circle, should have zero as alpha value.
 *
 * This widget inherits from @ref elm-container-class,
 * So you can use are:
 *
 * @li @ref elm_object_part_content_set
 * @li @ref elm_object_part_content_get
 * @li @ref elm_object_part_content_unset
 *
 * Default content parts of the clipper widget that you can use are:
 * @li @c "default" - The content to be clipped off
 *
 * @{
 */

#ifdef EFL_EO_API_SUPPORT
#include <elm_clipper_eo.h>
#endif
#ifndef EFL_NOLEGACY_API_SUPPORT
#include <elm_clipper_legacy.h>
#endif

/**
 * @}
 */
