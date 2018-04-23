#ifndef EFL_UI_ITEM_PRIVATE_H
#define EFL_UI_ITEM_PRIVATE_H

#include "Elementary.h"

typedef struct _Efl_Ui_Item_Data
{
   // Eo Objects
   Eo *obj; /* Self-Object */
   Eo *parent; /* Parent Widget */

   Efl_Ui_Select_Mode  *select_mode; /* Select Mdoe of parent widget */
   Ecore_Timer *longpress_timer; /* Timer for longpress handle */

   // Boolean Data
   Eina_Bool  selected : 1; /* State for item selected */
   Eina_Bool  needs_size_calc : 1; /* Flag for Size calculation */
} Efl_Ui_Item_Data;


#define EFL_UI_ITEM_DATA_GET(o, pd) \
  Efl_Ui_Item_Data * pd = efl_data_scope_safe_get(o, EFL_UI_ITEM_CLASS)

#define EFL_UI_ITEM_DATA_GET_OR_RETURN(o, ptr, ...)   \
  EFL_UI_ITEM_DATA_GET(o, ptr);                       \
  if (EINA_UNLIKELY(!ptr))                                 \
    {                                                      \
       ERR("No widget data for object %p (%s)",            \
           o, evas_object_type_get(o));                    \
       return __VA_ARGS__;                                 \
    }

#define EFL_UI_ITEM_CHECK_OR_RETURN(obj, ...)            \
  if (EINA_UNLIKELY(!efl_isa((obj), EFL_UI_ITEM_CLASS))) \
    return __VA_ARGS__;

#endif
