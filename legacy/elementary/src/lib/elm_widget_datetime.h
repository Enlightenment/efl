#ifndef ELM_WIDGET_DATETIME_H
#define ELM_WIDGET_DATETIME_H

#include "elm_widget_layout.h"

/**
 * @addtogroup Widget
 * @{
 *
 * @section elm-datetime-class The Elementary Datetime Class
 *
 * Elementary, besides having the @ref Datetime widget, exposes its
 * foundation -- the Elementary Datetime Class -- in order to create other
 * widgets which are a datetime with some more logic on top.
 */

/**
 * @def ELM_DATETIME_CLASS
 *
 * Use this macro to cast whichever subclass of
 * #Elm_Datetime_Smart_Class into it, so to access its fields.
 *
 * @ingroup Widget
 */
#define ELM_DATETIME_CLASS(x) ((Elm_Datetime_Smart_Class *)x)

/**
 * @def ELM_DATETIME_DATA
 *
 * Use this macro to cast whichever subdata of
 * #Elm_Datetime_Smart_Data into it, so to access its fields.
 *
 * @ingroup Widget
 */
#define ELM_DATETIME_DATA(x)  ((Elm_Datetime_Smart_Data *)x)

/**
 * @def ELM_DATETIME_SMART_CLASS_VERSION
 *
 * Current version for Elementary datetime @b base smart class, a value
 * which goes to _Elm_Datetime_Smart_Class::version.
 *
 * @ingroup Widget
 */
#define ELM_DATETIME_SMART_CLASS_VERSION 1

/**
 * @def ELM_DATETIME_SMART_CLASS_INIT
 *
 * Initializer for a whole #Elm_Datetime_Smart_Class structure, with
 * @c NULL values on its specific fields.
 *
 * @param smart_class_init initializer to use for the "base" field
 * (#Evas_Smart_Class).
 *
 * @see EVAS_SMART_CLASS_INIT_NULL
 * @see EVAS_SMART_CLASS_INIT_NAME_VERSION
 * @see ELM_DATETIME_SMART_CLASS_INIT_NULL
 * @see ELM_DATETIME_SMART_CLASS_INIT_NAME_VERSION
 *
 * @ingroup Widget
 */
#define ELM_DATETIME_SMART_CLASS_INIT(smart_class_init) \
  {smart_class_init, ELM_DATETIME_SMART_CLASS_VERSION}

/**
 * @def ELM_DATETIME_SMART_CLASS_INIT_NULL
 *
 * Initializer to zero out a whole #Elm_Datetime_Smart_Class structure.
 *
 * @see ELM_DATETIME_SMART_CLASS_INIT_NAME_VERSION
 * @see ELM_DATETIME_SMART_CLASS_INIT
 *
 * @ingroup Widget
 */
#define ELM_DATETIME_SMART_CLASS_INIT_NULL \
  ELM_DATETIME_SMART_CLASS_INIT(EVAS_SMART_CLASS_INIT_NULL)

/**
 * @def ELM_DATETIME_SMART_CLASS_INIT_NAME_VERSION
 *
 * Initializer to zero out a whole #Elm_Datetime_Smart_Class structure and
 * set its name and version.
 *
 * This is similar to #ELM_DATETIME_SMART_CLASS_INIT_NULL, but it will
 * also set the version field of #Elm_Datetime_Smart_Class (base field)
 * to the latest #ELM_DATETIME_SMART_CLASS_VERSION and name it to the
 * specific value.
 *
 * It will keep a reference to the name field as a <c>"const char *"</c>,
 * i.e., the name must be available while the structure is
 * used (hint: static or global variable!) and must not be modified.
 *
 * @see ELM_DATETIME_SMART_CLASS_INIT_NULL
 * @see ELM_DATETIME_SMART_CLASS_INIT
 *
 * @ingroup Widget
 */
#define ELM_DATETIME_SMART_CLASS_INIT_NAME_VERSION(name) \
  ELM_DATETIME_SMART_CLASS_INIT                          \
    (ELM_LAYOUT_SMART_CLASS_INIT_NAME_VERSION(name))

