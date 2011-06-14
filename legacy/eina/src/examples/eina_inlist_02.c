// Compile with:
// gcc -g `pkg-config --cflags --libs eina` eina_inlist_01.c -o eina_inlist_01
#include <Eina.h>
#include <stdio.h>

struct my_struct {
     EINA_INLIST;
     int a, b;
};

int
main(void)
{
   struct my_struct *d, *cur;
   int i;

   Eina_Inlist *inlist = NULL;
   Eina_List *list = NULL, *l_itr, *l_next;

   eina_init();

   for (i = 0; i < 100; i++)
     {
	d = malloc(sizeof(*d));
	d->a = i;
	d->b = i * 10;
	inlist = eina_inlist_append(inlist, EINA_INLIST_GET(d));
	if ((i % 2) == 0)
	  list = eina_list_prepend(list, d);
     }

   printf("inlist=%p\n", inlist);
   EINA_INLIST_FOREACH(inlist, cur)
     printf("\ta=%d, b=%d\n", cur->a, cur->b);

   printf("list=%p\n", list);
   EINA_LIST_FOREACH(list, l_itr, cur)
     printf("\ta=%d, b=%d\n", cur->a, cur->b);

   printf("inlist count=%d\n", eina_inlist_count(inlist));
   printf("list count=%d\n\n", eina_list_count(list));

   EINA_LIST_FOREACH_SAFE(list, l_itr, l_next, cur)
     {
	if ((cur->a % 3) == 0)
	  list = eina_list_remove_list(list, l_itr);
     }

   printf("inlist count=%d\n", eina_inlist_count(inlist));
   printf("list count=%d\n\n", eina_list_count(list));

   eina_list_free(list);

   while (inlist)
     {
        Eina_Inlist *aux = inlist;
        inlist = eina_inlist_remove(inlist, inlist);
        free(aux);
     }

   eina_shutdown();

   return 0;
}
