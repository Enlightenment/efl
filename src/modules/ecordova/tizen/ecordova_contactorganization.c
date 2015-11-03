#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef HAVE_TIZEN_CONTACTS_SERVICE
#include "ecordova_contactorganization_private.h"
#include "ecordova_contacts_record_utils.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#define MY_CLASS ECORDOVA_CONTACTORGANIZATION_CLASS
#define MY_CLASS_NAME "Ecordova_ContactOrganization"

static Eo_Base *
_ecordova_contactorganization_eo_base_constructor(Eo *obj,
                                                  Ecordova_ContactOrganization_Data *pd)
{
   DBG("(%p)", obj);

   pd->obj = obj;
   pd->id = 0;
   pd->record = NULL;
   int ret = contacts_record_create(_contacts_company._uri, &pd->record);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(CONTACTS_ERROR_NONE == ret, NULL);

   return eo_do_super_ret(obj, MY_CLASS, obj, eo_constructor());
}

static void
_ecordova_contactorganization_constructor(Eo *obj,
                                          Ecordova_ContactOrganization_Data *pd EINA_UNUSED,
                                          Eina_Bool pref EINA_UNUSED,
                                          const char *type,
                                          const char *name,
                                          const char *dept,
                                          const char *title)
{
   DBG("(%p)", obj);
   eo_do(obj,
         ecordova_contactorganization_type_set(type),
         ecordova_contactorganization_name_set(name),
         ecordova_contactorganization_dept_set(dept),
         ecordova_contactorganization_title_set(title));
}

static void
_ecordova_contactorganization_eo_base_destructor(Eo *obj,
                                                 Ecordova_ContactOrganization_Data *pd)
{
   DBG("(%p)", obj);

   int ret = contacts_record_destroy(pd->record, true);
   EINA_SAFETY_ON_FALSE_RETURN(CONTACTS_ERROR_NONE == ret);

   eo_do_super(obj, MY_CLASS, eo_destructor());
}

static int
_ecordova_contactorganization_id_get(Eo *obj EINA_UNUSED,
                                     Ecordova_ContactOrganization_Data *pd)
{
   return pd->id;
}

static Eina_Bool
_ecordova_contactorganization_pref_get(Eo *obj EINA_UNUSED,
                                       Ecordova_ContactOrganization_Data *pd EINA_UNUSED)
{
   ERR("%s", "Not implemented");
   return EINA_FALSE;
}

static void
_ecordova_contactorganization_pref_set(Eo *obj EINA_UNUSED,
                                       Ecordova_ContactOrganization_Data *pd EINA_UNUSED,
                                       Eina_Bool pref EINA_UNUSED)
{
   ERR("%s", "Not implemented");
}

static const char *
_ecordova_contactorganization_type_get(Eo *obj EINA_UNUSED,
                                       Ecordova_ContactOrganization_Data *pd)
{
   int type;
   if (!get_int(pd->record, _contacts_company.type, &type))
     return NULL;

   switch (type)
     {
      case CONTACTS_COMPANY_TYPE_WORK:
        return "Work";
      case CONTACTS_COMPANY_TYPE_OTHER:
      case CONTACTS_COMPANY_TYPE_CUSTOM:
      default:
        {
           const char *custom = NULL;
           get_str_p(pd->record, _contacts_company.label, &custom);
           return custom;
        }
     }
}

static void
_ecordova_contactorganization_type_set(Eo *obj EINA_UNUSED,
                                       Ecordova_ContactOrganization_Data *pd,
                                       const char *value)
{
   int type = CONTACTS_COMPANY_TYPE_OTHER;
   if (!value || strlen(value) == 0)
     type = CONTACTS_COMPANY_TYPE_OTHER;
   else if (strcasecmp(value, "Work"))
     type = CONTACTS_COMPANY_TYPE_WORK;
   else
     type = CONTACTS_COMPANY_TYPE_CUSTOM;

   set_int(pd->record, _contacts_company.type, type);
   if (strlen(value) != 0)
     set_str(pd->record, _contacts_company.label, value);
}

