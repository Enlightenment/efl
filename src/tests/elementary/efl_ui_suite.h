#ifndef EFL_UI_SUITE_H
#define EFL_UI_SUITE_H

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

#include <Eo.h>
void efl_ui_test_grid(TCase *tc);
void efl_ui_test_atspi(TCase *tc);
void efl_ui_test_image_zoomable(TCase *tc);
void efl_ui_test_layout(TCase *tc);
void efl_ui_test_image(TCase *tc);

void efl_ui_test_focus(TCase *tc);
void efl_ui_test_focus_sub(TCase *tc);

Eo *win_add();
Eo *win_add_focused();
#endif
