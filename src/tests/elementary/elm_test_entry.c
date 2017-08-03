#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define ELM_INTERFACE_ATSPI_TEXT_PROTECTED
#define ELM_INTERFACE_ATSPI_ACCESSIBLE_PROTECTED
#include <Elementary.h>
#include "elm_suite.h"

START_TEST (elm_entry_del)
{
   Evas_Object *win, *entry;

   elm_init(1, NULL);
   win = elm_win_add(NULL, "entry", ELM_WIN_BASIC);

   entry = elm_entry_add(win);
   elm_object_text_set(entry, "TEST");

   elm_shutdown();
}
END_TEST

void
_dummy_cb(void *data EINA_UNUSED, Eo *obj EINA_UNUSED,
          const char *sig EINA_UNUSED, const char *src EINA_UNUSED)
{
}

START_TEST (elm_entry_signal_callback)
{
   Evas_Object *win, *entry;
   void *data;
   int k;

   elm_init(1, NULL);
   win = elm_win_add(NULL, "entry", ELM_WIN_BASIC);

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

   elm_shutdown();
}
END_TEST

START_TEST (elm_entry_atspi_text_char_get)
{
   Evas_Object *win, *entry;
   Eina_Unicode *expected;
   Eina_Unicode val;

   const char *txt = "ĄA11Y Ł TEST";
   const char *mtxt = "<b>ĄA11Y</b> <title>Ł</> TEST";

   elm_init(1, NULL);
   win = elm_win_add(NULL, "entry", ELM_WIN_BASIC);

   entry = elm_entry_add(win);
   elm_object_text_set(entry, mtxt);

   expected = eina_unicode_utf8_to_unicode(txt, NULL);

   val = elm_interface_atspi_text_character_get(entry, -1);
   ck_assert(val == 0);

   val = elm_interface_atspi_text_character_get(entry, 0);
   ck_assert(val == expected[0]);

   val = elm_interface_atspi_text_character_get(entry, 1);
   ck_assert(val == expected[1]);

   val = elm_interface_atspi_text_character_get(entry, 2);
   ck_assert(val == expected[2]);

   val = elm_interface_atspi_text_character_get(entry, 6);
   ck_assert(val == expected[6]);

   val = elm_interface_atspi_text_character_get(entry, 26);
   ck_assert(val == 0);

   free(expected);
   elm_shutdown();
}
END_TEST

START_TEST (elm_entry_atspi_text_char_count)
{
   Evas_Object *win, *entry;
   int val;

   const char *mtxt = "<b>AĄ11Y</b> <title>Ł</> TEST";

   elm_init(1, NULL);
   win = elm_win_add(NULL, "entry", ELM_WIN_BASIC);

   entry = elm_entry_add(win);
   elm_object_text_set(entry, mtxt);

   val = elm_interface_atspi_text_character_count_get(entry);
   ck_assert(val == 12);

   elm_shutdown();
}
END_TEST

START_TEST (elm_entry_atspi_text_string_get_char)
{
   Evas_Object *win, *entry;
   char *val;
   int start, end;

   const char *txt = "Lorem ipśum   dolor sit amęt";

   elm_init(1, NULL);
   win = elm_win_add(NULL, "entry", ELM_WIN_BASIC);

   entry = elm_entry_add(win);
   elm_object_text_set(entry, txt);

   start = 1;
   val = elm_interface_atspi_text_string_get(entry, ELM_ATSPI_TEXT_GRANULARITY_CHAR, &start, &end);
   ck_assert_str_eq(val, "o");
   ck_assert(start == 1);
   ck_assert(end == 2);
   if (val) free(val);

   start = 8;
   val = elm_interface_atspi_text_string_get(entry, ELM_ATSPI_TEXT_GRANULARITY_CHAR, &start, &end);
   ck_assert_str_eq(val, "ś");
   ck_assert(start == 8);
   ck_assert(end == 9);
   if (val) free(val);

   start = 11;
   val = elm_interface_atspi_text_string_get(entry, ELM_ATSPI_TEXT_GRANULARITY_CHAR, &start, &end);
   ck_assert_str_eq(val, " ");
   ck_assert(start == 11);
   ck_assert(end == 12);
   if (val) free(val);

   start = 111;
   val = elm_interface_atspi_text_string_get(entry, ELM_ATSPI_TEXT_GRANULARITY_CHAR, &start, &end);
   ck_assert(start == -1);
   ck_assert(end == -1);
   ck_assert(val == NULL);

   elm_shutdown();
}
END_TEST

