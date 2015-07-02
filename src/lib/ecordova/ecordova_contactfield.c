#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ecordova_contactfield_private.h"
#include "ecordova_contacts_record_utils.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#define MY_CLASS ECORDOVA_CONTACTFIELD_CLASS
#define MY_CLASS_NAME "Ecordova_ContactField"

static Eo_Base *
_ecordova_contactfield_eo_base_constructor(Eo *obj,
                                           Ecordova_ContactField_Data *pd)
{
   DBG("(%p)", obj);

   pd->obj = obj;

   return eo_do_super_ret(obj, MY_CLASS, obj, eo_constructor());
}

static void
_ecordova_contactfield_constructor(Eo *obj,
                                  Ecordova_ContactField_Data *pd,
                                  const char *type,
                                  const char *value,
                                  Eina_Bool pref)
{
   DBG("(%p)", obj);
   pd->id = 0;
   pd->type = type ? strdup(type) : NULL;
   pd->value = value ? strdup(value) : NULL;
   pd->pref = pref;
}

static void
_ecordova_contactfield_eo_base_destructor(Eo *obj,
                                          Ecordova_ContactField_Data *pd)
{
   DBG("(%p)", obj);

   free(pd->type);
   free(pd->value);

   eo_do_super(obj, MY_CLASS, eo_destructor());
}

static int
_ecordova_contactfield_id_get(Eo *obj EINA_UNUSED,
                              Ecordova_ContactField_Data *pd)
{
   return pd->id;
}

static const char *
_ecordova_contactfield_type_get(Eo *obj EINA_UNUSED,
                                Ecordova_ContactField_Data *pd)
{
   return pd->type;
}

static void
_ecordova_contactfield_type_set(Eo *obj EINA_UNUSED,
                                Ecordova_ContactField_Data *pd,
                                const char *type)
{
   free(pd->type);
   pd->type = type ? strdup(type) : NULL;
}

static const char *
_ecordova_contactfield_value_get(Eo *obj EINA_UNUSED,
                                 Ecordova_ContactField_Data *pd)
{
   return pd->value;
}

static void
_ecordova_contactfield_value_set(Eo *obj EINA_UNUSED,
                                 Ecordova_ContactField_Data *pd,
                                 const char *value)
{
   free(pd->value);
   pd->value = value ? strdup(value) : NULL;
}

static Eina_Bool
_ecordova_contactfield_pref_get(Eo *obj EINA_UNUSED,
                                Ecordova_ContactField_Data *pd)
{
   return pd->pref;
}

static void
_ecordova_contactfield_pref_set(Eo *obj EINA_UNUSED,
                                Ecordova_ContactField_Data *pd,
                                Eina_Bool pref)
{
   pd->pref = pref;
}

bool
ecordova_contactfield_import(Ecordova_ContactField *obj,
                             contacts_record_h record,
                             const Ecordova_ContactField_Metadata metadata)
{
   DBG("%p", obj);
   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, false);
   EINA_SAFETY_ON_NULL_RETURN_VAL(record, false);

   Ecordova_ContactField_Data *pd = eo_data_scope_get(obj, ECORDOVA_CONTACTFIELD_CLASS);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pd, false);

   int type = 0;
   const char* label = NULL;
   if (!get_int(record,
                *metadata.ids[ECORDOVA_CONTACTFIELD_PROPERTY_TYPE],
                &type) ||
       !get_str_p(record,
                  *metadata.ids[ECORDOVA_CONTACTFIELD_PROPERTY_LABEL],
                  &label))
     return false;

   pd->type = metadata.type2label(type, label);

   if (!get_int(record,
                *metadata.ids[ECORDOVA_CONTACTFIELD_PROPERTY_ID],
                &pd->id) ||
       !get_str(record,
                *metadata.ids[ECORDOVA_CONTACTFIELD_PROPERTY_VALUE],
                &pd->value))
     return false;

   if (metadata.ids[ECORDOVA_CONTACTFIELD_PROPERTY_PREF] &&
       !get_bool(record,
                 *metadata.ids[ECORDOVA_CONTACTFIELD_PROPERTY_PREF],
                 &pd->pref))
     return false;

   return true;
}

bool
ecordova_contactfield_export(Ecordova_ContactField *obj,
                             contacts_record_h record,
                             const Ecordova_ContactField_Metadata metadata)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, false);
   EINA_SAFETY_ON_NULL_RETURN_VAL(record, false);

   Ecordova_ContactField_Data *pd = eo_data_scope_get(obj, ECORDOVA_CONTACTFIELD_CLASS);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pd, false);

   int type = metadata.label2type(pd->type);
   if (!set_int(record,
                *metadata.ids[ECORDOVA_CONTACTFIELD_PROPERTY_TYPE],
                type))
     return false;

   if (pd->type &&
       !set_str(record,
                *metadata.ids[ECORDOVA_CONTACTFIELD_PROPERTY_LABEL],
                pd->type))
     return false;

   if (!set_str(record,
                *metadata.ids[ECORDOVA_CONTACTFIELD_PROPERTY_VALUE],
                pd->value))
     return false;

   if (metadata.ids[ECORDOVA_CONTACTFIELD_PROPERTY_PREF] &&
       !set_bool(record,
                 *metadata.ids[ECORDOVA_CONTACTFIELD_PROPERTY_PREF],
                 pd->pref))
     return false;

   return true;
}

