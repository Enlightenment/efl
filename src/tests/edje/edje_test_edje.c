#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <unistd.h>
#include <stdio.h>

#define EFL_GFX_FILTER_BETA

#include <Eina.h>
#include <Edje.h>

#include "edje_suite.h"
#include "edje_tests_helpers.h"

#define EVAS_DATA_DIR TESTS_SRC_DIR "/../../lib/evas"

START_TEST(edje_test_edje_init)
{
   int ret;

   ret = edje_init();
   fail_if(ret != 1);

   ret = edje_shutdown();
   fail_if(ret != 0);
}
END_TEST

START_TEST(edje_test_edje_load)
{
   Evas *evas = EDJE_TEST_INIT_EVAS();
   Edje_Load_Error error;
   Evas_Object *obj;

   obj = edje_object_add(evas);
   edje_object_file_set(obj, "blaoeuaeoueoaua.edj", "test");
   error = edje_object_load_error_get(obj);
   fail_if(error != EDJE_LOAD_ERROR_DOES_NOT_EXIST);

   EDJE_TEST_FREE_EVAS();
}
END_TEST

static const char *
test_layout_get(const char *name)
{
   static char filename[PATH_MAX];

   snprintf(filename, PATH_MAX, TESTS_BUILD_DIR"/data/%s", name);

   return filename;
}

START_TEST(edje_test_load_simple_layout)
{
   Evas *evas = EDJE_TEST_INIT_EVAS();
   Evas_Object *obj;

   obj = edje_object_add(evas);
   fail_unless(edje_object_file_set(obj, test_layout_get("test_layout.edj"), "test_group"));

   fail_if(edje_object_part_exists(obj, "unexistant_part"));
   fail_unless(edje_object_part_exists(obj, "background"));


   EDJE_TEST_FREE_EVAS();
}
END_TEST

START_TEST(edje_test_simple_layout_geometry)
{
   int x, y, w, h;
   int r, g, b, a;
   Evas *evas = EDJE_TEST_INIT_EVAS();
   Evas_Object *obj;
   const Evas_Object *bg;

   obj = edje_object_add(evas);
   fail_unless(edje_object_file_set(obj, test_layout_get("test_layout.edj"), "test_group"));

   /* rel1.relative: 0.0 0.0;
    * rel2.relative: 1.0 1.0; */
   evas_object_resize(obj, 1000, 1000);
   edje_object_part_geometry_get(obj, "background", &x, &y, &w, &h);

   fail_if(x != 0 || y != 0);
   fail_if(w != 1000 || h != 1000);

   bg = edje_object_part_object_get(obj, "background");
   fail_if(!bg);

   evas_object_color_get(bg, &r, &g, &b, &a);
   fail_if(r != 255 || g != 255 || b != 255 || a != 255);

   EDJE_TEST_FREE_EVAS();
}
END_TEST

START_TEST(edje_test_complex_layout)
{
   int x, y, w, h;
   Evas *evas = EDJE_TEST_INIT_EVAS();
   Evas_Object *obj;

   obj = edje_object_add(evas);
   fail_unless(edje_object_file_set(obj, test_layout_get("complex_layout.edj"), "test_group"));
   evas_object_resize(obj, 1000, 1000);

   /* Apparently rel2 offsets are retarded and you have to introduce off-by-one
    * madness (rel2.offset: -1 -1; acts like there is zero offset...)
    * Take that into account when checking w and h */

   /* rel1.relative: 0.0 0.0; rel1.offset: 0 0;
    * rel2.relative: 1.0 1.0; rel2.offset -1 -1; */
   edje_object_part_geometry_get(obj, "background", &x, &y, &w, &h);
   fail_if(x != 0 || y != 0);
   fail_if(w != 1000-1 + 1 || h != 1000-1 + 1);

   /* rel1.relative: 0.0 0.0; rel1.offset: 1 1;
    * rel2.relative: 1.0 1.0; rel2.offset 0 0; */
   edje_object_part_geometry_get(obj, "background2", &x, &y, &w, &h);
   fail_if(x != 1 || y != 1);
   fail_if(w != 1000-1 + 1 || h != 1000-1 + 1);

   /* rel1.relative: 0.0 0.0; rel1.offset: 5 5;
    * rel2.relative: 0.5 0.5; rel2.offset -2 -2; */
   edje_object_part_geometry_get(obj, "ul", &x, &y, &w, &h);
   fail_if(x != 5 || y != 5);
   fail_if(w != 500-5-2 + 1 || h != 500-5-2 + 1);

   /* rel1.relative: 0.5 0.0; rel1.offset: 2 5;
    * rel2.relative: 1.0 0.5; rel2.offset -5 -2; */
   edje_object_part_geometry_get(obj, "ur", &x, &y, &w, &h);
   fail_if(x != 500+2 || y != 5);
   fail_if(w != 500-5-2 + 1 || h != 500-5-2 + 1);

   /* rel1.relative: 0.0 0.5; rel1.offset: 5 2;
    * rel2.relative: 1.0 1.0; rel2.offset -5 -5; */
   edje_object_part_geometry_get(obj, "l", &x, &y, &w, &h);
   fail_if(x != 5 || y != 500+2);
   fail_if(w != 1000-5-5 + 1 || h != 500-5-2 + 1);

   EDJE_TEST_FREE_EVAS();
}
END_TEST

