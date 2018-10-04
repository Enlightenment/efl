eeze_deps = [buildsystem]
eeze_pub_deps = [eina, efl]
pub_eo_file_target = []

eeze_header_src = [
  'Eeze.h',
  'Eeze_Net.h',
  'Eeze_Sensor.h',
  'Eeze_Disk.h',
  join_paths('..','..','bin','eeze','eeze_scanner','eeze_scanner.h')
]

eeze_src = [
  'eeze_main.c',
  'eeze_net.c',
  'eeze_net_private.h',
  'eeze_sensor.c',
  'eeze_sensor_private.h',
  'eeze_udev_find.c',
  'eeze_udev_private.c',
  'eeze_udev_private.h',
  'eeze_udev_syspath.c',
  'eeze_udev_walk.c',
  'eeze_udev_watch.c',
  'eeze_disk.c',
  'eeze_disk_udev.c',
  'eeze_disk_mount.c',
  'eeze_disk_private.h'
]

if get_option('v4l2')
  config_h.set('HAVE_V4L2', '1')
  if cc.has_header('linux/videodev2.h') == false
    error('v4l2 headers not found')
  endif
endif

if (get_option('libmount'))
  libmount = dependency('mount')
  eeze_deps += [libmount]
 if libmount.version() == '2.19'
    eeze_src +=   'eeze_disk_libmount.c'
  elif libmount.version() < '2.19'
    eeze_src +=   'eeze_disk_libmount_old.c'
  elif libmount.version() > '2.19'
    eeze_src +=   'eeze_disk_libmount_new.c'
  endif
  config_h.set('HAVE_EEZE_MOUNT', '1')
else
  eeze_src +=   'eeze_disk_dummy.c'
endif

options_to_check = ['eject', 'mount', 'unmount']

eeze_deps += dependency('libudev',
  version: ['>=148']
)

foreach option : options_to_check
  val = get_option(option+'-path')
  if val == 'detect'
    # i dont know why we call this unmount when the bin is called umount
    if option == 'unmount'
      _option = 'umount'
    else
      _option = option
    endif
    prog = find_program(_option, required: false)
    if prog.found()
      set_variable(option+'_bin', prog.path())
    else
      set_variable(option+'_bin', '')
    endif
  else
    set_variable(option+'_bin', val)
  endif
endforeach

config_h.set_quoted('EEZE_EJECT_BIN', eject_bin)
config_h.set_quoted('EEZE_MOUNT_BIN', mount_bin)
config_h.set_quoted('EEZE_UNMOUNT_BIN', unmount_bin)

eeze_lib = library('eeze',
    eeze_src, pub_eo_file_target,
    dependencies: eeze_deps + eeze_pub_deps + [ecore, ecore_file, buildsystem, eeze_deps],
    include_directories : config_dir,
    install: true,
    c_args : package_c_args,
    version : meson.project_version()
)

eeze = declare_dependency(
  include_directories: [include_directories('.')],
  link_with: eeze_lib,
  sources : pub_eo_file_target + priv_eo_file_target,
  dependencies: eeze_pub_deps,
)

install_headers(eeze_header_src,
  install_dir : dir_package_include,
)
