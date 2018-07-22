#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#include <Ecore.h>
#include <Ecore_File.h>
#include <Eio.h>

#include "eio_suite.h"
#include "eio_test_common.h"


#ifdef XATTR_TEST_DIR

static char *str_attr = "user.name";
static char *str_data = "Vivek Ellur";
static char *int_attr = "user.id";
static int  int_data = 1234;
static char *double_attr = "user.size";
static double double_data = 123.456;

static const char *attribute[] =
  {
     "user.comment1",
     "user.comment2",
     "user.comment3"
  };
static const char *attr_data[] =
  {
     "This is a test file",
     "This line is a comment",
     "This file has extra attributes"
  };

static Eina_Bool
_filter_cb(void *data EINA_UNUSED, Eio_File *handler EINA_UNUSED, const char *attr EINA_UNUSED)
{
   return EINA_TRUE;
}

static void
_main_cb(void *data EINA_UNUSED, Eio_File *handler EINA_UNUSED, const char *attr)
{
   int *num_of_attr = (int *)data;
   unsigned int i;

   for (i = 0; i < sizeof (attribute) / sizeof (attribute[0]); ++i)
     if (strcmp(attr, attribute[i]) == 0)
       {
          (*num_of_attr)++;
          break;
       }
}

static void
_done_cb(void *data, Eio_File *handler EINA_UNUSED)

{
   int *num_of_attr = (int *)data;

   (*num_of_attr)++;

   if (((*num_of_attr) + 1) == (sizeof (attribute) / sizeof (attribute[0])))
     ecore_main_loop_quit();
}

static void
_done_once_cb(void *data EINA_UNUSED, Eio_File *handler EINA_UNUSED)
{
   ecore_main_loop_quit();
}

static void
_done_file_cb(void *data EINA_UNUSED, Eio_File *handler EINA_UNUSED)
{
   int *num_of_attr = (int *)data;

   fail_if( (*num_of_attr) != (sizeof (attribute) / sizeof (attribute[0])));
   ecore_main_loop_quit();
}

static void
_done_get_cb(void *data, Eio_File *handler EINA_UNUSED, const char *name, unsigned int len EINA_UNUSED)

{
   int i = (int)(uintptr_t)data;

   fail_if(strcmp(name, attr_data[i]) != 0);

   if ((i + 1) == (sizeof (attribute) / sizeof (attribute[0])))
     ecore_main_loop_quit();
}

static void
_done_string_cb(void *data EINA_UNUSED, Eio_File *handler EINA_UNUSED, const char *xattr_string)
{
   fail_if(strcmp(xattr_string, str_data) != 0);

   ecore_main_loop_quit();
}

static void
_done_int_cb(void *data EINA_UNUSED, Eio_File *handler EINA_UNUSED, int xattr_int)
{
   fail_if(xattr_int != int_data);

   ecore_main_loop_quit();
}

static void
_done_double_cb(void *data EINA_UNUSED, Eio_File *handler EINA_UNUSED, double xattr_double)
{
   fail_if(!EINA_DBL_EQ(xattr_double, double_data));

   ecore_main_loop_quit();
}

static void
_error_cb(void *data EINA_UNUSED, Eio_File *handler EINA_UNUSED, int error)

{
   fprintf(stderr, "Something has gone wrong:%s\n", strerror(error));
   abort();

   ecore_main_loop_quit();
}

EFL_START_TEST(eio_test_xattr_set)
{
   char *filename = "eio-tmpfile";
   Eina_Tmpstr *test_file_path;
   Eio_File *fp;
   int num_of_attr = 0, fd;
   unsigned int i;

   test_file_path = get_full_path(XATTR_TEST_DIR, filename);
   fd = open(test_file_path,
             O_WRONLY | O_CREAT | O_TRUNC,
             S_IRWXU | S_IRWXG | S_IRWXO);
   fail_if(fd == 0);

   for (i = 0; i < sizeof(attribute) / sizeof(attribute[0]); ++i)
     {
        fp = eio_file_xattr_set(test_file_path, attribute[i],
                                attr_data[i], strlen(attr_data[i]),
                                EINA_XATTR_INSERT,
                                _done_cb, _error_cb, &num_of_attr);

        fail_if(num_of_attr != 0); // test asynchronous
        fail_if(!fp);
     }

   ecore_main_loop_begin();

   for (i = 0; i < sizeof(attribute) / sizeof(attribute[0]); ++i)
     {
        fp = eio_file_xattr_get(test_file_path, attribute[i],
                                _done_get_cb, _error_cb, (void*)(uintptr_t) i);
        fail_if(!fp);
     }

   ecore_main_loop_begin();

   num_of_attr = 0;
   fp = eio_file_xattr(test_file_path,
                       _filter_cb, _main_cb, _done_file_cb, _error_cb,
                       &num_of_attr);
   fail_if(num_of_attr != 0);
   fail_if(!fp);

   ecore_main_loop_begin();

   close(fd);
   unlink(test_file_path);
   eina_tmpstr_del(test_file_path);
}
EFL_END_TEST

EFL_START_TEST(eio_test_xattr_types_set)
{
   char *filename = "eio-tmpfile";

   Eina_Tmpstr *test_file_path;
   int  fd, num_of_attr=0;
   Eio_File *fp;

   test_file_path = get_full_path(XATTR_TEST_DIR, filename);
   fd = open(test_file_path,
             O_WRONLY | O_CREAT | O_TRUNC,
             S_IRWXU | S_IRWXG | S_IRWXO);
   fail_if(fd == 0);
   fp = eio_file_xattr_string_set(test_file_path, str_attr,
                                str_data, EINA_XATTR_INSERT,
                                _done_once_cb, _error_cb, &num_of_attr);

   fail_if(num_of_attr != 0); // test asynchronous
   fail_if(!fp);

   ecore_main_loop_begin();

   fp = eio_file_xattr_string_get(test_file_path, str_attr,
                                  _done_string_cb, _error_cb, NULL);
   fail_if(!fp);

   ecore_main_loop_begin();

   fp = eio_file_xattr_int_set(test_file_path, int_attr,
                                int_data, EINA_XATTR_INSERT,
                                _done_once_cb, _error_cb, &num_of_attr);

   fail_if(num_of_attr != 0); // test asynchronous
   fail_if(!fp);

   ecore_main_loop_begin();

   fp = eio_file_xattr_int_get(test_file_path, int_attr,
                                  _done_int_cb, _error_cb, NULL);
   fail_if(!fp);

   ecore_main_loop_begin();

   fp = eio_file_xattr_double_set(test_file_path, double_attr,
                                double_data, EINA_XATTR_INSERT,
                                _done_once_cb, _error_cb, &num_of_attr);

   fail_if(num_of_attr != 0); // test asynchronous
   fail_if(!fp);

   ecore_main_loop_begin();

   fp = eio_file_xattr_double_get(test_file_path, double_attr,
                                  _done_double_cb, _error_cb, NULL);
   fail_if(!fp);

   ecore_main_loop_begin();

   close(fd);
   unlink(test_file_path);
   eina_tmpstr_del(test_file_path);
}
EFL_END_TEST
#endif

void eio_test_xattr(TCase *tc)
{
#ifdef XATTR_TEST_DIR
   tcase_add_test(tc, eio_test_xattr_set);
   tcase_add_test(tc, eio_test_xattr_types_set);
#else
   (void)tc;
#endif
}
