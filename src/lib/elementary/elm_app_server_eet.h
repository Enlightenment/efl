#ifndef _ELM_APP_SERVER_EET_H_
#define _ELM_APP_SERVER_EET_H_

#include <Eina.h>
#include <Eet.h>

typedef struct _Elm_App_Server_View_Props Elm_App_Server_View_Props;
typedef struct _Elm_App_Server_Views_Eet Elm_App_Server_Views_Eet;

/* Elm_App_Server_View_Props */
Elm_App_Server_View_Props *elm_app_server_view_props_new(const char * id, const char * title, const char * icon_name, int new_events, char progress);
void elm_app_server_view_props_free(Elm_App_Server_View_Props *elm_app_server_view_props);

void elm_app_server_view_props_id_set(Elm_App_Server_View_Props *elm_app_server_view_props, const char * id);
const char * elm_app_server_view_props_id_get(const Elm_App_Server_View_Props *elm_app_server_view_props);
void elm_app_server_view_props_title_set(Elm_App_Server_View_Props *elm_app_server_view_props, const char * title);
const char * elm_app_server_view_props_title_get(const Elm_App_Server_View_Props *elm_app_server_view_props);
void elm_app_server_view_props_icon_name_set(Elm_App_Server_View_Props *elm_app_server_view_props, const char * icon_name);
const char * elm_app_server_view_props_icon_name_get(const Elm_App_Server_View_Props *elm_app_server_view_props);
void elm_app_server_view_props_new_events_set(Elm_App_Server_View_Props *elm_app_server_view_props, int new_events);
int elm_app_server_view_props_new_events_get(const Elm_App_Server_View_Props *elm_app_server_view_props);
void elm_app_server_view_props_progress_set(Elm_App_Server_View_Props *elm_app_server_view_props, char progress);
char elm_app_server_view_props_progress_get(const Elm_App_Server_View_Props *elm_app_server_view_props);

/* Elm_App_Server_Views_Eet */
Elm_App_Server_Views_Eet *elm_app_server_views_eet_new(unsigned int version, Eina_List * views_entries);
void elm_app_server_views_eet_free(Elm_App_Server_Views_Eet *elm_app_server_views_eet);

void elm_app_server_views_eet_version_set(Elm_App_Server_Views_Eet *elm_app_server_views_eet, unsigned int version);
unsigned int elm_app_server_views_eet_version_get(const Elm_App_Server_Views_Eet *elm_app_server_views_eet);
void elm_app_server_views_eet_views_entries_add(Elm_App_Server_Views_Eet *elm_app_server_views_eet, Elm_App_Server_View_Props *elm_app_server_view_props);
void elm_app_server_views_eet_views_entries_del(Elm_App_Server_Views_Eet *elm_app_server_views_eet, Elm_App_Server_View_Props *elm_app_server_view_props);
Elm_App_Server_View_Props *elm_app_server_views_eet_views_entries_get(const Elm_App_Server_Views_Eet *elm_app_server_views_eet, unsigned int nth);
unsigned int elm_app_server_views_eet_views_entries_count(const Elm_App_Server_Views_Eet *elm_app_server_views_eet);
Eina_List *elm_app_server_views_eet_views_entries_list_get(const Elm_App_Server_Views_Eet *elm_app_server_views_eet);
void elm_app_server_views_eet_views_entries_list_clear(Elm_App_Server_Views_Eet *elm_app_server_views_eet);
void elm_app_server_views_eet_views_entries_list_set(Elm_App_Server_Views_Eet *elm_app_server_views_eet, Eina_List *list);

Elm_App_Server_Views_Eet *elm_app_server_views_eet_load(const char *filename);
Eina_Bool elm_app_server_views_eet_save(Elm_App_Server_Views_Eet *elm_app_server_views_eet, const char *filename);

/* Global initializer / shutdown functions */
void app_server_views_eet_init(void);
void app_server_views_eet_shutdown(void);

#endif
