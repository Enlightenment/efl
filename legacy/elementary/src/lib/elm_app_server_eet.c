#include "elm_app_server_eet.h"

struct _Elm_App_Server_View_Props {
    const char * id;
    const char * title;
    const char * icon_name;
    int new_events;
    char progress;
};

struct _Elm_App_Server_Views_Eet {
    unsigned int version;
    Eina_List * views_entries;
    const char *__eet_filename;
};

static const char ELM_APP_SERVER_VIEW_PROPS_ENTRY[] = "elm_app_server_view_props";
static const char ELM_APP_SERVER_VIEWS_EET_ENTRY[] = "elm_app_server_views_eet";

static Eet_Data_Descriptor *_elm_app_server_view_props_descriptor = NULL;
static Eet_Data_Descriptor *_elm_app_server_views_eet_descriptor = NULL;

static inline void
_elm_app_server_view_props_init(void)
{
    Eet_Data_Descriptor_Class eddc;

    if (_elm_app_server_view_props_descriptor) return;

    EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, Elm_App_Server_View_Props);
    _elm_app_server_view_props_descriptor = eet_data_descriptor_stream_new(&eddc);

    EET_DATA_DESCRIPTOR_ADD_BASIC(_elm_app_server_view_props_descriptor, Elm_App_Server_View_Props, "id", id, EET_T_STRING);
    EET_DATA_DESCRIPTOR_ADD_BASIC(_elm_app_server_view_props_descriptor, Elm_App_Server_View_Props, "title", title, EET_T_STRING);
    EET_DATA_DESCRIPTOR_ADD_BASIC(_elm_app_server_view_props_descriptor, Elm_App_Server_View_Props, "icon_name", icon_name, EET_T_STRING);
    EET_DATA_DESCRIPTOR_ADD_BASIC(_elm_app_server_view_props_descriptor, Elm_App_Server_View_Props, "new_events", new_events, EET_T_INT);
    EET_DATA_DESCRIPTOR_ADD_BASIC(_elm_app_server_view_props_descriptor, Elm_App_Server_View_Props, "progress", progress, EET_T_CHAR);
}

static inline void
_elm_app_server_view_props_shutdown(void)
{
    if (!_elm_app_server_view_props_descriptor) return;
    eet_data_descriptor_free(_elm_app_server_view_props_descriptor);
    _elm_app_server_view_props_descriptor = NULL;
}

Elm_App_Server_View_Props *
elm_app_server_view_props_new(const char * id, const char * title, const char * icon_name, int new_events, char progress)
{
    Elm_App_Server_View_Props *elm_app_server_view_props = calloc(1, sizeof(Elm_App_Server_View_Props));

    if (!elm_app_server_view_props)
       {
          fprintf(stderr, "ERROR: could not calloc Elm_App_Server_View_Props\n");
          return NULL;
       }

    elm_app_server_view_props->id = eina_stringshare_add(id ? id : "");
    elm_app_server_view_props->title = eina_stringshare_add(title ? title : "");
    elm_app_server_view_props->icon_name = eina_stringshare_add(icon_name ? icon_name : "");
    elm_app_server_view_props->new_events = new_events;
    elm_app_server_view_props->progress = progress;

    return elm_app_server_view_props;
}

void
elm_app_server_view_props_free(Elm_App_Server_View_Props *elm_app_server_view_props)
{
    eina_stringshare_del(elm_app_server_view_props->id);
    eina_stringshare_del(elm_app_server_view_props->title);
    eina_stringshare_del(elm_app_server_view_props->icon_name);
    free(elm_app_server_view_props);
}

inline const char *
elm_app_server_view_props_id_get(const Elm_App_Server_View_Props *elm_app_server_view_props)
{
    return elm_app_server_view_props->id;
}

inline void
elm_app_server_view_props_id_set(Elm_App_Server_View_Props *elm_app_server_view_props, const char *id)
{
    EINA_SAFETY_ON_NULL_RETURN(elm_app_server_view_props);
    eina_stringshare_replace(&(elm_app_server_view_props->id), id);
}

inline const char *
elm_app_server_view_props_title_get(const Elm_App_Server_View_Props *elm_app_server_view_props)
{
    return elm_app_server_view_props->title;
}

inline void
elm_app_server_view_props_title_set(Elm_App_Server_View_Props *elm_app_server_view_props, const char *title)
{
    EINA_SAFETY_ON_NULL_RETURN(elm_app_server_view_props);
    eina_stringshare_replace(&(elm_app_server_view_props->title), title);
}

