#ifndef _ELM_CODE_SUITE_H
#define _ELM_CODE_SUITE_H

#include <check.h>

#define ck_assert_strn_eq(str1, str2, len) \
  { \
     unsigned int i = 0; \
     while (i < len) \
       { \
          ck_assert_int_eq(*(str1 + i), *(str2 + i)); \
          i++; \
       } \
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

#endif /* _EDLM_CODE_SUITE_H */
