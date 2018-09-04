int argc = 2;
char *argv[] = { "efl_ui_suite", "test" };
(void)parent;

_EFL_APP_VERSION_SET();
obj = efl_app_get();
efl_event_callback_add(obj, EFL_LOOP_EVENT_ARGUMENTS, efl_main, NULL);
fail_if(!ecore_init_ex(argc, argv));
__EFL_MAIN_CONSTRUCTOR;