inline const char *
elm_app_server_view_props_icon_name_get(const Elm_App_Server_View_Props *elm_app_server_view_props)
{
    return elm_app_server_view_props->icon_name;
}

inline void
elm_app_server_view_props_icon_name_set(Elm_App_Server_View_Props *elm_app_server_view_props, const char *icon_name)
{
    EINA_SAFETY_ON_NULL_RETURN(elm_app_server_view_props);
    eina_stringshare_replace(&(elm_app_server_view_props->icon_name), icon_name);
}

inline int
elm_app_server_view_props_new_events_get(const Elm_App_Server_View_Props *elm_app_server_view_props)
{
    return elm_app_server_view_props->new_events;
}

inline void
elm_app_server_view_props_new_events_set(Elm_App_Server_View_Props *elm_app_server_view_props, int new_events)
{
    EINA_SAFETY_ON_NULL_RETURN(elm_app_server_view_props);
    elm_app_server_view_props->new_events = new_events;
}

inline char
elm_app_server_view_props_progress_get(const Elm_App_Server_View_Props *elm_app_server_view_props)
{
    return elm_app_server_view_props->progress;
}

inline void
elm_app_server_view_props_progress_set(Elm_App_Server_View_Props *elm_app_server_view_props, char progress)
{
    EINA_SAFETY_ON_NULL_RETURN(elm_app_server_view_props);
    elm_app_server_view_props->progress = progress;
}

static inline void
_elm_app_server_views_eet_init(void)
{
    Eet_Data_Descriptor_Class eddc;

    if (_elm_app_server_views_eet_descriptor) return;

    EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, Elm_App_Server_Views_Eet);
    _elm_app_server_views_eet_descriptor = eet_data_descriptor_stream_new(&eddc);

    EET_DATA_DESCRIPTOR_ADD_BASIC(_elm_app_server_views_eet_descriptor, Elm_App_Server_Views_Eet, "version", version, EET_T_UINT);
    EET_DATA_DESCRIPTOR_ADD_LIST(_elm_app_server_views_eet_descriptor, Elm_App_Server_Views_Eet, "views_entries", views_entries, _elm_app_server_view_props_descriptor);
}

static inline void
_elm_app_server_views_eet_shutdown(void)
{
    if (!_elm_app_server_views_eet_descriptor) return;
    eet_data_descriptor_free(_elm_app_server_views_eet_descriptor);
    _elm_app_server_views_eet_descriptor = NULL;
}

Elm_App_Server_Views_Eet *
elm_app_server_views_eet_new(unsigned int version, Eina_List * views_entries)
{
    Elm_App_Server_Views_Eet *elm_app_server_views_eet = calloc(1, sizeof(Elm_App_Server_Views_Eet));

    if (!elm_app_server_views_eet)
       {
          fprintf(stderr, "ERROR: could not calloc Elm_App_Server_Views_Eet\n");
          return NULL;
       }

    elm_app_server_views_eet->version = version;
    elm_app_server_views_eet->views_entries = views_entries;

    return elm_app_server_views_eet;
}

void
elm_app_server_views_eet_free(Elm_App_Server_Views_Eet *elm_app_server_views_eet)
{
    if (elm_app_server_views_eet->views_entries)
       {
          Elm_App_Server_View_Props *views_entries_elem;
          EINA_LIST_FREE(elm_app_server_views_eet->views_entries, views_entries_elem)
             elm_app_server_view_props_free(views_entries_elem);
       }
    free(elm_app_server_views_eet);
}

inline unsigned int
elm_app_server_views_eet_version_get(const Elm_App_Server_Views_Eet *elm_app_server_views_eet)
{
    return elm_app_server_views_eet->version;
}

inline void
elm_app_server_views_eet_version_set(Elm_App_Server_Views_Eet *elm_app_server_views_eet, unsigned int version)
{
    EINA_SAFETY_ON_NULL_RETURN(elm_app_server_views_eet);
    elm_app_server_views_eet->version = version;
}

inline void
elm_app_server_views_eet_views_entries_add(Elm_App_Server_Views_Eet *elm_app_server_views_eet, Elm_App_Server_View_Props *elm_app_server_view_props)
{
    EINA_SAFETY_ON_NULL_RETURN(elm_app_server_views_eet);
    elm_app_server_views_eet->views_entries = eina_list_append(elm_app_server_views_eet->views_entries, elm_app_server_view_props);
}

