#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef HAVE_TIZEN_CONTACTS_SERVICE
#include "ecordova_contacts_private.h"
#include "ecordova_contact_private.h"
#include "ecordova_contacts_record_utils.h"

#include <contacts.h>

#include <stdint.h>
#include <stdlib.h>

#define MY_CLASS ECORDOVA_CONTACTS_CLASS
#define MY_CLASS_NAME "Ecordova_Contacts"

typedef enum Contacts_Search_Id {
  SEARCH_CONTACT,
  SEARCH_ADDRESS,
  SEARCH_EVENT,
  SEARCH_COMPANY,
  SEARCH_EMAIL,
  SEARCH_NAME,
  SEARCH_NICKNAME,
  SEARCH_MESSENGER,
  SEARCH_NOTE,
  SEARCH_NUMBER,
  SEARCH_URL,
  SEARCH_IMAGE,
  SEARCH_ID_COUNT
} Contacts_Search_Id;

static contacts_filter_h _filter_get(contacts_filter_h *, const char *);
static contacts_list_h _get_records(const Eina_List *, const Ecordova_Contacts_FindOptions *);
static Eina_Bool _search_records(contacts_filter_h, const char *, contacts_list_h *, Eina_Bool);
static Eina_Bool _populate_list(contacts_list_h, Eina_Hash *, contacts_filter_h, const char *, int, Eina_Bool);

static Eo_Base *
_ecordova_contacts_eo_base_constructor(Eo *obj, Ecordova_Contacts_Data *pd)
{
   DBG("(%p)", obj);

   pd->obj = obj;

   return eo_do_super_ret(obj, MY_CLASS, obj, eo_constructor());
}

static void
_ecordova_contacts_eo_base_destructor(Eo *obj,
                                      Ecordova_Contacts_Data *pd EINA_UNUSED)
{
   DBG("(%p)", obj);

   eo_do_super(obj, MY_CLASS, eo_destructor());
}

static void
_ecordova_contacts_find(Eo *obj,
                        Ecordova_Contacts_Data *pd EINA_UNUSED,
                        const Eina_List *fields,
                        const Ecordova_Contacts_FindOptions *options)
{
   DBG("(%p)", obj);
   if (!fields)
     {
        Ecordova_Contacts_Error error = ECORDOVA_CONTACTS_ERROR_INVALID_ARGUMENT_ERROR;
        eo_do(obj, eo_event_callback_call(ECORDOVA_CONTACTS_EVENT_ERROR, &error));
        return;
     }

   // TODO: load all records in a background thread

   contacts_list_h list = NULL;

   const char *field = eina_list_data_get(fields);
   if (strcmp("*", field) == 0)
     {
        int ret = contacts_db_get_all_records(_contacts_contact._uri, 0, 0, &list);
        if (CONTACTS_ERROR_NONE != ret)
          {
             ERR("%s returned %d", "contacts_db_get_all_records", ret);
             Ecordova_Contacts_Error error = ECORDOVA_CONTACTS_ERROR_UNKNOWN_ERROR;
             eo_do(obj, eo_event_callback_call(ECORDOVA_CONTACTS_EVENT_ERROR, &error));
             return;
          }
     }
   else
     {
        if (!options)
          {
             ERR("%s", "options cannot be NULL");
             Ecordova_Contacts_Error error = ECORDOVA_CONTACTS_ERROR_INVALID_ARGUMENT_ERROR;
             eo_do(obj, eo_event_callback_call(ECORDOVA_CONTACTS_EVENT_ERROR, &error));
             return;
          }

        if (!options->filter)
          {
             ERR("%s", "options.filter cannot be NULL");
             Ecordova_Contacts_Error error = ECORDOVA_CONTACTS_ERROR_INVALID_ARGUMENT_ERROR;
             eo_do(obj, eo_event_callback_call(ECORDOVA_CONTACTS_EVENT_ERROR, &error));
             return;
          }

        list = _get_records(fields, options);
        if (!list)
          {
             Ecordova_Contacts_Error error = ECORDOVA_CONTACTS_ERROR_INVALID_ARGUMENT_ERROR;
             eo_do(obj, eo_event_callback_call(ECORDOVA_CONTACTS_EVENT_ERROR, &error));
             return;
          }
     }

   Eina_Array *contacts = eina_array_new(1);
   do
     {
        contacts_record_h record = NULL;
        int ret = contacts_list_get_current_record_p(list, &record);
        if (CONTACTS_ERROR_NONE != ret || !record)
          {
             WRN("contacts_list_get_current_record_p returned %d", ret);
             break;
          }

        Ecordova_Contact *contact = eo_add(ECORDOVA_CONTACT_CLASS, NULL);
        if (!ecordova_contact_import(contact, record))
          {
             WRN("%s", "Error importing contact record");
             eo_unref(contact);
             continue;
          }

        eina_array_push(contacts, contact);

     } while (contacts_list_next(list) == CONTACTS_ERROR_NONE);

   eo_do(obj, eo_event_callback_call(ECORDOVA_CONTACTS_EVENT_FIND_SUCCESS, contacts));

   int ret;
   size_t i;
   Ecordova_Contact *contact;
   Eina_Array_Iterator it;

   EINA_ARRAY_ITER_NEXT(contacts, i, contact, it)
     eo_unref(contact);
   eina_array_free(contacts);

   ret = contacts_list_destroy(list, EINA_TRUE);
   EINA_SAFETY_ON_FALSE_RETURN(CONTACTS_ERROR_NONE == ret);
}

