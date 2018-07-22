#ifndef EVAS_TEST_HELPERS_H
#define EVAS_TEST_HELPERS_H

#include <Ecore_Evas.h>

/* FIXME: Currently depends on the buffer engine, should we do anything
 * fancier? */

#define EVAS_TEST_INIT_EVAS() _setup_evas()

#define EVAS_TEST_FREE_EVAS() \
do \
{ \
   evas_textblock_cursor_free(cur); \
   evas_object_del(tb); \
   evas_textblock_style_free(st); \
   evas_free(evas); \
} \
while (0)

static Evas *
_setup_evas()
{
   Evas *evas;
   Evas_Engine_Info *einfo;

   evas = evas_new();

   evas_output_method_set(evas, evas_render_method_lookup("buffer"));
   einfo = evas_engine_info_get(evas);
   evas_engine_info_set(evas, einfo);

   evas_output_size_set(evas, 500, 500);
   evas_output_viewport_set(evas, 0, 0, 500, 500);

   return evas;
}

#endif
