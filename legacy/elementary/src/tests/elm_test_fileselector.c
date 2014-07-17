#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_suite.h"

#define ELM_INTERFACE_ATSPI_ACCESSIBLE_PROTECTED
#include "elm_interface_atspi_accessible.h"
#include "elm_interface_atspi_accessible.eo.h"


START_TEST (elm_atspi_role_get)
{
   Evas_Object *win, *fileselector;
   Elm_Atspi_Role role;

   elm_init(1, NULL);
   win = elm_win_add(NULL, "fileselector", ELM_WIN_BASIC);

   fileselector = elm_fileselector_add(win);
   eo_do(fileselector, role = elm_interface_atspi_accessible_role_get());

   ck_assert(role == ELM_ATSPI_ROLE_FILE_CHOOSER);

   elm_shutdown();
}
END_TEST

static void
_directory_open_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
    Eina_Bool *ret = data;
    *ret = EINA_TRUE;
}

START_TEST (elm_fileselector_selected)
{
   Evas_Object *win, *fileselector;
   Eina_Tmpstr *tmp_path;
   Eina_Stringshare *exist, *no_exist;
   FILE *fp;
   char *path;
   Eina_Bool selected;

   elm_init(1, NULL);

   if (!eina_file_mkdtemp("elm_test-XXXXXX", &tmp_path))
     {
        /* can not test */
        ck_assert(EINA_FALSE);
        return;
     }

   path = strdup(tmp_path);
   eina_tmpstr_del(tmp_path);

   exist = eina_stringshare_printf("%s/exist", path);
   no_exist = eina_stringshare_printf("%s/no_exist", path);
   fp = fopen(exist, "w");
   fclose(fp);

   win = elm_win_add(NULL, "fileselector", ELM_WIN_BASIC);

   fileselector = elm_fileselector_add(win);
   evas_object_smart_callback_add(fileselector, "directory,open", _directory_open_cb, &selected);

   ck_assert(!elm_fileselector_selected_set(fileselector, no_exist));

   selected = EINA_FALSE;
   ck_assert(elm_fileselector_selected_set(fileselector, path));
   ck_assert(elm_test_helper_wait_flag(10, &selected));

   ck_assert_str_eq(elm_fileselector_selected_get(fileselector), path);

   selected = EINA_FALSE;
   ck_assert(elm_fileselector_selected_set(fileselector, exist));
   ck_assert(elm_test_helper_wait_flag(10, &selected));

   ck_assert_str_eq(elm_fileselector_selected_get(fileselector), exist);

   eina_stringshare_del(exist);
   eina_stringshare_del(no_exist);
   free(path);

   elm_shutdown();
}
END_TEST

void elm_test_fileselector(TCase *tc)
{
   tcase_add_test(tc, elm_atspi_role_get);
   tcase_add_test(tc, elm_fileselector_selected);
}

