#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "esoap_model_private.h"

#include <curl/curl.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#define MY_CLASS ESOAP_MODEL_CLASS
#define MY_CLASS_NAME "Esoap_Model"

typedef struct
{
   Esoap_Model_Data  *pd;
   Eina_Strbuf       *response;
   struct curl_slist *header;
   char              *soap_action;
   CURL              *curl;
   Ecore_Thread      *thread;
} Esoap_Model_Perform_Data;

static void _properties_load(Esoap_Model_Data *);
static void _children_load(Esoap_Model_Data *);
static bool _init(Esoap_Model_Data *);
static void _clear(Esoap_Model_Data *);
static void _perform(Esoap_Model_Data *, const char *);
static void _perform_cb(Esoap_Model_Perform_Data *, Ecore_Thread *);
static void _perform_end_cb(Esoap_Model_Perform_Data *, Ecore_Thread *);
static void _perform_cancel_cb(Esoap_Model_Perform_Data *, Ecore_Thread *);
static Eina_Bool _properties_changed_cb(void *, Eo *, const Eo_Event_Description *, void *);
static size_t _write_data_cb(void *, size_t, size_t, void *);
static void _perform_data_free(Esoap_Model_Perform_Data *data);

static int _esoap_model_init_count = 0;
int _esoap_model_log_dom = -1;

EAPI int
esoap_model_init(void)
{
   if (_esoap_model_init_count++ > 0)
     return _esoap_model_init_count;

   if (!eina_init())
     {
        fputs("Esoap_Model: Unable to initialize eina\n", stderr);
        return 0;
     }

   _esoap_model_log_dom = eina_log_domain_register("Esoap_Model", EINA_COLOR_CYAN);
   if (_esoap_model_log_dom < 0)
     {
        EINA_LOG_ERR("Unable to create an 'Esoap_Model' log domain");
        _esoap_model_log_dom = -1;
        eina_shutdown();
        return 0;
     }

   if (!ecore_init())
     {
        ERR("Unable to initialize ecore");
        goto on_error;
     }

     if (!eflat_xml_model_init())
       {
          ERR("Unable to initialize eflat_xml_model");
          goto on_error;
       }

   CURLcode code = curl_global_init(CURL_GLOBAL_ALL);
   if (CURLE_OK != code)
     {
        ERR("Unable to initialize curl");
        goto on_error;
     }

   return _esoap_model_init_count;

on_error:
  eina_log_domain_unregister(_esoap_model_log_dom);
  _esoap_model_log_dom = -1;
  eina_shutdown();
  return 0;
}

EAPI int
esoap_model_shutdown(void)
{
   if (_esoap_model_init_count <= 0)
     {
        ERR("Init count not greater than 0 in shutdown.");
        _esoap_model_init_count = 0;
        return 0;
     }

   if (--_esoap_model_init_count)
     return _esoap_model_init_count;

   curl_global_cleanup();
   eflat_xml_model_shutdown();
   ecore_shutdown();
   eina_log_domain_unregister(_esoap_model_log_dom);
   _esoap_model_log_dom = -1;
   eina_shutdown();
   return 0;
}

static Eo_Base *
_esoap_model_eo_base_constructor(Eo *obj, Esoap_Model_Data *pd)
{
   DBG("(%p)", obj);

   pd->obj = obj;
   pd->load.status = EFL_MODEL_LOAD_STATUS_UNLOADED;
   pd->children_list = NULL;
   pd->xml = NULL;
   pd->url = NULL;
   pd->action = NULL;

   return eo_do_super_ret(obj, MY_CLASS, obj, eo_constructor());
}

static void
_esoap_model_url_set(Eo *obj EINA_UNUSED,
                     Esoap_Model_Data *pd EINA_UNUSED,
                     const char *url)
{
   DBG("(%p)", obj);
   EINA_SAFETY_ON_NULL_RETURN(url);

   pd->url = strdup(url);
}

static const char*
_esoap_model_url_get(Eo *obj EINA_UNUSED,
                     Esoap_Model_Data *pd EINA_UNUSED)
{
   DBG("(%p)", obj);

   return pd->url;
}

