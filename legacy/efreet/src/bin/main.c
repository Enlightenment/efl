#include "Efreet.h"
#include <Ecore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int ef_cb_efreet_data_home(void);
int ef_cb_efreet_config_home(void);
int ef_cb_efreet_cache_home(void);
int ef_cb_efreet_data_dirs(void);
int ef_cb_efreet_config_dirs(void);
int ef_cb_efreet_icon_theme(void);
int ef_cb_efreet_icon_theme_list(void);
int ef_cb_efreet_icon_match(void);
int ef_cb_ini_parse(void);
int ef_cb_locale(void);
int ef_cb_desktop_parse(void);
int ef_cb_desktop_save(void);
int ef_cb_desktop_command_get(void);
int ef_cb_desktop_type_parse(void);
#if 0
int ef_cb_desktop_file_id(void);
#endif
int ef_cb_menu_get(void);
int ef_cb_menu_with_slashes(void);
int ef_cb_menu_save(void);
#if 0
int ef_cb_menu_edit(void);
#endif
int ef_cb_ini_long_line(void);
int ef_cb_utils(void);

typedef struct Efreet_Test Efreet_Test;
struct Efreet_Test
{
    char *name;
    int (*cb)(void);
};

static Efreet_Test tests[] = {
    {"Data Home", ef_cb_efreet_data_home},
    {"Config Home", ef_cb_efreet_config_home},
    {"Cache Home", ef_cb_efreet_cache_home},
    {"Data Directories", ef_cb_efreet_data_dirs},
    {"Config Directories", ef_cb_efreet_config_dirs},
    {"Icon Theme Basic", ef_cb_efreet_icon_theme},
    {"Icon Theme List", ef_cb_efreet_icon_theme_list},
    {"Icon Matching", ef_cb_efreet_icon_match},
    {"INI Parsing", ef_cb_ini_parse},
    {"INI Long Line Parsing", ef_cb_ini_long_line},
    {"Locale Parsing", ef_cb_locale},
    {"Desktop Parsing", ef_cb_desktop_parse},
    {"Desktop Type Parsing", ef_cb_desktop_type_parse},
    {"Desktop Save", ef_cb_desktop_save},
    {"Desktop Command", ef_cb_desktop_command_get},
#if 0
    {"Desktop File ID", ef_cb_desktop_file_id},
#endif
    {"Menu Parsing", ef_cb_menu_get},
    {"Menu Incorrect Names", ef_cb_menu_with_slashes},
    {"Menu Save", ef_cb_menu_save},
#if 0
    {"Menu Edit", ef_cb_menu_edit},
#endif
    {"Utils", ef_cb_utils},
    {NULL, NULL}
};

extern char **environ;
static Ecore_List *environment = NULL;

void
environment_store(void)
{
    char **e;

    if (environment)
        ecore_list_clear(environment);
    else
    {
        environment = ecore_list_new();
        ecore_list_set_free_cb(environment, ECORE_FREE_CB(free));
    }

    for (e = environ; *e; e++)
        ecore_list_append(environment, strdup(*e));
}

void
environment_restore(void)
{
    char *e;
    if (!environment) return;

    *environ = NULL;
    ecore_list_goto_first(environment);
    while ((e = ecore_list_next(environment)))
        putenv(e);
}

int
main(int argc, char ** argv)
{
    int i, passed = 0, num_tests = 0;
    Ecore_List *run = NULL;
    double total;

    total = ecore_time_get();
    if (argc > 1)
    {
        run = ecore_list_new();
        for (i = 1; i < argc; i++)
            ecore_list_append(run, argv[i]);
    }

    environment_store();
    for (i = 0; tests[i].name != NULL; i++)
    {
        int ret;
        double start;

        /* we've been given specific tests and it isn't in the list */
        if (run && !ecore_list_find(run, ECORE_COMPARE_CB(strcasecmp), 
                                                        tests[i].name))
            continue; 

        if (!efreet_init())
        {
            printf("Error initializing Efreet\n");
            continue;
        }

        num_tests ++;

        printf("%s:\t\t", tests[i].name); 
        fflush(stdout);
        start = ecore_time_get();
        ret = tests[i].cb();
        printf("%s in %.3f seconds\n", (ret ? "PASSED" : "FAILED"), 
                                            ecore_time_get() - start);
        passed += ret;

        efreet_shutdown();
        environment_restore();
    }

    printf("\n-----------------\n");
    if (environment) ecore_list_destroy(environment);
    printf("Passed %d of %d tests.\n", passed, num_tests);

    if (run) ecore_list_destroy(run);

    printf("Total run: %.3f seconds\n", ecore_time_get() - total);
    return 0;
}

