#ifndef EFL_UI_PANEL_H
#define EFL_UI_PANEL_H

#include "elm_interface_scrollable.h"
#include "elm_widget_layout.h"
#include "efl_ui_panel.eo.h"

/* DO NOT USE THIS HEADER UNLESS YOU ARE PREPARED FOR BREAKING OF YOUR
 * CODE. THIS IS ELEMENTARY'S INTERNAL WIDGET API (for now) AND IS NOT
 * FINAL. CALL elm_widget_api_check(EFL_UI_INTERNAL_API_VERSION) TO CHECK
 * IT AT RUNTIME.
 */

/**
 * @addtogroup Widget
 * @{
 *
 * @section elm-panel-class The Elementary Panel Class
 *
 * Elementary, besides having the @ref Panel widget, exposes its
 * foundation -- the Elementary Panel Class -- in order to create other
 * widgets which are a panel with some more logic on top.
 */

/**
 * Base layout smart data extended with panel instance data.
 */
typedef struct _Efl_Ui_Panel_Data Efl_Ui_Panel_Data;
struct _Efl_Ui_Panel_Data
{
   Evas_Object                          *bx, *content;
   Evas_Object                          *event;
   Evas_Object                          *scr_ly;
   Evas_Object                          *hit_rect, *panel_edje, *scr_edje;
   Evas_Object                          *scr_panel, *scr_event;


   Efl_Ui_Panel_Orient                   orient;

   double                                content_size_ratio;
   Evas_Coord                            down_x, down_y;
   Evas_Coord                            handler_size;
   Ecore_Timer                          *timer;

   Eina_Bool                             hidden : 1;
   Eina_Bool                             delete_me : 1;
   Eina_Bool                             scrollable : 1;
   Eina_Bool                             freeze: 1;
   Eina_Bool                             callback_added: 1;
};

/**
 * @}
 */

#define EFL_UI_PANEL_DATA_GET(o, sd) \
  Efl_Ui_Panel_Data * sd = efl_data_scope_get(o, EFL_UI_PANEL_CLASS)

#define EFL_UI_PANEL_DATA_GET_OR_RETURN(o, ptr)         \
  EFL_UI_PANEL_DATA_GET(o, ptr);                        \
  if (EINA_UNLIKELY(!ptr))                           \
    {                                                \
       ERR("No widget data for object %p (%s)",      \
           o, evas_object_type_get(o));              \
       return;                                       \
    }

#define EFL_UI_PANEL_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  EFL_UI_PANEL_DATA_GET(o, ptr);                         \
  if (EINA_UNLIKELY(!ptr))                            \
    {                                                 \
       ERR("No widget data for object %p (%s)",       \
           o, evas_object_type_get(o));               \
       return val;                                    \
    }

#define EFL_UI_PANEL_CHECK(obj)                              \
  if (EINA_UNLIKELY(!efl_isa((obj), EFL_UI_PANEL_CLASS))) \
    return

#endif
