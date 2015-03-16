//Compile with:
//gcc -g eina_inarray_01.c -o eina_inarray_01 `pkg-config --cflags --libs eina`

#include <Eina.h>
struct _Eo_Callback_Description
{
   //Eo_Callback_Description *next;


   void *func_data;
   int priority;


};
int
_eo_base_callback_priority_cmp(const void *a, const void *b)
{
   return ((struct _Eo_Callback_Description*)a)->priority > ((struct _Eo_Callback_Description*)b)->priority;
}
int
cmp(const void *a, const void *b)
{
   return *(int*)a > *(int*)b;
}
Eina_Bool
check( const void *iarr,  void *a,  void *b)
{
   return ((struct _Eo_Callback_Description*)a)->priority==2;
}

void insert( Eina_Inarray *iarr, int pr){
  struct _Eo_Callback_Description cb;

   cb.priority = pr;
   // _eo_callbacks_sorted_insert(pd, cb);
   eina_inarray_insert_sorted(iarr, &cb , _eo_base_callback_priority_cmp);

}
int main(int argc EINA_UNUSED, char **argv EINA_UNUSED)
{
   Eina_Inarray *iarr;
   char ch, *ch2;
   struct _Eo_Callback_Description a, *b;

   eina_init();
   iarr = eina_inarray_new(sizeof(struct _Eo_Callback_Description), 0);

//  struct _Eo_Callback_Description cb;

   //  cb = calloc(1, sizeof(*cb));
   //   if (!cb) return;

 //  cb.priority = 2;
   // _eo_callbacks_sorted_insert(pd, cb);
//   eina_inarray_insert_sorted(iarr, &cb , _eo_base_callback_priority_cmp);
insert(iarr, 2);
insert(iarr,  4);
insert(iarr, -1 );
  eina_inarray_foreach_remove( iarr, check, NULL);
   printf("Inline array of integers with %d elements:\n", eina_inarray_count(iarr));
   EINA_INARRAY_FOREACH(iarr, b)
     printf("int: %d(pointer: %p)\n", b->priority, b);

   eina_inarray_free(iarr);
   eina_shutdown();
}
