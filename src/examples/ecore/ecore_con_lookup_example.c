//Compile with:
// gcc -o ecore_con_lookup_example ecore_con_lookup_example.c `pkg-config --libs --cflags ecore ecore-con`

#include <stdio.h>
#include <Ecore.h>
#include <Ecore_Con.h>

static void
_lookup_done_cb(const char *canonname, const char *ip, struct sockaddr *addr, int addrlen EINA_UNUSED, void *data EINA_UNUSED)
{
   printf("canonname = %s\n", canonname);
   printf("ip = %s\n", ip);
   printf("addr = %p\n", addr);
   ecore_main_loop_quit();
}

int
main(int argc, const char *argv[])
{
   if (argc < 2)
     {
        printf("need one parameter: <address>\n");
        return -1;
     }

   ecore_con_init();

   if (!ecore_con_lookup(argv[1], _lookup_done_cb, NULL))
     {
        printf("error when trying to start lookup for %s\n", argv[1]);
        goto end;
     }

   ecore_main_loop_begin();

end:
   ecore_con_shutdown();

   return 0;
}

