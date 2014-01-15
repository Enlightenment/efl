#ifndef ELM_WIDGET_HOVER_H
#define ELM_WIDGET_HOVER_H

#include "Elementary.h"
#include "elm_widget_layout.h"

/**
 * @addtogroup Widget
 * @{
 *
 * @section elm-hover-class The Elementary Hover Class
 *
 * Elementary, besides having the @ref Hover widget, exposes its
 * foundation -- the Elementary Hover Class -- in order to create other
 * widgets which are a hover with some more logic on top.
 */

/**
 * Base widget smart data extended with hover instance data.
 */
typedef struct _Elm_Hover_Smart_Data Elm_Hover_Smart_Data;
typedef struct _Content_Info         Content_Info;

struct _Content_Info
{
   const char  *swallow;
   Evas_Object *obj;
};

/* WARNING: sync size with actual declaration, always */
extern const Elm_Layout_Part_Alias_Description _content_aliases[10];

struct _Elm_Hover_Smart_Data
{
   Evas_Object          *offset, *size;
   Evas_Object          *parent, *target;

   Content_Info         *smt_sub;  /* 'smart placement' sub object */
   Content_Info          subs[sizeof(_content_aliases) /
                              sizeof(_content_aliases[0]) - 1];

   Eina_Bool             on_del : 1;
};

/**
 * @}
 */

#define ELM_HOVER_DATA_GET(o, sd) \
  Elm_Hover_Smart_Data * sd = eo_data_scope_get(o, ELM_OBJ_HOVER_CLASS)

#define ELM_HOVER_DATA_GET_OR_RETURN(o, ptr)         \
  ELM_HOVER_DATA_GET(o, ptr);                        \
  if (EINA_UNLIKELY(!ptr))                           \
    {                                                \
       CRI("No widget data for object %p (%s)",      \
           o, evas_object_type_get(o));              \
       return;                                       \
    }

#define ELM_HOVER_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  ELM_HOVER_DATA_GET(o, ptr);                         \
  if (EINA_UNLIKELY(!ptr))                            \
    {                                                 \
       CRI("No widget data for object %p (%s)",       \
           o, evas_object_type_get(o));               \
       return val;                                    \
    }

#define ELM_HOVER_CHECK(obj)                              \
  if (EINA_UNLIKELY(!eo_isa((obj), ELM_OBJ_HOVER_CLASS))) \
    return

#endif
