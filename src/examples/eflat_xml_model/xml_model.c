//Compile with:
// gcc -o xml_model xml_model.c `pkg-config --cflags --libs eflat_xml_model ecore eina`

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include <Ecore.h>
#include <Eina.h>
#include <Eflat_Xml_Model.h>

#include <stdbool.h>

static const char *SAMPLE_XML =
  "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"
  "<soap:Envelope xmlns:soap=\"http://www.w3.org/2001/12/soap-envelope\" soap:encodingStyle=\"http://www.w3.org/2001/12/soap-encoding\">\n"
  "  <soap:Header>\n"
  "    <m:Trans xmlns:m=\"http://my.namespace/header\" soap:mustUnderstand=\"1\">\n"
  "      1234\n"
  "    </m:Trans>\n"
  "  </soap:Header>\n"
  "  <soap:Body>\n"
  "    <m:MethodName xmlns:m=\"http://my.namespece/body\">\n"
  "      <m:Item>\n"
  "        Value\n"
  "      </m:Item>\n"
  "    </m:MethodName>\n"
  "  </soap:Body>\n"
  "</soap:Envelope>\n";


static Eina_Bool
_event_load_status_cb(void *data EINA_UNUSED, Eo *model,
                           const Eo_Event_Description *desc EINA_UNUSED,
                           void *event_info)
{
   Efl_Model_Load *actual_load = (Efl_Model_Load*)event_info;

   if (EFL_MODEL_LOAD_STATUS_LOADED != actual_load->status)
     return EINA_TRUE;

   printf("Loaded\n");

   Eina_Array *properties_list;
   Eina_Array_Iterator iterator;
   Eina_Value const* property_value;
   char *property, *prop_str;
   unsigned int i;

   eo_do(model, efl_model_properties_get(&properties_list));
   if (!eina_array_count(properties_list))
     printf("Without properties\n");

   EINA_ARRAY_ITER_NEXT(properties_list, i, property, iterator)
     {
        prop_str = NULL;
        eo_do(model, efl_model_property_get(property, &property_value));
        if (property_value)
          prop_str = eina_value_to_string(property_value);

        printf("\nName:  %s\nValue: %s \n", property, prop_str);
        free(prop_str);
     }

   prop_str = NULL;
   eo_do(model, efl_model_property_get("soap:Envelope/", &property_value));
   if (property_value)
     prop_str = eina_value_to_string(property_value);

   printf("\n\"soap:Envelope/\" stream\n%s\n",  prop_str);
   free(prop_str);

   ecore_main_loop_quit();

   return EINA_FALSE;
}

int
main()
{
   eflat_xml_model_init();

   Efl_Model_Base *xml_model = eo_add(EFLAT_XML_MODEL_CLASS, NULL,
                                      eflat_xml_model_constructor(SAMPLE_XML));

   eo_do(xml_model, eo_event_callback_add(EFL_MODEL_BASE_EVENT_LOAD_STATUS, _event_load_status_cb, NULL));
   eo_do(xml_model, efl_model_load());

   ecore_main_loop_begin();

   eo_do(xml_model, eo_event_callback_del(EFL_MODEL_BASE_EVENT_LOAD_STATUS, _event_load_status_cb, NULL));
}

