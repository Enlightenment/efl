set(DESCRIPTION "Ecore event abstraction library")

set(PUBLIC_LIBRARIES
  eina
  ecore
)

set(LIBRARIES
  eo
  efl
  ecore_con
  ecore_file
  eet
  emile
)

set(PKG_CONFIG_REQUIRES_PRIVATE
   libudev
)


if (${HAVE_EEZE_MOUNT})
  list(APPEND PKG_CONFIG_REQUIRES_PRIVATE mount)
endif()

if (${ENABLE_TIZEN})
  list(APPEND PKG_CONFIG_REQUIRES_PRIVATE capi-system-sensor)
endif()


set(PUBLIC_HEADERS
  Eeze.h
  Eeze_Disk.h
  Eeze_Net.h
  Eeze_Sensor.h
)

set(SOURCES
  eeze_disk.c
  eeze_disk_mount.c
  eeze_disk_private.h
  eeze_disk_udev.c
  eeze_main.c
  eeze_net.c
  eeze_net_private.h
  eeze_sensor.c
  eeze_sensor_private.h
  eeze_udev_find.c
  eeze_udev_private.c
  eeze_udev_private.h
  eeze_udev_syspath.c
  eeze_udev_walk.c
  eeze_udev_watch.c
)

#mount files
if(${HAVE_EEZE_MOUNT})
  if (${MOUNT_VERSION} LESS 2.19.0)
    list(APPEND SOURCES eeze_disk_libmount_old.c)
  elseif (${MOUNT_VERSION} GREATER 2.19.0)
    list(APPEND SOURCES eeze_disk_libmount_new.c)
  else()
    list(APPEND SOURCES eeze_disk_libmount.c)
  endif()
else()
  list(APPEND SOURCES eeze_disk_dummy.c)
endif()
