
#check what versions of mount
pkg_check_modules(MOUNT mount)
EFL_OPTION(HAVE_EEZE_MOUNT "Whenever to use libmount" ${MOUNT_FOUND} DEPENDS MOUNT_FOUND OFF)
CHECK_APPEND_DEFINE(HAVE_EEZE_MOUNT ${HAVE_EEZE_MOUNT})

pkg_check_modules(UDEV REQUIRED udev>=148)

if(NOT ${UDEV_FOUND})
   error("Udev is required")
endif()

#todo tizen
if(ENABLED_TIZEN)
  pkg_check_modules(SENSORS REQUIRED capi-system-sensor>=0.1.17 )
endif()

set(EEZE_MODULE_TYPE_SENSOR_FAKE ON)
set(EEZE_MODULE_TYPE_SENSOR_TIZEN OFF)
set(EEZE_MODULE_TYPE_SENSOR_UDEV ON)

CHECK_APPEND_DEFINE(EEZE_MOUNT_BIN \"bla\")
CHECK_APPEND_DEFINE(EEZE_UNMOUNT_BIN \"bla\")
CHECK_APPEND_DEFINE(EEZE_EJECT_BIN \"bla\")
