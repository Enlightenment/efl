#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>

#include <Eina.h>
#include <Eio.h>
#include <Ecore.h>

Eina_Value
_close_cb(void *data EINA_UNUSED, const Eina_Value array, const Eina_Future *dead EINA_UNUSED)
{
   printf("%s closed all files.\n", __FUNCTION__);

   ecore_main_loop_quit();

   return array;
}

static Eina_Value
_open_cb(void *data, const Eina_Value array, const Eina_Future *dead EINA_UNUSED)
{
   Efl_Io_Manager *job = data;
   unsigned int i, len;
   Eina_Value v = EINA_VALUE_EMPTY;
   Eina_Future *futures[eina_value_array_count(&array) + 1];

   EINA_VALUE_ARRAY_FOREACH(&array, len, i, v)
     {
        if (v.type == EINA_VALUE_TYPE_ERROR)
          {
             Eina_Error err;

             eina_value_get(&v, &err);
             fprintf(stderr, "Something has gone wrong: %s at index: %i\n", eina_error_msg_get(err), i);
             goto on_error;
          }
        else if (v.type == EINA_VALUE_TYPE_FILE)
          {
             Eina_File *f;

             eina_value_get(&v, &f);

             futures[i] = efl_io_manager_close(job, f);
          }
        else
          {
             goto on_error;
          }
     }

   futures[i] = EINA_FUTURE_SENTINEL;

   return eina_future_as_value(eina_future_all_array(futures));

 on_error:
   for (; i > 0; i--)
     eina_future_cancel(futures[i - 1]);
   return v;
}

int main(int argc, char const *argv[])
{
   Efl_Io_Manager *job;
   Eina_Future *futures[3] = { NULL, NULL, EINA_FUTURE_SENTINEL };
   const char *path;
   const char *path2;

   eio_init();
   ecore_init();

   job = efl_add(EFL_IO_MANAGER_CLASS, efl_main_loop_get());

   path = getenv("HOME");
   path2 = "./";

   if (argc > 1)
     path = argv[1];
   if (argc > 2)
     path2 = argv[2];

   futures[0] = efl_io_manager_open(job, path, EINA_FALSE);
   futures[1] = efl_io_manager_open(job, path2, EINA_FALSE);

   eina_future_chain(eina_future_all_array(futures),
                     { .cb = _open_cb, .data = job },
                     { .cb = _close_cb, .data = NULL });

   ecore_main_loop_begin();

   efl_del(job);

   ecore_shutdown();
   eio_shutdown();
   return 0;
}
