#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef HAVE_TIZEN_CONTACTS_SERVICE
#include "ecordova_contact_private.h"

#include "ecordova_contactname_private.h"
#include "ecordova_contactfield_private.h"
#include "ecordova_contacts_record_utils.h"
#include "ecordova_contactaddress_private.h"
#include "ecordova_contactorganization_private.h"

#include <contacts.h>

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#define MY_CLASS ECORDOVA_CONTACT_CLASS
#define MY_CLASS_NAME "Ecordova_Contact"

static Ecordova_ContactName *_contactname_copy(Ecordova_ContactName *);
static Eina_Array *_contactfield_array_copy(Eina_Array *);
static Eina_Array *_contactaddress_array_copy(Eina_Array *);
static Eina_Array *_contactorganization_array_copy(Eina_Array *);
static void _contactfield_array_free(Eina_Array *);
static void _contactaddress_array_free(Eina_Array *);
static void _contactorganization_array_free(Eina_Array *);
static bool _ecordova_contactfields_import(Eina_Array *, contacts_record_h, const Ecordova_ContactField_Metadata);
static bool _ecordova_contactfields_export(Eina_Array *, contacts_record_h, const Ecordova_ContactField_Metadata);
static bool _ecordova_contactaddresses_import(Eina_Array *, contacts_record_h);
static bool _ecordova_contactaddresses_export(Eina_Array *, contacts_record_h);
static bool _ecordova_contactorganizations_import(Eina_Array *, contacts_record_h);
static bool _ecordova_contactorganizations_export(Eina_Array *, contacts_record_h);
static void _contactfield_array_clear_id(Eina_Array *);
static void _contactaddress_array_clear_id(Eina_Array *);
static void _contactorganization_array_clear_id(Eina_Array *);

static Eo_Base *
_ecordova_contact_eo_base_constructor(Eo *obj, Ecordova_Contact_Data *pd)
{
   DBG("(%p)", obj);

   pd->obj = obj;
   pd->record = NULL;
   int ret = contacts_record_create(_contacts_contact._uri, &pd->record);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(CONTACTS_ERROR_NONE == ret, NULL);
   pd->name = eo_add(ECORDOVA_CONTACTNAME_CLASS, NULL);
   pd->phone_numbers = eina_array_new(1);
   pd->emails = eina_array_new(1);
   pd->addresses = eina_array_new(1);
   pd->ims = eina_array_new(1);
   pd->organizations = eina_array_new(1);
   pd->photos = eina_array_new(1);
   pd->categories = eina_array_new(1);
   pd->urls = eina_array_new(1);

   return eo_do_super_ret(obj, MY_CLASS, obj, eo_constructor());
}

static void
_ecordova_contact_eo_base_destructor(Eo *obj, Ecordova_Contact_Data *pd)
{
   DBG("(%p)", obj);

   eo_unref(pd->name);
   _contactfield_array_free(pd->phone_numbers);
   _contactfield_array_free(pd->emails);
   _contactaddress_array_free(pd->addresses);
   _contactfield_array_free(pd->ims);
   _contactorganization_array_free(pd->organizations);
   _contactfield_array_free(pd->photos);
   _contactfield_array_free(pd->categories);
   _contactfield_array_free(pd->urls);

   int ret = contacts_record_destroy(pd->record, true);
   EINA_SAFETY_ON_FALSE_RETURN(CONTACTS_ERROR_NONE == ret);

   eo_do_super(obj, MY_CLASS, eo_destructor());
}

static void
_ecordova_contact_remove(Eo *obj, Ecordova_Contact_Data *pd)
{
   if (!pd->id)
     {
        ERR("%s", "No id");
        goto on_error;
     }

   int ret = contacts_db_delete_record(_contacts_contact._uri, pd->id);
   if (CONTACTS_ERROR_NONE != ret)
     {
        ERR("Error deleting record id: %d", pd->id);
        goto on_error;
     }

   eo_do(obj, eo_event_callback_call(ECORDOVA_CONTACT_EVENT_REMOVE_SUCCESS, NULL));
   return;

on_error:
   eo_do(obj, eo_event_callback_call(ECORDOVA_CONTACT_EVENT_ERROR, NULL));
}

