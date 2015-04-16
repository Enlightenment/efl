#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>

#include<Eina.h>
#include "Elua.h"
#include "elua_suite.h"

START_TEST(elua_api)
{
    fail_if(!elua_init());

    Elua_State *st = elua_state_new("test");
    fail_if(!st);

    elua_state_free(st);

    elua_shutdown();
}
END_TEST

void elua_lib_test(TCase *tc)
{
   tcase_add_test(tc, elua_api);
}

