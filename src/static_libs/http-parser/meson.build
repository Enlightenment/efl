http_parser_lib = [
  'http_parser.c',
  'http_parser.h',
]

http_parser_lib = static_library('http-parser', http_parser_lib)

http_parser = declare_dependency(
	include_directories: include_directories('.') ,
	link_with: http_parser_lib,
)