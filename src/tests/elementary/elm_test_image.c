#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_OBJECT_BETA
#include <Efl_Ui.h>
#include <Elementary.h>
#include "elm_suite.h"

static const char pathfmt[] =  ELM_IMAGE_DATA_DIR"/images/icon_%02d.png";
static const char invalid[] = "thereisnosuchimage.png";
#define MAX_IMAGE_ID 23

typedef struct _Test_Data Test_Data;
struct _Test_Data
{
   int image_id;
   int success;
};

static int
_file_to_memory(const char *filename, char **result)
{
   int size;
   FILE *f;

   f = fopen(filename, "rb");
   if (f == NULL)
     {
        *result = NULL;
        return -1;
     }

   fseek(f, 0, SEEK_END);
   size = ftell(f);
   fseek(f, 0, SEEK_SET);
   *result = (char *)malloc(size + 1);
   if ((size_t)size != fread(*result, sizeof(char), size, f))
     {
        free(*result);
        fclose(f);
        return -1;
     }
   fclose(f);
   (*result)[size] = 0;
   return size;
}

EFL_START_TEST(elm_image_legacy_type_check)
{
   Evas_Object *win, *image;
   const char *type;

   win = win_add(NULL, "image", ELM_WIN_BASIC);

   image = elm_image_add(win);

   type = elm_object_widget_type_get(image);
   ck_assert(type != NULL);
   ck_assert(!strcmp(type, "Elm_Image"));

   type = evas_object_type_get(image);
   ck_assert(type != NULL);
   ck_assert(!strcmp(type, "elm_image"));

}
EFL_END_TEST

EFL_START_TEST(elm_atspi_role_get)
{
   Evas_Object *win, *image;
   Efl_Access_Role role;

   win = win_add(NULL, "image", ELM_WIN_BASIC);

   image = elm_image_add(win);
   role = efl_access_object_role_get(image);

   ck_assert(role == EFL_ACCESS_ROLE_IMAGE);

}
EFL_END_TEST

static void
_async_error_cb(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Test_Data *td = data;
   char path[PATH_MAX];
   sprintf(path, pathfmt, td->image_id);
   elm_image_file_set(obj, path, NULL);

   td->success = 1;
}

static void
_async_opened_cb(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Test_Data *td = data;
   const char *ff, *kk, *r1, *r2;
   char path[PATH_MAX];

   sprintf(path, pathfmt, td->image_id);
   elm_image_file_get(obj, &ff, &kk);
   r1 = strrchr(ff, '/');
   r2 = strrchr(path, '/');
   ck_assert(eina_streq(r1, r2));
   ck_assert(!kk);

   fprintf(stderr, "opened: %s\n", ff);

   if (td->image_id < MAX_IMAGE_ID / 2)
     {
        td->image_id++;
        sprintf(path, pathfmt, td->image_id);
        elm_image_file_set(obj, path, NULL);
        return;
     }
   else if (td->image_id < MAX_IMAGE_ID)
     {
        // mini stress-test
        for (; td->image_id < MAX_IMAGE_ID;)
          {
             sprintf(path, pathfmt, ++td->image_id);
             elm_image_file_set(obj, path, NULL);
          }
        return;
     }

   if (td->success == 1)
     td->success = 2;
}

static void
_async_ready_cb(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Test_Data *td = data;

   const char *ff;
   elm_image_file_get(obj, &ff, 0);
   fprintf(stderr, "ready : %s\n", ff);
   if (td->success == 2)
     {
        td->success = 3;
        ecore_main_loop_quit();
     }
}

static Eina_Bool
_timeout_cb(void *data)
{
   Test_Data *td = data;

   td->success = 0;
   ecore_main_loop_quit();

   return ECORE_CALLBACK_CANCEL;
}

