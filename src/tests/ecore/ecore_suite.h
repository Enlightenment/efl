#ifndef _ECORE_SUITE_H
#define _ECORE_SUITE_H

#include <check.h>

void ecore_test_ecore(TCase *tc);
void ecore_test_ecore_con(TCase *tc);
void ecore_test_ecore_x(TCase *tc);
void ecore_test_ecore_imf(TCase *tc);
void ecore_test_ecore_audio(TCase *tc);
void ecore_test_timer(TCase *tc);
void ecore_test_ecore_evas(TCase *tc);
void ecore_test_animator(TCase *tc);
void ecore_test_ecore_thread_eina_thread_queue(TCase *tc);

#endif /* _ECORE_SUITE_H */
