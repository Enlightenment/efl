#ifndef _ELM_ICON_EO_H_
#define _ELM_ICON_EO_H_

#ifndef _ELM_ICON_EO_CLASS_TYPE
#define _ELM_ICON_EO_CLASS_TYPE

typedef Eo Elm_Icon;

#endif

#ifndef _ELM_ICON_EO_TYPES
#define _ELM_ICON_EO_TYPES


#endif
/** Elementary Icon Class
 *
 * @ingroup Elm_Icon
 */
#define ELM_ICON_CLASS elm_icon_class_get()

EWAPI const Efl_Class *elm_icon_class_get(void);

EWAPI extern const Efl_Event_Description _ELM_ICON_EVENT_THUMB_DONE;

/** Called when thumb nailing is successfully done
 *
 * @ingroup Elm_Icon
 */
#define ELM_ICON_EVENT_THUMB_DONE (&(_ELM_ICON_EVENT_THUMB_DONE))

EWAPI extern const Efl_Event_Description _ELM_ICON_EVENT_THUMB_ERROR;

/** Called when thumb nailing failed
 *
 * @ingroup Elm_Icon
 */
#define ELM_ICON_EVENT_THUMB_ERROR (&(_ELM_ICON_EVENT_THUMB_ERROR))

#endif
