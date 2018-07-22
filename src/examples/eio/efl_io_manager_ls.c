#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>

#include <Eina.h>
#include <Eio.h>
#include <Ecore.h>

static Eina_Value
done_cb(void *data EINA_UNUSED,
        const Eina_Value file,
        const Eina_Future *dead EINA_UNUSED)
{
   if (file.type == EINA_VALUE_TYPE_ERROR)
     {
        Eina_Error err;

        eina_value_get(&file, &err);
        fprintf(stderr, "Something has gone wrong: %s\n", eina_error_msg_get(err));
        abort();
     }
   if (file.type == EINA_VALUE_TYPE_UINT64)
     {
        uint64_t value;

        eina_value_get(&file, &value);

        printf("%s done listing files %"PRIu64".\n", __FUNCTION__, value);
     }

   ecore_main_loop_quit();

   return file;
}

// Progress used to be the "Eio_Main_Cb" family of callbacks in the legacy API.
static void
progress_cb(void *data EINA_UNUSED, Eina_Array *array)
{
   Eina_Array_Iterator it;
   const char *filename;
   unsigned int count;

   EINA_ARRAY_ITER_NEXT(array, count, filename, it)
     printf("%s listing filename: %s\n", __FUNCTION__, filename);
}

void list_files(void *data)
{
   Efl_Io_Manager *job = efl_add(EFL_IO_MANAGER_CLASS, efl_main_loop_get());
   const char *path = data;

   eina_future_then(efl_io_manager_ls(job, path, NULL, progress_cb, NULL), done_cb, NULL);
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