EFL_START_TEST(elm_image_async_path)
{
   Evas_Object *win, *image;
   Ecore_Timer *t;
   Eina_Bool ok;
   Test_Data td;

   win = win_add(NULL, "image", ELM_WIN_BASIC);

   td.success = 0;
   td.image_id = 0;

   image = elm_image_add(win);
   elm_image_async_open_set(image, 1);
   evas_object_smart_callback_add(image, "load,error", _async_error_cb, &td);
   evas_object_smart_callback_add(image, "load,open", _async_opened_cb, &td);
   evas_object_smart_callback_add(image, "load,ready", _async_ready_cb, &td);
   evas_object_show(image);
   ok = elm_image_file_set(image, invalid, NULL);
   ck_assert(ok);

   t = ecore_timer_add(10.0, _timeout_cb, &td);

   elm_run();
   ck_assert(td.success == 3);

   ecore_timer_del(t);
}
EFL_END_TEST

EFL_START_TEST(elm_image_async_mmap)
{
   Evas_Object *win, *image;
   Ecore_Timer *t;
   Eina_Bool ok;
   Test_Data td;
   Eina_File *f;
   char path[PATH_MAX];

   win = win_add(NULL, "image", ELM_WIN_BASIC);

   td.success = 1; // skip "error" case
   td.image_id = 1;

   sprintf(path, pathfmt, td.image_id);
   f = eina_file_open(path, 0);
   ck_assert(f);

   image = elm_image_add(win);
   elm_image_async_open_set(image, 1);
   evas_object_smart_callback_add(image, "load,error", _async_error_cb, &td);
   evas_object_smart_callback_add(image, "load,open", _async_opened_cb, &td);
   evas_object_smart_callback_add(image, "load,ready", _async_ready_cb, &td);
   evas_object_show(image);
   ok = elm_image_mmap_set(image, f, NULL);
   ck_assert(ok);

   t = ecore_timer_add(10.0, _timeout_cb, &td);

   elm_run();
   ck_assert(td.success == 3);

   eina_file_close(f);

   ecore_timer_del(t);
}
EFL_END_TEST

EFL_START_TEST(elm_image_evas_object_color_set)
{
   Evas_Object *win, *image;
   int r = 128, g = 99, b = 3, a = 230;
   int rr = 0, gg = 0, bb = 0, aa = 0;

   win = win_add(NULL, "image", ELM_WIN_BASIC);

   image = elm_image_add(win);
   evas_object_color_set(image, r, g, b, a);
   evas_object_color_get(image, &rr, &gg, &bb, &aa);
   ck_assert(r == rr);
   ck_assert(g == gg);
   ck_assert(b == bb);
   ck_assert(a == aa);
}
EFL_END_TEST

EFL_START_TEST(elm_image_evas_image_get)
{
   Evas_Object *win, *image, *obj;

   win = win_add(NULL, "image", ELM_WIN_BASIC);

   image = elm_image_add(win);
   obj = elm_image_object_get(image);

   ck_assert(obj);
}
EFL_END_TEST

EFL_START_TEST(elm_image_test_memfile_set)
{
   Evas_Object *win, *image, *image2;
   char *mem;
   int size;
   const char *file = NULL;
   int error_called = 0;

   win = win_add(NULL, "image", ELM_WIN_BASIC);

   image = elm_image_add(win);
   ck_assert(elm_image_file_set(image, ELM_IMAGE_DATA_DIR"/images/icon_01.png", NULL));
   size = _file_to_memory(ELM_IMAGE_DATA_DIR"/images/icon_02.png", &mem);
   ck_assert_int_ge(size, 0);
   ck_assert(elm_image_memfile_set(image, mem, size, "png", NULL));
   elm_image_file_get(image, &file, NULL);
   ck_assert_str_ne(file, ELM_IMAGE_DATA_DIR"/images/icon_01.png");
   ck_assert(elm_image_file_set(image, ELM_IMAGE_DATA_DIR"/images/icon_01.png", NULL));
   elm_image_file_get(image, &file, NULL);
   ck_assert_str_eq(file, ELM_IMAGE_DATA_DIR"/images/icon_01.png");

   image2 = elm_image_add(win);
   evas_object_smart_callback_add(image2, "load,ready", event_callback_that_quits_the_main_loop_when_called, NULL);
   evas_object_smart_callback_add(image2, "load,error", event_callback_single_call_int_data, &error_called);
   ck_assert(elm_image_memfile_set(image2, mem, size, "png", NULL));
   ck_assert_int_eq(error_called, 0);
   ecore_main_loop_begin();

   ck_assert_int_eq(error_called, 0);
}
EFL_END_TEST

