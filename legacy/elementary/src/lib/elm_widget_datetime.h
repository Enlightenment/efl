#ifndef ELM_WIDGET_DATETIME_H
#define ELM_WIDGET_DATETIME_H

#include "Elementary.h"

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
   void                      (*obj_hide)(Elm_Datetime_Module_Data *mdata);
   Evas_Object              *(*field_create)(Elm_Datetime_Module_Data * mdata,
                                             Elm_Datetime_Field_Type ftype);
   void                      (*field_value_display)(Elm_Datetime_Module_Data
                                                    *mdata,
                                                    Evas_Object *obj);
};

struct _Elm_Datetime_Smart_Data
{
   /* fixed set of fields. */
   Datetime_Field            field_list[ELM_DATETIME_TYPE_COUNT];
   struct tm                 curr_time, min_limit, max_limit;
   Elm_Datetime_Module_Data *mod_data;
   char                      format[ELM_DATETIME_MAX_FORMAT_LEN];
   Evas_Object              *access_obj;
   int                       enabled_field_count;
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

#define ELM_DATETIME_DATA_GET(o, sd) \
  Elm_Datetime_Smart_Data * sd = eo_data_scope_get(o, ELM_OBJ_DATETIME_CLASS)

#define ELM_DATETIME_DATA_GET_OR_RETURN(o, ptr)      \
  ELM_DATETIME_DATA_GET(o, ptr);                     \
  if (EINA_UNLIKELY(!ptr))                           \
    {                                                \
       CRI("No widget data for object %p (%s)",      \
           o, evas_object_type_get(o));              \
       return;                                       \
    }

#define ELM_DATETIME_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  ELM_DATETIME_DATA_GET(o, ptr);                         \
  if (EINA_UNLIKELY(!ptr))                               \
    {                                                    \
       CRI("No widget data for object %p (%s)",          \
           o, evas_object_type_get(o));                  \
       return val;                                       \
    }

#define ELM_DATETIME_CHECK(obj)                              \
  if (EINA_UNLIKELY(!eo_isa((obj), ELM_OBJ_DATETIME_CLASS))) \
    return

#endif
