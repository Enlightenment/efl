#ifndef _ELM_SUITE_H
#define _ELM_SUITE_H

#include <check.h>
#include "../efl_check.h"
#define ck_assert_strn_eq(s1, s2, len)          \
  {                                             \
    char expected[len+1], actual[len+1];        \
                                                \
    strncpy(expected, s1, len);                 \
    expected[len] = '\0';                       \
    strncpy(actual, s2, len);                   \
    actual[len] = '\0';                         \
                                                \
    ck_assert_str_eq(expected, actual);         \
  }

#include <Evas.h>
void elm_test_init(TCase *tc);
void elm_test_config(TCase *tc);
void elm_test_check(TCase *tc);
void elm_test_colorselector(TCase *tc);
void elm_test_entry(TCase *tc);
void elm_test_atspi(TCase *tc);
void elm_test_button(TCase *tc);
void elm_test_image(TCase *tc);
void elm_test_list(TCase *tc);
void elm_test_photo(TCase *tc);
void elm_test_actionslider(TCase *tc);
void elm_test_box(TCase *tc);
void elm_test_table(TCase *tc);
void elm_test_thumb(TCase *tc);
void elm_test_menu(TCase *tc);
void elm_test_photocam(TCase *tc);
void elm_test_win(TCase *tc);
void elm_test_icon(TCase *tc);
void elm_test_prefs(TCase *tc);
void elm_test_map(TCase *tc);
void elm_test_glview(TCase *tc);
void elm_test_web(TCase *tc);
void elm_test_toolbar(TCase *tc);
void elm_test_grid(TCase *tc);
void elm_test_diskselector(TCase *tc);
void elm_test_notify(TCase *tc);
void elm_test_mapbuf(TCase *tc);
void elm_test_flip(TCase *tc);
void elm_test_layout(TCase *tc);
void elm_test_slider(TCase *tc);
void elm_test_ctxpopup(TCase *tc);
void elm_test_separator(TCase *tc);
void elm_test_calendar(TCase *tc);
void elm_test_inwin(TCase *tc);
void elm_test_gengrid(TCase *tc);
void elm_test_radio(TCase *tc);
void elm_test_scroller(TCase *tc);
void elm_test_frame(TCase *tc);
void elm_test_datetime(TCase *tc);
void elm_test_player(TCase *tc);
void elm_test_bg(TCase *tc);
void elm_test_video(TCase *tc);
void elm_test_segmentcontrol(TCase *tc);
void elm_test_progressbar(TCase *tc);
void elm_test_fileselector(TCase *tc);
void elm_test_fileselector_button(TCase *tc);
void elm_test_fileselector_entry(TCase *tc);
void elm_test_hoversel(TCase *tc);
void elm_test_multibuttonentry(TCase *tc);
void elm_test_naviframe(TCase *tc);
void elm_test_popup(TCase *tc);
void elm_test_bubble(TCase *tc);
void elm_test_clock(TCase *tc);
void elm_test_conformant(TCase *tc);
void elm_test_dayselector(TCase *tc);
void elm_test_flipselector(TCase *tc);
void elm_test_genlist(TCase *tc);
void elm_test_hover(TCase *tc);
void elm_test_index(TCase *tc);
void elm_test_label(TCase *tc);
void elm_test_panel(TCase *tc);
void elm_test_panes(TCase *tc);
void elm_test_slideshow(TCase *tc);
void elm_test_spinner(TCase *tc);
void elm_test_plug(TCase *tc);
void elm_test_focus(TCase *tc);
void elm_test_focus_sub(TCase *tc);
void elm_test_widget_focus(TCase *tc);
/* FIXME : This test must move efl_ui_suite when it ready *
 * EFL_UI_TEST BEGIN */
void efl_ui_test_grid(TCase *tc);
/* EFL_UI_TEST END */

void elm_code_file_test_load(TCase *tc);
void elm_code_file_test_memory(TCase *tc);
void elm_code_test_basic(TCase *tc);
void elm_code_test_line(TCase *tc);
void elm_code_test_parse(TCase *tc);
void elm_code_test_syntax(TCase *tc);
void elm_code_test_text(TCase *tc);
void elm_code_test_indent(TCase *tc);
void elm_code_test_widget(TCase *tc);
void elm_code_test_widget_text(TCase *tc);
void elm_code_test_widget_selection(TCase *tc);
void elm_code_test_widget_undo(TCase *tc);

Evas_Object *win_add();

#endif /* _ELM_SUITE_H */
