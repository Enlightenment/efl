
#include "ecordova_contacts_service.h"

#include <dlfcn.h>

// Record
int (*contacts_record_create)(const char *view_uri, contacts_record_h *record) = 0;
int (*contacts_record_destroy )(contacts_record_h record, bool delete_child) = 0;
int (*contacts_record_clone)(contacts_record_h record, contacts_record_h* cloned_record) = 0;
int (*contacts_record_get_child_record_count)(contacts_record_h record, unsigned int property_id, int *count) = 0;
int (*contacts_record_get_child_record_at_p)(contacts_record_h record, unsigned int property_id, int index, contacts_record_h* child_record) = 0;
int (*contacts_record_get_str)(contacts_record_h record, unsigned int property_id, char** value) = 0;
int (*contacts_record_get_str_p)(contacts_record_h record, unsigned int property_id, char** value) = 0;
int (*contacts_record_get_int)(contacts_record_h record, unsigned int property_id, int* value) = 0;
int (*contacts_record_set_str)(contacts_record_h record, unsigned int property_id, const char* value) = 0;
int (*contacts_record_set_int)(contacts_record_h record, unsigned int property_id, int value) = 0;
int (*contacts_record_get_bool)(contacts_record_h record, unsigned int property_id, bool *value) = 0;
int (*contacts_record_set_bool)(contacts_record_h record, unsigned int property_id, bool value) = 0;
int (*contacts_record_remove_child_record)(contacts_record_h record, unsigned int property_id, contacts_record_h child_record) = 0;
int (*contacts_record_add_child_record)(contacts_record_h record, unsigned int property_id, contacts_record_h child_record) = 0;

// DB
int (*contacts_db_delete_record)(const char* view_uri, int record_id) = 0;
int (*contacts_db_update_record)(contacts_record_h record) = 0;
int (*contacts_db_insert_record)(contacts_record_h record, int *id) = 0;
int (*contacts_db_get_record)(const char* view_uri, int record_id, contacts_record_h* record) = 0;
int (*contacts_db_get_all_records)(const char* view_uri, int offset, int limit, contacts_list_h* record_list) = 0;
int (*contacts_db_get_records_with_query)(contacts_query_h query, int offset, int limit, contacts_list_h* record_list) = 0;

// List
int (*contacts_list_get_current_record_p)(contacts_list_h contacts_list, contacts_record_h* record) = 0;
int (*contacts_list_next)(contacts_list_h contacts_list) = 0;
int (*contacts_list_destroy)(contacts_list_h contacts_list, bool delete_child) = 0;
int (*contacts_list_first)(contacts_list_h contacts_list) = 0;
int (*contacts_list_create)(contacts_list_h* contacts_list) = 0;
int (*contacts_list_get_count)(contacts_list_h contacts_list, int *count) = 0;
int (*contacts_list_add)(contacts_list_h contacts_list, contacts_record_h record) = 0;

// Filter
int (*contacts_filter_create)(const char* view_uri, contacts_filter_h* filter) = 0;
int (*contacts_filter_add_operator)(contacts_filter_h filter, contacts_filter_operator_e operator_type) = 0;
int (*contacts_filter_add_int)(contacts_filter_h filter, unsigned int property_id, contacts_match_int_flag_e match, int match_value) = 0;
int (*contacts_filter_destroy)(contacts_filter_h filter) = 0;
int (*contacts_filter_add_str)(contacts_filter_h filter, unsigned int property_id, contacts_match_str_flag_e match, const char* match_value) = 0;

// Query
int (*contacts_query_create)(const char* view_uri, contacts_query_h* query) = 0;
int (*contacts_query_set_filter)(contacts_query_h query, contacts_filter_h filter) = 0;
int (*contacts_query_destroy)(contacts_query_h query) = 0;

_contacts_address_property_ids* _contacts_address = NULL;
_contacts_contact_property_ids* _contacts_contact = NULL;
_contacts_nickname_property_ids* _contacts_nickname = NULL;
_contacts_note_property_ids* _contacts_note = NULL;
_contacts_name_property_ids* _contacts_name = NULL;
_contacts_number_property_ids* _contacts_number = NULL;
_contacts_company_property_ids* _contacts_company = NULL;
_contacts_email_property_ids* _contacts_email = NULL;
_contacts_messenger_property_ids* _contacts_messenger = NULL;
_contacts_image_property_ids* _contacts_image = NULL;
_contacts_url_property_ids* _contacts_url = NULL;
_contacts_event_property_ids* _contacts_event = NULL;

static void* contacts_service_lib;

