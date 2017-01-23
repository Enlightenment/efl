set(DESCRIPTION "A library to parse .eo files ")

set(LIBRARIES
  eina
)

#if (WINDOWS)
# set(LIBRARIES ${LIBRARIES} evil)
#endif (WINDOWS)

set(PUBLIC_HEADERS
  Eolian.h
)

set(INCLUDE_DIRECTORIES
  ./
)

set(SOURCES
  database_class.c
  database_class_api.c
  database_constructor.c
  database_constructor_api.c
  database_event.c
  database_event_api.c
  database_expr.c
  database_expr_api.c
  database_function.c
  database_function_api.c
  database_function_parameter.c
  database_function_parameter_api.c
  database_implement.c
  database_implement_api.c
  database_type.c
  database_type_api.c
  database_validate.c
  database_var.c
  database_var_api.c
  eo_lexer.c
  eo_lexer.h
  eo_parser.c
  eo_parser.h
  eolian.c
  eolian_database.c
  eolian_database.h
)