char *
ecordova_contactnumber_type2label(int type, const char *custom)
{
   switch (type)
   {
    case CONTACTS_NUMBER_TYPE_HOME:
      return strdup("Home");
    case CONTACTS_NUMBER_TYPE_WORK:
      return strdup("Work");
    case CONTACTS_NUMBER_TYPE_VOICE:
      return strdup("Voice");
    case CONTACTS_NUMBER_TYPE_FAX:
      return strdup("Fax");
    case CONTACTS_NUMBER_TYPE_MSG:
      return strdup("Msg");
    case CONTACTS_NUMBER_TYPE_CELL:
      return strdup("Cell");
    case CONTACTS_NUMBER_TYPE_PAGER:
      return strdup("Pager");
    case CONTACTS_NUMBER_TYPE_BBS:
      return strdup("BBS");
    case CONTACTS_NUMBER_TYPE_MODEM:
      return strdup("Modem");
    case CONTACTS_NUMBER_TYPE_CAR:
      return strdup("Car");
    case CONTACTS_NUMBER_TYPE_ISDN:
      return strdup("ISDN");
    case CONTACTS_NUMBER_TYPE_VIDEO:
      return strdup("Video");
    case CONTACTS_NUMBER_TYPE_PCS:
      return strdup("Pcs");
    case CONTACTS_NUMBER_TYPE_COMPANY_MAIN:
      return strdup("Company_Main");
    case CONTACTS_NUMBER_TYPE_RADIO:
      return strdup("Radio");
    case CONTACTS_NUMBER_TYPE_MAIN:
      return strdup("Main");
    case CONTACTS_NUMBER_TYPE_ASSISTANT:
      return strdup("Assistant");
    case CONTACTS_NUMBER_TYPE_OTHER:
    case CONTACTS_NUMBER_TYPE_CUSTOM:
    default:
       if (custom)
         return strdup(custom);
       return NULL;
   }
}

int
ecordova_contactnumber_label2type(const char *label)
{
   if (!label)
     return CONTACTS_NUMBER_TYPE_OTHER;
   if (strcasecmp(label, "Home") == 0)
     return CONTACTS_NUMBER_TYPE_HOME;
   if (strcasecmp(label, "Work") == 0)
     return CONTACTS_NUMBER_TYPE_WORK;
   if (strcasecmp(label, "Voice") == 0)
     return CONTACTS_NUMBER_TYPE_VOICE;
   if (strcasecmp(label, "Fax") == 0)
     return CONTACTS_NUMBER_TYPE_FAX;
   if (strcasecmp(label, "Msg") == 0)
     return CONTACTS_NUMBER_TYPE_MSG;
   if (strcasecmp(label, "Cell") == 0)
     return CONTACTS_NUMBER_TYPE_CELL;
   if (strcasecmp(label, "Pager") == 0)
     return CONTACTS_NUMBER_TYPE_PAGER;
   if (strcasecmp(label, "BBS") == 0)
     return CONTACTS_NUMBER_TYPE_BBS;
   if (strcasecmp(label, "Modem") == 0)
     return CONTACTS_NUMBER_TYPE_MODEM;
   if (strcasecmp(label, "Car") == 0)
     return CONTACTS_NUMBER_TYPE_CAR;
   if (strcasecmp(label, "ISDN") == 0)
     return CONTACTS_NUMBER_TYPE_ISDN;
   if (strcasecmp(label, "Video") == 0)
     return CONTACTS_NUMBER_TYPE_VIDEO;
   if (strcasecmp(label, "Pcs") == 0)
     return CONTACTS_NUMBER_TYPE_PCS;
   if (strcasecmp(label, "Company_Main") == 0)
     return CONTACTS_NUMBER_TYPE_COMPANY_MAIN;
   if (strcasecmp(label, "Radio") == 0)
     return CONTACTS_NUMBER_TYPE_RADIO;
   if (strcasecmp(label, "Main") == 0)
     return CONTACTS_NUMBER_TYPE_MAIN;
   if (strcasecmp(label, "Assistant") == 0)
     return CONTACTS_NUMBER_TYPE_ASSISTANT;

   return CONTACTS_NUMBER_TYPE_CUSTOM;
}

