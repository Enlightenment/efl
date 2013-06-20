/* EINA - EFL data type library
 * Copyright (C) 2012 ProFUSION embedded systems
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
#include <inttypes.h>

#include "eina_suite.h"
#include "Eina.h"

static void
_eina_test_model_check_safety_null(const Eina_Log_Domain *d, Eina_Log_Level level, const char *file, const char *fnc, int line, const char *fmt, void *data, va_list args)
{
   Eina_Bool *ck = data;

   if ((level == EINA_LOG_LEVEL_ERR) && (strcmp(fmt, "%s") == 0))
     {
        const char *str;
        va_list cp_args;

        va_copy(cp_args, args);
        str = va_arg(cp_args, const char *);
        va_end(cp_args);
        if (eina_str_has_prefix(str, "safety check failed: ") &&
            eina_str_has_suffix(str, " == NULL"))
          {
             *ck = EINA_TRUE;
             return;
          }
     }
   *ck = EINA_FALSE;
   eina_log_print_cb_stderr(d, level, file, fnc, line, fmt, NULL, args);
}

static void
_eina_test_model_check_safety_false(const Eina_Log_Domain *d, Eina_Log_Level level, const char *file, const char *fnc, int line, const char *fmt, void *data, va_list args)
{
   Eina_Bool *ck = data;

   if ((level == EINA_LOG_LEVEL_ERR) && (strcmp(fmt, "%s") == 0))
     {
        const char *str;
        va_list cp_args;

        va_copy(cp_args, args);
        str = va_arg(cp_args, const char *);
        va_end(cp_args);
        if (eina_str_has_prefix(str, "safety check failed: ") &&
            eina_str_has_suffix(str, " is false"))
          {
             *ck = EINA_TRUE;
             return;
          }
     }
   *ck = EINA_FALSE;
   eina_log_print_cb_stderr(d, level, file, fnc, line, fmt, NULL, args);
}

static void
_eina_test_model_cb_count(void *data, Eina_Model *model, const Eina_Model_Event_Description *desc, void *event_info)
{
   unsigned *count = data;
   (*count)++;
#if SHOW_LOG
   if ((desc->type) && (strcmp(desc->type, "u") == 0))
     {
        unsigned *pos = event_info;
        printf("%2u %p %s at %u\n", *count, model, desc->name, *pos);
     }
   else
     printf("%2u %p %s\n", *count, model, desc->name);
#else
   (void)model;
   (void)desc;
   (void)event_info;
#endif
}

START_TEST(eina_model_test_properties)
{
   unsigned int count_del = 0, count_pset = 0, count_pdel = 0;
   Eina_Model *m;
   Eina_Value inv, outv;
   int i;
   char *s;
   Eina_List *lst;
   Eina_Bool ck;

   eina_init();

   m = eina_model_new(EINA_MODEL_TYPE_GENERIC);
   fail_unless(m != NULL);

   eina_model_event_callback_add
     (m, "deleted", _eina_test_model_cb_count, &count_del);
   eina_model_event_callback_add
     (m, "property,set", _eina_test_model_cb_count, &count_pset);
   eina_model_event_callback_add
     (m, "property,deleted", _eina_test_model_cb_count, &count_pdel);

   fail_unless(eina_value_setup(&inv, EINA_VALUE_TYPE_INT));
   fail_unless(eina_value_set(&inv, 1234));
   fail_unless(eina_value_get(&inv, &i));
   ck_assert_int_eq(i, 1234);

   fail_unless(eina_model_property_set(m, "abc", &inv));

   fail_unless(eina_value_set(&inv, 5678));
   fail_unless(eina_model_property_set(m, "xyz", &inv));

   fail_unless(eina_value_set(&inv, 171));
   fail_unless(eina_model_property_set(m, "value", &inv));

   lst = eina_model_properties_names_list_get(m);
   ck_assert_int_eq(eina_list_count(lst), 3);

   lst = eina_list_sort(lst, 0, EINA_COMPARE_CB(strcmp));
   ck_assert_str_eq("abc", eina_list_nth(lst, 0));
   ck_assert_str_eq("value", eina_list_nth(lst, 1));
   ck_assert_str_eq("xyz", eina_list_nth(lst, 2));

   eina_model_properties_names_list_free(lst);

   fail_unless(eina_model_property_get(m, "abc", &outv));
   fail_unless(eina_value_get(&outv, &i));
   ck_assert_int_eq(i, 1234);
   eina_value_flush(&outv);

   fail_unless(eina_model_property_get(m, "xyz", &outv));
   fail_unless(eina_value_get(&outv, &i));
   ck_assert_int_eq(i, 5678);
   eina_value_flush(&outv);

   fail_unless(eina_model_property_get(m, "value", &outv));
   fail_unless(eina_value_get(&outv, &i));
   ck_assert_int_eq(i, 171);
   eina_value_flush(&outv);

   fail_unless(eina_value_set(&inv, 666));
   fail_unless(eina_model_property_set(m, "value", &inv));
   fail_unless(eina_model_property_get(m, "value", &outv));
   fail_unless(eina_value_get(&outv, &i));
   ck_assert_int_eq(i, 666);

   eina_value_flush(&outv);
   eina_value_flush(&inv);

   fail_unless(eina_value_setup(&inv, EINA_VALUE_TYPE_STRING));
   fail_unless(eina_value_set(&inv, "Hello world!"));
   fail_unless(eina_model_property_set(m, "string", &inv));

   fail_unless(eina_model_property_get(m, "string", &outv));
   fail_unless(eina_value_get(&outv, &s));
   fail_unless(s != NULL);
   ck_assert_str_eq(s, "Hello world!");

   eina_value_flush(&outv);
   eina_value_flush(&inv);

   fail_unless(eina_value_setup(&inv, EINA_VALUE_TYPE_STRINGSHARE));
   fail_unless(eina_value_set(&inv, "Hello world-STRINGSHARED!"));
   fail_unless(eina_model_property_set(m, "stringshare", &inv));
   /* set twice to see if references drop to zero before new add, shouldn't */
   fail_unless(eina_model_property_set(m, "stringshare", &inv));

   fail_unless(eina_model_property_get(m, "stringshare", &outv));
   fail_unless(eina_value_get(&outv, &s));
   fail_unless(s != NULL);
   ck_assert_str_eq(s, "Hello world-STRINGSHARED!");

   eina_value_flush(&outv);
   eina_value_flush(&inv);

   s = eina_model_to_string(m);
   fail_unless(s != NULL);
   ck_assert_str_eq(s, "Eina_Model_Type_Generic({abc: 1234, string: Hello world!, stringshare: Hello world-STRINGSHARED!, value: 666, xyz: 5678}, [])");
   free(s);

   fail_unless(eina_model_property_del(m, "value"));

   /* negative test (check safety was displayed by using print_cb) */
   eina_log_print_cb_set(_eina_test_model_check_safety_null, &ck);

   ck = EINA_FALSE;
   fail_if(eina_model_property_get(m, "non-existent", &outv));
   fail_unless(ck == EINA_TRUE);

   ck = EINA_FALSE;
   fail_if(eina_model_property_get(m, NULL, &outv));
   fail_unless(ck == EINA_TRUE);

   ck = EINA_FALSE;
   fail_if(eina_model_property_del(m, "value"));
   fail_unless(ck == EINA_TRUE);

   /* revert print_cb to default */
   eina_log_print_cb_set(eina_log_print_cb_stderr, NULL);

   ck_assert_int_eq(eina_model_refcount(m), 1);

   eina_model_unref(m);
   ck_assert_int_eq(count_del, 1);
   ck_assert_int_eq(count_pset, 7);
   ck_assert_int_eq(count_pdel, 1);
   eina_shutdown();
}
END_TEST

