#ifndef ELM_WIDGET_CALENDAR_H
#define ELM_WIDGET_CALENDAR_H

#include "elm_widget_layout.h"

/**
 * @addtogroup Widget
 * @{
 *
 * @section elm-calendar-class The Elementary Calendar Class
 *
 * Elementary, besides having the @ref Calendar widget, exposes its
 * foundation -- the Elementary Calendar Class -- in order to create other
 * widgets which are a calendar with some more logic on top.
 */

/**
 * @def ELM_CALENDAR_CLASS
 *
 * Use this macro to cast whichever subclass of
 * #Elm_Calendar_Smart_Class into it, so to access its fields.
 *
 * @ingroup Widget
 */
#define ELM_CALENDAR_CLASS(x) ((Elm_Calendar_Smart_Class *)x)

/**
 * @def ELM_CALENDAR_DATA
 *
 * Use this macro to cast whichever subdata of
 * #Elm_Calendar_Smart_Data into it, so to access its fields.
 *
 * @ingroup Widget
 */
#define ELM_CALENDAR_DATA(x)  ((Elm_Calendar_Smart_Data *)x)

/**
 * @def ELM_CALENDAR_SMART_CLASS_VERSION
 *
 * Current version for Elementary calendar @b base smart class, a value
 * which goes to _Elm_Calendar_Smart_Class::version.
 *
 * @ingroup Widget
 */
#define ELM_CALENDAR_SMART_CLASS_VERSION 1

/**
 * @def ELM_CALENDAR_SMART_CLASS_INIT
 *
 * Initializer for a whole #Elm_Calendar_Smart_Class structure, with
 * @c NULL values on its specific fields.
 *
 * @param smart_class_init initializer to use for the "base" field
 * (#Evas_Smart_Class).
 *
 * @see EVAS_SMART_CLASS_INIT_NULL
 * @see EVAS_SMART_CLASS_INIT_NAME_VERSION
 * @see ELM_CALENDAR_SMART_CLASS_INIT_NULL
 * @see ELM_CALENDAR_SMART_CLASS_INIT_NAME_VERSION
 *
 * @ingroup Widget
 */
#define ELM_CALENDAR_SMART_CLASS_INIT(smart_class_init) \
  {smart_class_init, ELM_CALENDAR_SMART_CLASS_VERSION}

/**
 * @def ELM_CALENDAR_SMART_CLASS_INIT_NULL
 *
 * Initializer to zero out a whole #Elm_Calendar_Smart_Class structure.
 *
 * @see ELM_CALENDAR_SMART_CLASS_INIT_NAME_VERSION
 * @see ELM_CALENDAR_SMART_CLASS_INIT
 *
 * @ingroup Widget
 */
#define ELM_CALENDAR_SMART_CLASS_INIT_NULL \
  ELM_CALENDAR_SMART_CLASS_INIT(EVAS_SMART_CLASS_INIT_NULL)

/**
 * @def ELM_CALENDAR_SMART_CLASS_INIT_NAME_VERSION
 *
 * Initializer to zero out a whole #Elm_Calendar_Smart_Class structure and
 * set its name and version.
 *
 * This is similar to #ELM_CALENDAR_SMART_CLASS_INIT_NULL, but it will
 * also set the version field of #Elm_Calendar_Smart_Class (base field)
 * to the latest #ELM_CALENDAR_SMART_CLASS_VERSION and name it to the
 * specific value.
 *
 * It will keep a reference to the name field as a <c>"const char *"</c>,
 * i.e., the name must be available while the structure is
 * used (hint: static or global variable!) and must not be modified.
 *
 * @see ELM_CALENDAR_SMART_CLASS_INIT_NULL
 * @see ELM_CALENDAR_SMART_CLASS_INIT
 *
 * @ingroup Widget
 */
#define ELM_CALENDAR_SMART_CLASS_INIT_NAME_VERSION(name) \
  ELM_CALENDAR_SMART_CLASS_INIT                          \
    (ELM_LAYOUT_SMART_CLASS_INIT_NAME_VERSION(name))

/**
 * Elementary calendar base smart class. This inherits directly from
 * #Elm_Layout_Smart_Class and is meant to build widgets extending the
 * behavior of a calendar.
 *
 * All of the functions listed on @ref Calendar namespace will work for
 * objects deriving from #Elm_Calendar_Smart_Class.
 */
typedef struct _Elm_Calendar_Smart_Class
{
   Elm_Layout_Smart_Class base;

   int                    version;    /**< Version of this smart class definition */
} Elm_Calendar_Smart_Class;

/**
 * Base layout smart data extended with calendar instance data.
 */
typedef struct _Elm_Calendar_Smart_Data Elm_Calendar_Smart_Data;

typedef enum _Day_Color // EINA_DEPRECATED
{
   DAY_WEEKDAY = 0,
   DAY_SATURDAY = 1,
   DAY_SUNDAY = 2
} Day_Color;

struct _Elm_Calendar_Smart_Data
{
   Elm_Layout_Smart_Data    base;

   Eina_List               *marks;
   double                   interval, first_interval;
   int                      year_min, year_max, spin_speed;
   int                      today_it, selected_it, first_day_it;
   Elm_Calendar_Weekday     first_week_day;
   Ecore_Timer             *spin, *update_timer;
   Elm_Calendar_Format_Cb   format_func;
   const char              *weekdays[ELM_DAY_LAST];
   struct tm                current_time, selected_time, shown_time;
   Day_Color                day_color[42]; // EINA_DEPRECATED
   Elm_Calendar_Select_Mode select_mode;
   Eina_Bool                selected : 1;
};

struct _Elm_Calendar_Mark
{
   Evas_Object                  *obj;
   Eina_List                    *node;
   struct tm                     mark_time;
   const char                   *mark_type;
   Elm_Calendar_Mark_Repeat_Type repeat;
};

/**
 * @}
 */

EAPI extern const char ELM_CALENDAR_SMART_NAME[];
EAPI const Elm_Calendar_Smart_Class *elm_calendar_smart_class_get(void);

#define ELM_CALENDAR_DATA_GET(o, sd) \
  Elm_Calendar_Smart_Data * sd = evas_object_smart_data_get(o)

#define ELM_CALENDAR_DATA_GET_OR_RETURN(o, ptr)      \
  ELM_CALENDAR_DATA_GET(o, ptr);                     \
  if (!ptr)                                          \
    {                                                \
       CRITICAL("No widget data for object %p (%s)", \
                o, evas_object_type_get(o));         \
       return;                                       \
    }

#define ELM_CALENDAR_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  ELM_CALENDAR_DATA_GET(o, ptr);                         \
  if (!ptr)                                              \
    {                                                    \
       CRITICAL("No widget data for object %p (%s)",     \
                o, evas_object_type_get(o));             \
       return val;                                       \
    }

#define ELM_CALENDAR_CHECK(obj)                                      \
  if (!obj || !elm_widget_type_check((obj), ELM_CALENDAR_SMART_NAME, \
                                     __func__))                      \
    return

#endif
