#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_OBJECT_BETA
#define EFL_ACCESS_TEXT_BETA
#define EFL_ACCESS_TEXT_PROTECTED
#include <Elementary.h>
#include "elm_suite.h"

EFL_START_TEST(elm_entry_legacy_type_check)
{
   Evas_Object *win, *entry;
   const char *type;

   win = win_add(NULL, "entry", ELM_WIN_BASIC);

   entry = elm_entry_add(win);

   type = elm_object_widget_type_get(entry);
   ck_assert(type != NULL);
   ck_assert(!strcmp(type, "Elm_Entry"));

   type = evas_object_type_get(entry);
   ck_assert(type != NULL);
   ck_assert(!strcmp(type, "elm_entry"));

}
EFL_END_TEST

EFL_START_TEST(elm_entry_del)
{
   Evas_Object *win, *entry;

   win = win_add(NULL, "entry", ELM_WIN_BASIC);

   entry = elm_entry_add(win);
   elm_object_text_set(entry, "TEST");

}
EFL_END_TEST

void
_dummy_cb(void *data EINA_UNUSED, Eo *obj EINA_UNUSED,
          const char *sig EINA_UNUSED, const char *src EINA_UNUSED)
{
}

EFL_START_TEST(elm_entry_signal_callback)
{
   Evas_Object *win, *entry;
   void *data;
   int k;

   win = win_add(NULL, "entry", ELM_WIN_BASIC);

   entry = elm_entry_add(win);

   for (k = 1; k < 10; k++)
     {
        int *val = malloc(sizeof(int));
        *val = 42 * k;
        elm_layout_signal_callback_add(entry, "sig", "src", _dummy_cb, val);
     }

   data = elm_layout_signal_callback_del(entry, "notsig", "notsrc", _dummy_cb);
   fail_if(data);

   // this test verifies that the legacy wrapper returns the proper data
   // the eo API requires the data as input to "del()"
   for (k = 1; k < 10; k++)
     {
        int *val;
        data = elm_layout_signal_callback_del(entry, "sig", "src", _dummy_cb);
        fail_if(!data);
        val = (int *)data;
        ck_assert_int_ne(*val, 0);
        ck_assert_int_eq((*val) % 42, 0);
        free(data);
     }

   data = elm_layout_signal_callback_del(entry, "sig", "src", _dummy_cb);
   fail_if(data);

}
EFL_END_TEST

EFL_START_TEST(elm_entry_atspi_text_char_get)
{
   Evas_Object *win, *entry;
   Eina_Unicode *expected;
   Eina_Unicode val;

   const char *txt = "ĄA11Y Ł TEST";
   const char *mtxt = "<b>ĄA11Y</b> <title>Ł</> TEST";

   win = win_add(NULL, "entry", ELM_WIN_BASIC);

   entry = elm_entry_add(win);
   elm_object_text_set(entry, mtxt);

   expected = eina_unicode_utf8_to_unicode(txt, NULL);

   val = efl_access_text_character_get(entry, -1);
   ck_assert(val == 0);

   val = efl_access_text_character_get(entry, 0);
   ck_assert(val == expected[0]);

   val = efl_access_text_character_get(entry, 1);
   ck_assert(val == expected[1]);

   val = efl_access_text_character_get(entry, 2);
   ck_assert(val == expected[2]);

   val = efl_access_text_character_get(entry, 6);
   ck_assert(val == expected[6]);

   val = efl_access_text_character_get(entry, 26);
   ck_assert(val == 0);

   free(expected);
}
EFL_END_TEST

EFL_START_TEST(elm_entry_atspi_text_char_count)
{
   Evas_Object *win, *entry;
   int val;

   const char *mtxt = "<b>AĄ11Y</b> <title>Ł</> TEST";

   win = win_add(NULL, "entry", ELM_WIN_BASIC);

   entry = elm_entry_add(win);
   elm_object_text_set(entry, mtxt);

   val = efl_access_text_character_count_get(entry);
   ck_assert(val == 12);

}
EFL_END_TEST

