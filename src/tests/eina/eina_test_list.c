/* EINA - EFL data type library
 * Copyright (C) 2008 Cedric Bail
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library;
 * if not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>

#include <Eina.h>

#include "eina_suite.h"

static Eina_Bool eina_list_sorted_check(const Eina_List *list)
{
   const Eina_List *n;
   void *d;
   int last = *(int *)list->data;

   EINA_LIST_FOREACH(list->next, n, d)
   {
      int current = *(int *)d;
      if (last > current)
        {
           fprintf(stderr, "list is not sorted: last=%d, current=%d\n",
                   last, current);
           return 0;
        }

      last = current;
   }

   return 1;
}

static int eina_int_cmp(const void *a, const void *b)
{
   const int *ia = a;
   const int *ib = b;

   return *ia - *ib;
}

EFL_START_TEST(eina_test_list_simple)
{
   Eina_List *list = NULL;
   Eina_List *tmp;
   int *test1;
   int *test2;
   int *test3;
   int data[] = { 6, 9, 42, 1, 7, 9, 81, 1664, 1337 };
   int result[] = { 81, 9, 9, 7, 1 };
   int i;


   list = eina_list_append(list, &data[0]);
        fail_if(list == NULL);

   list = eina_list_prepend(list, &data[1]);
        fail_if(list == NULL);

   list = eina_list_append(list, &data[2]);
        fail_if(list == NULL);

   i = eina_list_data_idx(list, &data[1]);
   fail_if(i != 0);

   i = eina_list_data_idx(list, &data[2]);
   fail_if(i != 2);

   i = eina_list_data_idx(list, &data[3]);
   fail_if(i != -1);

   list = eina_list_demote_list(list, eina_list_nth_list(list, 1));
   test1 = eina_list_nth(list, 2);
   fail_if(test1 == NULL);
   fail_if(*test1 != 6);

   list = eina_list_remove(list, &data[0]);
        fail_if(list == NULL);

   list = eina_list_remove(list, &data[0]);
        fail_if(list == NULL);

   tmp = eina_list_data_find_list(list, &data[2]);
        fail_if(tmp == NULL);

   list = eina_list_append_relative_list(list, &data[3], tmp);
        fail_if(list == NULL);

   list = eina_list_prepend_relative_list(list, &data[4], tmp);
        fail_if(list == NULL);

   list = eina_list_promote_list(list, tmp);
        fail_if(list == NULL);

   list = eina_list_append_relative(list, &data[5], &data[2]);
        fail_if(list == NULL);

   list = eina_list_prepend_relative(list, &data[6], &data[2]);
        fail_if(list == NULL);

   list = eina_list_remove_list(list, tmp);
        fail_if(list == NULL);

        fail_if(eina_list_data_find_list(list, &data[2]) != NULL);
        fail_if(eina_list_data_find(list, &data[2]) != NULL);
        fail_if(eina_list_data_find(list, &data[5]) != &data[5]);

        fail_if(eina_list_count(list) != 5);
        fail_if(eina_list_nth(list, 4) != &data[3]);
        fail_if(eina_list_nth(list, 10) != NULL);
        fail_if(eina_list_nth_list(list, 10) != NULL);

   for (tmp = list, i = 0; tmp != NULL; tmp = eina_list_next(tmp), ++i)
     {
        int *d = eina_list_data_get(tmp);
        fail_if(d == NULL);
        fail_if(*d != result[i]);
     }

   list = eina_list_reverse(list);

   for (tmp = list; tmp != NULL; tmp = eina_list_next(tmp), --i)
     {
        int *d = eina_list_data_get(tmp);
        fail_if(d == NULL);
        fail_if(*d != result[i - 1]);
     }

   list = eina_list_append_relative(list, &data[7], &data[7]);
        fail_if(list == NULL);

   list = eina_list_prepend_relative(list, &data[8], &data[8]);
        fail_if(list == NULL);

   list = eina_list_sort(list, 2, eina_int_cmp);

   list = eina_list_sort(list, 2, eina_int_cmp);

   test1 = eina_list_nth(list, 0);
   test2 = eina_list_nth(list, 1);
   test3 = eina_list_nth(list, 2);

        fail_if(test1 == NULL || test2 == NULL || test3 == NULL);
        fail_if(*test1 > *test2);
        fail_if(*test3 == *test2);

   list = eina_list_sort(list, 5, eina_int_cmp);

   test1 = eina_list_nth(list, 3);
   test2 = eina_list_nth(list, 4);
   test3 = eina_list_nth(list, 5);

        fail_if(test1 == NULL || test2 == NULL || test3 == NULL);
        fail_if(*test1 > *test2);
        fail_if(*test3 > *test2);

   list = eina_list_append(list, &data[8]);
        fail_if(list == NULL);

   list = eina_list_append(list, &data[7]);
        fail_if(list == NULL);

   list = eina_list_sort(list, -1, eina_int_cmp);

   test1 = eina_list_nth(list, 0);
   for (tmp = list; tmp != NULL; tmp = eina_list_next(tmp))
     {
        int *d = eina_list_data_get(tmp);
        fail_if(*test1 > *d);

        test1 = d;
     }

   test3 = eina_list_nth(list, 5);
        fail_if(test3 == NULL);

   list = eina_list_promote_list(list, list);
        fail_if(list == NULL);

   list = eina_list_promote_list(list, eina_list_last(list));
        fail_if(list == NULL);

   test1 = eina_list_nth(list, 0);
   test2 = eina_list_nth(list, 1);

   list = eina_list_promote_list(eina_list_next(list), list);
        fail_if(list == NULL);
        fail_if(eina_list_data_get(list) != test1);
        fail_if(eina_list_data_get(eina_list_next(list)) != test2);

   list = eina_list_remove_list(list, list);
        fail_if(list == NULL);

   list = eina_list_remove_list(list, eina_list_last(list));
        fail_if(list == NULL);

   list = eina_list_free(list);
        fail_if(list != NULL);

}
EFL_END_TEST

EFL_START_TEST(eina_test_list_merge)
{
   Eina_List *l1;
   Eina_List *l2;
   Eina_List *l3;
   Eina_List *l4;
   Eina_List *l5;
   int data[] = { 6, 9, 42, 1, 7, 9, 81, 1664, 1337, 3, 21, 10, 0, 5, 2008 };
   int i;


   l1 = eina_list_append(NULL, &data[0]);
   l1 = eina_list_append(l1, &data[1]);
   l1 = eina_list_append(l1, &data[2]);
   l1 = eina_list_append(l1, &data[3]);
      fail_if(l1 == NULL);

   l2 = eina_list_append(NULL, &data[4]);
   l2 = eina_list_append(l2, &data[5]);
      fail_if(l2 == NULL);

   l1 = eina_list_merge(l1, l2);
      fail_if(l1 == NULL);
      fail_if(eina_list_count(l1) != 6);
   for (i = 0, l2 = l1; ((l2 != NULL) && (i < 6)); ++i, l2 = l2->next)
      fail_if(l2->data != &data[i]);
      fail_if(i != 6);
      fail_if(l2 != NULL);

      eina_list_free(l1);

   l1 = eina_list_append(NULL, &data[0]);
   l1 = eina_list_append(l1, &data[1]);
      fail_if(l1 == NULL);

   l2 = eina_list_append(NULL, &data[2]);
   l2 = eina_list_append(l2, &data[3]);
   l2 = eina_list_append(l2, &data[4]);
   l2 = eina_list_append(l2, &data[5]);
      fail_if(l2 == NULL);

   l1 = eina_list_merge(l1, l2);
      fail_if(l1 == NULL);
      fail_if(eina_list_count(l1) != 6);
   for (i = 0, l2 = l1; ((l2 != NULL) && (i < 6)); ++i, l2 = l2->next)
      fail_if(l2->data != &data[i]);
      fail_if(i != 6);
      fail_if(l2 != NULL);

   l3 = eina_list_append(NULL, &data[6]);
   l3 = eina_list_append(l3, &data[7]);
   l3 = eina_list_append(l3, &data[8]);

   l4 = eina_list_append(NULL, &data[9]);
   l4 = eina_list_append(l4, &data[10]);
   l4 = eina_list_append(l4, &data[11]);

   l5 = eina_list_append(NULL, &data[12]);
   l5 = eina_list_append(l5, &data[13]);
   l5 = eina_list_append(l5, &data[14]);

   l1 = eina_list_sort(l1, -1, eina_int_cmp);
   l3 = eina_list_sort(l3, -1, eina_int_cmp);
   l4 = eina_list_sort(l4, -1, eina_int_cmp);
   l5 = eina_list_sort(l5, -1, eina_int_cmp);

   l1 = eina_list_sorted_merge(l1, l3, eina_int_cmp);
      fail_if(l1 == NULL);
      fail_if(eina_list_count(l1) != 9);

   l1 = eina_list_sorted_merge(l1, l4, eina_int_cmp);
      fail_if(l1 == NULL);
      fail_if(eina_list_count(l1) != 12);

   l1 = eina_list_sorted_merge(l1, l5, eina_int_cmp);
      fail_if(l1 == NULL);
      fail_if(eina_list_count(l1) != 15);

      fail_if(!eina_list_sorted_check(l1));

}
EFL_END_TEST

EFL_START_TEST(eina_test_list_sorted_insert)
{
   const int data[] = {6, 9, 42, 1, 7, 9, 81, 1664, 1337, 3, 21, 10, 0, 5, 2008};
   const int data2[] = {5, 0, 3, 2, 1, 0, 1, 2, 3, 4, 5};
   int i, count;
   Eina_List *l1, *l2, *itr;
   void *d;
   int *res, val = 2009;


   count = sizeof(data) / sizeof(data[0]);

   l1 = NULL;
   for (i = 0; i < count; i++)
      l1 = eina_list_sorted_insert(l1, eina_int_cmp, data + i);

   fail_if(l1 == NULL);
   fail_if(!eina_list_sorted_check(l1));

   res = eina_list_search_sorted(l1, eina_int_cmp, &data[7]);
   fail_if(*res != 1664);

   res = eina_list_search_sorted(l1, eina_int_cmp, &val);
   fail_if(res != NULL);

   l2 = NULL;
   EINA_LIST_FOREACH(l1, itr, d)
   l2 = eina_list_sorted_insert(l2, eina_int_cmp, d);

   fail_if(l2 == NULL);
   fail_if(!eina_list_sorted_check(l2));
   eina_list_free(l2);

   l2 = NULL;
   EINA_LIST_REVERSE_FOREACH(l1, itr, d)
   l2 = eina_list_sorted_insert(l2, eina_int_cmp, d);

   fail_if(l2 == NULL);
   fail_if(!eina_list_sorted_check(l2));
   eina_list_free(l2);
   eina_list_free(l1);

   count = sizeof(data2) / sizeof(data2[0]);
   l1 = NULL;
   for (i = 0; i < count; i++)
      l1 = eina_list_sorted_insert(l1, eina_int_cmp, data2 + i);

   fail_if(l1 == NULL);
   fail_if(!eina_list_sorted_check(l1));
   eina_list_free(l1);

}
EFL_END_TEST

EFL_START_TEST(eina_test_list_split)
{
   Eina_List *left = NULL, *right = NULL ;
   Eina_List *list = NULL;
   unsigned int i;


   list = eina_list_append(list, "tigh");
   list = eina_list_append(list, "adar");
   list = eina_list_append(list, "baltar");
   list = eina_list_append(list, "roslin");
   list = eina_list_append(list, "baltar");
   list = eina_list_append(list, "roslin");
   list = eina_list_append(list, "baltar");
   list = eina_list_append(list, "roslin");

   fail_if(list == NULL);
   fail_if(eina_list_count(list) != 8);

   for ( i = 0; i <  200; i++)
     {
        left = eina_list_split_list(list, eina_list_nth_list(list, i % 2), &right);

        if (i % 2 == 0)
          fail_if(eina_list_count(left) == 1 && eina_list_count(right) + eina_list_count(left) == i + 7);
        else
          fail_if(eina_list_count(left) == 2 && eina_list_count(right) + eina_list_count(left) == i + 7);

        list = eina_list_merge(left, right);
        list = eina_list_append(list, "roslin");
     }

}
EFL_END_TEST

#define SHUFFLE_SZ 100
#define SHUFFLE_N 5
EFL_START_TEST(eina_test_list_shuffle)
{
   unsigned int *p;
   unsigned int i;
   unsigned int n[SHUFFLE_SZ];
   Eina_List *list = NULL;
   Eina_List *item = NULL;
   Eina_List *copy, *cl;


   for(i = 0; i < SHUFFLE_SZ; i++)
     {
        n[i] = i;
        list = eina_list_append(list, &n[i]);
     }
   copy = eina_list_clone(list);

   for(i = 0; i < SHUFFLE_N; i++)
     {
        list = eina_list_shuffle(list, NULL);
        cl = eina_list_data_get(copy);
        EINA_LIST_FOREACH(list, item, p)
          {
             if (eina_list_data_get(cl) != p) break;
             cl = eina_list_next(cl);
          }
        ck_assert_ptr_ne(item, NULL);
     }
}
EFL_END_TEST

#define DATA_SIZE 100
EFL_START_TEST(eina_test_list_clone)
{
   unsigned int i;
   unsigned int *d, *rd;
   unsigned int n[DATA_SIZE];
   Eina_List *list = NULL;
   Eina_List *clist = NULL;
   Eina_List *rclist = NULL;


   for(i = 0; i < DATA_SIZE; i++)
     {
        n[i] = i;
        list = eina_list_append(list, &n[i]);
     }

   clist = eina_list_clone(list);
   fail_if(clist == NULL);

   for(i = 0; i < DATA_SIZE; i++)
     {
        fail_if(eina_list_nth(list, i) != eina_list_nth(clist, i));
     }

   rclist = eina_list_reverse_clone(list);
   fail_if(rclist == NULL);

   for(i = 0; i < DATA_SIZE; i++)
     {
        d = eina_list_nth(list, i);
        rd = eina_list_nth(rclist, (DATA_SIZE - 1 - i));
        fail_if(d != rd);
     }

   list = eina_list_free(list);
   fail_if(list != NULL);

   clist = eina_list_free(clist);
   fail_if(clist != NULL);

   rclist = eina_list_free(rclist);
   fail_if(rclist != NULL);

}
EFL_END_TEST

EFL_START_TEST(eina_test_list_move)
{
   Eina_List *list1 = NULL, *list2 = NULL;
   Eina_Bool ret;
   int data1[] = {1, 2, 3, 4, 5};
   int data2[] = {6, 7, 8, 9, 10};
   int i, *list_data;


   for (i = 0; i < 5; i++)
   {
      list1 = eina_list_append(list1, &data1[i]);
      list2 = eina_list_append(list2, &data2[i]);
   }
   fail_if(eina_list_count(list1) != 5);
   fail_if(eina_list_count(list2) != 5);

   ret = eina_list_move(&list1, &list2, &data2[4]);
   fail_if(ret != EINA_TRUE);
   fail_if(eina_list_count(list1) != 6);
   fail_if(eina_list_count(list2) != 4);
   list_data = eina_list_nth(list1, 5);
   fail_if(*list_data != 10);

   ret = eina_list_move_list(&list1, &list2,
                             eina_list_nth_list(list2, 1));
   fail_if(ret != EINA_TRUE);
   fail_if(eina_list_count(list1) != 7);
   fail_if(eina_list_count(list2) != 3);
   list_data = eina_list_nth(list1, 6);
   fail_if(*list_data != 7);

   eina_list_free(list1);
   eina_list_free(list2);

}
EFL_END_TEST

void
eina_test_list(TCase *tc)
{
   tcase_add_test(tc, eina_test_list_simple);
   tcase_add_test(tc, eina_test_list_merge);
   tcase_add_test(tc, eina_test_list_sorted_insert);
   tcase_add_test(tc, eina_test_list_split);
   tcase_add_test(tc, eina_test_list_shuffle);
   tcase_add_test(tc, eina_test_list_clone);
   tcase_add_test(tc, eina_test_list_move);
}
