//Compile with:
//gcc -o eio_file_ls eio_file_ls.c `pkg-config --cflags --libs ecore eio`

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <Eio.h>
#include <Ecore.h>

static Eina_Bool
_filter_cb(void *data EINA_UNUSED, Eio_File *handler EINA_UNUSED, const char *file)
{
   char *last_slash = strrchr(file, '/');

   //Check if it is a hidden file
   if (last_slash != NULL && strlen(last_slash) > 1 && last_slash[1] == '.')
     return EINA_FALSE;

   return EINA_TRUE;
}

static void
_main_cb(void *data, Eio_File *handler EINA_UNUSED, const char *file)
{
   int *number_of_listed_files = (int *)data;

   fprintf(stdout, "Processing file:%s\n", file);

   (*number_of_listed_files)++;
}

static void
_done_cb(void *data, Eio_File *handler EINA_UNUSED)
{
   int *number_of_listed_files = (int *)data;

   fprintf(stdout, "Number of listed files:%d\n" \
           "ls operation is done, quitting.\n", *number_of_listed_files);

   ecore_main_loop_quit();
}

static void
_error_cb(void *data EINA_UNUSED, Eio_File *handler EINA_UNUSED, int error)
{
   fprintf(stderr, "Something wrong has happend:%s\n", strerror(error));
   ecore_main_loop_quit();
}

int
main(int argc, char **argv)
{
   int number_of_listed_files = 0;

   ecore_init();
   eio_init();

   if (argc < 2)
     {
        fprintf(stderr, "You must pass a path to execute the command.\n");
        return  -1;
     }

   eio_file_ls(argv[1], _filter_cb, _main_cb, _done_cb, _error_cb,
               &number_of_listed_files);

   ecore_main_loop_begin();

   eio_shutdown();
   ecore_shutdown();

   return 0;
}