START_TEST(edje_test_calculate_parens)
{
   int x, y, w, h;
   Evas *evas = EDJE_TEST_INIT_EVAS();
   Evas_Object *obj;

   obj = edje_object_add(evas);
   fail_unless(edje_object_file_set(obj, test_layout_get("test_parens.edj"), "test_group"));

   evas_object_resize(obj, 100, 100);
   edje_object_part_geometry_get(obj, "background", &x, &y, &w, &h);
   fail_if(x != 0 || y != 0 || w != 100 || h != 100);

   EDJE_TEST_FREE_EVAS();
}
END_TEST

START_TEST(edje_test_masking)
{
   int x, y, w, h;
   Evas *evas = EDJE_TEST_INIT_EVAS();
   const Evas_Object *sub, *clip2, *clip;
   Evas_Object *obj;

   obj = edje_object_add(evas);
   fail_unless(edje_object_file_set(obj, test_layout_get("test_masking.edj"), "test_group"));

   evas_object_resize(obj, 100, 100);
   edje_object_part_geometry_get(obj, "background", &x, &y, &w, &h);
   fail_if(x != 0 || y != 0 || w != 100 || h != 100);

   clip = edje_object_part_object_get(obj, "clip2");
   fail_if(!clip);

   /* check value of no_render flag as seen from evas land */
   sub = edje_object_part_object_get(obj, "mask");
   fail_if(!efl_canvas_object_no_render_get(sub));

   /* check that text has a clip (based on description.clip_to) */
   sub = edje_object_part_object_get(obj, "text");
   fail_if(!efl_canvas_object_clip_get(sub));

   /* test description.clip_to override */
   sub = edje_object_part_object_get(obj, "noclip");
   clip2 = efl_canvas_object_clip_get(sub);
   fail_if(clip != clip2);

   EDJE_TEST_FREE_EVAS();
}
END_TEST

START_TEST(edje_test_filters)
{
   Evas *evas = EDJE_TEST_INIT_EVAS();
   const Evas_Object *text, *sub;
   Evas_Object *obj, *src = NULL;
   const char *prg, *name;

   setenv("EVAS_DATA_DIR", EVAS_DATA_DIR, 1);

   obj = edje_object_add(evas);
   fail_unless(edje_object_file_set(obj, test_layout_get("test_filters.edj"), "test_group"));

   evas_object_resize(obj, 200, 200);

   /* check value of no_render flag as seen from evas land */
   sub = edje_object_part_object_get(obj, "mask");
   fail_if(!efl_canvas_object_no_render_get(sub));

   /* check no_render inheritance */
   sub = edje_object_part_object_get(obj, "mask2");
   fail_if(efl_canvas_object_no_render_get(sub));
   sub = edje_object_part_object_get(obj, "mask3");
   fail_if(!efl_canvas_object_no_render_get(sub));

   /* text part: check filter status */
   text = edje_object_part_object_get(obj, "text");
   fail_if(!text);

   efl_gfx_filter_program_get(text, &prg, &name);
   fail_if(!prg);
   fail_if(!name || strcmp(name, "filterfile"));

   src = efl_gfx_filter_source_get(text, "mask");
   fail_if(!src);

   // TODO: Verify properly that the filter runs well

   EDJE_TEST_FREE_EVAS();
}
END_TEST

