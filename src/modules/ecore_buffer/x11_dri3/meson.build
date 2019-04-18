if dependency('libtbm', required: false).found() == true and dependency('libdrm', required: false).found()
  ecore_buffer_x11_dri3_mod = shared_module('module',
    'ecore_buffer_x11_dri3.c',
    name_prefix: '',
    dependencies: [ecore, ecore_buffer, ecore_x],
    install: true,
    install_dir: join_paths(dir_lib, 'ecore_buffer', 'modules', 'x11_dri3', version_name)
  )
endif
