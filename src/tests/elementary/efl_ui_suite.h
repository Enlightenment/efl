#ifndef EFL_UI_SUITE_H
#define EFL_UI_SUITE_H

#include <check.h>

#define EFL_NOLEGACY_API_SUPPORT
#include <Efl_Ui.h>
#include "../efl_check.h"
#include "suite_helpers.h"

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

void efl_ui_test_box(TCase *tc);
void efl_ui_test_box_flow(TCase *tc);
void efl_ui_test_box_stack(TCase *tc);
void efl_ui_test_table(TCase *tc);
void efl_ui_test_grid(TCase *tc);
void efl_ui_test_relative_container(TCase *tc);
void efl_ui_test_atspi(TCase *tc);
void efl_ui_test_image_zoomable(TCase *tc);
void efl_ui_test_layout(TCase *tc);
void efl_ui_test_image(TCase *tc);
void efl_ui_test_callback(TCase *tc);
void efl_ui_test_focus(TCase *tc);
void efl_ui_test_focus_sub(TCase *tc);
void efl_ui_test_gesture(TCase *tc);
void efl_ui_model(TCase *tc);
void efl_ui_test_widget(TCase *tc);
void efl_ui_test_spotlight(TCase *tc);
void efl_ui_test_check(TCase *tc);
void efl_ui_test_progressbar(TCase *tc);
void efl_ui_test_radio_group(TCase *tc);
void efl_ui_test_slider(TCase *tc);
void efl_ui_test_win(TCase *tc);
void efl_ui_test_spin(TCase *tc);
void efl_ui_test_spin_button(TCase *tc);
void efl_ui_test_item_container(TCase *tc);
void efl_ui_test_list_container(TCase *tc);
void efl_ui_test_grid_container(TCase *tc);
void efl_ui_test_collection_view(TCase *tc);
void efl_ui_test_config(TCase *tc);
void efl_ui_test_popup(TCase *tc);
void efl_ui_test_scroller(TCase *tc);
void efl_ui_test_select_model(TCase *tc);
void efl_ui_test_view_model(TCase *tc);
void efl_ui_test_group_item(TCase *tc);
void efl_ui_test_text(TCase *tc);
void efl_ui_test_vg_animation(TCase *tc);

void loop_timer_interval_set(Eo *obj, double in);

#define efl_loop_timer_interval_set loop_timer_interval_set

const Efl_Class* efl_ui_widget_realized_class_get(void);
#define WIDGET_CLASS efl_ui_widget_realized_class_get()

Eo *win_add();
Eo *win_add_focused();
#endif
