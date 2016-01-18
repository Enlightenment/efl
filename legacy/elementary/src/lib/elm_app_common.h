
char *_dbus_package_to_path(const char *package);

Elm_App_View_State _string_state_to_id(const char *state);

const char *_state_id_to_string(Elm_App_View_State state);

void elm_app_client_view_internal_state_set(Eo *eo, Elm_App_View_State state);

