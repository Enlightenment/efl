/**
 * Ecore example illustrating the basics of ecore evas usage.
 *
 * You'll need at least one Evas engine built for it (excluding the
 * buffer one). See stdout/stderr for output.
 *
 * @verbatim
 * gcc -o ecore_evas_basics_example ecore_evas_basics_example.c `pkg-config --libs --cflags ecore evas ecore-evas`
 * @endverbatim
 */

#include <Ecore.h>
#include <Ecore_Evas.h>
#include <unistd.h>

static Eina_Bool
_stdin_cb(void *data EINA_UNUSED, Ecore_Fd_Handler *handler EINA_UNUSED)
{
   Eina_List *l;
   Ecore_Evas *ee;
   char c;

   int ret = scanf("%c", &c);
   if (ret < 1) return ECORE_CALLBACK_RENEW;

   if (c == 'h')
     EINA_LIST_FOREACH(ecore_evas_ecore_evas_list_get(), l, ee)
       ecore_evas_hide(ee);
   else if (c == 's')
     EINA_LIST_FOREACH(ecore_evas_ecore_evas_list_get(), l, ee)
       ecore_evas_show(ee);

   return ECORE_CALLBACK_RENEW;
}

static void
_on_delete(Ecore_Evas *ee)
{
   free(ecore_evas_data_get(ee, "key"));
   ecore_main_loop_quit();
}

int
main(void)
{
   Ecore_Evas *ee;
   Evas *canvas;
   Evas_Object *bg;
   Eina_List *engines, *l;
   char *data;

   if (ecore_evas_init() <= 0)
     return 1;

   engines = ecore_evas_engines_get();
   printf("Available engines:\n");
   EINA_LIST_FOREACH(engines, l, data)
     printf("%s\n", data);
   ecore_evas_engines_free(engines);

   ee = ecore_evas_new(NULL, 0, 0, 200, 200, NULL);
   ecore_evas_title_set(ee, "Ecore Evas basics Example");
   ecore_evas_show(ee);

   data = malloc(sizeof(char) * 6);
   sprintf(data, "%s", "hello");
   ecore_evas_data_set(ee, "key", data);
   ecore_evas_callback_delete_request_set(ee, _on_delete);

   printf("Using %s engine!\n", ecore_evas_engine_name_get(ee));

   canvas = ecore_evas_get(ee);
   if (ecore_evas_ecore_evas_get(canvas) == ee)
     printf("Everything is sane!\n");

   bg = evas_object_rectangle_add(canvas);
   evas_object_color_set(bg, 0, 0, 255, 255);
   evas_object_resize(bg, 200, 200);
   evas_object_show(bg);
   ecore_evas_object_associate(ee, bg, ECORE_EVAS_OBJECT_ASSOCIATE_BASE);

   ecore_main_fd_handler_add(STDIN_FILENO, ECORE_FD_READ, _stdin_cb, NULL, NULL, NULL);

   ecore_main_loop_begin();

   ecore_evas_free(ee);
   ecore_evas_shutdown();

   return 0;
}

