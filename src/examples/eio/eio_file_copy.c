//Compile with:
//gcc -o eio_file_copy eio_file_copy.c `pkg-config --cflags --libs ecore eio`

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <Eio.h>
#include <Ecore.h>

static void
_progress_cb(void *data EINA_UNUSED, Eio_File *handle EINA_UNUSED, const Eio_Progress *info)
{
   printf("progress: %5lld/%lld (%3.0f%%)\n", info->current, info->max,
          info->percent);
}

static void
_done_cb(void *data EINA_UNUSED, Eio_File *handle EINA_UNUSED)
{
   puts("Successfully finished copy");
   ecore_main_loop_quit();
}

static void
_error_cb(void *data EINA_UNUSED, Eio_File *handle EINA_UNUSED, int error)
{
   fprintf(stderr, "Something has gone wrong:%s\n", strerror(error));
   ecore_main_loop_quit();
}

int
main(int argc, char **argv)
{
   if (argc != 3)
     {
        fprintf(stderr, "Usage: %s <src> <dst>\n", argv[0]);
        return  EXIT_FAILURE;
     }

   ecore_init();
   eio_init();

   eio_file_copy(argv[1], argv[2], _progress_cb, _done_cb, _error_cb, NULL);

   ecore_main_loop_begin();

   eio_shutdown();
   ecore_shutdown();

   return 0;
}
