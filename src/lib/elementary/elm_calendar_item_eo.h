#ifndef _ELM_CALENDAR_ITEM_EO_H_
#define _ELM_CALENDAR_ITEM_EO_H_

#ifndef _ELM_CALENDAR_ITEM_EO_CLASS_TYPE
#define _ELM_CALENDAR_ITEM_EO_CLASS_TYPE

typedef Eo Elm_Calendar_Item;

#endif

#ifndef _ELM_CALENDAR_ITEM_EO_TYPES
#define _ELM_CALENDAR_ITEM_EO_TYPES


#endif
/** Elm Calendar Item class
 *
 * @ingroup Elm_Calendar_Item
 */
#define ELM_CALENDAR_ITEM_CLASS elm_calendar_item_class_get()

EWAPI const Efl_Class *elm_calendar_item_class_get(void);

/**
 * @brief Day number property
 *
 * @param[in] obj The object.
 * @param[in] i Day number
 *
 * @ingroup Elm_Calendar_Item
 */
EOAPI void elm_calendar_item_day_number_set(Eo *obj, int i);

/**
 * @brief Day number property
 *
 * @param[in] obj The object.
 *
 * @return Day number
 *
 * @ingroup Elm_Calendar_Item
 */
EOAPI int elm_calendar_item_day_number_get(const Eo *obj);

#endif