static int
eina_model_test_children_reverse_cmp(const Eina_Model *a, const Eina_Model *b)
{
   return - eina_model_compare(a, b);
}

START_TEST(eina_model_test_children)
{
   unsigned int count_del = 0, count_cset = 0, count_cins = 0, count_cdel = 0;
   Eina_Model *m, *c;
   char *s;
   int i;

   eina_init();

   m = eina_model_new(EINA_MODEL_TYPE_GENERIC);
   fail_unless(m != NULL);

   eina_model_event_callback_add
     (m, "deleted", _eina_test_model_cb_count, &count_del);
   eina_model_event_callback_add
     (m, "child,set", _eina_test_model_cb_count, &count_cset);
   eina_model_event_callback_add
     (m, "child,inserted", _eina_test_model_cb_count, &count_cins);
   eina_model_event_callback_add
     (m, "child,deleted", _eina_test_model_cb_count, &count_cdel);

   for (i = 0; i < 10; i++)
     {
        Eina_Value val;

        c = eina_model_new(EINA_MODEL_TYPE_GENERIC);
        fail_unless(c != NULL);

        eina_model_event_callback_add
          (c, "deleted", _eina_test_model_cb_count, &count_del);
        eina_model_event_callback_add
          (c, "child,set", _eina_test_model_cb_count, &count_cset);
        eina_model_event_callback_add
          (c, "child,inserted", _eina_test_model_cb_count, &count_cins);
        eina_model_event_callback_add
          (c, "child,deleted", _eina_test_model_cb_count, &count_cdel);

        fail_unless(eina_value_setup(&val, EINA_VALUE_TYPE_INT));
        fail_unless(eina_value_set(&val, i));
        fail_unless(eina_model_property_set(c, "value", &val));

        fail_unless(eina_model_child_append(m, c) >= 0);
        ck_assert_int_eq(eina_model_refcount(c), 2);

        eina_value_flush(&val);
        eina_model_unref(c);
     }

   ck_assert_int_eq(eina_model_child_count(m), 10);

   for (i = 0; i < 10; i++)
     {
        Eina_Value val;
        int x;

        c = eina_model_child_get(m, i);
        fail_unless(c != NULL);
        ck_assert_int_eq(eina_model_refcount(c), 2);

        fail_unless(eina_model_property_get(c, "value", &val));
        fail_unless(eina_value_get(&val, &x));
        ck_assert_int_eq(x, i);

        eina_value_flush(&val);
        eina_model_unref(c);
     }

   eina_model_child_sort(m, EINA_COMPARE_CB(eina_model_test_children_reverse_cmp));

   for (i = 0; i < 10; i++)
     {
        Eina_Value val;
        int x;

        c = eina_model_child_get(m, i);
        fail_unless(c != NULL);
        ck_assert_int_eq(eina_model_refcount(c), 2);

        fail_unless(eina_model_property_get(c, "value", &val));
        fail_unless(eina_value_get(&val, &x));
        ck_assert_int_eq(x, 10 - i - 1);

        eina_value_flush(&val);
        eina_model_unref(c);
     }

   eina_model_child_sort(m, EINA_COMPARE_CB(eina_model_compare));

   s = eina_model_to_string(m);
   fail_unless(s != NULL);
   ck_assert_str_eq(s, "Eina_Model_Type_Generic({}, [Eina_Model_Type_Generic({value: 0}, []), Eina_Model_Type_Generic({value: 1}, []), Eina_Model_Type_Generic({value: 2}, []), Eina_Model_Type_Generic({value: 3}, []), Eina_Model_Type_Generic({value: 4}, []), Eina_Model_Type_Generic({value: 5}, []), Eina_Model_Type_Generic({value: 6}, []), Eina_Model_Type_Generic({value: 7}, []), Eina_Model_Type_Generic({value: 8}, []), Eina_Model_Type_Generic({value: 9}, [])])");
   free(s);

   c = eina_model_child_get(m, 0);
   eina_model_child_set(m, 1, c);
   eina_model_unref(c);

   eina_model_child_del(m, 0);
   eina_model_child_del(m, 8);

   s = eina_model_to_string(m);
   fail_unless(s != NULL);
   ck_assert_str_eq(s, "Eina_Model_Type_Generic({}, [Eina_Model_Type_Generic({value: 0}, []), Eina_Model_Type_Generic({value: 2}, []), Eina_Model_Type_Generic({value: 3}, []), Eina_Model_Type_Generic({value: 4}, []), Eina_Model_Type_Generic({value: 5}, []), Eina_Model_Type_Generic({value: 6}, []), Eina_Model_Type_Generic({value: 7}, []), Eina_Model_Type_Generic({value: 8}, [])])");
   free(s);

   ck_assert_int_eq(eina_model_refcount(m), 1);
   eina_model_unref(m);

   ck_assert_int_eq(count_del, 11);
   ck_assert_int_eq(count_cins, 10);
   ck_assert_int_eq(count_cset, 1);
   ck_assert_int_eq(count_cdel, 2);

   eina_shutdown();
}
END_TEST