static void
_esoap_model_action_set(Eo *obj EINA_UNUSED,
                        Esoap_Model_Data *pd EINA_UNUSED,
                        const char *action)
{
   DBG("(%p)", obj);
   EINA_SAFETY_ON_NULL_RETURN(action);

   pd->action = strdup(action);
}

static const char*
_esoap_model_action_get(Eo *obj EINA_UNUSED,
                        Esoap_Model_Data *pd EINA_UNUSED)
{
   DBG("(%p)", obj);

   return pd->action;
}

static void
_esoap_model_eo_base_destructor(Eo *obj, Esoap_Model_Data *pd)
{
   DBG("(%p)", obj);

   _clear(pd);
   free(pd->url);
   free(pd->action);

   eo_do_super(obj, MY_CLASS, eo_destructor());
}

static Efl_Model_Load_Status
_esoap_model_efl_model_base_properties_get(Eo *obj EINA_UNUSED,
                                           Esoap_Model_Data *pd,
                                           Eina_Array * const* properties_array)
{
   DBG("(%p)", obj);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pd, EFL_MODEL_LOAD_STATUS_ERROR);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pd->obj, EFL_MODEL_LOAD_STATUS_ERROR);

   eo_do(pd->xml, efl_model_properties_get(properties_array));
   return pd->load.status;
}

static void
_esoap_model_efl_model_base_properties_load(Eo *obj, Esoap_Model_Data *pd)
{
   DBG("(%p)", obj);

   if (pd->load.status & EFL_MODEL_LOAD_STATUS_LOADED_PROPERTIES)
     return;

   if (!_init(pd))
     return;

   _properties_load(pd);
}

static void
_properties_load(Esoap_Model_Data *pd)
{
   DBG("(%p)", pd->obj);
   efl_model_load_set(pd->obj,
                      &pd->load,
                      EFL_MODEL_LOAD_STATUS_LOADING_PROPERTIES);
   eo_do(pd->xml, efl_model_properties_load());
   efl_model_load_set(pd->obj,
                      &pd->load,
                      EFL_MODEL_LOAD_STATUS_LOADED_PROPERTIES);
}

static Efl_Model_Load_Status
_esoap_model_efl_model_base_property_set(Eo *obj EINA_UNUSED,
                                         Esoap_Model_Data *pd,
                                         const char *property,
                                         const Eina_Value *value)
{
   Efl_Model_Load_Status status;
   eo_do(pd->xml, status = efl_model_property_set(property, value));
   EINA_SAFETY_ON_FALSE_RETURN_VAL(status != EFL_MODEL_LOAD_STATUS_ERROR, EFL_MODEL_LOAD_STATUS_ERROR);
   return pd->load.status;
}

static Efl_Model_Load_Status
_esoap_model_efl_model_base_property_get(Eo *obj EINA_UNUSED,
                                         Esoap_Model_Data *pd,
                                         const char *property,
                                         const Eina_Value **value)
{
   Efl_Model_Load_Status status;
   eo_do(pd->xml, status = efl_model_property_get(property, value));
   EINA_SAFETY_ON_FALSE_RETURN_VAL(status != EFL_MODEL_LOAD_STATUS_ERROR, EFL_MODEL_LOAD_STATUS_ERROR);
   return pd->load.status;
}

static void
_esoap_model_efl_model_base_load(Eo *obj EINA_UNUSED, Esoap_Model_Data *pd)
{
   DBG("(%p)", obj);

   if ((pd->load.status & EFL_MODEL_LOAD_STATUS_LOADED) == EFL_MODEL_LOAD_STATUS_LOADED)
     return;

   if (!_init(pd))
     return;

   if (!(pd->load.status & EFL_MODEL_LOAD_STATUS_LOADED_PROPERTIES))
     _properties_load(pd);

   if (!(pd->load.status & EFL_MODEL_LOAD_STATUS_LOADED_CHILDREN))
     _children_load(pd);
}

static Efl_Model_Load_Status
_esoap_model_efl_model_base_load_status_get(Eo *obj EINA_UNUSED,
                                            Esoap_Model_Data *pd)
{
   DBG("(%p)", obj);
   return pd->load.status;
}

