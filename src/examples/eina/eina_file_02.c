//Compile with:
//gcc -g eina_file_02.c -o eina_file_02 `pkg-config --cflags --libs eina`

#include <stdio.h>
#include <Eina.h>

static Eina_Bool
_progress_cb(void *data, unsigned long long done, unsigned long long total)
{
   const char **files = data;
   printf("%5llu/%llu of copy '%s' to '%s'\n", done, total, files[0], files[1]);
   return EINA_TRUE;
}

int
main(int argc, char **argv)
{
   Eina_Bool ret;

   if (argc != 3)
     {
        fprintf(stderr, "Usage: %s <src> <dst>\n", argv[0]);
        return EXIT_FAILURE;
     }

   eina_init();

   ret = eina_file_copy(argv[1], argv[2],
                        EINA_FILE_COPY_PERMISSION | EINA_FILE_COPY_XATTR,
                        _progress_cb, argv + 1);

   printf("copy finished: %s\n", ret ? "success" : "failure");

   eina_shutdown();

   return 0;
}
