#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef HAVE_TIZEN_CONTACTS_SERVICE
#include "ecordova_contactname_private.h"
#include "ecordova_contact_private.h"
#include "ecordova_contacts_record_utils.h"
#include "ecordova_contactname.eo.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#define MY_CLASS ECORDOVA_CONTACTNAME_CLASS
#define MY_CLASS_NAME "Ecordova_ContactName"

static Eo_Base *
_ecordova_contactname_eo_base_constructor(Eo *obj,
                                          Ecordova_ContactName_Data *pd)
{
   DBG("(%p)", obj);

   pd->obj = obj;
   pd->record = NULL;
   int ret = contacts_record_create(_contacts_name._uri, &pd->record);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(CONTACTS_ERROR_NONE == ret, NULL);

   return eo_do_super_ret(obj, MY_CLASS, obj, eo_constructor());
}

static void
_ecordova_contactname_constructor(Eo *obj,
                                  Ecordova_ContactName_Data *pd EINA_UNUSED,
                                  const char *formatted,
                                  const char *family_name,
                                  const char *given_name,
                                  const char *middle,
                                  const char *prefix,
                                  const char *suffix)
{
   DBG("(%p)", obj);
   eo_do(obj,
         ecordova_contactname_formatted_set(formatted),
         ecordova_contactname_family_name_set(family_name),
         ecordova_contactname_given_name_set(given_name),
         ecordova_contactname_middle_set(middle),
         ecordova_contactname_prefix_set(prefix),
         ecordova_contactname_suffix_set(suffix));
}

static void
_ecordova_contactname_eo_base_destructor(Eo *obj, Ecordova_ContactName_Data *pd)
{
   DBG("(%p)", obj);

   int ret = contacts_record_destroy(pd->record, true);
   EINA_SAFETY_ON_FALSE_RETURN(CONTACTS_ERROR_NONE == ret);

   eo_do_super(obj, MY_CLASS, eo_destructor());
}

static const char *
_ecordova_contactname_formatted_get(Eo *obj,
                                    Ecordova_ContactName_Data *pd EINA_UNUSED)
{
   Eo *parent = NULL;
   eo_do(obj, parent = eo_parent_get());
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);

   Ecordova_Contact_Data *parent_pd = eo_data_scope_get(parent, ECORDOVA_CONTACT_CLASS);
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent_pd, NULL);

   const char *value = NULL;
   get_str_p(parent_pd->record, _contacts_contact.display_name, &value);
   return value;
}

static void
_ecordova_contactname_formatted_set(Eo *obj,
                                    Ecordova_ContactName_Data *pd EINA_UNUSED,
                                    const char *value)
{
   Eo *parent = NULL;
   eo_do(obj, parent = eo_parent_get());
   EINA_SAFETY_ON_NULL_RETURN(parent);

   Ecordova_Contact_Data *parent_pd = eo_data_scope_get(parent, ECORDOVA_CONTACT_CLASS);
   EINA_SAFETY_ON_NULL_RETURN(parent_pd);

   set_str(parent_pd->record, _contacts_contact.display_name, value);
}

static const char *
_ecordova_contactname_family_name_get(Eo *obj EINA_UNUSED,
                                      Ecordova_ContactName_Data *pd)
{
   const char *value = NULL;
   get_str_p(pd->record, _contacts_name.last, &value);
   return value;
}

static void
_ecordova_contactname_family_name_set(Eo *obj EINA_UNUSED,
                                      Ecordova_ContactName_Data *pd,
                                      const char *value)
{
   set_str(pd->record, _contacts_name.last, value);
}

static const char *
_ecordova_contactname_given_name_get(Eo *obj EINA_UNUSED,
                                     Ecordova_ContactName_Data *pd)
{
   const char *value = NULL;
   get_str_p(pd->record, _contacts_name.first, &value);
   return value;
}

static void
_ecordova_contactname_given_name_set(Eo *obj EINA_UNUSED,
                                     Ecordova_ContactName_Data *pd,
                                     const char *value)
{
   set_str(pd->record, _contacts_name.first, value);
}

static const char *
_ecordova_contactname_middle_get(Eo *obj EINA_UNUSED,
                                 Ecordova_ContactName_Data *pd)
{
   const char *value = NULL;
   get_str_p(pd->record, _contacts_name.addition, &value);
   return value;
}