EFL_START_TEST(elm_entry_atspi_text_string_get_char)
{
   Evas_Object *win, *entry;
   char *val;
   int start, end;

   const char *txt = "Lorem ipśum   dolor sit amęt";

   win = win_add(NULL, "entry", ELM_WIN_BASIC);

   entry = elm_entry_add(win);
   elm_object_text_set(entry, txt);

   start = 1;
   val = efl_access_text_string_get(entry, EFL_ACCESS_TEXT_GRANULARITY_CHAR, &start, &end);
   ck_assert_str_eq(val, "o");
   ck_assert(start == 1);
   ck_assert(end == 2);
   if (val) free(val);

   start = 8;
   val = efl_access_text_string_get(entry, EFL_ACCESS_TEXT_GRANULARITY_CHAR, &start, &end);
   ck_assert_str_eq(val, "ś");
   ck_assert(start == 8);
   ck_assert(end == 9);
   if (val) free(val);

   start = 11;
   val = efl_access_text_string_get(entry, EFL_ACCESS_TEXT_GRANULARITY_CHAR, &start, &end);
   ck_assert_str_eq(val, " ");
   ck_assert(start == 11);
   ck_assert(end == 12);
   if (val) free(val);

   start = 111;
   val = efl_access_text_string_get(entry, EFL_ACCESS_TEXT_GRANULARITY_CHAR, &start, &end);
   ck_assert(start == -1);
   ck_assert(end == -1);
   ck_assert(val == NULL);

}
EFL_END_TEST

EFL_START_TEST(elm_entry_atspi_text_string_get_word)
{
   Evas_Object *win, *entry;
   char *val;
   int start, end;

   const char *txt = "Lorem ipśum   dolor sit amęt";

   win = win_add(NULL, "entry", ELM_WIN_BASIC);

   entry = elm_entry_add(win);
   elm_object_text_set(entry, txt);

   start = 1;
   val = efl_access_text_string_get(entry, EFL_ACCESS_TEXT_GRANULARITY_WORD, &start, &end);
   ck_assert_str_eq(val, "Lorem");
   ck_assert(start == 0);
   ck_assert(end == 5);
   if (val) free(val);

   start = 6;
   val = efl_access_text_string_get(entry, EFL_ACCESS_TEXT_GRANULARITY_WORD, &start, &end);
   ck_assert_str_eq(val, "ipśum");
   ck_assert(start == 6);
   ck_assert(end == 11);
   if (val) free(val);

   start = 19;
   val = efl_access_text_string_get(entry, EFL_ACCESS_TEXT_GRANULARITY_WORD, &start, &end);
   ck_assert_str_eq(val, "dolor");
   ck_assert(start == 14);
   ck_assert(end == 19);
   if (val) free(val);

   start = 111;
   val = efl_access_text_string_get(entry, EFL_ACCESS_TEXT_GRANULARITY_WORD, &start, &end);
   ck_assert(start == -1);
   ck_assert(end == -1);
   ck_assert(val == NULL);
   if (val) free(val);

}
EFL_END_TEST

EFL_START_TEST(elm_entry_atspi_text_string_get_paragraph)
{
   Evas_Object *win, *entry;
   char *val;
   int start, end;

   const char *txt = "Lorem ipśum<br>   dolor sit<br> amęt";

   win = win_add(NULL, "entry", ELM_WIN_BASIC);

   entry = elm_entry_add(win);
   elm_object_text_set(entry, txt);

   start = 1;
   val = efl_access_text_string_get(entry, EFL_ACCESS_TEXT_GRANULARITY_PARAGRAPH, &start, &end);
   ck_assert_str_eq(val, "Lorem ipśum");
   ck_assert(start == 0);
   ck_assert(end == 11);
   if (val) free(val);

   start = 20;
   val = efl_access_text_string_get(entry, EFL_ACCESS_TEXT_GRANULARITY_PARAGRAPH, &start, &end);
   ck_assert_str_eq(val, "   dolor sit");
   ck_assert(start == 12);
   ck_assert(end == 24);
   if (val) free(val);

   start = 25;
   val = efl_access_text_string_get(entry, EFL_ACCESS_TEXT_GRANULARITY_PARAGRAPH, &start, &end);
   ck_assert_str_eq(val, " amęt");
   ck_assert(start == 25);
   ck_assert(end == 30);
   if (val) free(val);

   start = 111;
   val = efl_access_text_string_get(entry, EFL_ACCESS_TEXT_GRANULARITY_WORD, &start, &end);
   ck_assert(start == -1);
   ck_assert(end == -1);
   ck_assert(val == NULL);
   if (val) free(val);

}
EFL_END_TEST

EFL_START_TEST(elm_entry_atspi_text_string_get_line)
{
   Evas_Object *win, *entry;
   char *val;
   int start, end;

   const char *txt = "Lorem ipśum<br>   dolor sit amęt";

   win = win_add(NULL, "entry", ELM_WIN_BASIC);

   entry = elm_entry_add(win);
   evas_object_resize(entry, 500, 500);

   elm_object_text_set(entry, txt);

   start = 1;
   val = efl_access_text_string_get(entry, EFL_ACCESS_TEXT_GRANULARITY_LINE, &start, &end);

   ck_assert_str_eq(val, "Lorem ipśum");
   ck_assert(start == 0);
   ck_assert(end == 11);
   if (val) free(val);

   start = 13;
   val = efl_access_text_string_get(entry, EFL_ACCESS_TEXT_GRANULARITY_LINE, &start, &end);
   ck_assert_str_eq(val, "   dolor sit amęt");
   ck_assert(start == 12);
   ck_assert(end == 29);
   if (val) free(val);

}
EFL_END_TEST

