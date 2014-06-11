#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_suite.h"

#define ELM_INTERFACE_ATSPI_TEXT_PROTECTED
#define ELM_INTERFACE_ATSPI_ACCESSIBLE_PROTECTED
#include "elm_interface_atspi_text.h"
#include "elm_interface_atspi_text.eo.h"
#include "elm_interface_atspi_accessible.h"
#include "elm_interface_atspi_accessible.eo.h"

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

   eo_do(entry, val = elm_interface_atspi_text_character_get(-1));
   ck_assert(val == 0);

   eo_do(entry, val = elm_interface_atspi_text_character_get(0));
   ck_assert(val == expected[0]);

   eo_do(entry, val = elm_interface_atspi_text_character_get(1));
   ck_assert(val == expected[1]);

   eo_do(entry, val = elm_interface_atspi_text_character_get(2));
   ck_assert(val == expected[2]);

   eo_do(entry, val = elm_interface_atspi_text_character_get(6));
   ck_assert(val == expected[6]);

   eo_do(entry, val = elm_interface_atspi_text_character_get(26));
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

   eo_do(entry, val = elm_interface_atspi_text_character_count_get());
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
   eo_do(entry, val = elm_interface_atspi_text_string_get(ELM_ATSPI_TEXT_GRANULARITY_CHAR, &start, &end));
   ck_assert_str_eq(val, "o");
   ck_assert(start == 1);
   ck_assert(end == 2);
   if (val) free(val);

   start = 8;
   eo_do(entry, val = elm_interface_atspi_text_string_get(ELM_ATSPI_TEXT_GRANULARITY_CHAR, &start, &end));
   ck_assert_str_eq(val, "ś");
   ck_assert(start == 8);
   ck_assert(end == 9);
   if (val) free(val);

   start = 11;
   eo_do(entry, val = elm_interface_atspi_text_string_get(ELM_ATSPI_TEXT_GRANULARITY_CHAR, &start, &end));
   ck_assert_str_eq(val, " ");
   ck_assert(start == 11);
   ck_assert(end == 12);
   if (val) free(val);

   start = 111;
   eo_do(entry, val = elm_interface_atspi_text_string_get(ELM_ATSPI_TEXT_GRANULARITY_CHAR, &start, &end));
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
   eo_do(entry, val = elm_interface_atspi_text_string_get(ELM_ATSPI_TEXT_GRANULARITY_WORD, &start, &end));
   ck_assert_str_eq(val, "Lorem");
   ck_assert(start == 0);
   ck_assert(end == 5);
   if (val) free(val);

   start = 6;
   eo_do(entry, val = elm_interface_atspi_text_string_get(ELM_ATSPI_TEXT_GRANULARITY_WORD, &start, &end));
   ck_assert_str_eq(val, "ipśum");
   ck_assert(start == 6);
   ck_assert(end == 11);
   if (val) free(val);

   start = 19;
   eo_do(entry, val = elm_interface_atspi_text_string_get(ELM_ATSPI_TEXT_GRANULARITY_WORD, &start, &end));
   ck_assert_str_eq(val, "dolor");
   ck_assert(start == 14);
   ck_assert(end == 19);
   if (val) free(val);

   start = 111;
   eo_do(entry, val = elm_interface_atspi_text_string_get(ELM_ATSPI_TEXT_GRANULARITY_WORD, &start, &end));
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

   const char *txt = "Lorem ipśum<br>   dolor sit\n amęt";

   elm_init(1, NULL);
   win = elm_win_add(NULL, "entry", ELM_WIN_BASIC);

   entry = elm_entry_add(win);
   elm_object_text_set(entry, txt);

   start = 1;
   eo_do(entry, val = elm_interface_atspi_text_string_get(ELM_ATSPI_TEXT_GRANULARITY_PARAGRAPH, &start, &end));
   ck_assert_str_eq(val, "Lorem ipśum");
   ck_assert(start == 0);
   ck_assert(end == 11);
   if (val) free(val);

   start = 22;
   eo_do(entry, val = elm_interface_atspi_text_string_get(ELM_ATSPI_TEXT_GRANULARITY_PARAGRAPH, &start, &end));
   ck_assert_str_eq(val, "   dolor sit");
   ck_assert(start == 11);
   ck_assert(end == 22);
   if (val) free(val);

   start = 27;
   eo_do(entry, val = elm_interface_atspi_text_string_get(ELM_ATSPI_TEXT_GRANULARITY_PARAGRAPH, &start, &end));
   ck_assert_str_eq(val, " amęt");
   ck_assert(start == 23);
   ck_assert(end == 27);
   if (val) free(val);

   start = 111;
   eo_do(entry, val = elm_interface_atspi_text_string_get(ELM_ATSPI_TEXT_GRANULARITY_WORD, &start, &end));
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

   const char *txt = "Lorem ipśum<br>   dolor sit\n amęt";

   elm_init(1, NULL);
   win = elm_win_add(NULL, "entry", ELM_WIN_BASIC);

   entry = elm_entry_add(win);
   elm_object_text_set(entry, txt);

   start = 1;
   eo_do(entry, val = elm_interface_atspi_text_string_get(ELM_ATSPI_TEXT_GRANULARITY_LINE, &start, &end));
   ck_assert_str_eq(val, "Lorem ipśum");
   ck_assert(start == 0);
   ck_assert(end == 11);
   if (val) free(val);

   start = 13;
   eo_do(entry, val = elm_interface_atspi_text_string_get(ELM_ATSPI_TEXT_GRANULARITY_LINE, &start, &end));
   ck_assert_str_eq(val, "   dolor sit");
   ck_assert(start == 11);
   ck_assert(end == 23);
   if (val) free(val);

   elm_shutdown();
}
END_TEST

