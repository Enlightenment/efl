#include "Efreet.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int
ef_cb_efreet_data_home(void)
{
    const char *tmp;
    int ret = 1;

    putenv("XDG_DATA_HOME=/var/tmp");

    tmp = efreet_data_home_get();
    if (strcmp(tmp, "/var/tmp"))
    {
        printf("efreet_data_home_get() returned incorrect "
                "value on XDG_DATA_HOME=/var/tmp\n");
        ret = 0;
    }

    /* reset efreet here so we can set a new home dir */
    efreet_shutdown();
    efreet_init();

    putenv("XDG_DATA_HOME=");
    putenv("HOME=/home/tmp");

    tmp = efreet_data_home_get();
    if (strcmp(tmp, "/home/tmp/.local/share"))
    {
        printf("efreet_data_home_get() returned incorrect "
                "value on blank XDG_DATA_HOME\n");
        ret = 0;
    }

    /* reset efreet here so we can set a new home dir */
    efreet_shutdown();
    efreet_init();

    putenv("XDG_DATA_HOME=");
    putenv("HOME=");

    tmp = efreet_data_home_get();
    if (strcmp(tmp, "/tmp/.local/share"))
    {
        printf("efreet_data_home_get() returned incorrect "
                "value (%s) on blank XDG_DATA_HOME and blank HOME\n", tmp);
        ret = 0;
    }

    return ret;
}

int
ef_cb_efreet_config_home(void)
{
    const char *tmp;
    int ret = 1;

    putenv("XDG_CONFIG_HOME=/var/tmp");

    tmp = efreet_config_home_get();
    if (strcmp(tmp, "/var/tmp"))
    {
        printf("efreet_config_home_get() returned incorrect "
                "value on XDG_CONFIG_HOME=/var/tmp\n");
        ret = 0;
    }

    /* reset efreet here so we can set a new home dir */
    efreet_shutdown();
    efreet_init();

    putenv("XDG_CONFIG_HOME=");
    putenv("HOME=/home/tmp");

    tmp = efreet_config_home_get();
    if (strcmp(tmp, "/home/tmp/.config"))
    {
        printf("efreet_config_home_get() returned incorrect "
                "value on blank XDG_CONFIG_HOME\n");
        ret = 0;
    }

    /* reset efreet here so we can set a new home dir */
    efreet_shutdown();
    efreet_init();

    putenv("XDG_CONFIG_HOME=");
    putenv("HOME=");

    tmp = efreet_config_home_get();
    if (strcmp(tmp, "/tmp/.config"))
    {
        printf("efreet_config_home_get() returned incorrect "
                "value (%s) on blank XDG_CONFIG_HOME and blank HOME\n", tmp);
        ret = 0;
    }

    return ret;
}

int
ef_cb_efreet_cache_home(void)
{
    const char *tmp;
    int ret = 1;

    putenv("XDG_CACHE_HOME=/var/tmp");

    tmp = efreet_cache_home_get();
    if (strcmp(tmp, "/var/tmp"))
    {
        printf("efreet_cache_home_get() returned incorrect "
                "value on XDG_CACHE_HOME=/var/tmp\n");
        ret = 0;
    }

    /* reset efreet here so we can set a new home dir */
    efreet_shutdown();
    efreet_init();

    putenv("XDG_CACHE_HOME=");
    putenv("HOME=/home/tmp");

    tmp = efreet_cache_home_get();
    if (strcmp(tmp, "/home/tmp/.cache"))
    {
        printf("efreet_cache_home_get() returned incorrect "
                "value on blank XDG_CACHE_HOME\n");
        ret = 0;
    }

    /* reset efreet here so we can set a new home dir */
    efreet_shutdown();
    efreet_init();

    putenv("XDG_CACHE_HOME=");
    putenv("HOME=");

    tmp = efreet_cache_home_get();
    if (strcmp(tmp, "/tmp/.cache"))
    {
        printf("efreet_cache_home_get() returned incorrect "
                "value (%s) on blank XDG_CACHE_HOME and blank HOME\n", tmp);
        ret = 0;
    }

    return ret;
}

