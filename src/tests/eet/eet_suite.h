#ifndef _EET_SUITE_H
# define _EET_SUITE_H

#include <check.h>
#include "../efl_check.h"
void eet_test_init(TCase *tc);
void eet_test_data(TCase *tc);
void eet_test_file(TCase *tc);
void eet_test_image(TCase *tc);
void eet_test_identity(TCase *tc);
void eet_test_cipher(TCase *tc);
void eet_test_cache(TCase *tc);
void eet_test_connection(TCase *tc);

#endif /* _EET_SUITE_H */