static Ecordova_Contact *
_ecordova_contact_clone(Eo *obj EINA_UNUSED, Ecordova_Contact_Data *pd)
{
   Ecordova_Contact *cloned = eo_add(ECORDOVA_CONTACTNAME_CLASS, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(cloned, NULL);

   if (!ecordova_contact_export(obj, pd->record))
     goto on_error;

   if (!ecordova_contact_import(cloned, pd->record))
     goto on_error;

   Ecordova_Contact_Data *cloned_pd = eo_data_scope_get(cloned, ECORDOVA_CONTACT_CLASS);
   EINA_SAFETY_ON_NULL_GOTO(cloned_pd, on_error);

   cloned_pd->id = 0;
   _contactfield_array_clear_id(pd->phone_numbers);
   _contactfield_array_clear_id(pd->emails);
   _contactaddress_array_clear_id(pd->addresses);
   _contactfield_array_clear_id(pd->ims);
   _contactorganization_array_clear_id(pd->organizations);
   _contactfield_array_clear_id(pd->photos);
   _contactfield_array_clear_id(pd->categories);
   _contactfield_array_clear_id(pd->urls);

   return cloned;

on_error:
   eo_unref(cloned);
   return NULL;
}

static void
_ecordova_contact_save(Eo *obj, Ecordova_Contact_Data *pd)
{
   int ret;

   // TODO: export records in a background thread
   if (!ecordova_contact_export(obj, pd->record))
     {
        ERR("%s", "Exporting record");
        goto on_error;
     }

   if (pd->id)
     {
        ret = contacts_db_update_record(pd->record);

        if (CONTACTS_ERROR_NONE != ret)
          {
             ERR("Error updating record: %d", ret);
             goto on_error;
          }
     }
   else
     {
        ret = contacts_db_insert_record(pd->record, &pd->id);
        if (CONTACTS_ERROR_NONE != ret)
          {
             ERR("Error inserting record: %d", ret);
             goto on_error;
          }

        // must get the inserted record so we can properly update it further
        contacts_record_h contacts_record = NULL;
        ret = contacts_db_get_record(_contacts_contact._uri, pd->id, &contacts_record);
        if (CONTACTS_ERROR_NONE != ret)
          {
             ERR("Error getting record: %d", ret);
             goto on_error;
          }

        ret = contacts_record_destroy(pd->record, true);
        if (CONTACTS_ERROR_NONE != ret)
          {
             ERR("Error destroying record: %d", ret);
             goto on_error;
          }

        pd->record = contacts_record;
     }

   // TODO: Check if it's necessary to update children records

   eo_do(obj, eo_event_callback_call(ECORDOVA_CONTACT_EVENT_SAVE_SUCCESS, NULL));
   return;

on_error:
   eo_do(obj, eo_event_callback_call(ECORDOVA_CONTACT_EVENT_ERROR, NULL));
}

static int
_ecordova_contact_id_get(Eo *obj EINA_UNUSED, Ecordova_Contact_Data *pd)
{
   return pd->id;
}

static const char *
_ecordova_contact_display_name_get(Eo *obj EINA_UNUSED,
                                   Ecordova_Contact_Data *pd)
{
   const char *value = NULL;
   get_str_p(pd->record, _contacts_contact.display_name, &value);
   return value;
}

static void
_ecordova_contact_display_name_set(Eo *obj EINA_UNUSED,
                                   Ecordova_Contact_Data *pd,
                                   const char *value)
{
   set_str(pd->record, _contacts_contact.display_name, value);
}

static Ecordova_ContactName *
_ecordova_contact_name_get(Eo *obj EINA_UNUSED, Ecordova_Contact_Data *pd)
{
   return pd->name;
}

static void
_ecordova_contact_name_set(Eo *obj EINA_UNUSED,
                           Ecordova_Contact_Data *pd,
                           Ecordova_ContactName *name)
{
   if (pd->name) eo_unref(pd->name);
   pd->name = _contactname_copy(name);
}

static const char *
_ecordova_contact_nickname_get(Eo *obj EINA_UNUSED, Ecordova_Contact_Data *pd)
{
   int ret;
   int count = 0;
   ret = contacts_record_get_child_record_count(pd->record,
                                                _contacts_contact.nickname,
                                                &count);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(CONTACTS_ERROR_NONE == ret, NULL);

   if (0 == count) return NULL;

   contacts_record_h child_record = NULL;
   ret = contacts_record_get_child_record_at_p(pd->record,
                                               _contacts_contact.nickname,
                                               0,
                                               &child_record);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(CONTACTS_ERROR_NONE == ret, NULL);

   const char *value = NULL;
   get_str_p(child_record, _contacts_nickname.name, &value);
   return value;
}

static void
_ecordova_contact_nickname_set(Eo *obj EINA_UNUSED,
                               Ecordova_Contact_Data *pd,
                               const char *value)
{
   int ret;
   contacts_record_h child_record = NULL;
   ret = contacts_record_create(_contacts_nickname._uri, &child_record);
   EINA_SAFETY_ON_FALSE_RETURN(CONTACTS_ERROR_NONE == ret);

   if (!set_str(child_record, _contacts_nickname.name, value))
     goto on_error;

   if (!clear_all_contact_record(pd->record, _contacts_contact.nickname))
     goto on_error;

   ret = contacts_record_add_child_record(pd->record,
                                          _contacts_contact.nickname,
                                          child_record);
   EINA_SAFETY_ON_FALSE_GOTO(CONTACTS_ERROR_NONE == ret, on_error);
   return;

on_error:
   ret = contacts_record_destroy(child_record, true);
   EINA_SAFETY_ON_FALSE_RETURN(CONTACTS_ERROR_NONE == ret);
}

static Eina_Array *
_ecordova_contact_phone_numbers_get(Eo *obj EINA_UNUSED,
                                    Ecordova_Contact_Data *pd)
{
   return pd->phone_numbers;
}

static void
_ecordova_contact_phone_numbers_set(Eo *obj EINA_UNUSED,
                                    Ecordova_Contact_Data *pd,
                                    Eina_Array *phone_numbers)
{
   _contactfield_array_free(pd->phone_numbers);
   pd->phone_numbers = _contactfield_array_copy(phone_numbers);
}

static Eina_Array *
_ecordova_contact_emails_get(Eo *obj EINA_UNUSED, Ecordova_Contact_Data *pd)
{
   return pd->emails;
}

static void
_ecordova_contact_emails_set(Eo *obj EINA_UNUSED,
                             Ecordova_Contact_Data *pd,
                             Eina_Array *emails)
{
   _contactfield_array_free(pd->emails);
   pd->emails = _contactfield_array_copy(emails);
}

static Eina_Array *
_ecordova_contact_addresses_get(Eo *obj EINA_UNUSED, Ecordova_Contact_Data *pd)
{
   return pd->addresses;
}

static void
_ecordova_contact_addresses_set(Eo *obj EINA_UNUSED,
                                Ecordova_Contact_Data *pd,
                                Eina_Array *addresses)
{
   _contactaddress_array_free(pd->addresses);
   pd->addresses = _contactaddress_array_copy(addresses);
}

static Eina_Array *
_ecordova_contact_ims_get(Eo *obj EINA_UNUSED, Ecordova_Contact_Data *pd)
{
   return pd->ims;
}

static void
_ecordova_contact_ims_set(Eo *obj EINA_UNUSED,
                          Ecordova_Contact_Data *pd,
                          Eina_Array *ims)
{
   _contactfield_array_free(pd->ims);
   pd->ims = _contactfield_array_copy(ims);
}

static Eina_Array *
_ecordova_contact_organizations_get(Eo *obj EINA_UNUSED,
                                    Ecordova_Contact_Data *pd)
{
   return pd->organizations;
}

static void
_ecordova_contact_organizations_set(Eo *obj EINA_UNUSED,
                                    Ecordova_Contact_Data *pd,
                                    Eina_Array *organizations)
{
   _contactorganization_array_free(pd->organizations);
   pd->organizations = _contactorganization_array_copy(organizations);
}

static time_t
_ecordova_contact_birthday_get(Eo *obj EINA_UNUSED,
                               Ecordova_Contact_Data *pd EINA_UNUSED)
{
   // TODO: get birthday
   return 0;
}

static void
_ecordova_contact_birthday_set(Eo *obj EINA_UNUSED,
                               Ecordova_Contact_Data *pd EINA_UNUSED,
                               time_t birthday EINA_UNUSED)
{
   // TODO: set birthday
}

static const char *
_ecordova_contact_note_get(Eo *obj EINA_UNUSED, Ecordova_Contact_Data *pd)
{
   int ret;
   int count = 0;
   ret = contacts_record_get_child_record_count(pd->record,
                                                _contacts_contact.note,
                                                &count);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(CONTACTS_ERROR_NONE == ret, NULL);

   if (0 == count)
     return NULL;

   contacts_record_h child_record = NULL;
   ret = contacts_record_get_child_record_at_p(pd->record,
                                               _contacts_contact.note,
                                               0,
                                               &child_record);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(CONTACTS_ERROR_NONE == ret, NULL);

   const char *value = NULL;
   get_str_p(child_record, _contacts_note.note, &value);
   return value;
}

static void
_ecordova_contact_note_set(Eo *obj EINA_UNUSED,
                           Ecordova_Contact_Data *pd,
                           const char *value)
{
   int ret;
   contacts_record_h child_record = NULL;
   ret = contacts_record_create(_contacts_note._uri, &child_record);
   EINA_SAFETY_ON_FALSE_RETURN(CONTACTS_ERROR_NONE == ret);

   if (!set_str(child_record, _contacts_note.note, value))
     goto on_error;

   if (!clear_all_contact_record(pd->record, _contacts_contact.note))
     goto on_error;

   ret = contacts_record_add_child_record(pd->record,
                                          _contacts_contact.note,
                                          child_record);
   EINA_SAFETY_ON_FALSE_GOTO(CONTACTS_ERROR_NONE == ret, on_error);
   return;

on_error:
   ret = contacts_record_destroy(child_record, true);
   EINA_SAFETY_ON_FALSE_RETURN(CONTACTS_ERROR_NONE == ret);
}

static Eina_Array *
_ecordova_contact_photos_get(Eo *obj EINA_UNUSED, Ecordova_Contact_Data *pd)
{
   return pd->photos;
}

static void
_ecordova_contact_photos_set(Eo *obj EINA_UNUSED,
                             Ecordova_Contact_Data *pd,
                             Eina_Array *photos)
{
   _contactfield_array_free(pd->photos);
   pd->photos = _contactfield_array_copy(photos);
}

static Eina_Array *
_ecordova_contact_categories_get(Eo *obj EINA_UNUSED, Ecordova_Contact_Data *pd)
{
   return pd->categories;
}

static void
_ecordova_contact_categories_set(Eo *obj EINA_UNUSED,
                                 Ecordova_Contact_Data *pd,
                                 Eina_Array *categories)
{
   _contactfield_array_free(pd->categories);
   pd->categories = _contactfield_array_copy(categories);
}

static Eina_Array *
_ecordova_contact_urls_get(Eo *obj EINA_UNUSED, Ecordova_Contact_Data *pd)
{
   return pd->urls;
}

static void
_ecordova_contact_urls_set(Eo *obj EINA_UNUSED,
                           Ecordova_Contact_Data *pd,
                           Eina_Array *urls)
{
   _contactfield_array_free(pd->urls);
   pd->urls = _contactfield_array_copy(urls);
}

static Ecordova_ContactName *
_contactname_copy(Ecordova_ContactName *name)
{
   if (!name)
     return eo_add(ECORDOVA_CONTACTNAME_CLASS, NULL);

   return ecordova_contactname_clone(name);
}

static Ecordova_ContactAddress *
_contactaddress_copy(Ecordova_ContactAddress *address)
{
   if (!address)
     return NULL;

   Eina_Bool pref = EINA_FALSE;
   const char *type = NULL;
   const char *formatted = NULL;
   const char *street_address = NULL;
   const char *locality = NULL;
   const char *region = NULL;
   const char *postal_code = NULL;
   const char *country = NULL;
   eo_do(address,
         pref = ecordova_contactaddress_pref_get(),
         type = ecordova_contactaddress_type_get(),
         formatted = ecordova_contactaddress_formatted_get(),
         street_address = ecordova_contactaddress_street_address_get(),
         locality = ecordova_contactaddress_locality_get(),
         region = ecordova_contactaddress_region_get(),
         postal_code = ecordova_contactaddress_postal_code_get(),
         country = ecordova_contactaddress_country_get());

  return eo_add(ECORDOVA_CONTACTADDRESS_CLASS,
                NULL,
                ecordova_contactaddress_constructor(pref,
                                                    type,
                                                    formatted,
                                                    street_address,
                                                    locality,
                                                    region,
                                                    postal_code,
                                                    country));
}

static Ecordova_ContactField *
_contactfield_copy(Ecordova_ContactField *field)
{
   if (!field)
     return NULL;

   const char *type = NULL;
   const char *value = NULL;
   Eina_Bool pref = EINA_FALSE;
   eo_do(field,
         type = ecordova_contactfield_type_get(),
         value = ecordova_contactfield_value_get(),
         pref = ecordova_contactfield_pref_get());

  return eo_add(ECORDOVA_CONTACTFIELD_CLASS, NULL,
     ecordova_contactfield_constructor(type, value, pref));
}

static Ecordova_ContactOrganization *
_contactorganization_copy(Ecordova_ContactOrganization *organization)
{
   if (!organization)
     return NULL;

   Eina_Bool pref = EINA_FALSE;
   const char *type = NULL;
   const char *name = NULL;
   const char *dept = NULL;
   const char *title = NULL;
   eo_do(organization,
         pref = ecordova_contactorganization_pref_get(),
         type = ecordova_contactorganization_type_get(),
         name = ecordova_contactorganization_name_get(),
         dept = ecordova_contactorganization_dept_get(),
         title = ecordova_contactorganization_title_get());

  return eo_add(ECORDOVA_CONTACTORGANIZATION_CLASS,
                NULL,
                ecordova_contactorganization_constructor(pref,
                                                         type,
                                                         name,
                                                         dept,
                                                         title));
}

static Eina_Array *
_contactfield_array_copy(Eina_Array *fields)
{
   Eina_Array *result = eina_array_new(1);
   if (!fields)
     return result;

   size_t i;
   Ecordova_ContactField *field;
   Eina_Array_Iterator it;
   EINA_ARRAY_ITER_NEXT(fields, i, field, it)
     eina_array_push(result, _contactfield_copy(field));
   return result;
}

static Eina_Array *
_contactaddress_array_copy(Eina_Array *addresses)
{
   Eina_Array *result = eina_array_new(1);
   if (!addresses)
     return result;

   size_t i;
   Ecordova_ContactAddress *address;
   Eina_Array_Iterator it;
   EINA_ARRAY_ITER_NEXT(addresses, i, address, it)
     eina_array_push(result, _contactaddress_copy(address));
   return result;
}

static Eina_Array *
_contactorganization_array_copy(Eina_Array *organizations)
{
   Eina_Array *result = eina_array_new(1);
   if (!organizations)
     return result;

   size_t i;
   Ecordova_ContactOrganization *organization;
   Eina_Array_Iterator it;
   EINA_ARRAY_ITER_NEXT(organizations, i, organization, it)
     eina_array_push(result, _contactorganization_copy(organization));
   return result;
}

static void
_contactfield_array_free(Eina_Array *fields)
{
   if (!fields)
     return;

   size_t i;
   Ecordova_ContactField *field;
   Eina_Array_Iterator it;
   EINA_ARRAY_ITER_NEXT(fields, i, field, it)
     eo_unref(field);
   eina_array_free(fields);
}

static void
_contactaddress_array_free(Eina_Array *addresses)
{
   if (!addresses)
     return;

   size_t i;
   Ecordova_ContactAddress *address;
   Eina_Array_Iterator it;
   EINA_ARRAY_ITER_NEXT(addresses, i, address, it)
     eo_unref(address);
   eina_array_free(addresses);
}

static void
_contactorganization_array_free(Eina_Array *organizations)
{
   if (!organizations)
     return;

   size_t i;
   Ecordova_ContactOrganization *organization;
   Eina_Array_Iterator it;
   EINA_ARRAY_ITER_NEXT(organizations, i, organization, it)
     eo_unref(organization);
   eina_array_free(organizations);
}

static void
_contactfield_array_clear_id(Eina_Array *fields)
{
   if (!fields)
     return;

   size_t i;
   Ecordova_ContactField *field;
   Eina_Array_Iterator it;
   EINA_ARRAY_ITER_NEXT(fields, i, field, it)
     {
        Ecordova_ContactField_Data *pd =
          eo_data_scope_get(field, ECORDOVA_CONTACTFIELD_CLASS);
        EINA_SAFETY_ON_NULL_RETURN(pd);

        pd->id = 0;
     }
}

static void
_contactaddress_array_clear_id(Eina_Array *addresses)
{
   if (!addresses)
     return;

   size_t i;
   Ecordova_ContactAddress *address;
   Eina_Array_Iterator it;
   EINA_ARRAY_ITER_NEXT(addresses, i, address, it)
     {
        Ecordova_ContactAddress_Data *pd =
          eo_data_scope_get(address, ECORDOVA_CONTACTADDRESS_CLASS);
        EINA_SAFETY_ON_NULL_RETURN(pd);

        pd->id = 0;
     }
}

static void
_contactorganization_array_clear_id(Eina_Array *organizations)
{
   if (!organizations)
     return;

   size_t i;
   Ecordova_ContactOrganization *organization;
   Eina_Array_Iterator it;
   EINA_ARRAY_ITER_NEXT(organizations, i, organization, it)
     {
        Ecordova_ContactOrganization_Data *pd =
          eo_data_scope_get(organization, ECORDOVA_CONTACTORGANIZATION_CLASS);
        EINA_SAFETY_ON_NULL_RETURN(pd);

        pd->id = 0;
     }
}

static const Ecordova_ContactField_Metadata
_contact_number_metadata = {
  .uri = &_contacts_number._uri,
  .ids = {
     [ECORDOVA_CONTACTFIELD_PARENT_PROPERTY_ID] = &_contacts_contact.number,
     [ECORDOVA_CONTACTFIELD_PROPERTY_ID]        = &_contacts_number.id,
     [ECORDOVA_CONTACTFIELD_PROPERTY_TYPE]      = &_contacts_number.type,
     [ECORDOVA_CONTACTFIELD_PROPERTY_LABEL]     = &_contacts_number.label,
     [ECORDOVA_CONTACTFIELD_PROPERTY_VALUE]     = &_contacts_number.number,
     [ECORDOVA_CONTACTFIELD_PROPERTY_PREF]      = &_contacts_number.is_default},
  .type2label = ecordova_contactnumber_type2label,
  .label2type = ecordova_contactnumber_label2type
};

static const Ecordova_ContactField_Metadata
_contact_email_metadata = {
  .uri = &_contacts_email._uri,
  .ids = {
     [ECORDOVA_CONTACTFIELD_PARENT_PROPERTY_ID] = &_contacts_contact.email,
     [ECORDOVA_CONTACTFIELD_PROPERTY_ID]        = &_contacts_email.id,
     [ECORDOVA_CONTACTFIELD_PROPERTY_TYPE]      = &_contacts_email.type,
     [ECORDOVA_CONTACTFIELD_PROPERTY_LABEL]     = &_contacts_email.label,
     [ECORDOVA_CONTACTFIELD_PROPERTY_VALUE]     = &_contacts_email.email,
     [ECORDOVA_CONTACTFIELD_PROPERTY_PREF]      = &_contacts_email.is_default},
  .type2label = ecordova_contactemail_type2label,
  .label2type = ecordova_contactemail_label2type
};

static const Ecordova_ContactField_Metadata
_contact_messenger_metadata = {
  .uri = &_contacts_messenger._uri,
  .ids = {
     [ECORDOVA_CONTACTFIELD_PARENT_PROPERTY_ID] = &_contacts_contact.messenger,
     [ECORDOVA_CONTACTFIELD_PROPERTY_ID]        = &_contacts_messenger.id,
     [ECORDOVA_CONTACTFIELD_PROPERTY_TYPE]      = &_contacts_messenger.type,
     [ECORDOVA_CONTACTFIELD_PROPERTY_LABEL]     = &_contacts_messenger.label,
     [ECORDOVA_CONTACTFIELD_PROPERTY_VALUE]     = &_contacts_messenger.im_id,
     [ECORDOVA_CONTACTFIELD_PROPERTY_PREF]      = NULL},
  .type2label = ecordova_contactmessenger_type2label,
  .label2type = ecordova_contactmessenger_label2type
};

static const Ecordova_ContactField_Metadata
_contact_image_metadata = {
  .uri = &_contacts_image._uri,
  .ids = {
     [ECORDOVA_CONTACTFIELD_PARENT_PROPERTY_ID] = &_contacts_contact.image,
     [ECORDOVA_CONTACTFIELD_PROPERTY_ID]        = &_contacts_image.id,
     [ECORDOVA_CONTACTFIELD_PROPERTY_TYPE]      = &_contacts_image.type,
     [ECORDOVA_CONTACTFIELD_PROPERTY_LABEL]     = &_contacts_image.label,
     [ECORDOVA_CONTACTFIELD_PROPERTY_VALUE]     = &_contacts_image.path,
     [ECORDOVA_CONTACTFIELD_PROPERTY_PREF]      = &_contacts_image.is_default},
  .type2label = ecordova_contactimage_type2label,
  .label2type = ecordova_contactimage_label2type
};

static const Ecordova_ContactField_Metadata
_contact_url_metadata = {
  .uri = &_contacts_url._uri,
  .ids = {
     [ECORDOVA_CONTACTFIELD_PARENT_PROPERTY_ID] = &_contacts_contact.url,
     [ECORDOVA_CONTACTFIELD_PROPERTY_ID]        = &_contacts_url.id,
     [ECORDOVA_CONTACTFIELD_PROPERTY_TYPE]      = &_contacts_url.type,
     [ECORDOVA_CONTACTFIELD_PROPERTY_LABEL]     = &_contacts_url.label,
     [ECORDOVA_CONTACTFIELD_PROPERTY_VALUE]     = &_contacts_url.url,
     [ECORDOVA_CONTACTFIELD_PROPERTY_PREF]      = NULL},
  .type2label = ecordova_contacturl_type2label,
  .label2type = ecordova_contacturl_label2type
};

bool
ecordova_contact_import(Ecordova_Contact *obj,
                        contacts_record_h contacts_record)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, false);
   EINA_SAFETY_ON_NULL_RETURN_VAL(contacts_record, false);

   Ecordova_Contact_Data *pd = eo_data_scope_get(obj, ECORDOVA_CONTACT_CLASS);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pd, false);

   int ret;

   ret = contacts_record_destroy(pd->record, true);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(CONTACTS_ERROR_NONE == ret, false);
   ret = contacts_record_clone(contacts_record, &pd->record);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(CONTACTS_ERROR_NONE == ret, false);

   ret = contacts_record_get_int(pd->record, _contacts_contact.id, &pd->id);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(CONTACTS_ERROR_NONE == ret, false);

   // name
   DBG("%s", "Importing name");
   if (!ecordova_contactname_import(pd->name, pd->record))
     return false;

   // phone_numbers
   DBG("%s", "Importing phone_numbers");
   if (!_ecordova_contactfields_import(pd->phone_numbers,
                                       pd->record,
                                       _contact_number_metadata))
     return false;

   // emails
   DBG("%s", "Importing emails");
   if (!_ecordova_contactfields_import(pd->emails,
                                       pd->record,
                                       _contact_email_metadata))
     return false;

   // addresses
   DBG("%s", "Importing addresses");
   if (!_ecordova_contactaddresses_import(pd->addresses, pd->record))
     return false;

   // ims
   DBG("%s", "Importing ims");
   if (!_ecordova_contactfields_import(pd->ims,
                                       pd->record,
                                       _contact_messenger_metadata))
     return false;

   // organizations
   DBG("%s", "Importing organizations");
   if (!_ecordova_contactorganizations_import(pd->organizations, pd->record))
     return false;

   // photos
   DBG("%s", "Importing photos");
   if (!_ecordova_contactfields_import(pd->photos,
                                       pd->record,
                                       _contact_image_metadata))
     return false;

   // TODO: categories

   // urls
   DBG("%s", "Importing urls");
   if (!_ecordova_contactfields_import(pd->urls,
                                       pd->record,
                                       _contact_url_metadata))
     return false;

   return true;
}

