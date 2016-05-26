#include <Elementary.hh>
#include <Eina.hh>
#include <Evas.hh>

static void _print_items(void *data, Evas_Object *obj, void *event_info);
static void _free(void *data, Evas_Object *obj, void *event_info);

EAPI_MAIN int
elm_main (int argc, char *argv[])
{
   efl::eina::eina_init eina_init;

   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_HIDDEN);

   ::elm::win::Standard win;
   win.title_set("Hoversel");
   win.autohide_set(true);

   ::evas::Rectangle rect(efl::eo::parent = win);
   rect.color_set(255, 0, 0, 255);
   rect.visible_set(true);

   ::elm::hoversel hoversel(efl::eo::parent = win);
   hoversel.horizontal_set(false);
   hoversel.part_text_set("elm.text", "Add an item to Hoversel");
   hoversel.part_content_set(nullptr, rect);

   hoversel.item_add("Print items", nullptr, ELM_ICON_NONE, &_print_items, NULL);
   hoversel.item_add( "Option 2", "home", ELM_ICON_STANDARD, NULL,NULL);

   auto add_item = std::bind([] (::elm::hoversel obj)
     {
        static int num = 0;
        char *str = new char[sizeof(char) * 10];

        snprintf(str, 10, "item %d", ++num);

        elm::widget_item hoversel_item = obj.item_add(str, nullptr, ELM_ICON_NONE, NULL, str);
        elm_object_item_del_cb_set(hoversel_item._eo_ptr(), &_free);
     }, std::placeholders::_1);

   hoversel.callback_clicked_add(add_item);

   hoversel.size_set(180, 30);
   hoversel.position_set( 10, 10);
   hoversel.visible_set(true);

   ::elm::button btn(efl::eo::parent = win);
   btn.text_set("elm.text", "Clear all Items");

   auto clear_btn_clicked = std::bind([&] () { hoversel.clear(); });

   btn.callback_clicked_add(clear_btn_clicked);
   btn.size_set(180, 30);
   btn.position_set(10, 50);
   btn.visible_set(true);

   win.size_set(200, 300);
   win.visible_set(true);

   elm_run();

   return 0;
}
ELM_MAIN()

static void
_print_items(void *data, Evas_Object *obj, void *event_info)
{
   efl::eina::range_list<const elm::hoversel_item> items(elm_hoversel_items_get(obj));

   for (auto i : items)
     try
       {
          std::cout << i.part_text_get(nullptr) << std::endl;
       }
     catch (std::logic_error const&) {}
}

static void
_free(void *data, Evas_Object *obj, void *event_info)
{
   delete[] static_cast<char *>(data);
}