START_TEST(edje_test_snapshot)
{
   Evas *evas = EDJE_TEST_INIT_EVAS();
   const Evas_Object *sub;
   Evas_Object *obj;

   setenv("EVAS_DATA_DIR", EVAS_DATA_DIR, 1);

   obj = edje_object_add(evas);
   fail_unless(edje_object_file_set(obj, test_layout_get("test_snapshot.edj"), "test_group"));

   evas_object_resize(obj, 200, 200);

   /* check value of no_render flag as seen from evas land */
   sub = edje_object_part_object_get(obj, "snap");
   fail_if(!efl_isa(sub, EFL_CANVAS_SNAPSHOT_CLASS) &&
           !evas_object_image_snapshot_get(sub));

   // TODO: Verify that evas snapshot actually works (and has a filter)

   EDJE_TEST_FREE_EVAS();
}
END_TEST

START_TEST(edje_test_size_class)
{
   int minw, minh, minw2, minh2;
   Evas *evas = EDJE_TEST_INIT_EVAS();
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

   EDJE_TEST_FREE_EVAS();
}
END_TEST

START_TEST(edje_test_color_class)
{
   Evas *evas = EDJE_TEST_INIT_EVAS();

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

   EDJE_TEST_FREE_EVAS();
}
END_TEST

START_TEST(edje_test_swallows)
{
   Evas *evas = EDJE_TEST_INIT_EVAS();
   Evas_Object *ly, *o1, *o2;

   ly = efl_add(EDJE_OBJECT_CLASS, evas);
   fail_unless(edje_object_file_set(ly, test_layout_get("test_swallows.edj"), "test_group"));

   fail_unless(edje_object_part_exists(ly, "swallow"));


   o1 = efl_add(EDJE_OBJECT_CLASS, ly);
   fail_if(!edje_object_part_swallow(ly, "swallow", o1));
   ck_assert_ptr_eq(efl_parent_get(o1), ly);

   edje_object_part_unswallow(ly, o1);
   ck_assert_ptr_eq(efl_parent_get(o1), evas_object_evas_get(o1));

   fail_if(!edje_object_part_swallow(ly, "swallow", o1));
   ck_assert_ptr_eq(efl_parent_get(o1), ly);

   o2 = efl_add(EDJE_OBJECT_CLASS, ly);
   fail_if(!edje_object_part_swallow(ly, "swallow", o2));
   ck_assert_ptr_eq(efl_parent_get(o2), ly);
   /* o1 is deleted at this point. */
   ck_assert_ptr_eq(efl_parent_get(o1), evas_object_evas_get(o1));

   EDJE_TEST_FREE_EVAS();
}
END_TEST

START_TEST(edje_test_swallows_eoapi)
{
   Evas *evas = EDJE_TEST_INIT_EVAS();
   Evas_Object *ly, *o1, *o2;

   ly = efl_add(EDJE_OBJECT_CLASS, evas);
   fail_unless(edje_object_file_set(ly, test_layout_get("test_swallows.edj"), "test_group"));

   fail_unless(edje_object_part_exists(ly, "swallow"));


   o1 = efl_add(EDJE_OBJECT_CLASS, ly);
   fail_if(!efl_content_set(efl_part(ly, "swallow"), o1));
   ck_assert_ptr_eq(efl_parent_get(o1), ly);

   efl_content_remove(ly, o1);
   ck_assert_ptr_eq(efl_parent_get(o1), evas_object_evas_get(o1));

   fail_if(!efl_content_set(efl_part(ly, "swallow"), o1));
   ck_assert_ptr_eq(efl_parent_get(o1), ly);

   o2 = efl_add(EDJE_OBJECT_CLASS, ly);
   fail_if(!efl_content_set(efl_part(ly, "swallow"), o2));
   ck_assert_ptr_eq(efl_parent_get(o2), ly);
   /* o1 is deleted at this point. */
   ck_assert_ptr_eq(efl_parent_get(o1), evas_object_evas_get(o1));

   EDJE_TEST_FREE_EVAS();
}
END_TEST