/**
 * Elementary datetime base smart class. This inherits directly from
 * #Elm_Layout_Smart_Class and is meant to build widgets extending the
 * behavior of a datetime.
 *
 * All of the functions listed on @ref Datetime namespace will work for
 * objects deriving from #Elm_Datetime_Smart_Class.
 */
typedef struct _Elm_Datetime_Smart_Class
{
   Elm_Layout_Smart_Class base;

   int                    version;    /**< Version of this smart class definition */
} Elm_Datetime_Smart_Class;

/**
 * Base layout smart data extended with datetime instance data.
 */
typedef struct _Elm_Datetime_Smart_Data Elm_Datetime_Smart_Data;
typedef struct _Datetime_Field          Datetime_Field;
typedef struct _Datetime_Mod_Api        Datetime_Mod_Api;
typedef struct _Format_Map              Format_Map;

#define ELM_DATETIME_TYPE_COUNT           6
#define ELM_DATETIME_MAX_FORMAT_LEN       64
#define ELM_DATETIME_MAX_FIELD_FORMAT_LEN 3

struct _Datetime_Field
{
   Evas_Object            *item_obj;
   char                    fmt[ELM_DATETIME_MAX_FIELD_FORMAT_LEN];
   Elm_Datetime_Field_Type type;
   const char             *separator;
   int                     location;  /* location of the field as per
                                       * current format */
   int                     min, max;
   Eina_Bool               fmt_exist : 1;  /* whether field format is
                                            * present or not */
   Eina_Bool               visible : 1;  /* whether field can be
                                          * visible or not */
};

struct _Datetime_Mod_Api
{
   Elm_Datetime_Module_Data *(*obj_hook)(Evas_Object * obj);
   void                      (*obj_unhook)(Elm_Datetime_Module_Data *mdata);
   Evas_Object              *(*field_create)(Elm_Datetime_Module_Data * mdata,
                                             Elm_Datetime_Field_Type ftype);
   void                      (*field_value_display)(Elm_Datetime_Module_Data
                                                    *mdata,
                                                    Evas_Object *obj);
};

struct _Elm_Datetime_Smart_Data
{
   Elm_Layout_Smart_Data     base;

   /* fixed set of fields. */
   Datetime_Field            field_list[ELM_DATETIME_TYPE_COUNT];
   struct tm                 curr_time, min_limit, max_limit;
   Elm_Datetime_Module_Data *mod_data;
   char                      format[ELM_DATETIME_MAX_FORMAT_LEN];
   Evas_Object              *access_obj;
   Eina_Bool                 user_format : 1;  /* whether user set
                                                * format or default
                                                * format. */
};

struct _Format_Map
{
   char *fmt_char;
   int   def_min;
   int   def_max;
   char *ignore_sep;
};

/**
 * @}
 */

EAPI extern const char ELM_DATETIME_SMART_NAME[];
EAPI const Elm_Datetime_Smart_Class *elm_datetime_smart_class_get(void);

#define ELM_DATETIME_DATA_GET(o, sd) \
  Elm_Datetime_Smart_Data * sd = evas_object_smart_data_get(o)

#define ELM_DATETIME_DATA_GET_OR_RETURN(o, ptr)      \
  ELM_DATETIME_DATA_GET(o, ptr);                     \
  if (!ptr)                                          \
    {                                                \
       CRITICAL("No widget data for object %p (%s)", \
                o, evas_object_type_get(o));         \
       return;                                       \
    }

#define ELM_DATETIME_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  ELM_DATETIME_DATA_GET(o, ptr);                         \
  if (!ptr)                                              \
    {                                                    \
       CRITICAL("No widget data for object %p (%s)",     \
                o, evas_object_type_get(o));             \
       return val;                                       \
    }

#define ELM_DATETIME_CHECK(obj)                                      \
  if (!obj || !elm_widget_type_check((obj), ELM_DATETIME_SMART_NAME, \
                                     __func__))                      \
    return

#endif
