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
   EINA_LOG_ERR("error: %s", msg);

   ecore_main_loop_quit();
}

void done_closing_cb(void* data EINA_UNUSED, const Efl_Event *ev EINA_UNUSED)
{
   printf("%s closed file.\n", __FUNCTION__);

   ecore_main_loop_quit();
}

void done_open_cb(void *data, const Efl_Event *ev)
{
   Efl_Future_Event_Success *success = ev->info;
   Eina_File *file = success->value;
   Efl_Io_Manager *job = data;

   printf("%s opened file %s\n", __FUNCTION__, eina_file_filename_get(file));

   efl_future_then(efl_io_manager_close(job, file), &done_closing_cb, &error_cb, NULL, NULL);
}

void open_file(const char *path)
{
   Efl_Io_Manager *job;

   job = efl_add(EFL_IO_MANAGER_CLASS, ecore_main_loop_get());

   efl_future_then(efl_io_manager_open(job, path, EINA_FALSE), &done_open_cb, &error_cb, NULL, job);
}

int main(int argc, char const *argv[])
{
   const char *path;

   eio_init();
   ecore_init();

   path = getenv("HOME");

   if (argc > 1)
     path = argv[1];

   open_file(path);

   ecore_main_loop_begin();

   ecore_shutdown();
   eio_shutdown();
   return 0;
}
