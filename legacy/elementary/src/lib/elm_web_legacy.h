/**
 * Add a new web object to the parent.
 *
 * @param parent The parent object.
 * @return The new object or @c NULL if it cannot be created.
 *
 * @see elm_web_url_set()
 * @see elm_web_webkit_view_get()
 *
 * @ingroup Web
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
 * @ingroup Web
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
 * @ingroup Web
 */
EINA_DEPRECATED EAPI const char       *elm_web_uri_get(const Evas_Object *obj);

#include "elm_web.eo.legacy.h"