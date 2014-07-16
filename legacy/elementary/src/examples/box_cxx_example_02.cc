//Compile with:
//gcc -g box_example_02.c -o box_example_02 `pkg-config --cflags --libs elementary`

#ifdef HAVE_CONFIG_H
# include <elementary_config.h>
#endif

#define ELM_INTERNAL_API_ARGESFSDFEFC
#define ELM_INTERFACE_ATSPI_ACCESSIBLE_PROTECTED
#define ELM_INTERFACE_ATSPI_COMPONENT_PROTECTED
#define ELM_INTERFACE_ATSPI_ACTION_PROTECTED
#define ELM_INTERFACE_ATSPI_VALUE_PROTECTED
#define ELM_INTERFACE_ATSPI_EDITABLE_TEXT_PROTECTED
#define ELM_INTERFACE_ATSPI_TEXT_PROTECTED
#define ELM_INTERFACE_ATSPI_SELECTION_PROTECTED
#define ELM_INTERFACE_ATSPI_IMAGE_PROTECTED
#define ELM_INTERFACE_ATSPI_WIDGET_ACTION_PROTECTED

#include <Elementary.h>

#include <Eo.h>
#include <Evas.h>
#include <Elementary.h>
#include <elm_widget.h>
#include <elm_interface_atspi_accessible.h>

#include <elm_win.eo.hh>
#include <elm_box.eo.hh>
#include <elm_button.eo.hh>

typedef struct
{
   Eina_List *transitions;
   Evas_Object *box;
   Evas_Object_Box_Layout last_layout;
} Transitions_Data;

static void
_add_cb(void *data, Evas_Object *obj, void *ev)
{
   Evas_Object *btn;
   Eina_List *children;
   Transitions_Data *tdata = static_cast<Transitions_Data*>(data);

   btn = elm_button_add(tdata->box);
   elm_object_text_set(btn, "I do nothing");
   children = (Eina_List *)elm_box_children_get(tdata->box);
   if (children)
     {
        elm_box_pack_after(tdata->box, btn, (Evas_Object *)children->data);
        eina_list_free(children);
     }
   else
     elm_box_pack_end(tdata->box, btn);
   evas_object_show(btn);
}

static void
_clear_cb(void *data, Evas_Object *obj, void *ev)
{
   Transitions_Data *tdata = static_cast<Transitions_Data*>(data);
   elm_box_clear(tdata->box);
}

static void
_unpack_cb(void *data, Evas_Object *obj, void *ev)
{
   Transitions_Data *tdata = static_cast<Transitions_Data*>(data);
   elm_box_unpack(tdata->box, obj);
   evas_object_move(obj, 0, 50);
   evas_object_color_set(obj, 128, 64, 0, 128);
}

static void
_test_box_transition_change(void *data)
{
   Transitions_Data *tdata = static_cast<Transitions_Data*>(data);
   Elm_Box_Transition *layout_data;
   Evas_Object_Box_Layout next_layout;

   if (!data) return;
   next_layout = reinterpret_cast<Evas_Object_Box_Layout>(eina_list_data_get(tdata->transitions));
   layout_data = elm_box_transition_new(2.0, tdata->last_layout,
                                        NULL, NULL, next_layout, NULL, NULL,
                                        _test_box_transition_change, tdata);
   elm_box_layout_set(tdata->box, elm_box_layout_transition, layout_data,
                      elm_box_transition_free);
   tdata->last_layout = next_layout;

   tdata->transitions = eina_list_demote_list(tdata->transitions,
                                              tdata->transitions);
}

