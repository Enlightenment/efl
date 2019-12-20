#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Efl_Ui.h>
#include <Elementary.h>

static void
_apply_style(const Eo *input, Eo *textblock)
{
   const char *style = efl_text_get(input);
   efl_canvas_textblock_style_apply(textblock, style);
}

static void
_style_changed_cb(void *data, const Efl_Event *ev)
{
   _apply_style(ev->object, data);
}


void
test_canvas_textblock(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Eo *win, *box, *textblock, *input;
   const char *default_style;

   win = efl_add(EFL_UI_WIN_CLASS, efl_main_loop_get(),
                                  efl_text_set(efl_added, "Efl.Canvas.Textblock style"),
                 efl_ui_win_autodel_set(efl_added, EINA_TRUE));

   box = efl_add(EFL_UI_BOX_CLASS, win,
                 efl_content_set(win, efl_added),
                 efl_ui_layout_orientation_set(efl_added, EFL_UI_LAYOUT_ORIENTATION_VERTICAL));

   efl_add(EFL_UI_TEXTBOX_CLASS, box,
           efl_gfx_hint_weight_set(efl_added, 1, 0),
           efl_text_interactive_selection_allowed_set(efl_added, EINA_FALSE),
           efl_text_interactive_editable_set(efl_added, EINA_FALSE),
           efl_text_set(efl_added, "Live style editor. Enter a style string below:"),
           efl_pack(box, efl_added));

   input = efl_add(EFL_UI_TEXTBOX_CLASS, box,
                   efl_gfx_hint_weight_set(efl_added, 1, 0),
                   efl_text_set(efl_added, "font=Sans font_size=24 color=white"),
                   efl_gfx_hint_size_min_set(efl_added, EINA_SIZE2D(0, 48)),
                   efl_text_multiline_set(efl_added, EINA_TRUE),
                   efl_text_wrap_set(efl_added, EFL_TEXT_FORMAT_WRAP_WORD),
                   efl_pack(box, efl_added));
   efl_ui_textbox_scrollable_set(input, EINA_TRUE);

   textblock = efl_add(EFL_CANVAS_TEXTBLOCK_CLASS, box,
                       efl_text_multiline_set(efl_added, EINA_TRUE),
                       efl_text_set(efl_added, "This is a sample text block."),
                       efl_pack(box, efl_added));

   efl_event_callback_add(input, EFL_TEXT_INTERACTIVE_EVENT_CHANGED_USER,
                          _style_changed_cb, textblock),

   efl_gfx_entity_size_set(win, EINA_SIZE2D(400, 240));

   default_style = efl_canvas_textblock_all_styles_get(textblock);
   printf("Default style string: %s\n", default_style);
   _apply_style(input, textblock);
}
