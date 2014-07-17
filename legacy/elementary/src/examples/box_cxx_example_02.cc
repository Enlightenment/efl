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

#include <deque>

typedef struct
{
   efl::eo::wref<elm_box> box;
   std::deque<Evas_Object_Box_Layout> transitions;
   Evas_Object_Box_Layout last_layout;
} Transitions_Data;

static void
_add_cb(void *data, Evas_Object *obj, void *ev)
{
   Eina_List *children;
   Transitions_Data *tdata = static_cast<Transitions_Data*>(data);

   if(efl::eina::optional<elm_box> box = tdata->box.lock())
   {
     elm_button btn ( efl::eo::parent = *box );
     btn.text_set("I do nothing");
     efl::eina::list<efl::eo::base> childrens (box->children_get());
     if (!children.empty())
       {
         box->pack_after(btn._eo_ptr(), childrens.front());
       }
     else
       box->pack_end(btn._eo_ptr());
     btn.visibility_set(true);
   }
}

static void
_clear_cb(void *data, Evas_Object *obj, void *ev)
{
   Transitions_Data *tdata = static_cast<Transitions_Data*>(data);
   tdata->box.lock()->clear();
}

static void
_unpack_cb(void *data, Evas_Object *obj, void *ev)
{
   Transitions_Data *tdata = static_cast<Transitions_Data*>(data);
   tdata->box.lock()->unpack(obj);
   evas_object_move(obj, 0, 50);
   evas_object_color_set(obj, 128, 64, 0, 128);
}

static void
_test_box_transition_change(void *data)
{
   Transitions_Data *tdata = static_cast<Transitions_Data*>(data);
   Elm_Box_Transition *layout_data;
   Evas_Object_Box_Layout next_layout;

   assert (!!data);
   assert (!tdata->transitions.empty());

   if(efl::eina::optional<elm_box> box = tdata->box.lock())
     {
        next_layout = tdata->transitions.front();
        layout_data = elm_box_transition_new(2.0, tdata->transitions.back(),
                                             nullptr, nullptr, next_layout, nullptr, nullptr,
                                             _test_box_transition_change, tdata);
        box->layout_set(elm_box_layout_transition, layout_data,
                        elm_box_transition_free);
        tdata->last_layout = next_layout;
       
        tdata->transitions.push_back(tdata->transitions[0]);
        tdata->transitions.pop_front();
     }
}

EAPI_MAIN int
elm_main(int argc, char *argv[])
{
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   Transitions_Data tdata
     {
        elm_box{nullptr}
        , {}
        , nullptr
     };

   {
     ::elm_win win (elm_win_util_standard_add("box-transition", "Box Transition"));
     win.autodel_set(true);

     elm_box bigbox ( efl::eo::parent = win );
     bigbox.size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
     win.resize_object_add(bigbox._eo_ptr());
     bigbox.visibility_set(true);

     elm_box buttons ( efl::eo::parent = win );
     buttons.horizontal_set(EINA_TRUE);
     bigbox.pack_end(buttons._eo_ptr());
     buttons.visibility_set(true);

     elm_button add ( efl::eo::parent = win );
     add.text_set("Add");
     buttons.pack_end(add._eo_ptr());
     add.visibility_set(true);
     evas_object_smart_callback_add(add._eo_ptr(), "clicked", _add_cb, &tdata);

     elm_button clear ( efl::eo::parent = win );
     clear.text_set("Clear");
     buttons.pack_end(clear._eo_ptr());
     clear.visibility_set(true);
     evas_object_smart_callback_add(clear._eo_ptr(), "clicked", _clear_cb, &tdata);

     elm_box dynamic ( efl::eo::parent = win );
     dynamic.size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
     dynamic.size_hint_align_set(EVAS_HINT_FILL, EVAS_HINT_FILL);
     bigbox.pack_end(dynamic._eo_ptr());
     dynamic.visibility_set(true);

     elm_button bt1 ( efl::eo::parent = win );
     bt1.text_set("Button 1");
     evas_object_smart_callback_add(bt1._eo_ptr(), "clicked", _unpack_cb, &tdata);
     bt1.size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
     bt1.size_hint_align_set(EVAS_HINT_FILL, EVAS_HINT_FILL);
     dynamic.pack_end(bt1._eo_ptr());
     bt1.visibility_set(true);

     elm_button bt2 ( efl::eo::parent = win );
     bt2.text_set("Button 2");
     bt2.size_hint_weight_set(EVAS_HINT_EXPAND, 0.0);
     bt2.size_hint_align_set(1.0, 0.5);
     evas_object_smart_callback_add(bt2._eo_ptr(), "clicked", _unpack_cb, &tdata);
     dynamic.pack_end(bt2._eo_ptr());
     bt2.visibility_set(true);

     elm_button bt3 ( efl::eo::parent = win );
     bt3.text_set("Button 3");
     evas_object_smart_callback_add(bt3._eo_ptr(), "clicked", _unpack_cb, &tdata);
     dynamic.pack_end(bt3._eo_ptr());
     bt3.visibility_set(true);

     tdata.box = dynamic;
     tdata.last_layout = evas_object_box_layout_horizontal;
     tdata.transitions.push_back(evas_object_box_layout_vertical);
     tdata.transitions.push_back(evas_object_box_layout_horizontal);
     tdata.transitions.push_back(evas_object_box_layout_stack);
     tdata.transitions.push_back(evas_object_box_layout_homogeneous_vertical);
     tdata.transitions.push_back(evas_object_box_layout_homogeneous_horizontal);
     tdata.transitions.push_back(evas_object_box_layout_flow_vertical);
     tdata.transitions.push_back(evas_object_box_layout_flow_horizontal);
     tdata.transitions.push_back(evas_object_box_layout_stack);

     dynamic.layout_set(evas_object_box_layout_horizontal, nullptr, nullptr);
     _test_box_transition_change(&tdata);
   
     win.size_set(300, 320);
     win.visibility_set(true);
     win._release();
   }
   
   elm_run();
   elm_shutdown();

   return 0;
}
ELM_MAIN()
