//Compile with:
// gcc -o busmodel busmodel.c `pkg-config --cflags --libs eldbus ecore eina`

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Eldbus.h>
#include <Eldbus_Model.h>
#include <Efl.h>
#include <Ecore.h>
#include <Ecore_Getopt.h>

#define DEFAULT_BUS_NAME  "org.freedesktop.DBus"
#define DEFAULT_PATH "/"

static int prop_count = 0;
static int retval = EXIT_SUCCESS;
static Eina_Bool quit_on_done = EINA_TRUE;

static void
future_properties_then(void* data, const Efl_Event *event)
{
   Eo* obj = data;
   Efl_Future_Event_Success *future = event->info;
   Eina_Value * property_value;
   const Eina_Array *properties_list;
   Eina_Array_Iterator a_it;
   char *property, *prop_str;
   const char *name;
   Eina_Iterator* it = future->value;

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
     {
        if (quit_on_done)
          ecore_main_loop_quit();
        else
          printf("monitoring events...\n");
     }
}

static void
error_cb(void* data EINA_UNUSED, const Efl_Event *event)
{
   Efl_Future_Event_Failure *future = event->info;
   printf(" ERROR: #%d '%s'\n", future->error, eina_error_msg_get(future->error));
   ecore_main_loop_quit();
   retval = EXIT_FAILURE;
}

static void
loop_children(Eina_Accessor *children)
{
   Eo* child;
   int i = 0;

   EINA_ACCESSOR_FOREACH(children, i, child)
     {
        const Eina_Array *properties_list = efl_model_properties_get(child);
        unsigned p_count = eina_array_count(properties_list);
        const char *name = eldbus_model_proxy_name_get(child);

        if (p_count)
          {
             Efl_Future **futures = calloc(p_count + 1, sizeof(Efl_Future *));
             Eina_Array_Iterator a_it;
             const char *property;
             unsigned j = 0;

             EINA_ARRAY_ITER_NEXT(properties_list, j, property, a_it)
               {
                  futures[j] = efl_model_property_get(child, property);
               }
             efl_future_then(efl_future_iterator_all(eina_carray_iterator_new((void **)futures)),
                             &future_properties_then, &error_cb, NULL, child);
             prop_count++;
             free(futures);
             printf(" %2d: %s (loading %u properties asynchronously)\n", i, name, j);
          }
        else
          {
             printf(" %2d: %s (no properties yet)\n", i, name);
          }
     }

   if (prop_count == 0)
     {
        if (quit_on_done)
          ecore_main_loop_quit();
        else
          printf("monitoring events...\n");
     }
}

static void
future_then(void* obj EINA_UNUSED, const Efl_Event *event)
{
   Efl_Future_Event_Success *future = event->info;
   Eina_Accessor *values = future->value;
   Eina_Accessor *children;
   unsigned int *count;

   if (!eina_accessor_data_get(values, 0, (void**)&children))
     {
        fprintf(stderr, "ERROR: missing future fulfillment value #0\n");
        retval = EXIT_FAILURE;
        ecore_main_loop_quit();
        return;
     }

   if (!eina_accessor_data_get(values, 1, (void**)&count))
     {
        fprintf(stderr, "ERROR: missing future fulfillment value #1\n");
        retval = EXIT_FAILURE;
        ecore_main_loop_quit();
        return;
     }

   printf("efl_model_loaded count %u\n", *count);
   loop_children(children);
}

static void
_on_properties_changed(void *data EINA_UNUSED, const Efl_Event *event)
{
   Efl_Model_Property_Event *ev = event->info;
   Eina_Array_Iterator it;
   const char *str;
   unsigned int i;

   printf("Properties changed:\n");
   EINA_ARRAY_ITER_NEXT(ev->changed_properties, i, str, it)
     printf("\t%s\n", str);

   printf("Properties invalidated:\n");
   EINA_ARRAY_ITER_NEXT(ev->invalidated_properties, i, str, it)
     printf("\t%s\n", str);
}

static void
_on_child_added(void *data EINA_UNUSED, const Efl_Event *event)
{
   Eo *child = event->info;
   printf("Children Added: %p\n", child);
}

