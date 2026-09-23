#ifndef _ETHUMB_SUITE_H
#define _ETHUMB_SUITE_H

#include <check.h>
#include "../efl_check.h"

extern const unsigned char ethumb_test_png[];
extern const size_t ethumb_test_png_size;

void ethumb_test_slave(TCase *tc);
void ethumb_test_ethumb(TCase *tc);

#endif
