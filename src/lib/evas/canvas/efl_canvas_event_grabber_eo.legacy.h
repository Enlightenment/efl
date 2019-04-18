#ifndef _EFL_CANVAS_EVENT_GRABBER_EO_LEGACY_H_
#define _EFL_CANVAS_EVENT_GRABBER_EO_LEGACY_H_

#ifndef _EFL_CANVAS_EVENT_GRABBER_EO_CLASS_TYPE
#define _EFL_CANVAS_EVENT_GRABBER_EO_CLASS_TYPE

typedef Eo Efl_Canvas_Event_Grabber;

#endif

#ifndef _EFL_CANVAS_EVENT_GRABBER_EO_TYPES
#define _EFL_CANVAS_EVENT_GRABBER_EO_TYPES


#endif

/**
 * @brief Stops the grabber from updating its internal stacking order while
 * visible
 *
 * @param[in] obj The object.
 * @param[in] set If @c true, stop updating
 *
 * @since 1.20
 *
 * @ingroup Evas_Object_Event_Grabber_Group
 */
EAPI void evas_object_event_grabber_freeze_when_visible_set(Efl_Canvas_Event_Grabber *obj, Eina_Bool set);

/**
 * @brief Stops the grabber from updating its internal stacking order while
 * visible
 *
 * @param[in] obj The object.
 *
 * @return If @c true, stop updating
 *
 * @since 1.20
 *
 * @ingroup Evas_Object_Event_Grabber_Group
 */
EAPI Eina_Bool evas_object_event_grabber_freeze_when_visible_get(const Efl_Canvas_Event_Grabber *obj);

#endif
