//Compile with:
//gcc -g eina_xattr_02.c -o eina_xattr_02 `pkg-config --cflags --libs eina`

#include <Eina.h>

int
main(int argc, char **argv)
{
   Eina_Bool ret;

   if (argc != 3)
     {
        fprintf(stderr, "ERROR: incorrect usage.\nUsage: %s <src> <dst>\n",
                argv[0]);
        return EXIT_FAILURE;
     }

   eina_init();
   ret = eina_xattr_copy(argv[1], argv[2]);
   printf("copied xattrs from '%s' to '%s': %s\n",
          argv[1], argv[2], ret ? "success" : "failure");
   eina_shutdown();
   return 0;
}