void ecordova_contacts_service_shutdown()
{
  dlclose(contacts_service_lib);
  contacts_service_lib = NULL;
  contacts_record_destroy  = NULL;
  contacts_record_clone = NULL;
  contacts_record_get_child_record_count = NULL;
  contacts_record_get_child_record_at_p = NULL;
  contacts_record_get_str = NULL;
  contacts_record_get_str_p = NULL;
  contacts_record_get_int = NULL;
  contacts_record_set_str = NULL;
  contacts_record_set_int = NULL;
  contacts_record_get_bool = NULL;
  contacts_record_set_bool = NULL;
  contacts_record_remove_child_record = NULL;
  contacts_record_add_child_record = NULL;
  contacts_db_delete_record = NULL;
  contacts_db_update_record = NULL;
  contacts_db_insert_record = NULL;
  contacts_db_get_record = NULL;
  contacts_db_get_all_records = NULL;
  contacts_db_get_records_with_query = NULL;
  contacts_list_get_current_record_p = NULL;
  contacts_list_next = NULL;
  contacts_list_destroy = NULL;
  contacts_list_first = NULL;
  contacts_list_create = NULL;
  contacts_list_get_count = NULL;
  contacts_list_add = NULL;
  contacts_filter_create = NULL;
  contacts_filter_add_operator = NULL;
  contacts_filter_add_int = NULL;
  contacts_filter_destroy = NULL;
  contacts_filter_add_str = NULL;
  contacts_query_create = NULL;
  contacts_query_set_filter = NULL;
  contacts_query_destroy = NULL;
  _contacts_address = NULL;
  _contacts_contact = NULL;
  _contacts_nickname = NULL;
  _contacts_note = NULL;
  _contacts_name = NULL;
  _contacts_number = NULL;
  _contacts_company = NULL;
  _contacts_email = NULL;
  _contacts_messenger = NULL;
  _contacts_image = NULL;
  _contacts_url = NULL;
  _contacts_event = NULL;
}

