//Compile with:
//gcc -g box_example_02.c -o box_example_02 `pkg-config --cflags --libs elementary`

#include <Elementary.hh>
#include <Eina.hh>

#include <deque>
#include <iostream>

struct Transitions_Data
{
   efl::eo::wref<elm::box> box;
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

   if(efl::eina::optional<elm::box> box = tdata->box.lock())
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
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_HIDDEN);

   Transitions_Data tdata;

   ::elm::win::Standard win;
   win.title_set("Box Transition");
   win.autohide_set(true);

   elm::Ui::Box bigbox ( efl::eo::parent = win );
   bigbox.size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   //win.resize_object_add(bigbox);
   bigbox.visible_set(true);

   elm::ui::Box buttons ( efl::eo::parent = win );
   //buttons.horizontal_set(true);
   bigbox.pack_end(buttons);
   buttons.visible_set(true);

   elm::Button add ( efl::eo::parent = win );
   add.text_set("elm.text", "Add");
   buttons.pack_end(add);
   add.visible_set(true);
   add.callback_clicked_add
     (std::bind([&tdata]
      {
         if(efl::eina::optional<elm::box> box = tdata.box.lock())
         {
            elm::button btn ( efl::eo::parent = *box );
            btn.text_set(nullptr, "I do nothing");
            box->pack_end(btn);
            btn.visible_set(true);
         }
      }));

   elm::Button clear ( efl::eo::parent = win );
   clear.text_set("elm.text", "Clear");
   buttons.pack_end(clear);
   clear.visible_set(true);
   clear.callback_clicked_add(std::bind([&tdata] { tdata.box.lock()->clear(); }));

   elm::Box dynamic ( efl::eo::parent = win );
   dynamic.size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   dynamic.size_hint_align_set(EVAS_HINT_FILL, EVAS_HINT_FILL);
   bigbox.pack_end(dynamic);
   dynamic.visible_set(true);

   auto unpack = std::bind([&tdata] (evas::clickable_interface obj)
     {
       elm::Button btn = efl::eo::downcast<elm::button>(obj);
        tdata.box.lock()->unpack(btn);
        btn.position_set(0, 50);
        btn.object_smart::color_set(128, 64, 0, 128);
     }, std::placeholders::_1)
   ;

   elm::Button bt1 ( efl::eo::parent = win );
   bt1.text_set("elm.text", "Button 1");
   bt1.callback_clicked_add(unpack);
   bt1.size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   bt1.size_hint_align_set(EVAS_HINT_FILL, EVAS_HINT_FILL);
   dynamic.pack_end(bt1);
   bt1.visible_set(true);

   elm::Button bt2 ( efl::eo::parent = win );
   bt2.text_set("elm.text", "Button 2");
   bt2.size_hint_weight_set(EVAS_HINT_EXPAND, 0.0);
   bt2.size_hint_align_set(1.0, 0.5);
   bt2.callback_clicked_add(unpack);
   dynamic.pack_end(bt2);
   bt2.visible_set(true);

   elm::Button bt3 ( efl::eo::parent = win );
   bt3.text_set("elm.text", "Button 3");
   bt3.callback_clicked_add(unpack);
   dynamic.pack_end(bt3);
   bt3.visible_set(true);

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
   win.visible_set(true);

   elm_run();
   return 0;
}
ELM_MAIN()
