#ifndef EINA_SUITE_H_
#define EINA_SUITE_H_

#include <check.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

void eina_test_stringshare(TCase *tc);
void eina_test_array(TCase *tc);
void eina_test_error(TCase *tc);
void eina_test_magic(TCase *tc);
void eina_test_inlist(TCase *tc);
void eina_test_lalloc(TCase *tc);
void eina_test_main(TCase *tc);
void eina_test_counter(TCase *tc);
void eina_test_hash(TCase *tc);

#endif /* EINA_SUITE_H_ */
