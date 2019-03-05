#ifndef _ELM_THUMB_EO_H_
#define _ELM_THUMB_EO_H_

#ifndef _ELM_THUMB_EO_CLASS_TYPE
#define _ELM_THUMB_EO_CLASS_TYPE

typedef Eo Elm_Thumb;

#endif

#ifndef _ELM_THUMB_EO_TYPES
#define _ELM_THUMB_EO_TYPES


#endif
/** Elementary thumbnail class
 *
 * @ingroup Elm_Thumb
 */
#define ELM_THUMB_CLASS elm_thumb_class_get()

EWAPI const Efl_Class *elm_thumb_class_get(void);

EWAPI extern const Efl_Event_Description _ELM_THUMB_EVENT_GENERATE_ERROR;

/** Called when an error occurred during thumbnail generation
 *
 * @ingroup Elm_Thumb
 */
#define ELM_THUMB_EVENT_GENERATE_ERROR (&(_ELM_THUMB_EVENT_GENERATE_ERROR))

EWAPI extern const Efl_Event_Description _ELM_THUMB_EVENT_GENERATE_START;

/** Called when thumbnail generation started
 *
 * @ingroup Elm_Thumb
 */
#define ELM_THUMB_EVENT_GENERATE_START (&(_ELM_THUMB_EVENT_GENERATE_START))

EWAPI extern const Efl_Event_Description _ELM_THUMB_EVENT_GENERATE_STOP;

/** Called when thumbnail generation stopped
 *
 * @ingroup Elm_Thumb
 */
#define ELM_THUMB_EVENT_GENERATE_STOP (&(_ELM_THUMB_EVENT_GENERATE_STOP))

EWAPI extern const Efl_Event_Description _ELM_THUMB_EVENT_LOAD_ERROR;

/** Called when an error occurred during loading
 *
 * @ingroup Elm_Thumb
 */
#define ELM_THUMB_EVENT_LOAD_ERROR (&(_ELM_THUMB_EVENT_LOAD_ERROR))

EWAPI extern const Efl_Event_Description _ELM_THUMB_EVENT_PRESS;

/** Called when pressed
 *
 * @ingroup Elm_Thumb
 */
#define ELM_THUMB_EVENT_PRESS (&(_ELM_THUMB_EVENT_PRESS))

#endif
