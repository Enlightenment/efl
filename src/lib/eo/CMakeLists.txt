set(PUBLIC_LIBRARIES
  eina
)

set(PUBLIC_HEADERS
  Eo.h
  efl_future.h
)

set(PUBLIC_EO_FILES
  efl_object.eo
  efl_class.eo
  efl_object_override.eo
  efl_interface.eo
)

set(SOURCES
  eo.c
  eo_ptr_indirection.c
  eo_ptr_indirection.h
  eo_base_class.c
  eo_class_class.c
  eo_add_fallback.c
  eo_add_fallback.h
  eo_private.h
  efl_future.c
)