START_TEST(eina_model_test_copy)
{
   unsigned int count_del = 0;
   Eina_Model *m, *cp;
   char *s1, *s2;
   int i;

   eina_init();

   m = eina_model_new(EINA_MODEL_TYPE_GENERIC);
   fail_unless(m != NULL);

   eina_model_event_callback_add
     (m, "deleted", _eina_test_model_cb_count, &count_del);

   for (i = 0; i < 5; i++)
     {
        Eina_Value val;
        char name[2] = {'a'+ i, 0};
        fail_unless(eina_value_setup(&val, EINA_VALUE_TYPE_INT));
        fail_unless(eina_value_set(&val, i));
        fail_unless(eina_model_property_set(m, name, &val));
        eina_value_flush(&val);
     }

   for (i = 0; i < 5; i++)
     {
        Eina_Value val;
        Eina_Model *c = eina_model_new(EINA_MODEL_TYPE_GENERIC);
        fail_unless(c != NULL);
        fail_unless(eina_value_setup(&val, EINA_VALUE_TYPE_INT));
        fail_unless(eina_value_set(&val, i));
        fail_unless(eina_model_property_set(c, "x", &val));

        eina_model_event_callback_add
          (c, "deleted", _eina_test_model_cb_count, &count_del);

        fail_unless(eina_model_child_append(m, c) >= 0);
        eina_model_unref(c);
        eina_value_flush(&val);
     }

   s1 = eina_model_to_string(m);
   fail_unless(s1 != NULL);
   ck_assert_str_eq(s1, "Eina_Model_Type_Generic({a: 0, b: 1, c: 2, d: 3, e: 4}, [Eina_Model_Type_Generic({x: 0}, []), Eina_Model_Type_Generic({x: 1}, []), Eina_Model_Type_Generic({x: 2}, []), Eina_Model_Type_Generic({x: 3}, []), Eina_Model_Type_Generic({x: 4}, [])])");

   cp = eina_model_copy(m);
   fail_unless(cp != NULL);
   fail_unless(cp != m);

   eina_model_event_callback_add
     (cp, "deleted", _eina_test_model_cb_count, &count_del);

   s2 = eina_model_to_string(cp);
   fail_unless(s2 != NULL);
   ck_assert_str_eq(s1, s2);

   for (i = 0; i < 5; i++)
     {
        Eina_Model *c1 = eina_model_child_get(m, i);
        Eina_Model *c2 = eina_model_child_get(cp, i);

        fail_unless(c1 != NULL);
        fail_unless(c1 == c2);
        ck_assert_int_eq(eina_model_refcount(c1), 4);

        eina_model_unref(c1);
        eina_model_unref(c2);
     }

   free(s1);
   free(s2);

   ck_assert_int_eq(eina_model_refcount(m), 1);
   eina_model_unref(m);

   ck_assert_int_eq(eina_model_refcount(cp), 1);
   eina_model_unref(cp);

   ck_assert_int_eq(count_del, 2 + 5);

   eina_shutdown();
}
END_TEST

START_TEST(eina_model_test_deep_copy)
{
   unsigned int count_del = 0;
   Eina_Model *m, *cp;
   char *s1, *s2;
   int i;

   eina_init();

   m = eina_model_new(EINA_MODEL_TYPE_GENERIC);
   fail_unless(m != NULL);

   eina_model_event_callback_add
     (m, "deleted", _eina_test_model_cb_count, &count_del);

   for (i = 0; i < 5; i++)
     {
        Eina_Value val;
        char name[2] = {'a'+ i, 0};
        fail_unless(eina_value_setup(&val, EINA_VALUE_TYPE_INT));
        fail_unless(eina_value_set(&val, i));
        fail_unless(eina_model_property_set(m, name, &val));
        eina_value_flush(&val);
     }

   for (i = 0; i < 5; i++)
     {
        Eina_Value val;
        Eina_Model *c = eina_model_new(EINA_MODEL_TYPE_GENERIC);
        fail_unless(c != NULL);
        fail_unless(eina_value_setup(&val, EINA_VALUE_TYPE_INT));
        fail_unless(eina_value_set(&val, i));
        fail_unless(eina_model_property_set(c, "x", &val));

        eina_model_event_callback_add
          (c, "deleted", _eina_test_model_cb_count, &count_del);

        fail_unless(eina_model_child_append(m, c) >= 0);
        eina_model_unref(c);
        eina_value_flush(&val);
     }

   s1 = eina_model_to_string(m);
   fail_unless(s1 != NULL);
   ck_assert_str_eq(s1, "Eina_Model_Type_Generic({a: 0, b: 1, c: 2, d: 3, e: 4}, [Eina_Model_Type_Generic({x: 0}, []), Eina_Model_Type_Generic({x: 1}, []), Eina_Model_Type_Generic({x: 2}, []), Eina_Model_Type_Generic({x: 3}, []), Eina_Model_Type_Generic({x: 4}, [])])");

   cp = eina_model_deep_copy(m);
   fail_unless(cp != NULL);
   fail_unless(cp != m);

   eina_model_event_callback_add
     (cp, "deleted", _eina_test_model_cb_count, &count_del);

   s2 = eina_model_to_string(cp);
   fail_unless(s2 != NULL);
   ck_assert_str_eq(s1, s2);

   for (i = 0; i < 5; i++)
     {
        Eina_Model *c1 = eina_model_child_get(m, i);
        Eina_Model *c2 = eina_model_child_get(cp, i);

        fail_unless(c1 != NULL);
        fail_unless(c1 != c2);
        ck_assert_int_eq(eina_model_refcount(c1), 2);
        ck_assert_int_eq(eina_model_refcount(c2), 2);

        eina_model_event_callback_add
          (c2, "deleted", _eina_test_model_cb_count, &count_del);

        eina_model_unref(c1);
        eina_model_unref(c2);
     }

   free(s1);
   free(s2);

   ck_assert_int_eq(eina_model_refcount(m), 1);
   eina_model_unref(m);

   ck_assert_int_eq(eina_model_refcount(cp), 1);
   eina_model_unref(cp);

   ck_assert_int_eq(count_del, 2 + 10);

   eina_shutdown();
}
END_TEST

