#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <unistd.h>

#include <Ecore_Wayland.h>

#include "ecore_suite.h"
#include "wayland-server.h"

#define MAX_ITER 10

static char test_socket[] = "test1";

START_TEST(ecore_test_ecore_wl_shutdown_bef_init)
{
   struct wl_display *test_display = NULL;
   int ret = 0;

   ret = ecore_wl_shutdown();
   fprintf(stderr, "Calling ecore_wl_shutdown without calling ecore_wl_init\n");
   fail_if(ret != 0);

   test_display = wl_display_create();
   fprintf(stderr, "Creating wayland display\n");
   fail_if(test_display == NULL);

   ret = wl_display_add_socket(test_display, test_socket);
   fprintf(stderr, "Connecting %s socket to wayland display\n", test_socket);
   fail_if(ret != 0);

   ret = ecore_wl_init(test_socket);
   fprintf(stderr, "Calling ecore_wl_init with %s\n", test_socket);
   fail_if(ret != 1);

   ret = ecore_wl_shutdown();
   fprintf(stderr, "Calling ecore_wl_shutdown after ecore_wl_init.\n");
   fail_if(ret != 0);

   wl_display_destroy(test_display);
}
END_TEST

START_TEST(ecore_test_ecore_wl_init_name)
{
   struct wl_display *test_display = NULL;
   int ret = 0, i, j;

   test_display = wl_display_create();
   fprintf(stderr, "Creating display\n");
   fail_if(test_display == NULL);

   ret = wl_display_add_socket(test_display, test_socket);
   fprintf(stderr, "Connecting socket to display\n");
   fail_if(ret != 0);

   for (i = 1; i <= MAX_ITER; i++)
     {
        ret = ecore_wl_init(test_socket);
        fprintf(stderr, "Created %d ecore wayland instance.\n", i);
        fail_if(ret != i);
     }

   for (j = MAX_ITER - 1; j >= 0; j--)
     {
        ret = ecore_wl_shutdown();
        fprintf(stderr, "Deleted %d ecore wayland instance.\n", MAX_ITER - j);
        fail_if(ret != j);
     }

   wl_display_destroy(test_display);
}
END_TEST

START_TEST(ecore_test_ecore_wl_init)
{
#define PATH_LEN  128
   struct wl_display *test_display = NULL;
   int ret, i, j, run_test = 0;
   char *wayland_display = NULL, *xdg_runtime_dir = NULL, path[PATH_LEN];

   xdg_runtime_dir = getenv("XDG_RUNTIME_DIR");
   if (xdg_runtime_dir)
     {
        wayland_display = getenv("WAYLAND_DISPLAY");
        if (!wayland_display)
          wayland_display = "wayland-0";

        snprintf(path, PATH_LEN, "%s/%s", xdg_runtime_dir, wayland_display);
        if (access(path, F_OK) != -1)
          {
             run_test = 1;
          }
        else
          {
             test_display = wl_display_create();
             fprintf(stderr, "Creating display\n");
             fail_if(test_display == NULL);

             ret = wl_display_add_socket(test_display, NULL);
             fprintf(stderr, "Connecting socket to display\n");
             fail_if(ret != 0);

             run_test = 1;
          }
     }

   if (run_test)
     {
        for (i = 1; i <= MAX_ITER; i++)
          {
             ret = ecore_wl_init(NULL);
             fprintf(stderr, "Created %d ecore wayland instance.\n", i);
             fail_if(ret != i);
          }

        for (j = MAX_ITER - 1; j >= 0; j--)
          {
             ret = ecore_wl_shutdown();
             fprintf(stderr, "Deleted %d ecore wayland instance.\n", MAX_ITER - j);
             fail_if(ret != j);
          }
     }
}
END_TEST

void ecore_test_ecore_wayland(TCase *tc)
{
   tcase_add_test(tc, ecore_test_ecore_wl_init);
   tcase_add_test(tc, ecore_test_ecore_wl_init_name);
   tcase_add_test(tc, ecore_test_ecore_wl_shutdown_bef_init);
}