bool
ecordova_contact_export(Ecordova_Contact *obj,
                        contacts_record_h contacts_record)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, false);
   EINA_SAFETY_ON_NULL_RETURN_VAL(contacts_record, false);

   Ecordova_Contact_Data *pd = eo_data_scope_get(obj, ECORDOVA_CONTACT_CLASS);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pd, false);

   // name
   if (!clear_all_contact_record(contacts_record, _contacts_contact.name) ||
       !ecordova_contactname_export(pd->name, contacts_record))
     return false;

   // phone_numbers
   if (!_ecordova_contactfields_export(pd->phone_numbers,
                                       contacts_record,
                                       _contact_number_metadata))
     return false;

   // emails
   if (!_ecordova_contactfields_export(pd->emails,
                                       contacts_record,
                                       _contact_email_metadata))
     return false;

   // addresses
   if (!_ecordova_contactaddresses_export(pd->addresses, contacts_record))
     return false;

   // ims
   if (!_ecordova_contactfields_export(pd->ims,
                                       contacts_record,
                                       _contact_messenger_metadata))
     return false;

   // organizations
   if (!_ecordova_contactorganizations_export(pd->organizations, contacts_record))
     return false;

   // photos
   if (!_ecordova_contactfields_export(pd->photos,
                                       contacts_record,
                                       _contact_image_metadata))
     return false;

   // TODO: categories

   // urls
   if (!_ecordova_contactfields_export(pd->urls,
                                       contacts_record,
                                       _contact_url_metadata))
     return false;

   return true;
}

