#include "edbus_private.h"
#include "edbus_private_types.h"

EAPI void
edbus_message_iter_dict_iterate(EDBus_Message_Iter *array, const char *signature, EDBus_Dict_Cb_Get cb, const void *data)
{
   EDBus_Message_Iter *entry;
   char *iter_sig;
   unsigned len;
   EINA_SAFETY_ON_FALSE_RETURN(array);
   EINA_SAFETY_ON_NULL_RETURN(signature);

   iter_sig = edbus_message_iter_signature_get(array);
   len = strlen(iter_sig + 1);
   if (strncmp(signature, iter_sig + 1, len - 1))
     {
        ERR("Unexpected signature, expected is: %s", iter_sig);
        free(iter_sig);
        return;
     }
   free(iter_sig);

   while (edbus_message_iter_get_and_next(array, 'e', &entry))
     {
        const void *key;
        EDBus_Message_Iter *var;
        if (!edbus_message_iter_arguments_get(entry, signature, &key, &var))
          continue;
        cb((void *)data, key, var);
     }
}
