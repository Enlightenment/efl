eldbus_deps = [ecore]
eldbus_pub_deps = [eina, eo, efl]

pub_eo_files = [
  'eldbus_model_connection.eo',
  'eldbus_model_object.eo',
  'eldbus_model_proxy.eo',
  'eldbus_model_method.eo',
  'eldbus_model_arguments.eo',
  'eldbus_model_signal.eo',
  'eldbus_model.eo'
]

foreach eo_file : pub_eo_files
  pub_eo_file_target += custom_target('eolian_gen_' + eo_file,
    input : eo_file,
    output : [eo_file + '.h'],
    install : true,
    install_dir : dir_package_include,
    command : [eolian_gen, '-I', meson.current_source_dir(), eolian_include_directories,
                           '-o', 'h:' + join_paths(meson.current_build_dir(), eo_file + '.h'),
                           '-o', 'c:' + join_paths(meson.current_build_dir(), eo_file + '.c'),
                           '-gch', '@INPUT@'])
endforeach


pub_eo_types_files = [
  'eldbus_types.eot'
]

foreach eo_file : pub_eo_types_files
  pub_eo_file_target += custom_target('eolian_gen_' + eo_file,
    input : eo_file,
    output : [eo_file + '.h'],
    install : true,
    install_dir : dir_package_include,
    command : [eolian_gen, '-I', meson.current_source_dir(), eolian_include_directories,
                           '-o', 'h:' + join_paths(meson.current_build_dir(), eo_file + '.h'),
                           '-gh', '@INPUT@'])
endforeach

eolian_include_directories += ['-I', meson.current_source_dir()]

eldbus_header_src = [
  'Eldbus.h',
  'eldbus_connection.h',
  'eldbus_freedesktop.h',
  'eldbus_message.h',
  'eldbus_object.h',
  'eldbus_pending.h',
  'eldbus_proxy.h',
  'eldbus_service.h',
  'eldbus_signal_handler.h',
  'eldbus_message_helper.h',
  'eldbus_introspection.h',
  'Eldbus_Model.h',
  'eldbus_message_eina_value.h'
]

eldbus_src = [
  'eldbus_private.h',
  'eldbus_private_types.h',
  'eldbus_model_private.h',
  'eldbus_model_proxy_private.h',
  'eldbus_model_object_private.h',
  'eldbus_model_arguments_private.h',
  'eldbus_model_connection_private.h',
  'eldbus_model_signal_private.h',
  'eldbus_model_method_private.h',
  'eldbus_proxy.c',
  'eldbus_core.c',
  'eldbus_message.c',
  'eldbus_object.c',
  'eldbus_pending.c',
  'eldbus_freedesktop.c',
  'eldbus_service.c',
  'eldbus_signal_handler.c',
  'eldbus_message_helper.c',
  'eldbus_message_to_eina_value.c',
  'eldbus_message_from_eina_value.c',
  'eldbus_model.c',
  'eldbus_model_connection.c',
  'eldbus_model_object.c',
  'eldbus_model_proxy.c',
  'eldbus_model_method.c',
  'eldbus_model_arguments.c',
  'eldbus_model_signal.c',
  'eldbus_introspection.c'
]

eldbus_deps += dependency('dbus-1')

eldbus_lib = library('eldbus',
    eldbus_src, pub_eo_file_target,
    dependencies: eldbus_pub_deps + eldbus_deps,
    include_directories : config_dir,
    install: true,
    version : meson.project_version()
)

eldbus = declare_dependency(
  include_directories: [include_directories('.')],
  link_with: eldbus_lib,
  sources : pub_eo_file_target + priv_eo_file_target,
  dependencies: eldbus_pub_deps,
)

install_data(pub_eo_files + pub_eo_types_files,
  install_dir: join_paths(eolian_include_dir, package_version_name)
)

install_headers(eldbus_header_src,
  install_dir : dir_package_include,
)