static void
_on_child_removed(void *data EINA_UNUSED, const Efl_Event *event)
{
   Eo *child = event->info;
   printf("Children Removed: %p\n", child);
}

EFL_CALLBACKS_ARRAY_DEFINE(event_cbs,
                           { EFL_MODEL_EVENT_PROPERTIES_CHANGED, _on_properties_changed },
                           { EFL_MODEL_EVENT_CHILD_ADDED, _on_child_added },
                           { EFL_MODEL_EVENT_CHILD_REMOVED, _on_child_removed });

static const Ecore_Getopt options = {
  "dbusmodel", /* program name */
  NULL, /* usage line */
  "1", /* version */
  "(C) 2016 Enlightenment Project", /* copyright */
  "BSD 2-Clause", /* license */
  /* long description, may be multiline and contain \n */
  "Example of Eldbus.Model.Object to fetch children and properties.\n",
  EINA_FALSE,
  {
    ECORE_GETOPT_STORE_TRUE('s', "system", "connect to the system bus, not user session."),
    ECORE_GETOPT_STORE_FALSE('w', "wait", "after done, wait for events (monitoring)"),

    ECORE_GETOPT_VERSION('V', "version"),
    ECORE_GETOPT_COPYRIGHT('C', "copyright"),
    ECORE_GETOPT_LICENSE('L', "license"),
    ECORE_GETOPT_HELP('h', "help"),

    ECORE_GETOPT_STORE_METAVAR_STR(0, NULL, "The bus name to connect.", "bus_name"),
    ECORE_GETOPT_STORE_METAVAR_STR(0, NULL, "The path to explore.", "path"),

    ECORE_GETOPT_SENTINEL
  }
};

int
main(int argc, char **argv EINA_UNUSED)
{
   Eldbus_Connection_Type conn_type;
   Eina_Bool is_system = EINA_FALSE;
   char *bus_name = DEFAULT_BUS_NAME;
   char *path = DEFAULT_PATH;
   Eina_Bool quit_option = EINA_FALSE;
   Ecore_Getopt_Value values[] = {
     ECORE_GETOPT_VALUE_BOOL(is_system),
     ECORE_GETOPT_VALUE_BOOL(quit_on_done),

     /* standard block to provide version, copyright, license and help */
     ECORE_GETOPT_VALUE_BOOL(quit_option), /* -V/--version quits */
     ECORE_GETOPT_VALUE_BOOL(quit_option), /* -C/--copyright quits */
     ECORE_GETOPT_VALUE_BOOL(quit_option), /* -L/--license quits */
     ECORE_GETOPT_VALUE_BOOL(quit_option), /* -h/--help quits */

     /* positional argument */
     ECORE_GETOPT_VALUE_STR(bus_name),
     ECORE_GETOPT_VALUE_STR(path),

     ECORE_GETOPT_VALUE_NONE /* sentinel */
   };
   int args;
   Eo *root;

   ecore_init();
   eldbus_init();

   args = ecore_getopt_parse(&options, values, argc, argv);
   if (args < 0)
     {
        fputs("ERROR: Could not parse command line options.\n", stderr);
        retval = EXIT_FAILURE;
        goto end;
     }

   if (quit_option) goto end;

   args = ecore_getopt_parse_positional(&options, values, argc, argv, args);
   if (args < 0)
     {
        fputs("ERROR: Could not parse positional arguments.\n", stderr);
        retval = EXIT_FAILURE;
        goto end;
     }

   conn_type = (is_system ?
                ELDBUS_CONNECTION_TYPE_SYSTEM :
                ELDBUS_CONNECTION_TYPE_SESSION);

   root = efl_add_ref(ELDBUS_MODEL_OBJECT_CLASS, ecore_main_loop_get(),
                      eldbus_model_object_constructor(efl_added, conn_type, NULL, EINA_FALSE, bus_name, path),
                      efl_event_callback_array_add(efl_added, event_cbs(), NULL));

   efl_future_then(efl_future_all(efl_model_children_slice_get(root, 0, 0),
                                  efl_model_children_count_get(root)),
                   &future_then, &error_cb, NULL, root);

   ecore_main_loop_begin();
   efl_del(root);

 end:
   eldbus_shutdown();
   ecore_shutdown();
   return retval;
}
