/**
 * @defgroup Icon Icon
 * @ingroup Elementary
 *
 * @image html icon_inheritance_tree.png
 * @image latex icon_inheritance_tree.eps
 *
 * @image html img/widget/icon/preview-00.png
 * @image latex img/widget/icon/preview-00.eps
 *
 * An icon object is used to display standard icon images ("delete",
 * "edit", "arrows", etc.) or images coming from a custom file (PNG,
 * JPG, EDJE, etc.), on icon contexts.
 *
 * The icon image requested can be in the Elementary theme in use, or
 * in the @c freedesktop.org theme paths. It's possible to set the
 * order of preference from where an image will be fetched.
 *
 * This widget inherits from the @ref Image one, so that all the
 * functions acting on it also work for icon objects.
 *
 * You should be using an icon, instead of an image, whenever one of
 * the following apply:
 * - you need a @b thumbnail version of an original image
 * - you need freedesktop.org provided icon images
 * - you need theme provided icon images (Edje groups)
 *
 * Various calls on the icon's API are marked as @b deprecated, as
 * they just wrap the image counterpart functions. Use the ones we
 * point you to, for each case of deprecation here, instead --
 * eventually the deprecated ones will be discarded (next major
 * release).
 *
 * Default images provided by Elementary's default theme are described
 * below.
 *
 * These are names for icons that were first intended to be used in
 * toolbars, but can be used in many other places too:
 * @li @c "home"
 * @li @c "close"
 * @li @c "apps"
 * @li @c "arrow_up"
 * @li @c "arrow_down"
 * @li @c "arrow_left"
 * @li @c "arrow_right"
 * @li @c "chat"
 * @li @c "clock"
 * @li @c "delete"
 * @li @c "edit"
 * @li @c "refresh"
 * @li @c "folder"
 * @li @c "file"
 *
 * These are names for icons that were designed to be used in menus
 * (but again, you can use them anywhere else):
 * @li @c "menu/home"
 * @li @c "menu/close"
 * @li @c "menu/apps"
 * @li @c "menu/arrow_up"
 * @li @c "menu/arrow_down"
 * @li @c "menu/arrow_left"
 * @li @c "menu/arrow_right"
 * @li @c "menu/chat"
 * @li @c "menu/clock"
 * @li @c "menu/delete"
 * @li @c "menu/edit"
 * @li @c "menu/refresh"
 * @li @c "menu/folder"
 * @li @c "menu/file"
 *
 * And these are names for some media player specific icons:
 * @li @c "media_player/forward"
 * @li @c "media_player/info"
 * @li @c "media_player/next"
 * @li @c "media_player/pause"
 * @li @c "media_player/play"
 * @li @c "media_player/prev"
 * @li @c "media_player/rewind"
 * @li @c "media_player/stop"
 *
 * This widget emits the following signals, besides the ones sent from
 * @ref Image:
 * - @c "thumb,done" - elm_icon_thumb_set() has completed with success
 *                     (since 1.7)
 * - @c "thumb,error" - elm_icon_thumb_set() has failed (since 1.7)
 *
 * Elementary icon objects support the following API calls:
 * @li elm_object_signal_emit()
 * @li elm_object_signal_callback_add()
 * @li elm_object_signal_callback_del()
 * for emmiting and listening to signals on the object, when the
 * internal image comes from an Edje object. This behavior was added
 * unintentionally, though, and is @b deprecated. Expect it to be
 * dropped on future releases.
 *
 * An example of usage for this API follows:
 * @li @ref tutorial_icon
 */

#include "elm_icon_common.h"
#ifdef EFL_EO_API_SUPPORT
#include "elm_icon_eo.h"
#endif
#ifndef EFL_NOLEGACY_API_SUPPORT
#include "elm_icon_legacy.h"
#endif
/**
 * @}
 */
