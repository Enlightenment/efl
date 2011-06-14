// Compile with:
// gcc -g `pkg-config --cflags --libs eina` eina_inlist_01.c -o eina_inlist_01
#include <Eina.h>
#include <stdio.h>

struct my_struct {
     EINA_INLIST;
     Eina_Inlist even;
     int a, b;
};

#define EVEN_INLIST_GET(Inlist) (& ((Inlist)->even))

#define EVEN_INLIST_CONTAINER_GET(ptr, type) \
   ((type *)((char *)ptr - offsetof(type, even)))

int
main(void)
{
   struct my_struct *d, *cur;
   int i;

   Eina_Inlist *list = NULL, *list_even = NULL, *itr;

   eina_init();

   for (i = 0; i < 100; i++)
     {
	d = malloc(sizeof(*d));
	d->a = i;
	d->b = i * 10;
	list = eina_inlist_append(list, EINA_INLIST_GET(d));
	if ((i % 2) == 0)
	  list_even = eina_inlist_prepend(list_even, EVEN_INLIST_GET(d));
     }

   printf("list=%p\n", list);
   EINA_INLIST_FOREACH(list, cur)
     printf("\ta=%d, b=%d\n", cur->a, cur->b);

   printf("list_even=%p\n", list_even);
   for (itr = list_even; itr != NULL; itr = itr->next)
     {
	cur = EVEN_INLIST_CONTAINER_GET(itr, struct my_struct);
	printf("\ta=%d, b=%d\n", cur->a, cur->b);
     }

   printf("list count=%d\n", eina_inlist_count(list));
   printf("list_even count=%d\n\n", eina_inlist_count(list_even));

   itr = list_even;
   while (itr)
     {
	Eina_Inlist *next = itr->next;
	cur = EVEN_INLIST_CONTAINER_GET(itr, struct my_struct);
	if ((cur->a % 3) == 0)
	  list_even = eina_inlist_remove(list_even, itr);
	itr = next;
     }
   printf("list count=%d\n", eina_inlist_count(list));
   printf("list_even count=%d\n\n", eina_inlist_count(list_even));

   while (list)
     {
        Eina_Inlist *aux = list;
        list = eina_inlist_remove(list, list);
        free(aux);
     }

   eina_shutdown();

   return 0;
}