START_TEST (elm_entry_atspi_text_text_get)
{
   Evas_Object *win, *entry;
   char *val;
   const char *txt = "Lorem ipśum<br>   dolor sit\n amęt";
   const char *txtnom = "Lorem ipśum   dolor sit\n amęt";

   elm_init(1, NULL);
   win = elm_win_add(NULL, "entry", ELM_WIN_BASIC);

   entry = elm_entry_add(win);
   elm_object_text_set(entry, txt);

   eo_do(entry, val = elm_interface_atspi_text_text_get(0, sizeof(txtnom)/sizeof(txtnom[0])));
   ck_assert_str_eq(val, txtnom);

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

   eo_do(entry, val = elm_interface_atspi_text_selections_count_get());
   ck_assert(val == 0);

   elm_entry_select_region_set(entry, 2, 4);
   eo_do(entry, val = elm_interface_atspi_text_selections_count_get());
   ck_assert(val == 1);
   eo_do(entry, elm_interface_atspi_text_selection_get(0, &start, &end));
   ck_assert(start == 2);
   ck_assert(end == 4);

   elm_entry_select_region_set(entry, 6, 10);
   eo_do(entry, val = elm_interface_atspi_text_selections_count_get());
   ck_assert(val == 1);
   eo_do(entry, elm_interface_atspi_text_selection_get(0, &start, &end));
   ck_assert(start == 6);
   ck_assert(end == 10);

   elm_entry_select_none(entry);
   eo_do(entry, ret = elm_interface_atspi_text_selection_add(2, 5));
   ck_assert(ret == EINA_TRUE);
   str = elm_entry_selection_get(entry);
   ck_assert_str_eq(str, "rem");

   eo_do(entry, ret = elm_interface_atspi_text_selection_remove(0));
   ck_assert(ret == EINA_TRUE);
   str = elm_entry_selection_get(entry);
   ck_assert(str == NULL);

   elm_shutdown();
}
END_TEST

START_TEST (elm_entry_atspi_text_attributes)
{
   Evas_Object *win, *entry;
   Eina_List *formats;
   const char txt[] = "<font_weight=Bold>Lorem ipśum<br></>  dolor sit\n amęt";
   int start = 0, end = sizeof(txt);
   Elm_Atspi_Text_Attribute *attr;

   elm_init(1, NULL);
   win = elm_win_add(NULL, "entry", ELM_WIN_BASIC);

   entry = elm_entry_add(win);
   elm_object_text_set(entry, txt);

   eo_do(entry, formats = elm_interface_atspi_text_attributes_get(&start, &end));
   EINA_LIST_FREE(formats, attr)
     {
        elm_atspi_text_text_attribute_free(attr);
     }

   eo_do(entry, formats = elm_interface_atspi_text_default_attributes_get());
   EINA_LIST_FREE(formats, attr)
     {
        elm_atspi_text_text_attribute_free(attr);
     }

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
   eo_do(entry, role = elm_interface_atspi_accessible_role_get());

   ck_assert(role == ELM_ATSPI_ROLE_ENTRY);

   elm_shutdown();
}
END_TEST

void elm_test_entry(TCase *tc)
{
   tcase_add_test(tc, elm_entry_del);
   tcase_add_test(tc, elm_entry_atspi_text_char_get);
   tcase_add_test(tc, elm_entry_atspi_text_char_count);
   tcase_add_test(tc, elm_entry_atspi_text_string_get_char);
   tcase_add_test(tc, elm_entry_atspi_text_string_get_word);
   tcase_add_test(tc, elm_entry_atspi_text_string_get_paragraph);
   tcase_add_test(tc, elm_entry_atspi_text_string_get_line);
   tcase_add_test(tc, elm_entry_atspi_text_text_get);
   tcase_add_test(tc, elm_entry_atspi_text_selections);
   tcase_add_test(tc, elm_entry_atspi_text_attributes);
   tcase_add_test(tc, elm_atspi_role_get);
}
