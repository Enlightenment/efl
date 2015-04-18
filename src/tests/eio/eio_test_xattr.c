#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#include <Eio.h>
#include <Ecore.h>
#include <Ecore_File.h>

#include "eio_suite.h"

const char *attribute[] =
  {
     "user.comment1",
     "user.comment2",
     "user.comment3"
  };
const char *attr_data[] =
  {
     "This is a test file",
     "This line is a comment",
     "This file has extra attributes"
  };

Eina_Tmpstr*
get_file_path(const char* tmpdirname, const char* filename)
{
    char file_path[PATH_MAX] = "";
    eina_str_join(file_path, sizeof(file_path), '/', tmpdirname, filename);
    return eina_tmpstr_add(file_path);
}

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
       break;

   fail_if(i == sizeof (attribute) / sizeof (attribute[0]));
   (*num_of_attr)++;
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
_done_get_cb(void *data, Eio_File *handler EINA_UNUSED, const char *name, unsigned int len EINA_UNUSED)

{
   int i = (int)(uintptr_t)data;

   fail_if(strcmp(name, attr_data[i]) != 0);

   if ((i + 1) == (sizeof (attribute) / sizeof (attribute[0])))
     ecore_main_loop_quit();
}

static void
_error_cb(void *data EINA_UNUSED, Eio_File *handler EINA_UNUSED, int error)

{
   fprintf(stderr, "Something wrong has happend:%s\n", strerror(error));
   abort();

   ecore_main_loop_quit();
}

#ifdef XATTR_TEST_DIR
START_TEST(eio_test_xattr_set)
{
   char *filename = "eio-tmpfile";
   Eina_Tmpstr *test_file_path;
   Eio_File *fp;
   int num_of_attr = 0, fd;
   unsigned int i;

   ecore_init();
   eina_init();
   eio_init();

   test_file_path = get_file_path(XATTR_TEST_DIR, filename);
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
                       _filter_cb, _main_cb, _done_once_cb, _error_cb,
                       &num_of_attr);
   fail_if(num_of_attr != 0);
   fail_if(!fp);

   ecore_main_loop_begin();

   fail_if(num_of_attr != sizeof (attribute)  / sizeof (attribute[0]));

   close(fd);
   unlink(test_file_path);
   eio_shutdown();
   ecore_shutdown();
}
END_TEST
#endif

void eio_test_xattr(TCase *tc)
{
#ifdef XATTR_TEST_DIR
   tcase_add_test(tc, eio_test_xattr_set);
#else
   (void)tc
#endif
}
