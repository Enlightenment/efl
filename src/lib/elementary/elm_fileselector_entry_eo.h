#ifndef _ELM_FILESELECTOR_ENTRY_EO_H_
#define _ELM_FILESELECTOR_ENTRY_EO_H_

#ifndef _ELM_FILESELECTOR_ENTRY_EO_CLASS_TYPE
#define _ELM_FILESELECTOR_ENTRY_EO_CLASS_TYPE

typedef Eo Elm_Fileselector_Entry;

#endif

#ifndef _ELM_FILESELECTOR_ENTRY_EO_TYPES
#define _ELM_FILESELECTOR_ENTRY_EO_TYPES


#endif
/** Elementary fileselector entry class
 *
 * @ingroup Elm_Fileselector_Entry
 */
#define ELM_FILESELECTOR_ENTRY_CLASS elm_fileselector_entry_class_get()

EWAPI const Efl_Class *elm_fileselector_entry_class_get(void);

EWAPI extern const Efl_Event_Description _ELM_FILESELECTOR_ENTRY_EVENT_CHANGED;

/** Called when the entry changed
 *
 * @ingroup Elm_Fileselector_Entry
 */
#define ELM_FILESELECTOR_ENTRY_EVENT_CHANGED (&(_ELM_FILESELECTOR_ENTRY_EVENT_CHANGED))

EWAPI extern const Efl_Event_Description _ELM_FILESELECTOR_ENTRY_EVENT_ACTIVATED;

/** Called when the entry was activated
 *
 * @ingroup Elm_Fileselector_Entry
 */
#define ELM_FILESELECTOR_ENTRY_EVENT_ACTIVATED (&(_ELM_FILESELECTOR_ENTRY_EVENT_ACTIVATED))

EWAPI extern const Efl_Event_Description _ELM_FILESELECTOR_ENTRY_EVENT_FILE_CHOSEN;

/** Called when a file was chosen in the fileselector
 *
 * @ingroup Elm_Fileselector_Entry
 */
#define ELM_FILESELECTOR_ENTRY_EVENT_FILE_CHOSEN (&(_ELM_FILESELECTOR_ENTRY_EVENT_FILE_CHOSEN))

EWAPI extern const Efl_Event_Description _ELM_FILESELECTOR_ENTRY_EVENT_PRESS;

/** Called when entry was pressed
 *
 * @ingroup Elm_Fileselector_Entry
 */
#define ELM_FILESELECTOR_ENTRY_EVENT_PRESS (&(_ELM_FILESELECTOR_ENTRY_EVENT_PRESS))

#endif
