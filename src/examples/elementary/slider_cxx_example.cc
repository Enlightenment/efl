
#ifdef HAVE_CONFIG_H
# include "config.h"
# include "elementary_config.h"
#endif

#include <Elementary.hh>

EAPI_MAIN int
elm_main (int argc, char *argv[])
{
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_HIDDEN);

   ::elm::win win(elm_win_util_standard_add("slider", "Slider Example"));
   win.autohide_set(true);

   ::elm::box bx(efl::eo::parent = win);
   bx.size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   win.resize_object_add(bx);
   bx.visible_set(true);

   ::elm::slider sl(efl::eo::parent = win);
   sl.size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   sl.size_hint_align_set(EVAS_HINT_FILL, 0.5);
   bx.pack_end(sl);
   sl.visible_set(true);

   ::elm::slider sl2(efl::eo::parent = win);
   sl2.text_set("elm.text", "Counter");
   ::elm::icon ic(efl::eo::parent = win);
   ic.standard_set("home");
   ic.resizable_set(false, false);
   sl2.content_set("icon", ic);

   ::elm::icon ic2(efl::eo::parent = win);
   ic2.standard_set("folder");
   ic2.resizable_set(false, false);
   sl2.content_set("end", ic2);

   sl2.size_hint_align_set(EVAS_HINT_FILL, 0.5);
   sl2.size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   bx.pack_end(sl2);
   sl2.visible_set(true);

   ::elm::slider sl3(efl::eo::parent = win);
   sl3.value_set(1);
   sl3.span_size_set(200);
   sl3.size_hint_align_set(EVAS_HINT_FILL, 0.5);
   sl3.size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   bx.pack_end(sl3);
   sl3.visible_set(true);

   ::elm::slider sl4(efl::eo::parent = win);
   sl4.unit_format_set("%1.0f units");
   sl4.min_max_set(0, 100);
   sl4.size_hint_align_set(EVAS_HINT_FILL, 0.5);
   sl4.size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   bx.pack_end(sl4);
   sl4.visible_set(true);

   ::elm::slider sl5(efl::eo::parent = win);
   sl5.indicator_format_set("%1.2f");
   sl5.inverted_set(true);
   sl5.size_hint_align_set(EVAS_HINT_FILL, 0.5);
   sl5.size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   bx.pack_end(sl5);
   sl5.visible_set(true);

   ::elm::slider sl6(efl::eo::parent = win);
   sl6.horizontal_set(false);

   auto indicator_format = [] (double val) {
                                            char *indicator = new char[32];
                                            snprintf(indicator, 32, "%1.2f u", val);
                                            return indicator;
                                           };

   auto indicator_free = [] (char *obj) {delete obj;} ;

   sl6.indicator_format_function_set(indicator_format, indicator_free);

   sl6.size_hint_align_set(0.5, EVAS_HINT_FILL);
   sl6.size_hint_weight_set(0, EVAS_HINT_EXPAND);
   bx.pack_end(sl6);
   sl6.visible_set(true);

   ::elm::slider sl7(efl::eo::parent = win);
   sl7.unit_format_set("%1.3f units");
   sl7.indicator_format_function_set(indicator_format, indicator_free);
   sl7.size_hint_align_set(EVAS_HINT_FILL, 0.5);
   sl7.size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   bx.pack_end(sl7);
   sl7.visible_set(true);

   auto changed = std::bind ( [] (::elm::slider obj)
                  {
                     double val = obj.value_get();
                     std::cout << "Changed to " << val << std::endl;
                  } , std::placeholders::_1  );

   auto delay =  std::bind ( [] (::elm::slider obj)
                 {
                    double val = obj.value_get();
                    std::cout << "Delay changed to " << val << std::endl;
                 } , std::placeholders::_1  );

   sl7.callback_changed_add(changed);
   sl7.callback_delay_changed_add(delay);

   win.visible_set(true);

   elm_run();
   return 0;
}
ELM_MAIN()
