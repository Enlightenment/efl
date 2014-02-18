#ifndef _EVAS_SUITE_H
#define _EVAS_SUITE_H

#include <check.h>

void evas_test_init(TCase *tc);
void evas_test_object(TCase *tc);
void evas_test_textblock(TCase *tc);
void evas_test_text(TCase *tc);
void evas_test_callbacks(TCase *tc);
void evas_test_render_engines(TCase *tc);
void evas_test_filters(TCase *tc);


#endif /* _EVAS_SUITE_H */
