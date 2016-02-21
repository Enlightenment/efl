#ifndef _ELM_CODE_SUITE_H
#define _ELM_CODE_SUITE_H

#include <check.h>

#define ck_assert_strn_eq(s1, s2, len) \
  { \
     char expected[len+1], actual[len+1]; \
\
     strncpy(expected, s1, len); \
     expected[len] = '\0'; \
     strncpy(actual, s2, len); \
     actual[len] = '\0'; \
\
     ck_assert_str_eq(expected, actual); \
  }

#include <Elm_Code.h>

void elm_code_file_test_load(TCase *tc);
void elm_code_file_test_memory(TCase *tc);
void elm_code_test_basic(TCase *tc);
void elm_code_test_line(TCase *tc);
void elm_code_test_parse(TCase *tc);
void elm_code_test_text(TCase *tc);
void elm_code_test_widget(TCase *tc);
void elm_code_test_widget_text(TCase *tc);
void elm_code_test_widget_selection(TCase *tc);
void elm_code_test_widget_undo(TCase *tc);

#endif /* _EDLM_CODE_SUITE_H */
