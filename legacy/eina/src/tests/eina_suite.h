#ifndef EINA_SUITE_H_
#define EINA_SUITE_H_

#include <check.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

void eina_test_stringshare(TCase *tc);
void eina_test_array(TCase *tc);

#endif /* EINA_SUITE_H_ */