static Eina_Model *
eina_model_test_iterator_setup(unsigned int *count_del)
{
   Eina_Model *m;
   int i;

   m = eina_model_new(EINA_MODEL_TYPE_GENERIC);
   fail_unless(m != NULL);

   eina_model_event_callback_add
     (m, "deleted", _eina_test_model_cb_count, count_del);

   for (i = 0; i < 5; i++)
     {
        Eina_Value val;
        Eina_Model *c = eina_model_new(EINA_MODEL_TYPE_GENERIC);
        fail_unless(c != NULL);
        fail_unless(eina_value_setup(&val, EINA_VALUE_TYPE_INT));
        fail_unless(eina_value_set(&val, i));
        fail_unless(eina_model_property_set(c, "x", &val));

        eina_model_event_callback_add
          (c, "deleted", _eina_test_model_cb_count, count_del);

        fail_unless(eina_model_child_append(m, c) >= 0);
        eina_model_unref(c);
        eina_value_flush(&val);
     }

   return m;
}

START_TEST(eina_model_test_child_iterator)
{
   unsigned int count_del = 0;
   Eina_Iterator *it;
   Eina_Model *m, *c;
   int i = 0;

   eina_init();

   m = eina_model_test_iterator_setup(&count_del);

   it = eina_model_child_iterator_get(m);
   fail_unless(it != NULL);
   EINA_ITERATOR_FOREACH(it, c)
     {
        Eina_Value tmp;
        int x;

        ck_assert_int_eq(eina_model_refcount(c), 2);
        fail_unless(eina_model_property_get(c, "x", &tmp));
        fail_unless(eina_value_get(&tmp, &x));
        ck_assert_int_eq(x, i);

        eina_model_unref(c);
        i++;
     }
   ck_assert_int_eq(i, 5);
   eina_iterator_free(it);

   ck_assert_int_eq(eina_model_refcount(m), 1);
   eina_model_unref(m);
   ck_assert_int_eq(count_del, 6);
   eina_shutdown();
}
END_TEST

START_TEST(eina_model_test_child_reversed_iterator)
{
   unsigned int count_del = 0;
   Eina_Iterator *it;
   Eina_Model *m, *c;
   int i = 4;

   eina_init();

   m = eina_model_test_iterator_setup(&count_del);

   it = eina_model_child_reversed_iterator_get(m);
   fail_unless(it != NULL);
   EINA_ITERATOR_FOREACH(it, c)
     {
        Eina_Value tmp;
        int x;

        ck_assert_int_eq(eina_model_refcount(c), 2);
        fail_unless(eina_model_property_get(c, "x", &tmp));
        fail_unless(eina_value_get(&tmp, &x));
        ck_assert_int_eq(x, i);

        eina_model_unref(c);
        i--;
     }
   ck_assert_int_eq(i, -1);
   eina_iterator_free(it);

   ck_assert_int_eq(eina_model_refcount(m), 1);
   eina_model_unref(m);
   ck_assert_int_eq(count_del, 6);
   eina_shutdown();
}
END_TEST

START_TEST(eina_model_test_child_sorted_iterator)
{
   unsigned int count_del = 0;
   Eina_Iterator *it;
   Eina_Model *m, *c;
   int i = 4;

   eina_init();

   m = eina_model_test_iterator_setup(&count_del);

   it = eina_model_child_sorted_iterator_get
     (m, EINA_COMPARE_CB(eina_model_test_children_reverse_cmp));
   fail_unless(it != NULL);
   EINA_ITERATOR_FOREACH(it, c)
     {
        Eina_Value tmp;
        int x;

        /* 3 because sort takes an extra reference for its temp array */
        ck_assert_int_eq(eina_model_refcount(c), 3);
        fail_unless(eina_model_property_get(c, "x", &tmp));
        fail_unless(eina_value_get(&tmp, &x));
        ck_assert_int_eq(x, i);

        eina_model_unref(c);
        i--;
     }
   ck_assert_int_eq(i, -1);
   eina_iterator_free(it);

   it = eina_model_child_sorted_iterator_get
     (m, EINA_COMPARE_CB(eina_model_compare));
   fail_unless(it != NULL);
   i = 0;
   EINA_ITERATOR_FOREACH(it, c)
     {
        Eina_Value tmp;
        int x;

        /* 3 because sort takes an extra reference for its temp array */
        ck_assert_int_eq(eina_model_refcount(c), 3);
        fail_unless(eina_model_property_get(c, "x", &tmp));
        fail_unless(eina_value_get(&tmp, &x));
        ck_assert_int_eq(x, i);

        eina_model_unref(c);
        i++;
     }
   ck_assert_int_eq(i, 5);
   eina_iterator_free(it);

   ck_assert_int_eq(eina_model_refcount(m), 1);
   eina_model_unref(m);
   ck_assert_int_eq(count_del, 6);
   eina_shutdown();
}
END_TEST