START_TEST(edje_test_access)
{
   Evas *evas = EDJE_TEST_INIT_EVAS();
   const char *name;
   Evas_Object *obj;
   Eina_Iterator *it;
   Eina_List *list;
   char buf[20];
   int i = 0;

   obj = edje_object_add(evas);
   fail_unless(edje_object_file_set(obj, test_layout_get("test_layout.edj"), "test_group"));

   /* eo api */
   it = edje_obj_access_part_iterate(obj);
   fail_if(!it);

   EINA_ITERATOR_FOREACH(it, name)
     {
        i++;
        sprintf(buf, "access_%d", i);
        fail_if(!name || strcmp(name, buf) != 0);
     }
   fail_if(i != 2);
   eina_iterator_free(it);

   i = 0;

   /* legacy api */
   list = edje_object_access_part_list_get(obj);
   fail_if(!list);
   EINA_LIST_FREE(list, name)
     {
        i++;
        sprintf(buf, "access_%d", i);
        fail_if(!name || strcmp(name, buf) != 0);
     }
   fail_if(i != 2);

   EDJE_TEST_FREE_EVAS();
}
END_TEST

START_TEST(edje_test_box)
{
   Evas *evas;
   Evas_Object *obj, *sobj, *sobjs[5];
   const Evas_Object *box;
   Eina_Iterator *it;
   int i;

   evas = EDJE_TEST_INIT_EVAS();

   obj = edje_object_add(evas);
   fail_unless(edje_object_file_set(obj, test_layout_get("test_box.edj"), "test_group"));

   for (i = 0; i < 5; i++)
     {
        sobjs[i] = evas_object_rectangle_add(evas);
        fail_if(!sobjs[i]);
     }

   edje_object_part_box_append(obj, "box", sobjs[3]);
   edje_object_part_box_prepend(obj, "box", sobjs[1]);
   edje_object_part_box_insert_before(obj, "box", sobjs[0], sobjs[1]);
   edje_object_part_box_insert_after(obj, "box", sobjs[4], sobjs[3]);
   edje_object_part_box_insert_at(obj, "box", sobjs[2], 2);

   box = edje_object_part_object_get(obj, "box");
   it = evas_object_box_iterator_new(box);

   i = 0;
   EINA_ITERATOR_FOREACH(it, sobj)
     {
        fail_if(sobj != sobjs[i++]);
     }
   eina_iterator_free(it);

   EDJE_TEST_FREE_EVAS();
}
END_TEST

START_TEST(edje_test_box_eoapi)
{
   Evas *evas;
   Evas_Object *obj, *sobj, *sobjs[5];
   Eina_Iterator *it;
   int i;

   evas = EDJE_TEST_INIT_EVAS();

   obj = edje_object_add(evas);
   fail_unless(edje_object_file_set(obj, test_layout_get("test_box.edj"), "test_group"));

   for (i = 0; i < 5; i++)
     {
        sobjs[i] = evas_object_rectangle_add(evas);
        fail_if(!sobjs[i]);
     }

   /* same test case as legacy api above */
   efl_pack_end(efl_part(obj, "box"), sobjs[3]);
   efl_pack_begin(efl_part(obj, "box"), sobjs[1]);
   efl_pack_before(efl_part(obj, "box"), sobjs[0], sobjs[1]);
   efl_pack_after(efl_part(obj, "box"), sobjs[4], sobjs[3]);
   efl_pack_at(efl_part(obj, "box"), sobjs[2], 2);
   fail_if(efl_content_count(efl_part(obj, "box")) != 5);

   it = efl_content_iterate(efl_part(obj, "box"));
   i = 0;
   EINA_ITERATOR_FOREACH(it, sobj)
     fail_if(sobj != sobjs[i++]);
   fail_if(i != 5);
   eina_iterator_free(it);

   /* clear up and test a bit more */
   efl_pack_unpack_all(efl_part(obj, "box"));
   fail_if(efl_content_count(efl_part(obj, "box")) != 0);

   efl_pack(efl_part(obj, "box"), sobjs[1]);
   efl_pack_at(efl_part(obj, "box"), sobjs[0], 0);
   efl_pack_at(efl_part(obj, "box"), sobjs[2], -1);
   it = efl_content_iterate(efl_part(obj, "box"));
   i = 0;
   EINA_ITERATOR_FOREACH(it, sobj)
     fail_if(sobj != sobjs[i++]);
   fail_if(i != 3);
   eina_iterator_free(it);

   fail_if(!efl_content_remove(efl_part(obj, "box"), sobjs[0]));
   fail_if(efl_content_count(efl_part(obj, "box")) != 2);
   fail_if(!efl_pack_unpack_at(efl_part(obj, "box"), 1));
   fail_if(efl_content_count(efl_part(obj, "box")) != 1);
   fail_if(efl_pack_index_get(efl_part(obj, "box"), sobjs[1]) != 0);

   efl_pack_clear(efl_part(obj, "box"));
   fail_if(efl_content_count(efl_part(obj, "box")) != 0);

   EDJE_TEST_FREE_EVAS();
}
END_TEST

