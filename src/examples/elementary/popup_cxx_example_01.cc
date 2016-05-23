
#ifdef HAVE_CONFIG_H
# include "config.h"
# include "elementary_config.h"
#endif

#include <Elementary.hh>

EAPI_MAIN int
elm_main(int argc, char **argv)
{
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_HIDDEN);

   ::elm::win_standard win;
   win.title_set("Popup");
   win.autohide_set(true);

   ::elm::popup popup(efl::eo::parent = win);
   popup.timeout_set(3.0);
   popup.text_set(nullptr, "Title");
   popup.visible_set(true);

   auto popup_hide = std::bind ( [&] ()
                     {
                        popup.visible_set(false);
                     });

   popup.callback_timeout_add(popup_hide);
   popup.callback_block_clicked_add(popup_hide);
   win.size_set(480, 800);
   win.visible_set(true);

   elm_run();
   return 0;
}
ELM_MAIN()