static void
_esoap_model_efl_model_base_unload(Eo *obj EINA_UNUSED, Esoap_Model_Data *pd)
{
   DBG("(%p)", obj);

   _clear(pd);

   efl_model_load_set(pd->obj, &pd->load, EFL_MODEL_LOAD_STATUS_UNLOADED);
}

Eo *
_esoap_model_efl_model_base_child_add(Eo *obj, Esoap_Model_Data *pd EINA_UNUSED)
{
   DBG("(%p)", obj);
   return NULL;
}

static Efl_Model_Load_Status
_esoap_model_efl_model_base_child_del(Eo *obj,
                                      Esoap_Model_Data *pd EINA_UNUSED,
                                      Eo *child EINA_UNUSED)
{
   DBG("(%p)", obj);
   return EFL_MODEL_LOAD_STATUS_ERROR;
}

static Efl_Model_Load_Status
_esoap_model_efl_model_base_children_slice_get(Eo *obj EINA_UNUSED,
                                               Esoap_Model_Data *pd,
                                               unsigned start EINA_UNUSED,
                                               unsigned count EINA_UNUSED,
                                               Eina_Accessor **children_accessor)
{
   DBG("(%p)", obj);

   if (!(pd->load.status & EFL_MODEL_LOAD_STATUS_LOADED_CHILDREN))
     {
        WRN("(%p): Children not loaded", obj);
        *children_accessor = NULL;
        return pd->load.status;
     }

   *children_accessor = efl_model_list_slice(pd->children_list, start, count);
   return pd->load.status;
}

static Efl_Model_Load_Status
_esoap_model_efl_model_base_children_count_get(Eo *obj EINA_UNUSED,
                                               Esoap_Model_Data *pd,
                                               unsigned *children_count)
{
   DBG("(%p)", obj);
   *children_count = eina_list_count(pd->children_list);
   return pd->load.status;
}

static void
_esoap_model_efl_model_base_children_load(Eo *obj EINA_UNUSED, Esoap_Model_Data *pd)
{
   DBG("(%p)", obj);

   if (pd->load.status & EFL_MODEL_LOAD_STATUS_LOADED_CHILDREN)
     return;

   if (!_init(pd))
     return;

   _children_load(pd);
}

static void
_children_load(Esoap_Model_Data *pd)
{
   DBG("(%p)", pd->obj);

   efl_model_load_set(pd->obj, &pd->load, EFL_MODEL_LOAD_STATUS_LOADING_CHILDREN);

   const Eina_Value *xml_value;
   Efl_Model_Load_Status status;
   eo_do(pd->xml, status = efl_model_property_get("/", &xml_value));
   EINA_SAFETY_ON_FALSE_RETURN(EFL_MODEL_LOAD_STATUS_LOADED_PROPERTIES & status);

   const char *xml = NULL;
   eina_value_get(xml_value, &xml);
   _perform(pd, xml);
}

static Eina_Bool
_properties_changed_cb(void *data,
                       Eo *child EINA_UNUSED,
                       const Eo_Event_Description *desc EINA_UNUSED,
                       void *event_info)
{
   eo_do(data, eo_event_callback_call(EFL_MODEL_BASE_EVENT_PROPERTIES_CHANGED, event_info));
   return EO_CALLBACK_CONTINUE;
}

static bool
_init(Esoap_Model_Data *pd)
{
   if (pd->xml)
     return true;

   pd->xml = eo_add(EFLAT_XML_MODEL_CLASS,
                    NULL,
                    eflat_xml_model_constructor(NULL));

   eo_do(pd->xml, eo_event_callback_add(EFL_MODEL_BASE_EVENT_PROPERTIES_CHANGED,
                                        _properties_changed_cb,
                                        pd->obj));
   return true;
}

static void
_clear(Esoap_Model_Data *pd)
{
   EINA_SAFETY_ON_NULL_RETURN(pd);

   if (pd->xml)
     {
        eo_unref(pd->xml);
        pd->xml = NULL;
     }

   Eo *child;
   EINA_LIST_FREE(pd->children_list, child)
     eo_del(child);
}

static size_t
_write_data_cb(void *ptr, size_t size, size_t nmeb, void *data)
{
    Eina_Strbuf *buf = (Eina_Strbuf *)data;
    Eina_Bool ret = eina_strbuf_append_length(buf, ptr, size * nmeb);
    EINA_SAFETY_ON_FALSE_RETURN_VAL(ret, 0);
    return size * nmeb;
}

