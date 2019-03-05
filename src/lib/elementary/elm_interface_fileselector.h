#ifndef ELM_INTEFARCE_FILESELECTOR_H
#define ELM_INTEFARCE_FILESELECTOR_H

#ifdef EFL_EO_API_SUPPORT
#include "elm_interface_fileselector_eo.h"
#endif
#ifndef EFL_NOLEGACY_API_SUPPORT
#include "elm_interface_fileselector_eo.legacy.h"
#endif

#define ELM_FILESELECTOR_INTERFACE_CHECK(obj, ...) \
  if (EINA_UNLIKELY(!efl_isa(obj, ELM_INTERFACE_FILESELECTOR_INTERFACE))) \
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

Eina_Bool
_elm_fileselector_selected_set_internal(Evas_Object *obj, const char *_path);

const char *
_elm_fileselector_selected_get_internal(const Evas_Object *obj);

const Eina_List *
_elm_fileselector_selected_paths_get_internal(const Evas_Object* obj);


void
_elm_fileselector_entry_path_set_internal(Evas_Object *obj, const char *path);

const char *
_elm_fileselector_entry_path_get_internal(const Evas_Object *obj);

Eina_Bool
_elm_fileselector_entry_selected_set_internal(Evas_Object *obj, const char *path);

const char *
_elm_fileselector_entry_selected_get_internal(const Evas_Object *obj);


void
_elm_fileselector_button_path_set_internal(Evas_Object *obj, const char *path);

const char *
_elm_fileselector_button_path_get_internal(const Evas_Object *obj);

Eina_Bool
_elm_fileselector_button_selected_set_internal(Evas_Object *obj, const char *_path);

const char *
_elm_fileselector_button_selected_get_internal(const Evas_Object *obj);

const Eina_List *
_elm_fileselector_button_selected_paths_get_internal(const Evas_Object *obj);


void
_event_to_legacy_call(Eo *obj, const Efl_Event_Description *evt_desc, void *event_info);

void
_model_event_call(Eo *obj, const Efl_Event_Description *evt_desc, Efl_Model *model, const char *path);

#endif
