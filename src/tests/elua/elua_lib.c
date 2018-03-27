#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>

#include <Eina.h>
#include <Elua.h>

#include "elua_suite.h"

EFL_START_TEST(elua_api)
{
    Elua_State *st;
    lua_State *lst;
    char buf[] = "tmpXXXXXX";
    FILE *f;
    int fd;
    char *cargv[2];
    char arg1[] = "test";
    char arg2[] = "test";
    int quit = 0;
    cargv[0] = arg1;
    cargv[1] = arg2;
    char *spath = NULL;

    st = elua_state_new("test");
    fail_if(!st);

    /* test env vars */
    setenv("ELUA_CORE_DIR", "foo", 1);
    setenv("ELUA_MODULES_DIR", "bar", 1);
    setenv("ELUA_APPS_DIR", "baz", 1);
    elua_state_dirs_fill(st, EINA_FALSE);

    spath = eina_file_path_sanitize("foo");
    fail_if(strcmp(elua_state_core_dir_get(st), spath));
    free(spath);
    spath = eina_file_path_sanitize("bar");
    fail_if(strcmp(elua_state_mod_dir_get(st), spath));
    free(spath);
    spath = eina_file_path_sanitize("baz");
    fail_if(strcmp(elua_state_apps_dir_get(st), spath));
    free(spath);
    unsetenv("ELUA_CORE_DIR");
    unsetenv("ELUA_MODULES_DIR");
    unsetenv("ELUA_APPS_DIR");

    /* now fill it properly */
    elua_state_dirs_set(st, ELUA_CORE_DIR, ELUA_MODULES_DIR, ELUA_APPS_DIR);
    spath = eina_file_path_sanitize(ELUA_CORE_DIR);
    fail_if(strcmp(elua_state_core_dir_get(st), spath));
    free(spath);
    spath = eina_file_path_sanitize(ELUA_MODULES_DIR);
    fail_if(strcmp(elua_state_mod_dir_get(st), spath));
    free(spath);
    spath = eina_file_path_sanitize(ELUA_APPS_DIR);
    fail_if(strcmp(elua_state_apps_dir_get(st), spath));
    free(spath);

    /* needed for later setup, but untestable alone */
    elua_state_include_path_add(st, ELUA_BINDINGS_DIR);

    lst = elua_state_lua_state_get(st);
    fail_if(!lst);

    fail_if(elua_state_from_lua_state_get(lst) != st);

    fail_if(strcmp(elua_state_prog_name_get(st), "test"));

    fail_if(!elua_state_setup(st));

    fail_if(!elua_state_require_ref_push(st));
    fail_if(lua_type(lst, -1) != LUA_TFUNCTION);
    lua_pop(lst, 1);

    fail_if(!elua_state_appload_ref_push(st));
    fail_if(lua_type(lst, -1) != LUA_TFUNCTION);
    lua_pop(lst, 1);

    fail_if(!elua_util_require(st, "util"));
    fail_if(!elua_util_string_run(st, "return 1337", "foo"));
    fail_if(elua_util_string_run(st, "foo bar", "foo")); /* invalid code */
    fail_if(elua_util_app_load(st, "test"));
    fail_if(lua_type(lst, -1) != LUA_TFUNCTION);
    lua_pop(lst, 1);
    fail_if(!elua_util_app_load(st, "non_existent_app"));
    fail_if(lua_type(lst, -1) != LUA_TSTRING);
    lua_pop(lst, 1);
    fail_if(elua_io_loadfile(st, ELUA_CORE_DIR "/util.lua"));
    fail_if(lua_type(lst, -1) != LUA_TFUNCTION);
    lua_pop(lst, 1);
    fail_if(!elua_io_loadfile(st, ELUA_CORE_DIR "/non_existent_file.lua"));
    fail_if(lua_type(lst, -1) != LUA_TSTRING);
    lua_pop(lst, 1);

    fd = mkstemp(buf);
    fail_if(fd < 0);
    f = fdopen(fd, "wb");
    fail_if(!f);
    fprintf(f, "return 5\n");
    fclose(f);
    fail_if(!elua_util_file_run(st, buf));
    fail_if(lua_tointeger(lst, -1) != 5);
    lua_pop(lst, 1);
    fail_if(remove(buf));

    /* halfassed testing here, but not possible otherwise */
    fail_if(elua_util_error_report(st, 0));
    lua_pushliteral(lst, "msg");
    fail_if(!elua_util_error_report(st, 5));
    fail_if(lua_gettop(lst) > 0);

    fail_if(!elua_util_script_run(st, 2, cargv, 1, &quit));
    fail_if(quit != 1);

    f = fopen(buf, "wb");
    fail_if(!f);
    fprintf(f, "return false");
    fclose(f);
    cargv[1] = buf;
    fail_if(!elua_util_script_run(st, 2, cargv, 1, &quit));
    fail_if(quit != 0);
    fail_if(remove(buf));

    elua_state_free(st);
}
EFL_END_TEST

void elua_lib_test(TCase *tc)
{
   tcase_add_test(tc, elua_api);
}

