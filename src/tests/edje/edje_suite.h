#ifndef _EDJE_SUITE_H
#define _EDJE_SUITE_H

#include <check.h>
#include "../efl_check.h"
#include <Edje.h>
Evas *_setup_evas(void);
const char *test_layout_get(const char *name);

void edje_test_edje(TCase *tc);
void edje_test_container(TCase *tc);
void edje_test_features(TCase *tc);
void edje_test_signal(TCase *tc);
void edje_test_swallow(TCase *tc);
void edje_test_text(TCase *tc);


#endif /* _EDJE_SUITE_H */
