#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <Eina.h>
#include "eina_suite.h"

Eina_Clist string_list = EINA_CLIST_INIT(string_list);

struct test_string
{
   Eina_Clist entry;
   const char *string;
};

static void add_string(const char *foo)
{
   struct test_string *t;

   t = malloc(sizeof *t);
   assert(t != NULL);

   t->string = foo;
   eina_clist_add_tail(&string_list, &t->entry);
}

static void print_strings(void)
{
   struct test_string *str;

   EINA_CLIST_FOR_EACH_ENTRY(str, &string_list, struct test_string, entry)
     {
        printf("%s ", str->string);
     }
   printf("\n");
}

static void free_list(void)
{
   struct test_string *str, *tmp;

   EINA_CLIST_FOR_EACH_ENTRY_SAFE(str, tmp, &string_list, struct test_string, entry)
     {
        eina_clist_remove(&str->entry);
     }
}

START_TEST(eina_clist_basic)
{
  unsigned int n = 0;

  add_string("this");
  n++;
  add_string("is");
  n++;
  add_string("a");
  n++;
  add_string("test");
  n++;
  add_string("of");
  n++;
  add_string("clists");
  n++;
  add_string("-");
  n++;
  add_string("hello");
  n++;
  add_string("world");
  n++;

  fail_if(eina_clist_count(&string_list) != n);

  print_strings();

  free_list();

  fail_if(eina_clist_count(&string_list) != 0);
}
END_TEST

void
eina_test_clist(TCase *tc)
{
   tcase_add_test(tc, eina_clist_basic);
}