START_TEST(edje_test_table)
{
   Evas *evas;
   Evas_Object *obj, *sobj, *sobjs[4];
   int i, k, l, cols, rows;

   evas = EDJE_TEST_INIT_EVAS();

   obj = edje_object_add(evas);
   fail_unless(edje_object_file_set(obj, test_layout_get("test_table.edj"), "test_group"));

   /* check items from EDC */
   for (l = 0; l < 2; l++)
     for (k = 0; k < 2; k++)
       {
          const char *txt;
          char buf[20];

          /* items have a text part "text" containing their position */
          sprintf(buf, "%d,%d", k, l);
          sobj = edje_object_part_table_child_get(obj, "table", k, l);
          fail_if(!sobj);
          txt = edje_object_part_text_get(sobj, "text");
          fail_if(!txt);
          fail_if(strcmp(txt, buf) != 0);
       }

   /* Add more items */
   for (l = 0; l < 2; l++)
     for (k = 0; k < 2; k++)
       {
          i = l*2 + k;
          sobjs[i] = evas_object_rectangle_add(evas);
          fail_if(!sobjs[i]);
          edje_object_part_table_pack(obj, "table", sobjs[i], k, l + 2, 1, 1);
       }

   for (l = 0; l < 2; l++)
     for (k = 0; k < 2; k++)
       {
          i = l*2 + k;
          sobj = edje_object_part_table_child_get(obj, "table", k, l + 2);
          fail_if(sobj != sobjs[i]);
       }

   /* table size and clear */
   edje_object_part_table_col_row_size_get(obj, "table", &cols, &rows);
   fail_if(cols != 2);
   fail_if(rows != 4);

   edje_object_part_table_clear(obj, "table", EINA_TRUE);

   edje_object_part_table_col_row_size_get(obj, "table", &cols, &rows);
   fail_if(cols != 2);
   fail_if(rows != 2);

   for (l = 0; l < 2; l++)
     for (k = 0; k < 2; k++)
       {
          const char *txt;
          char buf[20];

          /* items have a text part "text" containing their position */
          sprintf(buf, "%d,%d", k, l);
          sobj = edje_object_part_table_child_get(obj, "table", k, l);
          fail_if(!sobj);
          txt = edje_object_part_text_get(sobj, "text");
          fail_if(!txt);
          fail_if(strcmp(txt, buf) != 0);
       }

   EDJE_TEST_FREE_EVAS();
}
END_TEST

