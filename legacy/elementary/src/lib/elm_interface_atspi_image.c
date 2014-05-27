#ifdef HAVE_CONFIG_H
  #include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_widget.h"
#include "elm_priv.h"

#define ELM_INTERFACE_ATSPI_IMAGE_PROTECTED

#include "elm_interface_atspi_image.eo.h"

static void
_free_desc(void *data)
{
   eina_stringshare_del(data);
}

EOLIAN static const char*
_elm_interface_atspi_image_description_get(Eo *obj, void *sd EINA_UNUSED)
{
   const char *descr = NULL;

   eo_do(obj, descr = eo_key_data_get("atspi_image_description"));
   return descr;
}

EOLIAN static void
_elm_interface_atspi_image_description_set(Eo *obj, void *sd EINA_UNUSED, const char *description)
{
   const char *descr = eina_stringshare_add(description);
   eo_do(obj, eo_key_data_set("atspi_image_description", descr, _free_desc));
}

EOLIAN static const char*
_elm_interface_atspi_image_locale_get(Eo *obj EINA_UNUSED, void *sd EINA_UNUSED)
{
   // by default assume that descriptions are given in language of current
   // locale.
   return getenv("LANG");
}

#include "elm_interface_atspi_image.eo.c"
