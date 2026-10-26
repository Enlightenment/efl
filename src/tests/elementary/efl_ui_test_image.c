#define EFL_NOLEGACY_API_SUPPORT
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Efl_Ui.h>
#include "efl_ui_suite.h"


EFL_START_TEST(efl_ui_image_test_icon)
{
   Eo *win, *image;
   Eina_Bool ok;
   const char *icon_name;

   win = win_add(NULL, "image", EFL_UI_WIN_TYPE_BASIC);

   image = efl_add(EFL_UI_IMAGE_CLASS, win);
   efl_gfx_entity_visible_set(image, EINA_TRUE);

   ok = efl_ui_image_icon_set(image, "folder");
   ck_assert(ok);
   icon_name = efl_ui_image_icon_get(image);
   ck_assert_str_eq(icon_name, "folder");

   ok = efl_ui_image_icon_set(image, "None");
   ck_assert(ok == 0);
   icon_name = efl_ui_image_icon_get(image);
   ck_assert(icon_name == NULL);
}
EFL_END_TEST

typedef struct
{
   const char *icon;
   const char *file;
   int built;
} Factory_Check;

static Eina_Value
_factory_built(Eo *o EINA_UNUSED, void *data, const Eina_Value v)
{
   Factory_Check *check = data;
   unsigned int i, len;
   Eo *item;

   EINA_VALUE_ARRAY_FOREACH(&v, len, i, item)
     {
        if (check->icon)
          ck_assert_str_eq(efl_ui_image_icon_get(item), check->icon);
        if (check->file)
          ck_assert_str_eq(efl_file_get(item), check->file);
        check->built++;
     }
   efl_loop_quit(efl_main_loop_get(), EINA_VALUE_EMPTY);
   return v;
}

static Eina_Value
_factory_failed(Eo *o EINA_UNUSED, void *data EINA_UNUSED, Eina_Error err)
{
   efl_loop_quit(efl_main_loop_get(), EINA_VALUE_EMPTY);
   return eina_value_error_init(err);
}

static void
_factory_build(Eo *factory, Eo *child, Factory_Check *check)
{
   Eo *models[] = { child };
   Eina_Future *f;

   f = efl_ui_view_factory_create_with_event(factory, EINA_C_ARRAY_ITERATOR_NEW(models));
   efl_future_then(factory, f, .success = _factory_built, .error = _factory_failed,
                   .success_type = EINA_VALUE_TYPE_ARRAY, .data = check);
   ecore_main_loop_begin();
}

/* An image factory binds the property its key names, as every factory does:
   "icon" shows a named icon, and a file -- the key "filename", or none -- is
   what it always did. */
EFL_START_TEST(efl_ui_image_factory_binds_its_key)
{
   Factory_Check by_icon = { "folder", NULL, 0 };
   Factory_Check by_file = { NULL, ELM_IMAGE_DATA_DIR"/images/logo.png", 0 };
   Eo *win, *model, *child, *factory;
   Eina_Value *v;

   win = win_add(NULL, "image", EFL_UI_WIN_TYPE_BASIC);
   model = efl_add(EFL_GENERIC_MODEL_CLASS, win);
   child = efl_model_child_add(model);
   v = eina_value_string_new("folder");
   efl_model_property_set(child, "sky", v);
   eina_value_free(v);
   v = eina_value_string_new(ELM_IMAGE_DATA_DIR"/images/logo.png");
   efl_model_property_set(child, "path", v);
   eina_value_free(v);

   factory = efl_add(EFL_UI_IMAGE_FACTORY_CLASS, win);
   efl_ui_property_bind(factory, "icon", "sky");
   _factory_build(factory, child, &by_icon);
   ck_assert_int_eq(by_icon.built, 1);

   factory = efl_add(EFL_UI_IMAGE_FACTORY_CLASS, win);
   efl_ui_property_bind(factory, "", "path");
   _factory_build(factory, child, &by_file);
   ck_assert_int_eq(by_file.built, 1);
}
EFL_END_TEST

extern Eo *elm_image_object_get(Eo*);
EFL_START_TEST(efl_ui_image_test_border_insets_before_load)
{
   Eo *win, *image;
   int l, r, t, b;

   win = win_add(NULL, "image", EFL_UI_WIN_TYPE_BASIC);
   image = efl_add(EFL_UI_IMAGE_CLASS, win);

   /* The file can be selected before the image is actually loaded. Border
    * insets set in that window must not be lost when loading happens. */
   efl_file_set(image, ELM_IMAGE_DATA_DIR"/images/logo.png");
   efl_gfx_image_border_insets_set(image, 50, 50, 50, 50);

   efl_gfx_image_border_insets_get(image, &l, &r, &t, &b);
   ck_assert_int_eq(l, 50);
   ck_assert_int_eq(r, 50);
   ck_assert_int_eq(t, 50);
   ck_assert_int_eq(b, 50);

   ck_assert_int_eq(efl_file_load(image), 0);

   efl_gfx_image_border_insets_get(image, &l, &r, &t, &b);
   ck_assert_int_eq(l, 50);
   ck_assert_int_eq(r, 50);
   ck_assert_int_eq(t, 50);
   ck_assert_int_eq(b, 50);
}
EFL_END_TEST