static Eina_Bool
eina_model_test_filter_event(const void *m, void *c, void *fdata)
{
   Eina_Value tmp;
   int x;
   fail_unless(m == fdata);
   fail_unless(eina_model_property_get(c, "x", &tmp));
   fail_unless(eina_value_get(&tmp, &x));
   eina_value_flush(&tmp);
   return x % 2 == 0;
}

START_TEST(eina_model_test_child_filtered_iterator)
{
   unsigned int count_del = 0;
   Eina_Iterator *it;
   Eina_Model *m;
   int i = 0, idx;

   eina_init();

   m = eina_model_test_iterator_setup(&count_del);

   it = eina_model_child_filtered_iterator_get
     (m, eina_model_test_filter_event, m);
   fail_unless(it != NULL);
   EINA_ITERATOR_FOREACH(it, idx)
     {
        Eina_Model *c;
        Eina_Value tmp;
        int x;

        ck_assert_int_eq(idx % 2, 0);
        ck_assert_int_eq(idx, i);

        c = eina_model_child_get(m, idx);
        fail_unless(c != NULL);
        ck_assert_int_eq(eina_model_refcount(c), 2);
        fail_unless(eina_model_property_get(c, "x", &tmp));
        fail_unless(eina_value_get(&tmp, &x));
        ck_assert_int_eq(x, i);

        eina_model_unref(c);
        i += 2;
     }
   ck_assert_int_eq(i, 6);
   eina_iterator_free(it);

   ck_assert_int_eq(eina_model_refcount(m), 1);
   eina_model_unref(m);
   ck_assert_int_eq(count_del, 6);
   eina_shutdown();
}
END_TEST

START_TEST(eina_model_test_struct)
{
   unsigned int count_del = 0, count_pset = 0, count_pdel = 0;
   Eina_Model *m;
   struct myst {
      int i;
      char c;
   };
   const Eina_Value_Struct_Member myst_members[] = {
        EINA_VALUE_STRUCT_MEMBER(EINA_VALUE_TYPE_INT, struct myst, i),
        EINA_VALUE_STRUCT_MEMBER(EINA_VALUE_TYPE_CHAR, struct myst, c)
   };
   const Eina_Value_Struct_Desc myst_desc = {
     EINA_VALUE_STRUCT_DESC_VERSION,
     NULL, myst_members, EINA_C_ARRAY_LENGTH(myst_members), sizeof(struct myst)
   };
   Eina_Value inv, outv;
   int i;
   char c, *s;
   Eina_List *lst;
   Eina_Bool ck;

   eina_init();

   m = eina_model_struct_new(&myst_desc);
   fail_unless(m != NULL);

   eina_model_event_callback_add
     (m, "deleted", _eina_test_model_cb_count, &count_del);
   eina_model_event_callback_add
     (m, "property,set", _eina_test_model_cb_count, &count_pset);
   eina_model_event_callback_add
     (m, "property,deleted", _eina_test_model_cb_count, &count_pdel);

   fail_unless(eina_value_setup(&inv, EINA_VALUE_TYPE_INT));
   fail_unless(eina_value_set(&inv, 1234));
   fail_unless(eina_value_get(&inv, &i));
   ck_assert_int_eq(i, 1234);
   fail_unless(eina_model_property_set(m, "i", &inv));

   eina_value_flush(&inv);
   fail_unless(eina_value_setup(&inv, EINA_VALUE_TYPE_CHAR));
   fail_unless(eina_value_set(&inv, 33));
   fail_unless(eina_value_get(&inv, &c));
   ck_assert_int_eq(c, 33);
   fail_unless(eina_model_property_set(m, "c", &inv));

   lst = eina_model_properties_names_list_get(m);
   ck_assert_int_eq(eina_list_count(lst), 2);

   lst = eina_list_sort(lst, 0, EINA_COMPARE_CB(strcmp));
   ck_assert_str_eq("c", eina_list_nth(lst, 0));
   ck_assert_str_eq("i", eina_list_nth(lst, 1));

   eina_model_properties_names_list_free(lst);

   fail_unless(eina_model_property_get(m, "i", &outv));
   fail_unless(outv.type == EINA_VALUE_TYPE_INT);
   fail_unless(eina_value_get(&outv, &i));
   ck_assert_int_eq(i, 1234);
   eina_value_flush(&outv);

   fail_unless(eina_model_property_get(m, "c", &outv));
   fail_unless(outv.type == EINA_VALUE_TYPE_CHAR);
   fail_unless(eina_value_get(&outv, &c));
   ck_assert_int_eq(c, 33);
   eina_value_flush(&outv);

   eina_value_flush(&inv);

   /* negative test (check safety was displayed by using print_cb) */
   eina_log_print_cb_set(_eina_test_model_check_safety_null, &ck);

   fail_if(eina_model_property_get(m, "non-existent", &outv));

   ck = EINA_FALSE;
   fail_if(eina_model_property_get(m, NULL, &outv));
   fail_unless(ck == EINA_TRUE);

   fail_unless(eina_value_setup(&inv, EINA_VALUE_TYPE_STRING));
   fail_unless(eina_value_set(&inv, "hello world"));

   eina_log_print_cb_set(_eina_test_model_check_safety_false, &ck);

   ck = EINA_FALSE;
   fail_if(eina_model_property_set(m, "i", &inv));
   fail_unless(ck == EINA_TRUE);

   ck = EINA_FALSE;
   fail_if(eina_model_property_set(m, "c", &inv));
   fail_unless(ck == EINA_TRUE);

   /* revert print_cb to default */
   eina_log_print_cb_set(eina_log_print_cb_stderr, NULL);

   fail_if(eina_model_property_del(m, "value"));
   fail_if(eina_model_property_del(m, "i"));
   fail_if(eina_model_property_del(m, "c"));

   eina_value_flush(&inv);

   s = eina_model_to_string(m);
   fail_unless(s != NULL);
   ck_assert_str_eq(s, "Eina_Model_Type_Struct({c: 33, i: 1234}, [])");
   free(s);

   ck_assert_int_eq(eina_model_refcount(m), 1);

   eina_model_unref(m);
   ck_assert_int_eq(count_del, 1);
   ck_assert_int_eq(count_pset, 2);
   ck_assert_int_eq(count_pdel, 0);
   eina_shutdown();
}
END_TEST

