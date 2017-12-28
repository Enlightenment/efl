if dependency('libtbm', required: false).found() == true and dependency('libdrm', required: false).found()
  ecore_buffer_x11_dri3_mod = shared_module('ecore_buffer_x11_dri3_mod',
    'ecore_buffer_x11_dri3.c',
    dependencies: [ecore, ecore_buffer, ecore_x],
    install: true
  )
endif