int
ef_cb_efreet_data_dirs(void)
{
    Ecore_List *tmp;
    int ret = 1, i;
    char dirs[128], *val;
    char *vals[] = {"/var/tmp/a", "/tmp/b", "/usr/local/share", "/etc", NULL};
    char *def_vals[] = {"/usr/local/share", "/usr/share", NULL};

    dirs[0] = '\0';
    strcat(dirs, "XDG_DATA_DIRS=");
    for (i = 0; vals[i] != NULL; i++)
    {
        if (i > 0) strcat(dirs, ":");
        strcat(dirs, vals[i]);
    }

    putenv(dirs);

    i = 0;
    tmp = efreet_data_dirs_get();
    ecore_list_goto_first(tmp);
    while ((val = ecore_list_next(tmp)))
    {
        if (vals[i] == NULL)
        {
            printf("efreet_data_dirs_get() returned more values then it "
                    "should have given %s as input\n", dirs);
            ret = 0;
            break;
        }

        if (strcmp(val, vals[i]))
        {
            printf("efreet_data_dirs_get() returned incorrect value (%s) when "
                    "%s set\n", val, dirs);
            ret = 0;
        }

        i++;
    }

    efreet_shutdown();
    efreet_init();

    putenv("XDG_DATA_DIRS=");

    i = 0;
    tmp = efreet_data_dirs_get();
    ecore_list_goto_first(tmp);
    while ((val = ecore_list_next(tmp)))
    {
        if (def_vals[i] == NULL)
        {
            printf("efreet_data_dirs_get() returned more values then it "
                    "should have given %s as input\n", dirs);
            ret = 0;
            break;
        }

        if (strcmp(val, def_vals[i]))
        {
            printf("efreet_data_dirs_get() returned incorrect value (%s) when "
                    "XDG_DATA_DIRS= is set\n", val);
            ret = 0;
        }

        i++;
    }
    return ret;
}

int
ef_cb_efreet_config_dirs(void)
{
    Ecore_List *tmp;
    int ret = 1, i;
    char dirs[128], *val;
    char *vals[] = {"/var/tmp/a", "/tmp/b", "/usr/local/share", "/etc", NULL};
    char *def_vals[] = {"/etc/xdg", NULL};
    
    dirs[0] = '\0';

    strcat(dirs, "XDG_CONFIG_DIRS=");
    for (i = 0; vals[i] != NULL; i++)
    {
        if (i > 0) strcat(dirs, ":");
        strcat(dirs, vals[i]);
    }

    putenv(dirs);

    i = 0;
    tmp = efreet_config_dirs_get();
    ecore_list_goto_first(tmp);
    while ((val = ecore_list_next(tmp)))
    {
        if (vals[i] == NULL)
        {
            printf("efreet_config_dirs_get() returned more values then it "
                    "should have given %s as input\n", dirs);
            ret = 0;
            break;
        }

        if (strcmp(val, vals[i]))
        {
            printf("efreet_config_dirs_get() returned incorrect value (%s) when "
                    "%s set\n", val, dirs);
            ret = 0;
        }

        i++;
    }

    efreet_shutdown();
    efreet_init();

    putenv("XDG_CONFIG_DIRS=");

    i = 0;
    tmp = efreet_config_dirs_get();
    ecore_list_goto_first(tmp);
    while ((val = ecore_list_next(tmp)))
    {
        if (def_vals[i] == NULL)
        {
            printf("efreet_config_dirs_get() returned more values then it "
                    "should have given %s as input\n", dirs);
            ret = 0;
            break;
        }

        if (strcmp(val, def_vals[i]))
        {
            printf("efreet_config_dirs_get() returned incorrect value (%s) when "
                    "XDG_CONFIG_DIRS= is set\n", val);
            ret = 0;
        }

        i++;
    }
    return ret;
}

