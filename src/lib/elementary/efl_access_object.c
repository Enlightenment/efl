#ifdef HAVE_CONFIG_H
  #include "elementary_config.h"
#endif

#define EFL_ACCESS_OBJECT_PROTECTED

#include <Elementary.h>
#include "elm_widget.h"
#include "elm_priv.h"

EAPI void efl_access_attributes_list_free(Eina_List *list)
{
   Efl_Access_Attribute *attr;
   EINA_LIST_FREE(list, attr)
     {
        eina_stringshare_del(attr->key);
        eina_stringshare_del(attr->value);
        free(attr);
     }
}

#include "efl_access_object.eo.c"