EAPI_MAIN int
elm_main(int argc, char *argv[])
{
   static Transitions_Data tdata = {
        .transitions = NULL,
        .box = NULL,
        .last_layout = NULL
   };

   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   {
   ::elm_win win ( ::eo_ref( elm_win_util_standard_add("box-transition", "Box Transition") ));
   elm_win_autodel_set(win._eo_ptr(), EINA_TRUE);

   elm_box bigbox ( ::eo_ref( elm_box_add(win._eo_ptr()) ));
   bigbox.size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   win.resize_object_add(bigbox._eo_ptr());
   bigbox.visibility_set(true);

   {
     elm_box bx ( ::eo_ref( elm_box_add(win._eo_ptr()) ));
     bx.horizontal_set(EINA_TRUE);
     bigbox.pack_end(bx._eo_ptr());
     bx.visibility_set(true);

     {
       elm_button bt ( ::eo_ref(elm_button_add(win._eo_ptr()) ));
       bt.text_set("Add");
       bx.pack_end(bt._eo_ptr());
       bt.visibility_set(true);
       evas_object_smart_callback_add(bt._eo_ptr(), "clicked", _add_cb, &tdata);
     }

     {
       elm_button bt ( ::eo_ref(elm_button_add(win._eo_ptr())));
       bt.text_set("Clear");
       bx.pack_end(bt._eo_ptr());
       bt.visibility_set(true);
       evas_object_smart_callback_add(bt._eo_ptr(), "clicked", _clear_cb, &tdata);
     }
   }

   {
     elm_box bx ( ::eo_ref( elm_box_add(win._eo_ptr())));
     bx.size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
     bx.size_hint_align_set(EVAS_HINT_FILL, EVAS_HINT_FILL);
     bigbox.pack_end(bx._eo_ptr());
     bx.visibility_set(true);

     {
       elm_button bt ( ::eo_ref( elm_button_add(win._eo_ptr()) ));
       bt.text_set("Button 1");
       evas_object_smart_callback_add(bt._eo_ptr(), "clicked", _unpack_cb, &tdata);
       bt.size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
       bt.size_hint_align_set(EVAS_HINT_FILL, EVAS_HINT_FILL);
       bx.pack_end(bt._eo_ptr());
       bt.visibility_set(true);
     }

     {
       elm_button bt ( ::eo_ref( elm_button_add(win._eo_ptr()) ));
       bt.text_set("Button 2");
       bt.size_hint_weight_set(EVAS_HINT_EXPAND, 0.0);
       bt.size_hint_align_set(1.0, 0.5);
       evas_object_smart_callback_add(bt._eo_ptr(), "clicked", _unpack_cb, &tdata);
       bx.pack_end(bt._eo_ptr());
       bt.visibility_set(true);
     }

     {
       elm_button bt ( ::eo_ref(elm_button_add(win._eo_ptr())));
       bt.text_set("Button 3");
       evas_object_smart_callback_add(bt._eo_ptr(), "clicked", _unpack_cb, &tdata);
       bx.pack_end(bt._eo_ptr());
       bt.visibility_set(true);
     }

     tdata.box = bx._eo_ptr();
     tdata.last_layout = evas_object_box_layout_horizontal;
     tdata.transitions =
       (eina_list_append(tdata.transitions, (void*)evas_object_box_layout_vertical));
     tdata.transitions =
       (eina_list_append(tdata.transitions, (void*)evas_object_box_layout_horizontal));
     tdata.transitions =
       (eina_list_append(tdata.transitions, (void*)evas_object_box_layout_stack));
     tdata.transitions =
       (eina_list_append(tdata.transitions, (void*)evas_object_box_layout_homogeneous_vertical));
     tdata.transitions =
       (eina_list_append(tdata.transitions, (void*)evas_object_box_layout_homogeneous_horizontal));
     tdata.transitions =
       (eina_list_append(tdata.transitions, (void*)evas_object_box_layout_flow_vertical));
     tdata.transitions = 
       (eina_list_append(tdata.transitions, (void*)evas_object_box_layout_flow_horizontal));
     tdata.transitions = 
       (eina_list_append(tdata.transitions, (void*)evas_object_box_layout_stack));

     bx.layout_set(evas_object_box_layout_horizontal, NULL, NULL);
     _test_box_transition_change(&tdata);
   }
   
   win.size_set(300, 320);
   win.visibility_set(true);
   }
   
   elm_run();
   elm_shutdown();

   return 0;
}
ELM_MAIN()
