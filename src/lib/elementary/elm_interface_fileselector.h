#ifndef ELM_INTEFARCE_FILESELECTOR_H
#define ELM_INTEFARCE_FILESELECTOR_H

#ifdef EFL_EO_API_SUPPORT
#include "elm_interface_fileselector.eo.h"
#endif
#ifndef EFL_NOLEGACY_API_SUPPORT
#include "elm_interface_fileselector.eo.legacy.h"
#endif

#define ELM_FILESELECTOR_INTERFACE_CHECK(obj, ...) \
  if (EINA_UNLIKELY(!eo_isa(obj, ELM_INTERFACE_FILESELECTOR_INTERFACE))) \
    { \
       ERR("The object (%p) doesn't implement the Elementary fileselector" \
            " interface", obj); \
       if (getenv("ELM_ERROR_ABORT")) abort(); \
       return __VA_ARGS__; \
    }

void
_elm_fileselector_path_set_internal(Evas_Object *obj, const char *path);

const char *
_elm_fileselector_path_get_internal(const Evas_Object *obj);

void
_elm_fileselector_entry_path_set_internal(Evas_Object *obj, const char *path);

const char *
_elm_fileselector_entry_path_get_internal(const Evas_Object *obj);

void
_elm_fileselector_button_path_set_internal(Evas_Object *obj, const char *path);

const char *
_elm_fileselector_button_path_get_internal(const Evas_Object *obj);

#endif