static bool
_ecordova_contactfields_import(Eina_Array *fields,
                               contacts_record_h contacts_record,
                               const Ecordova_ContactField_Metadata metadata)
{
   int ret;
   int count = 0;
   ret = contacts_record_get_child_record_count(contacts_record,
                                                *metadata.ids[ECORDOVA_CONTACTFIELD_PARENT_PROPERTY_ID],
                                                &count);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(CONTACTS_ERROR_NONE == ret, false);

   DBG("Importing %d children", count);
   for (int index = 0; index < count; ++index)
     {
        contacts_record_h child_record = NULL;
        ret = contacts_record_get_child_record_at_p(contacts_record,
                                                    *metadata.ids[ECORDOVA_CONTACTFIELD_PARENT_PROPERTY_ID],
                                                    index,
                                                    &child_record);
        EINA_SAFETY_ON_FALSE_RETURN_VAL(CONTACTS_ERROR_NONE == ret, false);

        Ecordova_ContactField *field = eo_add(ECORDOVA_CONTACTFIELD_CLASS, NULL);
        if (!ecordova_contactfield_import(field, child_record, metadata))
          {
             eo_unref(field);
             return false;
          }

        Eina_Bool ret = eina_array_push(fields, field);
        EINA_SAFETY_ON_FALSE_RETURN_VAL(ret, false);
     }

   return true;
}

