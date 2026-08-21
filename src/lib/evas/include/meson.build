evas_include_directories += [include_directories('.')]


pub_eo_files = [
  'evas_ector_buffer.eo'
]

foreach eo_file : pub_eo_files
  # Both generated files are declared, so ninja knows the .eo.c exists and puts
  # it back if it goes missing. Only the header goes into the target list: the
  # .eo.c is #included by a hand-written source, and meson would otherwise
  # compile it alone, where it cannot see the types it needs. Indexing keeps
  # the dependency on the whole target.
  _eo_ct = custom_target('eolian_gen_' + eo_file,
    input : eo_file,
    output : [eo_file + '.h', eo_file + '.c'],
    depfile : eo_file + '.d',
    command : eolian_gen + [ '-I', meson.current_source_dir(), eolian_include_directories,
                           '-o', 'h:' + join_paths(meson.current_build_dir(), eo_file + '.h'),
                           '-o', 'c:' + join_paths(meson.current_build_dir(), eo_file + '.c'),
                           '-o', 'D:' + join_paths(meson.current_build_dir(), eo_file + '.d'),
                           '-e', 'EVAS_API',
                           '-gchD', '@INPUT@'])
  pub_eo_file_target += _eo_ct[0]
endforeach

eolian_include_directories += ['-I', meson.current_source_dir()]