static Eina_Bool
_struct_complex_members_constructor(Eina_Model *m)
{
   struct myst {
      Eina_Value_Array a;
      Eina_Value_List l;
      Eina_Value_Hash h;
      Eina_Value_Struct s;
   } st;
   struct subst {
      int i, j;
   };
   static Eina_Value_Struct_Member myst_members[] = {
        EINA_VALUE_STRUCT_MEMBER(NULL, struct myst, a),
        EINA_VALUE_STRUCT_MEMBER(NULL, struct myst, l),
        EINA_VALUE_STRUCT_MEMBER(NULL, struct myst, h),
        EINA_VALUE_STRUCT_MEMBER(NULL, struct myst, s)
   };
   static Eina_Value_Struct_Desc myst_desc = {
     EINA_VALUE_STRUCT_DESC_VERSION,
     NULL, myst_members, EINA_C_ARRAY_LENGTH(myst_members), sizeof(struct myst)
   };
   static Eina_Value_Struct_Member subst_members[] = {
        EINA_VALUE_STRUCT_MEMBER(NULL, struct subst, i),
        EINA_VALUE_STRUCT_MEMBER(NULL, struct subst, j)
   };
   static Eina_Value_Struct_Desc subst_desc = {
     EINA_VALUE_STRUCT_DESC_VERSION,
     NULL, subst_members, EINA_C_ARRAY_LENGTH(subst_members),
     sizeof(struct subst)
   };

   if (!myst_members[0].type)
     {
        myst_members[0].type = EINA_VALUE_TYPE_ARRAY;
        myst_members[1].type = EINA_VALUE_TYPE_LIST;
        myst_members[2].type = EINA_VALUE_TYPE_HASH;
        myst_members[3].type = EINA_VALUE_TYPE_STRUCT;
     }

   if (!subst_members[0].type)
     {
        subst_members[0].type = EINA_VALUE_TYPE_INT;
        subst_members[1].type = EINA_VALUE_TYPE_INT;
     }

   if (!eina_model_type_constructor(EINA_MODEL_TYPE_STRUCT, m))
     return EINA_FALSE;

   memset(&st, 0, sizeof(st));

   st.a.subtype = EINA_VALUE_TYPE_STRING;
   st.l.subtype = EINA_VALUE_TYPE_STRING;
   st.h.subtype = EINA_VALUE_TYPE_STRING;
   st.s.desc = &subst_desc;
   if (!eina_model_struct_set(m, &myst_desc, &st))
     return EINA_FALSE;

   return EINA_TRUE;
}

START_TEST(eina_model_test_struct_complex_members)
{
   Eina_Model *m;
   Eina_Value outv;
   char *s;
   Eina_Model_Type type = EINA_MODEL_TYPE_INIT_NOPRIVATE
     ("struct_complex_members", Eina_Model_Type, NULL, NULL, NULL);

   eina_init();

   type.constructor = _struct_complex_members_constructor;
   type.parent = EINA_MODEL_TYPE_STRUCT;

   m = eina_model_new(&type);
   fail_unless(m != NULL);

   fail_unless(eina_model_property_get(m, "a", &outv));
   fail_unless(eina_value_array_append(&outv, "Hello"));
   fail_unless(eina_value_array_append(&outv, "World"));
   fail_unless(eina_model_property_set(m, "a", &outv));
   eina_value_flush(&outv);

   fail_unless(eina_model_property_get(m, "l", &outv));
   fail_unless(eina_value_list_append(&outv, "Some"));
   fail_unless(eina_value_list_append(&outv, "Thing"));
   fail_unless(eina_model_property_set(m, "l", &outv));
   eina_value_flush(&outv);

   fail_unless(eina_model_property_get(m, "h", &outv));
   fail_unless(eina_value_hash_set(&outv, "key", "value"));
   fail_unless(eina_model_property_set(m, "h", &outv));
   eina_value_flush(&outv);

   fail_unless(eina_model_property_get(m, "s", &outv));
   fail_unless(eina_value_struct_set(&outv, "i", 1234));
   fail_unless(eina_value_struct_set(&outv, "j", 44));
   fail_unless(eina_model_property_set(m, "s", &outv));
   eina_value_flush(&outv);

   s = eina_model_to_string(m);
   fail_unless(s != NULL);
   ck_assert_str_eq(s, "struct_complex_members({a: [Hello, World], h: {key: value}, l: [Some, Thing], s: {i: 1234, j: 44}}, [])");
   free(s);

   ck_assert_int_eq(eina_model_refcount(m), 1);

   eina_model_unref(m);
   eina_shutdown();
}
END_TEST

typedef struct _Animal_Type
{
   Eina_Model_Type parent_class;
   void (*eat)(Eina_Model *mdl);
} Animal_Type;

typedef struct _Human_Type
{
   Animal_Type parent_class;
   void (*talk)(Eina_Model *mdl);
} Human_Type;

typedef struct _Pooper_Interface
{
   Eina_Model_Interface base_interface;
   void (*poop)(Eina_Model *mdl);
} Pooper_Interface;

#define ANIMAL_TYPE(x) ((Animal_Type *) x)
#define HUMAN_TYPE(x) ((Human_Type *) x)
#define POOPER_IFACE(x) ((Pooper_Interface *) x)
#define POOPER_IFACE_NAME "Pooper_Interace"

