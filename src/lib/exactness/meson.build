exactness_pub_deps = [eina, evas]
exactness_deps = []

exactness_src = [
  'Exactness.h',
  'exactness_private.h',
  'legacy_file.c',
  'unit.c'
]

exactness_lib = library('exactness', exactness_src,
  dependencies: [ eina, eet, evas ],
  install: true,
  version : meson.project_version()
)

exactness = declare_dependency(
  include_directories: include_directories('.'),
  link_with : exactness_lib,
  dependencies: [ eina, evas, eet ],
)

exactness_include_dir = join_paths(dir_data, 'exactness', 'include')

exactness_header_src = [
'Exactness.h',
]

install_headers(exactness_header_src,
   install_dir : dir_package_include
)
