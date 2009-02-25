/* vim: set sw=4 ts=4 sts=4 et: */
#include "Efreet.h"
#include "Efreet_Mime.h"
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
int ef_cb_ini_long_line(void);
int ef_cb_ini_garbage(void);
#if DEFAULT_VISIBILITY
int ef_cb_locale(void);
#endif
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
int ef_cb_utils(void);
int ef_mime_cb_get(void);

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
    {"INI Garbage Parsing", ef_cb_ini_garbage},
#if DEFAULT_VISIBILITY
    {"Locale Parsing", ef_cb_locale},
#endif
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
    {"Mime", ef_mime_cb_get},
    {NULL, NULL}
};

extern char **environ;
static Eina_List *environment = NULL;

void
environment_store(void)
{
    char *env;
    char **e;

    while (environment)
    {
        env = eina_list_data_get(environment);
        free(env);
        environment = eina_list_remove_list(environment, environment);
    }

    for (e = environ; *e; e++)
        environment = eina_list_append(environment, strdup(*e));
}

void
environment_restore(void)
{
    Eina_List *l;
    char *e;
    if (!environment) return;

    *environ = NULL;
    EINA_LIST_FOREACH(environment, l, e)
        putenv(e);
}

int
main(int argc, char ** argv)
{
    int i, passed = 0, num_tests = 0;
    Eina_List *run = NULL;
    double total;

    total = ecore_time_get();
    if (argc > 1)
    {
        for (i = 1; i < argc; i++)
        {
            if ((!strcmp(argv[i], "-h")) ||
                (!strcmp(argv[i], "--help")))
            {
                for (i = 0; tests[i].name != NULL; i++)
                {
                    printf("%s\n", tests[i].name);
                }
                return 1;
            }
            run = eina_list_append(run, argv[i]);
        }
    }

    environment_store();
    for (i = 0; tests[i].name != NULL; i++)
    {
        int ret;
        double start;

        /* we've been given specific tests and it isn't in the list */
        if (run && !eina_list_search_unsorted(run, ECORE_COMPARE_CB(strcasecmp),
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
    while (environment)
    {
        free(eina_list_data_get(environment));
        environment = eina_list_remove_list(environment, environment);
    }
    printf("Passed %d of %d tests.\n", passed, num_tests);

    while (run)
        run = eina_list_remove_list(run, run);

    printf("Total run: %.3f seconds\n", ecore_time_get() - total);
    return 0;
}
