#ifndef _ELM_TEST_HELPER_H
#define _ELM_TEST_HELPER_H

#include <Eina.h>

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


Eina_Bool elm_test_helper_wait_flag(double in, Eina_Bool *done);

#endif /* _ELM_TEST_HELPER_H */