static bool
_ecordova_contactfields_export(Eina_Array *fields,
                               contacts_record_h contacts_record,
                               const Ecordova_ContactField_Metadata metadata)
{
   if (!clear_all_contact_record(contacts_record,
                                 *metadata.ids[ECORDOVA_CONTACTFIELD_PARENT_PROPERTY_ID]))
     return false;

   Ecordova_ContactField *field;
   Eina_Array_Iterator iterator;
   unsigned int i;
   EINA_ARRAY_ITER_NEXT(fields, i, field, iterator)
     {
        contacts_record_h record = NULL;
        int ret = contacts_record_create(*metadata.uri, &record);
        EINA_SAFETY_ON_FALSE_RETURN_VAL(CONTACTS_ERROR_NONE == ret, false);

        if (!ecordova_contactfield_export(field, record, metadata))
          {
             ret = contacts_record_destroy(record, true);
             EINA_SAFETY_ON_FALSE_RETURN_VAL(CONTACTS_ERROR_NONE == ret, false);
             return false;
          }

        ret = contacts_record_add_child_record(contacts_record,
                                               *metadata.ids[ECORDOVA_CONTACTFIELD_PARENT_PROPERTY_ID],
                                               record);
        EINA_SAFETY_ON_FALSE_RETURN_VAL(CONTACTS_ERROR_NONE == ret, false);
     }

   return true;
}

