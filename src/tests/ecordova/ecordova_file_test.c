#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ecordova_file_tests.h"
#include "ecordova_suite.h"

static void
_setup(void)
{
   int ret = ecordova_init();
   ck_assert_int_eq(ret, 1);
}

static void
_teardown(void)
{
   int ret = ecordova_shutdown();
   ck_assert_int_eq(ret, 0);
}

static Ecordova_File *
_file_new(const char *name,
          const char *url,
          const char *type,
          time_t last_modified_date,
          long size)
{
   return eo_add(ECORDOVA_FILE_CLASS,
                 NULL,
                 ecordova_file_constructor(name,
                                           url,
                                           type,
                                           last_modified_date,
                                           size));
}

START_TEST(smoke)
{
   Ecordova_File *file = _file_new("", "", "", 0, 0);
   eo_unref(file);
}
END_TEST

static void
_check_start_end(Ecordova_File *file, long expected_start, long expected_end)
{
   long actual_start, actual_end;
   eo_do(file,
         actual_start = ecordova_file_start_get(),
         actual_end = ecordova_file_end_get());
   ck_assert_int_eq(expected_start, actual_start);
   ck_assert_int_eq(expected_end, actual_end);
}

START_TEST(slice)
{
   const long start = 0;
   const long size = 100;
   Ecordova_File *file = _file_new("", "", "", 0, size);
   _check_start_end(file, start, size);


   {
      // normal slice
      Ecordova_File *sliced_file = eo_do_ret(file,
                                             sliced_file,
                                             ecordova_file_slice(50, 70));
      _check_start_end(sliced_file, 50, 70);
      eo_unref(sliced_file);
   }

   {
      // cannot past end
      Ecordova_File *sliced_file = eo_do_ret(file,
                                             sliced_file,
                                             ecordova_file_slice(150, 170));
      _check_start_end(sliced_file, size, size);
      eo_unref(sliced_file);
   }

   {
      // negative values slices backwards
      Ecordova_File *sliced_file = eo_do_ret(file,
                                             sliced_file,
                                             ecordova_file_slice(-70, -20));
      _check_start_end(sliced_file, size - 70, size - 20);
      eo_unref(sliced_file);
   }

   {
      // negative values slices backwards (swapped)
      Ecordova_File *sliced_file = eo_do_ret(file,
                                             sliced_file,
                                             ecordova_file_slice(-20, -70));
      _check_start_end(sliced_file, size - 70, size - 20);
      eo_unref(sliced_file);
   }

   eo_unref(file);
}
END_TEST

START_TEST(name_get)
{
   const char *expected_filename = "filename.txt";
   Ecordova_File *file = _file_new(expected_filename, "", "", 0, 0);

   const char *actual_filename = eo_do_ret(file,
                                           actual_filename,
                                           ecordova_file_name_get());
   ck_assert_str_eq(expected_filename, actual_filename);

   eo_unref(file);
}
END_TEST

START_TEST(url_get)
{
   const char *expected_url = "/tmp/filename.txt";
   Ecordova_File *file = _file_new("filename.txt", expected_url, "", 0, 0);

   const char *actual_url = eo_do_ret(file,
                                      actual_url,
                                      ecordova_file_url_get());
   ck_assert_str_eq(expected_url, actual_url);

   eo_unref(file);
}
END_TEST

void
ecordova_file_test(TCase *tc)
{
   tcase_add_checked_fixture(tc, _setup, _teardown);
   tcase_add_test(tc, smoke);
   tcase_add_test(tc, slice);
   tcase_add_test(tc, name_get);
   tcase_add_test(tc, url_get);
}
