#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef HAVE_TIZEN_CONTACTS_SERVICE
#include "ecordova_contacts_record_utils.h"

#include <Eina.h>

Eina_Bool
get_str(contacts_record_h record, unsigned int property_id, char **value)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(record, EINA_FALSE);

   int ret = contacts_record_get_str(record, property_id, value);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(CONTACTS_ERROR_NONE == ret, EINA_FALSE);

   return EINA_TRUE;
}

Eina_Bool
get_str_p(contacts_record_h record, unsigned int property_id, const char **value)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(record, EINA_FALSE);

   int ret = contacts_record_get_str_p(record, property_id, (char**)value);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(CONTACTS_ERROR_NONE == ret, EINA_FALSE);

   return EINA_TRUE;
}

Eina_Bool
set_str(contacts_record_h record, unsigned int property_id, const char *value)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(record, EINA_FALSE);

   int ret = contacts_record_set_str(record, property_id, value);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(CONTACTS_ERROR_NONE == ret, EINA_FALSE);

   return EINA_TRUE;
}

Eina_Bool
get_int(contacts_record_h record, unsigned int property_id, int *value)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(record, EINA_FALSE);

   int ret = contacts_record_get_int(record, property_id, value);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(CONTACTS_ERROR_NONE == ret, EINA_FALSE);

   return EINA_TRUE;
}

Eina_Bool
set_int(contacts_record_h record, unsigned int property_id, int value)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(record, EINA_FALSE);

   int ret = contacts_record_set_int(record, property_id, value);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(CONTACTS_ERROR_NONE == ret, EINA_FALSE);

   return EINA_TRUE;
}

Eina_Bool
get_bool(contacts_record_h record, unsigned int property_id, Eina_Bool *value)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(record, EINA_FALSE);

   Eina_Bool raw_value;
   int ret = contacts_record_get_bool(record, property_id, &raw_value);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(CONTACTS_ERROR_NONE == ret, EINA_FALSE);

   *value = raw_value ? EINA_TRUE : EINA_FALSE;
   return EINA_TRUE;
}

Eina_Bool
set_bool(contacts_record_h record, unsigned int property_id, Eina_Bool value)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(record, EINA_FALSE);

   int ret = contacts_record_set_bool(record, property_id, value);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(CONTACTS_ERROR_NONE == ret, EINA_FALSE);

   return EINA_TRUE;
}

Eina_Bool
clear_all_contact_record(contacts_record_h contacts_record,
                         unsigned int property_id)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(contacts_record, EINA_FALSE);

   int ret;
   int count = 0;
   ret = contacts_record_get_child_record_count(contacts_record,
                                                property_id,
                                                &count);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(CONTACTS_ERROR_NONE == ret ||
                                   CONTACTS_ERROR_NO_DATA == ret, EINA_FALSE);

   for (int index = count - 1; index >= 0; --index)
   {
      contacts_record_h child_record = NULL;
      ret = contacts_record_get_child_record_at_p(contacts_record,
                                                  property_id,
                                                  index,
                                                  &child_record);
      EINA_SAFETY_ON_FALSE_RETURN_VAL(CONTACTS_ERROR_NONE == ret, EINA_FALSE);

      ret = contacts_record_remove_child_record(contacts_record,
                                                property_id,
                                                child_record);
      EINA_SAFETY_ON_FALSE_RETURN_VAL(CONTACTS_ERROR_NONE == ret, EINA_FALSE);

      ret = contacts_record_destroy(child_record, EINA_TRUE);
      EINA_SAFETY_ON_FALSE_RETURN_VAL(CONTACTS_ERROR_NONE == ret, EINA_FALSE);
   }

   return EINA_TRUE;
}
#endif
