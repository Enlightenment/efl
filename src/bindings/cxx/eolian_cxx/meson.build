
name_eolian_cxx_header_src= [
  'Eolian_Cxx.hh'
]

name_grammer_eolian_cxx_header_src= [
  'address_of.hpp',
  'alternative.hpp',
  'attribute_conditional.hpp',
  'attribute_reorder.hpp',
  'attribute_replace.hpp',
  'attributes.hpp',
  'blacklist.hpp',
  'base_class_definition.hpp',
  'case.hpp',
  'class_declaration.hpp',
  'class_definition.hpp',
  'class_implementation.hpp',
  'container.hpp',
  'context.hpp',
  'converting_argument.hpp',
  'c_type.hpp',
  'eps.hpp',
  'function_declaration.hpp',
  'function_definition.hpp',
  'generator.hpp',
  'header_guards.hpp',
  'header.hpp',
  'header_include_directive.hpp',
  'html_escaped_string.hpp',
  'implementation_include_directive.hpp',
  'impl_header.hpp',
  'indentation.hpp',
  'integral.hpp',
  'keyword.hpp',
  'klass_def.hpp',
  'kleene.hpp',
  'list.hpp',
  'meta.hpp',
  'namespace.hpp',
  'parameter.hpp',
  'part_declaration.hpp',
  'part_implementation.hpp',
  'qualifier_def.hpp',
  'sequence.hpp',
  'string.hpp',
  'type_function_declaration.hpp',
  'type.hpp',
  'type_impl.hpp',
  'types_definition.hpp',
  'type_traits.hpp'
]

cxx_header_src = []

foreach header : name_eolian_cxx_header_src
  cxx_header_src += files(join_paths('..', '..', '..', 'lib', 'eolian_cxx', header))
endforeach

foreach header : name_grammer_eolian_cxx_header_src
  cxx_header_src += files(join_paths('..', '..', '..', 'lib', 'eolian_cxx', 'grammar', header))
endforeach

inc_dir = include_directories(join_paths('..', '..', '..', 'lib', 'eolian_cxx'))

eolian_cxx_gen = executable('eolian_cxx',
                   join_paths('..', '..', '..', 'bin', 'eolian_cxx', 'eolian_cxx.cc'),
                   dependencies: [eina_cxx, eolian],
                   install: true,
                   include_directories : inc_dir,
                   cpp_args : package_c_args,
                 )
