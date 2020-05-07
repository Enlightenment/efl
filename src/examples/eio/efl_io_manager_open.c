#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>

#include <Eina.h>
#include <Eio.h>
#include <Ecore.h>

static Eina_Value
_closing_cb(void* data EINA_UNUSED,
            const Eina_Value file,
            const Eina_Future *dead EINA_UNUSED)
{
   if (file.type == EINA_VALUE_TYPE_ERROR)
     {
        Eina_Error err;

        eina_value_get(&file, &err);
        fprintf(stderr, "Something has gone wrong: %s\n", eina_error_msg_get(err));
     }
   else
     {
        printf("%s closed file.\n", __func__);
     }

   ecore_main_loop_quit();

   return file;
}

static Eina_Value
_open_cb(void *data,
         const Eina_Value file,
         const Eina_Future *dead EINA_UNUSED)
{
   Efl_Io_Manager *job = data;

   if (file.type == EINA_VALUE_TYPE_ERROR)
     {
        Eina_Error err;

        eina_value_get(&file, &err);
        fprintf(stderr, "Something has gone wrong: %s\n", eina_error_msg_get(err));
        ecore_main_loop_quit();

        return file;
     }
   if (file.type == EINA_VALUE_TYPE_FILE)
     {
        Eina_File *f;

        eina_value_get(&file, &f);

        printf("%s opened file %s\n", __func__, eina_file_filename_get(f));

        return eina_future_as_value(efl_io_manager_close(job, f));
     }
   return file;
}

void open_file(const char *path)
{
   Efl_Io_Manager *job;

   job = efl_add(EFL_IO_MANAGER_CLASS, efl_main_loop_get());

   eina_future_chain(efl_io_manager_open(job, path, EINA_FALSE),
                     { .cb = _open_cb, .data = job },
                     { .cb = _closing_cb, .data = NULL });
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
