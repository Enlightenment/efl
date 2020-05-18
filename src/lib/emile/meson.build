emile_deps = [jpeg, crypto, dependency('zlib')]
emile_pub_deps = [eina, efl]

emile_headers = [
  'Emile.h',
  'emile_cipher.h',
  'emile_compress.h',
  'emile_image.h',
  'emile_base64.h'
]

emile_src = [
  'emile_private.h',
  'emile_main.c',
  'emile_compress.c',
  'emile_image.c',
  'emile_base64.c',
]

if (get_option('crypto') == 'gnutls')
  emile_src += 'emile_cipher_gnutls.c'
elif (get_option('crypto') == 'openssl')
  emile_src += 'emile_cipher_openssl.c'
endif

emile_lib = library('emile',
    emile_src,
    include_directories: config_dir,
    dependencies: emile_pub_deps + emile_deps + [lz4, rg_etc, m],
    install: true,
    version : meson.project_version()
)

emile = declare_dependency(
  include_directories: include_directories('.'),
  link_with: emile_lib,
  dependencies: [eina, efl],
)

install_headers(emile_headers,
  install_dir: dir_package_include
)
