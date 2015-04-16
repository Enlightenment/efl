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
    Elua_State *st;
    lua_State *lst;

    fail_if(!elua_init());

    st = elua_state_new("test");
    fail_if(!st);

    lst = elua_state_lua_state_get(st);
    fail_if(!lst);

    fail_if(elua_state_from_lua_state_get(lst) != st);

    fail_if(strcmp(elua_state_prog_name_get(st), "test"));

    elua_state_free(st);

    elua_shutdown();
}
END_TEST

void elua_lib_test(TCase *tc)
{
   tcase_add_test(tc, elua_api);
}

