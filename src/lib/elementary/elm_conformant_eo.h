#ifndef _ELM_CONFORMANT_EO_H_
#define _ELM_CONFORMANT_EO_H_

#ifndef _ELM_CONFORMANT_EO_CLASS_TYPE
#define _ELM_CONFORMANT_EO_CLASS_TYPE

typedef Eo Elm_Conformant;

#endif

#ifndef _ELM_CONFORMANT_EO_TYPES
#define _ELM_CONFORMANT_EO_TYPES


#endif
/** Elementary conformant class
 *
 * @ingroup Elm_Conformant
 */
#define ELM_CONFORMANT_CLASS elm_conformant_class_get()

EWAPI const Efl_Class *elm_conformant_class_get(void);

EWAPI extern const Efl_Event_Description _ELM_CONFORMANT_EVENT_VIRTUALKEYPAD_STATE_ON;

/** Called when virtualkeypad was enabled
 *
 * @ingroup Elm_Conformant
 */
#define ELM_CONFORMANT_EVENT_VIRTUALKEYPAD_STATE_ON (&(_ELM_CONFORMANT_EVENT_VIRTUALKEYPAD_STATE_ON))

EWAPI extern const Efl_Event_Description _ELM_CONFORMANT_EVENT_VIRTUALKEYPAD_STATE_OFF;

/** Called when virtualkeypad was disabled
 *
 * @ingroup Elm_Conformant
 */
#define ELM_CONFORMANT_EVENT_VIRTUALKEYPAD_STATE_OFF (&(_ELM_CONFORMANT_EVENT_VIRTUALKEYPAD_STATE_OFF))

EWAPI extern const Efl_Event_Description _ELM_CONFORMANT_EVENT_CLIPBOARD_STATE_ON;

/** Called when clipboard was enabled
 *
 * @ingroup Elm_Conformant
 */
#define ELM_CONFORMANT_EVENT_CLIPBOARD_STATE_ON (&(_ELM_CONFORMANT_EVENT_CLIPBOARD_STATE_ON))

EWAPI extern const Efl_Event_Description _ELM_CONFORMANT_EVENT_CLIPBOARD_STATE_OFF;

/** Called when clipboard was disabled
 *
 * @ingroup Elm_Conformant
 */
#define ELM_CONFORMANT_EVENT_CLIPBOARD_STATE_OFF (&(_ELM_CONFORMANT_EVENT_CLIPBOARD_STATE_OFF))

#endif