START_TEST(edje_test_table_eoapi)
{
   Evas *evas;
   Evas_Object *obj, *sobj, *sobjs[4], *proxy;
   Eina_Iterator *it;
   int i, k, l, cs, rs, cols, rows;

   evas = EDJE_TEST_INIT_EVAS();

   obj = edje_object_add(evas);
   fail_unless(edje_object_file_set(obj, test_layout_get("test_table.edj"), "test_group"));

   /* check items from EDC */
   fail_if(efl_content_count(efl_part(obj, "table")) != 4);
   for (l = 0; l < 2; l++)
     for (k = 0; k < 2; k++)
       {
          const char *txt;
          char buf[20];

          /* items have a text part "text" containing their position */
          sprintf(buf, "%d,%d", k, l);
          sobj = efl_pack_grid_content_get(efl_part(obj, "table"), k, l);
          fail_if(!sobj);
          //txt = efl_part_text_get(sobj, "text");
          txt = edje_object_part_text_get(sobj, "text");
          fail_if(!txt);
          fail_if(strcmp(txt, buf) != 0);
       }

   /* Add more items */
   for (l = 0; l < 2; l++)
     for (k = 0; k < 2; k++)
       {
          i = l*2 + k;
          sobjs[i] = efl_add(EFL_CANVAS_RECTANGLE_CLASS, evas);
          fail_if(!sobjs[i]);
          efl_pack_grid(efl_part(obj, "table"), sobjs[i], k, l + 2, 1, 1);
       }

   fail_if(efl_content_count(efl_part(obj, "table")) != 8);

   i = 0;
   it = efl_content_iterate(efl_part(obj, "table"));
   EINA_ITERATOR_FOREACH(it, sobj)
     {
        efl_pack_grid_position_get(efl_part(obj, "table"), sobj, &k, &l, &cs, &rs);
        fail_if(cs != 1);
        fail_if(rs != 1);
        if (l >= 2)
          fail_if(sobj != sobjs[(l - 2)*2 + k]);
        i++;
     }
   eina_iterator_free(it);
   fail_if(i != 8);

   /* table size and clear */
   efl_pack_grid_size_get(efl_part(obj, "table"), &cols, &rows);
   fail_if(cols != 2);
   fail_if(rows != 4);

   efl_pack_clear(efl_part(obj, "table"));
   fail_if(efl_content_count(efl_part(obj, "table")) != 4);

   efl_pack_grid_size_get(efl_part(obj, "table"), &cols, &rows);
   fail_if(cols != 2);
   fail_if(rows != 2);

   /* test multiple ops on a proxy object */
   proxy = efl_ref(efl_part(obj, "table"));
   fail_if(!proxy);
   fail_if(!efl_pack_clear(proxy));
   fail_if(efl_content_count(efl_part(obj, "table")) != 4);
   fail_if(!efl_pack_clear(proxy));
   fail_if(efl_content_count(efl_part(obj, "table2")) != 1);
   fail_if(efl_content_count(proxy) != 4);
   efl_del(proxy);

   EDJE_TEST_FREE_EVAS();
}
END_TEST

START_TEST(edje_test_combine_keywords)
{
   Evas *evas;
   Evas_Object *obj;

   evas = EDJE_TEST_INIT_EVAS();

   obj = edje_object_add(evas);
   fail_unless(edje_object_file_set(obj, test_layout_get("test_combine_keywords.edj"), "test_group"));

   EDJE_TEST_FREE_EVAS();
}
END_TEST

static void
_message_signal_reply_cb(void *data, Evas_Object *obj EINA_UNUSED,
                         const char *emission, const char *source)
{
   int *id = data;

   fprintf(stderr, "source %s emit %s id %d\n", source, emission, *id);
   fflush(stderr);
   ck_assert_str_eq(source, "edc");
   ck_assert_ptr_ne(emission, NULL);

   if (!strncmp(emission, "int set", 7))
     ck_assert_str_eq(emission, "int set 7 12 42 255");
   else if (!strncmp(emission, "int", 3))
     ck_assert_str_eq(emission, "int 42");
   else if (!strncmp(emission, "float", 5))
     {
        char buf[64];
        sprintf(buf, "float %f", 0.12);
        ck_assert_str_eq(emission, buf);
     }
   else if (!strncmp(emission, "str", 3))
     ck_assert_str_eq(emission, "str hello world");
   else ck_abort_msg("Invalid emission!");

   (*id)++;
}

