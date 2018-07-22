#ifndef _EVAS_SUITE_H
#define _EVAS_SUITE_H

#include <check.h>
#include "../efl_check.h"
void evas_test_init(TCase *tc);
void evas_test_new(TCase *tc);
void evas_test_object(TCase *tc);
void evas_test_textblock(TCase *tc);
void evas_test_text(TCase *tc);
void evas_test_callbacks(TCase *tc);
void evas_test_render_engines(TCase *tc);
void evas_test_filters(TCase *tc);
void evas_test_image_object(TCase *tc);
void evas_test_image_object2(TCase *tc);
void evas_test_mesh(TCase *tc);
void evas_test_mesh1(TCase *tc);
void evas_test_mesh2(TCase *tc);
void evas_test_mesh3(TCase *tc);
void evas_test_mask(TCase *tc);
void evas_test_evasgl(TCase *tc);
void evas_test_object_smart(TCase *tc);
void evas_test_matrix(TCase *tc);

#endif /* _EVAS_SUITE_H */
