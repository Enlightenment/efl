#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <unistd.h>
#include <stdio.h>

#define EFL_GFX_FILTER_BETA
#define EFL_CANVAS_LAYOUT_BETA

#include "edje_suite.h"

#define EVAS_DATA_DIR TESTS_SRC_DIR "/../../lib/evas"



EFL_START_TEST(edje_test_box)
{
   Evas *evas;
   Evas_Object *obj, *sobj, *sobjs[5];
   const Evas_Object *box;
   Eina_Iterator *it;
   int i;

   evas = _setup_evas();

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

   edje_object_freeze(obj);
   box = edje_object_part_object_get(obj, "box");
   edje_object_thaw(obj);
   it = evas_object_box_iterator_new(box);

   i = 0;
   EINA_ITERATOR_FOREACH(it, sobj)
     {
        fail_if(sobj != sobjs[i++]);
     }
   eina_iterator_free(it);

   evas_free(evas);
}
EFL_END_TEST

EFL_START_TEST(edje_test_box_eoapi)
{
   Evas *evas;
   Evas_Object *obj, *sobj, *sobjs[5];
   Eina_Iterator *it;
   int i;

   evas = _setup_evas();

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

   evas_free(evas);
}
EFL_END_TEST

EFL_START_TEST(edje_test_table)
{
   Evas *evas;
   Evas_Object *obj, *sobj, *sobjs[4];
   int i, k, l, cols, rows;

   evas = _setup_evas();

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

   evas_free(evas);
}
EFL_END_TEST

EFL_START_TEST(edje_test_table_eoapi)
{
   Evas *evas;
   Evas_Object *obj, *sobj, *sobjs[4], *proxy;
   Eina_Iterator *it;
   int i, k, l, cs, rs, cols, rows;

   evas = _setup_evas();

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
          sobj = efl_pack_table_content_get(efl_part(obj, "table"), k, l);
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
          efl_pack_table(efl_part(obj, "table"), sobjs[i], k, l + 2, 1, 1);
       }

   fail_if(efl_content_count(efl_part(obj, "table")) != 8);

   i = 0;
   it = efl_content_iterate(efl_part(obj, "table"));
   EINA_ITERATOR_FOREACH(it, sobj)
     {
        efl_pack_table_position_get(efl_part(obj, "table"), sobj, &k, &l, &cs, &rs);
        fail_if(cs != 1);
        fail_if(rs != 1);
        if (l >= 2)
          fail_if(sobj != sobjs[(l - 2)*2 + k]);
        i++;
     }
   eina_iterator_free(it);
   fail_if(i != 8);

   /* table size and clear */
   efl_pack_table_size_get(efl_part(obj, "table"), &cols, &rows);
   fail_if(cols != 2);
   fail_if(rows != 4);

   efl_pack_clear(efl_part(obj, "table"));
   fail_if(efl_content_count(efl_part(obj, "table")) != 4);

   efl_pack_table_size_get(efl_part(obj, "table"), &cols, &rows);
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

   evas_free(evas);
}
EFL_END_TEST


void edje_test_container(TCase *tc)
{
   tcase_add_test(tc, edje_test_box);
   tcase_add_test(tc, edje_test_box_eoapi);
   tcase_add_test(tc, edje_test_table);
   tcase_add_test(tc, edje_test_table_eoapi);
}
