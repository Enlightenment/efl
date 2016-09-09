#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>

#include <Eina.h>
#include <Eio.h>
#include <Ecore.h>

void error_cb(void *data, const Efl_Event *ev)
{
   Efl_Future_Event_Failure *failure = ev->info;
   const char *msg = eina_error_msg_get(failure->error);
   Efl_Io_Manager *job = data;

   EINA_LOG_ERR("error: %s", msg);

   ecore_main_loop_quit();
}

void done_closing_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Efl_Io_Manager *job = data;

   printf("%s closed all files.\n", __FUNCTION__);

   ecore_main_loop_quit();
}

void done_open_cb(void *data, const Efl_Event *ev)
{
   Efl_Future_Event_Success *s = ev->info;
   Efl_Io_Manager *job = data;
   Eina_Accessor *ac = s->value;
   Eina_Iterator *it;
   Eina_Array stack;
   Eina_File *f;
   unsigned int i;

   eina_array_step_set(&stack, sizeof (Eina_Array), 4);

   EINA_ACCESSOR_FOREACH(ac, i, f)
     {
        printf("%s opened file %s [%i]\n", __FUNCTION__, eina_file_filename_get(f), i);
        eina_array_push(&stack, efl_io_manager_close(job, f));
     }

   it = eina_array_iterator_new(&stack);
   efl_future_then(efl_future_iterator_all(it), &done_closing_cb, &error_cb, NULL, job);

   eina_array_flush(&stack);
}

Efl_Future *open_file(Efl_Io_Manager *job, const char *path)
{
   return efl_io_manager_open(job, path, EINA_FALSE);
}

int main(int argc, char const *argv[])
{
   Efl_Io_Manager *job;
   const char *path;
   const char *path2;

   eio_init();
   ecore_init();

   job = efl_add(EFL_IO_MANAGER_CLASS, ecore_main_loop_get());

   path = getenv("HOME");
   path2 = "./";

   if (argc > 1)
     path = argv[1];
   if (argc > 2)
     path2 = argv[2];

   efl_future_then(efl_future_all(open_file(job, path), open_file(job, path2)),
                   &done_open_cb, &error_cb, NULL, job);

   ecore_main_loop_begin();

   efl_del(job);

   ecore_shutdown();
   eio_shutdown();
   return 0;
}
