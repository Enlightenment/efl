elput_deps = [ecore, ecore_input, eeze, eo, m]
elput_pub_deps = [eina, eldbus]
elput_ext_deps = []

elput_header_src = [
  'Elput.h'
]

elput_src = files([
  'elput_touch.c',
  'elput_evdev.c',
  'elput_input.c',
  'elput_logind.c',
  'elput_root.c',
  'elput_manager.c',
  'elput_gestures.c',
  'elput.c',
  'elput_private.h'
])

elput_ext_deps += dependency('libinput', version : ['>=1.7.0'])
elput_ext_deps += dependency('xkbcommon', version : ['>=0.3.0'])
elput_ext_deps += dependency('libudev')

elput_lib = library('elput',
    elput_src, pub_eo_file_target,
    c_args : package_c_args,
    dependencies: elput_pub_deps + elput_deps + elput_ext_deps,
    include_directories : config_dir,
    install: true,
    version : meson.project_version()
)

elput = declare_dependency(
  include_directories: [include_directories('.')],
  link_with: elput_lib,
  sources : pub_eo_file_target + priv_eo_file_target,
  dependencies: elput_pub_deps,
)

install_headers(elput_header_src,
  install_dir : dir_package_include,
)