void ecordova_contacts_service_init()
{
  if(!contacts_service_lib)
    {
       contacts_service_lib = dlopen("contacts-service2.so", RTLD_NOW);

       contacts_record_create = dlsym(contacts_service_lib, "contacts_record_create");
       if(!contacts_record_create)
         goto on_error;
       contacts_record_destroy  = dlsym(contacts_service_lib, "contacts_record_destroy ");
       if(!contacts_record_destroy )
         goto on_error;
       contacts_record_clone = dlsym(contacts_service_lib, "contacts_record_clone");
       if(!contacts_record_clone)
         goto on_error;
       contacts_record_get_child_record_count = dlsym(contacts_service_lib, "contacts_record_get_child_record_count");
       if(!contacts_record_get_child_record_count)
         goto on_error;
       contacts_record_get_child_record_at_p = dlsym(contacts_service_lib, "contacts_record_get_child_record_at_p");
       if(!contacts_record_get_child_record_at_p)
         goto on_error;
       contacts_record_get_str = dlsym(contacts_service_lib, "contacts_record_get_str");
       if(!contacts_record_get_str)
         goto on_error;
       contacts_record_get_str_p = dlsym(contacts_service_lib, "contacts_record_get_str_p");
       if(!contacts_record_get_str_p)
         goto on_error;
       contacts_record_get_int = dlsym(contacts_service_lib, "contacts_record_get_int");
       if(!contacts_record_get_int)
         goto on_error;
       contacts_record_set_str = dlsym(contacts_service_lib, "contacts_record_set_str");
       if(!contacts_record_set_str)
         goto on_error;
       contacts_record_set_int = dlsym(contacts_service_lib, "contacts_record_set_int");
       if(!contacts_record_set_int)
         goto on_error;
       contacts_record_get_bool = dlsym(contacts_service_lib, "contacts_record_get_bool");
       if(!contacts_record_get_bool)
         goto on_error;
       contacts_record_set_bool = dlsym(contacts_service_lib, "contacts_record_set_bool");
       if(!contacts_record_set_bool)
         goto on_error;
       contacts_record_remove_child_record = dlsym(contacts_service_lib, "contacts_record_remove_child_record");
       if(!contacts_record_remove_child_record)
         goto on_error;
       contacts_record_add_child_record = dlsym(contacts_service_lib, "contacts_record_add_child_record");
       if(!contacts_record_add_child_record)
         goto on_error;
       contacts_db_delete_record = dlsym(contacts_service_lib, "contacts_db_delete_record");
       if(!contacts_db_delete_record)
         goto on_error;
       contacts_db_update_record = dlsym(contacts_service_lib, "contacts_db_update_record");
       if(!contacts_db_update_record)
         goto on_error;
       contacts_db_insert_record = dlsym(contacts_service_lib, "contacts_db_insert_record");
       if(!contacts_db_insert_record)
         goto on_error;
       contacts_db_get_record = dlsym(contacts_service_lib, "contacts_db_get_record");
       if(!contacts_db_get_record)
         goto on_error;
       contacts_db_get_all_records = dlsym(contacts_service_lib, "contacts_db_get_all_records");
       if(!contacts_db_get_all_records)
         goto on_error;
       contacts_db_get_records_with_query = dlsym(contacts_service_lib, "contacts_db_get_records_with_query");
       if(!contacts_db_get_records_with_query)
         goto on_error;
       contacts_list_get_current_record_p = dlsym(contacts_service_lib, "contacts_list_get_current_record_p");
       if(!contacts_list_get_current_record_p)
         goto on_error;
       contacts_list_next = dlsym(contacts_service_lib, "contacts_list_next");
       if(!contacts_list_next)
         goto on_error;
       contacts_list_destroy = dlsym(contacts_service_lib, "contacts_list_destroy");
       if(!contacts_list_destroy)
         goto on_error;
       contacts_list_first = dlsym(contacts_service_lib, "contacts_list_first");
       if(!contacts_list_first)
         goto on_error;
       contacts_list_create = dlsym(contacts_service_lib, "contacts_list_create");
       if(!contacts_list_create)
         goto on_error;
       contacts_list_get_count = dlsym(contacts_service_lib, "contacts_list_get_count");
       if(!contacts_list_get_count)
         goto on_error;
       contacts_list_add = dlsym(contacts_service_lib, "contacts_list_add");
       if(!contacts_list_add)
         goto on_error;
       contacts_filter_create = dlsym(contacts_service_lib, "contacts_filter_create");
       if(!contacts_filter_create)
         goto on_error;
       contacts_filter_add_operator = dlsym(contacts_service_lib, "contacts_filter_add_operator");
       if(!contacts_filter_add_operator)
         goto on_error;
       contacts_filter_add_int = dlsym(contacts_service_lib, "contacts_filter_add_int");
       if(!contacts_filter_add_int)
         goto on_error;
       contacts_filter_destroy = dlsym(contacts_service_lib, "contacts_filter_destroy");
       if(!contacts_filter_destroy)
         goto on_error;
       contacts_filter_add_str = dlsym(contacts_service_lib, "contacts_filter_add_str");
       if(!contacts_filter_add_str)
         goto on_error;
       contacts_query_create = dlsym(contacts_service_lib, "contacts_query_create");
       if(!contacts_query_create)
         goto on_error;
       contacts_query_set_filter = dlsym(contacts_service_lib, "contacts_query_set_filter");
       if(!contacts_query_set_filter)
         goto on_error;
       contacts_query_destroy = dlsym(contacts_service_lib, "contacts_query_destroy");
       if(!contacts_query_destroy)
         goto on_error;
       _contacts_address = dlsym(contacts_service_lib, "_contacts_address");
       if(!_contacts_address)
         goto on_error;;
       _contacts_contact = dlsym(contacts_service_lib, "_contacts_contact");
       if(!_contacts_contact)
         goto on_error;;
       _contacts_nickname = dlsym(contacts_service_lib, "_contacts_nickname");
       if(!_contacts_nickname)
         goto on_error;;
       _contacts_note = dlsym(contacts_service_lib, "_contacts_note");
       if(!_contacts_note)
         goto on_error;;
       _contacts_name = dlsym(contacts_service_lib, "_contacts_name");
       if(!_contacts_name)
         goto on_error;;
       _contacts_number = dlsym(contacts_service_lib, "_contacts_number");
       if(!_contacts_number)
         goto on_error;;
       _contacts_company = dlsym(contacts_service_lib, "_contacts_company");
       if(!_contacts_company)
         goto on_error;;
       _contacts_email = dlsym(contacts_service_lib, "_contacts_email");
       if(!_contacts_email)
         goto on_error;;
       _contacts_messenger = dlsym(contacts_service_lib, "_contacts_messenger");
       if(!_contacts_messenger)
         goto on_error;;
       _contacts_image = dlsym(contacts_service_lib, "_contacts_image");
       if(!_contacts_image)
         goto on_error;;
       _contacts_url = dlsym(contacts_service_lib, "_contacts_url");
       if(!_contacts_url)
         goto on_error;;
       _contacts_event = dlsym(contacts_service_lib, "_contacts_event");
       if(!_contacts_event)
         goto on_error;;
    }

  return;
on_error:
  ecordova_contacts_service_shutdown();
}

