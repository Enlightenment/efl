#ifndef EDJE_TEST_HELPERS_H
#define EDJE_TEST_HELPERS_H

/* FIXME: Currently depends on the buffer engine, should we do anything
 * fancier? */

#define EDJE_TEST_INIT_EVAS() _setup_evas()

#define EDJE_TEST_FREE_EVAS() \
  do			      \
    {			      \
      edje_shutdown();	      \
      evas_free(evas);	      \
      evas_shutdown();	      \
    }			      \
  while (0)

static Evas *
_setup_evas()
{
   Evas *evas;
   Evas_Engine_Info *einfo;

   evas_init();
   edje_init();

   evas = evas_new();

   evas_output_method_set(evas, evas_render_method_lookup("buffer"));
   einfo = evas_engine_info_get(evas);
   evas_engine_info_set(evas, einfo);

   evas_output_size_set(evas, 500, 500);
   evas_output_viewport_set(evas, 0, 0, 500, 500);

   return evas;
}

#endif

