#include "Efreet.h"
#include "config.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include "ef_test.h"

static void *_cb_command(void *data, Efreet_Desktop *desktop, char *exec, int remaining);


int
ef_cb_desktop_parse(void)
{
    Efreet_Desktop *desktop;
    int ret = 1;

    desktop = efreet_desktop_get(PACKAGE_DATA_DIR"/test/test.desktop");
    if (!desktop)
    {
        printf("No desktop found.\n");
        return 0;
    }

    if (!desktop->name || strcmp(desktop->name, "Efreet Test Application"))
    {
        printf("Invalid Name\n");
        ret = 0;
    }

    if (!desktop->generic_name ||
        strcmp(desktop->generic_name, "Test Application"))
    {
        printf("Incorrent GenericName\n");
        ret = 0;
    }

    if (!desktop->exec || strcmp(desktop->exec, "efreet_test %F %i"))
    {
        printf("Incorrect Exec (%s)\n", (desktop->exec ? desktop->exec : "(null)"));
        ret = 0;
    }

    if (desktop->categories)
    {
        const char *categories[] = {"Test", "Enlightenment"};
        const char *cat;
        int num_categories = 2, i = 0;

        ecore_list_first_goto(desktop->categories);
        while ((cat = ecore_list_next(desktop->categories)))
        {
            if (i >= num_categories)
            {
                printf("Too many categories found.\n");
                ret = 0;
                break;
            }

            if (!cat || !categories[i] || strcmp(cat, categories[i]))
            {
                printf("Expected category %s, found %s\n", categories[i], cat);
                ret = 0;
            }
            i++;
        }
    }
    else ret = 0;

    efreet_desktop_free(desktop);

    return ret;
}

#if 0
int
ef_cb_desktop_file_id(void)
{
    Efreet_Desktop *desktop;
    int ret = 1;

    desktop = efreet_desktop_get(PACKAGE_DATA_DIR"/test/test.desktop");
    if (desktop)
    {
        const char *id;
        int i = 0;

        struct {
            char *dir;
            int legacy;
            char *prefix;
            char *expected;
        } tests[] = {
            {PACKAGE_DATA_DIR"/test/", 0, NULL, "test.desktop"},
            {PACKAGE_DATA_DIR"/", 0, NULL, "test-test.desktop"},
            {PACKAGE_DATA_DIR"/", 1, NULL, "test.desktop"},
            {PACKAGE_DATA_DIR"/", 1, "prefix", "prefix-test.desktop"},
            {NULL, 0, NULL, NULL}
        };

        for (i = 0; tests[i].dir != NULL; i++)
        {
            id = efreet_desktop_id_get(desktop,
                                       tests[i].dir,
                                       tests[i].legacy,
                                       tests[i].prefix);
            if (!id || strcmp(id, tests[i].expected))
            {
                printf("Expecting id: %s, got: %s\n", tests[i].expected, id);
                ret = 0;
            }
            if (id) eina_stringshare_del(id);
        }
    }
    else
        ret = 0;

    return ret;
}
#endif

int
ef_cb_desktop_save(void)
{
    Efreet_Desktop *desktop;

    printf("\n");
    desktop = efreet_desktop_get(PACKAGE_DATA_DIR"/test/test.desktop");
    if (!desktop)
    {
        printf("Failed to get Desktop file\n");
        return 0;
    }

    printf("save data: %d\n", efreet_desktop_save(desktop));
    efreet_desktop_free(desktop);

    desktop = efreet_desktop_empty_new("/tmp/test.desktop");
    desktop->name = strdup("Efreet Test Application");
    desktop->type = EFREET_DESKTOP_TYPE_APPLICATION;
    desktop->generic_name = strdup("Test Application");
    desktop->exec = strdup("efreet_test");
    desktop->categories = ecore_list_new();
    ecore_list_free_cb_set(desktop->categories, ECORE_FREE_CB(free));
    ecore_list_append(desktop->categories, strdup("Test"));
    ecore_list_append(desktop->categories, strdup("Enlightenment"));
    printf("save test: %d\n", efreet_desktop_save(desktop));
    unlink("/tmp/test.desktop");
    efreet_desktop_free(desktop);

    return 1;
}

