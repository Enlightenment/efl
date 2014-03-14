#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <unistd.h>
#include <stdio.h>

#include "evas_suite.h"
#include "Evas.h"
#include "evas_tests_helpers.h"

static const char *
_test_image_get(const char *name)
{
   static char filename[PATH_MAX];

   snprintf(filename, PATH_MAX, TESTS_SRC_DIR"/images/%s", name);

   return filename;
}

START_TEST(evas_object_image_loader)
{
   Evas *e = _setup_evas();
   Evas_Object *o;
   Eina_Iterator *it;
   const Eina_File_Direct_Info *file;

   o = evas_object_image_add(e);

   it = eina_file_direct_ls(TESTS_SRC_DIR"/images/");
   EINA_ITERATOR_FOREACH(it, file)
     {
        int w, h;

        evas_object_image_file_set(o, file->path, NULL);
        fail_if(evas_object_image_load_error_get(o) != EVAS_LOAD_ERROR_NONE);
        evas_object_image_size_get(o, &w, &h);
        fail_if(w == 0 || h == 0);
     }
   eina_iterator_free(it);

   evas_object_del(o);

   evas_free(e);
   evas_shutdown();
}
END_TEST

void evas_test_image_object(TCase *tc)
{
   tcase_add_test(tc, evas_object_image_loader);
}
