#ifndef _ELM_FILESELECTOR_EO_LEGACY_H_
#define _ELM_FILESELECTOR_EO_LEGACY_H_

#ifndef _ELM_FILESELECTOR_EO_CLASS_TYPE
#define _ELM_FILESELECTOR_EO_CLASS_TYPE

typedef Eo Elm_Fileselector;

#endif

#ifndef _ELM_FILESELECTOR_EO_TYPES
#define _ELM_FILESELECTOR_EO_TYPES


#endif

/**
 * @brief Enable/disable the "ok" and "cancel" buttons on a given file selector
 * widget
 *
 * @note A file selector without those buttons will never emit the "done" smart
 * event, and is only usable if one is just hooking to the other two events.
 *
 * See also @ref elm_fileselector_buttons_ok_cancel_get.
 *
 * @param[in] obj The object.
 * @param[in] visible @c true to show buttons, @c false to hide.
 *
 * @ingroup Elm_Fileselector_Group
 */
EAPI void elm_fileselector_buttons_ok_cancel_set(Elm_Fileselector *obj, Eina_Bool visible);

/**
 * @brief Get whether the "ok" and "cancel" buttons on a given file selector
 * widget are being shown.
 *
 * See also @ref elm_fileselector_buttons_ok_cancel_set for more details.
 *
 * @param[in] obj The object.
 *
 * @return @c true to show buttons, @c false to hide.
 *
 * @ingroup Elm_Fileselector_Group
 */
EAPI Eina_Bool elm_fileselector_buttons_ok_cancel_get(const Elm_Fileselector *obj);

#endif
