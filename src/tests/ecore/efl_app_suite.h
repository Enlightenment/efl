#ifndef _EFL_APP_SUITE_H
#define _EFL_APP_SUITE_H

#include <check.h>
#include "../efl_check.h"
void efl_app_test_efl_app(TCase *tc);
void efl_app_test_efl_loop(TCase *tc);
void efl_app_test_efl_loop_fd(TCase *tc);
void efl_app_test_efl_loop_timer(TCase *tc);
void efl_app_test_promise(TCase *tc);
void efl_app_test_promise_2(TCase *tc);
void efl_app_test_promise_3(TCase *tc);
void efl_app_test_promise_safety(TCase *tc);
void efl_test_efl_env(TCase *tc);
void efl_test_efl_cml(TCase *tc);

#endif /* _EFL_APP_SUITE_H */
