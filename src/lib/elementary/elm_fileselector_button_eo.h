#ifndef _ELM_FILESELECTOR_BUTTON_EO_H_
#define _ELM_FILESELECTOR_BUTTON_EO_H_

#ifndef _ELM_FILESELECTOR_BUTTON_EO_CLASS_TYPE
#define _ELM_FILESELECTOR_BUTTON_EO_CLASS_TYPE

typedef Eo Elm_Fileselector_Button;

#endif

#ifndef _ELM_FILESELECTOR_BUTTON_EO_TYPES
#define _ELM_FILESELECTOR_BUTTON_EO_TYPES


#endif
/** Elementary fileselector button class
 *
 * @ingroup Elm_Fileselector_Button
 */
#define ELM_FILESELECTOR_BUTTON_CLASS elm_fileselector_button_class_get()

EWAPI const Efl_Class *elm_fileselector_button_class_get(void);

EWAPI extern const Efl_Event_Description _ELM_FILESELECTOR_BUTTON_EVENT_FILE_CHOSEN;

/** Called when a file was chosen in the fileselector
 *
 * @ingroup Elm_Fileselector_Button
 */
#define ELM_FILESELECTOR_BUTTON_EVENT_FILE_CHOSEN (&(_ELM_FILESELECTOR_BUTTON_EVENT_FILE_CHOSEN))

#endif
