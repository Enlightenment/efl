#define ELM_APP_CLIENT_VIEW_OPEN_CANCELED "org.enlightenment.Application.ViewOpenCanceled"
#define ELM_APP_CLIENT_VIEW_OPEN_TIMEOUT "org.enlightenment.Application.ViewOpenTimeout"
#define ELM_APP_CLEINT_VIEW_OPEN_ERROR "org.enlightenment.Application.CouldNotCreateView"

#define ELM_APP_SERVER_VIEW_CREATE_DUPLICATE "org.enlightenment.Application.ViewDuplicate"

typedef Eo Elm_App_Server;
typedef Eo Elm_App_Server_View;

typedef Elm_App_Server_View *(*Elm_App_Server_Create_View_Cb)(Elm_App_Server *app, const Eina_Value *args, Eina_Stringshare **error_name, Eina_Stringshare **error_message);

#include "elm_app_server.eo.h"