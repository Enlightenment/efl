#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eo.h"

#include "../eunit_tests.h"
#include "children_simple.h"


#define CHECK_ITER_DATA(iter, data, expected_data) \
   fail_if(!eina_iterator_next(iter, &(data))); \
   fail_if(expected_data != data);

int
main(int argc, char *argv[])
{
   (void) argc;
   (void) argv;
   Eina_Iterator *iter = NULL;
   void *chld;

   eo_init();

   Eo *parent = eo_add(SIMPLE_CLASS, NULL);

   Eo *child1 = eo_add(SIMPLE_CLASS, parent);
   Eo *child2 = eo_add(SIMPLE_CLASS, parent);
   Eo *child3 = eo_add(SIMPLE_CLASS, parent);

   eo_do(parent, iter = eo_children_iterator_new());
   fail_if(!iter);

   CHECK_ITER_DATA(iter, chld, child1);
   CHECK_ITER_DATA(iter, chld, child2);
   CHECK_ITER_DATA(iter, chld, child3);
   fail_if(eina_iterator_next(iter, &chld));

   eina_iterator_free(iter);

   eo_del(child2);

   eo_do(parent, iter = eo_children_iterator_new());
   fail_if(!iter);

   CHECK_ITER_DATA(iter, chld, child1);
   CHECK_ITER_DATA(iter, chld, child3);
   fail_if(eina_iterator_next(iter, &chld));

   eina_iterator_free(iter);


   eo_del(child1);

   eo_do(parent, iter = eo_children_iterator_new());
   fail_if(!iter);

   CHECK_ITER_DATA(iter, chld, child3);
   fail_if(eina_iterator_next(iter,  &chld));
   eina_iterator_free(iter);


   eo_del(child3);

   eo_do(parent, iter = eo_children_iterator_new());
   fail_if(iter);

   eo_unref(parent);

   eo_shutdown();
   return 0;
}
