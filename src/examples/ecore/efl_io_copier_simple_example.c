#include <Efl_Core.h>

static void
_copier_done(void *data EINA_UNUSED, const Efl_Event *event EINA_UNUSED)
{
   fprintf(stderr, "INFO: done\n");

   efl_loop_quit(efl_loop_get(event->object), EINA_VALUE_EMPTY);
}

static void
_copier_error(void *data EINA_UNUSED, const Efl_Event *event)
{
   const Eina_Error *perr = event->info;
   fprintf(stderr, "INFO: error: #%d '%s'\n", *perr, eina_error_msg_get(*perr));

   efl_loop_quit(efl_loop_get(event->object), eina_value_int_init(EXIT_FAILURE));
}

EFL_CALLBACKS_ARRAY_DEFINE(copier_cbs,
                           { EFL_IO_COPIER_EVENT_DONE, _copier_done },
                           { EFL_IO_COPIER_EVENT_ERROR, _copier_error });

static Eo *copier = NULL;

EAPI_MAIN void
efl_pause(void *data EINA_UNUSED,
          const Efl_Event *ev EINA_UNUSED)
{
}

EAPI_MAIN void
efl_resume(void *data EINA_UNUSED,
           const Efl_Event *ev EINA_UNUSED)
{
}

EAPI_MAIN void
efl_terminate(void *data EINA_UNUSED,
              const Efl_Event *ev EINA_UNUSED)
{
   /* FIXME: For the moment the main loop doesn't get
      properly destroyed on shutdown which disallow
      relying on parent destroying their children */
   if (copier)
     {
        efl_io_closer_close(copier);
        efl_del(copier);
     }
   copier = NULL;
}

EAPI_MAIN void
efl_main(void *data EINA_UNUSED,
         const Efl_Event *ev)
{
   Eo *input, *output, *loop;

   loop = ev->object;
   input = efl_add(EFL_IO_STDIN_CLASS, loop);
   output = efl_add(EFL_IO_STDOUT_CLASS, loop);
   copier = efl_add(EFL_IO_COPIER_CLASS, loop,
                    efl_io_copier_source_set(efl_added, input), /* mandatory */
                    efl_io_copier_destination_set(efl_added, output), /* optional */
                    efl_event_callback_array_add(efl_added, copier_cbs(), NULL) /* recommended, at least EFL_IO_COPIER_EVENT_DONE. */
                    );
}

EFL_MAIN_EX();
