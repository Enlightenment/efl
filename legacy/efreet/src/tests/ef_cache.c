#include "Efreet.h"
#include <stdio.h>
#include <Ecore.h>
#include "ef_test.h"

#if 0
EAPI Efreet_Desktop *efreet_util_desktop_file_id_find(const char *file_id);

EAPI Eina_List *efreet_util_desktop_generic_name_glob_list(const char *glob);
EAPI Eina_List *efreet_util_desktop_comment_glob_list(const char *glob);
#endif

static void
check(void)
{
    Eina_List *list;
    Efreet_Desktop *desktop;
    double start;
    const char *id;

    // EAPI char *efreet_util_path_to_file_id(const char *path);
    start = ecore_time_get();
    id = efreet_util_path_to_file_id("/usr/share/applications/gnome-panel.desktop");
    if (id)
    {
        printf("efreet_util_path_to_file_id(/usr/share/applications/gnome-panel.desktop): %s %.6f\n", id, (ecore_time_get() - start));
    }
    else
        printf("efreet_util_path_to_file_id(/usr/share/applications/gnome-panel.desktop): NULL %.6f\n", (ecore_time_get() - start));

    //EAPI Efreet_Desktop *efreet_util_desktop_name_find(const char *name);
    start = ecore_time_get();
    desktop = efreet_util_desktop_name_find("Evolution");
    if (desktop)
        printf("efreet_util_desktop_name_find(Evolution): %s %.6f\n", desktop->orig_path, (ecore_time_get() - start));
    else
        printf("efreet_util_desktop_name_find(Evolution): NULL %.6f\n", (ecore_time_get() - start));
    efreet_desktop_free(desktop);

    //EAPI Efreet_Desktop *efreet_util_desktop_generic_name_find(const char *generic_name);
    start = ecore_time_get();
    desktop = efreet_util_desktop_generic_name_find("Spreadsheet");
    if (desktop)
        printf("efreet_util_desktop_generic_name_find(Spreadsheet): %s %.6f\n", desktop->orig_path, (ecore_time_get() - start));
    else
        printf("efreet_util_desktop_generic_name_find(Spreadsheet): NULL %.6f\n", (ecore_time_get() - start));
    efreet_desktop_free(desktop);

    //EAPI Efreet_Desktop *efreet_util_desktop_wm_class_find(const char *wmname, const char *wmclass);
    start = ecore_time_get();
    desktop = efreet_util_desktop_wm_class_find("Firefox", NULL);
    if (desktop)
        printf("efreet_util_desktop_wm_class_find(Firefox): %s %.6f\n", desktop->orig_path, (ecore_time_get() - start));
    else
        printf("efreet_util_desktop_wm_class_find(Firefox): NULL %.6f\n", (ecore_time_get() - start));
    efreet_desktop_free(desktop);

    //EAPI Efreet_Desktop *efreet_util_desktop_exec_find(const char *exec);
    start = ecore_time_get();
    desktop = efreet_util_desktop_exec_find("/usr/bin/update-manager");
    if (desktop)
        printf("efreet_util_desktop_exec_find(update-manager): %s %.6f\n", desktop->orig_path, (ecore_time_get() - start));
    else
        printf("efreet_util_desktop_exec_find(update-manager): NULL %.6f\n", (ecore_time_get() - start));
    efreet_desktop_free(desktop);

    //EAPI Eina_List *efreet_util_desktop_name_glob_list(const char *glob);
    start = ecore_time_get();
    list = efreet_util_desktop_name_glob_list("Ubuntu*");
    if (list)
    {
        EINA_LIST_FREE(list, desktop)
        {
            printf("efreet_util_desktop_name_glob_list(Ubuntu*): %s\n", desktop->name);
            efreet_desktop_free(desktop);
        }
    }
    printf("time: %.6f\n", (ecore_time_get() - start));

    //EAPI Eina_List *efreet_util_desktop_mime_list(const char *mime);
    start = ecore_time_get();
    list = efreet_util_desktop_mime_list("application/ogg");
    if (list)
    {
        EINA_LIST_FREE(list, desktop)
        {
            printf("efreet_util_desktop_mime_list(application/ogg): %s\n", desktop->name);
            efreet_desktop_free(desktop);
        }
    }
    printf("time: %.6f\n", (ecore_time_get() - start));

    //EAPI Eina_List *efreet_util_desktop_exec_glob_list(const char *glob);
    start = ecore_time_get();
    list = efreet_util_desktop_exec_glob_list("*gnome*");
    if (list)
    {
        EINA_LIST_FREE(list, desktop)
        {
            printf("efreet_util_desktop_exec_glob_list(*gnome*): %s\n", desktop->exec);
            efreet_desktop_free(desktop);
        }
    }
    printf("time: %.6f\n", (ecore_time_get() - start));

    //EAPI Eina_List *efreet_util_desktop_categories_list(void);
    start = ecore_time_get();
    list = efreet_util_desktop_categories_list();
    if (list)
    {
        EINA_LIST_FREE(list, id)
        {
            printf("efreet_util_desktop_categories_list(): %s\n", id);
        }
    }
    printf("time: %.6f\n", (ecore_time_get() - start));

    //EAPI Eina_List *efreet_util_desktop_category_list(const char *category);
    start = ecore_time_get();
    list = efreet_util_desktop_category_list("Graphics");
    if (list)
    {
        EINA_LIST_FREE(list, desktop)
        {
            printf("efreet_util_desktop_category_list(Graphics): %s\n", desktop->name);
            efreet_desktop_free(desktop);
        }
    }
    printf("time: %.6f\n", (ecore_time_get() - start));

    desktop = efreet_desktop_get("/opt/google/chrome/google-chrome.desktop");
    if (desktop) efreet_desktop_free(desktop);
}

int
main(int argc __UNUSED__, char **argv __UNUSED__)
{
    if (!efreet_init()) return 1;
    check();
    ecore_main_loop_begin();
    efreet_shutdown();
    return 0;
}
