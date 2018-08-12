ecore_buffer_deps = []

ecore_buffer_header_src = [
  'Ecore_Buffer.h',
  'Ecore_Buffer_Queue.h'
]

wayland = []
wayland += dependency('wayland-client')
wayland += dependency('wayland-server')

ecore_buffer_deps += wayland

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
    dependencies: [ecore, ecore_buffer_deps],
    include_directories : config_dir,
    install: true,
)

ecore_buffer = declare_dependency(
  include_directories: [include_directories('.')],
  link_with: ecore_buffer_lib,
  sources : pub_eo_file_target,
  dependencies: [eina],
)

install_headers(ecore_buffer_header_src,
  install_dir : dir_package_include,
)