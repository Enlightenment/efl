#if HAVE_CONFIG_H
#include <config.h>
#endif


#include <stdlib.h>
#include <stdio.h>

#include <Eina.h>
#include <Eio.h>
#include <Ecore.h>

void leave(Efl_Io_Manager *job)
{
   ecore_main_loop_quit();
}

void done_cb(void *data, const Efl_Event *ev)
{
   Efl_Future_Event_Success *success = ev->info;
   uint64_t *count = success->value;

   printf("%s done listing files %i.\n", __FUNCTION__, *count);

   leave(data);
}

void error_cb(void *data, const Efl_Event *ev)
{
   Efl_Future_Event_Failure *failure = ev->info;
   const char *msg = eina_error_msg_get(failure->error);

   printf("%s error: %s\n", __FUNCTION__, msg);

   leave(data);
}

// Progress used to be the "Eio_Main_Cb" family of callbacks in the legacy API.
void progress_cb(void *data EINA_UNUSED, const Efl_Event *ev)
{
   Efl_Future_Event_Progress *p = ev->info;
   const Eina_Array *batch = p->progress;
   Eina_Iterator *it;
   const char *filename;

   it = eina_array_iterator_new(batch);
   EINA_ITERATOR_FOREACH(it, filename)
     printf("%s listing filename: %s\n", __FUNCTION__, filename);
   eina_iterator_free(it);
}

void list_files(void *data)
{
   Efl_Io_Manager *job = efl_add(EFL_IO_MANAGER_CLASS, ecore_main_loop_get());
   const char *path = data;

   efl_future_then(efl_io_manager_ls(job, path), &done_cb, &error_cb, &progress_cb, job);
}

int main(int argc, char const *argv[])
{
   const char *path;
   Ecore_Job *job;

   eio_init();
   ecore_init();

   path = getenv("HOME");

   if (argc > 1)
     path = argv[1];

   job = ecore_job_add(&list_files, path);

   ecore_main_loop_begin();

   ecore_shutdown();
   eio_shutdown();
   return 0;
}
