//Compile with:
//gcc -g eina_xattr_01.c -o eina_xattr_01 `pkg-config --cflags --libs eina`

#include <Eina.h>

int
main(int argc, char **argv)
{
   if (argc < 3)
     {
        fprintf(stderr, "ERROR: incorrect usage.\n"
                "Usage:\n"
                "\t%s list <file>\n"
                "\t%s get <file> <key>\n"
                "\t%s del <file> <key>\n"
                "\t%s set <file> <key> <value>\n",
                argv[0], argv[0], argv[0], argv[0]);
        return EXIT_FAILURE;
     }

   if (strcmp(argv[1], "list") == 0)
     {
        Eina_Iterator *itr;
        Eina_Xattr *xattr;

        printf("xattrs of %s:\n", argv[2]);

        eina_init();

        itr = eina_xattr_value_ls(argv[2]);
        EINA_ITERATOR_FOREACH(itr, xattr)
          printf("\t%s: %.*s\n", xattr->name, (int)xattr->length, xattr->value);
        eina_iterator_free(itr);

        eina_shutdown();

     }
   else if (strcmp(argv[1], "get") == 0)
     {
        ssize_t size = 0;
        const char *value;

        if (argc < 4)
          {
             fputs("ERROR: missing key\n", stderr);
             return EXIT_FAILURE;
          }

        eina_init();
        value = eina_xattr_get(argv[2], argv[3], &size);
        printf("%s: %.*s\n", argv[3], (int)size, value);
        eina_shutdown();
     }
   else if (strcmp(argv[1], "del") == 0)
     {
        Eina_Bool ret;

        if (argc < 4)
          {
             fputs("ERROR: missing key\n", stderr);
             return EXIT_FAILURE;
          }

        eina_init();
        ret = eina_xattr_del(argv[2], argv[3]);
        printf("del xattr '%s': %s\n", argv[3], ret ? "success" : "failure");
        eina_shutdown();
     }
   else if (strcmp(argv[1], "set") == 0)
     {
        Eina_Bool ret;

        if (argc < 5)
          {
             fputs("ERROR: missing key or value\n", stderr);
             return EXIT_FAILURE;
          }

        eina_init();
        ret = eina_xattr_set(argv[2], argv[3], argv[4], strlen(argv[4]), 0);

        printf("set xattr '%s=%s' on '%s': %s\n",
               argv[3], argv[4], argv[2], ret ? "success" : "failure");

        eina_shutdown();
     }
   else
     fprintf(stderr, "ERROR: unknown command '%s'\n", argv[1]);

   return 0;
}
