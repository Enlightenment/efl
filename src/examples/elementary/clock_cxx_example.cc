#include <Elementary.hh>

using efl::eo::instantiate;

static void
efl_main(void *data EINA_UNUSED, const Efl_Event *ev EINA_UNUSED)
{
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_HIDDEN);

   efl::ui::Win win(instantiate);
   win.text_set("Clock Example");
   win.autohide_set(true);

   efl::ui::Box bx(instantiate, win);
   win.content_set(bx);

   efl::ui::Clock ck(instantiate, win);
   bx.pack_end(ck);

   efl::ui::Clock ck2(instantiate, win);
   ck2.format_set("%I:%M %p");
   bx.pack_end(ck2);

   efl::ui::Clock ck3(instantiate, win);
   ck2.format_set("%H:%M:%S");
   ck3.time_set(Efl_Time({.tm_hour = 12, .tm_min = 42, .tm_sec = 59}));
   bx.pack_end(ck3);

   efl::ui::Clock ck5(instantiate, win);
   int digedit = ELM_CLOCK_EDIT_HOUR_UNIT | ELM_CLOCK_EDIT_MIN_UNIT | ELM_CLOCK_EDIT_SEC_UNIT;
   ck5.edit_mode_set(static_cast<Elm_Clock_Edit_Mode>(digedit));
   bx.pack_end(ck5);

   win.size_set({500,500});
}
EFL_MAIN()
