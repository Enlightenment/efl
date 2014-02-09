#ifndef ELM_WIDGET_CTXPOPUP_H
#define ELM_WIDGET_CTXPOPUP_H

#include "elm_widget_layout.h"

/**
 * @addtogroup Widget
 * @{
 *
 * @section elm-ctxpopup-class The Elementary Ctxpopup Class
 *
 * Elementary, besides having the @ref Ctxpopup widget, exposes its
 * foundation -- the Elementary Ctxpopup Class -- in order to create other
 * widgets which are a ctxpopup with some more logic on top.
 */

typedef struct _Elm_Ctxpopup_Item       Elm_Ctxpopup_Item;

/**
 * Base widget smart data extended with ctxpopup instance data.
 */
typedef struct _Elm_Ctxpopup_Smart_Data Elm_Ctxpopup_Smart_Data;
struct _Elm_Ctxpopup_Item
{
   ELM_WIDGET_ITEM;

   Elm_Object_Item *list_item;

   struct
     {
        Evas_Smart_Cb org_func_cb;
        const void    *org_data;
        Evas_Object   *cobj;
     } wcb;
};

struct _Elm_Ctxpopup_Smart_Data
{
   Evas_Object           *parent;
   Evas_Object           *list;
   Evas_Object           *box;
   Eina_List             *items;

   Evas_Object           *arrow;
   Evas_Object           *bg;
   Evas_Object           *content;

   Elm_Ctxpopup_Direction dir;
   Elm_Ctxpopup_Direction dir_priority[4];

   Eina_Bool              list_visible : 1;
   Eina_Bool              horizontal : 1;
   Eina_Bool              finished : 1;
   Eina_Bool              emitted : 1;
   Eina_Bool              visible : 1;
   Eina_Bool              auto_hide : 1; /**< auto hide mode triggered by ctxpopup policy*/
};

/**
 * @}
 */

#define ELM_CTXPOPUP_DATA_GET(o, sd) \
  Elm_Ctxpopup_Smart_Data * sd = eo_data_scope_get(o, ELM_OBJ_CTXPOPUP_CLASS)

#define ELM_CTXPOPUP_DATA_GET_OR_RETURN(o, ptr)      \
  ELM_CTXPOPUP_DATA_GET(o, ptr);                     \
  if (EINA_UNLIKELY(!ptr))                           \
    {                                                \
       CRI("No widget data for object %p (%s)",      \
           o, evas_object_type_get(o));              \
       return;                                       \
    }

#define ELM_CTXPOPUP_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  ELM_CTXPOPUP_DATA_GET(o, ptr);                         \
  if (EINA_UNLIKELY(!ptr))                               \
    {                                                    \
       CRI("No widget data for object %p (%s)",          \
           o, evas_object_type_get(o));                  \
       return val;                                       \
    }

#define ELM_CTXPOPUP_CHECK(obj)                              \
  if (EINA_UNLIKELY(!eo_isa((obj), ELM_OBJ_CTXPOPUP_CLASS))) \
    return

#endif
