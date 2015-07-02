#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ecordova_contactaddress_private.h"
#include "ecordova_contacts_record_utils.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#define MY_CLASS ECORDOVA_CONTACTADDRESS_CLASS
#define MY_CLASS_NAME "Ecordova_ContactAddress"

static Eo_Base *
_ecordova_contactaddress_eo_base_constructor(Eo *obj,
                                             Ecordova_ContactAddress_Data *pd)
{
   DBG("(%p)", obj);

   pd->obj = obj;
   pd->id = 0;
   pd->record = NULL;
   int ret = contacts_record_create(_contacts_address._uri, &pd->record);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(CONTACTS_ERROR_NONE == ret, NULL);

   return eo_do_super_ret(obj, MY_CLASS, obj, eo_constructor());
}

static void
_ecordova_contactaddress_constructor(Eo *obj,
                                     Ecordova_ContactAddress_Data *pd EINA_UNUSED,
                                     Eina_Bool pref,
                                     const char *type,
                                     const char *formatted,
                                     const char *street_address,
                                     const char *locality,
                                     const char *region,
                                     const char *postal_code,
                                     const char *country)
{
   DBG("(%p)", obj);
   eo_do(obj,
         ecordova_contactaddress_pref_set(pref),
         ecordova_contactaddress_type_set(type),
         ecordova_contactaddress_formatted_set(formatted),
         ecordova_contactaddress_street_address_set(street_address),
         ecordova_contactaddress_locality_set(locality),
         ecordova_contactaddress_region_set(region),
         ecordova_contactaddress_postal_code_set(postal_code),
         ecordova_contactaddress_country_set(country));
}

static void
_ecordova_contactaddress_eo_base_destructor(Eo *obj,
                                            Ecordova_ContactAddress_Data *pd)
{
   DBG("(%p)", obj);

   int ret = contacts_record_destroy(pd->record, true);
   EINA_SAFETY_ON_FALSE_RETURN(CONTACTS_ERROR_NONE == ret);

   eo_do_super(obj, MY_CLASS, eo_destructor());
}

static int
_ecordova_contactaddress_id_get(Eo *obj EINA_UNUSED,
                                Ecordova_ContactAddress_Data *pd)
{
   return pd->id;
}

static Eina_Bool
_ecordova_contactaddress_pref_get(Eo *obj EINA_UNUSED,
                                  Ecordova_ContactAddress_Data *pd)
{
   Eina_Bool value = EINA_FALSE;
   get_bool(pd->record, _contacts_address.is_default, &value);
   return value;
}

static void
_ecordova_contactaddress_pref_set(Eo *obj EINA_UNUSED,
                                  Ecordova_ContactAddress_Data *pd,
                                  Eina_Bool value)
{
   set_bool(pd->record, _contacts_address.is_default, value);
}

static const char *
_ecordova_contactaddress_type_get(Eo *obj EINA_UNUSED,
                                  Ecordova_ContactAddress_Data *pd)
{
   int type;
   if (!get_int(pd->record, _contacts_address.type, &type))
     return NULL;

   switch (type)
     {
      case CONTACTS_ADDRESS_TYPE_HOME:
        return "Home";
      case CONTACTS_ADDRESS_TYPE_WORK:
        return "Work";
      case CONTACTS_ADDRESS_TYPE_DOM:
        return "Dom";
      case CONTACTS_ADDRESS_TYPE_INTL:
        return "Intl";
      case CONTACTS_ADDRESS_TYPE_POSTAL:
        return "Postal";
      case CONTACTS_ADDRESS_TYPE_PARCEL:
        return "Parcel";
      case CONTACTS_ADDRESS_TYPE_OTHER:
      case CONTACTS_ADDRESS_TYPE_CUSTOM:
      default:
        {
           const char *custom = NULL;
           get_str_p(pd->record, _contacts_address.label, &custom);
           return custom;
        }
     }
}

static void
_ecordova_contactaddress_type_set(Eo *obj EINA_UNUSED,
                                  Ecordova_ContactAddress_Data *pd,
                                  const char *value)
{
   int type = CONTACTS_ADDRESS_TYPE_OTHER;
   if (!value || strlen(value) == 0)
     type = CONTACTS_ADDRESS_TYPE_OTHER;
   else
   if (strcasecmp(value, "Home"))
     type = CONTACTS_ADDRESS_TYPE_HOME;
   else if (strcasecmp(value, "Work"))
     type = CONTACTS_ADDRESS_TYPE_WORK;
   else if (strcasecmp(value, "Dom"))
     type = CONTACTS_ADDRESS_TYPE_DOM;
   else if (strcasecmp(value, "Intl"))
     type = CONTACTS_ADDRESS_TYPE_INTL;
   else if (strcasecmp(value, "Postal"))
     type = CONTACTS_ADDRESS_TYPE_POSTAL;
   else if (strcasecmp(value, "Parcel"))
     type = CONTACTS_ADDRESS_TYPE_PARCEL;
   else
     type = CONTACTS_ADDRESS_TYPE_CUSTOM;

   set_int(pd->record, _contacts_address.type, type);
   if (strlen(value) != 0)
     set_str(pd->record, _contacts_address.label, value);
}

static const char *
_ecordova_contactaddress_formatted_get(Eo *obj EINA_UNUSED,
                                       Ecordova_ContactAddress_Data *pd EINA_UNUSED)
{
   // TODO: mount the formatted address
   ERR("%s", "Not implemented");
   return NULL;
}