static bool
_ecordova_contactaddresses_import(Eina_Array *fields,
                                  contacts_record_h contacts_record)
{
   int ret;
   int count = 0;
   ret = contacts_record_get_child_record_count(contacts_record,
                                                _contacts_contact.address,
                                                &count);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(CONTACTS_ERROR_NONE == ret, false);

   DBG("Importing %d addresses", count);
   for (int index = 0; index < count; ++index)
     {
        contacts_record_h child_record = NULL;
        ret = contacts_record_get_child_record_at_p(contacts_record,
                                                    _contacts_contact.address,
                                                    index,
                                                    &child_record);
        EINA_SAFETY_ON_FALSE_RETURN_VAL(CONTACTS_ERROR_NONE == ret, false);

        Ecordova_ContactAddress *address = eo_add(ECORDOVA_CONTACTADDRESS_CLASS, NULL);
        if (!ecordova_contactaddress_import(address, child_record))
          {
             eo_unref(address);
             return false;
          }

        Eina_Bool ret = eina_array_push(fields, address);
        EINA_SAFETY_ON_FALSE_RETURN_VAL(ret, false);
     }

   return true;
}

static bool
_ecordova_contactaddresses_export(Eina_Array *addresses,
                                  contacts_record_h contacts_record)
{
   if (!clear_all_contact_record(contacts_record, _contacts_contact.address))
     return false;

   Ecordova_ContactAddress *address;
   Eina_Array_Iterator iterator;
   unsigned int i;
   EINA_ARRAY_ITER_NEXT(addresses, i, address, iterator)
     {
        contacts_record_h record = NULL;
        int ret = contacts_record_create(_contacts_address._uri, &record);
        EINA_SAFETY_ON_FALSE_RETURN_VAL(CONTACTS_ERROR_NONE == ret, false);

        if (!ecordova_contactaddress_export(address, record))
          {
             ret = contacts_record_destroy(record, true);
             EINA_SAFETY_ON_FALSE_RETURN_VAL(CONTACTS_ERROR_NONE == ret, false);
             return false;
          }

        ret = contacts_record_add_child_record(contacts_record,
                                               _contacts_contact.address,
                                               record);
        EINA_SAFETY_ON_FALSE_RETURN_VAL(CONTACTS_ERROR_NONE == ret, false);
     }

   return true;
}

