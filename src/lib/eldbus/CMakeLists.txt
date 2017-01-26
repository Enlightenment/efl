set(DESCRIPTION "D-Bus access from Ecore")

set(PKG_CONFIG_REQUIRES_PRIVATE
  dbus-1
)

set(LIBRARIES
  eina
  eo
  efl
  ecore
)

set(PUBLIC_EO_FILES
  eldbus_model_arguments.eo
  eldbus_model_connection.eo
  eldbus_model_method.eo
  eldbus_model_object.eo
  eldbus_model_proxy.eo
  eldbus_model_signal.eo
  eldbus_types.eot
)

set(PUBLIC_HEADERS
  Eldbus.h
  eldbus_connection.h
  eldbus_freedesktop.h
  eldbus_introspection.h
  eldbus_message.h
  eldbus_message_eina_value.h
  eldbus_message_helper.h
  Eldbus_Model.h
  eldbus_object.h
  eldbus_pending.h
  eldbus_proxy.h
  eldbus_service.h
  eldbus_signal_handler.h
)

set(SOURCES
  eldbus_core.c
  eldbus_freedesktop.c
  eldbus_introspection.c
  eldbus_message.c
  eldbus_message_from_eina_value.c
  eldbus_message_helper.c
  eldbus_message_to_eina_value.c
  eldbus_model_arguments.c
  eldbus_model_arguments_private.h
  eldbus_model_connection.c
  eldbus_model_connection_private.h
  eldbus_model_method.c
  eldbus_model_method_private.h
  eldbus_model_object.c
  eldbus_model_object_private.h
  eldbus_model_private.h
  eldbus_model_proxy.c
  eldbus_model_proxy_private.h
  eldbus_model_signal.c
  eldbus_model_signal_private.h
  eldbus_object.c
  eldbus_pending.c
  eldbus_private.h
  eldbus_private_types.h
  eldbus_proxy.c
  eldbus_service.c
  eldbus_signal_handler.c
)

set(COMPILE_FLAGS "-include ${CMAKE_BINARY_DIR}/src/lib/efl/config.h")
