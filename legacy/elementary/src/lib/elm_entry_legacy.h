/**
 * This adds an entry to @p parent object.
 *
 * By default, entries are:
 * @li not scrolled
 * @li multi-line
 * @li word wrapped
 * @li autosave is enabled
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * @ingroup Entry
 */
EAPI Evas_Object       *elm_entry_add(Evas_Object *parent);

/**
 * This sets the text displayed within the entry to @p entry.
 *
 * @param obj The entry object
 * @param entry The text to be displayed
 *
 * @note Using this function bypasses text filters
 *
 * @ingroup Entry
 */
EAPI void               elm_entry_entry_set(Evas_Object *obj, const char *entry);

/**
 * This returns the text currently shown in object @p entry.
 * See also elm_entry_entry_set().
 *
 * @param obj The entry object
 * @return The currently displayed text or NULL on failure
 *
 * @ingroup Entry
 */
EAPI const char        *elm_entry_entry_get(const Evas_Object *obj);

#include "elm_entry.eo.legacy.h"
