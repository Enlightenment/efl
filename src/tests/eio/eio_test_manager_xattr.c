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
_main_cb(void *data, const Efl_Event *ev)
{
   Efl_Future_Event_Progress *progress = ev->info;
   const Eina_Array *attrs = progress->progress;
   const char* attr;
   int *num_of_attr = (int *)data;
   unsigned int i, j;
   Eina_Array_Iterator it;

   EINA_ARRAY_ITER_NEXT(attrs, j, attr, it)
     {
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

static void
_done_cb(void *data, const Efl_Event *ev EINA_UNUSED)

{
   int *num_of_attr = (int *)data;

   fail_if(*num_of_attr != total_attributes);

   ecore_main_loop_quit();
}

static void
_done_get_cb(void *data EINA_UNUSED, const Efl_Event *ev)
{
   Efl_Future_Event_Success *success = ev->info;
   Eina_Accessor *ac = success->value;
   int i = 0;
   Eina_Binbuf *buf;

   EINA_ACCESSOR_FOREACH(ac, i, buf)
     {
        fail_if(!buf);
        fail_if(strcmp((const char*) eina_binbuf_string_get(buf),
                       attr_data[i]) != 0);
     }

   fail_if(i != total_attributes);

   ecore_main_loop_quit();
}

static void
_done_set_cb(void *data, const Efl_Event *ev)
{
   Efl_Future_Event_Success *success = ev->info;
   Eina_Accessor *ac = success->value;
   int *placeholder;
   int *num_of_attr = data;
   int i = 0;

   EINA_ACCESSOR_FOREACH(ac, i, placeholder)
     *num_of_attr += 1;

   fail_if(*num_of_attr != total_attributes);

   ecore_main_loop_quit();
}

static void
_error_cb(void *data EINA_UNUSED, const Efl_Event *ev)
{
   Efl_Future_Event_Failure *failure = ev->info;

   fprintf(stderr, "Something has gone wrong:%s\n", eina_error_msg_get(failure->error));
   abort();

   ecore_main_loop_quit();
}

START_TEST(eio_test_job_xattr_set)
{
   char *filename = "eio-tmpfile";
   Eina_Tmpstr *test_file_path;
   int num_of_attr = 0, fd;
   unsigned int i;
   Eo *job;
   Efl_Future *ls = NULL;
   Efl_Future **futures = NULL;

   ecore_init();
   eina_init();
   eio_init();

   job = efl_add(EFL_IO_MANAGER_CLASS, ecore_main_loop_get());

   test_file_path = get_full_path(XATTR_TEST_DIR, filename);
   fd = open(test_file_path,
             O_WRONLY | O_CREAT | O_TRUNC,
             S_IRWXU | S_IRWXG | S_IRWXO);
   fail_if(fd == 0);

   futures = calloc(total_attributes + 1, sizeof(Efl_Future*));
   futures[total_attributes] = NULL;

   for (i = 0; i < sizeof(attribute) / sizeof(attribute[0]); ++i)
     {
        Eina_Binbuf *buf;

        buf = eina_binbuf_manage_new((const unsigned char*) attr_data[i], strlen(attr_data[i]), EINA_TRUE);
        futures[i] = efl_io_manager_xattr_set(job, test_file_path, attribute[i], buf, EINA_XATTR_INSERT);
        eina_binbuf_free(buf);

        fail_if(num_of_attr != 0); // test asynchronous
     }
   efl_future_then(efl_future_iterator_all(eina_carray_iterator_new((void**) futures)),
                   _done_set_cb, _error_cb, NULL, &num_of_attr);

   ecore_main_loop_begin();

   free(futures);

   num_of_attr = 0;

   futures = calloc(total_attributes + 1, sizeof(Efl_Future*));
   futures[total_attributes] = NULL;

   for (i = 0; i < sizeof(attribute) / sizeof(attribute[0]); ++i)
     {
        futures[i] = efl_io_manager_xattr_get(job, test_file_path, attribute[i]);
     }

   efl_future_then(efl_future_iterator_all(eina_carray_iterator_new((void**)futures)),
                   _done_get_cb, _error_cb, NULL, &num_of_attr);

   ecore_main_loop_begin();

   num_of_attr = 0;

   efl_future_use(&ls, efl_io_manager_xattr_ls(job, test_file_path));
   efl_future_then(ls, _done_cb, _error_cb, _main_cb, &num_of_attr);

   fail_if(num_of_attr != 0);

   ecore_main_loop_begin();

   free(futures);

   efl_unref(job);
   close(fd);
   unlink(test_file_path);
   eina_tmpstr_del(test_file_path);
   eio_shutdown();
   eina_shutdown();
   ecore_shutdown();
}
END_TEST

void eio_test_job_xattr(TCase *tc)
{
#ifndef DISABLE_XATTR_TEST
   tcase_add_test(tc, eio_test_job_xattr_set);
#else
   (void)eio_test_job_xattr_set;
   (void)tc;
#endif
}
