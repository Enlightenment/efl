#ifndef _ECORE_SUITE_H
#define _ECORE_SUITE_H

#include <check.h>
#include "../efl_check.h"

extern int timeout_reached;

void ecore_test_ecore(TCase *tc);
void ecore_test_ecore_idle(TCase *tc);
void ecore_test_ecore_poller(TCase *tc);
void ecore_test_ecore_x(TCase *tc);
void ecore_test_ecore_imf(TCase *tc);
void ecore_test_ecore_audio(TCase *tc);
void ecore_test_timer(TCase *tc);
void ecore_test_ecore_evas(TCase *tc);
void ecore_test_animator(TCase *tc);
void ecore_test_ecore_thread_eina_thread_queue(TCase *tc);
void ecore_test_ecore_thread_eina_thread_queue2(TCase *tc);
void ecore_test_ecore_fb(TCase *tc);
void ecore_test_ecore_input(TCase *tc);
void ecore_test_ecore_file(TCase *tc);
void ecore_test_ecore_job(TCase *tc);
void ecore_test_ecore_args(TCase *tc);
void ecore_test_ecore_pipe(TCase *tc);

#endif /* _ECORE_SUITE_H */
