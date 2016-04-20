//Compile with:
// gcc -o busmodel busmodel.c `pkg-config --cflags --libs eldbus ecore eina`

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Eldbus.h>
#include <Eldbus_Model.h>
#include <Efl.h>
#include <Ecore.h>

#define DEFAULT_BUS  "org.freedesktop.DBus"
#define DEFAULT_PATH "/"

static int prop_count = 0;

struct eina_iterator
{
  Eina_Iterator* success_iterator;
  Eina_Iterator* failure_iterator;
};

static void
promise_then_prop_c(Eo* obj, struct eina_iterator* it_struct)
{
   Eina_Value * property_value;
   const Eina_Array *properties_list;
   Eina_Array_Iterator a_it;
   char *property, *prop_str;
   const char *name;
   Eina_Iterator* it = it_struct->success_iterator;

   name = eldbus_model_proxy_name_get(obj);
   properties_list = efl_model_properties_get(obj);

   printf(" -> %s\n   Properties:\n", name);
   unsigned i = 0;
   EINA_ARRAY_ITER_NEXT(properties_list, i, property, a_it)
     {
        if (eina_iterator_next(it, (void **)&property_value) && property_value)
          {
             prop_str = eina_value_to_string(property_value);
             printf("    * %s=%s \n", property, prop_str);
             free(prop_str);
          }
     }

   prop_count--;
   if (prop_count == 0)
     ecore_main_loop_quit();
}

static void
error_cb(void* data EINA_UNUSED, const Eina_Error *error EINA_UNUSED)
{
   printf(" ERROR\n");
   ecore_main_loop_quit();
}

static void
promise_then_a(Eo* obj EINA_UNUSED, Eina_Accessor **accessor)
{
   const Eina_Array *properties_list;
   Eina_Array_Iterator a_it;
   Eina_Promise **promises;
   const char *name;
   char *property;
   Eo* child;
   int i = 0;

   EINA_ACCESSOR_FOREACH(*accessor, i, child)
     {
        properties_list = efl_model_properties_get(child);
        name = eldbus_model_proxy_name_get(child);

        unsigned p_count = eina_array_count(properties_list);

        if (p_count)
          {
             promises = (Eina_Promise **)calloc(p_count + 1, sizeof(Eina_Promise *));
             promises[p_count] = NULL;

             unsigned j = 0;
             EINA_ARRAY_ITER_NEXT(properties_list, j, property, a_it)
               {
                  efl_model_property_get(child, property, &promises[j]);
               }
             eina_promise_then(eina_promise_all(eina_carray_iterator_new((void **)promises)),
                                                         (Eina_Promise_Cb)&promise_then_prop_c, &error_cb, child);
             prop_count++;
          }
        else
          {
             printf(" -> %s\n", name);
          }
     }

   if (prop_count == 0)
     ecore_main_loop_quit();
}
static void
promise_then(Eo* obj EINA_UNUSED, struct eina_iterator* it_struct)
{
   Eina_Accessor **accessor;
   unsigned int* count;

   Eina_Iterator* iterator = it_struct->success_iterator;

   if (!eina_iterator_next(iterator, (void **)&accessor))
     {
        printf("bye\n");
        ecore_main_loop_quit();
        return;
     }

   eina_iterator_next(iterator, (void **)&count);

   printf("efl_model_loaded count %d\n", (int)*count); fflush(stdout);
   printf("efl_model_loaded accessor %p\n", accessor); fflush(stdout);

   promise_then_a(NULL, accessor);
}

int
main(int argc, char **argv EINA_UNUSED)
{
   const char *bus, *path;
   Eo *root;

   ecore_init();
   eldbus_init();

   bus = DEFAULT_BUS;
   path = DEFAULT_PATH;

   if (argc > 1) bus = argv[1];
   if (argc > 2) path = argv[2];

   root = eo_add_ref(ELDBUS_MODEL_OBJECT_CLASS, NULL, eldbus_model_object_constructor(eo_self, ELDBUS_CONNECTION_TYPE_SESSION, NULL, EINA_FALSE, bus, path));

   Eina_Promise *promises[] = { NULL, NULL, NULL};
   efl_model_children_slice_get(root, 0, 0, &promises[0]);
   efl_model_children_count_get(root, &promises[1]);

   eina_promise_then(eina_promise_all(eina_carray_iterator_new((void **)promises)),
                      (Eina_Promise_Cb)&promise_then, &error_cb, root);

   ecore_main_loop_begin();

   eo_unref(root);
   eldbus_shutdown();
}
