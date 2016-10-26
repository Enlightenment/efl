#define EFL_BETA_API_SUPPORT 1
#define EFL_EO_API_SUPPORT 1
#include <Ecore.h>
#include <Ecore_Con.h>
#include <Ecore_Getopt.h>
#include <fcntl.h>
#include <ctype.h>

static int retval = EXIT_SUCCESS;
static Eina_Bool do_read = EINA_FALSE;

static void
_connected(void *data EINA_UNUSED, const Efl_Event *event)
{
   fprintf(stderr,
           "INFO: connected to '%s' (%s)\n"
           "INFO:  - local address=%s\n"
           "INFO:  - read-after-write=%u\n",
           efl_net_dialer_address_dial_get(event->object),
           efl_net_socket_address_remote_get(event->object),
           efl_net_socket_address_local_get(event->object),
           do_read);
}

static void
_eos(void *data EINA_UNUSED, const Efl_Event *event EINA_UNUSED)
{
   fprintf(stderr, "INFO: end of stream. \n");
   ecore_main_loop_quit();
}

static void
_can_read(void *data EINA_UNUSED, const Efl_Event *event)
{
   char buf[4];
   Eina_Rw_Slice rw_slice = EINA_SLICE_ARRAY(buf);
   Eina_Error err;
   Eina_Bool can_read = efl_io_reader_can_read_get(event->object);

   /* NOTE: this message may appear with can read=0 BEFORE
    * "read '...'" because efl_io_readr_read() will change the status
    * of can_read to FALSE prior to return so we can print it!
    */
   fprintf(stderr, "INFO: can read=%d\n", can_read);
   if (!can_read) return;
   if (!do_read) return;

   err = efl_io_reader_read(event->object, &rw_slice);
   if (err)
     {
        fprintf(stderr, "ERROR: could not read: %s\n", eina_error_msg_get(err));
        retval = EXIT_FAILURE;
        ecore_main_loop_quit();
        return;
     }

   fprintf(stderr, "INFO: read '" EINA_SLICE_STR_FMT "'\n", EINA_SLICE_STR_PRINT(rw_slice));
}

static void
_can_write(void *data EINA_UNUSED, const Efl_Event *event)
{
   static Eina_Slice slice = EINA_SLICE_STR_LITERAL("Hello World!");
   Eina_Slice to_write;
   Eina_Error err;
   Eina_Bool can_write = efl_io_writer_can_write_get(event->object);

   /* NOTE: this message may appear with can write=0 BEFORE
    * "wrote '...'" because efl_io_writer_write() will change the status
    * of can_write to FALSE prior to return so we can print it!
    */
   fprintf(stderr, "INFO: can write=%d (wanted bytes=%zd)\n", can_write, slice.len);
   if (!can_write) return;
   if (slice.len == 0) return;

   to_write = slice;
   err = efl_io_writer_write(event->object, &to_write, &slice);
   if (err)
     {
        fprintf(stderr, "ERROR: could not write: %s\n", eina_error_msg_get(err));
        retval = EXIT_FAILURE;
        ecore_main_loop_quit();
        return;
     }

   fprintf(stderr, "INFO: wrote '" EINA_SLICE_STR_FMT "', still pending=%zd bytes\n", EINA_SLICE_STR_PRINT(to_write), slice.len);

   if ((!do_read) && (slice.len == 0))
     {
        retval = EXIT_SUCCESS;
        ecore_main_loop_quit();
        return;
     }
}

static void
_resolved(void *data EINA_UNUSED, const Efl_Event *event)
{
   fprintf(stderr, "INFO: resolved %s => %s\n",
           efl_net_dialer_address_dial_get(event->object),
           efl_net_socket_address_remote_get(event->object));
}

static void
_error(void *data EINA_UNUSED, const Efl_Event *event)
{
   const Eina_Error *perr = event->info;
   fprintf(stderr, "INFO: error: %d '%s'\n", *perr, eina_error_msg_get(*perr));
   retval = EXIT_FAILURE;
}

