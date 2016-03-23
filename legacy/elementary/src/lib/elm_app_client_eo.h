typedef Eo Elm_App_Client;
#define _ELM_APP_CLIENT_EO_CLASS_TYPE

typedef Eo Elm_App_Client_View;
#define _ELM_APP_CLIENT_VIEW_EO_CLASS_TYPE

typedef Eldbus_Pending Elm_App_Client_Pending;

typedef void (*Elm_App_Client_Open_View_Cb)(void *data, Elm_App_Client_View *view, const char *error, const char *error_message);

#include "elm_app_client.eo.h"