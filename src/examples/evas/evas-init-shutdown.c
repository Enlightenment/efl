/**
 * Example of initializing and shutting down Evas.
 *
 * For this introduction to Evas, we'll simply turn it on and off and
 * explain everything we're doing from the absolute basics, to provide
 * a starting point for understanding the other examples in this set,
 * which won't be explaining things quite so verbosely.
 *
 * See the @ref Example_Evas_Buffer_Simple as the next step in the tutorial.
 *
 * Evas examples can be built using the regular `make examples`, but each
 * example includes the compiler line for building that specific example if
 * you wish to build it in isolation:
 *
 * @verbatim
 * gcc -o evas-init-shutdown evas-init-shutdown.c `pkg-config --libs --cflags evas`
 * @endverbatim
 *
 * For this compilation to function properly, you'll need to have built and
 * installed the evas library and include files in a location that the pkg-config
 * tool will be able to find.  The example executable will be 'evas-init-shutdown'.
 */

#include <Evas.h>
#include <stdio.h>
#include <errno.h>

int
main(void)
{
   Eina_List *engine_list, *l;
   char *engine_name;

   /* Initialize Evas.  This will startup other dependencies such as
    * eina, eet, ecore, etc. and initializes various internal things
    * (threads, filters, etc.) */
   evas_init();

   /* When building Evas you can configure a variety of engines to be
    * built with it.  Get a list of what engines are available using the
    * evas_render_method_list routine.
    */
   engine_list = evas_render_method_list();
   if (!engine_list)
     {
        fprintf(stderr, "ERROR: Evas supports no engines! Exit.\n");
        exit(-1);
     }

   /* 'engine_list' is a linked list (@see Eina_List.)  The
    * EINA_LIST_FOREACH macro permits navigating through the items using
    * the iterator 'l', making the node data available as 'engine_name'.
    */
   printf("Available Evas Engines:\n");
   EINA_LIST_FOREACH(engine_list, l, engine_name)
     printf("%s\n", engine_name);

   /* To free the list, we use evas_render_method_list's corresponding
    * destructor routine.
    */
   evas_render_method_list_free(engine_list);

   /* Shuts down all dependencies if nothing else is using them, and
    * clears allocated data held internally.
    */
   evas_shutdown();
   return 0;
}
