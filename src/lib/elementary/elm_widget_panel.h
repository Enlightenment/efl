#ifndef ELM_WIDGET_PANEL_H
#define ELM_WIDGET_PANEL_H

#include "elm_interface_scrollable.h"
#include "elm_widget_layout.h"
#include "elm_panel.eo.h"

/* DO NOT USE THIS HEADER UNLESS YOU ARE PREPARED FOR BREAKING OF YOUR
 * CODE. THIS IS ELEMENTARY'S INTERNAL WIDGET API (for now) AND IS NOT
 * FINAL. CALL elm_widget_api_check(ELM_INTERNAL_API_VERSION) TO CHECK
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
typedef struct _Elm_Panel_Data Elm_Panel_Data;
struct _Elm_Panel_Data
{
   Evas_Object                          *bx, *content;
   Evas_Object                          *event;
   Evas_Object                          *scr_ly;
   Evas_Object                          *hit_rect, *panel_edje, *scr_edje;
   Evas_Object                          *scr_panel, *scr_event;


   Elm_Panel_Orient                      orient;

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

#define ELM_PANEL_DATA_GET(o, sd) \
  Elm_Panel_Data * sd = efl_data_scope_get(o, ELM_PANEL_CLASS)

#define ELM_PANEL_DATA_GET_OR_RETURN(o, ptr)         \
  ELM_PANEL_DATA_GET(o, ptr);                        \
  if (EINA_UNLIKELY(!ptr))                           \
    {                                                \
       ERR("No widget data for object %p (%s)",      \
           o, evas_object_type_get(o));              \
       return;                                       \
    }

#define ELM_PANEL_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  ELM_PANEL_DATA_GET(o, ptr);                         \
  if (EINA_UNLIKELY(!ptr))                            \
    {                                                 \
       ERR("No widget data for object %p (%s)",       \
           o, evas_object_type_get(o));               \
       return val;                                    \
    }

#define ELM_PANEL_CHECK(obj)                              \
  if (EINA_UNLIKELY(!efl_isa((obj), ELM_PANEL_CLASS))) \
    return

#endif
