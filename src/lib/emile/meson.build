emile_deps = []
emile_pub_deps = [eina, efl]
emile_ext_deps = [jpeg, crypto, dependency('zlib'), lz4, rg_etc, m]

emile_headers = [
  'Emile.h',
  'emile_cipher.h',
  'emile_compress.h',
  'emile_image.h',
  'emile_base64.h'
]

emile_src = files([
  'emile_private.h',
  'emile_main.c',
  'emile_compress.c',
  'emile_image.c',
  'emile_base64.c',
])

if (get_option('crypto') == 'gnutls')
  emile_src += files('emile_cipher_gnutls.c')
elif (get_option('crypto') == 'openssl')
  emile_src += files('emile_cipher_openssl.c')
endif

emile_lib = library('emile',
    emile_src,
    c_args : package_c_args,
    include_directories: config_dir,
    dependencies: emile_pub_deps + emile_deps + emile_ext_deps,
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
