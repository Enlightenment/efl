
#include <Elementary.hh>
#include <Evas.hh>

EAPI_MAIN int
elm_main (int argc, char *argv[])
{
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_HIDDEN);

   ::efl::ui::win::Standard win;
   //win.title_set("Separator");
   win.autohide_set(true);

   ::efl::ui::Box bx(win);
   bx.eo_cxx::efl::Orientation::orientation_set(EFL_ORIENT_HORIZONTAL);
   //bx.horizontal_set(true);
   // bx.size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   //win.resize_object_add(bx);
   bx.eo_cxx::efl::Gfx::size_set(200,200);
   bx.visible_set(true);

   efl::canvas::Rectangle rect(win);
   rect.color_set( 0, 255, 0, 255);
   // rect.size_hint_min_set( 90, 200);
   // rect.size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   // rect.size_hint_align_set(EVAS_HINT_FILL, EVAS_HINT_FILL);
   rect.visible_set(true);
   rect.eo_cxx::efl::Gfx::size_set(20,20);
   bx.pack_end(rect);

   ::elm::Separator separator(win);
   separator.eo_cxx::efl::Orientation::orientation_set(EFL_ORIENT_HORIZONTAL);
   // separator.horizontal_set(true);
   separator.visible_set(true);
   separator.eo_cxx::efl::Gfx::size_set(200,200);
   bx.pack_end(separator);

   efl::canvas::Rectangle rect2(win);
   rect2.color_set( 0, 0, 255, 255);
   // rect2.size_hint_min_set( 90, 200);
   // rect2.size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   // rect2.size_hint_align_set(EVAS_HINT_FILL, EVAS_HINT_FILL);
   rect2.visible_set(true);
   rect2.eo_cxx::efl::Gfx::size_set(10,10);
   bx.pack_end(rect2);

   win.eo_cxx::efl::Gfx::size_set(200,200);
   win.visible_set(true);

   elm_run();
   return 0;
}
ELM_MAIN()

