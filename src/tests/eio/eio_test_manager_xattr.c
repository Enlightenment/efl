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


#ifndef XATTR_TEST_DIR
#define DISABLE_XATTR_TEST
#define XATTR_TEST_DIR ""
#endif

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

int total_attributes = sizeof(attribute)/sizeof(attribute[0]);

static void
_main_cb(void *data, Eina_Array *array)
{
   const char* attr;
   int *num_of_attr = (int *)data;
   Eina_Array_Iterator iterator;
   unsigned int j;

   EINA_ARRAY_ITER_NEXT(array, j, attr, iterator)
     {
        unsigned int i;

        for (i = 0; i < sizeof (attribute) / sizeof (attribute[0]); ++i)
          {
             if (strcmp(attr, attribute[i]) == 0)
               {
                  (*num_of_attr)++;
                  break;
               }
          }
     }
}

static Eina_Value
_future_done_cb(void *data EINA_UNUSED,
                const Eina_Value array,
                const Eina_Future *dead EINA_UNUSED)
{
   Eina_Error err;
   unsigned int i, len;
   Eina_Value v = EINA_VALUE_EMPTY;
   Eina_Binbuf *buf;

   if (array.type == EINA_VALUE_TYPE_ERROR)
     {
        eina_value_get(&array, &err);
        fprintf(stderr, "Something has gone wrong: %s\n", eina_error_msg_get(err));
        abort();
     }
   EINA_VALUE_ARRAY_FOREACH(&array, len, i, v)
     {
        buf = eina_value_to_binbuf(&v);
        fail_if(!buf);
        fail_if(strcmp((const char*) eina_binbuf_string_get(buf),
                       attr_data[i]) != 0);
     }

   fail_if((int) i != total_attributes);

   ecore_main_loop_quit();
   return array;
}

static Eina_Value
_future_all_cb(void *data,
               const Eina_Value array,
               const Eina_Future *dead EINA_UNUSED)
{
   Eina_Error err;
   unsigned int i, len;
   Eina_Value v = EINA_VALUE_EMPTY;
   int *num_of_attr = (int *)data;

   if (array.type == EINA_VALUE_TYPE_ERROR)
     {
        eina_value_get(&array, &err);
        fprintf(stderr, "Something has gone wrong: %s\n", eina_error_msg_get(err));
        abort();
     }
   EINA_VALUE_ARRAY_FOREACH(&array, len, i, v)
     {
        if (v.type == EINA_VALUE_TYPE_ERROR)
          {
             eina_value_get(&v, &err);
             fprintf(stderr, "Something has gone wrong: %s\n", eina_error_msg_get(err));
             abort();
          }
     }

   fail_if(*num_of_attr != (int) len);
   return array;
}

EFL_START_TEST(eio_test_job_xattr_set)
{
   char *filename = "eio-tmpfile";
   Eina_Tmpstr *test_file_path;
   int num_of_attr = 0, fd;
   unsigned int i;
   Eo *job;
   Eina_Future **futures = NULL;

   job = efl_add(EFL_IO_MANAGER_CLASS, efl_main_loop_get());

   test_file_path = get_full_path(XATTR_TEST_DIR, filename);
   fd = open(test_file_path,
             O_WRONLY | O_CREAT | O_TRUNC,
             S_IRWXU | S_IRWXG | S_IRWXO);
   fail_if(fd == 0);

   futures = calloc(total_attributes + 1, sizeof(Eina_Future*));
   futures[total_attributes] = EINA_FUTURE_SENTINEL;

   for (i = 0; i < sizeof(attribute) / sizeof(attribute[0]); ++i)
     {
        Eina_Binbuf *buf;

        buf = eina_binbuf_manage_new((const unsigned char*) attr_data[i], strlen(attr_data[i]), EINA_TRUE);
        futures[i] = efl_io_manager_xattr_set(job, test_file_path, attribute[i], buf, EINA_XATTR_INSERT);
        eina_binbuf_free(buf);

        fail_if(num_of_attr != 0); // test asynchronous
     }
   eina_future_then(eina_future_all_array(futures),
                    _future_all_cb, &num_of_attr);

   ecore_main_loop_begin();

   free(futures);

   num_of_attr = 0;

   futures = calloc(total_attributes + 1, sizeof(Eina_Future*));
   futures[total_attributes] = NULL;

   for (i = 0; i < sizeof(attribute) / sizeof(attribute[0]); ++i)
     {
        futures[i] = efl_io_manager_xattr_get(job, test_file_path, attribute[i]);
     }

   eina_future_then(eina_future_all_array(futures),
                   _future_done_cb, &num_of_attr);

   ecore_main_loop_begin();

   num_of_attr = 0;

   eina_future_then(efl_io_manager_xattr_ls(job, test_file_path, &num_of_attr, _main_cb, NULL),
                    _future_done_cb, &num_of_attr);

   fail_if(num_of_attr != 0);

   ecore_main_loop_begin();

   free(futures);

   efl_unref(job);
   close(fd);
   unlink(test_file_path);
   eina_tmpstr_del(test_file_path);
}
EFL_END_TEST

void eio_test_job_xattr(TCase *tc)
{
#ifndef DISABLE_XATTR_TEST
   tcase_add_test(tc, eio_test_job_xattr_set);
#else
   (void)eio_test_job_xattr_set;
   (void)tc;
#endif
}