EFL_START_TEST(elm_image_test_scale_method)
{
   Evas_Object *win, *image;
   int w, h;

   win = win_add(NULL, "image", ELM_WIN_BASIC);
   evas_object_resize(win, 100, 100);

   image = elm_image_add(win);
   ck_assert(elm_image_file_set(image, ELM_IMAGE_DATA_DIR"/images/logo.png", NULL));
   evas_object_size_hint_align_set(image, 0.5, 0.0);
   efl_gfx_image_scale_method_set(image, EFL_GFX_IMAGE_SCALE_METHOD_FIT_WIDTH);
   evas_object_resize(image, 100, 100);
   evas_object_show(image);
   evas_object_show(win);
   get_me_to_those_events(win);
   evas_object_geometry_get(image, NULL, NULL, &w, &h);
   ck_assert_int_eq(w, 100);
   ck_assert_int_eq(h, 100);
   evas_object_geometry_get(elm_image_object_get(image), NULL, NULL, &w, &h);
   ck_assert_int_eq(w, 100);
   ck_assert_int_eq(h, 100);
}
EFL_END_TEST

#ifdef BUILD_LOADER_GIF
static void
_test_render(void *data, Evas *e EINA_UNUSED, void *event_info)
{
   int *pass = data;
   Evas_Event_Render_Post *ev = event_info;
   *pass = eina_list_count(ev->updated_area);
   ecore_main_loop_quit();
}

static void
_test_preload(void *data, Evas *e, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   if (evas_object_image_load_error_get(obj) == EVAS_LOAD_ERROR_NONE)
     evas_event_callback_add(e, EVAS_CALLBACK_RENDER_POST, _test_render, data);
   else
     ecore_main_loop_quit();
}

EFL_START_TEST(elm_image_test_gif)
{
   Evas_Object *win, *image;
   int pass = 0;

   win = win_add(NULL, "image", ELM_WIN_BASIC);

   image = elm_image_add(win);
   evas_object_resize(win, 100, 100);
   evas_object_resize(image, 100, 100);
   evas_object_show(win);
   evas_object_show(image);

   get_me_to_those_events(win);
   ck_assert(elm_image_file_set(image, ELM_IMAGE_DATA_DIR"/images/fire.gif", NULL));
   elm_image_animated_set(image, EINA_TRUE);
   elm_image_animated_play_set(image, EINA_TRUE);
   evas_object_event_callback_add(elm_image_object_get(image), EVAS_CALLBACK_IMAGE_PRELOADED, _test_preload, &pass);
   /* verify that we haven't tried to set a frame with index 0, as this is an error */
   ck_assert_int_gt(evas_object_image_animated_frame_get(elm_image_object_get(image)), 0);
   ecore_main_loop_begin();
   ck_assert_int_gt(pass, 0);
}
EFL_END_TEST

#endif

void elm_test_image(TCase *tc)
{
   tcase_add_test(tc, elm_image_legacy_type_check);
   tcase_add_test(tc, elm_atspi_role_get);
   tcase_add_test(tc, elm_image_async_path);
   tcase_add_test(tc, elm_image_async_mmap);
   tcase_add_test(tc, elm_image_evas_object_color_set);
   tcase_add_test(tc, elm_image_evas_image_get);
   tcase_add_test(tc, elm_image_test_memfile_set);
   tcase_add_test(tc, elm_image_test_scale_method);
#ifdef BUILD_LOADER_GIF
   tcase_add_test(tc, elm_image_test_gif);
#endif
}
