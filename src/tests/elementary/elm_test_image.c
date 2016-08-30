#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define ELM_INTERFACE_ATSPI_ACCESSIBLE_PROTECTED
#include <Elementary.h>
#include "elm_suite.h"

static const char pathfmt[] =  ELM_IMAGE_DATA_DIR"/images/icon_%02d.png";
static const char invalid[] = "thereisnosuchimage.png";
#define MAX_IMAGE_ID 23

typedef struct _Test_Data Test_Data;
struct _Test_Data
{
   int image_id;
   Eina_Bool success;
};

START_TEST (elm_atspi_role_get)
{
   Evas_Object *win, *image;
   Elm_Atspi_Role role;

   elm_init(1, NULL);
   win = elm_win_add(NULL, "image", ELM_WIN_BASIC);

   image = elm_image_add(win);
   role = elm_interface_atspi_accessible_role_get(image);

   ck_assert(role == ELM_ATSPI_ROLE_IMAGE);

   elm_shutdown();
}
END_TEST

static void
_async_error_cb(void *data, const Efl_Event *event)
{
   Test_Data *td = data;
   char path[PATH_MAX];
   sprintf(path, pathfmt, td->image_id);
   efl_file_set(event->object, path, NULL);
}

static void
_async_opened_cb(void *data, const Efl_Event *event)
{
   Test_Data *td = data;
   const char *ff, *kk, *r1, *r2;
   char path[PATH_MAX];

   sprintf(path, pathfmt, td->image_id);
   efl_file_get(event->object, &ff, &kk);
   r1 = strrchr(ff, '/');
   r2 = strrchr(path, '/');
   ck_assert(!strcmp(r1, r2));
   ck_assert(!kk);

   if (td->image_id < MAX_IMAGE_ID / 2)
     {
        td->image_id++;
        sprintf(path, pathfmt, td->image_id);
        efl_file_set(event->object, path, NULL);
        return;
     }
   else if (td->image_id < MAX_IMAGE_ID)
     {
        // mini stress-test
        for (; td->image_id < MAX_IMAGE_ID;)
          {
             sprintf(path, pathfmt, ++td->image_id);
             efl_file_set(event->object, path, NULL);
          }
        return;
     }

   td->success = 1;
   ecore_main_loop_quit();
}

static Eina_Bool
_timeout_cb(void *data)
{
   Test_Data *td = data;

   td->success = 0;
   ecore_main_loop_quit();

   return ECORE_CALLBACK_CANCEL;
}

START_TEST (elm_image_async_path)
{
   Evas_Object *win, *image;
   Eina_Bool one, two, ok;
   Test_Data td;

   elm_init(1, NULL);
   win = elm_win_add(NULL, "image", ELM_WIN_BASIC);

   td.success = 0;
   td.image_id = 0;

   image = elm_image_add(win);
   one = efl_file_async_get(image);
   efl_file_async_set(image, 1);
   efl_event_callback_add(image, EFL_FILE_EVENT_ASYNC_ERROR, _async_error_cb, &td);
   efl_event_callback_add(image, EFL_FILE_EVENT_ASYNC_OPENED, _async_opened_cb, &td);
   ok = efl_file_set(image, invalid, NULL);
   two = efl_file_async_get(image);
   ck_assert(!one && two);
   ck_assert(ok);

   ecore_timer_add(10.0, _timeout_cb, &td);

   elm_run();
   ck_assert(td.success);

   elm_shutdown();
}
END_TEST

START_TEST (elm_image_async_mmap)
{
   Evas_Object *win, *image;
   Eina_Bool ok;
   Test_Data td;
   Eina_File *f;
   char path[PATH_MAX];

   elm_init(1, NULL);
   win = elm_win_add(NULL, "image", ELM_WIN_BASIC);

   td.success = 0;
   td.image_id = 1;

   sprintf(path, pathfmt, td.image_id);
   f = eina_file_open(path, 0);
   ck_assert(f);

   image = elm_image_add(win);
   efl_file_async_set(image, 1);
   efl_event_callback_add(image, EFL_FILE_EVENT_ASYNC_ERROR, _async_error_cb, &td);
   efl_event_callback_add(image, EFL_FILE_EVENT_ASYNC_OPENED, _async_opened_cb, &td);
   ok = efl_file_mmap_set(image, f, NULL);
   ck_assert(ok);

   ecore_timer_add(10.0, _timeout_cb, &td);

   elm_run();
   ck_assert(td.success);

   eina_file_close(f);

   elm_shutdown();
}
END_TEST

void elm_test_image(TCase *tc)
{
    tcase_add_test(tc, elm_atspi_role_get);
    tcase_add_test(tc, elm_image_async_path);
    tcase_add_test(tc, elm_image_async_mmap);
}
