/**
 * @defgroup Notify Notify
 * @ingroup Elementary
 *
 * @image html notify_inheritance_tree.png
 * @image latex notify_inheritance_tree.eps
 *
 * @image html img/widget/notify/preview-00.png
 * @image latex img/widget/notify/preview-00.eps
 *
 * Display a container in a particular region of the parent(top, bottom,
 * etc).  A timeout can be set to automatically hide the notify. This is so
 * that, after an evas_object_show() on a notify object, if a timeout was set
 * on it, it will @b automatically get hidden after that time.
 *
 * Signals that you can add callbacks for are:
 * @li "timeout" - when timeout happens on notify and it's hidden
 * @li "block,clicked" - when a click outside of the notify happens
 *
 * This widget inherits from @ref elm-container-class, so that the
 * functions meant to act on it will wor work for mapbuf objects:
 *
 * @li @ref elm_object_part_content_set
 * @li @ref elm_object_part_content_get
 * @li @ref elm_object_part_content_unset
 *
 * Default content parts of the notify widget that you can use are:
 * @li @c "default" - The main content of the notify
 *
 * @ref tutorial_notify show usage of the API.
 *
 * @{
 */

#include <elm_notify_common.h>
#ifdef EFL_EO_API_SUPPORT
#include <elm_notify_eo.h>
#endif
#ifndef EFL_NOLEGACY_API_SUPPORT
#include <elm_notify_legacy.h>
#endif

/**
 * @}
 */
