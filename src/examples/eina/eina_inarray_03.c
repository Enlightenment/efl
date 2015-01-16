//Compile with:
//gcc -g eina_inarray_03.c -o eina_inarray_03 `pkg-config --cflags --libs eina`

#include <Eina.h>

int
cmp(const void *a, const void *b)
{
   return *(int*)a > *(int*)b;
}

int main(int argc EINA_UNUSED, char **argv EINA_UNUSED)
{
   Eina_Inarray *iarr;
   int a, *b;

   eina_init();
   iarr = eina_inarray_new(sizeof(int), 0);

   a = 1;
   eina_inarray_push(iarr, &a);
   a = 9;
   eina_inarray_push(iarr, &a);
   a = 6;
   eina_inarray_push(iarr, &a);
   a = 4;
   eina_inarray_push(iarr, &a);
   a = 10;
   eina_inarray_push(iarr, &a);

   printf("Inline array of integers with %d elements:\n", eina_inarray_count(iarr));
   EINA_INARRAY_FOREACH(iarr, b)
     printf("int: %d(pointer: %p)\n", *b, b);

   a = 8;
   eina_inarray_insert(iarr, &a, cmp);
   printf("Inserting %d to inline array using eina_inarray_insert.\n", a);
   EINA_INARRAY_FOREACH(iarr, b)
     printf("int: %d(pointer: %p)\n", *b, b);

   eina_inarray_remove(iarr, &a);
   printf("Removed %d from inline array using eina_inarray_remove.\n", a);
   EINA_INARRAY_FOREACH(iarr, b)
     printf("int: %d(pointer: %p)\n", *b, b);

   eina_inarray_insert_sorted(iarr, &a, cmp);
   printf("Inserting %d to inline array using eina_inarray_insert_sorted.\n",a);
   EINA_INARRAY_FOREACH(iarr, b)
     printf("int: %d(pointer: %p)\n", *b, b);

   printf("Position of element %d in the inline array is %d\n", a, eina_inarray_search(iarr, &a, cmp));

   eina_inarray_sort(iarr, cmp);
   printf("Sorted inline array:\n");
   EINA_INARRAY_FOREACH(iarr, b)
     printf("int: %d(pointer: %p)\n", *b, b);

   printf("Position of element %d in the sorted inline array is %d\n", a, eina_inarray_search_sorted(iarr, &a, cmp));

   eina_inarray_free(iarr);
   eina_shutdown();
}
