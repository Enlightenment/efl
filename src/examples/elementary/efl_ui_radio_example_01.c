/*
 * gcc -o efl_ui_radio_example_01 efl_ui_radio_example_01.c `pkg-config --cflags --libs elementary`
 */
#define EFL_BETA_API_SUPPORT 1

#include <Efl_Ui.h>
#include <Elementary.h>

const char *example_strings[] = {
  "Seoul",
  "Karlsruhe",
  "New York",
  "Hong Kong",
  "Hamburg",
  "Berlin",
  "Paris",
   NULL
};

EAPI_MAIN void
efl_main(void *data EINA_UNUSED, const Efl_Event *ev EINA_UNUSED)
{
   Eo *win, *box;

   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   win = efl_add(EFL_UI_WIN_CLASS, efl_main_loop_get(),
                                  efl_text_set(efl_added, "Efl.Ui.Radio example"),
                 efl_ui_win_autodel_set(efl_added, EINA_TRUE)
                );

   box = efl_add(EFL_UI_RADIO_BOX_CLASS, win,
                 efl_content_set(win, efl_added));

   for (int i = 0; example_strings[i]; ++i)
     {
        Eo *radio;

        radio = efl_add(EFL_UI_RADIO_CLASS, box);
        efl_text_set(radio, example_strings[i]);
        efl_ui_radio_state_value_set(radio, i + 1);
        efl_pack_end(box, radio);
     }
}
EFL_MAIN()
