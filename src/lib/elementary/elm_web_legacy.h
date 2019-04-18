/** Types of zoom available.
 *
 * @ingroup Elm_Web
 */
typedef enum
{
  ELM_WEB_ZOOM_MODE_MANUAL = 0, /**< Zoom controlled normally by
                                 * elm_web_zoom_set */
  ELM_WEB_ZOOM_MODE_AUTO_FIT, /**< Zoom until content fits in web object */
  ELM_WEB_ZOOM_MODE_AUTO_FILL, /**< Zoom until content fills web object */
  ELM_WEB_ZOOM_MODE_LAST /**< Sentinel value to indicate last enum field during
                          * iteration */
} Elm_Web_Zoom_Mode;

/**
 * Add a new web object to the parent.
 *
 * @param parent The parent object.
 * @return The new object or @c NULL if it cannot be created.
 *
 * @see elm_web_url_set()
 * @see elm_web_webkit_view_get()
 *
 * @ingroup Elm_Web
 */
EAPI Evas_Object      *elm_web_add(Evas_Object *parent);

/**
 * Sets the URI for the web object
 *
 * It must be a full URI, with resource included, in the form
 * http://www.enlightenment.org or file:///tmp/something.html
 *
 * @param obj The web object
 * @param uri The URI to set
 * @return @c EINA_TRUE if the URI could be set, @c EINA_FALSE if an error occurred.
 *
 * @deprecated Use elm_web_url_set() instead
 *
 * @see elm_web_url_set()
 *
 * @ingroup Elm_Web
 */
EINA_DEPRECATED EAPI Eina_Bool         elm_web_uri_set(Evas_Object *obj, const char *uri);

/**
 * Get the current URI for the object
 *
 * The returned string must not be freed and is guaranteed to be
 * stringshared.
 *
 * @param obj The web object
 * @return A stringshared internal string with the current URI, or @c NULL on
 * failure
 *
 * @deprecated Use elm_web_url_get() instead
 *
 * @see elm_web_url_get()
 *
 * @ingroup Elm_Web
 */
EINA_DEPRECATED EAPI const char       *elm_web_uri_get(const Evas_Object *obj);


/**
 * @brief Sets the zoom mode to use.
 *
 * The modes can be any of those defined in .Elm_Web_Zoom_Mode, except
 * .ELM_WEB_ZOOM_MODE_LAST. The default is .ELM_WEB_ZOOM_MODE_MANUAL.
 *
 * .ELM_WEB_ZOOM_MODE_MANUAL means the zoom level will be controlled with the
 * @ref elm_web_zoom_set function.
 *
 * .ELM_WEB_ZOOM_MODE_AUTO_FIT will calculate the needed zoom level to make
 * sure the entirety of the web object's contents are shown.
 *
 * .ELM_WEB_ZOOM_MODE_AUTO_FILL will calculate the needed zoom level to fit the
 * contents in the web object's size, without leaving any space unused.
 *
 * @param[in] obj The object.
 * @param[in] mode The mode to set.
 *
 * @ingroup Elm_Web
 */
EAPI void elm_web_zoom_mode_set(Evas_Object *obj, Elm_Web_Zoom_Mode mode);

/**
 * @brief Get the currently set zoom mode.
 *
 * @param[in] obj The object.
 *
 * @return The mode to set.
 *
 * @ingroup Elm_Web
 */
EAPI Elm_Web_Zoom_Mode elm_web_zoom_mode_get(const Evas_Object *obj);

/**
 * @brief Sets the zoom level of the web object.
 *
 * Zoom level matches the Webkit API, so 1.0 means normal zoom, with higher
 * values meaning zoom in and lower meaning zoom out. This function will only
 * affect the zoom level if the mode set with @ref elm_web_zoom_mode_set is
 * .ELM_WEB_ZOOM_MODE_MANUAL.
 *
 * @param[in] obj The object.
 * @param[in] zoom The zoom level to set.
 *
 * @ingroup Elm_Web
 */
EAPI void elm_web_zoom_set(Evas_Object *obj, double zoom);

/**
 * @brief Get the current zoom level set on the web object
 *
 * Note that this is the zoom level set on the web object and not that of the
 * underlying Webkit one. In the .ELM_WEB_ZOOM_MODE_MANUAL mode, the two zoom
 * levels should match, but for the other two modes the Webkit zoom is
 * calculated internally to match the chosen mode without changing the zoom
 * level set for the web object.
 *
 * @param[in] obj The object.
 *
 * @return The zoom level to set.
 *
 * @ingroup Elm_Web
 */
EAPI double elm_web_zoom_get(const Evas_Object *obj);
#include "elm_web_eo.legacy.h"
