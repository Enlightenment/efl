set(DESCRIPTION "E core library, File module")

set(LIBRARIES
  ecore
  ecore_con
  efl
  eina
  eo
)

set(PUBLIC_HEADERS
  Ecore_File.h
)

set(SOURCES
  ecore_file.c
  ecore_file_download.c
  ecore_file_monitor.c
  ecore_file_path.c
  ecore_file_private.h
)

if(HAVE_SYS_INOTIFY_H)
  list(APPEND SOURCES ecore_file_monitor_inotify.c)
elseif(WINDOWS)
  list(APPEND SOURCES ecore_file_monitor_win32.c)
else()
  list(APPEND SOURCES ecore_file_monitor_poll.c)
endif()
