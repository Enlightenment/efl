

subdir('shm')

#common dri packages
libtbm = dependency('libtbm', required: false)

if libtbm.found() == true
  libdri2 = dependency('libdri2', required: false)
  if libdri2.found()
    subdir('x11_dri2')
  endif

  xshmfence = dependency('xshmfence', required: false)
  xcb = dependency('xcb', required: false)
  x11_xcb = dependency('x11_xcb', required: false)
  xcb_sync = dependency('xcb_sync', required: false)
  xcb_dri3 = dependency('xkb_dri3', required: false)
  if xshmfence.found() == true and xcb.found() == true and x11_xcb.found() and xcb_sync.found() and xkb_dri3.found()
    subdir('x11_dri3')
  endif
endif
