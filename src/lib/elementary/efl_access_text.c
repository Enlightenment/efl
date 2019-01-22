#ifdef HAVE_CONFIG_H
  #include "elementary_config.h"
#endif

#define EFL_ACCESS_TEXT_PROTECTED

#include <Elementary.h>
#include "elm_widget.h"
#include "elm_priv.h"

void
EAPI elm_atspi_text_text_attribute_free(Efl_Access_Text_Attribute *attr)
{
   if (!attr) return;
   if (attr->name) eina_stringshare_del(attr->name);
   if (attr->value) eina_stringshare_del(attr->value);
   free(attr);
}

EAPI void
elm_atspi_text_text_range_free(Efl_Access_Text_Range *range)
{
   free(range->content);
   free(range);
}

#include "efl_access_text.eo.c"