static void
_ecordova_contactaddress_formatted_set(Eo *obj EINA_UNUSED,
                                       Ecordova_ContactAddress_Data *pd EINA_UNUSED,
                                       const char *formatted EINA_UNUSED)
{
   ERR("%s", "Not implemented");
}

static const char *
_ecordova_contactaddress_street_address_get(Eo *obj EINA_UNUSED,
                                            Ecordova_ContactAddress_Data *pd)
{
   const char *value = NULL;
   get_str_p(pd->record, _contacts_address.street, &value);
   return value;
}

static void
_ecordova_contactaddress_street_address_set(Eo *obj EINA_UNUSED,
                                            Ecordova_ContactAddress_Data *pd,
                                            const char *value)
{
   set_str(pd->record, _contacts_address.street, value);
}

static const char *
_ecordova_contactaddress_locality_get(Eo *obj EINA_UNUSED,
                                      Ecordova_ContactAddress_Data *pd)
{
   const char *value = NULL;
   get_str_p(pd->record, _contacts_address.locality, &value);
   return value;
}

static void
_ecordova_contactaddress_locality_set(Eo *obj EINA_UNUSED,
                                      Ecordova_ContactAddress_Data *pd,
                                      const char *value)
{
   set_str(pd->record, _contacts_address.locality, value);
}

static const char *
_ecordova_contactaddress_region_get(Eo *obj EINA_UNUSED,
                                    Ecordova_ContactAddress_Data *pd)
{
   const char *value = NULL;
   get_str_p(pd->record, _contacts_address.region, &value);
   return value;
}

static void
_ecordova_contactaddress_region_set(Eo *obj EINA_UNUSED,
                                    Ecordova_ContactAddress_Data *pd,
                                    const char *value)
{
   set_str(pd->record, _contacts_address.region, value);
}

static const char *
_ecordova_contactaddress_postal_code_get(Eo *obj EINA_UNUSED,
                                         Ecordova_ContactAddress_Data *pd)
{
   const char *value = NULL;
   get_str_p(pd->record, _contacts_address.postal_code, &value);
   return value;
}

static void
_ecordova_contactaddress_postal_code_set(Eo *obj EINA_UNUSED,
                                         Ecordova_ContactAddress_Data *pd,
                                         const char *value)
{
   set_str(pd->record, _contacts_address.postal_code, value);
}

static const char *
_ecordova_contactaddress_country_get(Eo *obj EINA_UNUSED,
                                     Ecordova_ContactAddress_Data *pd)
{
   const char *value = NULL;
   get_str_p(pd->record, _contacts_address.country, &value);
   return value;
}

static void
_ecordova_contactaddress_country_set(Eo *obj EINA_UNUSED,
                                     Ecordova_ContactAddress_Data *pd,
                                     const char *value)
{
   set_str(pd->record, _contacts_address.country, value);
}

bool
ecordova_contactaddress_import(Ecordova_ContactAddress *obj,
                               contacts_record_h child_record)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, false);
   EINA_SAFETY_ON_NULL_RETURN_VAL(child_record, false);

   Ecordova_ContactAddress_Data *pd = eo_data_scope_get(obj, ECORDOVA_CONTACTADDRESS_CLASS);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pd, false);

   int ret = contacts_record_destroy(pd->record, true);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(CONTACTS_ERROR_NONE == ret, false);
   ret = contacts_record_clone(child_record, &pd->record);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(CONTACTS_ERROR_NONE == ret, false);

   return get_int(child_record, _contacts_address.id, &pd->id);
}

bool
ecordova_contactaddress_export(Ecordova_ContactAddress *obj,
                               contacts_record_h contacts_record)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, false);
   EINA_SAFETY_ON_NULL_RETURN_VAL(contacts_record, false);

   Ecordova_ContactAddress_Data *pd = eo_data_scope_get(obj, ECORDOVA_CONTACTADDRESS_CLASS);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pd, false);

   contacts_record_h child_record = NULL;
   int ret = contacts_record_clone(pd->record, &child_record);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(CONTACTS_ERROR_NONE == ret, false);

   ret = contacts_record_add_child_record(contacts_record,
                                          _contacts_contact.address,
                                          child_record);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(CONTACTS_ERROR_NONE == ret, false);

   return true;
}

Ecordova_ContactAddress *
ecordova_contactaddress_clone(Ecordova_ContactAddress *other)
{
   Ecordova_ContactAddress *cloned = eo_add(ECORDOVA_CONTACTADDRESS_CLASS, NULL);

   Ecordova_ContactAddress_Data *cloned_pd = eo_data_scope_get(cloned, ECORDOVA_CONTACTADDRESS_CLASS);
   EINA_SAFETY_ON_NULL_RETURN_VAL(cloned_pd, NULL);
   Ecordova_ContactAddress_Data *other_pd = eo_data_scope_get(other, ECORDOVA_CONTACTADDRESS_CLASS);
   EINA_SAFETY_ON_NULL_RETURN_VAL(other_pd, NULL);

   int ret = contacts_record_destroy(cloned_pd->record, true);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(CONTACTS_ERROR_NONE == ret, NULL);
   ret = contacts_record_clone(other_pd->record, &cloned_pd->record);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(CONTACTS_ERROR_NONE == ret, NULL);

   return cloned;
}

#include "ecordova_contactaddress.eo.c"