START_TEST(edje_test_message_send_legacy)
{
   Evas *evas;
   Evas_Object *obj;
   Edje_Message_Int msgi;
   Edje_Message_Float msgf;
   Edje_Message_String msgs;
   Edje_Message_Int_Set *msgis;
   int id = 0;

   /* Ugly calls to process:
    *
    * 1. Send edje message (async)
    * 2. Process edje message (sync)
    * 3. EDC program emits edje signal (async)
    * 4. Process edje signal (sync)
    * 5. Finally reached signal cb
    */

   evas = EDJE_TEST_INIT_EVAS();

   obj = edje_object_add(evas);
   fail_unless(edje_object_file_set(obj, test_layout_get("test_messages.edj"), "test_group"));
   edje_object_signal_callback_add(obj, "*", "edc", _message_signal_reply_cb, &id);

   msgs.str = "hello world";
   edje_object_message_send(obj, EDJE_MESSAGE_STRING, 0, &msgs);
   edje_message_signal_process();
   ck_assert_int_eq(id, 1);

   msgi.val = 42;
   edje_object_message_send(obj, EDJE_MESSAGE_INT, 1, &msgi);
   edje_message_signal_process();
   ck_assert_int_eq(id, 2);

   msgf.val = 0.12;
   edje_object_message_send(obj, EDJE_MESSAGE_FLOAT, 2, &msgf);
   edje_message_signal_process();
   ck_assert_int_eq(id, 3);

   msgis = alloca(sizeof(*msgis) + 4 * sizeof(msgis->val));
   msgis->count = 4;
   msgis->val[0] = 7;
   msgis->val[1] = 12;
   msgis->val[2] = 42;
   msgis->val[3] = 255;
   edje_object_message_send(obj, EDJE_MESSAGE_INT_SET, 3, msgis);
   edje_message_signal_process();
   ck_assert_int_eq(id, 4);

   evas_object_del(obj);

   EDJE_TEST_FREE_EVAS();
}
END_TEST

START_TEST(edje_test_message_send_eo)
{
   Evas *evas;
   Evas_Object *obj;
   Eina_Value v, *va;
   int id = 0;

   evas = EDJE_TEST_INIT_EVAS();

   obj = efl_add(EDJE_OBJECT_CLASS, evas,
                 efl_file_set(efl_added, test_layout_get("test_messages.edj"), "test_group"));

   // FIXME: EO API HERE
   edje_object_signal_callback_add(obj, "*", "edc", _message_signal_reply_cb, &id);

   // NOTE: edje_object_message_signal_process may or may not be in EO (TBD)

   eina_value_setup(&v, EINA_VALUE_TYPE_STRING);
   eina_value_set(&v, "hello world");
   efl_canvas_layout_signal_message_send(obj, 0, v);
   eina_value_flush(&v);
   edje_message_signal_process();
   edje_object_calc_force(obj);
   ck_assert_int_eq(id, 1);

   eina_value_setup(&v, EINA_VALUE_TYPE_INT);
   eina_value_set(&v, 42);
   efl_canvas_layout_signal_message_send(obj, 1, v);
   eina_value_flush(&v);
   edje_message_signal_process();
   edje_object_calc_force(obj);
   ck_assert_int_eq(id, 2);

   eina_value_setup(&v, EINA_VALUE_TYPE_FLOAT);
   eina_value_set(&v, 0.12);
   efl_canvas_layout_signal_message_send(obj, 2, v);
   eina_value_flush(&v);
   edje_message_signal_process();
   edje_object_calc_force(obj);
   ck_assert_int_eq(id, 3);

   va = eina_value_array_new(EINA_VALUE_TYPE_INT, 4);
   eina_value_array_append(va, 7);
   eina_value_array_append(va, 12);
   eina_value_array_append(va, 42);
   eina_value_array_append(va, 255);
   efl_canvas_layout_signal_message_send(obj, 3, *va);
   eina_value_free(va);
   edje_message_signal_process();
   edje_object_calc_force(obj);
   ck_assert_int_eq(id, 4);

   efl_del(obj);

   EDJE_TEST_FREE_EVAS();
}
END_TEST

