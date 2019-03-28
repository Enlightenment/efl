#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_OBJECT_BETA
#include <Elementary.h>
#include "elm_suite.h"


EFL_START_TEST(elm_fileselector_legacy_type_check)
{
   Evas_Object *win, *fileselector;
   const char *type;

   win = win_add(NULL, "fileselector", ELM_WIN_BASIC);

   fileselector = elm_fileselector_add(win);

   type = elm_object_widget_type_get(fileselector);
   ck_assert(type != NULL);
   ck_assert(!strcmp(type, "Elm_Fileselector"));

   type = evas_object_type_get(fileselector);
   ck_assert(type != NULL);
   ck_assert(!strcmp(type, "elm_fileselector"));

}
EFL_END_TEST

EFL_START_TEST(elm_atspi_role_get)
{
   Evas_Object *win, *fileselector;
   Efl_Access_Role role;

   win = win_add(NULL, "fileselector", ELM_WIN_BASIC);

   fileselector = elm_fileselector_add(win);
   role = efl_access_object_role_get(fileselector);

   ck_assert(role == EFL_ACCESS_ROLE_FILE_CHOOSER);

}
EFL_END_TEST

#if 0

static Eina_Bool
timer_expired_cb(void *user_data)
{
   Eina_Bool *did_timeout = user_data;

   *did_timeout = EINA_TRUE;
   ecore_main_loop_quit();

   return EINA_TRUE;
}

static Eina_Bool
idler_done_cb(void *user_data)
{
   Eina_Bool *done = user_data;

   if (*done) ecore_main_loop_quit();

   return EINA_TRUE;
}

static Eina_Bool
fileselector_test_helper_wait_flag(double in, Eina_Bool *done)
{
   Eina_Bool did_timeout = EINA_FALSE;
   Ecore_Timer *tm;
   Ecore_Idle_Enterer *idle;

   tm = ecore_timer_add(in, timer_expired_cb, &did_timeout);
   idle = ecore_idle_enterer_add(idler_done_cb, done);

   ecore_main_loop_begin();

   ecore_idle_enterer_del(idle);
   ecore_timer_del(tm);

   return !did_timeout;
}

static void
_ready_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
    Eina_Bool *ret = data;
    *ret = EINA_TRUE;

    ecore_main_loop_quit();
}


EFL_START_TEST(elm_fileselector_selected)
{
   Evas_Object *win, *fileselector;
   Eina_Tmpstr *tmp_path;
   Eina_Stringshare *exist, *no_exist;
   FILE *fp;
   char *path;
   Eina_Bool open, selected;

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

   win = win_add(NULL, "fileselector", ELM_WIN_BASIC);

   fileselector = elm_fileselector_add(win);
   evas_object_smart_callback_add(fileselector, "directory,open", _ready_cb, &open);

   ck_assert(!elm_fileselector_selected_set(fileselector, no_exist));

   open = EINA_FALSE;
   ck_assert(elm_fileselector_selected_set(fileselector, path));
   ck_assert(fileselector_test_helper_wait_flag(10, &open));

   ck_assert_str_eq(elm_fileselector_selected_get(fileselector), path);

   evas_object_smart_callback_del(fileselector, "directory,open", _ready_cb);
   evas_object_smart_callback_add(fileselector, "selected", _ready_cb, &selected);

   selected = EINA_FALSE;
   ck_assert(elm_fileselector_selected_set(fileselector, exist));
   ck_assert(fileselector_test_helper_wait_flag(10, &selected));
   ck_assert(selected == EINA_TRUE);

   ck_assert_str_eq(elm_fileselector_selected_get(fileselector), exist);

   eina_stringshare_del(exist);
   eina_stringshare_del(no_exist);
   free(path);

}
EFL_END_TEST

#endif

void elm_test_fileselector(TCase *tc)
{
   tcase_add_test(tc, elm_fileselector_legacy_type_check);
   tcase_add_test(tc, elm_atspi_role_get);
   //FIXME this should be reenabled when issues on travis are fixed with this.
   //tcase_add_test(tc, elm_fileselector_selected);
}

