/**
 * Simple example illustrating usage of evas_init() and
 * evas_shutdown(). Usually one would instantiate a canvas to have
 * something useful out of Evas. For an example of this kind, see the
 * @ref Example_Evas_Buffer_Simple.
 *
 * Here, we are just listing the engine Evas was compiled with support
 * to.
 *
 * @verbatim
 * gcc -o evas-init-shutdown evas-init-shutdown.c `pkg-config --libs \
 * --cflags evas`
 * @endverbatim
 *
 */

#include <Evas.h>
#include <stdio.h>
#include <errno.h>

/*
 * Simple example illustrating usage of evas_init() and
 * evas_shutdown().  Usually one would instantiate a canvas to have
 * something useful out of Evas. For an example of this kind, see the
 * evas-buffer-simple.c, which requires the buffer engine module
 * compiled in Evas.
 *
 * Here, we are just listing the engine Evas was compiled with support
 * to.
 */

int
main(void)
{
   Eina_List *engine_list, *l;
   char *engine_name;

   evas_init();

   engine_list = evas_render_method_list();
   if (!engine_list)
     {
        fprintf(stderr, "ERROR: Evas supports no engines! Exit.\n");
        exit(-1);
     }

   printf("Available Evas Engines:\n");
   EINA_LIST_FOREACH(engine_list, l, engine_name)
     printf("%s\n", engine_name);

   evas_render_method_list_free(engine_list);

   evas_shutdown();
   return 0;
 }

