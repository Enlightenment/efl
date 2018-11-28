// gcc -o efl_canvas_layout_text2 efl_canvas_layout_text2.c `pkg-config --cflags --libs elementary`
// edje_cc efl_canvas_layout2.edc
// ./efl_canvas_layout_text2

#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#else
# define EFL_BETA_API_SUPPORT 1
# define EFL_EO_API_SUPPORT 1
#endif

#include <Efl.h>
#include <Elementary.h>
#include <string.h>

static void
_on_win_delete(void *data EINA_UNUSED, const Efl_Event *ev EINA_UNUSED)
{
   efl_exit(0);
}

static const char* groups[] = { "test" };
static size_t group_itr = 0;

static void
_help(void)
{
   printf("Press 'h' to display this help.\n");
}

static const char *edjefile = "efl_canvas_layout_text2.edj";

EAPI_MAIN void
efl_main(void *data EINA_UNUSED,
         const Efl_Event *ev)
{

   Eo *layout;
   Eo *win;

   win = efl_add(EFL_UI_WIN_CLASS, efl_main_loop_get(),
         efl_ui_win_type_set(efl_added, EFL_UI_WIN_BASIC),
         efl_text_set(efl_added, "Efl Canvas_Layout"),
         efl_ui_win_autodel_set(efl_added, EINA_TRUE),
         efl_event_callback_add(efl_added, EFL_UI_WIN_EVENT_DELETE_REQUEST, _on_win_delete, NULL));


   layout = efl_add(EFL_CANVAS_LAYOUT_CLASS, win);
   efl_file_set(layout, edjefile, groups[group_itr]);

   efl_text_markup_set(efl_part(layout, "text"), "This is an example text. This is a layout text part.");
   efl_text_wrap_set(efl_part(layout, "text"), EFL_TEXT_FORMAT_WRAP_WORD);

   efl_content_set(win, layout);
   efl_gfx_entity_size_set(win, EINA_SIZE2D(110, 100));
}
EFL_MAIN()
