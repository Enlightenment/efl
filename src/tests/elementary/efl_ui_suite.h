#ifndef EFL_UI_SUITE_H
#define EFL_UI_SUITE_H

#include <check.h>

#define EFL_NOLEGACY_API_SUPPORT
#include <Efl_Ui.h>
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

void efl_ui_test_box(TCase *tc);
void efl_ui_test_box_flow(TCase *tc);
void efl_ui_test_box_stack(TCase *tc);
void efl_ui_test_table(TCase *tc);
void efl_ui_test_grid(TCase *tc);
void efl_ui_test_relative_layout(TCase *tc);
void efl_ui_test_atspi(TCase *tc);
void efl_ui_test_image_zoomable(TCase *tc);
void efl_ui_test_layout(TCase *tc);
void efl_ui_test_image(TCase *tc);

void efl_ui_test_focus(TCase *tc);
void efl_ui_test_focus_sub(TCase *tc);

void efl_ui_model(TCase *tc);
void efl_ui_test_widget(TCase *tc);

void loop_timer_interval_set(Eo *obj, double in);

#define efl_loop_timer_interval_set loop_timer_interval_set

Eo *win_add();
Eo *win_add_focused();
#endif
