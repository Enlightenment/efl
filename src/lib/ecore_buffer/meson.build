ecore_buffer_deps = [dependency('wayland-client'), dependency('wayland-server'), ecore]
ecore_buffer_pub_deps = [eina]

ecore_buffer_header_src = [
  'Ecore_Buffer.h',
  'Ecore_Buffer_Queue.h'
]

ecore_buffer_src = [
  'bq_mgr_protocol.c',
  'buffer_queue.c',
  'shared_buffer.c',
  'ecore_buffer.c',
  'ecore_buffer_queue_main.c',
  'ecore_buffer_con.c',
  'ecore_buffer_provider.c',
  'ecore_buffer_consumer.c'
]

ecore_buffer_lib = library('ecore_buffer',
    ecore_buffer_src, pub_eo_file_target,
    dependencies: ecore_buffer_pub_deps + ecore_buffer_deps,
    include_directories : config_dir,
    install: true,
    version : meson.project_version()
)

ecore_buffer = declare_dependency(
  include_directories: [include_directories('.')],
  link_with: ecore_buffer_lib,
  sources : pub_eo_file_target + priv_eo_file_target,
  dependencies: ecore_buffer_pub_deps,
)

install_headers(ecore_buffer_header_src,
  install_dir : dir_package_include,
)
