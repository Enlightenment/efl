// Compile with:
// gcc -g `pkg-config --cflags --libs eina` eina_inlist_01.c -o eina_inlist_01
#include <Eina.h>
#include <stdio.h>

struct my_struct {
     EINA_INLIST;
     int a, b;
};

int
sort_cb(const void *d1, const void *d2)
{
   const Eina_Inlist *l1, *l2;
   const struct my_struct *x1, *x2;

   l1 = d1;
   l2 = d2;

   x1 = EINA_INLIST_CONTAINER_GET(l1, struct my_struct);
   x2 = EINA_INLIST_CONTAINER_GET(l2, struct my_struct);

   return x1->a - x2->a;
}

int
main(void)
{
   struct my_struct *d, *cur;
   Eina_Inlist *list, *itr, *tmp;

   eina_init();

   d = malloc(sizeof(*d));
   d->a = 1;
   d->b = 10;
   list = eina_inlist_append(NULL, EINA_INLIST_GET(d));

   d = malloc(sizeof(*d));
   d->a = 2;
   d->b = 20;
   list = eina_inlist_append(list, EINA_INLIST_GET(d));

   d = malloc(sizeof(*d));
   d->a = 3;
   d->b = 30;
   list = eina_inlist_prepend(list, EINA_INLIST_GET(d));

   printf("list=%p\n", list);
   EINA_INLIST_FOREACH(list, cur)
     printf("\ta=%d, b=%d\n", cur->a, cur->b);

   list = eina_inlist_promote(list, EINA_INLIST_GET(d));

   d = malloc(sizeof(*d));
   d->a = 4;
   d->b = 40;
   list = eina_inlist_append_relative(list, EINA_INLIST_GET(d), list);

   list = eina_inlist_demote(list, EINA_INLIST_GET(d));

   list = eina_inlist_sort(list, sort_cb);

   printf("list after sort=%p\n", list);
   EINA_INLIST_FOREACH(list, cur)
     printf("\ta=%d, b=%d\n", cur->a, cur->b);

   tmp = eina_inlist_find(list, EINA_INLIST_GET(d));
   if (tmp)
     cur = EINA_INLIST_CONTAINER_GET(tmp, struct my_struct);
   else
     cur = NULL;

   if (d != cur)
     printf("wrong node! cur=%p\n", cur);

   list = eina_inlist_remove(list, EINA_INLIST_GET(d));
   free(d);
   printf("list=%p\n", list);
   for (itr = list; itr != NULL; itr = itr->next)
     {
        cur = EINA_INLIST_CONTAINER_GET(itr, struct my_struct);
        printf("\ta=%d, b=%d\n", cur->a, cur->b);
     }

   while (list)
     {
        Eina_Inlist *aux = list;
        list = eina_inlist_remove(list, list);
        free(aux);
     }

   eina_shutdown();

   return 0;
}
