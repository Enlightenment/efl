#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <unistd.h>
#include <stdio.h>

#define EFL_GFX_FILTER_BETA
#define EFL_CANVAS_LAYOUT_BETA

#include "edje_suite.h"

#define EVAS_DATA_DIR TESTS_SRC_DIR "/../../lib/evas"


EFL_START_TEST(edje_test_masking)
{
   int x, y, w, h;
   Evas *evas = _setup_evas();
   const Evas_Object *sub, *clip2, *clip;
   Evas_Object *obj;

   obj = edje_object_add(evas);
   fail_unless(edje_object_file_set(obj, test_layout_get("test_masking.edj"), "test_group"));

   evas_object_resize(obj, 100, 100);
   edje_object_part_geometry_get(obj, "background", &x, &y, &w, &h);
   fail_if(x != 0 || y != 0 || w != 100 || h != 100);

   edje_object_freeze(obj);
   clip = edje_object_part_object_get(obj, "clip2");
   edje_object_thaw(obj);
   fail_if(!clip);

   /* check value of no_render flag as seen from evas land */
   edje_object_freeze(obj);
   sub = edje_object_part_object_get(obj, "mask");
   edje_object_thaw(obj);
   fail_if(!efl_canvas_object_no_render_get(sub));

   /* check that text has a clip (based on description.clip_to) */
   edje_object_freeze(obj);
   sub = edje_object_part_object_get(obj, "text");
   edje_object_thaw(obj);
   fail_if(!efl_canvas_object_clip_get(sub));

   /* test description.clip_to override */
   edje_object_freeze(obj);
   sub = edje_object_part_object_get(obj, "noclip");
   edje_object_thaw(obj);
   clip2 = efl_canvas_object_clip_get(sub);
   fail_if(clip != clip2);

   evas_free(evas);
}
EFL_END_TEST

EFL_START_TEST(edje_test_filters)
{
   Evas *evas = _setup_evas();
   const Evas_Object *text, *sub;
   Evas_Object *obj, *src = NULL;
   const char *prg, *name;

   setenv("EVAS_DATA_DIR", EVAS_DATA_DIR, 1);

   obj = edje_object_add(evas);
   fail_unless(edje_object_file_set(obj, test_layout_get("test_filters.edj"), "test_group"));

   evas_object_resize(obj, 200, 200);

   /* check value of no_render flag as seen from evas land */
   edje_object_freeze(obj);
   sub = edje_object_part_object_get(obj, "mask");
   edje_object_thaw(obj);
   fail_if(!efl_canvas_object_no_render_get(sub));

   /* check no_render inheritance */
   edje_object_freeze(obj);
   sub = edje_object_part_object_get(obj, "mask2");
   edje_object_thaw(obj);
   fail_if(efl_canvas_object_no_render_get(sub));
   edje_object_freeze(obj);
   sub = edje_object_part_object_get(obj, "mask3");
   edje_object_thaw(obj);
   fail_if(!efl_canvas_object_no_render_get(sub));

   /* text part: check filter status */
   edje_object_freeze(obj);
   text = edje_object_part_object_get(obj, "text");
   edje_object_thaw(obj);
   fail_if(!text);

   efl_gfx_filter_program_get(text, &prg, &name);
   fail_if(!prg);
   fail_if(!name || strcmp(name, "filterfile"));

   src = efl_gfx_filter_source_get(text, "mask");
   fail_if(!src);

   // TODO: Verify properly that the filter runs well

   evas_free(evas);
}
EFL_END_TEST

EFL_START_TEST(edje_test_snapshot)
{
   Evas *evas = _setup_evas();
   const Evas_Object *sub;
   Evas_Object *obj;

   setenv("EVAS_DATA_DIR", EVAS_DATA_DIR, 1);

   obj = edje_object_add(evas);
   fail_unless(edje_object_file_set(obj, test_layout_get("test_snapshot.edj"), "test_group"));

   evas_object_resize(obj, 200, 200);

   /* check value of no_render flag as seen from evas land */
   edje_object_freeze(obj);
   sub = edje_object_part_object_get(obj, "snap");
   edje_object_thaw(obj);
   fail_if(!efl_isa(sub, EFL_CANVAS_SNAPSHOT_CLASS) &&
           !evas_object_image_snapshot_get(sub));

   // TODO: Verify that evas snapshot actually works (and has a filter)

   evas_free(evas);
}
EFL_END_TEST