EFL_CALLBACKS_ARRAY_DEFINE(dialer_cbs,
                           { EFL_NET_DIALER_EVENT_CONNECTED, _connected },
                           { EFL_NET_DIALER_EVENT_RESOLVED, _resolved },
                           { EFL_NET_DIALER_EVENT_ERROR, _error },
                           { EFL_IO_READER_EVENT_EOS, _eos },
                           { EFL_IO_READER_EVENT_CAN_READ_CHANGED, _can_read },
                           { EFL_IO_WRITER_EVENT_CAN_WRITE_CHANGED, _can_write }
                           );

static const Ecore_Getopt options = {
  "efl_net_dialer_unix_example", /* program name */
  NULL, /* usage line */
  "1", /* version */
  "(C) 2016 Enlightenment Project", /* copyright */
  "BSD 2-Clause", /* license */
  /* long description, may be multiline and contain \n */
  "Example of Efl_Net_Dialer_Unix usage, sending a message and receiving a reply\n",
  EINA_FALSE,
  {
    ECORE_GETOPT_STORE_TRUE('r', "read-after-write", "Do a read after writes are done."),
    ECORE_GETOPT_VERSION('V', "version"),
    ECORE_GETOPT_COPYRIGHT('C', "copyright"),
    ECORE_GETOPT_LICENSE('L', "license"),
    ECORE_GETOPT_HELP('h', "help"),
    ECORE_GETOPT_STORE_METAVAR_STR(0, NULL,
                                   "The address (URL) to dial", "address"),
    ECORE_GETOPT_SENTINEL
  }
};

int
main(int argc, char **argv)
{
   char *address = NULL;
   Eina_Bool quit_option = EINA_FALSE;
   Ecore_Getopt_Value values[] = {
     ECORE_GETOPT_VALUE_BOOL(do_read),

     /* standard block to provide version, copyright, license and help */
     ECORE_GETOPT_VALUE_BOOL(quit_option), /* -V/--version quits */
     ECORE_GETOPT_VALUE_BOOL(quit_option), /* -C/--copyright quits */
     ECORE_GETOPT_VALUE_BOOL(quit_option), /* -L/--license quits */
     ECORE_GETOPT_VALUE_BOOL(quit_option), /* -h/--help quits */

     /* positional argument */
     ECORE_GETOPT_VALUE_STR(address),

     ECORE_GETOPT_VALUE_NONE /* sentinel */
   };
   int args;
   Eo *dialer, *loop;
   Eina_Error err;

   ecore_init();
   ecore_con_init();

   args = ecore_getopt_parse(&options, values, argc, argv);
   if (args < 0)
     {
        fputs("ERROR: Could not parse command line options.\n", stderr);
        retval = EXIT_FAILURE;
        goto end;
     }

   if (quit_option) goto end;

   loop = ecore_main_loop_get();

   args = ecore_getopt_parse_positional(&options, values, argc, argv, args);
   if (args < 0)
     {
        fputs("ERROR: Could not parse positional arguments.\n", stderr);
        retval = EXIT_FAILURE;
        goto end;
     }

   dialer = efl_add(EFL_NET_DIALER_UNIX_CLASS, loop,
                    efl_name_set(efl_added, "dialer"),
                    efl_event_callback_array_add(efl_added, dialer_cbs(), NULL));

   err = efl_net_dialer_dial(dialer, address);
   if (err != 0)
     {
        fprintf(stderr, "ERROR: could not dial '%s': %s",
                address, eina_error_msg_get(err));
        goto no_mainloop;
     }

   ecore_main_loop_begin();

   fprintf(stderr, "INFO: main loop finished.\n");

 no_mainloop:
   efl_del(dialer);

 end:
   ecore_con_shutdown();
   ecore_shutdown();

   return retval;
}
