#ifndef _ELM_FILESELECTOR_EO_H_
#define _ELM_FILESELECTOR_EO_H_

#ifndef _ELM_FILESELECTOR_EO_CLASS_TYPE
#define _ELM_FILESELECTOR_EO_CLASS_TYPE

typedef Eo Elm_Fileselector;

#endif

#ifndef _ELM_FILESELECTOR_EO_TYPES
#define _ELM_FILESELECTOR_EO_TYPES


#endif
/** Elementary fileselector class
 *
 * @ingroup Elm_Fileselector
 */
#define ELM_FILESELECTOR_CLASS elm_fileselector_class_get()

ELM_API ELM_API_WEAK const Efl_Class *elm_fileselector_class_get(void) EINA_CONST;

/**
 * @brief Enable/disable the "ok" and "cancel" buttons on a given file selector
 * widget
 *
 * @note A file selector without those buttons will never emit the "done" smart
 * event, and is only usable if one is just hooking to the other two events.
 *
 * See also @ref elm_obj_fileselector_buttons_ok_cancel_get.
 *
 * @param[in] obj The object.
 * @param[in] visible @c true to show buttons, @c false to hide.
 *
 * @ingroup Elm_Fileselector
 */
ELM_API ELM_API_WEAK void elm_obj_fileselector_buttons_ok_cancel_set(Eo *obj, Eina_Bool visible);

/**
 * @brief Get whether the "ok" and "cancel" buttons on a given file selector
 * widget are being shown.
 *
 * See also @ref elm_obj_fileselector_buttons_ok_cancel_set for more details.
 *
 * @param[in] obj The object.
 *
 * @return @c true to show buttons, @c false to hide.
 *
 * @ingroup Elm_Fileselector
 */
ELM_API ELM_API_WEAK Eina_Bool elm_obj_fileselector_buttons_ok_cancel_get(const Eo *obj);

ELM_API ELM_API_WEAK extern const Efl_Event_Description _ELM_FILESELECTOR_EVENT_DONE;

/** Called when OK button was pressed
 * @return const char *
 *
 * @ingroup Elm_Fileselector
 */
#define ELM_FILESELECTOR_EVENT_DONE (&(_ELM_FILESELECTOR_EVENT_DONE))

ELM_API ELM_API_WEAK extern const Efl_Event_Description _ELM_FILESELECTOR_EVENT_ACTIVATED;

/** Called when the filesector was activated
 *
 * @ingroup Elm_Fileselector
 */
#define ELM_FILESELECTOR_EVENT_ACTIVATED (&(_ELM_FILESELECTOR_EVENT_ACTIVATED))

ELM_API ELM_API_WEAK extern const Efl_Event_Description _ELM_FILESELECTOR_EVENT_SELECTED_INVALID;

/** Called when fileselector was selected
 *
 * @ingroup Elm_Fileselector
 */
#define ELM_FILESELECTOR_EVENT_SELECTED_INVALID (&(_ELM_FILESELECTOR_EVENT_SELECTED_INVALID))

ELM_API ELM_API_WEAK extern const Efl_Event_Description _ELM_FILESELECTOR_EVENT_DIRECTORY_OPEN;

/** Called when a directory was opened
 *
 * @ingroup Elm_Fileselector
 */
#define ELM_FILESELECTOR_EVENT_DIRECTORY_OPEN (&(_ELM_FILESELECTOR_EVENT_DIRECTORY_OPEN))

#endif
