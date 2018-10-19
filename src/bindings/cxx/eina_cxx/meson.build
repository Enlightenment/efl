eina_cxx_deps = [eina, eo]
eina_cxx_pub_deps = []

eina_cxx_header_src = files(
  'eina_accessor.hh',
  'eina_aligned_union.hh',
  'eina_array.hh',
  'eina_clone_allocators.hh',
  'eina_deleter.hh',
  'eina_error.hh',
  'eina_future.hh',
  'eina_eo_concrete_fwd.hh',
  'eina_fold.hh',
  'eina_function.hh',
  'eina_inarray.hh',
  'eina_inlist.hh',
  'eina_integer_sequence.hh',
  'eina_iterator.hh',
  'eina_lists_auxiliary.hh',
  'eina_list.hh',
  'eina_log.hh',
  'eina_logical.hh',
  'eina_optional.hh',
  'eina_pp.hh',
  'eina_ptrarray.hh',
  'eina_ptrlist.hh',
  'eina_range_types.hh',
  'eina_ref.hh',
  'eina_stringshare.hh',
  'eina_strbuf.hh',
  'eina_string_view.hh',
  'eina_thread.hh',
  'eina_throw.hh',
  'eina_tuple.hh',
  'eina_tuple_c.hh',
  'eina_tuple_unwrap.hh',
  'eina_type_traits.hh',
  'eina_value.hh',
  'eina_workarounds.hh',
  'eina_copy_traits.hh',
  'eina_variant.hh',
  'Eina.hh'
)

eina_cxx_lib = disabler()

eina_cxx = declare_dependency(
  include_directories: [include_directories('.')],
  dependencies: eina_cxx_deps
)

install_headers(eina_cxx_header_src,
  subdir: dir_package_include
)
