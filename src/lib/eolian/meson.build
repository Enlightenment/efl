eolian_src = [
'eo_lexer.c',
'eo_lexer.h',
'eo_parser.c',
'eo_parser.h',
'eolian.c',
'eolian_priv.h',
'eolian_database.c',
'eolian_database.h',
'database_check.c',
'database_class.c',
'database_class_api.c',
'database_function.c',
'database_function_api.c',
'database_function_parameter.c',
'database_function_parameter_api.c',
'database_type.c',
'database_type_api.c',
'database_implement.c',
'database_implement_api.c',
'database_constructor.c',
'database_constructor_api.c',
'database_event.c',
'database_event_api.c',
'database_part.c',
'database_part_api.c',
'database_expr.c',
'database_expr_api.c',
'database_var.c',
'database_var_api.c',
'database_validate.c'
]

eolian_lib = library('eolian', eolian_src,
    include_directories: config_dir,
    dependencies: eina,
    install: true,
    c_args : package_c_args,
)

eolian = declare_dependency(
  include_directories: include_directories('.'),
  link_with : eolian_lib,
  dependencies: eina,
)

eolian_include_dir = join_paths(dir_data, 'eolian', 'include')

install_headers('Eolian.h',
   install_dir : dir_package_include
)
