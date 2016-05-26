#ifdef HAVE_CONFIG_H
#include "config.h"
#include "elementary_config.h"
#endif

#include <Elementary.hh>
#include <Evas.hh>

EAPI_MAIN int
elm_main (int argc, char *argv[])
{
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_HIDDEN);

   efl::ui::win::Standard win;
   // win.title_set("Bubble");
   win.autohide_set(true);

   ::elm::Label label1(win);
   label1.text_set(nullptr, "Bubble with no icon, info or label");
   label1.visible_set(true);

   //::evas::Rectangle icon(efl::eo::parent = win);
   elm::Icon icon(win);
   icon.eo_cxx::efl::Gfx::color_set( 0, 0, 255, 255);
   icon.visible_set(true);

   ::elm::Bubble bubble1(win);
   efl::Container xxx = efl::eo::downcast<efl::Container>
     (bubble1.part("icon"));
   xxx.content_set(icon);
   // bubble1.part_content_set("icon", icon); 
   bubble1.text_set("info", "INFO");
   bubble1.text_set(nullptr, "LABEL");
   bubble1.content_set(label1);
   bubble1.eo_cxx::efl::Gfx::size_set(300, 100);
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

   efl::eolian::event_add(/*bubble1.clicked_event*/efl::ui::Clickable::clicked_event, bubble1, on_click);

   ::elm::Label label2(win);
   label2.text_set(nullptr, "Bubble with no icon, info or label");
   label2.visible_set(true);

   ::elm::Bubble bubble2(win);
   bubble2.content_set(label2);
   bubble2.eo_cxx::efl::Gfx::size_set(200, 50);
   bubble2.eo_cxx::efl::Gfx::position_set(0, 110);
   bubble2.visible_set(true);

   win.eo_cxx::efl::Gfx::size_set(300, 200);
   win.visible_set(true);

   elm_run();
   return 0;
}
ELM_MAIN()
