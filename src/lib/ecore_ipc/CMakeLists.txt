set(DESCRIPTION "E core library, IPC module")

set(LIBRARIES
  ecore
  ecore_con
  efl
  eina
  eo
)

set(PUBLIC_HEADERS
  Ecore_Ipc.h
)

set(SOURCES
  ecore_ipc.c
  ecore_ipc_private.h
)

