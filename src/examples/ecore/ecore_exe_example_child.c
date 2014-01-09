/**
   Compile with gcc -o ecore_exe_example_child ecore_exe_example_child.c `pkg-config --cflags --libs ecore`
 */

#include <stdio.h>
#include <unistd.h>
#include <Ecore.h>

#define BUFFER_SIZE 1024

static Eina_Bool
_fd_handler_cb(void *data EINA_UNUSED, Ecore_Fd_Handler *fd_handler EINA_UNUSED)
{
   static int numberOfMessages = 0;
   char message[BUFFER_SIZE];

   if (!fgets(message, BUFFER_SIZE, stdin))
     return ECORE_CALLBACK_RENEW;

   numberOfMessages++;

   if (numberOfMessages < 3)
     {
        fprintf(stdout, "My father sent this message to me:%s\n", message);
        fflush(stdout);
        return ECORE_CALLBACK_RENEW;
     }
   else
     {
        fprintf(stdout, "quit\n");
        fflush(stdout);
        ecore_main_loop_quit();
        return ECORE_CALLBACK_DONE;
     }
}

int
main(void)
{
   if (!ecore_init())
     goto error;

   ecore_main_fd_handler_add(STDIN_FILENO,
                             ECORE_FD_READ,
                             _fd_handler_cb,
                             NULL, NULL, NULL);
   ecore_main_loop_begin();

   ecore_shutdown();

   return EXIT_SUCCESS;

error:
   return EXIT_FAILURE;
}

