#ifndef EFL_UI_CLOCK_PRIVATE_H
#define EFL_UI_CLOCK_PRIVATE_H

#include "Elementary.h"

/* DO NOT USE THIS HEADER UNLESS YOU ARE PREPARED FOR BREAKING OF YOUR
 * CODE. THIS IS ELEMENTARY'S INTERNAL WIDGET API (for now) AND IS NOT
 * FINAL. CALL elm_widget_api_check(ELM_INTERNAL_API_VERSION) TO CHECK
 * IT AT RUNTIME.
 */

/**
 * @addtogroup Widget
 * @{
 *
 * @section efl-ui-clock-class The Elementary Clock Class
 *
 * Elementary, besides having the @ref Clock widget, exposes its
 * foundation -- the Elementary Clock Class -- in order to create other
 * widgets which are a Clock with some more logic on top.
 */

/**
 * Base layout smart data extended with Clock instance data.
 */
typedef struct _Efl_Ui_Clock_Module_Data Efl_Ui_Clock_Module_Data;
typedef struct _Efl_Ui_Clock_Data        Efl_Ui_Clock_Data;
typedef struct _Clock_Field              Clock_Field;
typedef struct _Clock_Mod_Api            Clock_Mod_Api;
typedef struct _Format_Map               Format_Map;

#define EFL_UI_CLOCK_TYPE_COUNT           8
#define EFL_UI_CLOCK_MAX_FORMAT_LEN       64
#define EFL_UI_CLOCK_MAX_FIELD_FORMAT_LEN 3

const char *PART_NAME_ARRAY[EFL_UI_CLOCK_TYPE_COUNT] = { "text",
                                                         "text",
                                                         "text",
                                                         "text",
                                                         "text",
                                                         "text",
                                                         "text",
                                                         "ampm"};

struct _Efl_Ui_Clock_Module_Data
{
   Evas_Object *base;
   void         (*field_limit_get)(Evas_Object *obj,
                                   Efl_Ui_Clock_Type field_type,
                                   int *range_min,
                                   int *range_max);
   const char  *(*field_format_get)(Evas_Object * obj,
                                    Efl_Ui_Clock_Type field_type);
};

struct _Clock_Field
{
   Evas_Object            *item_obj;
   char                    fmt[EFL_UI_CLOCK_MAX_FIELD_FORMAT_LEN];
   Efl_Ui_Clock_Type type;
   const char             *separator;
   int                     location;  /* location of the field as per
                                       * current format */
   int                     min, max;
   Eina_Bool               fmt_exist : 1;  /* whether field format is
                                            * present or not */
   Eina_Bool               visible : 1;  /* whether field can be
                                          * visible or not */
};

struct _Clock_Mod_Api
{
   Efl_Ui_Clock_Module_Data *(*obj_hook)(Evas_Object * obj);
   void                      (*obj_unhook)(Efl_Ui_Clock_Module_Data *mdata);
   void                      (*obj_hide)(Efl_Ui_Clock_Module_Data *mdata);
   Evas_Object              *(*field_create)(Efl_Ui_Clock_Module_Data * mdata,
                                             Efl_Ui_Clock_Type ftype);
   void                      (*field_value_display)(Efl_Ui_Clock_Module_Data
                                                    *mdata,
                                                    Evas_Object *obj);
};

struct _Efl_Ui_Clock_Data
{
   /* fixed set of fields. */
   Clock_Field            field_list[EFL_UI_CLOCK_TYPE_COUNT];
   struct tm                 curr_time, min_limit, max_limit;
   Efl_Ui_Clock_Module_Data *mod_data;
   char                      format[EFL_UI_CLOCK_MAX_FORMAT_LEN];
   Evas_Object              *access_obj;
   int                       enabled_field_count;
   Ecore_Timer              *ticker;
   Eina_Bool                 paused : 1;
   Eina_Bool                 edit_mode : 1;
   Eina_Bool                 user_format : 1;  /* whether user set
                                                * format or default
                                                * format. */
   Eina_Bool                 freeze_sizing : 1; /* freeze sizing_eval to
                                                 * reduce unnecessary sizing */
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

#define EFL_UI_CLOCK_DATA_GET(o, sd) \
  Efl_Ui_Clock_Data * sd = efl_data_scope_get(o, EFL_UI_CLOCK_CLASS)

#define EFL_UI_CLOCK_DATA_GET_OR_RETURN(o, ptr)      \
  EFL_UI_CLOCK_DATA_GET(o, ptr);                     \
  if (EINA_UNLIKELY(!ptr))                           \
    {                                                \
       ERR("No widget data for object %p (%s)",      \
           o, evas_object_type_get(o));              \
       return;                                       \
    }

#define EFL_UI_CLOCK_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  EFL_UI_CLOCK_DATA_GET(o, ptr);                         \
  if (EINA_UNLIKELY(!ptr))                               \
    {                                                    \
       ERR("No widget data for object %p (%s)",          \
           o, evas_object_type_get(o));                  \
       return val;                                       \
    }

#define EFL_UI_CLOCK_CHECK(obj)                              \
  if (EINA_UNLIKELY(!efl_isa((obj), EFL_UI_CLOCK_CLASS))) \
    return

#endif
