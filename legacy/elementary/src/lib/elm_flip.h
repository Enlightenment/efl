/**
 * @defgroup Flip Flip
 * @ingroup Elementary
 *
 * @image html flip_inheritance_tree.png
 * @image latex flip_inheritance_tree.eps
 *
 * @image html img/widget/flip/preview-00.png
 * @image latex img/widget/flip/preview-00.eps
 *
 * This widget holds 2 content objects(Evas_Object): one on the front and one
 * on the back. It allows you to flip from front to back and vice-versa using
 * various animations.
 *
 * If either the front or back contents are not set the flip will treat that
 * as transparent. So if you wore to set the front content but not the back,
 * and then call elm_flip_go() you would see whatever is below the flip.
 *
 * For a list of supported animations see elm_flip_go().
 *
 * Signals that you can add callbacks for are:
 * "animate,begin" - when a flip animation was started
 * "animate,done" - when a flip animation is finished
 *
 * Default content parts of the flip widget that you can use for are:
 * @li "front" - A front content of the flip
 * @li "back" - A back content of the flip
 *
 * This widget inherits from @ref elm-container-class, so that the
 * functions meant to act on it will work for mapbuf objects:
 *
 * @li @ref elm_object_part_content_set
 * @li @ref elm_object_part_content_get
 * @li @ref elm_object_part_content_unset
 *
 * @ref tutorial_flip show how to use most of the API.
 *
 * @{
 */

#include "elm_flip_common.h"
#ifdef EFL_EO_API_SUPPORT
#include "elm_flip_eo.h"
#endif
#ifndef EFL_NOLEGACY_API_SUPPORT
#include "elm_flip_legacy.h"
#endif
/**
 * @}
 */