EFL_START_TEST(edje_test_size_class)
{
   int minw, minh, minw2, minh2;
   Evas *evas = _setup_evas();
   Eina_List *l;
   Eina_Stringshare *name;
   Evas_Object *obj, *obj2;
   Eina_Bool b;

   obj = edje_object_add(evas);
   fail_unless(edje_object_file_set(obj, test_layout_get("test_size_class.edj"), "test_group"));

   obj2 = edje_object_add(evas);
   fail_unless(edje_object_file_set(obj2, test_layout_get("test_size_class.edj"), "test_group"));

   evas_object_resize(obj, 200, 200);
   evas_object_resize(obj2, 200, 200);

   /* check predefined min size of rect part by edc */
   edje_object_part_geometry_get(obj, "rect", NULL, NULL, &minw, &minh);
   fail_if(minw != 50 || minh != 50);

   /* check that edje_size_class_set works */
   b = edje_size_class_set("rect_size", 100, 100, -1, -1);
   edje_object_part_geometry_get(obj, "rect", NULL, NULL, &minw, &minh);
   edje_object_part_geometry_get(obj2, "rect", NULL, NULL, &minw2, &minh2);
   fail_if(!b || minw != 100 || minh != 100 || minw2 != 100 || minh2 != 100);

   /* check that edje_size_class_get works */
   b = edje_size_class_get("rect_size", &minw, &minh, NULL, NULL);
   fail_if(!b || minw != 100 || minh != 100);

   /* check that edje_size_class_list works */
   l = edje_size_class_list();
   EINA_LIST_FREE(l, name)
     {
        fail_if(strcmp(name, "rect_size"));
        eina_stringshare_del(name);
     }

   /* check that edje_object_size_class_set works */
   b = edje_object_size_class_set(obj, "rect_size", 150, 150, -1, -1);
   edje_object_part_geometry_get(obj, "rect", NULL, NULL, &minw, &minh);
   edje_object_part_geometry_get(obj2, "rect", NULL, NULL, &minw2, &minh2);
   fail_if(!b || minw != 150 || minh != 150 || minw2 != 100 || minh2 != 100);

   /* check that edje_object_size_class_get works */
   b = edje_object_size_class_get(obj, "rect_size", &minw, &minh, NULL, NULL);
   fail_if(!b || minw != 150 || minh != 150);

   evas_free(evas);
}
EFL_END_TEST

EFL_START_TEST(edje_test_color_class)
{
   Evas *evas = _setup_evas();

   Eina_File *f;
   Eina_Iterator *it;
   Edje_Color_Class *itcc, *cc = NULL;
   char *filename;

   filename = realpath(test_layout_get("test_color_class.edj"), NULL);
   fail_if(!filename);

   f = eina_file_open(filename, EINA_FALSE);
   fail_if(!f);

   it = edje_mmap_color_class_iterator_new(f);
   fail_if(!it);
   EINA_ITERATOR_FOREACH(it, itcc)
     {
        if (!strcmp(itcc->name, "test_color_class"))
          {
             cc = itcc;
             break;
          }
     }
   fail_if((!cc) || (cc->r != 100) || (cc->g != 100) || (cc->b != 100) || (cc->a != 100));

   eina_iterator_free(it);
   eina_file_close(f);
   free(filename);

   evas_free(evas);
}
EFL_END_TEST

void edje_test_features(TCase *tc)
{
   tcase_add_test(tc, edje_test_masking);
   tcase_add_test(tc, edje_test_filters);
   tcase_add_test(tc, edje_test_snapshot);
   tcase_add_test(tc, edje_test_size_class);
   tcase_add_test(tc, edje_test_color_class);
}
