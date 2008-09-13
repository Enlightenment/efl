#include <stdlib.h>
#include <stdio.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN

#include "suite.h"
#include "test_memcpy.h"


typedef void(*function)(void);

struct test
{
   const char *name;
   function    fct;
};

struct list
{
  void *data;
  list *next;
};

struct suite
{
   list         *first;
   list         *l;
};


unsigned char *buf1 = NULL;
unsigned char *buf2 = NULL;
size_t         page_size = 0;


suite *
suite_new(void)
{
   suite *s;

   if (!QueryPerformanceFrequency(&freq))
     return NULL;

   s = (suite *)malloc(sizeof(suite));
   if (!s) return NULL;

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
suite_time_start()
{
   QueryPerformanceCounter(&start);
}

void
suite_time_stop()
{
   QueryPerformanceCounter(&end);
}

double
suite_time_get()
{
   return (double)(end.QuadPart - start.QuadPart) / (double)freq.QuadPart;
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
        printf("%s test\n", t->name);
        t->fct();
        l = l->next;
     }
}

int
main()
{
   test tests[] = {
     { "memcpy", test_memcpy },
     { NULL,             NULL },
   };
   suite *s;
   int i;

   page_size = 2 * getpagesize();

   buf1 = (unsigned char *)malloc(4 * getpagesize());
   if (!buf1) return EXIT_FAILURE;

   buf2 = (unsigned char *)malloc(4 * getpagesize());
   if (!buf2)
     {
        free(buf1);
        return EXIT_FAILURE;
     }

   memset (buf1, 0xa5, page_size);
   memset (buf2, 0x5a, page_size);

   s = suite_new();
   if (!s)
     {
        free(buf2);
        free(buf1);
        return EXIT_FAILURE;
     }

   for (i = 0; ; ++i)
     {
        if (!tests[i].name)
          break;

        suite_test_add(s, tests[i].name, tests[i].fct);
     }

   suite_run(s);

   suite_del(s);
   free(buf2);
   free(buf1);

   return EXIT_SUCCESS;
}
