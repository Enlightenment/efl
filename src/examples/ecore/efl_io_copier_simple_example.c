#define EFL_BETA_API_SUPPORT 1
#define EFL_EO_API_SUPPORT 1
#include <Ecore.h>

static int retval = EXIT_SUCCESS;

static void
_copier_done(void *data, const Eo_Event *event)
{
   fprintf(stderr, "INFO: done\n");
   ecore_main_loop_quit();
}

static void
_copier_error(void *data, const Eo_Event *event)
{
   const Eina_Error *perr = event->info;
   fprintf(stderr, "INFO: error: #%d '%s'\n", *perr, eina_error_msg_get(*perr));
   ecore_main_loop_quit();
   retval = EXIT_FAILURE;
}

EFL_CALLBACKS_ARRAY_DEFINE(copier_cbs,
                           { EFL_IO_COPIER_EVENT_DONE, _copier_done },
                           { EFL_IO_COPIER_EVENT_ERROR, _copier_error });

int main(int argc, char *argv[])
{
   Eo *input, *output, *copier, *loop;

   ecore_init();

   loop = ecore_main_loop_get();
   input = efl_add(EFL_IO_STDIN_CLASS, loop);
   output = efl_add(EFL_IO_STDOUT_CLASS, loop);
   copier = efl_add(EFL_IO_COPIER_CLASS, loop,
                    efl_io_copier_source_set(efl_self, input), /* mandatory */
                    efl_io_copier_destination_set(efl_self, output), /* optional */
                    efl_event_callback_array_add(efl_self, copier_cbs(), NULL) /* recommended, at least EFL_IO_COPIER_EVENT_DONE. */
                    );

   ecore_main_loop_begin();

   efl_io_closer_close(copier);
   efl_del(copier);
   efl_del(output);
   efl_del(input);

   ecore_shutdown();
   return retval;
}
