#include <Efl_Ui.hh>

using efl::eo::instantiate;

static void
efl_main(void *data EINA_UNUSED, const Efl_Event *ev EINA_UNUSED)
{
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_HIDDEN);

   efl::ui::Win win(instantiate);
   win.text_set("Calendar Creation Example");
   win.autohide_set(true);

   efl::ui::Calendar cal(instantiate, win);
   win.content_set(cal);

   win.size_set({320,320});
}
EFL_MAIN()