START_TEST (elm_entry_atspi_text_string_get_word)
{
   Evas_Object *win, *entry;
   char *val;
   int start, end;

   const char *txt = "Lorem ipśum   dolor sit amęt";

   elm_init(1, NULL);
   win = elm_win_add(NULL, "entry", ELM_WIN_BASIC);

   entry = elm_entry_add(win);
   elm_object_text_set(entry, txt);

   start = 1;
   val = elm_interface_atspi_text_string_get(entry, ELM_ATSPI_TEXT_GRANULARITY_WORD, &start, &end);
   ck_assert_str_eq(val, "Lorem");
   ck_assert(start == 0);
   ck_assert(end == 5);
   if (val) free(val);

   start = 6;
   val = elm_interface_atspi_text_string_get(entry, ELM_ATSPI_TEXT_GRANULARITY_WORD, &start, &end);
   ck_assert_str_eq(val, "ipśum");
   ck_assert(start == 6);
   ck_assert(end == 11);
   if (val) free(val);

   start = 19;
   val = elm_interface_atspi_text_string_get(entry, ELM_ATSPI_TEXT_GRANULARITY_WORD, &start, &end);
   ck_assert_str_eq(val, "dolor");
   ck_assert(start == 14);
   ck_assert(end == 19);
   if (val) free(val);

   start = 111;
   val = elm_interface_atspi_text_string_get(entry, ELM_ATSPI_TEXT_GRANULARITY_WORD, &start, &end);
   ck_assert(start == -1);
   ck_assert(end == -1);
   ck_assert(val == NULL);
   if (val) free(val);

   elm_shutdown();
}
END_TEST

START_TEST (elm_entry_atspi_text_string_get_paragraph)
{
   Evas_Object *win, *entry;
   char *val;
   int start, end;

   const char *txt = "Lorem ipśum<br>   dolor sit<br> amęt";

   elm_init(1, NULL);
   win = elm_win_add(NULL, "entry", ELM_WIN_BASIC);

   entry = elm_entry_add(win);
   elm_object_text_set(entry, txt);

   start = 1;
   val = elm_interface_atspi_text_string_get(entry, ELM_ATSPI_TEXT_GRANULARITY_PARAGRAPH, &start, &end);
   ck_assert_str_eq(val, "Lorem ipśum");
   ck_assert(start == 0);
   ck_assert(end == 11);
   if (val) free(val);

   start = 20;
   val = elm_interface_atspi_text_string_get(entry, ELM_ATSPI_TEXT_GRANULARITY_PARAGRAPH, &start, &end);
   ck_assert_str_eq(val, "   dolor sit");
   ck_assert(start == 12);
   ck_assert(end == 24);
   if (val) free(val);

   start = 25;
   val = elm_interface_atspi_text_string_get(entry, ELM_ATSPI_TEXT_GRANULARITY_PARAGRAPH, &start, &end);
   ck_assert_str_eq(val, " amęt");
   ck_assert(start == 25);
   ck_assert(end == 30);
   if (val) free(val);

   start = 111;
   val = elm_interface_atspi_text_string_get(entry, ELM_ATSPI_TEXT_GRANULARITY_WORD, &start, &end);
   ck_assert(start == -1);
   ck_assert(end == -1);
   ck_assert(val == NULL);
   if (val) free(val);

   elm_shutdown();
}
END_TEST