START_TEST(edje_test_signals)
{
   Evas *evas;
   Evas_Object *obj;
   const char *state;

   evas = EDJE_TEST_INIT_EVAS();

   obj = efl_add(EDJE_OBJECT_CLASS, evas,
                 efl_file_set(efl_added, test_layout_get("test_signals.edj"), "level1"),
                 efl_gfx_size_set(efl_added, 320, 240),
                 efl_gfx_visible_set(efl_added, 1));

   edje_object_signal_emit(obj, "mouse,in", "text");

   edje_object_message_signal_process(obj);
   state = edje_object_part_state_get(obj, "group:group:text", NULL);
   ck_assert_str_eq(state, "default");

   edje_object_message_signal_process(obj);
   state = edje_object_part_state_get(obj, "group:group:text", NULL);
   ck_assert_str_eq(state, "default");

   edje_object_message_signal_recursive_process(obj);
   state = edje_object_part_state_get(obj, "group:group:text", NULL);
   ck_assert_str_eq(state, "over");

   efl_del(obj);

   EDJE_TEST_FREE_EVAS();
}
END_TEST

static int _signal_count;

static void
_signal_callback_count_cb(void *data, Evas_Object *obj EINA_UNUSED,
                         const char *emission EINA_UNUSED, const char *source EINA_UNUSED)
{
   int *_data = data;
   _signal_count += *_data;
}

START_TEST(edje_test_signal_callback_del_full)
{
   Evas *evas;
   Evas_Object *obj;
   int data[4] = { 1, 2, 4, 8 };

   evas = EDJE_TEST_INIT_EVAS();

   obj = efl_add(EDJE_OBJECT_CLASS, evas,
                 efl_file_set(efl_added,
                 test_layout_get("test_signal_callback_del_full.edj"), "test"),
                 efl_gfx_size_set(efl_added, 320, 240),
                 efl_gfx_visible_set(efl_added, 1));

   edje_object_signal_callback_add(obj, "some_signal", "event", _signal_callback_count_cb, &data[0]);
   edje_object_signal_callback_add(obj, "some_signal", "event", _signal_callback_count_cb, &data[1]);
   edje_object_signal_callback_add(obj, "some_signal", "event", _signal_callback_count_cb, &data[2]);
   edje_object_signal_callback_add(obj, "some_signal", "event", _signal_callback_count_cb, &data[3]);

   edje_object_signal_callback_del_full(obj, "some_signal", "event", _signal_callback_count_cb, &data[0]);
   edje_object_signal_callback_del_full(obj, "some_signal", "event", _signal_callback_count_cb, &data[3]);

   edje_object_signal_emit(obj, "some_signal", "event");

   edje_object_message_signal_process(obj);
   ck_assert_int_eq(_signal_count, (data[1] + data[2]));

   efl_del(obj);

   EDJE_TEST_FREE_EVAS();
}
END_TEST

void edje_test_edje(TCase *tc)
{
   tcase_add_test(tc, edje_test_edje_init);
   tcase_add_test(tc, edje_test_load_simple_layout);
   tcase_add_test(tc, edje_test_edje_load);
   tcase_add_test(tc, edje_test_simple_layout_geometry);
   tcase_add_test(tc, edje_test_complex_layout);
   tcase_add_test(tc, edje_test_calculate_parens);
   tcase_add_test(tc, edje_test_masking);
   tcase_add_test(tc, edje_test_filters);
   tcase_add_test(tc, edje_test_snapshot);
   tcase_add_test(tc, edje_test_size_class);
   tcase_add_test(tc, edje_test_color_class);
   tcase_add_test(tc, edje_test_swallows);
   tcase_add_test(tc, edje_test_swallows_eoapi);
   tcase_add_test(tc, edje_test_access);
   tcase_add_test(tc, edje_test_box);
   tcase_add_test(tc, edje_test_box_eoapi);
   tcase_add_test(tc, edje_test_table);
   tcase_add_test(tc, edje_test_table_eoapi);
   tcase_add_test(tc, edje_test_combine_keywords);
   tcase_add_test(tc, edje_test_message_send_legacy);
   tcase_add_test(tc, edje_test_message_send_eo);
   tcase_add_test(tc, edje_test_signals);
   tcase_add_test(tc, edje_test_signal_callback_del_full);
}