static void
_ecordova_contactname_middle_set(Eo *obj EINA_UNUSED,
                                 Ecordova_ContactName_Data *pd,
                                 const char *value)
{
   set_str(pd->record, _contacts_name.addition, value);
}

static const char *
_ecordova_contactname_prefix_get(Eo *obj EINA_UNUSED,
                                 Ecordova_ContactName_Data *pd)
{
   const char *value = NULL;
   get_str_p(pd->record, _contacts_name.prefix, &value);
   return value;
}

static void
_ecordova_contactname_prefix_set(Eo *obj EINA_UNUSED,
                                 Ecordova_ContactName_Data *pd,
                                 const char *value)
{
   set_str(pd->record, _contacts_name.prefix, value);
}

static const char *
_ecordova_contactname_suffix_get(Eo *obj EINA_UNUSED,
                                 Ecordova_ContactName_Data *pd)
{
   const char *value = NULL;
   get_str_p(pd->record, _contacts_name.suffix, &value);
   return value;
}

static void
_ecordova_contactname_suffix_set(Eo *obj EINA_UNUSED,
                                 Ecordova_ContactName_Data *pd,
                                 const char *value)
{
   set_str(pd->record, _contacts_name.suffix, value);
}

bool
ecordova_contactname_import(Ecordova_ContactName *obj,
                            contacts_record_h contacts_record)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, false);
   EINA_SAFETY_ON_NULL_RETURN_VAL(contacts_record, false);

   Ecordova_ContactName_Data *pd = eo_data_scope_get(obj, ECORDOVA_CONTACTNAME_CLASS);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pd, false);

   int ret;

   int count = 0;
   ret = contacts_record_get_child_record_count(contacts_record,
                                                _contacts_contact.name,
                                                &count);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(CONTACTS_ERROR_NONE == ret, false);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(count == 1, false);

   contacts_record_h contactname_record = NULL;
   ret = contacts_record_get_child_record_at_p(contacts_record,
                                               _contacts_contact.name,
                                               0,
                                               &contactname_record);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(CONTACTS_ERROR_NONE == ret, false);

   ret = contacts_record_destroy(pd->record, true);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(CONTACTS_ERROR_NONE == ret, false);
   ret = contacts_record_clone(contactname_record, &pd->record);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(CONTACTS_ERROR_NONE == ret, false);

   // TODO: check display name

   return true;
}

bool
ecordova_contactname_export(Ecordova_ContactName *obj,
                            contacts_record_h contacts_record)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, false);
   EINA_SAFETY_ON_NULL_RETURN_VAL(contacts_record, false);

   Ecordova_ContactName_Data *pd = eo_data_scope_get(obj, ECORDOVA_CONTACTNAME_CLASS);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pd, false);

   contacts_record_h contactname_record = NULL;
   int ret = contacts_record_clone(pd->record, &contactname_record);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(CONTACTS_ERROR_NONE == ret, false);

   ret = contacts_record_add_child_record(contacts_record,
                                          _contacts_contact.name,
                                          contactname_record);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(CONTACTS_ERROR_NONE == ret, false);

   // TODO: check display name

   return true;
}

Ecordova_ContactName *
ecordova_contactname_clone(Ecordova_ContactName *other)
{
   Ecordova_ContactName *cloned = eo_add(ECORDOVA_CONTACTNAME_CLASS, NULL);

   Ecordova_ContactName_Data *cloned_pd = eo_data_scope_get(cloned, ECORDOVA_CONTACTNAME_CLASS);
   EINA_SAFETY_ON_NULL_RETURN_VAL(cloned_pd, NULL);
   Ecordova_ContactName_Data *other_pd = eo_data_scope_get(other, ECORDOVA_CONTACTNAME_CLASS);
   EINA_SAFETY_ON_NULL_RETURN_VAL(other_pd, NULL);

   int ret = contacts_record_destroy(cloned_pd->record, true);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(CONTACTS_ERROR_NONE == ret, NULL);
   ret = contacts_record_clone(other_pd->record, &cloned_pd->record);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(CONTACTS_ERROR_NONE == ret, NULL);

   return cloned;
}


#include "ecordova_contactname.eo.c"
#endif