static const char *
_ecordova_contactorganization_name_get(Eo *obj EINA_UNUSED,
                                       Ecordova_ContactOrganization_Data *pd)
{
   const char *value = NULL;
   get_str_p(pd->record, _contacts_company.name, &value);
   return value;
}

static void
_ecordova_contactorganization_name_set(Eo *obj EINA_UNUSED,
                                       Ecordova_ContactOrganization_Data *pd,
                                       const char *value)
{
   set_str(pd->record, _contacts_company.name, value);
}

static const char *
_ecordova_contactorganization_dept_get(Eo *obj EINA_UNUSED,
                                       Ecordova_ContactOrganization_Data *pd)
{
   const char *value = NULL;
   get_str_p(pd->record, _contacts_company.department, &value);
   return value;
}

static void
_ecordova_contactorganization_dept_set(Eo *obj EINA_UNUSED,
                                       Ecordova_ContactOrganization_Data *pd,
                                       const char *value)
{
   set_str(pd->record, _contacts_company.department, value);
}

static const char *
_ecordova_contactorganization_title_get(Eo *obj EINA_UNUSED,
                                        Ecordova_ContactOrganization_Data *pd)
{
   const char *value = NULL;
   get_str_p(pd->record, _contacts_company.job_title, &value);
   return value;
}

static void
_ecordova_contactorganization_title_set(Eo *obj EINA_UNUSED,
                                        Ecordova_ContactOrganization_Data *pd,
                                        const char *value)
{
   set_str(pd->record, _contacts_company.job_title, value);
}

bool
ecordova_contactorganization_import(Ecordova_ContactOrganization *obj,
                                    contacts_record_h child_record)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, false);
   EINA_SAFETY_ON_NULL_RETURN_VAL(child_record, false);

   Ecordova_ContactOrganization_Data *pd = eo_data_scope_get(obj, ECORDOVA_CONTACTORGANIZATION_CLASS);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pd, false);

   int ret = contacts_record_destroy(pd->record, true);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(CONTACTS_ERROR_NONE == ret, false);
   ret = contacts_record_clone(child_record, &pd->record);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(CONTACTS_ERROR_NONE == ret, false);

   return get_int(child_record, _contacts_company.id, &pd->id);
}

bool
ecordova_contactorganization_export(Ecordova_ContactOrganization *obj,
                                    contacts_record_h contacts_record)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, false);
   EINA_SAFETY_ON_NULL_RETURN_VAL(contacts_record, false);

   Ecordova_ContactOrganization_Data *pd = eo_data_scope_get(obj, ECORDOVA_CONTACTORGANIZATION_CLASS);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pd, false);

   contacts_record_h child_record = NULL;
   int ret = contacts_record_clone(pd->record, &child_record);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(CONTACTS_ERROR_NONE == ret, false);

   ret = contacts_record_add_child_record(contacts_record,
                                          _contacts_contact.company,
                                          child_record);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(CONTACTS_ERROR_NONE == ret, false);

   return true;
}

Ecordova_ContactOrganization *
ecordova_contactorganization_clone(Ecordova_ContactOrganization *other)
{
   Ecordova_ContactOrganization *cloned = eo_add(ECORDOVA_CONTACTORGANIZATION_CLASS, NULL);

   Ecordova_ContactOrganization_Data *cloned_pd = eo_data_scope_get(cloned, ECORDOVA_CONTACTORGANIZATION_CLASS);
   EINA_SAFETY_ON_NULL_RETURN_VAL(cloned_pd, NULL);
   Ecordova_ContactOrganization_Data *other_pd = eo_data_scope_get(other, ECORDOVA_CONTACTORGANIZATION_CLASS);
   EINA_SAFETY_ON_NULL_RETURN_VAL(other_pd, NULL);

   int ret = contacts_record_destroy(cloned_pd->record, true);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(CONTACTS_ERROR_NONE == ret, NULL);
   ret = contacts_record_clone(other_pd->record, &cloned_pd->record);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(CONTACTS_ERROR_NONE == ret, NULL);

   return cloned;
}

#include "ecordova_contactorganization.eo.c"
#endif
