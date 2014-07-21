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

#include <iostream>

#include <Elementary.h>

#include <Eo.h>
#include <Evas.h>
#include <Elementary.h>
#include <elm_widget.h>
#include <elm_interface_atspi_accessible.h>

#include <elm_win.eo.hh>
#include <elm_box.eo.hh>
#include <elm_button.eo.hh>

#include <Eina.hh>

#include <deque>

struct Transitions_Data
{
   efl::eo::wref<elm_box> box;
   std::deque<Evas_Object_Box_Layout> transitions;
   Evas_Object_Box_Layout last_layout;
};

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

   Transitions_Data tdata;
   Eo* test;

   {
     ::elm_win win (elm_win_util_standard_add("box-transition", "Box Transition"));
     win.autodel_set(true);

     elm_box bigbox ( efl::eo::parent = win );
     bigbox.size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
     win.resize_object_add(bigbox);
     bigbox.visibility_set(true);

     elm_box buttons ( efl::eo::parent = win );
     buttons.horizontal_set(EINA_TRUE);
     bigbox.pack_end(buttons);
     buttons.visibility_set(true);

     elm_button add ( efl::eo::parent = win );
     add.text_set("Add");
     buttons.pack_end(add);
     add.visibility_set(true);
     add.event_clicked_callback_add
       (std::bind([&tdata]
        {
          if(efl::eina::optional<elm_box> box = tdata.box.lock())
          {
            elm_button btn ( efl::eo::parent = *box );
            btn.text_set("I do nothing");
            efl::eina::list<evas::object> childrens = box->children_get();
            if (!childrens.empty())
              {
                box->pack_after(btn, childrens.front());
              }
            else
              box->pack_end(btn);
            btn.visibility_set(true);
          }
        }));

     elm_button clear ( efl::eo::parent = win );
     clear.text_set("Clear");
     buttons.pack_end(clear);
     clear.visibility_set(true);
     clear.event_clicked_callback_add(std::bind([&tdata] { tdata.box.lock()->clear(); }));

     elm_box dynamic ( efl::eo::parent = win );
     dynamic.size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
     dynamic.size_hint_align_set(EVAS_HINT_FILL, EVAS_HINT_FILL);
     bigbox.pack_end(dynamic);
     dynamic.visibility_set(true);

     auto unpack = std::bind([&tdata] (evas::clickable_interface obj)
       {
         elm_button btn = efl::eo::downcast<elm_button>(obj);
         tdata.box.lock()->unpack(btn);
         btn.position_set(0, 50);
         btn.color_set(128, 64, 0, 128);
       }, std::placeholders::_1)
     ;

     elm_button bt1 ( efl::eo::parent = win );
     bt1.text_set("Button 1");
     bt1.event_clicked_callback_add(unpack);
     bt1.size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
     bt1.size_hint_align_set(EVAS_HINT_FILL, EVAS_HINT_FILL);
     dynamic.pack_end(bt1);
     bt1.visibility_set(true);

     elm_button bt2 ( efl::eo::parent = win );
     bt2.text_set("Button 2");
     bt2.size_hint_weight_set(EVAS_HINT_EXPAND, 0.0);
     bt2.size_hint_align_set(1.0, 0.5);
     bt2.event_clicked_callback_add(unpack);
     dynamic.pack_end(bt2);
     bt2.visibility_set(true);

     elm_button bt3 ( efl::eo::parent = win );
     bt3.text_set("Button 3");
     bt3.event_clicked_callback_add(unpack);
     dynamic.pack_end(bt3);
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

     // bigbox._release();
     // buttons._release();
     // add._release();
     // clear._release();
     // dynamic._release();
     // bt1._release();
     // bt2._release();
     // bt3._release();


     std::cout << "references to win " << win.ref_get() << std::endl;
     test = win._eo_ptr();
     //win._release();
   }
   std::cout << "references to win " << ::eo_ref_get(test) << std::endl;
  
   elm_run();
   elm_shutdown();

   return 0;
}
ELM_MAIN()
