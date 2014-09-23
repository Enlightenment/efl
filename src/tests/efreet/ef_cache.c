#include "config.h" 

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <Ecore.h>

#include "Efreet.h"
#include "ef_test.h"

#if 0
EAPI Efreet_Desktop *efreet_util_desktop_file_id_find(const char *file_id);

EAPI Eina_List *efreet_util_desktop_generic_name_glob_list(const char *glob);
EAPI Eina_List *efreet_util_desktop_comment_glob_list(const char *glob);
#endif

static Eina_Bool icon_cb = EINA_FALSE;
static Eina_Bool desktop_cb = EINA_FALSE;

static void
check(void)
{
   Eina_List *list;
   Efreet_Desktop *desktop, *desktop2;
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
   if (desktop)
     printf("%s: %d %d\n", desktop->orig_path, desktop->ref, desktop->eet);
   desktop2 = efreet_desktop_new("/opt/google/chrome/google-chrome.desktop");
   if (desktop2)
     {
        printf("%s: %d %d\n", desktop2->orig_path, desktop2->ref, desktop2->eet);
        efreet_desktop_free(desktop2);
     }
   if (desktop)
     efreet_desktop_free(desktop);

   desktop = efreet_desktop_get("/usr/share/applications/firefox.desktop");
   if (desktop)
     printf("%s: %d %d\n", desktop->orig_path, desktop->ref, desktop->eet);
   desktop2 = efreet_desktop_new("/usr/share/applications/firefox.desktop");
   if (desktop2)
     {
        printf("%s: %d %d\n", desktop2->orig_path, desktop2->ref, desktop2->eet);
        efreet_desktop_free(desktop2);
     }
   if (desktop)
     efreet_desktop_free(desktop);
   fflush(stdout);
}

static Eina_Bool
icon_handler_cb(void *data EINA_UNUSED, int event_type EINA_UNUSED, void *event EINA_UNUSED)
{
   icon_cb = EINA_TRUE;
   if (icon_cb && desktop_cb)
     {
        check();
        ecore_main_loop_quit();
     }
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
desktop_handler_cb(void *data EINA_UNUSED, int event_type EINA_UNUSED, void *event EINA_UNUSED)
{
   desktop_cb = EINA_TRUE;
   if (icon_cb && desktop_cb)
     {
        check();
        ecore_main_loop_quit();
     }
   return ECORE_CALLBACK_PASS_ON;
}

int
main(int argc EINA_UNUSED, char **argv EINA_UNUSED)
{
   Ecore_Event_Handler *icon_handler;
   Ecore_Event_Handler *desktop_handler;

   if (!efreet_init()) return 1;
   icon_handler = ecore_event_handler_add(EFREET_EVENT_ICON_CACHE_UPDATE, icon_handler_cb, NULL);
   desktop_handler = ecore_event_handler_add(EFREET_EVENT_DESKTOP_CACHE_UPDATE, desktop_handler_cb, NULL);
   check();
   ecore_main_loop_begin();
   ecore_event_handler_del(icon_handler);
   ecore_event_handler_del(desktop_handler);
   efreet_shutdown();
   return 0;
}
