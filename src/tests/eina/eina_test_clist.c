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

static void add_string_after(const char *str_to_add)
{
  struct test_string *entry_to_add;

  entry_to_add = malloc(sizeof(struct test_string));
  assert(entry_to_add != NULL);

  entry_to_add->string = str_to_add;

  eina_clist_add_after(&string_list, &entry_to_add->entry);
}

static void add_string_head(const char *string_to_add)
{
  struct test_string *entry_to_add;

  entry_to_add = malloc(sizeof(struct test_string));
  assert(entry_to_add != NULL);

  entry_to_add->string = string_to_add;

  eina_clist_add_head(&string_list, &entry_to_add->entry);
  fail_if(eina_clist_head(&string_list) != &entry_to_add->entry);
}

static void iterating_two_phase_with_add_head(unsigned int n, const char *str, int n_ent)
{
  int i;
  struct test_string *entries[n_ent];
  Eina_Clist *tail;

  tail = eina_clist_tail(&string_list);
  entries[0] = malloc(sizeof(struct test_string));
  assert(entries[0] != NULL);
  entries[0]->string = str;
  eina_clist_add_tail(&string_list, &entries[0]->entry);
  n++;
  fail_if(eina_clist_next(&string_list, tail) != &entries[0]->entry);
  fail_if(eina_clist_tail(&string_list) != &entries[0]->entry);
  for (i=1; i<n_ent; i++)
  {
	entries[i] = malloc(sizeof(struct test_string));
	assert(entries[i] != NULL);
	entries[i]->string = str;
	eina_clist_add_tail(&string_list, &entries[i]->entry);
	n++;
	fail_if(eina_clist_count(&string_list) != n);
	fail_if(eina_clist_tail(&string_list) != &entries[i]->entry);
	fail_if(eina_clist_next(&string_list, &entries[i-1]->entry) != &entries[i]->entry);
  };
}

EFL_START_TEST(eina_clist_basic)
{
  unsigned int n = 0;

  fail_if(!eina_clist_empty(&string_list));

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

  add_string_after("one\n");
  n++;
  add_string_after("two\n");
  n++;
  add_string_after("three\n");
  n++;
  add_string_after("four\n");
  n++;
  add_string_after("five\n");
  n++;
  add_string_after("six\n");
  n++;
  add_string_after("seven\n");
  n++;
  add_string_after("eight\n");
  n++;

  fail_if(eina_clist_count(&string_list) != n);

  add_string_head("one2\n");
  n++;
  add_string_head("two2\n");
  n++;
  add_string_head("three2\n");
  n++;
  add_string_head("four2\n");
  n++;
  add_string_head("five2\n");
  n++;
  add_string_head("six2\n");
  n++;
  add_string_head("seven2\n");
  n++;
  add_string_head("eight2\n");
  n++;

  fail_if(eina_clist_count(&string_list) != n);

  iterating_two_phase_with_add_head(n, "theString", 16);

  free_list();

  fail_if(eina_clist_count(&string_list) != 0);
}
EFL_END_TEST

void
eina_test_clist(TCase *tc)
{
   tcase_add_test(tc, eina_clist_basic);
}
