eet_deps = []

eet_header_src = [
  'Eet.h'
]

eet_src = [
  'Eet_private.h',
  'eet_alloc.c',
  'eet_cipher.c',
  'eet_connection.c',
  'eet_data.c',
  'eet_dictionary.c',
  'eet_image.c',
  'eet_lib.c',
  'eet_node.c',
  'eet_utils.c',
]


if (get_option('crypto') == 'gnutls')
  eet_deps += gnutls
elif (get_option('crypto') == 'openssl')
  eet_deps += openssl
endif

eet_deps += jpeg
eet_deps += rg_etc

eet_lib = library('eet',
    eet_src, pub_eo_file_target,
    dependencies: [eet_deps, eina, emile, m],
    include_directories : config_dir,
    install: true,
)

eet = declare_dependency(
  include_directories: [include_directories('.')],
  link_with: eet_lib,
  sources : pub_eo_file_target,
  dependencies: [eina, eo, emile, efl],
)

install_headers(eet_header_src,
  install_dir : dir_package_include,
)