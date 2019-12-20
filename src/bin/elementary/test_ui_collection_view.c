#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Efl_Ui.h>
#include <Elementary.h>

#define NUM_ITEMS 50

static Efl_Model*
_make_model(Efl_Ui_Win *win)
{
   Eina_Value vi;
   Efl_Generic_Model *model, *child;
   unsigned int i;

   model = efl_add(EFL_GENERIC_MODEL_CLASS, win);
   model = efl_add(EFL_UI_VIEW_MODEL_CLASS, model,
                   efl_ui_view_model_set(efl_added, model));
   eina_value_setup(&vi, EINA_VALUE_TYPE_INT);

   efl_ui_view_model_property_string_add(model, "relative", "Relative index ${child.index}", "WRONG", "WRONG");
   efl_ui_view_model_property_string_add(model, "title", "Initial index ${initial}", "WRONG", "WRONG");


   for (i = 0; i < (NUM_ITEMS); i++)
     {
        child = efl_model_child_add(model);
        eina_value_set(&vi, i);
        efl_model_property_set(child, "initial", &vi);
     }

   eina_value_flush(&vi);
   return model;
}

static void
_item_constructing(void *data EINA_UNUSED, const Efl_Event *ev)
{
   Efl_Gfx_Entity *item = ev->info;

   if (!efl_ui_item_calc_locked_get(item))
     efl_gfx_hint_size_min_set(item, EINA_SIZE2D(100, 50));
}

static void
_item_select(void *data, const Efl_Event *ev)
{
   Efl_Ui_Widget *w = efl_ui_selectable_last_selected_get(ev->object);
   Efl_Model *m = efl_ui_view_model_get(w);

   if (m) efl_ui_view_model_set(data, m);
}

void test_efl_ui_collection_view(void *data EINA_UNUSED,
                                 Evas_Object *obj EINA_UNUSED,
                                 void *event_info EINA_UNUSED)
{
   Efl_Ui_Win *win;
   Efl_Model *model;
   Efl_Ui_Frame *f;
   Efl_Ui_Table *tbl;
   Efl_Ui_List_View *lv;
   Efl_Ui_Grid_View *gv;
   Efl_Ui_Factory *fg, *fl;
   Efl_Ui_Box *ib;
   Efl_Ui_Widget *o;
   Efl_Model_Provider *provider;

   win = efl_add(EFL_UI_WIN_CLASS, efl_main_loop_get(),
                 efl_ui_win_type_set(efl_added, EFL_UI_WIN_TYPE_BASIC),
                 efl_text_set(efl_added, "Efl.Ui.Collection_View"),
                 efl_ui_win_autodel_set(efl_added, EINA_TRUE));
   tbl = efl_add(EFL_UI_TABLE_CLASS, win,
                 efl_gfx_hint_size_min_set(efl_added, EINA_SIZE2D(500, 300)));
   efl_content_set(win, tbl);

   model = _make_model(win);
   fg = efl_add(EFL_UI_LAYOUT_FACTORY_CLASS, win,
                efl_ui_property_bind(efl_added, "text", "title"),
                efl_ui_widget_factory_item_class_set(efl_added, EFL_UI_GRID_DEFAULT_ITEM_CLASS));
   efl_event_callback_add(fg, EFL_UI_FACTORY_EVENT_ITEM_CONSTRUCTING, _item_constructing, NULL);
   fl = efl_add(EFL_UI_LAYOUT_FACTORY_CLASS, win,
                efl_ui_property_bind(efl_added, "text", "title"),
                efl_ui_widget_factory_item_class_set(efl_added, EFL_UI_LIST_DEFAULT_ITEM_CLASS));
   provider = efl_add(EFL_MODEL_PROVIDER_CLASS, win);

   lv = efl_add(EFL_UI_LIST_VIEW_CLASS, tbl,
                efl_ui_collection_view_factory_set(efl_added, fl),
                efl_ui_view_model_set(efl_added, model));
   efl_pack_table(tbl, lv, 0, 1, 1, 1);

   gv = efl_add(EFL_UI_GRID_VIEW_CLASS, tbl,
                efl_ui_collection_view_factory_set(efl_added, fg),
                efl_ui_view_model_set(efl_added, model));
   efl_pack_table(tbl, gv, 2, 1, 1, 1);

   f = efl_add(EFL_UI_FRAME_CLASS, tbl,
               efl_text_set(efl_added, "Selected item"),
               efl_ui_frame_autocollapse_set(efl_added, EINA_FALSE));
   efl_provider_register(f, EFL_MODEL_PROVIDER_CLASS, provider);
   efl_pack_table(tbl, f, 1, 1, 1, 1);
   efl_event_callback_add(lv, EFL_UI_SELECTABLE_EVENT_SELECTION_CHANGED, _item_select, provider);

   ib = efl_add(EFL_UI_BOX_CLASS, f,
                efl_gfx_hint_align_set(efl_added, EVAS_HINT_FILL, EVAS_HINT_FILL),
                efl_gfx_hint_weight_set(efl_added, 1, 1),
                efl_gfx_hint_fill_set(efl_added, EINA_TRUE, EINA_TRUE),
                efl_content_set(f, efl_added));

   o = elm_label_add(ib);
   elm_object_text_set(o, "title:");
   efl_gfx_hint_weight_set(o, 1, 0);
   efl_gfx_hint_align_set(o, 0.5, 1.0);
   efl_gfx_hint_aspect_set(o, EFL_GFX_HINT_ASPECT_BOTH, EINA_SIZE2D(1, 1));
   efl_pack(ib, o);
   efl_gfx_entity_visible_set(o, 1);

   o = elm_label_add(ib);
   elm_object_text_set(o, "NONE");
   efl_ui_property_bind(o, "elm.text", "title");
   efl_gfx_hint_weight_set(o, 1, 0);
   efl_gfx_hint_align_set(o, 0.5, 1.0);
   efl_gfx_hint_aspect_set(o, EFL_GFX_HINT_ASPECT_BOTH, EINA_SIZE2D(1, 1));
   efl_pack(ib, o);
   efl_gfx_entity_visible_set(o, 1);

   o = elm_label_add(ib);
   elm_object_text_set(o, "relative:");
   efl_gfx_hint_weight_set(o, 1, 0);
   efl_gfx_hint_align_set(o, 0.5, 1.0);
   efl_gfx_hint_aspect_set(o, EFL_GFX_HINT_ASPECT_BOTH, EINA_SIZE2D(1, 1));
   efl_pack(ib, o);
   efl_gfx_entity_visible_set(o, 1);

   o = elm_label_add(ib);
   elm_object_text_set(o, "NONE");
   efl_ui_property_bind(o, "elm.text", "relative");
   efl_gfx_hint_weight_set(o, 1, 0);
   efl_gfx_hint_align_set(o, 0.5, 1.0);
   efl_gfx_hint_aspect_set(o, EFL_GFX_HINT_ASPECT_BOTH, EINA_SIZE2D(1, 1));
   efl_pack(ib, o);
   efl_gfx_entity_visible_set(o, 1);
}