START_TEST (elm_entry_atspi_text_string_get_line)
{
   Evas_Object *win, *entry;
   char *val;
   int start, end;

   const char *txt = "Lorem ipśum<br>   dolor sit amęt";

   elm_init(1, NULL);
   win = elm_win_add(NULL, "entry", ELM_WIN_BASIC);

   entry = elm_entry_add(win);
   evas_object_resize(entry, 500, 500);

   elm_object_text_set(entry, txt);

   start = 1;
   val = elm_interface_atspi_text_string_get(entry, ELM_ATSPI_TEXT_GRANULARITY_LINE, &start, &end);

   ck_assert_str_eq(val, "Lorem ipśum");
   ck_assert(start == 0);
   ck_assert(end == 11);
   if (val) free(val);

   start = 13;
   val = elm_interface_atspi_text_string_get(entry, ELM_ATSPI_TEXT_GRANULARITY_LINE, &start, &end);
   ck_assert_str_eq(val, "   dolor sit amęt");
   ck_assert(start == 12);
   ck_assert(end == 29);
   if (val) free(val);

   elm_shutdown();
}
END_TEST

START_TEST (elm_entry_atspi_text_text_get)
{
   Evas_Object *win, *entry;
   char *val;
   const char *txt = "Lorem Xpśum dolor sit amęt";

   elm_init(1, NULL);
   win = elm_win_add(NULL, "entry", ELM_WIN_BASIC);

   entry = elm_entry_add(win);
   elm_object_text_set(entry, txt);

   // invalid ranges
   val = elm_interface_atspi_text_get(entry, 6, 100);
   ck_assert(val == NULL);
   val = elm_interface_atspi_text_get(entry, -6, 10);
   ck_assert(val == NULL);
   val = elm_interface_atspi_text_get(entry, -6, -10);
   ck_assert(val == NULL);
   val = elm_interface_atspi_text_get(entry, 60, 100);
   ck_assert(val == NULL);

   // proper range
   val = elm_interface_atspi_text_get(entry, 6, 17);
   ck_assert_str_eq(val, "Xpśum dolor");

   if (val) free(val);
   elm_shutdown();
}
END_TEST

START_TEST (elm_entry_atspi_text_selections)
{
   Evas_Object *win, *entry;
   int val, start, end;
   const char *str;
   Eina_Bool ret;
   const char *txt = "Lorem ipśum<br>   dolor sit\n amęt";

   elm_init(1, NULL);
   win = elm_win_add(NULL, "entry", ELM_WIN_BASIC);

   entry = elm_entry_add(win);
   elm_object_text_set(entry, txt);

   val = elm_interface_atspi_text_selections_count_get(entry);
   ck_assert(val == 0);

   elm_entry_select_region_set(entry, 2, 4);
   val = elm_interface_atspi_text_selections_count_get(entry);
   ck_assert(val == 1);
   elm_interface_atspi_text_selection_get(entry, 0, &start, &end);
   ck_assert(start == 2);
   ck_assert(end == 4);

   elm_entry_select_region_set(entry, 6, 10);
   val = elm_interface_atspi_text_selections_count_get(entry);
   ck_assert(val == 1);
   elm_interface_atspi_text_selection_get(entry, 0, &start, &end);
   ck_assert(start == 6);
   ck_assert(end == 10);

   elm_entry_select_none(entry);
   ret = elm_interface_atspi_text_selection_add(entry, 2, 5);
   ck_assert(ret == EINA_TRUE);
   str = elm_entry_selection_get(entry);
   ck_assert_str_eq(str, "rem");

   ret = elm_interface_atspi_text_selection_remove(entry, 0);
   ck_assert(ret == EINA_TRUE);
   str = elm_entry_selection_get(entry);
   ck_assert(str == NULL);

   elm_shutdown();
}
END_TEST

START_TEST (elm_atspi_role_get)
{
   Evas_Object *win, *entry;
   Elm_Atspi_Role role;

   elm_init(1, NULL);
   win = elm_win_add(NULL, "entry", ELM_WIN_BASIC);

   entry = elm_entry_add(win);
   role = elm_interface_atspi_accessible_role_get(entry);

   ck_assert(role == ELM_ATSPI_ROLE_ENTRY);

   elm_shutdown();
}
END_TEST

void elm_test_entry(TCase *tc)
{
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
}