typedef struct
{
  Ecore_List *expected;
  int error;
  char type;
} Test_Info;

int
ef_cb_desktop_command_get(void)
{
    Efreet_Desktop *desktop;
    Ecore_List *files, *expected;
    char olddir[PATH_MAX];
    Test_Info *info;
    int ret;

    getcwd(olddir, PATH_MAX);
    chdir("/");

    printf("\n");
    desktop = efreet_desktop_empty_new("test.desktop");

    desktop->name = strdup("App Name");
    desktop->icon = strdup("icon.png");

    files = ecore_list_new();
    ecore_list_append(files, "/tmp/absolute_path");
    ecore_list_append(files, "relative_path");
    ecore_list_append(files, "file:///tmp/absolute_uri");
    ecore_list_append(files, "file:relative_uri");

    info = NEW(Test_Info, 1);
    expected = ecore_list_new();
    info->expected = expected;
    info->error = 0;

    /* test single full path */
    info->type = 'f';
    IF_FREE(desktop->exec);
    desktop->exec = strdup("app %f");
    ecore_list_append(expected, "app '/tmp/absolute_path'");
    ecore_list_append(expected, "app '/relative_path'");
    ecore_list_append(expected, "app '/tmp/absolute_uri'");
    ecore_list_append(expected, "app '/relative_uri'");

    ecore_list_first_goto(expected);
    efreet_desktop_command_get(desktop, files, _cb_command, info);
    ecore_list_clear(expected);

    /* test single uri */
    info->type = 'u';
    IF_FREE(desktop->exec);
    desktop->exec = strdup("app %u");
    ecore_list_append(expected, "app 'file:///tmp/absolute_path'");
    ecore_list_append(expected, "app 'file:///relative_path'");
    ecore_list_append(expected, "app 'file:///tmp/absolute_uri'");
    ecore_list_append(expected, "app 'file:///relative_uri'");

    ecore_list_first_goto(expected);
    efreet_desktop_command_get(desktop, files, _cb_command, info);
    ecore_list_clear(expected);

    /* test single dir */
    info->type = 'd';
    IF_FREE(desktop->exec);
    desktop->exec = strdup("app %d");
    ecore_list_append(expected, "app '/tmp'");
    ecore_list_append(expected, "app '/'");
    ecore_list_append(expected, "app '/tmp'");
    ecore_list_append(expected, "app '/'");

    ecore_list_first_goto(expected);
    efreet_desktop_command_get(desktop, files, _cb_command, info);
    ecore_list_clear(expected);


    /* test single names */
    info->type = 'n';
    IF_FREE(desktop->exec);
    desktop->exec = strdup("app %n");
    ecore_list_append(expected, "app 'absolute_path'");
    ecore_list_append(expected, "app 'relative_path'");
    ecore_list_append(expected, "app 'absolute_uri'");
    ecore_list_append(expected, "app 'relative_uri'");

    ecore_list_first_goto(expected);
    efreet_desktop_command_get(desktop, files, _cb_command, info);
    ecore_list_clear(expected);

    /* test multiple fullpaths */
    info->type = 'F';
    IF_FREE(desktop->exec);
    desktop->exec = strdup("app %F");
    ecore_list_append(expected, "app '/tmp/absolute_path' '/relative_path' '/tmp/absolute_uri' '/relative_uri'");

    ecore_list_first_goto(expected);
    efreet_desktop_command_get(desktop, files, _cb_command, info);
    ecore_list_clear(expected);

    /* test multiple URIs */
    info->type = 'U';
    IF_FREE(desktop->exec);
    desktop->exec = strdup("app %U");
    ecore_list_append(expected, "app 'file:///tmp/absolute_path' 'file:///relative_path' 'file:///tmp/absolute_uri' 'file:///relative_uri'");

    ecore_list_first_goto(expected);
    efreet_desktop_command_get(desktop, files, _cb_command, info);
    ecore_list_clear(expected);

    /* test multiple dirs */
    info->type = 'D';
    IF_FREE(desktop->exec);
    desktop->exec = strdup("app %D");
    ecore_list_append(expected, "app '/tmp' '/' '/tmp' '/'");

    ecore_list_first_goto(expected);
    efreet_desktop_command_get(desktop, files, _cb_command, info);
    ecore_list_clear(expected);

    /* test multiple names */
    info->type = 'N';
    IF_FREE(desktop->exec);
    desktop->exec = strdup("app %N");
    ecore_list_append(expected, "app 'absolute_path' 'relative_path' 'absolute_uri' 'relative_uri'");

    ecore_list_first_goto(expected);
    efreet_desktop_command_get(desktop, files, _cb_command, info);
    ecore_list_clear(expected);

    /* test icon appending */
    info->type = 'i';
    IF_FREE(desktop->exec);
    desktop->exec = strdup("app %i");
    ecore_list_append(expected, "app --icon 'icon.png'");

    ecore_list_first_goto(expected);
    efreet_desktop_command_get(desktop, NULL, _cb_command, info);
    ecore_list_clear(expected);

    /* test app name */
    info->type = 'c';
    IF_FREE(desktop->exec);
    desktop->exec = strdup("app %c");
    ecore_list_append(expected, "app 'App Name'");

    ecore_list_first_goto(expected);
    efreet_desktop_command_get(desktop, NULL, _cb_command, info);
    ecore_list_clear(expected);

    /* test desktop path */
    info->type = 'k';
    IF_FREE(desktop->exec);
    desktop->exec = strdup("app %k");
    ecore_list_append(expected, "app 'test.desktop'");

    ecore_list_first_goto(expected);
    efreet_desktop_command_get(desktop, NULL, _cb_command, info);
    ecore_list_clear(expected);

    /* clean up */
    efreet_desktop_free(desktop);
    ecore_list_destroy(files);
    ecore_list_destroy(expected);

    ret = info->error > 0 ? 0 : 1;
    free(info);

    chdir(olddir);

    return ret;
}

