
bindings = get_option('bindings')
bindings_order = ['luajit', 'cxx', 'mono']

if (bindings.contains('cxx') == false and bindings.contains('mono'))
  subdirs = ['eina_cxx', 'eolian_cxx']
  subdir(join_paths('cxx', 'eina_cxx'))
  inc_dir = []
  subdir(join_paths('cxx', 'eolian_cxx'))
  eolian_cxx = declare_dependency(
    include_directories: inc_dir,
    dependencies : eolian
  )
endif

foreach binding : bindings_order
  if bindings.contains(binding)
    subdir(join_paths( binding))
  endif
endforeach