static void
_perform(Esoap_Model_Data *pd, const char *xml)
{
   DBG("(%p)", pd->obj);
   Esoap_Model_Perform_Data *data = calloc(1, sizeof(Esoap_Model_Perform_Data));
   data->pd = pd;
   data->response = eina_strbuf_new();

   const char *SOAP_ACTION = "SOAPAction:";
   const size_t soap_action_lenght = strlen(SOAP_ACTION) + strlen(pd->action) + 1;
   data->soap_action = malloc(soap_action_lenght);
   snprintf(data->soap_action, soap_action_lenght, "%s%s", SOAP_ACTION, pd->action);

   data->header = curl_slist_append(data->header, "Content-Type:application/soap+xml");
   data->header = curl_slist_append(data->header, data->soap_action);
   data->header = curl_slist_append(data->header, "Transfer-Encoding: chunked");
   data->header = curl_slist_append(data->header, "Expect:");

   data->curl = curl_easy_init();
   EINA_SAFETY_ON_NULL_RETURN(data->curl);

   curl_easy_setopt(data->curl, CURLOPT_URL, pd->url);
   curl_easy_setopt(data->curl, CURLOPT_POST, 1L);
   curl_easy_setopt(data->curl, CURLOPT_WRITEFUNCTION, _write_data_cb);
   curl_easy_setopt(data->curl, CURLOPT_WRITEDATA, data->response);
   curl_easy_setopt(data->curl, CURLOPT_HTTPHEADER, data->header);
   curl_easy_setopt(data->curl, CURLOPT_COPYPOSTFIELDS, xml);
   curl_easy_setopt(data->curl, CURLOPT_VERBOSE, 1L);

   data->thread = ecore_thread_run((Ecore_Thread_Cb)_perform_cb,
                                   (Ecore_Thread_Cb)_perform_end_cb,
                                   (Ecore_Thread_Cb)_perform_cancel_cb, data);
   EINA_SAFETY_ON_NULL_RETURN(data->thread);
}

static void
_perform_cb(Esoap_Model_Perform_Data *data, Ecore_Thread *thread)
{
   DBG("(%p)", data->pd->obj);
   CURLcode res = curl_easy_perform(data->curl);
   if (CURLE_OK != res)
     {
        ERR("Could not perform: %d", res);
        ecore_thread_cancel(thread);
     }
}

static void
_perform_end_cb(Esoap_Model_Perform_Data *data,
                Ecore_Thread *thread EINA_UNUSED)
{
   DBG("(%p)", data->pd->obj);
   const char *response = eina_strbuf_string_get(data->response);

   DBG("(%p) response: %s", data->pd->obj, response);

   Eo *child = eo_add(EFLAT_XML_MODEL_CLASS,
                      NULL,
                      eflat_xml_model_constructor(response));
   data->pd->children_list = eina_list_append(data->pd->children_list, child);

   unsigned int count = eina_list_count(data->pd->children_list);
   Efl_Model_Children_Event evt = {.child = child, .index = count};
   eo_do(data->pd->obj, eo_event_callback_call(EFL_MODEL_BASE_EVENT_CHILD_ADDED, &evt),
                        eo_event_callback_call(EFL_MODEL_BASE_EVENT_CHILDREN_COUNT_CHANGED, &count));

   efl_model_load_set(data->pd->obj, &data->pd->load, EFL_MODEL_LOAD_STATUS_LOADED_CHILDREN);

   _perform_data_free(data);
}

static void
_perform_cancel_cb(Esoap_Model_Perform_Data *data,
                   Ecore_Thread *thread EINA_UNUSED)
{
   DBG("(%p)", data->pd->obj);
   efl_model_error_notify(data->pd->obj);
   _perform_data_free(data);
}

static void
_perform_data_free(Esoap_Model_Perform_Data *data)
{
   DBG("(%p)", data->pd->obj);
   curl_easy_cleanup(data->curl);
   eina_strbuf_free(data->response);
   curl_slist_free_all(data->header);
   free(data->soap_action);
   free(data);
}

#include "esoap_model.eo.c"
