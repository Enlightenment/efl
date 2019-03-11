#ifndef _ELM_PLAYER_EO_H_
#define _ELM_PLAYER_EO_H_

#ifndef _ELM_PLAYER_EO_CLASS_TYPE
#define _ELM_PLAYER_EO_CLASS_TYPE

typedef Eo Elm_Player;

#endif

#ifndef _ELM_PLAYER_EO_TYPES
#define _ELM_PLAYER_EO_TYPES


#endif
/** Elementary player class
 *
 * @ingroup Elm_Player
 */
#define ELM_PLAYER_CLASS elm_player_class_get()

EWAPI const Efl_Class *elm_player_class_get(void);

EWAPI extern const Efl_Event_Description _ELM_PLAYER_EVENT_FORWARD_CLICKED;

/** Called when forward was clicked
 *
 * @ingroup Elm_Player
 */
#define ELM_PLAYER_EVENT_FORWARD_CLICKED (&(_ELM_PLAYER_EVENT_FORWARD_CLICKED))

EWAPI extern const Efl_Event_Description _ELM_PLAYER_EVENT_INFO_CLICKED;

/** Called when info was clicked
 *
 * @ingroup Elm_Player
 */
#define ELM_PLAYER_EVENT_INFO_CLICKED (&(_ELM_PLAYER_EVENT_INFO_CLICKED))

EWAPI extern const Efl_Event_Description _ELM_PLAYER_EVENT_NEXT_CLICKED;

/** Called when next was clicked
 *
 * @ingroup Elm_Player
 */
#define ELM_PLAYER_EVENT_NEXT_CLICKED (&(_ELM_PLAYER_EVENT_NEXT_CLICKED))

EWAPI extern const Efl_Event_Description _ELM_PLAYER_EVENT_PAUSE_CLICKED;

/** Called when pause was clicked
 *
 * @ingroup Elm_Player
 */
#define ELM_PLAYER_EVENT_PAUSE_CLICKED (&(_ELM_PLAYER_EVENT_PAUSE_CLICKED))

EWAPI extern const Efl_Event_Description _ELM_PLAYER_EVENT_PLAY_CLICKED;

/** Called when play was clicked
 *
 * @ingroup Elm_Player
 */
#define ELM_PLAYER_EVENT_PLAY_CLICKED (&(_ELM_PLAYER_EVENT_PLAY_CLICKED))

EWAPI extern const Efl_Event_Description _ELM_PLAYER_EVENT_PREV_CLICKED;

/** Called when previous was clicked
 *
 * @ingroup Elm_Player
 */
#define ELM_PLAYER_EVENT_PREV_CLICKED (&(_ELM_PLAYER_EVENT_PREV_CLICKED))

EWAPI extern const Efl_Event_Description _ELM_PLAYER_EVENT_REWIND_CLICKED;

/** Called when rewind was clicked
 *
 * @ingroup Elm_Player
 */
#define ELM_PLAYER_EVENT_REWIND_CLICKED (&(_ELM_PLAYER_EVENT_REWIND_CLICKED))

EWAPI extern const Efl_Event_Description _ELM_PLAYER_EVENT_QUALITY_CLICKED;

/** Called when quality was clicked
 *
 * @ingroup Elm_Player
 */
#define ELM_PLAYER_EVENT_QUALITY_CLICKED (&(_ELM_PLAYER_EVENT_QUALITY_CLICKED))

EWAPI extern const Efl_Event_Description _ELM_PLAYER_EVENT_EJECT_CLICKED;

/** Called when eject was clicked
 *
 * @ingroup Elm_Player
 */
#define ELM_PLAYER_EVENT_EJECT_CLICKED (&(_ELM_PLAYER_EVENT_EJECT_CLICKED))

EWAPI extern const Efl_Event_Description _ELM_PLAYER_EVENT_VOLUME_CLICKED;

/** Called when volume was clicked
 *
 * @ingroup Elm_Player
 */
#define ELM_PLAYER_EVENT_VOLUME_CLICKED (&(_ELM_PLAYER_EVENT_VOLUME_CLICKED))

EWAPI extern const Efl_Event_Description _ELM_PLAYER_EVENT_MUTE_CLICKED;

/** Called when mute was clicked
 *
 * @ingroup Elm_Player
 */
#define ELM_PLAYER_EVENT_MUTE_CLICKED (&(_ELM_PLAYER_EVENT_MUTE_CLICKED))

#endif