char *
ecordova_contactemail_type2label(int type, const char *custom)
{
   switch (type)
     {
      case CONTACTS_EMAIL_TYPE_HOME:
        return strdup("Home");
      case CONTACTS_EMAIL_TYPE_WORK:
        return strdup("Work");
      case CONTACTS_EMAIL_TYPE_MOBILE:
        return strdup("Mobile");
      case CONTACTS_EMAIL_TYPE_OTHER:
      case CONTACTS_EMAIL_TYPE_CUSTOM:
      default:
        if (custom)
          return strdup(custom);
        return NULL;
     }
}

int
ecordova_contactemail_label2type(const char *label)
{
   if (!label)
     return CONTACTS_EMAIL_TYPE_OTHER;
   if (strcasecmp(label, "Home") == 0)
     return CONTACTS_EMAIL_TYPE_HOME;
   if (strcasecmp(label, "Work") == 0)
     return CONTACTS_EMAIL_TYPE_WORK;
   if (strcasecmp(label, "Mobile") == 0)
     return CONTACTS_EMAIL_TYPE_MOBILE;

   return CONTACTS_EMAIL_TYPE_CUSTOM;
}

char *
ecordova_contactmessenger_type2label(int type, const char *custom)
{
   switch (type)
     {
      case CONTACTS_MESSENGER_TYPE_GOOGLE:
        return strdup("Google");
      case CONTACTS_MESSENGER_TYPE_WLM:
        return strdup("Wlm");
      case CONTACTS_MESSENGER_TYPE_YAHOO:
        return strdup("Yahoo");
      case CONTACTS_MESSENGER_TYPE_FACEBOOK:
        return strdup("Facebook");
      case CONTACTS_MESSENGER_TYPE_ICQ:
        return strdup("Icq");
      case CONTACTS_MESSENGER_TYPE_AIM:
        return strdup("Aim");
      case CONTACTS_MESSENGER_TYPE_QQ:
        return strdup("Qq");
      case CONTACTS_MESSENGER_TYPE_JABBER:
        return strdup("Jabber");
      case CONTACTS_MESSENGER_TYPE_SKYPE:
        return strdup("Skype");
      case CONTACTS_MESSENGER_TYPE_IRC:
        return strdup("IRC");
      case CONTACTS_MESSENGER_TYPE_OTHER:
      case CONTACTS_MESSENGER_TYPE_CUSTOM:
      default:
        if (custom)
          return strdup(custom);
        return NULL;
     }
}

int
ecordova_contactmessenger_label2type(const char *label)
{
   if (!label)
     return CONTACTS_MESSENGER_TYPE_OTHER;
   if (strcasecmp(label, "Google") == 0)
      return CONTACTS_MESSENGER_TYPE_GOOGLE;
   if (strcasecmp(label, "Wlm") == 0)
      return CONTACTS_MESSENGER_TYPE_WLM;
   if (strcasecmp(label, "Yahoo") == 0)
      return CONTACTS_MESSENGER_TYPE_YAHOO;
   if (strcasecmp(label, "Facebook") == 0)
      return CONTACTS_MESSENGER_TYPE_FACEBOOK;
   if (strcasecmp(label, "Icq") == 0)
      return CONTACTS_MESSENGER_TYPE_ICQ;
   if (strcasecmp(label, "Aim") == 0)
      return CONTACTS_MESSENGER_TYPE_AIM;
   if (strcasecmp(label, "Qq") == 0)
      return CONTACTS_MESSENGER_TYPE_QQ;
   if (strcasecmp(label, "Jabber") == 0)
      return CONTACTS_MESSENGER_TYPE_JABBER;
   if (strcasecmp(label, "Skype") == 0)
      return CONTACTS_MESSENGER_TYPE_SKYPE;
   if (strcasecmp(label, "IRC") == 0)
      return CONTACTS_MESSENGER_TYPE_IRC;

   return CONTACTS_MESSENGER_TYPE_CUSTOM;
}

char *
ecordova_contactimage_type2label(int type EINA_UNUSED, const char *custom)
{
   return custom ? strdup(custom) : NULL;
}

int
ecordova_contactimage_label2type(const char *label EINA_UNUSED)
{
   return 0;
}

char *
ecordova_contacturl_type2label(int type, const char *custom)
{
   switch (type)
     {
      case CONTACTS_URL_TYPE_HOME:
        return strdup("Home");
      case CONTACTS_URL_TYPE_WORK:
        return strdup("Work");
      case CONTACTS_URL_TYPE_OTHER:
      case CONTACTS_URL_TYPE_CUSTOM:
      default:
        if (custom)
          return strdup(custom);
        return NULL;
     }
}

int
ecordova_contacturl_label2type(const char *label)
{
   if (!label)
     return CONTACTS_URL_TYPE_OTHER;
   if (strcasecmp(label, "Home") == 0)
     return CONTACTS_URL_TYPE_HOME;
   if (strcasecmp(label, "Work") == 0)
     return CONTACTS_URL_TYPE_WORK;
   return CONTACTS_URL_TYPE_CUSTOM;
}

#include "ecordova_contactfield.eo.c"