static void
_ecordova_contacts_contact_pick(Eo *obj EINA_UNUSED,
                                Ecordova_Contacts_Data *pd EINA_UNUSED)
{
   // TODO: pick_contact UI
}

static Ecordova_Contact *
_ecordova_contacts_create(Eo *obj EINA_UNUSED,
                          Ecordova_Contacts_Data *pd EINA_UNUSED,
                          Ecordova_Contact *other EINA_UNUSED)
{
   // TODO: clone 'other' if it's not NULL
   return eo_add(ECORDOVA_CONTACT_CLASS, NULL);
}

static contacts_filter_h
_filter_get(contacts_filter_h *filter, const char *uri)
{
   if (NULL == *filter)
     {
        int ret = contacts_filter_create(uri, filter);
        EINA_SAFETY_ON_FALSE_RETURN_VAL(CONTACTS_ERROR_NONE == ret, NULL);
     }
   else
     {
        int ret = contacts_filter_add_operator(*filter, CONTACTS_FILTER_OPERATOR_OR);
        EINA_SAFETY_ON_FALSE_RETURN_VAL(CONTACTS_ERROR_NONE == ret, NULL);
     }

   return *filter;
}

static contacts_list_h
_get_records(const Eina_List *fields,
             const Ecordova_Contacts_FindOptions *options)
{
   contacts_list_h list = NULL;

   typedef struct
   {
     const char *field;
     unsigned int property_id;
     enum {INT_FIELD, STRING_FIELD, BIRTHDAY_FIELD, ADDRESSES_FIELD} type;
     Contacts_Search_Id search_id;
   } Search_Metadata;

   Search_Metadata search_metadata[] = {
     {"id",               _contacts_contact.id,           INT_FIELD, SEARCH_CONTACT},
     {"display_name",     _contacts_contact.display_name, STRING_FIELD, SEARCH_CONTACT},
     {"formatted",        _contacts_contact.display_name, STRING_FIELD, SEARCH_CONTACT},
     {"name",             _contacts_contact.display_name, STRING_FIELD, SEARCH_CONTACT},
     {"given_name",       _contacts_name.first,           STRING_FIELD, SEARCH_NAME},
     {"family_name",      _contacts_name.last,            STRING_FIELD, SEARCH_NAME},
     {"honorific_prefix", _contacts_name.prefix,          STRING_FIELD, SEARCH_NAME},
     {"honorific_suffix", _contacts_name.suffix,          STRING_FIELD, SEARCH_NAME},
     {"middle_name",      _contacts_name.addition,        STRING_FIELD, SEARCH_NAME},
     {"country",          _contacts_address.country,      STRING_FIELD, SEARCH_ADDRESS},
     {"locality",         _contacts_address.locality,     STRING_FIELD, SEARCH_ADDRESS},
     {"postal_code",      _contacts_address.postal_code,  STRING_FIELD, SEARCH_ADDRESS},
     {"region",           _contacts_address.region,       STRING_FIELD, SEARCH_ADDRESS},
     {"street_address",   _contacts_address.street,       STRING_FIELD, SEARCH_ADDRESS},
     {"phone_numbers",    _contacts_number.number,        STRING_FIELD, SEARCH_NUMBER},
     {"emails",           _contacts_email.email,          STRING_FIELD, SEARCH_EMAIL},
     {"urls",             _contacts_url.url,              STRING_FIELD, SEARCH_URL},
     {"note",             _contacts_note.note,            STRING_FIELD, SEARCH_NOTE},
     {"ims",              _contacts_messenger.im_id,      STRING_FIELD, SEARCH_MESSENGER},
     {"nickname",         _contacts_nickname.name,        STRING_FIELD, SEARCH_NICKNAME},
     {"department",       _contacts_company.department,   STRING_FIELD, SEARCH_COMPANY},
     {"organizations",    _contacts_company.name,         STRING_FIELD, SEARCH_COMPANY},
     {"title",            _contacts_company.job_title,    STRING_FIELD, SEARCH_COMPANY},
     {"photos",           _contacts_image.path,           STRING_FIELD, SEARCH_IMAGE},
     {"birthday",         _contacts_event.date,           BIRTHDAY_FIELD, SEARCH_EVENT},
     {"addresses",        0,                              ADDRESSES_FIELD, SEARCH_ADDRESS},
     // TODO: categories:     "categories",       ?
     {0}
   };

   const char *uri[SEARCH_ID_COUNT] = {
     [SEARCH_CONTACT]   = _contacts_contact._uri,
     [SEARCH_ADDRESS]   = _contacts_address._uri,
     [SEARCH_EVENT]     = _contacts_event._uri,
     [SEARCH_COMPANY]   = _contacts_company._uri,
     [SEARCH_EMAIL]     = _contacts_email._uri,
     [SEARCH_NAME]      = _contacts_name._uri,
     [SEARCH_NICKNAME]  = _contacts_nickname._uri,
     [SEARCH_MESSENGER] = _contacts_messenger._uri,
     [SEARCH_NOTE]      = _contacts_note._uri,
     [SEARCH_NUMBER]    = _contacts_number._uri,
     [SEARCH_URL]       = _contacts_url._uri,
     [SEARCH_IMAGE]     = _contacts_image._uri
   };

   contacts_filter_h filters[SEARCH_ID_COUNT] = {NULL};
   int ret;
   Eina_List *it;
   const char *field;
   EINA_LIST_FOREACH((Eina_List*)fields, it, field)
     {
        contacts_filter_h filter = NULL;

        int i = 0;
        Search_Metadata *metadata = NULL;
        while ((metadata = &search_metadata[i++]) && metadata->field)
          {
             if (strcmp(metadata->field, field) != 0)
               continue;

             switch (metadata->type)
               {
                case INT_FIELD:
                  {
                     int value = 0;
                     if ((sscanf(options->filter, "%d", &value) != 1) &&
                         (sscanf(options->filter, "%x", &value) != 1) &&
                         (sscanf(options->filter, "%o", &value) != 1))
                       {
                          ERR("Cannot convert options.filter to int to match the contact %s", field);
                          goto on_error_1;
                       }

                     filter = _filter_get(&filters[metadata->search_id], uri[metadata->search_id]);
                     if (!filter) goto on_error_1;

                     ret = contacts_filter_add_int(filter, metadata->property_id, CONTACTS_MATCH_EQUAL, value);
                     EINA_SAFETY_ON_FALSE_GOTO(CONTACTS_ERROR_NONE == ret, on_error_1);
                     break;
                  }
                case STRING_FIELD:
                  {
                     filter = _filter_get(&filters[metadata->search_id], uri[metadata->search_id]);
                     if (!filter) goto on_error_1;

                     ret = contacts_filter_add_str(filter, metadata->property_id, CONTACTS_MATCH_CONTAINS, options->filter);
                     EINA_SAFETY_ON_FALSE_GOTO(CONTACTS_ERROR_NONE == ret, on_error_1);
                     break;
                  }
                case BIRTHDAY_FIELD:
                  {
                     int value = 0;

                     // TODO: Convert options->filter as date string to int

                     filter = _filter_get(&filters[metadata->search_id], uri[metadata->search_id]);
                     if (!filter) goto on_error_1;

                     ret = contacts_filter_add_int(filter, metadata->property_id, CONTACTS_MATCH_EQUAL, value);
                     EINA_SAFETY_ON_FALSE_GOTO(CONTACTS_ERROR_NONE == ret, on_error_1);

                     int ret = contacts_filter_add_operator(filter, CONTACTS_FILTER_OPERATOR_AND);
                     EINA_SAFETY_ON_FALSE_GOTO(CONTACTS_ERROR_NONE == ret, on_error_1);

                     ret = contacts_filter_add_int(filter, _contacts_event.type, CONTACTS_MATCH_EQUAL, CONTACTS_EVENT_TYPE_BIRTH);
                     EINA_SAFETY_ON_FALSE_GOTO(CONTACTS_ERROR_NONE == ret, on_error_1);

                     break;
                  }
                case ADDRESSES_FIELD:
                  {
                     filter = _filter_get(&filters[metadata->search_id], uri[metadata->search_id]);
                     if (!filter) goto on_error_1;

                     ret = contacts_filter_add_str(filter, _contacts_address.country, CONTACTS_MATCH_CONTAINS, options->filter);
                     EINA_SAFETY_ON_FALSE_GOTO(CONTACTS_ERROR_NONE == ret, on_error_1);

                     ret = contacts_filter_add_operator(filter, CONTACTS_FILTER_OPERATOR_OR);
                     EINA_SAFETY_ON_FALSE_GOTO(CONTACTS_ERROR_NONE == ret, on_error_1);

                     ret = contacts_filter_add_str(filter, _contacts_address.region, CONTACTS_MATCH_CONTAINS, options->filter);
                     EINA_SAFETY_ON_FALSE_GOTO(CONTACTS_ERROR_NONE == ret, on_error_1);

                     ret = contacts_filter_add_operator(filter, CONTACTS_FILTER_OPERATOR_OR);
                     EINA_SAFETY_ON_FALSE_GOTO(CONTACTS_ERROR_NONE == ret, on_error_1);

                     ret = contacts_filter_add_str(filter, _contacts_address.locality, CONTACTS_MATCH_CONTAINS, options->filter);
                     EINA_SAFETY_ON_FALSE_GOTO(CONTACTS_ERROR_NONE == ret, on_error_1);

                     ret = contacts_filter_add_operator(filter, CONTACTS_FILTER_OPERATOR_OR);
                     EINA_SAFETY_ON_FALSE_GOTO(CONTACTS_ERROR_NONE == ret, on_error_1);

                     ret = contacts_filter_add_str(filter, _contacts_address.street, CONTACTS_MATCH_CONTAINS, options->filter);
                     EINA_SAFETY_ON_FALSE_GOTO(CONTACTS_ERROR_NONE == ret, on_error_1);

                     ret = contacts_filter_add_operator(filter, CONTACTS_FILTER_OPERATOR_OR);
                     EINA_SAFETY_ON_FALSE_GOTO(CONTACTS_ERROR_NONE == ret, on_error_1);

                     ret = contacts_filter_add_str(filter, _contacts_address.postal_code, CONTACTS_MATCH_CONTAINS, options->filter);
                     EINA_SAFETY_ON_FALSE_GOTO(CONTACTS_ERROR_NONE == ret, on_error_1);

                     break;
                  }
               }
          }
     }

   Eina_Hash *contact_ids = eina_hash_int32_new(NULL);

   if (filters[SEARCH_CONTACT])
     {
        if (!_search_records(filters[SEARCH_CONTACT], uri[SEARCH_CONTACT], &list, options->multiple))
          goto on_error_2;

        do
          {
             contacts_record_h record = NULL;
             ret = contacts_list_get_current_record_p(list, &record);
             if (CONTACTS_ERROR_NONE != ret || !record)
               {
                  WRN("contacts_list_get_current_record_p returned %d", ret);
                  break;
               }

             int id;
             Eina_Bool ok = get_int(record, _contacts_contact.id, &id);
             EINA_SAFETY_ON_FALSE_GOTO(ok, on_error_2);

             ok = eina_hash_add(contact_ids, &id, &id) == EINA_TRUE;
             EINA_SAFETY_ON_FALSE_GOTO(ok, on_error_2);

          } while (contacts_list_next(list) == CONTACTS_ERROR_NONE);

        contacts_list_first(list);
     }
   else
     {
        ret = contacts_list_create(&list);
        EINA_SAFETY_ON_FALSE_GOTO(CONTACTS_ERROR_NONE == ret, on_error_2);
     }


   for (int search_id = 0; search_id < SEARCH_ID_COUNT; ++search_id)
     {
        if (SEARCH_CONTACT == search_id || !filters[search_id])
          continue;

        if (!options->multiple)
          {
             int count = 0;
             ret = contacts_list_get_count(list, &count);
             EINA_SAFETY_ON_FALSE_GOTO(CONTACTS_ERROR_NONE == ret, on_error_2);
             if (0 != count) break;
          }

        if (!_populate_list(list, contact_ids, filters[search_id], uri[search_id], search_id, options->multiple))
          continue;
     }

on_error_2:
   eina_hash_free(contact_ids);

on_error_1:
   for (int id = 0; id < SEARCH_ID_COUNT; ++id)
     {
        if (filters[id])
          contacts_filter_destroy(filters[id]);
     }

   return list;
}