EFL_START_TEST(efl_ui_image_test_scale_method)
{
   Eo *win, *image;
   Eina_Size2D sz;

   win = win_add(NULL, "image", EFL_UI_WIN_TYPE_BASIC);
   efl_gfx_entity_size_set(win, EINA_SIZE2D(100, 100));

   image = efl_add(EFL_UI_IMAGE_CLASS, win,
      efl_file_set(efl_added, ELM_IMAGE_DATA_DIR"/images/logo.png"),
      efl_gfx_arrangement_content_align_set(efl_added, 0.5, 0.0),
      efl_gfx_image_scale_method_set(efl_added, EFL_GFX_IMAGE_SCALE_METHOD_FIT_WIDTH)
   );
   efl_gfx_entity_size_set(image, EINA_SIZE2D(100, 100));
   get_me_to_those_events(win);
   sz = efl_gfx_entity_size_get(image);
   ck_assert_int_eq(sz.w, 100);
   ck_assert_int_eq(sz.h, 100);
   /* legacy operation on eo object: very illegal */
   sz = efl_gfx_entity_size_get(elm_image_object_get(image));
   ck_assert_int_eq(sz.w, 100);
   ck_assert_int_eq(sz.h, 100);
}
EFL_END_TEST

/* A load size is a promise that the image will not be decoded larger than it
 * has to be. The raster loaders only ever read the scale-down divisor -- the
 * PNG one reads opts->emile.scale_down_by and the region and never the
 * requested width and height -- so before the widget worked a divisor out from
 * the header the promise was empty: the getter answered with what had been
 * asked for and the whole image was decoded anyway.
 *
 * Measured once by hand on an 8000x8000 PNG: 246 MB decoded as it comes, 244 MB
 * with a load size, 1 MB with a scale-down. This test uses a small image on
 * purpose -- a test that reproduces the 246 MB is a test that will one day run
 * on a machine without room for it.
 */
EFL_START_TEST(efl_ui_image_test_load_size_bounds_the_decode)
{
   Eo *win, *plain, *bounded;
   Eina_Size2D natural, small, asked = EINA_SIZE2D(100, 100);

   win = win_add(NULL, "image", EFL_UI_WIN_TYPE_BASIC);

   plain = efl_add(EFL_UI_IMAGE_CLASS, win);
   ck_assert(efl_file_simple_load(plain, ELM_IMAGE_DATA_DIR"/images/logo.png", NULL));
   natural = efl_gfx_view_size_get(plain);
   ck_assert_int_gt(natural.w, asked.w * 2);
   ck_assert_int_gt(natural.h, asked.h * 2);

   bounded = efl_add(EFL_UI_IMAGE_CLASS, win);
   efl_gfx_image_load_controller_load_size_set(bounded, asked);
   ck_assert(efl_file_simple_load(bounded, ELM_IMAGE_DATA_DIR"/images/logo.png", NULL));
   small = efl_gfx_view_size_get(bounded);

   /* Smaller than the file, and no smaller than what was asked for: a request
      is a floor, not a target, because a loader may only be able to halve. */
   ck_assert_int_lt(small.w, natural.w);
   ck_assert_int_lt(small.h, natural.h);
   ck_assert_int_ge(small.w, asked.w);
   ck_assert_int_ge(small.h, asked.h);

   /* The property still answers with what was asked for, not with what the
      loader managed. */
   ck_assert_int_eq(efl_gfx_image_load_controller_load_size_get(bounded).w, asked.w);

   /* elm_image_prescale_set reaches the same property and is deliberately kept
      on the old behaviour -- legacy is frozen and prescale has never bounded a
      raster decode. It is not asserted here because this file is built without
      the legacy API. */
}
EFL_END_TEST

static void
_count_errors(const Eina_Log_Domain *d, Eina_Log_Level level,
              const char *file, const char *fnc, int line,
              const char *fmt, void *data, va_list args)
{
   int *errors = data;

   if (level <= EINA_LOG_LEVEL_ERR)
     (*errors)++;
   eina_log_print_cb_stderr(d, level, file, fnc, line, fmt, NULL, args);
}

EFL_START_TEST(efl_ui_image_test_mmap_set_null)
{
   Eo *win, *image;
   int errors = 0;

   win = win_add(NULL, "image", EFL_UI_WIN_TYPE_BASIC);

   image = efl_add(EFL_UI_IMAGE_CLASS, win);
   ck_assert(efl_file_simple_load(image, ELM_IMAGE_DATA_DIR"/images/logo.png", NULL));
   ck_assert_ptr_ne(efl_file_get(image), NULL);

   eina_log_print_cb_set(_count_errors, &errors);
   efl_file_mmap_set(image, NULL);
   eina_log_print_cb_set(eina_log_print_cb_stderr, NULL);

   ck_assert_int_eq(errors, 0);
   ck_assert_ptr_eq(efl_file_mmap_get(image), NULL);
   ck_assert_ptr_eq(efl_file_get(image), NULL);
}
EFL_END_TEST

void efl_ui_test_image(TCase *tc)
{
   tcase_add_test(tc, efl_ui_image_test_icon);
   tcase_add_test(tc, efl_ui_image_test_border_insets_before_load);
   tcase_add_test(tc, efl_ui_image_factory_binds_its_key);
   tcase_add_test(tc, efl_ui_image_test_scale_method);
   tcase_add_test(tc, efl_ui_image_test_load_size_bounds_the_decode);
   tcase_add_test(tc, efl_ui_image_test_mmap_set_null);
}
