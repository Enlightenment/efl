#include "config.h" 

#include "Efreet.h"
/* no logging */
#define EFREET_MODULE_LOG_DOM
#include "efreet_private.h"
#include "Efreet_Mime.h"
#include "config.h"
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
   {"Mime", ef_mime_cb_get},
   { }
};

extern char **environ;

const char *ef_test_path_get(const char *component)
{
   static int is_local = -1;
   static char buf[PATH_MAX];

   if (is_local == -1)
     {
        struct stat st;
        is_local = (stat(PACKAGE_BUILD_DIR"/src/tests/efreet/data/test.desktop", &st) == 0);
     }

   if (is_local)
     {
        eina_str_join(buf, sizeof(buf), '/',
                      PACKAGE_BUILD_DIR"/src/tests/efreet/data",
                      component);
     }
   else
     {
        eina_str_join(buf, sizeof(buf), '/', PACKAGE_DATA_DIR "/tests",
                      component);
     }

   return buf;
}

int
main(int argc, char ** argv)
{
   int i, passed = 0, num_tests = 0;
   Eina_List *run = NULL;
   double total;

   eina_init();
   ecore_init();

   total = ecore_time_get();
   if (argc > 1)
     {
        for (i = 1; i < argc; i++)
          {
             if ((!strcmp(argv[i], "-h")) ||
                 (!strcmp(argv[i], "--help")))
               {
                  for (i = 0; tests[i].name; i++)
                    printf("%s\n", tests[i].name);
                  eina_list_free(run);
                  return 0;
               }
             run = eina_list_append(run, argv[i]);
          }
     }

   efreet_cache_update = 0;
   for (i = 0; tests[i].name; i++)
     {
        int ret;
        double start;

        /* we've been given specific tests and it isn't in the list */
        if (run && !eina_list_search_unsorted(run, EINA_COMPARE_CB(strcasecmp),
                                              tests[i].name))
          continue;

        if (!efreet_init())
          {
             printf("Error initializing Efreet\n");
             continue;
          }

        num_tests ++;

        printf("Starting \'%s\':\n", tests[i].name);
        start = ecore_time_get();

        environ = NULL;
        ret = tests[i].cb();

        printf("FINISHED \'%s\': %s in %.3f seconds\n", tests[i].name,
               (ret ? "PASSED" : "FAILED"), ecore_time_get() - start);
        passed += !!ret;

        efreet_shutdown();
     }

   printf("\n-----------------\n");
   printf("Passed %d of %d tests.\n", passed, num_tests);

   while (run)
     run = eina_list_remove_list(run, run);

   printf("Total run: %.3f seconds\n", ecore_time_get() - total);

   ecore_shutdown();
   eina_shutdown();
   return 0;
}
