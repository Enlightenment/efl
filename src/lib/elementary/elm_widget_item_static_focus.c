#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define ELM_WIDGET_ITEM_PROTECTED

#include <Elementary.h>
#include "elm_priv.h"
#include "efl_ui_focus_composition_adapter.eo.h"

typedef struct {
   Eo *adapter;
} Elm_Widget_Item_Static_Focus_Data;

EOLIAN static void
_elm_widget_item_static_focus_efl_ui_focus_object_prepare_logical(Eo *obj, Elm_Widget_Item_Static_Focus_Data *pd EINA_UNUSED)
{
   Eo *logical_child;
   Elm_Widget_Item_Data *wpd = efl_data_scope_get(obj, ELM_WIDGET_ITEM_CLASS);

   efl_ui_focus_object_prepare_logical(efl_super(obj, ELM_WIDGET_ITEM_STATIC_FOCUS_CLASS));
   logical_child = efl_ui_focus_manager_request_subchild(wpd->widget, obj);

   if (!logical_child)
     {
        if (!pd->adapter)
          {
             pd->adapter = efl_add(EFL_UI_FOCUS_COMPOSITION_ADAPTER_CLASS, wpd->view, 
              efl_ui_focus_composition_adapter_canvas_object_set(efl_added,  wpd->view)
             );
             efl_wref_add(pd->adapter, &pd->adapter);
             efl_ui_focus_manager_calc_register(wpd->widget, pd->adapter, obj, NULL);
          }
     }
   else if (logical_child && logical_child != pd->adapter)
     {
        efl_ui_focus_manager_calc_unregister(wpd->widget, pd->adapter);
        efl_del(pd->adapter);
        pd->adapter = NULL;
     }
}

#include "elm_widget_item_static_focus.eo.c"