static Eina_Bool
_search_records(contacts_filter_h filter,
                const char *uri,
                contacts_list_h *list,
                Eina_Bool multiple)
{
   int ret;
   contacts_query_h query = NULL;

   ret = contacts_query_create(uri, &query);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(CONTACTS_ERROR_NONE == ret, EINA_FALSE);

   ret = contacts_query_set_filter(query, filter);
   EINA_SAFETY_ON_FALSE_GOTO(CONTACTS_ERROR_NONE == ret, on_error_1);

   ret = contacts_db_get_records_with_query(query, 0, multiple ? 0 : 1, list);
   EINA_SAFETY_ON_FALSE_GOTO(CONTACTS_ERROR_NONE == ret, on_error_1);

   ret = contacts_query_destroy(query);
   EINA_SAFETY_ON_FALSE_GOTO(CONTACTS_ERROR_NONE == ret, on_error_2);
   return EINA_TRUE;

on_error_2:
   ret = contacts_list_destroy(*list, EINA_TRUE);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(CONTACTS_ERROR_NONE == ret, EINA_FALSE);
   *list = NULL;

on_error_1:
   ret = contacts_query_destroy(query);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(CONTACTS_ERROR_NONE == ret, EINA_FALSE);
   return EINA_FALSE;
}

static Eina_Bool
_populate_list(contacts_list_h list,
               Eina_Hash *contact_ids,
               contacts_filter_h filter,
               const char *uri,
               int search_id,
               Eina_Bool multiple)
{
   contacts_list_h sub_list = NULL;
   if (!_search_records(filter, uri, &sub_list, multiple))
     return EINA_FALSE;

   unsigned int contact_id[SEARCH_ID_COUNT] = {
     [SEARCH_CONTACT]   = _contacts_contact.id,
     [SEARCH_ADDRESS]   = _contacts_address.contact_id,
     [SEARCH_EVENT]     = _contacts_event.contact_id,
     [SEARCH_COMPANY]   = _contacts_company.contact_id,
     [SEARCH_EMAIL]     = _contacts_email.contact_id,
     [SEARCH_NAME]      = _contacts_name.contact_id,
     [SEARCH_NICKNAME]  = _contacts_nickname.contact_id,
     [SEARCH_MESSENGER] = _contacts_messenger.contact_id,
     [SEARCH_NOTE]      = _contacts_note.contact_id,
     [SEARCH_NUMBER]    = _contacts_number.contact_id,
     [SEARCH_URL]       = _contacts_url.contact_id,
     [SEARCH_IMAGE]     = _contacts_image.contact_id
   };

   int ret;
   Eina_Bool result = EINA_TRUE;
   do
     {
        contacts_record_h record = NULL;
        ret = contacts_list_get_current_record_p(sub_list, &record);
        if (CONTACTS_ERROR_NONE != ret || !record)
          {
             WRN("contacts_list_get_current_record_p returned %d", ret);
             break;
          }

        int id;
        result = get_int(record, contact_id[search_id], &id);
        EINA_SAFETY_ON_FALSE_GOTO(result, on_error);

        if (eina_hash_find(contact_ids, &id))
          continue;

        contacts_record_h contact_record = NULL;
        result = contacts_db_get_record(uri, id, &contact_record) == CONTACTS_ERROR_NONE;
        EINA_SAFETY_ON_FALSE_GOTO(result, on_error);

        if (!contact_record) continue;

        result = contacts_list_add(list, contact_record) == CONTACTS_ERROR_NONE;
        EINA_SAFETY_ON_FALSE_GOTO(result, on_error);

        result = eina_hash_add(contact_ids, &id, &id) == EINA_TRUE;
        EINA_SAFETY_ON_FALSE_GOTO(result, on_error);

     } while (multiple && contacts_list_next(sub_list) == CONTACTS_ERROR_NONE);

on_error:
   ret = contacts_list_destroy(sub_list, EINA_TRUE);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(CONTACTS_ERROR_NONE == ret, EINA_FALSE);
   return result;
}

#include "ecordova_contacts.eo.c"
#endif
