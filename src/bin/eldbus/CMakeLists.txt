set(OUTPUT_NAME eldbus-codegen)

set(LIBRARIES
  eina
  ecore
)

set(SOURCES
  client.c
  codegen.h
  dbus.c
  parser.c
  source_client.c
  utils.c
)
