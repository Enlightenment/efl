#ifdef HAVE_CONFIG_H
  #include "elementary_config.h"
#endif

#define ELM_INTERFACE_ATSPI_TEXT_PROTECTED

#include <Elementary.h>
#include "elm_widget.h"
#include "elm_priv.h"

void
EAPI elm_atspi_text_text_attribute_free(Elm_Atspi_Text_Attribute *attr)
{
   if (!attr) return;
   if (attr->name) eina_stringshare_del(attr->name);
   if (attr->value) eina_stringshare_del(attr->value);
   free(attr);
}

#include "elm_interface_atspi_text.eo.c"
