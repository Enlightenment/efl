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
_filter_cb(void *data EINA_UNUSED, const Eo_Event *event)
{
   Eio_Filter_Name_Data *event_info = event->info;

   event_info->filter = EINA_TRUE;
}

static void
_main_cb(void *data, void *v)
{
   const char* attr = v;
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
_done_cb(void *data, void *value EINA_UNUSED)

{
   int *num_of_attr = (int *)data;

   fail_if(*num_of_attr != total_attributes);

   ecore_main_loop_quit();
}

static void
_done_get_cb(void *data EINA_UNUSED, void* v)
{
   Eina_Iterator** it = (Eina_Iterator**)v;
   int i = 0;
   Eio_Xattr_Data *get_data;

   while (eina_iterator_next((*it), (void**)&get_data))
     {
        fail_if(!get_data);
        fail_if(strcmp(get_data->data, attr_data[i]) != 0);
        i++;
     }

   fail_if(i != total_attributes);

   ecore_main_loop_quit();
}

static void
_done_set_cb(void *data, void* v)
{
   Eina_Iterator** it = (Eina_Iterator**)v;
   int *placeholder;
   int *num_of_attr = data;
   while(eina_iterator_next((*it), (void**)&placeholder))
     *num_of_attr += 1;

   fail_if(*num_of_attr != total_attributes);

   ecore_main_loop_quit();
}

static void
_error_cb(void *data EINA_UNUSED, Eina_Error error)

{
   fprintf(stderr, "Something has gone wrong:%s\n", eina_error_msg_get(error));
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
   Eina_Promise *list_promise = NULL;
   Eina_Promise **attrib_promises = NULL;

   ecore_init();
   eina_init();
   eio_init();

   fprintf(stderr, "eio_test_job_xattr_set\n"); fflush(stderr);

   job = eo_add(EFL_IO_MANAGER_CLASS, NULL);

   test_file_path = get_full_path(XATTR_TEST_DIR, filename);
   fd = open(test_file_path,
             O_WRONLY | O_CREAT | O_TRUNC,
             S_IRWXU | S_IRWXG | S_IRWXO);
   fail_if(fd == 0);

   attrib_promises = (Eina_Promise**)calloc(total_attributes + 1, sizeof(Eina_Promise*));
   attrib_promises[total_attributes] = NULL;

   for (i = 0; i < sizeof(attribute) / sizeof(attribute[0]); ++i)
     {
        attrib_promises[i] = efl_io_manager_file_xattr_set
          (job, test_file_path, attribute[i],
           attr_data[i], strlen(attr_data[i]),
           EINA_XATTR_INSERT);

        fail_if(num_of_attr != 0); // test asynchronous
     }
   eina_promise_then(eina_promise_all(eina_carray_iterator_new((void**)attrib_promises)),
         &_done_set_cb, _error_cb, &num_of_attr);

   ecore_main_loop_begin();

   free(attrib_promises);

   num_of_attr = 0;

   attrib_promises = (Eina_Promise**)calloc(total_attributes + 1, sizeof(Eina_Promise*));
   attrib_promises[total_attributes] = NULL;

   for (i = 0; i < sizeof(attribute) / sizeof(attribute[0]); ++i)
   {
     attrib_promises[i] = efl_io_manager_file_xattr_get(job, test_file_path, attribute[i]);
   }

   eina_promise_then(eina_promise_all(eina_carray_iterator_new((void**)attrib_promises)),
         _done_get_cb, _error_cb, &num_of_attr);

   ecore_main_loop_begin();

   num_of_attr = 0;

   efl_event_callback_add(job, EFL_IO_MANAGER_EVENT_XATTR, _filter_cb, NULL);
   list_promise = efl_io_manager_file_xattr_list_get(job, test_file_path);
   eina_promise_progress_cb_add(list_promise, _main_cb, &num_of_attr, NULL);
   eina_promise_then(list_promise, _done_cb, _error_cb, &num_of_attr);

   fail_if(num_of_attr != 0);

   ecore_main_loop_begin();

   free(attrib_promises);

   eo_unref(job);
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