static void *
_cb_command(void *data, Efreet_Desktop *desktop __UNUSED__,
            char *exec, int remaining __UNUSED__)
{
  Test_Info *info = data;
  char *expected;

  expected = ecore_list_next(info->expected);
  if (!expected)
  {
    printf("  ERROR: (%%%c) got \"%s\", expected nothing\n", info->type, exec);
    info->error++;
  }
  else
  {
    if (strcmp(exec, expected))
    {
      printf("  ERROR: (%%%c) got \"%s\", expected \"%s\"\n", info->type, exec, expected);
      info->error++;
    }
  }

  free(exec);
  return NULL;
}

static void *
cb_type_parse(Efreet_Desktop *desktop __UNUSED__, Efreet_Ini *ini)
{
    const char *val;
    val = efreet_ini_string_get(ini, "X-Test");
    if (!val) return NULL;
    return (void *)strdup(val);
}

int
ef_cb_desktop_type_parse(void)
{
    Efreet_Desktop *desktop;
    int my_type;
    char *val;
    int ret = 1;

    /* add my custom desktop type to efreet */
    my_type = efreet_desktop_type_add("My_Type", cb_type_parse, NULL,
                                        (Efreet_Desktop_Type_Free_Cb)free);

    desktop = efreet_desktop_get(PACKAGE_DATA_DIR"/test/test_type.desktop");
    if (!desktop)
    {
        printf("No desktop found.\n");
        return 0;
    }

    if (desktop->type != my_type)
    {
        printf("Invalid type returned in desktop");
        ret = 0;
    }

    val = (char *)efreet_desktop_type_data_get(desktop);
    if (!val || strcmp(val, "Own key"))
    {
        printf("Invalid value of custom key (%s).\n", val);
        ret = 0;
    }

    efreet_desktop_free(desktop);
    return ret;
}