EFL_START_TEST(elm_entry_atspi_text_text_get)
{
   Evas_Object *win, *entry;
   char *val;
   const char *txt = "Lorem Xpśum dolor sit amęt";

   win = win_add(NULL, "entry", ELM_WIN_BASIC);

   entry = elm_entry_add(win);
   elm_object_text_set(entry, txt);

   // invalid ranges
   val = efl_access_text_get(entry, 6, 100);
   ck_assert(val == NULL);
   val = efl_access_text_get(entry, -6, 10);
   ck_assert(val == NULL);
   val = efl_access_text_get(entry, -6, -10);
   ck_assert(val == NULL);
   val = efl_access_text_get(entry, 60, 100);
   ck_assert(val == NULL);

   // proper range
   val = efl_access_text_get(entry, 6, 17);
   ck_assert_str_eq(val, "Xpśum dolor");

   if (val) free(val);
}
EFL_END_TEST

EFL_START_TEST(elm_entry_atspi_text_selections)
{
   Evas_Object *win, *entry;
   int val, start, end;
   const char *str;
   Eina_Bool ret;
   const char *txt = "Lorem ipśum<br>   dolor sit\n amęt";

   win = win_add(NULL, "entry", ELM_WIN_BASIC);

   entry = elm_entry_add(win);
   elm_object_text_set(entry, txt);

   val = efl_access_text_selections_count_get(entry);
   ck_assert(val == 0);

   elm_entry_select_region_set(entry, 2, 4);
   val = efl_access_text_selections_count_get(entry);
   ck_assert(val == 1);
   efl_access_text_access_selection_get(entry, 0, &start, &end);
   ck_assert(start == 2);
   ck_assert(end == 4);

   elm_entry_select_region_set(entry, 6, 10);
   val = efl_access_text_selections_count_get(entry);
   ck_assert(val == 1);
   efl_access_text_access_selection_get(entry, 0, &start, &end);
   ck_assert(start == 6);
   ck_assert(end == 10);

   elm_entry_select_none(entry);
   ret = efl_access_text_selection_add(entry, 2, 5);
   ck_assert(ret == EINA_TRUE);
   str = elm_entry_selection_get(entry);
   ck_assert_str_eq(str, "rem");

   ret = efl_access_text_selection_remove(entry, 0);
   ck_assert(ret == EINA_TRUE);
   str = elm_entry_selection_get(entry);
   ck_assert(str == NULL);

}
EFL_END_TEST

EFL_START_TEST(elm_atspi_role_get)
{
   Evas_Object *win, *entry;
   Efl_Access_Role role;

   win = win_add(NULL, "entry", ELM_WIN_BASIC);

   entry = elm_entry_add(win);
   role = efl_access_object_role_get(entry);

   ck_assert(role == EFL_ACCESS_ROLE_ENTRY);

}
EFL_END_TEST

EFL_START_TEST(elm_entry_text_set)
{
   Evas_Object *win, *entry;
   const char *entry_text = "hello world";
   const char *entry_text2 = "scrollable";

   win = win_add(NULL, "entry", ELM_WIN_BASIC);

   entry = elm_entry_add(win);

   ck_assert(elm_layout_text_set(entry, NULL, entry_text));
   ck_assert_str_eq(elm_object_text_get(entry), entry_text);

   elm_entry_scrollable_set(entry, EINA_TRUE);
   ck_assert(elm_layout_text_set(entry, NULL, entry_text2));
   ck_assert_str_eq(elm_object_text_get(entry), entry_text2);
}
EFL_END_TEST

void elm_test_entry(TCase *tc)
{
   tcase_add_test(tc, elm_entry_legacy_type_check);
   tcase_add_test(tc, elm_entry_del);
   tcase_add_test(tc, elm_entry_signal_callback);
   tcase_add_test(tc, elm_entry_atspi_text_char_get);
   tcase_add_test(tc, elm_entry_atspi_text_char_count);
   tcase_add_test(tc, elm_entry_atspi_text_string_get_char);
   tcase_add_test(tc, elm_entry_atspi_text_string_get_word);
   tcase_add_test(tc, elm_entry_atspi_text_string_get_paragraph);
   tcase_add_test(tc, elm_entry_atspi_text_string_get_line);
   tcase_add_test(tc, elm_entry_atspi_text_text_get);
   tcase_add_test(tc, elm_entry_atspi_text_selections);
   tcase_add_test(tc, elm_atspi_role_get);
   tcase_add_test(tc, elm_entry_text_set);
}