static bool
_ecordova_contactorganizations_import(Eina_Array *organizations,
                                      contacts_record_h contacts_record)
{
   int ret;
   int count = 0;
   ret = contacts_record_get_child_record_count(contacts_record,
                                                _contacts_contact.company,
                                                &count);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(CONTACTS_ERROR_NONE == ret, false);

   DBG("Importing %d organizations", count);
   for (int index = 0; index < count; ++index)
     {
        contacts_record_h child_record = NULL;
        ret = contacts_record_get_child_record_at_p(contacts_record,
                                                    _contacts_contact.company,
                                                    index,
                                                    &child_record);
        EINA_SAFETY_ON_FALSE_RETURN_VAL(CONTACTS_ERROR_NONE == ret, false);

        Ecordova_ContactOrganization *organization = eo_add(ECORDOVA_CONTACTORGANIZATION_CLASS, NULL);
        if (!ecordova_contactorganization_import(organization, child_record))
          {
             eo_unref(organization);
             return false;
          }

        Eina_Bool ret = eina_array_push(organizations, organization);
        EINA_SAFETY_ON_FALSE_RETURN_VAL(ret, false);
     }

   return true;
}

static bool
_ecordova_contactorganizations_export(Eina_Array *organizations,
                                      contacts_record_h contacts_record)
{
   if (!clear_all_contact_record(contacts_record, _contacts_contact.company))
     return false;

   Ecordova_ContactOrganization *organization;
   Eina_Array_Iterator iterator;
   unsigned int i;
   EINA_ARRAY_ITER_NEXT(organizations, i, organization, iterator)
     {
        contacts_record_h record = NULL;
        int ret = contacts_record_create(_contacts_company._uri, &record);
        EINA_SAFETY_ON_FALSE_RETURN_VAL(CONTACTS_ERROR_NONE == ret, false);

        if (!ecordova_contactaddress_export(organization, record))
          {
             ret = contacts_record_destroy(record, true);
             EINA_SAFETY_ON_FALSE_RETURN_VAL(CONTACTS_ERROR_NONE == ret, false);
             return false;
          }

        ret = contacts_record_add_child_record(contacts_record,
                                               _contacts_contact.company,
                                               record);
        EINA_SAFETY_ON_FALSE_RETURN_VAL(CONTACTS_ERROR_NONE == ret, false);
     }

   return true;
}

#include "ecordova_contact.eo.c"
#endif
