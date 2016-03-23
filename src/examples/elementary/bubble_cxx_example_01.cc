#include <Elementary.hh>
#include <Evas.hh>

EAPI_MAIN int
elm_main (int argc, char *argv[])
{
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_HIDDEN);

   ::elm::win_standard win;
   win.title_set("Bg Plain");
   win.autohide_set(true);

   ::elm::bg bg(efl::eo::parent = win);
   bg.size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   win.resize_object_add(bg);
   bg.visible_set(true);

   ::elm::label label1(efl::eo::parent = win);
   label1.text_set(nullptr, "Bubble with no icon, info or label");
   label1.visible_set(true);

   evas::rectangle icon(efl::eo::parent = win);
   icon.color_set( 0, 0, 255, 255);
   icon.visible_set(true);

   ::elm::bubble bubble1(efl::eo::parent = win);
   bubble1.content_set("icon", icon);
   bubble1.text_set("info", "INFO");
   bubble1.text_set(nullptr, "LABEL");
   bubble1.content_set(nullptr, label1);
   bubble1.size_set(300, 100);
   bubble1.visible_set(true);

   auto on_click = std::bind([&] ()
                   {
                      static unsigned char corner = 0;
                      ++corner;
                      if (corner > 3)
                        bubble1.pos_set(ELM_BUBBLE_POS_TOP_LEFT);
                      else
                        bubble1.pos_set(static_cast<Elm_Bubble_Pos>(corner));
                   });

   bubble1.callback_clicked_add(on_click);

   ::elm::label label2(efl::eo::parent = win);
   label2.text_set(nullptr, "Bubble with no icon, info or label");
   label2.visible_set(true);

   ::elm::bubble bubble2(efl::eo::parent = win);
   bubble2.content_set(nullptr, label2);
   bubble2.size_set(200, 50);
   bubble2.position_set(0, 110);
   bubble2.visible_set(true);

   win.size_set(300, 200);
   win.visible_set(true);

   elm_run();
   return 0;
}
ELM_MAIN()
