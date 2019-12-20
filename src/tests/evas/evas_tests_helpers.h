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
   Ecore_Evas *ee;

   ee = ecore_evas_buffer_new(500, 500);
   return ecore_evas_get(ee);
}

#endif
