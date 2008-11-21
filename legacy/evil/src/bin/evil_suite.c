#include <stdlib.h>
#include <stdio.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN

#include "Evil.h"
#include "evil_suite.h"
#include "evil_test_environment.h"
#include "evil_test_gettimeofday.h"
#include "evil_test_link.h"
#include "evil_test_memcpy.h"


typedef int(*function)(suite *s);

struct test
{
   const char *name;
   function    fct;
};

struct list
{
  void *data;
   int  succeed;
  list *next;
};

struct suite
{
   LARGE_INTEGER freq;
   LARGE_INTEGER start;
   LARGE_INTEGER end;

   list         *first;
   list         *l;
};


suite *
suite_new(void)
{
   suite *s;

   s = (suite *)malloc(sizeof(suite));
   if (!s) return NULL;

   if (!QueryPerformanceFrequency(&s->freq))
     {
        free(s);
        return NULL;
     }

   s->first = NULL;
   s->l = NULL;

   return s;
}

void
suite_del(suite *s)
{
   list *l;
   list *tmp;

   if (!s) return;

   l = s->first;
   while (l)
     {
        tmp = l->next;
        free(l->data);
        free(l);
        l = tmp;
     }

   free(s);
}

void
suite_time_start(suite *s)
{
   QueryPerformanceCounter(&s->start);
}

void
suite_time_stop(suite *s)
{
   QueryPerformanceCounter(&s->end);
}

double
suite_time_get(suite *s)
{
   return (double)(s->end.QuadPart - s->start.QuadPart) / (double)s->freq.QuadPart;
}

void
suite_test_add(suite *s, const char *name, function fct)
{
   test *t;
   list *l;

   t = (test *)malloc(sizeof(test));
   if (!t) return;

   l = (list *)malloc(sizeof(list));
   if (!l)
     {
        free(t);
        return;
     }

   t->name = name;
   t->fct = fct;

   l->data = t;
   l->succeed = 0;
   l->next = NULL;

   if (!s->first) s->first = l;

   if (!s->l)
     s->l = l;
   else
     {
        s->l->next = l;
        s->l =l;
     }
}

void
suite_run(suite *s)
{
   list *l;

   l = s->first;
   while (l)
     {
        test *t;

        t = (test *)l->data;
        l->succeed = t->fct(s);
        printf("%s test: %s\n", t->name, l->succeed ? "success" : "failure");
        l = l->next;
     }
}

int
main()
{
   test tests[] = {
     { "environment ",  test_environment },
     { "gettimeofday",  test_gettimeofday },
     { "link        ",  test_link },
     { "memcpy      ",  test_memcpy },
     { NULL,            NULL },
   };
   suite *s;
   int i;

   if (!evil_init())
     return EXIT_FAILURE;

   s = suite_new();
   if (!s)
     {
        evil_shutdown();
        return EXIT_FAILURE;
     }

   for (i = 0; tests[i].name; ++i)
     {

        suite_test_add(s, tests[i].name, tests[i].fct);
     }

   suite_run(s);

   suite_del(s);
   evil_shutdown();

   return EXIT_SUCCESS;
}
