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

emile_dep = []
emile_dep += jpeg
emile_dep += dependency('zlib')

if (get_option('crypto') == 'gnutls')
  gnutls = dependency('gnutls')
  emile_dep += gnutls
  emile_src += 'emile_cipher_gnutls.c'
  config_h.set('HAVE_GNUTLS', '1')
elif (get_option('crypto') == 'openssl')
  openssl = dependency('openssl')
  emile_dep += openssl
  emile_src += 'emile_cipher_openssl.c'
  config_h.set('HAVE_OPENSSL', '1')
else
  emile_src += 'emile_cipher.c'
  config_h.set('HAVE_CIPHER', '1')
  message('No cipher selected, testsuite will not complete')
endif

emile_lib = library('emile',
    emile_src,
    include_directories: config_dir,
    dependencies: [eina, lz4, rg_etc, m, efl, emile_dep],
    install: true,
)

emile = declare_dependency(
  include_directories: include_directories('.'),
  link_with: emile_lib,
  dependencies: [eina, efl],
)

install_headers(emile_headers,
  install_dir: dir_package_include
)