inline void
elm_app_server_views_eet_views_entries_del(Elm_App_Server_Views_Eet *elm_app_server_views_eet, Elm_App_Server_View_Props *elm_app_server_view_props)
{
    EINA_SAFETY_ON_NULL_RETURN(elm_app_server_views_eet);
    elm_app_server_views_eet->views_entries = eina_list_remove(elm_app_server_views_eet->views_entries, elm_app_server_view_props);
}

inline Elm_App_Server_View_Props *
elm_app_server_views_eet_views_entries_get(const Elm_App_Server_Views_Eet *elm_app_server_views_eet, unsigned int nth)
{
    EINA_SAFETY_ON_NULL_RETURN_VAL(elm_app_server_views_eet, NULL);
    return eina_list_nth(elm_app_server_views_eet->views_entries, nth);
}

inline unsigned int
elm_app_server_views_eet_views_entries_count(const Elm_App_Server_Views_Eet *elm_app_server_views_eet)
{
    EINA_SAFETY_ON_NULL_RETURN_VAL(elm_app_server_views_eet, 0);
    return eina_list_count(elm_app_server_views_eet->views_entries);
}

void
elm_app_server_views_eet_views_entries_list_clear(Elm_App_Server_Views_Eet *elm_app_server_views_eet)
{
    EINA_SAFETY_ON_NULL_RETURN(elm_app_server_views_eet);
    Elm_App_Server_View_Props *data;
    EINA_LIST_FREE(elm_app_server_views_eet->views_entries, data) elm_app_server_view_props_free(data);
}

inline Eina_List *
elm_app_server_views_eet_views_entries_list_get(const Elm_App_Server_Views_Eet *elm_app_server_views_eet)
{
    EINA_SAFETY_ON_NULL_RETURN_VAL(elm_app_server_views_eet, NULL);
    return elm_app_server_views_eet->views_entries;
}

inline void
elm_app_server_views_eet_views_entries_list_set(Elm_App_Server_Views_Eet *elm_app_server_views_eet, Eina_List *list)
{
    EINA_SAFETY_ON_NULL_RETURN(elm_app_server_views_eet);
    elm_app_server_views_eet->views_entries = list;
}

Elm_App_Server_Views_Eet *
elm_app_server_views_eet_load(const char *filename)
{
    Elm_App_Server_Views_Eet *elm_app_server_views_eet = NULL;
    Eet_File *ef = eet_open(filename, EET_FILE_MODE_READ);
    if (!ef)
      {
        fprintf(stderr, "ERROR: could not open '%s' for read\n", filename);
        return NULL;
      }

    elm_app_server_views_eet = eet_data_read(ef, _elm_app_server_views_eet_descriptor, ELM_APP_SERVER_VIEWS_EET_ENTRY);
    if (!elm_app_server_views_eet) goto end;
    elm_app_server_views_eet->__eet_filename = eina_stringshare_add(filename);

end:
    eet_close(ef);
    return elm_app_server_views_eet;
}

Eina_Bool
elm_app_server_views_eet_save(Elm_App_Server_Views_Eet *elm_app_server_views_eet, const char *filename)
{
    Eet_File *ef;
    Eina_Bool ret;

    if (filename) eina_stringshare_replace(&(elm_app_server_views_eet->__eet_filename), filename);
    else if (elm_app_server_views_eet->__eet_filename) filename = elm_app_server_views_eet->__eet_filename;
    else return EINA_FALSE;

    ef = eet_open(filename, EET_FILE_MODE_READ_WRITE);
    if (!ef)
       {
          fprintf(stderr, "ERROR: could not open '%s' for write\n", filename);
          return EINA_FALSE;
       }

    ret = !!eet_data_write(ef, _elm_app_server_views_eet_descriptor, ELM_APP_SERVER_VIEWS_EET_ENTRY, elm_app_server_views_eet, EINA_TRUE);
    eet_close(ef);

    return ret;
}

void
app_server_views_eet_init(void)
{
    _elm_app_server_view_props_init();
    _elm_app_server_views_eet_init();
}

void
app_server_views_eet_shutdown(void)
{
    _elm_app_server_view_props_shutdown();
    _elm_app_server_views_eet_shutdown();
}