#define INHER_CB_COUNT(prefix) \
static int prefix ## _count = 0; \
static void \
prefix (Eina_Model *mdl) \
{ \
   (void) mdl; \
   (prefix ## _count)++; \
}

static void
animal_eat(Eina_Model *mdl)
{
   void (*pf)(Eina_Model *mdl);
   pf = eina_model_method_resolve(mdl, Animal_Type, eat);
   EINA_SAFETY_ON_NULL_RETURN(pf);
   pf(mdl);
}

static void
pooper_poop(Eina_Model *mdl)
{
   const Eina_Model_Interface *iface = NULL;
   iface = eina_model_interface_get(mdl, POOPER_IFACE_NAME);

   EINA_SAFETY_ON_NULL_RETURN(iface);

   void (*pf)(Eina_Model *);

   pf = eina_model_interface_method_resolve(iface, mdl, Pooper_Interface, poop);
   EINA_SAFETY_ON_NULL_RETURN(pf);
   pf(mdl);
}

INHER_CB_COUNT(_animal_poop);
INHER_CB_COUNT(_human_poop);
INHER_CB_COUNT(_animal_eat);
INHER_CB_COUNT(_human_eat);

START_TEST(eina_model_test_inheritance)
{
   eina_init();

   Pooper_Interface _ANIMAL_POOPER_IFACE;
   Eina_Model_Interface *ANIMAL_POOPER_IFACE = (Eina_Model_Interface *) &_ANIMAL_POOPER_IFACE;
   memset(&_ANIMAL_POOPER_IFACE, 0, sizeof(_ANIMAL_POOPER_IFACE));
   ANIMAL_POOPER_IFACE->version = EINA_MODEL_INTERFACE_VERSION;
   ANIMAL_POOPER_IFACE->interface_size = sizeof(Pooper_Interface);
   ANIMAL_POOPER_IFACE->name = POOPER_IFACE_NAME;
   POOPER_IFACE(ANIMAL_POOPER_IFACE)->poop = _animal_poop;

   Pooper_Interface _HUMAN_POOPER_IFACE;
   Eina_Model_Interface *HUMAN_POOPER_IFACE = (Eina_Model_Interface *) &_HUMAN_POOPER_IFACE;
   const Eina_Model_Interface *HUMAN_POOPER_IFACES[] = {
     ANIMAL_POOPER_IFACE, NULL
   };
   memset(&_HUMAN_POOPER_IFACE, 0, sizeof(_HUMAN_POOPER_IFACE));
   HUMAN_POOPER_IFACE->version = EINA_MODEL_INTERFACE_VERSION;
   HUMAN_POOPER_IFACE->interface_size = sizeof(Pooper_Interface);
   HUMAN_POOPER_IFACE->name = POOPER_IFACE_NAME;
   HUMAN_POOPER_IFACE->interfaces = HUMAN_POOPER_IFACES;
   POOPER_IFACE(HUMAN_POOPER_IFACE)->poop = _human_poop;

   const Eina_Model_Interface *ANIMAL_IFACES[] = {ANIMAL_POOPER_IFACE, NULL};
   const Eina_Model_Interface *HUMAN_IFACES[] = {HUMAN_POOPER_IFACE, NULL};

   /* Init Animal Type */
   Animal_Type _ANIMAL_TYPE;
   Eina_Model_Type *ANIMAL_TYPE = (Eina_Model_Type *) &_ANIMAL_TYPE;

   memset(&_ANIMAL_TYPE, 0, sizeof(_ANIMAL_TYPE));
   Eina_Model_Type *type = (Eina_Model_Type *) &_ANIMAL_TYPE;
   type->version = EINA_MODEL_TYPE_VERSION;
   type->parent = EINA_MODEL_TYPE_BASE;
   type->type_size = sizeof(Animal_Type);
   type->name = "Animal_Type";
   type->parent = EINA_MODEL_TYPE_GENERIC;
   type->interfaces = ANIMAL_IFACES;

   ANIMAL_TYPE(type)->eat = _animal_eat;

   /* Init Human Type */
   Animal_Type _HUMAN_TYPE;
   Eina_Model_Type *HUMAN_TYPE = (Eina_Model_Type *) &_HUMAN_TYPE;
   memset(&_HUMAN_TYPE, 0, sizeof(_HUMAN_TYPE));
   type = (Eina_Model_Type *) &_HUMAN_TYPE;
   type->version = EINA_MODEL_TYPE_VERSION;
   type->parent = ANIMAL_TYPE;
   type->type_size = sizeof(Human_Type);
   type->name = "Human_Type";
   type->interfaces = HUMAN_IFACES;

   ANIMAL_TYPE(type)->eat = _human_eat;

   Eina_Model *hm, *am;
   am = eina_model_new(ANIMAL_TYPE);
   hm = eina_model_new(HUMAN_TYPE);

   animal_eat(am);
   ck_assert_int_eq(_animal_eat_count, 1);
   animal_eat(hm);
   ck_assert_int_eq(_human_eat_count, 1);

   pooper_poop(am);
   ck_assert_int_eq(_animal_poop_count, 1);
   pooper_poop(hm);
   ck_assert_int_eq(_human_poop_count, 1);

   ck_assert_int_eq(_animal_eat_count, 1);
   ck_assert_int_eq(_human_eat_count, 1);
   ck_assert_int_eq(_animal_poop_count, 1);
   ck_assert_int_eq(_human_poop_count, 1);

   ck_assert_int_eq(eina_model_refcount(am), 1);
   ck_assert_int_eq(eina_model_refcount(hm), 1);

   eina_model_unref(am);
   eina_model_unref(hm);

   eina_shutdown();
}
END_TEST

static Eina_Bool
_myproperties_load(Eina_Model *m)
{
   Eina_Value v;
   Eina_Bool ret;
   int count;

   if (!eina_model_property_get(m, "load_count", &v))
     return EINA_FALSE;

   eina_value_get(&v, &count);
   count++;
   eina_value_set(&v, count);

   ret = eina_model_property_set(m, "load_count", &v);
   eina_value_flush(&v);

   return ret;
}

static Eina_Bool
_myproperties_unload(Eina_Model *m)
{
   Eina_Value v;
   Eina_Bool ret;
   int count;

   if (!eina_model_property_get(m, "load_count", &v))
     return EINA_FALSE;

   eina_value_get(&v, &count);
   count--;
   eina_value_set(&v, count);

   ret = eina_model_property_set(m, "load_count", &v);
   eina_value_flush(&v);

   return ret;
}

static Eina_Bool
_mychildren_load(Eina_Model *m)
{
   Eina_Model *c = eina_model_new(EINA_MODEL_TYPE_GENERIC);
   int ret = eina_model_child_append(m, c);
   eina_model_unref(c);
   return ret >= 0;
}

static Eina_Bool
_mychildren_unload(Eina_Model *m)
{
   int count = eina_model_child_count(m);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(count > 0, EINA_FALSE);
   return eina_model_child_del(m, count - 1);
}

START_TEST(eina_model_test_ifaces_load_unload)
{
   unsigned int count_loaded = 0, count_unloaded = 0;
   unsigned int count_ploaded = 0, count_punloaded = 0;
   unsigned int count_cloaded = 0, count_cunloaded = 0;
   static Eina_Model_Interface_Properties piface;
   static Eina_Model_Interface_Children ciface;
   static const Eina_Model_Interface *piface_parents[2] = {NULL, NULL};
   static const Eina_Model_Interface *ciface_parents[2] = {NULL, NULL};
   static const Eina_Model_Interface *type_ifaces[3] = {
     &piface.base, &ciface.base, NULL
   };
   static Eina_Model_Type type;
   Eina_Model *m;
   Eina_Value v;
   int count;

   eina_init();

   /* do after eina_init() otherwise interfaces are not set */
   piface_parents[0] = EINA_MODEL_INTERFACE_PROPERTIES_HASH;
   ciface_parents[0] = EINA_MODEL_INTERFACE_CHILDREN_INARRAY;

   memset(&piface, 0, sizeof(piface));
   piface.base.version = EINA_MODEL_INTERFACE_VERSION;
   piface.base.interface_size = sizeof(piface);
   piface.base.name = EINA_MODEL_INTERFACE_NAME_PROPERTIES;
   piface.base.interfaces = piface_parents;
   piface.load = _myproperties_load;
   piface.unload = _myproperties_unload;

   memset(&ciface, 0, sizeof(ciface));
   ciface.base.version = EINA_MODEL_INTERFACE_VERSION;
   ciface.base.interface_size = sizeof(ciface);
   ciface.base.name = EINA_MODEL_INTERFACE_NAME_CHILDREN;
   ciface.base.interfaces = ciface_parents;
   ciface.load = _mychildren_load;
   ciface.unload = _mychildren_unload;

   type.version = EINA_MODEL_TYPE_VERSION;
   type.private_size = 0;
   type.name = "MyType";
   eina_model_type_subclass_setup(&type,  EINA_MODEL_TYPE_GENERIC);
   type.interfaces = type_ifaces;

   m = eina_model_new(&type);
   fail_unless(m != NULL);

   eina_model_event_callback_add
     (m, "loaded", _eina_test_model_cb_count, &count_loaded);
   eina_model_event_callback_add
     (m, "unloaded", _eina_test_model_cb_count, &count_unloaded);

   eina_model_event_callback_add
     (m, "properties,loaded", _eina_test_model_cb_count, &count_ploaded);
   eina_model_event_callback_add
     (m, "properties,unloaded", _eina_test_model_cb_count, &count_punloaded);

   eina_model_event_callback_add
     (m, "children,loaded", _eina_test_model_cb_count, &count_cloaded);
   eina_model_event_callback_add
     (m, "children,unloaded", _eina_test_model_cb_count, &count_cunloaded);

   fail_unless(eina_value_setup(&v, EINA_VALUE_TYPE_INT));
   fail_unless(eina_value_set(&v, 0));
   fail_unless(eina_model_property_set(m, "load_count", &v));
   eina_value_flush(&v);

   fail_unless(eina_model_load(m));
   fail_unless(eina_model_load(m));
   fail_unless(eina_model_load(m));

   /* each load increments one for load_count property */
   fail_unless(eina_model_property_get(m, "load_count", &v));
   fail_unless(eina_value_pget(&v, &count));
   ck_assert_int_eq(count, 3);
   eina_value_flush(&v);

   /* each load adds one child */
   ck_assert_int_eq(eina_model_child_count(m), 3);

   fail_unless(eina_model_unload(m));
   fail_unless(eina_model_unload(m));
   fail_unless(eina_model_unload(m));

   ck_assert_int_eq(count_loaded, 3);
   ck_assert_int_eq(count_unloaded, 3);

   ck_assert_int_eq(count_ploaded, 3);
   ck_assert_int_eq(count_punloaded, 3);

   ck_assert_int_eq(count_cloaded, 3);
   ck_assert_int_eq(count_cunloaded, 3);

   ck_assert_int_eq(eina_model_refcount(m), 1);
   eina_model_unref(m);

   eina_shutdown();
}
END_TEST

void
eina_test_model(TCase *tc)
{
   tcase_add_test(tc, eina_model_test_properties);
   tcase_add_test(tc, eina_model_test_children);
   tcase_add_test(tc, eina_model_test_copy);
   tcase_add_test(tc, eina_model_test_deep_copy);
   tcase_add_test(tc, eina_model_test_child_iterator);
   tcase_add_test(tc, eina_model_test_child_reversed_iterator);
   tcase_add_test(tc, eina_model_test_child_sorted_iterator);
   tcase_add_test(tc, eina_model_test_child_filtered_iterator);
   tcase_add_test(tc, eina_model_test_struct);
   tcase_add_test(tc, eina_model_test_struct_complex_members);
   tcase_add_test(tc, eina_model_test_inheritance);
   tcase_add_test(tc, eina_model_test_ifaces_load_unload);
}
