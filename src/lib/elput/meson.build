elput_deps = [ecore, ecore_input, eeze, eo, m]
elput_pub_deps = [eina, eldbus]
pub_eo_file_target = []

elput_header_src = [
  'Elput.h'
]

elput_src = [
  'elput_touch.c',
  'elput_evdev.c',
  'elput_input.c',
  'elput_logind.c',
  'elput_manager.c',
  'elput.c',
  'elput_private.h'
]

elput_deps += dependency('libinput', version : ['>=1.7.0'])
elput_deps += dependency('xkbcommon', version : ['>=0.3.0'])
elput_deps += dependency('libudev')
if get_option('systemd')
  elput_deps += systemd
endif

elput_lib = library('elput',
    elput_src, pub_eo_file_target,
    dependencies: elput_pub_deps + elput_deps,
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
