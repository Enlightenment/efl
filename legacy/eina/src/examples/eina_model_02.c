//Compile with:
//gcc -g eina_model_02.c -o eina_model_02 `pkg-config --cflags --libs eina`

#include <Eina.h>

static void _cb_on_deleted(void *data, Eina_Model *model, const Eina_Model_Event_Description *desc, void *event_info);

int main(void)
{
   Eina_Model *m;
   char *s;
   int i;

   eina_init();

   m = eina_model_new(EINA_MODEL_TYPE_GENERIC);

   eina_model_event_callback_add(m, "deleted", _cb_on_deleted, NULL);

   //Adding properties to model
   for (i = 0; i < 5; i++)
     {
        Eina_Value val;
        char name[2] = {'a'+ i, 0};
        eina_value_setup(&val, EINA_VALUE_TYPE_INT);
        eina_value_set(&val, i);
        eina_model_property_set(m, name, &val);
        eina_value_flush(&val);
     }

   //Adding children to model
   for (i = 0; i < 5; i++)
     {
        Eina_Value val;
        Eina_Model *c = eina_model_new(EINA_MODEL_TYPE_GENERIC);
        eina_value_setup(&val, EINA_VALUE_TYPE_INT);
        eina_value_set(&val, i);
        eina_model_property_set(c, "x", &val);

        eina_model_event_callback_add(c, "deleted", _cb_on_deleted, NULL);

        eina_model_child_append(m, c);
        //Now that the child has been appended to a model, it's parent will manage it's lifecycle
        eina_model_unref(c);
        eina_value_flush(&val);
     }

   s = eina_model_to_string(m);
   printf("model as string:\n%s\n", s);

   free(s);
   eina_model_unref(m);
   eina_shutdown();

   return 0;
}

static void _cb_on_deleted(void *data, Eina_Model *model, const Eina_Model_Event_Description *desc, void *event_info)
{
   printf("deleted %p\n", model);
}
