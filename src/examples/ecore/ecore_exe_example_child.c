/**
   Compile with gcc -o ecore_exe_example_child ecore_exe_example_child.c `pkg-config --cflags --libs ecore`
 */

#include <stdio.h>
#include <unistd.h>
#include <Ecore.h>

#define BUFFER_SIZE 1024

#ifdef _WIN32
#define HANDLER_TYPE Ecore_Win32_Handler
#else
#define HANDLER_TYPE Ecore_Fd_Handler
#endif

static Eina_Bool
_fd_handler_cb(void *data EINA_UNUSED, HANDLER_TYPE *fd_handler EINA_UNUSED)
{
   static int numberOfMessages = 0;
   char message[BUFFER_SIZE];

   if (!fgets(message, BUFFER_SIZE, stdin))
     return ECORE_CALLBACK_RENEW;

   numberOfMessages++;

   if (numberOfMessages < 3)
     {
        printf("My father sent this message to me:%s\n", message);
        fflush(stdout);
        return ECORE_CALLBACK_RENEW;
     }
   else
     {
        printf("quit\n");
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

#ifdef _WIN32
   /* note that stdin fd's on windows don't work the same
    * as on unixes. this uses stdin just as a quick
    * example that's simple instead of a more complex
    * one, so this won't actually work on windows unless
    * you use a fd that comes from somewhere that is
    * select()able. */
   ecore_main_win32_handler_add(GetStdHandle(STD_INPUT_HANDLE),
                                _fd_handler_cb,
                                NULL);
#else
   ecore_main_fd_handler_add(STDIN_FILENO,
                             ECORE_FD_READ,
                             _fd_handler_cb,
                             NULL, NULL, NULL);
#endif
   ecore_main_loop_begin();

   ecore_shutdown();

   return EXIT_SUCCESS;

error:
   return EXIT_FAILURE;
}

